#include "LSH.h"
#include <iostream>
#include <algorithm>
#include <limits>
#include <chrono>

using namespace std::chrono;

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

    auto start = high_resolution_clock::now();

    int n = dataset.size();
    vector<vector<vector<double>>> projected_points(L, vector<vector<double>>(n, vector<double>(K)));
    for (int i = 0; i < L; ++i) {
        for (int idx = 0; idx < n; ++idx) {
            projected_points[i][idx] = project_point(dataset[idx], i);
        }
    }

    auto stop = high_resolution_clock::now();
    auto duration = duration_cast<microseconds>(stop - start);


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