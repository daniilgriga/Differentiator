#include <stdio.h>
#include <stdlib.h>

#include "tokens.h"
#include "tree.h"
#include "dsl.h"
#include "log.h"
#include "get_g.h"
#include "enum.h"

int Position =  0;

#define MOVE_POSITION Position++

/*         GRAMMAR
 *    G ::= E
 *    E ::= T {['+''-']T}*
 *    T ::= P {['*''/']P}*
 *    P ::= '('E')' | Id | N | Pow
 *  Pow ::= P {['^']P}*
 *
 *   Id ::= ['a'-'z']+
 *    N ::= ['0'-'9']+
 */

static struct Node_t*  GetE (struct Context_t* context);
static struct Node_t*  GetT (struct Context_t* context);
static struct Node_t*  GetP (struct Context_t* context);
static struct Node_t*  GetN (struct Context_t* context);
static struct Node_t* GetId (struct Context_t* context);

static struct Node_t* GetFunc (struct Context_t* context);
static struct Node_t* GetPow  (struct Context_t* context);

static void SyntaxError (struct Context_t* context, const char* filename const char* func, int line);

#define _IS_OP(val)  ( context->token[Position].type  == OP && \
                       context->token[Position].value == (val) )

struct Node_t* GetG (struct Context_t* context)
{
    struct Node_t* val = GetE (context);

    if ( !_IS_OP('$') )
        SyntaxError (context, __FILE__, __FUNCTION__, __LINE__); // TODO str in format printf;

    MOVE_POSITION;

    return val;
}

static struct Node_t* GetE (struct Context_t* context)
{
    struct Node_t* val = GetT (context);

    while ( _IS_OP (ADD) || _IS_OP (SUB) )
    {
        int op = context->token[Position].value;

        MOVE_POSITION;

        struct Node_t* val2 = GetT (context);

        if (op == ADD)
            val = _ADD (val, val2);
        else
            val = _SUB (val, val2);
    }

    return val;
}

static struct Node_t* GetT (struct Context_t* context)
{
    struct Node_t* val = GetPow (context);

    while ( _IS_OP (MUL) || _IS_OP (DIV) )
    {
        int op = context->token[Position].value;

        MOVE_POSITION;

        struct Node_t* val2 = GetPow (context);

        if (op == MUL)
            val = _MUL (val, val2);
        else
            val = _DIV (val, val2);
    }

    return val;
}

static struct Node_t* GetPow (struct Context_t* context)
{
    struct Node_t* node = GetP (context);

    while ( _IS_OP (POW) )
    {
        MOVE_POSITION;

        struct Node_t* exp = GetP (context);

        node = _POW (node, exp);
    }

    return node;
}

static struct Node_t* GetP (struct Context_t* context)
{
    fprintf (stderr, "\nIN GetP:\n");

    if ( _IS_OP (OP_BR) )
    {
        MOVE_POSITION;

        struct Node_t* val = GetE (context);

        if ( !_IS_OP (CL_BR) )
            SyntaxError (context, __FILE__, __FUNCTION__, __LINE__);

        MOVE_POSITION;

        return val;
    }
    else
    {
        fprintf (stderr, "waiting for GetId:\n");

        struct Node_t* node_ID = GetId (context);

        //fprintf (stderr, "we got it: value = '%c'\n", (int) node_ID->value);

        if (node_ID != NULL)
            return node_ID;
        else
        {
            return GetN (context);
            //struct Node_t* node_F = GetFunc ();
            //if (node_F != NULL)
            //    return node_F;
            //else
        }

    }
}

static struct Node_t* GetId (struct Context_t* context)
{
    struct Node_t* node = NULL;

    if ( context->token[Position].type  == ID )
    {
        node = _ID (context->token[Position].value);

        MOVE_POSITION;
    }

    return node;
}

static struct Node_t* GetN (struct Context_t* context)
{
    if (context->token[Position].type == NUM)
    {
        struct Node_t* node = _NUM (context->token[Position].value);

        MOVE_POSITION;

        return node;
    }
}

static struct Node_t* GetFunc (struct Context_t* context)
{
    int func = 0;

    if      ( context->token[Position].value == 'c' ) func = 'c';
    else if ( context->token[Position].value == 's' ) func = 's';
    else return NULL;

    MOVE_POSITION;

    struct Node_t* node_E = NULL;

    if ( _IS_OP (OP_BR) )
    {
        MOVE_POSITION;

        node_E = GetE (context);
        if ( !_IS_OP (CL_BR))
            SyntaxError (context, __FILE__, __FUNCTION__, __LINE__);

        MOVE_POSITION;

        struct Node_t* node_F = NULL;

        if (func == 'c') node_F = _COS (node_E);
        if (func == 's') node_F = _SIN (node_E);

        return node_F;
    }
    else
        SyntaxError (context, __FILE__, __FUNCTION__, __LINE__);
}

static void SyntaxError (struct Context_t* context, const char* filename const char* func, int line)
{
    fprintf (stderr, PURPLE_TEXT("%s: %s: %d: ") RED_TEXT("ERROR in read >>> SYNTAX-ERROR ") "CUR = %.10s..., SYMBOL = %c (%lg)\n\n",
                     filename, func, line, context->token[Position].str, (int) context->token[Position].value, context->token[Position].value);

    exit (1);
}
