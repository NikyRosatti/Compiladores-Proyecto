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

extern FILE *yyin;
extern int yylineno;
extern int semantic_error;

ScopeStack scope_stack;
Tree *ast_root;

int had_error = 0;

int yylex(void);


void yyerror(const char *s) {
    extern int yylineno;   
    printf("-> ERROR Sintáctico en la línea %d: %s\n", yylineno, s);
    had_error = 1;
}

%}

%union {
    int num;               /* para INT */
    char* id;             /* para ID */
    struct Tree* node;   /* para expresiones */
    struct Symbol* sym; /* para la tabla de simbolos*/
}

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

%type <node> program code var_decl method_decl params all_types block block_decl block_statement statement method_call args expr
%%
program : PROGRAM '{' code '}'  {
                                    ast_root = $3;
                                }
        ;

code : var_decl code { $$ = createNode(NODE_CODE, NULL,$1, $2); }
    | method_decl code { $$ = createNode(NODE_CODE, NULL, $1, $2); }
    | /* vacío */ { $$ = NULL; }
    ;

var_decl : all_types ID '=' expr ';' {
                SymbolTable *current = peekScope(&scope_stack);
                Symbol *s = lookupSymbol(current, $2);
                if (s) {
                    fprintf(stderr, "Error: variable '%s' ya declarada\n", $2);
                }
                // Declaración + inicialización
                SymbolType t;
                if ($1->tipo == NODE_T_INT) t = TYPE_INT;
                else if ($1->tipo == NODE_T_BOOL) t = TYPE_BOOL;
                else t = TYPE_VOID;

                Valores v = {0};
                Symbol *aux = insertSymbol(current, $2, t, v);
                $$ = createNode(NODE_DECLARATION, aux, $1, $4);
            }
        | all_types ID ';' { 
            SymbolTable *current = peekScope(&scope_stack);

            Symbol *s = lookupSymbol(current, $2);
            if (s) {
                fprintf(stderr, "Error: variable '%s' ya declarada en este scope\n", $2);
            }

            SymbolType t;
            if ($1->tipo == NODE_T_INT) t = TYPE_INT;
            else if ($1->tipo == NODE_T_BOOL) t = TYPE_BOOL;
            else t = TYPE_VOID;

            Valores v = {0};
            Symbol *aux = insertSymbol(current, $2, t, v);

            $$ = createNode(NODE_DECLARATION, aux, $1, NULL);
        }
        ;


method_decl : all_types ID '(' params ')' block { 
                    SymbolTable *global = peekScope(&scope_stack);
                    Symbol *s = lookupSymbol(global, $2);
                    if (s) {
                        fprintf(stderr, "Error: Metodo '%s' ya declarada\n", $2);
                    } else {
                        Valores v = {0};
                        SymbolType t;
                        if ($1->tipo == NODE_T_INT) t = TYPE_INT;
                        else if ($1->tipo == NODE_T_BOOL) t = TYPE_BOOL;
                        else t = TYPE_VOID;
                        s = insertSymbol(global, $2, t, v);
                        
                        SymbolTable *method_scope = createTable();
                        pushScope(&scope_stack, method_scope);
                        
                        Tree *param_node = $4;

                        Tree *methodInfo = createNode(NODE_METHOD_HEADER, 0, createNode(NODE_ID, s, $1, NULL), param_node);
                        s->node = methodInfo;
                        $$ = createNode(NODE_METHOD, s, methodInfo, $6);

                        popScope(&scope_stack);
                    }
                }
            | all_types ID '(' params ')' EXTERN ';' {
                SymbolTable *current = peekScope(&scope_stack);
                Symbol *s = lookupSymbol(current, $2);
                if (s) {
                    fprintf(stderr, "Error: Metodo '%s' ya declarada\n", $2);
                } else {
                    Valores v = {0};
                    SymbolType t;
                    if ($1->tipo == NODE_T_INT) t = TYPE_INT;
                    else if ($1->tipo == NODE_T_BOOL) t = TYPE_BOOL;
                    else t = TYPE_VOID;
                    s = insertSymbol(current, $2, t, v);
                    
                    Tree *param_node = $4;

                    Tree *methodInfo = createNode(NODE_METHOD_HEADER, 0, createNode(NODE_ID, s, $1, NULL), param_node);
                    s->node = methodInfo;
                    $$ = createNode(NODE_METHOD, s, methodInfo, NULL);

                }
            }
            ; 


params : all_types ID ',' params {
                SymbolTable *current = peekScope(&scope_stack);
                Symbol *s = lookupSymbol(current, $2);
                if (s) {
                    fprintf(stderr, "Error: parámetro '%s' ya declarado en este scope\n", $2);
                }

                SymbolType t;
                if ($1->tipo == NODE_T_INT) t = TYPE_INT;
                else if ($1->tipo == NODE_T_BOOL) t = TYPE_BOOL;
                else t = TYPE_VOID;

                Valores v = {0};
                Symbol *aux = insertSymbol(current, $2, t, v);

                $$ = createNode(NODE_LIST, 0, createNode(NODE_DECLARATION, aux, $1, NULL), $4);
            }
        | all_types ID  { 
                SymbolTable *current = peekScope(&scope_stack);
                Symbol *s = lookupSymbol(current, $2);
                if (s) {
                    fprintf(stderr, "Error: parámetro '%s' ya declarado en este scope\n", $2);
                }

                SymbolType t;
                if ($1->tipo == NODE_T_INT) t = TYPE_INT;
                else if ($1->tipo == NODE_T_BOOL) t = TYPE_BOOL;
                else t = TYPE_VOID;

                Valores v = {0};
                Symbol *aux = insertSymbol(current, $2, t, v);

                $$ = createNode(NODE_DECLARATION, aux, $1, NULL); 
            }
        | /* vacío */ { $$ = NULL; }
        ;

all_types : T_INT { $$ = createNode(NODE_T_INT, 0, NULL, NULL); }
        | T_BOOL { $$ = createNode(NODE_T_BOOL, 0, NULL, NULL); }
        | T_VOID { $$ = createNode(NODE_T_VOID, 0, NULL, NULL); }
    ;

block : '{' block_decl block_statement '}' {
        pushScope(&scope_stack, createTable());

        $$ = createNode(NODE_BLOCK, 0, $2, $3);

        popScope(&scope_stack);
    }
    ;

block_decl : var_decl block_decl { $$ = createNode(NODE_LIST, 0, $1, $2); }
           | /* vacío */ { $$ = NULL; }
    ;

block_statement : statement block_statement { $$ = createNode(NODE_LIST, 0, $1, $2); }
                | /* vacío */ { $$ = NULL; }
                ;
    
    /*sentencias*/
statement : ID '=' expr ';' {
                Symbol *s = lookupInScopes(&scope_stack, $1);
                if (!s) {
                    fprintf(stderr, "Error: variable '%s' no declarada\n", $1);
                }
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
        Symbol *s = lookupInScopes(&scope_stack, $1);
        // Creamos el nodo del identificador
        Tree *idNode = createNode(NODE_ID, s, NULL, NULL);

        // Creamos el nodo de argumentos (puede ser NULL si no hay)
        Tree *argsNode = createNode(NODE_ARGS, 0, $3, NULL);

        // Nodo final del método
        $$ = createNode(NODE_METHOD_CALL, s, idNode, argsNode);
    }
            ;
    
args : expr ',' args { $$ = createNode(NODE_LIST, 0, $1, $3); }
    | expr { $$ = createNode(NODE_LIST, 0, $1, NULL); }
    | /* vacío */ { $$ = NULL; }
    ;

expr : ID {
            Symbol *s = lookupInScopes(&scope_stack, $1);
            if (!s) {
                fprintf(stderr, "Error: variable '%s' no declarada\n", $1);
            }
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
