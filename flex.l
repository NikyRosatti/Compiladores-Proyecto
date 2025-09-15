%{
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "bison.tab.h"
%}

%option noyywrap noinput nounput
%option yylineno

    /* ======== EXPRESIONES REGULARES ======== */
ID          [a-zA-Z][a-zA-Z0-9_]*
NUMERO [0-9]+

    /* ======== PATRONES Y ACCIONES ========== */
%%
"Program"              {  return PROGRAM;    }
"extern"              {   return EXTERN;    }
"integer"               {  return T_INT;    }
"bool"              {   return T_BOOL;    }
"void"              {   return T_VOID;    }
"return"            {   return RETURN;    }
"if"                {   return IF;    }
"then"              {   return THEN;    }
"else"              {   return ELSE;    }
"while"             {   return WHILE;    }
"true"              { yylval.num = 1; return TRUE; }
"false"             { yylval.num = 0; return FALSE; }
    
    /* Números */
{NUMERO}            {
                    yylval.num = atoi(yytext);
                    return INT; }

    /* Identificadores */
{ID}                {
                    yylval.id = strdup(yytext);
                    return ID; }

    /* Operadores de un solo carácter y delimitadores */
[-+*;(){}=!,/<>%]         { return *yytext;    }

    /* Operadores de dos caracteres */
"||"          {   return OR;    }
"&&"          {   return AND;   }
"=="          {   return EQ;    }
"!="          {   return NEQ;   }
"<="          {   return LE;   }
">="          {   return GE;   }

    /* Ignorar espacios y tabs */
[ \t\r]+            { /* ignorar */ }

    /* Ignorar comentarios estilo C */
"//".*              { /* ignorar */ }
"/*"([^*]|\*+[^*/])*\*+"/" { /* ignorar */ }

    /* Saltos de línea (opcional si quieres contar líneas) */
\n                  { /* ignorar o contar */ }

.                   { printf("Caracter desconocido: %s\n", yytext); }
%%
