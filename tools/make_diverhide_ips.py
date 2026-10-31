"""Builds bin/patches/<version>/diverhide.ips: the animated diver in Sleeping with the Fishes Part 1
(object 119 = FAN slot, PropellerControl) disappears when its "off" animation ends (436 in the original level, 461 since Lara got 25
animations more in 1.0.3-1.0.8: a new Lara animation shifts every later number, so this one has to follow).

That animation ends with the KILL command; the engine only deactivates the item, so the diver
stayed drawn in its last pose. PropellerControl then does "if not SAW: collidable = 0". There are
no saws in any Lost Artefact level, so that check is replaced by one on the diver animation:

    lh    v1, 12(s0)      -> lh    v1, 20(s0)      ; anim_number
    addiu v0, zero, 105   -> lw    v0, 132(s0)     ; item flags
    beq   v1, v0, +5      -> addiu a0, zero, 461
    addiu v1, zero, -33   -> bne   v1, a0, +2
    lw    v0, 132(s0)     -> addiu a1, zero, -33
    nop                   -> addiu v0, v0, 2       ; only the diver: status DEACTIVATED -> INVISIBLE
    and   v0, v0, v1      -> and   v0, v0, a1      ; collidable = 0 as before
    sw    v0, 132(s0)     -> sw    v0, 132(s0)

Other FAN objects (Shakespeare Cliff, Reunion) behave exactly as before.

Usage: make_diverhide_ips.py [versions, e.g. uk,de for the Rev 0 executables]   (checks that bin/patch.sh's patches rebuild bin/mod exactly first, where it exists)"""
import os
import struct
import sys

BIN = os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "bin")
VERSIONS = ["us", "ukrev", "derev", "frrev", "itrev", "sprev", "jp", "jp2", "as", "as2"]
OLD = [0x8603000C, 0x24020069, None, 0x2403FFDF, 0x8E020084, 0x00000000, 0x00431024, 0xAE020084]  # from "lh v1,12(s0)"
NEW = [0x86030014, 0x8E020084, 0x240401CD, 0x14640002, 0x2405FFDF, 0x24420002, 0x00451024, 0xAE020084]


def apply_ips(data, ips):
    data = bytearray(data)
    assert ips[:5] == b"PATCH"
    p = 5
    while ips[p:p + 3] != b"EOF":
        off = int.from_bytes(ips[p:p + 3], "big"); size = int.from_bytes(ips[p + 3:p + 5], "big"); p += 5
        if size:
            chunk = ips[p:p + size]; p += size
        else:
            rle = int.from_bytes(ips[p:p + 2], "big"); chunk = ips[p + 2:p + 3] * rle; p += 3
        if off + len(chunk) > len(data):
            data.extend(b"\0" * (off + len(chunk) - len(data)))
        data[off:off + len(chunk)] = chunk
    return bytes(data)


def find(exe):
    word = lambda o: struct.unpack_from("<I", exe, o)[0]
    hits = [o for o in range(0x800, len(exe) - 28, 4)
            if all(v is None or word(o + 4 * k) == v for k, v in enumerate(OLD))
            and (word(o + 8) >> 16) == 0x1062]  # beq v1,v0,...
    assert len(hits) == 1, [hex(h) for h in hits]
    return hits[0]


if __name__ == "__main__":
    patch_sh = open(os.path.join(BIN, "patch.sh"), encoding="utf-8").read()
    patches = [l.split('"')[1].split(",") for l in patch_sh.splitlines() if l.startswith("patches=")][0]
    for ver in (sys.argv[1].split(",") if len(sys.argv) > 1 else VERSIONS):   # e.g. Rev 0: uk,de,...
        exe = open(os.path.join(BIN, "orig", f"{ver}.exe"), "rb").read()
        for pat in [p for p in patches if p != "diverhide"]:
            exe = apply_ips(exe, open(os.path.join(BIN, "patches", ver, f"{pat}.ips"), "rb").read())
        mod_path = os.path.join(BIN, "mod", f"{ver}.exe")
        same = exe == open(mod_path, "rb").read() if os.path.exists(mod_path) else None
        o = find(exe)
        ips = b"PATCH" + o.to_bytes(3, "big") + (4 * len(NEW)).to_bytes(2, "big") + struct.pack(f"<{len(NEW)}I", *NEW) + b"EOF"
        open(os.path.join(BIN, "patches", ver, "diverhide.ips"), "wb").write(ips)
        print(f"{ver}: bin/mod gleich Nachbau vorher: {same}; Stelle {hex(o)}")
