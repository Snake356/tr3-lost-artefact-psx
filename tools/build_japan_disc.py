"""Builds the Japanese/Asian Disc 1 images (jp, as).

The Japan/Asia Disc 1 changes (more pickups, fewer traps) are part of levels/convert.sh
(levels/japan/<level>.txt), so the converted levels are in levels/out/jp and levels/out/as like all
other versions. This script only builds the disc images from them:
  build/japan/tr3g_<version>.bin/.cue   (from disc/<version>.xml)

With --convert it first converts the levels again with tools/windows_build.py (Windows) and uses those
instead of levels/out (levels/out stays unchanged; copy the results yourself when they are good).

With --level-select the image gets tombpsx/out/<version>test.dat instead (level select on New Game
and a save crystal at the start of every level, for testing): build/japan/tr3g_<version>_level_select.bin

Usage: py -3 tools/build_japan_disc.py [--versions jp,as] [--convert] [--level-select] [--out DIR]
Needs mkpsxiso in PATH, or g++ (MinGW) to compile it from tools/src/mkpsxiso."""
import argparse
import glob
import os
import re
import shutil
import subprocess
import sys

PRJ = os.path.normpath(os.path.join(os.path.dirname(os.path.abspath(__file__)), ".."))
EXE = ".exe" if os.name == "nt" else ""
LEVELS = ["scotland", "willsden", "chunnel", "undersea", "underse2", "zoo", "slinc"]


def run(cmd, **kw):
    r = subprocess.run(cmd, capture_output=True, text=True, **kw)
    if r.returncode != 0:
        sys.exit(f"failed: {' '.join(cmd)}\n{r.stdout[-2000:]}\n{r.stderr[-2000:]}")
    return r.stdout


def mkpsxiso(bindir):
    mk = shutil.which("mkpsxiso")
    if mk:
        return mk
    mk = os.path.join(bindir, "mkpsxiso" + EXE)
    if not os.path.exists(mk):
        os.makedirs(bindir, exist_ok=True)
        files = sorted(glob.glob(os.path.join(PRJ, "tools", "src", "mkpsxiso", "src", "*.cpp")))
        run(["g++", "-O2", "-std=c++11", "-w", "-o", mk] + files + (["-static"] if os.name == "nt" else []))
        print(f"built {mk}")
    return mk


def build_image(mk, version, leveldir, outdir, levelselect=False):
    xml = open(os.path.join(PRJ, "disc", version + ".xml"), encoding="utf-8").read()
    name = f"tr3g_{version}"
    if levelselect:
        old = f'source="../tombpsx/out/{version}.dat"'
        assert xml.count(old) == 1, f"disc/{version}.xml: TOMBPSX.DAT source changed"
        xml = xml.replace(old, f'source="../tombpsx/out/{version}test.dat"')
        name += "_level_select"
    fwd = lambda p: os.path.abspath(p).replace("\\", "/")
    if leveldir:
        old = f"../levels/out/{version}/"
        assert xml.count(old) == len(LEVELS), f"disc/{version}.xml: level paths changed"
        xml = xml.replace(old, fwd(leveldir) + "/")
    xml = re.sub(r'image_name="[^"]*"', f'image_name="{fwd(os.path.join(outdir, name + ".bin"))}"', xml, 1)
    xml = re.sub(r'cue_sheet="[^"]*"', f'cue_sheet="{fwd(os.path.join(outdir, name + ".cue"))}"', xml, 1)
    tmpxml = os.path.join(outdir, name + ".xml")
    open(tmpxml, "w", encoding="utf-8").write(xml)
    run([mk, "-y", tmpxml], cwd=os.path.join(PRJ, "disc"))  # the sources are relative to disc/
    os.remove(tmpxml)
    cue = os.path.join(outdir, name + ".cue")
    c = open(cue, encoding="utf-8").read()
    open(cue, "w", encoding="utf-8").write(re.sub(r'FILE "[^"]*"', f'FILE "{name}.bin"', c))
    print(f"image: {os.path.join(outdir, name)}.bin / .cue")


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--versions", default="jp", help="jp, as or jp,as")
    ap.add_argument("--convert", action="store_true", help="convert the levels first (windows_build.py)")
    ap.add_argument("--level-select", action="store_true",
                    help="test image with level select (tombpsx/out/<version>test.dat, see make_jp_levelselect_dat.py)")
    ap.add_argument("--out", default=os.path.join(PRJ, "build", "japan"))
    a = ap.parse_args()
    versions = a.versions.split(",")
    for v in versions:
        if v not in ("jp", "as"):
            sys.exit(f"{v}: only jp and as (Japan/Asia Disc 1)")
    os.makedirs(a.out, exist_ok=True)
    result = None
    if a.convert:
        result = os.path.join(a.out, "result")
        r = subprocess.run([sys.executable, os.path.join(PRJ, "tools", "windows_build.py"),
                            "--versions", ",".join(versions), "--out", result])
        if r.returncode != 0:
            sys.exit("conversion failed")
    mk = mkpsxiso(os.path.join(a.out, "bin"))
    for v in versions:
        build_image(mk, v, os.path.join(result, "out", v) if result else None, a.out, a.level_select)


if __name__ == "__main__":
    main()
