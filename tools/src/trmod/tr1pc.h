#ifndef TRMOD_TR1PC_H_
#define TRMOD_TR1PC_H_

/* File Inclusions */
#include "fixedint.h" /* Definition of fixed-size integers */
#include "structs.h"  /* Definition of used structs */
#include "errors.h"   /* Definition of error codes */

/* Function Declarations */
void tr1pc_setup(struct level *level);
void tr1pc_navigate(struct level *level, struct error *error);
void tr1pc_readRoomHeaders(struct level *level, struct error *error);
void tr1pc_get_item(struct level *level, struct error *error,
                    uint32 item, uint16 print);
void tr1pc_additem(struct level *level, struct error *error);
void tr1pc_replaceitem(struct level *level, struct error *error, uint32 item,
                       uint16 id, uint16 room, int32 x, int32 y, int32 z,
                       int16 angle, int16 intensity, uint16 flags, uint16 rep);
void tr1pc_removeitem(struct level *level, struct error *error,
                      uint32 item, uint16 flags);
void tr1pc_removeallitems(struct level *level, struct error *error);
void tr1pc_get_staticmesh(struct level *level, struct error *error,
                          uint16 room, uint16 mesh, uint16 print);
void tr1pc_addstaticmesh(struct level *level, struct error *error, uint16 room);
void tr1pc_replacestaticmesh(struct level *level, struct error *error,
                             uint16 mesh, uint16 id, uint16 room, int32 x,
                             int32 y, int32 z, int16 angle, int16 intensity,
                             uint16 rep);
void tr1pc_removestaticmesh(struct level *level, struct error *error,
                            uint16 room, uint16 mesh);
void tr1pc_removeallstaticmeshes(struct level *level,
                                 struct error *error, uint16 room);
void tr1pc_get_light(struct level *level, struct error *error,
                     uint16 room, uint16 light, uint16 print);
void tr1pc_addlight(struct level *level, struct error *error, uint16 room);
void tr1pc_replacelight(struct level *level, struct error *error, uint16 light,
                        uint16 room, int32 x, int32 y, int32 z, int16 inten,
                        uint32 fade, uint16 rep);
void tr1pc_removelight(struct level *level, struct error *error,
                       uint16 room, uint16 light);
void tr1pc_removealllights(struct level *level, struct error *error,
                           uint16 room);
void tr1pc_get_soundsource(struct level *level, struct error *error,
                           uint32 source, uint16 print);
void tr1pc_addsoundsource(struct level *level, struct error *error);
void tr1pc_replacesoundsource(struct level *level, struct error *error,
                              uint32 source, uint16 id, uint16 room, int32 x,
                              int32 y, int32 z, uint16 flags, uint16 rep);
void tr1pc_removesoundsource(struct level *level, struct error *error,
                             uint32 source);
void tr1pc_removeallsoundsources(struct level *level, struct error *error);
void tr1pc_get_floordata(struct level *level, struct error *error,
                         uint16 room, uint16 column, uint16 row,
                         uint32 secpad, uint16 print);
void tr1pc_replace_floordata(struct level *level, struct error *error,
                             uint16 room, uint16 column, uint16 row,
                             char *string, uint32 secpad, uint16 flags);
void tr1pc_remove_floordata(struct level *level, struct error *error,
                            uint16 room, uint16 column,
                            uint16 row, uint32 secpad);
void tr1pc_remove_all_floordata(struct level *level, struct error *error);
void tr1pc_get_altroom(struct level *level, struct error *error,
                       uint16 room, uint16 print);
void tr1pc_altroom(struct level *level, struct error *error,
                   uint16 room, int16 altroom);
void tr1pc_get_roomflags(struct level *level, struct error *error, uint16 room,
                         uint32 flagpad, uint16 flagbits, uint16 print);
void tr1pc_setroomflags(struct level *level, struct error *error, uint16 room,
                        uint32 flagpad, uint16 mask, int operation);
void tr1pc_addroom(struct level *level, struct error *error);
void tr1pc_get_roompos(struct level *level, struct error *error,
                       uint16 room, uint16 print);
void tr1pc_moveroom(struct level *level, struct error *error,
                    uint16 room, int32 x, int32 y, int32 z, uint16 flags);
void tr1pc_get_roomsize(struct level *level, struct error *error,
                        uint16 room, uint16 print);
void tr1pc_resize_room(struct level *level, struct error *error, uint16 room,
                       uint16 columns, uint16 rows, int32 height);
void tr1pc_remove_all_rooms(struct level *level, struct error *error);
void tr1pc_getfloorceil(struct level *level, struct error *error,
                        uint16 room, uint16 column, uint16 row,
                        uint32 secpad, int type, uint16 print);
void tr1pc_setfloorceil(struct level *level, struct error *error,
                        uint16 room, uint16 column, uint16 row,
                        uint32 secpad, int32 height, uint16 type);
void tr1pc_getzone(struct level *level, struct error *error, uint16 room,
                   uint16 column, uint16 row, uint32 secpad, uint16 print);
void tr1pc_setzone(struct level *level, struct error *error, uint16 room,
                   uint16 column, uint16 row, uint32 secpad, int16 boxIndex);
void tr1pc_get_vertex(struct level *level, struct error *error,
                      uint16 room, uint16 vertex, uint16 print);
void tr1pc_add_vertex(struct level *level, struct error *error, uint16 room);
void tr1pc_replace_vertex(struct level *level, struct error *error,
                          uint16 room, uint16 vertex, int32 x,
                          int32 y, int32 z, int16 inten, uint16 rep);
void tr1pc_remove_vertex(struct level *level, struct error *error,
                         uint16 room, uint16 vertex);
void tr1pc_remove_geometry(struct level *level, struct error *error,
                           uint16 room);
void tr1pc_get_rectangle(struct level *level, struct error *error,
                         uint16 room, uint16 rectangle, uint16 print);
void tr1pc_add_rectangle(struct level *level, struct error *error, uint16 room);
void tr1pc_replace_rectangle(struct level *level, struct error *error,
                             uint16 room, uint16 rectangle, uint16 vertex1,
                             uint16 vertex2, uint16 vertex3, uint16 vertex4,
                             uint16 texture, uint16 doublesided, uint16 rep);
void tr1pc_remove_rectangle(struct level *level, struct error *error,
                            uint16 room, uint16 rectangle);
void tr1pc_remove_all_rectangles(struct level *level,
                                 struct error *error, uint16 room);
void tr1pc_get_triangle(struct level *level, struct error *error,
                        uint16 room, uint16 triangle, uint16 print);
void tr1pc_add_triangle(struct level *level, struct error *error, uint16 room);
void tr1pc_replace_triangle(struct level *level, struct error *error,
                            uint16 room, uint16 triangle, uint16 vertex1,
                            uint16 vertex2, uint16 vertex3, uint16 texture,
                            uint16 doublesided, uint16 rep);
void tr1pc_remove_triangle(struct level *level, struct error *error,
                           uint16 room, uint16 triangle);
void tr1pc_remove_all_triangles(struct level *level,
                                struct error *error, uint16 room);
void tr1pc_get_sprite(struct level *level, struct error *error,
                      uint16 room, uint16 sprite, uint16 print);
void tr1pc_add_sprite(struct level *level, struct error *error, uint16 room);
void tr1pc_replace_sprite(struct level *level, struct error *error,
                          uint16 room, uint16 sprite, uint32 id,
                          int32 x, int32 y, int32 z, int16 inten, uint16 rep);
void tr1pc_remove_sprite(struct level *level, struct error *error,
                         uint16 room, uint16 sprite);
void tr1pc_remove_all_sprites(struct level *level,
                              struct error *error, uint16 room);
void tr1pc_get_viewport(struct level *level, struct error *error,
                        uint16 room, uint16 viewport, uint16 print);
void tr1pc_add_viewport(struct level *level, struct error *error, uint16 room);
void tr1pc_replace_viewport(struct level *level, struct error *error,
                            uint16 room, uint16 viewport, uint16 adjroom,
                            int16 n_x, int16 n_y, int16 n_z, int16 v1_x,
                            int16 v1_y, int16 v1_z, int16 v2_x, int16 v2_y,
                            int16 v2_z, int16 v3_x, int16 v3_y, int16 v3_z,
                            int16 v4_x, int16 v4_y, int16 v4_z, uint16 rep);
void tr1pc_remove_viewport(struct level *level, struct error *error,
                           uint16 room, uint16 viewport);
void tr1pc_remove_all_viewports(struct level *level, struct error *error,
                                uint16 room);
void tr1pc_get_roomlight(struct level *level, struct error *error,
                         uint16 room, uint16 print);
void tr1pc_roomlight(struct level *level, struct error *error,
                     uint16 room, int16 roomlight);
void tr1pc_getbox(struct level *level, struct error *error,
                  uint32 box, uint16 print);
void tr1pc_add_box(struct level *level, struct error *error);
void tr1pc_replace_box(struct level *level, struct error *error, uint32 box,
                       uint16 room, int32 xmin, int32 xmax, int32 zmin,
                       int32 zmax, int16 floor, int16 gz1, int16 gz2, int16 fz,
                       int16 agz1, int16 agz2, int16 afz, char *overlapstr,
                       int blocked, int blockable, uint32 rep);
void tr1pc_remove_box(struct level *level, struct error *error, uint32 box);
void tr1pc_remove_all_boxes(struct level *level, struct error *error);
void tr1pc_hexdump(struct level *level, struct error *error, uint16 print);
uint16 tr1pc_nearestRoom(struct level *level, int32 x, int32 y, int32 z);
uint16 tr1pc_find_vertex(struct level *level, struct error *error,
                         uint16 room, int32 x, int32 y, int32 z, int16 inten);
uint16 tr1pc_addcamera(struct level *level, struct error *error, uint8 *camera);
void tr1pc_vardump(struct level *level);

#endif
