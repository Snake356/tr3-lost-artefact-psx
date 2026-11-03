// restore_model.py: puts back the meshes of objects the mesh optimisation emptied (they point to mesh 0), copied
// from the unprocessed level, which must use the same object textures for them (checked).
//     restore_model.py donor.tr2 in.tr2 out.tr2 object_id [object_id ...]
#include <format>
#include <map>

#include "geom.h"

namespace geom {

namespace {

std::map<int64_t, Moveable> moveable_map(const Bytes &b, const Layout &L) {
    std::map<int64_t, Moveable> mv;
    for (auto &m : moveables(b, L)) mv[m.id] = m;
    return mv;
}

const Moveable &get(const std::map<int64_t, Moveable> &mv, int64_t id) {
    auto it = mv.find(id);
    if (it == mv.end()) throw Error("KeyError: " + std::to_string(id));
    return it->second;
}

Bytes clamped(const Bytes &b, size_t a, size_t n) {     // b[a:a + n] as Python slices it
    a = std::min(a, b.size());
    return Bytes(b.begin() + ptrdiff_t(a), b.begin() + ptrdiff_t(std::min(a + n, b.size())));
}

void restore_model(ToolContext &ctx, const std::vector<std::string> &args) {
    if (args.size() < 4) throw Error("usage: restore_model.py donor.tr2 in.tr2 out.tr2 object_id [object_id ...]");
    std::vector<int64_t> oids;
    for (size_t i = 3; i < args.size(); i++) oids.push_back(parse_int(args[i]));
    Bytes d = read_file(ctx.path(args[0]));
    Bytes b = read_file(ctx.path(args[1]));
    Analysis Ad = analyse(d);
    Analysis A = analyse(b);
    std::vector<uint32_t> &ptr = A.ptr;
    auto mvd = moveable_map(d, Ad.L), mv = moveable_map(b, A.L);
    size_t mdd = Ad.L.meshdata.data(), md = A.L.meshdata.data();
    size_t otd = Ad.L.objtex.data(), ot = A.L.objtex.data();
    Bytes add_md;
    size_t size_md = size_t(A.L.meshdata.count) * 2;
    for (int64_t oid : oids) {
        const Moveable &m = get(mv, oid), &mdn = get(mvd, oid);
        if (m.nmeshes != mdn.nmeshes)
            throw Error(std::format("object {}: {} meshes here, {} in the donor", oid, m.nmeshes, mdn.nmeshes));
        for (size_t k = 0; k < m.nmeshes; k++) {
            if (ptr.at(m.start + k) != ptr.at(0)) throw Error(std::format("object {} mesh {} is not emptied - not changed", oid, k));
            uint32_t src = Ad.ptr.at(mdn.start + k);
            MeshInfo mi = mesh_info(d, mdd + src);
            for (int t : mi.tex)
                if (clamped(d, otd + size_t(t) * 20, 20) != clamped(b, ot + size_t(t) * 20, 20))
                    throw Error(std::format("object texture {} differs from the donor", t));
            ptr[m.start + k] = uint32_t(size_md + add_md.size());
            Writer{add_md}.bytes(clamped(d, mdd + src, mi.size));
            if (add_md.size() % 4) add_md.resize(add_md.size() + 4 - add_md.size() % 4);
            ctx.print(std::format("object {} mesh {}: {} bytes restored", oid, k, mi.size));
        }
    }
    size_t pp = A.L.meshptr.data();
    for (size_t i = 0; i < ptr.size(); i++) put32(b, pp + i * 4, ptr[i]);
    size_t mdc = A.L.meshdata.off;
    size_t end = md + size_md;
    Bytes out = clamped(b, 0, mdc);
    Writer{out}.u32(uint32_t((size_md + add_md.size()) / 2));
    Writer{out}.bytes(clamped(b, md, end - md));
    Writer{out}.bytes(add_md);
    Writer{out}.bytes(clamped(b, end, b.size()));
    write_file(ctx.path(args[2]), out);
    analyse(out);                                   // parses again = layout still consistent
    ctx.print("written " + args[2]);
}

}  // namespace

REGISTER_TOOL("restore_model.py", restore_model);

}  // namespace geom
