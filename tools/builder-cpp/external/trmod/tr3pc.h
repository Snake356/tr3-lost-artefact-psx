#ifndef TRMOD_TR3PC_H_
#define TRMOD_TR3PC_H_

/* File Inclusions */
#include "fixedint.h" /* Definition of fixed-size integers */
#include "structs.h"  /* Definition of used structs */
#include "errors.h"   /* Definition of error codes */

/* Function Declarations */
void tr3pc_setup(struct level *level);
void tr3pc_get_item(struct level *level, struct error *error,
                    uint32 item, uint16 print);
void tr3pc_get_staticmesh(struct level *level, struct error *error,
                          uint16 room, uint16 mesh, uint16 print);
void tr3pc_get_light(struct level *level, struct error *error,
                     uint16 room, uint16 light, uint16 print);
void tr3pc_replacelight(struct level *level, struct error *error, uint16 light,
                        uint16 room, int32 x, int32 y, int32 z, uint32 colour,
                        uint8 type, uint32 inten, uint32 fade, int16 nx,
                        int16 ny, int16 nz, uint16 rep);
void tr3pc_get_vertex(struct level *level, struct error *error,
                      uint16 room, uint16 vertex, uint16 print);
void tr3pc_replace_vertex(struct level *level, struct error *error,
                          uint16 room, uint16 vertex,
                          int32 x, int32 y, int32 z, int16 inten1,
                          int16 inten2, uint32 attrib, uint16 rep);
void tr3pc_replace_sprite(struct level *level, struct error *error,
                          uint16 room, uint16 sprite, uint32 id,
                          int32 x, int32 y, int32 z, int16 inten1,
                          int16 inten2, uint16 attrib, uint16 rep);
void tr3pc_getzone(struct level *level, struct error *error,
                   uint16 room, uint16 column, uint16 row, uint16 print);
void tr3pc_setzone(struct level *level, struct error *error, uint16 room,
                   uint16 column, uint16 row, int16 boxIndex);
void tr3pc_getstepsound(struct level *level, struct error *error,
                        uint16 room, uint16 column, uint16 row, uint16 print);
void tr3pc_setstepsound(struct level *level, struct error *error, uint16 room,
                        uint16 column, uint16 row, uint16 stepsound);
void tr3pc_get_roomlight(struct level *level, struct error *error,
                         uint16 room, uint16 print);
void tr3pc_roomlight(struct level *level, struct error *error,
                     uint16 room, int16 inten1, int16 inten2);
void tr3pc_addroom(struct level *level, struct error *error);

#endif
