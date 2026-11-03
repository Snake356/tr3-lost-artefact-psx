// close_dome_tip.py: closes the tip of the sky dome in a finished TR3 PSX level (the original HOUSE.PSX).
// The PSX sky mesh has no triangles, so the 16 around the apex are missing; they are added as 16
// rectangles (i, 0, i+1, i+1) in the cap colour. The mesh data grows by 96 bytes, the word count and the
// mesh pointers behind the dome follow.
#include <algorithm>
#include <array>
#include <format>
#include <set>

#include "../tool.h"
#include "psx_inventory.h"

namespace psx {
namespace {

constexpr unsigned SKY_VERTICES = 66, RING = 16;

struct Rect {
    uint16_t texture;
    std::array<uint8_t, 4> v;    // stored order a, b, d, c
};

struct Result {
    Bytes level;
    uint32_t mesh_offset;
    unsigned before, after, colour, moved;
    long long delta;
};

Result close_dome_tip(const Bytes &d) {
    Inventory z = inventory(d);
    require(z.ok(), z.status);
    size_t pmd = find_mesh_block(d, z);
    size_t md = pmd + 4, pptr = md + size_t(z.meshdata) * 2 + 4;
    std::vector<uint32_t> ptrs(z.meshptr);
    for (size_t i = 0; i < ptrs.size(); i++) ptrs[i] = u32(d, pptr + 4 * i);

    // the sky mesh: 66 vertices, vertex 0 on the y axis at the top, only rectangles
    std::vector<std::pair<uint32_t, unsigned>> sky;
    for (uint32_t off : std::set<uint32_t>(ptrs.begin(), ptrs.end())) {
        size_t m = md + off;
        if (u8(d, m) != SKY_VERTICES || u8(d, m + 1) != 0) continue;
        int16_t y_min = s16(d, m + 6);
        for (unsigned i = 0; i < SKY_VERTICES; i++) y_min = std::min(y_min, s16(d, m + 4 + 8 * i + 2));
        if (s16(d, m + 4) == 0 && s16(d, m + 8) == 0 && s16(d, m + 6) == y_min) sky.push_back({off, u16(d, m + 2)});
    }
    require(sky.size() == 1, "the sky mesh is found " + std::to_string(sky.size()) + " times");
    auto [off, nr] = sky[0];
    size_t m = md + off, verts = m + 4, rstart = verts + SKY_VERTICES * 8;

    // rectangles in pairs of 12 bytes, an odd last one in 8
    std::vector<Rect> rects;
    size_t p = rstart;
    for (unsigned i = 0; i < nr; i += 2) {
        Rect a{u16(d, p), {u8(d, p + 4), u8(d, p + 5), u8(d, p + 6), u8(d, p + 7)}};
        rects.push_back(a);
        if (i + 1 < nr) {
            rects.push_back({u16(d, p + 2), {u8(d, p + 8), u8(d, p + 9), u8(d, p + 10), u8(d, p + 11)}});
            p += 12;
        } else {
            p += 8;
        }
    }
    size_t old_len = p - m;
    auto uses = [](const Rect &r, auto pred) { return std::any_of(r.v.begin(), r.v.end(), pred); };
    require(std::none_of(rects.begin(), rects.end(), [&](const Rect &r) { return uses(r, [](uint8_t v) { return v == 0; }); }),
            "the tip is already closed");
    // cap colour: the coloured rectangles touching the first ring (vertices 1 to 16)
    std::set<unsigned> caps;
    bool any_cap = false;
    for (auto &r : rects)
        if (r.texture < 0x100 && uses(r, [](uint8_t v) { return v >= 1 && v <= RING; })) {
            caps.insert(r.texture);
            any_cap = true;
        }
    require(any_cap && caps.size() == 1, "no single cap colour");
    uint16_t col = uint16_t(*caps.begin());

    std::vector<Rect> all;
    for (auto &r : rects)
        if (r.texture < 0x100) all.push_back(r);
    for (unsigned i = 1; i <= RING; i++) {
        uint8_t next = uint8_t(i % RING + 1);
        all.push_back({col, {uint8_t(i), 0, next, next}});
    }
    for (auto &r : rects)
        if (r.texture >= 0x100) all.push_back(r);

    Bytes mesh;
    Writer w{mesh};
    w.u8(SKY_VERTICES).u8(0).u16(unsigned(all.size())).bytes(d.data() + verts, rstart - verts);
    for (size_t i = 0; i < all.size(); i += 2) {
        bool pair = i + 1 < all.size();
        w.u16(all[i].texture).u16(pair ? all[i + 1].texture : 0).bytes(all[i].v.data(), 4);
        if (pair) w.bytes(all[i + 1].v.data(), 4);
    }
    long long delta = (long long)mesh.size() - (long long)old_len;
    require(delta == 96, "the mesh grows by " + std::to_string(delta) + " bytes");

    Bytes out;
    out.reserve(d.size() + mesh.size());
    out.insert(out.end(), d.begin(), d.begin() + m);
    out.insert(out.end(), mesh.begin(), mesh.end());
    out.insert(out.end(), d.begin() + m + old_len, d.end());
    put32(out, pmd, uint32_t(z.meshdata + delta / 2));
    pptr += size_t(delta);
    unsigned moved = 0;
    for (size_t i = 0; i < ptrs.size(); i++)
        if (ptrs[i] > off) {
            put32(out, pptr + 4 * i, uint32_t(ptrs[i] + delta));
            moved++;
        }
    return {std::move(out), off, nr, unsigned(all.size()), col, moved, delta};
}

void run_close_dome_tip(ToolContext &ctx, const std::vector<std::string> &args) {
    require(args.size() >= 2, "usage: close_dome_tip.py in.psx out.psx");
    Bytes d = read_file(ctx.path(args[0]));
    Result r = close_dome_tip(d);
    write_file(ctx.path(args[1]), r.level);
    Inventory z = inventory(r.level);
    require(z.ok() && z.meshdata == inventory(d).meshdata + 48, z.status);
    ctx.print(std::format("{}: sky mesh @+{:#x}, {} -> {} rectangles, cap colour {}, mesh data +{} bytes, "
                          "{} pointers moved", args[0], r.mesh_offset, r.before, r.after, r.colour, r.delta, r.moved));
}

REGISTER_TOOL("close_dome_tip.py", run_close_dome_tip);

}  // namespace
}  // namespace psx
