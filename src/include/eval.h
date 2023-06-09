#ifndef _EVALH
#define _EVALH
#include "ast.h"
ASTNode* eval_tree(ASTNode* node);
void free_env();
#endif
