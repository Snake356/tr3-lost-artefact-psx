/*
 * Library of OS-dependant functions used by TR3PCPSX.
 *
 * The abilities to clear the terminal and use colours in the terminal are
 * optional, and to disable them, just leave CLEARTERMINAL and COLOURTERMINAL
 * undefined (by removing lines 16 and 17 from this file).
 */
#ifndef TR3PCPSX_OSDEP_H_
#define TR3PCPSX_OSDEP_H_

/* File Inclusions */
#include "params.h"   /* Handles command-line parameters */
#include "fixedint.h" /* Definition of fixed-size integers */

/* Macro definitions */
#define CLEARTERMINAL printf("\x1b[0;0H\x1b[2J"); /* Clears the terminal */
#define COLOURTERMINAL /* Whether to use colours in the terminal */

#ifdef COLOURTERMINAL
	#define PRINT_DONE if (isflag(FLAGS_VERBOSE)) \
	        printf("\x1b[32mDone\x1b[0m\n");
	#define PRINT_FAILED if (isflag(FLAGS_VERBOSE)) \
	        printf("\x1b[31mFailed\x1b[0m\n");
	#define PRINT_WARNING printf("\x1b[33mWarning:\x1b[0m ");
#else
	#define PRINT_DONE if (isflag(FLAGS_VERBOSE)) printf("Done\n");
	#define PRINT_FAILED if (isflag(FLAGS_VERBOSE)) printf("Failed\n");
	#define PRINT_WARNING printf("Warning: ");
#endif

/* Function Declarations */
int truncateFile(char *filePath, INTU32 offset);

#endif
