// add_climb_corners.py: TRX's climbing round outer and inner corners of climbable walls (animations 330 to 337).
#include "../tool.h"
#include "anim_data.h"
#include "anim_tools.h"

namespace anim {

std::optional<Bytes> add_climb_corners(const Bytes &d, const Sections &s, size_t stride) {
    constexpr unsigned INDEX = CLIMB_CORNERS_INDEX, BASE = CLIMB_CORNERS_BASE, N = 8;
    constexpr unsigned POSES[N] = {32, 1, 32, 1, 30, 1, 30, 1};
    constexpr unsigned FRAMES[N] = {122, 1, 122, 1, 116, 1, 116, 1};
    constexpr unsigned RATES[N] = {4, 1, 4, 1, 4, 1, 4, 1};
    constexpr unsigned FRAME_SIZE = 39, STANCE = 164, AS_STANCE = 56, AS_CORNER = 62;
    int b[N];
    for (unsigned k = 0, x = BASE; k < N; x += FRAMES[k], k++) b[k] = int(x);
    const std::vector<int> COMMANDS[4] = {                                                       // sounds
        {5, b[0] + 56, 0x4000, 5, b[0] + 92, 0x4000, 5, b[0] + 66, 0x4001},
        {5, b[2] + 56, 0x4000, 5, b[2] + 94, 0x4000, 5, b[2] + 66, 0x4001},
        {5, b[4] + 87, 0x4000, 5, b[4] + 99, 0x4005},
        {5, b[6] + 87, 0x4000, 5, b[6] + 100, 0x4005}};
    uint32_t na = count(d, s.anims), nf = count(d, s.frames), nc = count(d, s.commands);
    if (na > INDEX && u16(d, anim_at(s, INDEX) + 16) == BASE) return std::nullopt;
    unsigned second = second_first_anim(read_moveables(d, s, stride));
    require(second == INDEX && u16(d, anim_at(s, INDEX - 1) + 16) == CORNER_SHIMMY_BASE + 219,
            "Lara has not " + std::to_string(INDEX) + " animations ending with the corner shimmying (" +
                std::to_string(second) + ")");
    size_t total = 0;
    for (unsigned k = 0; k < N; k++) {
        total += 2 * FRAME_SIZE * POSES[k];
        require((FRAMES[k] - 1) / RATES[k] + 1 + ((FRAMES[k] - 1) % RATES[k] != 0) == POSES[k], "pose counts");
    }
    require(sizeof data::CLIMB_CORNERS_DATA == total, "pose data size");
    require(u16(d, anim_at(s, STANCE) + 6) == AS_STANCE, "animation 164 is not the climbing stance");
    unsigned stance = u16(d, anim_at(s, STANCE) + 16);
    Bytes w = d;
    shift_anim_refs(w, s, stride, INDEX, N);
    Insert in;
    in.index = INDEX;
    in.anims = N;
    unsigned long long offset = 2ULL * nf, cmd = nc;
    for (unsigned k = 0; k < N; k++) {
        bool turn = k % 2 == 0;
        unsigned next = turn ? INDEX + k + 1 : STANCE, next_frame = turn ? unsigned(b[k + 1]) : stance;
        unsigned ncmd = turn ? unsigned(COMMANDS[k / 2].size() / 3) : 0;
        require(offset <= 0xFFFFFFFFULL && cmd <= 0xFFFF, "level too large");
        append_record(in.records, {uint32_t(offset), RATES[k], FRAME_SIZE, AS_CORNER, 0, 0, unsigned(b[k]),
                                   unsigned(b[k]) + FRAMES[k] - 1, next, next_frame, 0, 0, ncmd, unsigned(cmd)});
        offset += 2ULL * FRAME_SIZE * POSES[k];
        cmd += turn ? COMMANDS[k / 2].size() : 0;
    }
    for (auto &c : COMMANDS) append_s16s(in.commands, c);
    in.frames.assign(std::begin(data::CLIMB_CORNERS_DATA), std::end(data::CLIMB_CORNERS_DATA));
    return splice(w, s, in);
}

}  // namespace anim

namespace {
void run_add_climb_corners(ToolContext &ctx, const std::vector<std::string> &args) {
    anim::require(!args.empty(), "usage: add_climb_corners.py <level.tr2>");
    Bytes d = read_file(ctx.path(args[0]));
    auto out = anim::add_climb_corners(d, anim::pc_sections(d));
    if (!out) {
        ctx.print(args[0] + ": has the climbing round corners already");
    } else {
        write_file(ctx.path(args[0]), *out);
        ctx.print(args[0] + ": climbing round corners added as animations " +
                  std::to_string(anim::CLIMB_CORNERS_INDEX) + " to " + std::to_string(anim::CLIMB_CORNERS_INDEX + 7));
    }
}
}  // namespace

REGISTER_TOOL("add_climb_corners.py", run_add_climb_corners);
