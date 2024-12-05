#include "Eigen/Dense"
#include <vector>
#include <iostream>
#include <unordered_map>
#include <random>
#include <cmath>
#include <limits>
#include <chrono>
#include <algorithm>
#include <set>
#include <fstream>

using namespace std;

class LSH {
private:
    int K, L, d;
    double w;
    vector<vector<pair<Eigen::VectorXd, double>>> H;
    random_device rd;
    mt19937 gen;
    normal_distribution<> dis;
    uniform_real_distribution<> uniform_dis;

    struct VectorHash {
        std::size_t operator()(const std::vector<double>& vec) const {
            std::size_t hash = 0;
            for (double num : vec) {
                hash ^= std::hash<double>()(num) + 0x9e3779b9 + (hash << 6) + (hash >> 2);
            }
            return hash;
        }
    };

    struct Comparator {
        bool operator()(const Eigen::VectorXd& a, const Eigen::VectorXd& b) const {
            return a.isApprox(b);
        }
    };

    
public:
    LSH(int K, int L, int d, double w);

    vector<double> project_point(const Eigen::VectorXd& point, int space_index);


    vector<vector<pair<Eigen::VectorXd, double>>> generate_hash_functions();
    vector<vector<vector<double>>> project_dataset(const vector<Eigen::VectorXd>& dataset);
    vector<unordered_map<vector<double>, vector<Eigen::VectorXd>, LSH::VectorHash>> assign_to_buckets(const vector<Eigen::VectorXd>& dataset);
    vector<Eigen::VectorXd> query(const Eigen::VectorXd& query_point, const vector<unordered_map<vector<double>, vector<Eigen::VectorXd>, VectorHash>>& buckets);
};

LSH::LSH(int K, int L, int d, double w) 
    : K(K), L(L), d(d), w(w), gen(rd()), dis(0.0, 1.0), uniform_dis(0.0, w) {
    H = generate_hash_functions();
}

vector<vector<pair<Eigen::VectorXd, double>>> LSH::generate_hash_functions() {
    vector<vector<pair<Eigen::VectorXd, double>>> H(L, vector<pair<Eigen::VectorXd, double>>(K));
    for (int i = 0; i < L; ++i) {
        for (int j = 0; j < K; ++j) {
            Eigen::VectorXd a = Eigen::VectorXd::NullaryExpr(d, [&]() { return dis(gen); });
            double b = uniform_dis(gen);
            H[i][j] = make_pair(a, b);
        }
    }
    return H;
}

vector<double> LSH::project_point(const Eigen::VectorXd& point, int space_index) {
    vector<double> hashes(K);
    for (int j = 0; j < K; ++j) {
        const auto& [a, b] = H[space_index][j];
        double h = floor((a.dot(point) + b) / w);
        hashes[j] = static_cast<int>(h);
    }
    return hashes;
}

vector<vector<vector<double>>> LSH::project_dataset(const vector<Eigen::VectorXd>& dataset) {

    auto start = std::chrono::high_resolution_clock::now();

    int n = dataset.size();
    vector<vector<vector<double>>> projected_points(L, vector<vector<double>>(n, vector<double>(K)));
    for (int i = 0; i < L; ++i) {
        for (int idx = 0; idx < n; ++idx) {
            projected_points[i][idx] = project_point(dataset[idx], i);
        }
    }

    auto stop = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);


    cout << "Project dataset: " << duration.count() << " microseconds" << endl;

    return projected_points;
}


vector<unordered_map<vector<double>, vector<Eigen::VectorXd>, LSH::VectorHash>> LSH::assign_to_buckets(const vector<Eigen::VectorXd>& dataset) {
    // Cada bucket es un unordered_map donde la llave es un vector<double> y el valor es un vector de Eigen::VectorXd.
    vector<unordered_map<vector<double>, vector<Eigen::VectorXd>, VectorHash>> buckets(L);
    for (const auto& point : dataset) {
        for (int space_index = 0; space_index < L; ++space_index) {
            // Proyectamos el punto al espacio actual
            vector<double> bucket_key = project_point(point, space_index);
            // Asignamos el punto al bucket correspondiente
            buckets[space_index][bucket_key].push_back(point);
        }
    }
    return buckets;
}

vector<Eigen::VectorXd> LSH::query(const Eigen::VectorXd& query_point, const vector<unordered_map<vector<double>, vector<Eigen::VectorXd>, VectorHash>>& buckets) {
    
    set<Eigen::VectorXd, LSH::Comparator> candidates;
    for (int space_index = 0; space_index < L; ++space_index) {
        
        vector<double> bucket_key = project_point(query_point, space_index);
        // Si el bucket existe, añadimos los candidatos
        auto it = buckets[space_index].find(bucket_key);
        if (it != buckets[space_index].end()) {
            candidates.insert(it->second.begin(), it->second.end());
        }
    }
    return vector<Eigen::VectorXd>(candidates.begin(), candidates.end());
}

vector<Eigen::VectorXd> read_fvecs(const string& filename) {
    ifstream input(filename, ios::binary);
    if (!input.is_open()) {
        throw runtime_error("No se pudo abrir el archivo " + filename);
    }

    vector<Eigen::VectorXd> dataset;
    while (!input.eof()) {
        int d;
        input.read(reinterpret_cast<char*>(&d), sizeof(int));
        if (input.eof()) break;

        Eigen::VectorXd vec(d);
        input.read(reinterpret_cast<char*>(vec.data()), d * sizeof(float));
        dataset.push_back(vec.cast<double>());
    }

    input.close();
    return dataset;
}

class LSHBreakpointAlgorithm {
public:
    // Corrected return type to match the algorithm specification
    std::vector<std::vector<std::vector<int>>> computeBreakpoints(
        int K,          // Number of hash functions per space
        int L,          // Number of hash spaces
        int n,          // Total number of points
        const std::vector<std::vector<std::vector<int>>>& projectedSpaces, // Projected points
        int sampleSize, // Sample size
        int numRegions  // Number of regions in each projected space
    ) {
        // Validate inputs
        if (numRegions <= 0 || numRegions > 1024) {
            throw std::invalid_argument("Number of regions must be between 1 and 1024");
        }

        // Final result: breakpoints for each space, hash function, and region
        std::vector<std::vector<std::vector<int>>> B(
            L,  // L spaces
            std::vector<std::vector<int>>(
                K,  // K hash functions per space
                std::vector<int>(numRegions + 1)  // Nr + 1 regions
            )
        );

        // Compute number of rounds safely
        int rounds = std::min(static_cast<int>(std::log2(numRegions)), 10);

        // Iterate through hash spaces
        for (int i = 0; i < L; ++i) {
            for (int j = 0; j < K; ++j) {
                // Sample points for this hash function
                std::vector<int> C;
                for (int k = 0; k < sampleSize; ++k) {
                    C.push_back(projectedSpaces[i][k][j]);
                }

                // Sort the sample to ensure we can safely select elements
                std::sort(C.begin(), C.end());

                // Compute final region size
                int finalRegionSize = std::max(1, static_cast<int>(std::floor(sampleSize / std::pow(2, rounds))));

                // First breakpoint (minimum of the first small region)
                B[i][j][0] = *std::min_element(C.begin(), C.begin() + finalRegionSize);

                // Compute intermediate breakpoints
                for (int z = 1; z < numRegions; ++z) {
                    // Compute index proportionally
                    double proportion = static_cast<double>(z) / numRegions;
                    int index = static_cast<int>(proportion * (C.size() - 1));
                    B[i][j][z] = C[index];
                }

                // Last breakpoint (maximum of the last small region)
                B[i][j][numRegions] = *std::max_element(
                    C.end() - finalRegionSize, 
                    C.end()
                );
            }
        }

        return B;
    }
};

class LSHPointEncoder {
public:
    // Binary search to find the region for a given hash value
    int findRegion(
        const std::vector<int>& breakpoints, 
        int hashValue
    ) {
        int left = 0;
        int right = breakpoints.size() - 2;  // Nr regions

        // Handle edge cases
        if (hashValue < breakpoints[0]) return 0;
        if (hashValue >= breakpoints.back()) return breakpoints.size() - 2;

        // Binary search to find the appropriate region
        while (left <= right) {
            int mid = left + (right - left) / 2;
            
            // Check if the hash value is within the current region
            if (hashValue >= breakpoints[mid] && 
                hashValue < breakpoints[mid + 1]) {
                return mid;
            }
            
            // Adjust search boundaries
            if (hashValue < breakpoints[mid]) {
                right = mid - 1;
            } else {
                left = mid + 1;
            }
        }

        // Fallback (should not happen with proper breakpoints)
        return 0;
    }

    // Main encoding function
    std::vector<std::vector<std::vector<uint8_t>>> encodePoints(
        int K,          // Number of hash functions per space
        int L,          // Number of hash spaces
        int n,          // Total number of points
        const std::vector<std::vector<std::vector<int>>>& projectedSpaces,  // Projected points
        const std::vector<std::vector<std::vector<int>>>& breakpoints,      // Precomputed breakpoints
        int numRegions  // Number of regions in each projected space
    ) {
        // Validate inputs
        if (K != breakpoints[0].size() || L != breakpoints.size()) {
            throw std::invalid_argument("Breakpoints dimensions do not match input parameters");
        }

        // Initialize encoded points structure
        std::vector<std::vector<std::vector<uint8_t>>> encodedPoints(
            L,  // L hash spaces
            std::vector<std::vector<uint8_t>>(
                K,  // K hash functions per space
                std::vector<uint8_t>(n)  // n points
            )
        );

        // Iterate through hash spaces
        for (int i = 0; i < L; ++i) {
            for (int j = 0; j < K; ++j) {
                for (int z = 0; z < n; ++z) {
                    // Get the hash value for this point in this space and hash function
                    int hashValue = projectedSpaces[i][z][j];

                    // Find the region using binary search on breakpoints
                    int region = findRegion(breakpoints[i][j], hashValue);

                    // Encode the region using an 8-bit alphabet (0-255)
                    // We'll use the region index directly, wrapping around if needed
                    encodedPoints[i][j][z] = static_cast<uint8_t>(
                        std::min(region, 255)  // Ensure we don't exceed 8-bit range
                    );
                }
            }
        }

        return encodedPoints;
    }
};

// Estructura para un nodo del árbol usando bytes
struct DETreeNode {
    bool is_leaf;
    vector<pair<vector<uint8_t>, int>> entries;  // <encoded_point (8-bit), position>
    vector<DETreeNode*> children;
    uint8_t split_dimension;
    uint8_t split_value;

    DETreeNode(bool leaf = true) : is_leaf(leaf), split_dimension(0), split_value(0) {}
};

class DETree {
private:
    DETreeNode* root;
    int K;  // Número de dimensiones
    int max_size;  // Tamaño máximo de nodo hoja

public:
    DETree(int K, int max_size) : K(K), max_size(max_size) {
        root = new DETreeNode(true);
    }

    void insertPoint(const vector<uint8_t>& encoded_point, int position) {
        DETreeNode* target_leaf = findTargetLeaf(encoded_point);
        target_leaf->entries.push_back({encoded_point, position});
        
        if (target_leaf->entries.size() >= max_size) {
            splitNode(target_leaf);
        }
    }

    DETreeNode* findTargetLeaf(const vector<uint8_t>& encoded_point) {
        DETreeNode* current = root;
        while (!current->is_leaf) {
            // Comparación a nivel de byte
            if (encoded_point[current->split_dimension] <= current->split_value) {
                current = current->children[0];
            } else {
                current = current->children[1];
            }
        }
        return current;
    }

    void splitNode(DETreeNode* node) {
        node->is_leaf = false;
        node->split_dimension = (node->split_dimension + 1) % K;

        // Calcular mediana de los valores de byte
        vector<uint8_t> values;
        for (const auto& entry : node->entries) {
            values.push_back(entry.first[node->split_dimension]);
        }
        sort(values.begin(), values.end());
        node->split_value = values[values.size() / 2];

        auto left = new DETreeNode(true);
        auto right = new DETreeNode(true);

        // Distribuir puntos basándose en comparación de bytes
        for (const auto& entry : node->entries) {
            if (entry.first[node->split_dimension] <= node->split_value) {
                left->entries.push_back(entry);
            } else {
                right->entries.push_back(entry);
            }
        }

        node->entries.clear();
        node->children = {left, right};
    }

        // Funciones auxiliares privadas
    double calculateHammingDistance(const vector<uint8_t>& a, const vector<uint8_t>& b) {
        double distance = 0;
        for (size_t i = 0; i < a.size(); ++i) {
            distance += (a[i] != b[i]);
        }
        return distance;
    }

    double calculateLowerBoundDistance(const vector<uint8_t>& query, DETreeNode* node) {
        if (node->is_leaf) {
            if (node->entries.empty()) return numeric_limits<double>::infinity();
            
            double minDist = numeric_limits<double>::infinity();
            for (const auto& entry : node->entries) {
                minDist = min(minDist, calculateHammingDistance(query, entry.first));
            }
            return minDist;
        }
        
        return abs(query[node->split_dimension] - node->split_value);
    }

    double calculateUpperBoundDistance(const vector<uint8_t>& query, DETreeNode* node) {
        if (!node->is_leaf) return numeric_limits<double>::infinity();
        
        double maxDist = 0;
        for (const auto& entry : node->entries) {
            maxDist = max(maxDist, calculateHammingDistance(query, entry.first));
        }
        return maxDist;
    }

        void traverseSubtree(DETreeNode* node, const vector<uint8_t>& query, double radius, vector<int>& results) {
        double lower_bound_dist = calculateLowerBoundDistance(query, node);
        
        if (lower_bound_dist > radius) {
            return;
        }
        
        if (node->is_leaf) {
            double upper_bound_dist = calculateUpperBoundDistance(query, node);
            
            if (upper_bound_dist <= radius) {
                for (const auto& entry : node->entries) {
                    results.push_back(entry.second);
                }
            } else {
                for (const auto& entry : node->entries) {
                    double dist = calculateHammingDistance(query, entry.first);
                    if (dist <= radius) {
                        results.push_back(entry.second);
                    }
                }
            }
        } else {
            traverseSubtree(node->children[0], query, radius, results);
            traverseSubtree(node->children[1], query, radius, results);
        }
    }

    // Método público para range query
    vector<int> rangeQuery(const vector<uint8_t>& query, double radius) {
        vector<int> results;
        
        if (!root || root->is_leaf) {
            traverseSubtree(root, query, radius, results);
            return results;
        }
        
        for (DETreeNode* child : root->children) {
            traverseSubtree(child, query, radius, results);
        }
        
        return results;
    }
};

class CreateIndex {
private:
    vector<DETree*> trees;
    int K, L, n, max_size;

public:
    CreateIndex(int K, int L, int n, int max_size) 
        : K(K), L(L), n(n), max_size(max_size) {
        for (int i = 0; i < L; i++) {
            trees.push_back(new DETree(K, max_size));
        }
    }

    vector<DETree*> buildIndex(const vector<vector<vector<uint8_t>>>& encodedPoints) {
        for (int i = 0; i < L; i++) {
            for (int z = 0; z < n; z++) {
                // Construir punto codificado como vector de bytes
                vector<uint8_t> ep_i;
                for (int k = 0; k < K; k++) {
                    ep_i.push_back(encodedPoints[i][k][z]);
                }
                trees[i]->insertPoint(ep_i, z);
            }
        }
        return trees;
    }
};

// Ejemplo de uso en el main:
void testRangeQuery(vector<DETree*>& dets, const vector<uint8_t>& query, double radius) {
    cout << "Ejecutando range query con radio " << radius << endl;
    
    for (int i = 0; i < dets.size(); ++i) {
        cout << "Árbol " << i << ":" << endl;
        vector<int> results = dets[i]->rangeQuery(query, radius);
        
        cout << "Resultados encontrados: ";
        for (int pos : results) {
            cout << pos << " ";
        }
        cout << endl;
    }
}


int main() {
    // Parámetros de LSH
    int K = 16; // Número de funciones hash por espacio
    int L = 4; // Número de espacios hash
    int d = 100; // Dimensión de los vectores
    double w = 4.0; // Ancho de las celdas hash

    // Inicializar LSH
    LSH lsh(K, L, d, w);

    // Generar 5 vectores aleatorios de dimensión 100
    int n = 50; // Número de puntos en el dataset
    vector<Eigen::VectorXd> dataset(n, Eigen::VectorXd(d));
    random_device rd;
    mt19937 gen(rd());
    normal_distribution<> dis(0.0, 1.0);

    for (auto& vec : dataset) {
        for (int i = 0; i < d; ++i) {
            vec[i] = dis(gen);
        }
    }

    // Proyectar el dataset
    vector<vector<vector<double>>> projected_dataset = lsh.project_dataset(dataset);

    // Convertir los puntos proyectados a vector<vector<vector<int>>>
    vector<vector<vector<int>>> projected_spaces(L, vector<vector<int>>(n, vector<int>(K)));
    for (int i = 0; i < L; ++i) {
        for (int j = 0; j < n; ++j) {
            for (int k = 0; k < K; ++k) {
                projected_spaces[i][j][k] = static_cast<int>(projected_dataset[i][j][k]);
            }
        }
    }

    // Parámetros para computeBreakpoints
    int sampleSize = 5; // Tamaño de la muestra
    int numRegions = 256; // Número de regiones

    // Inicializar LSHBreakpointAlgorithm
    LSHBreakpointAlgorithm lshBreakpointAlgorithm;

    // Llamar a computeBreakpoints
    vector<vector<vector<int>>> breakpoints = lshBreakpointAlgorithm.computeBreakpoints(K, L, n, projected_spaces, sampleSize, numRegions);

    // Inicializar LSHPointEncoder
    LSHPointEncoder encoder;

    // Codificar los puntos utilizando estos breakpoints
    auto encodedPoints = encoder.encodePoints(K, L, n, projected_spaces, breakpoints, numRegions);

    int max_size = 10; // Tamaño máximo de nodo hoja

    // Crear y construir el índice
    CreateIndex indexBuilder(K, L, n, max_size);
    vector<DETree*> dets = indexBuilder.buildIndex(encodedPoints);

    cout << "Índice DE-Tree creado con éxito" << endl;

    // Después de crear los DETs...
    vector<uint8_t> query_point(K, 0); 
    for (int i = 0; i < K; ++i) {
        query_point[i] = dis(gen);
    }
    
     // Punto de consulta de ejemplo
    double search_radius = 100.0;         // Radio de búsqueda

    testRangeQuery(dets, query_point, search_radius);

    return 0;
}