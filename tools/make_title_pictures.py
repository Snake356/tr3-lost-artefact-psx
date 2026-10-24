"""Builds the PS1 title pictures (art/loadscr/*/title*.raw) from the Lost Artefact PC titles in art/loadscr/src.

The PC title (640 x 480) is retouched first: its right edge, the plain blue corner at the bottom left and the light blue wedge beside Lara's arm at the bottom right. It is then resized to 512 x 256
(PAL, no border) or 512 x 240 inside 256 lines with 8 black lines above and below (NTSC), at 5 bits per colour;
only the retouched parts are dithered. The Japanese title is the UK NTSC picture with the Japanese logo of
art/loadscr/src/jp/titlejap_logo.png (alpha = logo). tric (build/win/bin) packs the pictures.

Usage: python make_title_pictures.py"""
import os
import shutil
import struct
import subprocess
import sys
import tempfile

import numpy as np
from PIL import Image

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
ART = os.path.join(ROOT, "art", "loadscr")
TRIC = os.path.join(ROOT, "build", "win", "bin", "tric.exe")

PICTURES = {"uk": "titleuk", "de": "titlede", "fr": "titlefr", "us": "titleus"}
PAL = ("uk", "de", "fr")
UK_COPIES = ("it", "es", "jp")


def retouch(path):
    src = np.asarray(Image.open(path).convert("RGB")).astype(float)
    f = src.copy()
    touched = np.zeros(src.shape[:2])
    # right edge: the last three columns are darker / bluish
    f[:, 637:640] = f[:, 636:637]
    # bottom left: the plain blue corner gets the frost pattern of the same rows 45 pixels to the right,
    # fading into the original between x 25 and 45 and between rows 365 and 385
    for y in range(365, 480):
        for x in range(0, 46):
            m = np.clip((45 - x) / 20.0, 0, 1) * np.clip((y - 365) / 20.0, 0, 1)
            if m > 0:
                f[y, x] = f[y, x] * (1 - m) + src[y, x + 45] * m
                touched[y, x] = max(touched[y, x], m)
    # bottom right: the lighter blue beside Lara's arm (x 626-639, y 418-456) takes the blue of row 466;
    # the arm and its shadow are kept
    for y in range(418, 457):
        for x in range(626, 640):
            r, g, b = src[y, x]
            t = np.clip((b - 100) / 10.0, 0, 1) * (b - r > 40)
            f[y, x] = f[y, x] * (1 - t) + f[466, x] * t
    return f, touched


def ps1(img, touched, size):
    """Resized to the PS1 picture, 5 bits per colour (values 0-248); dithered only where touched."""
    a = np.asarray(Image.fromarray(np.clip(img, 0, 255).astype(np.uint8)).resize(size, Image.BICUBIC)).astype(float)
    t = np.asarray(Image.fromarray((touched * 255).astype(np.uint8)).resize(size, Image.BILINEAR)) > 20
    d = (a.astype(int) & ~7).astype(float)
    d[t] = a[t]
    h, w = d.shape[:2]
    for y in range(h):
        for x in range(w):
            if not t[y, x]:
                continue
            old = d[y, x].copy()
            new = np.clip(np.floor(old / 8 + 0.5) * 8, 0, 248)
            d[y, x] = new
            e = old - new
            if x + 1 < w and t[y, x + 1]:
                d[y, x + 1] += e * 7 / 16
            if y + 1 < h:
                if x > 0 and t[y + 1, x - 1]:
                    d[y + 1, x - 1] += e * 3 / 16
                if t[y + 1, x]:
                    d[y + 1, x] += e * 5 / 16
                if x + 1 < w and t[y + 1, x + 1]:
                    d[y + 1, x + 1] += e / 16
    return np.clip(d, 0, 248).astype(np.uint8)


def ntsc(img, touched):
    out = np.zeros((256, 512, 3), np.uint8)
    out[8:248] = ps1(img, touched, (512, 240))
    return out


def write_bmp16(pic, path):
    """16 bpp (X1R5G5B5) BMP, bottom-up, as tric reads it."""
    h, w = pic.shape[:2]
    p = pic.astype(np.uint16) >> 3
    rows = (p[::-1, :, 0] << 10 | p[::-1, :, 1] << 5 | p[::-1, :, 2]).astype("<u2").tobytes()
    header = struct.pack("<2sIHHI", b"BM", 54 + len(rows), 0, 0, 54)
    info = struct.pack("<IiiHHIIiiII", 40, w, h, 1, 16, 0, len(rows), 0, 0, 0, 0)
    open(path, "wb").write(header + info + rows)


def pack(pic, raw, tmp):
    bmp = os.path.join(tmp, "in.bmp")
    back = os.path.join(tmp, "back.bmp")
    write_bmp16(pic, bmp)
    subprocess.run([TRIC, "tr3psx", bmp, raw], check=True, stdout=subprocess.DEVNULL)
    subprocess.run([TRIC, "tr3psx", raw, back], check=True, stdout=subprocess.DEVNULL)
    got = np.asarray(Image.open(back).convert("RGB")).astype(int) >> 3
    assert (got == pic.astype(int) >> 3).all(), f"{raw}: unpacked picture differs"
    print(f"{os.path.relpath(raw, ROOT)}: {os.path.getsize(raw)} bytes")


def main():
    with tempfile.TemporaryDirectory() as tmp:
        for lang, name in PICTURES.items():
            img, touched = retouch(os.path.join(ART, "src", lang, name + ".bmp"))
            pic = ntsc(img, touched)
            pack(pic, os.path.join(ART, lang, name + ".raw"), tmp)
            if lang in PAL:
                pack(ps1(img, touched, (512, 256)), os.path.join(ART, lang, name + "_pal.raw"), tmp)
            if lang == "uk":
                for copy in UK_COPIES:
                    shutil.copyfile(os.path.join(ART, "uk", "titleuk.raw"), os.path.join(ART, copy, "titleuk.raw"))
                logo = np.asarray(Image.open(os.path.join(ART, "src", "jp", "titlejap_logo.png")).convert("RGBA"))
                jap = pic.copy()
                inside = logo[:, :, 3] > 0
                jap[inside] = logo[:, :, :3][inside]
                pack(jap, os.path.join(ART, "jp", "titlejap.raw"), tmp)


if __name__ == "__main__":
    sys.exit(main())
