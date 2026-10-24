"""Removes the meshes of an object a Tomb Raider III PC level carries but never uses, to save level memory
on the PS1. Only for objects no code refers to either: the executable may use one the level never places
(e.g. 315, LARA_SKIN, Lara's real appearance; it was taken for an unused one once and must stay).

Only when the object is not placed in the level and its meshes belong to it alone (no other moveable,
no static mesh). Its mesh pointers then point to mesh 0, as the meshes the mesh optimisation empties,
its mesh data is cut out and the mesh pointers behind it move down. Its animation stays. A level without
the object, or with it placed, is left as it is.

Usage: drop_object.py <level.tr2> <object id> [...]   (changes the file in place)"""
import struct
import sys


def sections(d):
    """Offsets of the count fields of the lists between the rooms and the items (TR3 PC level)."""
    u = lambda f, o: struct.unpack_from(f, d, o)
    o = 4 + 768 + 1024
    n = u("<I", o)[0]
    o += 4 + n * 65536 + n * 131072 + 4
    rooms = u("<H", o)[0]
    o += 2
    for _ in range(rooms):
        o += 16
        o += 4 + 2 * u("<I", o)[0]
        o += 2 + 32 * u("<H", o)[0]
        zs, xs = u("<HH", o)
        o += 4 + 8 * zs * xs + 4
        o += 2 + 24 * u("<H", o)[0]
        o += 2 + 20 * u("<H", o)[0]
        o += 7
    s = {}
    for name, size in (("floor", 2), ("mesh", 2), ("meshptr", 4), ("anims", 32), ("states", 6),
                       ("dispatch", 8), ("commands", 2), ("trees", 4), ("frames", 2), ("moveables", 18),
                       ("statics", 32), ("spritetex", 16), ("spriteseq", 8), ("cameras", 16), ("sources", 16)):
        s[name] = o
        o += 4 + size * u("<I", o)[0]
    boxes = u("<I", o)[0]
    o += 4 + 8 * boxes
    o += 4 + 2 * u("<I", o)[0]                                   # overlaps
    o += 20 * boxes                                              # zones
    o += 4 + 2 * u("<I", o)[0]                                   # animated textures
    o += 4 + 20 * u("<I", o)[0]                                  # object textures
    s["items"] = o
    return s


def drop(d, oid):
    s = sections(d)
    u = lambda f, o: struct.unpack_from(f, d, o)
    count = lambda k: u("<I", s[k])[0]
    movs = [u("<IHHIIH", s["moveables"] + 4 + 18 * i) for i in range(count("moveables"))]
    mine = [m for m in movs if m[0] == oid]
    if not mine:
        return None, "not in the level"
    items = {u("<h", s["items"] + 4 + 24 * i)[0] for i in range(count("items"))}
    if oid in items:
        return None, "placed in the level"
    _, nmesh, start, *_ = mine[0]
    ptrs = list(u(f"<{count('meshptr')}I", s["meshptr"] + 4))
    own = set(range(start, start + nmesh))
    used_elsewhere = {i for m in movs if m[0] != oid for i in range(m[2], m[2] + m[1])}
    used_elsewhere |= {u("<H", s["statics"] + 4 + 32 * i + 4)[0] for i in range(count("statics"))}
    other = {ptrs[i] for i in range(len(ptrs)) if i not in own or i in used_elsewhere}
    cut = sorted({ptrs[i] for i in own} - other - {0})
    if not cut:
        return None, "no meshes of its own"
    ends = sorted(set(ptrs) | {2 * count("mesh")})
    ranges = [(p, ends[ends.index(p) + 1]) for p in cut]
    removed = sum(b - a for a, b in ranges)
    shift = lambda p: p - sum(b - a for a, b in ranges if b <= p)
    new_ptrs = [0 if i in own else shift(p) for i, p in enumerate(ptrs)]
    md = s["mesh"] + 4
    data = d[md:md + 2 * count("mesh")]
    kept = b"".join(data[a:b] for a, b in zip([0] + [b for _, b in ranges], [a for a, _ in ranges] + [len(data)]))
    out = (d[:s["mesh"]] + struct.pack("<I", len(kept) // 2) + kept
           + struct.pack("<I", len(new_ptrs)) + struct.pack(f"<{len(new_ptrs)}I", *new_ptrs)
           + d[s["meshptr"] + 4 + 4 * len(ptrs):])
    return out, f"{len(ranges)} meshes, {removed} bytes removed"


if __name__ == "__main__":
    path = sys.argv[1]
    d = open(path, "rb").read()
    for oid in map(int, sys.argv[2:]):
        out, what = drop(d, oid)
        if out is not None:
            d = out
            s = sections(d)                                      # read again: every pointer inside the data
            n = struct.unpack_from("<I", d, s["meshptr"])[0]
            assert max(struct.unpack_from(f"<{n}I", d, s["meshptr"] + 4)) < 2 * struct.unpack_from("<I", d, s["mesh"])[0]
        print(f"{path}: object {oid}: {what}")
    open(path, "wb").write(d)
