#ifndef DIFF_H_
#define DIFF_H_

struct Node_t* diff (struct Node_t* node);

int simplification_typical_operations (struct Node_t* root, struct Node_t* parent);

double constant_folding (struct Node_t* root);

int simplification_of_expression (struct Node_t* root, struct Node_t* parent);

void verificator (struct Node_t* node, const char* filename, int line);

#endif // DIFF_H_
