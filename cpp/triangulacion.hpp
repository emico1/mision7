// Mision 7, algoritmos de triangulacion

#pragma once

#include <algorithm>
#include <cmath>
#include <limits>
#include <numeric>
#include <random>
#include <utility>
#include <vector>

#ifdef _OPENMP
#include <omp.h>
#endif

// Se define aqui porque M_PI no es parte del estandar y MinGW no lo trae con -std=c++17.
const double PI = 3.14159265358979323846;

// Vertice del poligono.
struct Punto {
    double x;
    double y;
};

// Diagonales elegidas y su longitud total.
struct Diseno {
    double costo;
    std::vector<std::pair<int, int>> diagonales;
};

// O(1)
inline double distancia(const Punto& a, const Punto& b) {
    return std::hypot(a.x - b.x, a.y - b.y);
}

// O(1)
inline double cruz(const Punto& o, const Punto& a, const Punto& b) {
    return (a.x - o.x) * (b.y - o.y) - (a.y - o.y) * (b.x - o.x);
}

// Revisa que todos los giros tengan el mismo signo y que el poligono de una sola vuelta, asi se detectan vertices fuera de orden.
// O(n)
inline bool esConvexo(const std::vector<Punto>& p) {
    int n = static_cast<int>(p.size());
    if (n < 3) return false;
    int signo = 0;
    for (int i = 0; i < n; i++) {
        double c = cruz(p[i], p[(i + 1) % n], p[(i + 2) % n]);
        if (std::fabs(c) < 1e-12) return false;
        int s = c > 0 ? 1 : -1;
        if (signo == 0) signo = s;
        else if (s != signo) return false;
    }
    double giro = 0.0;
    for (int i = 0; i < n; i++) {
        const Punto& a = p[i];
        const Punto& b = p[(i + 1) % n];
        const Punto& c = p[(i + 2) % n];
        double d = std::atan2(c.y - b.y, c.x - b.x) - std::atan2(b.y - a.y, b.x - a.x);
        while (d <= -PI) d += 2 * PI;
        while (d > PI) d -= 2 * PI;
        giro += d;
    }
    return std::fabs(std::fabs(giro) - 2 * PI) < 1e-6;
}

// Los lados del perimetro no cuestan porque ya forman parte de la estructura.
// O(1)
inline bool esLado(int n, int i, int j) {
    if (i > j) std::swap(i, j);
    return j - i == 1 || (i == 0 && j == n - 1);
}

// Costo w(i,j) de la recurrencia, la longitud si es diagonal y 0 si es lado.
// O(1)
inline double pesoDiagonal(const std::vector<Punto>& p, int i, int j) {
    return esLado(static_cast<int>(p.size()), i, j) ? 0.0 : distancia(p[i], p[j]);
}

// Angulo en el vertice v del triangulo v a b, en radianes.
// O(1)
inline double anguloEn(const Punto& v, const Punto& a, const Punto& b) {
    double ax = a.x - v.x, ay = a.y - v.y, bx = b.x - v.x, by = b.y - v.y;
    return std::atan2(std::fabs(ax * by - ay * bx), ax * bx + ay * by);
}

// O(1)
inline double anguloMinimoTriangulo(const Punto& a, const Punto& b, const Punto& c) {
    double m = std::min({anguloEn(a, b, c), anguloEn(b, c, a), anguloEn(c, a, b)});
    return m * 180.0 / PI;
}

// En un poligono convexo dos diagonales se cruzan si un extremo de la segunda queda entre los de la primera y el otro no.
// O(1)
inline bool seCruzan(std::pair<int, int> a, std::pair<int, int> b) {
    int i = std::min(a.first, a.second), j = std::max(a.first, a.second);
    int k = b.first, l = b.second;
    if (i == k || i == l || j == k || j == l) return false;
    bool kd = k > i && k < j;
    bool ld = l > i && l < j;
    return kd != ld;
}

// O(n)
inline double sumaLongitudes(const std::vector<Punto>& p, const std::vector<std::pair<int, int>>& d) {
    double s = 0.0;
    for (const auto& e : d) s += distancia(p[e.first], p[e.second]);
    return s;
}

// O(n)
inline double diagonalMasLarga(const std::vector<Punto>& p, const std::vector<std::pair<int, int>>& d) {
    double m = 0.0;
    for (const auto& e : d) m = std::max(m, distancia(p[e.first], p[e.second]));
    return m;
}

// Reconstruye los triangulos a partir del perimetro y las diagonales.
// O(n^3)
inline std::vector<std::vector<int>> triangulosDe(int n, const std::vector<std::pair<int, int>>& d) {
    std::vector<std::vector<char>> ady(n, std::vector<char>(n, 0));
    for (int i = 0; i < n; i++) {
        int j = (i + 1) % n;
        ady[i][j] = ady[j][i] = 1;
    }
    for (const auto& e : d) ady[e.first][e.second] = ady[e.second][e.first] = 1;
    std::vector<std::vector<int>> t;
    for (int a = 0; a < n; a++)
        for (int b = a + 1; b < n; b++)
            if (ady[a][b])
                for (int c = b + 1; c < n; c++)
                    if (ady[a][c] && ady[b][c]) t.push_back({a, b, c});
    return t;
}

// O(n^3)
inline double anguloMinimoDiseno(const std::vector<Punto>& p, const std::vector<std::pair<int, int>>& d) {
    double m = 180.0;
    for (const auto& t : triangulosDe(static_cast<int>(p.size()), d))
        m = std::min(m, anguloMinimoTriangulo(p[t[0]], p[t[1]], p[t[2]]));
    return m;
}

// Criterio de la mision, minimiza la suma de las diagonales. Sus operaciones son O(1).
struct CriterioMenorMaterial {
    using Tipo = double;
    static constexpr bool optimiza = true;
    Tipo base() const { return 0.0; }
    Tipo neutro() const { return std::numeric_limits<double>::infinity(); }
    // O(1)
    Tipo triangulo(const std::vector<Punto>& p, int i, int k, int j) const {
        return pesoDiagonal(p, i, k) + pesoDiagonal(p, k, j);
    }
    Tipo combinar(Tipo a, Tipo b) const { return a + b; }
    bool mejor(Tipo a, Tipo b) const { return a < b; }
};

// Minimiza la diagonal mas larga, cambia la suma por el maximo. Sus operaciones son O(1).
struct CriterioDiagonalMasCorta {
    using Tipo = double;
    static constexpr bool optimiza = true;
    Tipo base() const { return 0.0; }
    Tipo neutro() const { return std::numeric_limits<double>::infinity(); }
    // O(1)
    Tipo triangulo(const std::vector<Punto>& p, int i, int k, int j) const {
        return std::max(pesoDiagonal(p, i, k), pesoDiagonal(p, k, j));
    }
    Tipo combinar(Tipo a, Tipo b) const { return std::max(a, b); }
    bool mejor(Tipo a, Tipo b) const { return a < b; }
};

// Maximiza el angulo mas pequeno de los triangulos. Sus operaciones son O(1).
struct CriterioAnguloMaximo {
    using Tipo = double;
    static constexpr bool optimiza = true;
    Tipo base() const { return std::numeric_limits<double>::infinity(); }
    Tipo neutro() const { return -std::numeric_limits<double>::infinity(); }
    // O(1)
    Tipo triangulo(const std::vector<Punto>& p, int i, int k, int j) const {
        return anguloMinimoTriangulo(p[i], p[k], p[j]);
    }
    Tipo combinar(Tipo a, Tipo b) const { return std::min(a, b); }
    bool mejor(Tipo a, Tipo b) const { return a > b; }
};

// Cuenta las triangulaciones, cambia el minimo por suma y la suma por producto. Sus operaciones son O(1).
template <typename T>
struct CriterioConteo {
    using Tipo = T;
    static constexpr bool optimiza = false;
    Tipo base() const { return 1; }
    Tipo neutro() const { return 0; }
    Tipo triangulo(const std::vector<Punto>&, int, int, int) const { return 1; }
    Tipo combinar(Tipo a, Tipo b) const { return a * b; }
    Tipo sumar(Tipo a, Tipo b) const { return a + b; }
};

// valor[i*n+j] guarda C[i][j] y corte[i*n+j] el vertice k elegido.
template <typename T>
struct TablaPD {
    int n;
    std::vector<T> valor;
    std::vector<int> corte;
    T respuesta() const { return valor[n - 1]; }
};

// Programacion dinamica generica. C[i][j] es el mejor valor para triangular los vertices i a j.
// Caso base C[i][i+1], dos vertices consecutivos no tienen nada que triangular.
// Se llena por longitud del intervalo porque cada estado depende de intervalos mas cortos.
// Para cada k entre i y j se combinan C[i][k], C[k][j] y el triangulo i k j, el criterio decide cual conservar.
// O(n^3) tiempo, O(n^2) espacio
template <typename Criterio>
TablaPD<typename Criterio::Tipo> resolverPD(const std::vector<Punto>& p, const Criterio& c) {
    using T = typename Criterio::Tipo;
    int n = static_cast<int>(p.size());
    TablaPD<T> t{n, std::vector<T>(static_cast<size_t>(n) * n, c.base()), std::vector<int>(static_cast<size_t>(n) * n, -1)};
    for (int largo = 2; largo < n; largo++) {
        for (int i = 0; i + largo < n; i++) {
            int j = i + largo;
            T acumulado = c.neutro();
            int elegido = -1;
            for (int k = i + 1; k < j; k++) {
                T v = c.combinar(c.combinar(t.valor[i * n + k], t.valor[k * n + j]), c.triangulo(p, i, k, j));
                if constexpr (Criterio::optimiza) {
                    if (elegido < 0 || c.mejor(v, acumulado)) {
                        acumulado = v;
                        elegido = k;
                    }
                } else {
                    acumulado = c.sumar(acumulado, v);
                }
            }
            t.valor[i * n + j] = acumulado;
            t.corte[i * n + j] = elegido;
        }
    }
    return t;
}

// Recorre los cortes desde C[0][n-1] y agrega las diagonales de cada triangulo elegido.
// O(n)
inline std::vector<std::pair<int, int>> reconstruir(const std::vector<int>& corte, int n) {
    std::vector<std::pair<int, int>> d;
    if (n < 4) return d;
    std::vector<std::pair<int, int>> pila{{0, n - 1}};
    while (!pila.empty()) {
        auto [i, j] = pila.back();
        pila.pop_back();
        if (j - i < 2) continue;
        int k = corte[i * n + j];
        if (k - i >= 2) d.push_back({i, k});
        if (j - k >= 2) d.push_back({k, j});
        pila.push_back({i, k});
        pila.push_back({k, j});
    }
    std::sort(d.begin(), d.end());
    return d;
}

// O(n^3)
template <typename Criterio>
Diseno disenoPorCriterio(const std::vector<Punto>& p, const Criterio& c) {
    auto t = resolverPD(p, c);
    Diseno r;
    r.diagonales = reconstruir(t.corte, static_cast<int>(p.size()));
    r.costo = sumaLongitudes(p, r.diagonales);
    return r;
}

// Version base, calcula las distancias dentro del ciclo.
// O(n^3) tiempo, O(n^2) espacio
inline Diseno triangulacionGenerica(const std::vector<Punto>& p) {
    auto t = resolverPD(p, CriterioMenorMaterial{});
    return {t.respuesta(), reconstruir(t.corte, static_cast<int>(p.size()))};
}

// Solucion de la mision. C[i][j] = min de C[i][k] + C[k][j] + w(i,k) + w(k,j), la respuesta es C[0][n-1].
// Las distancias se precalculan en w y corte guarda el k elegido para reconstruir las diagonales.
// O(n^3) tiempo, O(n^2) espacio
inline Diseno triangulacionMinima(const std::vector<Punto>& p) {
    int n = static_cast<int>(p.size());
    std::vector<std::vector<double>> w(n, std::vector<double>(n, 0.0)), costo(n, std::vector<double>(n, 0.0));
    std::vector<int> corte(static_cast<size_t>(n) * n, -1);
    for (int i = 0; i < n; i++)
        for (int j = 0; j < n; j++) w[i][j] = pesoDiagonal(p, i, j);
    for (int largo = 2; largo < n; largo++) {
        for (int i = 0; i + largo < n; i++) {
            int j = i + largo;
            double mejor = std::numeric_limits<double>::infinity();
            int elegido = -1;
            for (int k = i + 1; k < j; k++) {
                double v = (costo[i][k] + costo[k][j]) + (w[i][k] + w[k][j]);
                if (v < mejor) {
                    mejor = v;
                    elegido = k;
                }
            }
            costo[i][j] = mejor;
            corte[static_cast<size_t>(i) * n + j] = elegido;
        }
    }
    return {n >= 3 ? costo[0][n - 1] : 0.0, reconstruir(corte, n)};
}

// Misma recurrencia con una copia transpuesta de la tabla para leer la memoria en orden.
// Las celdas de una misma longitud no dependen entre si, por eso se reparten entre hilos.
// O(n^3 / hilos) tiempo, O(n^2) espacio
inline Diseno triangulacionMinimaRapida(const std::vector<Punto>& p, int hilos) {
    int n = static_cast<int>(p.size());
    size_t nn = static_cast<size_t>(n) * n;
    std::vector<double> w(nn, 0.0), a(nn, 0.0), at(nn, 0.0);
    std::vector<int> corte(nn, -1);
    for (int i = 0; i < n; i++)
        for (int j = 0; j < n; j++) w[static_cast<size_t>(i) * n + j] = pesoDiagonal(p, i, j);
#ifdef _OPENMP
    omp_set_num_threads(std::max(1, hilos));
#else
    (void)hilos;
#endif
    for (int largo = 2; largo < n; largo++) {
#ifdef _OPENMP
#pragma omp parallel for schedule(static)
#endif
        for (int i = 0; i < n - largo; i++) {
            int j = i + largo;
            const double* filaI = &a[static_cast<size_t>(i) * n];
            const double* filaJ = &at[static_cast<size_t>(j) * n];
            const double* wI = &w[static_cast<size_t>(i) * n];
            const double* wJ = &w[static_cast<size_t>(j) * n];
            double mejor = std::numeric_limits<double>::infinity();
            int elegido = -1;
            for (int k = i + 1; k < j; k++) {
                double v = filaI[k] + filaJ[k] + wI[k] + wJ[k];
                if (v < mejor) {
                    mejor = v;
                    elegido = k;
                }
            }
            a[static_cast<size_t>(i) * n + j] = mejor;
            at[static_cast<size_t>(j) * n + i] = mejor;
            corte[static_cast<size_t>(i) * n + j] = elegido;
        }
    }
    return {n >= 3 ? a[n - 1] : 0.0, reconstruir(corte, n)};
}

// Heuristica voraz, toma la diagonal mas corta que no cruce a las ya elegidas.
// O(n^3)
inline Diseno triangulacionVoraz(const std::vector<Punto>& p) {
    int n = static_cast<int>(p.size());
    std::vector<std::pair<int, int>> candidatas;
    for (int i = 0; i < n; i++)
        for (int j = i + 2; j < n; j++)
            if (!esLado(n, i, j)) candidatas.push_back({i, j});
    std::stable_sort(candidatas.begin(), candidatas.end(), [&](const auto& x, const auto& y) {
        return distancia(p[x.first], p[x.second]) < distancia(p[y.first], p[y.second]);
    });
    Diseno r{0.0, {}};
    for (const auto& c : candidatas) {
        if (static_cast<int>(r.diagonales.size()) == n - 3) break;
        bool libre = true;
        for (const auto& e : r.diagonales)
            if (seCruzan(c, e)) {
                libre = false;
                break;
            }
        if (libre) r.diagonales.push_back(c);
    }
    std::sort(r.diagonales.begin(), r.diagonales.end());
    r.costo = sumaLongitudes(p, r.diagonales);
    return r;
}

// Para cada lado elige el vertice que forma el mayor angulo, en un poligono convexo eso da la triangulacion de Delaunay.
// O(n^2)
inline Diseno triangulacionDelaunay(const std::vector<Punto>& p) {
    int n = static_cast<int>(p.size());
    Diseno r{0.0, {}};
    std::vector<std::pair<int, int>> pila{{0, n - 1}};
    while (!pila.empty()) {
        auto [i, j] = pila.back();
        pila.pop_back();
        if (j - i < 2) continue;
        int elegido = i + 1;
        double mejor = -1.0;
        for (int k = i + 1; k < j; k++) {
            double ang = anguloEn(p[k], p[i], p[j]);
            if (ang > mejor + 1e-12) {
                mejor = ang;
                elegido = k;
            }
        }
        if (elegido - i >= 2) r.diagonales.push_back({i, elegido});
        if (j - elegido >= 2) r.diagonales.push_back({elegido, j});
        pila.push_back({i, elegido});
        pila.push_back({elegido, j});
    }
    std::sort(r.diagonales.begin(), r.diagonales.end());
    r.costo = sumaLongitudes(p, r.diagonales);
    return r;
}

// Verifica que ningun vertice quede dentro del circulo de algun triangulo.
// O(n^2)
inline bool cumpleDelaunay(const std::vector<Punto>& p, const std::vector<std::pair<int, int>>& d) {
    int n = static_cast<int>(p.size());
    double orientacion = cruz(p[0], p[1], p[2]) > 0 ? 1.0 : -1.0;
    for (const auto& t : triangulosDe(n, d)) {
        const Punto& a = p[t[0]];
        const Punto& b = p[t[1]];
        const Punto& c = p[t[2]];
        double escala = std::max({distancia(a, b), distancia(b, c), distancia(a, c)});
        for (int q = 0; q < n; q++) {
            if (q == t[0] || q == t[1] || q == t[2]) continue;
            double ax = a.x - p[q].x, ay = a.y - p[q].y;
            double bx = b.x - p[q].x, by = b.y - p[q].y;
            double cx = c.x - p[q].x, cy = c.y - p[q].y;
            double det = (ax * ax + ay * ay) * (bx * cy - by * cx) - (bx * bx + by * by) * (ax * cy - ay * cx) +
                         (cx * cx + cy * cy) * (ax * by - ay * bx);
            if (det * orientacion > 1e-9 * std::pow(escala, 4)) return false;
        }
    }
    return true;
}

// Fuerza bruta, devuelve el costo de todas las triangulaciones para comprobar la PD.
// O(Catalan(n-2) * n)
inline void enumerarTriangulaciones(const std::vector<Punto>& p, int i, int j, std::vector<double>& salida) {
    if (j - i < 2) {
        salida.push_back(0.0);
        return;
    }
    for (int k = i + 1; k < j; k++) {
        std::vector<double> izq, der;
        enumerarTriangulaciones(p, i, k, izq);
        enumerarTriangulaciones(p, k, j, der);
        double extra = pesoDiagonal(p, i, k) + pesoDiagonal(p, k, j);
        for (double x : izq)
            for (double y : der) salida.push_back(x + y + extra);
    }
}

// Misma recurrencia sin tabla, vuelve a calcular los subproblemas repetidos.
// O(3^n) aprox
inline double recursionSinMemoria(const std::vector<Punto>& p, int i, int j) {
    if (j - i < 2) return 0.0;
    double mejor = std::numeric_limits<double>::infinity();
    for (int k = i + 1; k < j; k++)
        mejor = std::min(mejor, recursionSinMemoria(p, i, k) + recursionSinMemoria(p, k, j) + pesoDiagonal(p, i, k) +
                                    pesoDiagonal(p, k, j));
    return mejor;
}

// Numero de Catalan, cuantas triangulaciones tiene un poligono de m+2 vertices.
// O(m)
inline long double catalan(int m) {
    long double c = 1.0L;
    for (int i = 0; i < m; i++) c = c * 2.0L * (2 * i + 1) / (i + 2);
    return c;
}

// Genera un poligono convexo aleatorio con el algoritmo de Valtr.
// O(n log n)
inline std::vector<Punto> poligonoValtr(int n, std::mt19937_64& gen) {
    std::uniform_real_distribution<double> u(0.0, 1.0);
    std::bernoulli_distribution moneda(0.5);
    auto componentes = [&](std::vector<double> v) {
        std::sort(v.begin(), v.end());
        double minimo = v.front(), maximo = v.back();
        double ultimoA = minimo, ultimoB = minimo;
        std::vector<double> r;
        for (int i = 1; i + 1 < static_cast<int>(v.size()); i++) {
            if (moneda(gen)) {
                r.push_back(v[i] - ultimoA);
                ultimoA = v[i];
            } else {
                r.push_back(ultimoB - v[i]);
                ultimoB = v[i];
            }
        }
        r.push_back(maximo - ultimoA);
        r.push_back(ultimoB - maximo);
        return r;
    };
    while (true) {
        std::vector<double> xs(n), ys(n);
        for (auto& x : xs) x = u(gen);
        for (auto& y : ys) y = u(gen);
        std::vector<double> vx = componentes(xs), vy = componentes(ys);
        std::shuffle(vy.begin(), vy.end(), gen);
        std::vector<std::pair<double, double>> vec(n);
        for (int i = 0; i < n; i++) vec[i] = {vx[i], vy[i]};
        std::sort(vec.begin(), vec.end(), [](const auto& a, const auto& b) {
            return std::atan2(a.second, a.first) < std::atan2(b.second, b.first);
        });
        std::vector<Punto> p(n);
        double x = 0.0, y = 0.0, minX = 0.0, minY = 0.0;
        for (int i = 0; i < n; i++) {
            p[i] = {x, y};
            x += vec[i].first;
            y += vec[i].second;
            minX = std::min(minX, p[i].x);
            minY = std::min(minY, p[i].y);
        }
        for (auto& q : p) q = {(q.x - minX) * 100.0, (q.y - minY) * 100.0};
        if (esConvexo(p)) return p;
    }
}