// fix_crouch_roll_arm.py: TRX's right arm (bones 8 to 10) in the poses of the crouch roll's end (animation 220).
#include <array>

#include "../tool.h"
#include "anim_tools.h"

namespace anim {
namespace {

constexpr unsigned POSES = 10;     // 9 played, 1 stored behind
constexpr unsigned BONES[] = {8, 9, 10};
constexpr std::array<std::array<uint16_t, 6>, POSES> OLD = {{
    {0x002C, 0xAEFB, 0x0700, 0x0000, 0x3BC0, 0x1B3E}, {0x3F6C, 0xE6EC, 0x0970, 0x0000, 0x3A90, 0x86ED},
    {0x007D, 0x06E7, 0x0B50, 0x0000, 0x39C0, 0x5EED}, {0x031D, 0x32E9, 0x0D60, 0x0000, 0x396F, 0xFF17},
    {0x06CD, 0x92FB, 0x0F90, 0x0000, 0x39DF, 0x875E}, {0x0A3E, 0xA33A, 0x0E47, 0xFDFF, 0x3ABF, 0x23B2},
    {0x0AE0, 0xC7BE, 0x0C67, 0xFDFF, 0x3B9E, 0xC804}, {0x08E2, 0x140E, 0x0AF7, 0xFDFF, 0x3C8E, 0x7C49},
    {0x0722, 0x8027, 0x0A37, 0xFDFF, 0x3D3E, 0x4872}, {0x06E2, 0x8C29, 0x0A17, 0xFDFF, 0x3D5E, 0x4078}}};
constexpr std::array<std::array<uint16_t, 5>, POSES> NEW = {{
    {0x3FCC, 0xBAF7, 0x4080, 0x3B30, 0x5B01}, {0x3FCC, 0xF6E9, 0x40A6, 0x3A20, 0x76E8},
    {0x019D, 0x1AE6, 0x40C5, 0x3970, 0x36FC}, {0x034D, 0x36EE, 0x40D6, 0x39CF, 0xEF33},
    {0x03FD, 0x4EF4, 0x40DA, 0x3A4F, 0xC369}, {0x031D, 0x46FA, 0x40CD, 0x3B1F, 0xC39A},
    {0x024D, 0x42FF, 0x40BF, 0x3BDF, 0xC3CC}, {0x017D, 0x3B04, 0x40B2, 0x3C8F, 0xC3FA},
    {0x00CD, 0x3308, 0x40A8, 0x3D3F, 0xC022}, {0x008D, 0x330A, 0x40A4, 0x3D6F, 0xC02E}}};

using Rot = std::vector<uint16_t>;           // one rotation: one or two words
using Angles = std::array<unsigned, 3>;

// The 15 rotations of a pose (behind the box and the offset).
std::vector<Rot> split_rots(const std::vector<uint16_t> &words) {
    std::vector<Rot> out;
    size_t j = 9;
    for (int r = 0; r < 15; r++) {
        if (j >= words.size()) throw Error("pose shorter than its rotations");
        size_t n = (words[j] & 0xC000) ? 1 : 2;
        if (j + n > words.size()) n = words.size() - j;
        out.emplace_back(words.begin() + j, words.begin() + j + n);
        j += n;
    }
    return out;
}

Angles angles(const Rot &r) {
    if (r.size() == 2) return {unsigned(r[0] >> 4 & 0x3FF), unsigned((r[0] & 0xF) << 6 | r[1] >> 10), unsigned(r[1] & 0x3FF)};
    unsigned v = r[0] & 0x3FF, axis = r[0] >> 14;
    return {axis == 1 ? v : 0, axis == 2 ? v : 0, axis == 3 ? v : 0};
}

template <size_t N>
std::array<Rot, 3> as_rots(const std::array<uint16_t, N> &w) {
    Rot mid = N == 5 ? Rot{w[2]} : Rot{w[2], w[3]};
    return {Rot{w[0], w[1]}, mid, Rot{w[N - 2], w[N - 1]}};
}

template <size_t N>
std::array<Angles, 3> arm_angles(const std::array<uint16_t, N> &w) {
    auto r = as_rots(w);
    return {angles(r[0]), angles(r[1]), angles(r[2])};
}

}  // namespace

std::optional<Bytes> fix_crouch_roll_arm(const Bytes &d, const Sections &s, size_t /*stride*/) {
    size_t a = anim_at(s, CROUCH_ROLL_ANIM);
    uint32_t offset = u32(d, a);
    unsigned size = u8(d, a + 5), rate = u8(d, a + 4);
    long long first = u16(d, a + 16), last = u16(d, a + 18);
    require(rate != 0, "animation 220 has frame rate 0");
    require(floordiv(last - first, rate) + 1 == POSES - 1,
            "(" + std::to_string(first) + ", " + std::to_string(last) + ", " + std::to_string(rate) + ")");
    require(u32(d, a + 32) == offset + 2ULL * size * POSES, "no pose behind");
    Bytes w = d;
    unsigned done = 0;
    for (unsigned k = 0; k < POSES; k++) {
        size_t o = s.frames + 4 + offset + 2 * size_t(size) * k;
        std::vector<uint16_t> words(size);
        for (unsigned i = 0; i < size; i++) words[i] = u16(w, o + 2 * i);
        std::vector<uint16_t> padded = words;
        padded.resize(size + 4, 0);
        auto rots = split_rots(padded);
        std::array<Angles, 3> arm = {angles(rots[BONES[0]]), angles(rots[BONES[1]]), angles(rots[BONES[2]])};
        if (arm == arm_angles(NEW[k])) {
            done++;
            continue;
        }
        require(arm == arm_angles(OLD[k]), "pose " + std::to_string(k) + ": not the old arm");
        auto nr = as_rots(NEW[k]);
        rots[8] = nr[0], rots[9] = nr[1], rots[10] = nr[2];
        std::vector<uint16_t> out(words.begin(), words.begin() + 9);
        for (auto &r : rots) out.insert(out.end(), r.begin(), r.end());
        require(out.size() <= size, "pose " + std::to_string(k) + ": the new arm does not fit");
        out.resize(size, 0);
        for (unsigned i = 0; i < size; i++) put16(w, o + 2 * i, out[i]);
    }
    require(done == 0 || done == POSES, std::to_string(done));
    if (done) return std::nullopt;
    return w;
}

}  // namespace anim

namespace {
void run_fix_crouch_roll_arm(ToolContext &ctx, const std::vector<std::string> &args) {
    anim::require(!args.empty(), "usage: fix_crouch_roll_arm.py <level.tr2>");
    Bytes d = read_file(ctx.path(args[0]));
    auto out = anim::fix_crouch_roll_arm(d, anim::pc_sections(d));
    if (out) write_file(ctx.path(args[0]), *out);
    ctx.print(args[0] + ": crouch roll arm " + (out ? "replaced" : "already set"));
}
}  // namespace

REGISTER_TOOL("fix_crouch_roll_arm.py", run_fix_crouch_roll_arm);
