"""Turns the files extracted from the player's PS1 image (extract.py) into the disc files the Lost
Artefact layouts (disc/<layout>.xml) take, in the same folder structure as the repository's disc/files:

  files/xa/common/f1..f8, f10, f16.xa   speech/music streams; in F1, F2, F4, F5, F6 and F8 the tracks the
                                        Lost Artefact does not use are silenced (XA_SILENT: every sector
                                        in these interleave slots is zeroed, as tools/src/xarem.c does)
  files/xa/<language>/f11..f15.xa       the language's own speech
  files/data/<data>/fmv.mod, title.psx  as on the disc
  files/data/<data>/house.psx           with the sky dome tip closed (tools/close_dome_tip.py)
  files/fmv/common/logo.fmv             the Eidos logo video
  exe/<version>.exe                     the original executable (patched later)

Usage: disc_files.py <extracted folder> <version> <out folder>"""
import os
import shutil
import subprocess
import sys

import convert

# version: (folder of F11-F15 speech, data folder) as used by disc/<layout>.xml
LAYOUT = {"us": ("en", "us"), "ukrev": ("en", "uk"), "derev": ("de", "uk"), "frrev": ("fr", "uk"),
          "itrev": ("it", "uk"), "sprev": ("sp", "uk"), "jp": ("jp", "jp"), "jp2": ("en", "jp2"),
          "as": ("jp", "as"), "as2": ("en", "as2"),
          # Rev 0 (first releases): the same layouts
          "us10": ("en", "us"), "uk": ("en", "uk"), "de": ("de", "uk"), "fr": ("fr", "uk"), "it": ("it", "uk"),
          "sp": ("sp", "uk")}
XA_SILENT = {1: (5, 6), 2: (7,), 4: (2, 3, 5, 7), 5: (1, 3, 4, 7), 6: (5,), 8: (4, 5, 6, 7)}
SECTOR = 2336


def silence(data, slots):
    d = bytearray(data)
    for i in range(len(d) // SECTOR):
        if i % 8 in slots:
            d[i * SECTOR:(i + 1) * SECTOR] = bytes(SECTOR)
    return bytes(d)


def build(src, version, out):
    lang, data = LAYOUT[version]
    put = lambda rel, b: (os.makedirs(os.path.dirname(os.path.join(out, rel)), exist_ok=True),
                          open(os.path.join(out, rel), "wb").write(b))
    rd = lambda rel: open(os.path.join(src, rel), "rb").read()
    for n in (1, 2, 3, 4, 5, 6, 7, 8, 10, 16):
        b = rd(f"XA/F{n}.XA")
        put(f"files/xa/common/f{n}.xa", silence(b, XA_SILENT[n]) if n in XA_SILENT else b)
    for n in (11, 12, 13, 14, 15):
        put(f"files/xa/{lang}/f{n}.xa", rd(f"XA/F{n}.XA"))
    put(f"files/data/{data}/fmv.mod", rd("DATA/FMV.MOD"))
    put(f"files/data/{data}/title.psx", rd("DATA/TITLE.PSX"))
    house = os.path.join(out, f"files/data/{data}/house.psx")
    subprocess.run(convert.tool_command("close_dome_tip.py") + [os.path.join(src, "DATA/HOUSE.PSX"), house],
                   check=True, stdout=subprocess.DEVNULL, creationflags=convert.NO_WINDOW)
    put("files/fmv/common/logo.fmv", rd("FMV/LOGO.FMV"))
    put(f"exe/{version}.exe", rd(f"exe/{version}.exe"))


if __name__ == "__main__":
    build(sys.argv[1], sys.argv[2], sys.argv[3])
    print("done:", sys.argv[3])
