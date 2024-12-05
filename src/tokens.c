#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#include "enum.h"
#include "tokens.h"

int mmain (void)
{
    const char* string = "(8+9)*(5/2-3)$";

    const struct Token_t* token = tokenization (string);

    return 0;
}

struct Token_t* tokenization (const char* string)
{
    static struct Token_t tokens[MAX_SIZE_TOKENS] = {};

    int length_string = strlen (string);

    fprintf (stderr, "START_STRING = %s\n\n", string);
    int i = 0;
    int count_tokens = 0;

    while (string[i] != '$')
    {
        i = skip_spaces (string, length_string, i);

        int start_i = i;

        if ( isalpha(string[i]) )
        {
            fprintf (stderr, "starting if>>>  start_i = %d\n", start_i);

            while ( isalpha(string[i]) )
                i++;

            int end_i = i;

            fprintf (stderr, "after skiping spaces>>>  end_i = %d\n", end_i);

            int length = end_i - start_i;

            double value = check_keyword (&string[start_i], length);

            fprintf (stderr, "after check_keyword>>>  value = %lg\n", value);
            fprintf (stderr, "string in this moment>>>  string = '%s'\n", &string[start_i]);

            if (value != -1)
            {
                tokens[count_tokens].type   = OP;
                tokens[count_tokens].value  = value;

                count_tokens++;
            }
            else
            {
                if ( strncmp (&string[start_i], "x", length) == 0)
                {
                    tokens[count_tokens].type  = VAR;
                    tokens[count_tokens].value = string[start_i];
                }
                else
                {
                    tokens[count_tokens].type   = ID;
                    tokens[count_tokens].str    = &string[start_i];
                    tokens[count_tokens].length = length;
                }

                count_tokens++;
            }

            continue;
        }

        if ( isdigit (string[i]) )
        {
            int val = 0;

            while ('0' <= string[i] && string[i] <= '9')
            {
                val = val * 10 + string[i] - '0';
                i++;
            }

            tokens[count_tokens].type  = NUM;
            tokens[count_tokens].value = val;

            count_tokens++;

            continue;
        }

        if (strchr ("+-*/()^", string[i]) != NULL)
        {
            int value = check_keyword (&string[start_i], 1);

            tokens[count_tokens].type  = OP;
            tokens[count_tokens].value = value;

            count_tokens++;
            i++;
        }

    }

    int j = 0;
    while ( tokens[j].type != 0)
    {
        if ( tokens_dump (&tokens[j]) == 1 )
            return NULL;
        j++;
    }

    return tokens;
}

int check_keyword (const char* str, int length)
{
    struct Keyword_t
    {
        const char* string;
        enum Operations code;
    };

    struct Keyword_t keywords[AMOUNT_OF_KEYWORDS] = { { "sin",  SIN  },
                                                      { "cos",  COS  },
                                                      {  "ln",   LN  },
                                                      {   "+",  ADD  },
                                                      {   "-",  SUB  },
                                                      {   "*",  MUL  },
                                                      {   "/",  DIV  },
                                                      {   "^",  POW  },
                                                      {   "(", OP_BR },
                                                      {   ")", CL_BR } };

    for (int i = 0; i < AMOUNT_OF_KEYWORDS; i++)
        if ( strncmp (str, keywords[i].string, length) == 0 )
            return keywords[i].code;

    return -1;
}

int skip_spaces (const char* string, int length, int current_i)
{
    int i = current_i;

    for (i; i < length; i++)
        if ( !isspace(string[i]) )
            break;

    return i;
}

int tokens_dump (const struct Token_t* token)
{
    if (token == NULL)
    {
        fprintf (stderr, "Token is NULL\n");
        return 1;
    }

    switch (token->type)
    {
        case OP:
            fprintf (stderr, "token_type = OP ||| CUR = [%p] ||| token_op_code = '%c' (%lg)\n",
                             token, (int) token->value, token->value);
            break;

        case NUM:
            fprintf (stderr, "token_type = NUM ||| CUR = [%p] ||| token_value = %lg\n",
                             token, token->value);
            break;

        case ID:
            fprintf (stderr, "token_type = ID ||| CUR = [%p] ||| token_str = '%s' ||| token_length = %d\n",
                             token, token->str, token->length);
            break;

        case VAR:
            fprintf (stderr, "token_type = VAR ||| CUR = [%p] ||| token_str = '%c' (%lg)\n",
                             token, (int) token->value, token->value);
            break;

        default:
            fprintf (stderr, "ERROR in dump: type = %d\n", token->type);
    }

    return 0;
}
