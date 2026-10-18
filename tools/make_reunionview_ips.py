"""Builds bin/patches/<version>/reunionview.ips: a shorter draw distance and fog for some levels only
(Highland Fling, level 1: draw distance 14 for the rooms, and Reunion, level 7: 12 for the rooms, fog from 12 over 4
blocks, in every version, so they run smoother; the other levels keep the original values).

The PS1 executable has no variables for these; the values are immediates in the drawing code:
  - draw distance (20 blocks): vertex counters of the room and object renderers (sltiu ..,0x5000),
    object culling (addi ..,-0x5000), room vertex transform (slti ..,0x5000) and sprites/particles
    (z-0x100 < 0x4F00 and z < 0x5000),
  - fog start (12 blocks): z-0x3000 in the room renderer and in both object/figure lighting paths,
  - fog length (8 blocks): rooms darken 32 steps over it (slti ..,0x1FFF / srl 8), objects get the
    GTE interpolation factor IR0 = (z-start) >> 1 (4096 = full fog).
A small routine, called when a level is initialised (InitialiseLevel calls it instead of the function
it called there, and jumps on to that function), writes the values into these instructions for the chosen levels
(a bit mask of level numbers) and flushes the instruction cache (the game's own FlushCache wrapper). All other levels get the original
values back, so nothing changes for them and the drawing code runs as fast as before.
The routine sits in a library function nothing calls (Japanese and Asian executables) or, in the other
versions, in two game functions nothing refers to (code and tables apart): no jump, pointer or
lui/addiu pair in the executable, no reference from the version's code modules and no pointer in any
disc file (checked, see modules() and unused_routines()).

A level can also get the shorter distance for the room selection only ("r" after its values): rooms
further away are not drawn, but objects, room vertices and sprites keep the original 20 blocks, so a
distant object in a drawn room (the Loch Ness monster in Highland Fling) stays visible.

Usage: make_reunionview_ips.py [--versions us,ukrev,...] [--exe VERSION=PATH ...] [--werte 1=14,12,8,r 7=12,12,4]
(distances in blocks of 1024; the fog length must be 1, 2, 4, 8 or 16; --exe for executables not in
bin/orig, e.g. the first releases us10, uk, de, fr, it, sp)
Checks that bin/patch.sh's patches rebuild bin/mod exactly first, like the other make_*_ips.py tools."""
import argparse
import os
import re
import struct

HERE = os.path.dirname(os.path.abspath(__file__))
BIN = os.path.join(HERE, "..", "bin")
MODBLOB = os.path.join(HERE, "src", "tr3pcpsx", "mod", "modblob.h")
MODVER = {"us": "us_11", "ukrev": "uk_r1", "derev": "de_r1", "frrev": "fr_r1", "itrev": "it_r1", "sprev": "sp_r1",
          "jp": "jp1", "jp2": "jp2", "as": "asia1", "as2": "asia2",
          # Rev 0: the module blobs are the US 1.0 overlays, or the plain blobs for the European first releases
          "us10": "us_10", "uk": None, "de": None, "fr": None, "it": None, "sp": None}
LOAD = 0x80010000 - 0x800          # memory address = file offset + LOAD

ZERO, AT, V0, V1, A0, A1, A2, A3, T0, T1, T2, T3, T4, T5 = 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13
S2, SP, RA = 18, 29, 31


def I(op, rs, rt, imm): return (op << 26) | (rs << 21) | (rt << 16) | (imm & 0xFFFF)
def addiu(rt, rs, imm): return I(9, rs, rt, imm)
def lui(rt, imm): return I(15, 0, rt, imm)
def lw(rt, off, base): return I(35, base, rt, off)
def sw(rt, off, base): return I(43, base, rt, off)
def bne(rs, rt, off): return I(5, rs, rt, off)
def beq(rs, rt, off): return I(4, rs, rt, off)
def ori(rt, rs, imm): return I(13, rs, rt, imm)
def andi(rt, rs, imm): return I(12, rs, rt, imm)
def srlv(rd, rt, rs): return (rs << 21) | (rt << 16) | (rd << 11) | 0x06
def jal(t): return (3 << 26) | ((t >> 2) & 0x3FFFFFF)
def j(t): return (2 << 26) | ((t >> 2) & 0x3FFFFFF)
def hi_lo(a): return ((a + 0x8000) >> 16) & 0xFFFF, a & 0xFFFF


def apply_ips(data, ips):
    data = bytearray(data)
    assert ips[:5] == b"PATCH"
    p = 5
    while ips[p:p + 3] != b"EOF":
        off = int.from_bytes(ips[p:p + 3], "big"); size = int.from_bytes(ips[p + 3:p + 5], "big"); p += 5
        if size:
            chunk = ips[p:p + size]; p += size
        else:
            rle = int.from_bytes(ips[p:p + 2], "big"); chunk = ips[p + 2:p + 3] * rle; p += 3
        data[off:off + len(chunk)] = chunk
    return bytes(data)


def words(exe):
    return list(struct.unpack_from(f"<{(len(exe) - 0x800) // 4}I", exe, 0x800))


def find_one(W, pattern, what):
    """pattern: list of (word, (word, mask) or None); returns the file offset of the only match."""
    n = len(pattern)

    def ok(w, p):
        return p is None or (w & p[1] == p[0] if isinstance(p, tuple) else w == p)
    hits = [i for i in range(len(W) - n) if all(ok(W[i + k], p) for k, p in enumerate(pattern))]
    assert len(hits) == 1, f"{what}: {len(hits)} matches"
    return 0x800 + hits[0] * 4


def modules(ver):
    """Executable addresses the version's code modules refer to (jal targets, lui/addiu pairs, words)."""
    src = open(MODBLOB).read()
    arr = {m.group(2): [int(x, 0) for x in m.group(4).split(",") if x.strip()]
           for m in re.finditer(r"const INTU(8|16) (\w+)\[(\d+)\] = \{([^}]*)\}", src)}
    tag = MODVER[ver]
    refs = set()
    if tag is None:
        names = [k[len("module_base_"):] for k in arr if k.startswith("module_base_")]
    else:
        names = [k[len(f"overval_{tag}_"):] for k in arr if k.startswith(f"overval_{tag}_")]
    for name in names:
        code = bytearray(arr["module_base_" + name])
        for p, v in zip(arr.get(f"overpos_{tag}_{name}", []), arr.get(f"overval_{tag}_{name}", [])):
            code[p] = v
        ws = [struct.unpack_from("<I", code, i)[0] for i in range(0, len(code) - 3, 4)]
        refs |= xrefs(ws)
    return refs


def xrefs(ws):
    refs = set()
    for i, w in enumerate(ws):
        if w >> 26 in (2, 3):
            refs.add(0x80000000 | ((w & 0x3FFFFFF) << 2))
        if 0x80010000 <= w < 0x80200000:
            refs.add(w)
        if w >> 26 == 15:
            rt, hi = (w >> 16) & 31, (w & 0xFFFF) << 16
            for w2 in ws[i + 1:i + 40]:
                if (w2 >> 21) & 31 == rt and w2 >> 26 in (9, 13, 35, 43, 33, 37, 41, 32, 36, 40):
                    imm = w2 & 0xFFFF
                    if w2 >> 26 != 13 and imm & 0x8000:
                        imm -= 0x10000
                    refs.add((0x80000000 | hi) + imm & 0xFFFFFFFF)
                if w2 >> 26 == 15 and (w2 >> 16) & 31 == rt:
                    break
    return refs


def sites(exe):
    """[(file offset, original word, function(values) -> new word)]"""
    W = words(exe)
    out = []

    def imm_repl(word, imm): return (word & 0xFFFF0000) | (imm & 0xFFFF)

    # vertex counters, room/object renderer: sltiu at, tX, 0x5000 (three in a row, twice)
    for regs in ((9, 15, 25), (11, 12, 13)):
        o = find_one(W, [0x2C015000 | (regs[0] << 21), None, None, 0x2C015000 | (regs[1] << 21), None, None,
                         0x2C015000 | (regs[2] << 21)], f"vertex counter {regs}")
        for k in (0, 3, 6):
            w = W[(o - 0x800) // 4 + k]
            out.append((o + 4 * k, w, lambda v, w=w: imm_repl(w, v["far"])))
    o = find_one(W, [0x2108B000, None, 0x94880000], "object culling addi t0,-0x5000")
    out.append((o, W[(o - 0x800) // 4], lambda v: imm_repl(0x2108B000, -v["far"])))
    o = find_one(W, [0x2B215000], "room vertex transform slti at,t9,0x5000")
    out.append((o, 0x2B215000, lambda v: imm_repl(0x2B215000, v["far"])))
    # sprites/particles: addiu v0,vX,-0x100 ; sltiu v0,v0,0x4F00 ; ... slti v0,vX,0x5000
    for rx, what in ((3, "v1"), (6, "a2")):
        o = find_one(W, [0x2402FF00 | (rx << 21), 0x2C424F00], f"sprite range ({what})")
        out.append((o + 4, 0x2C424F00, lambda v: imm_repl(0x2C424F00, v["far"] - 0x100)))
        o2 = find_one(W, [0x28025000 | (rx << 21)], f"sprite far slti ({what})")
        out.append((o2, 0x28025000 | (rx << 21), lambda v, w=0x28025000 | (rx << 21): imm_repl(w, v["far"])))
    # fog, rooms: addi t6,t0,-0x3000 ; bltz ; slti at,t6,0x1FFF ; beqz ; srl t6,t6,8
    o = find_one(W, [0x210ED000, None, 0x29C11FFF, None, 0x000E7202], "room fog")
    out.append((o, 0x210ED000, lambda v: imm_repl(0x210ED000, -v["fog"])))
    out.append((o + 8, 0x29C11FFF, lambda v: imm_repl(0x29C11FFF, v["len"] - 1)))
    out.append((o + 16, 0x000E7202, lambda v: 0x000E7002 | (v["room_shift"] << 6)))
    # fog, objects: addi t0,t0,-0x3000 ; blez ; sll ; srl t1,t0,1   and   addi a1,a1,-0x3000 ; blez ; nop ; srl t0,a1,1
    o = find_one(W, [0x2108D000, None, None, 0x00084842], "object fog 1")
    out.append((o, 0x2108D000, lambda v: imm_repl(0x2108D000, -v["fog"])))
    out.append((o + 12, 0x00084842, lambda v: obj_shift(0x00084842, v)))
    o = find_one(W, [0x20A5D000, None, None, 0x00054042], "object fog 2")
    out.append((o, 0x20A5D000, lambda v: imm_repl(0x20A5D000, -v["fog"])))
    out.append((o + 12, 0x00054042, lambda v: obj_shift(0x00054042, v)))
    for off, w, _ in out:
        assert W[(off - 0x800) // 4] == w, hex(off)
    return out


def routine_end(W, o):
    """File offset after the routine starting at o (its jr ra and the delay slot)."""
    while W[(o - 0x800) // 4] != 0x03E00008:
        o += 4
    return o + 8


def unused_routines(W, ver):
    """[(start, end) file offsets] of functions nothing refers to: no jal/j and no pointer (0x80... or the
    0x000... form the game also stores) in the executable or in the version's code modules, no low half of
    an address constant, and no pointer in any file of the disc."""
    refs, los = xrefs(W) | modules(ver), set()
    for w in W:
        if 0x10000 <= w < 0xA0000:
            refs.add(w | 0x80000000)
        if w >> 26 in (8, 9, 13):
            los.add(w & 0xFFFF)
    disc = b"".join(open(os.path.join(dp, f), "rb").read()
                    for dp, _, fs in os.walk(os.path.join(HERE, "..", "disc", "files")) for f in fs)
    out = []
    for i in range(2, len(W)):
        if W[i] >> 16 == 0x27BD and W[i] & 0x8000 and W[i - 2] == 0x03E00008:
            a = 0x800 + 4 * i + LOAD
            if a in refs or a & 0xFFFF in los or struct.pack("<I", a) in disc                     or struct.pack("<I", a & 0x7FFFFFFF) in disc:
                continue
            out.append((0x800 + 4 * i, routine_end(W, 0x800 + 4 * i)))
    return out


def obj_shift(word, v):
    """srl rd, rt, 1 -> srl by k (or sll by -k): IR0 = (z - start) scaled so that the fog length is 4096."""
    k = v["obj_shift"]
    base = word & ~(0x1F << 6) & ~0x3F
    return base | (abs(k) << 6) | (0x02 if k >= 0 else 0x00)


def values(far, fog, length):
    assert length in (1, 2, 4, 8, 16), "fog length must be 1, 2, 4, 8 or 16 blocks"
    L = length * 1024
    return {"far": far * 1024, "fog": fog * 1024, "len": L,
            "room_shift": (L // 32).bit_length() - 1,              # 32 darkening steps over the length
            "obj_shift": (L // 4096).bit_length() - 1 if L >= 4096 else -((4096 // L).bit_length() - 1)}


ROOM_SITES = {0x2B215000,                                          # room selection (portal corners)
              0x210ED000, 0x29C11FFF, 0x000E7202,                  # fog, rooms
              0x2108D000, 0x00084842, 0x20A5D000, 0x00054042}      # fog, objects


def build(exe, ver, settings):
    """settings: {level number: (draw distance, fog start, fog length[, rooms only])} in blocks."""
    W = words(exe)
    S = sites(exe)
    orig = values(20, 12, 8)
    for off, w, f in S:
        assert f(orig) == w, f"site {hex(off)}: original values do not rebuild the original word"
    levels = sorted(settings)
    assert all(0 <= l < 16 for l in levels)
    # hook: InitialiseLevel keeps the level number in s2 (move s2,a0) and stores it into CurrentLevel;
    # the first jal after that store is redirected
    o = find_one(W, [0x24020004, 0x12220003, 0x24020003, (0xAF920000, 0xFFFF0000)],
                 "InitialiseLevel: ... sw s2,CurrentLevel(gp)") + 12        # s2 = level number (move s2,a0)
    hook = next(o + 4 * k for k in range(1, 12) if W[(o - 0x800) // 4 + k] >> 26 == 3)
    orig_fn = 0x80000000 | ((W[(hook - 0x800) // 4] & 0x3FFFFFF) << 2)
    flush = find_one(W, [0x240A00A0, 0x01400008, 0x24090044], "FlushCache wrapper") + LOAD
    n = len(S)
    code_len = 28 + 5 * len(levels)
    table_len = n * (2 + len(levels))              # addresses, original words, one word list per level
    # free space: in the Japanese and Asian executables an unused library routine (copies a 32-byte default
    # block onto the stack) holds code and tables when they fit; otherwise (and in the other versions, which
    # do not have it) the code and the tables go into two game functions nothing refers to (unused_routines)
    jp_lib = [i for i in range(len(W) - 5) if W[i:i + 3] == [0x27BDFFB0, 0x00805021, 0x00A05821]
              and W[i + 3] >> 16 == 0x3C07 and W[i + 4] >> 16 == 0x24E7]
    cave = None
    if jp_lib:
        assert len(jp_lib) == 1
        c = 0x800 + jp_lib[0] * 4
        end = routine_end(W, c)
        if c + 4 * (code_len + table_len) <= end:
            outside = list(W)                      # references from the routine itself do not count
            for k in range((c - 0x800) // 4, (end - 0x800) // 4):
                outside[k] = 0
            refs = xrefs(outside) | modules(ver)
            assert not any(c + LOAD <= r < end + LOAD for r in refs), "the unused routine is referenced"
            cave, data, data_end = c, c + 4 * code_len, end
    if cave is None:
        free = unused_routines(W, ver)
        cave, end = min((r for r in free if r[1] - r[0] >= 4 * code_len), key=lambda r: r[1] - r[0])
        data, data_end = min((r for r in free if r[1] - r[0] >= 4 * table_len and r[0] != cave),
                             key=lambda r: r[1] - r[0])
    cm, dm = cave + LOAD, data + LOAD
    sites_a, dflt_a = dm, dm + 4 * n
    code = [
        addiu(SP, SP, -32), sw(RA, 28, SP), sw(A0, 16, SP), sw(A1, 20, SP), sw(A2, 24, SP), sw(A3, 12, SP),
        lui(T0, hi_lo(sites_a)[0]), addiu(T0, T0, hi_lo(sites_a)[1]),
        lui(T1, hi_lo(dflt_a)[0]), addiu(T1, T1, hi_lo(dflt_a)[1]),
    ]
    for k, lvl in enumerate(levels):               # this level? then its word list instead of the original
        tab = dm + 4 * n * (2 + k)
        code += [addiu(T2, ZERO, lvl), bne(S2, T2, 3), 0, lui(T1, hi_lo(tab)[0]), addiu(T1, T1, hi_lo(tab)[1])]
    code += [
        addiu(T3, ZERO, n),
        lw(T4, 0, T0), lw(T5, 0, T1), addiu(T0, T0, 4), addiu(T1, T1, 4), addiu(T3, T3, -1),
        bne(T3, ZERO, -6), sw(T5, 0, T4),
        jal(flush), 0,
        lw(A0, 16, SP), lw(A1, 20, SP), lw(A2, 24, SP), lw(A3, 12, SP), lw(RA, 28, SP), addiu(SP, SP, 32),
        j(orig_fn), 0,
    ]
    assert len(code) == code_len
    table = [off + LOAD for off, _, _ in S] + [w for _, w, _ in S]
    for lvl in levels:
        far, fog, length, *flags = settings[lvl]
        v = values(far, fog, length)
        keep = values(20, fog, length) if flags and flags[0] else v      # rooms only: the rest stays at 20
        table += [f(v if w in ROOM_SITES else keep) for _, w, f in S]
    assert len(table) == table_len
    assert cave + 4 * len(code) <= end and data + 4 * len(table) <= data_end, "does not fit"
    if data == cave + 4 * code_len:
        patches = [(hook, [jal(cm)]), (cave, code + table)]
    else:
        patches = [(hook, [jal(cm)]), (cave, code), (data, table)]
    return patches, dict(hook=hook, orig=orig_fn, flush=flush, cave=cave, data=data, sites=S)


def ips(patches):
    out = b"PATCH"
    for off, ws in patches:
        data = struct.pack(f"<{len(ws)}I", *ws)
        out += off.to_bytes(3, "big") + len(data).to_bytes(2, "big") + data
    return out + b"EOF"


if __name__ == "__main__":
    ap = argparse.ArgumentParser()
    ap.add_argument("--versions", default="us,ukrev,derev,frrev,itrev,sprev,jp,jp2,as,as2")
    ap.add_argument("--exe", nargs="*", default=[], help="VERSION=PATH for executables not in bin/orig")
    ap.add_argument("--werte", nargs="+", default=["1=14,12,8,r", "7=12,12,4,r"],
                    help="level=draw distance,fog start,fog length[,r] (blocks; r = rooms only); "
                         "1 = Highland Fling, 7 = Reunion")
    a = ap.parse_args()
    settings = {int(l): tuple(int(x) for x in v.split(",")[:3]) + (v.split(",")[3:] == ["r"],)
                for l, v in (w.split("=") for w in a.werte)}
    paths = {v: os.path.join(BIN, "orig", f"{v}.exe") for v in a.versions.split(",") if v}
    paths.update(dict(e.split("=", 1) for e in a.exe))
    patch_sh = open(os.path.join(BIN, "patch.sh"), encoding="utf-8").read()
    patches = [l.split('"')[1].split(",") for l in patch_sh.splitlines() if l.startswith("patches=")][0]
    for ver, path in paths.items():
        exe = open(path, "rb").read()
        for pat in [p for p in patches if p not in ("reunionview", "roomguard")]:
            exe = apply_ips(exe, open(os.path.join(BIN, "patches", ver, f"{pat}.ips"), "rb").read())
        p, info = build(exe, ver, settings)
        open(os.path.join(BIN, "patches", ver, "reunionview.ips"), "wb").write(ips(p))
        print(f"{ver}: hook {hex(info['hook'])} -> code {hex(info['cave'] + LOAD)} (then {hex(info['orig'])}), "
              f"tables {hex(info['data'] + LOAD)}, FlushCache {hex(info['flush'])}, {len(info['sites'])} sites, "
              + "; ".join(f"level {l}: draw distance {v[0]}" + (" (rooms only)" if v[3] else "")
                          + f", fog from {v[1]}, fog length {v[2]}" for l, v in sorted(settings.items())))
