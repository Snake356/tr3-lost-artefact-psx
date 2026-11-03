// psx_lara_anims.py: adds Lara's new animations (crawl flip, crouch turns, crawl pickup, crouch roll arm,
// controlled drop, ledge jumps, corner shimmying, climbing round corners) to a finished PSX level and points
// the animations only missing objects start at a single pose. The PSX animation lists are those of the PC
// level, only a moveable takes 20 bytes. A level that grows is padded to a multiple of 2048 again.
#include <format>

#include "../anim/anim_tools.h"
#include "../tool.h"
#include "psx_inventory.h"

namespace psx {
namespace {

constexpr size_t MODEL = 20;

// The animation lists and the moveables behind the mesh data and mesh pointers, and the item list.
anim::Sections sections(const Bytes &d) {
    Inventory z = inventory(d);
    require(z.ok(), z.status);
    size_t o = find_mesh_block(d, z) + 8 + size_t(z.meshdata) * 2 + size_t(z.meshptr) * 4;
    anim::Sections s;
    std::pair<size_t *, size_t> lists[] = {{&s.anims, 32}, {&s.states, 6}, {&s.dispatch, 8}, {&s.commands, 2},
                                           {&s.trees, 4},  {&s.frames, 2}, {&s.moveables, MODEL}};
    for (auto [field, size] : lists) {
        *field = o;
        o += 4 + size * u32(d, o);
    }
    s.items = z.p_entities - 4;
    return s;
}

using AddFn = std::optional<Bytes> (*)(const Bytes &, const anim::Sections &, size_t);

std::optional<Bytes> strip_dead(const Bytes &d, const anim::Sections &s, size_t stride) {
    return anim::strip_dead_anims(d, s, stride);
}

void run_psx_lara_anims(ToolContext &ctx, const std::vector<std::string> &args) {
    require(!args.empty(), "usage: psx_lara_anims.py in.psx [out.psx]");
    const std::string &dst = args.size() > 1 ? args[1] : args[0];
    Bytes d = read_file(ctx.path(args[0]));
    size_t n = d.size();
    for (AddFn add : {&anim::add_crawl_flip, &anim::add_crouch_turn, &anim::add_crawl_pickup,
                      &anim::fix_crouch_roll_arm, &anim::add_controlled_drop, &anim::add_ledge_jumps,
                      &anim::add_corner_shimmy, &anim::add_climb_corners, &strip_dead})
        if (auto out = add(d, sections(d), MODEL)) d = std::move(*out);
    if (d.size() != n) d.resize(d.size() + (2048 - d.size() % 2048) % 2048);
    write_file(ctx.path(dst), d);
    ctx.print(std::format("{}: {:+d} bytes", dst, (long long)d.size() - (long long)n));
}

REGISTER_TOOL("psx_lara_anims.py", run_psx_lara_anims);

}  // namespace
}  // namespace psx
