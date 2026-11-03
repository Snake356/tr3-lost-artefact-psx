// Shared parts of the animation tools: where the lists of a level are, the records in them, and the
// splicing of new animations into a level.
#pragma once
#include <cstdint>
#include <optional>
#include <string>
#include <vector>

#include "../../core/bytes.h"

namespace anim {

inline constexpr size_t PC_STRIDE = 18;    // bytes per moveable in a PC level (PSX: 20)

// Offsets of the count fields of the lists (0 where a list is not known, e.g. in a PSX level).
struct Sections {
    size_t floor = 0, mesh = 0, meshptr = 0, anims = 0, states = 0, dispatch = 0, commands = 0, trees = 0,
           frames = 0, moveables = 0;
    size_t statics = 0, spritetex = 0, spriteseq = 0, cameras = 0, sources = 0, items = 0;
};

// The lists of a TR3 PC level from the floor data to the moveables; with_items: on to the item list
// (the lists drop_object.py knows).
Sections pc_sections(const Bytes &d, bool with_items = false);

struct Moveable {
    uint32_t id;
    uint16_t meshes, start_mesh;
    uint32_t tree, frame_offset;
    uint16_t anim;
};

// The first 20 bytes of an animation record.
struct AnimHead {
    uint32_t frame_offset;
    uint8_t rate, frame_size;
    uint16_t state;
    int32_t speed, accel;
    uint16_t frame_base, frame_end;
};

// A whole animation record (32 bytes).
struct AnimRecord {
    uint32_t frame_offset;
    unsigned rate, frame_size, state;
    int32_t speed, accel;
    unsigned frame_base, frame_end, next_anim, next_frame, changes, first_change, commands, first_command;
};

inline uint32_t count(const Bytes &d, size_t field) { return u32(d, field); }
inline size_t anim_at(const Sections &s, size_t i) { return s.anims + 4 + 32 * i; }

Moveable read_moveable(const Bytes &d, const Sections &s, size_t stride, size_t i);
std::vector<Moveable> read_moveables(const Bytes &d, const Sections &s, size_t stride);
AnimHead read_anim_head(const Bytes &d, const Sections &s, size_t i);

// Python's floor division and modulo.
inline long long floordiv(long long a, long long b) {
    long long q = a / b;
    return (a % b != 0 && ((a < 0) != (b < 0))) ? q - 1 : q;
}
inline long long floormod(long long a, long long b) { return a - floordiv(a, b) * b; }

// Packing with struct's range checks.
void pack_u16(Bytes &d, size_t o, long long v);
void pack_u32(Bytes &d, size_t o, long long v);
void append_u16(Bytes &b, long long v);
void append_s16(Bytes &b, long long v);
void append_u32(Bytes &b, long long v);
void append_record(Bytes &b, const AnimRecord &r);
void append_s16s(Bytes &b, const std::vector<int> &words);

// out += d[a:b] with Python's slice clipping.
void append_slice(Bytes &out, const Bytes &d, size_t a, size_t b);

inline void require(bool ok, const std::string &what) {
    if (!ok) throw Error(what);
}

// Moves every reference to an animation at or behind index up by n: next animations, state change
// targets and the moveables' first animations (in place).
void shift_anim_refs(Bytes &d, const Sections &s, size_t stride, unsigned index, unsigned n);

// The smallest first animation of a moveable other than Lara (the number of Lara's animations).
unsigned second_first_anim(const std::vector<Moveable> &movs);

// What goes into the lists: animation records inserted at index, state changes inserted at states_at
// with dispatch ranges appended (only when states is not empty), commands and frames appended.
struct Insert {
    unsigned index = 0;
    unsigned anims = 0;           // number of records
    Bytes records;
    size_t states_at = 0;         // index in the state change list
    unsigned new_states = 0, new_ranges = 0;
    Bytes states, ranges;
    Bytes commands;               // whole words
    Bytes frames;
};
Bytes splice(const Bytes &d, const Sections &s, const Insert &in);

}  // namespace anim
