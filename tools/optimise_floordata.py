"""Windows-safe equivalent of the FloorData optimisation loop in levels/convert.sh:
reads every sector's floor data as trmod commands, removes all floor data, then
re-adds it longest command first (trmod deduplicates identical sequences).

Usage: optimise_floordata.py <trmod executable> <level.tr2>
"""
import re
import subprocess
import sys

trmod, level = sys.argv[1], sys.argv[2]
BATCH = 8000  # same shell buffer size as convert.sh


def run(*cmds):
    r = subprocess.run([trmod, "tr3pc", level, *cmds], capture_output=True, text=True)
    if r.returncode != 0:
        sys.exit(f"trmod failed: {r.stderr or r.stdout}")
    return r.stdout


out = run("cgetfloordata(*,*,*)")
cmds = re.findall(r'"(replacefloordata\([^"]*\))"', out)
if not cmds:
    sys.exit("no floor data commands read -- aborting before removing anything")
cmds.sort(key=len, reverse=True)
run("removefloordata(*,*,*)")
batch, size = [], 0
for c in cmds:
    if batch and size + len(c) + 1 > BATCH:
        run(*batch)
        batch, size = [], 0
    batch.append(c)
    size += len(c) + 1
if batch:
    run(*batch)
print(f"{level}: {len(cmds)} floor data commands re-added")
