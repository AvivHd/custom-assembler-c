/* Aviv Hadar, ID: 207750993
 * This program is header of globals we need through the project. */


#ifndef MAMAN14_PROJ_GLOBALS_H
#define MAMAN14_PROJ_GLOBALS_H

#define MAX_STR 100 /*maximum str length allow is 81, we need extra to check if the line is valid */
#define MAX_LEN 81 /*maximum str length allowed is 80 plus \n - 81 */
#define MAX_LABEL 31 /*maximum label, macro and define length allowed is 30 plus '\0' - 31 */
#define INST_AMOUNT 16 /*The number of instructions we have in the program */
#define NUM_REGISTERS 9 /*The number of registers we have in the program */
#define ADDRESS_BASE 100 /*The base address from which we start counting the labels */
#define BASE 4 /*The base of our encrypted code */
#define MAX_BITS 7 /*The number of bits in code */
#define INST_MAX 4 /*name of instruction is 3 letters + '\0' */
#define MAX_MEMORY 4096 /*maximum memory in the imaginary computer */
#define BINARY 14 /*number of bits in binary for 7 bits in 4 base */
#define DIR_OPT 4 /*the number of directive options */
#define TWO 2

#endif
