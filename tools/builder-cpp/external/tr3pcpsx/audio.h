#ifndef TR3PCPSX_AUDIO_H_
#define TR3PCPSX_AUDIO_H_

/* File Inclusions */
#include <stdio.h>    /* Standard I/O */
#include "fixedint.h" /* Definition of fixed-size integers */

/* Function Declarations */
int convertSample(FILE *pc, FILE *psx, INTU32 pcOffset,
                  INTU32 psxOffset, int loops);
INTU32 outVagSize(FILE *sfx, INTU32 offset);

#endif
