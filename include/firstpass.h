/* Aviv Hadar, ID: 207750993
 * This program is the firstpass header. */

#ifndef MAMAN14_PROJ_FIRSTPASS_H
#define MAMAN14_PROJ_FIRSTPASS_H

#include "globals.h"
#include "tables.h"
#include "parser.h"
#include "encode.h"

int first_pass(char *line, ast *tree, symbol_table *symbols, long *ic, long *dc, code_table **codes);

int check_data(char *line, ast *tree, symbol_table *symbols);

#endif
