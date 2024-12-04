#ifndef CREATE_INDEX_H
#define CREATE_INDEX_H

#include <vector>
#include "tree_node.h"  
std::vector<TreeNode*> create_index(int K, int L, int n, const std::vector<std::vector<std::vector<int>>>& EP, int max_size);

#endif // CREATE_INDEX_H