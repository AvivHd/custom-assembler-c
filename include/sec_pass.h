/* Aviv Hadar, ID: 207750993
 * This program is the second pass header. */

#ifndef MAMAN14_PROJ_SEC_PASS_H
#define MAMAN14_PROJ_SEC_PASS_H

#include "globals.h"
#include "parser.h"
#include "encode.h"
#include "firstpass.h"

int second_pass(char *line, ast *tree, symbol_table **symbols, code_table **codes, long *ic, long *dc, long final_ic);

#endif
