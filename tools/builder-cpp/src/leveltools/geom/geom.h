// Shared code of the geometry/sound level tools: the level layouts of strip_unused_tail.py, drop_object.py and
// split_room.py, the PSX level sections of psx_lara_anims.py, and the trmod calls.
#pragma once
#include <array>
#include <cstdint>
#include <optional>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include "../tool.h"

namespace geom {

// ---- Python semantics -------------------------------------------------------------------------------------
inline int64_t floordiv(int64_t a, int64_t b) {
    int64_t q = a / b;
    if ((a % b != 0) && ((a < 0) != (b < 0))) q--;
    return q;
}
inline int64_t pymod(int64_t a, int64_t b) { return a - floordiv(a, b) * b; }

int64_t parse_int(const std::string &s, int base = 10);    // Python int(s) / int(s, 0)
double parse_float(const std::string &s);                   // Python float(s)
std::string basename(const std::string &path);              // os.path.basename (both slashes)
size_t count_nonoverlapping(const Bytes &hay, const Bytes &needle);   // bytes.count

// Offset arithmetic as Python's ints (a negative count moves back); reads check the range.
inline size_t add(size_t p, int64_t n) { return size_t(int64_t(p) + n); }

// ---- strip_unused_tail.py -----------------------------------------------------------------------------------
struct Section {
    size_t off = 0;        // the count field
    uint32_t count = 0;
    size_t size = 0;       // bytes per entry
    size_t data() const { return off + 4; }
};

struct Layout {
    size_t rooms = 0;                    // the room count field
    std::vector<size_t> room_faces;      // offsets of the texture words of all room quads and triangles
    Section floordata, meshdata, meshptr, anims, statech, dispatch, animcmd, meshtree, frames, moveables, statics,
        sprtex, sprseq, cameras, soundsrc, animtex, objtex, items;
};
Layout layout(const Bytes &b);

struct MeshInfo {
    size_t size = 0;
    std::vector<int> tex;                // object texture indices of the textured faces
};
MeshInfo mesh_info(const Bytes &b, size_t off);

struct Moveable {
    uint32_t id;
    uint16_t nmeshes, start;
    uint32_t tree, frame;
    uint16_t anim;
};
std::vector<Moveable> moveables(const Bytes &b, const Layout &L);

struct Analysis {
    Layout L;
    std::vector<uint32_t> ptr;
    std::set<size_t> used_ptr;
    int last_tex = 0;
    int64_t last_anim = 0;
    int64_t frames_end = 0;
};
Analysis analyse(const Bytes &b);

// strip_unused_tail.py main: the report lines go to ctx
void strip_unused_tail(ToolContext &ctx, const std::string &src, const std::string &dst);

// ---- drop_object.py ------------------------------------------------------------------------------------------
struct Sections {        // offsets of the count fields (TR3 PC level)
    size_t floor, mesh, meshptr, anims, states, dispatch, commands, trees, frames, moveables, statics, spritetex,
        spriteseq, cameras, sources, items;
};
Sections sections(const Bytes &d);

// The level without the meshes of object oid, or nullopt; what: what was done or why not.
std::optional<Bytes> drop(const Bytes &d, uint32_t oid, std::string &what);

// The sound map of a TR3 PC level (add_sound.py, sound_volumes.py): 370 sound slots, then the sound details.
size_t sound_map(const Bytes &b);

// ---- psx_lara_anims.py sections() (Lara's home, a PSX level) ------------------------------------------------
struct PsxSections {
    size_t anims, states, dispatch, commands, trees, frames, moveables, items;
};
constexpr size_t PSX_MODEL = 20;         // bytes per moveable in a PSX level
PsxSections psx_sections(const Bytes &d);

// ---- split_room.py --------------------------------------------------------------------------------------------
struct Room {
    int32_t x = 0, z = 0, yb = 0, yt = 0;
    std::vector<std::array<int32_t, 6>> verts;      // x, y, z, lighting, attributes, colour
    std::vector<std::array<int32_t, 5>> quads;
    std::vector<std::array<int32_t, 4>> tris;
    std::vector<std::array<int32_t, 2>> sprites;
    std::vector<std::array<int32_t, 16>> portals;   // room, normal, 4 vertices
    int32_t zs = 0, xs = 0;
    std::vector<std::array<int32_t, 6>> sectors;    // fd index, box, room below, floor, room above, ceiling
    Bytes ambient;
    std::vector<Bytes> lights, statics;
    int32_t alt = -1, flags = 0;
    Bytes tail;

    std::array<int32_t, 6> &sec(int xi, int zi) { return sectors.at(size_t(xi) * size_t(zs) + size_t(zi)); }
    const std::array<int32_t, 6> &sec(int xi, int zi) const { return sectors.at(size_t(xi) * size_t(zs) + size_t(zi)); }
    size_t faces() const { return quads.size() + tris.size(); }
};

struct ParsedRooms {
    Layout L;
    std::vector<Room> rooms;
    size_t start = 0, end = 0;           // the room list in the file
};
ParsedRooms parse_rooms(const Bytes &b);
Bytes build_rooms(const std::vector<Room> &rooms);

// split_room.py main: splits room ri of src at k along axis ("x" or else z) and writes dst
void split_room(ToolContext &ctx, const std::string &trmod, const std::string &src, const std::string &dst, int ri,
                const std::string &axis, int k);

// ---- trmod -------------------------------------------------------------------------------------------------------
// Runs <exe> tr3pc <level> cmds... in ctx.cwd, 60 commands per call; fails on an exit code or "rror" in the output.
std::string trmod(const ToolContext &ctx, const std::string &exe, const std::string &level,
                  const std::vector<std::string> &cmds);

// The level's FloorData as trmod commands, keyed by (room, x, z), in the order trmod lists them.
struct FloorData {
    std::vector<std::pair<std::array<int, 3>, std::string>> items;
    const std::string *get(const std::array<int, 3> &key) const;
};
FloorData floordata(const ToolContext &ctx, const std::string &exe, const std::string &level);

// The quoted "replacefloordata(...)" commands of trmod's output (the regex "(replacefloordata\([^"]*\))").
std::vector<std::string> replacefloordata_commands(const std::string &out);

}  // namespace geom
