"""Frees level memory on the PS1 by storing the bone rotations of the poses the way Core's own levels do: a rotation
about a single axis in one word (bits 14-15 = axis 1 x, 2 y, 3 z, 10-bit angle; mRotSuperPackedYXZ reads both
forms) instead of two, and every animation with its own frame size (the longest of its poses). The project's
animation files (levels/anims, tools/builder/animframes.py) keep every rotation in the two-word form and every
frame size at 9 + 2 * meshes words; this tool runs at the end of the animation chain, behind compact_frames.py.

Only rotations with two angles exactly 0 are packed, so every pose decodes to the same bounding box, offset and
angles as before. The poses keep their order, so the pose behind an animation's last key frame (which the game
blends towards in its last frames) stays directly behind it. Animations sharing poses get a common frame size.
The weapon animation objects (1 to 10) and the inventory objects get one frame size per object: the game finds
their poses from the object's first pose (frame number times the frame size of the current animation).
Frame offsets and frame sizes of the animations and the moveables' frame offsets are updated; nothing else
changes. Every pose is checked to decode the same afterwards. A level that is packed already is left as it is.

Usage: pack_rotations.py <level.tr2>   (changes the file in place)"""
import struct
import sys

from add_crawl_flip import sections
from compact_frames import read

UNIFORM = set(range(1, 11)) | {145, 146, 147, 158, 159} | set(range(181, 205)) | set(range(209, 213)) \
    | set(range(228, 232)) | {238, 239} | set(range(244, 248))
AXIS = (0x4000, 0x8000, 0xC000)


def decode(d, o, meshes):
    """The nine header words and the (x, y, z) angles of every mesh of the pose at o, and its length in bytes."""
    head = struct.unpack_from("<9h", d, o)
    p, rot = o + 18, []
    for _ in range(meshes):
        a = struct.unpack_from("<H", d, p)[0]
        if a & 0xC000:
            ang = [0, 0, 0]
            ang[(a >> 14) - 1] = a & 0x3FF
            rot.append(tuple(ang))
            p += 2
        else:
            b = struct.unpack_from("<H", d, p + 2)[0]
            rot.append(((a >> 4) & 0x3FF, (a & 15) << 6 | b >> 10, b & 0x3FF))
            p += 4
    return (head, tuple(rot)), p - o


def encode(pose):
    head, rot = pose
    out = struct.pack("<9h", *head)
    for x, y, z in rot:
        nz = [k for k, v in enumerate((x, y, z)) if v]
        if len(nz) <= 1:
            k = nz[0] if nz else 0
            out += struct.pack("<H", AXIS[k] | (x, y, z)[k])
        else:
            out += struct.pack("<HH", x << 4 | y >> 6, (y & 63) << 10 | z)
    return out


def layout(d, s, stride):
    """The animations, moveables, frame list and, per animation, its key frame count and whether it has a pose behind."""
    anims, movs, frames, size, _ = read(d, s, stride)
    keys = [(a[7] - a[6]) // a[1] + 1 for a in anims]
    behind = [(a[7] - a[6]) % a[1] != 0 for a in anims]
    starts = sorted({m[5] for m in movs if m[5] != 0xFFFF} | {len(anims)})
    obj = {}
    for k, m in enumerate(movs):
        if m[5] != 0xFFFF:
            end = min(x for x in starts if x > m[5])
            for i in range(m[5], end):
                obj.setdefault(i, set()).add(k)
    meshes = {}
    for i in range(len(anims)):
        n = {movs[k][1] for k in obj.get(i, ())}
        assert len(n) == 1, f"animation {i}: moveables {obj.get(i)} with different mesh counts"
        meshes[i] = n.pop()
    return anims, movs, frames, size, keys, behind, obj, meshes


def pack(d, s, stride):
    anims, movs, frames, size, keys, behind, obj, meshes = layout(d, s, stride)
    parent = {}

    def find(x):
        parent.setdefault(x, x)
        while parent[x] != x:
            parent[x] = parent[parent[x]]
            x = parent[x]
        return x

    def union(a, b):
        parent[find(a)] = find(b)

    slots = {}
    for i, a in enumerate(anims):
        w = 2 * a[2]
        for k in range(keys[i] + behind[i]):
            o = a[0] + k * w
            sl = slots.setdefault(o, {"len": w, "own": set(), "behind": set(), "meshes": set()})
            sl["len"] = max(sl["len"], w)
            sl["own" if k < keys[i] else "behind"].add(("a", i))
            sl["meshes"].add(meshes[i])
    for k, m in enumerate(movs):
        if m[5] == 0xFFFF:
            n = decode(d, frames + m[4], m[1])[1]
            sl = slots.setdefault(m[4], {"len": n, "own": set(), "behind": set(), "meshes": set()})
            sl["len"] = max(sl["len"], n)
            sl["own"].add(("m", k))
            sl["meshes"].add(m[1])
        else:
            assert m[4] == anims[m[5]][0], f"moveable {m[0]}: frame offset is not its first animation's"
    for o, sl in slots.items():
        assert len(sl["meshes"]) == 1 or sl["own"], f"pose at {o} read with different mesh counts"
        if not sl["own"]:
            sl["own"] = set(sl["behind"])
        owners = sorted(sl["own"])
        for x in owners[1:]:
            union(owners[0], x)
    for k, m in enumerate(movs):
        if m[0] in UNIFORM:
            for i in sorted(i for i in obj if k in obj[i]):
                union(("a", i), ("a", min(i for i in obj if k in obj[i])))
    order = sorted(slots)
    for a, b in zip(order, order[1:]):
        assert a + slots[a]["len"] <= b, f"poses at {a} and {b} overlap"
    assert not order or order[-1] + slots[order[-1]]["len"] <= size
    poses = {}
    for o in order:
        sl = slots[o]
        n = next(iter(sl["meshes"])) if len(sl["meshes"]) == 1 else meshes[min(i for t, i in sl["own"] if t == "a")]
        poses[o] = encode(decode(d, frames + o, n)[0])
    words = {}
    for o in order:
        g = find(min(slots[o]["own"]))
        words[g] = max(words.get(g, 0), len(poses[o]) // 2)
    new, pos, at = bytearray(), {}, 0
    for o in order:
        if at < o:
            new += d[frames + at:frames + o]
        pos[o] = len(new)
        w = 2 * words[find(min(slots[o]["own"]))]
        new += poses[o] + bytes(w - len(poses[o]))
        at = o + slots[o]["len"]
    new += d[frames + at:frames + size]
    anim_out = []
    for i, a in enumerate(anims):
        w = words[find(("a", i))]
        assert w <= 255
        for k in range(keys[i] + behind[i]):
            assert pos[a[0] + k * 2 * a[2]] == pos[a[0]] + k * 2 * w, f"poses of animation {i} not in a row"
        anim_out.append((pos[a[0]], w))
    mov_out = [pos[m[4]] for m in movs]
    for k, m in enumerate(movs):
        if m[0] in UNIFORM and m[5] != 0xFFFF:
            assert len({anim_out[i][1] for i in obj if k in obj[i]}) == 1, f"object {m[0]}: frame sizes differ"
    return anim_out, mov_out, bytes(new)


def add(d, s=None, stride=18):
    """s: offsets of the count fields (default: a PC level); stride: bytes per moveable (PSX: 20)."""
    s = s or sections(d)
    anims, movs, frames, size, *_ = layout(d, s, stride)
    anim_out, mov_out, new = pack(d, s, stride)
    out = bytearray(d)
    for i, (o, w) in enumerate(anim_out):
        struct.pack_into("<I", out, s["anims"] + 4 + 32 * i, o)
        out[s["anims"] + 4 + 32 * i + 5] = w
    for k, o in enumerate(mov_out):
        struct.pack_into("<I", out, s["moveables"] + 4 + stride * k + 12, o)
    out = bytes(out[:s["frames"]] + struct.pack("<I", len(new) // 2) + new + out[frames + size:])
    if out == bytes(d):
        return None
    check(d, s, out, stride)
    return out


def check(d, s, out, stride):
    """Every pose decodes the same, and nothing changed but the frames, the frame offsets and the frame sizes."""
    df = 2 * struct.unpack_from("<I", d, s["frames"])[0] - 2 * struct.unpack_from("<I", out, s["frames"])[0]
    assert len(d) - len(out) == df >= 0
    s2 = {k: v - (df if v > s["frames"] else 0) for k, v in s.items()}
    a1, m1, f1, _, k1, b1, _, n1 = layout(d, s, stride)
    a2, m2, f2, _, k2, b2, _, n2 = layout(out, s2, stride)
    assert (k1, b1, n1) == (k2, b2, n2)
    for i, (x, y) in enumerate(zip(a1, a2)):
        for k in range(k1[i] + b1[i]):
            p = decode(d, f1 + x[0] + k * 2 * x[2], n1[i])[0]
            q = decode(out, f2 + y[0] + k * 2 * y[2], n1[i])[0]
            assert p == q, f"animation {i}, pose {k} changed"
    for k, (x, y) in enumerate(zip(m1, m2)):
        assert decode(d, f1 + x[4], x[1])[0] == decode(out, f2 + y[4], x[1])[0], f"moveable {k}: first pose changed"
    back = bytearray(out)
    for i, a in enumerate(a1):
        struct.pack_into("<I", back, s2["anims"] + 4 + 32 * i, a[0])
        back[s2["anims"] + 4 + 32 * i + 5] = a[2]
    for k, m in enumerate(m1):
        struct.pack_into("<I", back, s2["moveables"] + 4 + stride * k + 12, m[4])
    assert back[:s["frames"]] == d[:s["frames"]], "something before the frames changed"
    assert back[s2["moveables"]:] == d[s["moveables"]:], "something behind the frames changed"


if __name__ == "__main__":
    path = sys.argv[1]
    d = open(path, "rb").read()
    out = add(d)
    if out is None:
        print(f"{path}: rotations packed already")
    else:
        open(path, "wb").write(out)
        print(f"{path}: {len(d) - len(out)} frame bytes removed")
