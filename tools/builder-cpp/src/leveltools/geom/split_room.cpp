// split_room.py: splits a room of a TR3 PC level at a block boundary into two rooms (A keeps the number and the
// sectors 0..k, B is appended with the sectors k-1..end, a portal between them); the alternate room of a flipmap
// is split the same way. Room links, FloorData links (with trmod), items and cameras follow; then checked.
//
//     split_room.py <trmod> in.tr2 out.tr2 <room> <x|z> <k>
#include <algorithm>
#include <format>
#include <map>
#include <regex>

#include "geom.h"

namespace geom {

namespace {

constexpr int WALL = -127;
constexpr int NOBOX = 0x7FF0;

using Portal = std::array<int32_t, 16>;
using Vertex3 = std::array<int64_t, 3>;
using PortalShape = std::array<Vertex3, 4>;     // the 4 vertices in world coordinates, sorted

const std::regex &link_regex() {
    static const std::regex re(R"(\b(portal|floorportal|ceilingportal)\((\d+)\))");
    return re;
}

std::string after_colon(const std::string *c) {
    if (!c) return "";
    size_t p = c->find(':');
    return p == std::string::npos ? *c : c->substr(p + 1);
}

// Trigger cameras are written room-relative by trmod: compared in world coordinates.
std::string world_cameras(const std::string &text, const std::vector<Room> &rooms) {
    static const std::regex re(R"(camera\((\d+),(-?\d+),(-?\d+),)");
    std::string out;
    size_t last = 0;
    for (auto it = std::sregex_iterator(text.begin(), text.end(), re); it != std::sregex_iterator(); ++it) {
        const std::smatch &m = *it;
        int64_t r = parse_int(m[1].str()), x = parse_int(m[2].str()), z = parse_int(m[3].str());
        const Room &R = rooms.at(size_t(r));
        out += text.substr(last, size_t(m.position(0)) - last);
        out += std::format("camera(W{},{},", R.x + x, R.z + z);
        last = size_t(m.position(0) + m.length(0));
    }
    return out + text.substr(last);
}

struct Half {
    int a, bi;
    int32_t Rx, Rz;
};

struct SplitResult {
    int bi;
    std::vector<std::array<int, 4>> trm;    // portal sectors to set: room, x, z, target
};

// Splits rooms[ri] in place (A) and appends B. link = (A, B) of the base room when rooms[ri] is its alternate
// room: the portals and portal sectors between the halves then name the base rooms.
SplitResult split(std::vector<Room> &rooms, int ri, bool ax_x, int k, std::optional<std::pair<int, int>> link) {
    const Room R = rooms.at(size_t(ri));
    const int ax = ax_x ? 0 : 2;
    const int n_ax = ax_x ? R.xs : R.zs;
    const int n_other = ax_x ? R.zs : R.xs;
    if (!(2 <= k && k <= n_ax - 2)) throw Error(std::format("k must be 2..{}", n_ax - 2));
    const int cut = k * 1024;
    const int off = (k - 1) * 1024;
    Room A, B;
    for (Room *h : {&A, &B}) {
        h->yb = R.yb;
        h->yt = R.yt;
        h->ambient = R.ambient;
        h->flags = R.flags;
        h->tail = R.tail;
        h->alt = -1;
        h->lights = R.lights;
    }
    A.x = R.x;
    A.z = R.z;
    B.x = ax_x ? R.x + off : R.x;
    B.z = ax_x ? R.z : R.z + off;

    // sectors
    auto cell = [&](int i, int j) { return ax_x ? R.sec(i, j) : R.sec(j, i); };   // i along the axis
    struct Border {
        bool is_a;
        int i, j;
    };
    std::vector<Border> border;
    for (int half = 0; half < 2; half++) {
        Room &h = half ? B : A;
        int lo = half ? k - 1 : 0, hi = half ? n_ax - 1 : k, bidx = half ? 0 : k;
        int n = hi - lo + 1;
        auto cells = std::vector<std::vector<std::array<int32_t, 6>>>(size_t(n), std::vector<std::array<int32_t, 6>>(size_t(n_other)));
        for (int i = lo; i <= hi; i++)
            for (int j = 0; j < n_other; j++) cells[size_t(i - lo)][size_t(j)] = cell(i, j);
        for (int j = 1; j < n_other - 1; j++) {
            auto &s = cells[size_t(bidx)][size_t(j)];
            if (s[3] != WALL) {             // open: becomes a wall with a portal
                s = {0, NOBOX | (s[1] & 0xF), 255, WALL, 255, WALL};
                border.push_back({half == 0, bidx, j});
            }
        }
        h.sectors.clear();
        if (ax_x) {
            h.xs = n;
            h.zs = n_other;
            for (int i = 0; i < n; i++)
                for (int j = 0; j < n_other; j++) h.sectors.push_back(cells[size_t(i)][size_t(j)]);
        } else {
            h.zs = n;
            h.xs = n_other;
            for (int j = 0; j < n_other; j++)
                for (int i = 0; i < n; i++) h.sectors.push_back(cells[size_t(i)][size_t(j)]);
        }
    }

    // geometry: faces by position, faces across the cut go into both halves
    std::map<int32_t, int32_t> vmap_a, vmap_b;
    auto vertex = [&](Room &h, int32_t vi) {
        auto &vmap = &h == &A ? vmap_a : vmap_b;
        auto it = vmap.find(vi);
        if (it != vmap.end()) return it->second;
        auto v = R.verts.at(size_t(vi));
        if (&h == &B) v[size_t(ax)] -= off;
        int32_t n = int32_t(h.verts.size());
        vmap[vi] = n;
        h.verts.push_back(v);
        return n;
    };
    auto place = [&](const auto &face, int nv, auto member) {
        int32_t lo = INT32_MAX, hi = INT32_MIN;
        for (int i = 0; i < nv; i++) {
            int32_t c = R.verts.at(size_t(face[size_t(i)]))[size_t(ax)];
            lo = std::min(lo, c);
            hi = std::max(hi, c);
        }
        for (Room *h : {&A, &B}) {
            bool in = h == &A ? lo < cut : (hi > cut || lo >= cut);
            if (!in) continue;
            auto f = face;
            for (int i = 0; i < nv; i++) f[size_t(i)] = vertex(*h, face[size_t(i)]);
            (h->*member).push_back(f);
        }
    };
    for (auto &f : R.quads) place(f, 4, &Room::quads);
    for (auto &f : R.tris) place(f, 3, &Room::tris);
    for (auto &s : R.sprites) {
        Room &h = R.verts.at(size_t(s[0]))[size_t(ax)] < cut ? A : B;
        h.sprites.push_back({vertex(h, s[0]), s[1]});
    }

    // the room's own portals
    auto clip = [&](const Portal &pt, int64_t lo, int64_t hi, int64_t shift) {
        Portal p = pt;
        for (int i = 0; i < 4; i++) {
            int32_t &q = p[size_t(4 + i * 3 + ax)];
            q = int32_t(std::min(std::max(int64_t(q), lo), hi) - shift);
        }
        return p;
    };
    for (auto &pt : R.portals) {
        int32_t lo = INT32_MAX, hi = INT32_MIN;
        for (int i = 0; i < 4; i++) {
            lo = std::min(lo, pt[size_t(4 + i * 3 + ax)]);
            hi = std::max(hi, pt[size_t(4 + i * 3 + ax)]);
        }
        if (hi <= cut) {
            A.portals.push_back(pt);
        } else if (lo >= cut) {
            B.portals.push_back(clip(pt, -1000000000, 1000000000, off));
        } else {
            A.portals.push_back(clip(pt, lo, cut, 0));
            B.portals.push_back(clip(pt, cut, hi, off));
        }
    }

    // portals between A and B (vertex order as the level's own portals)
    const int32_t lo_o = 1024, hi_o = (n_other - 1) * 1024, t = R.yt, bt = R.yb;
    std::array<std::array<int32_t, 3>, 4> pa, pb;
    std::array<int32_t, 3> na, nb;
    if (ax_x) {
        pa = {{{cut, t, lo_o}, {cut, bt, lo_o}, {cut, bt, hi_o}, {cut, t, hi_o}}};
        pb = {{{1023, bt, lo_o}, {1023, t, lo_o}, {1023, t, hi_o}, {1023, bt, hi_o}}};
        na = {-1, 0, 0};
        nb = {1, 0, 0};
    } else {
        pa = {{{lo_o, bt, cut}, {lo_o, t, cut}, {hi_o, t, cut}, {hi_o, bt, cut}}};
        pb = {{{lo_o, t, 1023}, {lo_o, bt, 1023}, {hi_o, bt, 1023}, {hi_o, t, 1023}}};
        na = {0, 0, -1};
        nb = {0, 0, 1};
    }
    const int bi = int(rooms.size());
    const int to_a = link ? link->first : ri, to_b = link ? link->second : bi;
    auto make = [](int target, const std::array<int32_t, 3> &n, const std::array<std::array<int32_t, 3>, 4> &v) {
        Portal p{};
        p[0] = target;
        for (int i = 0; i < 3; i++) p[size_t(1 + i)] = n[size_t(i)];
        for (int q = 0; q < 4; q++)
            for (int i = 0; i < 3; i++) p[size_t(4 + q * 3 + i)] = v[size_t(q)][size_t(i)];
        return p;
    };
    A.portals.push_back(make(to_b, na, pa));
    B.portals.push_back(make(to_a, nb, pb));

    // static meshes by position
    for (auto &s : R.statics) {
        int64_t w = ax_x ? int64_t(s32(s, 0)) - R.x : int64_t(s32(s, 8)) - R.z;
        (w < cut ? A : B).statics.push_back(s);
    }

    rooms[size_t(ri)] = std::move(A);
    rooms.push_back(std::move(B));
    SplitResult res{bi, {}};
    for (auto &e : border) {
        int x = ax_x ? e.i : e.j, z = ax_x ? e.j : e.i;
        res.trm.push_back({e.is_a ? ri : bi, x, z, e.is_a ? to_b : to_a});
    }
    return res;
}

// Portals of other rooms into the split room: to B where they lead into B's part.
void retarget_portals(std::vector<Room> &rooms, int ri, int bi, bool ax_x, int32_t R_x, int32_t R_z, int k,
                      const std::set<int> &skip) {
    const int ax = ax_x ? 0 : 2;
    const int64_t wcut = int64_t(ax_x ? R_x : R_z) + int64_t(k) * 1024;
    for (size_t n = 0; n < rooms.size(); n++) {
        if (skip.count(int(n))) continue;
        Room &r = rooms[n];
        std::vector<Portal> out;
        const int64_t org = ax_x ? r.x : r.z;
        for (auto &pt : r.portals) {
            if (pt[0] != ri) {
                out.push_back(pt);
                continue;
            }
            int64_t lo = INT64_MAX, hi = INT64_MIN;
            for (int i = 0; i < 4; i++) {
                int64_t c = pt[size_t(4 + i * 3 + ax)] + org;
                lo = std::min(lo, c);
                hi = std::max(hi, c);
            }
            if (hi <= wcut) {
                out.push_back(pt);
            } else if (lo >= wcut) {
                Portal p = pt;
                p[0] = bi;
                out.push_back(p);
            } else {
                Portal va = pt, vb = pt;
                va[0] = ri;
                vb[0] = bi;
                for (int i = 0; i < 4; i++) {
                    size_t c = size_t(4 + i * 3 + ax);
                    va[c] = int32_t(std::min(int64_t(pt[c]), wcut - org));
                    vb[c] = int32_t(std::max(int64_t(pt[c]), wcut - org));
                }
                out.push_back(va);
                out.push_back(vb);
            }
        }
        r.portals = std::move(out);
    }
}

// Problems of the level as the game sees it before and after the flipmap (some original levels have a few).
std::set<std::string> check_flip(const ToolContext &ctx, const std::string &exe, const std::string &path) {
    std::set<std::string> problems;
    std::vector<Room> rooms = parse_rooms(read_file(ctx.path(path))).rooms;
    std::map<int, std::vector<std::tuple<int, int, const std::string *>>> fdl;
    FloorData fd = floordata(ctx, exe, path);
    for (auto &[key, c] : fd.items) fdl[key[0]].push_back({key[1], key[2], &c});
    std::set<int64_t> alts;
    for (auto &r : rooms)
        if (r.alt != -1) alts.insert(r.alt);
    for (const char *state : {"normal", "flipped"}) {
        std::map<int64_t, int64_t> slot;
        for (size_t i = 0; i < rooms.size(); i++) slot[int64_t(i)] = int64_t(i);
        if (std::string(state) == "flipped")
            for (size_t i = 0; i < rooms.size(); i++)
                if (rooms[i].alt != -1) {
                    int64_t alt = rooms[i].alt;
                    slot[int64_t(i)] = alt;
                    slot[alt] = int64_t(i);
                }
        std::set<int64_t> active;
        std::vector<int64_t> order;
        for (size_t i = 0; i < rooms.size(); i++)
            if (!alts.count(int64_t(i))) {
                active.insert(int64_t(i));
                order.push_back(int64_t(i));
            }
        auto data = [&](int64_t i) -> const Room & {
            auto it = slot.find(i);
            if (it == slot.end() || it->second < 0) throw Error("room " + std::to_string(i) + " not found");
            return rooms.at(size_t(it->second));
        };
        std::map<std::pair<int64_t, int64_t>, std::vector<PortalShape>> ports;
        for (int64_t i : order) {
            const Room &R = data(i);
            for (auto &p : R.portals) {
                if (!active.count(p[0]))
                    problems.insert(std::format("{}: room {} has a portal to room {}, which is not in use", state, i, p[0]));
                PortalShape v;
                for (int j = 0; j < 4; j++)
                    v[size_t(j)] = {int64_t(p[size_t(4 + j * 3)]) + R.x, p[size_t(5 + j * 3)], int64_t(p[size_t(6 + j * 3)]) + R.z};
                std::sort(v.begin(), v.end());
                ports[{i, p[0]}].push_back(v);
            }
        }
        for (auto &[key, vs] : ports) {
            auto bit = ports.find({key.second, key.first});
            for (auto &v : vs) {
                bool found = false;
                if (bit != ports.end())
                    for (auto &w : bit->second) {
                        bool all = true;
                        for (int q = 0; q < 4 && all; q++)
                            for (int c = 0; c < 3; c++)
                                if (std::abs(v[size_t(q)][size_t(c)] - w[size_t(q)][size_t(c)]) > 1) {
                                    all = false;
                                    break;
                                }
                        if (all) {
                            found = true;
                            break;
                        }
                    }
                if (!found)
                    problems.insert(std::format("{}: portal {} -> {} has no matching portal back", state, key.first, key.second));
            }
        }
        for (int64_t i : order) {
            const Room &R = data(i);
            auto it = fdl.find(int(slot.at(i)));
            if (it == fdl.end()) continue;
            for (auto &[xi, zi, c] : it->second) {
                int64_t wx = int64_t(R.x) + xi * 1024 + 512, wz = int64_t(R.z) + zi * 1024 + 512;
                for (auto m = std::sregex_iterator(c->begin(), c->end(), link_regex()); m != std::sregex_iterator(); ++m) {
                    std::string kind = (*m)[1].str(), t = (*m)[2].str();
                    int64_t tn = parse_int(t);
                    const Room &T_ = data(tn);
                    int64_t fx = floordiv(wx - T_.x, 1024), fz = floordiv(wz - T_.z, 1024);
                    bool ok = active.count(tn) && 1 <= fx && fx <= T_.xs - 2 && 1 <= fz && fz <= T_.zs - 2;
                    if (!ok)
                        problems.insert(std::format("{}: room {} sector {},{}: {}({}) does not lead into a room there",
                                                    state, i, xi, zi, kind, t));
                }
            }
        }
    }
    return problems;
}

void check(const ToolContext &ctx, const std::string &exe, const std::string &src, const std::string &dst,
           const std::vector<Room> &old, const std::vector<Half> &halves, bool ax_x, const FloorData &fd_old) {
    Bytes b = read_file(ctx.path(dst));
    ParsedRooms P = parse_rooms(b);
    const std::vector<Room> &rooms = P.rooms;
    FloorData fd = floordata(ctx, exe, dst);
    for (auto &hf : halves) {
        const Room &R = old.at(size_t(hf.a));
        for (int xi = 0; xi < R.xs; xi++)
            for (int zi = 0; zi < R.zs; zi++) {
                int i = ax_x ? xi : zi;
                if (i == 0 || i == (ax_x ? R.xs : R.zs) - 1) continue;      // outer walls stay unchanged
                for (int hn : {hf.a, hf.bi}) {
                    const Room &h = rooms.at(size_t(hn));
                    int64_t hx = xi - floordiv(int64_t(h.x) - R.x, 1024), hz = zi - floordiv(int64_t(h.z) - R.z, 1024);
                    int64_t hi = ax_x ? hx : hz;
                    int64_t n_h = ax_x ? h.xs : h.zs;
                    if (!(1 <= hi && hi <= n_h - 2)) continue;              // not inside this half
                    int64_t idx = hx * h.zs + hz;
                    if (idx < 0) throw Error("sector index out of range");
                    const auto &s0 = R.sec(xi, zi);
                    const auto &s1 = h.sectors.at(size_t(idx));
                    if (!(s0[1] == s1[1] && s0[3] == s1[3] && s0[5] == s1[5]))
                        throw Error(std::format("sector differs: room {} sector {},{}", hf.a, xi, zi));
                    std::string f0 = world_cameras(after_colon(fd_old.get({hf.a, xi, zi})), old);
                    std::string f1 = world_cameras(after_colon(fd.get({hn, int(hx), int(hz)})), rooms);
                    if (f0 != f1)
                        throw Error(std::format("FloorData differs at room {} sector {},{}: '{}' / '{}'", hf.a, xi, zi, f0, f1));
                }
            }
        size_t na = rooms.at(size_t(hf.a)).faces() + rooms.at(size_t(hf.bi)).faces();
        if (na < R.faces()) throw Error("faces lost");
    }
    struct List {
        size_t base, n, size, rofs, xyz;
    };
    for (const List &l : {List{P.L.items.data(), P.L.items.count, 24, 2, 4}, List{P.L.cameras.data(), P.L.cameras.count, 16, 12, 0}}) {
        for (size_t i = 0; i < l.n; i++) {
            size_t o = l.base + i * l.size;
            int room = s16(b, o + l.rofs);
            int64_t x = s32(b, o + l.xyz), z = s32(b, o + l.xyz + 8);
            for (auto &hf : halves) {
                const Room &R = old.at(size_t(hf.a));
                bool inside_old = R.x <= x && x < R.x + int64_t(R.xs) * 1024 && R.z <= z && z < R.z + int64_t(R.zs) * 1024;
                if ((room == hf.a || room == hf.bi) && inside_old) {
                    const Room &h = rooms.at(size_t(room));
                    if (!(h.x <= x && x < h.x + int64_t(h.xs) * 1024 && h.z <= z && z < h.z + int64_t(h.zs) * 1024))
                        throw Error(std::format("{} {} outside its room {}", l.size == 24 ? "item" : "camera", i, room));
                }
            }
        }
    }
    std::set<std::string> after = check_flip(ctx, exe, dst), before = check_flip(ctx, exe, src);
    std::string added;
    for (auto &p : after)
        if (!before.count(p)) added += "\n  " + p;
    if (!added.empty()) throw Error("the split adds problems:" + added);
}

}  // namespace

// ---- parse/build -------------------------------------------------------------------------------------------------

ParsedRooms parse_rooms(const Bytes &b) {
    ParsedRooms P;
    P.L = layout(b);
    size_t p = P.L.rooms;
    unsigned nr = u16(b, p);
    p += 2;
    for (unsigned n = 0; n < nr; n++) {
        Room r;
        r.x = s32(b, p);
        r.z = s32(b, p + 4);
        r.yb = s32(b, p + 8);
        r.yt = s32(b, p + 12);
        p += 16;
        size_t ndw = u32(b, p);
        p += 4;
        size_t d = p;
        size_t nv = u16(b, d);
        d += 2;
        for (size_t i = 0; i < nv; i++, d += 12)
            r.verts.push_back({s16(b, d), s16(b, d + 2), s16(b, d + 4), s16(b, d + 6), u16(b, d + 8), u16(b, d + 10)});
        size_t nq = u16(b, d);
        d += 2;
        for (size_t i = 0; i < nq; i++, d += 10)
            r.quads.push_back({u16(b, d), u16(b, d + 2), u16(b, d + 4), u16(b, d + 6), u16(b, d + 8)});
        size_t nt = u16(b, d);
        d += 2;
        for (size_t i = 0; i < nt; i++, d += 8) r.tris.push_back({u16(b, d), u16(b, d + 2), u16(b, d + 4), u16(b, d + 6)});
        size_t ns = u16(b, d);
        d += 2;
        for (size_t i = 0; i < ns; i++, d += 4) r.sprites.push_back({s16(b, d), s16(b, d + 2)});
        if (d != p + ndw * 2) throw Error("room data size");
        p += ndw * 2;
        size_t np = u16(b, p);
        p += 2;
        for (size_t i = 0; i < np; i++, p += 32) {
            Portal pt{};
            pt[0] = u16(b, p);
            for (size_t j = 1; j < 16; j++) pt[j] = s16(b, p + 2 * j);
            r.portals.push_back(pt);
        }
        r.zs = u16(b, p);
        r.xs = u16(b, p + 2);
        p += 4;
        for (size_t i = 0; i < size_t(r.zs) * size_t(r.xs); i++, p += 8)
            r.sectors.push_back({u16(b, p), u16(b, p + 2), u8(b, p + 4), int8_t(u8(b, p + 5)), u8(b, p + 6), int8_t(u8(b, p + 7))});
        r.ambient = slice(b, p, 4);
        p += 4;
        size_t n2 = u16(b, p);
        p += 2;
        for (size_t i = 0; i < n2; i++, p += 24) r.lights.push_back(slice(b, p, 24));
        n2 = u16(b, p);
        p += 2;
        for (size_t i = 0; i < n2; i++, p += 20) r.statics.push_back(slice(b, p, 20));
        r.alt = s16(b, p);
        r.flags = s16(b, p + 2);
        r.tail = slice(b, p + 4, 3);
        p += 7;
        P.rooms.push_back(std::move(r));
    }
    P.start = P.L.rooms;
    P.end = p;
    return P;
}

Bytes build_rooms(const std::vector<Room> &rooms) {
    Bytes out;
    Writer w{out};
    w.u16(uint32_t(rooms.size()));
    for (auto &r : rooms) {
        Bytes data;
        Writer dw{data};
        dw.u16(uint32_t(r.verts.size()));
        for (auto &v : r.verts)
            for (int32_t c : v) dw.u16(uint32_t(c));
        dw.u16(uint32_t(r.quads.size()));
        for (auto &f : r.quads)
            for (int32_t c : f) dw.u16(uint32_t(c));
        dw.u16(uint32_t(r.tris.size()));
        for (auto &f : r.tris)
            for (int32_t c : f) dw.u16(uint32_t(c));
        dw.u16(uint32_t(r.sprites.size()));
        for (auto &s : r.sprites)
            for (int32_t c : s) dw.u16(uint32_t(c));
        w.u32(uint32_t(r.x)).u32(uint32_t(r.z)).u32(uint32_t(r.yb)).u32(uint32_t(r.yt)).u32(uint32_t(data.size() / 2));
        w.bytes(data);
        w.u16(uint32_t(r.portals.size()));
        for (auto &pt : r.portals)
            for (int32_t c : pt) w.u16(uint32_t(c));
        w.u16(uint32_t(r.zs)).u16(uint32_t(r.xs));
        for (auto &s : r.sectors) w.u16(uint32_t(s[0])).u16(uint32_t(s[1])).u8(uint32_t(s[2])).u8(uint32_t(s[3])).u8(uint32_t(s[4])).u8(uint32_t(s[5]));
        w.bytes(r.ambient);
        w.u16(uint32_t(r.lights.size()));
        for (auto &l : r.lights) w.bytes(l);
        w.u16(uint32_t(r.statics.size()));
        for (auto &s : r.statics) w.bytes(s);
        w.u16(uint32_t(r.alt)).u16(uint32_t(r.flags));
        w.bytes(r.tail);
    }
    return out;
}

// ---- trmod -------------------------------------------------------------------------------------------------------

std::string trmod(const ToolContext &ctx, const std::string &exe, const std::string &level,
                  const std::vector<std::string> &cmds) {
    std::string out;
    for (size_t i = 0; i < cmds.size(); i += 60) {
        std::vector<std::string> args{"tr3pc", level};
        args.insert(args.end(), cmds.begin() + ptrdiff_t(i), cmds.begin() + ptrdiff_t(std::min(cmds.size(), i + 60)));
        RunResult r = run_program(ctx.path(exe), args, ctx.cwd, {}, true);
        if (r.exit_code != 0 || r.out.find("rror") != std::string::npos)
            throw Error("trmod: " + (r.out.size() > 400 ? r.out.substr(r.out.size() - 400) : r.out));
        out += r.out;
    }
    return out;
}

std::vector<std::string> replacefloordata_commands(const std::string &out) {
    static const std::string head = "\"replacefloordata(";
    std::vector<std::string> cmds;
    size_t i = out.find(head);
    while (i != std::string::npos) {
        size_t q = out.find('"', i + 1);
        // the closing quote must follow a ')' that comes after the opening '('
        if (q != std::string::npos && q >= i + head.size() + 1 && out[q - 1] == ')') {
            cmds.push_back(out.substr(i + 1, q - i - 1));
            i = out.find(head, q + 1);
        } else {
            i = out.find(head, i + 1);
        }
    }
    return cmds;
}

const std::string *FloorData::get(const std::array<int, 3> &key) const {
    for (auto &[k, c] : items)
        if (k == key) return &c;
    return nullptr;
}

FloorData floordata(const ToolContext &ctx, const std::string &exe, const std::string &level) {
    // the regex "(replacefloordata\((\d+,\d+,\d+):[^"]*\))"
    FloorData fd;
    std::map<std::array<int, 3>, size_t> index;
    std::string out = trmod(ctx, exe, level, {"cgetfloordata(*,*,*)"});
    static const std::string head = "\"replacefloordata(";
    size_t i = out.find(head);
    while (i != std::string::npos) {
        size_t p = i + head.size();
        std::array<int, 3> key{};
        bool ok = true;
        for (int n = 0; n < 3 && ok; n++) {
            size_t s = p;
            int64_t v = 0;
            while (p < out.size() && out[p] >= '0' && out[p] <= '9') v = v * 10 + (out[p++] - '0');
            ok = p > s && p < out.size() && out[p] == (n < 2 ? ',' : ':');
            key[size_t(n)] = int(v);
            p++;
        }
        size_t q = ok ? out.find('"', p) : std::string::npos;
        if (ok && q != std::string::npos && q >= p + 1 && out[q - 1] == ')') {
            std::string c = out.substr(i + 1, q - i - 1);
            auto it = index.find(key);
            if (it == index.end()) {
                index[key] = fd.items.size();
                fd.items.push_back({key, c});
            } else {
                fd.items[it->second].second = c;
            }
            i = out.find(head, q + 1);
        } else {
            i = out.find(head, i + 1);
        }
    }
    return fd;
}

// ---- main ----------------------------------------------------------------------------------------------------------

void split_room(ToolContext &ctx, const std::string &exe, const std::string &src, const std::string &dst, int ri,
                const std::string &axis, int k) {
    const bool ax_x = axis == "x";
    Bytes b = read_file(ctx.path(src));
    ParsedRooms P = parse_rooms(b);
    std::vector<Room> &rooms = P.rooms;
    const std::vector<Room> old = rooms;
    FloorData fd_old = floordata(ctx, exe, src);
    std::vector<int> pairs{ri};
    if (rooms.at(size_t(ri)).alt != -1) pairs.push_back(rooms[size_t(ri)].alt);
    for (auto &r : rooms)
        if (r.alt == ri) throw Error("another room has this room as its alternate");

    std::vector<std::array<int, 4>> todo_fd;
    std::vector<Half> halves;
    for (size_t n = 0; n < pairs.size(); n++) {
        int r = pairs[n];
        const Room &R = rooms.at(size_t(r));
        int32_t Rx = R.x, Rz = R.z;
        if (n) {
            const Room &Pr = old[size_t(ri)];
            if (!(R.x == Pr.x && R.z == Pr.z && R.xs == Pr.xs && R.zs == Pr.zs)) throw Error("alternate room has another size");
            if (!(R.alt == -1 || R.alt == ri)) throw Error("alternate room links on");
        }
        std::optional<std::pair<int, int>> link;
        if (n) link = std::pair{halves[0].a, halves[0].bi};
        SplitResult s = split(rooms, r, ax_x, k, link);
        retarget_portals(rooms, r, s.bi, ax_x, Rx, Rz, k, {r, s.bi});
        todo_fd.insert(todo_fd.end(), s.trm.begin(), s.trm.end());
        halves.push_back({r, s.bi, Rx, Rz});
        ctx.print(std::format("room {} -> {} ({} faces) + {} ({} faces), {} before", r, r, rooms[size_t(r)].faces(), s.bi,
                              rooms[size_t(s.bi)].faces(), old.at(size_t(r)).faces()));
    }
    if (halves.size() == 2) {
        rooms[size_t(halves[0].a)].alt = halves[1].a;
        rooms[size_t(halves[0].bi)].alt = halves[1].bi;
    }
    // sector links in other rooms' sectors (room below/above bytes): to B where they lead into B
    for (auto &hf : halves) {
        int64_t wcut = int64_t(ax_x ? hf.Rx : hf.Rz) + int64_t(k) * 1024;
        for (size_t n = 0; n < rooms.size(); n++) {
            if (int(n) == hf.a || int(n) == hf.bi) continue;
            Room &r = rooms[n];
            for (int xi = 0; xi < r.xs; xi++)
                for (int zi = 0; zi < r.zs; zi++) {
                    auto &s = r.sec(xi, zi);
                    int64_t w = ax_x ? int64_t(r.x) + xi * 1024 + 512 : int64_t(r.z) + zi * 1024 + 512;
                    if (w > wcut) {
                        if (s[2] == hf.a) s[2] = hf.bi;
                        if (s[4] == hf.a) s[4] = hf.bi;
                    }
                }
        }
    }
    Bytes nb(b.begin(), b.begin() + ptrdiff_t(P.start));
    Writer{nb}.bytes(build_rooms(rooms));
    nb.insert(nb.end(), b.begin() + ptrdiff_t(P.end), b.end());
    // items and cameras: into the half that contains them
    Layout L2 = layout(nb);
    struct List {
        size_t base, n, size, rofs;
    };
    for (const List &l : {List{L2.items.data(), L2.items.count, 24, 2}, List{L2.cameras.data(), L2.cameras.count, 16, 12}}) {
        for (size_t i = 0; i < l.n; i++) {
            size_t o = l.base + i * l.size;
            int room = s16(nb, o + l.rofs);
            size_t xyz = l.size == 24 ? o + 4 : o;
            int64_t x = s32(nb, xyz), z = s32(nb, xyz + 8);
            for (auto &hf : halves) {
                int64_t w = ax_x ? x : z;
                if (room == hf.a && w >= int64_t(ax_x ? hf.Rx : hf.Rz) + int64_t(k) * 1024) put16(nb, o + l.rofs, uint32_t(hf.bi));
            }
        }
    }
    std::string tmp = dst + ".tmp";
    write_file(ctx.path(tmp), nb);

    // FloorData links in other rooms into B's part, and the new portal sectors between A and B
    FloorData fd = floordata(ctx, exe, tmp);
    std::vector<std::string> cmds;
    for (auto &hf : halves) {
        int64_t wcut = int64_t(ax_x ? hf.Rx : hf.Rz) + int64_t(k) * 1024;
        std::regex pat("\\b(portal|floorportal|ceilingportal)\\(" + std::to_string(hf.a) + "\\)");
        for (auto &[key, c] : fd.items) {
            if (key[0] == hf.a || key[0] == hf.bi || !std::regex_search(c, pat)) continue;
            const Room &r = rooms.at(size_t(key[0]));
            int64_t w = ax_x ? int64_t(r.x) + key[1] * 1024 + 512 : int64_t(r.z) + key[2] * 1024 + 512;
            if (w > wcut) cmds.push_back(std::regex_replace(c, pat, "$1(" + std::to_string(hf.bi) + ")"));
        }
    }
    for (auto &[r, x, z, t] : todo_fd) cmds.push_back(std::format("replacefloordata({},{},{}:portal({}))", r, x, z, t));
    if (!cmds.empty()) trmod(ctx, exe, tmp, cmds);
    check(ctx, exe, src, tmp, old, halves, ax_x, fd_old);
    fs::rename(ctx.path(tmp), ctx.path(dst));
    ctx.print(std::format("written {}, check ok", dst));
}

namespace {

void split_room_tool(ToolContext &ctx, const std::vector<std::string> &args) {
    if (args.size() != 6) throw Error("usage: split_room.py <trmod> in.tr2 out.tr2 <room> <x|z> <k>");
    split_room(ctx, args[0], args[1], args[2], int(parse_int(args[3])), args[4], int(parse_int(args[5])));
}

}  // namespace

REGISTER_TOOL("split_room.py", split_room_tool);

}  // namespace geom
