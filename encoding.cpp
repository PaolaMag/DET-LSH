#include <iostream>
#include <vector>
#include <algorithm>
#include <cmath>
#include <random>
#include <numeric>
#include <chrono>

using namespace std;
using namespace std::chrono;

void divide_and_conquer_breakpoints(vector<double>& C_ij, vector<double>& B_ij, int num_breakpoints, int round, int n_s) {
    for (int b = 0; b < num_breakpoints; ++b) {
        int q = (b + 1) * (n_s / pow(2, round)); // Selecciona el índice del punto de ruptura
        nth_element(C_ij.begin(), C_ij.begin() + q, C_ij.end());
        B_ij[b] = C_ij[q];
    }
}

vector<vector<vector<double>>> breakpoints_selection_non_optimized(int K, int L, int n, const vector<vector<vector<double>>>& P, int n_s, int N_r) {
    
    auto start = high_resolution_clock::now();
    
    vector<vector<vector<double>>> B(L, vector<vector<double>>(K, vector<double>(N_r + 1)));
    for (int i = 0; i < L; ++i) {
        for (int j = 0; j < K; ++j) {
            // Muestra aleatoria de n_s puntos
            vector<double> C_ij(n_s);
            for (int s = 0; s < n_s; ++s) {
                C_ij[s] = P[i][j][s];
            }
            int rounds = log2(N_r);
            for (int z = 1; z <= rounds; ++z) {
                int num_breakpoints = pow(2, z - 1);
                divide_and_conquer_breakpoints(C_ij, B[i][j], num_breakpoints, z, n_s);
            }
            // Asigna el valor mínimo y máximo a los extremos
            B[i][j][0] = *min_element(C_ij.begin(), C_ij.end());
            B[i][j][N_r] = *max_element(C_ij.begin(), C_ij.end());
        }
    }

    auto stop = high_resolution_clock::now();
    auto duration = duration_cast<microseconds>(stop - start);

    cout << "Breakpoints selection: " << duration.count() << " microseconds" << endl;

    return B;
}


vector<vector<vector<double>>> breakpoints_selection(int K, int L, int n, const vector<vector<vector<double>>>& P, int n_s, int N_r) {

    auto start = high_resolution_clock::now();

    vector<vector<vector<double>>> B(L, vector<vector<double>>(K, vector<double>(N_r + 1)));

    for (int i = 0; i < L; ++i) { 
        for (int j = 0; j < K; ++j) { 

            vector<double> C_ij(n_s);
            for (int s = 0; s < n_s; ++s) {
                C_ij[s] = P[i][j][s];
            }

            // Ordenamos completamente la muestra
            sort(C_ij.begin(), C_ij.end());

            // Seleccionamos los puntos de ruptura uniformemente
            for (int z = 1; z <= N_r; ++z) {
                int index = z * n_s / N_r; // Índice proporcional
                B[i][j][z] = C_ij[index - 1];
            }

            // Asignamos el valor mínimo y máximo
            B[i][j][0] = C_ij.front();
            B[i][j][N_r] = C_ij.back();
        }
    }

    auto stop = high_resolution_clock::now();
    auto duration = duration_cast<microseconds>(stop - start);

    cout << "Breakpoints selection: " << duration.count() << " microseconds" << endl;

    return B;
}


int binary_search_region(int value, const vector<int>& breakpoints) {
    auto it = upper_bound(breakpoints.begin(), breakpoints.end(), value);
    return distance(breakpoints.begin(), it) - 1;  // Índice de la región
}


vector<vector<vector<int>>> dynamic_encoding(int K, int L, int n, const vector<vector<vector<double>>>& P, int ns, int Nr) {

    vector<vector<vector<int>>> EP(L, vector<vector<int>>(P[0].size(), vector<int>(K, 0))); // 𝑛 · 𝐿 · 𝐾

    auto B = breakpoints_selection(K, L, n, P, ns, Nr);


    auto start = high_resolution_clock::now();

    // Iterar sobre los espacios proyectados, dimensiones y puntos
    for (int i = 0; i < L; ++i) {
        for (int j = 0; j < K; ++j) {


            for (int idx = 0; idx < n; ++idx) {

                // Encontrar el rango del punto según los breakpoints
                for (int r = 0; r < Nr; ++r) {
                    if (B[i][j][r] <= EP[i][idx][j] &&
                        EP[i][idx][j] < B[i][j][r + 1]) {
                        EP[i][idx][j] = r;
                        break;
                    }
                }
            }
        }
    }

    auto stop = high_resolution_clock::now();

    auto duration = duration_cast<microseconds>(stop - start);

    cout << "Dynamic encoding: " << duration.count() << " microseconds" << endl;

    return EP;

}

vector<vector<vector<int>>> dynamic_encoding_non_optimized(int K, int L, int n, const vector<vector<vector<double>>>& P, int ns, int Nr) {

    vector<vector<vector<int>>> EP(L, vector<vector<int>>(P[0].size(), vector<int>(K, 0))); // 𝑛 · 𝐿 · 𝐾

    auto B = breakpoints_selection_non_optimized(K, L, n, P, ns, Nr);


    auto start = high_resolution_clock::now();

    // Iterar sobre los espacios proyectados, dimensiones y puntos
    for (int i = 0; i < L; ++i) {
        for (int j = 0; j < K; ++j) {


            for (int idx = 0; idx < n; ++idx) {

                // Encontrar el rango del punto según los breakpoints
                for (int r = 0; r < Nr; ++r) {
                    if (B[i][j][r] <= EP[i][idx][j] &&
                        EP[i][idx][j] < B[i][j][r + 1]) {
                        EP[i][idx][j] = r;
                        break;
                    }
                }
            }
        }
    }

    auto stop = high_resolution_clock::now();

    auto duration = duration_cast<microseconds>(stop - start);

    cout << "Dynamic encoding: " << duration.count() << " microseconds" << endl;

    return EP;

}






// Algorithm 1: Breakpoints Selection
//----------------------------------
// Input: 
//   K: Dimension of the projected points
//   L: Number of projection spaces
//   n: Number of data points
//   P: All points in projected spaces P
//   ns: Sample size
//   Nr: Number of regions in each projected space
// Output:
//   B: A set of breakpoints
//----------------------------------

/*
vector<vector<vector<double> > > BreakpointsSelection(int K, int L, int n, const vector<vector<Point>>& P, int ns, int Nr) {
    vector<vector<vector<double> > > B(L, vector<vector<double> >(K, vector<double>(Nr + 1, 0)));

    for (int i = 0; i< L; i++) {
        for (int j = 0; j < K; j++) {
            vector<int> C;
            for (int o = 0; o < ns; o++)
                C.push_back(P[i][rand()%n].coordinates[j]); // i: Plano, o: objeto y j: dimension

            sort(C.begin(), C.end());

            for (int z = 1; z < Nr; z++) {
                B[i][j][z] = C[std::floor(z/Nr)*(ns)];
            }

            B[i][j][0] = C[0];
            B[i][j][Nr] = C[ns-1];
        }
    }
    
    return B;
}

*/