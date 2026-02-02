

/* Aviv Hadar, ID: 207750993
 * This program runs the preprocessor - spreading macros, deleting empty lines and comment lines.
 * at the end of the file we'll create an am file and return its name.
 * in this file we are not checking for errors in the macro, if there are any they will be discovered in
 * the first pass (according to the course booklet). */

#include "preprocessor.h"
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

/**
 * This function adds the macros name to the symbol table so we won't have label with macro name.
 * @param symbols - head of symbols linked list
 * @param macros - head of macro linked list
 */
void add_macro_to_symbol(symbol_table **symbols, macro *macros) {
    macro *tmp = macros;
    while (tmp != NULL) {
        add_symbol(symbols, tmp->name, macro_t,
                   macro_p, 0, 0, 0, 0);
        tmp = tmp->next;
    }
}

/**
 * This function checks if the macro name is new or used before
 * @param name - the new macro name
 * @param head - head of macro linked list
 * @return 1 if we already have macro with this name or 0 if the name is new
 */
int check_macro_name(char *name, macro *head) {
    macro *tmp = head;
    while (tmp != NULL) {
        if (strcmp(tmp->name, name) == 0) {
            return 1; /*we already have a macro with this name */
        }
        tmp = tmp->next;
    }
    return 0;
}

/**
 * This function creates a new macro node with its data
 * @param name - the new macro name
 * @param data - the new macro data
 * @param head - head of macro linked list
 * @return the new macro that was created
 */
macro *create_macro(char *name, char *data, macro *head) {
    macro *new = NULL;
    int len_name, len_data;
    len_name = strlen(name);
    len_data = strlen(data);
    char *tmp_name, *tmp_data;
    if (check_macro_name(name, head) == 0) {
        new = (macro *) malloc(sizeof(macro));
        if (!new) {
            printf("error in allocating memory for new macro, exiting.\n");
            exit(1);
        }
        tmp_name = (char *) malloc((sizeof(char)) * (len_name + 1));  /* Allocate memory for name */
        if (!tmp_name) {
            printf("error in allocating memory for new macro name.\n");
            exit(1);
        }
        strncpy(tmp_name, name, len_name);
        tmp_name[len_name] = '\0';
        new->name = tmp_name;
        tmp_data = (char *) malloc((sizeof(char)) * (len_data + 1));
        if (!tmp_data) {
            printf("error in allocating memory for new macro data, exiting.\n");
            free(tmp_name);
            exit(1);
        }
        strncpy(tmp_data, data, len_data);
        tmp_data[len_data] = '\0';
        new->data = tmp_data;
        new->next = NULL;
    }
    return new;
}

/**
 * This function adds macro to the macros linked list
 * @param head - the head of the macros linked list
 * @param new - the new macro we want to add to the list
 */
void add_macro_list(macro **head, macro *new) {
    if (*head == NULL) {
        *head = new;
    } else {
        macro *tmp = *head;
        while (tmp->next != NULL) {
            tmp = tmp->next;
        }
        tmp->next = new;
    }
}

/**
 * This function checks if str is empty or has only white chars.
 * @param str - the string we want to check if it's empty.
 * @return 1 if the string is empty, 0 if not
 */
int is_blank(char *str) {
    while (*str != '\0') {
        if (!isspace(*str)) {
            return 0;
        }
        str++;
    }
    return 1;
}

/**
 * This function checks if the string starts with a comment char ';'
 * @param str - the string we want to check
 * @return 1 if this is a comment, 0 otherwise
 */
int is_comment(char *str) {
    if (*str == ';') {
        return 1;
    }
    return 0;
}

/**
 * This function frees the memory of a given macro
 * @param m - the macro we want to free its memory
 */
void free_macro(macro *m) {
    free(m->data);
    free(m->name);
}

/**
 * This function frees the macro table using free_macro fun
 * @param head - head of macros linked list
 */
void free_macro_table(macro *head) {
    macro *tmp = head, *next_node;
    while (tmp != NULL) {
        next_node = tmp->next;
        free_macro(tmp);
        free(tmp);
        tmp = next_node;
    }
}

/**
 * This function checks if there is a macro defined in this line with the definition of "mcr"
 * @param str - the line we want to check for macro start
 * @return 1 if we have a macro start, 0 otherwise
 */
int is_macro(char *str) {
    int i = 0;
    skip_white(str, &i);
    if(strncmp(str+i, "mcr",3)==0){
        printf("now rest line is %s\n", str+i);
        i+=3;
        if(isspace(str[i])){
            return 1;
        }
    }

    return 0;
}

/**
 * This function checks if there is a macro end in this line with the definition of "endmcr"
 * @param str - the line we want to check for macro end
 * @return 1 if we have a macro end, 0 otherwise
 */
int end_macro(const char *str) {
    char w[] = "endmcr";
    if (strstr(str, w)) {
        return 1;
    }
    return 0;
}

/**
 * This function preprocessing the file - spreading the macros and deleting empty and comment lines.
 * if everything is ok we'll create an am file with the spread macros
 * @param file_name - the file we want to preprocess
 * @param symbols - head to the symbols linked list
 * @return the am file name
 */
char *preprocess_file(char *file_name, symbol_table **symbols) {
    FILE *fp, *final;
    size_t slength, data_size = 0;
    int count = 0, flag, len, i, mc = 0;
    unsigned long int filename_size = strlen(file_name);
    macro *head= NULL, *tmp, *new;
    char *as_name, *am_name, *macro_data = NULL, *macro_name = NULL, str[MAX_LEN];
    /*creating am file and as file */
    as_name = malloc(filename_size + strlen(AS_END) + 1);
    if (!as_name) {
        printf("error in allocating memory for file %s, exiting.\n", file_name);
        exit(1);
    }
    am_name = malloc(filename_size + strlen(AM_END) + 1);
    if (!am_name) {
        printf("error in allocating memory for file name %s, exiting.\n", file_name);
        exit(1);
    }
    strcpy(as_name, file_name);
    strcat(as_name, AS_END);
    strcpy(am_name, file_name);
    strcat(am_name, AM_END);
    /*open the file with the as ending, otherwise it's an error*/
    fp = fopen(as_name, "r");
    final = fopen(am_name, "w");
    /*read the as file and write to the am file*/
    if (fp) {
        while ((fgets(str, MAX_LEN, fp))) {
            flag = 0; /*in these line we have no errors */
            /*if this is a blank line or a comment line we'll skip it */
            if (is_blank(str) == 1 || is_comment(str) == 1) {
                continue;
            } else if (is_macro(str) == 1) {/* we found the beginning of a macro */
                i = 0;
                skip_white(str, &i);
                i += MCR; /*skipping the 'mcr' name*/
                skip_white(str, &i);
                len = i;
                while (str[len] != '\0' && !isspace(str[len])){
                    len++;
                }
                len = len - i;
                macro_name = (char *) malloc(sizeof(char) * (len + 1));
                if(!macro_name){
                    printf("error in allocating memory for macro name, exiting.\n");
                    exit(1);
                }
                strncpy(macro_name, str + i, len);
                macro_name[len] = '\0';
                printf("the name of the macro is :%s:\n", macro_name);
                /*while we are not getting the end of the macro - reading the macro data */
                while (fgets(str, MAX_LEN, fp) && end_macro(str) != 1) {
                    slength = strlen(str);
                    count++;
                    char *temp = realloc(macro_data, data_size + slength + 1); 
                    if (!temp) {
                        printf("error in allocating memory for macro data.\n");
                        free(macro_data); 
                        exit(1);
                    }
                    macro_data = temp; 
                    strcpy(macro_data + data_size, str); 
                    data_size += slength; 
                }
                if (end_macro(str) == 1) {
                    printf("we got end macro \n");
                    /* at the end of the macro we'll create it */
                    new = create_macro(macro_name, macro_data, head);
                    printf("after new name :%s: data :%s:\n", new->name, new->data);
                    add_macro_list(&head, new);
                    macro *ttt = head;
                    if(head == NULL){
                        printf("head is null");
                    }
                    while(ttt){
                        printf(" name :%s: data :%s:\n", ttt->name, ttt->data);
                        ttt=ttt->next;
                    }

                    /*free and reset allocated memory after use */
                    free(macro_name);
                    free(macro_data);
                    macro_data = NULL;
                    printf("after free name :%s: data :%s:\n", new->name, new->data);
                    data_size = 0;
                }
            } else {
                    tmp = head;
                    while (tmp != NULL) {
                        printf("now %s\n", tmp->name);
                        if (strstr(str, tmp->name)) {
                            /*making sure it's not a label with the same name - we'll handle this in the first pass as an error */
                            if (!(strstr(str, ":"))) {
                                printf("macro name is %s\n", tmp->name);
                                printf("macro data is: %s\n", tmp->data);
                                fprintf(final, "%s", tmp->data);
                                flag = 1; /* we need to spread the macro */
                            }
                        }
                        tmp = tmp->next;
                    }
                if(flag ==  0){
                 /*no macro spreading, writing the line as it is */
                    fprintf(final, "%s", str);
                }
            }
        }
        fclose(fp);
        fclose(final);
        add_macro_to_symbol(symbols, head); /*adding the macro names to the symbol table */
        free_macro_table(head);
        free(as_name);
        return am_name;
    } else {
        printf("not able to open the file %s\n", file_name);
        return NULL;
    }
}



