#define DEBUG

#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <sys/stat.h>
#include <stdarg.h>

#include "log.h"
#include "head.h"
#include "color_print.h"

#ifdef DEBUG
    #define ON_DBG(...) __VA_ARGS__
#else
    #define ON_DBG(...)
#endif

const int NUM = 1;
const int OP  = 2;
const int VAR = 3;

const int PLS = '+';
const int SUB = '-';
const int DIV = '/';

struct Node_t* new_node  (int type, double data);

int print_tree_preorder  (struct Node_t* root, FILE* filename);

int print_tree_postorder (struct Node_t* root);

int print_tree_inorder   (struct Node_t* root);

int delete_sub_tree      (struct Node_t* node);

int buffer_dtor (struct Buffer_t* buffer);

void print_tree_preorder_for_file (struct Node_t* root, FILE* filename);

int make_graph (struct Node_t* root);

void clean_buffer (void);

void dump_in_log_file (struct Node_t* node, const char* reason);

struct Node_t* read_example (FILE* file, struct Buffer_t* buffer);

struct Node_t* read_node (int level, struct Buffer_t* buffer);

// int write_log_file (struct Node_t* root, const char* reason_bro/* in c++ " = doesn't_care_bro"*/);

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

    dump_in_log_file (root, "");

    printf ("\npreorder: ");
    print_tree_preorder  (root, stdout);

    printf ("\npostorder: ");
    print_tree_postorder (root);

    printf ("\ninorder: ");
    print_tree_inorder   (root);
    printf ("\n");

    dump_in_log_file (root, "end of programm");

    close_log_file ();

    delete_sub_tree (root);

    buffer_dtor (&buffer);

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
    if (n < 0) { ON_DBG ( INDENT; printf ("No DATA found. Return NULL.\n"); ) return NULL; }

    char temp = 0;
    double temp_dbl = 0;

    if ( sscanf (buffer->current_ptr + bgn, "%lg", &temp_dbl) != 0)
    {
        printf ("\n\n %lg \n\n", temp_dbl);

        node->data = temp_dbl;
        node->type = NUM;

        ON_DBG ( INDENT; printf ("Got a NAME: '%lg'. Cur = %.10s..., [%p]. buffer_ptr = [%p]\n",
                node->data, buffer->current_ptr, buffer->current_ptr, buffer->buffer_ptr); )


        buffer->current_ptr += n;

        ON_DBG ( INDENT; printf ("CURRENT_PTR was shifted. Cur = %.10s..., [%p]. buffer_ptr = [%p]\n",
                                buffer->current_ptr, buffer->current_ptr, buffer->buffer_ptr); )
        n = -1;
        char chr = '\0';
        sscanf (buffer->current_ptr, " %c %n", &chr, &n);
        if (n < 0) { ON_DBG ( INDENT; printf ("No ending symbol (1) found. Return NULL.\n"); ) return NULL; }

        if (chr == ')')
        {
            buffer->current_ptr += n;

            ON_DBG ( INDENT; printf ("Got a ')', SHORT Node END (data = '%lg'). Return node. Cur = %.10s..., [%p]. buffer_ptr = [%p]\n",
                    node->data, buffer->current_ptr, buffer->current_ptr, buffer->buffer_ptr); )

            return node;
        }

        ON_DBG ( INDENT; printf ("')' NOT found. Supposing a left/right subtree. Reading left node. Cur = %.10s..., [%p]. buffer_ptr = [%p]\n",
                buffer->current_ptr, buffer->current_ptr, buffer->buffer_ptr); )

        node->left = read_node (level + 1, buffer);

        ON_DBG ( INDENT; printf ("\n" "LEFT subtree read. Data of left root = '%lg'\n\n", node->left->data); )

        ON_DBG ( printf ("Reading right node. Cur = %.10s...\n", buffer->current_ptr); )

        node->right = read_node (level + 1, buffer);

        ON_DBG ( INDENT; printf ("\n" "RIGHT subtree read. Data of right root = '%lg'\n", node->right->data); )

        chr = '\0';
        sscanf (buffer->current_ptr, " %c %n", &chr, &n);
        if (n < 0) { ON_DBG ( INDENT; printf ("No ending symbol (2) found. Return NULL.\n"); ) return NULL; }

        if (chr == ')')
        {
            buffer->current_ptr += n;

            ON_DBG ( INDENT; printf ("Got a ')', FULL Node END (data = '%lg'). Return node. Cur = %.10s..., [%p]. buffer_ptr = [%p]\n",
                    node->data, buffer->current_ptr, buffer->current_ptr, buffer->buffer_ptr); )

            return node;
        }

        ON_DBG ( INDENT; printf ("Does NOT get ')'. Syntax error. Return NULL. Cur = %.20s..., [%p]. buffer_ptr = [%p]\n",
                buffer->current_ptr, buffer->current_ptr, buffer->buffer_ptr); )

        return NULL;
    }

    sscanf (buffer->current_ptr + bgn, "%c", &temp);

    if (temp == 'x')
        node->type = VAR;

    node->type = OP;
    node->data = (int) temp;

    ON_DBG ( INDENT; printf ("Got a NAME: '%c'. n = %d. Cur = %.10s..., [%p]. buffer_ptr = [%p]\n",
                (int) node->data, n, buffer->current_ptr, buffer->current_ptr, buffer->buffer_ptr); )

    buffer->current_ptr += n;

    ON_DBG ( INDENT; printf ("Shifted CURRENT_PTR: '%c'. Cur = %.10s..., [%p]. buffer_ptr = [%p]\n",
            (int) node->data, buffer->current_ptr, buffer->current_ptr, buffer->buffer_ptr); )

    n = -1;
    char chr = '\0';
    sscanf (buffer->current_ptr, " %c %n", &chr, &n);
    if (n < 0) { ON_DBG ( INDENT; printf ("No ending symbol (1) found. Return NULL.\n"); ) return NULL; }

    if (chr == ')')
    {
        buffer->current_ptr += n;

        ON_DBG ( INDENT; printf ("Got a ')', SHORT Node END (data = '%c'). Return node. Cur = %.10s..., [%p]. buffer_ptr = [%p]\n",
                    (int) node->data, buffer->current_ptr, buffer->current_ptr, buffer->buffer_ptr); )

        return node;
    }

    ON_DBG ( INDENT; printf ("')' NOT found. Supposing a left/right subtree. Reading left node. Cur = %.10s..., [%p]. buffer_ptr = [%p]\n",
                buffer->current_ptr, buffer->current_ptr, buffer->buffer_ptr); )

    node->left = read_node (level + 1, buffer);

    ON_DBG ( INDENT; printf ("\n" "LEFT subtree read. Data of left root = '%c'\n\n", (int) node->left->data); )

    ON_DBG ( printf ("Reading right node. Cur = %.10s...\n", buffer->current_ptr); )

    node->right = read_node (level + 1, buffer);

    ON_DBG ( INDENT; printf ("\n" "RIGHT subtree read. Data of right root = '%c'\n", (int) node->right->data); )

    chr = '\0';
    sscanf (buffer->current_ptr, " %c %n", &chr, &n);
    if (n < 0) { ON_DBG ( INDENT; printf ("No ending symbol (2) found. Return NULL.\n"); ) return NULL; }

    if (chr == ')')
    {
        buffer->current_ptr += n;

        ON_DBG ( INDENT; printf ("Got a ')', FULL Node END (data = '%c'). Return node. Cur = %.10s..., [%p]. buffer_ptr = [%p]\n",
                                (int) node->data, buffer->current_ptr, buffer->current_ptr, buffer->buffer_ptr); )

        return node;
    }

    ON_DBG ( INDENT; printf ("Does NOT get ')'. Syntax error. Return NULL. Cur = %.20s..., [%p]. buffer_ptr = [%p]\n",
            buffer->current_ptr, buffer->current_ptr, buffer->buffer_ptr); )

    return NULL;

    if ( sscanf (buffer->current_ptr + bgn, "%lg", &temp_dbl) != 0)
    {
        printf ("\n\n %lg \n\n", temp_dbl);

        node->data = temp_dbl;

        ON_DBG ( INDENT; printf ("Got a NAME: '%lg'. Cur = %.10s..., [%p]. buffer_ptr = [%p]\n",
                node->data, buffer->current_ptr, buffer->current_ptr, buffer->buffer_ptr); )


        buffer->current_ptr += n;

        ON_DBG ( INDENT; printf ("CURRENT_PTR was shifted. Cur = %.10s..., [%p]. buffer_ptr = [%p]\n",
                                buffer->current_ptr, buffer->current_ptr, buffer->buffer_ptr); )
        n = -1;
        char chr = '\0';
        sscanf (buffer->current_ptr, " %c %n", &chr, &n);
        if (n < 0) { ON_DBG ( INDENT; printf ("No ending symbol (1) found. Return NULL.\n"); ) return NULL; }

        if (chr == ')')
        {
            buffer->current_ptr += n;

            ON_DBG ( INDENT; printf ("Got a ')', SHORT Node END (data = '%lg'). Return node. Cur = %.10s..., [%p]. buffer_ptr = [%p]\n",
                    node->data, buffer->current_ptr, buffer->current_ptr, buffer->buffer_ptr); )

            return node;
        }

        ON_DBG ( INDENT; printf ("')' NOT found. Supposing a left/right subtree. Reading left node. Cur = %.10s..., [%p]. buffer_ptr = [%p]\n",
                buffer->current_ptr, buffer->current_ptr, buffer->buffer_ptr); )

        node->left = read_node (level + 1, buffer);

        ON_DBG ( INDENT; printf ("\n" "LEFT subtree read. Data of left root = '%lg'\n\n", node->left->data); )

        ON_DBG ( printf ("Reading right node. Cur = %.10s...\n", buffer->current_ptr); )

        node->right = read_node (level + 1, buffer);

        ON_DBG ( INDENT; printf ("\n" "RIGHT subtree read. Data of right root = '%lg'\n", node->right->data); )

        chr = '\0';
        sscanf (buffer->current_ptr, " %c %n", &chr, &n);
        if (n < 0) { ON_DBG ( INDENT; printf ("No ending symbol (2) found. Return NULL.\n"); ) return NULL; }

        if (chr == ')')
        {
            buffer->current_ptr += n;

            ON_DBG ( INDENT; printf ("Got a ')', FULL Node END (data = '%lg'). Return node. Cur = %.10s..., [%p]. buffer_ptr = [%p]\n",
                    node->data, buffer->current_ptr, buffer->current_ptr, buffer->buffer_ptr); )

            return node;
        }

        ON_DBG ( INDENT; printf ("Does NOT get ')'. Syntax error. Return NULL. Cur = %.20s..., [%p]. buffer_ptr = [%p]\n",
                buffer->current_ptr, buffer->current_ptr, buffer->buffer_ptr); )

        return NULL;
    }
}

void dump_in_log_file (struct Node_t* node, const char* reason)
{
    make_graph (node);
    write_log_file (reason);
}

struct Node_t* new_node (int type, double data)
{
    struct Node_t* node = (struct Node_t*) calloc (1, sizeof(*node));
    assert (node); //FIXME

    node->type = type;
    node->data = data;

    node->left  = NULL;
    node->right = NULL;

    return node;
}

void clean_buffer(void)
{
    while((getchar()) != '\n') {;}
}

int delete_sub_tree (struct Node_t* node)
{
    node->data = 0;
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

int print_tree_preorder (struct Node_t* root, FILE* filename)
{
    if (!root)
        return 1; //FIXME

    fprintf (filename, " ( ");

    if (root->type == NUM)
        fprintf (filename, "%lg",  root->data);
    else
        fprintf (filename, "%c", (int) root->data);

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
        printf ("%lg",  root->data);
    else
        printf ("%c", (int) root->data);

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
        printf ("%lg", root->data);
    else
        printf ("%c", (int) root->data);

    if (root->right) print_tree_inorder (root->right);

    printf (" ) ");

    return 0;
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

void print_tree_preorder_for_file (struct Node_t* root, FILE* filename)
{
    if (!root)
        return ; //FIXME

    if (root->type == NUM)
        fprintf (filename, "node%p [shape=Mrecord; label = \" { [%p] | type = %d (NUM) | data = %.2g | { left = [%p] | right = [%p] } }\"; style = filled; fillcolor = \"#FFFFD0\"];\n",
             root, root, root->type, root->data, root->left, root->right);
    else if (root->type == OP)
        fprintf (filename, "node%p [shape=Mrecord; label = \" { [%p] | type = %d (OP) | data = '%c' | { left = [%p] | right = [%p] } }\"; style = filled; fillcolor = \"#FFFFD0\"];\n",
             root, root, root->type, (int) root->data, root->left, root->right);
    else
        fprintf (filename, "node%p [shape=Mrecord; label = \" { [%p] | type = %d (VAR) | data = '%c' | { left = [%p] | right = [%p] } }\"; style = filled; fillcolor = \"#FFFFD0\"];\n",
             root, root, root->type, (int) root->data, root->left, root->right);

    if (root->left)
        fprintf (filename, "node%p -> node%p\n;", root, root->left);

    if (root->right)
        fprintf (filename, "node%p -> node%p\n;", root, root->right);

    if (root->left)  print_tree_preorder_for_file (root->left , filename);

    if (root->right) print_tree_preorder_for_file (root->right, filename);
}
