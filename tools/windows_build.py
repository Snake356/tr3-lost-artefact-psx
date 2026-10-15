"""Builds the levels on Windows (Git Bash + MinGW gcc + Python), the current way to build this
project. The Linux binaries in tools/bin are outdated and must not be used (see tools/bin/OUTDATED.txt).

What it does:
  1. compiles the tools from tools/src into build/win/bin (tr3pcpsx from tools/src/tr3pcpsx/mod,
     texmod, troi, tr3mesh, trmod, trsfx, 3texrep);
  2. turns levels/convert.sh into build/win/convert_win.sh: work folder build/win/levels instead of
     /tmp/levels, the Windows tools, and the slow per-sector FloorData loop replaced by
     tools/optimise_floordata.py (same result);
  3. runs it from levels/; the converted levels end up in <out>/out/<version>/<level>.psx.
The results are NOT copied to levels/out; compare them and copy them yourself.

Usage (from anywhere):
  py -3 tools/windows_build.py [--levels scotland,zoo] [--versions us,jp] [--out DIR] [--tools-only]
                               [--convert other_convert.sh]  (e.g. a test variant in ergebnisse/)
Defaults: all levels and versions of convert.sh, out = build/win/result.
Needs: gcc (MinGW) and bash (Git Bash) in PATH."""
import argparse
import os
import shutil
import subprocess
import sys

PRJ = os.path.normpath(os.path.join(os.path.dirname(os.path.abspath(__file__)), ".."))
SRC = os.path.join(PRJ, "tools", "src")
WIN = os.path.join(PRJ, "build", "win")
BIN = os.path.join(WIN, "bin")

TOOLS = {  # name: (source folder or file, extra gcc options)
    "tr3pcpsx": (os.path.join(SRC, "tr3pcpsx", "mod"), []),
    "texmod": (os.path.join(SRC, "texmod"), []),
    "troi": (os.path.join(SRC, "troi"), []),
    "tr3mesh": (os.path.join(SRC, "tr3mesh"), []),
    "trmod": (os.path.join(SRC, "trmod"), []),
    "trsfx": (os.path.join(SRC, "trsfx"), []),
    "3texrep": (os.path.join(SRC, "3texrep.c"), []),
    # for the builder (tools/builder): texture export, the converter without built-in code modules, mkpsxiso
    "3texex": (os.path.join(SRC, "3texex.c"), []),
    "tr3pcpsx_builder": (os.path.join(SRC, "tr3pcpsx", "mod"), ["-DEXTERNAL_MODULES"]),
    "mkpsxiso": (os.path.join(SRC, "mkpsxiso", "src"), ["-std=c++11", "-static"]),   # no MinGW DLLs next to it
}


def posix(p):
    """Git Bash path of a Windows path (E:\\x -> /e/x)."""
    p = os.path.abspath(p).replace("\\", "/")
    return "/" + p[0].lower() + p[2:] if p[1] == ":" else p


def build_tools():
    os.makedirs(BIN, exist_ok=True)
    for name, (src, opts) in TOOLS.items():
        files = [src] if src.endswith(".c") else sorted(
            os.path.join(src, f) for f in os.listdir(src) if f.endswith((".c", ".cpp")))
        exe = os.path.join(BIN, name + ".exe")
        compiler = "g++" if any(f.endswith(".cpp") for f in files) else "gcc"
        r = subprocess.run([compiler, "-O2", "-o", exe] + files + opts, capture_output=True, text=True)
        if r.returncode != 0:
            sys.exit(f"{name}: gcc failed\n{r.stderr}")
        print(f"built {exe}")


def make_script(levels, versions, outdir, convert=None):
    s = open(convert or os.path.join(PRJ, "levels", "convert.sh"), encoding="utf-8", newline="").read().replace("\r", "")
    work = posix(os.path.join(WIN, "levels"))
    if levels:
        old = 'levels="scotland,willsden,chunnel,undersea,underse2,zoo,slinc"'
        assert s.count(old) == 1
        s = s.replace(old, f'levels="{levels}"', 1)
    if versions:
        old = 'versions="us,ukrev,derev,frrev,itrev,sprev,jp,jp2,as,as2"'
        assert s.count(old) == 1
        s = s.replace(old, f'versions="{versions}"', 1)
    # the original block also empties levels/out in the project ("rm -rf ./out"): never here
    old_setup = ("rm -rf ./out\nrm -rf /tmp/levels\ncp -r ../levels /tmp/.\ncp ../tools/bin/* /tmp/levels/.\n"
                 "cd /tmp/levels/.\n")
    assert s.count(old_setup) == 1, "convert.sh setup block changed"
    b = posix(BIN)
    new_setup = (f'rm -rf "{work}"\nmkdir -p "{posix(WIN)}"\ncp -r ../levels "{work}"\n'
                 f'rm -rf "{work}/out" "{work}/outsfx"\n'
                 + "".join(f'cp "{b}/{t}.exe" "{work}/{t}"\n' for t in TOOLS)
                 + f'cp "{b}/trmod.exe" "{work}/trmod.exe"\ncd "{work}"\n')
    s = s.replace(old_setup, new_setup, 1)
    # safety: before changing into the work folder nothing may be deleted except the work folder
    head = s[s.index("prjdir="):s.index(f'cd "{work}"')]
    for line in head.split("\n"):
        assert not line.startswith("rm ") or f'"{work}' in line, f"unsafe line before the work folder: {line}"
    # FloorData optimisation: per-sector trmod loop -> one Python pass (same result, much faster)
    start = s.index('for level in ${levels}; do\n\tfd="$(./trmod')
    end = s.index("done\n", s.index("for fdline in ${fd}"))
    end = s.index("done\n", end + 5) + 5
    s = (s[:start] + f'for level in ${{levels}}; do\n\tpy -3 "{posix(os.path.join(PRJ, "tools", "optimise_floordata.py"))}" '
         './trmod.exe "${level}.tr2" || exit 1\ndone\n' + s[end:])
    tail = 'cd "${prjdir}"\ncp -r /tmp/levels/out .'
    assert s.count(tail) == 1, "convert.sh end block changed"
    s = s.replace(tail, f'cd "${{prjdir}}"\nrm -rf "{posix(outdir)}"\nmkdir -p "{posix(outdir)}"\n'
                        f'cp -r "{work}/out" "{posix(outdir)}/"\ncp -r "{work}"/*.tr2 "{posix(outdir)}/"', 1)
    assert "/tmp/" not in s, "convert.sh still uses /tmp"
    dst = os.path.join(WIN, "convert_win.sh")
    open(dst, "w", encoding="utf-8", newline="\n").write(s)
    return dst


if __name__ == "__main__":
    ap = argparse.ArgumentParser()
    ap.add_argument("--levels")
    ap.add_argument("--versions")
    ap.add_argument("--out", default=os.path.join(WIN, "result"))
    ap.add_argument("--tools-only", action="store_true")
    ap.add_argument("--convert", help="use this convert.sh instead of levels/convert.sh (tests)")
    a = ap.parse_args()
    build_tools()
    if a.tools_only:
        sys.exit(0)
    script = make_script(a.levels, a.versions, a.out, a.convert)
    print(f"running {script} ...")
    r = subprocess.run(["bash", posix(script)], cwd=os.path.join(PRJ, "levels"))
    if r.returncode != 0:
        sys.exit(f"build failed ({r.returncode})")
    print(f"done: {a.out}/out/<version>/<level>.psx and the processed PC levels {a.out}/*.tr2 (for TRView)")
