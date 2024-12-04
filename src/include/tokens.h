#ifndef TOKENS_H_
#define TOKENS_H_

#define MAX_SIZE_TOKENS    1000
#define MAX_SIZE_OPERATOR  20
#define AMOUNT_OF_KEYWORDS 10

struct Token_t
{
    int type;

    const char* str;
    int         length;
    double      value;

    struct Token_t* left;
    struct Token_t* right;
};

struct Token_t* tokenization (const char* string);

int check_keyword (const char* str, int length);

int tokens_dump (const struct Token_t* token);

int skip_spaces (const char* string, int length, int current_i);


#endif // TOKENS_H_
