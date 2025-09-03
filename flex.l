%{
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
%}

%option noyywrap noinput nounput
%option yylineno

    /* ======== EXPRESIONES REGULARES ======== */
ALPHA       [a-zA-Z]
ID          [a-zA-Z][a-zA-Z0-9_]*
DIGIT      [0-9]
INTEGER_LITERAL [0-9]+
REL_OP [<>==]
ARIT_OP [-+*/%]
COND_OP [&&||]

    /* ======== PATRONES Y |ACCIONES ========== */
%%|
"Program"              {   return PROGRAM;    }
"extern"              {   return EXTERN;    }
"integer"               {   return T_INT;    }
"bool"              {   return T_BOOL;    }
"void"              {   return T_VOID;    }
"return"            {   return RETURN;    }
"if"                {   return IF;    }
"then"              {   return THEN;    }
"else"              {   return ELSE;    }
"while"             {   return WHILE;    }
"true"              {   return TRUE;    }
"false"             {   return FALSE;    }
    
    /* Números */
{DIGIT}            {
                    yylval.num = atoi(yytext);
                    return INT; }

    /* Identificadores */
{ID}                {
                    yylval.id = strdup(yytext);
                    return ID; }

    /* Operadores de un solo carácter y delimitadores */
[;(){},]         {   return *yytext;    }

    /* Operadores de dos caracteres */
{COND_OP}         {   return CONDITION_OP;    }
{REL_OP}          {   return RELATION_OP;      }
{ARIT_OP}         {   return ARITMETIC_OP;     }

    /* Ignorar espacios y tabs */
[ \t\r]+            { /* ignorar */ }

    /* Ignorar comentarios estilo C */
"//".*              { /* ignorar */ }
"/*"([^*]|\*+[^*/])*\*+"/" { /* ignorar */ }

    /* Saltos de línea (opcional si quieres contar líneas) */
\n                  { /* ignorar o contar */ }

.                   { printf("Caracter desconocido: %s\n", yytext); }
%%
