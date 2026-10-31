"""Adds the controlled drop to a Tomb Raider III PC level: with Action held, Lara stepping off a high edge
turns round and catches it (TRX's controlled drop, driven by the executable, controlleddrop.ips).

The animations are TRX's "controlled drop" and its continuation (data/trx/ship/games/tr3/injections/
lara_animations.bin, its 315 and 316), poses unchanged (8 and 3, frame rate 1, 39 words each, 858 bytes per
level). They go in as animations 316 and 317, right behind the crouch turns (314, 315, tools/add_crouch_turn.py,
which has to run first), so they have the same numbers in every level; the animations behind them and every
reference to them move up by two. Their frames and commands are added at the end of their lists. State
AS_REACH (11) as in TRX, no state changes; 316 goes on with 317, 317 with Lara's reach (95, its first frame,
then the ledge is caught as after a jump). In 317 Lara turns round (effect 0) at its second frame and is
moved 128 down at its end. Frame numbers DROP_BASE to DROP_BASE + 10. A level that has them already is left
as it is.

Usage: add_controlled_drop.py <level.tr2>   (changes the file in place)"""
import struct
import sys

from add_crawl_flip import sections
from add_crouch_turn import TURN_BASE

INDEX = 316
DROP_BASE = 32200
POSES, FRAME_SIZE, FRAME_RATE = (8, 3), 39, 1
AS_REACH = 11
REACH = 95
COMMANDS = [6, DROP_BASE + 9, 0, 1, 0, 128, 0]                  # turn round, move
DATA = bytes.fromhex(
    "94ff670007fde9ff0bffda00fdff54fefcff303efe1f7009f7038743f00103004001fc2b6f3500fcbf436f3f03a86f01"
    "e993bf0de9ab00c0503a1a1c730fc92400c08002fe3300000000000000007fff690007fdf0ff2bffe500e8ff56fe1400"
    "203e07f42f0908c8af37f3fb000203049001f52bff3500fc5f3e00fc9f3ff3b30f04e2935f0cf1cb3f00fb9be03b190c"
    "200e2eb830000964c002fe274eff9d0009fdfeff6efffd00d4ff59fe2c00e13d10ccff08198cef36e5fb200204083002"
    "ee2b8f3600fc6f0200f0df3fe2bbbf06da87ff0af5db4f00f62f6f3d18fc900c1960400012cc0003ff1f2effbf000dfd"
    "e9ffc9ff2001bfff5bfe4300823d17a4cf082a501036d6033002040c0003e6271f3700fcff07ffd71f00d2c35f09cf6b"
    "9f09f7e75e00f1c7ef3e16ecf00a113c41001b345003ff13d9fec7001ffdfaffe4ff2601a7ff6cfe4800133d1b80af08"
    "2c2c7f35e2fbc002035c2002e837f037fc077f07fcbf4f00d1cbff0bb8234f08f8eb5e00eb5b6f0619ec60090d2c2100"
    "24989003ff0b85feaf003ffd1b00daff1c018fff8afe4d00943c1c608f082b20cf34f0ef500303ac0001e847d038f907"
    "2f05fbbf8f00cfd34d0e66ebef06f9f34d00e6f3e00d2618c0070b20f13f2dfcdf03ffff37fe9d0074fd310087ff2401"
    "76ffbbfe5200053c1b406f082b142f3400d4e00305fc903ee857b039f507a03afd07bf00ced7190e59c28f05f9f73d00"
    "e18b970a12c620060914b23f35641f04fff387fec6008efd61001eff4101a1ffd2fe3500863b1628cf0620200f35ffe7"
    "300100ac203ee563003af907603afd07ff00d4dfb80b13be2f04fafb0d00dc1f57090aa680040810523f3cc86f04ffeb"
    "01ffc90094fd6900e6fe7a01ccffd6fe1700173b0e143f05162cef35fff7803e0064b03de16b503afd03303afc072f01"
    "dae71809fe79cf02fafbdc3fd8b707080692f0020708e33e4328af04ffdf3eff640072fd4d0061fe1301f7ffd6fef9ff"
    "d03a0300af030c446c43d03b0514403ddd77ab43f039fc076f01e0ef7806f3616f01faff9c3fd44fb706028250010704"
    "733e488cef04ffd30000000076ff730079fdd4ff5efe1101f7ffd6fef9ffdf3a03fc8f3d09446c43d03b0514403ddd77"
    "aa43f039fc076f01fcef7806f3616f01faff9c3fd44fb706028250010704733e488cef04ffd300000000")


def add(d, s=None, stride=18):
    """s: offsets of the count fields (default: a PC level); stride: bytes per moveable (PSX: 20)."""
    s = s or sections(d)
    u = lambda f, o: struct.unpack_from(f, d, o)
    na, nc, nf, nm = (u("<I", s[k])[0] for k in ("anims", "commands", "frames", "moveables"))
    anim = lambda i: s["anims"] + 4 + 32 * i
    if na > INDEX and u("<H", anim(INDEX) + 16)[0] == DROP_BASE:
        return None
    movs = [u("<IHHIIH", s["moveables"] + 4 + stride * i) for i in range(nm)]
    second = min(m[5] for m in movs if m[0] != 0)
    assert second == INDEX and u("<H", anim(INDEX - 1) + 16)[0] == TURN_BASE + 25,         f"Lara has not {INDEX} animations ending with the crouch turns ({second})"
    assert len(DATA) == 2 * FRAME_SIZE * sum(POSES)
    d = bytearray(d)
    n = len(POSES)
    for i in range(na):                                          # next animations
        if u("<H", anim(i) + 20)[0] >= INDEX:
            struct.pack_into("<H", d, anim(i) + 20, u("<H", anim(i) + 20)[0] + n)
    for i in range(u("<I", s["dispatch"])[0]):                   # state change targets
        o = s["dispatch"] + 4 + 8 * i + 4
        if u("<H", o)[0] >= INDEX:
            struct.pack_into("<H", d, o, u("<H", o)[0] + n)
    for i in range(nm):                                          # first animations of the moveables
        o = s["moveables"] + 4 + stride * i + 16
        if u("<H", o)[0] >= INDEX and u("<H", o)[0] != 0xFFFF:
            struct.pack_into("<H", d, o, u("<H", o)[0] + n)
    reach_base = u("<H", anim(REACH) + 16)[0]
    assert u("<H", anim(REACH) + 6)[0] == AS_REACH, "animation 95 is not the reach"
    first, second = DROP_BASE, DROP_BASE + POSES[0]
    new = (struct.pack("<IBBHiiHHHHHHHH", 2 * nf, FRAME_RATE, FRAME_SIZE, AS_REACH, 0, 0, first,
                       second - 1, INDEX + 1, second, 0, 0, 0, nc)
           + struct.pack("<IBBHiiHHHHHHHH", 2 * nf + 2 * FRAME_SIZE * POSES[0], FRAME_RATE, FRAME_SIZE, AS_REACH,
                         0, 0, second, second + POSES[1] - 1, REACH, reach_base, 0, 0, 2, nc))
    cmds = struct.pack(f"<{len(COMMANDS)}h", *COMMANDS)
    a_end = anim(na)
    c_end = s["commands"] + 4 + 2 * nc
    f_end = s["frames"] + 4 + 2 * nf
    out = (d[:s["anims"]] + struct.pack("<I", na + n) + d[s["anims"] + 4:anim(INDEX)] + new + d[anim(INDEX):a_end]
           + d[a_end:s["commands"]] + struct.pack("<I", nc + len(COMMANDS)) + d[s["commands"] + 4:c_end] + cmds
           + d[c_end:s["frames"]] + struct.pack("<I", nf + len(DATA) // 2) + d[s["frames"] + 4:f_end] + DATA
           + d[f_end:])
    return bytes(out)


if __name__ == "__main__":
    path = sys.argv[1]
    out = add(open(path, "rb").read())
    if out is None:
        print(f"{path}: has the controlled drop already")
    else:
        open(path, "wb").write(out)
        print(f"{path}: controlled drop added as animations {INDEX} and {INDEX + 1}")
