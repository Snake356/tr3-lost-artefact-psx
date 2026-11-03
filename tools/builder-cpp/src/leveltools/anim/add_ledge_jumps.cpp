// add_ledge_jumps.py: TRX's jumps up and back from the hang (animations 318 to 321).
#include "../tool.h"
#include "anim_data.h"
#include "anim_tools.h"

namespace anim {

std::optional<Bytes> add_ledge_jumps(const Bytes &d, const Sections &s, size_t stride) {
    constexpr unsigned INDEX = LEDGE_JUMPS_INDEX, BASE = LEDGE_JUMPS_BASE;
    constexpr unsigned POSES[] = {23, 10, 37, 2}, FRAME_SIZE = 39, FRAME_RATE = 1, N = 4;
    constexpr unsigned HANG = 96, UPJUMP = 28, FORWARD_START = 76;
    constexpr unsigned AS_HANG = 10, AS_UPJUMP = 28, AS_BACKJUMP = 25, AS_FORWARDJUMP = 3;
    constexpr int32_t BACK_SPEED = 99 << 16;
    constexpr int UP = BASE, UP2 = UP + POSES[0], BACK = UP2 + POSES[1], BACK2 = BACK + POSES[2];
    constexpr unsigned TOTAL = POSES[0] + POSES[1] + POSES[2] + POSES[3];
    const std::vector<int> COMMANDS[N] = {
        {5, UP + 10, 5, 5, UP + 19, 0, 3},                                          // sounds, hands free
        {5, UP2, 29, 1, 0, -703, 0, 2, -78, 2},                                     // sound, move, jump velocity
        {5, BACK + 10, 5, 5, BACK + 19, 0, 5, BACK + 35, 29, 6, BACK + 36, 0, 3},   // sounds, turn round
        {1, 0, -603, 17}};                                                          // move
    constexpr unsigned NCOMMANDS[N] = {3, 3, 5, 1};
    uint32_t na = count(d, s.anims), nd = count(d, s.dispatch), nf = count(d, s.frames), nc = count(d, s.commands);
    if (na > INDEX && u16(d, anim_at(s, INDEX) + 16) == BASE) return std::nullopt;
    unsigned second = second_first_anim(read_moveables(d, s, stride));
    require(second == INDEX && u16(d, anim_at(s, INDEX - 1) + 16) == CONTROLLED_DROP_BASE + 8,
            "Lara has not " + std::to_string(INDEX) + " animations ending with the controlled drop (" +
                std::to_string(second) + ")");
    static_assert(sizeof data::LEDGE_JUMPS_DATA == 2 * FRAME_SIZE * TOTAL);
    auto state = [&](unsigned i) { return u16(d, anim_at(s, i) + 6); };
    require(state(HANG) == AS_HANG && state(UPJUMP) == AS_UPJUMP && state(FORWARD_START) == AS_FORWARDJUMP,
            "animations 96, 28 and 76 are not the hang, the up jump and the forward jump's start");
    Bytes w = d;
    shift_anim_refs(w, s, stride, INDEX, N);
    unsigned hang_end = u16(w, anim_at(s, HANG) + 18);
    unsigned changes = u16(w, anim_at(s, HANG) + 24), first = u16(w, anim_at(s, HANG) + 26);
    bool has_jumps = false;
    for (unsigned c = 0; c < changes; c++) {
        unsigned st = u16(w, s.states + 4 + 6 * size_t(first + c));
        if (st == AS_UPJUMP || st == AS_BACKJUMP) has_jumps = true;
    }
    require(changes && !has_jumps, "the hang has state changes to the jumps already");
    size_t at = size_t(first) + changes;                  // the hang's state changes grow in place
    for (uint32_t i = 0; i < na; i++) {
        size_t o = anim_at(s, i) + 26;
        if (i != HANG && u16(w, o) >= at) pack_u16(w, o, u16(w, o) + 2);
    }
    pack_u16(w, anim_at(s, HANG) + 24, changes + 2);
    Insert in;
    in.index = INDEX;
    in.anims = N;
    in.states_at = at;
    in.new_states = 2;
    in.new_ranges = 2;
    for (long long v : {(long long)AS_UPJUMP, 1LL, (long long)nd, (long long)AS_BACKJUMP, 1LL, nd + 1LL})
        append_u16(in.states, v);
    int he = int(hang_end);
    append_s16s(in.ranges, {he, he + 1, int(INDEX), UP, he, he + 1, int(INDEX) + 2, BACK});
    unsigned upjump_base = u16(w, anim_at(s, UPJUMP) + 16);
    unsigned forward_end = u16(w, anim_at(s, FORWARD_START) + 18);
    struct Head {
        unsigned state;
        int32_t speed;
        unsigned base, end, next, next_frame;
    };
    const Head heads[N] = {{AS_UPJUMP, 0, UP, UP2 - 1, INDEX + 1, UP2},
                           {AS_UPJUMP, 0, UP2, BACK - 1, UPJUMP, upjump_base},
                           {AS_BACKJUMP, 0, BACK, BACK2 - 1, INDEX + 3, BACK2},
                           {AS_FORWARDJUMP, BACK_SPEED, BACK2, BASE + TOTAL - 1, FORWARD_START, forward_end}};
    unsigned long long offset = 2ULL * nf, cmd = nc;
    for (unsigned k = 0; k < N; k++) {
        const Head &h = heads[k];
        require(offset <= 0xFFFFFFFFULL && cmd <= 0xFFFF, "level too large");
        append_record(in.records, {uint32_t(offset), FRAME_RATE, FRAME_SIZE, h.state, h.speed, 0, h.base, h.end, h.next,
                                   h.next_frame, 0, 0, NCOMMANDS[k], unsigned(cmd)});
        offset += 2ULL * FRAME_SIZE * POSES[k];
        cmd += COMMANDS[k].size();
    }
    for (auto &c : COMMANDS) append_s16s(in.commands, c);
    in.frames.assign(std::begin(data::LEDGE_JUMPS_DATA), std::end(data::LEDGE_JUMPS_DATA));
    return splice(w, s, in);
}

}  // namespace anim

namespace {
void run_add_ledge_jumps(ToolContext &ctx, const std::vector<std::string> &args) {
    anim::require(!args.empty(), "usage: add_ledge_jumps.py <level.tr2>");
    Bytes d = read_file(ctx.path(args[0]));
    auto out = anim::add_ledge_jumps(d, anim::pc_sections(d));
    if (!out) {
        ctx.print(args[0] + ": has the ledge jumps already");
    } else {
        write_file(ctx.path(args[0]), *out);
        ctx.print(args[0] + ": ledge jumps added as animations " + std::to_string(anim::LEDGE_JUMPS_INDEX) + " to " +
                  std::to_string(anim::LEDGE_JUMPS_INDEX + 3));
    }
}
}  // namespace

REGISTER_TOOL("add_ledge_jumps.py", run_add_ledge_jumps);
