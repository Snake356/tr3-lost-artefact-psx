"""Silenced samples that end: the project's empty.wav (0 frames) becomes a 32-byte VAG in the level (a zero block
and a block flagged 0x07, loop start/end/repeat), a silent endless loop that keeps its sound slot busy. Here that
flag becomes 0x01 (end + mute), so the voice stops, at the same 32 bytes (leer_mit_ende.wav needed 96, which
pushed the samples of Shakespeare Cliff with the rocket launcher's shot into the reverb area at 0x7B7C0).
Usage: leer_ende.py level.psx [...]"""
import struct
import sys

EMPTY = bytes(16) + b"\x00\x07" + bytes(14)


def patch(path):
    d = bytearray(open(path, "rb").read())
    n = struct.unpack_from("<I", d, 4)[0]
    offs = list(struct.unpack_from("<%dI" % n, d, 8))
    total = struct.unpack_from("<I", d, 8 + 4 * n)[0]
    base = 8 + 4 * n + 4
    done = 0
    for a, b in zip(offs, offs[1:] + [total]):
        if d[base + a:base + b] == EMPTY:
            d[base + a + 17] = 0x01
            done += 1
    open(path, "wb").write(d)
    return done


if __name__ == "__main__":
    for p in sys.argv[1:]:
        print(p, patch(p))
