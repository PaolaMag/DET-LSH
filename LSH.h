#ifndef LSH_H
#define LSH_H

#include "Eigen/Dense"
#include <vector>
#include <unordered_map>
#include <random>
#include <cmath>
#include <set>

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
        std::size_t operator()(const std::vector<int>& vec) const {
            std::size_t hash = 0;
            for (int num : vec) {
                hash ^= std::hash<int>()(num) + 0x9e3779b9 + (hash << 6) + (hash >> 2);
            }
            return hash;
        }
    };

    struct Comparator {
        bool operator()(const Eigen::VectorXd& a, const Eigen::VectorXd& b) const {
            return a.isApprox(b);
        }
    };

    vector<int> project_point(const Eigen::VectorXd& point, int space_index);

public:
    LSH(int K, int L, int d, double w);
    vector<vector<pair<Eigen::VectorXd, double>>> generate_hash_functions();
    vector<vector<vector<int>>> project_dataset(const vector<Eigen::VectorXd>& dataset);
    vector<unordered_map<vector<int>, vector<Eigen::VectorXd>, VectorHash>> assign_to_buckets(const vector<Eigen::VectorXd>& dataset);
    vector<Eigen::VectorXd> query(const Eigen::VectorXd& query_point, const vector<unordered_map<vector<int>, vector<Eigen::VectorXd>, VectorHash>>& buckets);
};

#endif // LSH_H