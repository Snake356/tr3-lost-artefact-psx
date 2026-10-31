"""Bakes a point light into the room vertex colours of a TR3 PC level (the colour the PSX conversion
uses for room lighting). Every room vertex within the radius of a point gets
    colour += strength * (1 - distance / radius) * tint,   clamped to 31 per channel
(the nearest/strongest point counts when several overlap).
Costs nothing in the game (it is plain level lighting).

Usage:
    bake_point_light.py level.tr2 x,y,z [x,y,z ...] [--radius 3072] [--strength 10] [--tint 1.0,0.7,0.25] [--above y]
x, y, z in world units (as the item positions). --above: only vertices higher than this y (smaller y), so floors
and platforms below stay as they are. Writes the level in place."""
import argparse
import math
import struct


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("level")
    ap.add_argument("points", nargs="+")
    ap.add_argument("--radius", type=float, default=3072)
    ap.add_argument("--strength", type=float, default=10)
    ap.add_argument("--tint", default="1.0,0.7,0.25")
    ap.add_argument("--above", type=int, default=None)
    a = ap.parse_args()
    pts = [tuple(int(v) for v in p.split(",")) for p in a.points]
    tint = [float(v) for v in a.tint.split(",")]
    b = bytearray(open(a.level, "rb").read())
    u16 = lambda o: struct.unpack_from("<H", b, o)[0]
    i16 = lambda o: struct.unpack_from("<h", b, o)[0]
    u32 = lambda o: struct.unpack_from("<I", b, o)[0]
    i32 = lambda o: struct.unpack_from("<i", b, o)[0]
    p = 1804 + u32(1796) * 196608                 # after the TR3 palettes and texture tiles
    nrooms = u16(p)
    p += 2
    changed = 0
    for _ in range(nrooms):
        rx, rz = i32(p), i32(p + 4)
        p += 16
        ndw = u32(p)
        data = p + 4
        p = data + ndw * 2
        nv = i16(data)
        for k in range(nv):
            v = data + 2 + k * 12                     # x, y, z, lighting, attributes, colour (i16/u16)
            x, y, z = rx + i16(v), i16(v + 2), rz + i16(v + 4)
            if a.above is not None and y >= a.above:
                continue
            add = 0.0                                 # the strongest point counts (no sum: groups stay even)
            for px, py, pz in pts:
                d = math.dist((x, y, z), (px, py, pz))
                if d < a.radius:
                    add = max(add, a.strength * (1 - d / a.radius))
            if add <= 0:
                continue
            c = u16(v + 10)
            ch = [(c >> 10) & 31, (c >> 5) & 31, c & 31]
            ch = [min(31, int(round(ch[i] + add * tint[i]))) for i in range(3)]
            struct.pack_into("<H", b, v + 10, (c & 0x8000) | (ch[0] << 10) | (ch[1] << 5) | ch[2])
            changed += 1
        p += 2 + u16(p) * 32                          # portals
        nz, nx = u16(p), u16(p + 2)
        p += 4 + nz * nx * 8 + 4                      # sectors, ambient
        p += 2 + u16(p) * 24                          # lights
        p += 2 + u16(p) * 20 + 7                      # static meshes, alternate room, flags, water, reverb, filler
    open(a.level, "wb").write(b)
    print(f"{a.level}: {changed} room vertices brightened around {len(pts)} point(s)")


if __name__ == "__main__":
    main()
