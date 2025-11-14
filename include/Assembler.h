#ifndef ASSEMBLER_H
#define ASSEMBLER_H
#define DEBUG_OFFSETS
#include "Tree.h"
#include "Symbol.h"
#include "Intermediate.h"
#include "Globals.h"

/**
 * Calcula los offsets de parámetros y variables locales para la generación de código assembly.
 * Maneja variables globales, locales de método, y variables en bloques anidados.
 */
void calculate_offsets(Tree *node);

/**
 * Marca las declaraciones globales
 */
void mark_globals(Tree *node);

/**
 * Asigna offsets a variables dentro de un bloque
 */
void assign_block_locals(Tree *node, int *offset);

/**
 * Genera el código assembly completo a partir del AST.
 */
void generateAssembly(IRList *list);

// Nombres de registros para los primeros 6 parámetros
static const char* PARAM_REGISTERS[] = {
    "%rdi",  // Parámetro 1
    "%rsi",  // Parámetro 2
    "%rdx",  // Parámetro 3
    "%rcx",  // Parámetro 4
    "%r8",   // Parámetro 5
    "%r9"    // Parámetro 6
};

// Prototipos de helpers
void collect_globals(IRList *irlist);
void print_global_sections(SymbolNode *decl_vars);
void generateLoad(IRCode *inst);
void generateStorage(IRCode *inst);
void generateUminus(IRCode *inst);
void generateCall(IRCode *inst);
void generateCompare(IRCode *inst, const char *set_op);
void generateLogicalOp(IRCode *inst, const char *op);
void generateEnter(IRCode *inst);
void generateInstruction(IRCode *inst);
void generateBinaryOp(IRCode *inst, const char *op);
void generateAssign(IRCode *inst);
void generateLabel(IRCode *inst);
void generateGoto(IRCode *inst);
void generateReturn(IRCode *inst);

#endif // ASSEMBLER_H