"""Replaces object textures that only differ from an existing entry by corner rotation
(as level editors create them for rotated faces) with that existing entry, rotating the
face's vertex order instead, then drops the now unused trailing entries.

Usage: fold_rotated_textures.py in.tr2 out.tr2 first_new_index
Only TR3 PC levels. Refuses to write if anything else still references a dropped entry.
"""
import struct
import sys

src, dst, FIRST_NEW = sys.argv[1], sys.argv[2], int(sys.argv[3])
b = bytearray(open(src, "rb").read())
p = 0


def u16(o): return struct.unpack_from("<H", b, o)[0]
def i16(o): return struct.unpack_from("<h", b, o)[0]
def u32(o): return struct.unpack_from("<I", b, o)[0]


# --- header / textures
num_tiles = u32(1796)
p = 1804 + num_tiles * 196608
num_rooms = u16(p); p += 2

faces = []  # (offset, kind 'r'/'t', room)
for room in range(num_rooms):
    p += 16
    ndw = u32(p); p += 4
    d = p
    nv = u16(d); d += 2 + nv * 12
    nr = u16(d); d += 2
    for k in range(nr):
        faces.append((d + k * 10, "r", room))
    d += nr * 10
    nt = u16(d); d += 2
    for k in range(nt):
        faces.append((d + k * 8, "t", room))
    d += nt * 8
    ns = u16(d); d += 2 + ns * 4
    assert d == p + ndw * 2, f"room {room}: data words mismatch"
    p += ndw * 2
    npo = u16(p); p += 2 + npo * 32
    nz, nx = u16(p), u16(p + 2); p += 4 + nz * nx * 8
    p += 4
    nl = u16(p); p += 2 + nl * 24
    nsm = u16(p); p += 2 + nsm * 20
    p += 4 + 3

n = u32(p); p += 4 + n * 2                     # floordata
mesh_data_start = p + 4
n = u32(p); p += 4 + n * 2                     # meshdata
mp_off = p + 4
n_mp = u32(p); p += 4 + n_mp * 4               # mesh pointers
mesh_ptrs = sorted(set(struct.unpack_from(f"<{n_mp}I", b, mp_off)))
for size in (32, 6, 8, 2, 4, 2, 18, 32, 16, 8, 16, 16):
    n = u32(p); p += 4 + n * size              # anims..soundsources
n_boxes = u32(p); p += 4 + n_boxes * 8
n = u32(p); p += 4 + n * 2                     # overlaps
p += n_boxes * 20                              # zones
anim_off = p
n = u32(p); p += 4 + n * 2                     # animated textures
objtex_off = p
n_obj = u32(p)
print(f"rooms={num_rooms} objtex@0x{objtex_off:X} n={n_obj}")

# --- mapping new -> (old, rotation)
def entry(i):
    o = objtex_off + 4 + i * 20
    return u16(o), u16(o + 2), [bytes(b[o + 4 + k * 4: o + 8 + k * 4]) for k in range(4)]

old = {}
for i in range(FIRST_NEW):
    a, t, c = entry(i)
    old.setdefault((a, t, frozenset(c)), []).append((i, c))
mapping = {}
for i in range(FIRST_NEW, n_obj):
    a, t, c = entry(i)
    for j, oc in old.get((a, t, frozenset(c)), []):
        rot = next((r for r in range(4) if [oc[(k + r) % 4] for k in range(4)] == c), None)
        if rot is not None:
            mapping[i] = (j, rot)
            break
    else:
        sys.exit(f"entry {i} has no rotated original -- aborting")

# --- rewrite room faces
changed = 0
for off, kind, room in faces:
    tex_off = off + (8 if kind == "r" else 6)
    tex = u16(tex_off)
    idx = tex & 0x7FFF
    if idx < FIRST_NEW:
        continue
    if kind == "t":
        sys.exit(f"room {room}: triangle uses rotated texture {idx} -- not handled")
    j, rot = mapping[idx]
    verts = [u16(off + k * 2) for k in range(4)]
    new = [verts[(m - rot) % 4] for m in range(4)]
    struct.pack_into("<4HH", b, off, *new, (tex & 0x8000) | j)
    changed += 1
print("room rectangles rewritten:", changed)

# --- nothing else may reference a dropped entry
for k in range(u32(anim_off)):
    v = u16(anim_off + 4 + k * 2)
    assert k == 0 or v < FIRST_NEW, "animated texture list references a dropped entry"
bad_mesh = 0
for mp in mesh_ptrs:
    o = mesh_data_start + mp
    o += 10
    nv = i16(o); o += 2 + nv * 6
    nn = i16(o); o += 2 + (nn * 6 if nn > 0 else -nn * 2)
    nr = i16(o); o += 2
    for k in range(nr):
        if (u16(o + k * 10 + 8) & 0x7FFF) >= FIRST_NEW:
            bad_mesh += 1
    o += nr * 10
    nt = i16(o); o += 2
    for k in range(nt):
        if (u16(o + k * 8 + 6) & 0x7FFF) >= FIRST_NEW:
            bad_mesh += 1
print("mesh faces still using dropped entries:", bad_mesh)
if bad_mesh:
    sys.exit("meshes reference dropped entries -- aborting")

# --- drop the trailing entries
cut_from = objtex_off + 4 + FIRST_NEW * 20
cut_to = objtex_off + 4 + n_obj * 20
del b[cut_from:cut_to]
struct.pack_into("<I", b, objtex_off, FIRST_NEW)
open(dst, "wb").write(b)
print(f"dropped {n_obj - FIRST_NEW} entries, wrote {dst} ({len(b)} bytes)")
