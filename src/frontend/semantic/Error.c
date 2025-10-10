// main.c
#include <stdio.h>
#include <stdarg.h>
#include "Error.h"

int semantic_error = 0;   // declaramos la variable global
extern int yylineno;       // de Bison
extern char *yytext;       // de Flex

void yyerrorf(int lineno, const char *fmt, ...) {
    char msg[512];
    va_list args;
    va_start(args, fmt);
    vsnprintf(msg, sizeof(msg), fmt, args);
    va_end(args);

    fprintf(stderr, "-> ERROR en la línea %d: %s\n", lineno, msg);
    semantic_error = 1;
}


void yyerror(const char *msg) {
    fprintf(stderr, "-> ERROR sintáctico en la línea %d: %s. Token actual: '%s'\n",
            yylineno,
            msg,
            yytext ? yytext : "<NULL>");
}
