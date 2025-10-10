#ifndef ARGS_H
#define ARGS_H

#include "utils.h"

typedef struct {
    char *input_file;
    char *output_file;
    char *target;
    char *optimization;
    bool debug;
} Config;

bool parse_args(int argc, char **argv, Config *cfg);
FILE *open_input(const char *path);
FILE *open_output(const char *path);

#endif
