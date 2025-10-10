#ifndef SYMBOL_H
#define SYMBOL_H

#include <stdio.h>

typedef union {
    int value;             /* usado si es número */
    char *string;           /* usado si es identificador */
}Valores;
    
typedef enum {
    VAR,
    FUNC
} SymbolKind;

typedef enum {
    TYPE_INT,
    TYPE_BOOL,
    TYPE_VOID,
    TYPE_ERROR,
    TYPE_LABEL
} SymbolType;

typedef struct Symbol {
    SymbolType type;        /* tipo de dato */
    SymbolKind kind;       /* variable o función */
    char *name;   
    Valores valor;      /* usado si es identificador */
    struct Tree *node;

    // Campos para generación de código
    int offset;           // Offset relativo a %rbp (puede ser positivo o negativo)
    int is_param;         // 1 si es parámetro, 0 si es local
    int is_global;        // 1 si es variable global, 0 si es local
    int param_index;      // Índice del parámetro (0-5 por registro, 6+ por stack)
    
    // Solo para símbolos de métodos
    int param_count;
    int local_count;
    int total_stack_space;
    int is_temp;

} Symbol;
struct Tree;

Symbol *createSymbolCall(const char *name, SymbolKind kind);
Symbol* createSymbol(const char *name, struct Tree *typeNode, SymbolKind kind, Valores valor);
#endif /*SYMBOL_H*/