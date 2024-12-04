#ifndef TREE_NODE_H
#define TREE_NODE_H

#include <vector>
#include <utility> 
#include "point.h" 

struct Point;
struct TreeNode {
    std::vector<std::pair<Point, int>> entries; 
    TreeNode* left;   
    TreeNode* right;  

    TreeNode() : left(nullptr), right(nullptr) {}
    void add_entry(const Point& point, int value) {
        entries.push_back(std::make_pair(point, value));
    }
};

#endif // TREE_NODE_H
