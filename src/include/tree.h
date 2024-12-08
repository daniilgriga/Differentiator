#pragma once

#include "tokens.h"

struct Node_t
{
    int type;

    double value;

    struct Node_t* left;
    struct Node_t* right;
};

struct Buffer_t
{
    char* buffer_ptr;
    char* current_ptr;

    long file_size;
};

struct Node_t* new_node (int type, double value, struct Node_t* node_left, struct Node_t* node_right);

int print_tree_preorder    (struct Node_t* node, FILE* filename);

int print_tree_postorder   (struct Node_t* node);

int tex_printf_tree_inorder (struct Node_t* node, struct Node_t* parent, struct Context_t* context);

void tex_printf_expression (struct Node_t* node, struct Node_t* diff_node, struct Context_t* context, int first_equation);

void tex_printf_tree (struct Node_t* node, struct Node_t* diff_node, struct Context_t* context, const char* message, int first_equation);

int priority (int op);

int its_func_is_root (struct Node_t* node);

void print_func_in_tex (struct Node_t* node, struct Node_t* parent, struct Context_t* context);

int delete_sub_tree       (struct Node_t* node);

int delete_node (struct Node_t* node);

int buffer_dtor (struct Buffer_t* buffer);

int destructor  (struct Node_t* node, struct Buffer_t* buffer);

void print_tree_preorder_for_file (struct Node_t* node, FILE* filename);

int make_graph  (struct Node_t* node);

void clean_buffer (void);

void dump_in_log_file (struct Node_t* node, const char* reason, ...);

struct Node_t* read_example (FILE* file, struct Buffer_t* buffer);

struct Node_t* read_node (int level, struct Buffer_t* buffer);

struct Node_t* read_node_double (struct Node_t* node, int level, struct Buffer_t* buffer);

struct Node_t* read_node_symbol (struct Node_t* node, int level, struct Buffer_t* buffer);
