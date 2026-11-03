#ifndef UTIL_H_
#define UTIL_H_

/* File Inclusions */
#include "fixedint.h" /* Definition of fixed-size integers */

/* Function Declarations */
int insertbytes(FILE *level, INTU32 offset, INTU32 length);
int removebytes(FILE *level, char *levelPath, INTU32 offset, INTU32 length);
INT32 strtoint(char *string);

#endif
