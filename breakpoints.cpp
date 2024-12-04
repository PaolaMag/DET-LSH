#include <iostream>
#include <vector>
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include "breakpoints.h"
#include "point.h"
#include "tree_node.h"
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
vector<vector<vector<double>>> breakpoints_selection(int K, int L, int n,const vector<vector<Point>>& P, int ns, int Nr) {
   vector<vector<vector<double>>>B;

    for (int i = 0; i < L; i++) {
        for (int j = 0; j < K; j++) {
            vector<double> Cij;
            for (int z = 0; z < n; z++) {
                Cij.push_back(P[z][j]);
            }
            
            sort(Cij.begin(), Cij.end());

            int round_val = static_cast<int>(log2(Nr));
            vector<vector<double>> Bij;

            for (int z = 1; z <= round_val; z++) {
                int num_breakpoints = static_cast<int>(pow(2, z - 1));
                vector<double> breakpoints;

                for (int k = 0; k < num_breakpoints; k++) {
                    breakpoints.push_back(quickselect(Cij, k * (ns / num_breakpoints)));
                }
                Bij.push_back(breakpoints);
            }

            // Genera las regiones de ruptura finales
            int final_region_size = ns / (pow(2, round_val));
            vector<double> min_region(Cij.begin(), Cij.begin() + final_region_size);
            vector<double> max_region(Cij.end() - final_region_size, Cij.end());

            Bij[0] = min_region;
            Bij.push_back(max_region);

            B.push_back(Bij);
        }
    }

    return B;
}
//
int find_interval(const vector<double>& arr, double value) {
    int left = 0, right = arr.size() - 1;

    while (left <= right) {
        int mid = left + (right - left) / 2;
        if (mid < arr.size() - 1 && value >= arr[mid] && value <= arr[mid + 1]) {
            return mid + 1;
        }
        if (value < arr[mid]) {
            right = mid - 1;
        } else {
            left = mid + 1;
        }
    }

    return -1;
}
// Algoritmo 2: Codificación dinámica
vector<vector<vector<int>>> dynamic_encoding(int K, int L, int n, const vector<vector<Point>>& P, int ns, int Nr) {
    vector<vector<vector<int>>> EP(n, vector<vector<int>>(L, vector<int>(K, -1))); // EP tridimensional

    // Puntos de ruptura
    //vector<vector<vector<double>>> B = breakpoints_selection(K, L, n, P, ns, Nr);

    // Codificación
    for (int i = 0; i < L; i++) {
        for (int j = 0; j < K; j++) {
            for (int z = 0; z < n; z++) {
                double oz = P[z][i][j]; // 
                int b = find_interval(B[i][j], oz); // Intervalo

                EP[z][i][j] = b; // Guardar en EP z(punto), i(proyeccion), j(dimension)
            }
        }
    }

    return EP;
}

