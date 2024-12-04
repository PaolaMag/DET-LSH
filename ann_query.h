// ann_query.h
#ifndef ANN_QUERY_H
#define ANN_QUERY_H

#include <vector>
#include "point.h"
#include "tree_node.h"

// Función para realizar la consulta (r, c)-ANN
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
);
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
);


#endif // ANN_QUERY_H
