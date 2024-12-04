#include <Eigen/Dense>
#include <fstream>
#include <vector>

std::vector<Eigen::VectorXd> readFVECS(const std::string& filename) {
    std::vector<Eigen::VectorXd> vectors;

    std::ifstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        throw std::runtime_error("Error opening file: " + filename);
    }

    while (!file.eof()) {
        int32_t length = 0;
        file.read(reinterpret_cast<char*>(&length), sizeof(int32_t));

        if (file.eof()) {
            break;
        }

        std::vector<float> buffer(length);
        file.read(reinterpret_cast<char*>(buffer.data()), length * sizeof(float));

        if (!file) {
            throw std::runtime_error("Error reading data from file.");
        }
        
        Eigen::VectorXd eigenVector(length);
        for (int i = 0; i < length; ++i) {
            eigenVector[i] = buffer[i];
        }

        vectors.push_back(eigenVector);
    }

    return vectors;
}