#include <iostream>
#include <vector>
#include <unordered_set>
#include <cmath>
#include <algorithm>
#include <ctime>
#include "tree_node.h"
#include "breakpoints.h"
#include "DETRangeQuery.h"
#include "point.h"
#include "ann_query.h"  

using namespace std;

// Imprime el contenido de un árbol
void print_tree(TreeNode* node) {
    if (node == nullptr) return;

    cout << "Entradas del nodo:" << endl;
    for (const auto& entry : node->entries) {
        // Asumiendo que entry.first es un Point con un vector de enteros llamado coordinates
        for (int i : entry.first.coordinates) {
            cout << i << " ";
        }
        cout << "Etiqueta: " << entry.second << endl;
    }

    // Recursión para los nodos hijo
    print_tree(node->left);
    print_tree(node->right);
}


int main() {
    srand(time(0)); 

    int K = 3;      // Número de características de los puntos
    int L = 2;      // Número de árboles DET
    int n = 5;      // Número total de puntos
    vector<int> P;
    for (int i = 0; i < 10; i++) {
        P.push_back(rand() % 30); // Genera un número aleatorio entre 0 y 30
    }

    // Generar puntos codificados (EP)
    vector<vector<int>> EP = dynamic_encoding(K, L, n, P, 10, 5);
    cout << "Puntos Codificados (EP):" << endl;
    for (const auto& encoded_point : EP) {
        for (int value : encoded_point) {
            cout << value << " ";
        }
        cout << endl;
    }

    // Crear índice DET
    vector<TreeNode*> DETs = create_index(K, L, n, EP, 5);
    cout << "¡Árboles DET creados!" << endl;

    // Imprimir el contenido de los árboles DET
    cout << "Contenido de los árboles DET:" << endl;
    for (TreeNode* node : DETs) {
        print_tree(node);
    }

    // Definir parámetros para la consulta (r, c)-ANN Query
    Point q = {{2, 3, 4}};  // Punto de consulta
    double r = 1.0;         // Radio de consulta
    double c = 2.5;         // Factor de radio
    double epsilon = 2.5;   // Factor de escala (ε)
    double beta = 0.5;      // Factor de corte (β)

    // Realizar la consulta (r, c)-ANN
    Point result_point = ann_query(q, K, L, n, c, r, epsilon, beta, DETs);

    // Imprimir el resultado
    if (result_point.coordinates.empty()) {
        cout << "No se encontró un punto cercano." << endl;
    } else {
        cout << "Punto cercano encontrado: ";
        for (int i : result_point.coordinates) {
            cout << i << " ";
        }
        cout << endl;
    }

    // Limpiar memoria
    for (auto node : DETs) {
        delete node; // Liberar memoria de los árboles creados
    }

    return 0;
}
