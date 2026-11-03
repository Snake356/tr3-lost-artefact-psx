#include "anim_common.h"

#include <algorithm>
#include <utility>

namespace anim {

Sections pc_sections(const Bytes &d, bool with_items) {
    size_t o = 4 + 768 + 1024;
    size_t tiles = u32(d, o);
    o += 4 + tiles * 65536 + tiles * 131072 + 4;
    unsigned rooms = u16(d, o);
    o += 2;
    for (unsigned r = 0; r < rooms; r++) {
        o += 16;
        o += 4 + 2 * size_t(u32(d, o));
        o += 2 + 32 * size_t(u16(d, o));
        size_t zs = u16(d, o), xs = u16(d, o + 2);
        o += 4 + 8 * zs * xs + 4;
        o += 2 + 24 * size_t(u16(d, o));
        o += 2 + 20 * size_t(u16(d, o));
        o += 7;
    }
    Sections s;
    std::pair<size_t *, size_t> lists[] = {{&s.floor, 2},    {&s.mesh, 2},     {&s.meshptr, 4},  {&s.anims, 32},
                                           {&s.states, 6},   {&s.dispatch, 8}, {&s.commands, 2}, {&s.trees, 4},
                                           {&s.frames, 2},   {&s.moveables, 18}, {&s.statics, 32},
                                           {&s.spritetex, 16}, {&s.spriteseq, 8}, {&s.cameras, 16},
                                           {&s.sources, 16}};
    size_t n = with_items ? std::size(lists) : 10;
    for (size_t k = 0; k < n; k++) {
        *lists[k].first = o;
        o += 4 + lists[k].second * size_t(u32(d, o));
    }
    if (!with_items) return s;
    size_t boxes = u32(d, o);
    o += 4 + 8 * boxes;
    o += 4 + 2 * size_t(u32(d, o));      // overlaps
    o += 20 * boxes;                       // zones
    o += 4 + 2 * size_t(u32(d, o));      // animated textures
    o += 4 + 20 * size_t(u32(d, o));     // object textures
    s.items = o;
    return s;
}

Moveable read_moveable(const Bytes &d, const Sections &s, size_t stride, size_t i) {
    size_t o = s.moveables + 4 + stride * i;
    check_range(d, o, 18);
    return {u32(d, o), u16(d, o + 4), u16(d, o + 6), u32(d, o + 8), u32(d, o + 12), u16(d, o + 16)};
}

std::vector<Moveable> read_moveables(const Bytes &d, const Sections &s, size_t stride) {
    std::vector<Moveable> out;
    uint32_t n = count(d, s.moveables);
    out.reserve(n);
    for (uint32_t i = 0; i < n; i++) out.push_back(read_moveable(d, s, stride, i));
    return out;
}

AnimHead read_anim_head(const Bytes &d, const Sections &s, size_t i) {
    size_t a = anim_at(s, i);
    check_range(d, a, 20);
    return {u32(d, a), d[a + 4], d[a + 5], u16(d, a + 6), s32(d, a + 8), s32(d, a + 12), u16(d, a + 16),
            u16(d, a + 18)};
}

static void range_check(long long v, long long lo, long long hi) {
    if (v < lo || v > hi) throw Error("value " + std::to_string(v) + " out of range for packing");
}

void pack_u16(Bytes &d, size_t o, long long v) { range_check(v, 0, 0xFFFF); put16(d, o, uint32_t(v)); }
void pack_u32(Bytes &d, size_t o, long long v) { range_check(v, 0, 0xFFFFFFFFLL); put32(d, o, uint32_t(v)); }
void append_u16(Bytes &b, long long v) { range_check(v, 0, 0xFFFF); Writer{b}.u16(uint32_t(v)); }
void append_s16(Bytes &b, long long v) { range_check(v, -32768, 32767); Writer{b}.u16(uint32_t(v)); }
void append_u32(Bytes &b, long long v) { range_check(v, 0, 0xFFFFFFFFLL); Writer{b}.u32(uint32_t(v)); }

void append_record(Bytes &b, const AnimRecord &r) {
    append_u32(b, r.frame_offset);
    range_check(r.rate, 0, 255);
    range_check(r.frame_size, 0, 255);
    b.push_back(uint8_t(r.rate));
    b.push_back(uint8_t(r.frame_size));
    append_u16(b, r.state);
    Writer{b}.u32(uint32_t(r.speed)).u32(uint32_t(r.accel));
    for (unsigned v : {r.frame_base, r.frame_end, r.next_anim, r.next_frame, r.changes, r.first_change, r.commands,
                       r.first_command})
        append_u16(b, v);
}

void append_s16s(Bytes &b, const std::vector<int> &words) {
    for (int w : words) append_s16(b, w);
}

void append_slice(Bytes &out, const Bytes &d, size_t a, size_t b) {
    b = std::min(b, d.size());
    if (a < b) out.insert(out.end(), d.begin() + a, d.begin() + b);
}

void shift_anim_refs(Bytes &d, const Sections &s, size_t stride, unsigned index, unsigned n) {
    uint32_t na = count(d, s.anims), nd = count(d, s.dispatch), nm = count(d, s.moveables);
    for (uint32_t i = 0; i < na; i++) {                          // next animations
        size_t o = anim_at(s, i) + 20;
        if (u16(d, o) >= index) pack_u16(d, o, u16(d, o) + n);
    }
    for (uint32_t i = 0; i < nd; i++) {                          // state change targets
        size_t o = s.dispatch + 4 + 8 * size_t(i) + 4;
        if (u16(d, o) >= index) pack_u16(d, o, u16(d, o) + n);
    }
    for (uint32_t i = 0; i < nm; i++) {                          // first animations of the moveables
        size_t o = s.moveables + 4 + stride * i + 16;
        if (u16(d, o) >= index && u16(d, o) != 0xFFFF) pack_u16(d, o, u16(d, o) + n);
    }
}

unsigned second_first_anim(const std::vector<Moveable> &movs) {
    bool any = false;
    unsigned best = 0;
    for (auto &m : movs)
        if (m.id != 0 && (!any || m.anim < best)) best = m.anim, any = true;
    require(any, "no moveable but Lara");
    return best;
}

Bytes splice(const Bytes &d, const Sections &s, const Insert &in) {
    uint32_t na = count(d, s.anims), nc = count(d, s.commands), nf = count(d, s.frames);
    size_t at = anim_at(s, in.index), a_end = anim_at(s, na);
    size_t c_end = s.commands + 4 + 2 * size_t(nc), f_end = s.frames + 4 + 2 * size_t(nf);
    Bytes out;
    out.reserve(d.size() + in.records.size() + in.states.size() + in.ranges.size() + in.commands.size() +
                in.frames.size());
    append_slice(out, d, 0, s.anims);
    append_u32(out, na + in.anims);
    append_slice(out, d, s.anims + 4, at);
    out.insert(out.end(), in.records.begin(), in.records.end());
    append_slice(out, d, at, a_end);
    if (in.states.empty()) {
        append_slice(out, d, a_end, s.commands);
    } else {
        uint32_t nst = count(d, s.states), nd = count(d, s.dispatch);
        size_t st_at = s.states + 4 + 6 * in.states_at, st_end = s.states + 4 + 6 * size_t(nst);
        size_t dp_end = s.dispatch + 4 + 8 * size_t(nd);
        append_slice(out, d, a_end, s.states);
        append_u32(out, nst + in.new_states);
        append_slice(out, d, s.states + 4, st_at);
        out.insert(out.end(), in.states.begin(), in.states.end());
        append_slice(out, d, st_at, st_end);
        append_slice(out, d, st_end, s.dispatch);
        append_u32(out, nd + in.new_ranges);
        append_slice(out, d, s.dispatch + 4, dp_end);
        out.insert(out.end(), in.ranges.begin(), in.ranges.end());
        append_slice(out, d, dp_end, s.commands);
    }
    append_u32(out, nc + in.commands.size() / 2);
    append_slice(out, d, s.commands + 4, c_end);
    out.insert(out.end(), in.commands.begin(), in.commands.end());
    append_slice(out, d, c_end, s.frames);
    append_u32(out, nf + in.frames.size() / 2);
    append_slice(out, d, s.frames + 4, f_end);
    out.insert(out.end(), in.frames.begin(), in.frames.end());
    append_slice(out, d, f_end, d.size());
    return out;
}

}  // namespace anim
