#ifndef DETRANGEQUERY_H
#define DETRANGEQUERY_H

#include <vector>
#include "tree_node.h"
#include "point.h"  // Asegúrate de incluir el archivo de definición de Point

// Algoritmo 4: Consulta de rango en el árbol DET
std::vector<std::pair<Point, int>> det_range_query(
    TreeNode* root,
    const std::vector<double>& q_prime,
    double r_prime,
    int K
);

// Algoritmo 5: Recorrido del subárbol en el árbol DET para la consulta de rango
void traverse_subtree(TreeNode* node, const std::vector<double>& q_prime, double r_prime, std::vector<std::pair<Point, int>>& S);


std::vector<Point> DETRangeQuery(const std::vector<double>& query, double radius, int detTree, int K);


#endif // DETRANGEQUERY_H
