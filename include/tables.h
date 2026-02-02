/* Aviv Hadar, ID: 207750993
 * This program is the header of tables. */

#ifndef MAMAN14_PROJ_TABLES_H
#define MAMAN14_PROJ_TABLES_H

#include "globals.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>


/*  those are the types of symbols we can have in each file */
typedef enum symbol_type {
    gen = 0, code = 1, data = 2, macro_t = 3, mdefine = 4, external = 5
} symbol_type;

/* The places of symbols we can have in each file - A is general, E - 1, R - 2 and for macro -3 */

typedef enum symbol_place {
    general = 0, externa = 1, entry = 2, macro_p = 3
} symbol_place;


/*This is the struct of the symbol linked list as described in tables.c */
typedef struct symbol_table {
    symbol_type type; /*the type of the new symbol*/
    symbol_place place; /* the place of the new symbol - A,R,E*/
    int is_ext; /* if the symbol is extrn. */
    int declare_address; /*in case it was added as extern or entry this is its first label */
    int use_address; /* this is the label where the symbol was used in */
    int symbol_value; /* the value of the new symbol if its define*/
    char symbol_name[MAX_LABEL];/* the name of the symbol */
    struct symbol_table *next;
} symbol_table;


symbol_table *create_symbol_table();

void add_symbol(symbol_table **head, char *new_label, symbol_type type, symbol_place place, int val, int d_address,
                int u_address, int is_ext);

symbol_table *check_symbol_exist(symbol_table *head, char *label_name);

void free_symbols(symbol_table *head);

#endif
