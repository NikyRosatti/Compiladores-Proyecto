#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <stdbool.h>
#include <getopt.h>

// === Includes del compilador ===
#include "../build/bison.tab.h"
#include "Stack.h"
#include "Tree.h"
#include "SymbolTable.h"
#include "Intermediate.h"
#include "Assembler.h"
#include "Error.h"

// ==== Globals ====
extern FILE *yyin;
extern char *yytext;
extern int yylex(void);
extern int yyparse(void);
extern int yydebug;
extern int main_decl;
extern ScopeStack scope_Stack;
extern Tree *ast_root;
extern int had_error;
extern int semantic_error;

// ==== Macros ====
#define PRINT_TOKEN(tok) do { \
    const char *_name = NULL; \
    switch(tok) { \
        case PROGRAM: _name="PROGRAM"; break; \
        case EXTERN:  _name="EXTERN"; break; \
        case T_INT:   _name="T_INT"; break; \
        case T_BOOL:  _name="T_BOOL"; break; \
        case T_VOID:  _name="T_VOID"; break; \
        case RETURN:  _name="RETURN"; break; \
        case IF:      _name="IF"; break; \
        case THEN:    _name="THEN"; break; \
        case ELSE:    _name="ELSE"; break; \
        case WHILE:   _name="WHILE"; break; \
        case TRUE:    _name="TRUE"; break; \
        case FALSE:   _name="FALSE"; break; \
        case ID:      _name="ID"; break; \
        case INT:     _name="INT"; break; \
        case OR:      _name="OR"; break; \
        case AND:     _name="AND"; break; \
        case EQ:      _name="EQ"; break; \
        case NEQ:     _name="NEQ"; break; \
        case LE:      _name="LE"; break; \
        case GE:      _name="GE"; break; \
        case UNKNOW:  _name="UNKNOW"; break; \
        default:      _name="SIMBOLO"; break; \
    } \
    fprintf(f, "TOKEN: %s : '%s'\n", _name, yytext); \
} while(0)

void print_usage(void);

#endif
