"""Builds bin/patches/<version>/japancrystals.ips: save crystals as in the Japanese Disc 1.

In the Japanese and Asian Disc 1 a save crystal in a level heals Lara when she walks into it (hit points to 1000,
poison cleared, sound 0x74) instead of going into the inventory, and saving from the crystal in the inventory does
not use it up. Every other version (also the Japanese and Asian Disc 2) takes the crystal into the inventory and
uses one up for each save (giving it back when the save is cancelled). This optional patch gives those versions the
Japanese behaviour; Lara starts level 1 with one crystal (the start info), so she can always save. The builder
applies it when the option is ticked; it is not in bin/patch.sh.

  crystal control (AnimatingPickUp, object 180): 'addiu a0, zero, 0xd4; addiu a1, a1, 0x40; jal SoundEffect;
    move a2, zero; lh a0, 0xc(s0); jal Inv_AddItem; nop' (a1 = Lara) becomes 'addiu v0, zero, 1000; sh v0,
    0x22(a1); sh zero, poison(gp); addiu a0, zero, 0x74; addiu a1, a1, 0x40; jal SoundEffect; move a2, zero'
  saving: 'addiu a0, zero, 0xcc; ...; jal Inv_RemoveItem; sw v0, (s0)' (the call off; then CreateSaveGameInfo)
  cancelled: 'jal Inv_AddItem; addiu a0, zero, 0xcc' (the call off)
  poison: Lara's poison counter, 4 bytes before her flags (make_crawlflip_ips: gun status + 0x2E)

Usage: make_japancrystals_ips.py [--versions us,ukrev,...] [--exe VERSION=PATH ...]
(--exe for executables not in bin/orig, e.g. the first releases us10, uk, de, fr, it, sp)"""
import argparse
import os
import re
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
sys.path.insert(0, os.path.join(HERE, "builder"))
import make_crawlflip_ips as cf     # noqa: E402
from build import apply_ips         # noqa: E402

BIN = os.path.join(HERE, "..", "bin")
LOAD = 0x80010000 - 0x800
VERSIONS = "us,ukrev,derev,frrev,itrev,sprev,jp2,as2"   # jp and as have it already
POISON = 0x2E                                            # gp offset relative to the gun status


def word(d, o):
    return struct.unpack_from("<I", d, o)[0]


def sites(exe):
    n = len(exe) - 32
    pick = [o for o in range(0x800, n, 4)
            if word(exe, o) == 0x240400D4 and word(exe, o + 4) == 0x24A50040 and word(exe, o + 8) >> 26 == 3
            and word(exe, o + 12) == 0x00003021 and word(exe, o + 16) == 0x8604000C and word(exe, o + 20) >> 26 == 3
            and word(exe, o + 24) == 0]
    assert len(pick) == 1, f"crystal control found {len(pick)} times (a Japanese Disc 1?)"
    add = word(exe, pick[0] + 20)                                   # jal Inv_AddItem
    back = [o for o in range(0x800, n, 4) if word(exe, o) == add and word(exe, o + 4) == 0x240400CC]
    assert len(back) == 1, f"crystal given back found {len(back)} times"
    use = [o for o in range(0x800, n, 4) if word(exe, o) == 0x240400CC
           and any(word(exe, o + k) >> 26 == 3 and word(exe, o + k) != add for k in (4, 8, 12, 16))]
    assert len(use) == 1, f"crystal used for saving found {len(use)} times"
    k = next(k for k in (4, 8, 12, 16) if word(exe, use[0] + k) >> 26 == 3 and word(exe, use[0] + k) != add)
    return pick[0], use[0] + k, back[0], word(exe, pick[0] + 8)


def ips(exe):
    pick, use, back, sfx = sites(exe)
    poison = cf.gun_status(exe) + POISON
    code = [0x240203E8, 0xA4A20022, 0xA7800000 | poison, 0x24040074, 0x24A50040, sfx, 0x00003021]
    recs = [(pick, struct.pack("<7I", *code)), (use, bytes(4)), (back, bytes(4))]
    out = b"PATCH"
    for off, data in recs:
        out += off.to_bytes(3, "big") + len(data).to_bytes(2, "big") + data
    return out + b"EOF", pick, use, back, poison


def patched(path, ver):
    """The executable with every patch of bin/patch.sh (the sites must be untouched by them)."""
    exe = open(path, "rb").read()
    sh = open(os.path.join(BIN, "patch.sh"), encoding="utf-8").read()
    for name in re.search(r'^patches="([^"]*)"', sh, re.M).group(1).split(","):
        if name:
            exe = apply_ips(exe, open(os.path.join(BIN, "patches", ver, f"{name}.ips"), "rb").read())
    return exe


if __name__ == "__main__":
    ap = argparse.ArgumentParser()
    ap.add_argument("--versions", default=VERSIONS)
    ap.add_argument("--exe", nargs="*", default=[], help="VERSION=PATH for executables not in bin/orig")
    a = ap.parse_args()
    paths = {v: os.path.join(BIN, "orig", f"{v}.exe") for v in a.versions.split(",") if v}
    paths.update(dict(e.split("=", 1) for e in a.exe))
    for ver, path in paths.items():
        orig = open(path, "rb").read()
        data, pick, use, back, poison = ips(orig)
        full = patched(path, ver)
        assert sites(full)[:3] == (pick, use, back), "the other patches move the sites"
        open(os.path.join(BIN, "patches", ver, "japancrystals.ips"), "wb").write(data)
        print(f"{ver}: crystal {pick + LOAD:#x}, save {use + LOAD:#x}, given back {back + LOAD:#x}, poison gp+{poison:#x}")
