"""Adds the turns on the spot in the crouch to a Tomb Raider III PC level (Left/Right while crouched,
driven by the executable, crouchturn.ips).

The animations are a "crouch turn left" and a "crouch turn right", with their single axis rotations
turned from 12 to 10 bits (Tomb Raider III's precision); frame rate 2 as there (25 frames, 13 stored, 1 KB each per
level). They go in as animations 314 and 315, right behind the crawl flip (313, tools/add_crawl_flip.py,
which has to run first), so they have the same numbers in every level; the animations behind them and
every reference to them move up by two. Their frames and commands are added at the end of their lists.
State AS_DUCK (71), each loops into itself and has no state changes (the executable puts Lara back into
the crouch animation, 222, whenever the turn ends); the shuffle sound of the crawl turns (24, on land
only) at frame 14 (left) and 15 (right). Frame numbers TURN_BASE to
TURN_BASE + 49. A level that has them already is left as it is.

Usage: add_crouch_turn.py <level.tr2>   (changes the file in place)"""
import struct
import sys

from add_crawl_flip import FRAME_BASE as FLIP_BASE, sections

INDEX = 314
TURN_BASE = 32100
FRAMES, FRAME_SIZE, FRAME_RATE = 25, 38, 2
AS_DUCK = 71
SOUND = 0x4000 | 24
TURNS = [(14, bytes.fromhex(
    "80ffa100a5fe0d006fffd600000071ffdbff3e40b00355b4d73afffd5f01f223fd08dd2e383b00024e071af82533aa56"
    "8d000a33a3406f3d2ec07303fd4c5f40d03e9de0ef0af93f230000007dffa300a2fe0a0072ffda00000070ffdbfff003"
    "030010045084b73afffd9e010bec8d08e046383b00028e0717f42533b13a5c0129af89406f3d2ec03203e7f85f403002"
    "40e8ae09c947230078ffa6009cfe080074ffdd00ffff6dffdbfff003060060044a60873afffdee0122bc2d08e466283b"
    "0002ce0716f41533b8225c0242435f0700fc6f3d2ec0d202d3b05f40f004e27f3d08afab6effaf0091fe070077ffdd00"
    "ffff65ffdbffbf0303fca004476ce739fffdde0127846d08f3c2b83a00023f081e209532c7065c024c5f88406f3d2ec0"
    "8202c87c6040c005b52bad07a88f23005effbe007efe090074ffd900feff56ffdbff1f03f9f7f00445b0d738fffd5e01"
    "1c403e09137bd8390002af082f888431edae2c0146ffc7406f3d2ec05202c65c6440b005c7434d08aad7230058ffc600"
    "74fe0d0071ffd600feff4effdbffaf02f2ef00054adc2738fffdbe00111c4f09321358390002ff0849f8d3303ca34d00"
    "3987900f01046f3d2ec06202c85c5f405005ea7bce08b63368ffc2007ffe0b0061ffd900ffff59ffdbff8f02f2efa004"
    "5fac8738fffd2e000b3cff073903d83900022009798013314a671d002f9fb00f02046f3d2ec0b202d2944340c0040aa0"
    "9e08c97b64ffb90090fe070048ffdf00000069ffdbff9f02f3ef500478604739fffdce3f0574ae0639b3b83a0002e008"
    "a5ec73312dbf3d002587ec406f3d2ec01203dfec1f40f0032ecc2e08deb723005bffb1009bfe090043ffe200ffff6fff"
    "daffaf02efeb2004825cd739fffdbe3f00904e07368be83a0002d00894b4443109375d001e67eb406f3d2ec05303eb38"
    "0d4010034fe44e08e9db230064ffa6009ffe07004cffe400f9ff6affd9ffcf02e6dff003819c373afffdfe3ffc970e0a"
    "2a73883a00024f0848ecb6306e8a7d00192ff7406f3d2ec06303f47c0740100270f4ee08ecef230071ff9e00a3fe0400"
    "57ffe400f6ff67ffd8ffff02e2dbd00379d0773afffd5e00f89bfe0b0b0b383a00023f07136027310df69d00140bf740"
    "6f3d2ec07303fba40f40100189f49f09ee03230078ff9d00a5fe03005fffdf00f8ff69ffd8ff3f03eae3c0036dd0a73a"
    "fffdae00f49b6d0bed8a783a0002ff060c2c96315caa9d00100fdc406f3d2ec07303fe902940200095ec1f0af2172300"
    "77ff9f00a7fe040067ffda00fdff6effdaff8f03f7f7c0035dbcc73afffdfe00f09fbd09e14af83a00023f07160c9532"
    "9d968d000c27b5406f3d2ec07303fd604d40403f9ae4af0af72f2300")),
         (15, bytes.fromhex(
    "80ffa100a5fe0d006fffd600000071ffdbff3e40b00355b4d73afffd5f01f223fd08dd2e383b00024e071af82533aa56"
    "8d000a33a3406f3d2ec07303fd4c1740d03e9de0ef0af93f2300000079ff92009cfe0f0065ffd900000066ffdbffef03"
    "fdfbf0035d8c473afffd8f00f10fbd09036fa83a0002cf072b601632733e8c0006c32f0700fc6f3d2ec08303e6902940"
    "d03e40e0200a26506fff890093fe11005dffdf0000005cffdbffdf03fbf34004656ca739fffdaf3ff0034d0a2ab3283a"
    "00021f083cc0973109e69c00045f4f0400fc6f3d2ec0d303d0d03940d03ee9e30109412069ff7d008afe120051ffe200"
    "000056ffdaff7f03fbf3900467442739fffd9e3ef0f32d0b339bc8390002ff073ad41931b4292c010e373f0300fc6f3d"
    "2ec03305c4e44840d03ec4e3d1074c7462ff6d0081fe110040ffe500ffff53ffdaffbf02fcf7f004660c9738fffd2e3d"
    "efeb3c0c0edf983900027f0724986c30bdd07c022557df0300fc6f3d2ec0c307c2d05640d03ecfe3c106517867ff6600"
    "7dfe110038ffe500feff56ffdaff1f02fefb40055d007738fffd5e3cefe38b0ccdcea8390002cf0612602f31f8c7cc02"
    "3873af0400fc6f3d2ec08309b7905d40d03ee5e33106515488ff660082fe120045ffe100feff62ffdaffff01fffbc005"
    "454c3739fffdbe3cefe7cb0b980e583a00020f060a44c031c68bec0037434f0500fc6f3d2ec03309930c5a40d03ef9e3"
    "400646fc8cff69008bfe15005fffdb00feff72ffdaff0f0200f870062bb4673afffdbe3df0ebba0a7fd2283b00025f05"
    "07304132a51f5c3e3003bf0500fc6f3d2ec0c2077ab45140d03e12e0a006348888ff70008efe19006fffda00fdff77ff"
    "dbff3f02faf7a00625e0f73afffdbe3ef0f3bb0a8a2e583b00026f050b2851329c5f8b3d23eb3f0500fc6f3d2ec01207"
    "85c84740d03e2ae0c00626408fff81008bfe1b0078ffdd00faff71ffddff7f02eceb400633dc173bfffdfe3ff0f75c0b"
    "c352e83a00024f061830e131a1777b3e0bef9f0300fc6f3d2ec0e306af383d40d03e44e0d0061a1893ff90008cfe1c00"
    "7dffe000f8ff6dffdfffbf02e3e3a00542cc073bfffdfe00f1ff1d0bf652983a00020f072338523181138c3ffa0a0f03"
    "00fc6f3d2ec03306d58c3240d03e5de04f0710f48dff950096fe170078ffe000faff6dffdeffff02e9ebf0044ac4073b"
    "fffd4f01f10b4d0af466b83a00023f072224733107f31c00fb6e3f0500fc6f3d2ec02305e8882840d03e77e08f080abc"
    "85ff9c00a8fe0e006dffe000feff70ffdcff3f03f6f7200451b8e73afffd5f01f21b6d09e33a083b00023f071c086532"
    "b4266c0006f38a406f3d2ec00304f6601d40d03e90e02f0a016c2300"))]


def add(d, s=None, stride=18):
    """s: offsets of the count fields (default: a PC level); stride: bytes per moveable (PSX: 20)."""
    s = s or sections(d)
    u = lambda f, o: struct.unpack_from(f, d, o)
    na, nc, nf, nm = (u("<I", s[k])[0] for k in ("anims", "commands", "frames", "moveables"))
    anim = lambda i: s["anims"] + 4 + 32 * i
    if na > INDEX and u("<H", anim(INDEX) + 16)[0] == TURN_BASE:
        return None
    movs = [u("<IHHIIH", s["moveables"] + 4 + stride * i) for i in range(nm)]
    second = min(m[5] for m in movs if m[0] != 0)
    assert second == INDEX and u("<H", anim(INDEX - 1) + 16)[0] == FLIP_BASE,         f"Lara has not {INDEX} animations ending with the crawl flip ({second})"
    d = bytearray(d)
    n = len(TURNS)
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
    new, cmds, data = b"", [], b""
    for k, (sound_frame, frames) in enumerate(TURNS):
        base = TURN_BASE + FRAMES * k
        new += struct.pack("<IBBHiiHHHHHHHH", 2 * nf + len(data), FRAME_RATE, FRAME_SIZE, AS_DUCK, 0, 0, base,
                           base + FRAMES - 1, INDEX + k, base, 0, 0, 1, nc + len(cmds))
        cmds += [5, base + sound_frame, SOUND]
        data += frames
    cmds = struct.pack(f"<{len(cmds)}H", *cmds)
    a_end = anim(na)
    c_end = s["commands"] + 4 + 2 * nc
    f_end = s["frames"] + 4 + 2 * nf
    out = (d[:s["anims"]] + struct.pack("<I", na + n) + d[s["anims"] + 4:anim(INDEX)] + new + d[anim(INDEX):a_end]
           + d[a_end:s["commands"]] + struct.pack("<I", nc + len(cmds) // 2) + d[s["commands"] + 4:c_end] + cmds
           + d[c_end:s["frames"]] + struct.pack("<I", nf + len(data) // 2) + d[s["frames"] + 4:f_end] + data
           + d[f_end:])
    return bytes(out)


if __name__ == "__main__":
    path = sys.argv[1]
    out = add(open(path, "rb").read())
    if out is None:
        print(f"{path}: has the crouch turns already")
    else:
        open(path, "wb").write(out)
        print(f"{path}: crouch turns added as animations {INDEX} and {INDEX + 1}")
