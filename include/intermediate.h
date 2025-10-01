#ifndef INTERMEDIATE_H
#define INTERMEDIATE_H

#include "Tree.h"
#include <stdio.h>
#include <stdlib.h>

typedef enum {
    IR_LOAD, IR_STORE, IR_ADD, IR_SUB, IR_MUL, IR_DIV, IR_MOD,
    IR_AND, IR_OR, IR_NOT,
    IR_EQ, IR_NEQ, IR_LT, IR_LE, IR_GT, IR_GE,
    IR_LABEL, IR_GOTO, IR_IF, IR_IFELSE, IF_WHILE, IR_RETURN, IR_PARAM, IR_CALL, IR_METH_EXT,
    IR_PRINT
} IRInstr;


typedef struct {
    IRInstr op;
    char *arg1;
    char *arg2;
    char *result;
} IRCode;


typedef struct IRList {
    IRCode *codes;
    int size;
    int capacity;
} IRList;


void ir_init(IRList *list);
void ir_emit(IRList *list, IRInstr op, char *arg1, char *arg2, char *result);
void ir_print(IRList *list);
char* gen_code(Tree *node, IRList *list);

#endif