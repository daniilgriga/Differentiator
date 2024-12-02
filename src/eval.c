#include <stdio.h>
#include <stdarg.h>
#include <assert.h>
#include <stdlib.h>
#include <math.h>

#include "tree.h"
#include "eval.h"

const double GlobalX = 2;

double eval (struct Node_t* node)
{
    printf ("starting evaluation...\n");

    if (node->type == NUM)
    {
        double value = node->value;
        printf ("node->type = NUM >>> node->value = %lg\n\n", node->value);

        delete_node (node);

        return value;
    }

    if (node->type == VAR)
    {
        printf ("node->type = VAR >>> node->value = %c\n\n", (int) node->value);
        return NAN;
    }

    if (node->type == OP)
    {
        printf ("node->type = OP >>> node->value = %c\n\n", (int) node->value);

        switch ( (int) node->value )
        {
            case ADD:
            {
                double res = eval (node->left) +
                             eval (node->right);

                printf ("case ADD: result = %lg\n\n", res);

                delete_node (node);

                return res;
            }

            case SUB:
            {
                double res = eval (node->left) -
                             eval (node->right);
                printf ("case SUB: result = %lg\n\n", res);

                delete_node (node);

                return res;
            }

            case DIV:
            {
                double res = eval (node->left) /
                             eval (node->right);
                printf ("case DIV: result = %lg\n\n", res);

                delete_node (node);

                return res;
            }

            case MUL:
            {
                double res = eval (node->left) *
                             eval (node->right);
                printf ("case MUL: result = %lg\n\n", res);

                delete_node (node);

                return res;
            }

            case SIN:
            {
                double res = sin ( eval (node->left) );
                printf ("case SIN: result = %lg\n\n", res);

                delete_node (node);

                return res;
            }

            case COS:
            {
                double res = cos ( eval (node->left) );
                printf ("case COS: result = %lg\n\n", res);

                delete_node (node);

                return res;
            }

            default:
            {
                printf ("Unknown operation. returned 1");
                return 1;
            }
        }
    }
}
