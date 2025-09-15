#include <stdio.h>
#include "Stack.h"

TypeStack typeStack;

void initTypeStack(TypeStack *s) {
    s->top = -1;
}

void pushType(TypeStack *s, SymbolType t) {
    if (s->top == 99) {
        printf("Pila de tipos llena\n");
        return;
    }
    s->arr[++s->top] = t;
}

SymbolType popType(TypeStack *s) {
    if (s->top == -1) {
        printf("Pila de tipos vacía\n");
        return TYPE_VOID;
    }
    return s->arr[s->top--];
}

SymbolType peekType(TypeStack *s) {
    if (s->top == -1) return TYPE_VOID;
    return s->arr[s->top];
}

int isEmptyTypeStack(TypeStack *s) {
    return s->top == -1;
}

void initScopeStack(ScopeStack *s) {
    s->top = -1;
}

void pushScope(ScopeStack *s, SymbolTable *t) {
    if (s->top == 99) {
        printf("Pila de scopes llena\n");
        return;
    }
    s->arr[++s->top] = t;
}

void popScope(ScopeStack *s) {
    if (s->top == -1) {
        printf("Pila de scopes vacía\n");
        return;
    }
    s->top--;
}

SymbolTable* peekScope(ScopeStack *s) {
    if (s->top == -1) return NULL;
    return s->arr[s->top];
}

Symbol* lookupInScopes(ScopeStack *s, const char *name) {
    for (int i = s->top; i >= 0; i--) {
        Symbol *sym = lookupSymbol(s->arr[i], name);
        if (sym) return sym;
    }
    return NULL;
}