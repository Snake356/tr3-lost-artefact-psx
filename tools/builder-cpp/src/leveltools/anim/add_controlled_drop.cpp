// add_controlled_drop.py: TRX's controlled drop (animations 316 and 317).
#include "../tool.h"
#include "anim_data.h"
#include "anim_tools.h"

namespace anim {

std::optional<Bytes> add_controlled_drop(const Bytes &d, const Sections &s, size_t stride) {
    constexpr unsigned INDEX = CONTROLLED_DROP_INDEX, BASE = CONTROLLED_DROP_BASE;
    constexpr unsigned POSES[] = {8, 3}, FRAME_SIZE = 39, FRAME_RATE = 1, AS_REACH = 11, REACH = 95, N = 2;
    uint32_t na = count(d, s.anims), nf = count(d, s.frames), nc = count(d, s.commands);
    if (na > INDEX && u16(d, anim_at(s, INDEX) + 16) == BASE) return std::nullopt;
    unsigned second = second_first_anim(read_moveables(d, s, stride));
    require(second == INDEX && u16(d, anim_at(s, INDEX - 1) + 16) == CROUCH_TURN_BASE + 25,
            "Lara has not " + std::to_string(INDEX) + " animations ending with the crouch turns (" +
                std::to_string(second) + ")");
    static_assert(sizeof data::CONTROLLED_DROP_DATA == 2 * FRAME_SIZE * (POSES[0] + POSES[1]));
    Bytes w = d;
    shift_anim_refs(w, s, stride, INDEX, N);
    unsigned reach_base = u16(w, anim_at(s, REACH) + 16);
    require(u16(w, anim_at(s, REACH) + 6) == AS_REACH, "animation 95 is not the reach");
    unsigned first = BASE, next = BASE + POSES[0];
    Insert in;
    in.index = INDEX;
    in.anims = N;
    append_record(in.records, {2 * nf, FRAME_RATE, FRAME_SIZE, AS_REACH, 0, 0, first, next - 1, INDEX + 1, next, 0, 0,
                               0, nc});
    append_record(in.records, {2 * nf + 2 * FRAME_SIZE * POSES[0], FRAME_RATE, FRAME_SIZE, AS_REACH, 0, 0, next,
                               next + POSES[1] - 1, REACH, reach_base, 0, 0, 2, nc});
    append_s16s(in.commands, {6, int(BASE) + 9, 0, 1, 0, 128, 0});              // turn round, move
    in.frames.assign(std::begin(data::CONTROLLED_DROP_DATA), std::end(data::CONTROLLED_DROP_DATA));
    return splice(w, s, in);
}

}  // namespace anim

namespace {
void run_add_controlled_drop(ToolContext &ctx, const std::vector<std::string> &args) {
    anim::require(!args.empty(), "usage: add_controlled_drop.py <level.tr2>");
    Bytes d = read_file(ctx.path(args[0]));
    auto out = anim::add_controlled_drop(d, anim::pc_sections(d));
    if (!out) {
        ctx.print(args[0] + ": has the controlled drop already");
    } else {
        write_file(ctx.path(args[0]), *out);
        ctx.print(args[0] + ": controlled drop added as animations " + std::to_string(anim::CONTROLLED_DROP_INDEX) +
                  " and " + std::to_string(anim::CONTROLLED_DROP_INDEX + 1));
    }
}
}  // namespace

REGISTER_TOOL("add_controlled_drop.py", run_add_controlled_drop);
