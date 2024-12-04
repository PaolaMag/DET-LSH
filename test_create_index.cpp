#include <iostream>
#include <vector>
#include <random>
#include <cassert>
#include "tree_node.h" 
#include "point.h"     
#include "create_index.h"
using namespace std;

vector<vector<vector<int>>> generate_random_EP(int K, int L, int n) {
    vector<vector<vector<int>>> EP(n, vector<vector<int>>(L, vector<int>(K, 0)));
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<> dist(0, 1); // Generar bits aleatorios (0 o 1)

    for (int z = 0; z < n; z++) {
        for (int i = 0; i < L; i++) {
            for (int k = 0; k < K; k++) {
                EP[z][i][k] = dist(gen); // Bits aleatorios
            }
        }
    }
    return EP;
}

// Contar puntos en el árbol
int count_points(TreeNode* node) {
    if (!node) return 0;

    if (node->is_leaf()) {
        return node->entries.size();
    }

    int count = 0;
    if (node->left) count += count_points(node->left);
    if (node->right) count += count_points(node->right);
    for (TreeNode* child : node->children) {
        count += count_points(child);
    }

    return count;
}

// Verificar que todos los puntos cumplan con las condiciones del nodo hoja
void verify_leaf_conditions(TreeNode* node, int dimension, int bit) {
    if (!node || !node->is_leaf()) return;

    for (const auto& entry : node->entries) {
        const Point& point = entry.first;
        int bit_value = static_cast<int>(point.coordinates[dimension]) & (1 << bit);
        assert(bit_value == 0 || bit_value == 1); // Verifica que el bit esté bien clasificado
    }
}

// Imprimir árbol para depuración
void print_tree(TreeNode* node, int depth = 0, int node_id = 0) {
    if (!node) return;

    string indent(depth * 2, ' ');

    if (node->is_leaf()) {
        cout << indent << "Leaf Node (ID: " << node_id << "):" << endl;
        for (const auto& entry : node->entries) {
            cout << indent << "  Point: [";
            for (size_t i = 0; i < entry.first.coordinates.size(); ++i) {
                cout << entry.first.coordinates[i];
                if (i < entry.first.coordinates.size() - 1) cout << ", ";
            }
            cout << "], Position: " << entry.second << endl;
        }
    } else {
        cout << indent << "Internal Node (ID: " << node_id << ")" << endl;
    }

    if (node->left) {
        print_tree(node->left, depth + 1, node_id * 2 + 1);
    }
    if (node->right) {
        print_tree(node->right, depth + 1, node_id * 2 + 2);
    }
    for (size_t i = 0; i < node->children.size(); ++i) {
        cout << indent << "Child " << i << ":" << endl;
        print_tree(node->children[i], depth + 1, node_id * 10 + i + 1);
    }
}



void test_create_index_with_split() {
    int K = 2;       // Dimensiones
    int L = 1;       // Número de DE-Trees
    int n = 10;      // Número de puntos
    int max_size = 3; // Tamaño máximo de nodos hoja antes de dividir

    // Generar un EP aleatorio
    vector<vector<vector<int>>> EP = generate_random_EP(K, L, n);

    // Crear el índice
    vector<TreeNode*> DETs = create_index(K, L, n, EP, max_size);

    // Verificar la estructura del árbol
    TreeNode* root = DETs[0];
    assert(root->children.size() == (1 << K)); // 2^K hijos iniciales en la raíz

    // Imprimir el árbol
    cout << "Estructura del árbol:" << endl;
    print_tree(root);

    // Verificar que los puntos están insertados y los nodos se dividen correctamente
    for (int z = 0; z < n; z++) {
        TreeNode* target_leaf = root;
        const vector<int>& epi = EP[z][0];

        // Navegar al nodo hoja
        while (!target_leaf->is_leaf()) {
            if (epi[0] & (1 << 0)) {
                target_leaf = target_leaf->right;
            } else {
                target_leaf = target_leaf->left;
            }
        }

        // Verificar que el punto está en el nodo hoja
        bool found = false;
        for (const auto& entry : target_leaf->entries) {
            if (entry.first.coordinates == vector<double>(epi.begin(), epi.end())) {
                found = true;
                break;
            }
        }
        assert(found); // Asegurarse de que el punto fue encontrado
    }

    // Verificar divisiones de nodos
    for (TreeNode* node : DETs) {
        if (!node->is_leaf()) {
            assert(node->left != nullptr);  // Nodo izquierdo existe
            assert(node->right != nullptr); // Nodo derecho existe
        }
    }

    cout << "Prueba de create_index con splitNode exitosa" << endl;
}


int main() {
    test_create_index_with_split();
    return 0;
}
