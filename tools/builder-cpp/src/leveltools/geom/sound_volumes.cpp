// sound_volumes.py: two of Lara's quiet sounds louder (the volume byte of their sound details: 71 -> 127, 54 -> 100),
// and Shakespeare Cliff's wet step with one variant instead of four. A TR3 PC level or Lara's home (HOUSE.PSX, whose
// sound map and details have the PC layout; found from sound 71's details entry).
//     sound_volumes.py level.tr2 | house.psx
#include <format>

#include "geom.h"

namespace geom {

namespace {

constexpr std::pair<int, int> VOLUMES[] = {{71, 127}, {54, 100}};

std::string set_variants(Bytes &b, size_t smap, size_t det, const std::string &level) {
    std::string out;
    if (level != "chunnel") return out;
    for (auto [sound, n] : {std::pair<int, int>{17, 1}}) {
        int d = s16(b, smap + size_t(sound) * 2);
        if (d >= 0) {
            size_t o = det + size_t(d) * 8 + 6;
            uint32_t f = u16(b, o);
            put16(b, o, (f & ~0x3Cu) | uint32_t(n) << 2);
            out += std::format(", {}: {} -> {} variants", sound, (f >> 2) & 15, n);
        }
    }
    return out;
}

std::string set_volumes(Bytes &b, size_t smap, size_t det) {
    std::string out;
    for (auto [sound, volume] : VOLUMES) {
        int d = s16(b, smap + size_t(sound) * 2);
        if (d >= 0) {
            size_t o = det + size_t(d) * 8 + 2;
            if (!out.empty()) out += ", ";
            out += std::format("{}: {} -> {}", sound, u8(b, o), volume);
            put8(b, o, uint32_t(volume));
        }
    }
    return out;
}

std::pair<size_t, size_t> house(const Bytes &b) {
    static const Bytes entry = from_hex("4000330a00f40440");    // sound 71's details: sample 64, volume 51
    size_t n = count_nonoverlapping(b, entry);
    if (n != 1) throw Error(std::format("the swoosh's details found {} times", n));
    size_t off = find_bytes(b, entry);
    for (size_t k = 0; k < 400; k++) {
        int64_t det = int64_t(off) - 8 * int64_t(k);
        int64_t smap = det - 4 - 370 * 2;
        if (smap + 71 * 2 < 0 || det - 4 < 0) continue;
        if (s16(b, size_t(smap + 71 * 2)) == int64_t(k)) {
            uint32_t nd = u32(b, size_t(det - 4));
            if (k < nd && nd < 400) return {size_t(smap), size_t(det)};
        }
    }
    throw Error("sound map not found");
}

void sound_volumes(ToolContext &ctx, const std::vector<std::string> &args) {
    if (args.empty()) throw Error("usage: sound_volumes.py level.tr2 | house.psx");
    const std::string &path = args[0];
    Bytes b = read_file(ctx.path(path));
    size_t smap, det;
    if (ends_with(lower(path), ".psx")) {
        std::tie(smap, det) = house(b);
    } else {
        smap = sound_map(b);
        det = smap + 370 * 2 + 4;
    }
    std::string done = set_volumes(b, smap, det);
    done += set_variants(b, smap, det, split(basename(path), '.')[0]);
    write_file(ctx.path(path), b);
    ctx.print(path + " " + done);
}

}  // namespace

REGISTER_TOOL("sound_volumes.py", sound_volumes);

}  // namespace geom
