"""Converts any image into the 16 bpp (X1R5G5B5) BMP that tric needs for PS1 RAW pictures.

Usage: python bmp_to_16bit.py <input image> <output.bmp>"""
import struct
import sys

from PIL import Image

src, dst = sys.argv[1], sys.argv[2]
im = Image.open(src).convert("RGB")
w, h = im.size
px = im.load()
row_bytes = (w * 2 + 3) & ~3
rows = []
for y in range(h - 1, -1, -1):
    row = bytearray()
    for x in range(w):
        r, g, b = px[x, y]
        row += struct.pack("<H", (((r * 31 + 127) // 255) << 10) | (((g * 31 + 127) // 255) << 5) | ((b * 31 + 127) // 255))
    rows.append(bytes(row) + b"\0" * (row_bytes - len(row)))
data = b"".join(rows)
header = struct.pack("<2sIHHI", b"BM", 54 + len(data), 0, 0, 54)
info = struct.pack("<IiiHHIIiiII", 40, w, h, 1, 16, 0, len(data), 0, 0, 0, 0)
open(dst, "wb").write(header + info + data)
print(f"{dst}: {w}x{h} 16 bpp")
