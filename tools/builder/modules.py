"""Writes the code module file for the builder's converter (tr3pcpsx built with -DEXTERNAL_MODULES, which
reads it from the file named in TR3PCPSX_MODULES) from the player's own Tomb Raider III PS1 disc:
the modules are cut out of the original level files (modules.json: file, offset, length, CRC32) and the
project's byte changes are applied.

Usage: modules.py <PS1 image> <out file>"""
import json
import os
import struct
import sys
import zlib

import inputs

HERE = os.path.dirname(os.path.abspath(__file__))
TABLE = json.load(open(os.path.join(HERE, "modules.json"), encoding="utf-8"))


def module_file(disc, version):
    entries, cache = [], {}
    for index, e in sorted(TABLE[version].items(), key=lambda kv: int(kv[0])):
        if e["file"] not in cache:
            cache[e["file"]] = disc.file(e["file"])
        code = bytearray(cache[e["file"]][e["offset"]:e["offset"] + e["length"]])
        if "%08x" % zlib.crc32(code) != e["crc32"]:
            raise ValueError(f"{e['file']}: code module {e['name']} differs from the expected one")
        for pos, val in e.get("change", {}).items():
            code[int(pos)] = val
        entries.append(struct.pack("<II", int(index), len(code)) + bytes(code))
    return struct.pack("<I", len(entries)) + b"".join(entries)


if __name__ == "__main__":
    disc, how = inputs.open_image(sys.argv[1])
    info = inputs.recognise_ps1(disc)
    open(sys.argv[2], "wb").write(module_file(disc, info["version"]))
    print(f"{how}: {len(TABLE[info['version']])} modules -> {sys.argv[2]}")
