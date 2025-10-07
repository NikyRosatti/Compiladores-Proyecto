#include "args.h"

void print_usage() {
    printf("Uso: c-tds [opcion] archivo.ctds\n");
    printf("Opciones:\n");
    printf("  -o <salida>       Renombra el archivo de salida\n");
    printf("  -target <etapa>   Etapa: scan | parse | codinter | assembly\n");
    printf("  -opt [opt]        Realiza optimizaciones (all para todas)\n");
    printf("  -debug            Activa modo debug\n");
}

bool parse_args(int argc, char **argv, Config *cfg) {
    int opt;
    cfg->output_file = NULL;
    cfg->target = "parse";
    cfg->optimization = NULL;
    cfg->debug = false;

    static struct option long_options[] = {
        {"debug",   no_argument,       0, 'd'},
        {"target",  required_argument, 0, 't'},
        {"opt",     required_argument, 0, 'p'},
        {"o",       required_argument, 0, 'o'},
        {0, 0, 0, 0}
    };

    while ((opt = getopt_long(argc, argv, "do:o:t:p:", long_options, NULL)) != -1) {
        switch (opt) {
            case 'd': cfg->debug = true; break;
            case 'o': cfg->output_file = optarg; break;
            case 't': cfg->target = optarg; break;
            case 'p': cfg->optimization = optarg; break;
            default: print_usage(); return false;
        }
    }

    if (optind >= argc) {
        fprintf(stderr, "Error: falta el archivo de entrada\n");
        print_usage();
        return false;
    }

    cfg->input_file = argv[optind];

    char *ext = strrchr(cfg->input_file, '.');
    if (!ext || strcasecmp(ext, ".ctds") != 0) {
        fprintf(stderr, "Error: el archivo debe tener extensión .ctds\n");
        return false;
    }

    if (!cfg->output_file) cfg->output_file = "a.out";
    return true;
}

FILE *open_input(const char *path) {
    FILE *file = fopen(path, "r");
    if (!file) perror("Error al abrir el archivo de entrada");
    return file;
}

FILE *open_output(const char *path) {
    FILE *file = fopen(path, "w");
    if (!file) perror("Error al crear archivo de salida");
    return file;
}
