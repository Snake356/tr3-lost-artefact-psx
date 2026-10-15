"""Turns a finished Japan level (Japan/Asia Disc 1: more pickups, fewer traps) into trmod commands for
levels/convert.sh, so the Japan versions are built from levels/in like every other version.

It compares the level as convert.sh produces it (after all steps up to the FloorData optimisation,
e.g. build/win/result/<level>.tr2) with the finished Japan level and writes levels/japan/<level>.txt:
  - removeitem(i,1) for removed items (highest index first; ",1" also renumbers the triggers),
  - replaceitem for changed items, additem for new items (appended, as in the Japan level),
  - replacefloordata for every sector whose FloorData differs afterwards,
  - replacerectangle/replacetriangle for room faces that differ (e.g. another texture).
Then it applies the commands to a copy and checks that items, FloorData and room faces are
identical to the Japan level. Only these kinds of changes are supported; anything else (rooms,
lights, models) must be the same in both levels, which is checked too.

Usage: make_japan_commands.py <trmod> <normal.tr2> <japan.tr2> <out.txt> [<donor.tr2> <object id>...]
With a donor (levels/in/<level>.tr2) the models of the given objects are put back with
restore_model.py before the check, as convert.sh does it for the Japan versions.
"""
import difflib
import os
import re
import shutil
import struct
import subprocess
import sys
import tempfile

import strip_unused_tail as T

BATCH = 60  # commands per trmod call


def trmod(exe, level, *cmds):
    r = subprocess.run([exe, "tr3pc", level, *cmds], capture_output=True, text=True)
    if r.returncode != 0 or "rror" in r.stdout:
        sys.exit(f"trmod failed on {cmds[:3]}...: {r.stdout[-500:]} {r.stderr[-500:]}")
    return r.stdout


def run_all(exe, level, cmds):
    for i in range(0, len(cmds), BATCH):
        trmod(exe, level, *cmds[i:i + BATCH])


def items(exe, level):
    return re.findall(r'"(additem\([^"]*\))"', trmod(exe, level, "cgetitem(*)"))


def floordata(exe, level):
    out = trmod(exe, level, "cgetfloordata(*,*,*)")
    return {k: c for c, k in re.findall(r'"(replacefloordata\((\d+,\d+,\d+):[^"]*\))"', out)}


def faces(path):
    """{(room, 'r'|'t', index): raw bytes} of all room faces."""
    b = open(path, "rb").read()
    u16 = lambda o: struct.unpack_from("<H", b, o)[0]
    u32 = lambda o: struct.unpack_from("<I", b, o)[0]
    L = T.layout(b)
    p = L["rooms"]
    nr = u16(p); p += 2
    f = {}
    for r in range(nr):
        p += 16
        ndw = u32(p); p += 4
        d = p
        nv = u16(d); d += 2 + nv * 12
        nq = u16(d); d += 2
        for k in range(nq):
            f[(r, "r", k)] = b[d + k * 10:d + k * 10 + 10]
        d += nq * 10
        nt = u16(d); d += 2
        for k in range(nt):
            f[(r, "t", k)] = b[d + k * 8:d + k * 8 + 8]
        p += ndw * 2
        p += 2 + u16(p) * 32
        zs, xs = u16(p), u16(p + 2); p += 4 + zs * xs * 8
        p += 4
        p += 2 + u16(p) * 24
        p += 2 + u16(p) * 20
        p += 7
    return f


def rest_without_items_fd(path):
    """Everything except FloorData, the sector FloorData indices, room faces and items, for the check.
    Unused editor leftovers are removed first (as strip_unused_tail.py does)."""
    tmp = path + ".strip"
    subprocess.run([sys.executable, os.path.join(os.path.dirname(os.path.abspath(__file__)), "strip_unused_tail.py"),
                    path, tmp], check=True, capture_output=True)
    b = bytearray(open(tmp, "rb").read())
    os.remove(tmp)
    L = T.layout(b)
    u16 = lambda o: struct.unpack_from("<H", b, o)[0]
    u32 = lambda o: struct.unpack_from("<I", b, o)[0]
    p = L["rooms"]
    nr = u16(p); p += 2
    for r in range(nr):
        p += 16
        ndw = u32(p); p += 4
        d = p
        nv = u16(d); d += 2 + nv * 12
        nq = u16(d); b[d + 2:d + 2 + nq * 10] = bytes(nq * 10); d += 2 + nq * 10
        nt = u16(d); b[d + 2:d + 2 + nt * 8] = bytes(nt * 8)
        p += ndw * 2
        p += 2 + u16(p) * 32
        zs, xs = u16(p), u16(p + 2)
        for k in range(zs * xs):
            b[p + 4 + k * 8:p + 6 + k * 8] = b"\0\0"      # FloorData index
        p += 4 + zs * xs * 8
        p += 4
        p += 2 + u16(p) * 24
        p += 2 + u16(p) * 20
        p += 7
    fd0 = L["floordata"][0]
    an0 = L["anims"][0]
    it0 = L["items"][0]
    it_end = it0 + 4 + L["items"][1] * 24
    md = L["meshdata"][0] + 4
    pp, n, _ = L["meshptr"]
    meshes = b"".join(b[md + o:md + o + T.mesh_info(b, md + o)[0]]
                      for o in (struct.unpack_from("<I", b, pp + 4 + i * 4)[0] for i in range(n)))
    return bytes(b[:fd0]) + meshes + bytes(b[an0:it0]) + bytes(b[it_end:])


def face_cmd(exe, level, key):
    room, kind, k = key
    get = "cgetrectangle" if kind == "r" else "cgettriangle"
    out = trmod(exe, level, f"{get}({room},{k})")
    m = re.search(r'"((?:add|replace)(?:rectangle|triangle)\([^"]*\))"', out)
    cmd = m.group(1)
    if cmd.startswith("add"):
        name = "replacerectangle" if kind == "r" else "replacetriangle"
        cmd = re.sub(r"^add(rectangle|triangle)\((\d+),", rf"{name}(\2,{k},", cmd)
    return cmd


def main(exe, normal, japan, out, donor=None, *restore):
    tmp = tempfile.mkdtemp()
    work = os.path.join(tmp, "level.tr2")
    shutil.copyfile(normal, work)
    if donor:
        subprocess.run([sys.executable, os.path.join(os.path.dirname(os.path.abspath(__file__)), "restore_model.py"),
                        donor, work, work, *restore], check=True, capture_output=True)
    a, b = items(exe, normal), items(exe, japan)
    cmds, removes = [], []
    ops = difflib.SequenceMatcher(None, a, b, autojunk=False).get_opcodes()
    replaces, adds = [], []
    for tag, i1, i2, j1, j2 in ops:
        if tag == "delete":
            removes += range(i1, i2)
        elif tag == "insert":
            if i1 != len(a):
                sys.exit(f"items inserted in the middle (before normal item {i1}) - not supported")
            adds += b[j1:j2]
        elif tag == "replace":
            if i2 - i1 != j2 - j1:
                sys.exit(f"replace of different length at {i1} - not supported")
            replaces += [(j1 + k, b[j1 + k]) for k in range(j2 - j1)]
    cmds += [f"removeitem({i},1)" for i in sorted(removes, reverse=True)]
    cmds += ["replaceitem(%d,%s" % (j, s[len("additem("):]) for j, s in replaces]
    cmds += adds
    run_all(exe, work, cmds)
    fa, fb = floordata(exe, work), floordata(exe, japan)
    fdcmds = [f"removefloordata({k})" for k in fa if k not in fb]
    changed = [fb[k] for k in fb if fa.get(k) != fb[k]]
    fdcmds += sorted(changed, key=len, reverse=True)
    run_all(exe, work, fdcmds)
    ga, gb = faces(work), faces(japan)
    assert ga.keys() == gb.keys(), "different number of room faces"
    facecmds = [face_cmd(exe, japan, k) for k in sorted(gb) if ga[k] != gb[k]]
    run_all(exe, work, facecmds)
    # check
    assert items(exe, work) == b, "items differ"
    assert floordata(exe, work) == fb, "FloorData differs"
    assert faces(work) == gb, "room faces differ"
    assert rest_without_items_fd(work) == rest_without_items_fd(japan), \
        "the Japan level has other changes (rooms, lights, models...) that are not supported"
    os.makedirs(os.path.dirname(os.path.abspath(out)), exist_ok=True)
    with open(out, "w", encoding="utf-8", newline="\n") as f:
        f.write(f"# Japan/Asia Disc 1 changes, made by tools/make_japan_commands.py from {os.path.basename(japan)}\n")
        f.write(f"# {len(removes)} items removed, {len(replaces)} changed, {len(adds)} added, "
                f"{len(fdcmds)} sectors, {len(facecmds)} room faces\n")
        for c in cmds + fdcmds + facecmds:
            f.write(c + "\n")
    shutil.rmtree(tmp)
    print(f"{out}: {len(removes)} removed, {len(replaces)} changed, {len(adds)} added, "
          f"{len(fdcmds)} sectors, {len(facecmds)} faces - check ok")


if __name__ == "__main__":
    if len(sys.argv) < 5 or len(sys.argv) == 6:
        sys.exit(__doc__)
    main(*sys.argv[1:])
