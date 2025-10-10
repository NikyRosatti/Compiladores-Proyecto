#include "globals.h"

SymbolNode *decl_vars = NULL;

void add_decl(SymbolNode **head, Symbol *sym) {
    SymbolNode *nueva = malloc(sizeof(SymbolNode));
    if (!nueva) return;
    nueva->sym = sym;
    nueva->next = *head;
    *head = nueva;
}

void print_globals_data(SymbolNode *head) {
    for (SymbolNode *n = head; n; n = n->next) {
        if (n->sym->is_global) {
            if (n->sym->valor.value) {
                printf("%s: .quad %d\n", n->sym->name, n->sym->valor.value);
            }
        }
    }
}

void print_globals_bss(SymbolNode *head) {
    for (SymbolNode *n = head; n; n = n->next) {
        if (n->sym->is_global) {
            if (!n->sym->valor.value) {
                printf("%s: .zero 8\n", n->sym->name);
            }
        }
    }
}
