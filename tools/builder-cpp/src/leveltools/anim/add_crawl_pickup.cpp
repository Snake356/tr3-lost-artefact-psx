// add_crawl_pickup.py: the smooth pickup in a crawlspace (animation 292 gets 14 poses at frame rate 4).
#include "../tool.h"
#include "anim_data.h"
#include "anim_tools.h"

namespace anim {

std::optional<Bytes> add_crawl_pickup(const Bytes &d, const Sections &s, size_t /*stride*/) {
    constexpr unsigned INDEX = CRAWL_PICKUP_INDEX, FRAME_SIZE = 37, FRAME_RATE = 4;
    uint32_t nf = count(d, s.frames);
    size_t a = anim_at(s, INDEX);
    check_range(d, a, 20);
    if (d[a + 4] == FRAME_RATE) return std::nullopt;
    unsigned state = u16(d, a + 6), start = u16(d, a + 16), end = u16(d, a + 18);
    require(state == 39 && int(end) - int(start) == 55,
            "animation " + std::to_string(INDEX) + " is not the crawl pickup");
    Bytes w = d;
    pack_u32(w, a, 2LL * nf);
    w[a + 4] = FRAME_RATE;
    w[a + 5] = FRAME_SIZE;
    Bytes frames(std::begin(data::CRAWL_PICKUP_DATA), std::end(data::CRAWL_PICKUP_DATA));
    frames.insert(frames.end(), std::end(data::CRAWL_PICKUP_DATA) - 2 * FRAME_SIZE, std::end(data::CRAWL_PICKUP_DATA));
    size_t f_end = s.frames + 4 + 2 * size_t(nf);
    Bytes out;
    out.reserve(w.size() + frames.size());
    append_slice(out, w, 0, s.frames);
    append_u32(out, nf + frames.size() / 2);
    append_slice(out, w, s.frames + 4, f_end);
    out.insert(out.end(), frames.begin(), frames.end());
    append_slice(out, w, f_end, w.size());
    return out;
}

}  // namespace anim

namespace {
void run_add_crawl_pickup(ToolContext &ctx, const std::vector<std::string> &args) {
    anim::require(!args.empty(), "usage: add_crawl_pickup.py <level.tr2>");
    Bytes d = read_file(ctx.path(args[0]));
    auto out = anim::add_crawl_pickup(d, anim::pc_sections(d));
    if (!out) {
        ctx.print(args[0] + ": has the smooth crawl pickup already");
    } else {
        write_file(ctx.path(args[0]), *out);
        ctx.print(args[0] + ": crawl pickup smoothed (animation " + std::to_string(anim::CRAWL_PICKUP_INDEX) + ")");
    }
}
}  // namespace

REGISTER_TOOL("add_crawl_pickup.py", run_add_crawl_pickup);
