#include "psx_inventory.h"

#include <format>

namespace psx {

RoomInfo read_room(Reader &r) {
    RoomInfo room{};
    room.x = r.s32();
    room.z = r.s32();
    room.y_bottom = r.s32();
    room.y_top = r.s32();
    room.data_words = r.u32();
    size_t start = r.o;
    room.roomlets = r.u32();
    std::vector<uint32_t> offsets(room.roomlets);
    for (auto &off : offsets) off = r.u32();
    for (uint32_t off : offsets) {
        r.o = start + off;
        r.skip(6);                                   // bounding box
        unsigned v = r.u8(), t = r.u8();
        r.skip(4LL * (v + t));
        room.vertices += v;
        room.triangles += t;
        // quads come in groups of three under one texture word; 0x3ff ends the list
        for (bool more = true; more;) {
            uint32_t tex = r.u32();
            r.skip(12);
            for (int k = 0; k < 3; k++) {
                if (((tex >> (10 * k)) & 0x3ff) == 0x3ff) {
                    more = false;
                    break;
                }
                room.quads++;
            }
        }
    }
    r.o = start + size_t(room.data_words) * 2;
    room.portals = r.u16();
    r.skip(32LL * room.portals);
    room.sectors_z = r.u16();
    room.sectors_x = r.u16();
    r.skip(8LL * room.sectors_z * room.sectors_x);
    room.ambient = r.s16();
    room.light_mode = r.s16();
    room.lights = r.u16();
    r.skip(24LL * room.lights);
    room.statics = r.u16();
    r.skip(20LL * room.statics);
    room.alternate = r.s16();
    room.flags = r.u16();
    room.water = r.u8();
    room.hall = r.u8();
    r.skip(1);
    return room;
}

Inventory inventory(const Bytes &d) {
    Reader r{d};
    Inventory z;
    z.bytes = d.size();
    try {
        z.version = r.s32();
        z.samples = r.u32();
        r.skip(4LL * z.samples);
        if (z.samples) {                             // without samples there is no size field (cutscenes)
            z.sample_bytes = r.u32();
            r.skip(z.sample_bytes);
        }
        for (int i = 0; i < 13; i++) {
            int32_t s = r.s32();
            if (s != 0) {
                r.skip(s);
                int32_t s2 = r.s32();
                r.skip(s2);
                z.overlays++;
                z.overlay_bytes += (long long)s + s2;
            }
        }
        z.p_rooms = r.o;
        z.rooms = r.u16();
        for (unsigned i = 0; i < z.rooms; i++) {
            RoomInfo room = read_room(r);
            z.vertices += room.vertices;
            z.quads += room.quads;
            z.triangles += room.triangles;
            z.lights += room.lights;
            z.room_statics += room.statics;
        }
        auto list = [&](uint32_t &n, long long size) {
            n = r.u32();
            r.skip(size * n);
        };
        list(z.floordata, 2);
        r.skip(729 * 2);                             // outdoor map
        list(z.outdoor_table, 1);
        z.bboxes = r.s32();
        r.skip(8LL * z.bboxes);
        list(z.meshdata, 2);
        list(z.meshptr, 4);
        list(z.anims, 32);
        list(z.statechg, 6);
        list(z.dispatch, 8);
        list(z.animcmd, 2);
        list(z.meshtree, 4);
        list(z.frames, 2);
        list(z.models, 20);
        list(z.statics, 32);
        list(z.tiles4, 32768);
        z.p_clut = r.o;
        uint32_t ncl = r.u32();
        if (ncl & 0xffff0000) {                      // Japanese files: 2 bytes before the count
            r.o -= 2;
            ncl = r.u32();
            z.japan_bytes = 2;
        }
        r.skip(64LL * ncl);
        z.cluts = ncl * 2;
        z.p_objtex = r.o;
        list(z.objtex, 16);
        list(z.spritetex, 16);
        list(z.spriteseq, 8);
        list(z.cameras, 16);
        list(z.soundsources, 16);
        list(z.boxes, 8);
        list(z.overlaps, 2);
        r.skip(20LL * z.boxes);                      // zones
        list(z.animtex, 2);
        z.entities = r.u32();
        z.p_entities = r.o;
        r.skip(24LL * z.entities);
        z.horizon = r.u32();
        z.p_roomtex = r.o;
        list(z.roomtex, 48);
        z.p_soundmap = r.o;
        r.skip(370 * 2);
        list(z.sounddetails, 8);
        z.rest = d.size() - r.o;
        z.status = "ok";
    } catch (const std::exception &e) {              // the inventory names the position, it does not abort
        z.status = std::format("ERROR @{:#x}: {}", r.o, e.what());
    }
    return z;
}

size_t find_mesh_block(const Bytes &d, const Inventory &z) {
    size_t mesh = size_t(z.meshdata) * 2, ptrs = size_t(z.meshptr) * 4;
    std::vector<size_t> hits;
    for (size_t o = 0; o + 12 < d.size(); o++)
        if (u32(d, o) == z.meshdata && o + 8 + mesh + ptrs < d.size() && u32(d, o + 4 + mesh) == z.meshptr &&
            u32(d, o + 8 + mesh + ptrs) == z.anims)
            hits.push_back(o);
    require(hits.size() == 1, "the mesh data is found " + std::to_string(hits.size()) + " times");
    return hits[0];
}

}  // namespace psx
