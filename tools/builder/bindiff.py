"""Binary difference between two files, in plain Python (no extra modules).

A patch is lzma-compressed: magic b"LAD1", u32 source CRC32, u32 target CRC32, u32 target size, then
instructions: 0x01 COPY (u32 source offset, u32 length) or 0x02 ADD (u32 length, bytes). make() finds
the copies with a hash of 32-byte blocks of the source (slow but only run when making patches);
apply() checks both CRCs, so a patch never produces a wrong file.

Usage: bindiff.py make <source> <target> <patch> | apply <source> <patch> <target>"""
import lzma
import struct
import sys
import zlib

BLOCK = 32


def make(src, dst):
    index = {}
    for o in range(0, len(src) - BLOCK + 1, BLOCK // 2):
        index.setdefault(src[o:o + BLOCK], o)
    ops, lit, i = [], bytearray(), 0
    expect = 0                                     # where the last copy ended: try there first
    while i < len(dst):
        cand = []
        if dst[i:i + BLOCK] == src[expect:expect + BLOCK]:
            cand.append(expect)
        o = index.get(dst[i:i + BLOCK])
        if o is not None:
            cand.append(o)
        best_o, best_n = None, 0
        for c in cand:
            n = 0
            while i + n < len(dst) and c + n < len(src) and dst[i + n] == src[c + n]:
                n += 1
            if n > best_n:
                best_o, best_n = c, n
        if best_n >= BLOCK:
            if lit:
                ops.append(b"\x02" + struct.pack("<I", len(lit)) + bytes(lit))
                lit = bytearray()
            ops.append(b"\x01" + struct.pack("<II", best_o, best_n))
            i += best_n
            expect = best_o + best_n
        else:
            lit.append(dst[i])
            i += 1
            expect += 1
    if lit:
        ops.append(b"\x02" + struct.pack("<I", len(lit)) + bytes(lit))
    head = b"LAD1" + struct.pack("<III", zlib.crc32(src), zlib.crc32(dst), len(dst))
    return lzma.compress(head + b"".join(ops), preset=9)


def apply(src, patch):
    d = lzma.decompress(patch)
    assert d[:4] == b"LAD1", "not a patch"
    crc_src, crc_dst, size = struct.unpack_from("<III", d, 4)
    if zlib.crc32(src) != crc_src:
        raise ValueError("the patch belongs to another source file")
    out, p = bytearray(), 16
    while p < len(d):
        if d[p] == 1:
            o, n = struct.unpack_from("<II", d, p + 1)
            out += src[o:o + n]
            p += 9
        else:
            n = struct.unpack_from("<I", d, p + 1)[0]
            out += d[p + 5:p + 5 + n]
            p += 5 + n
    if len(out) != size or zlib.crc32(out) != crc_dst:
        raise ValueError("patch result is wrong")
    return bytes(out)


if __name__ == "__main__":
    if sys.argv[1] == "make":
        a, b = open(sys.argv[2], "rb").read(), open(sys.argv[3], "rb").read()
        open(sys.argv[4], "wb").write(make(a, b))
    else:
        a, p = open(sys.argv[2], "rb").read(), open(sys.argv[3], "rb").read()
        open(sys.argv[4], "wb").write(apply(a, p))
