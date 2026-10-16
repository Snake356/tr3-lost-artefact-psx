"""Builds bin/patches/<version>/noicons.ips: the game no longer puts carried-over artefact icons
(ICON_PICKUP1-4_ITEM, object 240-243) back into Lara's inventory at level start.

In The Lost Artefact the Hand of Rathmore is picked up at the end of Sleeping with the Fishes
Part 2 as ICON_PICKUP1. TR3 stores such icons in the level's start info and InitialiseLaraInventory
re-adds them in the next level, but It's a Madhouse! has no model for it, so an
empty "Die Hand von Rathmore" shows up there. Here the four

    lbu  v0, 28..31(sX)      ; start->num_icon1..4
    nop
    beq  v0, zero, +3        ; -> patched to beq zero, zero, +3 (always skip)
    nop
    jal  Inv_AddItem
    addiu a0, zero, 240..243

checks in InitialiseLaraInventory always skip the add. Picking the Hand up in Part 2 still works.

Usage: make_noicons_ips.py [versions, e.g. uk,de for the Rev 0 executables]   (checks that bin/patch.sh's patches rebuild bin/mod exactly first, where it exists)"""
import os
import struct
import sys

BIN = os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "bin")
VERSIONS = ["us", "ukrev", "derev", "frrev", "itrev", "sprev", "jp", "jp2", "as", "as2"]
BEQ_ALWAYS = 0x10000003


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


def find_icon_checks(exe):
    """Offsets of the four 'beq v0,zero,3' in front of jal Inv_AddItem(240..243)."""
    word = lambda o: struct.unpack_from("<I", exe, o)[0]
    hits = []
    for imm in range(240, 244):
        found = []
        for o in range(0x800, len(exe) - 24, 4):
            if word(o + 20) == (0x24040000 | imm) and (word(o + 16) >> 26) == 3 and word(o + 12) == 0 \
                    and word(o + 8) == 0x10400003 and word(o + 4) == 0 and (word(o) >> 26) == 0x24 \
                    and (word(o) >> 16) & 31 == 2 and word(o) & 0xFFFF == 28 + (imm - 240):
                found.append(o + 8)
        assert len(found) == 1, (imm, found)
        hits.append(found[0])
    return hits


patch_sh = open(os.path.join(BIN, "patch.sh"), encoding="utf-8").read()
patches = [l.split('"')[1].split(",") for l in patch_sh.splitlines() if l.startswith("patches=")][0]
for ver in (sys.argv[1].split(",") if len(sys.argv) > 1 else VERSIONS):   # e.g. Rev 0: uk,de,...
    exe = open(os.path.join(BIN, "orig", f"{ver}.exe"), "rb").read()
    for pat in [p for p in patches if p != "noicons"]:
        exe = apply_ips(exe, open(os.path.join(BIN, "patches", ver, f"{pat}.ips"), "rb").read())
    mod_path = os.path.join(BIN, "mod", f"{ver}.exe")
    same = exe == open(mod_path, "rb").read() if os.path.exists(mod_path) else None
    offs = find_icon_checks(exe)
    ips = b"PATCH" + b"".join(o.to_bytes(3, "big") + (4).to_bytes(2, "big") + struct.pack("<I", BEQ_ALWAYS) for o in offs) + b"EOF"
    open(os.path.join(BIN, "patches", ver, "noicons.ips"), "wb").write(ips)
    print(f"{ver}: bin/mod gleich Nachbau vorher: {same}; Stellen {[hex(o) for o in offs]}")
