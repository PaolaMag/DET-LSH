#include <iostream>
#include <vector>
#include <random>
#include <cassert>
#include <chrono>
#include "tree_node.h" 
#include "point.h"
#include "Eigen/Dense"     
#include "indexing.h"
#include "encoding.h"
#include "reader.h"
#include "LSH.h"
#include "ann_query.h"

using namespace std;
using namespace std::chrono;

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

vector<Eigen::VectorXd> generate_random_queries(int num_queries, int d) {
    vector<Eigen::VectorXd> queries(num_queries);
    random_device rd;
    mt19937 gen(rd());
    uniform_real_distribution<> dis(0.0, 100.0);

    for (auto& query : queries) {
        query = Eigen::VectorXd(d);
        for (int i = 0; i < d; ++i) {
            query[i] = dis(gen);
        }
    }

    return queries;
}

std::vector<Point> convertEigenToPoints(const std::vector<Eigen::VectorXd>& eigenVectors) {
    std::vector<Point> points;
    points.reserve(eigenVectors.size()); // Reservar memoria para optimización

    for (const auto& eigenVec : eigenVectors) {
        points.emplace_back(eigenVec); // Usar el constructor de Point
    }

    return points;
}



void test_indexing_with_queries(string dataset_path, string name) {
    vector<Eigen::VectorXd> dataset = readFVECS(dataset_path);

    int K = 16; // Dimensiones proyectadas
    int L = 4;  // Espacios proyectados
    int d = dataset[0].size(); // Dimensiones originales
    double w = 5.0;
    LSH lsh(K, L, d, w);

    cout << "Dataset " << name << endl;

    // 3. Proyección de puntos en L espacios
    auto projected_points = lsh.project_dataset(dataset);

    // 4. Codificación de puntos
    int ns = 20;  // Tamaño de la muestra para breakpoints
    int Nr = 8;   // Número de regiones
    cout << "Optimized Encoding" << endl;
    auto encodings = dynamic_encoding(K, L, dataset.size(), projected_points, ns, Nr);

    // 5. Construcción de índices (DE-Trees)
    int n = dataset.size();
    int max_size = 20;
    vector<TreeNode*> DETs = create_index(K, L, n, encodings, max_size);





    // 6. Fase de consulta
    int num_queries = 5;       // Número de consultas
    vector<Eigen::VectorXd> queriess = generate_random_queries(num_queries, d);
    double r_min = 10.0;       // Radio inicial para consultas ANN
    double c = 2.0;            // Escalamiento del radio
    double epsilon = 1.2;      // Factor de escala del radio proyectado
    double beta = 0.1;         // Parámetro beta para falsos positivos
    int k = 3;                 // Número de vecinos cercanos

    auto queries = convertEigenToPoints(queriess);

    cout << "Running queries..." << endl;

    for (int i = 0; i < queries.size(); ++i) {
        cout << "Query " << i + 1 << ": " << queries[i] << endl;


        auto start = chrono::high_resolution_clock::now();
        vector<Point> nearest_neighbors = c2_k_ANN_Query(
            queries[i], K, L, n, c, r_min, epsilon, beta, k, DETs
        );
        auto end = chrono::high_resolution_clock::now();

        cout << "Query completed in "
             << chrono::duration_cast<chrono::milliseconds>(end - start).count()
             << " ms" << endl;



        cout << "Top-" << k << " nearest neighbors found:" << endl;
        for (const auto& neighbor : nearest_neighbors) {
            cout << neighbor << endl;
        }

        cout << "------------------------------------" << endl;
    }

    // Liberar memoria de los DE-Trees
    for (auto tree : DETs) {
        delete tree;
    }
}


int main() {
    test_create_index_with_split();

    // test_indexing_with_queries("./datasets/movielens/movielens_base.fvecs", "movielens");
    // test_indexing_with_queries("./datasets/audio/audio_base.fvecs", "audio");
    // test_indexing_with_queries("./datasets/cifar60k/cifar60k_base.fvecs", "cifar60k");

    return 0;
}
