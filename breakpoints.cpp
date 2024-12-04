#include <iostream>
#include <vector>
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include "breakpoints.h"
#include "point.h"
using namespace std;

// Función auxiliar para quickselect
int quickselect(vector<int>& arr, int k) {
    if (arr.size() == 1)
        return arr[0];
    
    int pivot = arr[rand() % arr.size()];
    vector<int> lows, highs, pivots;
    
    for (int x : arr) {
        if (x < pivot) lows.push_back(x);
        else if (x > pivot) highs.push_back(x);
        else pivots.push_back(x);
    }
    
    if (k < lows.size()) {
        return quickselect(lows, k);
    } else if (k < lows.size() + pivots.size()) {
        return pivots[0];
    } else {
        return quickselect(highs, k - lows.size() - pivots.size());
    }
}

// Algoritmo 1: Selección de puntos de ruptura
vector<vector<vector<int>>> breakpoints_selection(int K, int L, int n,const vector<vector<int>>& P, int ns, int Nr) {
    vector<vector<vector<int>>> B;

    for (int i = 0; i < L; i++) {
        for (int j = 0; j < K; j++) {
            vector<int> Cij;
            for (int z = 0; z < n; z++) {
                Cij.push_back(P[z][j]);
            }
            
            sort(Cij.begin(), Cij.end());

            int round_val = static_cast<int>(log2(Nr));
            vector<vector<int>> Bij;

            for (int z = 1; z <= round_val; z++) {
                int num_breakpoints = static_cast<int>(pow(2, z - 1));
                vector<int> breakpoints;

                for (int k = 0; k < num_breakpoints; k++) {
                    breakpoints.push_back(quickselect(Cij, k * (ns / num_breakpoints)));
                }
                Bij.push_back(breakpoints);
            }

            // Genera las regiones de ruptura finales
            int final_region_size = ns / (pow(2, round_val));
            vector<int> min_region(Cij.begin(), Cij.begin() + final_region_size);
            vector<int> max_region(Cij.end() - final_region_size, Cij.end());

            Bij[0] = min_region;
            Bij.push_back(max_region);

            B.push_back(Bij);
        }
    }

    return B;
}
int find_interval(const vector<int>& arr, int oz) {
    int left = 0;
    int right = arr.size() - 1;

    if (right < 1) {
        return -1; 
    }

    // Binary search
    while (left <= right) {
        int mid = left + (right - left) / 2;

        if (mid < arr.size() - 1 && oz >= arr[mid] && oz <= arr[mid + 1]) {
            return mid + 1;
        }

        if (oz < arr[mid]) {
            right = mid - 1;
        } else {
            left = mid + 1;
        }
    }

    return -1; 
}
// Algoritmo 2: Codificación dinámica
vector<vector<vector<int>>> dynamic_encoding(int K, int L, int n, const vector<vector<int>>& P, int ns, int Nr) {
    vector<vector<vector<int>>> EP(n, vector<vector<int>>(L, vector<int>(K, -1))); // EP tridimensional

    // Puntos de ruptura
    vector<vector<vector<int>>> B = breakpoints_selection(K, L, n, P, ns, Nr);

    // Codificación
    for (int i = 0; i < L; i++) {
        for (int j = 0; j < K; j++) {
            for (int z = 0; z < n; z++) {
                int oz = P[z][j]; // Punto
                int b = find_interval(B[i][j], oz); // Intervalo

                EP[z][i][j] = b; // Guardar en EP
            }
        }
    }

    return EP;
}

// Función auxiliar para dividir un nodo cuando excede max_size
void splitNode(TreeNode* node, int max_size) {
    // Lógica de división simple: crea dos nodos hijos
    TreeNode* left = new TreeNode();
    TreeNode* right = new TreeNode();
    int mid = node->entries.size() / 2;

    for (int i = 0; i < mid; i++) {
        left->entries.push_back(node->entries[i]);
    }
    for (int i = mid; i < node->entries.size(); i++) {
        right->entries.push_back(node->entries[i]);
    }

    node->left = left;
    node->right = right;
    node->entries.clear();
}

// Algoritmo 3: Crear el índice del árbol
vector<TreeNode*> create_index(int K, int L, int n, const vector<vector<int>>& EP, int max_size) {
    vector<TreeNode*> DETs(L);

    for (int i = 0; i < L; i++) {
        TreeNode* Ti = new TreeNode();

        for (int j = 0; j < K; j++) {
            std::vector<double> epi; 
            for (int z = 0; z < n; z++) {
                epi.push_back(static_cast<double>(EP[i * K * n + j * n + z][0])); 
            }
            Ti->add_entry(Point(epi), j); 
        }
        for (int z = 0; z < n; z++) {
            std::vector<double> epi;  
            for (int j = 0; j < K; j++) {
                epi.push_back(static_cast<double>(EP[i * K * n + j * n + z][0])); 

            TreeNode* target_leaf = Ti;
            while (target_leaf->entries.size() >= max_size) {
                splitNode(target_leaf, max_size);
                target_leaf = target_leaf->left; 
            }
            target_leaf->add_entry(Point(epi), z);  
        }

        DETs[i] = Ti;
    }}

    return DETs;}