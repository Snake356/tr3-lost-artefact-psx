"""Builds bin/patches/<version>/cornershimmy.ips: TRX's corner shimmying. Hanging from a ledge with Action held,
Left or Right where the ledge does not go on takes Lara round an outer or an inner corner (animations 322 to 329,
tools/add_corner_shimmy.py and tools/psx_lara_anims.py); on a climbable wall the same round the wall's corners
(animations 330 to 337, tools/add_climb_corners.py).

As TRX's M_Hang / M_TryCornerShimmy with enable_corner_shimmying:

  - the hang's control routine (lara_control_routines[10]) runs with Left and Right taken out of the input
    word (and put back behind it), so it no longer starts the shimmy; its collision routine
    (lara_collision_routines[10]) decides first, when Action is held, Lara is alive and not on a climbable
    wall (lara.climb_status), Left or Right is held and the hang (animation 96) is at its frame 21:
  - M_CanHangSideways: Lara 16 to the side, the game's two probes of LaraHangTest (GetLaraCollisionInfo with
    the side and with her facing, 4 nearer the wall) and its stop test (left2/right2 floors, mid ceiling,
    COLL_FRONT, the side's front floor below 200, hit_static, the front floor against GetBoundsAccurate's
    min y), without LaraHangTest's changes to Lara; free: the shimmy (goal 30 or 31), as TRX,
  - else M_TestHangCorner: the inner corner (Lara's position in her sector mirrored onto the other face,
    turned 90 degrees) and, when that fails and the floor 116 to the side is not above her, the outer corner
    (the diagonal sector, turned back), each checked by M_IsValidHangPos (LaraFloorFront 100 ahead at least
    200 below, 4 nearer, GetLaraCollisionInfo with bad_neg -512: mid ceiling below, COLL_FRONT, no static,
    the front and right2 floors within 60, left2 level with right2 where it is level with the front) and the
    front floor within 60 of the one before; the outer corner only from the half of the sector that meets it,
  - a corner found: animation 322 (left outer), 324 (right outer), 326 (left inner) or 328 (right inner)
    from its first frame, both states AS_LARATEST1 (62), the corner's position kept behind the routines,
  - climbing (TRX's M_SideLadder / M_TryLadderCorner / M_TestLadderCorner): the calls of LaraDoClimbLeftRight in
    lara_col_climbleft and lara_col_climbright go through a routine that, when LaraTestClimbPos found no wall
    to the side (result 0), Left or Right is held and Lara is in the climb left (171) or right (170) animation
    (also on its first frame after the climbing stance, where the game moved her along lara.move_angle still
    pointing at the wall), puts her back at coll->old and tests
    the inner corner (climb bit of the wall she would face in the corner's floor sector: GetFloor and GetHeight
    at y 32512, trigger_index as TestTriggers reads it, lava skipped) and LaraTestClimbPos there (radius ahead,
    radius + 120 to the side, -512, 512), then the outer corner the same way; a corner found and the end of
    Lara's sector in the direction she climbs at most CLIMB_REACH away (else the game's LaraDoClimbLeftRight,
    Lara's facing and lara.move_angle as lara_col_climbleft/right set them, to the side, so the next try moves
    her along the wall):
    Lara moved along the wall to the distance from that end the animation is made for (outer 220 left, 226
    right, inner 254 left, 260 right: there its end pose meets the climbing stance at the corner's position;
    TRX starts where she stops, about 220, so the inner turns went 35 to 40 into the side wall and jumped at
    the end), the corner's position worked out from there, animation 330 (left outer), 332 (right outer), 334
    (left inner) or 336 (right inner), both states 62,
  - state 62 (both tables; the game's LARATEST1 routines are empty): the camera at 90 degrees and -6144 as
    TRX, the fall when Lara dies (animation 34, AS_FORWARDJUMP, 256 down, speed 2, fall speed 1, hands free,
    half the turn), and at the end animation (323 ... 337, the odd ones) back in the hang (96, frame 21,
    AS_HANG) or the climbing stance (164, AS_CLIMBSTNC), Lara at the corner's position (and coll->old), turned
    by 90 degrees; after an outer climbing corner with the chase camera (camera.type 0), as TRX, the camera is
    brought round first: Lara 256 towards the corner, camera.speed 1, CalculateCamera; a position further than
    2047 from Lara (a stale one) is not used, then she only goes back into the hang or stance where she is; its
    collision is TRX's M_ShimmyCorner (bad_pos 384, bad_neg -384, slopes are walls and pits),
  - hanging on a climbable wall (climb_status) stays as in the game.

Found in every version: Lara's state tables (make_duckroll_ips.state_tables), LaraHangTest (the first call
of lara_col_hang), GetLaraCollisionInfo (its first call) and GetBoundsAccurate (the call before
'lh x, 6(v0)'), LaraFloorFront (make_crawlflip_ips.sites), the input and climb status gp offsets
(make_ledgejump_ips.sites), the animation list's ('lw x, anims(gp)' before 'lh y, 0xc10(x)', animation 96's
frame base, in LaraHangTest and lara_col_hang), lara.move_angle (lara_col_hangleft, lara_col_hangright), the camera's target angle and elevation
(lara_as_hangleft, lara_as_hang), the gun status (make_crawlflip_ips.gun_status), trigger_index (GetCollisionInfo's
'lw x, trigger_index(gp) ... sw x, 0x7c(coll)'), LaraTestClimbPos and LaraDoClimbLeftRight (the calls of
lara_col_climbleft and lara_col_climbright, lara_collision_routines[58] and [60]), CalculateCamera with camera.type
and camera.speed (by its start, checked against LookUpDown and lara_as_switchon), GetFloor and GetHeight
(make_ledgejump_ips.sites) and the sine table (make_crawlflip_ips.sin_table). The routines go behind everything the patches before this one put
into the block pickup3d.ips appends (copied at start-up to the end of the level memory).

Usage: make_cornershimmy_ips.py [--versions us,ukrev,...] [--exe VERSION=PATH ...]"""
import argparse
import os
import re
import struct

import make_crawlflip_ips as cf
import make_duckroll_ips as dr
import make_fixes_ips as fx
import make_ledgejump_ips as lj
import make_pickup3d_ips as p3
import make_reunionview_ips as rv

HERE = os.path.dirname(os.path.abspath(__file__))
BIN = os.path.join(HERE, "..", "bin")
LOAD = rv.LOAD
NAME = "cornershimmy"

IN_LEFT, IN_RIGHT, IN_ACTION = 0x0404, 0x0808, 0x40     # Left / Right with the step buttons
AS_FORWARDJUMP, AS_HANG, AS_HANGLEFT, AS_HANGRIGHT, AS_CORNER = 3, 10, 30, 31, 62
HANG, HANG_FRAME, FALL_START, CORNER = 96, 21, 34, 322
CLIMB_CORNER, STANCE, LADDER_RIGHT = 330, 164, 170           # LADDER_LEFT is 171
AS_STANCE, AS_CLIMBLEFT, AS_CLIMBRIGHT = 56, 58, 60
CLIMB_WIDTH = 120
CLIMB_REACH = 260                                          # the sector end at most this far: a climbing corner
START_OUTER_LEFT, RIGHT_EXTRA, INNER_EXTRA = 220, 6, 34    # start distances the animations are made for
CAM_ELEVATION = -6144

CODE = """
as_hang:
    lh    $t0, CLIMB($gp)
    lw    $t1, INPUT($gp)
    bnez  $t0, as_hang_game
    addiu $t2, $zero, NO_SIDE
    addiu $sp, $sp, -24
    sw    $ra, 20($sp)
    sw    $t1, 16($sp)
    and   $t2, $t1, $t2
    jal   AS_HANG_FN
    sw    $t2, INPUT($gp)
    lw    $t1, 16($sp)
    lw    $ra, 20($sp)
    lw    $t0, INPUT($gp)
    andi  $t1, $t1, SIDE
    or    $t0, $t0, $t1
    sw    $t0, INPUT($gp)
    jr    $ra
    addiu $sp, $sp, 24
as_hang_game:
    j     AS_HANG_FN
    nop

col_hang:
    lw    $t0, INPUT($gp)
    lh    $t1, 0x22($a0)
    lh    $t3, 0x14($a0)
    andi  $t2, $t0, ACTION
    beqz  $t2, col_hang_game
    lh    $t4, CLIMB($gp)
    blez  $t1, col_hang_game
    addiu $t5, $zero, HANG
    bne   $t3, $t5, col_hang_game
    andi  $t6, $t0, LEFT
    bnez  $t4, col_hang_game
    andi  $t7, $t0, RIGHT
    or    $t5, $t6, $t7
    lw    $t2, ANIMS($gp)
    beqz  $t5, col_hang_game
    lh    $t3, 0x16($a0)
    lh    $t2, HANG_BASE($t2)
    addiu $t3, $t3, -HANG_FRAME
    beq   $t3, $t2, corner
    nop
col_hang_game:
    j     COL_HANG_FN
    nop

corner:
    addiu $sp, $sp, -64
    sw    $ra, 60($sp)
    sw    $s0, 56($sp)
    sw    $s1, 52($sp)
    sw    $s2, 48($sp)
    sw    $s3, 44($sp)
    sw    $s4, 40($sp)
    sw    $s5, 36($sp)
    sw    $s6, 32($sp)
    sw    $s7, 28($sp)
    move  $s0, $a0
    move  $s1, $a1
    sltu  $s2, $zero, $t6
    lhu   $s6, 0x4e($s0)
    lw    $s3, 0x40($s0)
    lw    $s4, 0x48($s0)
    sll   $t1, $s2, 15
    addiu $t0, $zero, 0x4000
    subu  $s7, $t0, $t1
    addu  $a1, $s6, $s7
    sh    $a1, MOVE($gp)
    move  $a0, $s0
    jal   step
    addiu $a2, $zero, 16
    jal   info
    addiu $a2, $zero, -0x7f00
    lw    $t0, 0xc($s1)
    sh    $s6, MOVE($gp)
    slti  $t0, $t0, 200
    sw    $t0, 24($sp)
    move  $a0, $s0
    move  $a1, $s6
    jal   step
    addiu $a2, $zero, 4
    jal   info
    addiu $a2, $zero, -384
    sw    $s3, 0x40($s0)
    sw    $s4, 0x48($s0)
    addu  $t0, $s6, $s7
    sh    $t0, MOVE($gp)
    lw    $t0, 0xc($s1)
    lw    $t1, 0x30($s1)
    bgtz  $t0, sideways
    lw    $t2, 0x3c($s1)
    lw    $t3, 4($s1)
    lh    $t4, 0x7a($s1)
    lb    $t5, 0x83($s1)
    lw    $t6, 24($sp)
    subu  $t1, $t1, $t2
    sra   $t9, $t1, 31
    xor   $t1, $t1, $t9
    subu  $t1, $t1, $t9
    slti  $t1, $t1, 60
    slti  $t3, $t3, 0
    and   $t1, $t1, $t3
    xori  $t4, $t4, 1
    or    $t4, $t4, $t5
    or    $t4, $t4, $t6
    sltiu $t4, $t4, 1
    and   $t1, $t1, $t4
    beqz  $t1, blocked
    nop
    jal   BOUNDS_FN
    move  $a0, $s0
    lh    $t0, 4($v0)
    lw    $t1, 0xc($s1)
    nop
    subu  $t1, $t1, $t0
    addiu $t1, $t1, 60
    sltiu $t1, $t1, 121
    beqz  $t1, blocked
    nop
sideways:
    addiu $t1, $zero, HANGRIGHT
    subu  $t1, $t1, $s2
    beq   $zero, $zero, corner_out
    sh    $t1, 0x10($s0)
blocked:
    lb    $t0, 0x83($s1)
    lw    $t1, 0xc($s1)
    bnez  $t0, corner_none
    addiu $t2, $s6, 0x2000
    sw    $t1, 24($sp)
    andi  $t2, $t2, 0xffff
    srl   $s5, $t2, 14
    sll   $t3, $s2, 1
    addu  $t3, $t3, $s2
    addu  $t3, $t3, $s5
    andi  $a2, $t3, 3
    sw    $a2, 20($sp)
    move  $a0, $s3
    jal   inner
    move  $a1, $s4
    sw    $v0, 0x40($s0)
    sw    $v1, 0x48($s0)
    lui   $t0, CORNER_HI
    sw    $v0, CORNER_LO($t0)
    sw    $v1, CORNER_LO4($t0)
    addu  $t1, $s6, $s7
    jal   valid
    sh    $t1, 0x4e($s0)
    beqz  $v0, inner_none
    lw    $t0, 24($sp)
    lw    $t1, 0xc($s1)
    nop
    subu  $t0, $t0, $t1
    sra   $t9, $t0, 31
    xor   $t0, $t0, $t9
    subu  $t0, $t0, $t9
    slti  $t0, $t0, 61
    bnez  $t0, corner_found
    addiu $v0, $zero, -1
inner_none:
    sw    $s3, 0x40($s0)
    sw    $s4, 0x48($s0)
    sh    $s6, 0x4e($s0)
    sh    $s6, MOVE($gp)
    move  $a0, $s0
    addu  $a1, $s6, $s7
    jal   FLOOR_FRONT_FN
    addiu $a2, $zero, 116
    bltz  $v0, corner_none
    lw    $a2, 20($sp)
    move  $a0, $s3
    jal   outer
    move  $a1, $s4
    sw    $v0, 0x40($s0)
    sw    $v1, 0x48($s0)
    lui   $t0, CORNER_HI
    sw    $v0, CORNER_LO($t0)
    sw    $v1, CORNER_LO4($t0)
    subu  $t1, $s6, $s7
    jal   valid
    sh    $t1, 0x4e($s0)
    sw    $s3, 0x40($s0)
    sw    $s4, 0x48($s0)
    sh    $s6, 0x4e($s0)
    beqz  $v0, corner_out
    sh    $s6, MOVE($gp)
    lw    $t0, 24($sp)
    lw    $t1, 0xc($s1)
    nop
    subu  $t0, $t0, $t1
    sra   $t9, $t0, 31
    xor   $t0, $t0, $t9
    subu  $t0, $t0, $t9
    slti  $t0, $t0, 61
    beqz  $t0, corner_out
    andi  $t1, $s5, 1
    beqz  $t1, side_x
    move  $t2, $s3
    move  $t2, $s4
side_x:
    andi  $t2, $t2, 1023
    addiu $t3, $s5, 3
    andi  $t3, $t3, 3
    sltiu $t3, $t3, 2
    xor   $t3, $t3, $s2
    xori  $t3, $t3, 1
    beqz  $t3, side_max
    slti  $t4, $t2, 512
    bnez  $t4, corner_out
    nop
    beq   $zero, $zero, corner_found
    addiu $v0, $zero, 1
side_max:
    slti  $t4, $t2, 513
    beqz  $t4, corner_out
    nop
    addiu $v0, $zero, 1
corner_found:
    sw    $s3, 0x40($s0)
    sw    $s4, 0x48($s0)
    sh    $s6, 0x4e($s0)
    sh    $s6, MOVE($gp)
    slti  $t0, $v0, 0
    sll   $t0, $t0, 2
    xori  $t1, $s2, 1
    sll   $t1, $t1, 1
    addu  $t0, $t0, $t1
    addiu $t0, $t0, CORNER_ANIM
    lw    $t1, ANIMS($gp)
    sll   $t2, $t0, 5
    addu  $t1, $t1, $t2
    lhu   $t3, 0x10($t1)
    sh    $t0, 0x14($s0)
    addiu $t4, $zero, AS_CORNER
    sh    $t4, 0xe($s0)
    sh    $t4, 0x10($s0)
    beq   $zero, $zero, corner_out
    sh    $t3, 0x16($s0)
corner_none:
corner_out:
    lw    $ra, 60($sp)
    lw    $s0, 56($sp)
    lw    $s1, 52($sp)
    lw    $s2, 48($sp)
    lw    $s3, 44($sp)
    lw    $s4, 40($sp)
    lw    $s5, 36($sp)
    lw    $s6, 32($sp)
    lw    $s7, 28($sp)
    jr    $ra
    addiu $sp, $sp, 64

valid:
    addiu $sp, $sp, -24
    sw    $ra, 20($sp)
    move  $a0, $s0
    lh    $a1, MOVE($gp)
    jal   FLOOR_FRONT_FN
    addiu $a2, $zero, 100
    slti  $v0, $v0, 200
    bnez  $v0, valid_no
    move  $a0, $s0
    lhu   $a1, 0x4e($s0)
    jal   step
    addiu $a2, $zero, 4
    lhu   $t0, 0x4e($s0)
    nop
    sh    $t0, MOVE($gp)
    jal   info
    addiu $a2, $zero, -512
    lw    $t1, 4($s1)
    lh    $t2, 0x7a($s1)
    lb    $t3, 0x83($s1)
    lw    $t4, 0xc($s1)
    lw    $t5, 0x3c($s1)
    lw    $t6, 0x30($s1)
    slti  $t1, $t1, 0
    xori  $t2, $t2, 1
    or    $t2, $t2, $t3
    sltiu $t2, $t2, 1
    and   $t1, $t1, $t2
    subu  $t7, $t4, $t5
    sra   $t9, $t7, 31
    xor   $t7, $t7, $t9
    subu  $t7, $t7, $t9
    slti  $t7, $t7, 60
    and   $t1, $t1, $t7
    beqz  $t1, valid_no
    subu  $t7, $t4, $t6
    sra   $t9, $t7, 31
    xor   $t7, $t7, $t9
    subu  $t7, $t7, $t9
    slti  $t7, $t7, 60
    subu  $t8, $t6, $t5
    sra   $t9, $t8, 31
    xor   $t8, $t8, $t9
    subu  $t8, $t8, $t9
    slti  $t8, $t8, 60
    xori  $t8, $t8, 1
    and   $t7, $t7, $t8
    lw    $ra, 20($sp)
    beq   $zero, $zero, valid_out
    sltiu $v0, $t7, 1
valid_no:
    lw    $ra, 20($sp)
    move  $v0, $zero
valid_out:
    jr    $ra
    addiu $sp, $sp, 24

step:
    addiu $t0, $a1, 0x2000
    andi  $t0, $t0, 0xffff
    srl   $t0, $t0, 14
    andi  $t1, $t0, 1
    sltiu $t2, $t0, 2
    bnez  $t2, step_add
    sll   $t1, $t1, 3
    subu  $a2, $zero, $a2
step_add:
    subu  $t1, $a0, $t1
    lw    $t2, 0x48($t1)
    nop
    addu  $t2, $t2, $a2
    jr    $ra
    sw    $t2, 0x48($t1)

info:
    addiu $t0, $zero, 0x7f00
    sw    $t0, 0x4c($s1)
    sw    $a2, 0x50($s1)
    sw    $zero, 0x54($s1)
    move  $a0, $s0
    j     COLL_INFO_FN
    move  $a1, $s1

inner:
    addiu $t0, $zero, -1024
    andi  $t1, $a0, 1023
    andi  $t2, $a1, 1023
    and   $t3, $a0, $t0
    and   $t4, $a1, $t0
    andi  $t5, $a2, 1
    bnez  $t5, inner_ew
    or    $v0, $t3, $t2
    jr    $ra
    or    $v1, $t4, $t1
inner_ew:
    subu  $v0, $t3, $t2
    addiu $v0, $v0, 1024
    subu  $v1, $t4, $t1
    jr    $ra
    addiu $v1, $v1, 1024

outer:
    addiu $t0, $zero, -1024
    andi  $t1, $a0, 1023
    andi  $t2, $a1, 1023
    and   $t3, $a0, $t0
    and   $t4, $a1, $t0
    addiu $t5, $a0, 1024
    and   $t5, $t5, $t0
    addiu $t6, $a1, 1024
    and   $t6, $t6, $t0
    beqz  $a2, outer_n
    addiu $t7, $a2, -2
    beqz  $t7, outer_s
    addiu $t7, $a2, -3
    beqz  $t7, outer_w
    or    $v0, $t5, $t2
    or    $v1, $t4, $t1
    jr    $ra
    addiu $v1, $v1, -1024
outer_n:
    subu  $v0, $t5, $t2
    addiu $v0, $v0, 1024
    subu  $v1, $t6, $t1
    jr    $ra
    addiu $v1, $v1, 1024
outer_s:
    subu  $v0, $t3, $t2
    jr    $ra
    subu  $v1, $t4, $t1
outer_w:
    or    $v0, $t3, $t2
    addiu $v0, $v0, -1024
    jr    $ra
    or    $v1, $t6, $t1

as_corner:
    lw    $t0, 0x84($a1)
    addiu $t1, $zero, -0x31
    and   $t0, $t0, $t1
    sw    $t0, 0x84($a1)
    lh    $t2, 0x14($a0)
    lh    $t3, 0x22($a0)
    addiu $t2, $t2, -CORNER_ANIM
    sltiu $t4, $t2, 16
    beqz  $t4, to_hang
    sra   $t5, $t2, 1
    andi  $t6, $t5, 3
    addiu $t6, $t6, 1
    andi  $t6, $t6, 2
    sll   $t6, $t6, 14
    addiu $t7, $zero, 0x4000
    subu  $t7, $t7, $t6
    sh    $t7, CAM_ANGLE($gp)
    addiu $t8, $zero, CAM_ELEVATION
    blez  $t3, dead
    sh    $t8, CAM_ELEV($gp)
    andi  $t4, $t2, 1
    bnez  $t4, at_end
    lw    $t3, 0x40($a0)
    jr    $ra
at_end:
    lw    $t4, 0x48($a0)
    lui   $t0, CORNER_HI
    lw    $t1, CORNER_LO($t0)
    lw    $t2, CORNER_LO4($t0)
    subu  $t3, $t1, $t3
    addiu $t3, $t3, 2047
    sltiu $t3, $t3, 4095
    subu  $t4, $t2, $t4
    addiu $t4, $t4, 2047
    sltiu $t4, $t4, 4095
    and   $t3, $t3, $t4
    addiu $sp, $sp, -40
    sw    $ra, 36($sp)
    sw    $s0, 32($sp)
    sw    $s1, 28($sp)
    sw    $s2, 24($sp)
    sw    $s3, 20($sp)
    move  $s0, $a0
    move  $s1, $a1
    move  $s2, $t7
    move  $s3, $t3
    lw    $t0, ANIMS($gp)
    sltiu $t9, $t5, 4
    beqz  $t9, end_ladder
    addiu $t1, $zero, HANG
    lhu   $t0, HANG_BASE($t0)
    addiu $t2, $zero, AS_HANG
    beq   $zero, $zero, end_set
    addiu $t0, $t0, HANG_FRAME
end_ladder:
    lhu   $t0, STANCE_BASE($t0)
    addiu $t1, $zero, STANCE
    addiu $t2, $zero, AS_STANCE
end_set:
    sh    $t1, 0x14($s0)
    sh    $t0, 0x16($s0)
    sh    $t2, 0xe($s0)
    sh    $t2, 0x10($s0)
    beqz  $s3, corner_done
    addiu $t3, $t5, -4
    sltiu $t3, $t3, 2
    beqz  $t3, teleport
    lw    $t4, CAM_TYPE($gp)
    lhu   $t0, 0x4e($s0)
    bnez  $t4, teleport
    subu  $t0, $t0, $s2
    srl   $t0, $t0, 2
    andi  $t0, $t0, 0x3ffc
    lui   $at, SIN_HI
    addu  $at, $at, $t0
    lh    $t1, SIN_LO($at)
    lh    $t2, SIN_LO2($at)
    lw    $t3, 0x40($s0)
    lw    $t4, 0x48($s0)
    sra   $t1, $t1, 4
    sra   $t2, $t2, 4
    addu  $t3, $t3, $t1
    addu  $t4, $t4, $t2
    sw    $t3, 0x40($s0)
    sw    $t4, 0x48($s0)
    addiu $t5, $zero, 1
    jal   CAMERA_FN
    sh    $t5, CAM_SPEED($gp)
teleport:
    lui   $t0, CORNER_HI
    lw    $t1, CORNER_LO($t0)
    lw    $t2, CORNER_LO4($t0)
    lhu   $t5, 0x4e($s0)
    sw    $t1, 0x64($s1)
    sw    $t2, 0x6c($s1)
    sw    $t1, 0x40($s0)
    sw    $t2, 0x48($s0)
    addu  $t5, $t5, $s2
    sh    $t5, 0x4e($s0)
corner_done:
    lw    $ra, 36($sp)
    lw    $s0, 32($sp)
    lw    $s1, 28($sp)
    lw    $s2, 24($sp)
    lw    $s3, 20($sp)
    jr    $ra
    addiu $sp, $sp, 40
to_hang:
    lw    $t0, ANIMS($gp)
    addiu $t1, $zero, HANG
    lhu   $t0, HANG_BASE($t0)
    addiu $t2, $zero, AS_HANG
    sh    $t1, 0x14($a0)
    addiu $t0, $t0, HANG_FRAME
    sh    $t0, 0x16($a0)
    sh    $t2, 0xe($a0)
    jr    $ra
    sh    $t2, 0x10($a0)
dead:
    lw    $t0, ANIMS($gp)
    addiu $t1, $zero, FALL_START
    lhu   $t0, FALL_BASE($t0)
    addiu $t2, $zero, AS_FORWARDJUMP
    sh    $t1, 0x14($a0)
    sh    $t0, 0x16($a0)
    sh    $t2, 0xe($a0)
    sh    $t2, 0x10($a0)
    lw    $t3, 0x44($a0)
    lw    $t4, 0x84($a0)
    addiu $t3, $t3, 256
    ori   $t4, $t4, 8
    sw    $t3, 0x44($a0)
    sw    $t4, 0x84($a0)
    addiu $t5, $zero, 2
    sh    $t5, 0x1e($a0)
    addiu $t5, $zero, 1
    sh    $t5, 0x20($a0)
    sh    $zero, GUN($gp)
    lhu   $t6, 0x4e($a0)
    sra   $t7, $t7, 1
    addu  $t6, $t6, $t7
    jr    $ra
    sh    $t6, 0x4e($a0)

climb_lr:
    bnez  $a2, climb_game
    lw    $t2, INPUT($gp)
    lh    $t6, 0x14($a0)
    andi  $t3, $t2, LEFT
    andi  $t4, $t2, RIGHT
    or    $t5, $t3, $t4
    beqz  $t5, climb_game
    sltu  $t7, $zero, $t3
    addiu $t8, $zero, LADDER_RIGHT
    addu  $t8, $t8, $t7
    beq   $t6, $t8, ladder
    nop
climb_game:
    j     CLIMB_LR_FN
    nop

ladder:
    addiu $sp, $sp, -96
    sw    $ra, 92($sp)
    sw    $s0, 88($sp)
    sw    $s1, 84($sp)
    sw    $s2, 80($sp)
    sw    $s3, 76($sp)
    sw    $s4, 72($sp)
    sw    $s5, 68($sp)
    sw    $s6, 64($sp)
    sw    $s7, 60($sp)
    move  $s0, $a0
    move  $s1, $a1
    move  $s2, $t7
    lw    $s3, 0x64($s1)
    lw    $s4, 0x6c($s1)
    lhu   $s6, 0x4e($s0)
    sw    $s3, 0x40($s0)
    sw    $s4, 0x48($s0)
    sll   $t1, $s2, 15
    addiu $t0, $zero, 0x4000
    subu  $s7, $t0, $t1
    addiu $t2, $s6, 0x2000
    andi  $t2, $t2, 0xffff
    srl   $s5, $t2, 14
    sll   $t3, $s2, 1
    addu  $t3, $t3, $s2
    addu  $t3, $t3, $s5
    andi  $t3, $t3, 3
    sw    $t3, 28($sp)
    lw    $t4, 0x48($s1)
    subu  $t6, $zero, $s2
    addiu $t4, $t4, CLIMB_WIDTH
    xor   $t5, $t4, $t6
    subu  $t5, $t5, $t6
    sw    $t5, 32($sp)
    move  $a2, $t3
    move  $a0, $s3
    jal   inner
    move  $a1, $s4
    sw    $v0, 36($sp)
    sw    $v1, 40($sp)
    move  $a0, $v0
    jal   flags
    move  $a1, $v1
    sll   $t0, $s2, 1
    addu  $t0, $t0, $s5
    addiu $t0, $t0, 1
    andi  $t0, $t0, 3
    addiu $t1, $zero, 1
    sllv  $t1, $t1, $t0
    and   $t1, $v0, $t1
    beqz  $t1, ladder_outer
    lw    $t2, 36($sp)
    lw    $t3, 40($sp)
    addu  $t4, $s6, $s7
    sw    $t2, 0x40($s0)
    sw    $t3, 0x48($s0)
    lui   $t5, CORNER_HI
    sw    $t2, CORNER_LO($t5)
    sw    $t3, CORNER_LO4($t5)
    sh    $t4, 0x4e($s0)
    sh    $t4, MOVE($gp)
    jal   testclimb
    lw    $a2, 32($sp)
    bnez  $v0, ladder_found
    addiu $v0, $zero, -1
ladder_outer:
    sw    $s3, 0x40($s0)
    sw    $s4, 0x48($s0)
    sh    $s6, 0x4e($s0)
    sh    $s6, MOVE($gp)
    lw    $a2, 28($sp)
    move  $a0, $s3
    jal   outer
    move  $a1, $s4
    sw    $v0, 36($sp)
    sw    $v1, 40($sp)
    move  $a0, $v0
    jal   flags
    move  $a1, $v1
    sll   $t0, $s2, 1
    subu  $t0, $s5, $t0
    addiu $t0, $t0, 3
    andi  $t0, $t0, 3
    addiu $t1, $zero, 1
    sllv  $t1, $t1, $t0
    and   $t1, $v0, $t1
    beqz  $t1, ladder_none
    lw    $t2, 36($sp)
    lw    $t3, 40($sp)
    subu  $t4, $s6, $s7
    sw    $t2, 0x40($s0)
    sw    $t3, 0x48($s0)
    lui   $t5, CORNER_HI
    sw    $t2, CORNER_LO($t5)
    sw    $t3, CORNER_LO4($t5)
    sh    $t4, 0x4e($s0)
    sh    $t4, MOVE($gp)
    jal   testclimb
    lw    $a2, 32($sp)
    beqz  $v0, ladder_none
    addiu $v0, $zero, 1
ladder_found:
    sw    $v0, 44($sp)
    sll   $t0, $s2, 1
    addu  $t0, $t0, $s5
    addiu $t0, $t0, 1
    andi  $t0, $t0, 3
    andi  $t3, $t0, 1
    andi  $t4, $s4, 1023
    beqz  $t3, reach_z
    sltiu $t5, $t0, 2
    andi  $t4, $s3, 1023
reach_z:
    beqz  $t5, reach_neg
    addiu $t6, $zero, 1024
    subu  $t4, $t6, $t4
reach_neg:
    slti  $t6, $t4, CLIMB_REACH
    beqz  $t6, ladder_none
    slti  $t7, $v0, 0
    subu  $t8, $zero, $t7
    andi  $t8, $t8, INNER_EXTRA
    xori  $t9, $s2, 1
    subu  $t9, $zero, $t9
    andi  $t9, $t9, RIGHT_EXTRA
    addu  $t8, $t8, $t9
    addiu $t8, $t8, START_OUTER_LEFT
    subu  $t4, $t4, $t8
    bnez  $t5, reach_pos
    nop
    subu  $t4, $zero, $t4
reach_pos:
    beqz  $t3, move_z
    nop
    beq   $zero, $zero, move_done
    addu  $s3, $s3, $t4
move_z:
    addu  $s4, $s4, $t4
move_done:
    lw    $a2, 28($sp)
    move  $a0, $s3
    bnez  $t7, start_inner
    move  $a1, $s4
    jal   outer
    nop
    beq   $zero, $zero, start_corner
    nop
start_inner:
    jal   inner
    nop
start_corner:
    lui   $t5, CORNER_HI
    sw    $v0, CORNER_LO($t5)
    sw    $v1, CORNER_LO4($t5)
    lw    $v0, 44($sp)
    sw    $s3, 0x40($s0)
    sw    $s4, 0x48($s0)
    sh    $s6, 0x4e($s0)
    sh    $s6, MOVE($gp)
    slti  $t0, $v0, 0
    sll   $t0, $t0, 2
    xori  $t1, $s2, 1
    sll   $t1, $t1, 1
    addu  $t0, $t0, $t1
    addiu $t0, $t0, CLIMB_ANIM
    lw    $t1, ANIMS($gp)
    sll   $t2, $t0, 5
    addu  $t1, $t1, $t2
    lhu   $t3, 0x10($t1)
    sh    $t0, 0x14($s0)
    addiu $t4, $zero, AS_CORNER
    sh    $t4, 0xe($s0)
    sh    $t4, 0x10($s0)
    beq   $zero, $zero, ladder_out
    sh    $t3, 0x16($s0)
ladder_none:
    sw    $s3, 0x40($s0)
    sw    $s4, 0x48($s0)
    sh    $s6, 0x4e($s0)
    addu  $t0, $s6, $s7
    sh    $t0, MOVE($gp)
    move  $a0, $s0
    move  $a1, $s1
    move  $a2, $zero
    lw    $ra, 92($sp)
    lw    $s0, 88($sp)
    lw    $s1, 84($sp)
    lw    $s2, 80($sp)
    lw    $s3, 76($sp)
    lw    $s4, 72($sp)
    lw    $s5, 68($sp)
    lw    $s6, 64($sp)
    lw    $s7, 60($sp)
    j     CLIMB_LR_FN
    addiu $sp, $sp, 96
ladder_out:
    lw    $ra, 92($sp)
    lw    $s0, 88($sp)
    lw    $s1, 84($sp)
    lw    $s2, 80($sp)
    lw    $s3, 76($sp)
    lw    $s4, 72($sp)
    lw    $s5, 68($sp)
    lw    $s6, 64($sp)
    lw    $s7, 60($sp)
    jr    $ra
    addiu $sp, $sp, 96

flags:
    addiu $sp, $sp, -32
    sw    $ra, 28($sp)
    sw    $a0, 20($sp)
    sw    $a1, 24($sp)
    lh    $t0, 0x18($s0)
    move  $a2, $a1
    addiu $a1, $zero, 0x7f00
    sh    $t0, 16($sp)
    jal   GET_FLOOR_FN
    addiu $a3, $sp, 16
    move  $a0, $v0
    lw    $a1, 20($sp)
    addiu $a2, $zero, 0x7f00
    jal   GET_HEIGHT_FN
    lw    $a3, 24($sp)
    lw    $t0, TRIGGER($gp)
    lw    $ra, 28($sp)
    addiu $sp, $sp, 32
    beqz  $t0, flags_none
    nop
    lhu   $t1, 0($t0)
    nop
    andi  $t2, $t1, 0x1f
    addiu $t3, $t2, -5
    bnez  $t3, flags_climb
    andi  $t3, $t1, 0x8000
    bnez  $t3, flags_none
    lhu   $t1, 2($t0)
    nop
    andi  $t2, $t1, 0x1f
flags_climb:
    addiu $t3, $t2, -6
    bnez  $t3, flags_none
    srl   $v0, $t1, 8
    jr    $ra
    andi  $v0, $v0, 15
flags_none:
    jr    $ra
    move  $v0, $zero

testclimb:
    addiu $sp, $sp, -32
    sw    $ra, 28($sp)
    addiu $t0, $zero, 512
    sw    $t0, 16($sp)
    addiu $t0, $sp, 24
    sw    $t0, 20($sp)
    lw    $a1, 0x48($s1)
    move  $a0, $s0
    jal   TEST_CLIMB_FN
    addiu $a3, $zero, -512
    lw    $ra, 28($sp)
    nop
    jr    $ra
    addiu $sp, $sp, 32

col_corner:
    lhu   $t0, 0x4e($a0)
    lw    $t2, 0x84($a1)
    addiu $t1, $zero, 384
    sh    $t0, MOVE($gp)
    sw    $t1, 0x4c($a1)
    addiu $t1, $zero, -384
    sw    $t1, 0x50($a1)
    sw    $zero, 0x54($a1)
    ori   $t2, $t2, 3
    j     COLL_INFO_FN
    sw    $t2, 0x84($a1)
data:
"""
ENTRIES = ("as_hang", "col_hang", "as_corner", "col_corner", "climb_lr")
DATA_SIZE = 8


def word(d, o):
    return struct.unpack_from("<I", d, o)[0]


def s16(x):
    return (x & 0xFFFF) - (0x10000 if x & 0x8000 else 0)


def patches_before(ver):
    sh = open(os.path.join(BIN, "patch.sh"), encoding="utf-8").read()
    names = [l.split('"')[1].split(",") for l in sh.splitlines() if l.startswith("patches=")][0]
    return names[:names.index(NAME)] if NAME in names else names


def patched(path, ver):
    exe = open(path, "rb").read()
    for name in patches_before(ver):
        exe = rv.apply_ips(exe, open(os.path.join(BIN, "patches", ver, f"{name}.ips"), "rb").read())
    return exe


def calls(exe, fn, n=0x400):
    """Targets of the calls in the function at fn (up to its 'jr ra')."""
    out, o = [], fn - LOAD
    while word(exe, o) != 0x03E00008 and o < fn - LOAD + n:
        if word(exe, o) >> 26 == 3:
            out.append(p3.jal_target(word(exe, o)))
        o += 4
    return out


def gp_stores(exe, fn, top, n=0x80):
    """gp offsets of 'sh <top>, x(gp)' in the first n bytes of the function at fn."""
    return [s16(word(exe, o)) for o in range(fn - LOAD, fn - LOAD + n, 4) if word(exe, o) >> 16 == top]


def anims_offset(exe, fn):
    """gp offset of the animation list: 'lw rX, x(gp)' before the first 'lh rY, 0xc10(rX)' (animation 96's
    frame base) in the function at fn."""
    o = fn - LOAD
    k = next(k for k in range(o, o + 0x400, 4) if word(exe, k) >> 26 == 0x21 and word(exe, k) & 0xFFFF == 0xC10)
    base = (word(exe, k) >> 21) & 31
    lw = next(j for j in range(k - 4, o, -4) if word(exe, j) >> 16 == (0x8F80 | base))
    return s16(word(exe, lw))


def collision_table(exe, ctl):
    """File offset of the collision table, behind the control table (which is longer than make_duckroll_ips'
    STATES): the routines for the hang and the shimmies (10, 30, 31) are three and all first call LaraHangTest
    (bad_pos 0x7f00 at its start)."""
    hits = []
    for k in range(dr.STATES, 2 * dr.STATES):
        t = [word(exe, ctl + 4 * (k + s)) for s in (AS_HANG, AS_HANGLEFT, AS_HANGRIGHT)]
        if all(0x10000 <= x < 0xA0000 for x in t) and len(set(t)) == 3:
            first = {(calls(exe, x | 0x80000000) or [0])[0] for x in t}
            if len(first) == 1 and first != {0} and 0x24027F00 in [word(exe, (min(first) - LOAD) + 4 * j)
                                                                   for j in range(12)]:
                hits.append(ctl + 4 * k)
    assert len(hits) == 1, f"collision table found {len(hits)} times"
    return hits[0]


def sites(us, orig, ver_path, ver):
    """Everything the routines use, from the released executable (orig) and the one before ledgejump.ips."""
    ctl, _ = dr.state_tables(orig)
    entry = lambda t, s: word(orig, t + 4 * s) | 0x80000000
    col = collision_table(orig, ctl)
    as_hang, col_hang = entry(ctl, AS_HANG), entry(col, AS_HANG)
    hang_test = calls(orig, col_hang)[0]
    coll_info = calls(orig, hang_test)[0]
    o = hang_test - LOAD
    bounds = {p3.jal_target(word(orig, k)) for k in range(o, o + 0x400, 4) if word(orig, k) >> 26 == 3
              and any(word(orig, k + 4 * j) >> 16 & 0xFFE0 == 0x8440 and word(orig, k + 4 * j) & 0xFFFF == 6
                      for j in (1, 2))}
    assert len(bounds) == 1, "GetBoundsAccurate"
    _, inp, floor_front, _, _ = cf.sites(orig)
    f = lj.sites(us, fx.Exe(lj.patched(ver_path, ver)), orig)
    assert f["input"] == inp
    anims = anims_offset(orig, hang_test)
    assert anims == anims_offset(orig, col_hang), "animation list"
    moves = {x for s in (30, 31) for x in gp_stores(orig, entry(col, s), 0xA782)}
    assert len(moves) == 1, f"lara.move_angle: {moves}"
    left = entry(ctl, AS_HANGLEFT)
    angle, elev = gp_stores(orig, left, 0xA780, 0x40), gp_stores(orig, left, 0xA783, 0x40)
    assert len(angle) == 1 and len(elev) == 1, "camera target"
    assert angle == gp_stores(orig, as_hang, 0xA780) and elev == gp_stores(orig, as_hang, 0xA782), "camera target"
    gun = s16(cf.gun_status(orig))
    assert gun in gp_stores(orig, hang_test, 0xA780, 0x400), "gun status"
    coll_fn = calls(orig, coll_info)[0]
    o = coll_fn - LOAD
    trig = {s16(word(orig, j)) for k in range(o, o + 0x400, 4) if word(orig, k) >> 26 == 0x2B
            and word(orig, k) & 0xFFFF == 0x7C for j in range(k - 32, k, 4)
            if word(orig, j) >> 21 == (0x23 << 5 | 28) and (word(orig, j) >> 16) & 31 == (word(orig, k) >> 16) & 31}
    assert len(trig) == 1, f"trigger_index: {trig}"
    climb_fns = [entry(col, AS_CLIMBLEFT), entry(col, AS_CLIMBRIGHT)]
    cl, cr = (calls(orig, x) for x in climb_fns)
    assert len(cl) == len(cr) == 3 and cl[1:] == cr[1:], "lara_col_climbleft/right"
    test_climb, climb_lr = cl[1:]
    climb_sites = []
    for x in climb_fns:
        end = next(k for k in range(x - LOAD, x - LOAD + 0x100, 4) if word(orig, k) == 0x03E00008)
        site = [k for k in range(x - LOAD, end, 4) if word(orig, k) == rv.jal(climb_lr)]
        assert len(site) == 1, "the call of LaraDoClimbLeftRight"
        climb_sites += site
    cams = [o for o in range(0x800, len(orig) - 0x50, 4)
            if word(orig, o) >> 16 == 0x8F82 and word(orig, o + 4) >> 16 == 0x8F83 and word(orig, o + 8) >> 16 == 0x8F84
            and word(orig, o + 12) >> 16 == 0x9385 and word(orig, o + 16) == 0x27BDFFC8
            and word(orig, o + 60) >> 16 == 0x10A0 and word(orig, o + 64) == 0x24020001
            and word(orig, o + 68) >> 16 == 0xAF82 and word(orig, o + 72) == 0x24020001 and word(orig, o + 76) >> 16 == 0xA782]
    assert len(cams) == 1, f"CalculateCamera found {len(cams)} times"
    camera, cam_type, cam_speed = cams[0] + LOAD, s16(word(orig, cams[0] + 68)), s16(word(orig, cams[0] + 76))
    on = entry(ctl, 40)
    assert [s16(word(orig, k + 4)) for k in range(on - LOAD, on - LOAD + 0x60, 4)
            if word(orig, k) == 0x24020006 and word(orig, k + 4) >> 16 == 0xA782] == [cam_speed], "camera.speed"
    look = calls(orig, as_hang)[0]
    assert [s16(word(orig, k + 4)) for k in range(look - LOAD, look - LOAD + 0x20, 4)
            if word(orig, k) == 0x24020002 and word(orig, k + 4) >> 16 == 0xAF82] == [cam_type], "camera.type"
    return dict(ctl=ctl, col=col, as_hang=as_hang, col_hang=col_hang, hang_test=hang_test, coll_info=coll_info,
                bounds=bounds.pop(), floor_front=floor_front, input=s16(inp), climb=s16(f["climb"]), anims=anims,
                move=moves.pop(), cam_angle=angle[0], cam_elev=elev[0], gun=gun, trigger=trig.pop(),
                test_climb=test_climb, climb_lr=climb_lr, climb_sites=climb_sites, camera=camera, cam_type=cam_type,
                cam_speed=cam_speed, sin=cf.sin_table(orig, floor_front), get_floor=f["get_floor"],
                get_height=f["get_height"])


def source(f, base):
    """CODE with the values of this version; calls of its own routines get their addresses (keystone makes
    'jal label' a call through gp)."""
    lab, data = labels(base)
    hi, lo = fx.hi_lo(data)
    rep = {"CLIMB": f["climb"], "INPUT": f["input"], "ANIMS": f["anims"], "MOVE": f["move"], "GUN": f["gun"],
           "CAM_ANGLE": f["cam_angle"], "CAM_ELEV": f["cam_elev"], "CAM_ELEVATION": CAM_ELEVATION,
           "NO_SIDE": ~(IN_LEFT | IN_RIGHT), "SIDE": IN_LEFT | IN_RIGHT, "ACTION": IN_ACTION, "LEFT": IN_LEFT,
           "RIGHT": IN_RIGHT, "HANG_BASE": 32 * HANG + 0x10, "HANG_FRAME": HANG_FRAME, "HANG": HANG,
           "FALL_BASE": 32 * FALL_START + 0x10, "FALL_START": FALL_START, "HANGRIGHT": AS_HANGRIGHT,
           "CORNER_ANIM": CORNER, "AS_CORNER": AS_CORNER, "AS_HANG": AS_HANG, "AS_FORWARDJUMP": AS_FORWARDJUMP,
           "CORNER_HI": hi, "CORNER_LO4": lo + 4, "CORNER_LO": lo,
           "AS_HANG_FN": hex(f["as_hang"]), "COL_HANG_FN": hex(f["col_hang"]), "BOUNDS_FN": hex(f["bounds"]),
           "FLOOR_FRONT_FN": hex(f["floor_front"]), "COLL_INFO_FN": hex(f["coll_info"]),
           "CLIMB_ANIM": CLIMB_CORNER, "STANCE_BASE": 32 * STANCE + 0x10, "STANCE": STANCE, "AS_STANCE": AS_STANCE,
           "LADDER_RIGHT": LADDER_RIGHT, "CLIMB_WIDTH": CLIMB_WIDTH, "CAM_TYPE": f["cam_type"],
           "CAM_SPEED": f["cam_speed"], "SIN_HI": p3.hi(f["sin"]), "SIN_LO": s16(f["sin"]), "SIN_LO2": s16(f["sin"] + 2),
           "TRIGGER": f["trigger"], "CAMERA_FN": hex(f["camera"]), "CLIMB_LR_FN": hex(f["climb_lr"]),
           "GET_FLOOR_FN": hex(f["get_floor"]), "GET_HEIGHT_FN": hex(f["get_height"]),
           "TEST_CLIMB_FN": hex(f["test_climb"]), "CLIMB_REACH": CLIMB_REACH + 1,
           "START_OUTER_LEFT": START_OUTER_LEFT, "RIGHT_EXTRA": RIGHT_EXTRA, "INNER_EXTRA": INNER_EXTRA}
    src = re.sub(r"\b[A-Z][A-Z0-9_]*\b", lambda m: str(rep[m.group(0)]) if m.group(0) in rep else m.group(0), CODE)
    return re.sub(r"\bjal   ([a-z_]+)$", lambda m: f"jal   {lab[m.group(1)]:#x}", src, flags=re.M)


def labels(base):
    """Addresses of all labels (every line of CODE that is not a label is one instruction) and of the data."""
    out, n = {}, 0
    for line in CODE.splitlines():
        line = line.strip()
        if line.endswith(":"):
            out[line[:-1]] = base + 4 * n
        elif line:
            n += 1
    return out, base + 4 * n


def block_room(orig, ver):
    """As make_ledgejump_ips.block_room: behind everything the patches before this one write into the block."""
    n = len(orig)
    extra = p3.extra(ver)
    pk = rv.apply_ips(orig, open(os.path.join(BIN, "patches", ver, "pickup3d.ips"), "rb").read())
    used = n + ((len(pk[n:n + extra].rstrip(b"\x00")) + 32 + 15) & ~15)
    folder = os.path.join(BIN, "patches", ver)
    before = set(patches_before(ver))
    for fname in sorted(os.listdir(folder)):
        stem = fname[:-4]
        if fname.endswith(".ips") and stem != "pickup3d" and (stem in before or stem.split("_")[0] in before):
            for s, e in p3.ips_ranges(open(os.path.join(folder, fname), "rb").read()):
                if s >= n:
                    used = max(used, (e + 15) & ~15)
    _, size, buffer = p3.level_memory(orig)
    return used, buffer + size - extra + used - n, n + extra


def build(us, orig, path, ver):
    f = sites(us, orig, path, ver)
    exe = patched(path, ver)
    for t in (f["ctl"], f["col"]):
        for s in (AS_HANG, AS_CORNER):
            assert word(exe, t + 4 * s) == word(orig, t + 4 * s), f"state table {s} is changed by another patch"
    for k in f["climb_sites"]:
        assert word(exe, k) == word(orig, k), "lara_col_climbleft/right are changed by another patch"
    for t in (f["ctl"], f["col"]):
        for s in (AS_CORNER, 63, 64):
            assert word(orig, (word(orig, t + 4 * s) | 0x80000000) - LOAD) == 0x03E00008, "LARATEST is not empty"
    off, addr, end = block_room(orig, ver)
    lab, data = labels(addr)
    lab = {k: lab[k] for k in ENTRIES}
    assert fx.hi_lo(data)[0] == fx.hi_lo(data + 4)[0]
    code = fx.asm(source(f, addr), addr)
    assert addr + len(code) == data and off + len(code) + DATA_SIZE <= end, \
        f"no room left in the appended block ({end - off} bytes, {len(code) + DATA_SIZE} needed)"
    words = list(struct.unpack(f"<{len(code) // 4}I", code)) + [0] * (DATA_SIZE // 4)
    patches = [(off, words),
               (f["ctl"] + 4 * AS_HANG, [lab["as_hang"] - 0x80000000]),
               (f["col"] + 4 * AS_HANG, [lab["col_hang"] - 0x80000000]),
               (f["ctl"] + 4 * AS_CORNER, [lab["as_corner"] - 0x80000000]),
               (f["col"] + 4 * AS_CORNER, [lab["col_corner"] - 0x80000000])]
    patches += [(k, [rv.jal(lab["climb_lr"])]) for k in f["climb_sites"]]
    return patches, f, lab, data, (
        f"routines {addr:#x} ({len(code)} bytes + {DATA_SIZE} data, {end - off - len(code) - DATA_SIZE} left), "
        + ", ".join(f"{k} {v:#x}" for k, v in lab.items())
        + f"; LaraHangTest {f['hang_test']:#x}, GetLaraCollisionInfo {f['coll_info']:#x}, "
          f"GetBoundsAccurate {f['bounds']:#x}, LaraFloorFront {f['floor_front']:#x}, input gp+{f['input']:#x}, "
          f"climb gp+{f['climb']:#x}, anims gp+{f['anims']:#x}, move_angle gp+{f['move']:#x}, "
          f"camera gp+{f['cam_angle']:#x}/{f['cam_elev']:#x}, gun gp+{f['gun']:#x}")


if __name__ == "__main__":
    ap = argparse.ArgumentParser()
    ap.add_argument("--versions", default="us,ukrev,derev,frrev,itrev,sprev,jp,jp2,as,as2")
    ap.add_argument("--exe", nargs="*", default=[], help="VERSION=PATH for executables not in bin/orig")
    a = ap.parse_args()
    paths = {v: os.path.join(BIN, "orig", f"{v}.exe") for v in a.versions.split(",") if v}
    paths.update(dict(e.split("=", 1) for e in a.exe))
    us = fx.Exe(lj.patched(os.path.join(BIN, "orig", "us.exe"), "us"))
    for ver, path in paths.items():
        p, _, _, _, desc = build(us, open(path, "rb").read(), path, ver)
        open(os.path.join(BIN, "patches", ver, f"{NAME}.ips"), "wb").write(rv.ips(p))
        print(f"{ver}: {desc}")
