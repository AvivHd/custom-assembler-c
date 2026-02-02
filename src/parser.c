/* Aviv Hadar, ID: 207750993
 * This program runs the parser - it takes a line from the file and breaks it to pieces of information and stores
 * them in our tree.
 * we'll check for the line type including its parameters and syntax errors.
 * at the end of the file we'll get a tree with all the line information. */

#include "parser.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

/*global instruction table, we should get from any file the code of the instruction, the number of its operands and the
 * instruction name */
instruction_table instructions[INST_AMOUNT] = {
        /* the first family  */
        {"mov", 2, "0,1,2,3", "1,2,3",   0},
        {"cmp", 2, "0,1,2,3", "0,1,2,3", 1},
        {"add", 2, "0,1,2,3", "1,2,3",   2},
        {"sub", 2, "0,1,2,3", "1,2,3",   3},
        {"lea", 2, "1,2",     "1,2,3",   6},
        /* second family */
        {"not", 1, NULL,      "1,2,3",   4},
        {"clr", 1, NULL,      "1,2,3",   5},
        {"inc", 1, NULL,      "1,2,3",   7},
        {"dec", 1, NULL,      "1,2,3",   8},
        {"jmp", 1, NULL,      "1,3",     9},
        {"bne", 1, NULL,      "1,3",     10},
        {"red", 1, NULL,      "1,2,3",   11},
        {"prn", 1, NULL,      "0,1,2,3", 12},
        {"jsr", 1, NULL,      "1,3",     13},
        /* the third family  */
        {"rts", 0, NULL, NULL,           14},
        {"hlt", 0, NULL, NULL,           15},
};

/**
 * This function checks if the number is between the numbers we can represent in binary two's complement with 12 bits.
 * @param num - the number we want to check
 * @return - 1 if the number is ok, else 0
 */
int check_range(int num) {
    if (num >= MIN_NUM && num <= MAX_NUM) {
        return 1;
    }
    return 0;
}

/**
 * Checking if the number we got is decimal integer
 * @param str - the string with the number
 * @return 1 if the number is decimal integer, 0 otherwise
 */
int is_decimal_integer(char *str) {
    while (isspace(*str)) {
        str++;
    }
    /*check is there is sign leading '-' or '+' */
    if (*str == '-' || *str == '+') {
        str++;
    }
    if (!isdigit(*str)) {
        return 0;
    }
    if (!check_range(*str)) {
        return 0;
    }
    /*check each char*/
    while (*str && !(isspace(*str))) {
        if (!isdigit(*str)) {
            return 0;
        }
        str++;
    }
    return 1;
}


/**
 * Checks by how many we need to increase the instruction counter according to the number of instructions
 * @param tree - our syntax tree
 * @return the number we need to increase the ic
 */
int increase_ic_count(ast *tree) {
    int num_words = 1, i = 0, max = 2;
    if (tree->line_type.inst_info.inst_data.num_args == 2) {
        i = 0; /*we'll check operand 0 and operand 1 */
    } else if (tree->line_type.inst_info.inst_data.num_args == 1) {
        i = 1; /*we'll check only operand 1 */
    }
    for (; i < max; i++) {
        if (tree->line_type.inst_info.inst_operands[i].addr_opt >= opt_imm
            && tree->line_type.inst_info.inst_operands[i].addr_opt <= opt_label) {
            /*if we have immediate or direct addressing we'll add 1 more word */
            num_words++;
        } else if (tree->line_type.inst_info.inst_operands[i].addr_opt == opt_label_w_const
                   || tree->line_type.inst_info.inst_operands[i].addr_opt == opt_label_w_index) {
            /*if we have direct index addressing we'll add 2 more words */
            num_words += 2;
        } else if (tree->line_type.inst_info.inst_data.num_args == 2 && i == 1 &&
                   tree->line_type.inst_info.inst_operands[0].addr_opt == opt_reg) {
            /*if we have 2 registers they will share the word (we're adding at the first reg)*/
            return num_words;
        } else if (tree->line_type.inst_info.inst_operands[i].addr_opt == opt_reg) {
            /*if we have register addressing we'll add 1 more word */
            num_words++;
        }
    }
    return num_words;
}

/**
 * This function skips white chars and updating the index of the line
 * @param line - the line we want to skip white in
 * @param i - the index of the line
 */
void skip_white(char *line, int *i) {
    while (isspace(line[*i])) {
        (*i)++;
    }
}


/**
 * Checks if the label is valid:
 * 1. Unique name - not like macro or define.
 * 2. first letter must be alphabetic
 * 3. can't be reserved name such as register name, ic, dc, define, entry and so.
 * 4. no more then 30 characters.
 * @param label - the label we want to check
 * @return 1 if the label is ok, else 0
 */
int check_valid_label(char *label) {
    int length = strlen(label), i;
    char *dir_opt[] = {".entry", ".extern", ".data", ".string"};
    char *registers[] = {"r0", "r1", "r2",
                         "r3", "r4", "r5",
                         "r6", "r7", "PSW"};
    if (length > MAX_LABEL) {
        return 0;
    }
    for (i = 0; i < NUM_REGISTERS; i++) {
        if (strcmp(label, registers[i]) == 0) {
            return 0;
        }
    }
    for (i = 0; i < INST_AMOUNT; i++) {
        if (strcmp(label, instructions[i].inst_name) == 0) {
            return 0;
        }
    }
    for (i = 0; i < DIR_OPT; i++) {
        if (strcmp(label, dir_opt[i]) == 0) {
            return 0;
        }
    } /* if the label is equal to define it's error */
    if (strcmp(label, "define") == 0) {
        return 0;
    }
    /* if the label starts with something diff then alphabetic */
    if (!(isalpha(label[0]))) {
        return 0;
    }
    return 1;
}

/**
 * This function check if there is a label in the line by searching for the ':' char.
 * @param line - the line we search in
 * @param label_length - the label length, we'll update it.
 * @return char the label name if we found and NULL if there is no label
 */
char *search_label(char *line, int *label_length) {
    char *opt_label;
    int index = 0, start = 0, len = 0;
    skip_white(line, &index);
    start = index;
    if (strchr(line, ':') != NULL) {
        while (line[index] != ':') {
            index++;
            len++;
        }
        *label_length = len;
        opt_label = (char *) malloc(sizeof(char) * ((*label_length) + 1));
        if (!opt_label) {
            printf("error allocating memory for label, exiting.\n");
            exit(1);
        }
        strncpy(opt_label, line + start, *label_length);
        opt_label[(*label_length)] = '\0';
        return opt_label;
    }
    return NULL;
}


/**
 * check if the string is valid:
 * 1. starts with " and ends with "
 * 2. all the characters in the string are printable (according to the forum)
 * @param line - the line we are checking the string in
 * @param index - the index of the line
 * @param tree - the data tree
 */
void check_string(char *line, int *index, ast *tree) {
    int i = *index;
    char *tmp = line;
    int start, length;
    skip_white(line, &i);
    if (line[i] == '\n' || line[i] == EOF || line[i] == '\0') {
        tree->ast_opt = ast_syntax_error;
        strcpy(tree->ast_errors, "empty string");
        *index = i;
        return;
    }
    if (line[i] != '"') {
        tree->ast_opt = ast_syntax_error;
        strcpy(tree->ast_errors, "invalid string start");
        *index = i;
        return;
    }
    if (line[i] == '"') {
        i++; /*pass the " */
        start = i;
        /*in the forum they said to print only visible ascii characters with isprint */
        while (tmp[i] != '\0' && tmp[i] != EOF && isprint(tmp[i])) {
            /* if we found " but there is no other " in the rest of the string it's the end */
            if (tmp[i] == '"' && !(strstr(tmp + i + 1, "\""))) {
                break;
            }
            i++;
        }
        /*end of string */
        if (tmp[i] == '"') {
            length = tmp + i - (line + start);
            line[start + length] = '\0';
            strncpy(tree->line_type.dir_info.dir_operands.string, line + start, length);
            tree->line_type.dir_info.dir_operands.string[length] = '\0';
            i++; /*skip the " of the end of string */
            *index = i;
            if (!is_blank(line + i)) {
                tree->ast_opt = ast_syntax_error;
                strcpy(tree->ast_errors, "extra characters after string.");
                *index = i;
                return;
            }
        } else {
            tree->ast_opt = ast_syntax_error;
            strcpy(tree->ast_errors, "String didn't end with \". ");
            *index = i;
            return;
        }
    }
}

/**
 * extracting the label name from the line and sending it to check label. if it's good we'll add it to the tree.
 * @param line - the line we want to extract the label from
 * @param index - the index of the line
 * @param tree - the data tree
 */
void get_label_name(char *line, int *index, ast *tree) {
    int start, len = 0, i = *index;
    char *name = NULL;
    if (line[i] == '\0' || line[i] == '\n' || line[i] == EOF) {
        tree->ast_opt = ast_syntax_error;
        strcpy(tree->ast_errors, "Label is empty");
        *index = i;
        return;
    }
    while (line[i] == ' ' || line[i] == '\t') {
        i++;
    }
    start = i;
    while (line[i] != '\0' && line[i] != '\n' && line[i] != EOF && !(isspace(line[i]))) {
        len++;
        i++;
        if (len >= MAX_LABEL) {
            tree->ast_opt = ast_syntax_error;
            strcpy(tree->ast_errors, "label name too long");
            *index = i;
            return;
        }
    }
    name = (char *) malloc(sizeof(char) * len + 1);
    if (!name) {
        printf("Error allocating memory, exiting.\n");
        exit(1);
    }
    strncpy(name, line + start, len);
    name[len] = '\0';
    if (check_valid_label(name)) {
        strncpy(tree->line_type.dir_info.dir_operands.label_name, name, len);
        tree->line_type.dir_info.dir_operands.label_name[len] = '\0';
        free(name);
        *index = i;
    } else {
        tree->ast_opt = ast_syntax_error;
        strcpy(tree->ast_errors, "label name invalid");
        free(name);
        *index = i;
        return;
    }
}

/**
 * checks if the string is digit
 * @param check - the string we want to check
 * @return 1 if it's digit, else 0
 */
int is_digits(char *check) {
    int i = 0;
    while (*(check + i) != '\0') {
        if (isdigit(*(check + i))) {
            i++;
        } else {
            return 0;
        }
    }
    return 1;
}


/**
 *checking immediate address method - check if it's a valid number or valid define value
 * @param new_op - the new operand we are checking
 * @param tree - the data tree
 * @param symbols - head to symbols linked list
 * @param op - the number of operand (first or second)
 * @return the addressing method of the operand
 */
enum inst_addr_opt check_imm_addr(char *new_op, ast *tree, symbol_table *symbols, int op) {
    symbol_table *tmp = symbols;
    int i = 0;
    /* if it's a number */
    if (new_op[i] == '+' || new_op[i] == '-' || is_digits(new_op + i)) {
        *new_op = *(new_op + i);
        /* checking if the number is valid decimal integer */
        if (is_decimal_integer(new_op)) {
            tree->line_type.inst_info.inst_operands[op].num = atoi(new_op);
            tree->line_type.inst_info.inst_operands[op].addr_opt = opt_imm;
            return opt_imm;
        } else {
            strcpy(tree->ast_errors, "new op is with bad syntax - number is not decimal integer.");
            tree->ast_opt = ast_syntax_error;
            return opt_error;
        }
        /*it's a define const */
    } else {
        while (tmp != NULL) {
            /*finding the const and its value */
            if (strcmp(new_op, tmp->symbol_name) == 0) {
                tree->line_type.inst_info.inst_operands[op].num = tmp->symbol_value;
                *new_op = *(new_op + i);
                tree->line_type.inst_info.inst_operands[op].addr_opt = opt_imm_const;
                return opt_imm_const;
            }
            tmp = tmp->next;
        }
        /*we haven't found the const, or it's not a const*/
        strcpy(tree->ast_errors, "the operand has bad syntax.");
        tree->ast_opt = ast_syntax_error;
        return opt_error;
    }
}

/**
 * checking the direct index addressing method and making sure it is valid. we are not checking if
 * the index is within borders - not required and the label may be extern or code.
 * we might get define or number as index.
 * @param new_op - the operand we are checking
 * @param tree - the data tree
 * @param symbols - head to symbols linked list
 * @param op - the number of operand (first or second)
 * @return the addressing method of the operand
 */
enum inst_addr_opt check_index_addr(char *new_op, ast *tree, symbol_table *symbols, int op) {
    char *tmp_label = NULL, *tmp_num = NULL;
    int i = 0, len = 0, count;
    enum inst_addr_opt index;
    /* there are no ending braces in new op */
    if (!strchr(new_op, ']')) {
        strcpy(tree->ast_errors, "the operand has bad syntax.");
        tree->ast_opt = ast_syntax_error;
        return opt_error;
    }
    /*finding the label length */
    while (new_op[i] != '[') {
        /* there are no braces right after the label */
        if (isspace(new_op[i])) {
            strcpy(tree->ast_errors, "The operand has bad syntax- do not have braces right after label.");
            free(tmp_label);
            return opt_error;
        }
        len++;
        i++;
    }
    tmp_label = (char *) malloc(sizeof(char) * len + 1);
    if (!tmp_label) {
        printf("Error allocating memory, exiting.\n");
        exit(1);
    }
    strncpy(tmp_label, new_op, len);
    tmp_label[len] = '\0';
    count = 0;
    i++; /*skipping the '[' */
    /* no number between braces */
    if (new_op[i] == ']') {
        strcpy(tree->ast_errors, "the operand has bad syntax - no number between braces.");
        free(tmp_label);
        return opt_error;
    }
    /*finding the number length */
    while (new_op[i] != ']') {
        if (isspace(new_op[i])) {
            strcpy(tree->ast_errors, "the operand has bad syntax - space between braces.");
            free(tmp_label);
            return opt_error;
        }
        count++;
        i++;
    }
    tmp_num = (char *) malloc(sizeof(char) * count);
    if (!tmp_num) {
        printf("Error allocating memory, exiting.\n");
        free(tmp_label);
        exit(1);
    }
    strncpy(tmp_num, new_op + (len + 1), count);
    tmp_num[count] = '\0';
    /* checking if the immediate is a number or define */
    index = check_imm_addr(tmp_num, tree, symbols, op);
    if (index == opt_imm) {
        strncpy(tree->line_type.inst_info.inst_operands[op].label, tmp_label, len);
        tree->line_type.inst_info.inst_operands[op].label[len] = '\0';
        free(tmp_label);
        free(tmp_num);
        tree->line_type.inst_info.inst_operands[op].addr_opt = opt_label_w_index;
        return opt_label_w_index;
    } else if (index == opt_imm_const) {
        strncpy(tree->line_type.inst_info.inst_operands[op].label, tmp_label, len);
        tree->line_type.inst_info.inst_operands[op].label[len] = '\0';
        free(tmp_label);
        free(tmp_num);
        tree->line_type.inst_info.inst_operands[op].addr_opt = opt_label_w_const;
        return opt_label_w_const;
    }
    free(tmp_label);
    free(tmp_num);
    strcpy(tree->ast_errors, "the operand has bad syntax.");
    return opt_error;
}

/**
 * checking the operand and finding its addressing method
 * @param new_op - the operand we are checking
 * @param tree - the data tree
 * @param symbols - head to the symbols linked list
 * @param op  - the number of operand we are checking - first, second
 * @return the addressing method
 */
enum inst_addr_opt check_address(char *new_op, ast *tree, symbol_table *symbols, int op) {
    int i = 0, len;
    enum inst_addr_opt addressing;
    char *tmp_label = NULL;
    if (strlen(new_op) == 0) {
        strcpy(tree->ast_errors, "the operand is empty.");
        tree->ast_opt = ast_syntax_error;
        return opt_error;
    }
    /* immediate addressing must start with '#' */
    if (is_digits(new_op)) {
        strcpy(tree->ast_errors, "the operand start with numbers instead of '#'.");
        tree->ast_opt = ast_syntax_error;
        return opt_error;
    }
    /* it is an immediate addressing - number or const */
    if (new_op[i] == '#') {
        i++; /*to skip the '#' */
        addressing = check_imm_addr(new_op + i, tree, symbols, op);
        tree->line_type.inst_info.inst_operands[op].label[0] = '\0';
        return addressing;
        /* it is direct index addressing */
    } else if (strchr(new_op, '[')) {
        addressing = check_index_addr(new_op, tree, symbols, op);
        return addressing;
    } /*it's reg or label addressing - 1/3 */
    else if (new_op[i] == 'r') {
        i++;
        if (new_op[i] >= '0' && new_op[i] <= '7') {
            tree->line_type.inst_info.inst_operands[op].reg = atoi(new_op + i);
            tree->line_type.inst_info.inst_operands[op].label[0] = '\0';
            tree->line_type.inst_info.inst_operands[op].addr_opt = opt_reg;
            return opt_reg;
        } else { /*maybe it's a label start with r */
            i--; /*returning before the 'r' */
        }
        /*if it's not any of the above we'll check if it's a label */
    } else {
        len = i;
        /*findimg the len of the operand */
        while (new_op[len] != '\0' && !(isspace(new_op[len]))) {
            len++;
        }
        len -= i;
        tmp_label = (char *) malloc(sizeof(char) * len + 1);
        if (!tmp_label) {
            printf("Error allocating memory. exiting\n");
            exit(1);
        }
        strncpy(tmp_label, new_op + i, len);
        strncpy(tree->line_type.inst_info.inst_operands[op].label, tmp_label, len);
        tree->line_type.inst_info.inst_operands[op].label[len] = '\0';
        free(tmp_label);
        return opt_label;
    }
    strcpy(tree->ast_errors, "the operand has bad syntax.");
    tree->ast_opt = ast_syntax_error;
    free(tmp_label);
    return opt_error;
}


/**
 * checking the src operand for the first family of instructions
 * @param tree - the data tree
 * @param op_num - the number of the operand
 * @param addr_method - the addressing methid of the operand
 * @return 1 if everything was ok, else 0
 */
int check_operand_src(ast *tree, int op_num, int addr_method) {
    if (addr_method == opt_error) {
        return 0;
    } else if (addr_method == opt_imm || addr_method == opt_imm_const) {
        /* the only instruction in first family that is not getting direct addressing as src is 'lea' */
        if (strcmp(tree->line_type.inst_info.inst_data.inst, "lea") == 0) {
            strcpy(tree->ast_errors, "The operand have bad address method.");
            tree->ast_opt = ast_syntax_error;
            return 0;
        }
    } else if (addr_method == opt_reg) {
        /* the only instruction in first family that is not getting register addressing as src is 'lea' */
        if (strcmp(tree->line_type.inst_info.inst_data.inst, "lea") == 0) {
            strcpy(tree->ast_errors, "The operand have bad address method.");
            tree->ast_opt = ast_syntax_error;
            return 0;
        }
    }
    tree->line_type.inst_info.inst_operands[op_num].addr_opt = addr_method;
    return 1;
}


/**
 * checking the dest operand for the first family of instructions
 * @param tree - the data tree
 * @param op_num - the number of the operand
 * @param addr_method - the addressing methid of the operand
 * @return 1 if everything was ok, else 0
 */
int check_operand_dest(ast *tree, int op_num, int addr_method) {
    if (addr_method == opt_error) {
        return 0;
    } else if (addr_method == opt_imm || addr_method == opt_imm_const) {
        /*only cmp gets immediate addressing as dest operand */
        if (strcmp(tree->line_type.inst_info.inst_data.inst, "cmp") == 0) {
            if (addr_method == opt_imm) {
                tree->line_type.inst_info.inst_operands[1].op_option = 1;
                tree->line_type.inst_info.inst_operands[op_num].addr_opt = opt_imm;
            }
            return 0;
        } else {
            strcpy(tree->ast_errors, "the operand have bad address method.");
            tree->ast_opt = ast_syntax_error;
            return 0;
        }
    }
    return 1;
}

/**
 * updating the tree with first family instruction data
 * @param line - the line we are checking
 * @param index - the index in the line
 * @param tree - the tree data
 * @param symbols - head of symbols linked list
 */
void first_family_operands(char *line, int *index, ast *tree, symbol_table *symbols) {
    int result, len = 0, start, i = *index, op = 0;
    char *new_op;
    skip_white(line, &i);
    if (line[i] == ',') {
        strcpy(tree->ast_errors, "found comma before first operand");
        tree->ast_opt = ast_syntax_error;
        *index = i;
        return;
    } else { /*searching for the first operand */
        start = i;
        /*finding the length of the first operand */
        while (line[i] != '\0' && line[i] != ',' && line[i] != EOF && line[i] != '\n' && !(isspace(line[i]))) {
            i++;
            len++;
        }
        new_op = (char *) malloc(sizeof(char) * len + 1);
        if (!new_op) {
            printf("Error allocating memory, exiting.\n");
            exit(1);
        }
        strncpy(new_op, line + start, len);
        new_op[len] = '\0';
        result = check_address(new_op, tree, symbols, op); /*checking the address method of the operand */
        /*checking the src operand*/
        if (!check_operand_src(tree, op, result)) {
            free(new_op);
            return;
        }
        free(new_op);
        skip_white(line, &i);
        if (line[i] == ',') {
            i++;
        }
        while (line[i] == ' ' || line[i] == '\t') {
            (i)++;
        }
        skip_white(line, &i);
        len = 0;
        start = i;
        /* finding the length of the second operand */
        while (line[i] != '\0' && line[i] != ',' && line[i] != EOF && line[i] != '\n'
               && !(isspace(line[i]))) {
            i++;
            len++;
        }
        new_op = (char *) malloc(sizeof(char) * len + 1);
        if (!new_op) {
            printf("Error allocating memory for second operand, exiting.\n");
            exit(1);
        }
        strncpy(new_op, line + start, len);
        new_op[len] = '\0';
        op++;
        result = check_address(new_op, tree, symbols, op); /*check the addressing method of the second operand */
        /*checking the dest operand*/
        if (!check_operand_dest(tree, op, result)) {
            free(new_op);
            return;
        }
        tree->line_type.inst_info.inst_operands[op].addr_opt = result;
        free(new_op);
        return;
    }
}

/**
 * checking the operand for the second family of instructions
 * @param tree - the data tree
 * @param addr_method - the addressing methid of the opernad
 * @return 1 if everything is ok, else 0
 */
int sec_fam_dest(ast *tree, int addr_method) {
    if (addr_method == opt_error) {
        return 0;
    } else if (addr_method == opt_imm || addr_method == opt_imm_const) {
        /*in the second family of instructions only prn gets immediate addressing */
        if (strcmp(tree->line_type.inst_info.inst_data.inst, "prn") == 0) {
            return 0;
        } else {
            strcpy(tree->ast_errors, "the operand have wrong address method.");
            tree->ast_opt = ast_syntax_error;
            return 0;
        }
    } else if (addr_method == opt_label_w_const || addr_method == opt_label_w_index) {
        /*jsr, bne and jmp don't get direct index addressing method */
        if (strcmp(tree->line_type.inst_info.inst_data.inst, "jmp") == 0
            || strcmp(tree->line_type.inst_info.inst_data.inst, "bne") == 0
            || strcmp(tree->line_type.inst_info.inst_data.inst, "jsr") == 0) {
            strcpy(tree->ast_errors, "The operand have bad address method.");
            tree->ast_opt = ast_syntax_error;
            return 0;
        }
    }
    return 1;
}

/**
 * updating the tree with second family instruction data
 * @param line - the line we are checking
 * @param index - the index in the line
 * @param tree - the tree data
 * @param symbols - head of symbols linked list
 */
void sec_family_operands(char *line, int *index, ast *tree, symbol_table *symbols) {
    int result, len = 0, start, i = *index, op = 1;
    char *new_op;
    while (line[i] == ' ' || line[i] == '\t') {
        i++;
    }
    /* we don't have any operand */
    if (is_blank(line + i)) {
        strcpy(tree->ast_errors, "not enough operands");
        tree->ast_opt = ast_syntax_error;
        return;
    }
    start = i;
    /*finding the length of the operand */
    while (line[i] != '\0' && line[i] != ',' && line[i] != EOF && line[i] != '\n' && !(isspace(line[i]))) {
        i++;
        len++;
    }
    /*after the operand we have more characters */
    if (!is_blank(line + i)) {
        strcpy(tree->ast_errors, "too many operands");
        tree->ast_opt = ast_syntax_error;
        return;
    }
    new_op = (char *) malloc(sizeof(char) * len + 1);
    if (!new_op) {
        printf("Error allocating memory, exiting.\n");
        exit(1);
    }
    strncpy(new_op, line + start, len);
    new_op[len] = '\0';
    result = check_address(new_op, tree, symbols, op); /*checking the address method of the operand */
    /*checking the operand*/
    if (!sec_fam_dest(tree, result)) {
        free(new_op);
        return;
    }
    /*updating the address method to the operand in the tree */
    tree->line_type.inst_info.inst_operands[1].addr_opt = result;
    free(new_op);
}

/**
 *getting the instruction information to the tree, checking the family of the instruction and
 * the number of operands for it.
 * @param tree - the data tree
 * @param line - the line we are checking
 * @param index - the index in the line
 * @param symbols - head to the symbols linked list
 */
void get_instruction(ast *tree, char *line, int *index, symbol_table *symbols) {
    int i = *index, start = *index, inst_len = 0, j;
    char *inst = NULL;
    skip_white(line, &i);
    /*finding the instruction length */
    while (line[i] != '\0' && !isspace(line[i])) {
        inst_len++;
        i++;
    }
    inst = (char *) malloc(sizeof(char) * (inst_len + 1));
    if (!inst) {
        printf("Error allocating memory, exiting.\n");
        exit(1);
    }
    strncpy(inst, line + start, inst_len);
    inst[inst_len] = '\0';
    /*checking if the instruction valid */
    for (j = 0; j < INST_AMOUNT; j++) {
        /*finding the right instruction */
        if (strcmp(inst, instructions[j].inst_name) == 0) {
            tree->ast_opt = instruction;
            strncpy(tree->line_type.inst_info.inst_data.inst, inst, inst_len);
            tree->line_type.inst_info.inst_data.inst[inst_len] = '\0';
            /*copying the number of arguments*/
            tree->line_type.inst_info.inst_data.num_args = instructions[j].n_ops;
            /*copying the instruction opcode*/
            tree->line_type.inst_info.inst_data.inst_code = instructions[j].code;
            if (strchr(line, ',')) { /*searching for comma */
                /*check if operand is from the first family */
                if (tree->line_type.inst_info.inst_data.num_args == 2) {
                    free(inst);
                    first_family_operands(line, &i, tree, symbols);
                    return;
                } else {/*if it's not first family instruction there is no need in comma */
                    strcpy(tree->ast_errors, "too many operands or extra comma");
                    tree->ast_opt = ast_syntax_error;
                    free(inst);
                    return;
                }
            } else { /*we haven't found comma separator */
                if (tree->line_type.inst_info.inst_data.num_args == 2) {
                    strcpy(tree->ast_errors, "not enough operands or operand without comma separator");
                    tree->ast_opt = ast_syntax_error;
                    free(inst);
                    return;
                } else if (tree->line_type.inst_info.inst_data.num_args == 1) {
                    free(inst);
                    sec_family_operands(line, &i, tree, symbols);
                    return;
                } else if (tree->line_type.inst_info.inst_data.num_args == 0) {
                    if (!is_blank(line + i)) {
                        strcpy(tree->ast_errors, "too many operands");
                        tree->ast_opt = ast_syntax_error;
                        free(inst);
                        return;
                    }
                    free(inst);
                    return;
                }
            }
            while (line[i] == ' ' || line[i] == '\t') {
                (i)++;
            }
        }
    }
    tree->ast_opt = ast_syntax_error;
    strcpy(tree->ast_errors, "syntax error");
    free(inst);
}

/**
 * check if the define const we found is valid, if so save it in the tree.
 * @param head - head of symbols linked list
 * @param index - the index of the line we are checking
 * @param line - the line we are checking
 * @param tree - the data tree
 * @return
 */
int check_def(symbol_table *head, int *index, char *line, ast *tree) {
    long def_val;
    int i = 0, j = 0, def_length = 0;
    char *tmp_name = NULL;
    i = *index; /* i - the beginning of the line without white char */
    j = i;
    skip_white(line, &i);
    while (line[j] != '=' && line[j] != '\n') {
        if (!isspace(line[j])) {
            def_length++;
        }
        j++; /* j - the index after the define name */
    }
    tmp_name = (char *) malloc(sizeof(char) * (def_length + 1));
    if (!tmp_name) {
        printf("Error allocating memory, exiting.\n");
        exit(1);
    }
    strncpy(tmp_name, line + i, def_length);
    tmp_name[def_length] = '\0';
    j++; /* skips the '=' */
    skip_white(line, &j);
    /* checking if the define value in decimal integer */
    if (!is_decimal_integer(line + j)) {
        free(tmp_name);
        return 0; /* the number is not valid */
    } else {
        /*the number is valid, convert it to int and save it in the tree */
        def_val = atoi(line + j);
        /*if the define name is new */
        if (check_symbol_exist(head, tmp_name) == NULL) {
            strncpy(tree->line_type.def_info.def_name, tmp_name, def_length);
            tree->line_type.def_info.def_name[def_length] = '\0';
            tree->line_type.def_info.def_value = def_val;
            free(tmp_name);
            return 1;
        } else {
            strcpy(tree->ast_errors, "the define name already exists\n");
            tree->ast_opt = ast_syntax_error;
            free(tmp_name);
            return 0;
        }
    }
}

/**
 * checking if we got directive or define - both starts with '.'
 * @param line - the line we are checking
 * @param index - the index in the line
 * @param head - head of symbols linked list
 * @param tree - the data tree
 */
void dir_or_def(char *line, int *index, symbol_table *head, ast *tree) {
    int i = 0, ok_flag;
    i = *index;
    skip_white(line, &i);
    if (strncmp(line + i, "define", DEF_LEN) == 0) { /*this is a definition of const */
        i += DEF_LEN;
        /*define cannot be with label */
        if (tree->label_name[0] != '\0') {
            strcpy(tree->ast_errors, "define with label\n");
            tree->ast_opt = ast_syntax_error;
        }
        /*checks if define is valid */
        ok_flag = check_def(head, index, line + i, tree);
        if (ok_flag) { /* define is ok */
            tree->ast_opt = def;
        } else {
            strcpy(tree->ast_errors, "error with definition of define");
            tree->ast_opt = ast_syntax_error;
        }
        return;
    } else if (strncmp(line + i, "string", STR_LEN) == 0) { /*This is a string */
        tree->ast_opt = directive;
        tree->line_type.dir_info.dir_opt = dir_string;
        i += STR_LEN;
        check_string(line, &i, tree);
        tree->line_type.dir_info.dir_operands.label_name[0] = '\0';
        return;
    } else if (strncmp(line + i, "data", DATA_LEN) == 0) { /*This is a data */
        tree->ast_opt = directive;
        tree->line_type.dir_info.dir_opt = dir_data;
        i += DATA_LEN;
        strncpy(tree->line_type.dir_info.dir_operands.string, line + i, MAX_LEN);
        tree->line_type.dir_info.dir_operands.label_name[0] = '\0';
        return;
    } else if (strncmp(line + i, "entry", ENT_LEN) == 0) { /*This is entry */
        tree->ast_opt = directive;
        tree->line_type.dir_info.dir_opt = dir_entry;
        i += ENT_LEN;
        get_label_name(line, &i, tree);
        return;
    } else if (strncmp(line + i, "extern", EXT_LEN) == 0) { /*This is extern */
        tree->ast_opt = directive;
        tree->line_type.dir_info.dir_opt = dir_extern;
        i += EXT_LEN;
        get_label_name(line, &i, tree);
        return;
    } else {
        tree->ast_opt = ast_syntax_error;
        strcpy(tree->ast_errors, "directive doesn't exist");
        return;
    }
}

/**
 * checks if there is a label at the beginning of the line, if so - add it to the tree.
 * @param tree - the data tree
 * @param line - the line we are checking
 * @param index - the index of the line
 */
void check_for_label(ast *tree, char *line, int *index) {
    char *tmp_label = NULL;
    int label_length = 0;
    skip_white(line, index);
    tmp_label = search_label(line, &label_length);
    /*if we found label at the beginning of the line with ':' */
    if (tmp_label != NULL) {
        *index += label_length;
        (*index)++; /*skip ths : */
        /*checking if the label is valid */
        if (check_valid_label(tmp_label) == 1) {
            strncpy(tree->label_name, tmp_label, label_length); /*copying the name to the tree */
            tree->label_name[label_length] = '\0';
            free(tmp_label);
        } else {/*there is a label but with syntax error */
            strcpy(tree->ast_errors, "label defined wrong");
            tree->ast_opt = ast_syntax_error;
            free(tmp_label);
        }
    } else { /*in this line we have no label */
        tree->label_name[0] = '\0';
    }
}


/**
 * The function find in which line type we are and updates the tree with the right data
 * @param line - the line we are reading and checking
 * @param symbols - head of symbols linked list
 * @return tree with the line data
 */
ast get_ast(char *line, symbol_table *symbols) {
    int index = 0;
    ast tree;
    tree.ast_opt = empty;
    tree.ast_errors[0] = '\0';
    skip_white(line, &index);
    /*checking for label */
    check_for_label(&tree, line, &index);
    if (tree.ast_opt == ast_syntax_error) {
        return tree;
    }
    skip_white(line, &index);
    /* checking for define or directive with '.' */
    if (strchr(line, '.')) {
        while (line[index] != '.') {
            index++;
        }
        index++;
        /*checks if it's define or directive */
        dir_or_def(line, &index, symbols, &tree);
        return tree;
    } else { /*the option left is instruction */
        while (line[index] == ' ' || line[index] == '\t') {
            index++;
        }
        get_instruction(&tree, line, &index, symbols);
        return tree;
    }
}