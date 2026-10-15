#ifndef TR3PCPSX_UTIL_H_
#define TR3PCPSX_UTIL_H_

/* File Inclusions */
#include <stdio.h>    /* Standard I/O */
#include "structs.h"  /* Definition of structs */
#include "fixedint.h" /* Definition of fixed-size integers */

/* Function Declarations */
int insertbytes(FILE *level, INTU32 offset, INTU32 length);
int removebytes(FILE *level, char *levelPath, INTU32 offset, INTU32 length);
int copybytes(FILE *file1, FILE *file2, INTU32 f1_offset, INTU32 f2_offset,
              INTU32 numBytes);
int zerobytes(FILE *file, INTU32 offset, INTU32 length);
int patternbytes(FILE *file, INTU32 offset, INTU32 length,
                 INTU8 *pattern, INTU32 patternlen);
void level_info_add(struct level_info *in, INTU32 offset, INTU32 amount);
void level_info_sub(struct level_info *in, INTU32 offset, INTU32 amount);
int compareString(char *a, char *b, size_t length);

#endif
