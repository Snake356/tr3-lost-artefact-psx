// nebel_raeume.py: marks the rooms with a mist layer (additive faces lying flat on a solid floor) with room flag
// 0x4000 and their strength in bits 0x1800, for the executable's mist sorting; rooms whose additive faces lie
// over a water room are not marked.
//     nebel_raeume.py level.tr2 [--list]
#include <algorithm>
#include <format>
#include <map>

#include "geom.h"

namespace geom {

namespace {

constexpr int MIST = 0x4000, STRENGTH_BITS = 0x1800;

int level_bits(int strength) {     // strength -> the value in bits 0x1800 (the PS1 bias: 128 >> it)
    switch (strength) {
    case 128: return 0;
    case 64: return 1;
    case 32: return 2;
    case 16: return 3;
    }
    throw Error("KeyError: " + std::to_string(strength));
}

int32_t room_flags(int32_t flags, int strength) {
    flags &= ~(MIST | STRENGTH_BITS);
    return strength == 0 ? flags : flags | MIST | level_bits(strength) << 11;
}

// per level: room -> strength (0 = the game's order)
std::map<int, int> own_strengths(const std::string &level) {
    std::map<int, int> m;
    auto set = [&](std::initializer_list<int> rooms, int s) {
        for (int r : rooms) m[r] = s;
    };
    if (level == "scotland.tr2") set({44, 55, 56, 59, 64, 65, 84, 97, 105, 109, 123}, 32);
    else if (level == "zoo.tr2") set({51, 124, 130, 133}, 16);
    else if (level == "slinc.tr2") set({3, 5, 7, 30, 34, 36, 52, 56, 61, 66, 68}, 16);
    else if (level == "willsden.tr2") {
        set({6, 25, 26, 28, 35, 36, 37, 56, 62, 81}, 64);
        m[86] = 16;
        m[90] = 16;
    } else if (level == "undersea.tr2") m[19] = 0;
    return m;
}

const Room &room_at(const std::vector<Room> &rooms, int64_t i) {    // Python indexing (negative from the end)
    if (i < 0) i += int64_t(rooms.size());
    if (i < 0) throw Error("list index out of range");
    return rooms.at(size_t(i));
}

// (mist rooms, water rooms): rooms with additive flat faces on a floor / over a water room
std::pair<std::set<int>, std::set<int>> classify(const Bytes &b) {
    Layout L = layout(b);
    std::vector<uint32_t> attr;
    for (size_t i = 0; i < L.objtex.count; i++) attr.push_back(u16(b, L.objtex.data() + 20 * i));
    std::vector<Room> rooms = parse_rooms(b).rooms;
    std::set<int> mist, water;
    for (size_t ri = 0; ri < rooms.size(); ri++) {
        const Room &r = rooms[ri];
        auto face = [&](const int32_t *f, int k) {
            if (attr.at(size_t(f[k] & 0x7FFF)) != 2) return;
            int64_t ylo = INT64_MAX, yhi = INT64_MIN, sx = 0, sy = 0, sz = 0;
            for (int i = 0; i < k; i++) {
                const auto &v = r.verts.at(size_t(f[i]));
                ylo = std::min<int64_t>(ylo, v[1]);
                yhi = std::max<int64_t>(yhi, v[1]);
                sx += v[0];
                sy += v[1];
                sz += v[2];
            }
            if (yhi - ylo >= 128) return;
            int64_t xi = floordiv(floordiv(sx, k), 1024), zi = floordiv(floordiv(sz, k), 1024);
            if (!(0 <= xi && xi < r.xs && 0 <= zi && zi < r.zs)) return;
            const auto &s = r.sec(int(xi), int(zi));
            const Room *below = s[2] != 255 ? &room_at(rooms, s[2]) : nullptr;
            if (below && (below->flags & 1 || (below->alt != -1 && below->alt != 0xFFFF && room_at(rooms, below->alt).flags & 1)))
                water.insert(int(ri));
            else if (std::abs(floordiv(sy, k) - int64_t(s[3]) * 256) <= 64)
                mist.insert(int(ri));
        };
        for (auto &q : r.quads) face(q.data(), 4);
        for (auto &t : r.tris) face(t.data(), 3);
    }
    return {mist, water};
}

std::string dict_repr(const std::map<int, int> &m) {
    std::string s = "{";
    for (auto &[k, v] : m) s += (s.size() > 1 ? ", " : "") + std::format("{}: {}", k, v);
    return s + "}";
}

void nebel_raeume(ToolContext &ctx, const std::vector<std::string> &args) {
    if (args.empty()) throw Error("usage: nebel_raeume.py level.tr2 [--list]");
    const std::string &path = args[0];
    Bytes b = read_file(ctx.path(path));
    auto [mist, water] = classify(b);
    for (int r : mist)
        if (water.count(r)) throw Error("rooms with mist and water");
    std::map<int, int> own = own_strengths(lower(basename(path)));
    for (auto &[r, s] : own)
        if (!mist.count(r)) throw Error("room " + std::to_string(r) + " has no mist layer");
    std::map<int, int> strength;
    for (int ri : mist) strength[ri] = own.count(ri) ? own[ri] : 128;
    if (std::find(args.begin(), args.end(), "--list") != args.end()) {
        ctx.print(dict_repr(strength));
        return;
    }
    ParsedRooms P = parse_rooms(b);
    for (auto &r : P.rooms)
        if (r.flags & (MIST | STRENGTH_BITS)) throw Error("a room has the mist flags already");
    for (auto &[ri, st] : strength) P.rooms.at(size_t(ri)).flags = room_flags(P.rooms[size_t(ri)].flags, st);
    Bytes out(b.begin(), b.begin() + ptrdiff_t(P.start));
    Writer{out}.bytes(build_rooms(P.rooms));
    out.insert(out.end(), b.begin() + ptrdiff_t(P.end), b.end());
    write_file(ctx.path(path), out);
    std::map<int, int> shown;
    for (auto &[ri, st] : strength)
        if (st) shown[ri] = st;
    ctx.print(std::format("{}: mist rooms {}", path, dict_repr(shown)));
}

}  // namespace

REGISTER_TOOL("nebel_raeume.py", nebel_raeume);

}  // namespace geom
