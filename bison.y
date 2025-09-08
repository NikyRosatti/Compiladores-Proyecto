%{
#include <stdio.h>
#include <stdlib.h>

int yylex(void);

void yyerror(const char *s) {
    extern int yylineno;   
    printf("-> ERROR Sintáctico en la línea %d: %s\n", yylineno, s);
}

%}

%union {
    int num;               /* para INT */
    char* id;             /* para ID */
    /* Agregar lo demas */
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
%left EQ NEQ '<' '>' LE GE
%left '+' '-'
%left '*' '/'
%right '!'


%%
program : PROGRAM '{' code '}'  { printf("Programa válido ✔️\n"); }
        ;

code : var_decl code { printf("ENTREEEE \n"); }
     | method_decl code
     | /* vacío */ { }
     ;

var_decl : tipo  ID '=' expr ';' 
        ;


method_decl : all_types ID '(' params ')' block
            | all_types ID '(' params ')' EXTERN ';' {}
            ; 

all_types : T_INT
          | T_BOOL
          | T_VOID
          ;

params : tipo ID ',' params 
        | tipo ID 
        | /* vacío */ { }
        ;


block : '{' block_decl block_statement '}' 
      ;

block_decl : var_decl block_decl 
           | /* vacío */ { }
           ;

block_statement : statement block_statement
                | /* vacío */ { }
                ;

tipo : T_INT
     | T_BOOL
        ;
    
    /*sentencias*/
statement : ID '=' expr ';'
            | method_call ';'
            | IF '(' expr ')' THEN block
            | IF '(' expr ')' THEN block ELSE block
            | WHILE expr block
            | RETURN ';'
            | block
            | ';'
            ;

method_call : ID '(' args ')'
            ;
    
args : expr ',' args
     | expr
     | /* vacío */ { }
     ;

expr : ID
     | method_call
     | INT
     | TRUE
     | FALSE
     | expr bin_op expr
     |  '-' expr 
     | '!' expr
     | '(' expr ')'
     ;


    /*Operadores*/
bin_op : arith_op 
        | rel_op 
        | cond_op
        ;

arith_op : '+' 
        | '-' 
        | '*' 
        | '/' 
        | '%'
        ;


rel_op : '<' 
       | '>' 
       | EQ
       | NEQ
       | LE
       | GE
       ;

cond_op : AND
        | OR
        ;


%%

int main(int argc, char **argv) {
    if (yyparse() == 0) {
        printf("Parseo terminado correctamente ✅\n");
    }
    return 0;
}        












        










        
