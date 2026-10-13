// Mision 7

#include <chrono>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "triangulacion.hpp"

// Vertices leidos y precio por unidad de longitud, -1 si el archivo no lo trae.
struct Entrada {
    std::vector<Punto> vertices;
    double precioPorUnidad = -1.0;
};

// Lee n, los vertices y una linea opcional con el precio. Rechaza archivos incompletos y poligonos no convexos.
// O(n)
bool leerPoligono(const std::string& nombre, Entrada& e, std::string& error) {
    std::ifstream f(nombre);
    if (!f) {
        error = "No se pudo abrir el archivo";
        return false;
    }
    e = Entrada{};
    std::string linea;
    int n = -1;
    while (std::getline(f, linea)) {
        size_t pos = linea.find('#');
        if (pos != std::string::npos) linea = linea.substr(0, pos);
        std::istringstream ss(linea);
        std::string primero;
        if (!(ss >> primero)) continue;
        try {
            if (primero == "precio") {
                if (!(ss >> e.precioPorUnidad) || e.precioPorUnidad < 0) {
                    error = "El precio debe ser un numero no negativo";
                    return false;
                }
            } else if (n < 0) {
                n = std::stoi(primero);
            } else {
                Punto q;
                q.x = std::stod(primero);
                if (!(ss >> q.y)) {
                    error = "Falta la coordenada y en la linea \"" + linea + "\"";
                    return false;
                }
                e.vertices.push_back(q);
            }
        } catch (...) {
            error = "Valor no numerico en la linea \"" + linea + "\"";
            return false;
        }
    }
    if (n < 3) {
        error = "El archivo debe iniciar con n mayor o igual a 3";
        return false;
    }
    if (static_cast<int>(e.vertices.size()) != n) {
        error = "Se esperaban " + std::to_string(n) + " vertices y se leyeron " + std::to_string(e.vertices.size());
        return false;
    }
    if (!esConvexo(e.vertices)) {
        error = "Los vertices no forman un poligono convexo en orden";
        return false;
    }
    return true;
}

// Comprueba que haya n-3 diagonales, que ninguna sea lado, que no se crucen y que su suma sea el costo.
// O(n^2)
bool validarEstructura(const std::vector<Punto>& p, const Diseno& r, std::string& detalle) {
    int n = static_cast<int>(p.size());
    std::ostringstream os;
    bool ok = true;
    if (static_cast<int>(r.diagonales.size()) != std::max(0, n - 3)) {
        ok = false;
        os << "    numero de diagonales incorrecto\n";
    }
    for (size_t a = 0; a < r.diagonales.size(); a++) {
        if (esLado(n, r.diagonales[a].first, r.diagonales[a].second)) {
            ok = false;
            os << "    hay un lado reportado como diagonal\n";
        }
        for (size_t b = a + 1; b < r.diagonales.size(); b++)
            if (r.diagonales[a] == r.diagonales[b] || seCruzan(r.diagonales[a], r.diagonales[b])) {
                ok = false;
                os << "    hay diagonales repetidas o cruzadas\n";
            }
    }
    double suma = sumaLongitudes(p, r.diagonales);
    if (std::fabs(suma - r.costo) > 1e-6 * std::max(1.0, suma)) {
        ok = false;
        os << "    la suma de longitudes no coincide con el costo\n";
    }
    detalle = os.str();
    return ok;
}

// O(1)
std::string porcentajeExtra(double costo, double optimo) {
    std::ostringstream os;
    os << std::fixed << std::setprecision(2);
    if (optimo <= 0.0) {
        os << (costo <= 1e-12 ? "0.00" : "inf");
    } else {
        double v = 100.0 * (costo - optimo) / optimo;
        os << (std::fabs(v) < 5e-3 ? 0.0 : v);
    }
    return os.str();
}

// O(1)
std::string pesos(double cantidad) {
    std::ostringstream os;
    os << std::fixed << std::setprecision(2) << "$" << (std::fabs(cantidad) < 5e-3 ? 0.0 : cantidad) << " MXN";
    return os.str();
}

// O(1)
std::string formatoConteo(long double c) {
    std::ostringstream os;
    if (c < 1e15L) os << std::fixed << std::setprecision(0) << c;
    else os << std::scientific << std::setprecision(4) << c;
    return os.str();
}

// O(n)
void imprimirDiagonales(const std::vector<Punto>& p, const Diseno& d) {
    for (const auto& e : d.diagonales)
        std::cout << "    v" << e.first << " - v" << e.second << "   longitud = " << distancia(p[e.first], p[e.second]) << "\n";
}

// Resuelve un archivo y muestra el diseno, la comparacion con las heuristicas, los otros criterios, el conteo y la fuerza bruta.
// O(n^3)
void procesar(const std::string& archivo) {
    Entrada e;
    std::string error;
    if (!leerPoligono(archivo, e, error)) {
        std::cout << "Error. " << error << "\n";
        return;
    }
    const std::vector<Punto>& p = e.vertices;
    int n = static_cast<int>(p.size());
    std::cout << std::fixed << std::setprecision(4);

    auto t0 = std::chrono::steady_clock::now();
    Diseno optimo = triangulacionMinima(p);
    auto t1 = std::chrono::steady_clock::now();
    double ms = std::chrono::duration<double, std::milli>(t1 - t0).count();

    std::cout << "\nPoligono de " << n << " vertices\n";
    std::cout << "\nDISENO DE MENOR MATERIAL\n";
    std::cout << "  Diagonales por construir (" << optimo.diagonales.size() << ")\n";
    imprimirDiagonales(p, optimo);
    std::cout << "  Costo total del material = " << optimo.costo << "\n";
    if (e.precioPorUnidad >= 0.0)
        std::cout << "  Presupuesto del material = " << pesos(optimo.costo * e.precioPorUnidad) << " (a "
                  << pesos(e.precioPorUnidad) << " por unidad de longitud)\n";
    std::cout << "  Tiempo de la programacion dinamica = " << ms << " ms\n";

    std::string detalle;
    bool ok = validarEstructura(p, optimo, detalle);
    std::cout << "  Validacion (n-3 diagonales, sin cruces, sin lados, suma = costo) -> " << (ok ? "correcta" : "con errores")
              << "\n" << detalle;

    std::cout << "\nCOMPARACION CONTRA ALTERNATIVAS INTUITIVAS\n";
    Diseno voraz = triangulacionVoraz(p);
    Diseno delaunay = triangulacionDelaunay(p);
    std::cout << "  Voraz (diagonal mas corta primero) costo = " << voraz.costo << ", "
              << porcentajeExtra(voraz.costo, optimo.costo) << "% mas material que el optimo\n";
    std::cout << "  Delaunay (maximiza angulos) costo = " << delaunay.costo << ", "
              << porcentajeExtra(delaunay.costo, optimo.costo) << "% mas material que el optimo\n";
    if (e.precioPorUnidad >= 0.0)
        std::cout << "  Sobrecosto en pesos, voraz " << pesos((voraz.costo - optimo.costo) * e.precioPorUnidad)
                  << ", Delaunay " << pesos((delaunay.costo - optimo.costo) * e.precioPorUnidad) << "\n";

    std::cout << "\nOTROS CRITERIOS DE DISENO CON LA MISMA PROGRAMACION DINAMICA\n";
    auto tablaLarga = resolverPD(p, CriterioDiagonalMasCorta{});
    Diseno minimax = disenoPorCriterio(p, CriterioDiagonalMasCorta{});
    std::cout << "  Minimizar la diagonal mas larga -> la mas larga mide " << tablaLarga.respuesta()
              << " (en el diseno de menor material mide " << diagonalMasLarga(p, optimo.diagonales)
              << "), material total = " << minimax.costo << "\n";
    auto tablaAngulo = resolverPD(p, CriterioAnguloMaximo{});
    Diseno angular = disenoPorCriterio(p, CriterioAnguloMaximo{});
    std::cout << "  Maximizar el angulo minimo -> angulo minimo de " << tablaAngulo.respuesta()
              << " grados (en el diseno de menor material es " << anguloMinimoDiseno(p, optimo.diagonales)
              << " grados), material total = " << angular.costo << "\n";

    long double conteo = resolverPD(p, CriterioConteo<long double>{}).respuesta();
    long double esperado = catalan(n - 2);
    std::cout << "\nCONTEO DE TRIANGULACIONES CON LA MISMA PROGRAMACION DINAMICA\n";
    std::cout << "  Triangulaciones posibles = " << formatoConteo(conteo) << ", Catalan(" << n - 2
              << ") = " << formatoConteo(esperado) << " -> "
              << (std::fabs(conteo - esperado) <= 1e-12L * esperado ? "coinciden" : "NO coinciden") << "\n";

    if (n <= 13) {
        std::vector<double> todas;
        enumerarTriangulaciones(p, 0, n - 1, todas);
        double minimo = *std::min_element(todas.begin(), todas.end());
        double maximo = *std::max_element(todas.begin(), todas.end());
        std::cout << "\nFUERZA BRUTA\n";
        std::cout << "  " << todas.size() << " triangulaciones revisadas, minimo = " << minimo << ", maximo = " << maximo
                  << " -> " << (std::fabs(minimo - optimo.costo) < 1e-9 ? "el minimo coincide con la PD" : "NO coincide")
                  << "\n";
        std::cout << "  El peor diseno posible usa " << porcentajeExtra(maximo, optimo.costo)
                  << "% mas material que el optimo\n";
        if (e.precioPorUnidad >= 0.0)
            std::cout << "  El peor diseno costaria " << pesos(maximo * e.precioPorUnidad) << ", el optimo ahorra "
                      << pesos((maximo - optimo.costo) * e.precioPorUnidad) << "\n";
    }
}

// Solo mide la PD, lo usa comparar_lenguajes.js.
// O(n^3) por archivo
void modoTiempos(int argc, char* argv[]) {
    std::cout << "lenguaje,archivo,n,ms,costo\n";
    for (int a = 2; a < argc; a++) {
        Entrada e;
        std::string error;
        if (!leerPoligono(argv[a], e, error)) continue;
        auto t0 = std::chrono::steady_clock::now();
        Diseno d = triangulacionMinima(e.vertices);
        auto t1 = std::chrono::steady_clock::now();
        std::cout << "C++," << argv[a] << "," << e.vertices.size() << "," << std::fixed << std::setprecision(3)
                  << std::chrono::duration<double, std::milli>(t1 - t0).count() << "," << std::setprecision(6) << d.costo
                  << "\n";
    }
}

// O(n^3) por archivo
int main(int argc, char* argv[]) {
    if (argc > 2 && std::string(argv[1]) == "--tiempos") {
        modoTiempos(argc, argv);
        return 0;
    }
    std::cout << "Triangulacion de costo minimo para poligonos convexos\n";
    std::string respuesta = "s";
    while (respuesta == "s" || respuesta == "S" || respuesta == "si" || respuesta == "Si" || respuesta == "SI") {
        std::cout << "\nNombre del archivo con los vertices > ";
        std::string archivo;
        if (!std::getline(std::cin, archivo)) break;
        procesar(archivo);
        std::cout << "\nDeseas procesar otro archivo? (s/n) > ";
        if (!std::getline(std::cin, respuesta)) break;
    }
    std::cout << "\nFin del programa\n";
    return 0;
}
