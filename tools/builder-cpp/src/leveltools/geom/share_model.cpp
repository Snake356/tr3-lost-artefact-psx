// share_model.py: gives a pickup the model of its inventory object (which the level loads anyway) instead of its own
// mesh. Both objects must have one mesh; the inventory mesh must not be emptied.
//     share_model.py in.tr2 out.tr2 pickup_id:inventory_id [...]
#include <format>
#include <map>

#include "geom.h"

namespace geom {

namespace {

void share_model(ToolContext &ctx, const std::vector<std::string> &args) {
    if (args.size() < 3) throw Error("usage: share_model.py in.tr2 out.tr2 pickup_id:inventory_id [...]");
    std::vector<std::pair<int64_t, int64_t>> pairs;
    for (size_t i = 2; i < args.size(); i++) {
        auto parts = split(args[i], ':');
        if (parts.size() != 2) throw Error("a pair is pickup_id:inventory_id: " + args[i]);
        pairs.push_back({parse_int(parts[0]), parse_int(parts[1])});
    }
    Bytes b = read_file(ctx.path(args[0]));
    Analysis A = analyse(b);
    std::map<int64_t, Moveable> mv;
    for (auto &m : moveables(b, A.L)) mv[m.id] = m;
    size_t md = A.L.meshdata.data(), pp = A.L.meshptr.data();
    auto get = [&](int64_t id) -> const Moveable & {
        auto it = mv.find(id);
        if (it == mv.end()) throw Error("KeyError: " + std::to_string(id));
        return it->second;
    };
    for (auto &[pickup, inv] : pairs) {
        const Moveable &m1 = get(pickup), &m2 = get(inv);
        if (!(m1.nmeshes == 1 && m2.nmeshes == 1))
            throw Error(std::format("{}/{}: one mesh each expected", pickup, inv));
        if (A.ptr.at(m2.start) == A.ptr.at(0)) throw Error(std::format("inventory object {} is emptied", inv));
        MeshInfo mi = mesh_info(b, md + A.ptr[m2.start]);
        put32(b, pp + size_t(m1.start) * 4, A.ptr[m2.start]);
        std::set<int> tex(mi.tex.begin(), mi.tex.end());
        ctx.print(std::format("object {} uses the mesh of object {} ({} bytes, {} object textures)", pickup, inv, mi.size,
                              tex.size()));
    }
    write_file(ctx.path(args[1]), b);
}

}  // namespace

REGISTER_TOOL("share_model.py", share_model);

}  // namespace geom
