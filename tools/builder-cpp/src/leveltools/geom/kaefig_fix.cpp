// kaefig_fix.py: the stacked cages of Sleeping with the Fishes 2 (static 35) flicker on the PS1 (no z-buffer). The
// outer bottom plate is turned to face down, and the lower cages get a model of their own (static 36 and its mesh
// slot) without roof and inner ceiling and with walls 5 units lower. Writes the level in place.
//     kaefig_fix.py underse2.tr2
#include <algorithm>
#include <format>
#include <map>

#include "geom.h"

namespace geom {

namespace {

constexpr uint32_t CAGE = 35, LOWER = 36;
constexpr int FLOOR_Y = -2304;               // the lower cages stand on the room's floor

struct MeshLayout {
    std::vector<std::array<int, 3>> verts;
    size_t vstart;
    int nq;
    size_t quads;
};

MeshLayout mesh_layout(const Bytes &b, size_t start) {
    MeshLayout m{};
    size_t o = start + 10;
    int nv = s16(b, o);
    for (int i = 0; i < nv; i++) {
        size_t v = o + 2 + 6 * size_t(i);
        m.verts.push_back({s16(b, v), s16(b, v + 2), s16(b, v + 4)});
    }
    m.vstart = o + 2;
    o = add(o, 2 + 6 * int64_t(nv));
    int64_t nn = s16(b, o);
    o = add(o, 2 + (nn > 0 ? 6 * nn : 2 * -nn));
    m.nq = s16(b, o);
    m.quads = o + 2;
    return m;
}

// (y, texture) -> quad number of the cage's full plates
std::map<std::pair<int, int>, int> plates(const Bytes &b, const MeshLayout &m) {
    std::map<std::pair<int, int>, int> out;
    for (int k = 0; k < m.nq; k++) {
        size_t q = m.quads + 10 * size_t(k);
        std::set<int> ys;
        int minx = INT32_MAX;
        for (int i = 0; i < 4; i++) {
            const auto &v = m.verts.at(u16(b, q + 2 * size_t(i)));
            ys.insert(v[1]);
            minx = std::min(minx, std::abs(v[0]));
        }
        if (ys.size() == 1 && minx > 500) out[{*ys.begin(), int(u16(b, q + 8) & 0x7FFF)}] = k;
    }
    return out;
}

int plate(const std::map<std::pair<int, int>, int> &pl, int y, int tex) {
    auto it = pl.find({y, tex});
    if (it == pl.end()) throw Error(std::format("KeyError: ({}, {})", y, tex));
    return it->second;
}

std::map<uint32_t, size_t> static_entries(const Bytes &b, const Sections &s) {
    std::map<uint32_t, size_t> entry;
    size_t info = s.statics + 4;
    for (size_t k = 0; k < u32(b, s.statics); k++) entry[u32(b, info + 32 * k)] = info + 32 * k;
    return entry;
}

size_t entry_of(const std::map<uint32_t, size_t> &e, uint32_t id) {
    auto it = e.find(id);
    if (it == e.end()) throw Error("KeyError: " + std::to_string(id));
    return it->second;
}

void kaefig_fix(ToolContext &ctx, const std::vector<std::string> &args) {
    if (args.empty()) throw Error("usage: kaefig_fix.py underse2.tr2");
    const std::string &path = args[0];
    Bytes b = read_file(ctx.path(path));
    Sections s = sections(b);
    auto entry = static_entries(b, s);
    uint32_t mesh = u16(b, entry_of(entry, CAGE) + 4), spare = u16(b, entry_of(entry, LOWER) + 4);
    size_t ptrs = s.meshptr + 4;
    size_t base = s.mesh + 4;
    size_t start = base + u32(b, ptrs + 4 * size_t(mesh));
    MeshLayout m = mesh_layout(b, start);
    auto pl = plates(b, m);

    // the outer bottom plate faces down (in every cage)
    size_t q = m.quads + 10 * size_t(plate(pl, -2, 1671));
    uint32_t q0 = u16(b, q), q1 = u16(b, q + 2), q2 = u16(b, q + 4), q3 = u16(b, q + 6);
    put16(b, q, q1);
    put16(b, q + 2, q0);
    put16(b, q + 4, q3);
    put16(b, q + 6, q2);

    // the lower cages' model: a copy without roof and inner ceiling, walls ending 5 units lower
    std::vector<uint32_t> ends;
    for (size_t i = 0; i < u32(b, s.meshptr); i++) ends.push_back(u32(b, ptrs + 4 * i));
    std::sort(ends.begin(), ends.end());
    auto next = std::find_if(ends.begin(), ends.end(), [&](uint32_t e) { return e > start - base; });
    if (next == ends.end()) throw Error("list index out of range");
    size_t size = *next - (start - base);
    Bytes copy = slice(b, start, size);
    for (int tex : {1671, 1670}) {
        size_t o = m.quads - start + 10 * size_t(plate(pl, -1031, tex));
        uint32_t v = u16(copy, o);
        for (int i = 0; i < 4; i++) put16(copy, o + 2 * size_t(i), v);
    }
    for (size_t i = 0; i < m.verts.size(); i++)
        if (m.verts[i][1] == -1031) put16(copy, m.vstart - start + 6 * i + 2, uint32_t(-1026));
    uint32_t words = u32(b, s.mesh);
    size_t end = base + 2 * size_t(words);
    size_t pad = size_t(pymod(-int64_t(end - base), 4));
    Bytes ins(pad, 0);
    Writer{ins}.bytes(copy);
    b.insert(b.begin() + ptrdiff_t(end), ins.begin(), ins.end());
    put32(b, s.mesh, uint32_t(words + (pad + copy.size()) / 2));
    s = sections(b);                         // the pointer list lies behind the mesh data and has moved
    put32(b, s.meshptr + 4 + 4 * size_t(spare), uint32_t(end - base + pad));

    // static 36 gets static 35's boxes and flags, the lower cages use it
    entry = static_entries(b, s);
    size_t lower = entry_of(entry, LOWER), cage = entry_of(entry, CAGE);
    std::copy(b.begin() + ptrdiff_t(cage + 6), b.begin() + ptrdiff_t(cage + 32), b.begin() + ptrdiff_t(lower + 6));
    ParsedRooms P = parse_rooms(b);
    int switched = 0;
    for (auto &r : P.rooms)
        for (auto &st : r.statics)
            if (u16(st, 18) == CAGE && s32(st, 4) == FLOOR_Y) {
                put16(st, 18, LOWER);
                switched++;
            }
    Bytes out(b.begin(), b.begin() + ptrdiff_t(P.start));
    Writer{out}.bytes(build_rooms(P.rooms));
    out.insert(out.end(), b.begin() + ptrdiff_t(P.end), b.end());
    write_file(ctx.path(path), out);
    ctx.print(std::format("{}: cage mesh {} bottom plate turned down, lower cage model in mesh slot {} ({} bytes), "
                          "{} lower cages on static {}",
                          path, mesh, spare, copy.size(), switched, LOWER));
}

}  // namespace

REGISTER_TOOL("kaefig_fix.py", kaefig_fix);

}  // namespace geom
