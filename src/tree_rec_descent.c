#include <stdio.h>
#include <stdlib.h>

#include "tokens.h"
#include "tree.h"
#include "dsl.h"
#include "log.h"
#include "get_g.h"
#include "enum.h"

struct Token_t* Token   = NULL;
int            Position =  0;

/*         GRAMMAR
 *
 *
 *
 *
 */

static struct Node_t* GetE (void);
static struct Node_t* GetT (void);
static struct Node_t* GetP (void);
static struct Node_t* GetN (void);
static struct Node_t* GetV (void);

static struct Node_t* GetFunc (void);
static struct Node_t* GetPow  (void);

static void SyntaxError (int line);

#define _IS_OP(val)  ( Token[Position].type == OP &&    \
                         Token[Position].value == (val) )

struct Node_t* GetG (struct Token_t* token)
{
    Token = token;

    struct Node_t* val = GetE ();

    if ( !_IS_OP('$') )
        SyntaxError (__LINE__); //TODO str in format printf;

    Position++;

    return val;
}

static struct Node_t* GetE (void)
{
    struct Node_t* val = GetT ();

    fprintf (stderr, "in Gete: val = %lg, CUR = %.10s\n", Token[Position].value, Token[Position].str);

    while ( _IS_OP (ADD) || _IS_OP (SUB) )
    {
        int op = Token[Position].value;

        Position++;

        struct Node_t* val2 = GetT ();

        if (op == '+')
            val = _ADD (val, val2);
        else
            val = _SUB (val, val2);
    }

    return val;
}

static struct Node_t* GetT (void)
{
    struct Node_t* val = GetPow ();

    while ( _IS_OP (MUL) || _IS_OP (DIV) )
    {
        int op = Token[Position].value;

        Position++;

        struct Node_t* val2 = GetPow ();

        if (op == '*')
            val = _MUL (val, val2);
        else
            val = _DIV (val, val2);
    }

    return val;
}

static struct Node_t* GetPow (void)
{
    struct Node_t* node = GetP ();

    while ( _IS_OP (POW) )
    {
        Position++;

        struct Node_t* exp = GetP ();

        node = _POW (node, exp);
    }

    return node;
}

static struct Node_t* GetP (void)
{
    if ( _IS_OP (OP_BR) )
    {
        Position++;

        struct Node_t* val = GetE ();

        fprintf (stderr, "val = %lg, CUR = %.10s --- ", Token[Position].value, Token[Position].str);
        fprintf (stderr, "node_value = %lg, node_type = %d\n\n", val->value, val->type);

        if ( !_IS_OP (CL_BR) )
            SyntaxError (__LINE__);

        Position++;

        return val;
    }
    else
    {
        struct Node_t* node_V = GetV ();

        if (node_V != NULL)
            return node_V;
        else
        {
            struct Node_t* node_F = GetFunc ();
            if (node_F != NULL)
                return node_F;
            else
                return GetN ();
        }

    }
}

static struct Node_t* GetV (void)
{
    struct Node_t* node = NULL;

    if ( _IS_OP ('x') )
    {
        node = _VAR (Token[Position].value);

        Position++;
    }

    return node;
}

static struct Node_t* GetN (void)
{
    if (Token[Position].type == NUM)
        return _NUM (Token[Position].value);
}

static struct Node_t* GetFunc (void)
{
    int func = 0;

    if      ( Token[Position].value == 'c' ) func = 'c';
    else if ( Token[Position].value == 's' ) func = 's';
    else return NULL;

    Position++;

    struct Node_t* node_E = NULL;

    if ( _IS_OP (OP_BR) )
    {
        Position++;

        node_E = GetE ();
        if ( !_IS_OP (CL_BR))
            SyntaxError (__LINE__);

        Position++;

        struct Node_t* node_F = NULL;

        if (func == 'c') node_F = _COS (node_E);
        if (func == 's') node_F = _SIN (node_E);

        return node_F;
    }
    else
        SyntaxError (__LINE__);
}

static void SyntaxError (int line)
{
    fprintf (stderr, "ERROR in read: SyntaxError in %d line, CUR = %.10s..., SYMBOL = %c (%lg)\n\n",
                     line, Token[Position].str, (int) Token[Position].value, Token[Position].value);
    exit (1);
}
