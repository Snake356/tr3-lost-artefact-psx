"""Builds bin/patches/<version>/pickup3d.ips: picked up items are shown as turning 3D models in the
bottom right corner, as in Tomb Raider IV.

The released PlayStation game has no pickup display; an early demo of it has one (a list of 12 items
for 75 frames each, drawn by the health bar with the item's world model in front of the camera). This
patch builds a display like the one of Tomb Raider IV out of the game's own functions:

  - an item slides in from the right edge, turns in the corner for about three seconds and slides out
    again; items picked up in the same frame (several in one place) are shown side by side, a later
    one waits until the shown ones have slid out (they start to leave at once),
  - the two calls 'Inv_AddItem(item->object_number)' of the pickup collision (on land and under water;
    the single flare Lara takes into her hand never gets there) go through a routine that puts the
    item into the display or its queue (8 items) and then jumps to Inv_AddItem,
  - the call of the air bar in the game info drawing goes through a routine that draws the display
    first: the three lights of the PC version (written to the light directions the mesh drawing reads,
    colour matrix, back colour 128), then for every item a matrix pushed on
    the stack, made smaller by SCALE (and meshes with a bounding sphere larger than RREF smaller in
    proportion, so long weapons are not much larger than an ammunition box) and moved close to the
    camera (Z), so the model is drawn in front
    of the level, turned about Y and by the rotation of the object's first animation frame (as the
    game draws it: some items stand), moved back by the centre of the item's first mesh (its bounding
    sphere, so it turns about its middle), the mesh drawn and the matrix popped,
  - the mesh drawing takes the palette shift of the underwater textures from the GTE register DQB;
    it is 0 while the display is drawn, so the items keep their colours under water,
  - in NTSC (the GTE's screen centre OFY 120, not 128) the corner is further right and higher, as
    the TV shows more of the width of the picture,
  - the display runs by wibble, a counter of the control phase (+4 every frame; the level timer
    stands still in Lara's home); it is not drawn and stands still while wibble does (inventory,
    pause), a gap counts as one frame; it is cleared in another level or when the level timer goes
    back (savegame loaded).

Found in every version: the pickup calls ('addiu v0, zero, 0xb3' (flare) ... 'beq a0, v0; nop; jal
Inv_AddItem; nop'), the air bar call behind the health bar call in the game info drawing, the matrix
stack functions and the translation ('lw a1, 4(a0); lw a2, 8(a0); lw a0, 0(a0)', entered behind them;
push: 'lw a0, stack(gp); cfc2 t0, $0'; pop: 'lw a0, stack(gp); nop; addi a0, a0,
-0x20'), RotY (the middle one of the first three axis rotations 'srl a0, a0, 2; andi a0, a0, 0x3ffc'), the
mesh drawing (called as 'jal x; addiu a1, zero, 1; jal pop'), the object table and mesh list
('addu s7, v0, objects; lh v0, 2(s7); lw v1, meshes(gp)'), SetColorMatrix and SetBackColor of libgte
(by their code), the level timer (make_newgameplus_ips) and CurrentLevel (read beside the health bar
timer), all on the released executable. The routines go into game functions nothing refers to there,
without the bytes the other patches write (bin/patch.sh, duckroll, newgameplus, japanweapons, blood
colours): the rest of a function another patch put a short routine in is free too.

Usage: make_pickup3d_ips.py [--versions us,ukrev,...] [--exe VERSION=PATH ...]"""
import argparse
import os
import struct

import make_newgameplus_ips as ngp
import make_reunionview_ips as rv

HERE = os.path.dirname(os.path.abspath(__file__))
BIN = os.path.join(HERE, "..", "bin")
LOAD = rv.LOAD

FLARE_ITEM = 0xB3
SCALE = 0x400                   # 4.12 matrix scale: the model at Z looks as large as at 4 * Z
Z = 256                         # close to the camera: drawn in front of the level
X0, Y0 = 157, 67                # corner position at Z
NTSC_DX, NTSC_Y0 = 17, 62       # NTSC (240 lines): the TV shows more of the width, so further right, higher
RREF = 120                      # meshes with a larger bounding sphere are made smaller to this size
SPACING = 85                    # between items picked up together
IN_END, HOLD_END, END = 6, 90, 96   # frames: slide in, turn, slide out
SLIDE_SHIFT = 4                 # 16 units per frame of sliding
QUEUE = 8
SHOWN = 4

# lights of the PC version: warm from the upper right, white from the upper left, blue from below
# (directions, 4.12) and their colours (colour matrix rows red, green, blue; columns the three lights)
LIGHTS = [0x9E0, -0x9E0, 0x770, -0x9E0, -0x9E0, 0x770, 0, 0x8E0, 0xD50]
COLOUR_MATRIX = [0xCF0 | 0xCF0 << 16, 0x680 << 16, 0xCF0, 0xCF0 << 16, 0xC00, 0, 0, 0]

# data block
T, ACT_N, ACT_STAMP, Q_N, OLD_TIMER, YROT, LEVEL, ACT, Q, Q_STAMP, COLOURS = 0, 2, 4, 6, 8, 12, 14, 16, 24, 40, 56
YPOS = COLOURS + 24             # behind the five words SetColorMatrix reads
OLD_WIBBLE = COLOURS + 26
DATA_SIZE = 88

REG = {n: i for i, n in enumerate("zero at v0 v1 a0 a1 a2 a3 t0 t1 t2 t3 t4 t5 t6 t7 s0 s1 s2 s3 s4 s5 s6 s7 "
                                  "t8 t9 k0 k1 gp sp fp ra".split())}


def word(d, o):
    return struct.unpack_from("<I", d, o)[0]


def hi(a):
    return ((a + 0x8000) >> 16) & 0xFFFF


def lo(a):
    return a & 0xFFFF


class Asm:
    """Tiny MIPS assembler for the routines: branches by label, filled in by code()."""

    def __init__(self):
        self.items, self.labels = [], {}

    def label(self, name):
        assert name not in self.labels, f"label {name} twice"
        self.labels[name] = len(self.items)

    def i(self, op, rs, rt, imm):
        self.items.append((op << 26) | (REG[rs] << 21) | (REG[rt] << 16) | (imm & 0xFFFF))

    def r(self, funct, rd, rs, rt, sa=0):
        self.items.append((REG[rs] << 21) | (REG[rt] << 16) | (REG[rd] << 11) | (sa << 6) | funct)

    def addiu(self, rt, rs, imm): self.i(0x09, rs, rt, imm)
    def slti(self, rt, rs, imm): self.i(0x0A, rs, rt, imm)
    def sltiu(self, rt, rs, imm): self.i(0x0B, rs, rt, imm)
    def lui(self, rt, imm): self.i(0x0F, "zero", rt, imm)
    def lh(self, rt, off, rs): self.i(0x21, rs, rt, off)
    def lhu(self, rt, off, rs): self.i(0x25, rs, rt, off)
    def lw(self, rt, off, rs): self.i(0x23, rs, rt, off)
    def sh(self, rt, off, rs): self.i(0x29, rs, rt, off)
    def sw(self, rt, off, rs): self.i(0x2B, rs, rt, off)
    def addu(self, rd, rs, rt): self.r(0x21, rd, rs, rt)
    def subu(self, rd, rs, rt): self.r(0x23, rd, rs, rt)
    def slt(self, rd, rs, rt): self.r(0x2A, rd, rs, rt)
    def div(self, rs, rt): self.r(0x1A, "zero", rs, rt)
    def mflo(self, rd): self.r(0x12, rd, "zero", "zero")
    def ori(self, rt, rs, imm): self.i(0x0D, rs, rt, imm)
    def andi(self, rt, rs, imm): self.i(0x0C, rs, rt, imm)
    def sll(self, rd, rt, sa): self.r(0x00, rd, "zero", rt, sa)
    def move(self, rd, rs): self.addu(rd, rs, "zero")
    def nop(self): self.items.append(0)
    def jr(self, rs): self.r(0x08, "zero", rs, "zero")
    def j(self, a): self.items.append(0x08000000 | (a & 0x0FFFFFFF) >> 2)
    def jal(self, a): self.items.append(0x0C000000 | (a & 0x0FFFFFFF) >> 2)
    def ctc2(self, rt, rd): self.items.append(0x48C00000 | REG[rt] << 16 | rd << 11)
    def cfc2(self, rt, rd): self.items.append(0x48400000 | REG[rt] << 16 | rd << 11)

    def b(self, op, rs, rt, label):
        self.items.append(("b", op, REG[rs], REG[rt], label))

    def beq(self, rs, rt, label): self.b(0x04, rs, rt, label)
    def bne(self, rs, rt, label): self.b(0x05, rs, rt, label)
    def beqz(self, rs, label): self.b(0x04, rs, "zero", label)
    def bnez(self, rs, label): self.b(0x05, rs, "zero", label)
    def blez(self, rs, label): self.b(0x06, rs, "zero", label)
    def bltz(self, rs, label): self.b(0x01, rs, "zero", label)
    def bra(self, label): self.b(0x04, "zero", "zero", label)

    def code(self):
        out = []
        for k, x in enumerate(self.items):
            if isinstance(x, tuple):
                _, op, rs, rt, label = x
                off = self.labels[label] - (k + 1)
                assert -0x8000 <= off < 0x8000
                x = (op << 26) | (rs << 21) | (rt << 16) | (off & 0xFFFF)
            out.append(x)
        return out


def add_routine(data, timer, inv_add):
    """Called instead of Inv_AddItem(a0) at the pickups; puts a0 into the display, then Inv_AddItem."""
    a = Asm()
    a.lui("t9", hi(data)); a.addiu("t9", "t9", lo(data))
    a.lui("t0", hi(timer)); a.lw("t1", lo(timer), "t0")
    a.lh("t2", ACT_N, "t9"); a.lh("t3", Q_N, "t9")
    a.sll("t1", "t1", 16); a.r(0x02, "t1", "zero", "t1", 16)      # srl: stamp = timer & 0xffff
    a.bnez("t2", "busy"); a.nop()
    a.bnez("t3", "enqueue"); a.nop()
    a.sh("zero", T, "t9")                                        # nothing shown: show it now
    a.addiu("t2", "zero", 1); a.sh("t2", ACT_N, "t9"); a.sh("t1", ACT_STAMP, "t9")
    a.j(inv_add); a.sh("a0", ACT, "t9")
    a.label("busy")
    a.bnez("t3", "enqueue"); a.lhu("t4", ACT_STAMP, "t9")
    a.nop()                                                      # load delay
    a.bne("t4", "t1", "leave"); a.slti("t5", "t2", SHOWN)
    a.beqz("t5", "leave"); a.sll("t5", "t2", 1)
    a.addu("t5", "t5", "t9"); a.sh("a0", ACT, "t5")              # same frame: side by side
    a.addiu("t2", "t2", 1)
    a.j(inv_add); a.sh("t2", ACT_N, "t9")
    a.label("leave")                                             # the shown ones start to slide out
    a.lh("t5", T, "t9"); a.nop()
    a.slti("t6", "t5", HOLD_END)
    a.beqz("t6", "enqueue"); a.addiu("t5", "zero", HOLD_END)
    a.sh("t5", T, "t9")
    a.label("enqueue")
    a.slti("t5", "t3", QUEUE)
    a.beqz("t5", "done"); a.sll("t5", "t3", 1)
    a.addu("t5", "t5", "t9"); a.sh("a0", Q, "t5"); a.sh("t1", Q_STAMP, "t5")
    a.addiu("t3", "t3", 1); a.sh("t3", Q_N, "t9")
    a.label("done")
    a.j(inv_add); a.nop()
    return a.code()


def draw_routines(data, timer, air_bar, f, b_addr, c_addr):
    """Called instead of the air bar (a0 = flash state); draws the display, then jumps to the air bar.
    Three parts, as no unused function is large enough for all: the timing and the queue (A), which
    jumps to the light, turn and slide (C) at c_addr, which jumps to the drawing (B) at b_addr."""
    b = Asm()
    b.move("s1", "zero")
    b.label("item")
    b.lh("t0", ACT_N, "s0"); b.nop()
    b.slt("t1", "s1", "t0")
    b.beqz("t1", "out"); b.nop()
    b.sll("t0", "s1", 1); b.addu("t0", "t0", "s0"); b.lh("t0", ACT, "t0"); b.nop()
    b.sll("t1", "t0", 1); b.addu("t1", "t1", "t0"); b.sll("t1", "t1", 2)   # * 52
    b.addu("t1", "t1", "t0"); b.sll("t1", "t1", 2)
    b.lui("t2", hi(f["objects"])); b.addiu("t2", "t2", lo(f["objects"])); b.addu("t1", "t1", "t2")
    b.sw("t1", 0, "sp")                                          # the item's object
    b.lh("t1", 2, "t1"); b.lw("t2", f["meshes"], "gp")
    b.sll("t1", "t1", 2); b.addu("t1", "t1", "t2"); b.lw("a0", 0, "t1")
    b.nop()
    b.sw("a0", 12, "sp")                                         # the item's first mesh
    b.lh("t3", 6, "a0")                                          # its bounding sphere's radius
    b.addiu("t0", "zero", SCALE)
    b.slti("t4", "t3", RREF + 1)
    b.bnez("t4", "scaled"); b.nop()
    b.lui("t5", (SCALE * RREF) >> 16); b.ori("t5", "t5", (SCALE * RREF) & 0xFFFF)
    b.div("t5", "t3"); b.mflo("t0")                              # larger meshes smaller
    b.label("scaled")
    b.sw("t0", 8, "sp")
    b.jal(f["push"]); b.nop()
    b.lw("t0", 8, "sp"); b.lw("v0", f["stack"], "gp")
    b.lh("t1", YPOS, "s0"); b.addiu("t2", "zero", Z)
    b.ctc2("t0", 0); b.ctc2("zero", 1); b.ctc2("t0", 2); b.ctc2("zero", 3); b.ctc2("t0", 4)
    b.ctc2("s2", 5); b.ctc2("t1", 6); b.ctc2("t2", 7)
    b.sw("t0", 0, "v0"); b.sw("zero", 4, "v0"); b.sw("t0", 8, "v0"); b.sw("zero", 12, "v0")
    b.sw("t0", 16, "v0"); b.sw("s2", 20, "v0"); b.sw("t1", 24, "v0"); b.sw("t2", 28, "v0")
    b.lh("a0", YROT, "s0")                                       # (not in the delay slot: load delay)
    b.jal(f["rot_y"]); b.nop()
    b.lw("t0", 0, "sp"); b.nop()                                 # the rotation of the object's first frame,
    b.lw("t1", 8, "t0"); b.nop()                                 # which stands some items up
    b.addiu("t1", "t1", 0x12); b.sw("t1", 4, "sp")
    b.addiu("a0", "sp", 4)
    b.jal(f["rot_frame"]); b.move("a1", "zero")
    b.lw("a0", 12, "sp"); b.nop()                                # turn about the mesh's centre, not its origin
    b.lh("t0", 0, "a0"); b.lh("t1", 2, "a0"); b.lh("t2", 4, "a0"); b.nop()
    b.subu("a0", "zero", "t0"); b.subu("a1", "zero", "t1")
    b.jal(f["translate"]); b.subu("a2", "zero", "t2")
    b.cfc2("t0", 28); b.nop()                                    # DQB: the palette shift under water
    b.sw("t0", 36, "sp"); b.ctc2("zero", 28)
    b.lw("a0", 12, "sp")
    b.jal(f["put_polygons"]); b.addiu("a1", "zero", 1)
    b.lw("t0", 36, "sp"); b.nop()
    b.ctc2("t0", 28)
    b.jal(f["pop"]); b.nop()
    b.addiu("s2", "s2", -SPACING)
    b.bra("item"); b.addiu("s1", "s1", 1)
    b.label("out")
    b.lw("a0", 28, "sp"); b.lw("ra", 32, "sp"); b.lw("s0", 24, "sp"); b.lw("s1", 20, "sp"); b.lw("s2", 16, "sp")
    b.j(air_bar); b.addiu("sp", "sp", 40)
    b_out = b_addr + 4 * b.labels["out"]

    a = Asm()
    a.addiu("sp", "sp", -40)
    a.sw("ra", 32, "sp"); a.sw("a0", 28, "sp"); a.sw("s0", 24, "sp"); a.sw("s1", 20, "sp"); a.sw("s2", 16, "sp")
    a.lui("s0", hi(data)); a.addiu("s0", "s0", lo(data))
    a.lui("t0", hi(timer)); a.lw("t1", lo(timer), "t0")
    a.lw("t2", OLD_TIMER, "s0"); a.nop()
    a.sw("t1", OLD_TIMER, "s0")
    a.subu("t5", "t1", "t2")                                     # level timer: only to see it go back
    a.lw("t1", f["wibble"], "gp"); a.lhu("t2", OLD_WIBBLE, "s0"); a.nop()
    a.sh("t1", OLD_WIBBLE, "s0")
    a.subu("s1", "t1", "t2"); a.andi("s1", "s1", 0xFC)          # wibble: +4 every frame, in every level
    a.r(0x02, "s1", "zero", "s1", 2)                             # frames since the last call
    a.lw("t3", f["level"], "gp"); a.lh("t4", LEVEL, "s0"); a.nop()
    a.beq("t3", "t4", "same"); a.nop()
    a.sh("t3", LEVEL, "s0")                                      # another level: clear
    a.label("clear")
    a.sh("zero", ACT_N, "s0"); a.bra("out"); a.sh("zero", Q_N, "s0")
    a.label("same")
    a.bltz("t5", "clear"); a.nop()                               # timer went back: savegame loaded
    a.beqz("s1", "out"); a.sltiu("t3", "s1", 8)                  # standing still: inventory, pause
    a.bnez("t3", "running"); a.nop()
    a.addiu("s1", "zero", 1)                                     # a gap after a pause: one frame
    a.label("running")
    a.lh("t4", ACT_N, "s0"); a.nop()
    a.bnez("t4", "have"); a.nop()
    a.lh("t5", Q_N, "s0"); a.nop()                               # take the next items of the queue
    a.blez("t5", "out"); a.lhu("t6", Q_STAMP, "s0")
    a.move("t0", "zero")
    a.label("take")
    a.slt("t1", "t0", "t5")
    a.beqz("t1", "taken"); a.slti("t1", "t0", SHOWN)
    a.beqz("t1", "taken"); a.sll("t2", "t0", 1)
    a.addu("t2", "t2", "s0")
    a.lhu("t3", Q_STAMP, "t2"); a.lh("t4", Q, "t2"); a.nop()
    a.bne("t3", "t6", "taken"); a.nop()
    a.sh("t4", ACT, "t2")
    a.bra("take"); a.addiu("t0", "t0", 1)
    a.label("taken")
    a.sh("t0", ACT_N, "s0"); a.sh("t6", ACT_STAMP, "s0"); a.sh("zero", T, "s0")
    a.subu("t5", "t5", "t0"); a.sh("t5", Q_N, "s0")
    a.move("t1", "zero")
    a.label("shift")
    a.slt("t2", "t1", "t5")
    a.beqz("t2", "have"); a.addu("t3", "t1", "t0")
    a.sll("t3", "t3", 1); a.addu("t3", "t3", "s0")
    a.lh("t4", Q, "t3"); a.lhu("t7", Q_STAMP, "t3")
    a.sll("t2", "t1", 1); a.addu("t2", "t2", "s0")
    a.sh("t4", Q, "t2"); a.sh("t7", Q_STAMP, "t2")
    a.bra("shift"); a.addiu("t1", "t1", 1)
    a.label("have")
    a.lh("t6", T, "s0"); a.nop()
    a.addu("t6", "t6", "s1"); a.sh("t6", T, "s0")
    a.slti("t7", "t6", END)
    a.bnez("t7", "draw"); a.nop()
    a.sh("zero", ACT_N, "s0")                                    # slid out
    a.label("out")
    a.j(b_out); a.nop()
    a.label("draw")
    a.j(c_addr); a.nop()

    c = Asm()                                                    # light for the models
    c.lui("t0", hi(f["lights"])); c.addiu("t0", "t0", lo(f["lights"]))
    for k, v in enumerate(LIGHTS):
        c.addiu("t1", "zero", v); c.sh("t1", 2 * k, "t0")
    c.jal(f["SetColorMatrix"]); c.addiu("a0", "s0", COLOURS)
    c.addiu("a0", "zero", 128); c.addiu("a1", "zero", 128)
    c.jal(f["SetBackColor"]); c.addiu("a2", "zero", 128)
    c.lh("t6", T, "s0")
    c.lhu("t0", YROT, "s0"); c.sll("t1", "s1", 9); c.addu("t0", "t0", "t1"); c.sh("t0", YROT, "s0")
    c.slti("t7", "t6", IN_END)
    c.beqz("t7", "notin"); c.addiu("t0", "zero", IN_END)
    c.subu("t0", "t0", "t6")
    c.bra("offset"); c.sll("s2", "t0", SLIDE_SHIFT)
    c.label("notin")
    c.slti("t7", "t6", HOLD_END)
    c.bnez("t7", "offset"); c.move("s2", "zero")
    c.addiu("t0", "t6", -HOLD_END); c.sll("s2", "t0", SLIDE_SHIFT)
    c.label("offset")
    c.addiu("s2", "s2", X0)
    c.cfc2("t0", 25); c.nop()                                    # OFY: 120 in NTSC, 128 in PAL
    c.r(0x03, "t0", "zero", "t0", 16)                            # sra t0, t0, 16
    c.slti("t0", "t0", 128); c.addiu("t1", "zero", Y0)
    c.beqz("t0", "pal"); c.nop()
    c.addiu("s2", "s2", NTSC_DX); c.addiu("t1", "zero", NTSC_Y0)
    c.label("pal")
    c.j(b_addr); c.sh("t1", YPOS, "s0")
    return a.code(), b.code(), c.code()


def wibble(exe):
    """gp offset of wibble, +4 (& 0xfc) every frame of the control phase in every level (the level timer
    does not run in Lara's home): 'lw v0, w(gp); lw v1, x(gp); addiu v0, v0, 4; andi v0, v0, 0xfc'."""
    hits = {word(exe, o) & 0xFFFF for o in range(0x800, len(exe) - 24, 4)
            if word(exe, o) >> 16 == 0x8F82 and word(exe, o + 8) == 0x24420004 and word(exe, o + 12) == 0x304200FC
            and any(word(exe, o + 4 * k) == 0xAF820000 | word(exe, o) & 0xFFFF for k in range(4, 8))}
    assert len(hits) == 1, f"wibble found {len(hits)} times"
    return hits.pop()


def current_level(exe):
    """gp offset of CurrentLevel: 'lw v0, hbt(gp); lw v1, level(gp); addiu v0, v0, -1; sw v0, hbt(gp)',
    the decrement of the health bar timer in the control phase (the level timer runs unless level 0)."""
    hits = {word(exe, o + 4) & 0xFFFF for o in range(0x800, len(exe) - 16, 4)
            if word(exe, o) >> 16 == 0x8F82 and word(exe, o + 4) >> 16 == 0x8F83 and word(exe, o + 8) == 0x2442FFFF
            and word(exe, o + 12) == 0xAF820000 | word(exe, o) & 0xFFFF}
    assert len(hits) == 1, f"CurrentLevel found {len(hits)} times"
    return hits.pop()


def jal_target(x):
    return (x & 0x03FFFFFF) << 2 | 0x80000000


def pickup_calls(exe):
    """File offsets of the two 'jal Inv_AddItem' of the pickup collision, and Inv_AddItem."""
    sites = []
    for o in range(0x800, len(exe) - 16, 4):
        if word(exe, o) == 0x10820000 | (word(exe, o) & 0xFFFF) and word(exe, o + 4) == 0 \
                and word(exe, o + 8) >> 26 == 3 and word(exe, o + 12) == 0 \
                and any(word(exe, k) == 0x24020000 | FLARE_ITEM for k in range(o - 32, o, 4)):
            sites.append(o + 8)
    assert len(sites) == 2, f"pickup calls found {len(sites)} times"
    targets = {jal_target(word(exe, s)) for s in sites}
    assert len(targets) == 1
    return sites, targets.pop()


def air_bar_call(exe):
    """File offset of 'jal air_bar' behind 'jal health_bar; move a0, s0' and the air bar address."""
    bar = ngp.health_bar(exe) - 12 + LOAD
    jal = 0x0C000000 | (bar & 0x0FFFFFFF) >> 2
    sites = [o + 8 for o in range(0x800, len(exe) - 16, 4)
             if word(exe, o) == jal and word(exe, o + 4) == 0x02002021 and word(exe, o + 8) >> 26 == 3
             and word(exe, o + 12) == 0x02002021]
    assert len(sites) == 1, f"air bar call found {len(sites)} times"
    return sites[0], jal_target(word(exe, sites[0]))


def matrix_functions(exe):
    push = [o for o in range(0x800, len(exe) - 8, 4)
            if word(exe, o) >> 16 == 0x8F84 and word(exe, o + 4) == 0x4848_0000 and word(exe, o + 8) == 0x4849_0800]
    assert len(push) == 1, f"push matrix found {len(push)} times"
    stack = word(exe, push[0]) & 0xFFFF
    pop = [o for o in range(0x800, len(exe) - 12, 4)
           if word(exe, o) == 0x8F840000 | stack and word(exe, o + 4) == 0 and word(exe, o + 8) == 0x2084FFE0]
    assert len(pop) == 1, f"pop matrix found {len(pop)} times"
    rot = [o for o in range(0x800, len(exe) - 8, 4)
           if word(exe, o) == 0x00042082 and word(exe, o + 4) == 0x30843FFC and word(exe, o + 8) >> 16 == 0x1480]
    assert len(rot) in (3, 6), f"axis rotations found {len(rot)} times"
    jal_pop = 0x0C000000 | ((pop[0] + LOAD) & 0x0FFFFFFF) >> 2
    calls = {jal_target(word(exe, o)) for o in range(0x800, len(exe) - 12, 4)
             if word(exe, o) >> 26 == 3 and word(exe, o + 4) == 0x24050001 and word(exe, o + 8) == jal_pop}
    assert len(calls) == 1, f"mesh drawing found {len(calls)} times"
    put = calls.pop()
    frame = {jal_target(word(exe, o + 4)) for o in range(0x800, len(exe) - 12, 4)
             if word(exe, o) == 0x27A40010 and word(exe, o + 4) >> 26 == 3 and word(exe, o + 8) == 0x00002821}
    assert len(frame) == 1, f"frame rotation found {len(frame)} times"
    tr = [o for o in range(0x800, len(exe) - 12, 4)
          if word(exe, o) == 0x8C850004 and word(exe, o + 4) == 0x8C860008 and word(exe, o + 8) == 0x8C840000]
    assert len(tr) == 1, f"translation found {len(tr)} times"
    o = put - LOAD + 0x8C                                        # lui t9, hi; ori t9, t9, lo: the light directions
    assert word(exe, o) >> 16 == 0x3C19 and word(exe, o + 4) >> 16 == 0x3739, "light directions not found"
    lights = (word(exe, o) & 0xFFFF) << 16 | word(exe, o + 4) & 0xFFFF
    return {"push": push[0] + LOAD, "pop": pop[0] + LOAD, "stack": stack, "rot_y": sorted(rot)[1] + LOAD,
            "put_polygons": put, "lights": lights, "translate": tr[0] + 12 + LOAD,
            "rot_frame": frame.pop()}


def object_tables(exe):
    hits = set()
    for o in range(0x800, len(exe) - 12, 4):
        if word(exe, o) == 0x86E2_0002 and word(exe, o + 4) >> 16 == 0x8F83 and word(exe, o - 4) == 0x0043B821 \
                and word(exe, o - 12) >> 16 == 0x3C03 and word(exe, o - 8) >> 16 == 0x2463:
            l = word(exe, o - 8) & 0xFFFF
            hits.add((((word(exe, o - 12) & 0xFFFF) << 16) + (l - 0x10000 if l & 0x8000 else l),
                      word(exe, o + 4) & 0xFFFF))
    assert len(hits) == 1, f"object table found {len(hits)} times"
    objects, meshes = hits.pop()
    return {"objects": objects | 0x80000000, "meshes": meshes}


def libgte(exe):
    """SetColorMatrix and SetBackColor by their code (taken from the demo, which calls them)."""
    found = {}
    for name, code in (("SetColorMatrix", SET_COLOR_MATRIX), ("SetBackColor", SET_BACK_COLOR)):
        n = len(code)
        hits = [o for o in range(0x800, len(exe) - 4 * n, 4)
                if all(word(exe, o + 4 * k) == code[k] for k in range(n))]
        assert len(hits) == 1, f"{name} found {len(hits)} times"
        found[name] = hits[0] + LOAD
    return found


SET_COLOR_MATRIX = [0x8C880000, 0x8C890004, 0x8C8A0008, 0x8C8B000C, 0x8C8C0010, 0x48C88000, 0x48C98800,
                    0x48CA9000, 0x48CB9800, 0x48CCA000, 0x03E00008, 0]
SET_BACK_COLOR = [0x00042100, 0x00052900, 0x00063100, 0x48C46800, 0x48C57000, 0x48C67800, 0x03E00008, 0]


EXTRA = 0x1A40                  # bytes taken from the end of the level memory for the routines (with the
                                # ledge jumps, make_ledgejump_ips.py, and the corner shimmying with the climbing
                                # round corners, make_cornershimmy_ips.py)
EXTRA_US = 0x1D40               # the US executables: no unused function left for the crawl routines
                                # (make_crawlflip_ips.py), so the block takes them too
BOOT_ROOM = 104                 # the unused function of the two copy routines before 30.10.2026, so the
                                # patches after this one keep their places


def extra(ver):
    return EXTRA_US if ver in ("us", "us10") else EXTRA


def level_memory(exe):
    """File offset of 'ori v0, v0, size_lo' in init_game_malloc ('lui v0, size_hi; lw v1, x(gp); ori v0,
    v0, size_lo; subu v0, v0, v1; sw v0, malloc_free(gp); lui v0, hi; addiu v0, v0, lo' = malloc_buffer),
    the size and the buffer's address."""
    hits = []
    for o in range(0x800, len(exe) - 28, 4):
        w = [word(exe, o + 4 * k) for k in range(7)]
        if (w[0] >> 16 == 0x3C02 and w[1] >> 16 == 0x8F83 and w[2] >> 16 == 0x3442 and w[3] == 0x00431023
                and w[4] >> 16 == 0xAF82 and w[5] >> 16 == 0x3C02 and w[6] >> 16 == 0x2442):
            l = w[6] & 0xFFFF
            hits.append((o + 8, (w[0] & 0xFFFF) << 16 | w[2] & 0xFFFF,
                         ((w[5] & 0xFFFF) << 16) + (l - 0x10000 if l & 0x8000 else l) | 0x80000000))
    assert len(hits) == 1, f"init_game_malloc found {len(hits)} times"
    return hits[0]


def start_up(exe):
    """The start code (pc0 of the header) clears the BSS from 'lui v0; addiu v0' to 'lui v1; addiu v1' ('sw zero,
    0(v0); addiu v0, v0, 4; sltu at, v0, v1; bnez at; nop'): (pc0, bss start, bss end), both 0x80... ."""
    pc0 = word(exe, 0x10) | 0x80000000
    o = pc0 - LOAD
    w = [word(exe, o + 4 * k) for k in range(9)]
    assert (w[0] >> 16 == 0x3C02 and w[1] >> 16 == 0x2442 and w[2] >> 16 == 0x3C03 and w[3] >> 16 == 0x2463
            and w[4:9] == [0xAC400000, 0x24420004, 0x0043082B, 0x1420FFFC, 0]), "BSS clearing not found"
    start, end = (((w[k] & 0xFFFF) << 16) + (w[k + 1] & 0xFFFF) - (w[k + 1] & 0x8000) * 2 for k in (0, 2))
    return pc0, start | 0x80000000, end | 0x80000000


def boot_routine(src, dst, n, clear, clear_end, then):
    """Copies n bytes from src to dst word by word, clears clear to clear_end, then jumps to then (t0-t3 only)."""
    a = Asm()
    a.lui("t0", hi(src)); a.addiu("t0", "t0", lo(src))
    a.lui("t1", hi(dst)); a.addiu("t1", "t1", lo(dst))
    a.lui("t2", hi(src + n)); a.addiu("t2", "t2", lo(src + n))
    a.label("copy")
    a.lw("t3", 0, "t0"); a.addiu("t0", "t0", 4)
    a.sw("t3", 0, "t1")
    a.bne("t0", "t2", "copy"); a.addiu("t1", "t1", 4)
    a.lui("t0", hi(clear)); a.addiu("t0", "t0", lo(clear))
    a.lui("t2", hi(clear_end)); a.addiu("t2", "t2", lo(clear_end))
    a.label("clear")
    a.addiu("t0", "t0", 4)
    a.bne("t0", "t2", "clear"); a.sw("zero", -4, "t0")
    a.j(then); a.nop()
    return a.code()


def build(exe, ver, orig):
    """exe: with all other patches (to check the places hooked), orig: the released one (for the search).

    The routines and their data do not fit into the unused functions of every version, so they go into
    RAM taken from the level memory: init_game_malloc gets EXTRA bytes less, and the routines live in the
    last EXTRA bytes of its buffer. They are appended to the executable (the header's text size grows),
    so the loader puts them behind the code, into the BSS, which the start code clears: a routine at the
    new start address first copies them straight to their place (the level buffer lies in the BSS, but not
    at its end), clears the BSS behind the level buffer and jumps to the start code, whose clearing now
    ends where the routines begin. Only this boot routine goes into an unused function."""
    sites, inv_add = pickup_calls(orig)
    air_site, air_bar = air_bar_call(orig)
    f = {**matrix_functions(orig), **object_tables(orig), **libgte(orig), "level": current_level(orig),
         "wibble": wibble(orig)}
    timer = ngp.level_timer(orig, ngp.health_bar(orig))
    size_site, size, buffer = level_memory(orig)
    pc0, bss_start, bss_end = start_up(orig)
    end_site = pc0 - LOAD + 8                                    # lui v1, hi(bss end); addiu v1, v1, lo(bss end)
    for s in sites + [air_site, size_site, end_site, end_site + 4]:
        assert word(exe, s) == word(orig, s), f"{s + LOAD:#x} is changed by another patch"
    EXTRA = extra(ver)
    assert (size - EXTRA) >> 16 == size >> 16

    region = buffer + size - EXTRA
    dummy = draw_routines(0, timer, air_bar, f, 0, 0)
    n_add = len(add_routine(0, timer, inv_add))
    a_at = region + 4 * n_add
    c_at = a_at + 4 * len(dummy[0])
    b_at = c_at + 4 * len(dummy[2])
    data = b_at + 4 * len(dummy[1])
    add = add_routine(data, timer, inv_add)
    part_a, part_b, part_c = draw_routines(data, timer, air_bar, f, b_at, c_at)
    blob = add + part_a + part_c + part_b + [0] * (COLOURS // 4) + COLOUR_MATRIX
    blob += [0] * (EXTRA // 4 - len(blob))                       # the whole block: room for the later patches
    n = 4 * len(blob)
    pad = [0] * (-n % 2048 // 4)                                 # the executable in whole CD sectors
    assert data + DATA_SIZE <= region + EXTRA and n <= EXTRA

    t_size = word(orig, 0x1C)
    assert len(orig) == 0x800 + t_size
    loaded = 0x80010000 + t_size                                 # where the loader puts the appended block
    assert bss_start < loaded and loaded + n <= region, "the appended block must lie in the BSS cleared before the routines"
    assert buffer <= region and region + n == buffer + size <= bss_end, "the routines must end the level buffer"
    assert region % 4 == 0 and (bss_end - region - n) % 4 == 0 and bss_end > region + n
    target = region & 0x7FFFFFFF if word(orig, end_site) & 0x8000 == 0 else region   # the start code's segment
    stub = boot_routine(loaded, region, n, region + n, bss_end, pc0)
    free = [r for r in free_space(orig, ver) if r[1] - r[0] >= max(4 * len(stub), BOOT_ROOM)]
    assert free, "no unused function for the boot routine"
    cave = free[0][0]
    assert cave + 4 * len(stub) <= free[0][1]
    patches = [(0x10, [cave + LOAD]), (0x1C, [t_size + n + 4 * len(pad)]),   # start address, text size
               (len(orig), blob + pad), (cave, stub),
               (end_site, [0x3C030000 | hi(target), 0x24630000 | lo(target)]),
               (size_site, [0x34420000 | (size - EXTRA) & 0xFFFF]),
               (air_site, [rv.jal(a_at)])] + [(s, [rv.jal(region)]) for s in sites]
    desc = (f"level memory {size:#x} -> {size - EXTRA:#x} ({size_site + LOAD:#x}), routines at {region:#x} "
            f"({n} bytes appended, copied {loaded:#x} -> {region:#x} at {cave + LOAD:#x}, BSS cleared "
            f"{bss_start:#x}-{region:#x} and {region + n:#x}-{bss_end:#x}), "
            f"pickups {', '.join(hex(s + LOAD) for s in sites)} -> {inv_add:#x}, "
            f"air bar {air_site + LOAD:#x} -> {air_bar:#x}, data {data:#x}, "
            + ", ".join(f"{k} {v:#x}" for k, v in f.items()))
    return patches, desc


PATCHES = [l.split('"')[1].split(",") for l in open(os.path.join(BIN, "patch.sh"), encoding="utf-8").read().splitlines()
           if l.startswith("patches=")][0]
PATCHES = PATCHES[:PATCHES.index("pickup3d")] + ["japanweapons", "bloodred", "bloodpurple"]   # all but this one


def ips_ranges(data):
    """[(start, end)] file offsets an IPS patch writes."""
    assert data[:5] == b"PATCH"
    o, out = 5, []
    while data[o:o + 3] != b"EOF":
        off = int.from_bytes(data[o:o + 3], "big")
        size = int.from_bytes(data[o + 3:o + 5], "big")
        o += 5
        if size == 0:
            size = int.from_bytes(data[o:o + 2], "big")
            o += 3
        else:
            o += size
        out.append((off, off + size))
    return out


def patched(ver, path):
    """The executable with the patches of bin/patch.sh, duckroll and newgameplus (to check that the
    places hooked are not changed by them)."""
    exe = open(path, "rb").read()
    for name in PATCHES[:PATCHES.index("japanweapons")]:
        exe = rv.apply_ips(exe, open(os.path.join(BIN, "patches", ver, f"{name}.ips"), "rb").read())
    return exe


def free_space(orig, ver):
    """Unused functions of the released executable without the bytes the other patches write (and 16
    bytes behind each), smallest first."""
    taken = []
    for name in PATCHES:
        path = os.path.join(BIN, "patches", ver, f"{name}.ips")
        if os.path.exists(path):
            taken += ips_ranges(open(path, "rb").read())
    out = []
    for s, e in rv.unused_routines(rv.words(orig), ver):
        pieces = [(s, e)]
        for ts, te in taken:
            ts, te = ts & ~3, (te + 16 + 3) & ~3
            pieces = [p for a, b in pieces for p in ((a, min(b, ts)), (max(a, te), b)) if p[1] - p[0] >= 16]
        out += pieces
    return sorted(out, key=lambda r: r[1] - r[0])


if __name__ == "__main__":
    ap = argparse.ArgumentParser()
    ap.add_argument("--versions", default="us,ukrev,derev,frrev,itrev,sprev,jp,jp2,as,as2")
    ap.add_argument("--exe", nargs="*", default=[], help="VERSION=PATH for executables not in bin/orig")
    a = ap.parse_args()
    paths = {v: os.path.join(BIN, "orig", f"{v}.exe") for v in a.versions.split(",") if v}
    paths.update(dict(e.split("=", 1) for e in a.exe))
    for ver, path in paths.items():
        p, desc = build(patched(ver, path), ver, open(path, "rb").read())
        open(os.path.join(BIN, "patches", ver, "pickup3d.ips"), "wb").write(rv.ips(p))
        print(f"{ver}: {desc}")
