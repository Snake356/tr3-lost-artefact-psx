"""Builds test versions of the Japanese/Asian Disc 1 TOMBPSX.DAT with level select:
    tombpsx/out/jptest.dat, tombpsx/out/astest.dat

Same as make_jp_gameflow.py --level-select (SELECT_ANY_LEVEL: level select on New Game, and a save
crystal in the inventory at the start of every level, because the Japanese game saves from the crystal
in the inventory), but without the original Japanese TR3 TOMBPSX.DAT: the Japanese glyph strings are
taken from the finished tombpsx/out/jp.dat. tombpsx/src/jp holds the same texts as ASCII placeholders
of the same length (Z<3 hex digits>Q...), so the source is built once as it is (the result must equal
jp.dat apart from the placeholders, which is checked) and once with level select, and every
placeholder gets the glyph bytes from jp.dat. astest.dat gets the Asian memory card code like as.dat.

Usage: make_jp_levelselect_dat.py <gameflow executable (tomb3 PSX version)>"""
import os
import re
import shutil
import subprocess
import sys
import tempfile

PRJ = os.path.normpath(os.path.join(os.path.dirname(os.path.abspath(__file__)), ".."))
SRC = os.path.join(PRJ, "tombpsx", "src", "jp")
OUT = os.path.join(PRJ, "tombpsx", "out")
TOKEN = re.compile(rb"Z[0-9A-F]{3}Q+")


def build(gameflow, script_lines):
    tmp = tempfile.mkdtemp()
    for f in ("pc_strings.txt", "psx_strings.txt"):
        shutil.copy(os.path.join(SRC, f), tmp)
    open(os.path.join(tmp, "tr3_gold.txt"), "w", encoding="latin-1", newline="").write("\n".join(script_lines))
    subprocess.run([gameflow, "tr3_gold.txt"], cwd=tmp, capture_output=True)
    raw = open(os.path.join(tmp, "tombPSX.dat"), "rb").read()
    shutil.rmtree(tmp)
    return raw


def main(gameflow):
    jp = open(os.path.join(OUT, "jp.dat"), "rb").read()
    script = open(os.path.join(SRC, "tr3_gold.txt"), encoding="latin-1", newline="").read().split("\n")

    # 1. glyph bytes of every placeholder, taken from jp.dat at the placeholder's position
    plain = build(gameflow, script)
    assert len(plain) == len(jp), "tombpsx/src/jp does not build a file of jp.dat's length"
    glyphs = {}
    for m in TOKEN.finditer(plain):
        tok = m.group()
        assert plain.count(tok) == 1, tok
        glyphs[tok] = jp[m.start():m.end()]
    check = bytearray(plain)
    for tok, g in glyphs.items():
        p = check.index(tok); check[p:p + len(tok)] = g
    assert bytes(check) == jp, "tombpsx/src/jp and jp.dat differ outside the placeholders"

    # 2. the script with level select (the same changes as make_jp_gameflow.py --level-select)
    res, in_level = [], False
    for line in script:
        if re.match(r"\s*LANGUAGE:", line):
            line += "\n   SELECT_ANY_LEVEL:"
        if re.match(r"\s*GAME:\s*data\\(?!title)", line, re.I) and in_level:
            res.append("   PSXSTARTINV:\tCRYSTAL")
        res.append(line)
        if re.match(r"\s*LEVEL:", line):
            in_level = True
        elif re.match(r"\s*END:", line):
            in_level = False
    raw = bytearray(build(gameflow, res))
    for tok, g in glyphs.items():
        assert raw.count(tok) == 1, tok
        p = raw.index(tok); raw[p:p + len(tok)] = g
    assert not TOKEN.search(raw), "placeholder left"
    assert raw[382] == 0 and raw[383] == 4, (raw[382], raw[383])
    open(os.path.join(OUT, "jptest.dat"), "wb").write(raw)
    print(f"jptest.dat: {len(raw)} bytes (jp.dat {len(jp)}), level select, crystal at every level start")

    # 3. Asia: the memory card code as in make_region_dat.py (no cypher, so a plain replacement)
    old, new = b"BISLPM-86196", b"BISCPS-45385"
    assert raw.count(old) == 1
    open(os.path.join(OUT, "astest.dat"), "wb").write(bytes(raw).replace(old, new))
    asdat = open(os.path.join(OUT, "as.dat"), "rb").read()
    assert asdat == jp.replace(old, new), "as.dat is not jp.dat with the Asian code"
    print("astest.dat: from jptest.dat with BISCPS-45385")


if __name__ == "__main__":
    if len(sys.argv) != 2:
        sys.exit(__doc__)
    main(sys.argv[1])
