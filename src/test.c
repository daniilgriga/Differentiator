#include <stdio.h>
#include <stdlib.h>

const char* String   = "2*(5+3)$";
int         Position =   0;

int GetG (void);
int GetN (void);
int GetE (void);
int GetT (void);
int GetP (void);
void SyntaxError (int line);

int main (void)
{
    int answer = GetG ();
    fprintf (stderr, "\n\nanswer = %d\n\n", answer);

    return 0;
}

int GetG (void)
{
    int val = GetE ();

    if ( String [Position] != '$')
        SyntaxError (__LINE__);

    Position++;

    return val;
}

int GetN (void)
{
    int val = 0;
    int old_p = Position;

    while ('0' <= String[Position] && String[Position] <= '9')
    {
        fprintf (stderr, "pos = %d, str = %s, val = %d\n", Position, String, val);
        val = val * 10 + String[Position] - '0';
        Position++;
        fprintf (stderr, "pos = %d, str = %s, val = %d\n", Position, String, val);
    }

    if (old_p == Position) SyntaxError (__LINE__);

    return val;
}

int GetE (void)
{
    int val = GetT ();

    while ( String[Position] == '+' || String[Position] == '-' )
    {
        int op = String[Position];
        Position++;
        int val2 = GetT ();

        if (op == '+')
            val += val2;
        else
            val -= val2;
    }

    return val;
}

int GetT (void)
{
    int val = GetP ();
    while ( String[Position] == '*' || String[Position] == '/' )
    {
        int op = String[Position];
        Position++;
        int val2 = GetP ();

        if (op == '*')
            val *= val2;
        else
            val /= val2;
    }

    return val;
}

int GetP (void)
{
    if ( String[Position] == '(' )
    {
        Position++;

        int val = GetE ();
        if (String[Position] != ')')
            SyntaxError (__LINE__);

        Position++;
        return val;
    }
    else
        return GetN ();
}

void SyntaxError (int line)
{
    fprintf (stderr, "ERROR in read: SyntaxError in %d line\n\n", line);
    exit (1);
}
