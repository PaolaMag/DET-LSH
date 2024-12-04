#include <vector>
#include <memory>
#include <iostream>
#include <cmath>
#include <algorithm>

// Estructura para representar un nodo en el árbol
struct TreeNode {
    bool isLeaf = true; // Determina si es un nodo hoja
    std::vector<std::pair<std::vector<int>, int>> points; // Pares (codificación iSAX, posición)
    int splitDimension = -1; // Dimensión usada para dividir
    int splitValue = 0;      // Valor de división
    std::shared_ptr<TreeNode> leftChild = nullptr;
    std::shared_ptr<TreeNode> rightChild = nullptr;
};

// Estructura para representar un árbol DE-Tree
struct DETree {
    std::shared_ptr<TreeNode> root = std::make_shared<TreeNode>(); // Nodo raíz del árbol
};
void SplitNode(std::shared_ptr<TreeNode>& node, int K) {
    if (node->isLeaf && node->points.empty()) {
        std::cerr << "Error: Intentando dividir un nodo vacío.\n";
        return;
    }

    if (!node->isLeaf) {
        std::cerr << "Advertencia: Intentando dividir un nodo que ya no es hoja.\n";
        return;
    }

    std::cout << "Dividiendo nodo con " << node->points.size() << " puntos.\n";

    // Verificar si todos los puntos son idénticos
    bool allIdentical = true;
    for (size_t dim = 0; dim < K; ++dim) {
        int firstValue = node->points[0].first[dim];
        if (std::any_of(node->points.begin(), node->points.end(),
                        [&](const std::pair<std::vector<int>, int>& p) { return p.first[dim] != firstValue; })) {
            allIdentical = false;
            break;
        }
    }

    if (allIdentical) {
        std::cerr << "Error: Todos los puntos son idénticos. Nodo no será dividido.\n";
        node->isLeaf = true; // Confirmar que sigue siendo hoja
        return;
    }

    node->isLeaf = false; // Ya no es un nodo hoja
    node->leftChild = std::make_shared<TreeNode>();
    node->rightChild = std::make_shared<TreeNode>();

    if (!node->leftChild || !node->rightChild) {
        std::cerr << "Error: No se pudieron inicializar los nodos hijos.\n";
        return;
    }

    // Seleccionar dimensión para dividir
    int bestDimension = -1;
    int bestSplitPoint = 0;
    size_t minDifference = node->points.size();

    for (int dim = 0; dim < K; ++dim) {
        std::vector<int> values;
        for (const auto& point : node->points) {
            // Filtrar valores inválidos como -1
            if (point.first[dim] >= 0) {
                values.push_back(point.first[dim]);
            }
        }

        if (values.empty() || std::all_of(values.begin(), values.end(), [&](int val) { return val == values[0]; })) {
            continue; // Saltar dimensión si no hay valores válidos o todos son idénticos
        }

        std::sort(values.begin(), values.end());
        int splitPoint = values[values.size() / 2]; // Punto de división (mediana)
        size_t leftSize = std::count_if(node->points.begin(), node->points.end(),
            [&](const std::pair<std::vector<int>, int>& p) {
                return p.first[dim] <= splitPoint;
            });
        size_t difference = std::abs(static_cast<int>(node->points.size() - 2 * leftSize));
        if (difference < minDifference) {
            minDifference = difference;
            bestDimension = dim;
            bestSplitPoint = splitPoint;
        }
    }

    // Validar si se encontró una dimensión válida para dividir
    if (bestDimension == -1) {
        std::cerr << "Error: No se encontró una dimensión válida para dividir. Nodo no será dividido.\n";
        node->isLeaf = true; // Mantener como nodo hoja
        node->leftChild = nullptr;
        node->rightChild = nullptr;
        return;
    }

    std::cout << "Dividiendo en dimensión " << bestDimension
              << " con valor " << bestSplitPoint << "\n";

    // Dividir puntos en los nodos hijos
    for (const auto& point : node->points) {
        if (point.first[bestDimension] <= bestSplitPoint) {
            node->leftChild->points.push_back(point);
        } else {
            node->rightChild->points.push_back(point);
        }
    }

    node->points.clear(); // Limpiar puntos del nodo original

    // Validar que los hijos no estén vacíos
    if (node->leftChild->points.empty() || node->rightChild->points.empty()) {
        std::cerr << "Advertencia: Uno de los hijos está vacío después de dividir. Revirtiendo.\n";
        node->isLeaf = true; // Revertir la división
        node->leftChild = nullptr;
        node->rightChild = nullptr;
    }
}



// Función para crear los DE-Trees
std::vector<DETree> CreateIndex(
    int K, int L, int n, const std::vector<std::vector<int>>& EP, int max_size) {

    std::vector<DETree> DETs(L);

    for (int i = 0; i < L; ++i) {
        auto& root = DETs[i].root;

        // Insertar puntos en el árbol
        for (int z = 0; z < n; ++z) {
            auto e_pi = EP[z]; // Codificación iSAX
            int pos_z = z;     // Posición en el dataset

            auto targetLeaf = root;
            while (targetLeaf && !targetLeaf->isLeaf) {
                int dim = targetLeaf->splitDimension;
                if (e_pi[dim] <= targetLeaf->splitValue) {
                    targetLeaf = targetLeaf->leftChild;
                } else {
                    targetLeaf = targetLeaf->rightChild;
                }
            }

            if (!targetLeaf) {
                std::cerr << "Error: Nodo hoja no encontrado.\n";
                continue;
            }

            targetLeaf->points.emplace_back(e_pi, pos_z);

            // Si el nodo excede el tamaño máximo, dividirlo
            if (targetLeaf->points.size() > max_size && targetLeaf->isLeaf) {
                SplitNode(targetLeaf, K);
            }
        }
    }

    return DETs;
}