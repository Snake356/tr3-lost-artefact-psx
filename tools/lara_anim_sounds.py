"""Lara's animation sounds as TRX's lara_animations.bin has them, where Lara's animations call a sound no Lost
Artefact level (nor Lara's home) has. Each command is changed in place (frame and sound; the land/water bits stay),
so the command lists keep their size:
  run, walk, back, side steps, run-jump start (1, 21, 40, 57, 58, 73): the water-only step called sound 20
    (SFX_ICKET_BARRIER), now SFX_LARA_WET_FEET (17) on the frames of the dry steps
  shimmying left/right (136, 137): SFX_LARA_SHIMMY (28) -> SFX_LARA_GRABHAND (54)
  fast turn (44): the water-only SFX_LARA_SWIM (35) on frame 0 -> SFX_LARA_FLOATING (52) on frame 9
  hanging round an inner corner (326, 328, TRX's corner shimmying, which still has SFX_LARA_SHIMMY): -> GRABHAND
A Tomb Raider III PC level (.tr2, moveables of 18 bytes; after the animations are added) or a PSX level (Lara's
home, 20 bytes; after tools/psx_lara_anims.py). Commands already changed are left as they are.
Usage: lara_anim_sounds.py level.tr2 | house.psx"""
import os
import struct
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import drop_object                     # noqa: E402

# animation: [((frame, sound), (frame, sound)), ...], frames counted from the animation's first frame
CHANGES = {
    1: [((0, 20), (4, 17)), ((18, 20), (24, 17))],
    21: [((13, 20), (6, 17))],
    40: [((10, 20), (12, 17)), ((46, 20), (42, 17))],
    57: [((10, 20), (2, 17))],
    58: [((12, 20), (5, 17))],
    73: [((8, 20), (3, 17))],
    136: [((11, 28), (11, 54)), ((15, 28), (15, 54))],
    137: [((12, 28), (12, 54)), ((15, 28), (15, 54))],
    44: [((0, 35), (9, 52))],
    326: [((20, 28), (20, 54))],
    328: [((20, 28), (20, 54))],
}


def patch_bytes(d, s, model):
    d = bytearray(d)
    u = lambda f, o: struct.unpack_from(f, d, o)
    first = next(u("<H", s["moveables"] + 4 + model * i + 16)[0] for i in range(u("<I", s["moveables"])[0])
                 if u("<I", s["moveables"] + 4 + model * i)[0] == 0)
    done = left = 0
    for a, pairs in CHANGES.items():
        rec = s["anims"] + 4 + 32 * (first + a)
        fstart, ncmd, cofs = u("<H", rec + 16)[0], u("<H", rec + 28)[0], u("<H", rec + 30)[0]
        for (of, osid), (nf, nsid) in pairs:
            p = s["commands"] + 4 + 2 * cofs
            hit = False
            for _ in range(ncmd):
                t = u("<H", p)[0]
                p += 2
                if t == 1:
                    p += 6
                elif t == 2:
                    p += 4
                elif t in (5, 6):
                    f, v = u("<HH", p)
                    if t == 5 and f == fstart + of and v & 0x3FFF == osid:
                        struct.pack_into("<HH", d, p, fstart + nf, (v & 0xC000) | nsid)
                        hit = True
                        break
                    p += 4
            done += hit
            left += not hit
    return bytes(d), done, left


def patch(path):
    d = open(path, "rb").read()
    if path.lower().endswith(".psx"):
        import psx_lara_anims
        s, model = psx_lara_anims.sections(d, path), psx_lara_anims.MODEL
    else:
        s, model = drop_object.sections(d), 18
    out, done, left = patch_bytes(d, s, model)
    open(path, "wb").write(out)
    return "%d commands changed, %d not found" % (done, left)


if __name__ == "__main__":
    print(sys.argv[1], patch(sys.argv[1]))
