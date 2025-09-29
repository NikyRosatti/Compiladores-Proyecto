#ifndef SYMBOL_TABLE_H
#define SYMBOL_TABLE_H

#include "Symbol.h"

typedef struct SymbolTable {
    Symbol **symbols;
    int size;
    int capacity;
} SymbolTable;

SymbolTable* createTable();
Symbol* insertSymbol(SymbolTable *table, const char *name, SymbolType type, Valores value);
Symbol* lookupSymbol(SymbolTable *table, const char *name);
void printSymbolTable(SymbolTable *table);

#endif
