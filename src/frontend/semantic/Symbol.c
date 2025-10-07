#include <stdlib.h>
#include <string.h>
#include "Symbol.h"
#include "Tree.h"

Symbol* createSymbol(const char *name, struct Tree *typeNode, SymbolKind kind, Valores valor) {
    Symbol *s = malloc(sizeof(Symbol));
    if (!s) {
        perror("malloc");
        exit(1);
    }

    // traducir nodo a SymbolType
    SymbolType t;
    switch (typeNode->tipo) {
        case NODE_T_INT:  t = TYPE_INT; break;
        case NODE_T_BOOL: t = TYPE_BOOL; break;
        case NODE_T_VOID: t = TYPE_VOID; break;
        default:          t = TYPE_ERROR; break;
    }

    s->type = t;
    s->kind = kind;

    // copiamos el nombre
    s->name = strdup(name);
    if (!s->name) {
        perror("strdup");
        free(s);
        exit(1);
    }

    s->valor = valor;
    s->node = NULL; // lo podés linkear después si necesitás

    return s;
}


Symbol *createSymbolCall(const char *name, SymbolKind kind) {
    Symbol *sym = malloc(sizeof(Symbol));
    if (!sym) {
        fprintf(stderr, "Error: no se pudo asignar memoria para Symbol\n");
        exit(EXIT_FAILURE);
    }

    // Inicializamos con valores neutros / desconocidos
    sym->type = TYPE_ERROR;    // tipo desconocido por ahora
    sym->kind = kind;           
    sym->name = strdup(name);  // copiamos el nombre
    Valores v = {0};
    sym->valor = v;  // nada asignado todavía
    sym->node = NULL;          // no hay nodo asociado aún

    return sym;
}
