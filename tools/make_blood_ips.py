"""Builds bin/patches/<version>/bloodred.ips and bloodpurple.ips: the blood colour regardless of the language.

The German versions show purple blood instead of red. The executable decides it at run time: the two blood
effect routines load the gameflow's language byte (lbu v0,language(gp)), compare it with 2 (German, held in
s3) and take the purple colours when they are equal:
    lbu v0,language(gp) ; nop ; bne v0,s3,red ; addiu v0,zero,0xE0
bloodred.ips turns the bne into an unconditional branch (always red), bloodpurple.ips into a nop (always
purple), in both routines. The code modules do not read the language. Either patch is optional and goes on
top of bin/patch.sh's patches (it touches none of their bytes).

Usage: make_blood_ips.py [--versions us,ukrev,...]"""
import argparse
import os
import struct

HERE = os.path.dirname(os.path.abspath(__file__))
BIN = os.path.join(HERE, "..", "bin")
VERSIONS = "us,ukrev,derev,frrev,itrev,sprev,jp,jp2,as,as2"


def sites(exe):
    """File offsets of the two 'bne v0,s3,red' instructions."""
    W = struct.unpack_from(f"<{(len(exe) - 0x800) // 4}I", exe, 0x800)
    hits = []
    for i in range(len(W) - 3):
        if (W[i] & 0xFFFF0000) == 0x93820000 and W[i + 1] == 0 and (W[i + 2] & 0xFFFF0000) == 0x14530000 \
                and W[i + 3] == 0x240200E0:
            hits.append(i + 2)
    assert len(hits) == 2, f"{len(hits)} blood colour checks found"
    assert W[hits[0] - 2] == W[hits[1] - 2], "the two routines read different bytes"
    for i in hits:                                 # s3 must hold 2 (German) at the compare
        assert any(W[k] == 0x24130002 for k in range(i - 40, i)), "s3 = 2 not found"
    return [(0x800 + 4 * i, W[i]) for i in hits]


def ips(patches):
    out = b"PATCH"
    for off, w in patches:
        out += off.to_bytes(3, "big") + (4).to_bytes(2, "big") + struct.pack("<I", w)
    return out + b"EOF"


if __name__ == "__main__":
    ap = argparse.ArgumentParser()
    ap.add_argument("--versions", default=VERSIONS)
    a = ap.parse_args()
    for ver in a.versions.split(","):
        exe = open(os.path.join(BIN, "orig", f"{ver}.exe"), "rb").read()
        S = sites(exe)
        red = [(off, 0x10000000 | (w & 0xFFFF)) for off, w in S]      # beq zero,zero,red
        purple = [(off, 0) for off, w in S]                          # nop
        for name, p in (("bloodred", red), ("bloodpurple", purple)):
            open(os.path.join(BIN, "patches", ver, f"{name}.ips"), "wb").write(ips(p))
        print(f"{ver}: blood colour checks at {', '.join(hex(o) for o, _ in S)}")
