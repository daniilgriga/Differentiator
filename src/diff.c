// #define TEST

#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <math.h>

#include "tree.h"
#include "tex.h"
#include "dsl.h"
#include "log.h"
#include "diff.h"
#include "eval.h"
#include "color_print.h"

#ifdef TEST
    #define ON_TST(...) __VA_ARGS__
#else
    #define ON_TST(...)
#endif

struct Node_t* copy (struct Node_t* node);

#define _COMPOUND(diff_res) _MUL (diff_res, diff (node->left) )

#define _L         node->left
#define _R         node->right
#define _dL  diff (node->left)
#define _dR  diff (node->right)
#define _cL  copy (node->left)
#define _cR  copy (node->right)

struct Node_t* diff (struct Node_t* node)
{
    if (node->type == NUM) return _NUM(0);
    if (node->type == VAR) return _NUM(1);
    if (node->type == ROOT) {;}
    if (node->type == OP)
        switch ((int) node->value)
        {
            case ADD:
            {
                struct Node_t* diff_node = _ADD (_dL, _dR);

                tex_printf_tree (node, diff_node, "the derivative of the sum can be represented as follows: ", 'n');

                return diff_node;
            }

            case MUL:
            {
                struct Node_t* diff_node = _ADD ( _MUL (_dL, _cR), _MUL (_cL, _dR) );

                tex_printf_tree (node, diff_node, "obviously, the derivative of multiplication looks like this: ", 'n');

                return diff_node;
            }

            case DIV:
            {
                tex_printf ("calculate the derivative of the fraction.\\newline ");

                struct Node_t* u   = node->left;
                struct Node_t* du  = diff (u);

                tex_printf_tree (u, du, "the derivative of the numerator is calculated as follows: ", 'n');

                struct Node_t* cu  = copy (u);

                struct Node_t* v   = node->right;
                struct Node_t* dv  = diff (v);

                tex_printf_tree (v, dv, "Karzhemanov said that the denominator is equal to: ", 'n');

                struct Node_t* cv  = copy (v);

                struct Node_t* v2  = _MUL (cv, cv);

                struct Node_t* duv = _MUL (du, cv);

                struct Node_t* udv = _MUL (cu, dv);

                struct Node_t* duv_udv = _SUB (duv, udv);

                struct Node_t* diff_node = _DIV (duv_udv, v2);

                tex_printf_tree (node, diff_node, "final differentiated fraction: ", 'n');

                return diff_node;
            }

            case POW:
            {
                struct Node_t* diff_node = _COMPOUND ( _MUL ( _cR, _POW (_cL, _SUB (_cR, _NUM(1) ) ) ) );

                tex_printf_tree (node, diff_node, "Znamenskaya forbade doing this, but: ", 'n');

                return diff_node;
            }

            case SIN:
            {
                struct Node_t* diff_node = _COMPOUND ( _COS ( _cL ) );

                tex_printf_tree (node, diff_node, "every kindergartener in the USSR knew that: ", 'n');

                return diff_node;
            }

            case COS:
            {
                struct Node_t* diff_node = _COMPOUND ( _MUL ( _SIN ( _cL ), _NUM(-1) ) );

                tex_printf_tree (node, diff_node, "Ostap once said: ", 'n');

                return diff_node;
            }

            default:
                printf ("unknown operation %c (%d)", (int) node->value, (int) node->value);
                return NULL;
        }
}

struct Node_t* copy (struct Node_t* node)
{
    if (node == NULL)
        return NULL;

    struct Node_t* copy_left  = (node->left)  ? copy (node->left)  : NULL;
    struct Node_t* copy_right = (node->right) ? copy (node->right) : NULL;

    struct Node_t* copy_node = new_node (node->type, node->value, copy_left, copy_right);

    return copy_node;
}

#define _IS(where, what) (root && root->where &&         \
                          root->where->type  ==  NUM  && \
                          root->where->value == (what))

#define $ verificator(root, __FILE__, __LINE__);

int simplification_typical_operations (struct Node_t* root, struct Node_t* parent)
{
    assert (root);

    int count_changes = 0;

    if (root->left)
        simplification_typical_operations (root->left, root);

    if (root->right)
        simplification_typical_operations (root->right, root);

    if ( (root->type == OP) && ((int) root->value == MUL ))
        if ( _IS (left, 0) || _IS (right, 0) )
        {
            dump_in_log_file (parent, "<h1> BEFORE DELETE MUL: </h1>", root->left, root->right);

            delete_sub_tree (root->left);
            delete_sub_tree (root->right);

            root->type  = NUM;
            root->value = 0;

            root->left  = NULL;
            root->right = NULL;

            dump_in_log_file (parent, "<h1>AFTER DELETE: delete node->left [%p], delete node->right [%p]:</h1>", root->left, root->right);

            count_changes++;
        }

    if ( (root->type == OP) && ((int) root->value == MUL ))
    {
        if ( _IS (left, 1) )
        {
            dump_in_log_file (parent, "<h1> BEFORE DELETE MUL: </h1>", root->left, root->right);

            if (parent->left == root)
                parent->left  = root->right;
            else
                parent->right = root->right;

            delete_node (root->left);
            delete_node (root);

            dump_in_log_file (parent, "<h1>AFTER DELETE: delete node->left [%p], delete node->right [%p]:</h1>", root->left, root->right);

            count_changes++;
        }

        if ( _IS (right, 1) )
        {
            dump_in_log_file (parent, "<h1> BEFORE DELETE MUL: </h1>", root->left, root->right);

            if (parent->left == root)
                parent->left  = root->left;
            else
                parent->right = root->left;

            delete_node (root->right);
            delete_node (root);

            dump_in_log_file (parent, "<h1>AFTER DELETE: delete node->left [%p], delete node->right [%p]:</h1>", root->left, root->right);

            count_changes++;
        }
    }

    if ( (root->type == OP) && ((int) root->value == ADD))
    {
        if ( _IS (left, 0) )
        {
$           if (parent->left == root)
                parent->left  = root->right;
            else
                parent->right = root->right;
$
            dump_in_log_file (parent, "<h1>BEFORE DELETE ADD left:</h1>", root->left, root);

            delete_node (root->left);
            delete_node (root);

            dump_in_log_file (parent, "<h1>AFTER DELETE: delete node->left [%p], delete node [%p] :</h1>", root->left, root);
        }

        if ( _IS (right, 0) )
        {
            dump_in_log_file (parent, "<h1>BEFORE DELETE ADD right");

            if (parent->left == root)
                parent->left  = root->left;
            else
                parent->right = root->left; //func

            delete_node (root->right);
            delete_node (root);

            dump_in_log_file (parent, "<h1>delete node->left [%p], delete node [%p]:</h1>", root->left, root->right);
        }
    }
}

#undef $
#undef _IS

void verificator (struct Node_t* node, const char* filename, int line)
{
    if (node->type == 0)
        printf ("%s:%d: vasalam u have a problem: node [%p]: type = %d, value = %c (%lg)\n\n", filename, line, node, node->type, (int) node->value, node->value);

    if (node->left)  verificator (node->left, filename, line);
    if (node->right) verificator (node->right, filename, line);
}

double constant_folding (struct Node_t* root)
{
    assert (root);

    if (root->left)
        constant_folding (root->left);

    if (root->right)
        constant_folding (root->right);

    if (root->type == OP && root->left->type == NUM && root->right->type == NUM)
    {
        double answer = eval (root);
        fprintf (stderr, "\nlol im in if: node [%p]: answer = %lg\n\n", root, answer);

        root->type  = NUM;
        root->value = answer;
    }

    return 0;
}
