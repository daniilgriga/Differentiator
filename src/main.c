#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "log.h"
#include "tex.h"
#include "tree.h"
#include "eval.h"
#include "diff.h"
#include "color_print.h"

int main (int argc, const char* argv[]) //TODO - open texfile...
{
    struct Buffer_t buffer = {};

    const char* input = (argc >= 2)? argv[1] : "../build/example.txt";

    FILE* example = fopen (input, "rb");
    if (!example) { fprintf (stderr, "Error: File '%s' NOT opened", input); perror (""); return 1; }

    struct Node_t* root = read_example (example, &buffer);
    if (root == NULL)
        return 1;

    open_log_file ("../build/dump.html");

    dump_in_log_file (root, "start of programm");

    open_tex_file ("output.tex");

    struct Node_t* diff_node = diff (root);
    // printf ("\n""answer = %lg\n", eval (root));

    dump_in_log_file (     root, "root --- end of programm");
    dump_in_log_file (diff_node, "diff_node --- end of programm");

    close_tex_file (); // create_pdf_latex - fflush and system , atexit

    system ("pdflatex output.tex");

    close_log_file ();

    destructor (root, &buffer);

    return 0;
}
