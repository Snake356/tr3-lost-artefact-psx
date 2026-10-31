"""The cages of Sleeping with the Fishes 2 (static 35, room 83, stacked in pairs). The PS1 has no z-buffer, so faces
that lie on each other are drawn in turn and flicker:
  - the model has two full bottom plates 1 unit apart that both face up (texture 1671 at y -2, 1670 at y -3): the
    outer one (1671, the cage's underside) is turned to face down
  - where a cage stands on another, the lower one's roof, inner ceiling and top wall band lie on the upper one's
    floor and bottom band: the lower cages get a model of their own (static 36, not used in the level, and its
    mesh slot) without roof and inner ceiling and with walls 5 units lower, ending where the upper cage starts
Writes the level in place (the copies the statics lines of convert.sh put into neighbour rooms are switched as well).
Usage: kaefig_fix.py underse2.tr2"""
import struct
import sys

import drop_object
import split_room as S

CAGE, LOWER = 35, 36
FLOOR_Y = -2304                              # the lower cages stand on the room's floor


def mesh_layout(b, start):
    """Offsets inside one mesh: vertices, quads (count, first)."""
    u = lambda f, o: struct.unpack_from(f, b, o)
    o = start + 10
    nv = u("<h", o)[0]
    verts = [u("<hhh", o + 2 + 6 * i) for i in range(nv)]
    vstart = o + 2
    o += 2 + 6 * nv
    nn = u("<h", o)[0]
    o += 2 + (6 * nn if nn > 0 else 2 * -nn)
    return verts, vstart, u("<h", o)[0], o + 2


def plates(b, verts, nq, quads):
    """(y, texture) -> quad number of the cage's full plates."""
    u = lambda f, o: struct.unpack_from(f, b, o)
    out = {}
    for k in range(nq):
        q = u("<5H", quads + 10 * k)
        ys = {verts[i][1] for i in q[:4]}
        if len(ys) == 1 and min(abs(verts[i][0]) for i in q[:4]) > 500:
            out[(ys.pop(), q[4] & 0x7FFF)] = k
    return out


def main():
    path = sys.argv[1]
    b = bytearray(open(path, "rb").read())
    s = drop_object.sections(b)
    u = lambda f, o: struct.unpack_from(f, b, o)
    info = s["statics"] + 4
    nst = u("<I", s["statics"])[0]
    entry = {u("<I", info + 32 * k)[0]: info + 32 * k for k in range(nst)}
    mesh, spare = u("<H", entry[CAGE] + 4)[0], u("<H", entry[LOWER] + 4)[0]
    ptrs = s["meshptr"] + 4
    base = s["mesh"] + 4
    start = base + u("<I", ptrs + 4 * mesh)[0]
    verts, vstart, nq, quads = mesh_layout(b, start)
    pl = plates(b, verts, nq, quads)

    # the outer bottom plate faces down (in every cage)
    k = pl[(-2, 1671)]
    q = u("<4H", quads + 10 * k)
    struct.pack_into("<4H", b, quads + 10 * k, q[1], q[0], q[3], q[2])

    # the lower cages' model: a copy without roof and inner ceiling, walls ending 5 units lower
    ends = sorted(u("<I", ptrs + 4 * i)[0] for i in range(u("<I", s["meshptr"])[0]))
    size = [e for e in ends if e > start - base][0] - (start - base)
    copy = bytearray(b[start:start + size])
    for key in ((-1031, 1671), (-1031, 1670)):
        o = quads - start + 10 * pl[key]
        v = struct.unpack_from("<H", copy, o)[0]
        struct.pack_into("<4H", copy, o, v, v, v, v)
    for i, (x, y, z) in enumerate(verts):
        if y == -1031:
            struct.pack_into("<hhh", copy, vstart - start + 6 * i, x, -1026, z)
    words = u("<I", s["mesh"])[0]
    end = base + 2 * words
    pad = (-(end - base)) % 4
    b[end:end] = bytes(pad) + copy
    struct.pack_into("<I", b, s["mesh"], words + (pad + len(copy)) // 2)
    s = drop_object.sections(b)                  # the pointer list lies behind the mesh data and has moved
    struct.pack_into("<I", b, s["meshptr"] + 4 + 4 * spare, end - base + pad)

    # static 36 gets static 35's boxes and flags, the lower cages use it
    info = s["statics"] + 4
    entry = {u("<I", info + 32 * k)[0]: info + 32 * k for k in range(u("<I", s["statics"])[0])}
    b[entry[LOWER] + 6:entry[LOWER] + 32] = b[entry[CAGE] + 6:entry[CAGE] + 32]
    _, rooms, rstart, rend = S.parse_rooms(bytes(b))
    switched = 0
    for r in rooms:
        for k, st in enumerate(r.statics):
            x, y, z, rot, i1, i2, sid = struct.unpack("<iiiHHHH", st)
            if sid == CAGE and y == FLOOR_Y:
                r.statics[k] = struct.pack("<iiiHHHH", x, y, z, rot, i1, i2, LOWER)
                switched += 1
    b = bytes(b[:rstart]) + S.build_rooms(rooms) + bytes(b[rend:])
    open(path, "wb").write(b)
    print(f"{path}: cage mesh {mesh} bottom plate turned down, lower cage model in mesh slot {spare} "
          f"({len(copy)} bytes), {switched} lower cages on static {LOWER}")


if __name__ == "__main__":
    main()
