// psx_inventory.py: walks a TR3 PSX level file (header word -56) along the layout trview's
// Level_tr3_psx.cpp reads and counts its lists. A file the walk breaks on gets a status naming the position.
#pragma once
#include <cstdint>
#include <string>
#include <utility>

#include "../../core/bytes.h"

namespace psx {

// Sequential little-endian reading; skip() moves without a range check, as the Python reader does.
struct Reader {
    const Bytes &d;
    size_t o = 0;

    uint8_t u8() { return ::u8(d, std::exchange(o, o + 1)); }
    uint16_t u16() { return ::u16(d, std::exchange(o, o + 2)); }
    int16_t s16() { return ::s16(d, std::exchange(o, o + 2)); }
    uint32_t u32() { return ::u32(d, std::exchange(o, o + 4)); }
    int32_t s32() { return ::s32(d, std::exchange(o, o + 4)); }
    void skip(long long n) { o = size_t((long long)o + n); }
};

struct RoomInfo {
    int32_t x, z, y_bottom, y_top;
    uint32_t data_words, roomlets;
    unsigned vertices = 0, quads = 0, triangles = 0;
    unsigned portals, sectors_z, sectors_x;
    int16_t ambient, light_mode;
    unsigned lights, statics;
    int16_t alternate;
    uint16_t flags;
    uint8_t water, hall;
};

// Reads one room at r.o and leaves r behind it.
RoomInfo read_room(Reader &r);

struct Inventory {
    std::string status;            // "ok" or "ERROR @<pos>: ..."
    size_t bytes = 0, rest = 0;
    int32_t version = 0;
    uint32_t samples = 0, sample_bytes = 0;
    unsigned overlays = 0;
    long long overlay_bytes = 0;
    size_t p_rooms = 0, p_clut = 0, p_objtex = 0, p_entities = 0, p_roomtex = 0, p_soundmap = 0;
    unsigned rooms = 0, vertices = 0, quads = 0, triangles = 0, lights = 0, room_statics = 0;
    uint32_t floordata = 0, outdoor_table = 0;
    int32_t bboxes = 0;
    uint32_t meshdata = 0, meshptr = 0, anims = 0, statechg = 0, dispatch = 0, animcmd = 0, meshtree = 0,
             frames = 0, models = 0, statics = 0, tiles4 = 0;
    unsigned japan_bytes = 0;
    uint32_t cluts = 0, objtex = 0, spritetex = 0, spriteseq = 0, cameras = 0, soundsources = 0, boxes = 0,
             overlaps = 0, animtex = 0, entities = 0, horizon = 0, roomtex = 0, sounddetails = 0;

    bool ok() const { return status == "ok"; }
};

Inventory inventory(const Bytes &d);

// Offset of the mesh data's word count, found by its counts and those of the mesh pointers and animations
// behind it (exactly one place must match).
size_t find_mesh_block(const Bytes &d, const Inventory &z);

inline void require(bool ok, const std::string &what) {
    if (!ok) throw Error(what);
}

}  // namespace psx
