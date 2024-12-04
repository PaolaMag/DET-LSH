#include <Eigen/Dense>
#include <vector>
#include <random>

class HashFunctionGenerator {
public:
    HashFunctionGenerator(int L, int K, int d, double w)
        : L(L), K(K), d(d), w(w), gen(rd()), dis(0.0, 1.0), uniform_dis(0.0, w) {}

    std::vector<std::vector<std::pair<Eigen::VectorXd, double>>> generate_hash_functions() {
        std::vector<std::vector<std::pair<Eigen::VectorXd, double>>> H(L, std::vector<std::pair<Eigen::VectorXd, double>>(K));
        for (int i = 0; i < L; ++i) {
            for (int j = 0; j < K; ++j) {
                Eigen::VectorXd a = Eigen::VectorXd::NullaryExpr(d, [&]() { return dis(gen); });
                double b = uniform_dis(gen);
                H[i][j] = std::make_pair(a, b);
            }
        }
        return H;
    }

private:
    int L, K, d;
    double w;
    std::random_device rd;
    std::mt19937 gen;
    std::normal_distribution<> dis;
    std::uniform_real_distribution<> uniform_dis;
};