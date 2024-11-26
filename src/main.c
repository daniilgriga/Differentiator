#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "log.h"
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

    printing_in_all_ways (root);

    FILE* tex_file = tex_file_open ("output.tex");

    struct Node_t* diff_node = diff (root, tex_file);
    // printf ("\n""answer = %lg\n", eval (root));

    dump_in_log_file (     root, "root --- end of programm");
    dump_in_log_file (diff_node, "diff_node --- end of programm");

    tex_file_close (tex_file); // create_pdf_latex - fflush and system , atexit

    system ("pdflatex output.tex");

    close_log_file ();

    destructor (root, &buffer);

    return 0;
}
