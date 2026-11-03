// compact_frames.py: shares equal pose blocks and mesh trees and removes the frame and tree bytes nothing uses.
#include <algorithm>
#include <cstring>
#include <map>
#include <set>

#include "../tool.h"
#include "anim_tools.h"

namespace anim {

FrameLayout read_frames(const Bytes &d, const Sections &s, size_t stride) {
    FrameLayout f;
    uint32_t na = count(d, s.anims);
    f.anims.reserve(na);
    for (uint32_t i = 0; i < na; i++) f.anims.push_back(read_anim_head(d, s, i));
    f.movs = read_moveables(d, s, stride);
    f.frames = s.frames + 4;
    f.size = 2 * size_t(count(d, s.frames));
    for (size_t i = 0; i < f.anims.size(); i++) {
        const AnimHead &a = f.anims[i];
        require(a.rate >= 1 && a.frame_end >= a.frame_base,
                "(" + std::to_string(i) + ", " + std::to_string(a.rate) + ", " + std::to_string(a.frame_base) + ", " +
                    std::to_string(a.frame_end) + ")");
        unsigned span = a.frame_end - a.frame_base;
        size_t n = span / a.rate + 1 + (span % a.rate != 0);
        size_t len = 2 * size_t(a.frame_size) * n;
        require(a.frame_offset % 2 == 0 && a.frame_offset + len <= f.size,
                "(" + std::to_string(i) + ", " + std::to_string(a.frame_offset) + ", " + std::to_string(a.frame_size) +
                    ", " + std::to_string(n) + ", " + std::to_string(f.size) + ")");
        f.spans.emplace_back(a.frame_offset, len);
    }
    return f;
}

namespace {

constexpr uint16_t NO_ANIM = 0xFFFF;

// Size of one pose at o: bounding box, offset, then one or two words per mesh.
size_t pose_bytes(const Bytes &d, size_t o, unsigned meshes) {
    size_t p = o + 18;
    for (unsigned i = 0; i < meshes; i++) p += (u16(d, p) & 0xC000) ? 2 : 4;
    return p - o;
}

// d[o:o + n] with Python's clipping.
Bytes py_slice(const Bytes &d, size_t o, size_t n) {
    Bytes out;
    append_slice(out, d, o, o + n);
    return out;
}

struct Block {
    size_t off, len;
    std::vector<size_t> anims, movs;     // animations in it, moveables whose frame offset is in it
};

std::vector<Block> blocks(const Bytes &d, const FrameLayout &f) {
    std::set<size_t> start_set = {0, f.anims.size()};
    for (auto &m : f.movs)
        if (m.anim != NO_ANIM) start_set.insert(m.anim);
    std::vector<size_t> starts(start_set.begin(), start_set.end());
    std::vector<Block> out;
    for (size_t g = 0; g + 1 < starts.size(); g++) {
        std::vector<size_t> group;
        for (size_t i = starts[g]; i < starts[g + 1]; i++) group.push_back(i);
        std::stable_sort(group.begin(), group.end(),
                         [&](size_t a, size_t b) { return f.spans.at(a).first < f.spans.at(b).first; });
        size_t cur = SIZE_MAX;
        for (size_t i : group) {
            auto [off, n] = f.spans.at(i);
            if (cur != SIZE_MAX && off <= out[cur].off + out[cur].len) {
                out[cur].len = std::max(out[cur].len, off + n - out[cur].off);
                out[cur].anims.push_back(i);
            } else {
                out.push_back({off, n, {i}, {}});
                cur = out.size() - 1;
            }
        }
    }
    for (size_t k = 0; k < f.movs.size(); k++) {
        const Moveable &m = f.movs[k];
        if (m.anim == NO_ANIM) {
            require(m.frame_offset % 2 == 0 && m.frame_offset < f.size, "moveable " + std::to_string(k));
            out.push_back({m.frame_offset, pose_bytes(d, f.frames + m.frame_offset, m.meshes), {}, {k}});
        } else {
            size_t first = f.spans.at(m.anim).first;
            require(m.frame_offset == first, "moveable " + std::to_string(m.id) + ": frame offset " +
                                                 std::to_string(m.frame_offset) + " is not its first animation's " +
                                                 std::to_string(first));
            Block *blk = nullptr;
            size_t hits = 0;
            for (auto &b : out)
                if (std::find(b.anims.begin(), b.anims.end(), m.anim) != b.anims.end()) blk = &b, hits++;
            require(hits == 1, "moveable " + std::to_string(k) + ": not in one block");
            blk->movs.push_back(k);
        }
    }
    return out;
}

// Marks used[o:o + n] (clipped to the list).
void mark(std::vector<uint8_t> &used, size_t o, size_t n) {
    for (size_t i = o; i < std::min(o + n, used.size()); i++) used[i] = 1;
}

std::vector<size_t> unused_before(const std::vector<uint8_t> &used) {
    std::vector<size_t> before(used.size() + 1, 0);
    for (size_t o = 0; o < used.size(); o++) before[o + 1] = before[o] + (used[o] ? 0 : 1);
    return before;
}

struct FrameResult {
    std::vector<size_t> anim_off, mov_off;
    Bytes frames;
};

FrameResult compact_frame_list(const Bytes &d, const FrameLayout &f, CompactReport &report) {
    auto blks = blocks(d, f);
    std::stable_sort(blks.begin(), blks.end(), [](const Block &a, const Block &b) { return a.off < b.off; });
    std::map<Bytes, size_t> first;
    std::map<size_t, size_t> moved_anim, moved_mov;
    size_t shared = 0;
    for (auto &b : blks) {
        size_t to = b.len ? first.emplace(py_slice(d, f.frames + b.off, b.len), b.off).first->second : b.off;
        if (to != b.off) {
            shared++;
            for (size_t i : b.anims) moved_anim[i] = f.spans[i].first - b.off + to;
            for (size_t k : b.movs) moved_mov[k] = to;
        }
    }
    FrameResult r;
    for (size_t i = 0; i < f.anims.size(); i++) {
        auto it = moved_anim.find(i);
        r.anim_off.push_back(it != moved_anim.end() ? it->second : f.spans[i].first);
    }
    for (size_t k = 0; k < f.movs.size(); k++) {
        auto it = moved_mov.find(k);
        r.mov_off.push_back(it != moved_mov.end() ? it->second : f.movs[k].frame_offset);
    }
    std::vector<uint8_t> used(f.size, 0);
    for (size_t i = 0; i < f.spans.size(); i++) mark(used, r.anim_off[i], f.spans[i].second);
    for (size_t k = 0; k < f.movs.size(); k++) {
        const Moveable &m = f.movs[k];
        size_t n = m.anim == NO_ANIM ? pose_bytes(d, f.frames + m.frame_offset, m.meshes) : f.spans.at(m.anim).second;
        mark(used, r.mov_off[k], n);
    }
    auto before = unused_before(used);
    bool even = before[f.size] % 2 == 0;
    for (auto *offs : {&r.anim_off, &r.mov_off})
        for (auto &o : *offs) {
            o -= before.at(o);
            even = even && o % 2 == 0;
        }
    require(even, "frame offsets not even after compacting");
    report.frames = before[f.size];
    report.shared = shared;
    r.frames.reserve(f.size - before[f.size]);
    for (size_t o = 0; o < f.size; o++)
        if (used[o]) r.frames.push_back(d[f.frames + o]);
    return r;
}

struct TreeResult {
    std::vector<size_t> tree;
    Bytes trees;
};

TreeResult compact_trees(const Bytes &d, const Sections &s, const std::vector<Moveable> &movs) {
    size_t trees = s.trees + 4, cnt = count(d, s.trees);
    std::map<Bytes, size_t> first;
    std::vector<size_t> tree;
    for (size_t k = 0; k < movs.size(); k++) {
        const Moveable &m = movs[k];
        size_t n = 4 * size_t(std::max(int(m.meshes) - 1, 0));
        require(m.tree + n <= cnt, "moveable " + std::to_string(k) + ": tree beyond the list");
        tree.push_back(n ? first.emplace(py_slice(d, trees + 4 * size_t(m.tree), 4 * n), m.tree).first->second
                         : m.tree);
    }
    std::vector<uint8_t> used(cnt, 0);
    for (size_t k = 0; k < movs.size(); k++) {
        size_t n = 4 * size_t(std::max(int(movs[k].meshes) - 1, 0));
        for (size_t wd = tree[k]; wd < tree[k] + n; wd++) used.at(wd) = 1;
    }
    auto before = unused_before(used);
    TreeResult r;
    for (size_t wd = 0; wd < cnt; wd++)
        if (used[wd]) r.trees.insert(r.trees.end(), d.begin() + trees + 4 * wd, d.begin() + trees + 4 * wd + 4);
    for (size_t t : tree) r.tree.push_back(t - before[std::min(t, cnt)]);
    return r;
}

Sections moved_sections(const Sections &s, long long dt, long long df) {
    Sections s2 = s;
    for (size_t *v : {&s2.floor, &s2.mesh, &s2.meshptr, &s2.anims, &s2.states, &s2.dispatch, &s2.commands, &s2.trees,
                      &s2.frames, &s2.moveables, &s2.statics, &s2.spritetex, &s2.spriteseq, &s2.cameras, &s2.sources,
                      &s2.items}) {
        long long x = (long long)*v;
        x -= (*v >= s.frames ? dt : 0) + (*v > s.frames ? df : 0);
        *v = size_t(x);
    }
    return s2;
}

// Every pose and every tree in use is the same as before, and nothing else changed but the offsets.
void check(const Bytes &d, const Sections &s, const Bytes &out, size_t stride) {
    long long dt = 4 * ((long long)count(d, s.trees) - count(out, s.trees));
    long long df = 2LL * count(d, s.frames) - 2LL * count(out, size_t((long long)s.frames - dt));
    require((long long)d.size() - (long long)out.size() == dt + df && dt + df >= 0, "sizes do not add up");
    Sections s2 = moved_sections(s, dt, df);
    FrameLayout f1 = read_frames(d, s, stride), f2 = read_frames(out, s2, stride);
    require(f1.anims.size() == f2.anims.size() && f1.movs.size() == f2.movs.size(), "list lengths changed");
    for (size_t i = 0; i < f1.spans.size(); i++) {
        auto [o1, n1] = f1.spans[i];
        auto [o2, n2] = f2.spans[i];
        require(n1 == n2 && py_slice(d, f1.frames + o1, n1) == py_slice(out, f2.frames + o2, n2),
                "poses of animation " + std::to_string(i) + " changed");
    }
    size_t t1 = s.trees + 4, t2 = s2.trees + 4;
    for (size_t k = 0; k < f1.movs.size(); k++) {
        const Moveable &x = f1.movs[k], &y = f2.movs[k];
        size_t n = 16 * size_t(std::max(int(x.meshes) - 1, 0));
        require(py_slice(d, t1 + 4 * size_t(x.tree), n) == py_slice(out, t2 + 4 * size_t(y.tree), n),
                "tree of moveable " + std::to_string(k) + " changed");
        n = x.anim == NO_ANIM ? pose_bytes(d, f1.frames + x.frame_offset, x.meshes) : f1.spans.at(x.anim).second;
        require(py_slice(d, f1.frames + x.frame_offset, n) == py_slice(out, f2.frames + y.frame_offset, n),
                "poses of moveable " + std::to_string(k) + " changed");
    }
    for (auto &b : blocks(d, f1)) {
        for (size_t j = 1; j < b.anims.size(); j++) {
            size_t i = b.anims[j], i0 = b.anims[0];
            require((long long)f2.spans[i].first - (long long)f2.spans[i0].first ==
                        (long long)f1.spans[i].first - (long long)f1.spans[i0].first,
                    "poses of animation " + std::to_string(i) + " moved apart");
        }
        for (size_t k : b.movs) {
            if (b.anims.empty()) continue;
            size_t lo = *std::min_element(b.anims.begin(), b.anims.end(), [&](size_t a, size_t c) {
                return f1.spans[a].first < f1.spans[c].first;
            });
            require(f2.movs[k].frame_offset == f2.spans[lo].first, "moveable " + std::to_string(k) + " left its poses");
        }
    }
    Bytes back = out;
    for (size_t i = 0; i < f1.anims.size(); i++) put32(back, anim_at(s2, i), f1.anims[i].frame_offset);
    for (size_t k = 0; k < f1.movs.size(); k++) {
        size_t o = s2.moveables + 4 + stride * k + 8;
        put32(back, o, f1.movs[k].tree);
        put32(back, o + 4, f1.movs[k].frame_offset);
    }
    auto same = [](const Bytes &a, size_t ao, size_t ae, const Bytes &b, size_t bo, size_t be) {
        Bytes x, y;
        append_slice(x, a, ao, ae);
        append_slice(y, b, bo, be);
        return x == y;
    };
    require(same(back, 0, s.trees, d, 0, s.trees), "something before the mesh trees changed");
    size_t tree_end = t1 + 4 * size_t(count(d, s.trees));
    require(same(back, s2.frames - (s.frames - tree_end), s2.frames, d, tree_end, s.frames),
            "something between the trees and the frames changed");
    require(same(back, s2.moveables, back.size(), d, s.moveables, d.size()), "something behind the frames changed");
}

}  // namespace

std::optional<Bytes> compact_frames(const Bytes &d, const Sections &s, size_t stride, CompactReport *report) {
    CompactReport local;
    CompactReport &rep = report ? *report : local;
    FrameLayout f = read_frames(d, s, stride);
    FrameResult fr = compact_frame_list(d, f, rep);
    TreeResult tr = compact_trees(d, s, f.movs);
    size_t t_end = s.trees + 4 + 4 * size_t(count(d, s.trees));
    rep.trees = t_end - s.trees - 4 - tr.trees.size();
    Bytes w = d;
    for (size_t i = 0; i < fr.anim_off.size(); i++) pack_u32(w, anim_at(s, i), (long long)fr.anim_off[i]);
    for (size_t k = 0; k < f.movs.size(); k++) {
        size_t o = s.moveables + 4 + stride * k + 8;
        pack_u32(w, o, (long long)tr.tree[k]);
        pack_u32(w, o + 4, (long long)fr.mov_off[k]);
    }
    Bytes out;
    out.reserve(w.size());
    append_slice(out, w, 0, s.trees);
    append_u32(out, tr.trees.size() / 4);
    out.insert(out.end(), tr.trees.begin(), tr.trees.end());
    append_slice(out, w, t_end, s.frames);
    append_u32(out, fr.frames.size() / 2);
    out.insert(out.end(), fr.frames.begin(), fr.frames.end());
    append_slice(out, w, f.frames + f.size, w.size());
    if (out == d) return std::nullopt;
    check(d, s, out, stride);
    return out;
}

}  // namespace anim

namespace {
void run_compact_frames(ToolContext &ctx, const std::vector<std::string> &args) {
    anim::require(!args.empty(), "usage: compact_frames.py <level.tr2>");
    Bytes d = read_file(ctx.path(args[0]));
    anim::CompactReport report;
    auto out = anim::compact_frames(d, anim::pc_sections(d), anim::PC_STRIDE, &report);
    if (!out) {
        ctx.print(args[0] + ": frames and mesh trees compact already");
    } else {
        write_file(ctx.path(args[0]), *out);
        ctx.print(args[0] + ": " + std::to_string(report.frames) + " frame bytes removed (" +
                  std::to_string(report.shared) + " pose blocks shared), " + std::to_string(report.trees) +
                  " mesh tree bytes removed");
    }
}
}  // namespace

REGISTER_TOOL("compact_frames.py", run_compact_frames);
