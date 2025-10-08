/* =========================================================================
 * Descripción:
 *   Este archivo contiene la gramática del compilador. Implementa el análisis sintáctico, verificando
 *   que el código fuente cumpla con las reglas definidas en la especificación.
 *
 * Funcionalidad:
 *   - Define la unión de tipos semánticos (números, identificadores, etc.).
 *   - Declara los tokens del lenguaje (palabras reservadas, operadores, literales).
 *   - Establece la precedencia y asociatividad de operadores.
 *   - Implementa la gramática para:
 *       * Programa principal
 *       * Declaraciones de variables y métodos
 *       * Bloques, sentencias y expresiones
 *       * Llamadas a métodos
 *
 * Mensajes de error:
 *   Si se detecta un error sintáctico, se reporta con la línea correspondiente.
 *
 * Integración:
 *   Este parser trabaja junto con el analizador léxico definido en Flex,
 *   que provee los tokens mediante la función yylex().
 * ======================================================================== */
%{
#include <stdio.h>
#include <stdlib.h>
#include "Stack.h"
#include "Tree.h"
#include "SymbolTable.h"
#include "Error.h"

extern FILE *yyin;
extern int yylineno;

ScopeStack scope_stack;
Tree *ast_root;

int had_error = 0;

int yylex(void);

%}

%union {
    int num;               /* para INT */
    char* id;             /* para ID */
    struct Tree* node;   /* para expresiones */
    struct Symbol* sym; /* para la tabla de simbolos*/
}
%define parse.trace



%token PROGRAM EXTERN RETURN
%token IF THEN ELSE WHILE  
%token T_INT T_BOOL T_VOID  
%token EQ NEQ LE GE AND OR
%token UNKNOW
%token <num> INT
%token <num> TRUE FALSE
%token <id> ID 


%left OR
%left AND
%nonassoc EQ NEQ '<' '>' LE GE
%left '+' '-'
%left '*' '/' '%'
%right UMINUS '!'   /* operadores unarios */

%type <node> program code var_decl method_decl params all_types block block_decl block_statement statement method_call args expr block_item
%%
program : PROGRAM '{' code '}'  {
                                    ast_root = $3;
                                }
        ;

code: var_decl code { $$ = createNode(NODE_CODE, NULL,$1, $2); }
    | method_decl code { $$ = createNode(NODE_CODE, NULL, $1, $2); }
    | /* vacío */ { $$ = NULL; }
    ;

var_decl: all_types ID '=' expr ';' {
            if ($1->tipo == NODE_T_VOID) {
                yyerrorf(yylineno,"No se puede declarar variable '%s' de tipo void");
                $$ = NULL;  // opcional: seguir parseando
            } else {
                Valores v = {0};
                Symbol *s = createSymbol($2,$1,VAR,v);
                $$ = createNode(NODE_DECLARATION, s, $1, $4);
            }
        }
        | all_types ID ';' { 
            if ($1->tipo == NODE_T_VOID) {
                yyerrorf(yylineno,"No se puede declarar variable '%s' de tipo void",$2);
                $$ = NULL;
            } else {
                Valores v = {0};
                Symbol *s = createSymbol($2,$1,VAR,v);
                $$ = createNode(NODE_DECLARATION, s, $1, NULL);
            }
        }
        ;

method_decl : all_types ID '(' params ')' block { 
                Valores v = {0};
                Symbol *s = createSymbol($2,$1,FUNC,v);
                Tree *methodInfo;
                methodInfo = createNode(NODE_METHOD_HEADER, 0, createNode(NODE_ID, s, $1, NULL), createNode(NODE_ARGS, 0, $4, NULL));
                $$ = createNode(NODE_METHOD, s, methodInfo, $6);
                s->node =$$;
            }
            | all_types ID '(' params ')' EXTERN ';' {
                Valores v = {0};
                Symbol *s = createSymbol($2,$1,FUNC,v);
                Tree *methodInfo = createNode(NODE_METHOD_HEADER, 0, createNode(NODE_ID, s, $1, NULL), createNode(NODE_ARGS, 0, $4, NULL));
                $$ = createNode(NODE_METHOD, s, methodInfo, NULL);
                s->node = $$;
            }
            ;


params  : all_types ID ',' params {
            if ($1->tipo == NODE_T_VOID) {
                yyerrorf(yylineno,"No se puede declarar parámetro '%s' de tipo void", $2);
                $$ = $4;  // ignoramos este parámetro y seguimos
            } else {
                Valores v = {0};
                Symbol *s = createSymbol($2,$1,VAR,v);
                $$ = createNode(NODE_LIST, 0, createNode(NODE_DECLARATION, s, $1, NULL), $4);
            }
        }
        | all_types ID  { 
            if ($1->tipo == NODE_T_VOID) {
                yyerrorf(yylineno,"No se puede declarar parámetro '%s' de tipo void", $2);
                $$ = NULL;
            } else {
                Valores v = {0};
                Symbol *s = createSymbol($2,$1,VAR,v);
                Tree *decl = createNode(NODE_DECLARATION, s, $1, NULL);
                $$ = createNode(NODE_LIST, 0, decl, NULL);
            }
        }
        | /* vacío */ { $$ = NULL; }
        ;


all_types   : T_INT { $$ = createNode(NODE_T_INT, 0, NULL, NULL); }
            | T_BOOL { $$ = createNode(NODE_T_BOOL, 0, NULL, NULL); }
            | T_VOID { $$ = createNode(NODE_T_VOID, 0, NULL, NULL); }
            ;

block   : '{' block_decl block_statement '}' {
            $$ = createNode(NODE_BLOCK, 0, $2, $3);
        }
        ;

block_decl  : var_decl block_decl { $$ = createNode(NODE_LIST, 0, $1, $2); }
                | /* vacío */ { $$ = NULL; }
            ;

block_statement : block_item block_statement { $$ = createNode(NODE_LIST, 0, $1, $2); }
                | /* vacío */ { $$ = NULL; }
                ;
    
block_item  : statement
            | var_decl
            ;    

    /*sentencias*/
statement   : ID '=' expr ';' {
                Symbol *s = createSymbolCall($1,VAR);
                $$ = createNode(NODE_ASSIGN, s, $3, NULL);
            }
            | method_call ';' { $$ = $1; }
            | IF '(' expr ')' THEN block {$$ = createNode(NODE_IF, 0, $3, $6);}
            | IF '(' expr ')' THEN block ELSE block { $$ = createNode(NODE_IF_ELSE, 0, $3, createNode(NODE_LIST, 0, $6, $8)); }
            | WHILE expr block { $$ = createNode(NODE_WHILE, 0, $2, $3); }
            | RETURN ';' { $$ = createNode(NODE_RETURN, 0, NULL, NULL); }
            | RETURN expr ';' { $$ = createNode(NODE_RETURN, 0, $2, NULL); }
            | block { $$ = $1; }
            | ';' { $$ = NULL; }
        ;

method_call : ID '(' args ')' { 
                Symbol *s = createSymbolCall($1,FUNC);
                // Creamos el nodo del identificador
                Tree *idNode = createNode(NODE_ID, s, NULL, NULL);

                // Creamos el nodo de argumentos (puede ser NULL si no hay)
                Tree *argsNode = createNode(NODE_ARGS, 0, $3, NULL);

                // Nodo final del método
                $$ = createNode(NODE_METHOD_CALL, s, idNode, argsNode);
            }
            ;
    
args: expr ',' args { $$ = createNode(NODE_LIST, 0, $1, $3); }
    | expr { $$ = createNode(NODE_LIST, 0, $1, NULL); }
    | /* vacío */ { $$ = NULL; }
    ;

expr: ID {
        Symbol *s = createSymbolCall($1,VAR);
        $$ = createNode(NODE_ID, s, NULL, NULL);
    }
    | method_call {$$ = $1;}
    | INT {
        Symbol *s = malloc(sizeof(Symbol));
        if (!s) { fprintf(stderr, "Error: sin memoria\n"); exit(1); }
        s->valor.value = $1;
        s->type = TYPE_INT;
        $$ = createNode(NODE_INT, s, NULL, NULL);
    }
    | TRUE {
        Symbol *s = malloc(sizeof(Symbol));
        s->valor.value = 1;
        s->type = TYPE_BOOL;
        $$ = createNode(NODE_TRUE, s, NULL, NULL);
    }
    | FALSE {
        Symbol *s = malloc(sizeof(Symbol));
        s->valor.value = 0;
        s->type = TYPE_BOOL;
        $$ = createNode(NODE_FALSE, s, NULL, NULL);
    }
    | expr '+' expr { $$ = createNode(NODE_SUM,0,$1,$3); }
    | expr '-' expr { $$ = createNode(NODE_RES,0,$1,$3); }
    | expr '*' expr { $$ = createNode(NODE_MUL,0,$1,$3); }
    | expr '/' expr { $$ = createNode(NODE_DIV,0,$1,$3); }
    | expr '%' expr { $$ = createNode(NODE_MOD,0,$1,$3); }
    | expr EQ expr  { $$ = createNode(NODE_EQ,0,$1,$3); }
    | expr NEQ expr { $$ = createNode(NODE_NEQ,0,$1,$3); }
    | expr '<' expr { $$ = createNode(NODE_LT,0,$1,$3); }
    | expr '>' expr { $$ = createNode(NODE_GT,0,$1,$3); }
    | expr LE expr  { $$ = createNode(NODE_LE,0,$1,$3); }
    | expr GE expr  { $$ = createNode(NODE_GE,0,$1,$3); }
    | expr AND expr { $$ = createNode(NODE_AND,0,$1,$3); }
    | expr OR expr  { $$ = createNode(NODE_OR,0,$1,$3); }
    | '-' expr %prec UMINUS { $$ = createNode(NODE_UMINUS, 0, $2, NULL); } 
    | '!' expr %prec UMINUS { $$ = createNode(NODE_NOT,0,$2,NULL); }
    | '(' expr ')' { $$ = createNode(NODE_PARENS,0,$2,NULL); }
    ;

%%  
