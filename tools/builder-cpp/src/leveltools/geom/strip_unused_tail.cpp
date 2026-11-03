// strip_unused_tail.py: removes data of a TR3 PC level nothing uses (unused meshes, trailing animations, frames and
// object textures); everything used keeps its index.
//     strip_unused_tail.py in.tr2 out.tr2
#include "geom.h"

namespace geom {

namespace {

void strip_unused_tail_tool(ToolContext &ctx, const std::vector<std::string> &args) {
    if (args.size() != 2) throw Error("usage: strip_unused_tail.py in.tr2 out.tr2");
    strip_unused_tail(ctx, args[0], args[1]);
}

}  // namespace

REGISTER_TOOL("strip_unused_tail.py", strip_unused_tail_tool);

}  // namespace geom
