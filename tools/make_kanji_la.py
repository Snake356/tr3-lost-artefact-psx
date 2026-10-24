"""Builds art/kanji/kanji_la.psx: the Japanese TR3 KANJI.PSX with 13 extra glyphs for the Lost
Artefact translations and the New Game+ (cells 243-256: ユ回園屋手球眠起路隠電魚鳥＋), drawn with MS Gothic 13 px,
1 bit, 1 pixel up. KANJI.PSX = u32 data size (26624) + 256x208 4-bit sheet (low nibble first),
16x13 cells, 16 per row; glyph code cell N is sheet cell N-1. The output ends after the image
data like the original file (the disc-extracted source may be padded to 2048 bytes).
Also adds the new cells to tools/tr3_japan_kanji_cells.json.

Usage: make_kanji_la.py <original Japanese TR3 KANJI.PSX> [--check]
       --check: only compare the result with the existing art/kanji/kanji_la.psx"""
import json
import os
import sys

import numpy as np
from PIL import Image, ImageDraw, ImageFont

PRJ = os.path.join(os.path.dirname(os.path.abspath(__file__)), "..")
OUT = os.path.join(PRJ, "art", "kanji", "kanji_la.psx")
CELLS = os.path.join(PRJ, "tools", "tr3_japan_kanji_cells.json")
NEW = "ユ回園屋手球眠起路隠電魚鳥＋"
FIRST_CELL = 243

raw = bytearray(open(sys.argv[1], "rb").read())
size = int.from_bytes(raw[:4], "little")
assert size == 26624, size
raw = raw[:4 + size]
font = ImageFont.truetype("C:/Windows/Fonts/msgothic.ttc", 13)
cells = {int(k): v for k, v in json.load(open(CELLS, encoding="utf-8")).items()}
for n, ch in enumerate(NEW):
    cell = FIRST_CELL + n
    idx = cell - 1
    x0, y0 = (idx % 16) * 16, (idx // 16) * 13
    im = Image.new("L", (16, 14), 0)
    d = ImageDraw.Draw(im)
    d.fontmode = "1"
    d.text((0, -1), ch, font=font, fill=255)
    g = np.asarray(im)[:13] > 127
    for y in range(13):
        for x in range(16):
            v = 15 if g[y, x] else 0
            p = (y0 + y) * 256 + (x0 + x)
            b = 4 + p // 2
            raw[b] = (raw[b] & 0xF0) | v if p % 2 == 0 else (raw[b] & 0x0F) | (v << 4)
    cells[cell] = ch

if "--check" in sys.argv:
    old = open(OUT, "rb").read()
    print("same as art/kanji/kanji_la.psx:", bytes(raw) == old, len(raw), len(old))
else:
    open(OUT, "wb").write(raw)
    json.dump({str(k): v for k, v in sorted(cells.items())}, open(CELLS, "w", encoding="utf-8"),
              ensure_ascii=False, indent=0)
    print(f"{OUT}: {len(raw)} bytes, cells {FIRST_CELL}-{FIRST_CELL + len(NEW) - 1}")
