// Error.h
#ifndef ERROR_H
#define ERROR_H

#include <stdarg.h>

extern int semantic_error;

extern int yylineno;         // declarada en Bison
extern char *yytext;         // declarada en Flex

void yyerrorf(int lineno, const char *fmt, ...);
void yyerror(const char *s);
#endif
