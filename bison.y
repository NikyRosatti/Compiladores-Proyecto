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
#include "Tree.h"
#include "SymbolTable.h"

extern FILE *yyin;
extern int yylineno;
extern int semantic_error;

SymbolTable *symtab;
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
                                    
                                    printf("Programa válido ✔️\n");
                                
                                    ast_root = $3;
                                    if (!had_error) {
                                        printf("Árbol AST generado correctamente.\n");
                                    }
                                }
        ;

code : var_decl code { $$ = new Tree(NODE_CODE, $1, $2); }
    | method_decl code { $$ = new Tree(NODE_METHOD, $1, $2); }
    | /* vacío */ { $$ = NULL; }
    ;

var_decl : all_types ID '=' expr ';' {
                Symbol *s = lookupSymbol(symtab, $2);
                if (s) {
                    fprintf(stderr, "Error: variable '%s' ya declarada\n", $2);
                }
                // Declaración + inicialización
                SymbolType t;
                if ($1->tipo == NODE_T_INT) t = TYPE_INT;
                else if ($1->tipo == NODE_T_BOOL) t = TYPE_BOOL;
                else t = TYPE_VOID;

                Valores v = {0};
                Symbol *aux = insertSymbol(symtab, $2, t, v);
                $$ = createNode(NODE_DECLARATION, aux, $1, $4);
            }
        | all_types ID ';' { 
            Symbol *s = lookupSymbol(symtab, $2);
            if (s) {
                fprintf(stderr, "Error: variable '%s' ya declarada\n", $2);
            }
            // Mapear el nodo de tipo T a SymbolType
            SymbolType t;
            if ($1->tipo == NODE_T_INT) t = TYPE_INT;
            else if ($1->tipo == NODE_T_BOOL) t = TYPE_BOOL;
            else t = TYPE_VOID;

            // Insertar en la tabla
            Valores v = {0}; 
            Symbol *aux = insertSymbol(symtab, $2, t, v);
            // Crear nodo AST con símbolo
            $$ = createNode(NODE_DECLARATION, aux, $1, NULL);
        }
        ;


method_decl : all_types ID '(' params ')' block { 
                Symbol *s = lookupSymbol(symtab, $2);
                if (s) {
                    fprintf(stderr, "Error: variable '%s' ya declarada\n", $2);
                }
                $$ = createNode(NODE_METHOD_DECL, 0, createLeaf(NODE_ID, $2), $5);
                // Aquí podrías manejar la inserción en la tabla de símbolos para métodos
            }
            | all_types ID '(' params ')' EXTERN ';' {}
            ; 


params : all_types ID ',' params 
        | all_types ID 
        | /* vacío */ { }
        ;

all_types : T_INT { $$ = createNode(NODE_T_INT, 0, NULL, NULL); }
        | T_BOOL { $$ = createNode(NODE_T_BOOL, 0, NULL, NULL); }
        | T_VOID { $$ = createNode(NODE_T_VOID, 0, NULL, NULL); }
    ;

block : '{' block_decl block_statement '}' 
    ;

block_decl : var_decl block_decl 
           | /* vacío */ { }
    ;

block_statement : statement block_statement
                | /* vacío */ { }
                ;
    
    /*sentencias*/
statement : ID '=' expr ';' {
                // Buscar símbolo (debe existir previamente en tabla)
                Symbol *s = lookupSymbol(symtab, $1);
                if (!s) {
                    fprintf(stderr, "Error: variable '%s' no declarada\n", $1);
                }

                // Nodo asignación, enlazado al símbolo
                $$ = createNode(NODE_ASSIGN, s, $3, NULL);
            }
        | method_call ';'
        | IF '(' expr ')' THEN block
        | IF '(' expr ')' THEN block ELSE block
        | WHILE expr block
        | RETURN ';'
        | RETURN expr ';'
        | block
        | ';'
    ;

method_call : ID '(' args ')' {
                $$ = createNode(NODE_METHOD_CALL, 0, createLeaf(NODE_ID, $1), $3);
    }
            ;
    
args : expr ',' args
    | expr
    | /* vacío */ { }
    ;

expr : ID {
            Symbol *s = lookupSymbol(symtab, $1);
            if (!s) {
                fprintf(stderr, "Error: variable '%s' no declarada\n", $1);
            }
            $$ = createNode(NODE_ID, s, NULL, NULL);
        }
    | method_call {
            $$ = createNode(NODE_METHOD_CALL, 0, NULL, $1);
        }
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
    | '-' expr %prec UMINUS
    | '!' expr %prec UMINUS { $$ = createNode(NODE_NOT,0,$2,NULL); }
    | '(' expr ')' { $$ = createNode(NODE_PARENS,0,$2,NULL); }
    ;


%%

int main(int argc, char **argv) {
    symtab = createTable();
    ++argv,--argc;
    if (argc > 0)
        yyin = fopen(argv[0],"r");
    else
        yyin = stdin;

    yyparse();

    if (had_error || !ast_root) {
        fprintf(stderr, "Se detectaron errores. No se ejecutará el AST.\n");
        return 1;
    }
    printf("Árbol antes de ejecutar asignaciones:\n");
    printTree(ast_root, 0);

    // Chequeo semantico
    check_types(ast_root);
    if (semantic_error) {
        fprintf(stderr, "Error semántico\n");
        return 2;
    } else {
        printf("\nSIN ERRORES SEMANTICOS\n");
    }

    printSymbolTable(symtab);
    return 0;
}        












        










        
