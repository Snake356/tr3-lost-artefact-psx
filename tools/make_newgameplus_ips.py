"""Builds bin/patches/<version>/newgameplus.ips: New Game+ after finishing the game, as in Tomb Raider II.

The executable still has the bonus mode of the savegame (one byte): all weapons in every level from the
start, unlimited ammunition and 255 flares (the ammo display is hidden), enemies with twice their hit
points. In the original Tomb Raider III it is switched on only when level 20 is completed; The Lost
Artefact has seven levels, so it never is. Starting a new game computes the start equipment of every
level and only then clears the byte; switching the console on clears it as well.

This patch:
  - sets the byte when the game is finished (GAMECOMPLETE: the call of the end credits goes to a small
    routine that sets it and then jumps to the credits),
  - no longer clears it when a new game starts,
  - starts the bonus game with one save crystal, as a normal game (the start equipment gave 50; the
    first releases did not touch them),
  - puts the pistols into Lara's hands instead of the rocket launcher,
  - clears the shotgun in the first level's normal start equipment too (a normal game after a New Game+ in the
    same session kept it; it is a secret of Highland Fling),
  - gives a small and a large medipack in every level (the bonus part gave none; with the level select a
    later level started without),
  - gives limited ammunition instead of unlimited: the start equipment gets START_AMMO (Desert Eagle 10,
    Uzis 80, shotgun 12, MP5 80, rockets 4, harpoon 24, grenades 6; pistols unlimited as always) and
    START_FLARES flares instead of 10000 and 255; every place that treats the bonus game differently
    except the enemy hit points is switched off: ammunition and flares are used up, the empty Desert
    Eagle, Uzis, MP5 and shotgun are no longer refilled to 1000, and the ammunition counts are shown
    (this also ends the harpoon's reload after every bolt: it reloads when its count is a multiple
    of 4, and the bonus game's count never changed),
  - doubles the damage Lara takes in the bonus game: the health bar routine reads her hit points every
    frame; a routine before it compares them with the last frame and takes the loss once more. It
    keeps the last value with the level timer and skips the frame when the timer did not run on
    (0 to 30 frames), so loading a savegame or starting a level is no damage; demos are left alone.
So after the credits, back in the title menu, "New Game" is the New Game+. It stays until the console
is switched off or reset, or until a normal savegame is loaded (the byte is part of the savegame); a
game saved in the New Game+ keeps it.

Found in every version: the byte by the enemy hit points code (lui v0, hi; lbu v0, lo(v0); nop; beqz v0;
nop; lw v0, DemoPlay(gp); nop; bnez v0; nop; lh v0, 0x22(s0); nop; sll v0, v0, 1), the two places that
clear it (the one followed by a jal is the start-up, the other one the new game), the credits call by
the gameflow's command switch (23 commands, entry 15 = GAMECOMPLETE), the crystals in the bonus part of
the start equipment (addiu v0, zero, 50; sb v0, x(t4) after 'lbu v0, bonus; nop; beqz v0; nop; beqz t5'),
its ammunition (addiu v0, zero, 10000; sh v0, x(t4) seven times; addiu v0, zero, 255: flares), every
other read of the byte ('lui rX, hi; lbu rX, lo(rX)' and its first branch), the health bar routine (lw v0, lara_item(gp);
addiu sp; sw ra; lh a1, 0x22(v0); nop; bgez a1) and the level timer behind the health bar timer's
decrement (lw v0, timer(gp); ...; addiu v0, v0, -1; sw v0, timer(gp) ... lui v1, hi; addiu v1, lo). The
routines go into a game
function nothing refers to, found on the executable with the patches before this one in bin/patch.sh
(duckroll among them) applied, so they never share one.

Usage: make_newgameplus_ips.py [--versions us,ukrev,...] [--exe VERSION=PATH ...]"""
import argparse
import os
import struct

import make_reunionview_ips as rv

HERE = os.path.dirname(os.path.abspath(__file__))
BIN = os.path.join(HERE, "..", "bin")
LOAD = rv.LOAD
GAMECOMPLETE = 15


def word(d, o):
    return struct.unpack_from("<I", d, o)[0]


def bonus_byte(exe):
    """Memory address of the bonus byte."""
    hits = []
    for o in range(0x800, len(exe) - 52, 4):
        w = [word(exe, o + 4 * k) for k in range(13)]
        if (w[0] >> 16 == 0x3C02 and w[1] >> 16 == 0x9042 and w[2] == 0 and w[3] >> 16 == 0x1040 and w[4] == 0
                and w[5] >> 16 == 0x8F82 and w[6] == 0 and w[7] >> 16 == 0x1440 and w[8] == 0
                and w[9] == 0x86020022 and w[10] == 0 and w[11] == 0x00021040):
            lo = w[1] & 0xFFFF
            hits.append(((w[0] & 0xFFFF) << 16) + (lo - 0x10000 if lo & 0x8000 else lo))
    assert len(set(hits)) == 1, f"bonus byte found {len(hits)} times"
    return hits[0] & 0xFFFFFFFF


def clears(exe, addr):
    """File offsets of 'lui at, hi; sb zero, lo(at)' for the byte: (start-up, new game)."""
    hi, lo = ((addr + 0x8000) >> 16) & 0xFFFF, addr & 0xFFFF
    sites = [o + 4 for o in range(0x800, len(exe) - 12, 4)
             if word(exe, o) == 0x3C010000 | hi and word(exe, o + 4) == 0xA0200000 | lo]
    assert len(sites) == 2, f"the byte is cleared {len(sites)} times"
    boot = [s for s in sites if word(exe, s + 4) >> 26 == 3]
    assert len(boot) == 1, "start-up clearing not recognised"
    return boot[0], [s for s in sites if s != boot[0]][0]


def credits_call(exe):
    """File offset of the 'jal credits' at the start of the GAMECOMPLETE case and the credits address."""
    for o in range(0x800, len(exe) - 32, 4):
        a, b, c, d_, e, f = (word(exe, o + 4 * k) for k in range(6))
        if (a & 0xFC1F0000) == 0x2C020000 and (a & 0xFFFF) == 23 and b >> 16 == 0x1040 \
                and c >> 21 == 0 and (c & 0x3F) == 0 and d_ == 0x3C010000 | (d_ & 0xFFFF):
            hi = d_ & 0xFFFF
            lw = f
            if lw >> 16 == 0x8C22:                          # lw v0, lo(at)
                lo = lw & 0xFFFF
                table = (hi << 16) + (lo - 0x10000 if lo & 0x8000 else lo)
                case = word(exe, (table & 0x0FFFFFFF) + 4 * GAMECOMPLETE - 0x10000 + 0x800)
                site = (case | 0x80000000) - LOAD
                j = word(exe, site)
                assert j >> 26 == 3, "GAMECOMPLETE does not start with a call"
                return site, (j & 0x03FFFFFF) << 2 | 0x80000000
    raise AssertionError("gameflow command switch not found")


def crystals(exe, addr):
    """File offset of 'addiu v0, zero, 50' in the bonus part of the start equipment, or None (the first
    releases leave the crystals alone)."""
    hi, lo = ((addr + 0x8000) >> 16) & 0xFFFF, addr & 0xFFFF
    for o in range(0x800, len(exe) - 0x100, 4):
        if word(exe, o) == 0x3C020000 | hi and word(exe, o + 4) == 0x90420000 | lo \
                and word(exe, o + 12) >> 16 == 0x1040 and word(exe, o + 20) >> 16 == 0x11A0:
            for k in range(o, o + 0x100, 4):
                if word(exe, k) == 0x03E00008:
                    return None
                if word(exe, k) == 0x24020032 and word(exe, k + 4) >> 16 == 0xA182:
                    return k
    raise AssertionError("bonus start equipment not found")


# New Game+ start ammunition (start_info offset: value; the shotgun counts 6 per shell)
START_AMMO = {2: 10, 4: 80, 6: 12 * 6, 8: 80, 0xA: 4, 0xC: 24, 0xE: 6}   # Desert Eagle, Uzis, shotgun,
PISTOLS = 1                                                               # gun type
START_FLARES = 2
START_MEDIS = 1                                                           # one of each, as the first level                                                          # MP5, rockets, harpoon, grenades


def bonus_checks(exe, addr):
    """The places that read the bonus byte ('lui rX, hi; lbu rX, lo(rX)') and their first branch:
    'bnez' skips something in the bonus game (ammo--, flares--, the ammo counts in the inventory):
    becomes nop; 'beqz' does something only in the bonus game (hide the ammo text, refill to 1000):
    becomes an unconditional branch. Kept: the enemy hit points (beqz; nop; lw v0, DemoPlay(gp)) and
    the start equipment (beqz; nop; beqz t5). Returns (patches, start equipment branch offset)."""
    hi, lo = ((addr + 0x8000) >> 16) & 0xFFFF, addr & 0xFFFF
    patches, start = [], None
    for o in range(0x800, len(exe) - 40, 4):
        x = word(exe, o)
        if x >> 26 == 0x0F and x & 0xFFFF == hi and word(exe, o + 4) == 0x90000000 | ((x >> 16) & 31) * 0x210000 | lo:
            b = next(k for k in range(o + 8, o + 24, 4) if word(exe, k) >> 26 in (4, 5))
            w = word(exe, b)
            if w >> 16 == 0x1440:                                # bnez v0
                patches.append((b, [0]))
            elif w >> 16 == 0x1040:                              # beqz v0
                if word(exe, b + 8) >> 16 == 0x8F82 and word(exe, b + 4) == 0:
                    continue                                     # enemy hit points
                if word(exe, b + 8) >> 16 == 0x11A0:
                    start = b + 8                                # 'beqz t5' of the start equipment
                    continue
                patches.append((b, [0x10000000 | w & 0xFFFF]))   # beq zero, zero
    assert start is not None and len(patches) >= 15, f"bonus checks: {len(patches)}"
    return patches, start


def start_equipment(exe, start, cave):
    """At 'beqz t5; addiu v0, zero, 10000; sh v0, x(t4) * 7; addiu v0, zero, 255': the first store jumps
    to a routine that stores START_AMMO and START_MEDIS and jumps back behind the seven stores; flares
    START_FLARES."""
    o = start + 8
    assert word(exe, start + 4) == 0x24022710 and all(word(exe, o + 4 * k) >> 16 == 0xA582 for k in range(7))
    assert word(exe, o + 28) == 0x240200FF
    back = o + 28 + LOAD
    code = []
    for off, v in START_AMMO.items():
        code += [0x24080000 | v, 0xA5880000 | off]               # addiu t0, zero, v; sh t0, off(t4)
    code += [0x24080000 | START_MEDIS, 0xA1880010, 0xA1880011]   # small and large medipacks (sb t0, 0x10/0x11(t4))
    code += [0x08000000 | (back & 0x0FFFFFFF) >> 2, 0]           # j back
    patches = [(o, [0x08000000 | ((cave + LOAD) & 0x0FFFFFFF) >> 2] + [0] * 6),
               (o + 28, [0x24020000 | START_FLARES])]
    return code, patches


def gun_type(exe, start):
    """File offset of 'addiu v0, zero, 6' (the rocket launcher) before 'sb v0, x(t4)' in the bonus part of the
    start equipment: the weapon in Lara's hands; the New Game+ starts with the pistols instead."""
    hits = [o for o in range(start, start + 0xC0, 4)
            if word(exe, o) == 0x24020006 and word(exe, o + 4) >> 16 == 0xA182]
    assert len(hits) == 1, f"weapon in hand found {len(hits)} times"
    return hits[0]


def first_level_shotgun(exe, start):
    """ModifyStartInfo's first level part clears the weapons Lara has not got at the start, but not the shotgun
    (flag 0x10000000 in the word at 0x14, ammunition at 6): it keeps what the start info held, in the released
    game nothing, but after a New Game+ in the same session its shotgun. Its masks 'lui a1, 0x7fff' (in a delay
    slot, ori 0xffff at the branch target) and 'lui a0, 0xbfff; ori a0, a0, 0xffff' become one mask 0x2fffffff
    in a1 (bits 31, 30 and the shotgun's 28), and the two a0 words 'sh zero, 6(t4); addiu a0, zero, -1'."""
    w = lambda o: word(exe, o)
    fn = next(o for o in range(start, start - 0x400, -4) if w(o) == 0x00806821)      # move t5, a0
    a1 = [o for o in range(fn, start, 4) if w(o) == 0x3C057FFF]
    a0 = [o for o in range(fn, start, 4) if w(o) == 0x3C04BFFF and w(o + 4) == 0x3484FFFF and w(o - 4) == 0x34A5FFFF]
    assert len(a1) == 1 and len(a0) == 1, "first level masks not found"
    return [(a1[0], [0x3C052FFF]), (a0[0], [0xA5800006, 0x2404FFFF])]


def health_bar(exe):
    """File offset of 'lh a1, 0x22(v0)' in the health bar routine."""
    sites = [o for o in range(0x800, len(exe) - 8, 4)
             if word(exe, o) == 0x84450022 and word(exe, o + 4) == 0 and word(exe, o + 8) >> 16 == 0x04A1
             and word(exe, o - 12) >> 16 == 0x8F82 and word(exe, o - 8) >> 16 == 0x27BD
             and word(exe, o - 4) >> 16 == 0xAFBF]
    assert len(sites) == 1, f"health bar routine found {len(sites)} times"
    return sites[0]


def level_timer(exe, bar):
    """Memory address of the level timer, increased behind the health bar timer (set to 40 in the health
    bar routine: addiu v0, zero, 40 ... sw v0, timer(gp))."""
    k = next(k for k in range(bar, bar + 0x40, 4) if word(exe, k) == 0x24020028)
    hbt = next(word(exe, j) & 0xFFFF for j in range(k + 4, k + 16, 4) if word(exe, j) >> 16 == 0xAF82
               and word(exe, j - 4) >> 16 == 0xAF85)
    hits = set()
    for o in range(0x800, len(exe) - 64, 4):
        a, b = word(exe, o), word(exe, o + 4)
        if b == 0xAF820000 | hbt and a == 0x2442FFFF:
            for k in range(o + 8, o + 48, 4):
                if word(exe, k) >> 16 == 0x3C03 and word(exe, k + 4) >> 16 == 0x2463:
                    lo = word(exe, k + 4) & 0xFFFF
                    hits.add(((word(exe, k) & 0xFFFF) << 16) + (lo - 0x10000 if lo & 0x8000 else lo))
                    break
    assert len(hits) == 1, f"level timer found {len(hits)} times"
    return hits.pop() & 0xFFFFFFFF


def demo_play(exe):
    """gp offset of DemoPlay from the enemy hit points code."""
    for o in range(0x800, len(exe) - 52, 4):
        if word(exe, o) >> 16 == 0x3C02 and word(exe, o + 4) >> 16 == 0x9042 and word(exe, o + 20) >> 16 == 0x8F82 \
                and word(exe, o + 28) >> 16 == 0x1440 and word(exe, o + 36) == 0x86020022:
            return word(exe, o + 20) & 0xFFFF
    raise AssertionError("DemoPlay not found")


def damage_routine(addr, timer, demo, data):
    """Called instead of 'lh a1, 0x22(v0)' (v0 = lara_item); returns the hit points in a1."""
    bhi, blo = ((addr + 0x8000) >> 16) & 0xFFFF, addr & 0xFFFF
    thi, tlo = ((timer + 0x8000) >> 16) & 0xFFFF, timer & 0xFFFF
    dhi, dlo = ((data + 0x8000) >> 16) & 0xFFFF, data & 0xFFFF
    assert bhi == thi and (dlo + 4) >> 15 == dlo >> 15
    return [0x84450022,                 # lh a1, 0x22(v0)
            0x3C080000 | bhi,           # lui t0, hi(bonus)
            0x91090000 | blo,           # lbu t1, lo(bonus)(t0)
            0x8D0A0000 | tlo,           # lw t2, lo(timer)(t0)
            0x3C0B0000 | dhi,           # lui t3, hi(data)
            0x11200011,                 # beqz t1, save
            0x856D0000 | dlo,           # lh t5, lo(old)(t3)
            0x8F8C0000 | demo,          # lw t4, DemoPlay(gp)
            0x8D6E0000 | dlo + 4,       # lw t6, lo(time)(t3)
            0x1580000D,                 # bnez t4, save
            0x014E7023,                 # subu t6, t2, t6
            0x2DCE001F,                 # sltiu t6, t6, 31
            0x11C0000A,                 # beqz t6, save
            0x00AD782A,                 # slt t7, a1, t5
            0x11E00008,                 # beqz t7, save
            0x00000000,
            0x18A00006,                 # blez a1, save
            0x01A57823,                 # subu t7, t5, a1   the loss
            0x00AF2823,                 # subu a1, a1, t7   once more
            0x04A10002,                 # bgez a1, store
            0x00000000,
            0x00002821,                 # move a1, zero
            0xA4450022,                 # store: sh a1, 0x22(v0)
            0xA5650000 | dlo,           # save: sh a1, lo(old)(t3)
            0x03E00008,                 # jr ra
            0xAD6A0000 | dlo + 4]       # sw t2, lo(time)(t3)


def build(exe, ver):
    addr = bonus_byte(exe)
    boot, newgame = clears(exe, addr)
    site, credits = credits_call(exe)
    hi, lo = ((addr + 0x8000) >> 16) & 0xFFFF, addr & 0xFFFF
    crys, bar, demo = crystals(exe, addr), health_bar(exe), demo_play(exe)
    timer = level_timer(exe, bar)
    checks, start = bonus_checks(exe, addr)
    code = [0x24080001,                     # addiu t0, zero, 1
            0x3C010000 | hi,                # lui at, hi(bonus)
            0x08000000 | (credits & 0x0FFFFFFF) >> 2,   # j credits (returns to the gameflow)
            0xA0280000 | lo]                # sb t0, lo(bonus)(at)
    size = 4 * (len(code) + 26 + 2 + 2 * len(START_AMMO) + 3 + 2)
    free = [f for f in rv.unused_routines(rv.words(exe), ver) if f[1] - f[0] >= size]
    assert free, "no unused function large enough"
    cave = min(free, key=lambda f: f[1] - f[0])[0]
    dmg = cave + 4 * len(code)
    code += damage_routine(addr, timer, demo, dmg + 4 * 26 + LOAD) + [0, 0]
    start_code, start_patches = start_equipment(exe, start, cave + 4 * len(code))
    code += start_code
    weapon = gun_type(exe, start)
    patches = [(cave, code), (site, [rv.jal(cave + LOAD)]), (newgame, [0]), (weapon, [0x24020000 | PISTOLS]),
               *first_level_shotgun(exe, start),
               (bar, [rv.jal(dmg + LOAD), 0])] + checks + start_patches + ([(crys, [0x24020001])] if crys else [])
    return patches, (f"bonus byte {addr:#x}, credits call {site + LOAD:#x} -> {cave + LOAD:#x} -> {credits:#x}, "
                     f"new game no longer clears it at {newgame + LOAD:#x} (start-up {boot + LOAD:#x} still does), "
                     f"crystals {f'{crys + LOAD:#x}' if crys else 'not set'}, {len(checks)} bonus checks off, "
                     f"start ammo {start + 8 + LOAD:#x}, health bar {bar + LOAD:#x} -> "
                     f"{dmg + LOAD:#x} (timer {timer:#x}, DemoPlay gp+{demo:#x})")


def patched(ver, path):
    exe = open(path, "rb").read()
    sh = open(os.path.join(BIN, "patch.sh"), encoding="utf-8").read()
    names = [l.split('"')[1].split(",") for l in sh.splitlines() if l.startswith("patches=")][0]
    for name in names[:names.index("newgameplus")]:              # the patches before this one
        exe = rv.apply_ips(exe, open(os.path.join(BIN, "patches", ver, f"{name}.ips"), "rb").read())
    return exe


if __name__ == "__main__":
    ap = argparse.ArgumentParser()
    ap.add_argument("--versions", default="us,ukrev,derev,frrev,itrev,sprev,jp,jp2,as,as2")
    ap.add_argument("--exe", nargs="*", default=[], help="VERSION=PATH for executables not in bin/orig")
    a = ap.parse_args()
    paths = {v: os.path.join(BIN, "orig", f"{v}.exe") for v in a.versions.split(",") if v}
    paths.update(dict(e.split("=", 1) for e in a.exe))
    for ver, path in paths.items():
        p, desc = build(patched(ver, path), ver)
        open(os.path.join(BIN, "patches", ver, "newgameplus.ips"), "wb").write(rv.ips(p))
        print(f"{ver}: {desc}")
