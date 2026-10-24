"""Builds bin/patches/<version>/ngpfile.ips: the New Game+ keeps its own savegame on the memory card.

Tomb Raider III on the PS1 has one savegame per memory card, a file of two blocks whose name is a text of
the game (bu00:BASLUS-00691TR3G, bu00:BESLES-01649TR3G, ...); saving overwrites it. With this patch a game
in the New Game+ (bonus byte set, see make_newgameplus_ips.py) uses the same name with a P instead of the
last letter, so a normal and a New Game+ savegame can be on one card side by side.

The four places that open the savegame (check the card, save, create, load) all load the name and call
the BIOS function open; these calls go to a routine that, with the bonus byte set, copies the name into
its own buffer, replaces the last letter by P and passes that on, and then jumps to open. Saving in Lara's
home reads the savegame first through the same load routine, so it takes the right file as well.

Found in every version: the BIOS stub open ('addiu t2, zero, 0xb0; jr t2; addiu t1, zero, 0x32') and its
four callers, the bonus byte (make_newgameplus_ips.bonus_byte). The routine goes behind the crouch turn's
routine into the block pickup3d.ips appends to the executable.

Usage: make_ngpfile_ips.py [--versions us,ukrev,...] [--exe VERSION=PATH ...]"""
import argparse
import os
import struct

import make_crouchturn_ips as ct
import make_newgameplus_ips as ngp
import make_pickup3d_ips as p3
import make_reunionview_ips as rv

HERE = os.path.dirname(os.path.abspath(__file__))
BIN = os.path.join(HERE, "..", "bin")
LOAD = rv.LOAD
NAME_BUFFER = 24                       # the longest name, bu00:BESLES-01683TR3G, has 22 bytes
LETTER = ord("P")


def word(d, o):
    return struct.unpack_from("<I", d, o)[0]


def open_calls(exe):
    """Address of the BIOS stub open and the file offsets of its four callers."""
    stubs = [o for o in range(0x800, len(exe) - 12, 4)
             if word(exe, o) == 0x240A00B0 and word(exe, o + 4) == 0x01400008 and word(exe, o + 8) == 0x24090032]
    assert len(stubs) == 1, f"open found {len(stubs)} times"
    stub = stubs[0] + LOAD
    calls = [o for o in range(0x800, len(exe), 4) if word(exe, o) == 0x0C000000 | (stub & 0x0FFFFFFF) >> 2]
    assert len(calls) == 4, f"calls of open found {len(calls)} times"
    return stub, calls


def routine(stub, bonus, buf):
    a = p3.Asm()
    a.lui("t0", (bonus + 0x8000) >> 16)
    a.i(0x24, "t0", "t0", bonus & 0xFFFF)                        # lbu t0, bonus
    a.nop()
    a.beqz("t0", "open"); a.addiu("t4", "zero", LETTER)
    a.lui("t1", (buf + 0x8000) >> 16); a.addiu("t1", "t1", buf & 0xFFFF)
    a.move("t2", "t1")
    a.label("copy")
    a.i(0x24, "a0", "t3", 0)                                     # lbu t3, 0(a0)
    a.addiu("a0", "a0", 1)
    a.i(0x28, "t2", "t3", 0)                                     # sb t3, 0(t2)
    a.bnez("t3", "copy"); a.addiu("t2", "t2", 1)
    a.i(0x28, "t2", "t4", -2)                                    # the last letter (t2 is behind the 0)
    a.move("a0", "t1")
    a.label("open")
    a.j(stub); a.nop()
    return a.code()


def appended_after(orig, ver):
    """File offset and address of the first free bytes behind this patch's routine and buffer, and the block's end."""
    off, addr, end = ct.appended_after(orig, ver)
    stub, _ = open_calls(orig)
    n = (4 * len(routine(stub, ngp.bonus_byte(orig), addr)) + NAME_BUFFER + 15) & ~15
    return off + n, addr + n, end


def name_buffer(orig, ver):
    """Address of the routine's name buffer (ngpchoice.ips's card check uses it too)."""
    _, addr, _ = ct.appended_after(orig, ver)
    stub, _ = open_calls(orig)
    return addr + 4 * len(routine(stub, ngp.bonus_byte(orig), addr))


def build(orig, ver):
    stub, calls = open_calls(orig)
    bonus = ngp.bonus_byte(orig)
    off, addr, end = ct.appended_after(orig, ver)
    code = routine(stub, bonus, 0)
    buf = addr + 4 * len(code)
    code = routine(stub, bonus, buf)
    assert off + 4 * len(code) + NAME_BUFFER <= end, "no room left in the appended block"
    patches = [(off, code + [0] * (NAME_BUFFER // 4))] + [(c, [rv.jal(addr)]) for c in calls]
    return patches, (f"open {stub:#x}, calls " + " ".join(hex(c + LOAD) for c in calls) + f", bonus {bonus:#x} -> "
                     f"{addr:#x} ({4 * len(code)} + {NAME_BUFFER} bytes, {end - off - 4 * len(code) - NAME_BUFFER} left)")


if __name__ == "__main__":
    ap = argparse.ArgumentParser()
    ap.add_argument("--versions", default="us,ukrev,derev,frrev,itrev,sprev,jp,jp2,as,as2")
    ap.add_argument("--exe", nargs="*", default=[], help="VERSION=PATH for executables not in bin/orig")
    a = ap.parse_args()
    paths = {v: os.path.join(BIN, "orig", f"{v}.exe") for v in a.versions.split(",") if v}
    paths.update(dict(e.split("=", 1) for e in a.exe))
    for ver, path in paths.items():
        p, desc = build(open(path, "rb").read(), ver)
        open(os.path.join(BIN, "patches", ver, "ngpfile.ips"), "wb").write(rv.ips(p))
        print(f"{ver}: {desc}")
