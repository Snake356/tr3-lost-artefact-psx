#ifndef TR3PCPSX_TEXTURES_H_
#define TR3PCPSX_TEXTURES_H_

/* File Inclusions */
#include <stdio.h>    /* Standard I/O */
#include "structs.h"  /* Definition of structs */
#include "fixedint.h" /* Definition of fixed-size integers */

/* Macro Definitions */
#define ROOMTEXTURE_TEXTURE  0x3FFF /* The actual texture value */
#define ROOMTEXTURE_WATER    0x4000 /* Use this one under water */
#define ROOMTEXTURE_MIRRORED 0x8000 /* Whether this is mirrored */

/* Function Declarations */
int convertAllTextures(FILE *pc, FILE *psx, struct level_info *pc_info,
                       struct level_info *psx_info, char *psxpath,
                       INTU32 flags, INTU16 *misorientedTextures,
                       INTU16 **roomTextptr, INTU16 **objectTextptr,
                       struct water_colour *waterColour);
int findMisorientedTextures(FILE *pc, struct level_info *pc_info,
                            INTU16 *numMisorientedTextures);
int addKanji(FILE *psx, struct level_info *psx_info);
int convertAnimatedTextures(FILE *pc, FILE *psx, struct level_info *pc_info,
                            struct level_info *psx_info, char *psxpath,
                            INTU16 *roomTextures);

#endif
