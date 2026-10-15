"""The edited textures, animations and meshes (levels/textures, levels/anims, levels/mesh) as differences
to what the converter reads from the player's own level at that point, so no Core data is shipped.

  textures: per pixel of the texture pages (16-bit BMP from 3texex) the 5-bit red, green and blue steps
            (modulo 32) and the transparency bit that turn the player's page into the project's page;
            mostly zero, so lzma packs it small. assets/<level>.tex
  anims:    the level's animation block (troi export), rewritten by animframes.rebuild (the project's
            files store every rotation in the long form), then bindiff to the project's .dat file: only
            the project's own additions remain (save crystal and two more objects). assets/<level>.anim
  mesh:     bindiff from tr3mesh's text of the level's meshes to the project's mesh text. assets/<level>.mesh

HOOKS are the converter hooks (convert.Converter) that apply them.
Usage: assets.py make   (runs the converter with the repository's files and writes assets/)
       assets.py check  (runs it with HOOKS and compares the result with levels/out)"""
import lzma
import os
import struct
import sys
import zlib

import animframes
import bindiff
import convert

HERE = os.path.dirname(os.path.abspath(__file__))
OUT = os.path.join(HERE, "assets")


def bmp_words(data):
    off = struct.unpack_from("<I", data, 10)[0]
    w, h = struct.unpack_from("<ii", data, 18)
    assert struct.unpack_from("<H", data, 28)[0] == 16 and (w * 2) % 4 == 0
    return off, struct.unpack_from(f"<{w * abs(h)}H", data, off)


def make_tex(cur, target):
    o1, a = bmp_words(cur)
    o2, b = bmp_words(target)
    assert cur[:o1] == target[:o2], "texture page layout changed"
    d = bytearray()
    for x, y in zip(a, b):
        d += bytes(((((y >> s) & 31) - ((x >> s) & 31)) & 31) for s in (10, 5, 0)) + bytes([(x ^ y) >> 15])
    head = struct.pack("<II", zlib.crc32(cur), zlib.crc32(target))
    return lzma.compress(head + bytes(d), preset=9 | lzma.PRESET_EXTREME)


def apply_tex(cur, patch):
    d = lzma.decompress(patch)
    crc_cur, crc_out = struct.unpack_from("<II", d, 0)
    if zlib.crc32(cur) != crc_cur:
        raise ValueError("texture pages differ from the expected level")
    off, a = bmp_words(cur)
    out = []
    for i, x in enumerate(a):
        dr, dg, db, st = d[8 + 4 * i:12 + 4 * i]
        out.append(((x >> 15) ^ st) << 15 | (((x >> 10) + dr) & 31) << 10 | (((x >> 5) + dg) & 31) << 5 | ((x + db) & 31))
    res = cur[:off] + struct.pack(f"<{len(out)}H", *out) + cur[off + 2 * len(out):]
    if zlib.crc32(res) != crc_out:
        raise ValueError("texture result is wrong")
    return res


def rd(name):
    return open(os.path.join(OUT, name), "rb").read()


HOOKS = {
    "textures": lambda lv, cur: apply_tex(cur, rd(f"{lv}.tex")),
    "anims": lambda lv, cur: bindiff.apply(animframes.rebuild(cur), rd(f"{lv}.anim")),
    "mesh": lambda lv, cur: bindiff.apply(cur, rd(f"{lv}.mesh")),
}


def work(name):
    return os.path.join(convert.PRJ, "ergebnisse", "gui_builder", name)


def make():
    c = convert.Converter(work("assets_make"), os.path.join(convert.PRJ, "levels", "in"),
                          os.path.join(convert.PRJ, "levels", "sfx", "common"), versions=["us"])
    c.convert()
    os.makedirs(OUT, exist_ok=True)
    for (kind, lv), cur in sorted(c.seen.items()):
        target = convert.REPO[kind](lv, cur)
        if kind == "textures":
            p, ext = make_tex(cur, target), "tex"
            assert apply_tex(cur, p) == target
        elif kind == "anims":
            src = animframes.rebuild(cur)
            p, ext = bindiff.make(src, target), "anim"
            assert bindiff.apply(src, p) == target
        else:
            p, ext = bindiff.make(cur, target), "mesh"
            assert bindiff.apply(cur, p) == target
        open(os.path.join(OUT, f"{lv}.{ext}"), "wb").write(p)
        print(f"{lv}.{ext}: {len(p)} bytes (project file {len(target)} bytes)")


def check(module_dir=None):
    mf = {v: os.path.join(module_dir, f"{v}.mod") for v in convert.setting(convert.script(), "versions").split(",")} if module_dir else None
    c = convert.Converter(work("assets_check"), os.path.join(convert.PRJ, "levels", "in"),
                          os.path.join(convert.PRJ, "levels", "sfx", "common"), hooks=HOOKS, module_files=mf)
    c.convert()
    bad = [f"{v}/{l}" for v in c.versions for l in c.levels
           if open(c.path("out", v, f"{l}.psx"), "rb").read()
           != open(os.path.join(convert.PRJ, "levels", "out", v, f"{l}.psx"), "rb").read()]
    print(f"{len(c.versions) * len(c.levels) - len(bad)} same as levels/out, different: {bad}")


if __name__ == "__main__":
    {"make": make, "check": check}[sys.argv[1]](*sys.argv[2:])
