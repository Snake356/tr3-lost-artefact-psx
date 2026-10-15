"""Checks that every room face maps each vertex to the same texture corner (tile+pixels)
in two TR3 PC levels, and that everything outside the objtex table is unchanged."""
import struct
import sys


def load(path):
    b = open(path, "rb").read()
    u16 = lambda o: struct.unpack_from("<H", b, o)[0]
    u32 = lambda o: struct.unpack_from("<I", b, o)[0]
    p = 1804 + u32(1796) * 196608
    nrooms = u16(p); p += 2
    faces = []
    for room in range(nrooms):
        p += 16
        ndw = u32(p); p += 4
        d = p
        nv = u16(d); d += 2 + nv * 12
        for kind, cnt_size, nvert in (("r", 10, 4), ("t", 8, 3)):
            n = u16(d); d += 2
            for k in range(n):
                o = d + k * cnt_size
                faces.append((room, kind, k, [u16(o + i * 2) for i in range(nvert)], u16(o + nvert * 2)))
            d += n * cnt_size
        p += ndw * 2
        npo = u16(p); p += 2 + npo * 32
        nz, nx = u16(p), u16(p + 2); p += 4 + nz * nx * 8 + 4
        nl = u16(p); p += 2 + nl * 24
        ns = u16(p); p += 2 + ns * 20 + 7
    return b, faces


def objtex_table(b, off):
    n = struct.unpack_from("<I", b, off)[0]
    return [(b[off + 4 + i * 20 + 2: off + 4 + i * 20 + 4],
             [b[off + 8 + i * 20 + k * 4: off + 12 + i * 20 + k * 4] for k in range(4)]) for i in range(n)]


a_path, b_path, off = sys.argv[1], sys.argv[2], int(sys.argv[3], 0)
ba, fa = load(a_path)
bb, fb = load(b_path)
ta, tb = objtex_table(ba, off), objtex_table(bb, off)
assert len(fa) == len(fb)
bad = changed = 0
for (r, k, i, va, xa), (_, _, _, vb, xb) in zip(fa, fb):
    ea, eb = ta[xa & 0x7FFF], tb[xb & 0x7FFF]
    pa = sorted((v, ea[0], ea[1][n]) for n, v in enumerate(va))
    pb = sorted((v, eb[0], eb[1][n]) for n, v in enumerate(vb))
    if (va, xa) != (vb, xb):
        changed += 1
    if pa != pb or (xa & 0x8000) != (xb & 0x8000):
        bad += 1
        print("MISMATCH room", r, k, i)
print(f"faces={len(fa)} changed={changed} mismatches={bad}")
end_a = off + 4 + len(ta) * 20
end_b = off + 4 + len(tb) * 20
print("tail after objtex identical:", ba[end_a:] == bb[end_b:])
