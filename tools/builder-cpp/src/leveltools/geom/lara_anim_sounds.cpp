// lara_anim_sounds.py: Lara's animation sounds as TRX's lara_animations.bin has them, where her animations call a
// sound no level has. Each sound command is changed in place (frame and sound; the land/water bits stay).
// A TR3 PC level (moveables of 18 bytes) or a PSX level (Lara's home, 20 bytes).
//     lara_anim_sounds.py level.tr2 | house.psx
#include <format>

#include "geom.h"

namespace geom {

namespace {

struct Change {
    int anim;
    int old_frame, old_sound, new_frame, new_sound;
};

// frames counted from the animation's first frame; in the order of the Python table
constexpr Change CHANGES[] = {
    {1, 0, 20, 4, 17},     {1, 18, 20, 24, 17},   {21, 13, 20, 6, 17},   {40, 10, 20, 12, 17},
    {40, 46, 20, 42, 17},  {57, 10, 20, 2, 17},   {58, 12, 20, 5, 17},   {73, 8, 20, 3, 17},
    {136, 11, 28, 11, 54}, {136, 15, 28, 15, 54}, {137, 12, 28, 12, 54}, {137, 15, 28, 15, 54},
    {44, 0, 35, 9, 52},    {326, 20, 28, 20, 54}, {328, 20, 28, 20, 54},
};

struct Lists {
    size_t moveables, anims, commands;
};

void patch_bytes(Bytes &d, const Lists &s, size_t model, int &done, int &left) {
    size_t first = 0;
    bool found = false;
    for (size_t i = 0; i < u32(d, s.moveables); i++)
        if (u32(d, s.moveables + 4 + model * i) == 0) {
            first = u16(d, s.moveables + 4 + model * i + 16);
            found = true;
            break;
        }
    if (!found) throw Error("no moveable 0 (Lara)");
    done = left = 0;
    for (const Change &c : CHANGES) {
        size_t rec = s.anims + 4 + 32 * (first + size_t(c.anim));
        uint32_t fstart = u16(d, rec + 16), ncmd = u16(d, rec + 28), cofs = u16(d, rec + 30);
        size_t p = s.commands + 4 + 2 * size_t(cofs);
        bool hit = false;
        for (uint32_t n = 0; n < ncmd; n++) {
            uint32_t t = u16(d, p);
            p += 2;
            if (t == 1) {
                p += 6;
            } else if (t == 2) {
                p += 4;
            } else if (t == 5 || t == 6) {
                uint32_t f = u16(d, p), v = u16(d, p + 2);
                if (t == 5 && f == fstart + uint32_t(c.old_frame) && (v & 0x3FFF) == uint32_t(c.old_sound)) {
                    put16(d, p, fstart + uint32_t(c.new_frame));
                    put16(d, p + 2, (v & 0xC000) | uint32_t(c.new_sound));
                    hit = true;
                    break;
                }
                p += 4;
            }
        }
        (hit ? done : left)++;
    }
}

void lara_anim_sounds(ToolContext &ctx, const std::vector<std::string> &args) {
    if (args.empty()) throw Error("usage: lara_anim_sounds.py level.tr2 | house.psx");
    const std::string &path = args[0];
    Bytes d = read_file(ctx.path(path));
    Lists s;
    size_t model;
    if (ends_with(lower(path), ".psx")) {
        PsxSections p = psx_sections(d);
        s = {p.moveables, p.anims, p.commands};
        model = PSX_MODEL;
    } else {
        Sections p = sections(d);
        s = {p.moveables, p.anims, p.commands};
        model = 18;
    }
    int done = 0, left = 0;
    patch_bytes(d, s, model, done, left);
    write_file(ctx.path(path), d);
    ctx.print(std::format("{} {} commands changed, {} not found", path, done, left));
}

}  // namespace

REGISTER_TOOL("lara_anim_sounds.py", lara_anim_sounds);

}  // namespace geom
