/*
 * Library of OS-dependant functions used by TRMOD
 */
#ifndef TRMOD_OSDEP_H_
#define TRMOD_OSDEP_H_

/* File Inclusions */
#include "fixedint.h" /* Definition of fixed-size integers */

/* Function Declarations */
int truncateFile(char *filePath, uint32 offset);

#endif
