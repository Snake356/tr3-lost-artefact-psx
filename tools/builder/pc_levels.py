"""Turns the player's Lost Artefact PC levels into the converter's input levels (levels/in/*.tr2).

levels.json lists, per level and per known source CRC32, either "same" (the player's file is already the
input) or a patch in levels/ (bindiff.py) that turns it into the input: the project's own changes, e.g.
the rebuilt part of Highland Fling. Sleeping with the Fishes part 2 (underse2) starts as a copy of
undersea (or undersea with the patch "from_undersea", when the project's underse2 input differs); the
converter splits the level. "input_crc" is the CRC32 of the project's input level (tools/builder/make_data.py).

Usage: pc_levels.py <PC disc image or game folder> <out folder>"""
import json
import os
import sys
import zlib

import bindiff
import inputs

HERE = os.path.dirname(os.path.abspath(__file__))
TABLE = json.load(open(os.path.join(HERE, "levels.json"), encoding="utf-8"))


def prepare(files):
    """files: {"scotland.tr2": bytes, ...} -> {"scotland.tr2": bytes, ...} for all seven input levels."""
    out = {}
    for lv in inputs.LEVELS:
        data = files[lv + ".tr2"]
        how = TABLE[lv].get("%08x" % zlib.crc32(data))
        if how is None:
            raise ValueError(f"{lv}.tr2: unknown version of the level")
        out[lv + ".tr2"] = data if how == "same" else bindiff.apply(data, open(os.path.join(HERE, "levels", how), "rb").read())
    u2 = TABLE["underse2"].get("from_undersea") if isinstance(TABLE.get("underse2"), dict) else None      # only if levels/in/underse2.tr2 was edited on its own
    out["underse2.tr2"] = bindiff.apply(out["undersea.tr2"], open(os.path.join(HERE, "levels", u2), "rb").read())         if u2 else out["undersea.tr2"]
    return out


if __name__ == "__main__":
    src, dst = sys.argv[1], sys.argv[2]
    files = inputs.pc_files_from_folder(src) if os.path.isdir(src) else inputs.pc_files_from_disc(inputs.open_image(src)[0])
    os.makedirs(dst, exist_ok=True)
    for name, data in prepare(files).items():
        open(os.path.join(dst, name), "wb").write(data)
    print("done:", dst)
