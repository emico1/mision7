// Mision 7, simulador

"use strict";

(function () {
  const T = window.Triangulacion;
  const COLORES = { navy: "#1c2b45", navySoft: "#33456b", green: "#2f6b4f", gray: "#6b6f76", vidrio: "#e4eaf0", texto: "#2a2a28" };
  const MUESTRAS = {
    ventanal9: "9\n0 0\n4 0\n7 1\n9 4\n8 7\n5 9\n2 8\n0 6\n-1 3\n",
    metros: "11\nprecio 230.33\n0.0 0.0\n3.2 0.0\n6.8 0.4\n9.5 1.6\n10.6 3.4\n10.2 5.1\n7.9 6.3\n4.6 6.6\n1.6 5.8\n-0.2 3.6\n-0.5 1.5\n",
    voraz: "5\n4 10\n1 7\n0 4\n3 0\n8 10\n",
    delaunay: "4\n0 10\n4 0\n5 1\n6 3\n",
  };
  const NOMBRES = { optimo: "Menor material", voraz: "Voraz", delaunay: "Delaunay", minimax: "Diagonal más corta", angulo: "Ángulo máximo" };
  const $ = (id) => document.getElementById(id);
  const e = { p: [], precio: null, disenos: {}, pasos: [], indice: 0, modo: "optimo", timer: null, celdas: [], min: 0, max: 1 };

  // Poligono convexo aleatorio con el algoritmo de Valtr.
  // O(n log n)
  function poligonoAleatorio(n) {
    for (;;) {
      const componentes = (v) => {
        v.sort((a, b) => a - b);
        const min = v[0], max = v[v.length - 1];
        let a = min, b = min;
        const r = [];
        for (let i = 1; i < v.length - 1; i++) {
          if (Math.random() < 0.5) { r.push(v[i] - a); a = v[i]; } else { r.push(b - v[i]); b = v[i]; }
        }
        r.push(max - a, b - max);
        return r;
      };
      const vx = componentes(Array.from({ length: n }, Math.random));
      const vy = componentes(Array.from({ length: n }, Math.random));
      for (let i = vy.length - 1; i > 0; i--) { const j = Math.floor(Math.random() * (i + 1)); [vy[i], vy[j]] = [vy[j], vy[i]]; }
      const vec = vx.map((x, i) => [x, vy[i]]).sort((a, b) => Math.atan2(a[1], a[0]) - Math.atan2(b[1], b[0]));
      const p = [];
      let x = 0, y = 0;
      for (const [dx, dy] of vec) { p.push([Math.round(x * 1000) / 10, Math.round(y * 1000) / 10]); x += dx; y += dy; }
      if (T.esConvexo(p)) return `${n}\n` + p.map(([a, b]) => `${a} ${b}`).join("\n") + "\n";
    }
  }

  // Resuelve el poligono con todos los criterios y prepara los pasos de la animacion.
  // O(n^3)
  function cargar(texto) {
    const { vertices, precio } = T.leerTexto(texto);
    detener();
    e.p = vertices;
    e.precio = precio;
    e.disenos = {
      optimo: T.triangulacionMinima(vertices),
      voraz: T.triangulacionVoraz(vertices),
      delaunay: T.triangulacionDelaunay(vertices),
      minimax: T.disenoPorCriterio(vertices, T.DIAGONAL_MAS_CORTA),
      angulo: T.disenoPorCriterio(vertices, T.ANGULO_MAXIMO),
    };
    e.pasos = T.pasosPD(vertices);
    const valores = e.pasos.map((s) => s.valor);
    e.min = Math.min(0, ...valores);
    e.max = Math.max(1e-9, ...valores);
    e.indice = e.pasos.length;
    $("demoError").textContent = "";
    $("ownText").value = texto.trim() + "\n";
    construirTabla();
    actualizar();
    barras();
  }

  // Durante la animacion muestra el mejor diseno del ultimo subpoligono resuelto.
  // O(n^2)
  function diagonalesParciales() {
    const n = e.p.length;
    if (e.indice === 0) return [];
    const corte = Array.from({ length: n }, () => new Int32Array(n).fill(-1));
    for (let s = 0; s < e.indice; s++) { const { i, j, k } = e.pasos[s]; corte[i][j] = k; }
    const { i, j } = e.pasos[e.indice - 1];
    const r = !T.esLado(n, i, j) ? [[i, j]] : [];
    const pila = [[i, j]];
    while (pila.length) {
      const [a, b] = pila.pop();
      if (b - a < 2) continue;
      const k = corte[a][b];
      if (k - a >= 2) r.push([a, k]);
      if (b - k >= 2) r.push([k, b]);
      pila.push([a, k], [k, b]);
    }
    return r;
  }

  // O(n)
  function dibujar() {
    const lienzo = $("canvas");
    const caja = lienzo.getBoundingClientRect();
    const dpr = window.devicePixelRatio || 1;
    lienzo.width = Math.max(1, Math.round(caja.width * dpr));
    lienzo.height = Math.max(1, Math.round(caja.height * dpr));
    const ctx = lienzo.getContext("2d");
    ctx.setTransform(dpr, 0, 0, dpr, 0, 0);
    ctx.clearRect(0, 0, caja.width, caja.height);
    const p = e.p;
    if (!p.length) return;
    const xs = p.map((v) => v[0]), ys = p.map((v) => v[1]);
    const minX = Math.min(...xs), maxX = Math.max(...xs), minY = Math.min(...ys), maxY = Math.max(...ys);
    const margen = 32;
    const escala = Math.min((caja.width - 2 * margen) / Math.max(maxX - minX, 1e-9), (caja.height - 2 * margen) / Math.max(maxY - minY, 1e-9));
    const ox = (caja.width - (maxX - minX) * escala) / 2, oy = (caja.height - (maxY - minY) * escala) / 2;
    const X = (x) => ox + (x - minX) * escala;
    const Y = (y) => caja.height - oy - (y - minY) * escala;
    const camino = (idx) => { ctx.beginPath(); idx.forEach((k, t) => (t ? ctx.lineTo(X(p[k][0]), Y(p[k][1])) : ctx.moveTo(X(p[k][0]), Y(p[k][1])))); ctx.closePath(); };
    const todos = p.map((_, i) => i);

    camino(todos);
    ctx.fillStyle = COLORES.vidrio;
    ctx.fill();

    const actual = e.indice < e.pasos.length ? e.pasos[e.indice] : null;
    const diagonales = actual && e.modo === "optimo" ? diagonalesParciales() : e.disenos[e.modo].diagonales;
    if (actual) {
      const sub = [];
      for (let k = actual.i; k <= actual.j; k++) sub.push(k);
      camino(sub);
      ctx.fillStyle = "rgba(51, 69, 107, 0.16)";
      ctx.fill();
      camino([actual.i, actual.k, actual.j]);
      ctx.fillStyle = "rgba(51, 69, 107, 0.22)";
      ctx.fill();
    }
    ctx.lineCap = "round";
    ctx.strokeStyle = COLORES.green;
    ctx.lineWidth = 2.5;
    for (const [i, j] of diagonales) { ctx.beginPath(); ctx.moveTo(X(p[i][0]), Y(p[i][1])); ctx.lineTo(X(p[j][0]), Y(p[j][1])); ctx.stroke(); }
    if (actual) {
      ctx.strokeStyle = COLORES.navySoft;
      ctx.lineWidth = 2;
      ctx.setLineDash([6, 4]);
      camino([actual.i, actual.k, actual.j]);
      ctx.stroke();
      ctx.setLineDash([]);
    }
    camino(todos);
    ctx.strokeStyle = COLORES.navy;
    ctx.lineWidth = 3.5;
    ctx.stroke();

    const cx = xs.reduce((a, b) => a + b, 0) / p.length, cy = ys.reduce((a, b) => a + b, 0) / p.length;
    ctx.font = "12px -apple-system, BlinkMacSystemFont, 'Segoe UI', Helvetica, Arial, sans-serif";
    ctx.textAlign = "center";
    ctx.textBaseline = "middle";
    p.forEach(([x, y], i) => {
      ctx.beginPath(); ctx.arc(X(x), Y(y), 3.5, 0, 2 * Math.PI); ctx.fillStyle = COLORES.navy; ctx.fill();
      const dx = x - cx, dy = y - cy, d = Math.hypot(dx, dy) || 1;
      ctx.fillStyle = COLORES.texto;
      ctx.fillText(`v${i}`, X(x) + (dx / d) * 15, Y(y) - (dy / d) * 15);
    });
  }

  // O(n^2)
  function construirTabla() {
    const n = e.p.length;
    const g = $("grid");
    g.innerHTML = "";
    g.style.gridTemplateColumns = `repeat(${n + 1}, 22px)`;
    const celda = (texto, clase) => { const c = document.createElement("div"); c.className = "cell" + (clase ? " " + clase : ""); c.textContent = texto; g.append(c); return c; };
    celda("i\\j");
    for (let j = 0; j < n; j++) celda(j);
    e.celdas = [];
    for (let i = 0; i < n; i++) {
      celda(i);
      e.celdas.push([]);
      for (let j = 0; j < n; j++) {
        const c = celda(j === i + 1 ? "0" : "", j === i + 1 ? "base" : "");
        if (j === i + 1) c.title = `C[${i}][${j}] = 0, caso base`;
        e.celdas[i].push(c);
      }
    }
  }

  // Colorea las celdas ya calculadas y marca la actual y las dos que reutiliza.
  // O(n^2)
  function pintarTabla() {
    e.celdas.forEach((f) => f.forEach((c) => c.classList.remove("current", "dep")));
    e.pasos.forEach((s, idx) => {
      const c = e.celdas[s.i][s.j];
      if (idx < e.indice) {
        const t = (s.valor - e.min) / (e.max - e.min || 1);
        c.style.background = `rgba(47, 107, 79, ${(0.12 + t * 0.78).toFixed(3)})`;
        c.title = `C[${s.i}][${s.j}] = ${s.valor.toFixed(3)}, mejor k = ${s.k}`;
        c.classList.add("done");
      } else {
        c.style.background = "";
        c.title = `C[${s.i}][${s.j}] pendiente`;
        c.classList.remove("done");
      }
    });
    const a = e.pasos[e.indice];
    if (a) {
      e.celdas[a.i][a.j].classList.add("current");
      e.celdas[a.i][a.k].classList.add("dep");
      e.celdas[a.k][a.j].classList.add("dep");
    }
  }

  // O(1)
  function describir() {
    const a = e.pasos[e.indice];
    if (a) {
      $("stats").innerHTML = `Paso <b>${e.indice + 1}</b> de ${e.pasos.length}, <b>C[${a.i}][${a.j}]</b> = ${a.valor.toFixed(3)} con k = <b>${a.k}</b>.`;
      return;
    }
    const d = e.disenos[e.modo];
    let extra = "";
    if (e.modo === "minimax") extra = `, la diagonal más larga mide <b>${d.valor.toFixed(3)}</b>`;
    if (e.modo === "angulo") extra = `, el ángulo más pequeño es de <b>${d.valor.toFixed(2)}°</b>`;
    $("stats").innerHTML = `${NOMBRES[e.modo]}, <b>${d.diagonales.length}</b> diagonales, material <b>${d.costo.toFixed(3)}</b>${extra}.`;
  }

  // O(1)
  function barras() {
    const d = e.disenos;
    const filas = [["optimo", "Menor material (PD)"], ["minimax", "Diagonal más corta (PD)"], ["voraz", "Voraz"], ["angulo", "Ángulo máximo (PD)"], ["delaunay", "Delaunay"]];
    const maximo = Math.max(...filas.map(([c]) => d[c].costo), 1e-9);
    const conPrecio = e.precio !== null;
    $("bars").innerHTML = filas.map(([clave, nombre]) => {
      const c = d[clave].costo;
      const extra = T.porcentajeExtra(c, d.optimo.costo);
      const marca = Number(extra) > 0 ? `<span class="over">+${extra}%</span>` : "óptimo";
      const pesos = conPrecio ? ` · ${T.pesos(c * e.precio)}` : "";
      return `<div class="bar${clave === "optimo" ? " best" : ""}"><span class="name">${nombre}</span><div class="track"><div class="fill" style="width:${(100 * c) / maximo}%"></div></div><span class="fig">${c.toFixed(3)}${pesos} · ${marca}</span></div>`;
    }).join("");
    const n = e.p.length;
    $("footDemo").textContent = `${T.catalan(n - 2).toLocaleString("es-MX")} triangulaciones posibles.`;
  }

  // O(n^2)
  function actualizar() {
    pintarTabla();
    dibujar();
    describir();
  }

  // O(1)
  function marcar(contenedor, atributo, valor) {
    $(contenedor).querySelectorAll("button").forEach((b) => b.classList.toggle("active", b.dataset[atributo] === valor));
  }

  // O(n^2)
  function avanzar() {
    if (e.indice < e.pasos.length) e.indice++;
    actualizar();
    if (e.indice >= e.pasos.length) detener();
  }

  // O(1)
  function detener() {
    clearInterval(e.timer);
    e.timer = null;
    $("play").textContent = "Animar llenado";
  }

  // O(1)
  function iniciar() {
    if (e.timer) { detener(); return; }
    if (e.indice >= e.pasos.length) e.indice = 0;
    e.modo = "optimo";
    marcar("modes", "mode", "optimo");
    $("play").textContent = "Pausar";
    actualizar();
    e.timer = setInterval(avanzar, 1000 / Number($("speed").value));
  }

  // O(n^3)
  function elegir(preset) {
    marcar("presets", "preset", preset);
    $("randomControls").hidden = preset !== "aleatorio";
    cargar(preset === "aleatorio" ? poligonoAleatorio(Number($("nRandom").value)) : MUESTRAS[preset]);
  }

  $("presets").querySelectorAll("button").forEach((b) => b.addEventListener("click", () => elegir(b.dataset.preset)));
  $("nRandom").addEventListener("input", () => { $("nValue").textContent = $("nRandom").value; });
  $("nRandom").addEventListener("change", () => elegir("aleatorio"));
  $("another").addEventListener("click", () => elegir("aleatorio"));
  $("useText").addEventListener("click", () => {
    try {
      cargar($("ownText").value);
      marcar("presets", "preset", "");
      $("randomControls").hidden = true;
    } catch (err) {
      $("demoError").textContent = `Error. ${err.message}`;
    }
  });
  $("modes").querySelectorAll("button").forEach((b) => b.addEventListener("click", () => {
    detener();
    e.indice = e.pasos.length;
    e.modo = b.dataset.mode;
    marcar("modes", "mode", e.modo);
    actualizar();
  }));
  $("play").addEventListener("click", iniciar);
  $("step").addEventListener("click", () => {
    detener();
    if (e.indice >= e.pasos.length) e.indice = 0;
    e.modo = "optimo";
    marcar("modes", "mode", "optimo");
    avanzar();
  });
  $("finish").addEventListener("click", () => { detener(); e.indice = e.pasos.length; actualizar(); });
  $("speed").addEventListener("input", () => { if (e.timer) { clearInterval(e.timer); e.timer = setInterval(avanzar, 1000 / Number($("speed").value)); } });
  $("fileInput").addEventListener("change", async (ev) => {
    const f = ev.target.files[0];
    if (!f) return;
    try {
      cargar(await f.text());
      marcar("presets", "preset", "");
      $("randomControls").hidden = true;
    } catch (err) {
      $("demoError").textContent = `Error. ${err.message}`;
    }
  });
  new ResizeObserver(dibujar).observe($("canvas"));

  elegir("ventanal9");
})();
