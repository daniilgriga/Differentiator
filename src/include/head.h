#ifndef HEAD_H_
#define HEAD_H_

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
    DIV = '/'
};

#endif
