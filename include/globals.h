#ifndef GLOBALS_H
#define GLOBALS_H

#include "Symbol.h"
#include <stdio.h>
#include <stdlib.h>


/* Nodo de lista enlazada de símbolos */
typedef struct SymbolNode {
    Symbol *sym;
    Symbol *valor;
    struct SymbolNode *next;
} SymbolNode;

/* Lista global con todas las variables globales declaradas */
extern SymbolNode *decl_vars;

/* Funciones para manejar la lista */
void add_decl(SymbolNode **head, Symbol *sym, Symbol *valor);
void print_globals(SymbolNode *head);

#endif /* GLOBALS_H */
