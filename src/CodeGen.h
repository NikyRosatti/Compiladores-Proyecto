#ifndef CODEGEN_H
#define CODEGEN_H

#include "Tree.h"
#include "SymbolTable.h"

void genCode(Tree *root, int level);

char* newTemp();

char* genExpr(Tree *node);

#endif /* CODEGEN_H */