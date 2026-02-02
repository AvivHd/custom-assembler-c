/* Aviv Hadar, ID: 207750993
 * This program runs the first pass on the assembler checking the ast values. */

#include "sec_pass.h"

/**
 * This function checks if there are errors with given symbol - label or define
 * @param op_num - the number of the operand we are checking
 * @param tree - the ast tree with the parser data
 * @param symbols - head of the symbol table linked list
 * @return 1 if everything was ok, 0 if found error
 */
int check_label_error(int op_num, ast *tree, symbol_table *symbols) {
    symbol_table *tmp;
    /* if we have operand with label or define addressing we'll check if this label or define are known to us */
    if ((tree->line_type.inst_info.inst_operands[op_num].addr_opt >= opt_label
         && tree->line_type.inst_info.inst_operands[op_num].addr_opt <= opt_label_w_const) ||
        tree->line_type.inst_info.inst_operands[op_num].addr_opt == opt_imm_const) {
        tmp = check_symbol_exist(symbols, tree->line_type.inst_info.inst_operands[op_num].label);
        /* the label or define is new to us, it's an error in this stage we supposed to know them all */
        if (tmp == NULL) {
            strcpy(tree->ast_errors, "we used a label or define that has not been declared before.");
            tree->ast_opt = ast_syntax_error;
            return 0;
        }
        return 1;
    }
    return 1;
}

/**
 * This function check the addressing form of the operand
 * @param tree - the ast tree with the parser data
 * @param op_num - the number of the operand we are checking
 * @return the addressing option or -1 if error
 */
int check_addr(ast *tree, int op_num) {
    /* immediate addressing */
    if (tree->line_type.inst_info.inst_operands[op_num].addr_opt == opt_imm
        || tree->line_type.inst_info.inst_operands[op_num].addr_opt == opt_imm_const) {
        return 0;
        /* direct addressing */
    } else if (tree->line_type.inst_info.inst_operands[op_num].addr_opt == opt_label) {
        return 1;
        /* const index addressing */
    } else if (tree->line_type.inst_info.inst_operands[op_num].addr_opt >= opt_label_w_index
               && tree->line_type.inst_info.inst_operands[op_num].addr_opt <= opt_label_w_const) {
        return 2;
        /* register addressing */
    } else if (tree->line_type.inst_info.inst_operands[op_num].addr_opt == opt_reg) {
        return 3;
    }
    /* if found no addressing so far we have an error */
    return -1;
}

/**
 * This function codes instruction from the first family of instructions
 * @param tree - the ast tree with the parser data
 * @param symbols - head to symbol table linked list
 * @param codes - head to the code table linked list
 * @param ic - instruction counter
 * @return 1 if everything was ok, 0 if found error
 */
int first_fam(ast *tree, symbol_table **symbols, code_table **codes, long *ic) {
    int opcode, src, dest, are, address = ADDRESS_BASE + *ic;
    symbol_table *tmp;
    opcode = tree->line_type.inst_info.inst_data.inst_code;
    src = check_addr(tree, 0); /* finding the src operand addressing */
    dest = check_addr(tree, 1); /* finding the dest operand addressing */
    /* first we'll check if we have any error with the dest or source operands */
    if (src == -1 || dest == -1) {
        printf("dest or src operand addressing is wrong.\n");
        strcpy(tree->ast_errors, "Error, dest or src operand address is wrong\n");
        return 0;
    }
    /* encoding the first instruction word */
    encode_inst(opcode, 0, src, dest, codes, address);
    (*ic)++;
    address = ADDRESS_BASE + *ic;
    /* if we have 2 operands that are registers they will share the additional word */
    if (src == 3 && dest == 3) {
        encode_reg(tree->line_type.inst_info.inst_operands[0].reg,
                   tree->line_type.inst_info.inst_operands[1].reg, codes, address);
        (*ic)++;
        address = ADDRESS_BASE + *ic;
    } else {
        if (src == 0) {
            encode_num(tree->line_type.inst_info.inst_operands[0].num, 0, codes, address);
            (*ic)++;
            address = ADDRESS_BASE + *ic;
        }
        if (src == 1) {
            tmp = check_symbol_exist(*symbols, tree->line_type.inst_info.inst_operands[0].label);
            if (!tmp) {
                printf("couldn't find symbol.\n");
                return 0;
            }
            are = tmp->type == data ? 2 : tmp->place;
            encode_num(tmp->use_address, are, codes, address);
            /* if we found a use of symbol which declared extern we'll add it to know where it was used */
            if (tmp->place == externa) {
                add_symbol(symbols, tmp->symbol_name, code,
                           tmp->place, 0, 0, address, 1);
            }
            (*ic)++;
            address = ADDRESS_BASE + *ic;
        }
        if (src == 2) {
            tmp = check_symbol_exist(*symbols, tree->line_type.inst_info.inst_operands[0].label);
            if (!tmp) {
                printf("couldn't find symbol.\n");
                return 0;
            }
            are = tmp->type == data ? 2 : tmp->place;
            encode_num(tmp->use_address, are, codes, address);
            (*ic)++;
            address = ADDRESS_BASE + *ic;
            encode_num(tree->line_type.inst_info.inst_operands[0].num, 0, codes, address);
            (*ic)++;
            address = ADDRESS_BASE + *ic;
        }
        if (src == 3) {
            encode_reg(tree->line_type.inst_info.inst_operands[0].reg, 0, codes, address);
            (*ic)++;
            address = ADDRESS_BASE + *ic;
        }
        if (dest == 0) {
            encode_num(tree->line_type.inst_info.inst_operands[1].num, 0, codes, address);
            (*ic)++;
            address = ADDRESS_BASE + *ic;
        }
        if (dest == 1) {
            tmp = check_symbol_exist(*symbols, tree->line_type.inst_info.inst_operands[1].label);
            if (!tmp) {
                printf("couldn't find symbol.\n");
                return 0;
            }
            are = tmp->type == data ? 2 : tmp->place;
            encode_num(tmp->use_address, are, codes, address);
            /* if we found a use of symbol which declared extern we'll add it to know where it was used */
            if (tmp->place == externa) {
                add_symbol(symbols, tmp->symbol_name, code, tmp->place,
                           0, 0, address, 1);
            }
            (*ic)++;
            address = ADDRESS_BASE + *ic;
        }
        if (dest == 2) {
            tmp = check_symbol_exist(*symbols, tree->line_type.inst_info.inst_operands[1].label);
            if (!tmp) {
                printf("couldn't find symbol.\n");
                return 0;
            }
            are = tmp->type == data ? 2 : tmp->place;
            encode_num(tmp->use_address, are, codes, address);
            (*ic)++;
            address = ADDRESS_BASE + *ic;
            encode_num(tree->line_type.inst_info.inst_operands[1].num, 0, codes, address);
            (*ic)++;
            address = ADDRESS_BASE + *ic;
        }
        if (dest == 3) {
            encode_reg(0, tree->line_type.inst_info.inst_operands[1].reg, codes, address);
            (*ic)++;
            address = ADDRESS_BASE + *ic;
        }
    }
    return 1;
}

/**
 * This function codes instruction from the second family of instructions
 * @param tree - the ast tree with the parser data
 * @param symbols - head to symbol table linked list
 * @param codes - head to the code table linked list
 * @param ic - instruction counter
 * @return 1 if everything was ok, 0 if found error
 */
int sec_fam(ast *tree, symbol_table **symbols, code_table **codes, long *ic) {
    int opcode, src = 0, dest, are, address = ADDRESS_BASE + *ic;
    symbol_table *tmp;
    opcode = tree->line_type.inst_info.inst_data.inst_code;
    dest = check_addr(tree, 1); /* finding the dest operand addressing */
    /* first we'll check if we have any error with the dest operand */
    if (dest == -1) {
        printf("dest or src operand address is wrong.\n");
        strcpy(tree->ast_errors, "Error, dest or src operand address is wrong\n");
        return 0;
    }
    /* encoding the first instruction word*/
    encode_inst(opcode, 0, src, dest, codes, address);
    (*ic)++;
    address = ADDRESS_BASE + *ic;
    if (dest == 0) {
        encode_num(tree->line_type.inst_info.inst_operands[1].num, 0, codes, address);
        (*ic)++;
        address = ADDRESS_BASE + *ic;
    }
    if (dest == 1) {
        tmp = check_symbol_exist(*symbols, tree->line_type.inst_info.inst_operands[1].label);
        if (!tmp) {
            printf("couldn't find symbol.\n");
            return 0;
        }
        are = tmp->type == data ? 2 : tmp->place;
        encode_num(tmp->use_address, are, codes, address);
        /* if we found a use of symbol which declared extern we'll add it to know where it was used */
        if (tmp->place == externa) {
            add_symbol(symbols, tmp->symbol_name, code, tmp->place,
                       0, 0, address, 1);
        }
        (*ic)++;
        address = ADDRESS_BASE + *ic;
    }
    if (dest == 2) {
        tmp = check_symbol_exist(*symbols, tree->line_type.inst_info.inst_operands[1].label);
        if (!tmp) {
            printf("couldn't find symbol.\n");
            return 0;
        }
        are = tmp->type == data ? 2 : tmp->place;
        encode_num(tmp->use_address, are, codes, address);
        (*ic)++;
        address = ADDRESS_BASE + *ic;
        encode_num(tree->line_type.inst_info.inst_operands[1].num, 0, codes, address);
        (*ic)++;
        address = ADDRESS_BASE + *ic;
    }
    if (dest == 3) {
        encode_reg(0, tree->line_type.inst_info.inst_operands[1].reg, codes, address);
        (*ic)++;
        address = ADDRESS_BASE + *ic;
    }
    return 1;
}

/**
 * This function runs the second pass of the assembler. Checking if we have code or data ans send it to encoding.
 * @param line - the line we are checking now
 * @param tree - the ast tree with the parser data
 * @param symbols - head to symbol table linked list
 * @param codes - head to the code table linked list
 * @param ic - instruction counter (code)
 * @param dc - data counter
 * @param final_ic - the final ic we got from the first pass
 * @return 1 if everything was ok, 0 if found error
 */
int second_pass(char *line, ast *tree, symbol_table **symbols, code_table **codes, long *ic, long *dc, long final_ic) {
    int success = 1, address = ADDRESS_BASE + *ic;
    if (tree->ast_opt == instruction) {
        /* first family of instructions with 2 operands */
        if (tree->line_type.inst_info.inst_data.num_args == 2) {
            success = check_label_error(0, tree, *symbols); /*check for errors with the source operand */
            success = check_label_error(1, tree, *symbols); /*check for errors with the dest operand */
            if (success) {
                success = first_fam(tree, symbols, codes, ic);
            }
            /* second family instruction with one operand */
        } else if (tree->line_type.inst_info.inst_data.num_args == 1) {
            success = check_label_error(1, tree, *symbols); /*check for errors with the dest operand */
            if(success) {
                success = sec_fam(tree, symbols, codes, ic);
            }
        } else { /* we have instruction from the third family with 0 operands */
            encode_inst(tree->line_type.inst_info.inst_data.inst_code, 0, 0, 0, codes, address);
            (*ic)++;
        }
        return success;
    } else if (tree->ast_opt == directive) {
        /* we want to print the data codes after the instruction, so we'll add the final ic to the address */
        address = ADDRESS_BASE + final_ic;
        if (tree->line_type.dir_info.dir_opt == dir_string) {
            encode_string(tree->line_type.dir_info.dir_operands.string, address + *dc, codes);
            *dc += strlen(tree->line_type.dir_info.dir_operands.string) + 1;
            address += *dc;
        } else if (tree->line_type.dir_info.dir_opt == dir_data) {
            if (check_data(tree->line_type.dir_info.dir_operands.string, tree, *symbols)) {
                encode_data(tree->line_type.dir_info.dir_operands.data_info.data,
                            tree->line_type.dir_info.dir_operands.data_info.data_counter, codes,
                            *dc + address);
                *dc += tree->line_type.dir_info.dir_operands.data_info.data_counter;
                address += *dc;
            }
        }
        success = 1;
    }
    return success;
}