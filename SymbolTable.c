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

//params de un metodo al scope actual
void addSymbol(SymbolTable *table, Symbol *sym) {
    if (!table || !sym) return;

    // Chequeo duplicados
    for (int i = 0; i < table->size; i++) {
        if (table->symbols[i] && table->symbols[i]->name) {
            if (strcmp(table->symbols[i]->name, sym->name) == 0) {
                // Ya existe, no lo agregamos
                return;
            }
        }
    }

    if (table->size == table->capacity) {
        table->capacity *= 2;
        table->symbols = realloc(table->symbols, sizeof(Symbol*) * table->capacity);
    }

    table->symbols[table->size++] = sym;
}


Symbol* lookupSymbol(SymbolTable *table, const char *name) {
    if (!table) {
        fprintf(stderr, "Error: intento de lookup en tabla NULL para %s\n", name);
        return NULL;
    }
    for (int i = 0; i < table->size; i++) {
        if (strcmp(table->symbols[i]->name, name) == 0)
            return table->symbols[i];
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

void printTable(SymbolTable *table) {
    for (int i = 0; i < table->size; i++) {
        Symbol *s = table->symbols[i];
        if (s) {
            printf("  - %s (type=%d, value=%d)\n",
                   s->name ? s->name : "anon",
                   s->type,
                   s->valor.value);
        }
    }
}

Symbol* copySymbol(Symbol *orig) {
    if (!orig) return NULL;
    Symbol *s = malloc(sizeof(Symbol));
    if (!s) { fprintf(stderr, "Error: sin memoria\n"); exit(1); }
    s->type = orig->type;
    s->valor = orig->valor;
    if(orig->name) {
        s->name = strdup(orig->name);
    } else {
        s->name = NULL;
    }
    return s;
}

