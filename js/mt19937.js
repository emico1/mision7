// Mision 7, generador pseudoaleatorio

"use strict";

class MT19937 {
  // Mismo generador y misma siembra que random.Random de Python, para reproducir los archivos de prueba.
  // O(1)
  constructor(semilla) {
    this.mt = new Uint32Array(624);
    this.indice = 625;
    this.iniciarConArreglo([semilla >>> 0]);
  }

  // O(1)
  iniciar(s) {
    const mt = this.mt;
    mt[0] = s >>> 0;
    for (let i = 1; i < 624; i++) {
      const p = mt[i - 1] ^ (mt[i - 1] >>> 30);
      mt[i] = (Math.imul(1812433253, p) + i) >>> 0;
    }
    this.indice = 624;
  }

  // O(1)
  iniciarConArreglo(clave) {
    const mt = this.mt;
    this.iniciar(19650218);
    let i = 1, j = 0;
    for (let k = Math.max(624, clave.length); k > 0; k--) {
      const p = mt[i - 1] ^ (mt[i - 1] >>> 30);
      mt[i] = ((mt[i] ^ Math.imul(p, 1664525)) + clave[j] + j) >>> 0;
      i++;
      j++;
      if (i >= 624) { mt[0] = mt[623]; i = 1; }
      if (j >= clave.length) j = 0;
    }
    for (let k = 623; k > 0; k--) {
      const p = mt[i - 1] ^ (mt[i - 1] >>> 30);
      mt[i] = ((mt[i] ^ Math.imul(p, 1566083941)) - i) >>> 0;
      i++;
      if (i >= 624) { mt[0] = mt[623]; i = 1; }
    }
    mt[0] = 0x80000000;
  }

  // O(1) amortizado
  entero32() {
    const mt = this.mt;
    if (this.indice >= 624) {
      for (let k = 0; k < 624; k++) {
        const y = (mt[k] & 0x80000000) | (mt[(k + 1) % 624] & 0x7fffffff);
        mt[k] = mt[(k + 397) % 624] ^ (y >>> 1) ^ (y & 1 ? 0x9908b0df : 0);
      }
      this.indice = 0;
    }
    let y = mt[this.indice++];
    y ^= y >>> 11;
    y ^= (y << 7) & 0x9d2c5680;
    y ^= (y << 15) & 0xefc60000;
    y ^= y >>> 18;
    return y >>> 0;
  }

  // O(1)
  aleatorio() {
    const a = this.entero32() >>> 5, b = this.entero32() >>> 6;
    return (a * 67108864 + b) / 9007199254740992;
  }

  // O(1)
  uniforme(a, b) {
    return a + (b - a) * this.aleatorio();
  }
}

module.exports = { MT19937 };
