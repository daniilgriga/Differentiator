#include <stdio.h>
#include <stdarg.h>
#include <assert.h>
#include <stdlib.h>

#include "head.h"
#include "log.h"

static FILE* LOG_FILE = NULL;

int log_printf (const char* message, ...)
{
    va_list args;
    va_start (args, message);

    vfprintf (LOG_FILE, message, args);

    va_end (args);

    return 0;
}

FILE* open_log_file (const char* filename)
{
    LOG_FILE = fopen (filename, "wb");
    if (LOG_FILE == NULL)
    {
        printf("ERROR");
        return NULL;
    }

    fprintf (LOG_FILE, "<pre>\n");

    return LOG_FILE;
}

int write_log_file (const char* reason)
{
    log_printf ("<body style=\"background-color: #AFEEEE\">");

    log_printf ("<hr> <h2> %s </h2> <br> <hr>\n\n", reason);

    static int dump_number = 1;
    static char filename[50] = {};
    char    command_name[100] = {};

    sprintf (filename, "../build/graph_tree%d.svg", dump_number++);
    sprintf (command_name, "dot ../build/graph_tree.dot -Tsvg -o %s", filename);

    system  (command_name);

    log_printf ("\n\n<img src=\"%s\">", filename);

    fflush (LOG_FILE);

    return 0;
}

void close_log_file (void)
{
    fclose (LOG_FILE);
}
