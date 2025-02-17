#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#include "enum.h"
#include "tokens.h"
#include "color_print.h"

int mmain (void)
{
    struct Context_t context = {};

    ctor_keywords (&context);

    const char* string = "sin(temp+5*size)/kek+LOOH$";

    int error = tokenization (&context, string);

    if (error != 0)
        return 1;

    return 0;
}

int tokenization (struct Context_t* context, const char* string)
{
    int length_string = strlen (string);

    fprintf (stderr, PURPLE_TEXT("\nSTART_STRING = %s\n\n"), string);
    int i = 0;

    int old_size      = context->table_size;
    int count_tokens  = 0;

    while (string[i] != '$')
    {
        i = skip_spaces (string, length_string, i);

        int start_i = i;

        if ( isalpha(string[i]) )
        {
            fprintf (stderr, "starting if >>>  start_i = %d\n", start_i);

            while ( isalpha(string[i]) )
                i++;

            int end_i = i;

            fprintf (stderr, "after skiping spaces >>>  end_i = %d\n", end_i);

            size_t length = end_i - start_i;

            double value = check_keyword (context, &string[start_i], length);

            fprintf (stderr, "after check_keyword >>>  value = %lg\n", value);
            fprintf (stderr, "string in this moment >>>  string = '%s'\n\n", &string[start_i]);

            if (value != -1)
            {
                context->token[count_tokens].type   = OP;
                context->token[count_tokens].value  = value;

                count_tokens++;
            }
            else
            {
                add_struct_in_keywords (context, &string[start_i], ID, length);

                context->token[count_tokens].type  = ID;
                context->token[count_tokens].value = context->table_size;

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

            context->token[count_tokens].type  = NUM;
            context->token[count_tokens].value = val;

            count_tokens++;

            continue;
        }

        if (strchr ("+-*/()^", string[i]) != NULL)
        {
            int value = check_keyword (context, &string[start_i], 1);

            context->token[count_tokens].type  = OP;
            context->token[count_tokens].value = value;

            count_tokens++;
            i++;
        }
    }

    context->token[count_tokens].type  = OP;
    context->token[count_tokens].value = '$';

    count_tokens++;

    fprintf (stderr, BLUE_TEXT("\nTOKENS DUMP:\n\n"));

    tokens_dump (context, old_size);

    fprintf (stderr, BLUE_TEXT("\n\nNAME TABLE DUMP:\n\n"));

    name_table_dump (context);

    return 0;
}

int check_keyword (struct Context_t* context, const char* str, int length)
{
    for (int i = 0; i < context->table_size; i++)
        if ( strncmp (str, context->name_table[i].name.str_pointer, length) == 0 )
            return context->name_table[i].name.code;

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

int tokens_dump (struct Context_t* context, int old_size)
{
    if (context == NULL)
    {
        fprintf (stderr, "context is NULL\n");
        return 1;
    }

    int Id_count = context->table_size - (context->table_size - old_size);
    int j = 0;

    fprintf (stderr, "\n\nId_c = %d, cntxt->tbl_sz = %d\n\n", Id_count, context->table_size);

    while (context->token[j].type != 0)
    {
        switch (context->token[j].type)
        {
            case OP:
                fprintf (stderr, BLUE_TEXT("[%.2d] ") "token_type = OP  ||| ADDRESS = [%p] ||| token_value = '%c' (%lg)\n",
                                 j, context[j].token, (int) context->token[j].value, context->token[j].value);
                break;

            case NUM:
                fprintf (stderr, BLUE_TEXT("[%.2d] ") "token_type = NUM ||| ADDRESS = [%p] ||| token_value = %lg\n",
                                 j, context[j].token, context->token[j].value);
                break;

            case ID:
                fprintf (stderr, BLUE_TEXT("[%.2d] ") GREEN_TEXT("token_type = ID  ||| ADDRESS = [%p] ||| token_value = ") BLUE_TEXT("[%lg]\n"), // NOTE in name_table
                                 j, context[j].token, context->token[j].value - 1); //NOTE

                fprintf (stderr, GREEN_TEXT ("     ADDRESS = [%p], name = '%.*s', length = %zu\n\n"),
                                 context[Id_count].name_table, (int) context->name_table[Id_count].name.length, context->name_table[Id_count].name.str_pointer, context->name_table[Id_count].name.length);

                Id_count++;

                break;

            default:
                fprintf (stderr, "ERROR in dump: type = %d\n", context->token->type);
        }

        j++;
    }
    return 0;
}

int name_table_dump (struct Context_t* context)
{
    if (context == NULL)
    {
        fprintf (stderr, "context is NULL\n");
        return 1;
    }

    int j = 0;
    while ( context->name_table[j].name.str_pointer != NULL)
    {
        fprintf (stderr, BLUE_TEXT("[%.2d]: ") "ADDRESS = [%p], name = '%.*s', length = %zu\n",
                         j, context[j].name_table, (int) context->name_table[j].name.length,
                         context->name_table[j].name.str_pointer, context->name_table[j].name.length);
        j++;
    }

    return 0;
}
