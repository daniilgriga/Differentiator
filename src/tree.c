//#define DEBUG

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
#include "enum.h"

static struct Node_t* GlobalNode = NULL;

#ifdef DEBUG
    #define ON_DBG(...) __VA_ARGS__
#else
    #define ON_DBG(...)
#endif

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
    if (node->left)  delete_sub_tree (node->left);
    if (node->right) delete_sub_tree (node->right);

    delete_node (node);

    return 0;
}

int delete_node (struct Node_t* node)
{
    if (node == NULL)
        fprintf (stderr, "IN DELETE: node = NULL\n");

    node->type  = 666;
    node->value = 0;

    node->left  = NULL;
    node->right = NULL;

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
    fprintf (stderr, BLUE_TEXT("IN its_func_is_root ") "node = [%p]\n", node);

    if (node->type == OP && (int) node->value == SIN)
        return 1;

    if (node->type == OP && (int) node->value == COS)
        return 1;

    if (node->type == OP && (int) node->value == LN)
        return 1;

    return 0;
}

void print_func_in_tex (struct Node_t* node, struct Node_t* parent, struct Context_t* context)
{
    assert (node && "node is NULL in print_func");

    if ( (int) node->value == SIN)
    {
        if (parent != NULL && parent->value == POW)
            tex_printf ("(sin ");
        else
            tex_printf ("sin ");
    }

    if ( (int) node->value == COS)
    {
        if (parent != NULL && parent->value == POW)
            tex_printf ("(cos ");
        else
            tex_printf ("cos ");
    }

    if ( (int) node->value == LN)
    {
        if (parent != NULL && parent->value == POW)
            tex_printf ("(ln ");
        else
            tex_printf ("ln ");
    }

   if (node->left)
    {
        tex_printf (" { ");
        tex_printf_tree_inorder (node->left, node, context);

        if (parent != NULL && parent->value == POW)
            tex_printf (")");

        tex_printf (" } ");
    }

    if (node->right)
    {
        tex_printf (" { ");
        tex_printf_tree_inorder (node->right, node, context);

        if (parent != NULL && parent->value == POW)
            tex_printf (")");

        tex_printf (" } ");
    }
}

int tex_printf_tree_inorder (struct Node_t* node, struct Node_t* parent, struct Context_t* context)
{
    assert (node && "node is NULL in tex_printf_tree_inorder");

    if (its_func_is_root (node))
    {
        print_func_in_tex (node, parent, context);
        return 0;
    }

    bool extra_brackets = 0;

    bool brackets = (node->left != NULL && node->right != NULL);

    if (node && parent && node->type == OP && parent->type == OP)
        if (priority (node->value) >= priority (parent->value))
            brackets = 0;

    if (parent == NULL)
        brackets = 0;

    if (parent && parent->type == OP && parent->value == SIN)
        brackets = 1;

    if (parent && parent->type == OP && parent->value == COS)
        brackets = 1;

    if (node->type == OP)
        if ( (int) node->value == DIV)
            brackets = 0;

    if (node->type == OP)
        if ( (int) node->value == POW)
            brackets = 0;

    if (node->type == OP)
        if ( (int) node->value == SUB)
            brackets = 0;

    if ( parent != NULL && node->type == OP )
        if ( (int) node->value == COS )
            if ( (int) parent->value == POW )
                brackets = 1;

    if ( parent != NULL && node->type == OP)
        if ( (int) node->value == SIN )
            if ( (int) parent->value == POW )
                brackets = 1;

    if (node->type == NUM)
        if ( (int) node->value == -1)
            brackets = 1;

    if ( node->type == OP && node->value == SUB && (node->right->value == SUB || node->right->value == ADD) )
        extra_brackets = 1;

    if (brackets)
        tex_printf (" ( ");

    if (node->left)
    {
        if (extra_brackets == 1)
        {
            tex_printf (" { (");
            tex_printf_tree_inorder (node->left, node, context);
            tex_printf (" } )");
        }
        else
        {
            tex_printf (" { ");
            tex_printf_tree_inorder (node->left, node, context);
            tex_printf (" } ");
        }
    }

    if (node->type == NUM)
        tex_printf (" %lg ", node->value);
    else
    {
        if (node->type == OP && node->value == MUL && parent && parent->value != SIN && parent->value != COS)
            tex_printf (" \\cdot ");
        else if (node->type == OP && node->value == DIV)
            tex_printf (" \\over ");
        else if (node->type == OP && node->value == MUL && parent && (parent->value == SIN || parent->value == COS) )
            tex_printf (" ");
        else if (node->type == OP && (node->value == ADD || node->value == SUB))
            tex_printf ("%c", (int) node->value);
        else if (node->type == ID)
            tex_printf ("%.*s", (int) context->name_table[(int) node->value].name.length, context->name_table[(int) node->value].name.str_pointer);
    }

    if (node->right)
    {
        if (extra_brackets == 1)
        {
            tex_printf (" { (");
            tex_printf_tree_inorder (node->right, node, context);
            tex_printf (" } )");
        }
        else
        {
            tex_printf (" { ");
            tex_printf_tree_inorder (node->right, node, context);
            tex_printf (" } ");
        }
    }

    if (brackets)
        tex_printf (" ) ");

    return 0;
}

void tex_printf_expression (struct Node_t* node, struct Node_t* diff_node, struct Context_t* context, int first_equation)
{
    //assert (node);
    //assert (diff_node);

    if (first_equation == 'y')
    {
        tex_printf ("$$ f(x) = {");
        tex_printf_tree_inorder (node, NULL, context);
        tex_printf ("} $$");
    }
    else
    {
        tex_printf ("$$ ({");
        tex_printf_tree_inorder (node, NULL, context);

        tex_printf ("})' = {");

        fprintf (stderr, "in tex_printf_expression: node   = [%p]\n", node);
        fprintf (stderr, "in tex_printf_expression: diff_n = [%p]\n\n", diff_node);

        tex_printf_tree_inorder (diff_node, NULL, context);
        tex_printf ("} $$\n");
    }
}

void tex_printf_tree (struct Node_t* node, struct Node_t* diff_node, struct Context_t* context, const char* message, int first_equation)
{

    fprintf (stderr, "in tex_printf_tree: node = [%p] diif_node = [%p]\n\n", node, diff_node);
    //assert (diff_node);

    //if (GlobalNode != diff_node)
    //{
    if (node != NULL && diff_node != NULL)
    {
        fprintf (stderr, "in tex_printf_tree IN_IF: node = [%p] diif_node = [%p]\n\n", node, diff_node);
        tex_printf ("%s", message);
        tex_printf_expression (node, diff_node, context, first_equation);

        GlobalNode = diff_node;
    }

    //}
    //else
    //    tex_printf ("%s... wait wait its the same, see above bro\\newline \\newline ", message);

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

        case LN:  return 1;

        default:
            printf ("priority of operation %d (%c) dont exists\n", op, op);
            return -1;
    }
}

void print_tree_preorder_for_file (struct Node_t* node, FILE* filename)
{
    //assert (node);
    //assert (filename);

    //assert (node->type == NUM || node->type == OP || node->type == ID);

    if (node->type == NUM)
        fprintf (filename, "node%p [shape=Mrecord; label = \" { [%p] | type = %d (NUM)  | value = %g   | { left = [%p] | right = [%p] } }\"; style = filled; fillcolor = \"#FFD700\"];\n",
             node, node, node->type, node->value, node->left, node->right);
    else if (node->type == OP)
        fprintf (filename, "node%p [shape=Mrecord; label = \" { [%p] | type = %d (OP)   | value = '%c' | { left = [%p] | right = [%p] } }\"; style = filled; fillcolor = \"#20B2AA\"];\n",
             node, node, node->type, (int) node->value, node->left, node->right);
    else if (node->type == ID)
        fprintf (filename, "node%p [shape=Mrecord; label = \" { [%p] | type = %d (ID)   | number of name in name table = '%lg' | { left = [%p] | right = [%p] } }\"; style = filled; fillcolor = \"#F00000\"];\n",
             node, node, node->type, node->value, node->left, node->right);
    else if (node->type == ROOT)
        fprintf (filename, "node%p [shape=Mrecord; label = \" { [%p] | type = %d (ROOT) | value = '%lg' | { son_node = [%p] } }\"; style = filled; fillcolor = \"#F0FFFF\"];\n",
             node, node, node->type, node->value, node->left);

    if (node->left)
        fprintf (filename, "node%p -> node%p;\n", node, node->left);

    if (node->right)
        fprintf (filename, "node%p -> node%p;\n", node, node->right);

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

void dump_in_log_file (struct Node_t* node, const char* reason, ...)
{
    if (node == NULL)
        fprintf (stderr, "got node == NULL in dump, reason = \"%s\"\n", reason);

    make_graph (node);

    va_list args;
    va_start (args, reason);

    write_log_file (reason, args);

    va_end (args);
}

void clean_buffer(void)
{
    while((getchar()) != '\n') {;}
}
