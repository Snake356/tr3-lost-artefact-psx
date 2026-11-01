"""Two of Lara's quiet sounds louder, the volume byte of their sound details set (every Lost Artefact level and
Lara's home have the same originals): the swoosh of the flare throw (SFX_GENERIC_SWOOSH, 71: 51 -> 127) and the
hand grip of shimmying and the monkey swing (SFX_LARA_GRABHAND, 54: 63 -> 100). On the PS1 the samples play at
11 kHz and lose their high part, so the original settings are hardly heard.
Shakespeare Cliff keeps one of Lara's four wet-feet steps (SFX_LARA_WET_FEET, 17; samples 022 to 024 are silenced
for sound memory): its sound takes one variant instead of four, so no wet step is silent.
A Tomb Raider III PC level (.tr2) or Lara's home (Core's HOUSE.PSX, whose sound map and details have the PC layout;
the details table is found from sound 71's entry, whose index the map in front of the table holds).
Usage: sound_volumes.py level.tr2 | house.psx"""
import os
import struct
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import drop_object                     # noqa: E402

VOLUMES = {71: 127, 54: 100}
VARIANTS = {"chunnel": {17: 1}}
HOUSE_ENTRY = bytes.fromhex("4000330a00f40440")    # sound 71's details in HOUSE.PSX: sample 64, volume 51


def set_variants(b, smap, det, level):
    out = []
    for sound, n in VARIANTS.get(level, {}).items():
        d = struct.unpack_from("<h", b, smap + sound * 2)[0]
        if d >= 0:
            f = struct.unpack_from("<H", b, det + d * 8 + 6)[0]
            struct.pack_into("<H", b, det + d * 8 + 6, (f & ~0x3C) | n << 2)
            out.append(", %d: %d -> %d variants" % (sound, (f >> 2) & 15, n))
    return "".join(out)


def set_volumes(b, smap, det):
    old = {}
    for sound, volume in VOLUMES.items():
        d = struct.unpack_from("<h", b, smap + sound * 2)[0]
        if d >= 0:
            old[sound] = b[det + d * 8 + 2]
            b[det + d * 8 + 2] = volume
    return ", ".join("%d: %d -> %d" % (s, v, VOLUMES[s]) for s, v in old.items())


def level(b):
    u16 = lambda o: struct.unpack_from("<H", b, o)[0]
    u32 = lambda o: struct.unpack_from("<I", b, o)[0]
    p = drop_object.sections(bytes(b))["items"]
    p += 4 + u32(p) * 24
    p += 8192
    p += 2 + u16(p) * 16
    p += 2 + u16(p)
    return p, p + 370 * 2 + 4


def house(b):
    assert b.count(HOUSE_ENTRY) == 1, "the swoosh's details found %d times" % b.count(HOUSE_ENTRY)
    off = b.find(HOUSE_ENTRY)
    for k in range(400):
        det = off - 8 * k
        smap = det - 4 - 370 * 2
        if struct.unpack_from("<h", b, smap + 71 * 2)[0] == k and k < struct.unpack_from("<I", b, det - 4)[0] < 400:
            return smap, det
    raise AssertionError("sound map not found")


def patch(path):
    b = bytearray(open(path, "rb").read())
    smap, det = house(b) if path.lower().endswith(".psx") else level(b)
    done = set_volumes(b, smap, det) + set_variants(b, smap, det, os.path.basename(path).split(".")[0])
    open(path, "wb").write(b)
    return done


if __name__ == "__main__":
    print(sys.argv[1], patch(sys.argv[1]))
