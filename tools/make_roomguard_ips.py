"""Builds bin/patches/<version>/roomguard.ips: a guard against overrunning the GPU primitive buffer.

Each frame the game writes its GPU primitives into one of two buffers of 103,480 bytes (one is drawn while
the other is filled). The routines for objects, sprites and effects check the end of the buffer and leave
out what no longer fits; the hand-written routine for the room geometry does not. In views with very much
geometry (Highland Fling between room 2 and room 3, and close to it under the bridge) the room pieces run
past the end into the other buffer, which the GPU is drawing at that moment: broken polygons, a hanging
picture and, as the memory behind the second buffer is hit as well, a crash at the next loading screen.

The patch redirects the call that draws each room piece (addiu a1, gp, <buffer structure>; jal <room
piece>; move a0, s5) to a small routine that draws the piece only when at least MARGIN bytes are still free
(the largest room piece of the project needs about 7 KB) and otherwise leaves it out for this frame. In
normal views it changes nothing; at its limit the farthest room pieces, already in the black distance fade,
are left out instead of the buffer being overrun.

The routine goes into a game function nothing refers to, found on the executable with all other patches
of bin/patch.sh applied (make_reunionview_ips.unused_routines, so the draw distance patch's code is not
touched). Call site and buffer structure are found by their code in every version.

Usage: make_roomguard_ips.py [--versions us,ukrev,...] [--exe VERSION=PATH ...]
(--exe for executables not in bin/orig, e.g. the first releases: us10, uk, de, fr, it, sp)"""
import argparse
import os
import struct

import make_reunionview_ips as rv

HERE = os.path.dirname(os.path.abspath(__file__))
BIN = os.path.join(HERE, "..", "bin")
LOAD = rv.LOAD                                     # memory address = file offset + LOAD
MARGIN = 0x2000


def words(data, off, n):
    return struct.unpack_from(f"<{n}I", data, off)


def find_call(exe):
    """(file offset of 'addiu a1, gp, X', X, room piece address)."""
    hits = []
    for off in range(0x800, len(exe) - 12, 4):
        a, j, m = words(exe, off, 3)
        if a >> 16 == 0x2785 and j >> 26 == 3 and m == 0x02A02021:      # addiu a1,gp,X ; jal ; move a0,s5
            hits.append((off, a & 0xFFFF, (j & 0x03FFFFFF) << 2 | 0x80000000))
    assert len(hits) == 1, f"room piece call found {len(hits)} times"
    off, db, piece = hits[0]
    assert words(exe, piece - LOAD, 1)[0] == 0x23BDFFD0, "room piece routine does not start as expected"
    return off, db, piece


def check_buffer_structure(exe, db):
    """The frame setup stores the buffer start into db+0xC and db+8 and start+0x19438 into db+0x10."""
    i = exe.find(struct.pack("<I", 0x34639438))                       # ori v1, v1, 0x9438
    assert i > 0 and exe.find(struct.pack("<I", 0x34639438), i + 1) < 0, "buffer setup not found exactly once"
    stores = [w & 0xFFFF for w in words(exe, i, 12) if w >> 16 == 0xAF82]   # sw v0, X(gp)
    assert stores == [db + 0xC, db + 8, db + 0x10], f"buffer structure differs: {[hex(s) for s in stores]}"


def routine(db, piece):
    w = [0x27850000 | db,                                    # addiu a1, gp, db
         0x8CA80008,                                         # lw t0, 8(a1)        primitive pointer
         0x8CA90010,                                         # lw t1, 0x10(a1)     end of the buffer
         0x00000000,
         0x25080000 | MARGIN,                                # addiu t0, t0, MARGIN
         0x0109082B,                                         # sltu at, t0, t1
         0x10200003,                                         # beqz at, skip
         0x00000000,
         0x08000000 | (piece & 0x0FFFFFFF) >> 2,             # j room piece (it returns to the caller)
         0x00000000,
         0x03E00008,                                         # skip: jr ra
         0x00000000]
    return w


def build(exe, ver):
    """exe has all other patches applied; [(file offset, [words])] and a description."""
    call, db, piece = find_call(exe)
    check_buffer_structure(exe, db)
    code = routine(db, piece)
    W = rv.words(exe)
    free = [r for r in rv.unused_routines(W, ver) if r[1] - r[0] >= 4 * len(code)]
    assert free, "no unused function large enough"
    cave = min(free, key=lambda r: r[1] - r[0])[0]
    patches = [(cave, code), (call + 4, [rv.jal(cave + LOAD)])]
    return patches, f"call {call + 4 + LOAD:#x} -> {cave + LOAD:#x} (buffer gp+{db:#x}, room piece {piece:#x})"


def patched(ver, path):
    exe = open(path, "rb").read()
    patch_sh = open(os.path.join(BIN, "patch.sh"), encoding="utf-8").read()
    names = [l.split('"')[1].split(",") for l in patch_sh.splitlines() if l.startswith("patches=")][0]
    for name in names[:names.index("roomguard")]:                # the patches before this one
        exe = rv.apply_ips(exe, open(os.path.join(BIN, "patches", ver, f"{name}.ips"), "rb").read())
    return exe


if __name__ == "__main__":
    ap = argparse.ArgumentParser()
    ap.add_argument("--versions", default="us,ukrev,derev,frrev,itrev,sprev,jp,jp2,as,as2")
    ap.add_argument("--exe", nargs="*", default=[], help="VERSION=PATH for executables not in bin/orig")
    a = ap.parse_args()
    paths = {v: os.path.join(BIN, "orig", f"{v}.exe") for v in a.versions.split(",") if v}
    paths.update(dict(e.split("=", 1) for e in a.exe))
    for ver, path in paths.items():
        p, desc = build(patched(ver, path), ver)
        open(os.path.join(BIN, "patches", ver, "roomguard.ips"), "wb").write(rv.ips(p))
        print(f"{ver}: {desc}")
