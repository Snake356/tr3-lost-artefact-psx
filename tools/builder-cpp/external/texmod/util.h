#ifndef TRMOD_UTIL_H_
#define TRMOD_UTIL_H_

/* File Inclusions */
#include <stdio.h>    /* Standard I/O */
#include "structs.h"  /* Definition of structs */
#include "fixedint.h" /* Definition of fixed-size integers */
#include "errors.h"   /* Error Handling for TRMOD */

/* Function Declarations */
void insertbytes(FILE *file, char *path, INTU32 offset, INTU32 length,
                 struct error *error);
void removebytes(FILE *file, char *path, INTU32 offset, INTU32 length,
                 struct error *error);
void copybytes(FILE *file1, FILE *file2, char *file1path, char *file2path,
               INTU32 f1_offset, INTU32 f2_offset, INTU32 numBytes,
               struct error *error);
void zerobytes(FILE *file, char *path, INTU32 offset, INTU32 length,
               struct error *error);
void patternbytes(FILE *file, char *path, INTU32 offset, INTU32 length,
                  INTU8 *pattern, INTU32 patternlen, struct error *error);
void level_struct_add(struct level *in, INTU32 offset, INTU32 amount);
void level_struct_sub(struct level *in, INTU32 offset, INTU32 amount);
int compareString(char *a, char *b, size_t length);
INTU16 splitString(char *in, char ***out, INTU16 len,
                   char sep, int dep, struct error *error);
INTU16 charindex(char *in, INTU16 len, char c, int dir);
void repchar(char *string, size_t length, char o, char n);

#endif
