#include "Eigen/Dense"
#include <vector>
#include <unordered_map>
#include <random>
#include <cmath>
#include <iostream>
#include <set>


class LSH {
public:
    LSH(int K, int L, int d, double w)
        : K(K), L(L), d(d), w(w), gen(rd()), dis(0.0, 1.0), uniform_dis(0.0, w) {
        H = generate_hash_functions();
    }

    std::vector<std::vector<std::pair<Eigen::VectorXd, double>>> generate_hash_functions() {
        std::vector<std::vector<std::pair<Eigen::VectorXd, double>>> H(L, std::vector<std::pair<Eigen::VectorXd, double>>(K));
        for (int i = 0; i < L; ++i) {
            for (int j = 0; j < K; ++j) {
                Eigen::VectorXd a = Eigen::VectorXd::NullaryExpr(d, [&]() { return dis(gen); }); // Vector aleatorio en d dimensiones
                double b = uniform_dis(gen);  // Desplazamiento aleatorio
                H[i][j] = std::make_pair(a, b);
            }
        }
        return H;
    }

    std::vector<int> project_point(const Eigen::VectorXd& point, int space_index) {
        std::vector<int> hashes(K);
        for (int j = 0; j < K; ++j) {
            const auto& [a, b] = H[space_index][j];
            double h = std::floor((a.dot(point) + b) / w);
            hashes[j] = static_cast<int>(h);
        }
        return hashes;
    }

    std::vector<std::vector<std::vector<int>>> project_dataset(const std::vector<Eigen::VectorXd>& dataset) {
        int n = dataset.size();
        std::vector<std::vector<std::vector<int>>> projected_points(L, std::vector<std::vector<int>>(n, std::vector<int>(K)));
        for (int i = 0; i < L; ++i) {
            for (int idx = 0; idx < n; ++idx) {
                projected_points[i][idx] = project_point(dataset[idx], i);
            }
        }
        return projected_points;
    }

private:
    int K, L, d;
    double w;
    std::vector<std::vector<std::pair<Eigen::VectorXd, double>>> H;
    std::random_device rd;
    std::mt19937 gen;
    std::normal_distribution<> dis;
    std::uniform_real_distribution<> uniform_dis;
};