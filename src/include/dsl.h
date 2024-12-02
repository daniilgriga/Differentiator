#ifndef DSL_H_
#define DSL_H_

#define _ADD(left, right) new_node (OP, ADD, (left), (right))
#define _SUB(left, right) new_node (OP, SUB, (left), (right))
#define _MUL(left, right) new_node (OP, MUL, (left), (right))
#define _POW(left, right) new_node (OP, POW, (left), (right))
#define _DIV(left, right) new_node (OP, DIV, (left), (right))
#define _SIN(arg)         new_node (OP, SIN,  (arg),    NULL)
#define _COS(arg)         new_node (OP, COS,  (arg),    NULL)
#define _NUM(value)       new_node (NUM, (value), NULL, NULL)
#define _VAR(value)       new_node (VAR, (value), NULL, NULL)

#define _COMPOUND(diff_res) _MUL (diff_res, diff (node->left) )

#define _L         node->left
#define _R         node->right
#define _dL  diff (node->left)
#define _dR  diff (node->right)
#define _cL  copy (node->left)
#define _cR  copy (node->right)

#endif // DSL_H_
