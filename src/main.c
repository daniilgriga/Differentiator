#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "log.h"
#include "tex.h"
#include "tree.h"
#include "eval.h"
#include "enum.h"
#include "diff.h"
#include "tokens.h"
#include "get_g.h"
#include "string.h"
#include "file_data.h"
#include "color_print.h"

int main (int argc, const char* argv[])
{
    struct  Buffer_t  buffer = {};
    struct Context_t context = {};

    const char* input = (argc >= 2)? argv[1] : "../build/example.txt";

    const char* string = file_reader (&buffer, input);

    ctor_keywords (&context);

    fprintf (stderr, BLUE_TEXT("START Tokenization\n\n"));
    int error = tokenization (&context, string);
    if (error != 0)
        return 1;
    fprintf (stderr, BLUE_TEXT("\nEND Tokenization\n\n"));

    struct Node_t* root = new_node (ROOT, -1, NULL, NULL);

    fprintf (stderr, BLUE_TEXT("\nSTART Rec_descent\n\n"));
    struct Node_t* node = GetG (&context);
    root->left = node;
    fprintf (stderr, BLUE_TEXT("\nEND Rec_descent\n\n"));

    open_log_file ("../build/dump.html");

    fprintf (stderr, "root = [%p] || type: %d, root->value = %lg, root->left = [%p] || type: %d, root->left->value = %lg\n",
                      root, root->type, root->value, root->left, root->left->type, root->left->value);

    dump_in_log_file (root, "node --- start of programm");

    open_tex_file ("output.tex");

    tex_printf_tree (node, NULL, &context, "wazzzuuuup, shut up and take my derivative of this function: \\newline ", 'y');

    struct Node_t* diff_node = node; //diff (node);
    root->left = diff_node;

    dump_in_log_file (root, "diff_node --- WITHOUT simplification");

    simplification_of_expression (root, NULL);
    diff_node = root->left;

    tex_printf_tree (node, diff_node, &context, "with all simplification", 'n');

    dump_in_log_file (root, "diff_node --- WITH all simplification");

    close_tex_file (); // create_pdf_latex - fflush and system , atexit

    system ("pdflatex output.tex");

    close_log_file ();

    destructor (root, &buffer);

    return 0;
}

int ctor_keywords (struct Context_t* context)
{
    context->table_size = 0;

    add_struct_in_keywords (context, "sin",  SIN , strlen ("sin") );
    add_struct_in_keywords (context, "cos",  COS , strlen ("cos") );
    add_struct_in_keywords (context,  "ln",   LN , strlen ( "ln") );
    add_struct_in_keywords (context,   "+",  ADD , strlen (  "+") );
    add_struct_in_keywords (context,   "-",  SUB , strlen (  "-") );
    add_struct_in_keywords (context,   "*",  MUL , strlen (  "*") );
    add_struct_in_keywords (context,   "/",  DIV , strlen (  "/") );
    add_struct_in_keywords (context,   "^",  POW , strlen (  "^") );
    add_struct_in_keywords (context,   "(", OP_BR, strlen (  "(") );
    add_struct_in_keywords (context,   ")", CL_BR, strlen (  ")") );

    return 0;
}

int add_struct_in_keywords (struct Context_t* context, const char* str, enum Operations code, int length)
{
    context->name_table[context->table_size].name.str_pointer = str;
    context->name_table[context->table_size].name.code        = code;
    context->name_table[context->table_size].name.is_keyword  = 1;    // YES = 1;
    context->name_table[context->table_size].name.length      = length;

    context->table_size++;

    return 0;
}
