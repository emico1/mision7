// Mision 7

"use strict";

// Funciona como modulo de Node y como script del navegador.
(function (raiz, fabrica) {
  const modulo = fabrica();
  if (typeof module !== "undefined" && module.exports) module.exports = modulo;
  else raiz.Triangulacion = modulo;
})(typeof self !== "undefined" ? self : this, function () {
  // O(1)
  function distancia(a, b) {
    return Math.hypot(a[0] - b[0], a[1] - b[1]);
  }

  // O(1)
  function cruz(o, a, b) {
    return (a[0] - o[0]) * (b[1] - o[1]) - (a[1] - o[1]) * (b[0] - o[0]);
  }

  // Revisa que todos los giros tengan el mismo signo y que el poligono de una sola vuelta.
  // O(n)
  function esConvexo(p) {
    const n = p.length;
    if (n < 3) return false;
    let signo = 0;
    for (let i = 0; i < n; i++) {
      const c = cruz(p[i], p[(i + 1) % n], p[(i + 2) % n]);
      if (Math.abs(c) < 1e-12) return false;
      const s = c > 0 ? 1 : -1;
      if (signo === 0) signo = s;
      else if (s !== signo) return false;
    }
    let giro = 0;
    for (let i = 0; i < n; i++) {
      const a = p[i], b = p[(i + 1) % n], c = p[(i + 2) % n];
      let d = Math.atan2(c[1] - b[1], c[0] - b[0]) - Math.atan2(b[1] - a[1], b[0] - a[0]);
      while (d <= -Math.PI) d += 2 * Math.PI;
      while (d > Math.PI) d -= 2 * Math.PI;
      giro += d;
    }
    return Math.abs(Math.abs(giro) - 2 * Math.PI) < 1e-6;
  }

  // Los lados del perimetro no cuestan porque ya forman parte de la estructura.
  // O(1)
  function esLado(n, i, j) {
    const a = Math.min(i, j), b = Math.max(i, j);
    return b - a === 1 || (a === 0 && b === n - 1);
  }

  // Costo w(i,j) de la recurrencia, la longitud si es diagonal y 0 si es lado.
  // O(1)
  function pesoDiagonal(p, i, j) {
    return esLado(p.length, i, j) ? 0 : distancia(p[i], p[j]);
  }

  // O(1)
  function anguloEn(v, a, b) {
    const ax = a[0] - v[0], ay = a[1] - v[1], bx = b[0] - v[0], by = b[1] - v[1];
    return Math.atan2(Math.abs(ax * by - ay * bx), ax * bx + ay * by);
  }

  // O(1)
  function anguloMinimoTriangulo(a, b, c) {
    return (Math.min(anguloEn(a, b, c), anguloEn(b, c, a), anguloEn(c, a, b)) * 180) / Math.PI;
  }

  // En un poligono convexo dos diagonales se cruzan si un extremo de la segunda queda entre los de la primera y el otro no.
  // O(1)
  function seCruzan(a, b) {
    const i = Math.min(a[0], a[1]), j = Math.max(a[0], a[1]);
    const k = b[0], l = b[1];
    if (i === k || i === l || j === k || j === l) return false;
    return (k > i && k < j) !== (l > i && l < j);
  }

  // O(n)
  function sumaLongitudes(p, d) {
    return d.reduce((s, [i, j]) => s + distancia(p[i], p[j]), 0);
  }

  // O(n log n)
  function ordenar(d) {
    return d.sort((x, y) => x[0] - y[0] || x[1] - y[1]);
  }

  // O(n^3)
  function triangulosDe(n, d) {
    const ady = Array.from({ length: n }, () => new Uint8Array(n));
    for (let i = 0; i < n; i++) {
      const j = (i + 1) % n;
      ady[i][j] = ady[j][i] = 1;
    }
    for (const [i, j] of d) ady[i][j] = ady[j][i] = 1;
    const t = [];
    for (let a = 0; a < n; a++)
      for (let b = a + 1; b < n; b++)
        if (ady[a][b]) for (let c = b + 1; c < n; c++) if (ady[a][c] && ady[b][c]) t.push([a, b, c]);
    return t;
  }

  // O(n^3)
  function anguloMinimoDiseno(p, d) {
    return Math.min(...triangulosDe(p.length, d).map(([a, b, c]) => anguloMinimoTriangulo(p[a], p[b], p[c])));
  }

  const MENOR_MATERIAL = {
    base: 0, neutro: Infinity, optimiza: true,
    triangulo: (p, i, k, j) => pesoDiagonal(p, i, k) + pesoDiagonal(p, k, j),
    combinar: (a, b) => a + b, mejor: (a, b) => a < b,
  };

  const DIAGONAL_MAS_CORTA = {
    base: 0, neutro: Infinity, optimiza: true,
    triangulo: (p, i, k, j) => Math.max(pesoDiagonal(p, i, k), pesoDiagonal(p, k, j)),
    combinar: Math.max, mejor: (a, b) => a < b,
  };

  const ANGULO_MAXIMO = {
    base: Infinity, neutro: -Infinity, optimiza: true,
    triangulo: (p, i, k, j) => anguloMinimoTriangulo(p[i], p[k], p[j]),
    combinar: Math.min, mejor: (a, b) => a > b,
  };

  const CONTEO = {
    base: 1n, neutro: 0n, optimiza: false,
    triangulo: () => 1n, combinar: (a, b) => a * b, sumar: (a, b) => a + b,
  };

  // Programacion dinamica generica, igual que en C++. C[i][j] es el mejor valor para los vertices i a j.
  // Caso base C[i][i+1] y llenado por longitud del intervalo.
  // O(n^3) tiempo, O(n^2) espacio
  function resolverPD(p, c) {
    const n = p.length;
    const valor = Array.from({ length: n }, () => new Array(n).fill(c.base));
    const corte = Array.from({ length: n }, () => new Int32Array(n).fill(-1));
    for (let largo = 2; largo < n; largo++) {
      for (let i = 0; i + largo < n; i++) {
        const j = i + largo;
        let acumulado = c.neutro, elegido = -1;
        for (let k = i + 1; k < j; k++) {
          const v = c.combinar(c.combinar(valor[i][k], valor[k][j]), c.triangulo(p, i, k, j));
          if (c.optimiza) {
            if (elegido < 0 || c.mejor(v, acumulado)) {
              acumulado = v;
              elegido = k;
            }
          } else acumulado = c.sumar(acumulado, v);
        }
        valor[i][j] = acumulado;
        corte[i][j] = elegido;
      }
    }
    return { valor, corte };
  }

  // Recorre los cortes desde C[0][n-1] y agrega las diagonales de cada triangulo elegido.
  // O(n)
  function reconstruir(corte, n) {
    const d = [];
    if (n < 4) return d;
    const pila = [[0, n - 1]];
    while (pila.length) {
      const [i, j] = pila.pop();
      if (j - i < 2) continue;
      const k = corte[i][j];
      if (k - i >= 2) d.push([i, k]);
      if (j - k >= 2) d.push([k, j]);
      pila.push([i, k], [k, j]);
    }
    return ordenar(d);
  }

  // Solucion de la mision, misma recurrencia y mismo orden de suma que la version en C++.
  // O(n^3) tiempo, O(n^2) espacio
  function triangulacionMinima(p) {
    const n = p.length;
    const costo = Array.from({ length: n }, () => new Float64Array(n));
    const corte = Array.from({ length: n }, () => new Int32Array(n).fill(-1));
    const w = Array.from({ length: n }, (_, i) => Float64Array.from({ length: n }, (_, j) => pesoDiagonal(p, i, j)));
    for (let largo = 2; largo < n; largo++) {
      for (let i = 0; i + largo < n; i++) {
        const j = i + largo, filaI = costo[i], wI = w[i];
        let mejor = Infinity, elegido = -1;
        for (let k = i + 1; k < j; k++) {
          const v = (filaI[k] + costo[k][j]) + (wI[k] + w[k][j]);
          if (v < mejor) {
            mejor = v;
            elegido = k;
          }
        }
        costo[i][j] = mejor;
        corte[i][j] = elegido;
      }
    }
    return { costo: n >= 3 ? costo[0][n - 1] : 0, diagonales: reconstruir(corte, n), tabla: costo, corte };
  }

  // Lista las celdas en el orden en que se llenan, la usa el simulador.
  // O(n^3)
  function pasosPD(p) {
    const n = p.length;
    const { tabla, corte } = triangulacionMinima(p);
    const pasos = [];
    for (let largo = 2; largo < n; largo++)
      for (let i = 0; i + largo < n; i++) pasos.push({ i, j: i + largo, k: corte[i][i + largo], valor: tabla[i][i + largo] });
    return pasos;
  }

  // O(n^3)
  function disenoPorCriterio(p, c) {
    const { valor, corte } = resolverPD(p, c);
    const d = reconstruir(corte, p.length);
    return { valor: valor[0][p.length - 1], costo: sumaLongitudes(p, d), diagonales: d };
  }

  // Heuristica voraz, toma la diagonal mas corta que no cruce a las ya elegidas.
  // O(n^3)
  function triangulacionVoraz(p) {
    const n = p.length, candidatas = [];
    for (let i = 0; i < n; i++) for (let j = i + 2; j < n; j++) if (!esLado(n, i, j)) candidatas.push([i, j]);
    candidatas.sort((x, y) => distancia(p[x[0]], p[x[1]]) - distancia(p[y[0]], p[y[1]]));
    const elegidas = [];
    for (const c of candidatas) {
      if (elegidas.length === n - 3) break;
      if (elegidas.every((e) => !seCruzan(c, e))) elegidas.push(c);
    }
    ordenar(elegidas);
    return { costo: sumaLongitudes(p, elegidas), diagonales: elegidas };
  }

  // Para cada lado elige el vertice que forma el mayor angulo.
  // O(n^2)
  function triangulacionDelaunay(p) {
    const n = p.length, d = [], pila = [[0, n - 1]];
    while (pila.length) {
      const [i, j] = pila.pop();
      if (j - i < 2) continue;
      let elegido = i + 1, mejor = -1;
      for (let k = i + 1; k < j; k++) {
        const a = anguloEn(p[k], p[i], p[j]);
        if (a > mejor + 1e-12) {
          mejor = a;
          elegido = k;
        }
      }
      if (elegido - i >= 2) d.push([i, elegido]);
      if (j - elegido >= 2) d.push([elegido, j]);
      pila.push([i, elegido], [elegido, j]);
    }
    ordenar(d);
    return { costo: sumaLongitudes(p, d), diagonales: d };
  }

  // Fuerza bruta para comprobar la PD.
  // O(Catalan(n-2) * n)
  function enumerarTriangulaciones(p, i, j) {
    if (j - i < 2) return [0];
    const salida = [];
    for (let k = i + 1; k < j; k++) {
      const extra = pesoDiagonal(p, i, k) + pesoDiagonal(p, k, j);
      const izq = enumerarTriangulaciones(p, i, k), der = enumerarTriangulaciones(p, k, j);
      for (const a of izq) for (const b of der) salida.push(a + b + extra);
    }
    return salida;
  }

  // O(m)
  function catalan(m) {
    let c = 1n;
    for (let i = 0n; i < BigInt(m); i++) c = (c * 2n * (2n * i + 1n)) / (i + 2n);
    return c;
  }

  // Lee n, los vertices y una linea opcional con el precio.
  // O(n)
  function leerTexto(texto) {
    const vertices = [];
    let n = -1, precio = null;
    for (const cruda of texto.split(/\r?\n/)) {
      const t = cruda.split("#")[0].trim().split(/\s+/).filter(Boolean);
      if (!t.length) continue;
      if (t[0] === "precio") {
        precio = Number(t[1]);
        if (!Number.isFinite(precio) || precio < 0) throw new Error("El precio debe ser un numero no negativo");
      } else if (n < 0) {
        n = Number.parseInt(t[0], 10);
        if (!Number.isFinite(n)) throw new Error(`Valor no numerico en la linea "${t.join(" ")}"`);
      } else {
        const x = Number(t[0]), y = Number(t[1]);
        if (!Number.isFinite(x) || !Number.isFinite(y)) throw new Error(`Valor no numerico o incompleto en la linea "${t.join(" ")}"`);
        vertices.push([x, y]);
      }
    }
    if (n < 3) throw new Error("El archivo debe iniciar con n mayor o igual a 3");
    if (vertices.length !== n) throw new Error(`Se esperaban ${n} vertices y se leyeron ${vertices.length}`);
    if (!esConvexo(vertices)) throw new Error("Los vertices no forman un poligono convexo en orden");
    return { vertices, precio };
  }

  // O(1)
  function porcentajeExtra(costo, optimo) {
    if (optimo <= 0) return costo <= 1e-12 ? "0.00" : "inf";
    const v = (100 * (costo - optimo)) / optimo;
    return (Math.abs(v) < 5e-3 ? 0 : v).toFixed(2);
  }

  // O(1)
  function pesos(cantidad) {
    return `$${(Math.abs(cantidad) < 5e-3 ? 0 : cantidad).toFixed(2)} MXN`;
  }

  // O(n^2)
  function validar(p, costo, d) {
    const n = p.length;
    if (d.length !== Math.max(0, n - 3)) return false;
    const vistos = new Set();
    for (let a = 0; a < d.length; a++) {
      if (esLado(n, d[a][0], d[a][1])) return false;
      const clave = d[a].join(",");
      if (vistos.has(clave)) return false;
      vistos.add(clave);
      for (let b = a + 1; b < d.length; b++) if (seCruzan(d[a], d[b])) return false;
    }
    return Math.abs(sumaLongitudes(p, d) - costo) <= 1e-6 * Math.max(1, costo);
  }

  // Arma el mismo reporte que imprime el programa en C++.
  // O(n^3)
  function informe(texto) {
    const { vertices: p, precio } = leerTexto(texto);
    const n = p.length, l = [];
    const t0 = (typeof performance !== "undefined" ? performance : Date).now();
    const opt = triangulacionMinima(p);
    const ms = (typeof performance !== "undefined" ? performance : Date).now() - t0;
    l.push(`\nPoligono de ${n} vertices`, "\nDISENO DE MENOR MATERIAL", `  Diagonales por construir (${opt.diagonales.length})`);
    for (const [i, j] of opt.diagonales) l.push(`    v${i} - v${j}   longitud = ${distancia(p[i], p[j]).toFixed(4)}`);
    l.push(`  Costo total del material = ${opt.costo.toFixed(4)}`);
    if (precio !== null) l.push(`  Presupuesto del material = ${pesos(opt.costo * precio)} (a ${pesos(precio)} por unidad de longitud)`);
    l.push(`  Tiempo de la programacion dinamica = ${ms.toFixed(4)} ms`);
    l.push(`  Validacion (n-3 diagonales, sin cruces, sin lados, suma = costo) -> ${validar(p, opt.costo, opt.diagonales) ? "correcta" : "con errores"}`);

    const voraz = triangulacionVoraz(p), del = triangulacionDelaunay(p);
    l.push("\nCOMPARACION CONTRA ALTERNATIVAS INTUITIVAS");
    l.push(`  Voraz (diagonal mas corta primero) costo = ${voraz.costo.toFixed(4)}, ${porcentajeExtra(voraz.costo, opt.costo)}% mas material que el optimo`);
    l.push(`  Delaunay (maximiza angulos) costo = ${del.costo.toFixed(4)}, ${porcentajeExtra(del.costo, opt.costo)}% mas material que el optimo`);
    if (precio !== null) l.push(`  Sobrecosto en pesos, voraz ${pesos((voraz.costo - opt.costo) * precio)}, Delaunay ${pesos((del.costo - opt.costo) * precio)}`);

    const larga = disenoPorCriterio(p, DIAGONAL_MAS_CORTA), ang = disenoPorCriterio(p, ANGULO_MAXIMO);
    const masLargaOpt = opt.diagonales.reduce((m, [i, j]) => Math.max(m, distancia(p[i], p[j])), 0);
    l.push("\nOTROS CRITERIOS DE DISENO CON LA MISMA PROGRAMACION DINAMICA");
    l.push(`  Minimizar la diagonal mas larga -> la mas larga mide ${larga.valor.toFixed(4)} (en el diseno de menor material mide ${masLargaOpt.toFixed(4)}), material total = ${larga.costo.toFixed(4)}`);
    l.push(`  Maximizar el angulo minimo -> angulo minimo de ${ang.valor.toFixed(4)} grados (en el diseno de menor material es ${anguloMinimoDiseno(p, opt.diagonales).toFixed(4)} grados), material total = ${ang.costo.toFixed(4)}`);

    const conteo = resolverPD(p, CONTEO).valor[0][n - 1], esperado = catalan(n - 2);
    l.push("\nCONTEO DE TRIANGULACIONES CON LA MISMA PROGRAMACION DINAMICA");
    l.push(`  Triangulaciones posibles = ${conteo}, Catalan(${n - 2}) = ${esperado} -> ${conteo === esperado ? "coinciden" : "NO coinciden"}`);

    if (n <= 13) {
      const todas = enumerarTriangulaciones(p, 0, n - 1);
      let minimo = Infinity, maximo = -Infinity;
      for (const v of todas) {
        if (v < minimo) minimo = v;
        if (v > maximo) maximo = v;
      }
      l.push("\nFUERZA BRUTA");
      l.push(`  ${todas.length} triangulaciones revisadas, minimo = ${minimo.toFixed(4)}, maximo = ${maximo.toFixed(4)} -> ${Math.abs(minimo - opt.costo) < 1e-9 ? "el minimo coincide con la PD" : "NO coincide"}`);
      l.push(`  El peor diseno posible usa ${porcentajeExtra(maximo, opt.costo)}% mas material que el optimo`);
      if (precio !== null) l.push(`  El peor diseno costaria ${pesos(maximo * precio)}, el optimo ahorra ${pesos((maximo - opt.costo) * precio)}`);
    }
    return l.join("\n");
  }

  return {
    distancia, esConvexo, esLado, seCruzan, anguloMinimoDiseno, triangulosDe, resolverPD, reconstruir,
    triangulacionMinima, pasosPD, disenoPorCriterio, triangulacionVoraz, triangulacionDelaunay,
    enumerarTriangulaciones, catalan, leerTexto, informe, validar, porcentajeExtra, pesos,
    MENOR_MATERIAL, DIAGONAL_MAS_CORTA, ANGULO_MAXIMO, CONTEO,
  };
});

// Programa de consola, solo corre con node js/triangulacion.js.
if (typeof require !== "undefined" && typeof module !== "undefined" && require.main === module) {
  const fs = require("fs");
  const T = module.exports;
  const args = process.argv.slice(2);

  // O(n)
  function leerArchivo(nombre) {
    try {
      return fs.readFileSync(nombre, "utf8");
    } catch {
      throw new Error("No se pudo abrir el archivo");
    }
  }

  if (args[0] === "--tiempos") {
    console.log("lenguaje,archivo,n,ms,costo");
    for (const archivo of args.slice(1)) {
      let p;
      try {
        p = T.leerTexto(leerArchivo(archivo)).vertices;
      } catch {
        continue;
      }
      const t0 = performance.now();
      const r = T.triangulacionMinima(p);
      console.log(`JavaScript,${archivo},${p.length},${(performance.now() - t0).toFixed(3)},${r.costo.toFixed(6)}`);
    }
  } else {
    const readline = require("readline");
    const rl = readline.createInterface({ input: process.stdin, output: process.stdout, terminal: false });
    const lineas = rl[Symbol.asyncIterator]();
    // O(1)
    const preguntar = async (texto) => {
      process.stdout.write(texto);
      const r = await lineas.next();
      return r.done ? null : r.value.trim();
    };
    // O(n^3) por archivo
    (async () => {
      console.log("Triangulacion de costo minimo para poligonos convexos");
      let respuesta = "s";
      while (["s", "si"].includes((respuesta || "").toLowerCase())) {
        const archivo = await preguntar("\nNombre del archivo con los vertices > ");
        if (archivo === null) break;
        try {
          console.log(T.informe(leerArchivo(archivo)));
        } catch (e) {
          console.log(`Error. ${e.message}`);
        }
        respuesta = await preguntar("\nDeseas procesar otro archivo? (s/n) > ");
        if (respuesta === null) break;
      }
      console.log("\nFin del programa");
      rl.close();
    })();
  }
}
