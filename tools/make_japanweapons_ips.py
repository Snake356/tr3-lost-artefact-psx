"""Builds bin/patches/<version>/japanweapons.ips: Lara's weapons as strong as in the Japanese Disc 1.

The Japanese and Asian Disc 1 executables double the damage of all eleven entries of the weapon table
when a level starts (and halve it again for a demo), and the rocket and grenade explosions hit with 60
and 40 instead of 30 and 20. Every other version (also the Japanese and Asian Disc 2) has the normal
values. This optional patch gives those versions the same: the damage bytes of the weapon table are
doubled in place (0, 1, 21, 1, 3, 4, 30, 20, 6, 0, 3 -> 0, 2, 42, 2, 6, 8, 60, 40, 12, 0, 6) and the
two explosion values are set to 60 and 40. The builder applies it when the option is ticked; it is not
in bin/patch.sh.

  weapon table: the damage bytes, 38 bytes apart, found by their values
  rocket:  addiu a2, zero, 30 ; jal HitTarget ; sllv a2, a2, v0
  grenade: move a1, zero ; jal HitTarget ; addiu a2, zero, 20   (the same HitTarget)

Usage: make_japanweapons_ips.py [--versions us,ukrev,...] [--exe VERSION=PATH ...]
(--exe for executables not in bin/orig, e.g. the first releases us10, uk, de, fr, it, sp)"""
import argparse
import os
import struct

HERE = os.path.dirname(os.path.abspath(__file__))
BIN = os.path.join(HERE, "..", "bin")
LOAD = 0x80010000 - 0x800                          # memory address = file offset + LOAD
TABLE = [0, 1, 21, 1, 3, 4, 30, 20, 6, 0, 3]
STRIDE = 0x26
VERSIONS = "us,ukrev,derev,frrev,itrev,sprev,jp2,as2"   # jp and as have it already


def word(d, o):
    return struct.unpack_from("<I", d, o)[0]


def sites(exe):
    """(damage byte offsets, rocket instruction offset, grenade instruction offset)."""
    tab = [o for o in range(0x800, len(exe) - STRIDE * len(TABLE))
           if exe[o + STRIDE] == 1 and [exe[o + STRIDE * k] for k in range(len(TABLE))] == TABLE]
    assert len(tab) == 1, f"weapon table found {len(tab)} times (already patched or a Japanese Disc 1?)"
    rocket, grenade = [], []
    for o in range(0x800, len(exe) - 12, 4):
        a, j, s = word(exe, o), word(exe, o + 4), word(exe, o + 8)
        if a == 0x2406001E and j >> 26 == 3 and s == 0x00463004:            # addiu a2,zero,30; jal; sllv a2,a2,v0
            rocket.append((o, j))
        if a == 0x00002821 and j >> 26 == 3 and s == 0x24060014:            # move a1,zero; jal; addiu a2,zero,20
            grenade.append((o + 8, j))
    assert len(rocket) == 1, f"rocket explosion found {len(rocket)} times"
    grenade = [g for g in grenade if g[1] == rocket[0][1]]                  # the same HitTarget call
    assert len(grenade) == 1, f"grenade explosion found {len(grenade)} times"
    return [tab[0] + STRIDE * k for k in range(len(TABLE))], rocket[0][0], grenade[0][0]


def ips(exe):
    table, rocket, grenade = sites(exe)
    recs = [(o, bytes([exe[o] * 2])) for o in table if exe[o]]
    recs += [(rocket, struct.pack("<I", 0x2406003C)), (grenade, struct.pack("<I", 0x24060028))]
    out = b"PATCH"
    for off, data in recs:
        out += off.to_bytes(3, "big") + len(data).to_bytes(2, "big") + data
    return out + b"EOF", table[0], rocket, grenade


if __name__ == "__main__":
    ap = argparse.ArgumentParser()
    ap.add_argument("--versions", default=VERSIONS)
    ap.add_argument("--exe", nargs="*", default=[], help="VERSION=PATH for executables not in bin/orig")
    a = ap.parse_args()
    paths = {v: os.path.join(BIN, "orig", f"{v}.exe") for v in a.versions.split(",") if v}
    paths.update(dict(e.split("=", 1) for e in a.exe))
    for ver, path in paths.items():
        data, tab, rocket, grenade = ips(open(path, "rb").read())
        open(os.path.join(BIN, "patches", ver, "japanweapons.ips"), "wb").write(data)
        print(f"{ver}: weapon table {tab + LOAD:#x}, rocket {rocket + LOAD:#x}, grenade {grenade + LOAD:#x}")
