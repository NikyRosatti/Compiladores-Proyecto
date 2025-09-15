#ifndef TREE_H
#define TREE_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "Symbol.h"

struct Tree;

typedef enum {
    NODE_CODE,
    NODE_METHOD,
    NODE_PROGRAM,
    NODE_METHOD_HEADER,
    NODE_UMINUS,
    NODE_ASSIGN,
    NODE_ARGS,
    NODE_ID,
    NODE_INT,
    NODE_TRUE,
    NODE_FALSE,
    NODE_DECLARATION,
    NODE_SUM,
    NODE_RES,
    NODE_MUL,
    NODE_DIV,
    NODE_MOD,
    NODE_EQ,
    NODE_NEQ,
    NODE_LT,
    NODE_GT,
    NODE_LE,
    NODE_GE,
    NODE_AND,
    NODE_OR,
    NODE_NOT,
    NODE_PARENS,
    NODE_IF,
    NODE_IF_ELSE,
    NODE_WHILE,
    NODE_RETURN,
    NODE_METHOD_CALL,
    NODE_LIST,
    NODE_BLOCK,
    NODE_T_INT,
    NODE_T_BOOL,
    NODE_T_VOID
} typeTree;

typedef struct Tree {
    typeTree tipo;            /* nombre del nodo: "INT", "+", "*" ... */
    struct Symbol *sym;
    struct Tree *left;
    struct Tree *right;
}Tree;

typedef struct Queue {
    int cola[100]; // no "int[100] cola"
    int head;      // índice del frente de la cola
    int tail;      // índice del final de la cola
    int size;      // cantidad de elementos
} Queue;

/* Funciones principales de manejo de árbol */
Tree* createNode(typeTree tipo, Symbol *sym, Tree *left, Tree *right);
void printTree(Tree *n, int level);
const char* tipoToStr(typeTree t);
void execute(Tree *node);

/*Chequeo semantico */
SymbolType check_types(Tree *node);


#endif /* TREE_H */
