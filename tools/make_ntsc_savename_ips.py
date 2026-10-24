"""Builds bin/patches/<us version>/savename_<language>.ips: the savegame title of a European disc built as NTSC
(tools/builder/build.py), which runs the US executable. The US savename.ips writes "Tomb Raider: The Lost
Artifact" into the 64 byte Shift-JIS title of the savegame header, the one the memory card managers show; these
write the title of the language's own PAL savename.ips at the same place (English: the UK disc's "The Lost
Artefact"), so the NTSC savegames show their language's title.

Usage: make_ntsc_savename_ips.py"""
import os
import struct

import make_pickup3d_ips as p3
import make_reunionview_ips as rv

BIN = os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "bin")
US_VERSIONS = ("us", "us10")
PAL = {"de": "derev", "fr": "frrev", "it": "itrev", "sp": "sprev", "en": "ukrev"}


def title(ver):
    """(file offset, 64 title bytes) of a version's savename.ips."""
    d = open(os.path.join(BIN, "patches", ver, "savename.ips"), "rb").read()
    (a, b), = p3.ips_ranges(d)
    assert b - a == 64, f"{ver}: savename.ips is not one 64 byte title"
    return a, rv.apply_ips(bytes(b), d)[a:b]


if __name__ == "__main__":
    for us in US_VERSIONS:
        off, _ = title(us)
        for lang, pal in PAL.items():
            text = title(pal)[1]
            open(os.path.join(BIN, "patches", us, f"savename_{lang}.ips"), "wb").write(rv.ips([(off, list(struct.unpack("<16I", text)))]))
            print(f"{us} ({lang}): {text.split(b'\0\0')[0].decode('shift_jis')}")
