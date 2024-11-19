#ifndef HEAD_H_
#define HEAD_H_

struct Node_t
{
    int type;

    double data;

    struct Node_t* left;
    struct Node_t* right;
};

struct Buffer_t
{
    char* buffer_ptr;
    char* current_ptr;
};

#endif
