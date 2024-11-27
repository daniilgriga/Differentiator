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
#include "tex.h"
#include "color_print.h"

static struct Node_t* GlobalNode = NULL;

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
    assert (node   &&   "node is NULL in read_double_func");
    assert (buffer && "buffer is NULL in read_double_func");

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

    ON_DBG ( PRINT ("LEFT subtree read. value of left node = '%lg'", node->left->value); )

    ON_DBG ( PRINT ("Reading right node."); )

    node->right = read_node (level + 1, buffer);

    ON_DBG ( PRINT ("RIGHT subtree read. value of right node = '%lg'", node->right->value); )

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
    assert (node   &&   "node is NULL in read_symbol_func");
    assert (buffer && "buffer is NULL in read_symbol_func");

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

    ON_DBG ( PRINT ("LEFT subtree read. value of left node = '%c' (%lg), TYPE %d", (int) node->left->value, node->left->value, node->left->type); )

    ON_DBG ( PRINT ("Reading right node."); )

    node->right = read_node (level + 1, buffer);

    ON_DBG ( PRINT ("RIGHT subtree read. value of right node = '%c' (%lg), TYPE %d", (int) node->right->value, node->right->value, node->right->type); )

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
    assert (node);
    assert (buffer);

    delete_sub_tree (node);
    buffer_dtor (buffer);
}

int print_tree_preorder (struct Node_t* node, FILE* file)
{
    assert (node);
    assert (file);

    fprintf (file, " ( ");

    if (node->type == NUM)
        fprintf (file, "%lg",  node->value);
    else
        fprintf (file, "%c", (int) node->value);

    if (node->left)  print_tree_preorder (node->left,  file);

    if (node->right) print_tree_preorder (node->right, file);

    fprintf (file, " ) ");

    return 0;
}

int print_tree_postorder (struct Node_t* node)
{
    assert (node);

    printf (" ( ");

    if (node->left)  print_tree_postorder (node->left);

    if (node->right) print_tree_postorder (node->right);

    if (node->type == NUM)
        printf ("%lg",  node->value);
    else
        printf ("%c", (int) node->value);

    printf (" ) ");

    return 0;
}

int its_func_is_root (struct Node_t* node)
{
    assert (node);

    if (node->type == OP && (int) node->value == 's')
        return 1;

    if (node->type == OP && (int) node->value == 'c')
        return 1;

    return 0;
}

void print_func_in_tex (struct Node_t* node)
{
    assert (node && "node is NULL in print_func");

    if ( (int) node->value == 's')
        tex_printf ("sin ");

    if ( (int) node->value == 'c')
        tex_printf ("cos ");

   if (node->left)
    {
        tex_printf (" { ");
        tex_printf_tree_inorder (node->left, node);
        tex_printf (" } ");
    }
}

int tex_printf_tree_inorder (struct Node_t* node, struct Node_t* parent)
{
    assert (node && "node is NULL in tex_printf_tree_inorder");

    if (its_func_is_root (node))
    {
        print_func_in_tex (node);
        return 0;
    }

    bool brackets = (node->left != NULL && node->right != NULL);

    if (node && parent && node->type == OP && parent->type == OP)
        if (priority (node->value) >= priority (parent->value))
            brackets = 0;

    if (parent == NULL)
        brackets = 0;

    if (node->type == OP)
        if (node->value == DIV)
            brackets = 0;

    if (node->type == OP)
        if (node->value == POW)
            brackets = 0;

    if (node->type == OP)
        if (node->value == POW)
            if (node->left->value == COS)
                brackets = 1;

    if (node->type == OP)
        if (node->value == POW)
            if (node->left->value == SIN)
                brackets = 1;

    if (node->type == OP)
        if (node->value == SUB)
            brackets = 0;

    if (node->type == NUM)
        if (node->value == -1)
            brackets = 1;

    if (brackets)
        tex_printf (" ( ");

    if (node->left)
    {
        tex_printf (" { ");
        tex_printf_tree_inorder (node->left, node);
        tex_printf (" } ");
    }

    if (node->type == NUM)
        tex_printf (" %lg ", node->value);
    else
    {
        if (node->type == OP && node->value == MUL)
            tex_printf (" \\cdot ");
        else
        if (node->type == OP && node->value == DIV)
            tex_printf (" \\over ");
        else
            tex_printf (" %c ", (int) node->value);
    }

    if (node->right)
    {
        tex_printf (" { ");
        tex_printf_tree_inorder (node->right, node);
        tex_printf (" } ");
    }

    if (brackets)
        tex_printf (" ) ");

    return 0;
}

void tex_printf_expression (struct Node_t* node, struct Node_t* diff_node)
{
    assert (node);
    assert (diff_node);

    tex_printf ("$$ ({");
    tex_printf_tree_inorder (node, NULL);

    tex_printf ("})' = {");

    tex_printf_tree_inorder (diff_node, NULL);
    tex_printf ("} $$\n");
}

void tex_printf_tree (struct Node_t* node, struct Node_t* diff_node, const char* message)
{
    assert (node);
    assert (diff_node);
    assert (message);

    if (GlobalNode != diff_node)
    {
        tex_printf ("%s", message);
        tex_printf_expression (node, diff_node);
    }
    else
        tex_printf ("%s... wait wait its the same, see above bro\\newline \\newline ", message);

    GlobalNode = diff_node;
}

int priority (int op)
{
    switch (op)
    {
        case ADD:
        case SUB: return 3;

        case MUL: return 4;
        case DIV: return 0;

        case POW: return 2;

        case SIN: return 1;

        case COS: return 1;

        default:
            printf ("priority of operation %d (%c) dont exists\n", op, op);
            return -1;
    }
}

void print_tree_preorder_for_file (struct Node_t* node, FILE* filename)
{
    assert (node);
    assert (filename);

    if (node->type == NUM)
        fprintf (filename, "node%p [shape=Mrecord; label = \" { [%p] | type = %d (NUM) | value = %g | { left = [%p] | right = [%p] } }\"; style = filled; fillcolor = \"#FFD700\"];\n",
             node, node, node->type, node->value, node->left, node->right);
    else if (node->type == OP)
        fprintf (filename, "node%p [shape=Mrecord; label = \" { [%p] | type = %d (OP) | value = '%c' | { left = [%p] | right = [%p] } }\"; style = filled; fillcolor = \"#20B2AA\"];\n",
             node, node, node->type, (int) node->value, node->left, node->right);
    else if (node->type == VAR)
        fprintf (filename, "node%p [shape=Mrecord; label = \" { [%p] | type = %d (VAR) | value = '%c' | { left = [%p] | right = [%p] } }\"; style = filled; fillcolor = \"#F00000\"];\n",
             node, node, node->type, (int) node->value, node->left, node->right);

    if (node->left)
        fprintf (filename, "node%p -> node%p\n;", node, node->left);

    if (node->right)
        fprintf (filename, "node%p -> node%p\n;", node, node->right);

    if (node->left)  print_tree_preorder_for_file (node->left , filename);

    if (node->right) print_tree_preorder_for_file (node->right, filename);
}

int make_graph (struct Node_t* node)
{
    assert (node);

    FILE* graph_file = fopen ("../build/graph_tree.dot", "w");
    if (graph_file == NULL)
    {
        printf("ERROR open graph_file\n");
        return 1;
    }

    fprintf (graph_file, "digraph\n{\n");

    fprintf (graph_file, "bgcolor=\"transparent\"\n");

    print_tree_preorder_for_file (node, graph_file);
    fprintf (graph_file, "\n");

    fprintf (graph_file, "}");
    fprintf (graph_file, "\n");

    fclose  (graph_file);

    return 0;
}

void dump_in_log_file (struct Node_t* node, const char* reason)
{
    assert (node);
    assert (reason);

    make_graph (node);
    write_log_file (reason);
}

void clean_buffer(void)
{
    while((getchar()) != '\n') {;}
}
