#ifndef TR3PCPSX_COLOUR_H_
#define TR3PCPSX_COLOUR_H_

/* File Inclusions */
#include "fixedint.h" /* Definition of fixed-size integers */

/* Colour Struct */
struct colour
{
	INTU8 r;         /* Colour's red value */
	INTU8 g;         /* Colour's green value */
	INTU8 b;         /* Colour's blue value */
	INTU8 bucket;    /* Bucket the colour is in */
	INTU16 in_index; /* Colour's input index */
	INTU16 weight;   /* Colour's weight */
};

/* Function Declarations */
void reduceColours(struct colour *colours, INTU16 numColours,
                   INTU8 outColours, INTU16 *palette);
INTU8 getBucket(struct colour *colours, INTU16 numColours, INTU16 colour);

/* Conversion of a 16-bit RGB colour to a 16-bit BGR colour */
#define rgbbgr1616(a) (INTU16) (((a & 0x001F) << 10U) | \
                                (a & 0x03E0)          | \
                                ((a & 0x7C00) >> 10U));

/* Conversion of a 24-bit colour to a 16-bit colour */
#define colour2416(r, g, b) (INTU16) (0x8000 | (((INTU16) (b & 0xF8)) >> 3) | \
                                      (((INTU16) (g & 0xF8)) << 2) |          \
                                      (((INTU16) (r & 0xF8)) << 7));

/* Conversion of a 16-bit colour to a 24-bit colour */
#define colour1624(c, r, g, b) r = (INTU8) ((c & ((INTU16) 0x001F)) << 3); \
                               g = (INTU8) ((c & ((INTU16) 0x03E0)) >> 2); \
                               b = (INTU8) ((c & ((INTU16) 0x7C00)) >> 7);

#endif
