/*
 * Library of OS-dependant functions used by TRMOD
 *
 * The abilities to clear the terminal and use colours in the terminal are
 * optional, and to disable them, just leave CLEARTERMINAL and COLOURTERMINAL
 * undefined (by removing lines 16 and 17 from this file).
 */
#ifndef TRMOD_OSDEP_H_
#define TRMOD_OSDEP_H_

/* File Inclusions */
#include "fixedint.h" /* Definition of fixed-size integers */

/* Macro definitions */
#define CLEARTERMINAL printf("\x1b[0;0H\x1b[2J"); /* Clears the terminal */
#define COLOURTERMINAL /* Whether to use colours in the terminal */

/* Function Declarations */
int truncateFile(char *filePath, INTU32 offset);

#endif
