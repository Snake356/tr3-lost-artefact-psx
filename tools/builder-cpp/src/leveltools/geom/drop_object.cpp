// drop_object.py: removes the meshes of objects a TR3 PC level carries but never places and no other object or
// static mesh shares. Changes the file in place.
//     drop_object.py <level.tr2> <object id> [...]
#include <algorithm>
#include <format>

#include "geom.h"

namespace geom {

namespace {

void drop_object_tool(ToolContext &ctx, const std::vector<std::string> &args) {
    if (args.empty()) throw Error("usage: drop_object.py <level.tr2> <object id> [...]");
    const std::string &path = args[0];
    Bytes d = read_file(ctx.path(path));
    for (size_t i = 1; i < args.size(); i++) {
        int64_t oid = parse_int(args[i]);
        std::string what;
        std::optional<Bytes> out = drop(d, uint32_t(oid), what);
        if (out) {
            d = std::move(*out);
            Sections s = sections(d);          // read again: every pointer inside the data
            uint32_t n = u32(d, s.meshptr), mx = 0;
            for (uint32_t k = 0; k < n; k++) mx = std::max(mx, u32(d, s.meshptr + 4 + 4 * size_t(k)));
            if (n == 0 || !(mx < 2 * u32(d, s.mesh))) throw Error("a mesh pointer lies outside the mesh data");
        }
        ctx.print(std::format("{}: object {}: {}", path, oid, what));
    }
    write_file(ctx.path(path), d);
}

}  // namespace

REGISTER_TOOL("drop_object.py", drop_object_tool);

}  // namespace geom
