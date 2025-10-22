#ifndef STAGES_H
#define STAGES_H

#include "Args.h"
#include "Intermediate.h"

int run_scan_stage(FILE *f, bool debug);
int run_parse_stage(Config *cfg);
int run_codinter_stage(void);
int run_assembly_stage(FILE *f, bool debug);
void offset_temps(IRList *list);

#endif
