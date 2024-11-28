#include <stdio.h>
#include <stdlib.h>

#include "tree.h"
#include "dsl.h"
#include "log.h"

const char* String   = "(c(1+x)+2)*(5+s(x))$";
int         Position = 0;

static struct Node_t* GetG (void);
static struct Node_t* GetE (void);
static struct Node_t* GetT (void);
static struct Node_t* GetP (void);
static struct Node_t* GetN (void);
static struct Node_t* GetV (void);

static struct Node_t* GetFunc (void);
static struct Node_t* GetPow  (void);

static void SyntaxError (int line);

int main (void)
{
    open_log_file ("graph_rec.html");

    struct Node_t* node = GetG ();
    dump_in_log_file (node, "");

    close_log_file ();

    return 0;
}

static struct Node_t* GetG (void)
{
    struct Node_t* val = GetE ();

    if ( String [Position] != '$')
        SyntaxError (__LINE__);

    Position++;

    return val;
}

static struct Node_t* GetE (void)
{
    struct Node_t* val = GetT ();

    while ( String[Position] == '+' || String[Position] == '-' )
    {
        int op = String[Position];
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
    struct Node_t* val = GetP ();

    while ( String[Position] == '*' || String[Position] == '/' )
    {
        int op = String[Position];
        Position++;
        struct Node_t* val2 = GetP ();

        if (op == '*')
            val = _MUL (val, val2);
        else
            val = _DIV (val, val2);
    }

    return val;
}

static struct Node_t* GetP (void)
{
    if ( String[Position] == '(' )
    {
        Position++;

        struct Node_t* val = GetE ();

        if (String[Position] != ')')
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

    if (String[Position] == 'x')
    {
        node = _VAR (String[Position]);
        Position++;
    }

    return node;
}

static struct Node_t* GetN (void)
{
    int val = 0;
    int old_p = Position;

    while ('0' <= String[Position] && String[Position] <= '9')
    {
        val = val * 10 + String[Position] - '0';
        Position++;
    }

    if (old_p == Position) SyntaxError (__LINE__);

    return _NUM (val);
}

static struct Node_t* GetFunc (void)
{
    int func = 0;

    if      ( String[Position] == 'c' ) func = 'c';
    else if ( String[Position] == 's' ) func = 's';
    else return NULL;

    Position++;

    struct Node_t* node_E = NULL;

    if ( String[Position] == '(')
    {
        Position++;

        node_E = GetE ();
        if ( String[Position] != ')')
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
    fprintf (stderr, "ERROR in read: SyntaxError in %d line, CUR = %.10s..., SYMBOL = %c (%d)\n\n",
                     line, String, String[Position], String[Position]);
    exit (1);
}
