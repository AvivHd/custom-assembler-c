/* Aviv Hadar, ID: 207750993
 * This program writes the output files that needed. */

#include "output.h"

/**
 *  * This function writes to entries file
 * @param ent_name - the file name with ent ending
 * @param symbols - head to symbol table linked list
 * @param codes - head to the code table linked list
 */
void write_ent(char *ent_name, symbol_table **symbols, code_table **codes) {
    symbol_table *sym_tmp = *symbols;
    FILE *ent_f;
    ent_f = fopen(ent_name, "w");
    if (!ent_f) {
        printf("Error, couldn't open new file for ent file.\n");
        /* if we can't open the file we'll free the allocated memory and exit the program */
        free_symbols(*symbols);
        free_codes(*codes);
        exit(1);
    }
    while (sym_tmp != NULL) {
        if (sym_tmp->place == entry && sym_tmp->type != gen) {
            fprintf(ent_f, "%s\t%04d\n", sym_tmp->symbol_name, sym_tmp->use_address);
        }
        sym_tmp = sym_tmp->next;
    }
    fclose(ent_f);
}

/**
 * This function writes to externals file
 * @param ext_name - the file name with ext ending
 * @param symbols - head to symbol table linked list
 * @param codes - head to the code table linked list
 */
void write_ext(char *ext_name, symbol_table **symbols, code_table **codes) {
    symbol_table *sym_tmp = *symbols;
    FILE *ext_f;
    sym_tmp = *symbols;
    ext_f = fopen(ext_name, "w");
    if (!ext_f) {
        /* if we can't open the file we'll free the allocated memory and exit the program */
        printf("Error, couldn't open new file for ext file\n");
        free_symbols(*symbols);
        free_codes(*codes);
        exit(1);
    }
    while (sym_tmp != NULL) {
        if (sym_tmp->place == externa && sym_tmp->type > 0) {
            fprintf(ext_f, "%s\t%04d\n", sym_tmp->symbol_name, sym_tmp->use_address);
        }
        sym_tmp = sym_tmp->next;
    }
    fclose(ext_f);
}

/**
 * This function writes to object file
 * @param ob_name - the file name with ob ending
 * @param symbols - head to symbol table linked list
 * @param codes - head to the code table linked list
 * @param ic - instruction counter
 * @param dc - data counter
 */
void write_ob(char *ob_name, symbol_table **symbols, code_table **codes, long ic, long dc) {
    code_table *tmp = *codes;
    FILE *ob_f;
    ob_f = fopen(ob_name, "w");
    if (!ob_f) {
        /* if we can't open the file we'll free the allocated memory and exit the program */
        printf("Error, couldn't open new file for object file.\n");
        free_symbols(*symbols);
        free_codes(*codes);
        exit(1);
    }
    fprintf(ob_f, "%ld %ld\n", ic, dc);
    while (tmp != NULL) {
        fprintf(ob_f, "%04d %s\n", tmp->address, tmp->code);
        tmp = tmp->next;
    }
    fclose(ob_f);
}

/**
 * This function checks which files are required and accordingly writes the output to the right file
 * @param name - the name of the file
 * @param codes - head to the code table linked list
 * @param ent - if we need entry file or not
 * @param ext - if we need ext file or not
 * @param symbols - head to symbol table linked list
 * @param ic - instruction counter
 * @param dc - data counter
 */
void write_files(char *name, code_table **codes, int ent, int ext, symbol_table **symbols, long ic, long dc) {
    int i = 0;
    char ob_name[MAX_LEN], ent_name[MAX_LEN], ext_name[MAX_LEN], tmp_name[MAX_LEN];
    ob_name[0] = '\0';
    ent_name[0] = '\0';
    ext_name[0] = '\0';
    while (name[i] != '.') {
        i++;
    }
    /*copying the name of the file */
    strncpy(tmp_name, name, i);
    tmp_name[i] = '\0';
    /*creating ob file name - if we have no errors we always have ob file */
    strcat(ob_name, tmp_name);
    strcat(ob_name, OB_FILE_END);
    /* if we have entries we'll create ent file name */
    if (ent) {
        strcat(ent_name, tmp_name);
        strcat(ent_name, ENT_FILE_END);
    }
    /* if we have externals we'll create ext file name */
    if (ext) {
        strcat(ext_name, tmp_name);
        strcat(ext_name, EXT_FILE_END);
    }
    write_ob(ob_name, symbols, codes, ic, dc);
    if (ent) {
        write_ent(ent_name, symbols, codes);
    }
    if (ext) {
        write_ext(ext_name, symbols, codes);
    }
}


