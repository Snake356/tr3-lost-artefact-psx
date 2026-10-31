"""Builds the fixes of 1.0.8 as IPS patches (bin/patches/<version>/<name>.ips), found in every executable:

  debugspace   the level loader's debug texts ('Loading ...', 968 bytes, the same in every executable): their
               printf calls become nops, the space takes the code of the patches below
  collision    in the crawl and the crouch (collision height 400) the front probe starts from Lara's own room,
               not from the room the middle probe ended in (a crawlspace under an opening read as a wall); a
               steep slope ahead is a wall only above Lara's feet (TR4/TR5), not at the bottom of a pit
  mistsort     semi-transparent faces of mist rooms are sorted nearer in the ordering table (the PS1 has no
               z-buffer, the mist was drawn under the floor it lies on); the strength comes from the room flags
               (+0x4E: 0x4000 mist room, 0x1800 strength, set by nebel_raeume.py): 128 >> strength table entries
  cdpause      the end of a speech pauses the CD without waiting for the drive (the picture stood still), and
               the first speech of a level no longer waits a frame
  sprintstep   sprinting up a step keeps the sprint (TRX's responsive sprint)
  killedwater  a killed orca, diver or alligator is not filed into a room again (a diver killed by a rocket
               stayed visible)
  bodies       dead divers and alligators disappear out of sight (their death animation never ends)
  endmusic     the final statistics play track 14 (as TRX); GameStats stops the CD before going back to the title
  pacing       the frame counter is no longer cleared at the end of a frame (even game speed)
  rollcheck    the crouch roll (duckroll.ips) checks the floor ahead as TRX's M_CanCrouchRoll
  harpoon      under water Draw takes the harpoon gun when Lara has it, out of the water the gun used before
               (TRX's enable_underwater_auto_draw)
  voicesteal   a voice taken from the quietest sound when all 24 are busy is not keyed off before the new sound
               keys it on (the key on right after the key off did not take: the old loop, e.g. the helicopter
               of Highland Fling, went on unfiled until the inventory); it only goes back to the free list
  controlleddrop  TRX's controlled drop: Lara stepping off an edge at least 954 high with Action held and her
               hands free, standing level and flat (her old position's collision) and lined up with an axis
               turns round and catches it (animations 316 and 317, tools/add_controlled_drop.py)

The places are found from the US executable (bin/orig/us.exe with the patches of bin/patch.sh before these):
windows of its words compared with jump targets, lui and gp/at offsets masked, or the patterns named below.
Usage: make_fixes_ips.py [--versions us,ukrev,...] [--exe VERSION=PATH ...]"""
import argparse
import os
import struct
from collections import Counter

import keystone

import make_crawlflip_ips as cf
import make_duckroll_ips as dr
import make_reunionview_ips as rv

HERE = os.path.dirname(os.path.abspath(__file__))
BIN = os.path.join(HERE, "..", "bin")
LOAD = rv.LOAD
NAMES = ("debugspace", "collision", "mistsort", "cdpause", "sprintstep", "killedwater", "bodies", "endmusic",
         "pacing", "rollcheck", "harpoon", "voicesteal", "controlleddrop")
DEBUG_TEXT = b"        Loading '%s'...\n\x00\x00\x00\x00Allocating m"
DEBUG_SIZE = 968
PRINTF_CALLS = 33
HARPOON_ITEM = 164
TRACK = 14
DROP_ANIM, FALL_ANIM = 316, 0x22
AS_REACH, AS_FORWARDJUMP = 11, 3
IN_ACTION = 0x40
DROP_MIN = 954                                        # LARA_HEIGHT + 3/4 step
HANG_ANGLE = 6370                                     # 35 degrees
LARA_HITE = 762


def word(d, o):
    return struct.unpack_from("<I", d, o)[0]


def j(a):
    return 0x08000000 | (a & 0x0FFFFFFF) >> 2


def jal(a):
    return 0x0C000000 | (a & 0x0FFFFFFF) >> 2


def target(w):
    return 0x80000000 | (w & 0x3FFFFFF) << 2


def asm(src, base):
    ks = keystone.Ks(keystone.KS_ARCH_MIPS, keystone.KS_MODE_MIPS32 + keystone.KS_MODE_LITTLE_ENDIAN)
    return bytes(ks.asm(".set noreorder\n" + src, base)[0])


def hi_lo(a):
    lo = a & 0xFFFF
    return (a + 0x8000) >> 16 & 0xFFFF, lo - 0x10000 if lo & 0x8000 else lo


def mask(w):
    op = w >> 26
    if op in (2, 3):                                  # j / jal
        return op << 26
    if op == 0x0F:                                    # lui
        return w & 0xFFFF0000
    if op >= 8 and ((w >> 21) & 31) in (1, 28):       # I-type on at or gp
        return w & 0xFFFF0000
    return w


class Exe:
    def __init__(self, data):
        self.d = bytearray(data)
        self.W = rv.words(data)
        self.M = [mask(w) for w in self.W]

    def w(self, a):
        return word(self.d, a - LOAD)

    def window(self, us, a, n=8, before=0):
        """Address of the US window [a - 4*before, a + 4*n) in this executable (found once)."""
        i = (a - LOAD - 0x800) // 4 - before
        pat = us.M[i:i + before + n]
        hits = [k for k in range(len(self.M) - len(pat)) if self.M[k:k + len(pat)] == pat]
        assert len(hits) == 1, f"{a:#x}: {len(hits)} places"
        return LOAD + 0x800 + 4 * (hits[0] + before)

    def pattern(self, pat, what):
        """Address of the only place where the words match pat (word, (word, mask) or None)."""
        return rv.find_one(self.W, pat, what) + LOAD


class Patch:
    def __init__(self):
        self.parts = []                               # (address, bytes)

    def put(self, a, data):
        if isinstance(data, list):
            data = struct.pack(f"<{len(data)}I", *data)
        self.parts.append((a, data))

    def ips(self):
        return rv.ips([(a - LOAD, list(struct.unpack(f"<{len(b) // 4}I", b))) for a, b in self.parts])

    def apply(self, exe):
        for a, b in self.parts:
            exe.d[a - LOAD:a - LOAD + len(b)] = b


# ---- the patches; each takes (us, exe, orig, at) and returns (Patch, bytes used in the debug text space)

def debugspace(us, exe, orig, at):
    lo = exe.d.find(DEBUG_TEXT)
    assert lo > 0 and exe.d.find(DEBUG_TEXT, lo + 1) < 0
    lo += LOAD
    calls = []
    for k, w in enumerate(exe.W):
        if w >> 26 != 3:
            continue
        hi16 = val = None
        for x in exe.W[k - 10:k + 2]:
            if x >> 26 == 0x0F and (x >> 16) & 31 == 4:
                hi16 = (x & 0xFFFF) << 16
            elif x >> 26 == 9 and (x >> 16) & 31 == 4 and (x >> 21) & 31 == 4 and hi16 is not None:
                imm = x & 0xFFFF
                val = (hi16 + imm - (0x10000 if imm & 0x8000 else 0)) | 0x80000000
        if val is not None and lo <= val < lo + DEBUG_SIZE:
            calls.append((LOAD + 0x800 + 4 * k, w))
    printf, n = Counter(w for _, w in calls).most_common(1)[0]
    assert n == PRINTF_CALLS, f"printf calls: {n}"
    p = Patch()
    for a, w in calls:
        if w == printf:
            p.put(a, [0])
    return p, lo


def collision(us, exe, orig, at):
    front = exe.window(us, 0x80026744, 8, 8)
    assert exe.w(front) == 0x27A70018                 # addiu a3, sp, 0x18
    slope = exe.window(us, 0x80026860, 8, 8)
    assert [exe.w(slope), exe.w(slope + 4)] == [0x14400024, 0x24028001]    # bnez v0, +0x24; addiu v0, zero, -0x7FFF
    p = Patch()
    tramp = [0x8FAB00CC,                              # lw t3, 0xCC(sp)     collision height
             0x240C0190,                              # addiu t4, zero, 400 (crawl, crouch)
             0x156C0004,                              # bne t3, t4, back
             0x27A70018,                              # addiu a3, sp, 0x18
             0x97AB0028,                              # lhu t3, 0x28(sp)    room_number
             0,
             0xA7AB0018,                              # sh t3, 0x18(sp)
             j(front + 8), 0]                         # back
    p.put(at, tramp)
    p.put(front, [j(at)])
    s = at + 4 * len(tramp)
    p.put(s, [0x10400003, 0,                          # beqz v0, no          (height2 >= front_floor)
              0x04600003, 0,                          # bltz v1, wall        (front_floor < 0: above her feet)
              j(slope + 8), 0,                        # no
              j(slope + 0x94), 0x24028001])           # wall: addiu v0, zero, -0x7FFF
    p.put(slope, [j(s), 0])
    return p, 4 * len(tramp) + 32


MIST_ROOM = """
    lhu   $t0, 0x4e($fp)
    nop
    srl   $t0, $t0, 11
    andi  $t2, $t0, 8
    beqz  $t2, keep
    andi  $t0, $t0, 3
    addiu $t2, $zero, 512
    srlv  $t2, $t2, $t0
keep:
    lui   $t0, BIAS_HI
    sh    $t2, BIAS_LO($t0)
    lbu   $t0, 0x36($fp)
    j     RETURN
    nop
"""
MIST_FACE = """
    lw    $t0, 4($a3)
    nop
    srl   $t0, $t0, 25
    andi  $t0, $t0, 1
    beqz  $t0, add
    nop
    lui   $t0, BIAS_HI
    lhu   $t0, BIAS_LO($t0)
    nop
    subu  $t9, $t9, $t0
    bgez  $t9, add
    nop
    move  $t9, $zero
add:
    add   $t9, $t9, $s0
    lw    $t0, 0($t9)
    sw    $a3, 0($t9)
    or    $t0, $t0, $t1
    sw    $t0, 0($a3)
    jr    $ra
    add   $a3, $a3, $t2
"""
MIST_PIECE = """
    lw    $t5, 4($a3)
    addi  $t9, $t9, 0x80
    srl   $t5, $t5, 25
    andi  $t5, $t5, 1
    beqz  $t5, back
    nop
    lui   $t5, BIAS_HI
    lhu   $t5, BIAS_LO($t5)
    nop
    subu  $t9, $t9, $t5
    slt   $t5, $t9, $s0
    beqz  $t5, back
    nop
    move  $t9, $s0
back:
    lui   $t5, SIZE
    j     RETURN
    nop
"""
ADDPRIM = [0x0330C820, 0x8F280000, 0xAF270000, 0x01094025, 0xACE80000, 0x03E00008, 0x00EA3820]


def mistsort(us, exe, orig, at):
    """The room loop ('lbu t0, 0x36(fp)', fp = room; delay slot 'lui t1, 0x1F') keeps the room's bias in bytes
    (4 per table entry) in BIAS; AddPrim ('add t9, t9, s0', t9 = farthest corner) and the two piece routines
    ('addi t9, t9, 0x80; lui t5, size') move semi-transparent faces nearer by it. gp is not the game's gp inside
    the renderer: BIAS is addressed absolutely."""
    room = exe.pattern([0x93C80036, 0x3C09001F], "room loop")
    addprim = exe.pattern(ADDPRIM, "AddPrim")
    pieces = [(exe.pattern([0x23390080, 0x3C0D0000 | size], f"pieces {size:#x}"), size) for size in (0x0C00, 0x0900)]
    bias = at
    hi, lo = hi_lo(bias)
    sub = lambda src, **kw: src.replace("BIAS_HI", str(hi)).replace("BIAS_LO", str(lo)).replace(
        "RETURN", hex(kw.get("ret", 0))).replace("SIZE", hex(kw.get("size", 0)))
    p = Patch()
    p.put(bias, bytes(4))
    a = bias + 4
    code = asm(sub(MIST_ROOM, ret=room + 8), a)
    p.put(a, code)
    p.put(room, [j(a)])
    a += len(code)
    code = asm(sub(MIST_FACE), a)
    p.put(a, code)
    p.put(addprim, [j(a), 0])
    a += len(code)
    for site, size in pieces:
        code = asm(sub(MIST_PIECE, ret=site + 8, size=size), a)
        p.put(a, code)
        p.put(site, [j(a)])
        a += len(code)
    return p, a - at


def cdpause(us, exe, orig, at):
    site = exe.window(us, 0x80063AC8, 8, 8)
    wait = exe.pattern([0x2404000E, 0x27A50010, (0x0C000000, 0xFC000000), 0x00003021, (0x0C000000, 0xFC000000),
                        0x24040003, 0x24040009], "VSync(3) after setting the CD mode") + 20
    pause_b = exe.window(us, 0x8007813C, 8, 8)       # CdControlB
    pause_f = exe.window(us, 0x80078008, 8)          # CdControlF
    assert exe.w(site) == jal(pause_b) and exe.w(wait) == 0x24040003
    p = Patch()
    p.put(site, [jal(pause_f)])
    p.put(wait, [0x2404FFFF])                         # VSync(3) -> VSync(-1)
    return p, 0


def sprintstep(us, exe, orig, at):
    site = exe.window(us, 0x80013644, 8, 16)    # lara_col_dash (lara_col_run has the same test)
    assert exe.w(site) == 0x10400013                  # beqz v0, +0x13 -> b
    p = Patch()
    p.put(site, [0x10000013])
    return p, 0


def killedwater(us, exe, orig, at):
    hook = exe.window(us, 0x80042A74, 8, 8)
    assert exe.w(hook) == 0x85030018                  # lh v1, 0x18(t0); delay slot 'addiu v0, zero, 0xFF'
    tramp = [0x95030028,                              # lhu v1, 0x28(t0)    item flags
             0,
             0x30638000,                              # andi v1, v1, 0x8000 (KillItem)
             0x10600009,                              # beqz v1, normal
             0,
             0x8503000C,                              # lh v1, 0xC(t0)      object
             0,
             0x2462FFE7,                              # addiu v0, v1, -25
             0x2C420002,                              # sltiu v0, v0, 2     orca, diver
             0x14400007,                              # bnez v0, gone
             0x2463FFE0,                              # addiu v1, v1, -32   alligator
             0x10600005,                              # beqz v1, gone
             0,
             0x85030018,                              # normal: lh v1, 0x18(t0)
             0x240200FF,                              # addiu v0, zero, 0xFF
             j(hook + 8), 0,
             0x03E00008, 0]                           # gone: jr ra
    p = Patch()
    p.put(at, tramp)
    p.put(hook, [j(at)])
    return p, 4 * len(tramp)


BODIES_ORIG = [0x8E620084, 0, 0x30420100, 0x10400017, 0, 0x86620022, 0, 0x1C400013, 0,
               0x86620030, 0, 0x28420020, 0x1440000E, 0]
BODIES_NEW = [0x8E620084,                             # lw v0, 0x84(s3)    status
              0x86630022,                             # lh v1, 0x22(s3)    hit points
              0x30420100,                             # andi v0, v0, 0x100 clear body
              0x10400017,                             # beqz v0, keep
              0x8664000C,                             # lh a0, 0xC(s3)     object
              0x1C600015,                             # bgtz v1, keep
              0x86620030,                             # lh v0, 0x30(s3)    after_death
              0x2483FFE6,                             # addiu v1, a0, -26
              0x10600005,                             # beqz v1, remove    diver
              0x28420020,                             # slti v0, v0, 0x20
              0x10400003,                             # beqz v0, remove    after_death >= 32
              0x2484FFE0,                             # addiu a0, a0, -32
              0x1480000E,                             # bnez a0, keep      not an alligator
              0]


def bodies(us, exe, orig, at):
    site = exe.pattern(BODIES_ORIG, "dead body test")
    p = Patch()
    p.put(site, BODIES_NEW)
    return p, 0


def endmusic(us, exe, orig, at):
    """The credits routine's end ('lw a0, 0(v0); jal; addiu a2, zero, 6; jal; nop; lw ra, ...; jr ra') leaves
    through the XA call (the one inside the routine the credits start their track 0x79 with): a0 = 14, a1 = 1.
    GameStats' last eleven words are reordered around a call of the CD stop (the credits' call after waiting)."""
    end = exe.pattern([0x8C440000, (0x0C000000, 0xFC000000), 0x24060006, (0x0C000000, 0xFC000000), 0,
                       (0x8FBF0000, 0xFFFF0000), (0x8FB20000, 0xFFFF0000), (0x8FB10000, 0xFFFF0000),
                       (0x8FB00000, 0xFFFF0000), (0x27BD0000, 0xFFFF0000), 0x03E00008, 0], "end of the credits")
    stop = exe.w(end - 4 * 16)
    assert stop >> 26 == 3 and exe.w(end - 4 * 17) == 0 and exe.w(end - 4 * 18) >> 16 == 0x1440
    start = exe.pattern([(0x0C000000, 0xFC000000), 0x24040079], "credits track")
    wrapper = target(exe.w(start))
    xa = exe.w(wrapper + 0x1C)
    assert xa >> 26 == 3 and exe.w(wrapper + 0x20) == 0x00002821      # jal xa; move a1, zero
    p = Patch()
    p.put(end + 16, [0x24040000 | TRACK])             # delay slot of the last call: a0 = 14
    p.put(end + 40, [j(target(xa)), 0x24050001])      # jr ra -> j xa; a1 = 1 (loop)
    stats = exe.window(us, 0x8006DEBC, 11)
    o = [exe.w(stats + 4 * k) for k in range(11)]
    assert o[1] == 0x24030001 and o[3] == 0x0002102B and o[9] == 0x03E00008 and o[10] == 0
    p.put(stats, [stop, o[5], o[1], o[2], o[0], o[4], o[6], o[3], o[7], o[9], o[8]])
    return p, 0


def pacing(us, exe, orig, at):
    hits = [o for o in range(0x800, len(exe.d) - 32, 4)
            if word(exe.d, o) >> 26 == 3 and word(exe.d, o + 4) == 0x00002021 and word(exe.d, o + 8) >> 16 == 0x8F82
            and word(exe.d, o + 12) == 0 and word(exe.d, o + 16) == 0x24420001
            and word(exe.d, o + 20) == 0xAF820000 | word(exe.d, o + 8) & 0xFFFF
            and any(word(exe.d, o + k) >> 16 == 0xAF80 for k in (24, 28))]
    assert len(hits) == 1, f"frame end found {len(hits)} times"
    p = Patch()
    p.put(hits[0] + LOAD, [0])
    p.put(hits[0] + 16 + LOAD, [0])
    return p, 0


ROLL = """
    lw    $R, INPUT($gp)
    lh    $t3, 0xe($s0)
    lui   $t0, 0x4000
    and   $t0, $R, $t0
    lh    $t1, GUN($gp)
    beqz  $t0, released
    nop
    bnez  $t1, released
    nop
    addiu $sp, $sp, -16
    sw    $ra, 12($sp)
    sw    $R, 8($sp)
    move  $a0, $s0
    lh    $a1, 0x4e($s0)
    jal   FLOOR_FRONT
    addiu $a2, $zero, 256
    slti  $t0, $v0, -384
    bnez  $t0, refuse
    move  $a0, $s0
    lh    $a1, 0x4e($s0)
    jal   FLOOR_FRONT
    addiu $a2, $zero, 512
    slti  $t0, $v0, 384
    beqz  $t0, refuse
    nop
    lw    $ra, 12($sp)
    addiu $sp, $sp, 16
    addiu $at, $zero, 72
    sh    $at, 0x10($s0)
    jr    $ra
    move  $R, $zero
refuse:
    lh    $t3, 0xe($s0)
    lw    $R, 8($sp)
    lw    $ra, 12($sp)
    addiu $sp, $sp, 16
released:
    addiu $at, $zero, 72
    bne   $t3, $at, done
    addiu $t4, $zero, 71
    sh    $t4, 0x10($s0)
done:
    jr    $ra
    nop
"""


def rollcheck(us, exe, orig, at):
    """duckroll.ips' call in the crouch routine goes to this routine instead: no roll when the floor 256 ahead is
    more than 384 higher (or a wall) or the floor 512 ahead 384 or more lower (an edge); a roll under way then
    ends in the crouch. The input word is returned in the register the crouch routine reads (a0 in Japan)."""
    ctl, _ = dr.state_tables(orig)
    site, inp, r = dr.input_site(orig, word(orig, ctl + 4 * dr.DUCK))
    site += LOAD
    assert exe.w(site) >> 26 == 3 and exe.w(site + 4) == 0, "duckroll.ips not applied"
    floor_front = cf.sites(orig)[2]
    src = (ROLL.replace("$R", "$" + {2: "v0", 4: "a0"}[r]).replace("INPUT", hex(inp))
           .replace("GUN", hex(cf.gun_status(orig))).replace("FLOOR_FRONT", hex(floor_front)))
    code = asm(src, at)
    p = Patch()
    p.put(at, code)
    p.put(site, [jal(at)])
    return p, len(code)


HARPOON = """
    lhu   $v0, LAST($gp)
    lh    $t0, WATER($gp)
    lui   $t2, SAVE_HI
    addiu $t0, $t0, -1
    sltiu $t0, $t0, 2
    bnez  $t0, water
    nop
    lhu   $t1, SAVE_LO($t2)
    addiu $t0, $zero, 8
    bne   $v0, $t0, out
    nop
    beqz  $t1, out
    nop
    move  $v0, $t1
    b     out
    sh    $zero, SAVE_LO($t2)
water:
    addiu $sp, $sp, -16
    sw    $ra, 12($sp)
    sw    $v0, 8($sp)
    jal   REQUEST
    addiu $a0, $zero, ITEM
    move  $t0, $v0
    lw    $v0, 8($sp)
    lw    $ra, 12($sp)
    addiu $sp, $sp, 16
    beqz  $t0, out
    addiu $t0, $zero, 8
    beq   $v0, $t0, out
    lui   $t2, SAVE_HI
    sh    $v0, SAVE_LO($t2)
    addiu $v0, $zero, 8
out:
    j     BACK
    nop
"""


def harpoon(us, exe, orig, at):
    """LaraGun's Draw ('andi v0, v1, 0x20; beqz v0; lui v0, 8; lhu v0, last_gun(gp); j request; nop'): under water
    or at the surface (water status = last_gun + 4: 1, 2) with the harpoon gun (Inv_RequestItem 164) the request is
    the harpoon (8) and the last gun is kept in SAVE; the next Draw out of the water takes that gun again."""
    a = exe.pattern([0x30620020, 0x10400004, 0x3C020008, (0x97820000, 0xFFFF0000), (0x08000000, 0xFC000000), 0,
                     0x00621024], "Draw") + 12
    last, back = exe.w(a) & 0xFFFF, target(exe.w(a + 4))
    request = exe.window(us, 0x8003EED0, 8)
    def src(save):
        hi, lo = hi_lo(save)
        return (HARPOON.replace("LAST", hex(last)).replace("WATER", hex(last + 4)).replace("SAVE_HI", str(hi))
                .replace("SAVE_LO", str(lo)).replace("REQUEST", hex(request)).replace("ITEM", str(HARPOON_ITEM))
                .replace("BACK", hex(back)))
    n = len(asm(src(at), at))
    code = asm(src(at + n), at) + bytes(4)
    p = Patch()
    p.put(at, code)
    p.put(a, [j(at), 0])
    return p, len(code)


VOICESTEAL = """
    lui   $at, STATE_HI
    addu  $at, $at, $a0
    lbu   $v0, STATE_LO($at)
    nop
    beqz  $v0, out
    nop
    j     PUSH
    nop
out:
    jr    $ra
    nop
"""


def voicesteal(us, exe, orig, at):
    """SoundEffect's 'jal stop' (US 0x8005D850) before it starts the new sound on the stolen voice: the stop
    (+0x10: the voice library's key off, state byte table, push to the free list) without the key off."""
    site = exe.window(us, 0x8005D850, 8)
    wrapper = target(exe.w(site))
    assert exe.w(site) >> 26 == 3 and exe.w(wrapper + 0x10) >> 26 == 3, "stop call"
    stop = target(exe.w(wrapper + 0x10))
    lui, lbu, push = exe.w(stop + 0x20), exe.w(stop + 0x28), exe.w(stop + 0x38)
    assert lui >> 16 == 0x3C01 and lbu >> 16 == 0x9022 and push >> 26 == 3, "stop routine"
    lo = lbu & 0xFFFF
    code = asm(VOICESTEAL.replace("STATE_HI", hex(lui & 0xFFFF)).replace("STATE_LO", str(lo - 0x10000 if lo & 0x8000 else lo))
               .replace("PUSH", hex(target(push))), at)
    p = Patch()
    p.put(at, code)
    p.put(site, [jal(at)])
    return p, len(code)


FALLEN = """
    lw    $v0, 0($a1)
    lh    $v1, WATER($gp)
    slti  $v0, $v0, 0x181
    bnez  $v0, ret0
    addiu $v0, $zero, 4
    beq   $v1, $v0, ret0
    lw    $t0, INPUT($gp)
    j     CHECK
    lh    $t1, GUN($gp)
set:
    lw    $a2, ANIMS($gp)
    sll   $t0, $t4, 5
    addu  $a2, $a2, $t0
    lhu   $a1, 0x10($a2)
    lw    $v1, 0x84($a0)
    sh    $t4, 0x14($a0)
    sh    $t5, 0xe($a0)
    sh    $t5, 0x10($a0)
    sh    $zero, 0x20($a0)
    ori   $v1, $v1, 8
    sw    $v1, 0x84($a0)
    sh    $a1, 0x16($a0)
    jr    $ra
    addiu $v0, $zero, 1
ret0:
    jr    $ra
    move  $v0, $zero
"""
DROP = """
    lw    $t2, 0($a1)
    lh    $t3, 0x4e($a0)
    andi  $t0, $t0, ACTION
    beqz  $t0, fall
    slti  $t2, $t2, MIN
    bnez  $t1, fall
    addiu $t3, $t3, CONE
    bnez  $t2, fall
    andi  $t3, $t3, 0x3fff
    sltiu $t3, $t3, WIDTH
    beqz  $t3, fall
    lh    $t0, 0x18($a0)
    addiu $sp, $sp, -0xa8
    sw    $ra, 0xa4($sp)
    sw    $a0, 0xa0($sp)
    sw    $t0, 16($sp)
    addiu $t0, $zero, HITE
    sw    $t0, 20($sp)
    lh    $t0, 0x76($a1)
    sw    $zero, 0x60($sp)
    sh    $t0, 0x8e($sp)
    lw    $a3, 0x6c($a1)
    lw    $a2, 0x68($a1)
    lw    $a1, 0x64($a1)
    jal   COLLISION
    addiu $a0, $sp, 24
    lw    $t2, 24($sp)
    lh    $t3, 0x90($sp)
    lb    $t0, 0x98($sp)
    lb    $t1, 0x99($sp)
    lw    $a0, 0xa0($sp)
    lw    $ra, 0xa4($sp)
    andi  $t3, $t3, 1
    beqz  $t3, axis
    addiu $sp, $sp, 0xa8
    move  $t0, $t1
axis:
    addiu $t0, $t0, 1
    sltiu $t0, $t0, 3
    bnez  $t2, fall
    addiu $t4, $zero, DROP_ANIM
    beqz  $t0, fall
    addiu $t5, $zero, REACH
    addiu $t0, $zero, 2
    j     SET
    sh    $t0, 0x1e($a0)
fall:
    addiu $t4, $zero, FALL_ANIM
    j     SET
    addiu $t5, $zero, JUMP
"""


def controlleddrop(us, exe, orig, at):
    """LaraFallen (US 0x8001A750, 25 words; Lara's mid floor more than 384 below and not wading) is rewritten
    in place: it loads the input and the gun status and jumps to DROP, which decides between TRX's controlled
    drop and the fall and comes back to the common part (set: animation t4 from its first frame, both states
    t5, fall speed 0, gravity, return 1). The test of Lara's old position is the game's GetCollisionInfo
    (US 0x80026370) on a COLL_INFO of its own on the stack at coll->old, in her room, with her collision's
    facing (lara.move_angle in every caller), radius 0 and LARA_HITE, as TRX's M_CanControlDrop: mid floor 0
    and |tilt| < 2 in the axis of its quadrant (x north/south, z east/west)."""
    fallen = exe.window(us, 0x8001A750, 25)
    w = [exe.w(fallen + 4 * k) for k in range(25)]
    assert w[0] == 0x8CA20000 and w[5] >> 16 == 0x8783 and w[9] >> 16 == 0x8F84 and w[12] == 0x94850450, "LaraFallen"
    s16 = lambda x: (x & 0xFFFF) - (0x10000 if x & 0x8000 else 0)
    water, anims = s16(w[5]), s16(w[9])
    gun = s16(cf.gun_status(orig))
    ctl, _ = dr.state_tables(orig)
    _, inp, _ = dr.input_site(orig, word(orig, ctl + 4 * dr.DUCK))
    assert water - gun == 10, (water, gun)
    coll = exe.window(us, 0x80026370, 40)
    stores = [exe.w(coll + 4 * k) for k in range(0xA0)]
    assert exe.w(coll + 0x68) == 0xA6220078 and 0xA2200080 in [x & 0xFFE0FFFF for x in stores]         and 0xA2200081 in [x & 0xFFE0FFFF for x in stores], "GetCollisionInfo"
    p = Patch()
    drop = asm(DROP.replace("ACTION", hex(IN_ACTION)).replace("MIN", str(DROP_MIN)).replace("CONE", str(HANG_ANGLE))
               .replace("WIDTH", str(2 * HANG_ANGLE + 1)).replace("HITE", str(LARA_HITE))
               .replace("COLLISION", hex(coll)).replace("DROP_ANIM", str(DROP_ANIM)).replace("REACH", str(AS_REACH))
               .replace("FALL_ANIM", str(FALL_ANIM)).replace("JUMP", str(AS_FORWARDJUMP))
               .replace("SET", hex(fallen + 36)), at)
    code = asm(FALLEN.replace("WATER", str(water)).replace("INPUT", str(s16(inp))).replace("GUN", str(gun))
               .replace("ANIMS", str(anims)).replace("CHECK", hex(at)), fallen)
    assert len(code) == 100
    p.put(at, drop)
    p.put(fallen, code)
    return p, len(drop)


def base(path, ver):
    """The executable with the patches of bin/patch.sh before these."""
    exe = open(path, "rb").read()
    sh = open(os.path.join(BIN, "patch.sh"), encoding="utf-8").read()
    names = [l.split('"')[1].split(",") for l in sh.splitlines() if l.startswith("patches=")][0]
    for name in names[:names.index(NAMES[0])] if NAMES[0] in names else names:
        exe = rv.apply_ips(exe, open(os.path.join(BIN, "patches", ver, f"{name}.ips"), "rb").read())
    return open(path, "rb").read(), exe


def build(us, orig, data):
    exe = Exe(data)
    out, at = {}, None
    for name in NAMES:
        p, used = globals()[name](us, exe, orig, at)
        if name == "debugspace":
            lo = at = used
        else:
            at += used
        p.apply(exe)
        out[name] = p
    assert at <= lo + DEBUG_SIZE, f"{at - lo} bytes, {DEBUG_SIZE} free"
    return out, at - lo


if __name__ == "__main__":
    ap = argparse.ArgumentParser()
    ap.add_argument("--versions", default="us,ukrev,derev,frrev,itrev,sprev,jp,jp2,as,as2")
    ap.add_argument("--exe", nargs="*", default=[], help="VERSION=PATH for executables not in bin/orig")
    ap.add_argument("--dry", action="store_true", help="only report, write nothing")
    a = ap.parse_args()
    paths = {v: os.path.join(BIN, "orig", f"{v}.exe") for v in a.versions.split(",") if v}
    paths.update(dict(e.split("=", 1) for e in a.exe))
    us = Exe(base(os.path.join(BIN, "orig", "us.exe"), "us")[1])
    for ver, path in paths.items():
        orig, data = base(path, ver)
        patches, used = build(us, orig, data)
        print(f"{ver}: {used} of {DEBUG_SIZE} bytes; " + ", ".join(
            f"{n} {min(a for a, _ in p.parts):#x}" for n, p in patches.items()))
        if not a.dry:
            for n, p in patches.items():
                open(os.path.join(BIN, "patches", ver, f"{n}.ips"), "wb").write(p.ips())
