"""Builds bin/patches/<version>/lotslots4.ips: 4 instead of 5 creature slots with pathfinding.

Every slot gets a pathfinding block of (number of boxes * 8) bytes from the level memory when a level
starts; Highland Fling (1396 boxes) has not enough memory left for the fifth (the guard in room 19 then
writes its pathfinding to address 0, over the BIOS tables, and the game crashes: the 3D pickups, the
crawl flip and its animation took about 3.8 KB of it, and Highland Fling had only about 1 KB to spare).
With 4 slots a fifth active creature takes the slot of the one farthest away, as the game always does
when more creatures are active than it has slots; tested in the Highland Fling courtyard with 14
creatures awake (the slots changed every few seconds, nothing to notice).

Found in every version: the slot setup ('addiu a0, zero, 0x17c' = 5 slots of 0x4c bytes, followed by
the slot table 'sw v0, slots(gp)'), then every 'slti rX, rY, 5' and 'addiu rX, zero, 5' within 0x100
bytes of a use of the slot table.

Usage: make_lotslots_ips.py [--versions us,ukrev,...] [--exe VERSION=PATH ...]"""
import argparse
import os
import struct

import make_reunionview_ips as rv

HERE = os.path.dirname(os.path.abspath(__file__))
BIN = os.path.join(HERE, "..", "bin")
LOAD = rv.LOAD
SLOTS = 4


def word(d, o):
    return struct.unpack_from("<I", d, o)[0]


def build(exe):
    setup = [o for o in range(0x800, len(exe) - 0x40, 4)
             if word(exe, o) == 0x2404017C and word(exe, o + 4) == 0x24050021]
    assert len(setup) == 1, f"slot setup found {len(setup)} times"
    table = next(word(exe, k) & 0xFFFF for k in range(setup[0], setup[0] + 0x40, 4) if word(exe, k) >> 16 == 0xAF82)
    refs = [o for o in range(0x800, len(exe), 4)
            if (word(exe, o) >> 21) & 31 == 28 and word(exe, o) & 0xFFFF == table and word(exe, o) >> 26 in (0x23, 0x2B)]
    sites = set()
    for r in refs:
        for k in range(r - 0x100, r + 0x100, 4):
            w = word(exe, k)
            if w >> 26 == 0x0A and w & 0xFFFF == 5:                          # slti rX, rY, 5
                sites.add(k)
            if w >> 26 == 0x09 and w & 0xFFFF == 5 and (w >> 21) & 31 == 0:  # addiu rX, zero, 5
                sites.add(k)
    patches = [(setup[0], [0x24040000 | SLOTS * 0x4C])] + [(k, [word(exe, k) & 0xFFFF0000 | SLOTS]) for k in sorted(sites)]
    return patches, f"slot table gp+{table:#x}, {len(sites)} places: " + " ".join(hex(k + LOAD) for k in sorted(sites))


if __name__ == "__main__":
    ap = argparse.ArgumentParser()
    ap.add_argument("--versions", default="us,ukrev,derev,frrev,itrev,sprev,jp,jp2,as,as2")
    ap.add_argument("--exe", nargs="*", default=[], help="VERSION=PATH for executables not in bin/orig")
    a = ap.parse_args()
    paths = {v: os.path.join(BIN, "orig", f"{v}.exe") for v in a.versions.split(",") if v}
    paths.update(dict(e.split("=", 1) for e in a.exe))
    for ver, path in paths.items():
        p, desc = build(open(path, "rb").read())
        open(os.path.join(BIN, "patches", ver, "lotslots4.ips"), "wb").write(rv.ips(p))
        print(f"{ver}: {desc}")
