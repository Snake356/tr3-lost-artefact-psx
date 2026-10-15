"""Splits a room of a TR3 PC level at a block boundary into two rooms, to save drawing time on the PS1:
the engine draws every room it can see through portals completely, so a big room whose far part lies
beyond the draw distance costs as much as if all of it were close. After the split the far half can
be left out on its own.

    split_room.py <trmod> in.tr2 out.tr2 <room> <x|z> <k>

The room is cut between its sector columns (axis x) or rows (axis z) k-1 and k:
  - A keeps the room number and the sectors 0..k; its sector line k becomes a wall with a portal to B,
  - B is added as a new room with the sectors k-1..end; its sector line 0 becomes a wall with a portal
    to A (as all TR rooms, A and B overlap by these two lines).
Only sectors that were open become portals; solid sectors stay solid.
If the room has an alternate room (flipmap), that one is split the same way and the halves are
linked again (A <-> A', B <-> B').

Everything else is carried over:
  - faces go to the half they lie in; faces across the cut line go into both halves (no gaps),
  - portals of the room and of all other rooms that lead into it are given to the right half
    (and cut in two where they reach over the cut line); A and B get portals to each other,
  - sector links from other rooms (portal/floorportal/ceilingportal in their FloorData) to the room
    point to B where they lead into B's part (done with trmod),
  - room lights are copied into both halves (they light the objects in their room), static meshes,
    items and fixed cameras go to the half they are in.
Boxes, zones and overlaps are in world coordinates and stay as they are.

Checked afterwards (also in the flipped state, see check_flip): every sector of the old room has, in the half that now contains it, the same
floor, ceiling, box, room links and FloorData; every face of the old room is in A or B; every
item/camera is in a room that contains its position.
"""
import os
import re
import struct
import subprocess
import sys

import strip_unused_tail as T

WALL = -127
NOBOX = 0x7FF0


class Room:
    pass


def parse_rooms(b):
    L = T.layout(b)
    p = L["rooms"]
    nr = struct.unpack_from("<H", b, p)[0]; p += 2
    rooms = []
    for _ in range(nr):
        r = Room()
        r.x, r.z, r.yb, r.yt = struct.unpack_from("<iiii", b, p); p += 16
        ndw = struct.unpack_from("<I", b, p)[0]; p += 4
        d = p
        nv = struct.unpack_from("<H", b, d)[0]; d += 2
        r.verts = [list(struct.unpack_from("<hhhhHH", b, d + i * 12)) for i in range(nv)]; d += nv * 12
        nq = struct.unpack_from("<H", b, d)[0]; d += 2
        r.quads = [list(struct.unpack_from("<5H", b, d + i * 10)) for i in range(nq)]; d += nq * 10
        nt = struct.unpack_from("<H", b, d)[0]; d += 2
        r.tris = [list(struct.unpack_from("<4H", b, d + i * 8)) for i in range(nt)]; d += nt * 8
        ns = struct.unpack_from("<H", b, d)[0]; d += 2
        r.sprites = [list(struct.unpack_from("<hh", b, d + i * 4)) for i in range(ns)]; d += ns * 4
        assert d == p + ndw * 2, "room data size"
        p += ndw * 2
        n = struct.unpack_from("<H", b, p)[0]; p += 2
        r.portals = [list(struct.unpack_from("<H3h12h", b, p + i * 32)) for i in range(n)]; p += n * 32
        r.zs, r.xs = struct.unpack_from("<HH", b, p); p += 4
        r.sectors = [list(struct.unpack_from("<HHBbBb", b, p + i * 8)) for i in range(r.zs * r.xs)]
        p += r.zs * r.xs * 8
        r.ambient = b[p:p + 4]; p += 4
        n = struct.unpack_from("<H", b, p)[0]; p += 2
        r.lights = [b[p + i * 24:p + i * 24 + 24] for i in range(n)]; p += n * 24
        n = struct.unpack_from("<H", b, p)[0]; p += 2
        r.statics = [b[p + i * 20:p + i * 20 + 20] for i in range(n)]; p += n * 20
        r.alt, r.flags = struct.unpack_from("<hh", b, p)
        r.tail = b[p + 4:p + 7]; p += 7
        rooms.append(r)
    return L, rooms, L["rooms"], p


def build_rooms(rooms):
    out = bytearray(struct.pack("<H", len(rooms)))
    for r in rooms:
        data = bytearray()
        data += struct.pack("<H", len(r.verts)) + b"".join(struct.pack("<hhhhHH", *v) for v in r.verts)
        data += struct.pack("<H", len(r.quads)) + b"".join(struct.pack("<5H", *f) for f in r.quads)
        data += struct.pack("<H", len(r.tris)) + b"".join(struct.pack("<4H", *f) for f in r.tris)
        data += struct.pack("<H", len(r.sprites)) + b"".join(struct.pack("<hh", *s) for s in r.sprites)
        out += struct.pack("<iiii", r.x, r.z, r.yb, r.yt) + struct.pack("<I", len(data) // 2) + data
        out += struct.pack("<H", len(r.portals)) + b"".join(struct.pack("<H3h12h", *pt) for pt in r.portals)
        out += struct.pack("<HH", r.zs, r.xs) + b"".join(struct.pack("<HHBbBb", *s) for s in r.sectors)
        out += r.ambient + struct.pack("<H", len(r.lights)) + b"".join(r.lights)
        out += struct.pack("<H", len(r.statics)) + b"".join(r.statics)
        out += struct.pack("<hh", r.alt, r.flags) + r.tail
    return bytes(out)


def sec(r, xi, zi):
    return r.sectors[xi * r.zs + zi]


def split(rooms, ri, axis, k, link=None):
    """Splits rooms[ri] in place (A) and appends B; returns B's index and the portal sectors to set
    with trmod: [(room, x, z, target)].
    link = (A, B) of the base room when rooms[ri] is its alternate (flipmap) room: the portals and
    portal sectors between the halves must then name the BASE rooms, as all portals of alternate
    rooms do - flipping swaps the contents of each room with its alternate, so after the flip the
    alternate halves sit in the base rooms' slots (the alternate numbers then hold the old data)."""
    R = rooms[ri]
    ax = 0 if axis == "x" else 2                         # vertex/portal coordinate index
    n_ax = R.xs if axis == "x" else R.zs
    n_other = R.zs if axis == "x" else R.xs
    assert 2 <= k <= n_ax - 2, f"k must be 2..{n_ax - 2}"
    cut = k * 1024                                       # cut line, room-relative
    off = (k - 1) * 1024                                 # B's origin shift along the axis
    A, B = Room(), Room()
    for h in (A, B):
        h.yb, h.yt, h.ambient, h.flags, h.tail, h.alt = R.yb, R.yt, R.ambient, R.flags, R.tail, -1
        h.lights = list(R.lights)
    A.x, A.z = R.x, R.z
    B.x, B.z = (R.x + off, R.z) if axis == "x" else (R.x, R.z + off)

    # sectors
    def cell(i, j):  # i along the axis, j across
        return sec(R, i, j) if axis == "x" else sec(R, j, i)

    def grid(lo, hi):
        cells = {}
        for i in range(lo, hi + 1):
            for j in range(n_other):
                cells[(i - lo, j)] = list(cell(i, j))
        return cells, hi - lo + 1

    border = []
    for h, lo, hi, bi in ((A, 0, k, k), (B, k - 1, n_ax - 1, 0)):
        cells, n = grid(lo, hi)
        for j in range(1, n_other - 1):
            s = cells[(bi, j)]
            if s[3] != WALL:                             # open: becomes a wall with a portal
                cells[(bi, j)] = [0, NOBOX | (s[1] & 0xF), 255, WALL, 255, WALL]
                border.append((h, bi, j))
            # a solid sector stays as it is
        if axis == "x":
            h.xs, h.zs = n, n_other
            h.sectors = [cells[(i, j)] for i in range(n) for j in range(n_other)]
        else:
            h.zs, h.xs = n, n_other
            h.sectors = [cells[(i, j)] for j in range(n_other) for i in range(n)]

    # geometry: faces by position, faces across the cut go into both halves
    def add_face(h, vmap, face, nv):
        idx = []
        for vi in face[:nv]:
            if vi not in vmap:
                v = list(R.verts[vi])
                if h is B:
                    v[ax] -= off
                vmap[vi] = len(h.verts)
                h.verts.append(v)
            idx.append(vmap[vi])
        return idx + face[nv:]

    for h in (A, B):
        h.verts, h.quads, h.tris, h.sprites, h.vmap = [], [], [], [], {}
    for faces, nv, attr in ((R.quads, 4, "quads"), (R.tris, 3, "tris")):
        for f in faces:
            c = [R.verts[v][ax] for v in f[:nv]]
            if min(c) < cut:
                getattr(A, attr).append(add_face(A, A.vmap, f, nv))
            if max(c) > cut or (min(c) >= cut):
                getattr(B, attr).append(add_face(B, B.vmap, f, nv))
    for s in R.sprites:
        h = A if R.verts[s[0]][ax] < cut else B
        (h.sprites).append([add_face(h, h.vmap, [s[0]], 1)[0], s[1]])

    # the room's own portals
    def clip(pt, lo, hi, shift):
        v = [list(pt[4 + i * 3:7 + i * 3]) for i in range(4)]
        for q in v:
            q[ax] = min(max(q[ax], lo), hi) - shift
        return [pt[0], *pt[1:4], *[c for q in v for c in q]]

    A.portals, B.portals = [], []
    for pt in R.portals:
        c = [pt[4 + i * 3 + ax] for i in range(4)]
        if max(c) <= cut:
            A.portals.append(list(pt))
        elif min(c) >= cut:
            B.portals.append(clip(pt, -10 ** 9, 10 ** 9, off))
        else:
            A.portals.append(clip(pt, min(c), cut, 0))
            B.portals.append(clip(pt, cut, max(c), off))

    # portals between A and B (vertex order as the level's own portals)
    lo_o, hi_o = 1024, (n_other - 1) * 1024
    t, bt = R.yt, R.yb
    if axis == "x":
        pa = [(cut, t, lo_o), (cut, bt, lo_o), (cut, bt, hi_o), (cut, t, hi_o)]
        pb = [(1023, bt, lo_o), (1023, t, lo_o), (1023, t, hi_o), (1023, bt, hi_o)]
        na, nb = (-1, 0, 0), (1, 0, 0)
    else:
        pa = [(lo_o, bt, cut), (lo_o, t, cut), (hi_o, t, cut), (hi_o, bt, cut)]
        pb = [(lo_o, t, 1023), (lo_o, bt, 1023), (hi_o, bt, 1023), (hi_o, t, 1023)]
        na, nb = (0, 0, -1), (0, 0, 1)
    bi = len(rooms)
    to_a, to_b = link if link else (ri, bi)
    A.portals.append([to_b, *na, *[c for q in pa for c in q]])
    B.portals.append([to_a, *nb, *[c for q in pb for c in q]])

    # static meshes by position
    A.statics, B.statics = [], []
    for s in R.statics:
        wx, _, wz = struct.unpack_from("<iii", s, 0)
        w = (wx - R.x) if axis == "x" else (wz - R.z)
        (A if w < cut else B).statics.append(s)

    for h in (A, B):
        del h.vmap
    rooms[ri] = A
    rooms.append(B)
    trm = [(ri if h is A else bi, *((i, j) if axis == "x" else (j, i)), to_b if h is A else to_a)
           for h, i, j in border]
    return bi, trm


def retarget_portals(rooms, ri, bi, axis, R_x, R_z, k, skip):
    """Portals of other rooms into the split room: to B where they lead into B's part."""
    ax = 0 if axis == "x" else 2
    wcut = (R_x if axis == "x" else R_z) + k * 1024
    for n, r in enumerate(rooms):
        if n in skip:
            continue
        new = []
        org = r.x if axis == "x" else r.z
        for pt in r.portals:
            if pt[0] != ri:
                new.append(pt); continue
            c = [pt[4 + i * 3 + ax] + org for i in range(4)]
            if max(c) <= wcut:
                new.append(pt)
            elif min(c) >= wcut:
                new.append([bi] + pt[1:])
            else:
                v = [list(pt[4 + i * 3:7 + i * 3]) for i in range(4)]
                va = [[*q] for q in v]; vb = [[*q] for q in v]
                for q in va: q[ax] = min(q[ax], wcut - org)
                for q in vb: q[ax] = max(q[ax], wcut - org)
                new.append([ri, *pt[1:4], *[x for q in va for x in q]])
                new.append([bi, *pt[1:4], *[x for q in vb for x in q]])
        r.portals = new


def trmod(exe, level, *cmds):
    out = ""
    for i in range(0, len(cmds), 60):
        r = subprocess.run([exe, "tr3pc", level, *cmds[i:i + 60]], capture_output=True, text=True)
        if r.returncode != 0 or "rror" in r.stdout:
            sys.exit(f"trmod: {r.stdout[-400:]} {r.stderr[-400:]}")
        out += r.stdout
    return out


def floordata(exe, level):
    return {tuple(map(int, k.split(","))): c for c, k in
            re.findall(r'"(replacefloordata\((\d+,\d+,\d+):[^"]*\))"', trmod(exe, level, "cgetfloordata(*,*,*)"))}


def main(exe, src, dst, ri, axis, k):
    b = open(src, "rb").read()
    L, rooms, r0, r1 = parse_rooms(b)
    old = [r for r in rooms]
    fd_old = floordata(exe, src)
    pairs = [(ri, None)]
    if rooms[ri].alt != -1:
        pairs.append((rooms[ri].alt, None))
    assert all(r.alt != ri for r in rooms), "another room has this room as its alternate"

    todo_fd, halves = [], []
    for n, (r, _) in enumerate(pairs):
        R = rooms[r]
        Rx, Rz = R.x, R.z
        if n:
            P = old[ri]
            assert (R.x, R.z, R.xs, R.zs) == (P.x, P.z, P.xs, P.zs), "alternate room has another size"
            assert all(s == -1 or s is None for s in [R.alt]) or R.alt == ri, "alternate room links on"

        bi, trm = split(rooms, r, axis, k, link=(halves[0][0], halves[0][1]) if n else None)
        retarget_portals(rooms, r, bi, axis, Rx, Rz, k, skip={r, bi})
        todo_fd += trm
        halves.append((r, bi, Rx, Rz))
        print(f"room {r} -> {r} ({len(rooms[r].quads) + len(rooms[r].tris)} faces) + "
              f"{bi} ({len(rooms[bi].quads) + len(rooms[bi].tris)} faces), "
              f"{len(old[r].quads) + len(old[r].tris)} before")
    if len(halves) == 2:
        (a, b_, *_), (a2, b2, *_) = halves
        rooms[a].alt, rooms[b_].alt = a2, b2
        # the alternate halves keep the flag bits of the alternate room
    # sector links in other rooms' sectors (room below/above bytes): to B where they lead into B
    for (a, bi, Rx, Rz) in halves:
        wcut = (Rx if axis == "x" else Rz) + k * 1024
        for n, r in enumerate(rooms):
            if n in (a, bi):
                continue
            for xi in range(r.xs):
                for zi in range(r.zs):
                    s = sec(r, xi, zi)
                    w = (r.x + xi * 1024 + 512) if axis == "x" else (r.z + zi * 1024 + 512)
                    if w > wcut:
                        if s[2] == a: s[2] = bi
                        if s[4] == a: s[4] = bi
    body = build_rooms(rooms)
    nb = bytearray(b[:r0]) + body + b[r1:]
    # items and cameras: into the half that contains them
    L2 = T.layout(bytes(nb))
    for base, n, size, rofs in ((L2["items"][0] + 4, L2["items"][1], 24, 2), (L2["cameras"][0] + 4, L2["cameras"][1], 16, 12)):
        for i in range(n):
            o = base + i * size
            room = struct.unpack_from("<h", nb, o + rofs)[0]
            if size == 24:
                x, y, z = struct.unpack_from("<iii", nb, o + 4)
            else:
                x, y, z = struct.unpack_from("<iii", nb, o)
            for (a, bi, Rx, Rz) in halves:
                w = x if axis == "x" else z
                if room == a and w >= (Rx if axis == "x" else Rz) + k * 1024:
                    struct.pack_into("<h", nb, o + rofs, bi)
    tmp = dst + ".tmp"
    open(tmp, "wb").write(nb)

    # FloorData links (portal/floorportal/ceilingportal) in other rooms into B's part, and the new
    # portal sectors between A and B
    fd = floordata(exe, tmp)
    cmds = []
    for (a, bi, Rx, Rz) in halves:
        wcut = (Rx if axis == "x" else Rz) + k * 1024
        pat = re.compile(r"\b(portal|floorportal|ceilingportal)\(%d\)" % a)
        for (rn, xi, zi), c in fd.items():
            if rn in (a, bi) or not pat.search(c):
                continue
            r = rooms[rn]
            w = (r.x + xi * 1024 + 512) if axis == "x" else (r.z + zi * 1024 + 512)
            if w > wcut:
                cmds.append(pat.sub(lambda m: f"{m.group(1)}({bi})", c))
    cmds += [f"replacefloordata({r},{x},{z}:portal({t}))" for r, x, z, t in todo_fd]
    if cmds:
        trmod(exe, tmp, *cmds)
    check(exe, src, tmp, old, halves, axis, k, fd_old)
    os.replace(tmp, dst)
    print(f"written {dst}, check ok")


def world_cameras(text, rooms):
    """Trigger cameras are written room-relative by trmod: compare them in world coordinates."""
    def w(m):
        r, x, z = int(m.group(1)), int(m.group(2)), int(m.group(3))
        return f"camera(W{rooms[r].x + x},{rooms[r].z + z},"
    return re.sub(r"camera\((\d+),(-?\d+),(-?\d+),", w, text)


def check(exe, src, dst, old, halves, axis, k, fd_old):
    b = open(dst, "rb").read()
    L, rooms, *_ = parse_rooms(b)
    fd = floordata(exe, dst)
    for (a, bi, Rx, Rz) in halves:
        R = old[a]
        for xi in range(R.xs):
            for zi in range(R.zs):
                i = xi if axis == "x" else zi
                if i in (0, (R.xs if axis == "x" else R.zs) - 1):
                    continue                              # outer walls stay in A or B unchanged
                for h, hn in ((rooms[a], a), (rooms[bi], bi)):
                    hx, hz = xi - (h.x - R.x) // 1024, zi - (h.z - R.z) // 1024
                    hi = hx if axis == "x" else hz
                    n_h = h.xs if axis == "x" else h.zs
                    if not (1 <= hi <= n_h - 2):
                        continue                          # not inside this half
                    s0, s1 = sec(R, xi, zi), sec(h, hx, hz)
                    assert s0[1] == s1[1] and s0[3] == s1[3] and s0[5] == s1[5], (a, xi, zi, s0, s1)
                    f0 = world_cameras(fd_old.get((a, xi, zi), "").split(":", 1)[-1], old)
                    f1 = world_cameras(fd.get((hn, hx, hz), "").split(":", 1)[-1], rooms)
                    assert f0 == f1, f"FloorData differs at room {a} sector {xi},{zi}: {f0!r} / {f1!r}"
        na = len(rooms[a].quads) + len(rooms[a].tris) + len(rooms[bi].quads) + len(rooms[bi].tris)
        assert na >= len(R.quads) + len(R.tris), "faces lost"
    for base, n, size, rofs, xyz in ((L["items"][0] + 4, L["items"][1], 24, 2, 4),
                                     (L["cameras"][0] + 4, L["cameras"][1], 16, 12, 0)):
        for i in range(n):
            o = base + i * size
            room = struct.unpack_from("<h", b, o + rofs)[0]
            x, y, z = struct.unpack_from("<iii", b, o + xyz)
            for (a, bi, *_ ) in halves:
                R = old[a]
                inside_old = R.x <= x < R.x + R.xs * 1024 and R.z <= z < R.z + R.zs * 1024
                if room in (a, bi) and inside_old:      # some cameras lie outside their room already
                    h = rooms[room]
                    assert h.x <= x < h.x + h.xs * 1024 and h.z <= z < h.z + h.zs * 1024, \
                        f"{'item' if size == 24 else 'camera'} {i} outside its room {room}"
    new = check_flip(exe, dst) - check_flip(exe, src)
    assert not new, "the split adds problems:\n  " + "\n  ".join(sorted(new))


def check_flip(exe, path):
    """Problems of the level as the game sees it before and after the flipmap: flipping swaps the
    contents of every room with its alternate room, so in both states every portal and sector link of
    the rooms in use must lead to a room in use that contains that spot, with a matching portal back.
    Returns the problems as a set of strings (some original levels already have a few)."""
    problems = set()
    b = open(path, "rb").read()
    _, rooms, *_ = parse_rooms(b)
    fdl = {}
    for (rn, xi, zi), c in floordata(exe, path).items():
        fdl.setdefault(rn, []).append((xi, zi, c))
    alts = {r.alt for r in rooms if r.alt != -1}
    for state in ("normal", "flipped"):
        slot = {i: i for i in range(len(rooms))}
        if state == "flipped":
            for i, r in enumerate(rooms):
                if r.alt != -1:
                    slot[i], slot[r.alt] = r.alt, i
        active = [i for i in range(len(rooms)) if i not in alts]
        data = lambda i: rooms[slot[i]]
        ports = {}
        for i in active:
            R = data(i)
            for p in R.portals:
                if p[0] not in active:
                    problems.add(f"{state}: room {i} has a portal to room {p[0]}, which is not in use")
                v = tuple(sorted((p[4 + j * 3] + R.x, p[5 + j * 3], p[6 + j * 3] + R.z) for j in range(4)))
                ports.setdefault((i, p[0]), []).append(v)
        for (i, t), vs in ports.items():
            for v in vs:
                back = ports.get((t, i), [])
                if not any(all(abs(a - c) <= 1 for pa, pb in zip(v, w) for a, c in zip(pa, pb)) for w in back):
                    problems.add(f"{state}: portal {i} -> {t} has no matching portal back")
        for i in active:
            R = data(i)
            for xi, zi, c in fdl.get(slot[i], []):
                wx, wz = R.x + xi * 1024 + 512, R.z + zi * 1024 + 512
                for kind, t in re.findall(r"\b(portal|floorportal|ceilingportal)\((\d+)\)", c):
                    T_ = data(int(t))
                    ok = int(t) in active and 1 <= (wx - T_.x) // 1024 <= T_.xs - 2 and 1 <= (wz - T_.z) // 1024 <= T_.zs - 2
                    if not ok:
                        problems.add(f"{state}: room {i} sector {xi},{zi}: {kind}({t}) does not lead into a room there")
    return problems


if __name__ == "__main__":
    if len(sys.argv) != 7:
        sys.exit(__doc__)
    main(sys.argv[1], sys.argv[2], sys.argv[3], int(sys.argv[4]), sys.argv[5], int(sys.argv[6]))
