"""The spine of the closed passport (object 158, the inventory ring) is its one coloured face; in the levels of The
Lost Artefact its palette entry is black, on the PlayStation disc (title, Lara's home) it is red. The face gets a
16-bit palette entry of its own (one that is empty and no coloured face uses) set to dark red; tr3pcpsx takes the
colour of a coloured face from the 16-bit palette (upper byte of its colour field). Writes the level in place.
Usage: passport_spine.py level.tr2"""
import struct
import sys

import drop_object

PASSPORT_CLOSED = 158
RED = (128, 24, 24)
PAL16 = 4 + 768


def faces(b, s):
    """Offsets of the colour fields of all coloured faces, per moveable mesh."""
    u = lambda f, o: struct.unpack_from(f, b, o)
    base, ptrs = s["mesh"] + 4, s["meshptr"] + 4
    out = {}
    for m in range(u("<I", s["meshptr"])[0]):
        o = base + u("<I", ptrs + 4 * m)[0] + 10
        nv = u("<h", o)[0]
        o += 2 + 6 * nv
        nn = u("<h", o)[0]
        o += 2 + (6 * nn if nn > 0 else 2 * -nn)
        fields = []
        for size, idx, coloured in ((10, 4, False), (8, 3, False), (10, 4, True), (8, 3, True)):
            n = u("<h", o)[0]
            o += 2
            if coloured:
                fields += [o + size * k + 2 * idx for k in range(n)]
            o += size * n
        out[m] = fields
    return out


def main():
    path = sys.argv[1]
    b = bytearray(open(path, "rb").read())
    s = drop_object.sections(b)
    u = lambda f, o: struct.unpack_from(f, b, o)
    mov = {u("<I", s["moveables"] + 4 + 18 * k)[0]: u("<HH", s["moveables"] + 8 + 18 * k)
           for k in range(u("<I", s["moveables"])[0])}
    nmesh, start = mov[PASSPORT_CLOSED]
    all_faces = faces(b, s)
    used = {u("<H", f)[0] >> 8 for fs in all_faces.values() for f in fs}
    spine = [f for m in range(start, start + nmesh) for f in all_faces[m]]
    assert len(spine) == 1, f"{len(spine)} coloured faces in the passport"
    old = u("<H", spine[0])[0]
    assert not any(b[PAL16 + 4 * (old >> 8):PAL16 + 4 * (old >> 8) + 3]), "the spine is not black"
    free = [i for i in range(1, 256) if i not in used and not any(b[PAL16 + 4 * i:PAL16 + 4 * i + 4])]
    assert free, "no free 16-bit palette entry"
    k = free[0]
    b[PAL16 + 4 * k:PAL16 + 4 * k + 3] = bytes(RED)
    struct.pack_into("<H", b, spine[0], k << 8 | old & 0xFF)
    open(path, "wb").write(b)
    print(f"{path}: passport spine {old >> 8} -> 16-bit palette entry {k} {RED}")


if __name__ == "__main__":
    main()
