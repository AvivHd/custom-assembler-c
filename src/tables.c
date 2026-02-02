/* Aviv Hadar, ID: 207750993
 * This program handles the main table of the program - the symbol table.*/

#include "tables.h"

/**
 * This function adds a symbol and its data to the symbol table
 * @param head - the head of the linked list symbol table
 * @param new_label - the name of the new symbol
 * @param type - the type of the new symbol
 * @param place - the place of the new symbol - A,R,E
 * @param val - the value of the new symbol if its define
 * @param d_address -in case it was added as extern or entry this is its first label
 * @param u_address - this is the label where the symbol was used in
 * @param is_ext - if the symbol is extrn.
 */
void add_symbol(symbol_table **head, char *new_label, symbol_type type, symbol_place place,
                int val, int d_address, int u_address, int is_ext) {
    symbol_table *tmp = *head;
    symbol_table *new;
    new = (symbol_table *) malloc(sizeof(symbol_table));
    if (!new) {
        printf("Error allocating new symbol, exiting.\n");
        free_symbols(*head);
        exit(1);
    }
    strcpy(new->symbol_name, new_label);
    new->type = type;
    new->next = NULL;
    new->place = place;
    new->symbol_value = val;
    new->declare_address = d_address;
    new->use_address = u_address;
    new->is_ext = is_ext;
    if (*head == NULL) {
        *head = new;
        return;
    }
    while (tmp->next != NULL) {
        tmp = tmp->next;
    }
    tmp->next = new;
}

/**This function gets a name and return a node with the symbol data if this name exits or NULL if its
 * a new symbol.
 * @param head - the head of the symbols linked list.
 * @param label_name - the name of the symbol we are looking for
 * @return symbol node if exists, else NULL
 */
symbol_table *check_symbol_exist(symbol_table *head, char *label_name) {
    symbol_table *tmp = head;
    while (tmp != NULL) {
        if (strcmp(label_name, tmp->symbol_name) == 0) {
            return tmp;
        }
        tmp = tmp->next;
    }
    return NULL;
}

/**
 * This function creates the symbol linked list
 * @return a pointer to the head of the list
 */
symbol_table *create_symbol_table() {
    symbol_table *symbols;
    symbols = (symbol_table *) malloc(sizeof(symbol_table));
    if (!symbols) {
        exit(1);
    }
    return symbols;
}

/**
 * This function free all the symbol linked list and its data.
 * @param head - the head of the symbol linked list
 */
void free_symbols(symbol_table *head) {
    symbol_table *tmp;
    while (head != NULL) {
        tmp = head;
        head = head->next;
        free(tmp);
    }
}