#include <stdio.h>
#include "Stack.h"
#define MAX_SCOPES 100

SymbolTable *allScopes[MAX_SCOPES];
int allScopesCount = 0;

TypeStack typeStack;
ScopeStack scopeStack;

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
    // guardamos el scope antes de eliminarlo
    if (allScopesCount < MAX_SCOPES) {
        allScopes[allScopesCount++] = s->arr[s->top];
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

void printAllScopes() {
    printf("=== TODOS LOS SCOPES ===\n");
    for (int i = 0; i <= scopeStack.top; i++) {
        printf("Scope %d:\n", i);
        printTable(scopeStack.arr[i]); // asumimos que printTable() imprime todos los símbolos del scope
    }
    printf("=======================\n");
}
