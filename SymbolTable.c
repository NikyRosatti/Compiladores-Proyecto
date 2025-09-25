#include <stdlib.h>
#include <string.h>
#include "SymbolTable.h"

SymbolTable* createTable() {
    SymbolTable *t = malloc(sizeof(SymbolTable));
    t->size = 0;
    t->capacity = 16;
    t->symbols = malloc(sizeof(Symbol*) * t->capacity);
    return t;
}

Symbol* insertSymbol(SymbolTable *table, const char *name, SymbolType type, Valores value ) {
    if (!name) {
    fprintf(stderr, "Error: insertSymbol recibió un nombre NULL\n");
    return NULL;
    }
    // Chequear duplicados
    for (int i = 0; i < table->size; i++) {
        if (table->symbols[i] && table->symbols[i]->name) {
            if (strcmp(table->symbols[i]->name, name) == 0) {
                return table->symbols[i]; // ya existe
            }
        }
    }

    if (table->size == table->capacity) {
        table->capacity *= 2;
        table->symbols = realloc(table->symbols, sizeof(Symbol*) * table->capacity);
    }

    Symbol *s = malloc(sizeof(Symbol));
    s->name = strdup(name);
    s->type = type;
    if (type == TYPE_INT )
    {
        s->valor.value = value.value;    
    } else if (type == TYPE_BOOL)
    {
        s->valor.value = value.value;
    }

    table->symbols[table->size++] = s;
    return s;
}

Symbol* lookupSymbol(SymbolTable *table, const char *name) {
    if (table == NULL || table->symbols == NULL) return NULL;

    for (int i = 0; i < table->size; i++) {
        if (table->symbols[i] && strcmp(table->symbols[i]->name, name) == 0) {
            return table->symbols[i];
        }
    }
    return NULL;
}

void printSymbolTable(SymbolTable *table) {
    printf("Tabla de símbolos:\n");
    for (int i = 0; i < table->size; i++) {
        Symbol *s = table->symbols[i];
        printf("  %s = %d (tipo: %d)\n", s->name, s->valor.value, s->type);
    }
}
