/* Aviv Hadar, ID: 207750993
 * This program encodes the instrucion and data to encrypted base 4. */
#include "encode.h"

#define SRC 4 /*the bits of the src operand */
#define DEST 2 /*the bits of the dest operand */
#define REG_SRC 5 /*the bits of the register src operand */
#define OPCODE 6 /*the bits of the opcode value */

/**
 * This function converts decimal number to encrypted base 4.
 * @param num - the number we want to convert
 * @param codes - the head of the codes linked list
 * @param address - the address which we want to encode to.
 */
void num_to_base(int num, code_table **codes, int address) {
    char base[] = {'*', '#', '%', '!'};
    char code[MAX_BITS + 1]; /* +1 for '\0' */
    int is_negative = 0;
    int i = MAX_BITS - 1; /* filling from the end */
    if (num < 0) {
        is_negative = 1;
        num = (1 << BINARY) + num;
    }
    /* initialize the code array with the sign bit padding */
    memset(code, is_negative ? base[(BASE - 1)] : base[0], MAX_BITS);
    code[MAX_BITS] = '\0';
    /*convert to encrypted base 4 */
    while (num != 0 && i >= 0) {
        code[i] = base[num % BASE];
        num /= BASE;
        i--;
    }
    /*adding to code table */
    add_code_table(code, codes, address);
}

/**
 * This function adds the current code to the code table in its right place (ascending order).
 * @param code - the code in encrypted base 4
 * @param codes - the head of the codes linked list
 * @param address - the address which we want to encode to.
 */
void add_code_table(char *code, code_table **codes, int address) {
    code_table *tmp, *prev;
    code_table *new;
    new = (code_table *) malloc(sizeof(code_table));
    if (!new) {
        printf("error allocating memory, exiting.\n");
        free_codes(*codes);
        exit(1);
    }
    strncpy(new->code, code, MAX_BITS);
    new->address = address;
    new->next = NULL;
    /* If the list is empty or the new node should be the new head because it's instruction and its code should be
     * before the data section */
    if (*codes == NULL || (*codes)->address > new->address) {
        new->next = *codes;
        *codes = new;
        return;
    }
    /* moving through the list to find the correct place */
    prev = NULL;
    tmp = *codes;
    while (tmp != NULL && tmp->address <= new->address) {
        prev = tmp;
        tmp = tmp->next;
    }
    new->next = tmp;
    if (prev != NULL) {
        prev->next = new;
    }
}

/**
 * This function encode string by encoding each char as an integer (its ascii value)
 * @param str - the string we want to encode
 * @param codes - the head of the codes linked list
 * @param address - the address which we want to encode to.
 */
void encode_string(char *str, int address, code_table **codes) {
    int i, len  = strlen(str), letter;
    for (i = 0; i < len; i++) {
        letter = (int) str[i];
        num_to_base(letter, codes, address + i);
    }
    num_to_base(0, codes, address + len); /*adding additional 0 as '\0' at the end of string */
}

/**
 * encoding label and number are the same, we use this function for both.
 * 12 leftmost bits are the number (or address in case of label) and the left 2 a,r,e.
 * @param num - the number we want to code or the address of the label
 * @param are - the a,r,e value
 * @param codes - the head of the codes linked list
 * @param address - the address which we want to encode to.
 */
void encode_num(int num, int are, code_table **codes, int address) {
    int tmp = 0;
    tmp |= (num << TWO);
    tmp |= (are);
    num_to_base(tmp, codes, address);
}

/**
 * encoding data directive - a number at a time.
 * @param data - int array with all the data numbers.
 * @param counter - the counter of the array, how many numbers we need to encode.
 * @param codes - the head of the codes linked list.
 * @param address - the address which we want to encode to.
 */
void encode_data(int *data, int counter, code_table **codes, int address) {
    int i;
    for (i = 0; i < counter; i++) {
        num_to_base(data[i], codes, address + i);
    }
}

/**
 * This function encode the first instruction word using bitwise
 * @param opcode - the opcode of the instruction
 * @param are - the a,r,e of the instruction, when its the first word its 0.
 * @param src - the src operand address
 * @param dest - the dest operand address
 * @param codes - the head of the codes linked list.
 * @param address - the address which we want to encode to.
 */
void encode_inst(int opcode, int are, int src, int dest, code_table **codes, int address) {
    int num = 0;
    num |= are;
    num |= (dest << DEST);
    num |= (src << SRC);
    num |= (opcode << OPCODE);
    num_to_base(num, codes, address);
}

/**
 * This function encode the register additional word using bitwise
 * @param src - the src operand address
 * @param dest - the dest operand address
 * @param codes - the head of the codes linked list.
 * @param address - the address which we want to encode to.
 */
void encode_reg(int src, int dest, code_table **codes, int address) {
    int reg = 0;
    reg |= (dest << DEST);
    reg |= (src << REG_SRC);
    num_to_base(reg, codes, address);
}


/**
 * This function frees the memory allocated for the code table.
 * @param head - head of code table.
 */
void free_codes(code_table *head) {
    code_table *tmp;
    while (head != NULL) {
        tmp = head;
        head = head->next;
        free(tmp);
    }
}