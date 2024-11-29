#ifndef HEAD_H_
#define HEAD_H_

#include <math.h>

struct Node_t
{
    int type;

    double value;

    struct Node_t* left;
    struct Node_t* right;
};

struct Buffer_t
{
    char* buffer_ptr;
    char* current_ptr;
    int   file_size;
};

enum symbols
{
    NUM = 1,
    OP  = 2,
    VAR = 3
};

enum operations
{
    ADD = '+',
    SUB = '-',
    DIV = '/',
    MUL = '*',
    SIN = 's',
    COS = 'c',
    POW = '^',
    LOG = 'l'
};

#endif
