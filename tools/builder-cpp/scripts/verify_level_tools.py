"""Checks the C++ level tools against the Python tools of the project on the real inputs.

Runs the project's level conversion (tools/builder/convert.py, the repository's textures/anims/meshes) for one
version. Every Python tool call of it is done twice: the Python tool in the level's folder and the C++ tool
(builder --tool <name> ...) in a copy of the folder made just before; then every file of both folders is
compared. Tools the C++ builder does not have yet are reported as not ported (the Python result goes on).

Usage: verify_level_tools.py <builder exe> [--levels scotland,zoo] [--only add_crawl_flip.py,...] [--work <folder>]"""
import argparse
import os
import shutil
import subprocess
import sys

PRJ = os.path.abspath(os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "..", ".."))
sys.path.insert(0, PRJ + "/tools/builder")
sys.path.insert(0, PRJ + "/tools")
import convert  # noqa: E402

ap = argparse.ArgumentParser()
ap.add_argument("exe")
ap.add_argument("--levels")
ap.add_argument("--only")
ap.add_argument("--work", default=os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "scratch", "verify_tools"))
a = ap.parse_args()
exe = os.path.abspath(a.exe)
ported = set(subprocess.run([exe, "--tools"], capture_output=True, text=True).stdout.split())
only = set(a.only.split(",")) if a.only else None
results = []


def files(folder):
    out = {}
    for root, _, names in os.walk(folder):
        for n in names:
            p = os.path.join(root, n)
            out[os.path.relpath(p, folder)] = p
    return out


def compare(lv, tool, args, mine_dir, ref_dir, cpp_out):
    a_files, b_files = files(ref_dir), files(mine_dir)
    bad = sorted(set(a_files) ^ set(b_files))
    for rel in sorted(set(a_files) & set(b_files)):
        if open(a_files[rel], "rb").read() != open(b_files[rel], "rb").read():
            bad.append(rel)
    state = "SAME" if not bad else "DIFFERENT " + ", ".join(bad)
    results.append((lv, tool, " ".join(args), state, cpp_out.strip()[-300:] if bad else ""))
    print(f"{lv:9s} {tool:24s} {state}", flush=True)


orig_py = convert.Converter.py


def checked_py(self, lv, tool, *args):
    if tool not in ported or (only and tool not in only):
        if not only or tool in only:
            results.append((lv, tool, " ".join(args), "NOT PORTED", ""))
        return orig_py(self, lv, tool, *args)
    folder = self.path(lv)
    mine = folder + "__cpp"
    shutil.rmtree(mine, ignore_errors=True)
    shutil.copytree(folder, mine)
    orig_py(self, lv, tool, *args)
    r = subprocess.run([exe, "--tool", tool] + [x.replace(folder, mine) for x in args], cwd=mine,
                       capture_output=True, text=True, errors="replace")
    compare(lv, tool, args, mine, folder, r.stdout + (f"\nexit {r.returncode}" if r.returncode else ""))
    shutil.rmtree(mine, ignore_errors=True)


convert.Converter.py = checked_py
c = convert.Converter(os.path.abspath(a.work), os.path.join(PRJ, "levels", "in"), os.path.join(PRJ, "levels", "sfx", "common"),
                      levels=a.levels.split(",") if a.levels else None, versions=["us"])
c.convert()
print()
same = sum(1 for r in results if r[3] == "SAME")
diff = [r for r in results if r[3].startswith("DIFFERENT")]
todo = sorted({r[1] for r in results if r[3] == "NOT PORTED"})
print(f"{same} same, {len(diff)} different, not ported: {', '.join(todo) or '-'}")
for lv, tool, args, state, out in diff:
    print(f"  {lv} {tool} {args}: {state}\n    {out}")
sys.exit(1 if diff else 0)
