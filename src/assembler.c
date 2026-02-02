/* Aviv Hadar, ID: 207750993
 * This program is the assembler of our program.
 * It iterate the files and runs the preprocessor, first pass and second pass. */


#include "assembler.h"
#include "preprocessor.h"


/**
 * This function checks if there are file names in the command line.
 * @param argc - number or arguments in the command line.
 * @return int - 1 if we have enough arguments, 0 otherwise.
 */
int check_arguments(int argc) {
    if (argc < 2) {
        printf("There are no file names in the command, exiting.\n");
        return 0;
    }
    return 1;
}

/**
 * This function checks if there are used entry labels in the file.
 * @param symbols - head of the symbols linked list.
 * @return int 1 - if there are, 0 otherwise.
 */
int check_entry(symbol_table *symbols) {
    symbol_table *tmp = symbols;
    while (tmp != NULL) {
        /* we check if the entry label was also used and not only declared */
        if (tmp->place == entry && (tmp->type == code || tmp->type == data)) {
            return 1;
        }
        tmp = tmp->next;
    }
    return 0;
}

/**
 * This function checks if there are used extern labels in the file.
 * @param symbols - head of the symbols linked list.
 * @return int 1 - if there are, 0 otherwise.
 */
int check_ext(symbol_table *head) {
    symbol_table *tmp = head;
    while (tmp != NULL) {
        if (tmp->place == externa && tmp->type == code) {
            return 1;
        }
        tmp = tmp->next;
    }
    return 0;
}


/**
 * This function is running the second pass on the files and prints the errors if there are any
 * @param file_name - the name of the file we are scanning.
 * @param symbols - the head of the symbols linked list.
 * @param codes -the head of the codes in encrypted base 4 linked list.
 * @param tree - the ast tree.
 * @param first - int 1 if the first pass was successful, 0 otherwise.
 * @param final_ic - the IC counter we got after the first pass.
 */
void run_sec(char *file_name, symbol_table **symbols, code_table **codes, ast *tree, int first, long final_ic) {
    int ext, ent, success_sec = 0;
    char tmp_line[MAX_LEN];
    FILE *fptr = fopen(file_name, "r");
    long ic = 0, dc = 0, line_count;
    if (!fptr) {
        printf("file could not be opened, error happened.\n");
        return;
    }
    line_count = 0;
    while (fgets(tmp_line, MAX_LEN, fptr)) {
        line_count++;
        /*we are getting the parsing of the current line */
        *tree = get_ast(tmp_line, *symbols);
        /* we are passing the parser data to the second pass function */
        success_sec = second_pass(tmp_line, tree, symbols, codes, &ic, &dc, final_ic);
    }
    /* if the first pass and the second pass were ok we'll check which files we need to write and to it */
    if (first && success_sec) {
        ent = check_entry(*symbols);
        ext = check_ext(*symbols);
        write_files(file_name, codes, ent, ext, symbols, ic, dc);
    }
    fclose(fptr);
    free_symbols(*symbols);
    free_codes(*codes);
    free(file_name);
}


/**
 * This function is running the first pass on the files and prints the errors if there are any
 * @param file_name - the name of the file we are scanning.
 * @param symbols - the head of the symbols linked list.
 */
void run_file(char *file_name, symbol_table **symbols) {
    code_table *codes = NULL;
    ast tree;
    int success_first = 0, continue_flag = 1;
    long ic = 0, dc = 0, line_count = 0;
    char tmp_line[MAX_STR];
    FILE *fptr = fopen(file_name, "r");
    if (!fptr) {
        printf("file could not be opened, error happened.\n");
        return;
    }
    while (fgets(tmp_line, MAX_STR, fptr) != NULL) {
        /* there is a maximum length for line - 81 chars, if the line is more than that we'll print an error */
        if (strlen(tmp_line) > MAX_LEN) {
            printf("Error in file %s, line %ld: line is too long\n", file_name, line_count);
            continue_flag = 0;
        }
        /* in our imaginary computer we have only 4096 bytes, if we are out of memory we'll print an error */
        if (ic + dc >= MAX_MEMORY) {
            printf("Error, out of memory in the imaginary computer\n");
            continue_flag = 0;
        }
        line_count++;
        tree = get_ast(tmp_line, *symbols);
        /* if we got any errors during the ast function we'll print them */
        if (tree.ast_opt == ast_syntax_error) {
            printf("Error in file %s, line %ld: %s\n", file_name, line_count, tree.ast_errors);
            continue_flag = 0;
        } else {
            success_first = first_pass(tmp_line, &tree, *symbols, &ic, &dc, &codes);
            /* if we got any errors during the first pass function they will be added to the tree, we'll print them */
            if (tree.ast_opt == ast_syntax_error) {
                printf("Error in file %s, line %ld: %s\n", file_name, line_count, tree.ast_errors);
                continue_flag = 0;
            }
            if (!success_first) {
                continue_flag = 0;
            }
        }
    }
    rewind(fptr);
    fclose(fptr);
    run_sec(file_name, symbols, &codes, &tree, continue_flag, ic);
}


/**
 * This function iterates through the files from the command line and send them to the preprocessor and first pass.
 * @param argc - the number of files we got from the command line.
 * @param argv - char array with the names of the files.
 * @param symbols - the head of the symbols linked list.
 */
void iterate_files(int argc, char *argv[], symbol_table **symbols) {
    char *file_name, *file_am;
    int len, i;
    for (i = 1; i < argc; i++) {
        len = strlen(argv[i]);
        file_name = (char *) malloc(sizeof(char) * (len + 1));
        if (!file_name) {
            printf("error allocating memory for file name, exiting\n");
            free(file_name);
            free_symbols(*symbols);
            exit(1);
        }
        strcpy(file_name, argv[i]);
        /*reading the file through the preprocessor */
        file_am = preprocess_file(file_name, symbols);
        free(file_name);
        if (!file_am) {
            printf("could not open file %s, continue to next file or exiting if last.\n", argv[i]);
        } else {/* if everything is ok we'll run the first and second passes */
            run_file(file_am, symbols);
        }
    }
}


/**
 * This is the main function of the program, it calls the other function by order to process the files.
 * @param argc - the number of files we got from the command line.
 * @param argv - char array with the names of the files.
 * @return int
 */
int main(int argc, char *argv[]) {
    if (check_arguments(argc)) {
        symbol_table *symbols = create_symbol_table();
        iterate_files(argc, argv, &symbols);
    }
    return 1;
}
