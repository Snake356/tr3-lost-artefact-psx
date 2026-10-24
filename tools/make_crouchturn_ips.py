"""Builds bin/patches/<version>/crouchturn.ips: turning on the spot in the crouch, as in Tomb Raider
IV. In the released game Left/Right do nothing in the crouch (state 71); only the crawl turns.

The animations are added to every level as animations 314 (left) and 315 (right) (tools/add_crouch_turn.py,
frames 32100-32124 and 32125-32149, state AS_DUCK, each loops into itself; Lara's home, the released
HOUSE.PSX, by tools/psx_lara_anims.py). The control routine of the
crouch (state 71 of Lara's control table; the roll, 72, shares it and keeps it) becomes a routine that
calls it first and then, with Lara as the item:

  - in the crouch animation (222) with the goal state still the crouch, Duck held or the ceiling too low to
    stand up (keep_ducked in Lara's flag word, as the game's own crouch tests), neither Forward nor Back and
    Left or Right held: starts the turn animation of that side,
  - in a turn animation: turns her 273 per frame (1.5 degrees, as the crawl turns) while its side and Duck
    (or the low ceiling) stay, neither Forward nor Back is pressed and the goal state is still the crouch; otherwise puts
    her back into the crouch animation (frame 6487 in every level), whose state changes then take over
    (crawl, roll, standing up; dying sets the goal state to the crawl).

Found in every version: Lara's state tables (make_duckroll_ips.state_tables), the input word
(make_crawlflip_ips.sites). The routine goes behind the
crawl flip's routine into the block pickup3d.ips appends to the executable, which the start-up copies to
the end of the level memory; the table entry holds its address as the table does (without 0x80000000).

Usage: make_crouchturn_ips.py [--versions us,ukrev,...] [--exe VERSION=PATH ...]"""
import argparse
import os
import struct

import make_crawlflip_ips as cf
import make_duckroll_ips as dr
import make_pickup3d_ips as p3
import make_reunionview_ips as rv

HERE = os.path.dirname(os.path.abspath(__file__))
BIN = os.path.join(HERE, "..", "bin")
LOAD = rv.LOAD

DUCK_ANIM, DUCK_BASE = 222, 6487                   # crouch animation and its first frame
TURN_LEFT, TURN_RIGHT = 314, 315                   # tools/add_crouch_turn.py
TURN_BASE, TURN_FRAMES = 32100, 25
TURN_SPEED = 273
IN_FORWARD_BACK, IN_LEFT, IN_RIGHT = 3, 4, 8
IN_DUCK = 0x20000000
KEEP_DUCKED = 0x20                                 # in Lara's flag word: the ceiling is too low to stand up


def word(d, o):
    return struct.unpack_from("<I", d, o)[0]


def lara_flags(orig, duck_fn):
    """gp offset of Lara's flag word: the crouch routine sets IsDucked in it ('lw v0, x(gp)', 'ori v0, v0, 0x400');
    keep_ducked (KEEP_DUCKED) is in the same word."""
    o = duck_fn - LOAD
    hits = [word(orig, k) & 0xFFFF for k in range(o, o + 0x40, 4)
            if word(orig, k) >> 16 == 0x8F82 and 0x34420400 in (word(orig, k + 4), word(orig, k + 8))]
    assert len(hits) == 1, f"Lara's flags found {len(hits)} times"
    return hits[0]


def routine(duck_fn, inp, flags):
    a = p3.Asm()
    a.addiu("sp", "sp", -24); a.sw("ra", 20, "sp"); a.sw("s0", 16, "sp")
    a.jal(duck_fn); a.move("s0", "a0")                            # the crouch routine (item, coll)
    a.lh("t0", 0x14, "s0"); a.lw("t1", inp, "gp"); a.lw("v1", flags, "gp")
    a.lh("t2", 0x10, "s0")
    a.addiu("t4", "zero", 71)
    a.lui("t9", IN_DUCK >> 16); a.r(0x24, "t9", "t1", "t9")      # Duck held
    a.andi("v1", "v1", KEEP_DUCKED); a.r(0x25, "t9", "t9", "v1")  # t9: or she cannot stand up (a crawlspace)
    a.addiu("t3", "zero", TURN_LEFT); a.addiu("t7", "zero", IN_LEFT)
    a.beq("t0", "t3", "turning"); a.addiu("t6", "zero", -TURN_SPEED)
    a.addiu("t3", "zero", TURN_RIGHT); a.addiu("t7", "zero", IN_RIGHT)
    a.beq("t0", "t3", "turning"); a.addiu("t6", "zero", TURN_SPEED)
    a.addiu("t3", "zero", DUCK_ANIM)
    a.bne("t0", "t3", "out"); a.addiu("t0", "zero", TURN_LEFT)   # crouch animation
    a.bne("t2", "t4", "out"); a.addiu("t8", "zero", TURN_BASE)   # goal state still the crouch
    a.beqz("t9", "out"); a.andi("t5", "t1", IN_FORWARD_BACK)
    a.bnez("t5", "out"); a.andi("t5", "t1", IN_LEFT)
    a.bnez("t5", "start"); a.addiu("t6", "zero", -TURN_SPEED)
    a.andi("t5", "t1", IN_RIGHT)
    a.beqz("t5", "out"); a.addiu("t6", "zero", TURN_SPEED)
    a.addiu("t0", "zero", TURN_RIGHT); a.addiu("t8", "zero", TURN_BASE + TURN_FRAMES)
    a.label("start")
    a.sh("t0", 0x14, "s0"); a.bra("rotate"); a.sh("t8", 0x16, "s0")
    a.label("turning")
    a.bne("t2", "t4", "back"); a.andi("t5", "t1", IN_FORWARD_BACK)
    a.bnez("t5", "back"); a.r(0x24, "t5", "t1", "t7")
    a.beqz("t5", "back"); a.nop()                                 # its side held
    a.beqz("t9", "back"); a.nop()
    a.label("rotate")
    a.lhu("t5", 0x4E, "s0"); a.nop()
    a.addu("t5", "t5", "t6")
    a.bra("out"); a.sh("t5", 0x4E, "s0")
    a.label("back")
    a.addiu("t0", "zero", DUCK_ANIM); a.sh("t0", 0x14, "s0")
    a.addiu("t0", "zero", DUCK_BASE); a.sh("t0", 0x16, "s0")
    a.label("out")
    a.lw("ra", 20, "sp"); a.lw("s0", 16, "sp")                   # ra two words before the jr
    a.jr("ra"); a.addiu("sp", "sp", 24)
    return a.code()


def appended_after(orig, ver):
    """File offset and address of the first free bytes behind this patch's routine, and the block's end."""
    off, addr, end = cf.appended_after(orig, ver)
    ctl, _ = dr.state_tables(orig)
    n = (4 * len(routine(word(orig, ctl + 4 * dr.DUCK) + 0x80000000, cf.sites(orig)[1], 0)) + 15) & ~15
    return off + n, addr + n, end


def build(orig, ver):
    ctl, _ = dr.state_tables(orig)
    duck_fn = word(orig, ctl + 4 * dr.DUCK) + 0x80000000
    _, inp, _, _, _ = cf.sites(orig)
    off, addr, end = cf.appended_after(orig, ver)
    code = routine(duck_fn, inp, lara_flags(orig, duck_fn))
    assert off + 4 * len(code) <= end, "no room left in the appended block"
    patches = [(off, code), (ctl + 4 * dr.DUCK, [addr - 0x80000000])]
    return patches, (f"crouch control {ctl + 4 * dr.DUCK + LOAD:#x}: {duck_fn:#x} -> {addr:#x} "
                     f"({4 * len(code)} bytes, {end - off - 4 * len(code)} left)")


if __name__ == "__main__":
    ap = argparse.ArgumentParser()
    ap.add_argument("--versions", default="us,ukrev,derev,frrev,itrev,sprev,jp,jp2,as,as2")
    ap.add_argument("--exe", nargs="*", default=[], help="VERSION=PATH for executables not in bin/orig")
    a = ap.parse_args()
    paths = {v: os.path.join(BIN, "orig", f"{v}.exe") for v in a.versions.split(",") if v}
    paths.update(dict(e.split("=", 1) for e in a.exe))
    for ver, path in paths.items():
        p, desc = build(open(path, "rb").read(), ver)
        open(os.path.join(BIN, "patches", ver, "crouchturn.ips"), "wb").write(rv.ips(p))
        print(f"{ver}: {desc}")
