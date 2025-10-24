#include "Stages.h"

int main(int argc, char **argv) {
    Config cfg;
    if (!parse_args(argc, argv, &cfg)) return 1;

    yyin = open_input(cfg.input_file);
    if (!yyin) return 1;

    FILE *f = open_output(cfg.output_file);
    if (!f) {
        fclose(yyin);
        return 1;
    }

    if (cfg.debug) {
        printf("[DEBUG] Archivo de entrada: %s\n", cfg.input_file);
        printf("[DEBUG] Archivo de salida: %s\n", cfg.output_file);
        printf("[DEBUG] Target: %s\n", cfg.target);
        if (cfg.optimization) printf("[DEBUG] Optimizacion: %s\n", cfg.optimization);
    }

    initScopeStack(&scope_Stack);
    pushScope(&scope_Stack, createTable());

    int result = 0;

    if (strcasecmp(cfg.target, "scan") == 0)
        result = run_scan_stage(f, cfg.debug);
    else if (strcasecmp(cfg.target, "parse") == 0)
        result = run_parse_stage(&cfg);
    else if (strcasecmp(cfg.target, "codinter") == 0) {
        if ((result = run_parse_stage(&cfg)) == 0)
            result = run_codinter_stage();
    } else if (strcasecmp(cfg.target, "assembly") == 0) {
        if ((result = run_parse_stage(&cfg)) == 0)
            result = run_assembly_stage(f, cfg.debug);
    } else {
        fprintf(stderr, "Target desconocido: %s\n", cfg.target);
        result = 1;
    }

    fclose(f);
    fclose(yyin);

    if (cfg.debug) printf("[DEBUG] Finalizado con código %d\n", result);
    return result;
}
