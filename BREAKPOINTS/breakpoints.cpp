#include <iostream>
#include <vector>
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include "point.h"

using namespace std;

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





#include <vector>
#include <algorithm>
#include <cmath>
#include <numeric>

using namespace std;

vector<int> sample_dimension(const vector<int>& dimension, int ns) {
    vector<int> sample;
    for (int i = 0; i < min(ns, (int)dimension.size()); ++i) {
        sample.push_back(dimension[i]);
    }
    return sample;
}

void divide_and_conquer_breakpoints(vector<int>& C_ij, vector<int>& B_ij, int num_breakpoints, int round) {
    for (int b = 0; b < num_breakpoints; ++b) {
        int q = (b + 1) * (C_ij.size() / pow(2, round)); // Índice del punto de ruptura
        nth_element(C_ij.begin(), C_ij.begin() + q, C_ij.end());
        B_ij[b] = C_ij[q];
    }
}


vector<vector<vector<int>>> select_breakpoints(int K, int L, int n, const vector<vector<vector<int>>>& P, int ns, int Nr) {
        
    vector<vector<vector<int>>> B(L, vector<vector<int>>(K, vector<int>(Nr + 1)));

    for (int i = 0; i < L; ++i) {
        for (int j = 0; j < K; ++j) {

            vector<int> C_ij = sample_dimension(P[i][j], ns);

            int rounds = log2(Nr);
            for (int z = 1; z <= rounds; ++z) {
                int num_breakpoints = pow(2, z - 1);
                divide_and_conquer_breakpoints(C_ij, B[i][j], num_breakpoints, z);
            }

            B[i][j][0] = *min_element(C_ij.begin(), C_ij.end());
            B[i][j][Nr] = *max_element(C_ij.begin(), C_ij.end());
        }
    }
    return B;
}


int main() {

    vector<Eigen::VectorXd> dataset = readFVECS("./movielens/movielens_base.fvecs");

    int K = 50;
    int L = 16;
    int d = dataset[0].size();
    double w = 5.0;
    LSH lsh(K, L, d, w);


    vector<vector<vector<int>>> projected_points = lsh.project_dataset(dataset);

    int ns = 20;
    int Nr = 8;

    vector<vector<vector<int>>> breakpoints = select_breakpoints(K, L, dataset.size(), projected_points, ns, Nr);

    print3DVector(breakpoints);

    cout << ":)" << endl;


    return 0;
}