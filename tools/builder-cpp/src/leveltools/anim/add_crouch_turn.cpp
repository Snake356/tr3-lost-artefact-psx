// add_crouch_turn.py: the turns on the spot in the crouch (animations 314 and 315).
#include "../tool.h"
#include "anim_data.h"
#include "anim_tools.h"

namespace anim {

std::optional<Bytes> add_crouch_turn(const Bytes &d, const Sections &s, size_t stride) {
    constexpr unsigned INDEX = CROUCH_TURN_INDEX, BASE = CROUCH_TURN_BASE;
    constexpr unsigned FRAMES = 25, FRAME_SIZE = 38, FRAME_RATE = 2, AS_DUCK = 71, SOUND = 0x4000 | 24;
    struct Turn {
        unsigned sound_frame;
        const uint8_t *data;
        size_t size;
    };
    const Turn turns[] = {{14, data::CROUCH_TURN_LEFT_DATA, sizeof data::CROUCH_TURN_LEFT_DATA},
                          {15, data::CROUCH_TURN_RIGHT_DATA, sizeof data::CROUCH_TURN_RIGHT_DATA}};
    constexpr unsigned N = 2;
    uint32_t na = count(d, s.anims), nf = count(d, s.frames), nc = count(d, s.commands);
    if (na > INDEX && u16(d, anim_at(s, INDEX) + 16) == BASE) return std::nullopt;
    unsigned second = second_first_anim(read_moveables(d, s, stride));
    require(second == INDEX && u16(d, anim_at(s, INDEX - 1) + 16) == CRAWL_FLIP_BASE,
            "Lara has not " + std::to_string(INDEX) + " animations ending with the crawl flip (" +
                std::to_string(second) + ")");
    Bytes w = d;
    shift_anim_refs(w, s, stride, INDEX, N);
    Insert in;
    in.index = INDEX;
    in.anims = N;
    for (unsigned k = 0; k < N; k++) {
        unsigned base = BASE + FRAMES * k;
        append_record(in.records, {uint32_t(2 * nf + in.frames.size()), FRAME_RATE, FRAME_SIZE, AS_DUCK, 0, 0, base,
                                   base + FRAMES - 1, INDEX + k, base, 0, 0, 1,
                                   unsigned(nc + in.commands.size() / 2)});
        for (unsigned v : {5u, base + turns[k].sound_frame, SOUND}) append_u16(in.commands, v);
        in.frames.insert(in.frames.end(), turns[k].data, turns[k].data + turns[k].size);
    }
    return splice(w, s, in);
}

}  // namespace anim

namespace {
void run_add_crouch_turn(ToolContext &ctx, const std::vector<std::string> &args) {
    anim::require(!args.empty(), "usage: add_crouch_turn.py <level.tr2>");
    Bytes d = read_file(ctx.path(args[0]));
    auto out = anim::add_crouch_turn(d, anim::pc_sections(d));
    if (!out) {
        ctx.print(args[0] + ": has the crouch turns already");
    } else {
        write_file(ctx.path(args[0]), *out);
        ctx.print(args[0] + ": crouch turns added as animations " + std::to_string(anim::CROUCH_TURN_INDEX) + " and " +
                  std::to_string(anim::CROUCH_TURN_INDEX + 1));
    }
}
}  // namespace

REGISTER_TOOL("add_crouch_turn.py", run_add_crouch_turn);
