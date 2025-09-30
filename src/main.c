#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <stdbool.h>
#include <getopt.h>
#include "../build/bison.tab.h"
#include "Stack.h"
#include "Tree.h"
#include "SymbolTable.h"
#include "Error.h"
// Macro para mostrar token
#define PRINT_TOKEN(tok) do {       \
    const char *_name = NULL;       \
    switch(tok) {                   \
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
        case UNKNOW:  _name="UNKNOW"; break;\
        default:      _name="SIMBOLO"; break; \
    }                               \
    fprintf(f, "TOKEN: %s : '%s'\n", _name, yytext); \
} while(0)


int yyparse(void);
extern FILE *yyin;
extern char *yytext;
extern int yylex(void);
int semantic_error = 0;   // variable global
extern int main_decl;
extern ScopeStack scope_Stack;
extern Tree *ast_root;
extern int had_error;
extern int yydebug;

void print_usage() {
    printf("Uso: c-tds [opcion] archivo.ctds\n");
    printf("Opciones:\n");
    printf("  -o <salida>       Renombra el archivo de salida\n");
    printf("  -target <etapa>   Etapa: scan | parse | codinter | assembly\n");
    printf("  -opt [opt]        Realiza optimizaciones (all para todas)\n");
    printf("  -debug            Activa modo debug\n");
}

int main(int argc, char **argv) {
    int opt;
    char *output_file = NULL;
    char *target = "parse";
    char *optimization = NULL;
    bool debug = false;
    initScopeStack(&scope_Stack);
    pushScope(&scope_Stack, createTable());
    
    static struct option long_options[] = {
        {"debug",   no_argument,       0, 'd'},
        {"target",  required_argument, 0, 't'},
        {"opt",     required_argument, 0, 'p'},
        {"o",       required_argument, 0, 'o'},
        {0, 0, 0, 0}
    };

    while ((opt = getopt_long(argc, argv, "do:o:t:p:", long_options, NULL)) != -1) {
        switch (opt) {
            case 'd': debug = true; break;
            case 'o': output_file = optarg; break;
            case 't': target = optarg; break;
            case 'p': optimization = optarg; break;
            default: print_usage(); return 1;
        }
    }

    if (optind >= argc) {
        fprintf(stderr, "Error: falta el archivo de entrada\n");
        print_usage();
        return 1;
    }
    char *input_file = argv[optind];

    char *ext = strrchr(input_file, '.');
    if (!ext || strcasecmp(ext, ".ctds") != 0) {
        fprintf(stderr, "Error: el archivo debe tener extension .ctds\n");
        return 1;
    }

    yyin = fopen(input_file, "r");
    if (!yyin) {
        perror("Error al abrir el archivo de entrada");
        return 1;
    }

    if (!output_file) output_file = "a.out";

    if (debug) {
        printf("[DEBUG] Archivo de entrada: %s\n", input_file);
        printf("[DEBUG] Archivo de salida: %s\n", output_file);
        if (target) printf("[DEBUG] Target: %s\n", target);
        if (optimization) printf("[DEBUG] Optimizacion: %s\n", optimization);
    }

    // Abrir archivo de salida
    FILE *f = fopen(output_file, "w");
    if (!f) {
        perror("Error al crear archivo de salida");
        fclose(yyin);
        return 1;
    }

    // ===== Etapas =====
    if(target == NULL){
        printf("Error: debe indicar un target con -target <etapa>\n");
        fclose(f);
        fclose(yyin);
        return 1;
    }
    if (strcasecmp(target, "scan") == 0) {
        int tok;
        int lexico_valido = 1; // bandera para saber si todo es válido
        
        while ((tok = yylex()) != 0) {
            if (tok == UNKNOW) {   // Si hay un token desconocido
                fprintf(stderr, "Error léxico: '%s'\n", yytext);
                lexico_valido = 0;
                break;             // salir del while sin imprimir nada más
            }
            if (debug) PRINT_TOKEN(tok);  // Solo imprimir si todo sigue siendo válido
        }
        
        fclose(f);
        fclose(yyin);

        if (lexico_valido){
            printf("Léxico válido ✔️\n");
            return 0;
        } else {
            printf("Léxico inválido ⚠️\n");
            return 1;
        }
    }
    else if (strcasecmp(target, "parse") == 0) {
        if (debug) {
            yydebug = 1;
        }
        if (yyparse() == 0) {
            if (had_error || !ast_root) {
                fprintf(stderr, "Se detectaron errores. No se ejecutará el AST.\n");
                return 1;
            }
            printf("Árbol antes de ejecutar asignaciones:\n");
            printTree(ast_root, 0);
            fflush(stdout);
            // Chequeo semantico
            check_scopes(ast_root);
            check_types(ast_root);

            if (main_decl == 0) {
                printf("Error semántico: no se encontro definido el metodo main");
                return 2;
            }
        
            if (semantic_error) {
                fprintf(stderr, "Error semántico\n");
                return 2;
            } else {
                printf("\nSIN ERRORES SEMANTICOS\n");
            }

            

            printSymbolTable(peekScope(&scope_Stack));
        } else {
            fprintf(stderr, "Error en el parseo ❌\n");
            fclose(f);
            fclose(yyin);
            return 1;
        }
    }
    else if (strcasecmp(target, "codinter") == 0) {
        fprintf(f, "Código intermedio generado (simulado)\n");
        fprintf(f, "t1 = ...\n");
    }
    else if (strcasecmp(target, "assembly") == 0) {
        fprintf(f, "Código ensamblador generado (simulado)\n");
        fprintf(f, "mov eax, 1\n");
    } else {
        printf("Target desconocido: %s\n", target);
        return 1;
    }

    fclose(f);
    fclose(yyin);

    if (debug) printf("[DEBUG] Generando salida en %s\n", output_file);

    return 0;
}
