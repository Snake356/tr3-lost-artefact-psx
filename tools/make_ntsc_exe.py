"""The NTSC executable for a European disc (used by tools/builder/build.py): the US executable (Rev 0, 1
or 2) with the XA clip lengths of the language's speech files F11-F15, so the NTSC game plays that
language's voice recordings in Lara's house.

The XA clip table (128 u16, one per XA channel; entry n belongs to F(n/8+1)) gives how far
before the end of its XA file each channel stops. The start comes from the file itself,
so only these end offsets are language dependent.

It also points the two English best-times headings in Lara's house (used for every
language without its own text, including AMERICAN) at the language's strings that are
already in the executable. The gameflow stays LANGUAGE AMERICAN, so the German blood
censorship stays off."""
import re
import struct
import zlib


# CRC32 of entries 0-79 (F1-F10) of the XA clip table (160 bytes, 4-byte aligned), the same in every
# language and revision; the table is found by it
TABLE_HEAD_CRC = 0x7F0FCCE7

# CRC32 of the best-times headings, NUL-terminated strings in the US executable (it holds every language's)
HEADING_CRC = {   # language: (assault course, quad bike track)
    "de": (0x5F360B91, 0x9E1D17C8),
    "fr": (0xE53D09C4, 0xD7239B95),
    "it": (0x02874E27, 0x6DC5FA9E),
    "sp": (0x06D2FB56, 0x73B1D368),
    "en": (0x25DD3440, 0x38EF40F2),     # the US executable's own
}
F14_FIX = 111, 65440                    # Rev 0 has 0 here (Rev 1 corrected it)


def find_table(data, what):
    hits = [i for i in range(0, len(data) - 160, 4) if zlib.crc32(data[i:i + 160]) == TABLE_HEAD_CRC]
    assert len(hits) == 1, f"{what}: found {len(hits)} times"
    return hits[0]


def find_string(data, crc, what):
    """The one NUL-terminated string with this CRC32 (its start follows a NUL)."""
    hits = [m.start() for m in re.finditer(rb"(?<=\0)[^\0]{4,64}(?=\0)", data) if zlib.crc32(m.group()) == crc]
    assert len(hits) == 1, f"{what}: found {len(hits)} times"
    return hits[0]


def find_word(data, words, what):
    """The one 4-byte aligned place of the instruction words."""
    needle = b"".join(struct.pack("<I", w) for w in words)
    hits, i = [], data.find(needle)
    while i >= 0:
        if i % 4 == 0:
            hits.append(i)
        i = data.find(needle, i + 1)
    assert len(hits) == 1, f"{what}: found {len(hits)} times"
    return hits[0]


def ram(offset):
    return offset + 0x10000 - 0x800                     # file offset -> address in RAM (kuseg)


def hi(a):
    return (a + 0x8000) >> 16


def make(us_exe, lang_exe, lang="de"):
    """NTSC executable for a European language from the patched US executable and that language's patched
    PAL executable (bytes): the language's XA clip lengths for F11-F15 and its best-times headings."""
    us = bytearray(us_exe)
    us_table = find_table(us_exe, "XA clip table (US)")
    table = find_table(lang_exe, "XA clip table (language)")
    u = struct.unpack_from("<128H", us, us_table)
    d = struct.unpack_from("<128H", lang_exe, table)
    assert u[120:] == d[120:], "tables differ after F15 - not the XA clip table?"
    if lang != "en":
        us[us_table + 160:us_table + 240] = lang_exe[table + 160:table + 240]
    elif u[F14_FIX[0]] == 0:                             # English: the US executable's own table
        struct.pack_into("<H", us, us_table + 2 * F14_FIX[0], F14_FIX[1])

    en = [ram(find_string(us_exe, c, f"heading en {k}")) for k, c in enumerate(HEADING_CRC["en"])]
    new = [ram(find_string(us_exe, c, f"heading {lang} {k}")) for k, c in enumerate(HEADING_CRC[lang])]
    assault = find_word(us_exe, [0x24A50000 | (en[0] & 0xFFFF)], "addiu Assault Course")
    quad = find_word(us_exe, [0x3C050000 | hi(en[1]), 0x24A50000 | (en[1] & 0xFFFF)], "lui/addiu Quadbike Track")
    assert hi(new[0]) == hi(en[0]), "the lui before the assault course addiu does not fit"
    struct.pack_into("<I", us, assault, 0x24A50000 | (new[0] & 0xFFFF))
    struct.pack_into("<II", us, quad, 0x3C050000 | hi(new[1]), 0x24A50000 | (new[1] & 0xFFFF))
    return bytes(us)

