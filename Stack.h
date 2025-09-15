#ifndef STACK_H
#define STACK_H

#include "SymbolTable.h" // para SymbolType

//Pila de tipos
typedef struct TypeStack {
    SymbolType arr[100];
    int top;
} TypeStack;

void initTypeStack(TypeStack *s);
void pushType(TypeStack *s, SymbolType t);
SymbolType popType(TypeStack *s);
SymbolType peekType(TypeStack *s);
int isEmptyTypeStack(TypeStack *s);

typedef struct ScopeStack {
    SymbolTable *arr[100];  // tablas de símbolos
    int top;
} ScopeStack;

void initScopeStack(ScopeStack *s);
void pushScope(ScopeStack *s, SymbolTable *t);
void popScope(ScopeStack *s);
SymbolTable* peekScope(ScopeStack *s);
Symbol* lookupInScopes(ScopeStack *s, const char *name);

#endif