#ifndef TRMOD_TR2PC_H_
#define TRMOD_TR2PC_H_

/* File Inclusions */
#include "fixedint.h" /* Definition of fixed-size integers */
#include "structs.h"  /* Definition of used structs */
#include "errors.h"   /* Definition of error codes */

/* Function Declarations */
void tr2pc_setup(struct level *level);
void tr2pc_get_item(struct level *level, struct error *error,
                    uint32 item, uint16 print);
void tr2pc_replaceitem(struct level *level, struct error *error,
                       uint32 item, uint16 id, uint16 room,
                       int32 x, int32 y, int32 z, int16 angle,
                       int16 inten1, int16 inten2, uint16 flags, uint16 rep);
void tr2pc_get_staticmesh(struct level *level, struct error *error,
                          uint16 room, uint16 mesh, uint16 print);
void tr2pc_replacestaticmesh(struct level *level, struct error *error,
                             uint16 mesh, uint16 id, uint16 room, int32 x,
                             int32 y, int32 z, int16 angle, int16 intensity1,
                             int16 intensity2, uint16 rep);
void tr2pc_get_light(struct level *level, struct error *error,
                     uint16 room, uint16 light, uint16 print);
void tr2pc_replacelight(struct level *level, struct error *error, uint16 light,
                        uint16 room, int32 x, int32 y, int32 z, int16 inten1,
                        int16 inten2, uint32 fade1, uint32 fade2, uint16 rep);
void tr2pc_get_vertex(struct level *level, struct error *error,
                      uint16 room, uint16 vertex, uint16 print);
void tr2pc_replace_vertex(struct level *level, struct error *error,
                          uint16 room, uint16 vertex,
                          int32 x, int32 y, int32 z, int16 inten1,
                          int16 inten2, uint32 attrib, uint16 rep);
void tr2pc_replace_sprite(struct level *level, struct error *error,
                          uint16 room, uint16 sprite, uint32 id,
                          int32 x, int32 y, int32 z, int16 inten1,
                          int16 inten2, uint16 attrib, uint16 rep);
void tr2pc_get_roomlight(struct level *level, struct error *error,
                         uint16 room, uint16 print);
void tr2pc_roomlight(struct level *level, struct error *error, uint16 room,
                     int16 inten1, int16 inten2, uint16 lightmode);
void tr2pc_addroom(struct level *level, struct error *error);
void tr2pc_moveroom(struct level *level, struct error *error,
                    uint16 room, int32 x, int32 y, int32 z, uint16 flags);
void tr2pc_getbox(struct level *level, struct error *error,
                  uint32 box, uint16 print);
void tr2pc_add_box(struct level *level, struct error *error);
void tr2pc_replace_box(struct level *level, struct error *error, uint32 box,
                       uint16 room, int32 xmin, int32 xmax, int32 zmin,
                       int32 zmax, int16 floor, int16 gz1, int16 gz2,
                       int16 gz3, int16 gz4, int16 fz, int16 agz1, int16 agz2,
                       int16 agz3, int16 agz4, int16 afz, char *overlapstr,
                       int blocked, int blockable, uint32 rep);
uint16 tr2pc_find_vertex(struct level *level, struct error *error,
                         uint16 room, int32 x, int32 y, int32 z,
                         int16 inten1, int16 inten2, uint16 attrib);

#endif
