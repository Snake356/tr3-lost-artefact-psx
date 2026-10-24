"""Adds the forward crawl exit to a Tomb Raider III PC level: Lara flips out of a crawlspace head first
and falls (Walk + Jump at the edge, started by the executable, crawlflip.ips).

The animation is a "roll jump down from crawl", with its single axis rotations turned from 12 to 10 bits
(Tomb Raider III's precision) and only every second frame and the last kept (frame rate 2, as most of Lara's animations
in Tomb Raider III: the game interpolates; 23 instead of 44 frames, 1.7 instead of 3.3 KB per level). It goes in as animation 313, right behind Lara's own 313, so it has the same number in every
level; the animations behind it and every reference to them (next animations, state change targets,
the moveables' first animations) move up by one. Its frames and commands are added at the end of their
lists. State AS_NULL (19), as Tomb Raider III's own climb ups; at the end Lara is moved 620 forward and
815 down, falls (jump velocity 0, 0) and goes on with animation 23 (falling); Lara's short effort sound of pushing a block (46)
at frame 28. Frame numbers FRAME_BASE to FRAME_BASE + 43. A level that has it already is left as it is.

Usage: add_crawl_flip.py <level.tr2>   (changes the file in place)"""
import struct
import sys

INDEX = 313
FRAME_BASE = 32000
FRAMES, FRAME_SIZE, FRAME_RATE = 44, 38, 2
AS_NULL = 19
COMMANDS = [1, 0, 815, 620, 2, 0, 0, 5, FRAME_BASE + 28, 46]   # move, jump velocity, sound
DATA = bytes.fromhex(
    "6cff9400c2fe0b00d7fe1901000050ffdbff2643ef0d46b0e73300fe40020f04e00db94fe83300024f02f0fbf443ac02"
    "3a87e001fb038f3ffd96a302c578e001fd03803f0269aa40230000006dff9300d5fe0e00dafe410100004fffedff2643"
    "2f0d325cd73200fe40020f04200dcda3e83200024f02f0fbeb43ac024087e001fb038f3fd896a302be78e001fd03803f"
    "25697540230000006cff9200e7fe1000dbfe590100004efffcff26436f0c2830d73100fe40020f04600cd7cf08320002"
    "4f02f0fbe343ac024687e001fb038f3fb696a302b778e001fd03803f46694440230000006bff9100edfe1300dffe6301"
    "00004fff070026430f0c2420473100fe40020f04000cdbdf783100024f02f0fbdf43ac024b87e001fe038f3fa496a302"
    "b378e001fa03803f58692d40230000006aff9200ecfe1600e0fe6501000050ff09002643ff0b231c173100fe40020f04"
    "f00bdce3483100024f02f0fbe143ac024d87e00108008f3fa096a302b478e001f003803f5b693340230000006bff9100"
    "edfe1400e0fe6101000051ff050026432f0c2528873100fe40020f04200cdad7b83100024f02f0fbe043ac025687e001"
    "fb038f3fab96a302ad78e001fd03803f51693440230000006cff9400f3fe0800cdfe4501000050ffeaff26436f0d3974"
    "473300fef03f0e00600dc68b783300021f00f1ffda432c02736fe00172038f3ffd9623028d94e0019200803ff8682540"
    "230000006eff920004ff1400b9fefd00000059ffa6ff2743840e95ed473500fe603d0e00ab0e743a78350002af3df1ff"
    "d3438c013c4be00148038f3f5e977301c4b0e001b900803fa96810402300000074ff8c001eff0d00b1fede00000066ff"
    "8aff2843150dced5c73600fe403b0e001a0d322ef8360002af3bf1ffca43dc002f33e0011c038f3f9297c300d2c8e001"
    "e600803f6a68fa432300000076ff8a0034ff0c00aafed40000006aff84ff2743d50cd1e9073700fe403a0d04ca0c2d12"
    "38370002bf3af1fbc0433c002f1be001f4028f3faf972300d2e0e0010f01803f4268e9432300000071ff8c002fff1100"
    "b6fef30000005cffa7ff2743a50eae61973500fef03b0e107a0e5dcaa83500027f3cf0efb3431c004517e001e6028f3f"
    "88971300b5e4e0011d01803f6d68de432300000071ff8b001cff1500c8fe1c01000049ffd7ff24437f0e2950473300fe"
    "403f1020600eb00728330002bf3feedfa4432c00631be001e2028f3f769723009be0e0011d01803f8f68d84323000000"
    "75ff890003ff0e00d7fe440100002fff0f001c43ff0b16285030ff03d0001128100cecdb5030ff035f01ecd795433c00"
    "6d1be00119038f3ffc9633008cdce001ec00803ff568cf43230000005dff7b00d7fe1f00bafe4c01000007ff37000730"
    "fffd600938648f39ffffe03a0c08200a0af87a431f3cf1f787433c004d1be00124038f3fe6963300addce001e100803f"
    "0e69bc432300000082ff8300b2fe1c00f1fe56010000e6fe7d001732fffdd00844f45f3effff20380a00210a1700d643"
    "8f39f3ff6c432c00241be0013b038f3fcb963300d7e0ef01c9fc803f2d69ab432300000099ff950097fe01004eff5201"
    "0000cefee1005734fffd110a523ccf3fffff70370904610b1d18f043df38f5fb47433c00ea1ae00160038f3fb1963300"
    "12e1ef01a3fc803f49699a432300000099ff93006bfe050061ff62010000a7fe35014738fffd510b647c4f3fffff0037"
    "070cb10c233cbc436f38f7f73443fc007316e00184037f3fff9633008ce1ef017dfc703ffd6c8e432300000082ff7600"
    "e8fd0d00c5ffb6010000c1fe8901073dfffd110b5e5ccf3bffffd0360514310d295871433f38f9f33f432c01300aef01"
    "9eff9f3f3c9b1300cce1f0016300903fc364af3800fc23006cff680078fd08007e001402000016ffe001470200fe1009"
    "43dc7f33ffffc0360320d10c2440c032ff031f38fbeb6243ec00250eff01b3ffcf3f65abf33fcee500024e00c03f9d54"
    "df3800fc85ff64006ffed1ff6400450300007aff130238080002100738a0973200fed036012c600811ec9030ff031f38"
    "fde379432b023bf6ff01c5ffff3f8cbfc300d1e510023b04e03f7b403f3900f89bff9600c0fed800b20052030100eeff"
    "2302d80e020a40053384573600fed036ff3700000bc45031ff030f38ffdb85430b0574f60f02d7ff0f00b0d37302d5e5"
    "30022a0400005a28af3900fc6effb5000fff48026c011403feff9c004102a009010410082b48773800fed036fd43e03c"
    "0b885d43ff3702d07f3bfffbad03a1360f02e8ff1f00d4eb3203aef55002190400003a14003afe0b45ffc6007ffff002"
    "ae012703fbff1401580260050204ff0b11d09738fffdc036fc47303e0860a243ef3703cc5f3efef3ae0097460f02f1ff"
    "0f00e6f7120289016002110400002908403afc1b")


def sections(d):
    """Offsets of the count fields of the animation lists (TR3 PC level)."""
    u = lambda f, o: struct.unpack_from(f, d, o)
    o = 4 + 768 + 1024
    n = u("<I", o)[0]
    o += 4 + n * 65536 + n * 131072 + 4
    rooms = u("<H", o)[0]
    o += 2
    for _ in range(rooms):
        o += 16
        o += 4 + 2 * u("<I", o)[0]
        o += 2 + 32 * u("<H", o)[0]
        zs, xs = u("<HH", o)
        o += 4 + 8 * zs * xs + 4
        o += 2 + 24 * u("<H", o)[0]
        o += 2 + 20 * u("<H", o)[0]
        o += 7
    s = {}
    for name, size in (("floor", 2), ("mesh", 2), ("meshptr", 4), ("anims", 32), ("states", 6),
                       ("dispatch", 8), ("commands", 2), ("trees", 4), ("frames", 2), ("moveables", 18)):
        s[name] = o
        o += 4 + size * u("<I", o)[0]
    return s


def add(d, s=None, stride=18):
    """s: offsets of the count fields (default: a PC level); stride: bytes per moveable (PSX: 20)."""
    s = s or sections(d)
    u = lambda f, o: struct.unpack_from(f, d, o)
    na, nd, nc, nf, nm = (u("<I", s[k])[0] for k in ("anims", "dispatch", "commands", "frames", "moveables"))
    anim = lambda i: s["anims"] + 4 + 32 * i
    if na > INDEX and u("<H", anim(INDEX) + 16)[0] == FRAME_BASE:
        return None
    movs = [u("<IHHIIH", s["moveables"] + 4 + stride * i) for i in range(nm)]
    lara = [m for m in movs if m[0] == 0]
    second = min(m[5] for m in movs if m[0] != 0)
    assert lara and lara[0][5] == 0 and second == INDEX, f"Lara has not {INDEX} animations ({second})"
    d = bytearray(d)
    for i in range(na):                                          # next animations
        if u("<H", anim(i) + 20)[0] >= INDEX:
            struct.pack_into("<H", d, anim(i) + 20, u("<H", anim(i) + 20)[0] + 1)
    for i in range(nd):                                          # state change targets
        o = s["dispatch"] + 4 + 8 * i + 4
        if u("<H", o)[0] >= INDEX:
            struct.pack_into("<H", d, o, u("<H", o)[0] + 1)
    for i in range(nm):                                          # first animations of the moveables
        o = s["moveables"] + 4 + stride * i + 16
        if u("<H", o)[0] >= INDEX and u("<H", o)[0] != 0xFFFF:
            struct.pack_into("<H", d, o, u("<H", o)[0] + 1)
    fall_base = u("<H", anim(23) + 16)[0]
    new = struct.pack("<IBBHiiHHHHHHHH", 2 * nf, FRAME_RATE, FRAME_SIZE, AS_NULL, 0, 0, FRAME_BASE,
                      FRAME_BASE + FRAMES - 1, 23, fall_base, 0, 0, 3, nc)
    cmds = struct.pack(f"<{len(COMMANDS)}h", *COMMANDS)
    a_end = anim(na)
    c_end = s["commands"] + 4 + 2 * nc
    f_end = s["frames"] + 4 + 2 * nf
    out = (d[:s["anims"]] + struct.pack("<I", na + 1) + d[s["anims"] + 4:anim(INDEX)] + new + d[anim(INDEX):a_end]
           + d[a_end:s["commands"]] + struct.pack("<I", nc + len(COMMANDS)) + d[s["commands"] + 4:c_end] + cmds
           + d[c_end:s["frames"]] + struct.pack("<I", nf + len(DATA) // 2) + d[s["frames"] + 4:f_end] + DATA
           + d[f_end:])
    return bytes(out)


if __name__ == "__main__":
    path = sys.argv[1]
    out = add(open(path, "rb").read())
    if out is None:
        print(f"{path}: has the crawl flip already")
    else:
        open(path, "wb").write(out)
        print(f"{path}: crawl flip added as animation {INDEX}")
