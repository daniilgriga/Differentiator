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
#include "file_data.h"
#include "color_print.h"

int main (int argc, const char* argv[])
{
    struct Buffer_t buffer = {};

    const char* input = (argc >= 2)? argv[1] : "../build/example.txt";

    const char* string = file_reader (&buffer, input);

    struct Token_t* token = tokenization (string);

    struct Node_t* root = new_node (ROOT, -1, NULL, NULL);

    struct Node_t* node = GetG (token);
    root->left = node;

    open_log_file ("../build/dump.html");

    dump_in_log_file (root, "node --- start of programm");

    open_tex_file ("output.tex");

    tex_printf_tree (node, NULL, "wazzzuuuup, shut up and take my derivative of this function: \\newline ", 'y');

    struct Node_t* diff_node = diff (node);
    root->left = diff_node;

    dump_in_log_file (root, "diff_node --- WITHOUT simplification");

    simplification_of_expression (root, NULL);
    diff_node = root->left;

    tex_printf_tree (node, diff_node, "with all simplification", 'n');

    dump_in_log_file (root, "diff_node --- WITH all simplification");

    close_tex_file (); // create_pdf_latex - fflush and system , atexit

    system ("pdflatex output.tex");

    close_log_file ();

    destructor (root, &buffer);

    return 0;
}
