#include <stdio.h>
#include <stdarg.h>
#include <assert.h>

#include "tex.h"

static          FILE* GlobalTex  = NULL;

FILE* open_tex_file (const char* filename)
{
    GlobalTex = fopen (filename, "wb");
    if (GlobalTex == NULL)
    {
        fprintf(stderr, "ERROR open tex_file\n");
        return NULL;
    }

    tex_printf ("\\documentclass{article}\n");
    tex_printf ("\\begin{document}\n");
    tex_printf ("\\section{Differentiator}\n");
    tex_printf ("wazzzuuuup, shut up and take my money.\\newline ");

    return GlobalTex;
}

int tex_printf (const char* message, ...)
{
    va_list args;
    va_start (args, message);

    vfprintf (GlobalTex, message, args);

    va_end (args);

    return 0;
}

void close_tex_file (void)
{
    tex_printf ("\\end{document}\n");

    fflush (GlobalTex);
    fclose (GlobalTex);
}

// void func (void) {}
