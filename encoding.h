#ifndef BREAKPOINTS_H
#define BREAKPOINTS_H

#include <vector>

using namespace std;

vector<vector<vector<double>>> breakpoints_selection(int K, int L, int n, const vector<vector<vector<double>>>& P, int n_s, int N_r);

vector<vector<vector<double>>> breakpoints_selection_non_optimized(int K, int L, int n, const vector<vector<vector<double>>>& P, int n_s, int N_r);

vector<vector<vector<int>>> dynamic_encoding(int K, int L, int n, const vector<vector<vector<double>>>& P, int ns, int Nr);

vector<vector<vector<int>>> dynamic_encoding_non_optimized(int K, int L, int n, const vector<vector<vector<double>>>& P, int ns, int Nr);

#endif // BREAKPOINTS_H
