#ifndef TRMOD_UTIL_H_
#define TRMOD_UTIL_H_

/* File Inclusions */
#include <stdio.h>    /* Standard I/O */
#include "fixedint.h" /* Definition of fixed-size integers */
#include "structs.h"  /* Definition of structs */
#include "errors.h"   /* Error Handling for TRMOD */

/* Function Declarations */
void insertbytes(FILE *file, char *path, uint32 offset, uint32 length,
                 struct error *error);
void removebytes(FILE *file, char *path, uint32 offset, uint32 length,
                 struct error *error);
void copybytes(FILE *file1, FILE *file2, char *file1path, char *file2path,
               uint32 f1_offset, uint32 f2_offset, uint32 numBytes,
               struct error *error);
void zerobytes(FILE *file, char *path, uint32 offset, uint32 length,
               struct error *error);
void patternbytes(FILE *file, char *path, uint32 offset, uint32 length,
                  uint8 *pattern, uint32 patternlen, struct error *error);
void level_struct_add(struct level *in, uint32 offset, uint32 amount);
void level_struct_sub(struct level *in, uint32 offset, uint32 amount);
int compareString(char *a, char *b, size_t length);
uint16 splitString(char *in, char ***out, uint16 len, char sep,
                   int dep, int run, struct error *error);
uint16 charindex(char *in, uint16 len, char c, int dir);
void repchar(char *string, size_t length, char o, char n);
void filterchars(char *string, char *charset);

#endif
