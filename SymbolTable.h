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
void addSymbol(SymbolTable *table, Symbol *sym);
Symbol* lookupSymbol(SymbolTable *table, const char *name);
void printSymbolTable(SymbolTable *table);
void printTable(SymbolTable *table);


#endif
