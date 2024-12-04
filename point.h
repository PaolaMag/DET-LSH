#ifndef POINT_H
#define POINT_H

#include <vector>
#include <cmath>
#include <functional>

struct Point {
    std::vector<double> coordinates;  // Coordenadas del punto

    Point() = default;

    // Constructor
    Point(std::vector<double> coords) : coordinates(coords) {}

    double distance_squared_to(const Point& other) const {
        double sum = 0.0;
        for (size_t i = 0; i < coordinates.size(); ++i) {
            sum += std::pow(coordinates[i] - other.coordinates[i], 2);
        }
        return sum;
    }
    bool operator==(const Point& other) const {
        const double tolerance = 1e-6; // Tolerancia a las diferencias
        if (coordinates.size() != other.coordinates.size()) return false;
        for (size_t i = 0; i < coordinates.size(); ++i) {
            if (std::abs(coordinates[i] - other.coordinates[i]) > tolerance) {
                return false;
            }
        }
        return true;
    }

    double& operator[](size_t index) {
        if (index >= coordinates.size()) {
            throw std::out_of_range("Index out of range for Point coordinates.");
        }
        return coordinates[index];
    }

    const double& operator[](size_t index) const {
        if (index >= coordinates.size()) {
            throw std::out_of_range("Index out of range for Point coordinates.");
        }
        return coordinates[index];
    }

    bool operator<(const Point& other) const {
        return coordinates < other.coordinates;
    }

    bool operator>(const Point& other) const {
        return coordinates > other.coordinates;
    }

    bool operator<=(const Point& other) const {
        return !(other < *this);
    }

    bool operator>=(const Point& other) const {
        return !(*this < other);
    }

};

// Especialización de std::hash para Point
namespace std {
    template <>
    struct hash<Point> {
        size_t operator()(const Point& p) const {
            size_t result = 0;
            for (const auto& val : p.coordinates) {
                result ^= std::hash<double>{}(val) + 0x9e3779b9 + (result << 6) + (result >> 2);
            }
            return result;
        }
    };
}

#endif // POINT_H
