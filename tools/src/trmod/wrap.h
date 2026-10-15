#ifndef TRMOD_WRAP_H_
#define TRMOD_WRAP_H_

/* File Inclusions */
#include "structs.h" /* Definition of used structs */
#include "errors.h"  /* Definition of error codes */

/* Function Declarations */
void setup(struct level *level, struct error *error);
void navigate(struct level *level, struct error *error);
void post_navigation(struct level *level, struct error *error);
void readRoomHeaders(struct level *level, struct error *error);
void get_item(struct level *level, struct error *error,
              uint32 item, uint16 print);
void add_item(struct level *level, struct error *error, uint16 id, uint16 room,
              int32 x, int32 y, int32 z, int16 angle, char *intensity1,
              char *intensity2, uint16 flags);
void replace_item(struct level *level, struct error *error, uint32 item,
                  uint16 id, uint16 room, int32 x, int32 y, int32 z,
                  int16 angle, char *intensity1, char *intensity2, uint16 flags,
                  uint16 rep);
void remove_item(struct level *level, struct error *error,
                 uint32 item, uint16 flags);
void remove_all_items(struct level *level, struct error *error);
void get_staticmesh(struct level *level, struct error *error,
                    uint16 room, uint16 mesh, uint16 print);
void add_staticmesh(struct level *level, struct error *error, uint16 id,
                    uint16 room, int32 x, int32 y, int32 z, int16 angle,
                    char *intensity1, char *intensity2);
void replace_staticmesh(struct level *level, struct error *error, uint16 mesh,
                        uint16 id, uint16 room, int32 x, int32 y, int32 z,
                        int16 angle, char *intensity1, char *intensity2,
                        uint16 rep);
void remove_staticmesh(struct level *level, struct error *error,
                       uint16 room, uint16 mesh);
void remove_all_staticmeshes(struct level *level,
                             struct error *error, uint16 room);
void get_light(struct level *level, struct error *error,
               uint16 room, uint16 light, uint16 print);
void add_light(struct level *level, struct error *error,
               uint16 room, int32 x, int32 y, int32 z, char *intensity1,
               char *intensity2, uint32 fade1, uint32 fade2, uint8 type,
               int16 nx, int16 ny, int16 nz);
void replace_light(struct level *level, struct error *error, uint16 room,
                   uint16 light, int32 x, int32 y, int32 z, char *intensity1,
                   char *intensity2, uint32 fade1, uint32 fade2, uint8 type,
                   int16 nx, int16 ny, int16 nz, uint16 rep);
void remove_light(struct level *level, struct error *error,
                  uint16 room, uint16 light);
void remove_all_lights(struct level *level, struct error *error, uint16 room);
void get_soundsource(struct level *level, struct error *error,
                     uint32 source, uint16 print);
void add_soundsource(struct level *level, struct error *error, uint16 id,
                     uint16 room, int32 x, int32 y, int32 z, uint16 flags);
void replace_soundsource(struct level *level, struct error *error,
                         uint32 source, uint16 id, uint16 room, int32 x,
                         int32 y, int32 z, uint16 flags, uint16 rep);
void remove_soundsource(struct level *level, struct error *error,
                        uint32 source);
void remove_all_soundsources(struct level *level, struct error *error);
void get_floordata(struct level *level, struct error *error, uint16 room,
                   uint16 column, uint16 row, uint16 print);
void replace_floordata(struct level *level, struct error *error, uint16 room,
                       uint16 column, uint16 row, char *string, uint16 flags);
void remove_floordata(struct level *level, struct error *error,
                      uint16 room, uint16 column, uint16 row);
void remove_all_floordata(struct level *level, struct error *error);
void add_room(struct level *level, struct error *error);
void get_roompos(struct level *level, struct error *error,
                 uint16 room, uint16 print);
void move_room(struct level *level, struct error *error,
               uint16 room, int32 x, int32 y, int32 z, uint16 flags);
void get_roomsize(struct level *level, struct error *error,
                  uint16 room, uint16 print);
void resize_room(struct level *level, struct error *error, uint16 room,
                 uint16 columns, uint16 rows, int32 height);
void remove_all_rooms(struct level *level, struct error *error);
void get_floor_ceiling(struct level *level, struct error *error, uint16 room,
                       uint16 column, uint16 row, int type, uint16 print);
void set_floor_ceiling(struct level *level, struct error *error, uint16 room,
                       uint16 column, uint16 row, int32 height, uint16 type);
void get_zone(struct level *level, struct error *error, uint16 room,
              uint16 column, uint16 row, uint16 print);
void set_zone(struct level *level, struct error *error, uint16 room,
              uint16 column, uint16 row, int16 boxIndex);
void get_stepsound(struct level *level, struct error *error, uint16 room,
                   uint16 column, uint16 row, uint16 print);
void set_stepsound(struct level *level, struct error *error, uint16 room,
                   uint16 column, uint16 row, uint16 stepsound);
void get_vertex(struct level *level, struct error *error,
                uint16 room, uint16 vertex, uint16 print);
void add_vertex(struct level *level, struct error *error, uint16 room,
                int32 x, int32 y, int32 z, char *intensity1,
                char *intensity2, uint16 attributes);
void replace_vertex(struct level *level, struct error *error, uint16 room,
                    uint16 vertex, int32 x, int32 y, int32 z, char *intensity1,
                    char *intensity2, uint16 attributes, uint16 rep);
void remove_vertex(struct level *level, struct error *error,
                   uint16 room, uint16 vertex);
void remove_geometry(struct level *level, struct error *error, uint16 room);
void get_rectangle(struct level *level, struct error *error,
                   uint16 room, uint16 rectangle, uint16 print);
void add_rectangle(struct level *level, struct error *error, uint16 room,
                   uint16 vertex1, uint16 vertex2, uint16 vertex3,
                   uint16 vertex4, uint16 texture, uint16 doublesided);
void replace_rectangle(struct level *level, struct error *error, uint16 room,
                       uint16 rectangle, uint16 vertex1, uint16 vertex2,
                       uint16 vertex3, uint16 vertex4, uint16 texture,
                       uint16 doublesided, uint16 rep);
void remove_rectangle(struct level *level, struct error *error,
                      uint16 room, uint16 rectangle);
void remove_all_rectangles(struct level *level,
                           struct error *error, uint16 room);
void get_triangle(struct level *level, struct error *error,
                  uint16 room, uint16 triangle, uint16 print);
void add_triangle(struct level *level, struct error *error, uint16 room,
                  uint16 vertex1, uint16 vertex2, uint16 vertex3,
                  uint16 texture, uint16 doublesided);
void replace_triangle(struct level *level, struct error *error, uint16 room,
                      uint16 triangle, uint16 vertex1, uint16 vertex2,
                      uint16 vertex3, uint16 texture,
                      uint16 doublesided, uint16 rep);
void remove_triangle(struct level *level, struct error *error,
                     uint16 room, uint16 triangle);
void remove_all_triangles(struct level *level,
                          struct error *error, uint16 room);
void get_sprite(struct level *level, struct error *error,
                uint16 room, uint16 sprite, uint16 print);
void add_sprite(struct level *level, struct error *error, uint16 id,
                uint16 room, int32 x, int32 y, int32 z, char *intensity1,
                char *intensity2, uint16 attributes);
void replace_sprite(struct level *level, struct error *error, uint16 room,
                    uint16 sprite, uint16 id, int32 x, int32 y, int32 z,
                    char *intensity1, char *intensity2, uint16 attributes,
                    uint16 rep);
void remove_sprite(struct level *level, struct error *error,
                   uint16 room, uint16 sprite);
void remove_all_sprites(struct level *level, struct error *error, uint16 room);
void get_viewport(struct level *level, struct error *error,
                  uint16 room, uint16 viewport, uint16 print);
void add_viewport(struct level *level, struct error *error, uint16 room,
                  uint16 adjroom, int16 n_x, int16 n_y, int16 n_z, int16 v1_x,
                  int16 v1_y, int16 v1_z, int16 v2_x, int16 v2_y, int16 v2_z,
                  int16 v3_x, int16 v3_y, int16 v3_z, int16 v4_x, int16 v4_y,
                  int16 v4_z);
void replace_viewport(struct level *level, struct error *error,
                      uint16 room, uint16 viewport, uint16 adjroom,
                      int16 n_x, int16 n_y, int16 n_z, int16 v1_x,
                      int16 v1_y, int16 v1_z, int16 v2_x, int16 v2_y,
                      int16 v2_z, int16 v3_x, int16 v3_y, int16 v3_z,
                      int16 v4_x, int16 v4_y, int16 v4_z, uint16 rep);
void remove_viewport(struct level *level, struct error *error,
                     uint16 room, uint16 viewport);
void remove_all_viewports(struct level *level,
                          struct error *error, uint16 room);
void get_roomlight(struct level *level, struct error *error,
                   uint16 room, uint16 print);
void roomlight(struct level *level, struct error *error, uint16 room,
               char *intensity1, char *intensity2, uint16 lightmode);
void get_altroom(struct level *level, struct error *error,
                 uint16 room, uint16 print);
void altroom(struct level *level, struct error *error,
             uint16 room, int16 altroom);
void get_roomflags(struct level *level, struct error *error,
                   uint16 room, uint16 flagbits, uint16 print);
void roomflags(struct level *level, struct error *error, uint16 room,
               uint16 mask, int operation);
void get_box(struct level *level, struct error *error,
             uint32 box, uint16 print);
void add_box(struct level *level, struct error *error, uint16 room, int32 xmin,
             int32 xmax, int32 zmin, int32 zmax, int16 floor, int16 gz1,
             int16 gz2, int16 gz3, int16 gz4, int16 fz, int16 agz1, int16 agz2,
             int16 agz3, int16 agz4, int16 afz, char *overlapstr, int blocked,
             int blockable);
void replace_box(struct level *level, struct error *error, uint32 box,
                 uint16 room, int32 xmin, int32 xmax, int32 zmin, int32 zmax,
                 int16 floor, int16 gz1, int16 gz2, int16 gz3, int16 gz4,
                 int16 fz, int16 agz1, int16 agz2, int16 agz3, int16 agz4,
                 int16 afz, char *overlapstr, int blocked, int blockable,
                 uint32 rep);
void remove_all_boxes(struct level *level, struct error *error);
void hexdump(struct level *level, struct error *error, uint16 print);
uint16 nearestRoom(struct level *level, struct error *error,
                   int32 x, int32 y, int32 z);
uint16 addcamera(struct level *level, struct error *error, uint8 *camera);
void vardump(struct level *level);

#endif
