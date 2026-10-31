"""Builds bin/patches/<version>/ledgejump.ips: TRX's ledge jumps. Hanging from a ledge, Jump with Forward makes
Lara jump up, Jump with Back makes her push off and jump backwards (animations 318 to 321, tools/add_ledge_jumps.py
and tools/psx_lara_anims.py; the hang's state changes to AS_UPJUMP and AS_BACKJUMP start them).

The hang's collision routine (lara_collision_routines[10], US 0x80017384) reads the input word behind LaraHangTest
and its goal state test ('lw a0, input(gp); nop'); that load becomes a call of a routine that returns the input word
and, as TRX's M_TestLedgeJump, when

  - Jump is held with Forward or Back (not both), and not Forward with Walk (that is the handstand),
  - Lara is not climbing (lara.climb_status, 'lh v0, climb(gp)' further down in the routine),
  - the ceiling over her (coll->mid_ceiling, from LaraHangTest) is more than 858 (up) or 442 (back) above her head,
  - she has a solid surface in front to push against: 256 ahead (the sine table, 4.12) at her own height y, in her
    room (GetFloor, GetHeight, GetCeiling), the floor is above y (NO_HEIGHT, -32512, counts as above any y) or
    the ceiling at most 240 above y,

sets the goal state to AS_UPJUMP (28, Forward) or AS_BACKJUMP (25, Back) and leaves the collision routine through
its end, as TRX returns there.

Found in every version: the routine by a window of the US one (the input load, the 'bne v1, v0' to its end before
it, the climb status load behind it), GetFloor and GetHeight as the calls of LaraFloorFront, GetCeiling as the
second call of LaraCeilingFront (make_crawlflip_ips.sites), the sine table (make_crawlflip_ips.sin_table).
The routine (280 bytes) goes behind the routines of the block pickup3d.ips appends to the executable (copied at
start-up to the end of the level memory) where it has room for it, else into the smallest unused game function
large enough. The block has room for it in every version (make_pickup3d_ips.EXTRA, EXTRA_US).

Usage: make_ledgejump_ips.py [--versions us,ukrev,...] [--exe VERSION=PATH ...]"""
import argparse
import os
import struct

import make_crawlflip_ips as cf
import make_fixes_ips as fx
import make_pickup3d_ips as p3
import make_reunionview_ips as rv

HERE = os.path.dirname(os.path.abspath(__file__))
BIN = os.path.join(HERE, "..", "bin")
LOAD = rv.LOAD
NAME = "ledgejump"

IN_FORWARD, IN_BACK, IN_JUMP, IN_WALK = 0x01, 0x02, 0x10, 0x80
UP_HEIGHT, BACK_HEIGHT = 858, 442                     # LARA_HEIGHT + 3/8 step, LARA_HEIGHT - 5/4 step
PUSH_HEIGHT = 240                                     # STEP_L - 16
FRONT = 256
AS_UPJUMP, AS_BACKJUMP = 28, 25
HANG_SITE = 0x800173B0                                # US: 'lw a0, input(gp)' in lara_col_hang


def word(d, o):
    return struct.unpack_from("<I", d, o)[0]


def s16(x):
    return (x & 0xFFFF) - (0x10000 if x & 0x8000 else 0)


def patches_before(ver):
    """The names of bin/patch.sh before this one."""
    sh = open(os.path.join(BIN, "patch.sh"), encoding="utf-8").read()
    names = [l.split('"')[1].split(",") for l in sh.splitlines() if l.startswith("patches=")][0]
    return names[:names.index(NAME)] if NAME in names else names


def patches_after(ver):
    """The names of bin/patch.sh behind this one."""
    sh = open(os.path.join(BIN, "patch.sh"), encoding="utf-8").read()
    names = [l.split('"')[1].split(",") for l in sh.splitlines() if l.startswith("patches=")][0]
    return names[names.index(NAME) + 1:] if NAME in names else []


def patched(path, ver):
    exe = open(path, "rb").read()
    for name in patches_before(ver):
        exe = rv.apply_ips(exe, open(os.path.join(BIN, "patches", ver, f"{name}.ips"), "rb").read())
    return exe


def sites(us, exe, orig):
    """The input load, the routine's end, the climb status's and the input's gp offsets, GetFloor, GetHeight,
    GetCeiling and the sine table."""
    site = exe.window(us, HANG_SITE, 8, 8)
    br = exe.w(site - 8)
    end = site - 4 + (s16(br) << 2)
    climb = exe.w(site + 0x9C)
    assert exe.w(site) >> 16 == 0x8F84 and exe.w(site + 4) == 0 and br >> 16 == 0x1462 \
        and exe.w(end) == 0x8FBF0018 and climb >> 16 == 0x8782, "lara_col_hang"
    _, inp, floor_front, ceiling_front, _ = cf.sites(orig)
    assert exe.w(site) & 0xFFFF == inp
    calls = lambda f, n: [p3.jal_target(word(orig, k)) for k in range(f - LOAD, f - LOAD + 0xC0, 4)
                          if word(orig, k) >> 26 == 3][:n]
    get_floor, get_height = calls(floor_front, 2)
    get_floor2, get_ceiling = calls(ceiling_front, 2)
    assert get_floor == get_floor2
    return dict(site=site, end=end, climb=climb & 0xFFFF, input=inp, get_floor=get_floor, get_height=get_height,
                get_ceiling=get_ceiling, sin=cf.sin_table(orig, floor_front))


def routine(f):
    """The routine as p3.Asm (s0 = coll, s1 = Lara's item; returns the input word in a0)."""
    a = p3.Asm()
    a.lw("a0", f["input"], "gp"); a.lh("t0", f["climb"], "gp")
    a.andi("t1", "a0", IN_JUMP)
    a.beqz("t1", "out"); a.andi("t2", "a0", IN_FORWARD | IN_BACK)
    a.addiu("t2", "t2", -1); a.sltiu("t2", "t2", 2)              # Forward or Back, not both
    a.beqz("t2", "out"); a.andi("t3", "a0", IN_FORWARD | IN_WALK)
    a.addiu("t3", "t3", -(IN_FORWARD | IN_WALK))
    a.beqz("t3", "out"); a.lw("t4", 4, "s0")                     # coll->mid_ceiling
    a.bnez("t0", "out"); a.andi("t1", "a0", IN_FORWARD)
    a.beqz("t1", "high"); a.addiu("t5", "t4", BACK_HEIGHT)
    a.addiu("t5", "t4", UP_HEIGHT)
    a.label("high")
    a.b(0x01, "t5", "at", "out"); a.lhu("t0", 0x4E, "s1")         # bgez: no room above
    a.addiu("sp", "sp", -40); a.sw("ra", 36, "sp")
    a.r(0x02, "t0", "zero", "t0", 2); a.andi("t0", "t0", 0x3FFC)    # sine table index
    assert p3.hi(f["sin"]) == p3.hi(f["sin"] + 2)
    a.lui("at", p3.hi(f["sin"])); a.addu("at", "at", "t0")
    a.lh("t1", p3.lo(f["sin"]), "at"); a.lh("t2", p3.lo(f["sin"] + 2), "at")
    a.lw("a0", 0x40, "s1"); a.lw("a2", 0x48, "s1")
    a.r(0x03, "t1", "zero", "t1", 4); a.r(0x03, "t2", "zero", "t2", 4)  # sra: FRONT * sin >> 12
    a.addu("a0", "a0", "t1"); a.addu("a2", "a2", "t2")
    a.lhu("t3", 0x18, "s1")
    a.sw("a0", 16, "sp"); a.sw("a2", 20, "sp"); a.sh("t3", 32, "sp")
    a.lw("a1", 0x44, "s1")
    a.jal(f["get_floor"]); a.addiu("a3", "sp", 32)
    a.sw("v0", 28, "sp"); a.lw("a1", 16, "sp"); a.lw("a2", 0x44, "s1"); a.lw("a3", 20, "sp")
    a.jal(f["get_height"]); a.move("a0", "v0")
    a.lw("t0", 0x44, "s1"); a.lw("a1", 16, "sp")
    a.slt("t1", "v0", "t0")                                      # the floor above her (or NO_HEIGHT): push
    a.bnez("t1", "done"); a.lw("a0", 28, "sp")
    a.lw("a2", 0x44, "s1"); a.lw("a3", 20, "sp")
    a.jal(f["get_ceiling"]); a.nop()
    a.lw("t0", 0x44, "s1"); a.addiu("v0", "v0", PUSH_HEIGHT + 1)
    a.slt("t1", "t0", "v0")                                      # the ceiling at most 240 above her
    a.label("done")
    a.lw("a0", f["input"], "gp"); a.lw("ra", 36, "sp")
    a.beqz("t1", "out"); a.addiu("sp", "sp", 40)
    a.andi("t0", "a0", IN_FORWARD)
    a.beqz("t0", "set"); a.addiu("t1", "zero", AS_BACKJUMP)
    a.addiu("t1", "zero", AS_UPJUMP)
    a.label("set")
    a.j(f["end"]); a.sh("t1", 0x10, "s1")
    a.label("out")
    a.jr("ra"); a.nop()
    assert FRONT == 1 << 8
    return a


def block_room(orig, ver):
    """File offset and address of the first free bytes in the block pickup3d.ips appends (behind everything the
    patches of this version write there, the language variants of ngpchoice.ips too, not those of the patches
    behind this one in bin/patch.sh), and the block's end."""
    n = len(orig)
    extra = p3.extra(ver)
    pk = rv.apply_ips(orig, open(os.path.join(BIN, "patches", ver, "pickup3d.ips"), "rb").read())
    used = n + ((len(pk[n:n + extra].rstrip(b"\x00")) + 32 + 15) & ~15)
    folder = os.path.join(BIN, "patches", ver)
    later = [f"{name}.ips" for name in patches_after(ver)]
    for f in sorted(os.listdir(folder)):
        if f.endswith(".ips") and f not in ["pickup3d.ips", f"{NAME}.ips"] + later:
            for s, e in p3.ips_ranges(open(os.path.join(folder, f), "rb").read()):
                if s >= n:
                    used = max(used, (e + 15) & ~15)
    _, size, buffer = p3.level_memory(orig)
    return used, buffer + size - extra + used - n, n + extra


def function_room(orig, ver):
    """Unused game functions without the bytes any patch of this version writes (and 16 behind each),
    largest first: [(file offset, end)]."""
    taken = []
    folder = os.path.join(BIN, "patches", ver)
    for f in sorted(os.listdir(folder)):
        if f.endswith(".ips") and f != f"{NAME}.ips":
            taken += p3.ips_ranges(open(os.path.join(folder, f), "rb").read())
    out = []
    for s, e in rv.unused_routines(rv.words(orig), ver):
        pieces = [(s, e)]
        for ts, te in taken:
            ts, te = ts & ~3, (te + 16 + 3) & ~3
            pieces = [p for x, y in pieces for p in ((x, min(y, ts)), (max(x, te), y)) if p[1] - p[0] >= 16]
        out += pieces
    return sorted(out, key=lambda r: r[0] - r[1])


def build(us, orig, data, ver):
    exe = fx.Exe(data)
    f = sites(us, exe, orig)
    size = 4 * len(routine(f).items)
    off, addr, end = block_room(orig, ver)
    where = "block"
    if end - off < size:
        fits = [r for r in function_room(orig, ver) if r[1] - r[0] >= size]
        assert fits, "no room for the routine"
        off, end = min(fits, key=lambda r: r[1] - r[0])
        addr, where = off + LOAD, "unused function"
    code = routine(f).code()
    patches = [(off, code), (f["site"] - LOAD, [rv.jal(addr), 0])]
    return patches, (f"hang collision {f['site']:#x} -> {addr:#x} ({where}, {4 * len(code)} bytes, "
                     f"{end - off - 4 * len(code)} left), end {f['end']:#x}, climb gp+{f['climb']:#x}, "
                     f"input gp+{f['input']:#x}, GetFloor {f['get_floor']:#x}, GetHeight {f['get_height']:#x}, "
                     f"GetCeiling {f['get_ceiling']:#x}, sine {f['sin']:#x}")


if __name__ == "__main__":
    ap = argparse.ArgumentParser()
    ap.add_argument("--versions", default="us,ukrev,derev,frrev,itrev,sprev,jp,jp2,as,as2")
    ap.add_argument("--exe", nargs="*", default=[], help="VERSION=PATH for executables not in bin/orig")
    a = ap.parse_args()
    paths = {v: os.path.join(BIN, "orig", f"{v}.exe") for v in a.versions.split(",") if v}
    paths.update(dict(e.split("=", 1) for e in a.exe))
    us = fx.Exe(patched(os.path.join(BIN, "orig", "us.exe"), "us"))
    for ver, path in paths.items():
        p, desc = build(us, open(path, "rb").read(), patched(path, ver), ver)
        open(os.path.join(BIN, "patches", ver, f"{NAME}.ips"), "wb").write(rv.ips(p))
        print(f"{ver}: {desc}")
