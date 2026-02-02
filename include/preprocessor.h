/* Aviv Hadar, ID: 207750993
 * This program is the preprocessor header. */

#ifndef MAMAN14_PROJ_PREPROCESSOR_H
#define MAMAN14_PROJ_PREPROCESSOR_H

#include "globals.h"
#include "tables.h"
#include "parser.h"
#define AS_END ".as" /*the ending of as file*/
#define AM_END ".am" /*the ending of am file*/
#define MCR 3 /*the length of mcr name*/

/* the struct of macro node */
typedef struct macro {
    char *name;
    char *data;
    struct macro *next;
} macro;

char *preprocess_file(char *file_name, symbol_table **symbols);
int is_blank(char *str);

#endif
