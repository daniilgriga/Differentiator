#include <stdio.h>
#include <assert.h>

#include "tree.h"

struct Node_t* copy (struct Node_t* node);

#define _ADD(left, right) new_node (OP, ADD, (left), (right))
#define _SUB(left, right) new_node (OP, SUB, (left), (right))
#define _MUL(left, right) new_node (OP, MUL, (left), (right))
#define _POW(left, right) new_node (OP, POW, (left), (right))
#define _DIV(left, right) new_node (OP, DIV, (left), (right))
#define _SIN(left, right) new_node (OP, SIN, (left), (right))
#define _COS(left, right) new_node (OP, COS, (left), (right))
#define _NUM(value)       new_node (NUM, (value), NULL, NULL)


#define _COMPOUND(diff_res) _MUL (diff_res, diff (node->left) )

#define _L          node->left
#define _R          node->right
#define _dL   diff (node->left)
#define _dR   diff (node->right)
#define _cL   copy (node->left)
#define _cR   copy (node->right)

struct Node_t* diff (struct Node_t* node)
{
    if (node->type == NUM) return _NUM(0);
    if (node->type == VAR) return _NUM(1);
    if (node->type == OP)
        switch ((int) node->value)
        {
            case ADD: return _ADD (_dL, _dR);
            case MUL: return _ADD ( _MUL (_dL, _cR), _MUL (_cL, _dR));
            case DIV:
            {
                struct Node_t* u   = node->left;
                struct Node_t* du  = diff (u);
                struct Node_t* cu  = copy (u);

                struct Node_t* v   = node->right;
                struct Node_t* dv  = diff (v);
                struct Node_t* cv  = copy (v);

                struct Node_t* v2  = _MUL (cv, cv);

                struct Node_t* duv = _MUL (du, cv);
                struct Node_t* udv = _MUL (cu, dv);

                struct Node_t* duv_udv = _SUB (duv, udv);

                return _DIV (duv_udv, v2);
            }

            case POW: return _COMPOUND ( _MUL ( _cR, _POW (_cL, _SUB (_cR, _NUM(1) ))));

            case SIN: return _COMPOUND ( _COS ( _cL, _cR) );

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
