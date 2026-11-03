// add_corner_shimmy.py: TRX's hanging round outer and inner corners (animations 322 to 329).
#include "../tool.h"
#include "anim_data.h"
#include "anim_tools.h"

namespace anim {

std::optional<Bytes> add_corner_shimmy(const Bytes &d, const Sections &s, size_t stride) {
    constexpr unsigned INDEX = CORNER_SHIMMY_INDEX, BASE = CORNER_SHIMMY_BASE, N = 8;
    constexpr unsigned POSES[N] = {22, 1, 22, 1, 7, 1, 7, 1};
    constexpr unsigned FRAMES[N] = {84, 1, 84, 1, 24, 1, 24, 1};
    constexpr unsigned RATES[N] = {4, 1, 4, 1, 4, 1, 4, 1};
    constexpr unsigned SIZES[N] = {38, 37, 37, 37, 37, 37, 37, 37};
    constexpr unsigned HANG = 96, HANG_FRAME = 21, AS_HANG = 10, AS_CORNER = 62;
    unsigned bases[N];
    for (unsigned k = 0, b = BASE; k < N; b += FRAMES[k], k++) bases[k] = b;
    const std::vector<int> COMMANDS[4] = {{5, int(bases[0]) + 53, 0x4001}, {5, int(bases[2]) + 52, 0x4001},   // sounds
                                          {5, int(bases[4]) + 20, 0x401C}, {5, int(bases[6]) + 20, 0x401C}};
    uint32_t na = count(d, s.anims), nf = count(d, s.frames), nc = count(d, s.commands);
    if (na > INDEX && u16(d, anim_at(s, INDEX) + 16) == BASE) return std::nullopt;
    unsigned second = second_first_anim(read_moveables(d, s, stride));
    require(second == INDEX && u16(d, anim_at(s, INDEX - 1) + 16) == LEDGE_JUMPS_BASE + 70,
            "Lara has not " + std::to_string(INDEX) + " animations ending with the ledge jumps (" +
                std::to_string(second) + ")");
    size_t total = 0;
    for (unsigned k = 0; k < N; k++) {
        total += 2 * POSES[k] * SIZES[k];
        require((FRAMES[k] - 1) / RATES[k] + 1 + ((FRAMES[k] - 1) % RATES[k] != 0) == POSES[k], "pose counts");
    }
    require(sizeof data::CORNER_SHIMMY_DATA == total, "pose data size");
    require(u16(d, anim_at(s, HANG) + 6) == AS_HANG, "animation 96 is not the hang");
    unsigned hang = u16(d, anim_at(s, HANG) + 16) + HANG_FRAME;
    Bytes w = d;
    shift_anim_refs(w, s, stride, INDEX, N);
    Insert in;
    in.index = INDEX;
    in.anims = N;
    unsigned long long offset = 2ULL * nf, cmd = nc;
    for (unsigned k = 0; k < N; k++) {
        bool turn = k % 2 == 0;
        unsigned next = turn ? INDEX + k + 1 : HANG, next_frame = turn ? bases[k + 1] : hang;
        require(offset <= 0xFFFFFFFFULL && cmd <= 0xFFFF, "level too large");
        append_record(in.records, {uint32_t(offset), RATES[k], SIZES[k], AS_CORNER, 0, 0, bases[k],
                                   bases[k] + FRAMES[k] - 1, next, next_frame, 0, 0, turn ? 1u : 0u, unsigned(cmd)});
        offset += 2ULL * SIZES[k] * POSES[k];
        cmd += turn ? COMMANDS[k / 2].size() : 0;
    }
    for (auto &c : COMMANDS) append_s16s(in.commands, c);
    in.frames.assign(std::begin(data::CORNER_SHIMMY_DATA), std::end(data::CORNER_SHIMMY_DATA));
    return splice(w, s, in);
}

}  // namespace anim

namespace {
void run_add_corner_shimmy(ToolContext &ctx, const std::vector<std::string> &args) {
    anim::require(!args.empty(), "usage: add_corner_shimmy.py <level.tr2>");
    Bytes d = read_file(ctx.path(args[0]));
    auto out = anim::add_corner_shimmy(d, anim::pc_sections(d));
    if (!out) {
        ctx.print(args[0] + ": has the corner shimmying already");
    } else {
        write_file(ctx.path(args[0]), *out);
        ctx.print(args[0] + ": corner shimmying added as animations " + std::to_string(anim::CORNER_SHIMMY_INDEX) +
                  " to " + std::to_string(anim::CORNER_SHIMMY_INDEX + 7));
    }
}
}  // namespace

REGISTER_TOOL("add_corner_shimmy.py", run_add_corner_shimmy);
