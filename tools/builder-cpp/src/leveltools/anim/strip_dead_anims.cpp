// strip_dead_anims.py: Lara's animations only a missing object starts take the poses of animation 11.
#include <algorithm>
#include <set>

#include "../tool.h"
#include "anim_tools.h"

namespace anim {
namespace {

constexpr unsigned DONOR = 11, RATE = 255;

struct Group {
    std::vector<int> objects;
    std::vector<unsigned> states, anims;
};
const Group GROUPS[] = {{{130}, {40}, {129}},
                        {{232, 233, 234, 235}, {42}, {131}},
                        {{87}, {}, {149}},
                        {{118}, {}, {195, 196}},
                        {{128}, {}, {197}},
                        {{127}, {70}, {214, 215, 216}},
                        {{97, 98, 99, 100}, {36, 37, 38}, {120, 121, 122, 123}},
                        {{213, 214, 215, 216}, {43}, {134}},
                        {{88, 89, 112, 126}, {}, {139}}};

// The animations and the states of the objects the level has no item of.
void dead(const Bytes &d, const Sections &s, std::vector<unsigned> &anims, std::set<unsigned> &states) {
    std::set<int> types;
    uint32_t n = count(d, s.items);
    for (uint32_t i = 0; i < n; i++) types.insert(s16(d, s.items + 4 + 24 * size_t(i)));
    require(types.count(0) != 0, "no Lara in the item list");
    for (auto &g : GROUPS) {
        bool present = std::any_of(g.objects.begin(), g.objects.end(), [&](int o) { return types.count(o) != 0; });
        if (!present) {
            anims.insert(anims.end(), g.anims.begin(), g.anims.end());
            states.insert(g.states.begin(), g.states.end());
        }
    }
    std::sort(anims.begin(), anims.end());
}

void check_unreached(const Bytes &d, const Sections &s, size_t stride, const std::vector<unsigned> &anims,
                     const std::set<unsigned> &states) {
    auto movs = read_moveables(d, s, stride);
    unsigned lara = second_first_anim(movs);
    require(!movs.empty() && movs[0].id == 0 && movs[0].anim == 0 && !anims.empty() && lara > anims.back(),
            "Lara has " + std::to_string(lara) + " animations");
    auto dead_anim = [&](unsigned a) { return std::binary_search(anims.begin(), anims.end(), a); };
    for (unsigned i = 0; i < lara; i++) {
        if (dead_anim(i)) continue;
        size_t a = anim_at(s, i);
        unsigned next = u16(d, a + 20);
        require(!dead_anim(next), "animation " + std::to_string(i) + " goes on with " + std::to_string(next));
        unsigned changes = u16(d, a + 24), first = u16(d, a + 26);
        for (unsigned c = 0; c < changes; c++) {
            size_t o = s.states + 4 + 6 * size_t(first + c);
            unsigned state = u16(d, o), ranges = u16(d, o + 2), r = u16(d, o + 4);
            for (unsigned k = 0; k < ranges; k++) {
                unsigned target = u16(d, s.dispatch + 4 + 8 * size_t(r + k) + 4);
                require(!dead_anim(target) || states.count(state) != 0,
                        "animation " + std::to_string(i) + " changes to " + std::to_string(target) + " (state " +
                            std::to_string(state) + ")");
            }
        }
    }
}

}  // namespace

std::optional<Bytes> strip_dead_anims(const Bytes &d, const Sections &s, size_t stride, StripReport *report) {
    std::vector<unsigned> anims;
    std::set<unsigned> states;
    dead(d, s, anims, states);
    if (report) report->anims = anims;
    AnimHead donor = read_anim_head(d, s, DONOR);
    require(donor.rate != 0, "animation 11 has frame rate 0");
    long long span = (long long)donor.frame_end - donor.frame_base;
    require(floordiv(span, donor.rate) + 1 + (floormod(span, donor.rate) != 0) >= 2, "animation 11 uses one pose");
    std::vector<unsigned> todo;
    for (unsigned i : anims) {
        AnimHead h = read_anim_head(d, s, i);
        if (!(h.frame_offset == donor.frame_offset && h.rate == RATE && h.frame_size == donor.frame_size))
            todo.push_back(i);
    }
    if (todo.empty()) return std::nullopt;
    check_unreached(d, s, stride, anims, states);
    Bytes w = d;
    for (unsigned i : todo) {
        size_t a = anim_at(s, i);
        int diff = int(u16(w, a + 18)) - int(u16(w, a + 16));
        require(0 <= diff && diff < int(RATE), "(" + std::to_string(i) + ", " + std::to_string(u16(w, a + 16)) + ", " +
                                                   std::to_string(u16(w, a + 18)) + ")");
        put32(w, a, donor.frame_offset);
        w[a + 4] = uint8_t(RATE);
        w[a + 5] = donor.frame_size;
    }
    return w;
}

}  // namespace anim

namespace {
void run_strip_dead_anims(ToolContext &ctx, const std::vector<std::string> &args) {
    anim::require(!args.empty(), "usage: strip_dead_anims.py <level.tr2>");
    Bytes d = read_file(ctx.path(args[0]));
    anim::StripReport report;
    auto out = anim::strip_dead_anims(d, anim::pc_sections(d, true), anim::PC_STRIDE, &report);
    if (out) write_file(ctx.path(args[0]), *out);
    std::string names;
    for (unsigned a : report.anims) names += (names.empty() ? "" : ", ") + std::to_string(a);
    if (names.empty()) names = "none";
    ctx.print(args[0] + ": animations " + names + " " + (out ? "freed" : "freed already"));
}
}  // namespace

REGISTER_TOOL("strip_dead_anims.py", run_strip_dead_anims);
