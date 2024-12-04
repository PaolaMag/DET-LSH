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
    std::vector<TreeNode*> children;

    TreeNode() : left(nullptr), right(nullptr) {}
    void add_entry(const Point& point, int value) {
        entries.push_back(std::make_pair(point, value));
    }
    bool is_leaf() const {
    return left == nullptr && right == nullptr;}
    void add_child(TreeNode* child) {
        children.push_back(child);
    }
};

#endif // TREE_NODE_H
