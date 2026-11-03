// pack_rotations.py: single axis rotations in one word, every animation with its own frame size.
#include <algorithm>
#include <array>
#include <map>
#include <set>

#include "../tool.h"
#include "anim_tools.h"

namespace anim {
namespace {

constexpr uint16_t NO_ANIM = 0xFFFF;
constexpr uint16_t AXIS[3] = {0x4000, 0x8000, 0xC000};

bool uniform(uint32_t id) {
    return (id >= 1 && id <= 10) || id == 145 || id == 146 || id == 147 || id == 158 || id == 159 ||
           (id >= 181 && id <= 204) || (id >= 209 && id <= 212) || (id >= 228 && id <= 231) || id == 238 ||
           id == 239 || (id >= 244 && id <= 247);
}

// A decoded pose: the nine header words and the (x, y, z) angles of every mesh.
struct Pose {
    std::array<uint16_t, 9> head;
    std::vector<std::array<unsigned, 3>> rot;
    bool operator==(const Pose &) const = default;
};

Pose decode(const Bytes &d, size_t o, unsigned meshes, size_t *length = nullptr) {
    Pose p;
    for (size_t i = 0; i < 9; i++) p.head[i] = u16(d, o + 2 * i);
    size_t q = o + 18;
    for (unsigned m = 0; m < meshes; m++) {
        unsigned a = u16(d, q);
        if (a & 0xC000) {
            std::array<unsigned, 3> ang = {0, 0, 0};
            ang[(a >> 14) - 1] = a & 0x3FF;
            p.rot.push_back(ang);
            q += 2;
        } else {
            unsigned b = u16(d, q + 2);
            p.rot.push_back({(a >> 4) & 0x3FF, (a & 15) << 6 | b >> 10, b & 0x3FF});
            q += 4;
        }
    }
    if (length) *length = q - o;
    return p;
}

Bytes encode(const Pose &p) {
    Bytes out;
    Writer w{out};
    for (uint16_t h : p.head) w.u16(h);
    for (auto &r : p.rot) {
        int nonzero = 0, k = 0;
        for (int i = 2; i >= 0; i--)
            if (r[i]) nonzero++, k = i;
        if (nonzero <= 1) {
            w.u16(AXIS[k] | r[k]);
        } else {
            append_u16(out, (long long)(r[0] << 4 | r[1] >> 6));
            append_u16(out, (long long)((r[1] & 63) << 10 | r[2]));
        }
    }
    return out;
}

struct Layout {
    FrameLayout f;
    std::vector<size_t> keys;
    std::vector<bool> behind;
    std::map<size_t, std::set<size_t>> obj;      // animation -> moveables it belongs to
    std::vector<unsigned> meshes;                // per animation
};

Layout layout(const Bytes &d, const Sections &s, size_t stride) {
    Layout l;
    l.f = read_frames(d, s, stride);
    const auto &anims = l.f.anims;
    const auto &movs = l.f.movs;
    for (auto &a : anims) {
        unsigned span = a.frame_end - a.frame_base;
        l.keys.push_back(span / a.rate + 1);
        l.behind.push_back(span % a.rate != 0);
    }
    std::set<size_t> starts = {anims.size()};
    for (auto &m : movs)
        if (m.anim != NO_ANIM) starts.insert(m.anim);
    for (size_t k = 0; k < movs.size(); k++) {
        if (movs[k].anim == NO_ANIM) continue;
        auto it = starts.upper_bound(movs[k].anim);
        require(it != starts.end(), "moveable " + std::to_string(k) + ": first animation beyond the list");
        for (size_t i = movs[k].anim; i < *it; i++) l.obj[i].insert(k);
    }
    for (size_t i = 0; i < anims.size(); i++) {
        std::set<unsigned> n;
        auto it = l.obj.find(i);
        if (it != l.obj.end())
            for (size_t k : it->second) n.insert(movs[k].meshes);
        require(n.size() == 1, "animation " + std::to_string(i) + ": moveables with different mesh counts");
        l.meshes.push_back(*n.begin());
    }
    return l;
}

struct Slot {
    size_t len;
    std::set<size_t> own, behind;      // nodes: animation i is i, moveable k is anims + k
    std::set<unsigned> meshes;
};

struct Packed {
    std::vector<std::pair<size_t, size_t>> anims;    // frame offset, frame size in words
    std::vector<size_t> movs;
    Bytes frames;
};

Packed pack(const Bytes &d, const Sections &s, size_t stride) {
    Layout l = layout(d, s, stride);
    const auto &anims = l.f.anims;
    const auto &movs = l.f.movs;
    size_t na = anims.size();
    std::vector<size_t> parent(na + movs.size());
    for (size_t i = 0; i < parent.size(); i++) parent[i] = i;
    auto find = [&](size_t x) {
        while (parent[x] != x) {
            parent[x] = parent[parent[x]];
            x = parent[x];
        }
        return x;
    };
    auto unite = [&](size_t a, size_t b) { parent[find(a)] = find(b); };

    std::map<size_t, Slot> slots;
    auto slot = [&](size_t o, size_t len) -> Slot & {
        auto [it, fresh] = slots.try_emplace(o, Slot{len, {}, {}, {}});
        it->second.len = std::max(it->second.len, len);
        return it->second;
    };
    for (size_t i = 0; i < na; i++) {
        size_t w = 2 * size_t(anims[i].frame_size);
        for (size_t k = 0; k < l.keys[i] + l.behind[i]; k++) {
            Slot &sl = slot(anims[i].frame_offset + k * w, w);
            (k < l.keys[i] ? sl.own : sl.behind).insert(i);
            sl.meshes.insert(l.meshes[i]);
        }
    }
    for (size_t k = 0; k < movs.size(); k++) {
        const Moveable &m = movs[k];
        if (m.anim == NO_ANIM) {
            size_t n = 0;
            decode(d, l.f.frames + m.frame_offset, m.meshes, &n);
            Slot &sl = slot(m.frame_offset, n);
            sl.own.insert(na + k);
            sl.meshes.insert(m.meshes);
        } else {
            require(m.frame_offset == anims.at(m.anim).frame_offset,
                    "moveable " + std::to_string(m.id) + ": frame offset is not its first animation's");
        }
    }
    for (auto &[o, sl] : slots) {
        require(sl.meshes.size() == 1 || !sl.own.empty(),
                "pose at " + std::to_string(o) + " read with different mesh counts");
        if (sl.own.empty()) sl.own = sl.behind;
        for (size_t x : sl.own) unite(*sl.own.begin(), x);
    }
    for (size_t k = 0; k < movs.size(); k++) {
        if (!uniform(movs[k].id)) continue;
        std::vector<size_t> mine;
        for (auto &[i, ks] : l.obj)
            if (ks.count(k)) mine.push_back(i);
        for (size_t i : mine) unite(i, mine.front());
    }
    size_t prev_end = 0;
    bool any = false;
    for (auto &[o, sl] : slots) {
        require(!any || prev_end <= o, "poses overlap at " + std::to_string(o));
        prev_end = o + sl.len;
        any = true;
    }
    require(!any || prev_end <= l.f.size, "the last pose runs past the frames");
    std::map<size_t, Bytes> poses;
    std::map<size_t, size_t> words;          // group -> frame size in words
    for (auto &[o, sl] : slots) {
        unsigned n;
        if (sl.meshes.size() == 1) {
            n = *sl.meshes.begin();
        } else {
            require(*sl.own.begin() < na, "pose at " + std::to_string(o) + " has no animation");
            n = l.meshes[*sl.own.begin()];
        }
        Bytes &p = poses[o] = encode(decode(d, l.f.frames + o, n));
        size_t &wd = words[find(*sl.own.begin())];
        wd = std::max(wd, p.size() / 2);
    }
    Packed r;
    std::map<size_t, size_t> pos;
    size_t at = 0;
    for (auto &[o, sl] : slots) {
        if (at < o) append_slice(r.frames, d, l.f.frames + at, l.f.frames + o);
        pos[o] = r.frames.size();
        const Bytes &p = poses[o];
        size_t w = 2 * words[find(*sl.own.begin())];
        r.frames.insert(r.frames.end(), p.begin(), p.end());
        r.frames.insert(r.frames.end(), w - p.size(), 0);
        at = o + sl.len;
    }
    if (l.f.frames + at < l.f.frames + l.f.size) append_slice(r.frames, d, l.f.frames + at, l.f.frames + l.f.size);
    for (size_t i = 0; i < na; i++) {
        size_t w = words.at(find(i));
        require(w <= 255, "animation " + std::to_string(i) + ": frame size above 255");
        size_t base = pos.at(anims[i].frame_offset);
        for (size_t k = 0; k < l.keys[i] + l.behind[i]; k++)
            require(pos.at(anims[i].frame_offset + k * 2 * size_t(anims[i].frame_size)) == base + k * 2 * w,
                    "poses of animation " + std::to_string(i) + " not in a row");
        r.anims.emplace_back(base, w);
    }
    for (auto &m : movs) r.movs.push_back(pos.at(m.frame_offset));
    for (size_t k = 0; k < movs.size(); k++) {
        if (!uniform(movs[k].id) || movs[k].anim == NO_ANIM) continue;
        std::set<size_t> sizes;
        for (auto &[i, ks] : l.obj)
            if (ks.count(k)) sizes.insert(r.anims[i].second);
        require(sizes.size() == 1, "object " + std::to_string(movs[k].id) + ": frame sizes differ");
    }
    return r;
}

// Every pose decodes the same, and nothing changed but the frames, the frame offsets and the frame sizes.
void check(const Bytes &d, const Sections &s, const Bytes &out, size_t stride) {
    long long df = 2LL * count(d, s.frames) - 2LL * count(out, s.frames);
    require((long long)d.size() - (long long)out.size() == df && df >= 0, "sizes do not add up");
    Sections s2 = s;
    for (size_t *v : {&s2.floor, &s2.mesh, &s2.meshptr, &s2.anims, &s2.states, &s2.dispatch, &s2.commands, &s2.trees,
                      &s2.frames, &s2.moveables, &s2.statics, &s2.spritetex, &s2.spriteseq, &s2.cameras, &s2.sources,
                      &s2.items})
        if (*v > s.frames) *v = size_t((long long)*v - df);
    Layout l1 = layout(d, s, stride), l2 = layout(out, s2, stride);
    require(l1.keys == l2.keys && l1.behind == l2.behind && l1.meshes == l2.meshes, "pose counts changed");
    for (size_t i = 0; i < l1.f.anims.size(); i++) {
        const AnimHead &x = l1.f.anims[i], &y = l2.f.anims[i];
        for (size_t k = 0; k < l1.keys[i] + l1.behind[i]; k++) {
            Pose p = decode(d, l1.f.frames + x.frame_offset + k * 2 * size_t(x.frame_size), l1.meshes[i]);
            Pose q = decode(out, l2.f.frames + y.frame_offset + k * 2 * size_t(y.frame_size), l1.meshes[i]);
            require(p == q, "animation " + std::to_string(i) + ", pose " + std::to_string(k) + " changed");
        }
    }
    for (size_t k = 0; k < l1.f.movs.size(); k++) {
        const Moveable &x = l1.f.movs[k], &y = l2.f.movs[k];
        require(decode(d, l1.f.frames + x.frame_offset, x.meshes) == decode(out, l2.f.frames + y.frame_offset, x.meshes),
                "moveable " + std::to_string(k) + ": first pose changed");
    }
    Bytes back = out;
    for (size_t i = 0; i < l1.f.anims.size(); i++) {
        put32(back, anim_at(s2, i), l1.f.anims[i].frame_offset);
        put8(back, anim_at(s2, i) + 5, l1.f.anims[i].frame_size);
    }
    for (size_t k = 0; k < l1.f.movs.size(); k++)
        put32(back, s2.moveables + 4 + stride * k + 12, l1.f.movs[k].frame_offset);
    require(std::equal(back.begin(), back.begin() + std::min(s.frames, back.size()), d.begin(),
                       d.begin() + std::min(s.frames, d.size())),
            "something before the frames changed");
    Bytes x, y;
    append_slice(x, back, s2.moveables, back.size());
    append_slice(y, d, s.moveables, d.size());
    require(x == y, "something behind the frames changed");
}

}  // namespace

std::optional<Bytes> pack_rotations(const Bytes &d, const Sections &s, size_t stride) {
    Layout l = layout(d, s, stride);
    Packed p = pack(d, s, stride);
    Bytes w = d;
    for (size_t i = 0; i < p.anims.size(); i++) {
        pack_u32(w, anim_at(s, i), (long long)p.anims[i].first);
        put8(w, anim_at(s, i) + 5, uint32_t(p.anims[i].second));
    }
    for (size_t k = 0; k < p.movs.size(); k++) pack_u32(w, s.moveables + 4 + stride * k + 12, (long long)p.movs[k]);
    Bytes out;
    out.reserve(w.size());
    append_slice(out, w, 0, s.frames);
    append_u32(out, p.frames.size() / 2);
    out.insert(out.end(), p.frames.begin(), p.frames.end());
    append_slice(out, w, l.f.frames + l.f.size, w.size());
    if (out == d) return std::nullopt;
    check(d, s, out, stride);
    return out;
}

}  // namespace anim

namespace {
void run_pack_rotations(ToolContext &ctx, const std::vector<std::string> &args) {
    anim::require(!args.empty(), "usage: pack_rotations.py <level.tr2>");
    Bytes d = read_file(ctx.path(args[0]));
    auto out = anim::pack_rotations(d, anim::pc_sections(d));
    if (!out) {
        ctx.print(args[0] + ": rotations packed already");
    } else {
        write_file(ctx.path(args[0]), *out);
        ctx.print(args[0] + ": " + std::to_string((long long)d.size() - (long long)out->size()) + " frame bytes removed");
    }
}
}  // namespace

REGISTER_TOOL("pack_rotations.py", run_pack_rotations);
