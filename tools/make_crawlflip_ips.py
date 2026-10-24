"""Builds bin/patches/<version>/crawlflip.ips: a forward crawl exit. Crouched
in a crawlspace at its edge, Walk + Jump makes Lara flip out head first and fall (R1 + square on the
western discs; it follows the game's Walk and Jump, so the Japanese and Asian layouts too).

The animation is added to every level as animation 313 (tools/add_crawl_flip.py; Lara's home, the
released HOUSE.PSX, by tools/psx_lara_anims.py; frames 32000-32043,
state AS_NULL; at its end Lara is moved 620 forward and 815 down and falls). This patch starts it:
the crawl collision routine reads the input word at 'lw v1, input(gp); sw v0, 0x44(s0); lui v0,
0x2000' (s0 = Lara); that load becomes a call of a routine that returns the input word and, when

  - Walk and Jump are held and Lara is in a crawl standstill animation (263 or 264),
  - the floor 256 ahead (LaraFloorFront) is more than 768 lower,
  - the ceiling 256 ahead (LaraCeilingFront) leaves her standing height,

turns her square to the nearest axis, puts her 225 short of the edge of her square (as the crawl to
hang of Tomb Raider III does backwards), starts animation 313 in state AS_NULL and returns only
"crouch held", so the rest of the routine leaves her alone.

Found in every version: the input load, LaraCeilingFront and LaraFloorFront as the two calls with
'addiu a2, zero, -300' behind it (crawl to hang), the standstill animations ('addiu v0, v0, -263').
LaraCeilingFront of the Japanese and Asian Disc 1 takes the height to test from as a fourth argument
(the others use Lara's height, 762): it gets 762.
The routine goes behind the 3D pickups' routines into the block pickup3d.ips appends to the executable,
which the start-up copies to the end of the level memory (there is no unused function left for it in
every version).

Usage: make_crawlflip_ips.py [--versions us,ukrev,...] [--exe VERSION=PATH ...]"""
import argparse
import os
import struct

import make_pickup3d_ips as p3
import make_reunionview_ips as rv

HERE = os.path.dirname(os.path.abspath(__file__))
BIN = os.path.join(HERE, "..", "bin")
LOAD = rv.LOAD

ANIM, FRAME_BASE, AS_NULL = 313, 32000, 19          # tools/add_crawl_flip.py
IN_WALK_JUMP = 0x80 | 0x10                          # Walk (slow), Jump
IN_DUCK = 0x20000000
NO_HEIGHT = -32512
EDGE = 225


def word(d, o):
    return struct.unpack_from("<I", d, o)[0]


def sites(exe):
    """(file offset of the input load, input gp offset, LaraFloorFront, LaraCeilingFront, standstill anim)."""
    hits = []
    for o in range(0x800, len(exe) - 0x200, 4):
        if word(exe, o) >> 16 == 0x8F83 and word(exe, o + 4) == 0xAE020044 and word(exe, o + 8) == 0x3C022000:
            calls = [p3.jal_target(word(exe, k)) for k in range(o, o + 0x200, 4)
                     if word(exe, k) >> 26 == 3 and 0x2406FED4 in [word(exe, k + 4 * j) for j in range(-3, 2)]]
            anim = next(word(exe, k) & 0xFFFF for k in range(o, o + 0x80, 4) if word(exe, k) >> 16 == 0x2442
                        and word(exe, k) & 0x8000)
            hits.append((o, word(exe, o) & 0xFFFF, calls[1], calls[0], 0x10000 - anim))
    assert len(hits) == 1 and hits[0][4] == 263, f"crawl collision found {len(hits)} times"
    return hits[0]


def routine(inp, floor_front, ceiling_front):
    a = p3.Asm()
    a.lw("v1", inp, "gp")
    a.addiu("sp", "sp", -24); a.sw("ra", 20, "sp"); a.sw("v1", 16, "sp")
    a.andi("t0", "v1", IN_WALK_JUMP); a.addiu("t1", "zero", IN_WALK_JUMP)
    a.bne("t0", "t1", "out"); a.nop()                             # Walk + Jump
    a.lhu("t0", 0x14, "s0"); a.nop()
    a.addiu("t0", "t0", -263); a.sltiu("t0", "t0", 2)
    a.beqz("t0", "out"); a.nop()                                  # crawl standstill
    a.move("a0", "s0"); a.lh("a1", 0x4E, "s0")
    a.jal(floor_front); a.addiu("a2", "zero", 256)
    a.slti("t0", "v0", 769)
    a.bnez("t0", "out"); a.nop()                                  # floor ahead more than 768 lower
    a.move("a0", "s0"); a.lh("a1", 0x4E, "s0"); a.addiu("a3", "zero", 762)   # Japan: the height (else 762)
    a.jal(ceiling_front); a.addiu("a2", "zero", 256)
    a.addiu("t0", "zero", NO_HEIGHT)
    a.beq("v0", "t0", "out"); a.nop()
    a.b(0x07, "v0", "zero", "out"); a.nop()                       # bgtz: standing height ahead
    a.lhu("t0", 0x4E, "s0"); a.nop()                              # square to the axis: 0 N, 1 E, 2 S, 3 W
    a.addiu("t0", "t0", 0x2000); a.andi("t0", "t0", 0xFFFF)
    a.r(0x02, "t0", "zero", "t0", 14)
    a.sll("t1", "t0", 14); a.sh("t1", 0x4E, "s0")
    a.andi("t2", "t0", 1); a.r(0x02, "t3", "zero", "t0", 1)
    a.addiu("t4", "s0", 0x48)                                     # z (north, south)
    a.beqz("t2", "coord"); a.nop()
    a.addiu("t4", "s0", 0x40)                                     # x (east, west)
    a.label("coord")
    a.lw("t5", 0, "t4"); a.nop()
    a.bnez("t3", "negative"); a.nop()
    a.ori("t5", "t5", 1023); a.bra("store"); a.addiu("t5", "t5", -EDGE)
    a.label("negative")
    a.addiu("t6", "zero", -1024); a.r(0x24, "t5", "t5", "t6"); a.addiu("t5", "t5", EDGE)
    a.label("store")
    a.sw("t5", 0, "t4")
    a.addiu("t0", "zero", ANIM); a.sh("t0", 0x14, "s0")
    a.addiu("t0", "zero", FRAME_BASE); a.sh("t0", 0x16, "s0")
    a.addiu("t0", "zero", AS_NULL); a.sh("t0", 0x0E, "s0"); a.sh("t0", 0x10, "s0")
    a.sh("zero", 0x1E, "s0"); a.sh("zero", 0x20, "s0")            # speed, fall speed
    a.lui("t0", IN_DUCK >> 16); a.sw("t0", 16, "sp")              # the rest of the routine: crouch held
    a.label("out")
    a.lw("v1", 16, "sp"); a.lw("ra", 20, "sp"); a.nop()             # load delay: jr needs the new ra
    a.jr("ra"); a.addiu("sp", "sp", 24)
    return a.code()


def appended(orig, ver):
    """File offset and address of the first free bytes in the block pickup3d.ips appends to the executable
    (copied at start-up to the end of the level memory): behind its last word that is not 0, plus 32 for
    the zeros at the end of its data block."""
    for off, end in p3.ips_ranges(open(os.path.join(BIN, "patches", ver, "pickup3d.ips"), "rb").read()):
        if off == len(orig):
            break
    else:
        raise AssertionError("pickup3d.ips appends nothing")
    ips = rv.apply_ips(orig, open(os.path.join(BIN, "patches", ver, "pickup3d.ips"), "rb").read())
    used = (len(ips[off:end].rstrip(b"\x00")) + 32 + 15) & ~15
    _, size, buffer = p3.level_memory(orig)
    return off + used, buffer + size - p3.EXTRA + used, min(end, len(orig) + p3.EXTRA)   # the padding is not copied


def appended_after(orig, ver):
    """File offset and address of the first free bytes behind this patch's routine, and the block's end."""
    off, addr, end = appended(orig, ver)
    _, inp, floor_front, ceiling_front, _ = sites(orig)
    n = (4 * len(routine(inp, floor_front, ceiling_front)) + 15) & ~15
    return off + n, addr + n, end


def build(orig, ver):
    site, inp, floor_front, ceiling_front, _ = sites(orig)
    off, addr, end = appended(orig, ver)
    code = routine(inp, floor_front, ceiling_front)
    assert off + 4 * len(code) <= end, "no room left in the appended block"
    patches = [(off, code), (site, [rv.jal(addr)])]
    return patches, (f"input load {site + LOAD:#x} (gp+{inp:#x}) -> {addr:#x} ({4 * len(code)} bytes, "
                     f"{end - off - 4 * len(code)} left), LaraFloorFront {floor_front:#x}, "
                     f"LaraCeilingFront {ceiling_front:#x}")


if __name__ == "__main__":
    ap = argparse.ArgumentParser()
    ap.add_argument("--versions", default="us,ukrev,derev,frrev,itrev,sprev,jp,jp2,as,as2")
    ap.add_argument("--exe", nargs="*", default=[], help="VERSION=PATH for executables not in bin/orig")
    a = ap.parse_args()
    paths = {v: os.path.join(BIN, "orig", f"{v}.exe") for v in a.versions.split(",") if v}
    paths.update(dict(e.split("=", 1) for e in a.exe))
    for ver, path in paths.items():
        p, desc = build(open(path, "rb").read(), ver)
        open(os.path.join(BIN, "patches", ver, "crawlflip.ips"), "wb").write(rv.ips(p))
        print(f"{ver}: {desc}")
