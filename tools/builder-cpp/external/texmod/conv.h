#ifndef TRMOD_CONV_H_
#define TRMOD_CONV_H_

/* File Inclusions */
#include "fixedint.h" /* Definition of fixed-size integers */

INT16 convertLight(char *string, int type);
void colourComponents(char *string, INTU16 *red, INTU16 *green, INTU16 *blue);
int validHex(char *string, INTU16 length);

#endif
