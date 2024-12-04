#include "DETRangeQuery.h"
#include "tree_node.h"
#include "point.h"  // Incluir para el tipo Point
#include <cmath>
#include <iostream>
using namespace std;

// Cálculo de la distancia Euclidiana entre dos puntos (representados como vectores)
double calculate_distance(const Point& a, const Point& b) {
    double sum = 0.0;
    for (size_t i = 0; i < a.coordinates.size(); ++i) {
        sum += std::pow(a.coordinates[i] - b.coordinates[i], 2);
    }
    return std::sqrt(sum);
}

// Cálculo de la distancia mínima (lower bound) entre el punto de consulta y el nodo
double calculate_lower_bound_distance(const std::vector<double>& q_prime, TreeNode* node) {
    double lower_bound = 0;
    for (const auto& entry : node->entries) {
        lower_bound += calculate_distance(Point(entry.first), Point(q_prime));  
    }
    return lower_bound / node->entries.size();
}

// Cálculo de la distancia máxima (upper bound) entre el punto de consulta y el nodo
double calculate_upper_bound_distance(const std::vector<double>& q_prime, TreeNode* node) {
    double upper_bound = 0;
    for (const auto& entry : node->entries) {
        upper_bound += calculate_distance(Point(entry.first), Point(q_prime));
    }
    return upper_bound / node->entries.size();
}


// Función recursiva para recorrer el subárbol y encontrar puntos dentro del rango
void traverse_subtree(TreeNode* node, const std::vector<double>& q_prime, double r_prime, std::vector<Point>& S) {
    if (node == nullptr) return;  // Si el nodo es nulo, terminamos.

    // 1. Calcula la distancia mínima entre q_prime y el nodo
    double lower_bound_dist = calculate_lower_bound_distance(q_prime, node);
    if (lower_bound_dist > r_prime) return;  // Si la distancia mínima es mayor que el radio, terminamos.

    // 2. Si el nodo es una hoja
    if (node->left == nullptr && node->right == nullptr) {
        double upper_bound_dist = calculate_upper_bound_distance(q_prime, node);
        if (upper_bound_dist <= r_prime) {
            // Agrega todos los puntos de este nodo
            for (const auto& entry : node->entries) {
                S.push_back(entry.first);  // Agregar solo el punto
            }
        } else {
            // Si no, recorrer los puntos del nodo
            for (const auto& entry : node->entries) {
                double dist = calculate_distance(Point(entry.first), Point(q_prime));  // Comparar con el punto proyectado
                if (dist <= r_prime) {
                    S.push_back(entry.first);  // Agregar punto si está dentro del rango
                }
            }
        }
    }

    // 3. Recursión sobre los hijos izquierdo y derecho
    traverse_subtree(node->left, q_prime, r_prime, S);
    traverse_subtree(node->right, q_prime, r_prime, S);
}


// Algoritmo de consulta DET en rango
std::vector<Point> DETRangeQuery(const std::vector<double>& query, double radius, TreeNode* root, int K) {
    std::vector<Point> result;

    // Llamar a TraverseSubtree para comenzar la búsqueda en la raíz
    traverse_subtree(root, query, radius, result);

    // Limitar el número de resultados a K si es necesario
    if (result.size() > K) {
        result.resize(K);  // Cortar la lista si hay más de K puntos
    }

    return result;
}
std::vector<std::pair<Point, int>> det_range_query(
    TreeNode* root,
    const std::vector<double>& q_prime,
    double r_prime,
    int K
) {
    std::vector<std::pair<Point, int>> result;  // Conjunto de resultados

    // Verificar si el nodo actual es nulo
    if (root == nullptr) {
        return result;
    }

    // Aquí iría la lógica para procesar el nodo actual en base a q_prime y r_prime
    // Por ejemplo, puedes buscar los puntos en el nodo que estén dentro del rango r_prime
    for (const auto& entry : root->entries) {
        // Compara la distancia entre el punto en "entry" y q_prime
        double dist = 0.0;
        for (size_t i = 0; i < q_prime.size(); ++i) {
            dist += std::pow(entry.first.coordinates[i] - q_prime[i], 2);
        }
        dist = std::sqrt(dist);

        // Si la distancia es menor o igual a r_prime, agregamos el punto a los resultados
        if (dist <= r_prime) {
            result.push_back(entry);
        }
    }

    // Llamamos recursivamente a los subárboles izquierdo y derecho si son relevantes
    if (root->left != nullptr) {
        result = det_range_query(root->left, q_prime, r_prime, K);  // Consulta recursiva en el subárbol izquierdo
    }
    if (root->right != nullptr) {
        result = det_range_query(root->right, q_prime, r_prime, K);  // Consulta recursiva en el subárbol derecho
    }

    return result;  
}
