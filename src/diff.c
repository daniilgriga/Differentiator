#include <stdio.h>
#include <assert.h>

#include "tree.h"
#include "tex.h"

struct Node_t* GlobalNode = NULL;

struct Node_t* copy (struct Node_t* node);

#define _ADD(left, right) new_node (OP, ADD, (left), (right))
#define _SUB(left, right) new_node (OP, SUB, (left), (right))
#define _MUL(left, right) new_node (OP, MUL, (left), (right))
#define _POW(left, right) new_node (OP, POW, (left), (right))
#define _DIV(left, right) new_node (OP, DIV, (left), (right))
#define _SIN(left, right) new_node (OP, SIN, (left), (right))
#define _COS(left, right) new_node (OP, COS, (left), (right))
#define _NUM(value)       new_node (NUM, (value), NULL, NULL)

#define _COMPOUND(diff_res) _MUL (diff_res, diff (node->left, tex) )

#define _L          node->left
#define _R          node->right
#define _dL   diff (node->left,  tex)
#define _dR   diff (node->right, tex)
#define _cL   copy (node->left)
#define _cR   copy (node->right)

struct Node_t* diff (struct Node_t* node, FILE* tex)
{
    if (node->type == NUM) return _NUM(0);
    if (node->type == VAR) return _NUM(1);
    if (node->type == OP)
        switch ((int) node->value)
        {
            case ADD:
            {
                struct Node_t* diff_node = _ADD (_dL, _dR);
                /*if (GlobalNode == diff_node)
                    fprintf (tex, "bro, sorry, but you'll have to look at this again: ");*/
                fprintf (tex, "the derivative of the sum can be represented as follows:");
                tex_expression_print (node, diff_node, tex);

                GlobalNode = diff_node;

                return diff_node;
            }

            case MUL:
            {
                struct Node_t* diff_node = _ADD ( _MUL (_dL, _cR), _MUL (_cL, _dR));
                /*if (GlobalNode == diff_node)
                    fprintf (tex, "bro, sorry, but you'll have to look at this again: ");*/
                fprintf (tex, "obviously, the derivative of multiplication looks like this: ");
                tex_expression_print (node, diff_node, tex);

                GlobalNode = diff_node;

                return diff_node;
            }

            case DIV:
            {
                fprintf (tex, "calculate the derivative of the fraction.\n");

                struct Node_t* u   = node->left;
                struct Node_t* du  = diff (u, tex);
                /*if (GlobalNode == du)
                    fprintf (tex, "bro, sorry, but you'll have to look at this again: ");*/
                fprintf (tex, "the derivative of the numerator is calculated as follows: ");
                tex_expression_print (u, du, tex);
                GlobalNode = du;

                struct Node_t* cu  = copy (u);

                struct Node_t* v   = node->right;
                struct Node_t* dv  = diff (v, tex);
                /*if (GlobalNode == dv)
                    fprintf (tex, "bro, sorry, but you'll have to look at this again: ");*/
                fprintf (tex, "karzhemanov said that the denominator is equal to: ");
                tex_expression_print (v, dv, tex);

                GlobalNode = dv;

                struct Node_t* cv  = copy (v);

                struct Node_t* v2  = _MUL (cv, cv);

                struct Node_t* duv = _MUL (du, cv);

                struct Node_t* udv = _MUL (cu, dv);

                struct Node_t* duv_udv = _SUB (duv, udv);

                struct Node_t* diff_node = _DIV (duv_udv, v2);

                fprintf (tex, "final differentiated fraction: ");
                tex_expression_print (node, diff_node, tex);

                return diff_node;
            }

            case POW: return _COMPOUND ( _MUL ( _cR, _POW (_cL, _SUB (_cR, _NUM(1) )))); // here

            case SIN: return _COMPOUND ( _COS ( _cL, _cR) );

            case COS: return _COMPOUND ( _MUL ( _SIN ( _cL, _cR), _NUM(-1) ) );



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
