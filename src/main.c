// #define DEBUG

#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <sys/stat.h>
#include <stdarg.h>

#include "log.h"
#include "head.h"
#include "eval.h"
#include "color_print.h"

#ifdef DEBUG
    #define ON_DBG(...) __VA_ARGS__
#else
    #define ON_DBG(...)
#endif

struct Node_t* new_node   (int type, double value);

int print_tree_preorder   (struct Node_t* root, FILE* filename);

int print_tree_postorder  (struct Node_t* root);

int print_tree_inorder    (struct Node_t* root);

void printing_in_all_ways (struct Node_t* node);

int delete_sub_tree       (struct Node_t* node);

int buffer_dtor (struct Buffer_t* buffer);

int destructor (struct Node_t* node, struct Buffer_t* buffer);

void print_tree_preorder_for_file (struct Node_t* root, FILE* filename);

int make_graph (struct Node_t* root);

void clean_buffer (void);

void dump_in_log_file (struct Node_t* node, const char* reason);

struct Node_t* read_example (FILE* file, struct Buffer_t* buffer);

struct Node_t* read_node (int level, struct Buffer_t* buffer);

struct Node_t* read_node_double (struct Node_t* node, int level, struct Buffer_t* buffer);

struct Node_t* read_node_symbol (struct Node_t* node, int level, struct Buffer_t* buffer);

int main (int argc, const char* argv[])
{
    struct Buffer_t buffer = {};

    const char* input = (argc >= 2)? argv[1] : "../build/example.txt";

    FILE* example = fopen (input, "rb");
    if (!example) { fprintf (stderr, "Error: File '%s' NOT opened", input); perror (""); return 1; }

    struct Node_t* root = read_example (example, &buffer);
    if (root == NULL)
        return 1;

    open_log_file ("../build/dump.html");

    dump_in_log_file (root, "before eval");

    printing_in_all_ways (root);

    printf ("\n""answer = %lg\n", eval (root));

    dump_in_log_file (root, "after eval --- end of programm");

    close_log_file ();

    destructor (root, &buffer);

    return 0;
}

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

#define INDENT printf ("%*s", level*2, "")

struct Node_t* read_node (int level, struct Buffer_t* buffer)
{
    assert (buffer && "buffer is NULL in read_node()\n");

    ON_DBG ( printf ("\n"); )
    ON_DBG ( INDENT; printf ("Starting read_node(). Cur = %.10s..., [%p]. buffer_ptr = [%p]\n",
               buffer->current_ptr,  buffer->current_ptr, buffer->buffer_ptr); )

    int n = -1;
    sscanf (buffer->current_ptr, " ( %n", &n);
    if (n < 0) { ON_DBG ( INDENT; printf ("No '(' found. Return NULL.\n"); ) return NULL; }

    buffer->current_ptr += n;

    ON_DBG ( INDENT; printf ("Got an '('. Creating a node. Cur = %.10s..., [%p]. buffer_ptr = [%p]\n",
               buffer->current_ptr,  buffer->current_ptr, buffer->buffer_ptr); )

    struct Node_t* node = NULL;

    node = new_node (0, 0);

    n = -1;
    int bgn = 0;
    int end = 0;
    sscanf (buffer->current_ptr, " %n%*[^()]%n %n", &bgn, &end, &n);
    if (n < 0) { ON_DBG ( INDENT; printf ("No value found. Return NULL.\n"); ) return NULL; }

    char temp = 0;
    double temp_dbl = 0;

    if ( sscanf (buffer->current_ptr + bgn, "%lg", &temp_dbl) != 0)
    {
        node->value = temp_dbl;
        node->type = NUM;

        ON_DBG ( INDENT; printf ("Got a NAME: '%lg'. Cur = %.10s..., [%p]. buffer_ptr = [%p]\n",
        node->value, buffer->current_ptr, buffer->current_ptr, buffer->buffer_ptr); )

        buffer->current_ptr += n;

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

        ON_DBG ( INDENT; printf ("Got a NAME: '%c'. n = %d. Cur = %.10s..., [%p]. buffer_ptr = [%p]\n",
                (int) node->value, n, buffer->current_ptr, buffer->current_ptr, buffer->buffer_ptr); )

        buffer->current_ptr += n;

        read_node_symbol (node, level, buffer);
    }
}

struct Node_t* read_node_double (struct Node_t* node, int level, struct Buffer_t* buffer)
{
    ON_DBG ( INDENT; printf ("CURRENT_PTR was shifted. Cur = %.10s..., [%p]. buffer_ptr = [%p]\n",
                            buffer->current_ptr, buffer->current_ptr, buffer->buffer_ptr); )
    int n = -1;
    char chr = '\0';
    sscanf (buffer->current_ptr, " %c %n", &chr, &n);
    if (n < 0) { ON_DBG ( INDENT; printf ("No ending symbol (1) found. Return NULL.\n"); ) return NULL; }

    if (chr == ')')
    {
        buffer->current_ptr += n;

        ON_DBG ( INDENT; printf ("Got a ')', SHORT Node END (value = '%lg'). Return node. Cur = %.10s..., [%p]. buffer_ptr = [%p]\n",
                node->value, buffer->current_ptr, buffer->current_ptr, buffer->buffer_ptr); )

        return node;
    }

    ON_DBG ( INDENT; printf ("')' NOT found. Supposing a left/right subtree. Reading left node. Cur = %.10s..., [%p]. buffer_ptr = [%p]\n",
            buffer->current_ptr, buffer->current_ptr, buffer->buffer_ptr); )

    node->left = read_node (level + 1, buffer);

    ON_DBG ( INDENT; printf ("\n" "LEFT subtree read. value of left root = '%lg'\n\n", node->left->value); )

    ON_DBG ( printf ("Reading right node. Cur = %.10s...\n", buffer->current_ptr); )

    node->right = read_node (level + 1, buffer);

    ON_DBG ( INDENT; printf ("\n" "RIGHT subtree read. value of right root = '%lg'\n", node->right->value); )

    chr = '\0';
    sscanf (buffer->current_ptr, " %c %n", &chr, &n);
    if (n < 0) { ON_DBG ( INDENT; printf ("No ending symbol (2) found. Return NULL.\n"); ) return NULL; }

    if (chr == ')')
    {
        buffer->current_ptr += n;

        ON_DBG ( INDENT; printf ("Got a ')', FULL Node END (value = '%lg'). Return node. Cur = %.10s..., [%p]. buffer_ptr = [%p]\n",
                node->value, buffer->current_ptr, buffer->current_ptr, buffer->buffer_ptr); )

        return node;
    }

    ON_DBG ( INDENT; printf ("Does NOT get ')'. Syntax error. Return NULL. Cur = %.20s..., [%p]. buffer_ptr = [%p]\n",
            buffer->current_ptr, buffer->current_ptr, buffer->buffer_ptr); )

    return NULL;
}

struct Node_t* read_node_symbol (struct Node_t* node, int level, struct Buffer_t* buffer)
{
    ON_DBG ( INDENT; printf ("Shifted CURRENT_PTR: '%c'. Cur = %.10s..., [%p]. buffer_ptr = [%p]\n",
            (int) node->value, buffer->current_ptr, buffer->current_ptr, buffer->buffer_ptr); )

    int n = -1;
    char chr = '\0';
    sscanf (buffer->current_ptr, " %c %n", &chr, &n);
    if (n < 0) { ON_DBG ( INDENT; printf ("No ending symbol (1) found. Return NULL.\n"); ) return NULL; }

    if (chr == ')')
    {
        buffer->current_ptr += n;

        ON_DBG ( INDENT; printf ("Got a ')', SHORT Node END (value = '%c'). Return node. Cur = %.10s..., [%p]. buffer_ptr = [%p]\n",
                    (int) node->value, buffer->current_ptr, buffer->current_ptr, buffer->buffer_ptr); )

        return node;
    }

    ON_DBG ( INDENT; printf ("')' NOT found. Supposing a left/right subtree. Reading left node. Cur = %.10s..., [%p]. buffer_ptr = [%p]\n",
                buffer->current_ptr, buffer->current_ptr, buffer->buffer_ptr); )

    node->left = read_node (level + 1, buffer);

    ON_DBG ( INDENT; printf ("\n" "LEFT subtree read. value of left root = '%c'\n\n", (int) node->left->value); )

    ON_DBG ( printf ("Reading right node. Cur = %.10s...\n", buffer->current_ptr); )

    node->right = read_node (level + 1, buffer);

    ON_DBG ( INDENT; printf ("\n" "RIGHT subtree read. value of right root = '%c'\n", (int) node->right->value); )

    chr = '\0';
    sscanf (buffer->current_ptr, " %c %n", &chr, &n);
    if (n < 0) { ON_DBG ( INDENT; printf ("No ending symbol (2) found. Return NULL.\n"); ) return NULL; }

    if (chr == ')')
    {
        buffer->current_ptr += n;

        ON_DBG ( INDENT; printf ("Got a ')', FULL Node END (value = '%c'). Return node. Cur = %.10s..., [%p]. buffer_ptr = [%p]\n",
                                (int) node->value, buffer->current_ptr, buffer->current_ptr, buffer->buffer_ptr); )

        return node;
    }

    ON_DBG ( INDENT; printf ("Does NOT get ')'. Syntax error. Return NULL. Cur = %.20s..., [%p]. buffer_ptr = [%p]\n",
            buffer->current_ptr, buffer->current_ptr, buffer->buffer_ptr); )

    return NULL;
}

#undef INDENT

struct Node_t* new_node (int type, double value)
{
    struct Node_t* node = (struct Node_t*) calloc (1, sizeof(*node));
    assert (node); //FIXME

    node->type = type;
    node->value = value;

    node->left  = NULL;
    node->right = NULL;

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

int print_tree_preorder (struct Node_t* root, FILE* filename)
{
    if (!root)
        return 1; //FIXME

    fprintf (filename, " ( ");

    if (root->type == NUM)
        fprintf (filename, "%lg",  root->value);
    else
        fprintf (filename, "%c", (int) root->value);

    if (root->left)  print_tree_preorder (root->left,  filename);

    if (root->right) print_tree_preorder (root->right, filename);

    fprintf (filename, " ) ");

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

int print_tree_inorder (struct Node_t* root)
{
    if (!root)
        return 1; //FIXME

    printf (" ( ");

    if (root->left)  print_tree_inorder (root->left);

    if (root->type == NUM)
        printf ("%lg", root->value);
    else
        printf ("%c", (int) root->value);

    if (root->right) print_tree_inorder (root->right);

    printf (" ) ");

    return 0;
}

void printing_in_all_ways (struct Node_t* node)
{
    printf ("\n\n");

    printf ("\npreorder: ");
    print_tree_preorder  (node, stdout);

    printf ("\npostorder: ");
    print_tree_postorder (node);

    printf ("\ninorder: ");
    print_tree_inorder   (node);

    printf ("\n\n");
}

void print_tree_preorder_for_file (struct Node_t* root, FILE* filename)
{
    if (!root)
        return ; //FIXME

    if (root->type == NUM)
        fprintf (filename, "node%p [shape=Mrecord; label = \" { [%p] | type = %d (NUM) | value = %.2g | { left = [%p] | right = [%p] } }\"; style = filled; fillcolor = \"#FFD700\"];\n",
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

void dump_in_log_file (struct Node_t* node, const char* reason)
{
    make_graph (node);
    write_log_file (reason);
}

void clean_buffer(void)
{
    while((getchar()) != '\n') {;}
}
