#ifndef BREAKPOINTS_H
#define BREAKPOINTS_H

#include <vector>
#include "tree_node.h"  

std::vector<std::vector<std::vector<int>>> breakpoints_selection(int K, int L, int n, const std::vector<int>& P, int ns, int Nr);
std::vector<std::vector<std::vector<int>>>dynamic_encoding(int K, int L, int n, const vector<vector<int>>& P, int ns, int Nr);
std::vector<TreeNode*> create_index(int K, int L, int n, const std::vector<std::vector<int>>& EP, int max_size);

#endif // BREAKPOINTS_H
