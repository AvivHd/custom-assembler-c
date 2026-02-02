/* Aviv Hadar, ID: 207750993
 * This program is the firstpass header. */

#ifndef MAMAN14_PROJ_OUTPUT_H
#define MAMAN14_PROJ_OUTPUT_H

#include "parser.h"
#include "encode.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define EXT_FILE_END ".ext" /* externals file ending */
#define ENT_FILE_END ".ent" /*entries file ending */
#define OB_FILE_END ".ob" /*object file ending */

void write_files(char *name, code_table **codes, int ent, int ext, symbol_table **symbols, long ic, long dc);

#endif
