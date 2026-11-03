#ifndef TRMOD_TR1SS_H_
#define TRMOD_TR1SS_H_

/* File Inclusions */
#include "fixedint.h" /* Definition of fixed-size integers */
#include "structs.h"  /* Definition of used structs */
#include "errors.h"   /* Definition of error codes */

/* Function Declarations */
void tr1ss_setup(struct level *level);
void tr1ss_post_navigation(struct level *level, struct error *error);
void tr1ss_readRoomHeaders(struct level *level, struct error *error);
void tr1ss_replaceitem(struct level *level, struct error *error, uint32 item,
                       uint16 id, uint16 room, int32 x, int32 y, int32 z,
                       int16 angle, int16 intensity, uint16 flags, uint16 rep);
void tr1ss_replacestaticmesh(struct level *level, struct error *error,
                             uint16 mesh, uint16 id, uint16 room, int32 x,
                             int32 y, int32 z, int16 angle, int16 intensity,
                             uint16 rep);
void tr1ss_get_light(struct level *level, struct error *error,
                     uint16 room, uint16 light, uint16 print);
void tr1ss_replacelight(struct level *level, struct error *error, uint16 light,
                        uint16 room, int32 x, int32 y, int32 z, int16 inten,
                        uint32 fade, uint16 rep);

#endif
