// bake_point_light.py: bakes point lights into the room vertex colours of a TR3 PC level. Every room vertex within
// the radius of a point gets colour += strength * (1 - distance / radius) * tint (the strongest point counts),
// clamped to 31 per channel.
//     bake_point_light.py level.tr2 x,y,z [x,y,z ...] [--radius 3072] [--strength 10] [--tint 1.0,0.7,0.25] [--above y]
#include <cmath>
#include <format>
#include <optional>

#include "geom.h"

namespace geom {

namespace {

struct Options {
    std::string level;
    std::vector<std::array<int64_t, 3>> points;
    double radius = 3072, strength = 10;
    std::string tint = "1.0,0.7,0.25";
    std::optional<int64_t> above;
};

// Python's round() of a float to an int: halves to the even neighbour.
double pyround(double x) {
    double f = std::floor(x), diff = x - f;
    if (diff > 0.5) return f + 1;
    if (diff < 0.5) return f;
    return std::fmod(f, 2.0) == 0 ? f : f + 1;
}

// The command line as argparse reads it (long options, also as --name=value or a unique prefix).
Options parse(const std::vector<std::string> &args) {
    Options o;
    std::vector<std::string> pos;
    static const char *names[] = {"radius", "strength", "tint", "above"};
    for (size_t i = 0; i < args.size(); i++) {
        const std::string &a = args[i];
        if (a.size() > 2 && a.compare(0, 2, "--") == 0) {
            std::string name = a.substr(2), value;
            size_t eq = name.find('=');
            bool inline_value = eq != std::string::npos;
            if (inline_value) {
                value = name.substr(eq + 1);
                name = name.substr(0, eq);
            }
            std::string full;
            int matches = 0;
            for (const char *n : names) {
                if (name == n) {
                    full = n;
                    matches = 1;
                    break;
                }
                if (std::string(n).compare(0, name.size(), name) == 0) {
                    full = n;
                    matches++;
                }
            }
            if (matches != 1) throw Error("unrecognized arguments: " + a);
            if (!inline_value) {
                if (i + 1 >= args.size()) throw Error("argument --" + full + ": expected one argument");
                value = args[++i];
            }
            if (full == "radius") o.radius = parse_float(value);
            else if (full == "strength") o.strength = parse_float(value);
            else if (full == "tint") o.tint = value;
            else o.above = parse_int(value);
        } else {
            pos.push_back(a);
        }
    }
    if (pos.size() < 2) throw Error("the following arguments are required: level, points");
    o.level = pos[0];
    for (size_t i = 1; i < pos.size(); i++) {
        auto parts = split(pos[i], ',');
        if (parts.size() != 3) throw Error("a point is x,y,z: " + pos[i]);
        o.points.push_back({parse_int(parts[0]), parse_int(parts[1]), parse_int(parts[2])});
    }
    return o;
}

void bake_point_light(ToolContext &ctx, const std::vector<std::string> &args) {
    Options a = parse(args);
    std::vector<double> tint;
    for (auto &v : split(a.tint, ',')) tint.push_back(parse_float(v));
    if (tint.size() < 3) throw Error("list index out of range");
    Bytes b = read_file(ctx.path(a.level));
    size_t p = 1804 + size_t(u32(b, 1796)) * 196608;   // after the TR3 palettes and texture tiles
    unsigned nrooms = u16(b, p);
    p += 2;
    int changed = 0;
    for (unsigned r = 0; r < nrooms; r++) {
        int64_t rx = s32(b, p), rz = s32(b, p + 4);
        p += 16;
        size_t ndw = u32(b, p);
        size_t data = p + 4;
        p = data + ndw * 2;
        int nv = s16(b, data);
        for (int k = 0; k < nv; k++) {
            size_t v = data + 2 + size_t(k) * 12;   // x, y, z, lighting, attributes, colour
            int64_t x = rx + s16(b, v), y = s16(b, v + 2), z = rz + s16(b, v + 4);
            if (a.above && y >= *a.above) continue;
            double add = 0.0;                       // the strongest point counts
            for (auto &pt : a.points) {
                double dx = double(x) - double(pt[0]), dy = double(y) - double(pt[1]), dz = double(z) - double(pt[2]);
                double d = std::sqrt(dx * dx + dy * dy + dz * dz);
                if (d < a.radius) {
                    double s = a.strength * (1 - d / a.radius);
                    if (s > add) add = s;
                }
            }
            if (add <= 0) continue;
            uint32_t c = u16(b, v + 10);
            int64_t ch[3] = {(c >> 10) & 31, (c >> 5) & 31, c & 31};
            for (int i = 0; i < 3; i++)
                ch[i] = std::min<int64_t>(31, int64_t(pyround(double(ch[i]) + add * tint[size_t(i)])));
            put16(b, v + 10, uint32_t((c & 0x8000) | (ch[0] << 10) | (ch[1] << 5) | ch[2]));
            changed++;
        }
        p += 2 + size_t(u16(b, p)) * 32;            // portals
        size_t nz = u16(b, p), nx = u16(b, p + 2);
        p += 4 + nz * nx * 8 + 4;                   // sectors, ambient
        p += 2 + size_t(u16(b, p)) * 24;            // lights
        p += 2 + size_t(u16(b, p)) * 20 + 7;        // static meshes, alternate room, flags, water, reverb, filler
    }
    write_file(ctx.path(a.level), b);
    ctx.print(std::format("{}: {} room vertices brightened around {} point(s)", a.level, changed, a.points.size()));
}

}  // namespace

REGISTER_TOOL("bake_point_light.py", bake_point_light);

}  // namespace geom
