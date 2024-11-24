#define DEBUG

#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <sys/stat.h>
#include <stdarg.h>

#include "log.h"
#include "tree.h"
#include "eval.h"
#include "diff.h"
#include "color_print.h"

#ifdef DEBUG
    #define ON_DBG(...) __VA_ARGS__
#else
    #define ON_DBG(...)
#endif

struct Node_t* read_example (FILE* file, struct Buffer_t* buffer)
{
    assert (file   && "FILE* is NULL\n");
    assert (buffer && "buffer is NULL\n");

    struct stat st = {};
    fstat (fileno (file), &st);
    long file_size = st.st_size;

    buffer->buffer_ptr = (char*) calloc ( (size_t) file_size + 1, sizeof(buffer->buffer_ptr[0]));

    ON_DBG ( printf ("\n\n\n\nbuffer->buffer_ptr = [%p]\n\n\n\n", buffer->buffer_ptr); )

    size_t count = fread (buffer->buffer_ptr, sizeof(buffer->buffer_ptr[0]), (size_t) file_size, file);
    if ((long) count != file_size)
    {
        printf ("count = %zu != file_size = %ld", count, file_size);
        return NULL;
    }

    fclose (file);

    buffer->current_ptr = buffer->buffer_ptr;

    return read_node (0, buffer);
}

#define INDENT      fprintf (stderr, "%*s", level*2, "")

#define PRINT(...)  { int n =  fprintf (stderr, "%s:%d: ", __FILE__, __LINE__);                      \
                          n += fprintf (stderr, "%*s", level*2, "");                                 \
                          n += fprintf (stderr, __VA_ARGS__);                                        \
                          n += fprintf (stderr, "%*s" "Cur = '%.25s'..., [%p].\n",                   \
                                                100-n, "", buffer->current_ptr, buffer->current_ptr); }

struct Node_t* read_node (int level, struct Buffer_t* buffer)
{
    assert (buffer && "buffer is NULL in read_node()\n");

    ON_DBG ( fprintf (stderr, "\n"); )
    ON_DBG ( PRINT ("Starting read_node()."); )

    int n = -1;
    sscanf (buffer->current_ptr, " ( %n", &n);
    if (n < 0) { ON_DBG ( PRINT ("No '(' found. Return NULL."); ) return NULL; }

    buffer->current_ptr += n;

    ON_DBG ( PRINT ("Got an '('. Creating a node."); )

    struct Node_t* node = NULL;

    node = new_node (0, 0, NULL, NULL);

    n = -1;
    int bgn = 0;
    int end = 0;
    sscanf (buffer->current_ptr, " %n%*[^()]%n %n", &bgn, &end, &n);
    if (n < 0) { PRINT ("No value found. Return NULL."); return NULL; }

    char temp = 0;
    double temp_dbl = 0;

    if (sscanf (buffer->current_ptr + bgn, " %lg ", &temp_dbl) != 0)
    {
        node->value = temp_dbl;
        node->type = NUM;

        ON_DBG ( PRINT ("IN DOUBLE: Got a NAME: '%lg'.", node->value); )

        buffer->current_ptr += n;
        ON_DBG ( PRINT ("CURRENT_PTR was shifted."); )

        read_node_double (node, level, buffer);
    }
    else
    {
        sscanf (buffer->current_ptr + bgn, " %c ", &temp);

        node->value = (int) temp;

        if (temp == 'x')
            node->type = VAR;
        else
            node->type = OP;

        ON_DBG ( PRINT ("IN SYMBOL: Got a NAME: '%c' (%d). n = %d.", (int) node->value, (int) node->value, n); )

        buffer->current_ptr += n;
        ON_DBG ( PRINT ("CURRENT_PTR was shifted."); )

        read_node_symbol (node, level, buffer);
    }
}

struct Node_t* read_node_double (struct Node_t* node, int level, struct Buffer_t* buffer)
{
    int n = -1;
    char chr = '\0';
    sscanf (buffer->current_ptr, " %c %n", &chr, &n);
    if (n < 0) { ON_DBG ( PRINT ("No ending symbol (1) found. Return NULL."); ) return NULL; }

    if (chr == ')')
    {
        buffer->current_ptr += n;

        ON_DBG ( PRINT ("Got a ')', SHORT Node END (value = '%lg'). Return node.", node->value); )

        return node;
    }

    ON_DBG ( PRINT ("')' NOT found. Supposing a left/right subtree. Reading left node."); )

    node->left = read_node (level + 1, buffer);

    ON_DBG ( PRINT ("LEFT subtree read. value of left root = '%lg'", node->left->value); )

    ON_DBG ( PRINT ("Reading right node."); )

    node->right = read_node (level + 1, buffer);

    ON_DBG ( PRINT ("RIGHT subtree read. value of right root = '%lg'", node->right->value); )

    chr = '\0';
    sscanf (buffer->current_ptr, " %c %n", &chr, &n);
    if (n < 0) { ON_DBG ( PRINT ("No ending symbol (2) found. Return NULL."); ) return NULL; }

    if (chr == ')')
    {
        buffer->current_ptr += n;

        ON_DBG ( PRINT ("Got a ')', FULL Node END (value = '%lg'). Return node.", node->value); )

        return node;
    }

    ON_DBG ( PRINT ("Does NOT get ')'. Syntax error. Return NULL."); )

    return NULL;
}

struct Node_t* read_node_symbol (struct Node_t* node, int level, struct Buffer_t* buffer)
{
    int n = -1;
    char chr = '\0';
    sscanf (buffer->current_ptr, " %c %n", &chr, &n);
    if (n < 0) { ON_DBG ( PRINT ("No ending symbol (1) found. Return NULL."); ) return NULL; }

    if (chr == ')')
    {
        buffer->current_ptr += n;

        ON_DBG ( PRINT ("Got a ')', SHORT Node END (value = '%c'). Return node.", (int) node->value); )

        return node;
    }

    ON_DBG ( PRINT ("')' NOT found. Supposing a left/right subtree. Reading left node."); )

    node->left = read_node (level + 1, buffer);

    ON_DBG ( PRINT ("LEFT subtree read. value of left root = '%c' (%lg), TYPE %d", (int) node->left->value, node->left->value, node->left->type); )

    ON_DBG ( PRINT ("Reading right node."); )

    node->right = read_node (level + 1, buffer);

    ON_DBG ( PRINT ("RIGHT subtree read. value of right root = '%c' (%lg), TYPE %d", (int) node->right->value, node->right->value, node->right->type); )

    chr = '\0';
    sscanf (buffer->current_ptr, " %c %n", &chr, &n);
    if (n < 0) { ON_DBG ( PRINT ("No ending symbol (2) found. Return NULL."); ) return NULL; }

    if (chr == ')')
    {
        buffer->current_ptr += n;

        ON_DBG ( PRINT ("Got a ')', FULL Node END (value = '%c'). Return node.", (int) node->value); )

        return node;
    }

    ON_DBG ( PRINT ("Does NOT get ')'. Syntax error. Return NULL."); )

    return NULL;
}

#undef INDENT
#undef PRINT

struct Node_t* new_node (int type, double value, struct Node_t* node_left, struct Node_t* node_right)
{
    struct Node_t* node = (struct Node_t*) calloc (1, sizeof(*node));
    assert (node); //FIXME

    node->type  = type;
    node->value = value;

    node->left  = node_left;
    node->right = node_right;

    return node;
}

int delete_sub_tree (struct Node_t* node)
{
    node->value = 0;
    node->type = 0;

    if (node->left)  delete_sub_tree (node->left);

    if (node->right) delete_sub_tree (node->right);

    free (node);

    return 0;
}

int buffer_dtor (struct Buffer_t* buffer)
{
    buffer->current_ptr = NULL;

    free (buffer->buffer_ptr);

    return 0;
}

int destructor (struct Node_t* node, struct Buffer_t* buffer)
{
    delete_sub_tree (node);
    buffer_dtor (buffer);
}

int print_tree_preorder (struct Node_t* root, FILE* file)
{
    if (!root)
        return 1; //FIXME

    fprintf (file, " ( ");

    if (root->type == NUM)
        fprintf (file, "%lg",  root->value);
    else
        fprintf (file, "%c", (int) root->value);

    if (root->left)  print_tree_preorder (root->left,  file);

    if (root->right) print_tree_preorder (root->right, file);

    fprintf (file, " ) ");

    return 0;
}

int print_tree_postorder (struct Node_t* root)
{
    if (!root)
        return 1; //FIXME

    printf (" ( ");

    if (root->left)  print_tree_postorder (root->left);

    if (root->right) print_tree_postorder (root->right);

    if (root->type == NUM)
        printf ("%lg",  root->value);
    else
        printf ("%c", (int) root->value);

    printf (" ) ");

    return 0;
}

int print_tree_inorder (struct Node_t* root, struct Node_t* parent, FILE* file)
{
    if (!root)
        return 1; //FIXME

    bool brackets = (root->left != NULL && root->right != NULL);

    if (root && parent && root->type == OP && parent->type == OP)
        if (priority (root->value) > priority (parent->value))
            brackets = 0;

    if (root->type == OP)
        if (root->value == DIV)
            brackets = 0;

    if (parent == NULL)
        brackets = 0;

    if (brackets)
        fprintf (file, " ( ");

    if (root->left)
        {
        fprintf (file, " { ");
        print_tree_inorder (root->left, root, file);
        fprintf (file, " } ");
        }

    if (root->type == NUM)
        fprintf (file, " %lg ", root->value);
    else
    {
        if (root->type == OP && root->value == MUL)
            fprintf (file, " \\cdot ");
        else
        if (root->type == OP && root->value == DIV)
            fprintf (file, " \\over ");
        else
            fprintf (file, " %c ", (int) root->value);
    }

    if (root->right)
        {
        fprintf (file, " { ");
        print_tree_inorder (root->right, root, file);
        fprintf (file, " } ");
        }

    if (brackets)
        fprintf (file, " ) ");

    return 0;
}

int priority (int op)
{
    switch (op)
    {
        case ADD:
        case SUB: return 1;

        case MUL:
        case DIV: return 2;

        case POW: return 3;

        default:
            printf ("priority of operation %d (%c) dont exists\n", op, op);
            return -1;
    }
}

void printing_in_all_ways (struct Node_t* node)
{
    printf ("\n\n");

    printf ("\npreorder: ");
    print_tree_preorder  (node, stdout);

    printf ("\npostorder: ");
    print_tree_postorder (node);

    printf ("\ninorder: ");
    print_tree_inorder   (node, NULL, stdout);

    printf ("\n\n");
}

void print_tree_preorder_for_file (struct Node_t* root, FILE* filename)
{
    if (!root)
        return ; //FIXME

    if (root->type == NUM)
        fprintf (filename, "node%p [shape=Mrecord; label = \" { [%p] | type = %d (NUM) | value = %g | { left = [%p] | right = [%p] } }\"; style = filled; fillcolor = \"#FFD700\"];\n",
             root, root, root->type, root->value, root->left, root->right);
    else if (root->type == OP)
        fprintf (filename, "node%p [shape=Mrecord; label = \" { [%p] | type = %d (OP) | value = '%c' | { left = [%p] | right = [%p] } }\"; style = filled; fillcolor = \"#20B2AA\"];\n",
             root, root, root->type, (int) root->value, root->left, root->right);
    else if (root->type == VAR)
        fprintf (filename, "node%p [shape=Mrecord; label = \" { [%p] | type = %d (VAR) | value = '%c' | { left = [%p] | right = [%p] } }\"; style = filled; fillcolor = \"#F00000\"];\n",
             root, root, root->type, (int) root->value, root->left, root->right);

    if (root->left)
        fprintf (filename, "node%p -> node%p\n;", root, root->left);

    if (root->right)
        fprintf (filename, "node%p -> node%p\n;", root, root->right);

    if (root->left)  print_tree_preorder_for_file (root->left , filename);

    if (root->right) print_tree_preorder_for_file (root->right, filename);
}

int make_graph (struct Node_t* root)
{
    assert (root);

    FILE* graph_file = fopen ("../build/graph_tree.dot", "w");
    if (graph_file == NULL)
    {
        printf("ERROR open graph_file\n");
        return 1;
    }

    fprintf (graph_file, "digraph\n{\n");

    fprintf (graph_file, "bgcolor=\"transparent\"\n");

    print_tree_preorder_for_file (root, graph_file);
    fprintf (graph_file, "\n");

    fprintf (graph_file, "}");
    fprintf (graph_file, "\n");

    fclose  (graph_file);

    return 0;
}

void tex_print (struct Node_t* root, const char* filename)
{
    FILE* file = fopen (filename, "wb");

    fprintf (file, "\\documentclass{article}\n");
    fprintf (file, "\\begin{document}\n");
    fprintf (file, "\\section{Introduction}\n");
    fprintf (file, "wazzzuuuup, shut up and take my money:");

    fprintf (file, "$$ ");
    print_tree_inorder (root, NULL, file);
    fprintf (file, " $$\n");

    fprintf (file, "\\end{document}\n");

    fclose (file);
}

void dump_in_log_file (struct Node_t* node, const char* reason)
{
    make_graph (node);
    write_log_file (reason);
}

void clean_buffer(void)
{
    while((getchar()) != '\n') {;}
}
