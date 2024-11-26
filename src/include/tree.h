#pragma once

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
};

enum symbols
{
    NUM = 1,
    OP  = 2,
    VAR = 3
};

enum operations
{
    ADD = '+',
    SUB = '-',
    DIV = '/',
    MUL = '*',
    SIN = 's',
    COS = 'c',
    POW = '^',
    LOG = 'l'
};

struct Node_t* new_node (int type, double value, struct Node_t* node_left, struct Node_t* node_right);

int print_tree_preorder    (struct Node_t* root, FILE* filename);

int print_tree_postorder   (struct Node_t* root);

int print_tree_inorder     (struct Node_t* root, struct Node_t* parent, FILE* file);

void  tex_expression_print (struct Node_t* root, struct Node_t* diff_node, FILE* file);

FILE* tex_file_open  (const char* filename);

void  tex_file_close (FILE* file);

int priority (int op);

int its_func_is_root (struct Node_t* root);

void print_func (struct Node_t* root, FILE* file);

void printing_in_all_ways (struct Node_t* node);

int delete_sub_tree       (struct Node_t* node);

int buffer_dtor (struct Buffer_t* buffer);

int destructor  (struct Node_t* node, struct Buffer_t* buffer);

void print_tree_preorder_for_file (struct Node_t* root, FILE* filename);

int make_graph  (struct Node_t* root);

void clean_buffer (void);

void dump_in_log_file (struct Node_t* node, const char* reason);

struct Node_t* read_example (FILE* file, struct Buffer_t* buffer);

struct Node_t* read_node (int level, struct Buffer_t* buffer);

struct Node_t* read_node_double (struct Node_t* node, int level, struct Buffer_t* buffer);

struct Node_t* read_node_symbol (struct Node_t* node, int level, struct Buffer_t* buffer);
