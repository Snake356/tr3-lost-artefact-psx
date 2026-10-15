"""Builds the TOMBPSX.DAT of the Japanese/Asian discs as the original TR3 discs have them:

    as.dat  = jp.dat with memory card code BISCPS-45385 (Asia Disc 1 = Japan Disc 1, Japanese
              texts); the codes have the same length, so it is replaced in place.
    jp2.dat = built from tombpsx/src/uk (English texts, language 0 as on the original discs)
    as2.dat   with memory card code BISLPM-86197 / BISCPS-45386, and all three title/legal
              pairs pointing at pixJAP\\titleJAP.raw / legalJAP.raw: the executable picks the pair
              by language (0 = the pixUK one), and these discs only have PIXJAP. The sources are
              written to tombpsx/src/jp2 and tombpsx/src/as2.

Strings are XOR-ed with the file's cypher byte (offset 382; 0 = none).

Usage: make_region_dat.py <gameflow.exe>"""
import os
import shutil
import subprocess
import sys

PRJ = os.path.join(os.path.dirname(os.path.abspath(__file__)), "..")
OUT = os.path.join(PRJ, "tombpsx", "out")
SRC = os.path.join(PRJ, "tombpsx", "src")


def replace_code(src, dst, old, new):
    d = open(os.path.join(OUT, src), "rb").read()
    key = d[382]
    body = bytes(b ^ key for b in d[384:])
    assert body.count(old) == 1 and len(old) == len(new), (src, body.count(old))
    out = d[:384] + bytes(b ^ key for b in body.replace(old, new))
    open(os.path.join(OUT, dst), "wb").write(out)
    print(f"{dst}: from {src}, {old.decode()} -> {new.decode()}, language {out[383]}")


def build_english(ver, code, gameflow):
    src, dst = os.path.join(SRC, "uk"), os.path.join(SRC, ver)
    os.makedirs(dst, exist_ok=True)
    for f in ("pc_strings.txt", "psx_strings.txt"):
        s = open(os.path.join(src, f), encoding="latin-1", newline="").read()
        assert s.count("BASLUS-00691") == 1, f
        open(os.path.join(dst, f), "w", encoding="latin-1", newline="").write(s.replace("BASLUS-00691", code))
    s = open(os.path.join(src, "tr3_gold.txt"), encoding="latin-1", newline="").read()
    for old, new in (("pixUK\\titleuk.raw", "pixJAP\\titleJAP.raw"), ("pixUK\\legaluk.raw", "pixJAP\\legalJAP.raw"),
                     ("pixUS\\titleUS.raw", "pixJAP\\titleJAP.raw"), ("pixUS\\legalUS.raw", "pixJAP\\legalJAP.raw")):
        assert s.count("PSXFILE:\t" + old) == 1, old
        s = s.replace("PSXFILE:\t" + old, "PSXFILE:\t" + new)
    open(os.path.join(dst, "tr3_gold.txt"), "w", encoding="latin-1", newline="").write(s)
    subprocess.run([gameflow, "tr3_gold.txt"], cwd=dst, capture_output=True)  # exits with 11 normally
    shutil.move(os.path.join(dst, "tombPSX.dat"), os.path.join(OUT, f"{ver}.dat"))
    for f in ("tr3_gold.log", "tr3_gold.str"):
        if os.path.exists(os.path.join(dst, f)):
            os.remove(os.path.join(dst, f))
    d = open(os.path.join(OUT, f"{ver}.dat"), "rb").read()
    body = bytes(b ^ d[382] for b in d[384:])
    assert d[383] == 0 and code.encode() in body and b"pixUK" not in body and b"pixUS" not in body
    print(f"{ver}.dat: from tombpsx/src/{ver} (uk texts), {code}, titles only pixJAP, {len(d)} bytes")


if __name__ == "__main__":
    replace_code("jp.dat", "as.dat", b"BISLPM-86196", b"BISCPS-45385")
    build_english("jp2", "BISLPM-86197", sys.argv[1])
    build_english("as2", "BISCPS-45386", sys.argv[1])
