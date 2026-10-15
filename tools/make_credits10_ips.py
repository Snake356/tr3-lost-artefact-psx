"""Builds bin/patches/<version>/credits10.ips: the end credits show exactly ten pictures,
CREDIT01.RAW - CREDIT10.RAW (Lost Artefact credits 1-8, the PlayStation edition credit 9 and the
thanks picture 10).

The credits routine loads THEEND.RAW, keeps its pointer at sp+0x74, then loads credit%02d.raw for
i = 1..N into the pointer array at sp+0x50 + 4*(i-1) and shows them in a second loop. Both loops end
with 'slti v0, s1, N' (N = 9 in the US/European/Asian executables, 13 in the Japanese ones). With
ten pictures in the 0x88-byte frame the last pointer lands on sp+0x74, whose THEEND pointer has been
used before the loop and is not read again; an eleventh would overwrite the saved s0 at sp+0x78.
The Japanese executables (0x98-byte frame) drop from 13 to 10, so they do not look for CREDIT11-13
(a missing file is not fatal, but leaves an empty/stale picture).

Each picture is shown for a fixed time, 'addiu a1, zero, T' before the call in the display loop:
T = 210 frames on the NTSC discs (us, as, as2), 175 on PAL (ukrev etc., the same 3.5 seconds at
50 Hz) and 150 in the Japanese executables (jp, jp2), which were timed for 13 pictures (13 x 150 is
about 9 x 210). With ten pictures the Japanese credits went by too fast, so jp and jp2 get 210 like
the other NTSC discs.

Usage: make_credits10_ips.py [versions, e.g. uk,de for the Rev 0 executables]"""
import os
import struct
import sys

BIN = os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "bin")
VERSIONS = ["us", "ukrev", "derev", "frrev", "itrev", "sprev", "jp", "jp2", "as", "as2"]
COUNT = 10
NTSC_FRAMES = 210


def loop_counts(exe):
    """File offsets and values of the two 'slti v0, s1, N' of the credits routine."""
    word = lambda o: struct.unpack_from("<I", exe, o)[0]
    s = exe.find(b"pix\\credit%02d.raw")
    lo = (s - 0x800 + 0x80010000) & 0xFFFF
    refs = [o for o in range(0x800, len(exe), 4) if word(o) >> 16 == 0x24A5 and word(o) & 0xFFFF == lo]
    assert len(refs) == 1, refs
    hits = [o for o in range(refs[0], refs[0] + 0xA0, 4) if word(o) >> 16 == 0x2A22]
    assert len(hits) == 2 and word(hits[0]) == word(hits[1]), hits
    n = word(hits[0]) & 0xFFFF
    # frame check: prologue 'addiu sp, sp, -frame' and, for the 9-picture versions, the THEEND
    # pointer at sp+0x74 must not be read again after the load loop
    f = next(o for o in range(refs[0], refs[0] - 0x200, -4) if word(o) >> 16 == 0x27BD and word(o) & 0x8000)
    frame = 0x10000 - (word(f) & 0xFFFF)
    end = next(o for o in range(hits[1], hits[1] + 0x200, 4) if word(o) == 0x03E00008)
    if n == 9:
        assert frame == 0x88 and word(f + 0x14) == 0xAFB00078, (hex(frame), hex(word(f + 0x14)))
        assert not any(word(o) >> 16 in range(0x8FA0, 0x8FC0) and word(o) & 0xFFFF == 0x74
                       for o in range(hits[0], end, 4))
    else:
        assert n == 13 and frame == 0x98, (n, hex(frame))
    # display time per picture: 'addiu a1, zero, T' in the second loop, right after 'lw a0, 0x40(s0)'
    t = [o for o in range(hits[0], hits[1], 4) if word(o) >> 16 == 0x2405 and word(o - 4) == 0x8E040040]
    assert len(t) == 1, t
    return hits, n, t[0], word(t[0]) & 0xFFFF


def ips(records):
    out = b"PATCH"
    for off, data in records:
        out += off.to_bytes(3, "big") + len(data).to_bytes(2, "big") + data
    return out + b"EOF"


for v in (sys.argv[1].split(",") if len(sys.argv) > 1 else VERSIONS):   # e.g. Rev 0: uk,de,...
    exe = open(os.path.join(BIN, "orig", f"{v}.exe"), "rb").read()
    hits, n, toff, frames = loop_counts(exe)
    recs = [(o, struct.pack("<I", 0x2A220000 | COUNT)) for o in hits]
    if n == 13:
        assert frames == 150, frames
        recs.append((toff, struct.pack("<I", 0x24050000 | NTSC_FRAMES)))
    open(os.path.join(BIN, "patches", v, "credits10.ips"), "wb").write(ips(recs))
    print(f"{v}: {n} -> {COUNT} at " + ", ".join(f"0x{o:X}" for o in hits)
          + (f", {frames} -> {NTSC_FRAMES} frames per picture at 0x{toff:X}" if n == 13 else f", {frames} frames per picture"))
