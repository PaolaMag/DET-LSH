#include <iostream>
#include <vector>
#include <unordered_set>  
#include <cmath>
#include <algorithm>
#include "tree_node.h"
#include "point.h"
#include "DETRangeQuery.h"

using namespace std;


std::vector<int> project_query(const Point& q, int K) {
    if (q.coordinates.size() < K) {
        throw std::invalid_argument("Dimensión del punto insuficiente para la proyección.");
    }
    std::vector<int> q_prime(q.coordinates.begin(), q.coordinates.begin() + K); 
    return q_prime;
}

double distance(const Point& a, const Point& b) {
    double sum = 0.0;
    for (size_t i = 0; i < a.coordinates.size(); ++i) {
        sum += std::pow(a.coordinates[i] - b.coordinates[i], 2);
    }
    return std::sqrt(sum);
}

// Implementación de la función (r, c)-ANN Query
Point ann_query(
    const Point& q, 
    int K, 
    int L, 
    int n, 
    double c, 
    double r, 
    double epsilon, 
    double beta, 
    const std::vector<TreeNode*>& DETs
) {
    std::unordered_set<Point> S;  

    for (int i = 0; i < L; ++i) {

        std::vector<int> q_prime = project_query(q, K);  
        std::vector<double> q_prime_double(q_prime.begin(), q_prime.end());


        double r_prime = epsilon * r;
        std::vector<std::pair<Point, int>> Si = det_range_query(DETs[i], q_prime_double, r_prime, K);


        for (const auto& entry : Si) {
            S.insert(entry.first);
        }

        // Si el tamaño de S es suficientemente grande, devolvemos el punto más cercano
        if (S.size() >= beta * n + 1) {
            return *std::min_element(S.begin(), S.end(), [&q](const Point& a, const Point& b) {
                return distance(a, q) < distance(b, q);
            });
        }

        // Verificamos si existe un punto en S con distancia menor o igual a c * r
        auto close_points = std::count_if(S.begin(), S.end(), [&q, c, r](const Point& p) {
            return distance(p, q) <= c * r;
        });

        if (close_points >= 1) {
            return *std::min_element(S.begin(), S.end(), [&q](const Point& a, const Point& b) {
                return distance(a, q) < distance(b, q);
            });
        }
    }

    return Point{}; 
}
// Implementación del algoritmo c²-k-ANN Query


std::vector<Point> c2_k_ANN_Query(
    const Point& q,       // Punto de consulta
    int K,                // Número de características de los puntos
    int L,                // Número de árboles DET
    int n,                // Número total de puntos
    double c,             // Factor de escalamiento del radio
    double r_min,         // Radio mínimo inicial
    double epsilon,       // Factor de escala para el radio
    double beta,          // Parámetro beta
    int k,                // Número de vecinos cercanos
    const std::vector<TreeNode*>& DETs  // Índices de los DE-Trees
) {
    std::unordered_set<Point> S;  // Conjunto de candidatos
    double r = r_min;             // Inicializamos el radio

    while (true) {
        for (int i = 0; i < L; ++i) {
            // Proyección Hi sobre q
            std::vector<int> q_prime = project_query(q, K);
            std::vector<double> q_prime_double(q_prime.begin(), q_prime.end());

            // Realizamos la consulta DETRangeQuery
            double r_prime = epsilon * r;
            std::vector<std::pair<Point, int>> Si = det_range_query(DETs[i], q_prime_double, r_prime, K);

            // Añadimos los puntos encontrados al conjunto S
            for (const auto& entry : Si) {
                S.insert(entry.first);
            }

            // Si el tamaño de S es suficientemente grande, devolvemos los puntos más cercanos
            if (S.size() >= beta * n + k) {
                std::vector<Point> closest_points;
                for (const auto& point : S) {
                    closest_points.push_back(point);
                }
                std::sort(closest_points.begin(), closest_points.end(), [&q](const Point& a, const Point& b) {
                    return distance(a, q) < distance(b, q);
                });
                closest_points.resize(k); // Tomamos los k más cercanos
                return closest_points;
            }
        }

        // Verificamos si existe un punto en S dentro del radio escalado c * r
        std::vector<Point> valid_points;
        for (const auto& o : S) {
            if (distance(o, q) <= c * r) {
                valid_points.push_back(o);
            }
        }

        // Si hay suficientes puntos válidos, devolvemos los k más cercanos
        if (valid_points.size() >= k) {
            std::sort(valid_points.begin(), valid_points.end(), [&q](const Point& a, const Point& b) {
                return distance(a, q) < distance(b, q);
            });
            valid_points.resize(k); // Tomamos los k más cercanos
            return valid_points;
        }

        // Incrementamos el radio
        r *= c;
    }

    return {};  // Retorna vacío si no encuentra los puntos más cercanos
}