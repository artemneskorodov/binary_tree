#include <stdio.h>
#include <stdlib.h>

#include "binary_tree.h"
#include "custom_assert.h"
#include "colors.h"

static tree_error_t binary_tree_print_nodes          (tree_node_t   *node,
                                                      FILE          *output_file,
                                                      tree_node_t   *current_node,
                                                      size_t         level);

static tree_error_t binary_tree_print_node_preorder  (tree_node_t   *node);

static tree_error_t binary_tree_print_node_postorder (tree_node_t   *node);

static tree_error_t binary_tree_print_node_inorder   (tree_node_t   *node);

static tree_error_t binary_tree_dump_child           (tree_node_t   *node,
                                                      FILE          *output_file,
                                                      tree_node_t   *current_node,
                                                      size_t         level,
                                                      tree_node_t   *child);

static tree_error_t binary_tree_create_dot_file      (binary_tree_t *tree,
                                                      tree_node_t   *current_node,
                                                      const char    *filename);

static tree_error_t binary_tree_create_new_node      (binary_tree_t *tree,
                                                      tree_node_t  **node,
                                                      tree_data_t    data);

static tree_error_t binary_tree_initialize_dump      (binary_tree_t *tree);

static tree_error_t binary_tree_initialize_data      (binary_tree_t *tree,
                                                      size_t         capacity);

static tree_error_t binary_tree_initialize_free      (binary_tree_t *tree,
                                                      size_t         capacity);

static const char *root_color         = "#f4acb7";
static const char *node_color         = "#d8e2dc";
static const char *leaf_color         = "#ffe5d9";
static const char *current_node_color = "#9d8189";

static const char *general_dump_filename = "logs/tree.html";
static const char *logs_direction        = "logs";
static const char *png_dump_direction    = "img";
static const char *dot_dump_direction    = "dot";

static const size_t system_command_max_length = 256;

tree_error_t binary_tree_init(binary_tree_t *tree, size_t capacity) {
    C_ASSERT(tree     != NULL, return BINARY_TREE_NULL            );
    C_ASSERT(capacity > 0,     return BINARY_TREE_INVALID_CAPACITY);

    tree_error_t error_code = BINARY_TREE_SUCCESS;
    if((error_code = binary_tree_initialize_data(tree, capacity)) != BINARY_TREE_SUCCESS) {
        return error_code;
    }
    if((error_code = binary_tree_initialize_dump(tree)) != BINARY_TREE_SUCCESS) {
        return error_code;
    }
    if((error_code = binary_tree_initialize_free(tree, capacity)) != BINARY_TREE_SUCCESS) {
        return error_code;
    }
    return BINARY_TREE_SUCCESS;
}

tree_error_t binary_tree_dtor(binary_tree_t *tree) {
    free(tree->data);
    fclose(tree->general_dump);
    return BINARY_TREE_SUCCESS;
}

tree_error_t binary_tree_insert(binary_tree_t *tree, tree_data_t data) {
    C_ASSERT(tree != NULL, return BINARY_TREE_NULL);

    tree_node_t *new_node = NULL;
    tree_error_t error_code = BINARY_TREE_SUCCESS;
    if((error_code = binary_tree_create_new_node(tree, &new_node, data)) != BINARY_TREE_SUCCESS) {
        return error_code;
    }

    tree_node_t *current_node = tree->root;
    if(current_node == NULL) {
        tree->root = new_node;
        return BINARY_TREE_SUCCESS;
    }

    while(true) {
        if(data > current_node->data) {
            if(current_node->right == NULL) {
                current_node->right = new_node;
                return BINARY_TREE_SUCCESS;
            }
            else {
                current_node = current_node->right;
            }
        }
        else {
            if(current_node->left == NULL) {
                current_node->left = new_node;
                return BINARY_TREE_SUCCESS;
            }
            else {
                current_node = current_node->left;
            }
        }
    }
}

tree_error_t binary_tree_dump(binary_tree_t *tree, tree_node_t *current_node) {
    C_ASSERT(tree != NULL, return BINARY_TREE_NULL);

    char dot_file_name[256] = {};
    char png_file_name[256] = {};
    sprintf(dot_file_name,
            "%s/%s/tree_dump%04llx.dot",
            logs_direction,
            dot_dump_direction,
            tree->dumps_number);
    sprintf(png_file_name,
            "%s/tree_dump%04llx.png",
            png_dump_direction,
            tree->dumps_number);

    tree_error_t error_code = BINARY_TREE_SUCCESS;
    if((error_code = binary_tree_create_dot_file(tree,
                                                 current_node,
                                                 dot_file_name)) != BINARY_TREE_SUCCESS) {
        return error_code;
    }

    char system_dot_command[256] = {};
    sprintf(system_dot_command,
            "dot %s -Tpng -o %s/%s",
            dot_file_name,
            logs_direction,
            png_file_name);
    system(system_dot_command);

    if(fprintf(tree->general_dump,
               "<h2>Dump %llu</h2>\r\n"
               "<img src = \"%s\">\r\n",
               tree->dumps_number,
               png_file_name) < 0) {
        return BINARY_TREE_DUMP_ERROR;
    }
    fflush(tree->general_dump);

    tree->dumps_number++;
    return BINARY_TREE_SUCCESS;
}

tree_error_t binary_tree_print_nodes(tree_node_t *node, FILE *output_file, tree_node_t *current_node, size_t level) {
    C_ASSERT(node        != NULL, return BINARY_TREE_DUMP_ERROR);
    C_ASSERT(output_file != NULL, return BINARY_TREE_DUMP_ERROR);

    const char *color = NULL;
    if(level == 0) {
        color = root_color;
    }
    else if(node == current_node) {
        color = current_node_color;
    }
    else if(node->right == NULL && node->left == NULL) {
        color = leaf_color;
    }
    else {
        color = node_color;
    }

    if(fprintf(output_file,
               "node%p[rank = %llu, "
               "label = \"{ data = %d | { <left> LEFT | <right> RIGHT } }\", "
               "fillcolor = \"%s\"];\r\n",
               node,
               level,
               node->data,
               color) < 0) {
        return BINARY_TREE_DUMP_ERROR;
    }

    tree_error_t error_code = BINARY_TREE_SUCCESS;
    if(node->left != NULL) {
        if((error_code = binary_tree_dump_child(node,
                                                output_file,
                                                current_node,
                                                level + 1,
                                                node->left )) != BINARY_TREE_SUCCESS) {
            return error_code;
        }
    }
    if(node->right != NULL) {
        if((error_code = binary_tree_dump_child(node,
                                                output_file,
                                                current_node,
                                                level + 1,
                                                node->right)) != BINARY_TREE_SUCCESS) {
            return error_code;
        }
    }
    return BINARY_TREE_SUCCESS;
}

tree_error_t binary_tree_print_preorder(binary_tree_t *tree) {
    C_ASSERT(tree != NULL, return BINARY_TREE_NULL);

    return binary_tree_print_node_preorder(tree->root);
}

tree_error_t binary_tree_print_postorder(binary_tree_t *tree) {
    C_ASSERT(tree != NULL, return BINARY_TREE_NULL);

    return binary_tree_print_node_postorder(tree->root);
}

tree_error_t binary_tree_print_inorder(binary_tree_t *tree) {
    C_ASSERT(tree != NULL, return BINARY_TREE_NULL);

    return binary_tree_print_node_inorder(tree->root);
}

tree_error_t binary_tree_print_node_preorder(tree_node_t *node) {
    printf("(");

    if(node == NULL) {
        printf(")");
        return BINARY_TREE_SUCCESS;
    }

    tree_error_t error_code = BINARY_TREE_SUCCESS;

    printf("%d", node->data);

    if((error_code = binary_tree_print_node_preorder(node->left )) != BINARY_TREE_SUCCESS) {
        return error_code;
    }

    if((error_code = binary_tree_print_node_preorder(node->right)) != BINARY_TREE_SUCCESS) {
        return error_code;
    }

    printf(")");
    return BINARY_TREE_SUCCESS;
}

tree_error_t binary_tree_print_node_postorder(tree_node_t *node) {
    printf("(");

    if(node == NULL) {
        printf(")");
        return BINARY_TREE_SUCCESS;
    }

    tree_error_t error_code = BINARY_TREE_SUCCESS;

    if((error_code = binary_tree_print_node_postorder(node->left )) != BINARY_TREE_SUCCESS) {
        return error_code;
    }

    if((error_code = binary_tree_print_node_postorder(node->right)) != BINARY_TREE_SUCCESS) {
        return error_code;
    }

    printf("%d", node->data);

    printf(")");
    return BINARY_TREE_SUCCESS;
}

tree_error_t binary_tree_print_node_inorder(tree_node_t *node) {
    printf("(");

    if(node == NULL) {
        printf(")");
        return BINARY_TREE_SUCCESS;
    }

    tree_error_t error_code = BINARY_TREE_SUCCESS;

    if((error_code = binary_tree_print_node_inorder(node->left )) != BINARY_TREE_SUCCESS) {
        return error_code;
    }

    printf("%d", node->data);

    if((error_code = binary_tree_print_node_inorder(node->right)) != BINARY_TREE_SUCCESS) {
        return error_code;
    }

    printf(")");
    return BINARY_TREE_SUCCESS;
}

tree_error_t binary_tree_dump_child(tree_node_t *node,
                                    FILE        *output_file,
                                    tree_node_t *current_node,
                                    size_t       level,
                                    tree_node_t *child) {
    C_ASSERT(node        != NULL, return BINARY_TREE_DUMP_ERROR);
    C_ASSERT(output_file != NULL, return BINARY_TREE_DUMP_ERROR);
    C_ASSERT(child       != NULL, return BINARY_TREE_DUMP_ERROR);

    tree_error_t error_code = BINARY_TREE_SUCCESS;
    const char *direction = NULL;
    if(node->left == child) {
        direction = "<left>";
    }
    else {
        direction = "<right>";
    }

    if(fprintf(output_file,
               "node%p:%s->node%p;\r\n",
               node,
               direction,
               child) < 0) {
        return BINARY_TREE_DUMP_ERROR;
    }

    if((error_code = binary_tree_print_nodes(child,
                                             output_file,
                                             current_node,
                                             level + 1)) != BINARY_TREE_SUCCESS) {
        return error_code;
    }

    return BINARY_TREE_SUCCESS;
}

tree_error_t binary_tree_create_dot_file(binary_tree_t *tree,
                                         tree_node_t   *current_node,
                                         const char    *filename) {
    C_ASSERT(tree     != NULL, return BINARY_TREE_NULL      );
    C_ASSERT(filename != NULL, return BINARY_TREE_DUMP_ERROR);

    FILE *dot_file = fopen(filename, "wb");
    if(dot_file == NULL) {
        color_printf(RED_TEXT, BOLD_TEXT, DEFAULT_BACKGROUND,
                     "Error while opening dot file.\r\n");
        return BINARY_TREE_DUMP_ERROR;
    }

    if(fprintf(dot_file,
               "digraph {\r\n"
               "node[shape = Mrecord, style = filled];\r\n"
               "rankdir = TB;\r\n") < 0) {
        return BINARY_TREE_DUMP_ERROR;
    }

    tree_error_t error_code = BINARY_TREE_SUCCESS;
    if((error_code = binary_tree_print_nodes(tree->root,
                                             dot_file,
                                             current_node, 0)) != BINARY_TREE_SUCCESS) {
        return error_code;
    }

    fprintf(dot_file, "}\r\n");
    fclose(dot_file);

    return BINARY_TREE_SUCCESS;
}

tree_error_t binary_tree_create_new_node(binary_tree_t *tree,
                                         tree_node_t  **node,
                                         tree_data_t    data) {
    C_ASSERT(tree != NULL, return BINARY_TREE_NULL);
    C_ASSERT(node != NULL, return BINARY_TREE_NULL);

    if(tree->free == NULL) {
        color_printf(RED_TEXT, BOLD_TEXT, DEFAULT_BACKGROUND,
                     "Binary tree overflow. Create tree with more memory available.\r\n");
        return BINARY_TREE_OVERFLOW;
    }

    *node          = tree->free;
    tree->free     = tree->free->right;

    (*node)->data  = data;
    (*node)->left  = NULL;
    (*node)->right = NULL;

    return BINARY_TREE_SUCCESS;
}

tree_error_t binary_tree_initialize_dump(binary_tree_t *tree) {
    C_ASSERT(tree != NULL, return BINARY_TREE_NULL);

    char make_direction[system_command_max_length] = {};

    sprintf(make_direction,
            "md %s",
            logs_direction);
    system(make_direction);

    sprintf(make_direction,
            "md %s\\%s",
            logs_direction,
            png_dump_direction);
    system(make_direction);

    sprintf(make_direction,
            "md %s\\%s",
            logs_direction,
            dot_dump_direction);
    system(make_direction);

    tree->general_dump = fopen(general_dump_filename, "wb");
    if(tree->general_dump == NULL) {
        color_printf(RED_TEXT, BOLD_TEXT, DEFAULT_BACKGROUND,
                     "Error while opening general dump file.\r\n");
        return BINARY_TREE_DUMP_ERROR;
    }

    return BINARY_TREE_SUCCESS;
}

tree_error_t binary_tree_initialize_data(binary_tree_t *tree,
                                         size_t         capacity) {
    C_ASSERT(tree != NULL, return BINARY_TREE_NULL);

    tree->data = (tree_node_t *)calloc(capacity, sizeof(tree->data[0]));
    if(tree->data == NULL) {
        color_printf(RED_TEXT, BOLD_TEXT, DEFAULT_BACKGROUND,
                     "Error while allocating memory to tree data.\r\n");
        return BINARY_TREE_MEMORY_ERROR;
    }

    return BINARY_TREE_SUCCESS;
}

tree_error_t binary_tree_initialize_free(binary_tree_t *tree,
                                         size_t         capacity) {
    C_ASSERT(tree != NULL, return BINARY_TREE_NULL);

    tree->free = tree->data;
    for(size_t element = 0; element + 1 < capacity; element++) {
        tree->data[element].right = tree->data + element + 1;
    }
    tree->root = NULL;

    return BINARY_TREE_SUCCESS;
}
