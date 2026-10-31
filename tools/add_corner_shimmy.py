"""Adds the corner shimmying to a Tomb Raider III PC level: hanging from a ledge, Left or Right with Action held
where the ledge does not go on takes Lara round an outer or an inner corner (TRX's corner shimmying, driven by
the executable, cornershimmy.ips).

The animations are TRX's "hang round a corner" ones (data/trx/ship/games/tr3/injections/lara_animations.bin,
its 342 to 349: left outer, right outer, left inner, right inner, each a turn and its one-pose end), poses
unchanged (22, 1, 22, 1, 7, 1, 7, 1 with the pose behind the last frame, frame rate 4 and 1, 38 words for the
first, 37 for the others, 4632 bytes per level). They go in as animations 322 to 329, right behind the ledge
jumps (318 to 321, tools/add_ledge_jumps.py, which has to run first), so they have the same numbers in every
level; the animations behind them and every reference to them move up by eight. Their frames and commands
are added at the end of their lists. State AS_LARATEST1 (62, unused by the game), no state changes: the
executable switches to them; each turn goes on with its end, the end with the hang (96, its frame 21), and the
executable moves Lara to the corner when the end is reached. The sounds (1 for the outer, 28 for the inner
corners) as in TRX. Frame numbers CORNER_BASE to CORNER_BASE + 219. A level that has them already is left as
it is.

Usage: add_corner_shimmy.py <level.tr2>   (changes the file in place)"""
import struct
import sys

from add_crawl_flip import sections
from add_ledge_jumps import LEDGE_BASE

INDEX = 322
CORNER_BASE = 32292
POSES = (22, 1, 22, 1, 7, 1, 7, 1)
FRAMES = (84, 1, 84, 1, 24, 1, 24, 1)
RATES = (4, 1, 4, 1, 4, 1, 4, 1)
SIZES = (38, 37, 37, 37, 37, 37, 37, 37)
HANG, HANG_FRAME = 96, 21
AS_HANG, AS_CORNER = 10, 62
BASES = [CORNER_BASE + sum(FRAMES[:k]) for k in range(len(FRAMES))]
COMMANDS = [[5, BASES[0] + 53, 0x4001], [5, BASES[2] + 52, 0x4001],              # sounds
            [5, BASES[4] + 20, 0x401C], [5, BASES[6] + 20, 0x401C]]
DATA = bytes.fromhex(
    "88ff70002afdc100caff8700f8ffd6fe2d00203e03007f02ed5bf543503c05145002fd33ef43d03afd032f03fceb3804"
    "f5695f01faff9c3f374c6704017e30010700733ecb8f1f0500d400006bff4d0031fdcf00cbff7200d5ffe0fe2d002f3e"
    "12fcbf02d3abcc43303c04102003de7bf3435f39fafb2003f00ba803ee2d3f013ffc8c001d741704f48d1440533ff8a7"
    "0005e21b0000000043ff11002ffdd600cdff6f00aeffe0fe2d002f3e21fccf02cccbc643203c04107003dc97f443bf38"
    "f9f72003ef1bb803241a3f0129fc9c000280c703f6615001f703a33ff1b32005c51f00002dfff9ff3ffdd300ccff8e00"
    "95ffe1fe2d002f3e13f07f03f0c3de43003c040cf002ff93f243cf38f9f720030014b80336263f0104fc1c00f57f7903"
    "58da5001bf03833fbeaf3f05aadf000018fff1ff41fdd400d1ffe2008affe9fe3100303ef5378f041be8d643f03b040c"
    "b0021c90e843cf38f9f720030e1427040bf63f0110fc2c00f77ffa02dbbe50018f03b33fbec39f049de70000e0fedcff"
    "44fdd300daff130178ffeefe3900a03ee4bff0043830ad43d03b030cb0023098d643cf38f9f720031714f704ed8d3f01"
    "03fc2c00f77f3a0554a350018603f33fc4cf60039c370000c8fedfff46fdd200dfff02017cfff1fe3d00e03ed9fb2005"
    "4e5cab43b03b03080003306cce43cf38f9f720031a140705de5d3f01fdff2c00f77f5a05a0af5001b1032300cddbf001"
    "a4430000d5feffff44fdd300dfff040194fff3fe3c00d03ed4f320055c68ca43a03b030880031c08cf43cf38f9f72003"
    "16149704e0693f01feff2c00f77f3a05d5a75001e9036300d8e36f00b8ff000004ff240044fdd400e1ff0901b4fff3fe"
    "3c00d03ed3e7bf0458f4ee43803b03043f0316d0d043cf38f9f730030e1c5704e9713f01ffff2c00f77f0a05fb975001"
    "fc039300e4ef1f3fc0bf000031ff430040fdd400d5ffb800dcffeffe3c00c03ed8db9f03480cfc43603b03044f0220d0"
    "d043cf38f9f7400303283704fa713f01ffff2c00f77f9a040a7c5001e803c400f103df3dbd930000fcfe33003efdda00"
    "ddffbd00d0fff1fe3b00d13ee617ae033ab0c443503b03048f0213ccd043cf38f9f7ef02efd7c703f6854f01ffff2c00"
    "f67f7b07f5275001aa03e400fe1f0f3fc98f0000f3fe040040fde300c4ff22018ffff9fe3a00b13efda74e052ad01843"
    "303b0200cf03eec3d043cf38f9f7df01d733d702db9d5f01ffff2c00f57f2c0edabf50014903f4000c44af02e5cf0000"
    "b1feccff42fded00d7ff180164fffdfe4a00b23e087cdf0741044030ff03203b0200ef02e0cbd043cf38f9f78f00ca27"
    "263fc5652f00ffff2c00fd7fc906939e2f0163fbe4001878bf05f1cfd8febdff46fddc000000520142ffeafe9300b33e"
    "1c886f061a345b43003b02000f02c1cfcf43bf38f9f7cf3fdf2f553be6352f3efcff2c005b7c2a00692e1f01bbf3c400"
    "24c0ff06b5a70000d3fe97ff32fdd800dfff54013affe0feeb00643e360ccf03c78b8c43ef3a02fc5f02abfbd2430f39"
    "f9f7ff00ddb3c73da4315f3f07f80c00f47f0a014d76e002f0ab25002d7cb006d6330000e1fe90ff33fddf005e006101"
    "44ffe5fef700543e3200bf02c6c7ba43df3a02fc9002d23bd6436f39faf39002d35b08017a958001a500dc3ff47fdb00"
    "2f36c002f4a7773e1e78e00511940000f3fe97ff30fdd4006500510147ffe1fee600443e1f006f01eadb0340df3a02fc"
    "a002f74fdb43ef39faf37002ec3f3803d3114f013df89c3ff9831b3f339aa002f797b73e0fc4c005ef870000f7fea2ff"
    "2ffdca00510043014affddfed100443e0a00ef02f3bbe743df3a02fca002de5fe0435f3afaf34f0200d46803037a3f01"
    "19fc6c3f0280cb3e40ae6002fa7f473f0444a005cd6f0000fbfea1ff2efdcb000f003a014bffdcfec100443ef503df02"
    "0facd943df3a02fca002e363e543df3afaef4f0210b4e80201722f010dfc2c3f0e7c0a01408e2002fc63d63ff78b9005"
    "d46b0000cefea6ff2dfdcd00f8ff34014bffdcfeb200443ee1030f02319cd243df3a02fca002f463ea434f3bfaef4f02"
    "1cb8a802fd693f0106fcfc3e1b78880322eae001fe431500e8a79005f36b0000cefeacff2bfdcb00170032014effdafe"
    "b000343ee003df0131a4d743df3a02fca0020374ee43cf3bfaef8f0219d40803fa513f0101f8bc3e2774570408de9001"
    "01241400d9bb700501580000f6feb0ff2cfdc90051002e0154ffd8feb900333ef1ff1f0215bce643df3a02fc80020c90"
    "f2431f3cfaefef020cfce803f92d3f01fcfb9c3e2f74a70405b27001030cd43fce173005003c0000f4feb2ff2dfdc200"
    "55002e0158ffd6fec100233efcff5f0202ccef43df3a02fc700212a4f5435f3cfbef200303148804f8193f01f8fb7c3e"
    "3470d704039650010500933fc8b31005ff2f88ff70002afdc100caff8700f8ffd6fe2d00203e03007f02ed5bf543503c"
    "05145002fd33ef43d03afd032f03fceb3804f5695f01faff9c3f374c6704017e30010700733ecb8f1f0500d4b1ff9100"
    "34fdce00cbff76002700e0fe2d00203eef034f031b64f443c039050ca0022634ca43d03afd032f030de80804096a5f01"
    "00fc9c3f084cb7030fce3001c1034300e37f0f051dccf0ffbe0031fdd900ccff6e005300e0fe2d00203ede036f03246c"
    "f44390380608c0023438c643d03afd032f0310e8c8030aa25f0109fc8c3f0e50b703dbe53001d603a300fd832f053ae8"
    "0700d3003efdd500cbff8f006b00e1fe2d00203eec0fff02006cf243c038060870030f3cdf43d03afd032f03ffeb7603"
    "a7255f0140fc8c3f4150b703c9d93001fb0313000a80300555240f00e7003ffdd600cfffe1007600e9fe31003f3e0ac8"
    "bf02e36fe843c03806088004e41bd643d03afd032f03f1ebf50224415f0170fcbc3f413c2804f4093001ef0323000880"
    "9004621824001f0142fdd500d7ff12018800eefe3900af3e1b40bf02cf67d643c0380608ff04c7cfad43d03afd032f03"
    "e8eb3505ab5c5f0179fcfc3f3b30f80412723001fc03230008806f0363c82100360143fdd400ddff01018400f1fe3d00"
    "ef3e26040f03cf93ce43c03806082f05b1a3ab43d03afd032f03e5eb55055f505f014efc2c003224080521a230010200"
    "23000880ff015bbcffff280142fdd500ddffff006c00f3fe3c00df3e2b0c8f03e3f7cf43c03806082f05a397ca43d03a"
    "fd032f03e9eb35052a585f0116fc6c00271c98041f96300101002300088060004800dafff90042fdd600e0ff05014c00"
    "f3fe3c00df3e2c183003e92fd043c0380608b004a70bee43d03afd033f03f1e3050504685f0103fc9c001b105804168e"
    "134023000880103f3f400000bdffcc003ffdd500d3ffb4002400effe3c00cf3e27244002df2fd043c03806089003b7f3"
    "fc43d03afd034f03fcd79504f5835f0117fccb000efc3804058e134023000880d03d426c0000ccff03013dfddc00dbff"
    "bc003000f1fe3b00de3e19e88002ec33d043c0380608a103c54fc443d03afd03e002102874070ad85f0155fceb0001e0"
    "c803097a144023000980003f36700000faff0b013efde500c6ff21017100f9fe3a00be3e0258c003113cd043c0380608"
    "4105d52f8f3100fcd03afd03d00128cc230e25405f01b6fcfb00f4bbd8022462154023000a80a0021a3036004d0140fd"
    "ee00d5ff16019c00fdfe4a00bd3ef783e0021f34d043c0380608d007befb4f3000fcd03afd03800035d8c6066c612001"
    "9c04eb00e787293f3a9a024023000280b0050e304400250144fddc00feff5101be00eafe9300bc3ee37700023e30cf43"
    "b03806086006e5cb5b43d03afd03c03f20d0250096d11001440ccb00db3f5a3b19ca203e03002300a483f0064a586700"
    "2d0135fdd800dfff5501c600e0feeb006b3ec9f350025404d24300390608c00338748c43d03afd03f000224c0501b289"
    "ef020f542a00d283c83d5bce503ff80703000b80bf0629cc6e001d0136fde1005c006201bc00e5fef7005b3ecdff9f02"
    "2dc4d6436039050cb0023938ba43d03afd039f022ca4d400d0c9cf020b58783ee1870701856a8f015affd33f0b80ef05"
    "ef6b67000e0133fdd50064005301b900e1fee6004b3ee0ffaf0208b0db43e039050c600115240340d03afd037f0213c0"
    "143fcc65af020868b83ef03b37032cee4001c307933f067ccf0510785c00090132fdcb0050004401b600ddfed1004b3e"
    "f5ffaf0221a0e043503a050ce0020c44e743d03afd034002ff2bc43ebf516f020580483ffbbb6703fc853001e607633f"
    "fd7faf0532905e0005012efdca000e003c01b500dcfec1004b3e0afcaf021c9ce543d03a0510d002f053d943d03afd03"
    "4002ef4b0501bf712f02039cd93f0874e702fe8d2001f203233ff1839f052b945a0030012afdcc00f7ff3601b500dcfe"
    "b2004b3e1efcaf020b9cea43403b05100002ce63d243d03afd034002e3478703dd15ef0101bc1a001758a70202963001"
    "f903f33ee4879f050c9453002f0128fdcb0016003401b200dafeb0003b3e1ffcaf02fc8bee43c03b0510d001ce5bd743"
    "d03afd038002e62b5804f7219f01fedb1b002644070305ae3001fe07b33ed88b7f05fea74f00090129fdc80052003001"
    "ac00d8feb9003c3e0e008f02f36ff243103c05101002ea43e643d03afd03e002f303a804fa4d7f01fcf3db3f31e8e703"
    "06d230010304933ed08b3f05ffc34d000b012afdc10057003001a800d6fec1002c3e03007f02ed5bf543503c05145002"
    "fd33ef43d03afd032f03fcebd804fc695f01fbff9c3f374c870407e630010704733ecb8f1f0500d488ff70002afdc100"
    "caff8700f8ffd6fe2d00203e03007f02ed5bf543503c05145002fd33ef43d03afd032f03fceb3804f5695f01faff9c3f"
    "374c6704017e30010700733ecb8f1f0500d497ff9e0029fdbd00cbff6f003800d8fe2d002f3e0ffc5f020fd0e443003c"
    "040c80021540d4432f3bfdff2003df17c8030a123f01f3fffc3f299437044d965001a503a33f2aac1e04d07f80ffee00"
    "2cfdc900b4ff6f005c00dffe2e00203ef0034f02fad3d343c03b03088002ee43ba437f3bfcfb2003e61bf803f809ff00"
    "f1ff1c0010981703c5aa50011403c33f22b05d04c2f781ff32012efdac0099ff69005500e2fe2a000f3ee2d78f02d9cf"
    "c043703b03048002df439f43cf3bfcf77f03efef6804ef51af00f3ff2c00f7973702e6a25001e502d33f10b06d04bdc7"
    "80ff1d012cfda2008dff6b000300d6fe0e007e3df5bfff04d6978a43203b02005002c02778430f3cfbf36f04e14fc905"
    "dff5bf00f5ff1c00e49397024b7a50019803e33ff8ab5d04becb68ff5e0022fdaf007fff7f00c5ffd2fee7ff3b3e10e8"
    "c003ec4ba143df3a02fc1104ce9b9e435f3cfbef41020040dc07ed1d6f05f4ebcc3fe49336021fae5001e403e33fdda7"
    "b00508206dff150024fdbe0073ff7800bcffd4fee0ff2b3e06e8d002f907d043df3a02fcf102f617d2435f3cfbefc002"
    "03989905e4c11f06f5eb0c3f118437030f225001fb03e33fcba35005043060ff1a0028fdc2006eff4d00b9ffd6fedeff"
    "1b3efeff5f0202d0ef43df3a02fc700212a4f5435f3cfbef200303147803f8153f01f7ff7c3e347037040a9650010500"
    "e33fc09f1005ff2b88ff70002afdc100caff8700f8ffd6fe2d00203e03007f02ed5bf543503c05145002fd33ef43d03a"
    "fd032f03fceb3804f5695f01faff9c3f374c6704017e30010700733ecb8f1f0500d462ff680028fdbb00ccff7000c8ff"
    "d8fe2d00203ef0038f02ebbfd843503c05145002f033e043d03afd032f0320e83804b2695f015afcac3fd553c703f5ed"
    "30010c000300d66f11042f740eff7d002afdcb00b8ff6f00a4ffdffe2e002f3e0ffc8f0211bcbc43503c05144002052c"
    "d143d03afd032f0319e418033b555f01ebfcbc3fdd53f70307f6f0000e003300ef6b52043d0ccdfe7c002dfdad0099ff"
    "6900abffe2fe2a00003e1d288f0220bca043503c051480022630bf43d03afd03700310103802195d5f011afdbc3fef57"
    "670410aea0000d003300086c62044238e3fe7f002afda1008dff6b00fdffd6fe0e00713d0a405f023fd87843503c0514"
    "f00429688a43d03afd0360041eb09802b4855f0167fccc3f0758c605200ab0000a0013001b6c52044134a0ff970024fd"
    "ae007eff80003b00d2fee7ff343eef171e0431649e43503c0514cf0313b4a143d03afd034e02ffbf3902e0515f011bfc"
    "dc3f2258d30712e260050b14c33f1b6cbf05f7dfeaff930027fdbd0071ff78004400d4fee0ff243ef917fe0209e8d243"
    "503c0514df0206f8d043d03afd03cf02fc673803f0dd5f0104fcdc3f345c96051b3e10060a14033fee7b5f05fbcfe5ff"
    "9e0026fdc1006cff4b004700d6fedeff143e01007f02ed5bf543503c05145002fd33ef43d03afd032f03fceb3804f569"
    "5f01faffec3f3f60770307ea30010800733ecb8f1f0500d4")


def add(d, s=None, stride=18):
    """s: offsets of the count fields (default: a PC level); stride: bytes per moveable (PSX: 20)."""
    s = s or sections(d)
    u = lambda f, o: struct.unpack_from(f, d, o)
    na, nc, nf, nm = (u("<I", s[k])[0] for k in ("anims", "commands", "frames", "moveables"))
    anim = lambda i: s["anims"] + 4 + 32 * i
    if na > INDEX and u("<H", anim(INDEX) + 16)[0] == CORNER_BASE:
        return None
    movs = [u("<IHHIIH", s["moveables"] + 4 + stride * i) for i in range(nm)]
    second = min(m[5] for m in movs if m[0] != 0)
    assert second == INDEX and u("<H", anim(INDEX - 1) + 16)[0] == LEDGE_BASE + 70, \
        f"Lara has not {INDEX} animations ending with the ledge jumps ({second})"
    assert len(DATA) == 2 * sum(p * w for p, w in zip(POSES, SIZES))
    assert all((f - 1) // r + 1 + ((f - 1) % r != 0) == p for f, r, p in zip(FRAMES, RATES, POSES))
    assert u("<H", anim(HANG) + 6)[0] == AS_HANG, "animation 96 is not the hang"
    hang = u("<H", anim(HANG) + 16)[0] + HANG_FRAME
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
    new, offset, cmd = b"", 2 * nf, nc
    for k in range(n):
        turn = k % 2 == 0
        nxt, nxt_frame = (INDEX + k + 1, BASES[k + 1]) if turn else (HANG, hang)
        new += struct.pack("<IBBHiiHHHHHHHH", offset, RATES[k], SIZES[k], AS_CORNER, 0, 0, BASES[k],
                           BASES[k] + FRAMES[k] - 1, nxt, nxt_frame, 0, 0, 1 if turn else 0, cmd)
        offset += 2 * SIZES[k] * POSES[k]
        cmd += len(COMMANDS[k // 2]) if turn else 0
    cmds = b"".join(struct.pack(f"<{len(c)}h", *c) for c in COMMANDS)
    a_end = anim(na)
    c_end = s["commands"] + 4 + 2 * nc
    f_end = s["frames"] + 4 + 2 * nf
    out = (d[:s["anims"]] + struct.pack("<I", na + n) + d[s["anims"] + 4:anim(INDEX)] + new + d[anim(INDEX):a_end]
           + d[a_end:s["commands"]] + struct.pack("<I", nc + len(cmds) // 2) + d[s["commands"] + 4:c_end] + cmds
           + d[c_end:s["frames"]] + struct.pack("<I", nf + len(DATA) // 2) + d[s["frames"] + 4:f_end] + DATA
           + d[f_end:])
    return bytes(out)


if __name__ == "__main__":
    path = sys.argv[1]
    out = add(open(path, "rb").read())
    if out is None:
        print(f"{path}: has the corner shimmying already")
    else:
        open(path, "wb").write(out)
        print(f"{path}: corner shimmying added as animations {INDEX} to {INDEX + len(POSES) - 1}")
