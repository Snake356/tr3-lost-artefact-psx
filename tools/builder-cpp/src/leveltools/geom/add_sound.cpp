// add_sound.py: adds a sound a level's sound map lacks: one sample index and one sound details entry are appended and
// the sound map points the sound at them. Default: SFX_ALARM_1 (208) with sample 215 for Shakespeare Cliff.
//     add_sound.py level.tr2 [sound sample volume radius chance pitch flags]
#include <format>

#include "geom.h"

namespace geom {

// the sound map of a TR3 PC level: behind the items, the light map, the cinematic frames and the demo data
size_t sound_map(const Bytes &b) {
    size_t p = sections(b).items;
    p += 4 + size_t(u32(b, p)) * 24;
    p += 8192;
    p += 2 + size_t(u16(b, p)) * 16;
    p += 2 + size_t(u16(b, p));
    return p;
}

namespace {

std::string patch(ToolContext &ctx, const std::string &path, const std::vector<int64_t> &a) {
    if (a.size() != 7) throw Error(std::format("patch() takes 8 positional arguments but {} were given", a.size() + 1));
    int64_t sound = a[0], sample = a[1];
    Bytes b = read_file(ctx.path(path));
    size_t smap = sound_map(b);
    size_t so = add(smap, 2 * sound);
    if (s16(b, so) >= 0) return std::format("sound {} is there already", sound);
    size_t pd = smap + 370 * 2;
    uint32_t nd = u32(b, pd);
    size_t pi = pd + 4 + size_t(nd) * 8;
    uint32_t ni = u32(b, pi);
    int64_t k = -1;
    for (uint32_t i = 0; i < ni; i++)
        if (int64_t(u32(b, pi + 4 + 4 * size_t(i))) == sample) {
            k = i;
            break;
        }
    if (k < 0) {
        k = ni;
        Bytes s;
        Writer{s}.u32(uint32_t(sample));
        size_t at = pi + 4 + size_t(ni) * 4;
        check_range(b, 0, at);
        b.insert(b.begin() + ptrdiff_t(at), s.begin(), s.end());
        put32(b, pi, ni + 1);
    }
    Bytes det;
    Writer{det}.u16(uint32_t(k)).u8(uint32_t(a[2])).u8(uint32_t(a[3])).u8(uint32_t(a[4])).u8(uint32_t(a[5])).u16(uint32_t(a[6]));
    b.insert(b.begin() + ptrdiff_t(pi), det.begin(), det.end());
    put32(b, pd, nd + 1);
    put16(b, so, nd);
    write_file(ctx.path(path), b);
    return std::format("sound {} -> details {}, sample index {} (sample {:03})", sound, nd, k, sample);
}

void add_sound(ToolContext &ctx, const std::vector<std::string> &args) {
    if (args.empty()) throw Error("usage: add_sound.py level.tr2 [sound sample volume radius chance pitch flags]");
    std::vector<int64_t> a;
    for (size_t i = 1; i < args.size(); i++) a.push_back(parse_int(args[i], 0));
    if (a.empty()) a = {208, 215, 76, 11, 0, 199, 0x0004};
    ctx.print(args[0] + " " + patch(ctx, args[0], a));
}

}  // namespace

REGISTER_TOOL("add_sound.py", add_sound);

}  // namespace geom
