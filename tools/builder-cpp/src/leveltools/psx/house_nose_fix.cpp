// house_nose_fix.py: gives Lara's nose its texture in a finished PSX level (the original HOUSE.PSX), as the
// other levels get it in the build. Two triangles of her head (mesh 14 of LARA_SKIN) show a crop meant for
// the eyebrows; the skin piece (art/lara/nose.bin, 40 x 32 PSX colours), toned to the level's own skin, goes
// as 4-bit texels into an empty 40 x 32 area of a tile, and three object textures are appended after the
// last one: two for the nose triangles, one of plain skin for the two triangles beside it. The level data
// grows by 48 bytes, taken from the padding at its end when it has room, else padded to 2048 again.
#include <algorithm>
#include <array>
#include <cmath>
#include <format>
#include <optional>
#include <set>

#include "../../build/data.h"
#include "../tool.h"
#include "psx_inventory.h"

namespace psx {
namespace {

constexpr uint32_t LARA_SKIN = 315, LARA = 0;
constexpr unsigned HEAD = 14;
constexpr unsigned W = 40, H = 32;
using Verts = std::array<unsigned, 3>;
using Point = std::pair<unsigned, unsigned>;
constexpr Verts TRIANGLES[] = {{10, 13, 17}, {10, 18, 13}};
constexpr Verts SKIN = {9, 0, 16};                        // a cheek triangle: the face's skin colour
constexpr double SKIN_PC[3] = {176, 128, 72};             // the same skin in the patch's level
constexpr Verts BESIDE[] = {{17, 35, 16}, {15, 32, 18}};  // beside the nose, the same crop: plain skin
// the piece's corners per nose triangle vertex, then plain skin in its top left corner
const std::vector<std::vector<Point>> CROPS = {{{39, 31}, {39, 0}, {0, 31}},
                                               {{39, 31}, {0, 31}, {39, 0}},
                                               {{1, 1}, {6, 1}, {1, 5}, {6, 5}}};

using Rgb = std::array<double, 3>;

Rgb rgb(unsigned c) { return {double((c & 31) << 3), double((c >> 5 & 31) << 3), double((c >> 10 & 31) << 3)}; }

struct Layout {
    uint32_t n_models = 0, n_tiles = 0, n_objtex = 0, n_spritetex = 0, n_roomtex = 0;
    size_t meshdata = 0, meshptr = 0, models = 0, tiles = 0, clut = 0, p_objtex = 0, objtex = 0, spritetex = 0,
           roomtex = 0, end = 0;
};

// The parts the fix needs, walked as psx_inventory walks the file, on to the end of the level data.
Layout layout(const Bytes &d) {
    Reader r{d};
    Layout z;
    r.s32();
    uint32_t n = r.u32();
    r.skip(4LL * n);
    if (n) r.skip(r.u32());
    for (int i = 0; i < 13; i++)
        if (int32_t s = r.s32()) {
            r.skip(s);
            r.skip(r.s32());
        }
    for (unsigned i = 0, rooms = r.u16(); i < rooms; i++) read_room(r);
    r.skip(2LL * r.u32());
    r.skip(729 * 2);
    r.skip(r.u32());
    r.skip(8LL * r.s32());
    auto list = [&](long long size, size_t *at = nullptr, uint32_t *count = nullptr) {
        uint32_t c = r.u32();
        if (count) *count = c;
        if (at) *at = r.o;
        r.skip(size * c);
    };
    list(2, &z.meshdata);
    list(4, &z.meshptr);
    for (long long size : {32, 6, 8, 2, 4, 2}) list(size);                  // the animation lists
    list(20, &z.models, &z.n_models);
    list(32);                                                               // statics
    list(32768, &z.tiles, &z.n_tiles);
    uint32_t ncl = r.u32();
    if (ncl & 0xFFFF0000) {                                                 // Japanese files: 2 bytes before
        r.o -= 2;
        ncl = r.u32();
    }
    z.clut = r.o;
    r.skip(64LL * ncl);
    z.p_objtex = r.o;
    list(16, &z.objtex, &z.n_objtex);
    list(16, &z.spritetex, &z.n_spritetex);
    for (long long size : {8, 16, 16}) list(size);                          // sprite sequences, cameras, sources
    uint32_t boxes = r.u32();
    r.skip(8LL * boxes);
    list(2);                                                                // overlaps
    r.skip(20LL * boxes);                                                   // zones
    list(2);                                                                // animated textures
    list(24);                                                               // entities
    r.skip(4);                                                              // horizon colour
    list(48, &z.roomtex, &z.n_roomtex);
    r.skip(370 * 2);                                                        // sound map
    list(8);                                                                // sound details
    r.skip(4);                                                              // sample count
    r.skip(16LL * r.u16());                                                 // cinematic frames
    r.skip(r.u16());                                                        // demo data
    z.end = r.o;                                                            // then only padding
    return z;
}

struct Triangle {
    size_t low, word;    // offsets of the low texture byte and of the triangle word
    Verts v;
    unsigned texture;
};

// PSX triangles come in groups of four: a word with the low texture bytes, then one word per triangle with
// its three vertex indices and the high texture byte.
std::vector<Triangle> head_triangles(const Bytes &d, const Layout &z) {
    std::optional<unsigned> mesh;
    for (uint32_t id : {LARA_SKIN, LARA})
        for (uint32_t i = 0; i < z.n_models && !mesh; i++) {
            size_t o = z.models + 20 * size_t(i);
            if (u32(d, o) == id) mesh = u16(d, o + 6) + HEAD;
        }
    require(mesh.has_value(), "no Lara in the models");
    size_t p = z.meshdata + u32(d, z.meshptr + 4 * size_t(*mesh));
    size_t q = p + 12 + u16(d, p + 10);
    unsigned count = u16(d, q);
    q += 4;
    std::vector<Triangle> out;
    size_t low = 0;
    for (unsigned t = 0; t < count; t++) {
        if (t % 4 == 0) {
            low = q;
            q += 4;
        }
        uint32_t b = u32(d, q);
        out.push_back({low + t % 4, q, {b & 0xFF, b >> 8 & 0xFF, b >> 16 & 0xFF}, u8(d, low + t % 4) | (b >> 24) << 8});
        q += 4;
    }
    return out;
}

const Triangle &find_triangle(const std::vector<Triangle> &tris, const Verts &v) {
    for (auto &t : tris)
        if (t.v == v) return t;
    throw Error(std::format("the head has no triangle {}, {}, {}", v[0], v[1], v[2]));
}

unsigned texel(const Bytes &d, const Layout &z, unsigned tile, unsigned x, unsigned y) {
    uint8_t b = u8(d, z.tiles + size_t(tile) * 32768 + y * 128 + x / 2);
    return x & 1 ? b >> 4 : b & 15;
}

struct Area {
    unsigned tile, x, y;
};

bool all_zero(Bytes::const_iterator a, Bytes::const_iterator b) {
    return std::all_of(a, b, [](uint8_t v) { return v == 0; });
}

// An empty 40 x 32 area that no object, room or sprite texture touches.
Area free_area(const Bytes &d, const Layout &z) {
    std::vector<Bytes> used(z.n_tiles, Bytes(65536));
    auto mark = [&](unsigned tile, std::initializer_list<unsigned> xs, std::initializer_list<unsigned> ys) {
        if (tile >= z.n_tiles) return;
        auto [x0, x1] = std::minmax(xs);
        auto [y0, y1] = std::minmax(ys);
        for (unsigned y = y0; y <= y1; y++) std::fill_n(used[tile].begin() + y * 256 + x0, x1 - x0 + 1, uint8_t(1));
    };
    auto entry = [&](size_t o) {
        Bytes e = slice(d, o, 16);
        // a triangle has the fourth point at 0, 0 (unless one of its three is there too)
        bool triangle = e[12] == 0 && e[13] == 0;
        for (int k = 0; k < 3 && triangle; k++) triangle = !(e[4 * k] == 0 && e[4 * k + 1] == 0);
        if (triangle)
            mark(u16(d, o + 6), {e[0], e[4], e[8]}, {e[1], e[5], e[9]});
        else
            mark(u16(d, o + 6), {e[0], e[4], e[8], e[12]}, {e[1], e[5], e[9], e[13]});
    };
    for (uint32_t i = 0; i < z.n_objtex; i++) entry(z.objtex + 16 * size_t(i));
    for (uint32_t i = 0; i < z.n_roomtex; i++)
        for (int k = 0; k < 3; k++) entry(z.roomtex + 48 * size_t(i) + 16 * k);
    for (uint32_t i = 0; i < z.n_spritetex; i++) {
        size_t o = z.spritetex + 16 * size_t(i);
        mark(u16(d, o + 10), {u8(d, o + 12), u8(d, o + 14)}, {u8(d, o + 13), u8(d, o + 15)});
    }
    for (unsigned t = 0; t < z.n_tiles; t++) {
        size_t base = z.tiles + size_t(t) * 32768;
        check_range(d, base, 32768);
        for (unsigned y = 0; y <= 256 - H; y += 8)
            for (unsigned x = 0; x <= 256 - W; x += 8) {
                bool empty = true;
                for (unsigned k = 0; k < H && empty; k++) {
                    auto u = used[t].cbegin() + (y + k) * 256 + x;
                    auto p = d.cbegin() + base + (y + k) * 128;
                    empty = all_zero(u, u + W) && all_zero(p + x / 2, p + (x + W) / 2);
                }
                if (empty) return {t, x, y};
            }
    }
    throw Error("no empty 40 x 32 area");
}

std::array<Rgb, 16> palette(const Bytes &d, const Layout &z, unsigned clut) {
    std::array<Rgb, 16> pal;
    for (unsigned i = 0; i < 16; i++) pal[i] = rgb(u16(d, z.clut + 32 * size_t(clut) + 2 * i));
    return pal;
}

// The piece toned to the level's skin: the mean colour of the cheek triangle's crop against the patch's.
std::vector<Rgb> toned_piece(const Bytes &d, const Layout &z, const Bytes &e) {
    auto pal = palette(d, z, u16(e, 2));
    long long sum[3] = {};
    long long samples = 0;
    auto [ylo, yhi] = std::minmax({e[1], e[5], e[9]});
    auto [xlo, xhi] = std::minmax({e[0], e[4], e[8]});
    for (unsigned y = ylo; y <= yhi; y++)
        for (unsigned x = xlo; x <= xhi; x++) {
            const Rgb &c = pal[texel(d, z, u16(e, 6), x, y)];
            for (int k = 0; k < 3; k++) sum[k] += (long long)c[k];
            samples++;
        }
    Rgb tone;
    for (int k = 0; k < 3; k++) tone[k] = double(sum[k]) / double(samples) / SKIN_PC[k];

    const Bytes &raw = data_file("prj/art/lara/nose.bin");
    require(raw.size() == size_t(W * H * 2), "art/lara/nose.bin is not 40 x 32 colours");
    std::vector<Rgb> piece(W * H);
    for (size_t i = 0; i < piece.size(); i++) {
        Rgb c = rgb(u16(raw, 2 * i));
        for (int k = 0; k < 3; k++) piece[i][k] = std::min(248.0, c[k] * tone[k]);   // the Japanese skin is lighter
    }
    return piece;
}

struct Match {
    double err = 0;
    unsigned clut = 0;
    std::vector<uint8_t> idx;
};

// The head's CLUT that matches the piece best, and the piece's texels in it. The distances are computed as
// Python computes them (pow), so that ties and the reported error come out the same.
Match best_clut(const Bytes &d, const Layout &z, const std::vector<Triangle> &tris, const std::vector<Rgb> &piece) {
    std::set<unsigned> cluts;
    for (auto &t : tris)
        if (t.texture >= 256) cluts.insert(u16(d, z.objtex + 16 * size_t(t.texture) + 2));
    std::optional<Match> best;
    for (unsigned clut : cluts) {
        auto pal = palette(d, z, clut);
        Match m{0, clut, {}};
        for (auto &c : piece) {
            double nearest = 0;
            unsigned index = 0;
            for (unsigned i = 1; i < 16; i++) {            // index 0: see-through
                double dist = 0;
                for (int k = 0; k < 3; k++) dist += std::pow(c[k] - pal[i][k], 2.0);
                if (i == 1 || dist < nearest) {
                    nearest = dist;
                    index = i;
                }
            }
            m.idx.push_back(uint8_t(index));
            m.err += std::pow(nearest, 0.5);
        }
        if (!best || m.err < best->err) best = std::move(m);
    }
    require(best.has_value(), "the head has no CLUT");
    return *best;
}

struct Fix {
    Bytes level;
    std::string report;
};

std::optional<Fix> fix(const Bytes &d) {
    Layout z = layout(d);
    auto tris = head_triangles(d, z);
    std::vector<Triangle> sel;
    for (auto &v : TRIANGLES) sel.push_back(find_triangle(tris, v));
    if (std::all_of(sel.begin(), sel.end(), [&](const Triangle &t) { return (long long)t.texture >= (long long)z.n_objtex - 3; }))
        return std::nullopt;
    auto objtex = [&](unsigned i) { return slice(d, z.objtex + 16 * size_t(i), 16); };
    for (auto &t : sel) {
        Bytes e = objtex(t.texture);
        auto [lo, hi] = std::minmax({e[0], e[4], e[8]});
        require(hi - lo < 8, "the triangles do not have the eyebrow crop");
    }
    Area area = free_area(d, z);
    auto piece = toned_piece(d, z, objtex(find_triangle(tris, SKIN).texture));
    Match m = best_clut(d, z, tris, piece);

    Bytes out = d;
    for (unsigned y = 0; y < H; y++) {
        size_t row = z.tiles + size_t(area.tile) * 32768 + (area.y + y) * 128 + area.x / 2;
        for (unsigned k = 0; k < W / 2; k++) put8(out, row + k, m.idx[y * W + 2 * k] | m.idx[y * W + 2 * k + 1] << 4);
    }
    Bytes entries;
    for (auto &corners : CROPS) {
        Bytes e = objtex(sel[0].texture);
        put16(e, 2, m.clut);
        put16(e, 6, area.tile);
        auto set_point = [&](size_t k, Point c) {
            unsigned x = area.x + c.first, y = area.y + c.second;
            require(x < 256 && y < 256, "the piece's corner is off the tile");
            e[4 * k] = uint8_t(x);
            e[4 * k + 1] = uint8_t(y);
        };
        for (size_t k = 0; k < corners.size(); k++) set_point(k, corners[k]);
        if (corners.size() == 3) set_point(3, corners[2]);
        entries.insert(entries.end(), e.begin(), e.end());
    }
    uint32_t n = z.n_objtex;
    auto point = [&](const Triangle &t, uint32_t tex) {
        put8(out, t.low, tex & 0xFF);
        put8(out, t.word + 3, tex >> 8);
    };
    for (size_t k = 0; k < sel.size(); k++) point(sel[k], n + uint32_t(k));
    for (auto &v : BESIDE) point(find_triangle(tris, v), n + 2);
    put32(out, z.p_objtex, n + 3);
    size_t end = z.objtex + 16 * size_t(n);
    check_range(out, end, 0);
    out.insert(out.begin() + end, entries.begin(), entries.end());
    if ((long long)d.size() - (long long)z.end >= (long long)entries.size() && d.size() >= entries.size() &&
        all_zero(d.end() - entries.size(), d.end()))
        out.resize(out.size() - entries.size());                 // the padding takes the 48 bytes
    else
        out.resize(out.size() + (2048 - out.size() % 2048) % 2048);
    return Fix{std::move(out), std::format("tile {} at {}, {}, CLUT {} (error {:.1f}), object textures {} to {}",
                                           area.tile, area.x, area.y, m.clut, m.err / double(piece.size()), n, n + 2)};
}

void run_house_nose_fix(ToolContext &ctx, const std::vector<std::string> &args) {
    require(!args.empty(), "usage: house_nose_fix.py in.psx [out.psx]");
    const std::string &dst = args.size() > 1 ? args[1] : args[0];
    Bytes d = read_file(ctx.path(args[0]));
    Bytes out = d;
    if (auto f = fix(d)) {
        ctx.print(f->report);
        out = std::move(f->level);
    } else {
        ctx.print(dst + ": has the nose already");
    }
    write_file(ctx.path(dst), out);
    ctx.print(std::format("{}: {:+d} bytes", dst, (long long)out.size() - (long long)d.size()));
}

REGISTER_TOOL("house_nose_fix.py", run_house_nose_fix);

}  // namespace
}  // namespace psx
