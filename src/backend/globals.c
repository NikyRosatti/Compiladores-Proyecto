#include "Globals.h"
#include <stdio.h>
#include <stdlib.h>

SymbolNode *decl_vars = NULL;

void add_decl(SymbolNode **head, Symbol *sym, Symbol *valor) {
    SymbolNode *nueva = malloc(sizeof(SymbolNode));
    if (!nueva) return;
    nueva->sym = sym;
    nueva->valor = valor;
    nueva->next = *head;
    *head = nueva;
}

/**
 * Imprime las secciones .data y .bss
 * Recorre la lista de globales dos veces.
 * - .data: Para variables con valor inicial explícito distinto de cero.
 * - .bss: Para variables no inicializadas o inicializadas en cero.
 */
void print_global_sections(SymbolNode *head) {
    SymbolNode *n;
    int printed_data_header = 0;
    int printed_bss_header = 0;

    // SECCIÓN .data
    // Imprime solo las variables con un valor inicial NO nulo.
    for (n = head; n; n = n->next) {
        if (n->sym && n->valor && n->valor->valor.value != 0) {
            
            // Imprimir el header solo si no lo hemos hecho
            if (!printed_data_header) {
                printf(".data\n");
                printed_data_header = 1;
            }
            
            // Imprimir la variable inicializada
            printf("%s: .quad %d\n", n->sym->name, n->valor->valor.value);
        }
    }

    // SECCIÓN .bss
    // Imprime las variables no inicializadas (valor es NULL) o inicializadas a 0.
    for (n = head; n; n = n->next) {
        if (n->sym && (!n->valor || n->valor->valor.value == 0)) {
            
            // Imprimir el header (y un espaciado) si no lo hemos hecho
            if (!printed_bss_header) {
                if (printed_data_header) {
                    printf("\n"); // Separador
                }
                printf(".bss\n");
                printed_bss_header = 1;
            }
            
            // Usar .comm para reservar espacio en .bss (8 bytes para un quad)
            printf(".comm %s, 8\n", n->sym->name);
        }
    }

    // Dejar una línea en blanco antes de la sección .text
    if (printed_data_header || printed_bss_header) {
        printf("\n");
    }
}