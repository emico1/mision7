// Mision 7, experimentos

#include <chrono>
#include <fstream>
#include <functional>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "triangulacion.hpp"

using Reloj = std::chrono::steady_clock;

// O(1)
double milisegundos(Reloj::time_point a, Reloj::time_point b) {
    return std::chrono::duration<double, std::milli>(b - a).count();
}

// O(log v)
std::string texto128(unsigned __int128 v) {
    if (v == 0) return "0";
    std::string s;
    while (v > 0) {
        s.push_back(static_cast<char>('0' + static_cast<int>(v % 10)));
        v /= 10;
    }
    return std::string(s.rbegin(), s.rend());
}

// O(n)
std::vector<Punto> poligonoCasiCircular(int n, std::mt19937_64& gen) {
    std::uniform_real_distribution<double> u(-0.35, 0.35), r(0.8, 1.4);
    double rx = 100.0 * r(gen), ry = 60.0 * r(gen);
    std::vector<Punto> p(n);
    for (int i = 0; i < n; i++) {
        double a = (i + u(gen)) * 2.0 * PI / n;
        p[i] = {rx * std::cos(a), ry * std::sin(a)};
    }
    return p;
}

// Compara la voraz y Delaunay contra la PD en 500 poligonos por tamano.
// O(casos * n^3)
void heuristicas() {
    std::mt19937_64 gen(7);
    std::cout << "familia,n,casos,voraz_no_optimo_pct,voraz_exceso_prom_pct,voraz_exceso_max_pct,"
                 "delaunay_no_optimo_pct,delaunay_exceso_prom_pct,delaunay_exceso_max_pct\n";
    std::vector<std::pair<std::string, std::function<std::vector<Punto>(int)>>> familias = {
        {"aleatorio_valtr", [&](int n) { return poligonoValtr(n, gen); }},
        {"casi_circular", [&](int n) { return poligonoCasiCircular(n, gen); }}};
    for (auto& [nombre, generar] : familias) {
        for (int n : {5, 6, 8, 10, 15, 20, 30, 50, 100}) {
            int casos = 500;
            int fallaV = 0, fallaD = 0;
            double sumaV = 0, sumaD = 0, maxV = 0, maxD = 0;
            for (int c = 0; c < casos; c++) {
                std::vector<Punto> p = generar(n);
                double opt = triangulacionMinima(p).costo;
                double v = 100.0 * (triangulacionVoraz(p).costo - opt) / opt;
                double d = 100.0 * (triangulacionDelaunay(p).costo - opt) / opt;
                if (v > 1e-9) fallaV++;
                if (d > 1e-9) fallaD++;
                sumaV += v;
                sumaD += d;
                maxV = std::max(maxV, v);
                maxD = std::max(maxD, d);
            }
            std::cout << std::fixed << std::setprecision(2) << nombre << "," << n << "," << casos << ","
                      << 100.0 * fallaV / casos << "," << sumaV / casos << "," << maxV << "," << 100.0 * fallaD / casos << ","
                      << sumaD / casos << "," << maxD << "\n";
        }
    }
}

// O(n log n)
std::vector<Punto> poligonoEntero(int n, int lado, std::mt19937_64& gen) {
    while (true) {
        std::vector<Punto> p = poligonoValtr(n, gen);
        double maximo = 0.0;
        for (const auto& q : p) maximo = std::max({maximo, q.x, q.y});
        for (auto& q : p) q = {std::round(q.x / maximo * lado), std::round(q.y / maximo * lado)};
        if (esConvexo(p)) return p;
    }
}

void guardarContraejemplo(const std::string& archivo, const std::string& titulo, const std::vector<Punto>& p,
                          // O(n)
                          const Diseno& opt, const Diseno& h, const std::string& nombreH) {
    std::ofstream f(archivo);
    f << p.size() << "  # " << titulo << "\n";
    for (const auto& q : p) f << q.x << " " << q.y << "\n";
    f << "# optimo PD costo " << std::fixed << std::setprecision(4) << opt.costo << ", diagonales";
    for (auto& d : opt.diagonales) f << " v" << d.first << "-v" << d.second;
    f << "\n# " << nombreH << " costo " << h.costo << ", diagonales";
    for (auto& d : h.diagonales) f << " v" << d.first << "-v" << d.second;
    f << "\n";
}

// Busca el poligono con coordenadas enteras mas pequeno donde cada heuristica no sea optima.
// O(intentos * n^3)
void contraejemplos() {
    std::mt19937_64 gen(11);
    std::vector<std::pair<std::string, std::function<Diseno(const std::vector<Punto>&)>>> heur = {
        {"voraz", triangulacionVoraz}, {"delaunay", triangulacionDelaunay}};
    for (auto& [nombre, h] : heur) {
        bool hallado = false;
        for (int n = 4; n <= 8 && !hallado; n++) {
            double mejorExceso = 0.0;
            std::vector<Punto> mejorP;
            for (int intento = 0; intento < 40000; intento++) {
                std::vector<Punto> p = poligonoEntero(n, 10, gen);
                double opt = triangulacionMinima(p).costo;
                double exceso = 100.0 * (h(p).costo - opt) / opt;
                if (exceso > mejorExceso + 1e-9) {
                    mejorExceso = exceso;
                    mejorP = p;
                }
            }
            if (mejorExceso > 1e-6) {
                hallado = true;
                Diseno opt = triangulacionMinima(mejorP), dh = h(mejorP);
                std::cout << nombre << " falla desde n = " << n << " con coordenadas enteras de 0 a 10, peor caso hallado +"
                          << std::fixed << std::setprecision(2) << mejorExceso << "%\n  vertices";
                for (auto& q : mejorP) std::cout << " (" << q.x << "," << q.y << ")";
                std::cout << "\n  optimo " << std::setprecision(4) << opt.costo << ", " << nombre << " " << dh.costo << "\n";
                guardarContraejemplo("pruebas/contraejemplo_" + nombre + ".txt",
                                     "contraejemplo donde " + nombre + " no es optimo", mejorP, opt, dh, nombre);
            } else {
                std::cout << nombre << " fue optimo en todos los casos de n = " << n << "\n";
            }
        }
    }
}

// Compara la PD contra fuerza bruta en 10,000 poligonos de 4 a 13 vertices.
// O(casos * Catalan(n-2) * n)
void masiva() {
    std::mt19937_64 gen(2026);
    std::uniform_int_distribution<int> tam(4, 13);
    int casos = 10000, difPD = 0, difRapida = 0, difDelaunay = 0, difAngulo = 0;
    long long triangulaciones = 0;
    auto a = Reloj::now();
    for (int c = 0; c < casos; c++) {
        int n = tam(gen);
        std::vector<Punto> p = poligonoValtr(n, gen);
        std::vector<double> todas;
        enumerarTriangulaciones(p, 0, n - 1, todas);
        triangulaciones += static_cast<long long>(todas.size());
        double fb = *std::min_element(todas.begin(), todas.end());
        Diseno d = triangulacionMinima(p);
        if (std::fabs(fb - d.costo) > 1e-9 * fb) difPD++;
        if (std::fabs(triangulacionMinimaRapida(p, 1).costo - d.costo) > 1e-9 * fb) difRapida++;
        Diseno del = triangulacionDelaunay(p);
        if (!cumpleDelaunay(p, del.diagonales)) difDelaunay++;
        if (std::fabs(anguloMinimoDiseno(p, del.diagonales) - resolverPD(p, CriterioAnguloMaximo{}).respuesta()) > 1e-9)
            difAngulo++;
    }
    auto b = Reloj::now();
    std::cout << "casos," << casos << "\ntriangulaciones_revisadas," << triangulaciones << "\ndiferencias_pd_vs_fuerza_bruta,"
              << difPD << "\ndiferencias_pd_rapida_vs_pd," << difRapida << "\ndelaunay_que_no_cumplen_circulo_vacio,"
              << difDelaunay << "\ndiferencias_angulo_delaunay_vs_pd_angular," << difAngulo << "\nsegundos," << std::fixed
              << std::setprecision(2) << milisegundos(a, b) / 1000.0 << "\n";
}

// Cuenta triangulaciones con la PD y las compara con la formula de Catalan.
// O(n^4)
void conteoCatalan() {
    std::mt19937_64 gen(5);
    std::cout << "n,triangulaciones_pd,catalan_formula,fuerza_bruta,coincide\n";
    unsigned __int128 c = 1;
    for (int m = 1; m <= 60; m++) {
        int n = m + 2;
        std::vector<Punto> p = poligonoValtr(n, gen);
        unsigned __int128 pd = resolverPD(p, CriterioConteo<unsigned __int128>{}).respuesta();
        std::string fb = "-";
        bool ok = pd == c;
        if (n <= 13) {
            std::vector<double> todas;
            enumerarTriangulaciones(p, 0, n - 1, todas);
            fb = std::to_string(todas.size());
            ok = ok && todas.size() == static_cast<size_t>(pd);
        }
        std::cout << n << "," << texto128(pd) << "," << texto128(c) << "," << fb << "," << (ok ? "si" : "NO") << "\n";
        c = c * 2 * (2 * m + 1) / (m + 2);
    }
}

// Mide la version generica, con precalculo, transpuesta y con 2 hilos.
// O(n^3) por tamano
void escala() {
    std::mt19937_64 gen(3);
    std::cout << "n,ms_generica,ms_precalculo,ms_rapida_1_hilo,ms_rapida_2_hilos,aceleracion_precalculo,"
                 "aceleracion_rapida_1_hilo,aceleracion_rapida_2_hilos,costos_iguales\n";
    for (int n : {100, 200, 400, 800, 1600, 3200}) {
        std::vector<Punto> p = poligonoValtr(n, gen);
        double tg = -1.0, tp = -1.0, cg = 0.0, cp = 0.0;
        if (n <= 1600) {
            auto a = Reloj::now();
            cg = triangulacionGenerica(p).costo;
            tg = milisegundos(a, Reloj::now());
            a = Reloj::now();
            cp = triangulacionMinima(p).costo;
            tp = milisegundos(a, Reloj::now());
        }
        auto a = Reloj::now();
        double c1 = triangulacionMinimaRapida(p, 1).costo;
        double r1 = milisegundos(a, Reloj::now());
        a = Reloj::now();
        double c2 = triangulacionMinimaRapida(p, 2).costo;
        double r2 = milisegundos(a, Reloj::now());
        bool iguales = std::fabs(c1 - c2) < 1e-9 * c1 &&
                       (tg < 0 || (std::fabs(c1 - cg) < 1e-9 * c1 && std::fabs(c1 - cp) < 1e-9 * c1));
        std::ostringstream os;
        os << std::fixed << std::setprecision(1) << n << ",";
        if (tg < 0) os << "-,-,";
        else os << tg << "," << tp << ",";
        os << r1 << "," << r2 << "," << std::setprecision(2);
        if (tg < 0) os << "-,-,-,";
        else os << tg / tp << "," << tg / r1 << "," << tg / r2 << ",";
        os << (iguales ? "si" : "NO");
        std::cout << os.str() << "\n";
    }
}

// Mide la recursion sin memoria contra la PD.
// O(3^n)
void recursion() {
    std::mt19937_64 gen(9);
    std::cout << "n,ms_recursion_sin_memoria,ms_pd,veces_mas_lenta,mismo_costo\n";
    for (int n = 5; n <= 18; n++) {
        std::vector<Punto> p = poligonoValtr(n, gen);
        auto a = Reloj::now();
        double r = recursionSinMemoria(p, 0, n - 1);
        double tr = milisegundos(a, Reloj::now());
        a = Reloj::now();
        double d = 0.0;
        for (int rep = 0; rep < 100; rep++) d = triangulacionMinima(p).costo;
        double td = milisegundos(a, Reloj::now()) / 100.0;
        std::cout << n << "," << std::fixed << std::setprecision(4) << tr << "," << td << "," << std::setprecision(0) << tr / td
                  << "," << (std::fabs(r - d) < 1e-9 * d ? "si" : "NO") << "\n";
    }
}

// O(depende del experimento)
int main(int argc, char* argv[]) {
    std::string modo = argc > 1 ? argv[1] : "";
    if (modo == "heuristicas") heuristicas();
    else if (modo == "contraejemplos") contraejemplos();
    else if (modo == "masiva") masiva();
    else if (modo == "catalan") conteoCatalan();
    else if (modo == "escala") escala();
    else if (modo == "recursion") recursion();
    else std::cout << "Uso ./laboratorio heuristicas | contraejemplos | masiva | catalan | escala | recursion\n";
    return 0;
}