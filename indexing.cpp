#include <iostream>
#include <vector>
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include "point.h"
#include "tree_node.h"
#include "chrono"

using namespace std;
using namespace std::chrono;

void splitNode(TreeNode* node, int dimension, int bit) {
    TreeNode* left = new TreeNode();
    TreeNode* right = new TreeNode();

    for (const auto& entry : node->entries) {
        // Acceder al punto
        const Point& point = entry.first;

        // Dividir según el bit de la coordenada en la dimensión especificada
        if (static_cast<int>(point.coordinates[dimension]) & (1 << bit)) { // Evaluar el bit
            right->entries.push_back(entry);
        } else {
            left->entries.push_back(entry);
        }
    }

    node->left = left;
    node->right = right;
    node->entries.clear();
}


// Algoritmo 3: Crear el índice del árbol
vector<TreeNode*> create_index(int K, int L, int n, const vector<vector<vector<int>>>& EP, int max_size) {

    auto start = high_resolution_clock::now();

    vector<TreeNode*> DETs(L);

    for (int i = 0; i < L; i++) {
        TreeNode* root = new TreeNode();

        // Inicializar hijos de la raíz (2^K nodos iniciales)
        for (int child = 0; child < (1 << K); child++) {
            TreeNode* childNode = new TreeNode();
            root->add_child(childNode); // Agregar como hijo
        }

        for (int z = 0; z < n; z++) {
            const vector<int>& epi = EP[z][i];
            int pos = z;

            TreeNode* target_leaf = root;

            // Navegar el árbol hasta encontrar el nodo hoja
            while (!target_leaf->is_leaf()) {
                int dimension = 0; // Lógica para elegir dimensión
                if (epi[dimension] & (1 << 0)) {
                    target_leaf = target_leaf->right;
                } else {
                    target_leaf = target_leaf->left;
                }
            }

            // Insertar el punto en el nodo hoja
            target_leaf->add_entry(Point(std::vector<double>(epi.begin(), epi.end())), pos);

            // Dividir el nodo si excede el tamaño máximo
            // Dividirse en dos nodos hijos según una dimensión y un bit relevantes.
            if (target_leaf->entries.size() >= max_size) {
                splitNode(target_leaf, 0, 0); // Mejorar lógica de división
                target_leaf = target_leaf->left;
            }
        }

        DETs[i] = root;
    }

    auto stop = high_resolution_clock::now();
    auto duration = duration_cast<microseconds>(stop - start);

    cout << "Indexing: " << duration.count() << " microseconds" << endl;

    return DETs;
}
