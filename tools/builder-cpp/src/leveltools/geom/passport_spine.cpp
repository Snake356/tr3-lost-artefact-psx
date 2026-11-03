// passport_spine.py: the spine of the closed passport (object 158) is its one coloured face; it gets a 16-bit palette
// entry of its own (empty, used by no coloured face) set to dark red. Writes the level in place.
//     passport_spine.py level.tr2
#include <format>
#include <map>

#include "geom.h"

namespace geom {

namespace {

constexpr uint32_t PASSPORT_CLOSED = 158;
constexpr uint8_t RED[3] = {128, 24, 24};
constexpr size_t PAL16 = 4 + 768;

// offsets of the colour fields of all coloured faces, per mesh pointer
std::vector<std::vector<size_t>> faces(const Bytes &b, const Sections &s) {
    size_t base = s.mesh + 4, ptrs = s.meshptr + 4;
    std::vector<std::vector<size_t>> out;
    for (size_t m = 0; m < u32(b, s.meshptr); m++) {
        size_t o = base + u32(b, ptrs + 4 * m) + 10;
        int64_t nv = s16(b, o);
        o = add(o, 2 + 6 * nv);
        int64_t nn = s16(b, o);
        o = add(o, 2 + (nn > 0 ? 6 * nn : 2 * -nn));
        std::vector<size_t> fields;
        struct List {
            int64_t size, idx;
            bool coloured;
        };
        for (const List &l : {List{10, 4, false}, List{8, 3, false}, List{10, 4, true}, List{8, 3, true}}) {
            int64_t n = s16(b, o);
            o += 2;
            if (l.coloured)
                for (int64_t k = 0; k < n; k++) fields.push_back(size_t(int64_t(o) + l.size * k + 2 * l.idx));
            o = add(o, l.size * n);
        }
        out.push_back(std::move(fields));
    }
    return out;
}

bool all_zero(const Bytes &b, size_t o, size_t n) {
    for (size_t i = o; i < std::min(o + n, b.size()); i++)
        if (b[i]) return false;
    return true;
}

void passport_spine(ToolContext &ctx, const std::vector<std::string> &args) {
    if (args.empty()) throw Error("usage: passport_spine.py level.tr2");
    const std::string &path = args[0];
    Bytes b = read_file(ctx.path(path));
    Sections s = sections(b);
    std::map<uint32_t, std::pair<uint32_t, uint32_t>> mov;
    for (size_t k = 0; k < u32(b, s.moveables); k++)
        mov[u32(b, s.moveables + 4 + 18 * k)] = {u16(b, s.moveables + 8 + 18 * k), u16(b, s.moveables + 10 + 18 * k)};
    auto it = mov.find(PASSPORT_CLOSED);
    if (it == mov.end()) throw Error("KeyError: 158");
    auto [nmesh, start] = it->second;
    auto all_faces = faces(b, s);
    std::set<uint32_t> used;
    for (auto &fs : all_faces)
        for (size_t f : fs) used.insert(u16(b, f) >> 8);
    std::vector<size_t> spine;
    for (size_t m = start; m < size_t(start) + nmesh; m++)
        for (size_t f : all_faces.at(m)) spine.push_back(f);
    if (spine.size() != 1) throw Error(std::format("{} coloured faces in the passport", spine.size()));
    uint32_t old = u16(b, spine[0]);
    if (!all_zero(b, PAL16 + 4 * (old >> 8), 3)) throw Error("the spine is not black");
    uint32_t k = 0;
    for (uint32_t i = 1; i < 256 && !k; i++)
        if (!used.count(i) && all_zero(b, PAL16 + 4 * i, 4)) k = i;
    if (!k) throw Error("no free 16-bit palette entry");
    for (int i = 0; i < 3; i++) b[PAL16 + 4 * k + size_t(i)] = RED[i];
    put16(b, spine[0], k << 8 | (old & 0xFF));
    write_file(ctx.path(path), b);
    ctx.print(std::format("{}: passport spine {} -> 16-bit palette entry {} ({}, {}, {})", path, old >> 8, k, int(RED[0]),
                          int(RED[1]), int(RED[2])));
}

}  // namespace

REGISTER_TOOL("passport_spine.py", passport_spine);

}  // namespace geom
