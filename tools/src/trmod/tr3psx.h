#ifndef TRMOD_TR3PSX_H_
#define TRMOD_TR3PSX_H_

/* File Inclusions */
#include "structs.h" /* Definition of used structs */
#include "errors.h"  /* Definition of error codes */

/* Function Declarations */
void tr3psx_setup(struct level *level);
void tr3psx_post_navigation(struct level *level, struct error *error);

#endif
