#include "Globals.h"

SymbolNode *decl_vars = NULL;

void add_decl(SymbolNode **head, Symbol *sym, Symbol *valor) {
    SymbolNode *nueva = malloc(sizeof(SymbolNode));
    if (!nueva) return;
    nueva->sym = sym;
    nueva->valor = valor;
    nueva->next = *head;
    *head = nueva;
}

void print_globals_data(SymbolNode *head) {
    for (SymbolNode *n = head; n; n = n->next) {
        if (n->sym) {
            printf("%s: .quad %d\n", n->sym->name, n->valor->valor.value);
        }
    }
}
