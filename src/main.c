#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "log.h"
#include "tex.h"
#include "tree.h"
#include "eval.h"
#include "diff.h"
#include "get_g.h"
#include "file_data.h"
#include "color_print.h"

int main (int argc, const char* argv[])
{
    struct Buffer_t buffer = {};

    const char* input = (argc >= 2)? argv[1] : "../build/example.txt";

    const char* string = file_reader (&buffer, input);
    /*struct Node_t* root = read_example (example, &buffer);
    if (root == NULL)
        return 1;*/

    fprintf (stderr, "START of reading.\n");
    struct Node_t* root = GetG (string);
    fprintf (stderr, "END of reading.\n");

    open_log_file ("../build/dump.html");

    dump_in_log_file (root, "node --- start of programm");

    open_tex_file ("output.tex");

    struct Node_t* diff_node = root; //diff (root);

    //dump_in_log_file (diff_node, "diff_node --- WITHOUT simplification");

    verificator (diff_node, __FILE__, __LINE__);

    fprintf (stderr, "-------------------------------------BEFORE_SIMPLIFY-------------------------------------\n");

    int changes = simplification (diff_node, NULL);

    fprintf (stderr, "-------------------------------------AFTER_SIMPLIFY-------------------------------------\n");

    verificator (diff_node, __FILE__, __LINE__);

    dump_in_log_file (diff_node, "diff_node --- WITH simplification");

    close_tex_file (); // create_pdf_latex - fflush and system , atexit

    system ("pdflatex output.tex");

    close_log_file ();

    fprintf (stderr, "start destructor\n");
    destructor (root, &buffer);

    return 0;
}
