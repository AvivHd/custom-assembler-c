/* Aviv Hadar, ID: 207750993
 * This program is the header of parser.
 * We have here all the structures and enums of the parser. */

#ifndef MAMAN14_PROJ_PARSER_H
#define MAMAN14_PROJ_PARSER_H

#include <limits.h>
#include <string.h>
#include <stdio.h>
#include "globals.h"
#include <ctype.h>
#include "tables.h"
#include "preprocessor.h"

#define DEF_LEN 6 /*length of 'define' */
#define STR_LEN 6 /*length of 'string' */
#define DATA_LEN 4 /*length of 'data' */
#define ENT_LEN 5 /*length of 'entry' */
#define EXT_LEN 6 /*length of 'extern' */
#define MAX_NUM 2047 /* maximum number we can get with 12 bits in binary two's complement */
#define MIN_NUM (-2048) /*minimum number we can get with 12 bits in binary two's complement */


/* instruction  struct */
typedef struct instruction_table {
    const char *inst_name;
    int n_ops;
    const char *source_op;
    const char *dest_op;
    int code;
} instruction_table;

/* enum of the instruction addressing methods */
typedef enum inst_addr_opt {
    opt_error = -1,
    opt_imm = 0,
    opt_imm_const = 1,
    opt_label = 2,
    opt_label_w_index = 3,
    opt_label_w_const = 4,
    opt_reg = 5
} inst_addr_opt;

/* the options of each instruction operand */
typedef enum inst_op {
    regi = 0,
    num = 1,
    lab = 2
} inst_op;

/* all the options we have for directive */
typedef enum dir_options {
    dir_entry, dir_extern, dir_string, dir_data
} dir_options;

/* the options of the ast */
typedef enum ast_options {
    instruction, directive, def, ast_syntax_error, empty
} ast_options;

/*This is the parser while tree struct, it holds all the data of the current line */
typedef struct ast {
    char ast_errors[MAX_LEN]; /*if there is an error in the line here we'll save the details about it */
    char label_name[MAX_LABEL]; /* the label at the beginning of the line*/
    ast_options ast_opt;
    struct { /*the type of the current line and its data */
        struct {
            dir_options dir_opt;
            struct { /*the struct of the directive operands */
                char label_name[MAX_LABEL];
                char string[MAX_LEN];
                struct { /*the data struct */
                    int data[MAX_LEN];
                    int data_counter;
                } data_info;
            } dir_operands;
        } dir_info;
        struct { /* the instruction data struct */
            struct { /* struct of the operands in the instruction */
                inst_addr_opt addr_opt;
                inst_op op_option;
                int num;
                int reg;
                char label[MAX_LABEL];
            } inst_operands[2];
            struct {
                char inst[INST_MAX];
                int num_args;
                int inst_code;
            } inst_data;
        } inst_info;
        struct { /*the define data - the name and value of the const */
            char def_name[MAX_LABEL];
            int def_value;
        } def_info;
    } line_type;
} ast;


void skip_white(char *line, int *i);

int is_digits(char *check);

int check_valid_label(char *label);

int is_decimal_integer(char *str);

char *search_label(char *line, int *label_length);

int increase_ic_count(ast *tree);

enum inst_addr_opt check_address(char *new_op, ast *tree, symbol_table *symbols, int op);

void check_string(char *line, int *index, ast *tree);

void get_label_name(char *line, int *index, ast *tree);

void first_family_operands(char *line, int *index, ast *tree, symbol_table *symbols);

void sec_family_operands(char *line, int *index, ast *tree, symbol_table *defs);

int check_def(symbol_table *head, int *index, char *line, ast *tree);

void dir_or_def(char *line, int *index, symbol_table *head, ast *tree);

ast get_ast(char *line, symbol_table *symbols);

#endif
