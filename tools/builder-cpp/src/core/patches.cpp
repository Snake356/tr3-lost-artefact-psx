#include "patches.h"

#include <algorithm>
#include <map>
#include <set>

#include "xz.h"

Bytes apply_ips(const Bytes &data, const Bytes &ips) {
    if (ips.size() < 8 || std::string(ips.begin(), ips.begin() + 5) != "PATCH") throw Error("not an IPS patch");
    Bytes d = data;
    size_t p = 5;
    auto be = [&](size_t o, int n) {
        uint32_t v = 0;
        for (int i = 0; i < n; i++) v = v << 8 | u8(ips, o + size_t(i));
        return v;
    };
    while (!(p + 3 <= ips.size() && ips[p] == 'E' && ips[p + 1] == 'O' && ips[p + 2] == 'F')) {
        size_t off = be(p, 3), size = be(p + 3, 2);
        p += 5;
        Bytes chunk;
        if (size) {
            chunk = slice(ips, p, size);
            p += size;
        } else {
            chunk.assign(be(p, 2), u8(ips, p + 2));
            p += 3;
        }
        off = std::min(off, d.size());
        if (off + chunk.size() > d.size()) d.resize(off + chunk.size());
        std::copy(chunk.begin(), chunk.end(), d.begin() + long(off));
    }
    return d;
}

Bytes apply_bindiff(const Bytes &src, const Bytes &patch) {
    Bytes d = xz_decompress(patch);
    if (d.size() < 16 || std::string(d.begin(), d.begin() + 4) != "LAD1") throw Error("not a LAD1 patch");
    if (crc32(src) != u32(d, 4)) throw Error("the patch belongs to another source file");
    uint32_t size = u32(d, 12);
    Bytes out;
    out.reserve(size);
    for (size_t p = 16; p < d.size();) {
        if (d[p] == 1) {
            size_t o = u32(d, p + 1), n = u32(d, p + 5);
            o = std::min(o, src.size());
            n = std::min(n, src.size() - o);
            out.insert(out.end(), src.begin() + long(o), src.begin() + long(o + n));
            p += 9;
        } else {
            size_t n = u32(d, p + 1);
            check_range(d, p + 5, n);
            out.insert(out.end(), d.begin() + long(p + 5), d.begin() + long(p + 5 + n));
            p += 5 + n;
        }
    }
    if (out.size() != size || crc32(out) != u32(d, 8)) throw Error("patch result is wrong");
    return out;
}

Bytes apply_texture_patch(const Bytes &bmp, const Bytes &patch) {
    Bytes d = xz_decompress(patch);
    if (crc32(bmp) != u32(d, 0)) throw Error("texture pages differ from the expected level");
    size_t off = u32(bmp, 10);
    int32_t w = s32(bmp, 18), h = s32(bmp, 22);
    if (u16(bmp, 28) != 16) throw Error("texture pages are not 16-bit");
    size_t n = size_t(w) * size_t(h < 0 ? -h : h);
    Bytes out = bmp;
    check_range(d, 8, 4 * n);
    for (size_t i = 0; i < n; i++) {
        uint32_t x = u16(bmp, off + 2 * i);
        const uint8_t *q = &d[8 + 4 * i];
        uint32_t v = ((x >> 15) ^ q[3]) << 15 | (((x >> 10) + q[0]) & 31) << 10 | (((x >> 5) + q[1]) & 31) << 5 |
                     ((x + q[2]) & 31);
        put16(out, off + 2 * i, v);
    }
    if (crc32(out) != u32(d, 4)) throw Error("texture result is wrong");
    return out;
}

namespace {
const std::pair<const char *, size_t> ANIM_SECTIONS[] = {{"anims", 32}, {"statechanges", 6}, {"dispatches", 8},
                                                         {"commands", 2}, {"meshtrees", 4}, {"frames", 2},
                                                         {"moveables", 18}, {"statics", 32}};
}

Bytes rebuild_anim_block(const Bytes &block) {
    std::map<std::string, Bytes> s;
    size_t p = 0;
    for (auto &[name, size] : ANIM_SECTIONS) {
        size_t n = u32(block, p);
        s[name] = slice(block, p + 4, n * size);
        p += 4 + n * size;
    }
    if (p != block.size()) throw Error("unexpected data behind the static meshes");

    const Bytes &fr = s["frames"];
    size_t nframes = fr.size() / 2;
    auto frame = [&](size_t i) { return u16(fr, 2 * i); };
    Bytes &an = s["anims"], &mv = s["moveables"];
    size_t nanims = an.size() / 32, nmovs = mv.size() / 18;

    std::map<size_t, uint32_t> meshes;               // animation -> number of meshes of its moveable
    std::set<std::pair<uint32_t, uint32_t>> starts;  // (first animation, meshes)
    for (size_t i = 0; i < nmovs; i++) {
        uint32_t first = u16(mv, 18 * i + 16), n = u16(mv, 18 * i + 4);
        if (first < nanims) starts.insert({first, n});
    }
    std::vector<std::pair<uint32_t, uint32_t>> st(starts.begin(), starts.end());
    for (size_t i = 0; i < st.size(); i++) {
        size_t end = i + 1 < st.size() ? st[i + 1].first : nanims;
        for (size_t k = st[i].first; k < end; k++) meshes[k] = st[i].second;
    }
    auto mesh_count = [&](size_t k) { auto it = meshes.find(k); return it == meshes.end() ? 0u : it->second; };

    std::set<size_t> offsets;
    for (size_t k = 0; k < nanims; k++) offsets.insert(u32(an, 32 * k) / 2);
    std::vector<uint16_t> out;
    std::map<size_t, size_t> remap;
    for (size_t k = 0; k < nanims; k++) {
        size_t off = u32(an, 32 * k) / 2, words = u8(an, 32 * k + 5);
        if (remap.count(off)) continue;
        auto later = offsets.upper_bound(off);
        size_t limit = later != offsets.end() ? *later : nframes;
        size_t count = words ? (limit >= off ? (limit - off) / words : 0) : 0;
        remap[off] = out.size();
        for (size_t f = 0; f < count; f++) {
            size_t q = off + f * words;
            for (size_t i = 0; i < 9 && q + i < nframes; i++) out.push_back(frame(q + i));
            q += 9;
            for (uint32_t m = 0; m < mesh_count(k); m++) {
                uint32_t w = frame(q);
                if ((w & 0xC000) == 0) {
                    out.push_back(uint16_t(w));
                    if (q + 1 < nframes) out.push_back(frame(q + 1));
                    q += 2;
                } else {
                    uint32_t v = w & 0x3FF, axis = w & 0xC000, x = 0, y = 0, z = 0;
                    (axis == 0x4000 ? x : axis == 0x8000 ? y : z) = v;
                    out.push_back(uint16_t((x << 4) | (y >> 6)));
                    out.push_back(uint16_t(((y & 0x3F) << 10) | z));
                    q += 1;
                }
            }
        }
    }
    for (size_t k = 0; k < nanims; k++) {
        put32(an, 32 * k, uint32_t(remap[u32(an, 32 * k) / 2] * 2));
        put8(an, 32 * k + 5, 9 + 2 * mesh_count(k));
    }
    for (size_t i = 0; i < nmovs; i++) {
        auto it = remap.find(u32(mv, 18 * i + 12) / 2);
        if (it != remap.end()) put32(mv, 18 * i + 12, uint32_t(it->second * 2));
    }
    Bytes nf;
    Writer w{nf};
    for (uint16_t x : out) w.u16(x);
    s["frames"] = nf;
    Bytes res;
    Writer r{res};
    for (auto &[name, size] : ANIM_SECTIONS) r.u32(uint32_t(s[name].size() / size)).bytes(s[name]);
    return res;
}
