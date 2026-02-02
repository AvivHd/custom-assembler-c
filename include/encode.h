/* Aviv Hadar, ID: 207750993
 * This program is the header of encode. */

#ifndef MAMAN14_PROJ_ENCODE_H
#define MAMAN14_PROJ_ENCODE_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "globals.h"
#include "tables.h"
#include <stdint.h>

/**
 * this is the struct of the code table
 */
typedef struct code_table {
    int address;
    char code[MAX_BITS];
    struct code_table *next;
} code_table;


void encode_data(int *data, int counter, code_table **codes, int address);
void encode_string(char *str, int address, code_table **codes);
void encode_inst(int opcode, int are, int src, int dest, code_table **codes, int address);
void encode_num(int num, int are, code_table **codes, int address);
void encode_reg(int src, int dest, code_table **codes, int address);
void add_code_table(char *code, code_table **codes, int address);
void reverse(char *buffer, code_table **codes, int address);
void free_codes(code_table *head);

#endif
