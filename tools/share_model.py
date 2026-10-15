"""Gives a pickup the model of its inventory object (which the level loads anyway) instead of its own
mesh, so it costs no memory: the mesh optimisation (levels/mesh/<level>.txt) empties pickup models a
level does not use - an emptied mesh points to mesh 0 (Lara's hips), so a pickup placed later showed
Lara's leg - and putting the real mesh back made Highland Fling (Japan) too big to load.

The inventory model is the same weapon with (nearly) the same textures and lies the same way.
Both objects must have one mesh; the inventory mesh must not be emptied.

Usage: share_model.py in.tr2 out.tr2 pickup_id:inventory_id [...]
e.g.   share_model.py scotland_jp.tr2 scotland_jp.tr2 167:192   (grenade launcher)
"""
import struct
import sys

import strip_unused_tail as T


def main(src, dst, pairs):
    b = bytearray(open(src, "rb").read())
    L, ptr, *_ = T.analyse(b)
    mv = {m[0]: m for m in (struct.unpack_from("<IHHIIH", b, L["moveables"][0] + 4 + i * 18)
                            for i in range(L["moveables"][1]))}
    md = L["meshdata"][0] + 4
    pp = L["meshptr"][0] + 4
    for pickup, inv in pairs:
        _, n1, s1, *_ = mv[pickup]
        _, n2, s2, *_ = mv[inv]
        assert n1 == 1 and n2 == 1, f"{pickup}/{inv}: one mesh each expected"
        assert ptr[s2] != ptr[0], f"inventory object {inv} is emptied"
        size, tex = T.mesh_info(b, md + ptr[s2])
        struct.pack_into("<I", b, pp + s1 * 4, ptr[s2])
        print(f"object {pickup} uses the mesh of object {inv} ({size} bytes, {len(set(tex))} object textures)")
    open(dst, "wb").write(b)


if __name__ == "__main__":
    if len(sys.argv) < 4:
        sys.exit(__doc__)
    main(sys.argv[1], sys.argv[2], [tuple(int(x) for x in a.split(":")) for a in sys.argv[3:]])
