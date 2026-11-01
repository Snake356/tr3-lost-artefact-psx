"""Builds bin/patches/<version>/soundfix.ips: two sound behaviours after TRX.

Animation sounds of items in water rooms: AnimateItem plays a sound command of an item in an underwater room with
flags 0, which SoundEffect drops while the camera is under water (doors, trapdoors, animating objects stay silent
when Lara dives; an object in a water room is silent when the camera is above water). TRX
(audio.enable_underwater_anim_sfx) plays them as underwater sounds and near the surface as always audible ones.
Here the water branch goes to the game's own call with flags 2 (always audible, as for the harpoon gun), so such an
item is heard from either side of the surface; land-only commands in a water room are still left out.
  'andi v0, v0, 1; beqz v0, +5; andi a0, a2, 0xffff; beqz a0, +7; ori v0, zero, 0x8000; j land_test; nop;
   beqz a0, +3; addiu v0, zero, 0x4000' (the room's water flag, then the command's land/water bits):
   'beqz a0, +7' -> 'beqz a0, always'; 'j land_test' -> 'beq a0, v0, always'; its delay slot 'nop' -> 'ori a0, a0, 1'
   (a land-only command falls on to the land test and fails it)
  always: 'move a0, a1; addiu a1, s1, 0x40; j call; addiu a2, zero, 2', 40 bytes before (the harpoon gun's call)

Ceiling spikes (TRX spike_ceiling.c): ControlCeilingSpikes animates the drill, and so plays its drill loops, only
when the trigger's timer was 1 (item_flags[0], which also doubles the speed); TRX animates it whenever it moves.
  'lh v1, 0x34(s0); nop; bne v1, v0, +3; nop; jal AnimateItem; move a0, s0': the bne -> nop

Usage: make_soundfix_ips.py [--versions us,ukrev,...] [--exe VERSION=PATH ...]
(--exe for executables not in bin/orig, e.g. the first releases us10, uk, de, fr, it, sp)"""
import argparse
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
from make_japancrystals_ips import patched, BIN, LOAD   # noqa: E402

VERSIONS = "us,ukrev,derev,frrev,itrev,sprev,jp,jp2,as,as2"


def word(exe, o):
    return struct.unpack_from("<I", exe, o)[0]


def sites(exe):
    n = len(exe) - 64
    water = [o for o in range(0x800 + 0x28, n, 4)
             if word(exe, o) == 0x30420001 and word(exe, o + 4) == 0x10400005 and word(exe, o + 8) == 0x30C4FFFF
             and word(exe, o + 12) == 0x10800007 and word(exe, o + 16) == 0x34028000 and word(exe, o + 20) >> 26 == 2
             and word(exe, o + 24) == 0 and word(exe, o + 28) == 0x10800003 and word(exe, o + 32) == 0x24024000
             and word(exe, o - 0x28) == 0x00A02021 and word(exe, o - 0x24) == 0x26250040
             and word(exe, o - 0x20) >> 26 == 2 and word(exe, o - 0x1C) == 0x24060002]
    assert len(water) == 1, f"AnimateItem's sound branch found {len(water)} times"
    drill = [o for o in range(0x800, n, 4)
             if word(exe, o) == 0x86030034 and word(exe, o + 4) == 0 and word(exe, o + 8) == 0x14620003
             and word(exe, o + 12) == 0 and word(exe, o + 16) >> 26 == 3 and word(exe, o + 20) == 0x02002021]
    assert len(drill) == 1, f"ControlCeilingSpikes' animation test found {len(drill)} times"
    return water[0], drill[0]


def ips(exe):
    w, d = sites(exe)
    recs = [(w + 12, struct.pack("<I", 0x1080FFF2)),                       # beqz a0, w - 0x28
            (w + 20, struct.pack("<2I", 0x1082FFF0, 0x34840001)),          # beq a0, v0, w - 0x28; ori a0, a0, 1
            (d + 8, bytes(4))]
    out = b"PATCH"
    for off, data in recs:
        out += off.to_bytes(3, "big") + len(data).to_bytes(2, "big") + data
    return out + b"EOF", w, d


if __name__ == "__main__":
    ap = argparse.ArgumentParser()
    ap.add_argument("--versions", default=VERSIONS)
    ap.add_argument("--exe", nargs="*", default=[], help="VERSION=PATH for executables not in bin/orig")
    a = ap.parse_args()
    paths = {v: os.path.join(BIN, "orig", f"{v}.exe") for v in a.versions.split(",") if v}
    paths.update(dict(e.split("=", 1) for e in a.exe))
    for ver, path in paths.items():
        data, w, d = ips(open(path, "rb").read())
        assert sites(patched(path, ver)) == (w, d), "the other patches touch or move the sites"
        open(os.path.join(BIN, "patches", ver, "soundfix.ips"), "wb").write(data)
        print(f"{ver}: water sounds {w + LOAD:#x}, drill {d + LOAD:#x}")
