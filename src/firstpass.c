/* Aviv Hadar, ID: 207750993
 * This program runs the first pass on the assembler checking the ast values. */

#include "firstpass.h"
#include "encode.h"


/**
 * This function checks that the data array is valid:
 * @param line - the string of the data array
 * @param tree - the ast tree
 * @param symbols - the head of the symbols linked list
 * @return 1 if everything is ok, 0 if found error
 */
int check_data(char *line, ast *tree, symbol_table *symbols) {
    int num, found = 0, comma = 0, i = 0, def_length;
    char define_check[MAX_LABEL];
    symbol_table *symbol;
    while (line[i] != '\0' && line[i] != '\n' && line[i] != EOF) {
        skip_white(line, &i);
        /* if we haven't found any number and found a comma it's an error */
        if (found == 0 && line[i] == ',') {
            tree->ast_opt = ast_syntax_error;
            strcpy(tree->ast_errors, "wrong data syntax - there is a comma before the numbers");
            return 0;
        }
        skip_white(line, &i);
        /* if we already found comma and now we are finding another one it's an error */
        if (comma == 1 && line[i] == ',') {
            tree->ast_opt = ast_syntax_error;
            strcpy(tree->ast_errors, "wrong data syntax - two commas in a row");
            return 0;
        }
        /* if we found comma and the rest onf the line is empty it's an error */
        if (line[i] == ',') {
            comma = 1;
            i++;
            if (is_blank(line + i)) {
                tree->ast_opt = ast_syntax_error;
                strcpy(tree->ast_errors, "wrong data syntax - the data ends with comma instead of number");
                return 0;
            }
            continue;
        }
        def_length = 0;
        /* check if the argument is a define symbol or a decimal number */
        while (line[i] != ',' && line[i] != ' ' && line[i] != '\t' && line[i] != '\n' && line[i] != '\0') {
            define_check[def_length++] = line[i++];
        }
        define_check[def_length] = '\0';
        while (def_length > 0 && isspace(define_check[def_length - 1])) {
            define_check[--def_length] = '\0';
        }
        /* check if the define_check is a defined symbol */
        symbol = check_symbol_exist(symbols, define_check);
        if (symbol && symbol->type == mdefine) {
            num = symbol->symbol_value;
        } else {
            if (is_decimal_integer(define_check)) {
                num = atoi(define_check);
            } else {
                tree->ast_opt = ast_syntax_error;
                strcpy(tree->ast_errors, "data has an invalid number");
                return 0;
            }
        }
        tree->line_type.dir_info.dir_operands.data_info.data[found] = num;
        found++;
        tree->line_type.dir_info.dir_operands.data_info.data_counter = found;
        while (line[i] == ' ' || line[i] == '\t') {
            i++;
        }
        comma = 0;
        /* we checked all numbers and the string ends with a number so we're good */
        if (is_blank(line + i)) {
            tree->line_type.dir_info.dir_operands.string[0] = '\0';
            return 1;
        }
    }
    /* if we have empty data we have nothing to code, so it's an error */
    if (found == 0) {
        tree->ast_opt = ast_syntax_error;
        strcpy(tree->ast_errors, "empty data");
        return 0;
    }
    /*if we haven't found any error after going through all the data array we'll return 1 */
    return 1;
}

/**
 * This function handles define declare in first pass, making sure its name is new and valid.
 * @param tree - the tree with the parsing data
 * @param symbols - head to the symbols linked list
 * @return 1 if everything was ok, 0 if we found error
 */
int handle_def(ast *tree, symbol_table *symbols) {
    if (check_symbol_exist(symbols, tree->line_type.def_info.def_name)) {
        printf("Error in the line: %s\n", tree->ast_errors); /*maybe func that will print the errors? */
        return 0;
    } else {
        if (check_valid_label(tree->line_type.def_info.def_name)) {
            add_symbol(&symbols, tree->line_type.def_info.def_name, mdefine, general,
                       tree->line_type.def_info.def_value, 0, 0, 0);
            return 1;
        } else {
            strcpy(tree->ast_errors, "error with define name\n"); /*maybe func that will print the errors? */
            return 0;
        }
    }
}

/**
 * This function handles the first pass if we found a label in the line, it checks for double declaration
 * and change the address when needed.
 * @param tree - the tree with the parsing data
 * @param symbols - head to the symbols linked list
 * @param ic - the instruction counter
 * @param dc - the data counter
 * @param l_place - if we need the current place to be entry or general
 * @return 1 if everything was ok, 0 if we found error
 */
int handle_label(ast *tree, symbol_table *symbols, long *ic, long *dc, int *l_place) {
    symbol_table *local_symbol = NULL;
    local_symbol = check_symbol_exist(symbols, tree->label_name);
    if (local_symbol != NULL) {
        /* if we found a symbel that already exist because it was declared as .entry ot .extern we'll update
         * its use address */
        if (local_symbol->use_address == 0 && (local_symbol->place == externa || local_symbol->place == entry)) {
            local_symbol->use_address = *dc + *ic + ADDRESS_BASE;
        }
        /* if we are trying to add a symbol that has the same name as a macro it's an error */
        if (local_symbol->place == macro_p) {
            strcpy(tree->ast_errors, "label name already exists as macro");
            tree->ast_opt = ast_syntax_error;
            return 0;
        }
        /* handle redefine label cases if we declare the same label more than once it's an error*/
        if (local_symbol->place == externa) {
            strcpy(tree->ast_errors, "redefine of label.");
            tree->ast_opt = ast_syntax_error;
            return 0;
        } else if (local_symbol->place == entry && local_symbol->type != gen) {
            strcpy(tree->ast_errors, "redefine of label.");
            tree->ast_opt = ast_syntax_error;
            return 0;
        } else if (local_symbol->place == general && local_symbol->type != gen) {
            strcpy(tree->ast_errors, "redefine of label.");
            tree->ast_opt = ast_syntax_error;
            return 0;
        } /* if we want to add new label that was declared as .entry its ok, we'll change its place */
        else if (local_symbol->place == entry && local_symbol->type == gen) {
            *l_place = entry;
        } else {
            /* if the symbol was previously extern or entry with address 0, update it with the new address */
            if (local_symbol->use_address == 0 && (local_symbol->place == externa
                                                   || local_symbol->place == entry)) {
                local_symbol->use_address = *dc + *ic + ADDRESS_BASE;
            }
        }
    }
    return 1;
}

/**
 * This function handles the first pass of the assembler on the file.
 * using the tree it checks the type of line and its parameters.
 * @param line - the line we are currently checking
 * @param tree - the tree with the parsing data
 * @param symbols - head to the symbols linked list
 * @param ic - the instruction counter
 * @param dc - the data counter
 * @param codes - head to the codes linked list
 * @return 1 if everything was ok, 0 if we found error
 */
int first_pass(char *line, ast *tree, symbol_table *symbols, long *ic, long *dc, code_table **codes) {
    int ic_count, success = 0, lab_flag = 0, l_place = 0;
    symbol_table *local_symbol = NULL;
    if (tree->ast_opt == ast_syntax_error) {
        return success;
    }
    if (tree->ast_opt == def) {
        success = handle_def(tree, symbols);
        return success;
    }
    if (tree->label_name[0] != '\0') {
        lab_flag = 1;
        success = handle_label(tree, symbols, ic, dc, &l_place);
        if (!success) {
            return success;
        }
    }
    if (tree->ast_opt == directive) {
        if (tree->line_type.dir_info.dir_opt == dir_extern) {
            if (tree->line_type.dir_info.dir_operands.label_name[0] != '\0') {
                local_symbol = check_symbol_exist(symbols, tree->line_type.dir_info.dir_operands.label_name);
                if (local_symbol) {
                    strcpy(tree->ast_errors, "redefine of label.");
                    tree->ast_opt = ast_syntax_error;
                    return 0;
                } else {
                    add_symbol(&symbols, tree->line_type.dir_info.dir_operands.label_name,
                               gen, externa, 0, 0, 0, 0);
                }
            }
            success = 1;
        } else if (tree->line_type.dir_info.dir_opt == dir_entry) {
            if (tree->line_type.dir_info.dir_operands.label_name[0] != '\0') {
                local_symbol = check_symbol_exist(symbols, tree->line_type.dir_info.dir_operands.label_name);
                if (local_symbol) {
                    if (local_symbol->type != gen && local_symbol->place != entry) {
                        local_symbol->place = entry;
                        return 1;
                    }
                    strcpy(tree->ast_errors, "redefine of label.");
                    tree->ast_opt = ast_syntax_error;
                    return 0;
                } else {
                    add_symbol(&symbols, tree->line_type.dir_info.dir_operands.label_name,
                               gen, entry, 0, 0, 0, 0);
                }
            }
            success = 1;
        } else if (tree->line_type.dir_info.dir_opt == dir_string) {
            if (lab_flag) {
                add_symbol(&symbols, tree->label_name, data, l_place,
                           *dc, *dc + *ic + ADDRESS_BASE, *dc + *ic + ADDRESS_BASE, 0);
            }
            *dc += strlen(tree->line_type.dir_info.dir_operands.string) + 1; /* +1 for '\0' */
            success = 1;
        } else if (tree->line_type.dir_info.dir_opt == dir_data) {
            if (check_data(tree->line_type.dir_info.dir_operands.string, tree, symbols)) {
                if (lab_flag) {
                    add_symbol(&symbols, tree->label_name, data, l_place, *dc,
                               *dc + *ic + ADDRESS_BASE, *dc + *ic + ADDRESS_BASE, 0);
                }
                *dc += tree->line_type.dir_info.dir_operands.data_info.data_counter;
                success = 1;
            }
        }
        return success;
    } else if (tree->ast_opt == instruction) {
        if (lab_flag) {
            add_symbol(&symbols, tree->label_name, code, l_place, 0,
                       ADDRESS_BASE + *ic, ADDRESS_BASE + *ic, 0);
        }
        ic_count = increase_ic_count(tree);
        *ic += ic_count;
        success = 1;
        return success;
    }
    lab_flag = 0;
    return success;
}
