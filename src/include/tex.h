#ifndef TEX_C_
#define TEX_C_

#pragma GCC diagnostic warning "-Wformat=2"

FILE* open_tex_file (const char* filename);

int tex_printf (const char* message, ...) __attribute__((format(printf, 1, 2)));

void close_tex_file (void);

#endif // TEX_C_
