"""Builds bin/patches/<version>/crawlflip.ips: a forward crawl exit, and picking items up in the crawl.

Crawl exit (as Tomb Raider V and TRX do it): Jump in the crawl standstill, or in the crouch with Duck held
and the hands free, makes Lara flip out head first and fall, in the direction she faces (it follows the
game's Jump, so the Japanese and Asian button layouts too).

The animation is added to every level as animation 313 (tools/add_crawl_flip.py; Lara's home, the
released HOUSE.PSX, by tools/psx_lara_anims.py; frames 32000-32043,
state AS_NULL; at its end Lara is moved 620 forward and 815 down and falls). This patch starts it:
the crawl collision routine reads the input word at 'lw v1, input(gp); sw v0, 0x44(s0); lui v0,
0x2000' (s0 = Lara); that load becomes a call of a routine (the crouch's control routine of
make_crouchturn_ips.py calls it as well) that returns the input word and, when

  - Jump is held and Lara is in a crawl standstill animation (263 or 264, or 266 or 268, the ends of stopping
    after crawling forward, as TRX takes them; from 1.0.7), or in the crouch animation (222)
    with Duck held or kept down by a low ceiling (lara.keep_ducked, as TRX's keep_crouched; from 1.0.9) and the
    gun status 0,
  - the floor 768 ahead (LaraFloorFront) is at least 512 lower, and 256 ahead as well (she is at the edge:
    the animation takes her 620 forward, from further back it began in the crawlspace),
  - the ceiling 768 ahead (LaraCeilingFront, measured from her standing height) is at least as high as the
    crawlspace's (512 over her feet: at most 250),
  - the line from 96 over her feet to 768 ahead and 64 under her feet is free (the game's LOS, as the
    enemies use it: walls and the room's floor and ceiling),

starts animation 313 in state AS_NULL (from the crouch it sets the goal state to the crawl first and the
required state to AS_NULL as a wish: the crouch changes into the crawl (258, 273, 264), and in the crawl
standstill the wish starts the flip without Jump, all tests made again; anywhere else the wish is dropped;
from 1.0.7), frees her hands (the crawl set the gun status busy; left so, the crouch
refused the crawl afterwards) and returns only "crouch held", so the rest of the routine leaves her alone.
Up to 1.0.6 Walk + Jump in the crawl only, with the floor and ceiling 256 ahead, and Lara turned square to the
nearest axis and put 225 short of the edge.

Picking up in the crawl: Lara's animations hold it (the crawl standstills 263/264 change into 292 in state
PICKUP, 39, and 292 goes back to the crawl), but the pickup collision sends Lara into the crouch first when
Action is pressed in the crawl ('addiu v0, zero, 80; bne a0, v0; addiu v0, zero, 71; j out; sh v0, 0x10(s0)').
The 'j out' goes to a routine instead: in a crawl standstill, for any item but a flare (no crawl animation
for it), it goes to the pickup's start ('lui a0; addiu a0, position; jal MoveLaraPosition; move a2, s0; addiu
v0, zero, 39', a1 = the item) past the test of the hands (the crawl keeps them busy). The test of the frame
that takes the item ('lh v1, 0x2470(v1)': animation 291, the crouch's, frame 20 or 22) goes through a
routine that also takes frame 20 of animation 292; the goal state set after the start ('sh v0, 0x10(s0);
addiu v0, zero, 1': the crouch or standing) becomes the crawl (80) in animation 292 (from 1.0.7). The pickup's
control routine turns the camera to Lara's front ('addiu v0, zero, -0x5c6c; sh v0, angle(gp); addiu v0, zero,
-0xaaa; sh; addiu v0, zero, 0x400; sw; jr ra'); in animation 292 the camera stays behind her (from 1.0.7).

Weapons: LaraGun refuses the two-handed guns in the crouch only (state 71); the roll (72, unused in the
released game) and the flip (313, state AS_NULL, hands freed) let every gun be drawn. Its crouch test ('lw v0,
lara(gp); nop; lh v1, 0xe(v0); addiu v0, zero, 71') calls a routine behind this one instead of the load: in the
roll it returns a pointer to a halfword 71, so the crouch's rule applies, and in the flip it also makes the
requested gun a two-handed one, so nothing is drawn until Lara has landed (from 1.0.7). That routine goes into
an unused function; the US executables have none left, there it goes behind this routine (the block is larger
for them, make_pickup3d_ips.EXTRA_US).

Found in every version: the input load, LaraCeilingFront and LaraFloorFront as the two calls with
'addiu a2, zero, -300' behind it (crawl to hang), the standstill animations ('addiu v0, v0, -263').
LaraCeilingFront of the Japanese and Asian Disc 1 takes the height to test from as a fourth argument
(the others use Lara's height, 762): it gets 762.
The routine goes behind the 3D pickups' routines into the block pickup3d.ips appends to the executable,
which the start-up copies to the end of the level memory (there is no unused function left for it in
every version).

Usage: make_crawlflip_ips.py [--versions us,ukrev,...] [--exe VERSION=PATH ...]"""
import argparse
import os
import struct

import make_pickup3d_ips as p3
import make_reunionview_ips as rv

HERE = os.path.dirname(os.path.abspath(__file__))
BIN = os.path.join(HERE, "..", "bin")
LOAD = rv.LOAD

ANIM, FRAME_BASE, AS_NULL = 313, 32000, 19          # tools/add_crawl_flip.py
IN_JUMP = 0x10
IN_DUCK = 0x20000000
LARA_FLAGS, KEEP_DUCKED = 0x32, 0x20          # lara.flags behind the gun status (gp), keep_ducked: a low ceiling
NO_HEIGHT = -32512
CRAWL_PICKUP = 292                                  # Lara's pickup animation in the crawl
CROUCH_TO_CRAWL = (258, 273)                        # the crouch's way into the crawl (then 264, 263)


def word(d, o):
    return struct.unpack_from("<I", d, o)[0]


def sites(exe):
    """(file offset of the input load, input gp offset, LaraFloorFront, LaraCeilingFront, standstill anim)."""
    hits = []
    for o in range(0x800, len(exe) - 0x200, 4):
        if word(exe, o) >> 16 == 0x8F83 and word(exe, o + 4) == 0xAE020044 and word(exe, o + 8) == 0x3C022000:
            calls = [p3.jal_target(word(exe, k)) for k in range(o, o + 0x200, 4)
                     if word(exe, k) >> 26 == 3 and 0x2406FED4 in [word(exe, k + 4 * j) for j in range(-3, 2)]]
            anim = next(word(exe, k) & 0xFFFF for k in range(o, o + 0x80, 4) if word(exe, k) >> 16 == 0x2442
                        and word(exe, k) & 0x8000)
            hits.append((o, word(exe, o) & 0xFFFF, calls[1], calls[0], 0x10000 - anim))
    assert len(hits) == 1 and hits[0][4] == 263, f"crawl collision found {len(hits)} times"
    return hits[0]


def gun_status(exe):
    """gp offset of Lara's gun status: the crawl sets it busy at the start of its animation 258
    ('addiu v0, zero, 0x102; bne v1, v0; move a0, s0; addiu v0, zero, 1; sh v0, gun(gp)')."""
    hits = [word(exe, o + 16) & 0xFFFF for o in range(0x800, len(exe) - 20, 4)
            if word(exe, o) == 0x24020102 and word(exe, o + 4) >> 16 == 0x1462 and word(exe, o + 12) == 0x24020001
            and word(exe, o + 16) >> 16 == 0xA782]
    assert len(hits) == 1, f"gun status found {len(hits)} times"
    return hits[0]


def gun_site(exe):
    """File offset and Lara's gp offset of LaraGun's crouch test before drawing a weapon ('lw v0, lara(gp); nop;
    lh v1, 0xe(v0); addiu v0, zero, 71; bne v1, v0; addiu v0, zero, 9': in the crouch the requested gun in a0
    and a1 may not be a two-handed one, 4 to 8, else nothing is drawn)."""
    hits = [o for o in range(0x800, len(exe) - 24, 4) if word(exe, o) >> 16 == 0x8F82 and word(exe, o + 4) == 0
            and word(exe, o + 8) == 0x8443000E and word(exe, o + 12) == 0x24020047
            and word(exe, o + 16) >> 16 == 0x1462 and word(exe, o + 20) == 0x24020009]
    assert len(hits) == 1, f"LaraGun's crouch test found {len(hits)} times"
    return hits[0], word(exe, hits[0]) & 0xFFFF


def gun_guard(lara, base):
    """Called instead of 'lw v0, lara(gp); nop' of that test (at base): returns Lara, or, in the roll (72) as in
    the crouch (71), a pointer whose state reads 71, so the roll refuses the two-handed guns as the crouch does;
    in the crawl flip (animation 313) the requested gun becomes a two-handed one too, so nothing is drawn. LaraGun
    keeps nothing in t registers."""
    a = p3.Asm()
    a.lw("v0", lara, "gp"); a.nop()
    a.lh("t1", 0x0E, "v0"); a.lh("t3", 0x14, "v0")
    a.addiu("t4", "t1", -71); a.sltiu("t4", "t4", 2)              # the crouch or the roll
    a.addiu("t5", "zero", ANIM)
    a.bne("t3", "t5", "g_state"); a.nop()
    a.addiu("a0", "zero", 4); a.addiu("a1", "zero", 4)             # the crawl flip: as a shotgun in the crouch
    a.addiu("t4", "zero", 1)
    a.label("g_state")
    a.beqz("t4", "g_out"); a.nop()
    fake = base + 4 * 18 - 0x0E                                    # the halfword 71 behind the routine
    a.lui("v0", (fake + 0x8000) >> 16 & 0xFFFF); a.addiu("v0", "v0", fake & 0xFFFF)
    a.label("g_out")
    a.jr("ra"); a.nop()
    code = a.code()
    assert len(code) == 18, len(code)
    return code + [71]


def los_function(exe):
    """Address of LOS(start, target) (GAME_VECTOR: x, y, z, room): it calls xLOS and zLOS in one order or the
    other ('jal; move a1, s0; move s3, v0; move a0, s1; jal; move a1, s0'), its start is the 'addiu sp, sp, -40'
    before that."""
    hits = [o for o in range(0x800, len(exe) - 24, 4) if word(exe, o) >> 26 == 3 and word(exe, o + 4) == 0x02002821
            and word(exe, o + 8) == 0x00409821 and word(exe, o + 12) == 0x02202021 and word(exe, o + 16) >> 26 == 3
            and word(exe, o + 20) == 0x02002821]
    assert len(hits) == 2 and hits[1] - hits[0] < 0x40, f"LOS found {len(hits)} times"
    start = next(o for o in range(hits[0], hits[0] - 0x100, -4) if word(exe, o) == 0x27BDFFD8)
    return start + LOAD


def sin_table(exe, floor_front):
    """Address of the sine table (sine, then cosine, 4.12, index (angle >> 2) & 0x3ffc): LaraFloorFront reads
    it with 'lui at, hi; addu at, at, a1; lh v0, lo(at)'."""
    o = floor_front - LOAD
    k = next(k for k in range(o, o + 0x40, 4) if word(exe, k) >> 16 == 0x3C01 and word(exe, k + 4) == 0x00250821
             and word(exe, k + 8) >> 16 == 0x8422)
    lo = word(exe, k + 8) & 0xFFFF
    return (((word(exe, k) & 0xFFFF) << 16) + (lo - 0x10000 if lo & 0x8000 else lo)) | 0x80000000


def pickup_sites(exe):
    """The pickup collision: the crawl's 'j out' and out, the start of a pickup, the frame test of animation 291
    ('bne a0, v0, out'), the address it goes on with when equal, the animation table's gp offset, and the goal
    state set after the start."""
    w = lambda o: word(exe, o)
    n = len(exe) - 32
    p1 = [o for o in range(0x800, n, 4) if w(o) == 0x24020050 and w(o + 4) >> 16 == 0x1482 and w(o + 8) == 0x24020047
          and w(o + 12) >> 26 == 2 and w(o + 16) == 0xA6020010]
    p2 = [o for o in range(0x800, n, 4) if w(o) == 0x84632470 and w(o + 4) == 0 and w(o + 8) == 0x24620016
          and w(o + 12) >> 16 == 0x1082 and w(o + 16) == 0x24620014 and w(o + 20) >> 16 == 0x1482 and w(o + 24) == 0]
    p3s = [o for o in range(0x800, n, 4) if w(o) == 0x30420400 and w(o + 4) >> 16 == 0x1440
           and w(o + 8) == 0x24020047 and w(o + 12) == 0x24020002 and w(o + 16) == 0xA6020010
           and w(o + 20) == 0x24020001 and w(o + 24) >> 16 == 0xA782]
    st = [o for o in range(0x800, n, 4) if w(o) >> 16 == 0x3C04 and w(o + 4) >> 16 == 0x2484 and w(o + 8) >> 26 == 3
          and w(o + 12) == 0x02003021 and w(o + 16) == 0x24020027 and w(o + 20) == 0xA6020010]
    cam = [o for o in range(0x800, n, 4) if w(o) == 0x2402A394 and w(o + 4) >> 16 == 0xA782 and w(o + 8) == 0x2402F556
           and w(o + 12) >> 16 == 0xA782 and w(o + 16) == 0x24020400 and w(o + 20) >> 16 == 0xAF82
           and w(o + 24) == 0x03E00008]
    assert len(p1) == len(p2) == len(p3s) == len(st) == len(cam) == 1, "pickup collision not found"
    j_out = p1[0] + 12
    out = (w(j_out) & 0x3FFFFFF) << 2 | 0x80000000
    beq = p2[0] + 12
    cont = beq + 4 + ((((w(beq) & 0xFFFF) ^ 0x8000) - 0x8000) << 2) + LOAD
    table = next(w(k) & 0xFFFF for k in range(p2[0] - 4, p2[0] - 0x80, -4) if w(k) >> 16 == 0x8F83)
    return dict(j_out=j_out, out=out, start=st[0] + LOAD, frame_test=p2[0] + 20, cont=cont, table=table,
                goal=p3s[0] + 16, cam=cam[0], cam_gp=[w(cam[0] + k) & 0xFFFF for k in (4, 12, 20)])


def pickup_routines(base, ps):
    """At base: the crawl's start (j from the crawl's 'j out', whose delay slot set the crouch), the frame test
    (j from the 'bne a0, v0, out', v0 = the crouch pickup's frame) and the goal state (jal, replacing 'sh v0,
    0x10(s0); addiu v0, zero, 1'). Returns (words, labels)."""
    a = p3.Asm()
    a.label("crawl")
    a.lh("t0", 0x14, "s0"); a.lh("t1", 0x0C, "s1")
    a.addiu("t2", "t0", -263); a.sltiu("t2", "t2", 2)            # a crawl standstill
    a.beqz("t2", "c_out"); a.addiu("t3", "zero", 0xB3)
    a.beq("t1", "t3", "c_out"); a.nop()                           # not a flare
    a.j(ps["start"]); a.move("a1", "s1")
    a.label("c_out")
    a.j(ps["out"]); a.nop()
    a.label("frame")
    a.beq("a0", "v0", "f_on"); a.lw("t0", ps["table"], "gp")
    a.nop()
    a.lh("t0", 32 * CRAWL_PICKUP + 0x10, "t0"); a.nop()          # its first frame
    a.addiu("t0", "t0", 20)
    a.bne("a0", "t0", "f_out"); a.nop()
    a.label("f_on")
    a.j(ps["cont"]); a.nop()
    a.label("f_out")
    a.j(ps["out"]); a.nop()
    a.label("goal")
    a.lh("t0", 0x14, "s0"); a.addiu("t1", "zero", CRAWL_PICKUP)
    a.bne("t0", "t1", "g_keep"); a.nop()
    a.addiu("v0", "zero", 80)                                     # back into the crawl, not the crouch
    a.label("g_keep")
    a.sh("v0", 0x10, "s0")
    a.jr("ra"); a.addiu("v0", "zero", 1)
    angle, elevation, distance = ps["cam_gp"]
    a.label("cam")
    a.lh("t0", 0x14, "a0"); a.addiu("t1", "zero", CRAWL_PICKUP)
    a.beq("t0", "t1", "k_out"); a.addiu("v0", "zero", -0x5C6C)   # the crawl pickup: the camera stays behind
    a.sh("v0", angle, "gp"); a.addiu("v0", "zero", -0xAAA)
    a.sh("v0", elevation, "gp"); a.addiu("v0", "zero", 0x400)
    a.sw("v0", distance, "gp")
    a.label("k_out")
    a.jr("ra"); a.nop()
    wish_routine(a)
    return a.code(), {k: base + 4 * v for k, v in a.labels.items()}


def wish_routine(a):
    """The wish from the crouch (Lara's required state AS_NULL): t9 = 1 in the crawl standstill (the wish
    cleared), else 0; kept on the way into the crawl, dropped anywhere else."""
    a.label("wish")
    a.lh("t9", 0x12, "s0"); a.lh("t0", 0x14, "s0")
    a.addiu("t9", "t9", -AS_NULL)
    a.bnez("t9", "w_none"); a.addiu("t1", "t0", -263)
    a.sltiu("t1", "t1", 2)
    a.bnez("t1", "w_fire"); a.addiu("t2", "t0", -CROUCH_TO_CRAWL[0])
    a.beqz("t2", "w_none"); a.addiu("t2", "t0", -CROUCH_TO_CRAWL[1])
    a.beqz("t2", "w_none"); a.addiu("t2", "t0", -222)
    a.beqz("t2", "w_none"); a.nop()
    a.sh("zero", 0x12, "s0")
    a.label("w_none")
    a.jr("ra"); a.move("t9", "zero")
    a.label("w_fire")
    a.sh("zero", 0x12, "s0")
    a.jr("ra"); a.addiu("t9", "zero", 1)


def routine(inp, floor_front, ceiling_front, gun, los, sincos, wish=0):
    hi = lambda x: ((x + 0x8000) >> 16) & 0xFFFF
    a = p3.Asm()
    a.lw("v1", inp, "gp")
    a.addiu("sp", "sp", -56); a.sw("ra", 52, "sp"); a.sw("v1", 48, "sp")
    a.jal(wish); a.nop()
    a.bnez("t9", "ahead"); a.lw("v1", 48, "sp")                  # the wish from the crouch: no Jump needed
    a.andi("t0", "v1", IN_JUMP)
    a.beqz("t0", "out"); a.lh("t0", 0x14, "s0")                  # Jump
    a.nop()
    a.addiu("t1", "t0", -263); a.sltiu("t1", "t1", 2)
    a.bnez("t1", "ahead"); a.addiu("t1", "t0", -266)            # a crawl standstill,
    a.ori("t1", "t1", 2); a.addiu("t1", "t1", -2)
    a.beqz("t1", "ahead"); a.addiu("t1", "t0", -222)            # the end of stopping (266, 268)
    a.bnez("t1", "out"); a.lui("t2", IN_DUCK >> 16)              # or the crouch
    a.r(0x24, "t2", "v1", "t2")
    a.lw("t3", gun + LARA_FLAGS, "gp"); a.nop()
    a.andi("t3", "t3", KEEP_DUCKED)
    a.r(0x25, "t2", "t2", "t3")
    a.beqz("t2", "out"); a.lh("t3", gun, "gp")                   # with Duck held or kept down by the ceiling
    a.nop()
    a.bnez("t3", "out"); a.nop()                                  # and the hands free
    a.label("ahead")
    a.move("a0", "s0"); a.lh("a1", 0x4E, "s0")
    a.jal(floor_front); a.addiu("a2", "zero", 768)
    a.slti("t0", "v0", 512)
    a.bnez("t0", "out"); a.nop()                                  # floor 768 ahead at least 512 lower
    a.move("a0", "s0"); a.lh("a1", 0x4E, "s0")
    a.jal(floor_front); a.addiu("a2", "zero", 256)
    a.slti("t0", "v0", 512)
    a.bnez("t0", "out"); a.nop()                                  # and 256 ahead: she is at the edge
    a.move("a0", "s0"); a.lh("a1", 0x4E, "s0"); a.addiu("a3", "zero", 762)   # Japan: the height (else 762)
    a.jal(ceiling_front); a.addiu("a2", "zero", 768)
    a.addiu("t0", "zero", NO_HEIGHT)
    a.beq("v0", "t0", "out"); a.slti("t1", "v0", 251)
    a.beqz("t1", "out"); a.nop()                                  # the crawlspace's height ahead
    a.lw("t0", 0x40, "s0"); a.lw("t1", 0x44, "s0"); a.lw("t2", 0x48, "s0")   # the line: from 96 over her feet
    a.lh("t3", 0x18, "s0"); a.lhu("t4", 0x4E, "s0")
    a.addiu("t1", "t1", -96)
    a.sw("t0", 16, "sp"); a.sw("t1", 20, "sp"); a.sw("t2", 24, "sp")
    a.sh("t3", 28, "sp"); a.sh("t3", 44, "sp")
    a.r(0x02, "t4", "zero", "t4", 2); a.andi("t4", "t4", 0x3FFC)
    a.lui("t5", hi(sincos)); a.addu("t5", "t5", "t4")
    a.lh("t6", sincos & 0xFFFF, "t5"); a.lh("t7", (sincos + 2) & 0xFFFF, "t5")
    a.addiu("t1", "t1", 160)                                     # to 768 ahead, 64 under her feet
    a.sll("t8", "t6", 1); a.addu("t6", "t6", "t8"); a.r(0x03, "t6", "zero", "t6", 4)   # sine * 768 >> 12
    a.sll("t8", "t7", 1); a.addu("t7", "t7", "t8"); a.r(0x03, "t7", "zero", "t7", 4)
    a.addu("t0", "t0", "t6"); a.addu("t2", "t2", "t7")
    a.sw("t0", 32, "sp"); a.sw("t1", 36, "sp"); a.sw("t2", 40, "sp")
    a.addiu("a0", "sp", 16)
    a.jal(los); a.addiu("a1", "sp", 32)
    a.beqz("v0", "out"); a.nop()                                  # the line is free
    a.lh("t0", 0x14, "s0"); a.addiu("t1", "zero", 222)
    a.bne("t0", "t1", "flip"); a.addiu("t0", "zero", 80)
    a.sh("t0", 0x10, "s0")                                        # the crouch: into the crawl first,
    a.addiu("t0", "zero", AS_NULL)
    a.bra("out"); a.sh("t0", 0x12, "s0")                         # the flip wished for there
    a.label("flip")
    a.addiu("t0", "zero", ANIM); a.sh("t0", 0x14, "s0")
    a.addiu("t0", "zero", FRAME_BASE); a.sh("t0", 0x16, "s0")
    a.addiu("t0", "zero", AS_NULL); a.sh("t0", 0x0E, "s0"); a.sh("t0", 0x10, "s0")
    a.sh("zero", 0x1E, "s0"); a.sh("zero", 0x20, "s0")            # speed, fall speed
    a.sh("zero", gun, "gp")                                       # hands free again (the crawl had them busy)
    a.lui("t0", IN_DUCK >> 16); a.sw("t0", 48, "sp")              # the rest of the routine: crouch held
    a.label("out")
    a.lw("ra", 52, "sp"); a.lw("v1", 48, "sp")
    a.jr("ra"); a.addiu("sp", "sp", 56)
    return a.code()


def appended(orig, ver):
    """File offset and address of the first free bytes in the block pickup3d.ips appends to the executable
    (copied at start-up to the end of the level memory): behind its last word that is not 0, plus 32 for
    the zeros at the end of its data block."""
    for off, end in p3.ips_ranges(open(os.path.join(BIN, "patches", ver, "pickup3d.ips"), "rb").read()):
        if off == len(orig):
            break
    else:
        raise AssertionError("pickup3d.ips appends nothing")
    ips = rv.apply_ips(orig, open(os.path.join(BIN, "patches", ver, "pickup3d.ips"), "rb").read())
    used = (len(ips[off:end].rstrip(b"\x00")) + 32 + 15) & ~15
    _, size, buffer = p3.level_memory(orig)
    return off + used, buffer + size - p3.extra(ver) + used, min(end, len(orig) + p3.extra(ver))   # the padding is not copied


def parts(orig, ver, guard_at=0, wish=0):
    """The words of the routines: the flip, the gun guard (at guard_at) and the pickup routines (with the
    wish routine, at wish)."""
    _, inp, floor_front, ceiling_front, _ = sites(orig)
    flip = routine(inp, floor_front, ceiling_front, gun_status(orig), los_function(orig),
                   sin_table(orig, floor_front), wish)
    _, lara = gun_site(orig)
    return flip, gun_guard(lara, guard_at), pickup_routines(0, pickup_sites(orig))[0]


def places(orig, ver):
    """File offsets for the flip, the gun guard and the pickup routines, or None: the US executables have no
    unused functions left, there all go into the block (pickup3d's EXTRA_US). The flip takes the largest
    unused function; the others go behind it or behind each other where they fit, else into the largest
    function left, so the patches behind keep as many whole functions as possible."""
    if ver in ("us", "us10"):
        return None
    flip, guard, pick = parts(orig, ver)
    taken = p3.ips_ranges(open(os.path.join(BIN, "patches", ver, "pickup3d.ips"), "rb").read())
    out, ours = [], []
    for n in (4 * len(flip), 4 * len(guard), 4 * len(pick)):
        tails = [(e, b) for a, b in ours for e in [max(y for x, y in taken if a <= x < b)] if b - e >= n]
        free = [(a, b) for a, b in p3.free_space(orig, ver)
                if b - a >= n and not any(x < b and a < y for x, y in taken)]
        assert tails or free, "no unused function left for the crawl routines"
        if tails:
            a, _ = min(tails, key=lambda t: t[1] - t[0])
        else:
            a, b = free[-1]
            ours.append((a, b))
        out.append(a)
        taken.append((a, a + n))
    return out


def appended_after(orig, ver):
    """File offset and address of the first free bytes behind this patch's routines, and the block's end."""
    off, addr, end = appended(orig, ver)
    if places(orig, ver) is not None:
        return off, addr, end
    flip, guard, pick = parts(orig, ver)
    n = (4 * (len(flip) + len(guard) + len(pick)) + 15) & ~15
    return off + n, addr + n, end


def flip_address(orig, ver):
    """Where the flip routine is (the crouch's control routine calls it too)."""
    pl = places(orig, ver)
    return appended(orig, ver)[1] if pl is None else pl[0] + LOAD


def build(orig, ver):
    site, inp, floor_front, ceiling_front, _ = sites(orig)
    off, addr, end = appended(orig, ver)
    pl = places(orig, ver)
    flip_at = addr if pl is None else pl[0] + LOAD
    flip, _, _ = parts(orig, ver)
    guard_at = flip_at + 4 * len(flip) if pl is None else pl[1] + LOAD
    flip, guard, _ = parts(orig, ver, guard_at)
    pick_at = guard_at + 4 * len(guard) if pl is None else pl[2] + LOAD
    ps = pickup_sites(orig)
    pick, lab = pickup_routines(pick_at, ps)
    flip, guard, _ = parts(orig, ver, guard_at, lab["wish"])
    if pl is None:
        assert off + 4 * (len(flip) + len(guard) + len(pick)) <= end, "no room left in the appended block"
        writes = [(off, flip + guard + pick)]
    else:
        writes = [(pl[0], flip), (pl[1], guard), (pl[2], pick)]
    gsite, _ = gun_site(orig)
    jump = lambda x: 0x08000000 | (x & 0x0FFFFFFF) >> 2
    patches = writes + [(site, [rv.jal(flip_at)]), (gsite, [rv.jal(guard_at), 0]),
                        (ps["j_out"], [jump(lab["crawl"])]),
                        (ps["frame_test"], [jump(lab["frame"]), 0]),
                        (ps["goal"], [rv.jal(lab["goal"]), 0]),
                        (ps["cam"], [jump(lab["cam"]), 0])]
    where = "block" if pl is None else "functions " + ", ".join(f"{x + LOAD:#x}" for x in pl)
    return patches, (f"input load {site + LOAD:#x} (gp+{inp:#x}) -> {flip_at:#x} ({where}; flip {4 * len(flip)}, "
                     f"guard {4 * len(guard)}, pickup {4 * len(pick)} bytes), LOS {los_function(orig):#x}, "
                     f"sine {sin_table(orig, floor_front):#x}, pickup start {ps['start']:#x}")


if __name__ == "__main__":
    ap = argparse.ArgumentParser()
    ap.add_argument("--versions", default="us,ukrev,derev,frrev,itrev,sprev,jp,jp2,as,as2")
    ap.add_argument("--exe", nargs="*", default=[], help="VERSION=PATH for executables not in bin/orig")
    a = ap.parse_args()
    paths = {v: os.path.join(BIN, "orig", f"{v}.exe") for v in a.versions.split(",") if v}
    paths.update(dict(e.split("=", 1) for e in a.exe))
    for ver, path in paths.items():
        p, desc = build(open(path, "rb").read(), ver)
        open(os.path.join(BIN, "patches", ver, "crawlflip.ips"), "wb").write(rv.ips(p))
        print(f"{ver}: {desc}")
