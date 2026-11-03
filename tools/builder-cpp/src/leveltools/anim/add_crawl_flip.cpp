// add_crawl_flip.py: the forward crawl exit (animation 313, a roll out of a crawlspace head first).
#include "../tool.h"
#include "anim_data.h"
#include "anim_tools.h"

namespace anim {

std::optional<Bytes> add_crawl_flip(const Bytes &d, const Sections &s, size_t stride) {
    constexpr unsigned INDEX = CRAWL_FLIP_INDEX, BASE = CRAWL_FLIP_BASE;
    constexpr unsigned FRAMES = 44, FRAME_SIZE = 38, FRAME_RATE = 2, AS_NULL = 19, FALL = 23;
    uint32_t na = count(d, s.anims), nf = count(d, s.frames), nc = count(d, s.commands);
    if (na > INDEX && u16(d, anim_at(s, INDEX) + 16) == BASE) return std::nullopt;
    auto movs = read_moveables(d, s, stride);
    unsigned second = second_first_anim(movs);
    bool lara = false;
    for (auto &m : movs)
        if (m.id == 0) { lara = m.anim == 0; break; }
    require(lara && second == INDEX,
            "Lara has not " + std::to_string(INDEX) + " animations (" + std::to_string(second) + ")");
    Bytes w = d;
    shift_anim_refs(w, s, stride, INDEX, 1);
    Insert in;
    in.index = INDEX;
    in.anims = 1;
    append_record(in.records, {2 * nf, FRAME_RATE, FRAME_SIZE, AS_NULL, 0, 0, BASE, BASE + FRAMES - 1, FALL,
                               u16(w, anim_at(s, FALL) + 16), 0, 0, 3, nc});
    append_s16s(in.commands, {1, 0, 815, 620, 2, 0, 0, 5, int(BASE) + 28, 46});   // move, jump velocity, sound
    in.frames.assign(std::begin(data::CRAWL_FLIP_DATA), std::end(data::CRAWL_FLIP_DATA));
    return splice(w, s, in);
}

}  // namespace anim

namespace {
void run_add_crawl_flip(ToolContext &ctx, const std::vector<std::string> &args) {
    anim::require(!args.empty(), "usage: add_crawl_flip.py <level.tr2>");
    Bytes d = read_file(ctx.path(args[0]));
    auto out = anim::add_crawl_flip(d, anim::pc_sections(d));
    if (!out) {
        ctx.print(args[0] + ": has the crawl flip already");
    } else {
        write_file(ctx.path(args[0]), *out);
        ctx.print(args[0] + ": crawl flip added as animation " + std::to_string(anim::CRAWL_FLIP_INDEX));
    }
}
}  // namespace

REGISTER_TOOL("add_crawl_flip.py", run_add_crawl_flip);
