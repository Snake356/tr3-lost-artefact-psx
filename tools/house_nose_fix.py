"""Gives Lara's nose its texture in a finished TR3 PSX level (the original HOUSE.PSX), the same fix our own
levels get in the build (levels/textures, texmod, levels/mesh): two triangles of her head (mesh 14 of
LARA_SKIN, vertices 10, 13, 17 and 10, 18, 13) take a 5 x 4 texel crop that is meant for the eyebrows and
show a yellow band instead of the nose. As in the other levels:

  - the skin piece (art/lara/nose.bin: 40 x 32 PSX colours, red in the low bits, from the Lost Artefact
    levels patch) goes into an empty 40 x 32 area of a tile that no object, room or sprite texture uses, as
    4-bit texels with the head's CLUT that matches it best,
  - two object textures (three, see below) are appended after the last one, so no index shifts (inventory
    icons, crystal),
  - only the two triangles are pointed at them; the two beside the nose (vertices 17, 35, 16 and 15, 32, 18)
    have the same crop and get a third one, a small crop of plain skin in the piece, as in the patch.

The piece is first toned to the level's own skin (the Japanese discs' textures are lighter).

PSX object texture (16 bytes): u8 x0, y0; u16 CLUT; u8 x1, y1; u16 tile; u8 x2, y2; u8 flags x 2; u8 x3, y3;
u16. PSX triangles come in groups of four: a word with the low texture bytes of the four, then one word per
triangle with its three vertex indices and the high texture byte. The level data grows by 48 bytes, taken from
the padding at its end when it has room, else the level is padded to a multiple of 2048 again. A level that
has the fix already is left as it is.

Usage: house_nose_fix.py in.psx [out.psx]   (without out.psx it changes in.psx)"""
import os
import struct
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import psx_inventory  # noqa: E402

PRJ = os.path.join(os.path.dirname(os.path.abspath(__file__)), "..")
PIECE = os.path.join(PRJ, "art", "lara", "nose.bin")
LARA_SKIN, LARA, HEAD = 315, 0, 14
TRIANGLES = ((10, 13, 17), (10, 18, 13))
SKIN = (9, 0, 16)                                                   # a cheek triangle: the face's skin colour
SKIN_PC = (176, 128, 72)                                            # the same skin in the patch's level
BESIDE = ((17, 35, 16), (15, 32, 18))                               # beside the nose, the same crop: plain skin
# the piece's corners per triangle vertex (from the patch: x 176-215, y 40-71 of its tile)
CORNERS = (((39, 31), (39, 0), (0, 31)), ((39, 31), (0, 31), (39, 0)))
PLAIN = ((1, 1), (6, 1), (1, 5), (6, 5))                           # plain skin in the piece's top left corner
W, H = 40, 32


def u16(d, o): return struct.unpack_from("<H", d, o)[0]
def u32(d, o): return struct.unpack_from("<I", d, o)[0]


def rgb(c):
    return ((c & 31) << 3, (c >> 5 & 31) << 3, (c >> 10 & 31) << 3)


def sections(d):
    """Offsets of the parts the fix needs, walked as psx_inventory.py walks the file."""
    r = psx_inventory.Reader(d)
    z = {}
    r.i32()
    n = r.u32()
    r.skip(n * 4)
    if n:
        r.skip(r.u32())
    for _ in range(13):
        s = r.i32()
        if s:
            r.skip(s)
            r.skip(r.i32())
    rooms = []
    for _ in range(r.u16()):
        psx_inventory.read_room(r, rooms)
    r.skip(r.u32() * 2)
    r.skip(729 * 2)
    r.skip(r.u32())
    r.skip(r.i32() * 8)
    for name, size in (("meshdata", 2), ("meshptr", 4), ("anims", 32), ("statechg", 6), ("dispatch", 8),
                       ("animcmd", 2), ("meshtree", 4), ("frames", 2), ("models", 20), ("statics", 32),
                       ("tiles", 32768)):
        z["n_" + name] = n = r.u32()
        z[name] = r.o
        r.skip(n * size)
    ncl = r.u32()
    if ncl & 0xFFFF0000:                                            # Japanese files: 2 bytes before the count
        r.o -= 2
        ncl = r.u32()
    z["clut"] = r.o
    r.skip(ncl * 64)
    for name in ("objtex", "spritetex"):
        z["p_" + name] = r.o
        z["n_" + name] = n = r.u32()
        z[name] = r.o
        r.skip(n * 16)
    for size in (8, 16, 16):                                        # sprite sequences, cameras, sound sources
        r.skip(r.u32() * size)
    nb = r.u32()
    r.skip(nb * 8)
    r.skip(r.u32() * 2)                                             # overlaps
    r.skip(nb * 20)                                                 # zones
    r.skip(r.u32() * 2)                                             # animated textures
    r.skip(r.u32() * 24)                                            # entities
    r.skip(4)                                                       # horizon colour
    z["n_roomtex"] = n = r.u32()
    z["roomtex"] = r.o
    r.skip(n * 48)
    r.skip(370 * 2)                                                 # sound map
    r.skip(r.u32() * 8)                                             # sound details
    r.skip(4)                                                       # sample count
    r.skip(r.u16() * 16)                                            # cinematic frames
    r.skip(r.u16())                                                 # demo data
    z["end"] = r.o                                                  # then only padding
    return z


def head_triangles(d, z):
    """[(offset of the low texture byte, offset of the triangle word, vertices, texture)] of the head mesh."""
    mesh = None
    for oid in (LARA_SKIN, LARA):
        for i in range(z["n_models"]):
            o = z["models"] + i * 20
            if u32(d, o) == oid and mesh is None:
                mesh = u16(d, o + 6) + HEAD
    p = z["meshdata"] + u32(d, z["meshptr"] + mesh * 4)
    q = p + 12 + u16(d, p + 10)
    count = u16(d, q)
    q += 4
    out = []
    for t in range(count):
        if t % 4 == 0:
            low = q
            q += 4
        b = u32(d, q)
        out.append((low + t % 4, q, (b & 0xFF, (b >> 8) & 0xFF, (b >> 16) & 0xFF), d[low + t % 4] | (b >> 24) << 8))
        q += 4
    return out


def texel(d, z, tile, x, y):
    b = d[z["tiles"] + tile * 32768 + y * 128 + x // 2]
    return b >> 4 if x & 1 else b & 15


def free_area(d, z):
    """(tile, x, y) of an empty 40 x 32 area that no object, room or sprite texture touches."""
    used = [bytearray(65536) for _ in range(z["n_tiles"])]

    def mark(tile, xs, ys):
        if tile < z["n_tiles"]:
            for y in range(min(ys), max(ys) + 1):
                used[tile][y * 256 + min(xs):y * 256 + max(xs) + 1] = b"\1" * (max(xs) - min(xs) + 1)

    def entry(o):
        e = d[o:o + 16]
        pts = [(e[0], e[1]), (e[4], e[5]), (e[8], e[9]), (e[12], e[13])]
        if pts[3] == (0, 0) and (0, 0) not in pts[:3]:
            pts = pts[:3]
        mark(u16(d, o + 6), [p[0] for p in pts], [p[1] for p in pts])

    for i in range(z["n_objtex"]):
        entry(z["objtex"] + i * 16)
    for i in range(z["n_roomtex"]):
        for k in range(3):
            entry(z["roomtex"] + i * 48 + k * 16)
    for i in range(z["n_spritetex"]):
        o = z["spritetex"] + i * 16
        mark(u16(d, o + 10), [d[o + 12], d[o + 14]], [d[o + 13], d[o + 15]])
    for t in range(z["n_tiles"]):
        base = z["tiles"] + t * 32768
        for y in range(0, 257 - H, 8):
            for x in range(0, 257 - W, 8):
                if not any(any(used[t][(y + k) * 256 + x:(y + k) * 256 + x + W]) or
                           any(d[base + (y + k) * 128 + x // 2:base + (y + k) * 128 + (x + W) // 2])
                           for k in range(H)):
                    return t, x, y
    raise AssertionError("no empty 40 x 32 area")


def palette(d, z, clut):
    return [rgb(c) for c in struct.unpack_from("<16H", d, z["clut"] + clut * 32)]


def fix(d):
    """The fixed level, or None when it has the fix already."""
    z = sections(d)
    tris = head_triangles(d, z)
    sel = [next(t for t in tris if t[2] == v) for v in TRIANGLES]
    if all(t[3] >= z["n_objtex"] - 3 for t in sel):
        return None
    for t in sel:
        e = d[z["objtex"] + t[3] * 16:z["objtex"] + t[3] * 16 + 16]
        assert max(e[0], e[4], e[8]) - min(e[0], e[4], e[8]) < 8, "the triangles do not have the eyebrow crop"
    tile, x0, y0 = free_area(d, z)
    e = d[z["objtex"] + next(t for t in tris if t[2] == SKIN)[3] * 16:][:16]
    pal = palette(d, z, u16(e, 2))
    sample = [pal[texel(d, z, u16(e, 6), x, y)]
              for y in range(min(e[1], e[5], e[9]), max(e[1], e[5], e[9]) + 1)
              for x in range(min(e[0], e[4], e[8]), max(e[0], e[4], e[8]) + 1)]
    tone = [sum(c[k] for c in sample) / len(sample) / SKIN_PC[k] for k in range(3)]
    piece = [tuple(min(248.0, c[k] * tone[k]) for k in range(3))             # the Japanese discs' skin is lighter
             for c in (rgb(w) for w in struct.unpack(f"<{W * H}H", open(PIECE, "rb").read()))]
    best = None
    for clut in sorted({u16(d, z["objtex"] + t[3] * 16 + 2) for t in tris if t[3] >= 256}):
        pal = palette(d, z, clut)
        idx, err = [], 0.0
        for c in piece:
            dist = [sum((c[k] - p[k]) ** 2 for k in range(3)) for p in pal[1:]]   # index 0: see-through
            m = min(range(15), key=dist.__getitem__)
            idx.append(m + 1)
            err += dist[m] ** 0.5
        if best is None or err < best[0]:
            best = (err, clut, idx)
    err, clut, idx = best
    out = bytearray(d)
    for y in range(H):
        row = z["tiles"] + tile * 32768 + (y0 + y) * 128 + x0 // 2
        out[row:row + W // 2] = bytes(idx[y * W + 2 * k] | idx[y * W + 2 * k + 1] << 4 for k in range(W // 2))
    template = d[z["objtex"] + sel[0][3] * 16:z["objtex"] + sel[0][3] * 16 + 16]
    entries = b""
    for corners in CORNERS + (PLAIN,):
        e = bytearray(template)
        struct.pack_into("<H", e, 2, clut)
        struct.pack_into("<H", e, 6, tile)
        for k, (cx, cy) in enumerate(corners):
            e[4 * k], e[4 * k + 1] = x0 + cx, y0 + cy
        if len(corners) == 3:
            e[12], e[13] = x0 + corners[2][0], y0 + corners[2][1]
        entries += bytes(e)
    n = z["n_objtex"]
    for k, (low, word, _, _) in enumerate(sel):
        out[low] = (n + k) & 0xFF
        out[word + 3] = (n + k) >> 8
    for low, word, _, _ in (next(t for t in tris if t[2] == v) for v in BESIDE):
        out[low] = (n + 2) & 0xFF
        out[word + 3] = (n + 2) >> 8
    struct.pack_into("<I", out, z["p_objtex"], n + 3)
    end = z["objtex"] + n * 16
    out[end:end] = entries
    if len(d) - z["end"] >= len(entries) and not any(d[len(d) - len(entries):]):
        del out[len(out) - len(entries):]                           # the padding takes the 48 bytes
    else:
        out += bytes(-len(out) % 2048)
    print(f"tile {tile} at {x0}, {y0}, CLUT {clut} (error {err / len(piece):.1f}), object textures {n} to {n + 2}")
    return bytes(out)


def main(src, dst):
    d = open(src, "rb").read()
    out = fix(d)
    if out is None:
        print(f"{dst}: has the nose already")
        out = d
    open(dst, "wb").write(out)
    print(f"{dst}: {len(out) - len(d):+d} bytes")


if __name__ == "__main__":
    main(sys.argv[1], sys.argv[2] if len(sys.argv) > 2 else sys.argv[1])
