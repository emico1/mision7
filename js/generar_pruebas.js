// Mision 7, generador de pruebas

"use strict";

const fs = require("fs");
const path = require("path");
const { MT19937 } = require("./mt19937.js");

const CARPETA = "pruebas";

// O(n)
function escribir(nombre, puntos, descripcion) {
  const lineas = [`${puntos.length}  # ${descripcion}`, ...puntos.map(([x, y]) => `${x.toFixed(2)} ${y.toFixed(2)}`)];
  fs.writeFileSync(path.join(CARPETA, nombre), lineas.join("\n") + "\n");
}

// Angulos casi uniformes sobre una elipse, asi el poligono siempre es convexo.
// O(n)
function poligonoAleatorio(n, semilla) {
  const r = new MT19937(semilla);
  const paso = (2 * Math.PI) / n;
  const angulos = [];
  for (let i = 0; i < n; i++) angulos.push(i * paso + r.uniforme(-0.35, 0.35) * paso);
  const rx = r.uniforme(80, 140), ry = r.uniforme(50, 100);
  return angulos.map((a) => [rx * Math.cos(a), ry * Math.sin(a)]);
}

// O(n)
function regular(n, radio) {
  return Array.from({ length: n }, (_, i) => [radio * Math.cos((2 * Math.PI * i) / n), radio * Math.sin((2 * Math.PI * i) / n)]);
}

// O(suma de n)
function main() {
  fs.mkdirSync(CARPETA, { recursive: true });
  escribir("prueba01_cuadrado.txt", [[0, 0], [4, 0], [4, 3], [0, 3]], "rectangulo 4x3, empate entre dos diagonales de 5");
  escribir("prueba02_rombo.txt", [[0, 0], [6, 2], [12, 0], [6, -2]], "rombo, la diagonal corta mide 4 y la larga 12");
  escribir("prueba03_hexagono.txt", regular(6, 10), "hexagono regular de radio 10");
  escribir("prueba04_ventanal9.txt", [[0, 0], [4, 0], [7, 1], [9, 4], [8, 7], [5, 9], [2, 8], [0, 6], [-1, 3]],
    "ventanal irregular de 9 vertices, prueba documentada a mano");
  [12, 20, 35, 50, 75, 100].forEach((n, idx) => {
    const numero = String(idx + 5).padStart(2, "0");
    escribir(`prueba${numero}_aleatorio${n}.txt`, poligonoAleatorio(n, 100 + n), `poligono convexo aleatorio de ${n} vertices`);
  });
  escribir("prueba11_no_convexo.txt", [[0, 0], [4, 0], [2, 1], [4, 4], [0, 4]], "caso de error, poligono no convexo");
  fs.writeFileSync(path.join(CARPETA, "prueba12_incompleto.txt"), "5  # caso de error, declara 5 vertices pero solo trae 3\n0 0\n4 0\n4 4\n");
}

main();
