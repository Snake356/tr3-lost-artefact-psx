"""Frees level memory on the PS1 by compacting the frame list and the mesh tree list of a Tomb Raider III PC
level (the converter copies both as they are); Highland Fling has the least left.

Frames: an animation uses (frame_end - frame_base) // rate + 1 poses from its frame offset, and one more
when (frame_end - frame_base) % rate is not 0, because in its last frames the game blends towards the
pose behind (animation 220 showed that it matters). The animations of a moveable whose poses join up form
a block; its first block also holds the moveable's own frame offset (the inventory reads its poses from
there one after the other). A moveable without animations uses one pose at its frame offset. Blocks with
the same bytes are shared (e.g. a puzzle item and its inventory object, AI and emitter objects), with
everything in them moved by the same amount, so the poses keep their order. Bytes no animation and no
moveable uses are then removed (e.g. the old poses of animation 292, which add_crawl_pickup.py
replaced), and all frame offsets move down.

Mesh trees: moveables with the same tree (e.g. Lara's weapon animation objects) share one copy, and the
tree words no moveable uses are removed. Nothing else in the level refers to them.

Meshes that are the same (the large medipack has one twice) are not shared here: the mesh optimisation
(tr3mesh, levels/mesh/<level>.txt) rewrites all mesh pointers afterwards.

Every pose an animation or a moveable uses and every tree a moveable uses is checked to be the same
afterwards. A level that is compact already is left as it is.

Usage: compact_frames.py <level.tr2>   (changes the file in place)"""
import struct
import sys

from add_crawl_flip import sections


def pose_bytes(d, o, meshes):
    """Size of one pose at o: bounding box, offset, then one or two words per mesh."""
    p = o + 18
    for _ in range(meshes):
        p += 2 if struct.unpack_from("<H", d, p)[0] & 0xC000 else 4
    return p - o


def read(d, s, stride):
    u = lambda f, o: struct.unpack_from(f, d, o)
    anims = [u("<IBBHiiHH", s["anims"] + 4 + 32 * i) for i in range(u("<I", s["anims"])[0])]
    movs = [u("<IHHIIH", s["moveables"] + 4 + stride * i) for i in range(u("<I", s["moveables"])[0])]
    frames = s["frames"] + 4
    size = 2 * u("<I", s["frames"])[0]
    spans = []
    for i, (off, rate, words, *_, base, end) in enumerate(anims):
        assert rate >= 1 and end >= base, (i, rate, base, end)
        n = (end - base) // rate + 1 + ((end - base) % rate != 0)
        assert off % 2 == 0 and off + 2 * words * n <= size, (i, off, words, n, size)
        spans.append((off, 2 * words * n))
    return anims, movs, frames, size, spans


def blocks(d, anims, movs, frames, size, spans):
    """The blocks: (offset, length, animations in it, moveables whose frame offset is in it)."""
    starts = sorted({m[5] for m in movs if m[5] != 0xFFFF} | {0, len(anims)})
    out = []
    for a, b in zip(starts, starts[1:]):
        group = sorted(range(a, b), key=lambda i: spans[i][0])
        cur = None
        for i in group:
            off, n = spans[i]
            if cur and off <= cur[0] + cur[1]:
                cur[1] = max(cur[1], off + n - cur[0])
                cur[2].append(i)
            else:
                cur = [off, n, [i], []]
                out.append(cur)
    for k, m in enumerate(movs):
        if m[5] == 0xFFFF:
            assert m[4] % 2 == 0 and m[4] < size, (k, m)
            out.append([m[4], pose_bytes(d, frames + m[4], m[1]), [], [k]])
        else:
            first = spans[m[5]][0]
            assert m[4] == first, f"moveable {m[0]}: frame offset {m[4]} is not its first animation's {first}"
            [blk] = [b for b in out if m[5] in b[2]]
            blk[3].append(k)
    return out


def compact_frames(d, s, stride, report):
    anims, movs, frames, size, spans = read(d, s, stride)
    first, moved = {}, {}
    shared = 0
    for off, n, ai, mi in sorted(blocks(d, anims, movs, frames, size, spans), key=lambda b: b[0]):
        to = first.setdefault(d[frames + off:frames + off + n], off) if n else off
        if to != off:
            shared += 1
            moved.update({("a", i): spans[i][0] - off + to for i in ai})
            moved.update({("m", k): to for k in mi})
    anim_off = [moved.get(("a", i), spans[i][0]) for i in range(len(anims))]
    mov_off = [moved.get(("m", k), m[4]) for k, m in enumerate(movs)]
    used = bytearray(size)
    for i, (_, n) in enumerate(spans):
        used[anim_off[i]:anim_off[i] + n] = b"" * n
    for k, m in enumerate(movs):
        n = pose_bytes(d, frames + m[4], m[1]) if m[5] == 0xFFFF else spans[m[5]][1]
        used[mov_off[k]:mov_off[k] + n] = b"" * n
    before = [0] * (size + 1)
    for o in range(size):
        before[o + 1] = before[o] + (not used[o])
    anim_off = [o - before[o] for o in anim_off]
    mov_off = [o - before[o] for o in mov_off]
    assert all(o % 2 == 0 for o in anim_off + mov_off) and before[size] % 2 == 0
    report["frames"], report["shared"] = before[size], shared
    return anim_off, mov_off, bytes(d[frames + o] for o in range(size) if used[o])


def compact_trees(d, s, movs):
    u = lambda f, o: struct.unpack_from(f, d, o)
    trees = s["trees"] + 4
    count = u("<I", s["trees"])[0]
    first, tree = {}, []
    for k, m in enumerate(movs):
        n = 4 * max(m[1] - 1, 0)
        assert m[3] + n <= count, (k, m, count)
        tree.append(first.setdefault(d[trees + 4 * m[3]:trees + 4 * (m[3] + n)], m[3]) if n else m[3])
    used = [False] * count
    for k, m in enumerate(movs):
        for w in range(tree[k], tree[k] + 4 * max(m[1] - 1, 0)):
            used[w] = True
    before = [0] * (count + 1)
    for w in range(count):
        before[w + 1] = before[w] + (not used[w])
    new = b"".join(d[trees + 4 * w:trees + 4 * w + 4] for w in range(count) if used[w])
    return [t - before[min(t, count)] for t in tree], new


def add(d, s=None, stride=18, report=None):
    """s: offsets of the count fields (default: a PC level); stride: bytes per moveable (PSX: 20).
    report (a dict) gets the bytes removed per part."""
    s = s or sections(d)
    report = {} if report is None else report
    _, movs, frames, size, _ = read(d, s, stride)
    anim_off, mov_off, new_frames = compact_frames(d, s, stride, report)
    tree, new_trees = compact_trees(d, s, movs)
    t_end = s["trees"] + 4 + 4 * struct.unpack_from("<I", d, s["trees"])[0]
    report["trees"] = t_end - s["trees"] - 4 - len(new_trees)
    out = bytearray(d)
    for i, o in enumerate(anim_off):
        struct.pack_into("<I", out, s["anims"] + 4 + 32 * i, o)
    for k in range(len(movs)):
        struct.pack_into("<II", out, s["moveables"] + 4 + stride * k + 8, tree[k], mov_off[k])
    out = bytes(out[:s["trees"]] + struct.pack("<I", len(new_trees) // 4) + new_trees + out[t_end:s["frames"]]
                + struct.pack("<I", len(new_frames) // 2) + new_frames + out[frames + size:])
    if out == bytes(d):
        return None
    check(d, s, out, stride)
    return out


def check(d, s, out, stride):
    """Every pose and every tree in use is the same as before, and nothing else changed but the offsets."""
    count = lambda b, o: struct.unpack_from("<I", b, o)[0]
    dt = 4 * (count(d, s["trees"]) - count(out, s["trees"]))
    df = 2 * count(d, s["frames"]) - 2 * count(out, s["frames"] - dt)
    assert len(d) - len(out) == dt + df >= 0
    s2 = {k: v - (dt if v >= s["frames"] else 0) - (df if v > s["frames"] else 0) for k, v in s.items()}
    a1, m1, f1, size1, sp1 = read(d, s, stride)
    a2, m2, f2, _, sp2 = read(out, s2, stride)
    assert len(a1) == len(a2) and len(m1) == len(m2)
    for i, ((o1, n1), (o2, n2)) in enumerate(zip(sp1, sp2)):
        assert n1 == n2 and d[f1 + o1:f1 + o1 + n1] == out[f2 + o2:f2 + o2 + n2], f"poses of animation {i} changed"
    t1, t2 = s["trees"] + 4, s2["trees"] + 4
    for k, (x, y) in enumerate(zip(m1, m2)):
        n = 16 * max(x[1] - 1, 0)
        assert d[t1 + 4 * x[3]:t1 + 4 * x[3] + n] == out[t2 + 4 * y[3]:t2 + 4 * y[3] + n], f"tree of moveable {k} changed"
        n = pose_bytes(d, f1 + x[4], x[1]) if x[5] == 0xFFFF else sp1[x[5]][1]
        assert d[f1 + x[4]:f1 + x[4] + n] == out[f2 + y[4]:f2 + y[4] + n], f"poses of moveable {k} changed"
    for off, n, ai, mi in blocks(d, a1, m1, f1, size1, sp1):
        for i in ai[1:]:
            assert sp2[i][0] - sp2[ai[0]][0] == sp1[i][0] - sp1[ai[0]][0], f"poses of animation {i} moved apart"
        for k in mi:
            assert ai == [] or m2[k][4] == sp2[min(ai, key=lambda i: sp1[i][0])][0], f"moveable {k} left its poses"
    back = bytearray(out)
    for i, a in enumerate(a1):
        struct.pack_into("<I", back, s2["anims"] + 4 + 32 * i, a[0])
    for k, m in enumerate(m1):
        struct.pack_into("<II", back, s2["moveables"] + 4 + stride * k + 8, m[3], m[4])
    assert back[:s["trees"]] == d[:s["trees"]], "something before the mesh trees changed"
    assert back[s2["frames"] - (s["frames"] - t1 - 4 * count(d, s["trees"])):s2["frames"]] ==         d[t1 + 4 * count(d, s["trees"]):s["frames"]], "something between the trees and the frames changed"
    assert back[s2["moveables"]:] == d[s["moveables"]:], "something behind the frames changed"


if __name__ == "__main__":
    path = sys.argv[1]
    report = {}
    out = add(open(path, "rb").read(), report=report)
    if out is None:
        print(f"{path}: frames and mesh trees compact already")
    else:
        open(path, "wb").write(out)
        print(f"{path}: {report['frames']} frame bytes removed ({report['shared']} pose blocks shared), "
              f"{report['trees']} mesh tree bytes removed")
