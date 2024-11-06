#ifndef BINARY_TREE_H
#define BINARY_TREE_H

#include <stdio.h>

enum tree_error_t {
    BINARY_TREE_SUCCESS = 0,
    BINARY_TREE_MEMORY_ERROR = 1,
    BINARY_TREE_INVALID_CAPACITY = 2,
    BINARY_TREE_NULL = 3,
    BINARY_TREE_DUMP_ERROR = 4,
    BINARY_TREE_OVERFLOW = 5,
};

typedef int tree_data_t;

struct tree_node_t {
    tree_data_t data;
    tree_node_t *left;
    tree_node_t *right;
};

struct binary_tree_t {
    tree_node_t *data;
    tree_node_t *root;
    tree_node_t *free;

    size_t dumps_number;
    FILE *general_dump;
};

tree_error_t binary_tree_init(binary_tree_t *tree, size_t capacity);
tree_error_t binary_tree_insert(binary_tree_t *tree, tree_data_t data);
tree_error_t binary_tree_dump(binary_tree_t *tree, tree_node_t *current_node);
tree_error_t binary_tree_print_preorder(binary_tree_t *tree);
tree_error_t binary_tree_print_postorder(binary_tree_t *tree);
tree_error_t binary_tree_print_inorder(binary_tree_t *tree);
tree_error_t binary_tree_dtor(binary_tree_t *tree);

#endif
