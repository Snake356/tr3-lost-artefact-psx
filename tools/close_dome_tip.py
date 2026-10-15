"""Closes the tip of the sky dome in a finished TR3 PSX level (e.g. the original HOUSE.PSX).

The PSX sky object (HORIZON) has no triangles at all (tr3pcpsx drops them, and so do Core's
files), so the 16 triangles around the apex of the dome are missing and the background shows:
dark on the US executables, light cyan on the Japanese ones. For our own levels the apex
triangles are rectangles in levels/mesh; this tool adds the same 16 rectangles (i, 0, i+1, i+1)
with the cap colour directly to a PSX file.

PSX sky mesh: u8 NumVertices, u8 0, u16 NumRectangles, NumVertices x 8 bytes, then the
rectangles in pairs of 12 bytes (u16 texture word x 2, 4 + 4 vertex indices; PC order a,b,c,d is
stored a,b,d,c), coloured ones first; an odd last rectangle takes 8 bytes. The mesh data grows by
96 bytes; the mesh data word count and all mesh pointers behind the dome are updated.

Usage: close_dome_tip.py in.psx out.psx"""
import os
import pathlib
import struct
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import psx_inventory  # noqa: E402

src, dst = sys.argv[1], sys.argv[2]
z = psx_inventory.inventory(pathlib.Path(src))[0]
assert z["status"] == "ok", z["status"]
d = bytearray(open(src, "rb").read())
u32 = lambda o: struct.unpack_from("<I", d, o)[0]

# mesh data block: u32 words, data; u32 pointers, data; u32 anims
nmd, nmp, na = z["meshdata"], z["meshptr"], z["anims"]
hits = [o for o in range(0, len(d) - 12)
        if u32(o) == nmd and o + 8 + nmd * 2 + nmp * 4 < len(d)
        and u32(o + 4 + nmd * 2) == nmp and u32(o + 8 + nmd * 2 + nmp * 4) == na]
assert len(hits) == 1, hits
pmd = hits[0]
md = pmd + 4
pptr = md + nmd * 2 + 4
ptrs = [u32(pptr + 4 * i) for i in range(nmp)]

# sky mesh: 66 vertices, vertex 0 on the y axis at the top, only rectangles
sky = []
for off in sorted(set(ptrs)):
    m = md + off
    nv, zero, nr = d[m], d[m + 1], struct.unpack_from("<H", d, m + 2)[0]
    if nv != 66 or zero != 0:
        continue
    x0, y0, z0 = struct.unpack_from("<hhh", d, m + 4)
    ys = [struct.unpack_from("<h", d, m + 4 + 8 * i + 2)[0] for i in range(nv)]
    if x0 == 0 and z0 == 0 and y0 == min(ys):
        sky.append((off, nr))
assert len(sky) == 1, sky
off, nr = sky[0]
m = md + off
verts = m + 4
rstart = verts + 66 * 8

# read rectangles
rects = []
p = rstart
for i in range(0, nr, 2):
    tw = struct.unpack_from("<HH", d, p)
    if i + 1 < nr:
        rects.append((tw[0], tuple(d[p + 4:p + 8])))
        rects.append((tw[1], tuple(d[p + 8:p + 12])))
        p += 12
    else:
        rects.append((tw[0], tuple(d[p + 4:p + 8])))
        p += 8
old_len = p - m
coloured = [r for r in rects if r[0] < 0x100]
apex_before = [r for r in rects if 0 in r[1]]
assert not apex_before, "the tip is already closed"
# cap colour: coloured rectangle touching the first ring (vertices 1..16)
cap = [r[0] for r in coloured if any(1 <= v <= 16 for v in r[1])]
assert cap and len(set(cap)) == 1, cap
col = cap[0]
new = [(col, (i, 0, (i % 16) + 1, (i % 16) + 1)) for i in range(1, 17)]  # stored order a,b,d,c
rects = coloured + new + [r for r in rects if r[0] >= 0x100]
nr2 = len(rects)

# pack
out = bytearray(struct.pack("<BBH", 66, 0, nr2)) + d[verts:rstart]
for i in range(0, nr2, 2):
    if i + 1 < nr2:
        out += struct.pack("<HH", rects[i][0], rects[i + 1][0]) + bytes(rects[i][1]) + bytes(rects[i + 1][1])
    else:
        out += struct.pack("<HH", rects[i][0], 0) + bytes(rects[i][1])
delta = len(out) - old_len
assert delta == 96, delta

# splice and fix counts / pointers
d[m:m + old_len] = out
struct.pack_into("<I", d, pmd, nmd + delta // 2)
pptr += delta
for i in range(nmp):
    v = u32(pptr + 4 * i)
    if v > off:
        struct.pack_into("<I", d, pptr + 4 * i, v + delta)
open(dst, "wb").write(d)
z2 = psx_inventory.inventory(pathlib.Path(dst))[0]
assert z2["status"] == "ok" and z2["meshdata"] == nmd + 48, z2["status"]
print(f"{src}: Himmel-Mesh @+{off:#x}, {nr} -> {nr2} Rechtecke, Kappenfarbe {col}, Mesh-Daten +{delta} Bytes, "
      f"{sum(1 for v in ptrs if v > off)} Zeiger verschoben")
