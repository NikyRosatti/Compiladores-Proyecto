#ifndef STAGES_H
#define STAGES_H

#include "args.h"

int run_scan_stage(FILE *f, bool debug);
int run_parse_stage(Config *cfg, FILE *f);
int run_codinter_stage(void);
int run_assembly_stage(FILE *f, bool debug);

#endif
