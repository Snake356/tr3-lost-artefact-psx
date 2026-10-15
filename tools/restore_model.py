"""Puts back the meshes of objects that the mesh optimisation (levels/mesh/<level>.txt, tr3mesh) emptied
because the object was not used in the level - an emptied mesh points to mesh 0 (Lara's hips), so a
pickup placed later shows Lara's leg. The meshes are copied from the unprocessed level in levels/in,
which must use the same object textures for them (checked).

Usage: restore_model.py donor.tr2 in.tr2 out.tr2 object_id [object_id ...]
e.g.   restore_model.py levels/in/scotland.tr2 scotland_japan.tr2 out.tr2 167   (grenade launcher)
"""
import struct
import sys

import strip_unused_tail as T


def moveables(b, L):
    return {m[0]: m for m in (struct.unpack_from("<IHHIIH", b, L["moveables"][0] + 4 + i * 18)
                              for i in range(L["moveables"][1]))}


def main(donor, src, dst, oids):
    d = open(donor, "rb").read()
    b = bytearray(open(src, "rb").read())
    Ld, pd, *_ = T.analyse(d)
    L, ptr, *_ = T.analyse(b)
    mvd, mv = moveables(d, Ld), moveables(b, L)
    mdd, md = Ld["meshdata"][0] + 4, L["meshdata"][0] + 4
    otd, ot = Ld["objtex"][0] + 4, L["objtex"][0] + 4
    new = bytearray()
    size_md = L["meshdata"][1] * 2
    for oid in oids:
        _, nm, sm, *_ = mv[oid]
        _, nmd, smd, *_ = mvd[oid]
        assert nm == nmd, f"object {oid}: {nm} meshes here, {nmd} in the donor"
        for k in range(nm):
            if ptr[sm + k] != ptr[0]:
                sys.exit(f"object {oid} mesh {k} is not emptied - not changed")
            size, tex = T.mesh_info(d, mdd + pd[smd + k])
            for t in tex:
                assert d[otd + t * 20:otd + t * 20 + 20] == b[ot + t * 20:ot + t * 20 + 20], \
                    f"object texture {t} differs from the donor"
            ptr[sm + k] = size_md + len(new)
            new += d[mdd + pd[smd + k]:mdd + pd[smd + k] + size]
            if len(new) % 4:
                new += bytes(4 - len(new) % 4)
            print(f"object {oid} mesh {k}: {size} bytes restored")
    pp = L["meshptr"][0] + 4
    for i, p in enumerate(ptr):
        struct.pack_into("<I", b, pp + i * 4, p)
    mdc = L["meshdata"][0]
    end = md + size_md
    out = b[:mdc] + struct.pack("<I", (size_md + len(new)) // 2) + b[md:end] + new + b[end:]
    open(dst, "wb").write(out)
    L2, ptr2, *_ = T.analyse(bytes(out))    # parses again = layout still consistent
    print(f"written {dst}")


if __name__ == "__main__":
    if len(sys.argv) < 5:
        sys.exit(__doc__)
    main(sys.argv[1], sys.argv[2], sys.argv[3], [int(x) for x in sys.argv[4:]])
