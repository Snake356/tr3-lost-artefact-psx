#ifndef TR3PCPSX_MODULES_H_
#define TR3PCPSX_MODULES_H_

/* File Inclusions */
#include <stdio.h>    /* Definition of Standard I/O */
#include "fixedint.h" /* Definition of fixed-size integers */
#include "structs.h"  /* Definition of structs used by TR3PCPSX */

/* Function Declarations */
int addModules(FILE *pc, struct level_info *pc_info, char *psxpath,
               FILE *psx, struct level_info *psx_info, INTU32 flags);

#endif
