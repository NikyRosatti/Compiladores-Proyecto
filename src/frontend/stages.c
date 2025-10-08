#include "stages.h"

int run_scan_stage(FILE *f, bool debug) {
    int tok;
    int lexico_valido = 1;

    while ((tok = yylex()) != 0) {
        if (tok == UNKNOW) {
            fprintf(stderr, "Error léxico: '%s'\n", yytext);
            lexico_valido = 0;
            break;
        }
        if (debug) PRINT_TOKEN(tok);
    }

    printf(lexico_valido ? "Léxico válido ✔️\n" : "Léxico inválido ⚠️\n");
    return lexico_valido ? 0 : 1;
}

int run_parse_stage(Config *cfg, FILE *f) {
    if (cfg->debug) yydebug = 1;

    if (yyparse() != 0) {
        fprintf(stderr, "Error en el parseo ❌\n");
        return 1;
    }

    if (had_error || !ast_root) {
        fprintf(stderr, "Se detectaron errores. No se ejecutará el AST.\n");
        return 1;
    }

    if (strcasecmp(cfg->target, "parse") == 0) {
        printf("Árbol antes de ejecutar asignaciones:\n");
        printTree(ast_root, 0);
    }

    check_scopes(ast_root);
    check_types(ast_root);

    if (main_decl == 0) {
        fprintf(stderr, "Error semántico: no se encontró definido el método main\n");
        return 2;
    }
    if (semantic_error) {
        fprintf(stderr, "Error semántico\n");
        return 2;
    }

    return 0;
}

int run_codinter_stage(void) {
    IRList list;
    ir_init(&list);
    gen_code(ast_root, &list);
    ir_print(&list);
    return 0;
}

int run_assembly_stage(FILE *f, bool debug) {
    if (debug) printf("[DEBUG] Calculando offsets...\n");
    calculate_offsets(ast_root);
    if (debug) printf("[DEBUG] Offsets calculados correctamente\n");

    IRList list;
    ir_init(&list);
    gen_code(ast_root, &list);

    if (debug) printf("[DEBUG] Generando código assembly...\n");
    generateAssembly(&list, f);

    printf("Código assembly generado correctamente ✔️\n");
    return 0;
}
