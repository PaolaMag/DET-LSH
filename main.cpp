#include <iostream>
#include <vector>
#include <set>
#include <queue>
#include <random>
#include <algorithm>
#include <numeric>
#include "LSH.h"
#include "encoding.h"
#include "indexing.h"
#include "reader.h"

using namespace std;

/*
    El paper utiliza la distancia euclidiana
*/

double euclideanDistance(const Eigen::VectorXd& vec1, const Eigen::VectorXd& vec2) {
    if (vec1.size() != vec2.size()) {
        throw invalid_argument("Vectors must have the same size.");
    }
    return (vec1 - vec2).norm();
}

/*
    Computar vecinos exactos
    Importante para el recall
*/
vector<pair<int, double>> findKNearestNeighbors(
    const Eigen::VectorXd& query,
    const vector<Eigen::VectorXd>& dataset,
    int K) {
    if (K <= 0) {
        throw invalid_argument("K must be greater than 0.");
    }

    using Neighbor = pair<double, int>;
    priority_queue<Neighbor, vector<Neighbor>, greater<>> minHeap;

    for (int i = 0; i < dataset.size(); ++i) {
        double distance = euclideanDistance(query, dataset[i]);
        minHeap.push({distance, i});
    }

    vector<pair<int, double>> kNearestNeighbors;
    for (int i = 0; i < K && !minHeap.empty(); ++i) {
        kNearestNeighbors.push_back({minHeap.top().second, minHeap.top().first});
        minHeap.pop();
    }

    return kNearestNeighbors;
}

// PARA EL CALCULO DEL RECALL
vector<int> kNearestNeighbors(const Eigen::VectorXd& query, const vector<Eigen::VectorXd>& dataset, int K) {
    using Neighbor = pair<double, int>; // {distancia, índice}
    priority_queue<Neighbor, vector<Neighbor>, greater<>> minHeap;

    for (int i = 0; i < dataset.size(); ++i) {
        double distance = euclideanDistance(query, dataset[i]);
        minHeap.push({distance, i});
    }

    vector<int> neighbors;
    for (int i = 0; i < K && !minHeap.empty(); ++i) {
        neighbors.push_back(minHeap.top().second);
        minHeap.pop();
    }

    return neighbors;
}


double calculateRecall(const set<int>& groundTruth, const set<int>& predicted) {
    int correct = 0;

    for (int neighbor : predicted) {
        if (groundTruth.find(neighbor) != groundTruth.end()) {
            correct++;
        }
    }

    return static_cast<double>(correct) / groundTruth.size();
}

set<int> simulateSystem(const set<int>& groundTruth, int datasetSize, double minRecall, double maxRecall) {
    set<int> result;

    int minCorrect = static_cast<int>(minRecall * groundTruth.size());
    int maxCorrect = static_cast<int>(maxRecall * groundTruth.size());

    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<> dist(minCorrect, maxCorrect);
    int correctCount = dist(gen);

    vector<int> datasetIndices(datasetSize);
    iota(datasetIndices.begin(), datasetIndices.end(), 0); 
    shuffle(datasetIndices.begin(), datasetIndices.end(), gen);

    auto it = groundTruth.begin();
    for (int i = 0; i < correctCount && it != groundTruth.end(); ++i, ++it) {
        result.insert(*it);
    }

    for (int i = 0; result.size() < groundTruth.size(); ++i) {
        if (groundTruth.find(datasetIndices[i]) == groundTruth.end()) {
            result.insert(datasetIndices[i]);
        }
    }

    return result;
}

void test_query(string dataset_path, string query_path, string name) {

    vector<Eigen::VectorXd> dataset = readFVECS(dataset_path);
    Eigen::VectorXd query = readFVECS(query_path)[0];



    int datasetSize = dataset.size(); // Tamaño total del dataset
    int vectorDim = dataset[0].size();     // Dimensión de cada vector


    int K = 50;
    double minRecall = 0.6; // Mínimo recall deseado
    double maxRecall = 1.0; // Máximo recall deseado


    vector<int> groundTruthVec = kNearestNeighbors(query, dataset, K);
    set<int> groundTruth(groundTruthVec.begin(), groundTruthVec.end());

    set<int> predicted = simulateSystem(groundTruth, datasetSize, minRecall, maxRecall);


    double recall = calculateRecall(groundTruth, predicted);


    cout << "Ground Truth: ";
    for (int neighbor : groundTruth) {
        cout << neighbor << " ";
    }
    cout << "\n";

    cout << "Predicted: ";
    for (int neighbor : predicted) {
        cout << neighbor << " ";
    }
    cout << "\n";

    cout << "Recall: " << recall << endl;
}

void test_encoding(string dataset_path, string name) {

    vector<Eigen::VectorXd> dataset = readFVECS(dataset_path);

    int K = 16;
    int L = 4;
    int d = dataset[0].size();
    double w = 5.0;
    LSH lsh(K, L, d, w);

    cout << "Dataset " << name << endl;

    /* 3. LSH proyecta todos los puntos en L espacios. */
    auto projected_points = lsh.project_dataset(dataset);

    /* 4. Codifica todos lo puntos. */
    int ns = 20;
    int Nr = 8;
    cout << "Optimized" << endl;
    auto encodings = dynamic_encoding(K, L, dataset.size(), projected_points, ns, Nr);
    cout << "Non optimized" << endl;
    encodings = dynamic_encoding_non_optimized(K, L, dataset.size(), projected_points, ns, Nr);
}

void test_indexing(string dataset_path, string name) {

    vector<Eigen::VectorXd> dataset = readFVECS(dataset_path);

    int K = 16;
    int L = 4;
    int d = dataset[0].size();
    double w = 5.0;
    LSH lsh(K, L, d, w);

    cout << "Dataset " << name << endl;

    /* 3. LSH proyecta todos los puntos en L espacios. */
    auto projected_points = lsh.project_dataset(dataset);

    /* 4. Codifica todos lo puntos. */
    int ns = 20;
    int Nr = 8;
    cout << "Optimized" << endl;
    auto encodings = dynamic_encoding(K, L, dataset.size(), projected_points, ns, Nr);
    cout << "Non optimized" << endl;
    encodings = dynamic_encoding_non_optimized(K, L, dataset.size(), projected_points, ns, Nr);

    /* 5. Indexacion */
    int n = dataset.size();
    int max_size = 20;

    // segmentation fall
    auto index = create_index(K, L, n, encodings, max_size);

}


int main() {

    // test_encoding("./datasets/movielens/movielens_base.fvecs", "movielens");
    // test_encoding("./datasets/audio/audio_base.fvecs", "audio");
    // test_encoding("./datasets/cifar60k/cifar60k_base.fvecs", "cifar60k");


    test_encoding("./datasets/msong/msong_base.fvecs", "msong");
    test_encoding("./datasets/deep1M/deep1M_base.fvecs", "deep1M");

    // test_indexing(
    //     "./datasets/movielens/movielens_base.fvecs",
    //     "movilens"
    // );


    // test_query(
    //     "./datasets/movielens/movielens_base.fvecs",
    //     "./datasets/movielens/movielens_query.fvecs",
    //     "movielens"
    // );




    return 0;
}