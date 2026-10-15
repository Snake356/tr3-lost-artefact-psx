"""The animation block of a TR3 PC level (what troi exports: animations, state changes, dispatches,
commands, mesh trees, frames, moveables, static meshes) rewritten the way the project's levels/anims
files store it: every bone rotation in the two-word form.

TR3 PC packs a rotation about a single axis into one word (bits 14-15 = axis, 10-bit angle); the
project's files have all rotations as two words (x<<4 | y>>6, (y&63)<<10 | z), so each key frame is
9 + 2 * meshes words long. Each animation's key frames are converted up to the next animation's frames
(the files keep frames behind the last key frame), frame offsets and frame sizes are updated.
What then remains between this and the project's file is only the project's own additions."""
import struct

SECTIONS = [("anims", 32), ("statechanges", 6), ("dispatches", 8), ("commands", 2), ("meshtrees", 4),
            ("frames", 2), ("moveables", 18), ("statics", 32)]
ANIM = "<IBBHiiHHHHHHHH"           # frame offset, frame rate, frame size, state, ..., frame start, frame end, ...
MOVEABLE = "<IHHIIH"               # id, meshes, first mesh, mesh tree, frame offset, first animation


def parse(block):
    out, p = {}, 0
    for name, size in SECTIONS:
        n = struct.unpack_from("<I", block, p)[0]
        out[name] = block[p + 4:p + 4 + n * size]
        p += 4 + n * size
    assert p == len(block), "unexpected data behind the static meshes"
    return out


def rebuild(block):
    s = parse(block)
    frames = struct.unpack(f"<{len(s['frames']) // 2}H", s["frames"])
    anims = [list(struct.unpack_from(ANIM, s["anims"], i)) for i in range(0, len(s["anims"]), 32)]
    movs = [list(struct.unpack_from(MOVEABLE, s["moveables"], i)) for i in range(0, len(s["moveables"]), 18)]
    meshes = {}                                    # animation -> number of meshes of its moveable
    starts = sorted({(m[5], m[1]) for m in movs if m[5] < len(anims)})
    for i, (first, n) in enumerate(starts):
        end = starts[i + 1][0] if i + 1 < len(starts) else len(anims)
        for k in range(first, end):
            meshes[k] = n
    offsets = sorted({a[0] // 2 for a in anims})
    new, remap = [], {}
    for k, a in enumerate(anims):
        off = a[0] // 2
        if off in remap:
            continue
        later = [o for o in offsets if o > off]
        count = ((later[0] if later else len(frames)) - off) // a[2] if a[2] else 0
        remap[off] = len(new)
        for f in range(count):
            p = off + f * a[2]
            new.extend(frames[p:p + 9])
            p += 9
            for _ in range(meshes.get(k, 0)):
                w = frames[p]
                if w & 0xC000 == 0:
                    new.extend(frames[p:p + 2])
                    p += 2
                else:
                    v, axis = w & 0x3FF, w & 0xC000
                    x, y, z = (v, 0, 0) if axis == 0x4000 else (0, v, 0) if axis == 0x8000 else (0, 0, v)
                    new.extend(((x << 4) | (y >> 6), ((y & 0x3F) << 10) | z))
                    p += 1
    for k, a in enumerate(anims):
        a[0] = remap[a[0] // 2] * 2
        a[2] = 9 + 2 * meshes.get(k, 0)
    for m in movs:
        if m[4] // 2 in remap:
            m[4] = remap[m[4] // 2] * 2
    s["anims"] = b"".join(struct.pack(ANIM, *a) for a in anims)
    s["frames"] = struct.pack(f"<{len(new)}H", *new)
    s["moveables"] = b"".join(struct.pack(MOVEABLE, *m) for m in movs)
    return b"".join(struct.pack("<I", len(s[n]) // size) + s[n] for n, size in SECTIONS)
