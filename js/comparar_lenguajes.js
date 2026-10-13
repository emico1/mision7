// Mision 7, comparacion entre lenguajes

"use strict";

const fs = require("fs");
const path = require("path");
const { execFileSync } = require("child_process");
const { MT19937 } = require("./mt19937.js");

const TAMANOS = [25, 50, 100, 200, 400, 800];
const REPETICIONES = 3;

// O(n)
function generar(n, semilla) {
  const r = new MT19937(semilla);
  const paso = (2 * Math.PI) / n;
  const rx = r.uniforme(80, 140), ry = r.uniforme(50, 100);
  const lineas = [String(n)];
  for (let i = 0; i < n; i++) {
    const a = i * paso + r.uniforme(-0.35, 0.35) * paso;
    lineas.push(`${(rx * Math.cos(a)).toFixed(6)} ${(ry * Math.sin(a)).toFixed(6)}`);
  }
  const nombre = path.join("tiempos", `poligono_${n}.txt`);
  fs.writeFileSync(nombre, lineas.join("\n") + "\n");
  return nombre;
}

// O(n^3)
function medir(comando, args, archivo) {
  const salida = execFileSync(comando, [...args, "--tiempos", archivo], { encoding: "utf8" });
  const fila = salida.trim().split(/\r?\n/)[1].split(",");
  return { ms: Number(fila[3]), costo: Number(fila[4]) };
}

// Compila el programa en C++ y mide los dos lenguajes con los mismos poligonos, se queda con el mejor de 3 corridas.
// O(lenguajes * tamanos * n^3)
function main() {
  fs.mkdirSync("tiempos", { recursive: true });
  fs.mkdirSync("resultados", { recursive: true });
  const ejecutable = path.join("cpp", process.platform === "win32" ? "triangulacion.exe" : "triangulacion");
  execFileSync("g++", ["-std=c++17", "-O2", "-o", ejecutable, path.join("cpp", "triangulacion.cpp")], { stdio: "inherit" });
  const lenguajes = {
    "C++": [path.resolve(ejecutable), []],
    JavaScript: [process.execPath, [path.join("js", "triangulacion.js")]],
  };
  const filas = ["n,C++,JavaScript,veces_mas_lento_js,mismo_costo"];
  for (const n of TAMANOS) {
    const archivo = generar(n, 1000 + n);
    const tiempos = {}, costos = new Set();
    for (const [nombre, [comando, args]] of Object.entries(lenguajes)) {
      const corridas = Array.from({ length: REPETICIONES }, () => medir(comando, args, archivo));
      tiempos[nombre] = Math.min(...corridas.map((c) => c.ms));
      costos.add(corridas[0].costo.toFixed(6));
    }
    const fila = `${n},${tiempos["C++"].toFixed(3)},${tiempos.JavaScript.toFixed(3)},${(tiempos.JavaScript / tiempos["C++"]).toFixed(2)},${costos.size === 1 ? "si" : "NO"}`;
    console.log(fila);
    filas.push(fila);
  }
  fs.writeFileSync(path.join("resultados", "lenguajes.csv"), filas.join("\n") + "\n");
}

main();
