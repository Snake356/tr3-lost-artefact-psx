"""Builds bin/patches/<version>/duckroll.ips: the forward roll out of the crouch.

Lara's animations still hold the roll (state 72: animations 218 roll, 219 rolling, 220 back into the
crouch), and the crouch animation (222) has its state change to it, but the executable never asks for
it: in the released game "forward" in the crouch starts the crawl, and the collision routine of state
72 is an empty function, so a roll would go through walls. This patch:

  - makes Sprint in the crouch (it does nothing there in the released game) set the goal state to 72,
    so the crouch animation changes into the roll by its own state change; while Sprint is held Lara
    keeps rolling, without it the goal state goes back to the crouch (71) during the roll, so it ends
    in the crouch; "forward" still starts the crawl. It is the game's Sprint function, so it follows
    the button layout (R2 on the western discs),
  - nothing else: the collision routine of state 72 is already the crouch's (state 71), so Lara stays on
    the floor, is pushed out of walls and does not roll over an edge. (Lara's collision table does not
    follow the control table directly: a table of 13 routines for special animations lies between.)

The crouch routine reads the input word at 'lw v0, input(gp); nop; andi v0, v0, 3' (forward/back ->
crawl); these two instructions become 'jal routine; nop', and the routine returns the input word, or 0
after setting the goal state when Sprint is pressed (so the crawl check is skipped). The
routine goes into a game function nothing refers to, found on the executable with all patches of
bin/patch.sh applied (make_reunionview_ips.unused_routines).

Lara's control routines are a table of 89 pointers, stored as 0x0001xxxx; it is recognised by the crouch
and the roll sharing one routine and by the empty function 89 + 72 words behind its start.

Usage: make_duckroll_ips.py [--versions us,ukrev,...] [--exe VERSION=PATH ...]"""
import argparse
import os
import struct

import make_crawlflip_ips as cf
import make_reunionview_ips as rv

HERE = os.path.dirname(os.path.abspath(__file__))
BIN = os.path.join(HERE, "..", "bin")
LOAD = rv.LOAD
STATES = 89
DUCK, ROLL = 71, 72
IN_DASH = 0x40000000                    # Sprint (R2 on the western PlayStation pad)


def word(d, o):
    return struct.unpack_from("<I", d, o)[0]


def state_tables(exe):
    """File offsets of the control and collision tables."""
    hits = []
    for o in range(0x800, len(exe) - 8 * STATES, 4):
        if not (0x10000 <= word(exe, o) < 0xA0000):
            continue
        t = struct.unpack_from(f"<{2 * STATES}I", exe, o)
        if all(0x10000 <= x < 0xA0000 for x in t) and t[DUCK] == t[ROLL] \
                and word(exe, t[STATES + ROLL] - 0x10000 + 0x800) == 0x03E00008:
            hits.append(o)
    assert len(hits) == 1, f"Lara's state tables found {len(hits)} times"
    return hits[0], hits[0] + 4 * STATES


def input_site(exe, duck_fn):
    """(file offset, input offset, register) of 'lw rX, input(gp); nop; andi v0, rX, 3; beqz v0' in the
    crouch routine (rX is v0, or a0 in the Japanese executables)."""
    o0 = duck_fn - 0x10000 + 0x800
    for o in range(o0, o0 + 0x100, 4):
        a, n, b, c = (word(exe, o + 4 * k) for k in range(4))
        r = (a >> 16) & 31
        if a >> 21 == (0x23 << 5 | 28) and n == 0 and b == (0x30020003 | r << 21) and c >> 16 == 0x1040:
            return o, a & 0xFFFF, r
    raise AssertionError("crawl input check not found in the crouch routine")


def routine(inp, r, gun):
    """Sprint pressed with the hands free (gun status 0): goal state roll, return 0 (no crawl this frame).
    Not so while rolling: goal state crouch, so the roll ends in the crouch. Otherwise the input word unchanged
    (the goal state is left alone: the game sets standing up before this routine). With a gun drawn the roll
    is refused, Lara has to put it away first; she may draw one while rolling, then the roll ends (from 1.0.7)."""
    def br(op, rs, at, to):                                       # branch at word index at to word index to
        return op << 26 | rs << 21 | (to - at - 1) & 0xFFFF
    T1, T3, AT, T0 = 9, 11, 1, 8
    return [0x8F800000 | r << 16 | inp,  # 0  lw rX, input(gp)
            0x860B000E,                  # 1  lh t3, 0xe(s0)       current_anim_state
            0x3C080000 | IN_DASH >> 16,  # 2  lui t0, IN_DASH >> 16
            0x00084024 | r << 21,        # 3  and t0, rX, t0
            0x87890000 | gun,            # 4  lh t1, gun(gp)       gun status
            0x24010000 | ROLL,           # 5  addiu at, zero, 72
            br(4, T0, 6, 13),            # 6  beqz t0, released
            0x240C0000 | DUCK,           # 7  addiu t4, zero, 71
            br(5, T1, 8, 13),            # 8  bnez t1, released    a gun drawn: no roll
            0x00000000,                  # 9
            0xA6010010,                  # 10 sh at, 0x10(s0)      goal_anim_state = roll
            0x03E00008,                  # 11 jr ra
            0x00000021 | r << 11,        # 12 move rX, zero
            br(5, T3, 13, 16) | AT << 16,  # 13 released: bne t3, at, done
            0x00000000,                  # 14
            0xA60C0010,                  # 15 sh t4, 0x10(s0)      goal_anim_state = crouch
            0x03E00008,                  # 16 done: jr ra
            0x00000000]                  # 17


def build(exe, ver):
    ctl, col = state_tables(exe)
    duck_fn = word(exe, ctl + 4 * DUCK)
    site, inp, r = input_site(exe, duck_fn)
    code = routine(inp, r, cf.gun_status(exe))
    free = [f for f in rv.unused_routines(rv.words(exe), ver) if f[1] - f[0] >= 4 * len(code)]
    assert free, "no unused function large enough"
    cave = min(free, key=lambda f: f[1] - f[0])[0]
    patches = [(cave, code), (site, [rv.jal(cave + LOAD), 0])]
    return patches, (f"tables {ctl + LOAD:#x}/{col + LOAD:#x}, crouch input check {site + LOAD:#x} "
                     f"(input gp+{inp:#x}, register {r}) -> {cave + LOAD:#x}")


def patched(ver, path):
    exe = open(path, "rb").read()
    sh = open(os.path.join(BIN, "patch.sh"), encoding="utf-8").read()
    names = [l.split('"')[1].split(",") for l in sh.splitlines() if l.startswith("patches=")][0]
    for name in names[:names.index("duckroll")]:                 # the patches before this one
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
        open(os.path.join(BIN, "patches", ver, "duckroll.ips"), "wb").write(rv.ips(p))
        print(f"{ver}: {desc}")
