"""Adds the ledge jumps to a Tomb Raider III PC level: hanging from a ledge, Jump with Forward makes Lara jump up,
Jump with Back makes her push off and jump backwards (TRX's ledge jumps, driven by the executable, ledgejump.ips).

The animations are TRX's "hang to jump up", "hang to jump back" and their continuations
(data/trx/ship/games/tr3/injections/lara_animations.bin, its 317 to 320), poses unchanged (23, 10, 37 and 2,
frame rate 1, 39 words each, 5616 bytes per level). They go in as animations 318 to 321, right behind the
controlled drop (316, 317, tools/add_controlled_drop.py, which has to run first), so they have the same numbers
in every level; the animations behind them and every reference to them move up by four. Their frames, commands
and state change ranges are added at the end of their lists; the hang (96) gets two state changes at its last
frame, as in TRX: AS_UPJUMP (28) to 318 and AS_BACKJUMP (25) to 320; its state change list grows in place, the
later ones move up by two. 318 (state 28) goes on with 319, 319 with Lara's up jump (28, its first frame): she
is moved 703 up and jumps (fall speed -78, speed 2). 320 (state 25) turns her round (effect 0) at its last frame
and goes on with 321 (state AS_FORWARDJUMP, 3, speed 99), which moves her 603 up and 17 forward and goes on with
the end of the forward jump's start (76, its last frame: there she jumps). 318 and 320 free her hands; the sounds
(5, 0 and 29) as in TRX. Frame numbers LEDGE_BASE to LEDGE_BASE + 71. A level that has them already is left as
it is.

Usage: add_ledge_jumps.py <level.tr2>   (changes the file in place)"""
import struct
import sys

from add_controlled_drop import DROP_BASE
from add_crawl_flip import sections

INDEX = 318
LEDGE_BASE = 32220
POSES, FRAME_SIZE, FRAME_RATE = (23, 10, 37, 2), 39, 1
HANG, UPJUMP, FORWARD_START = 96, 28, 76
AS_HANG, AS_UPJUMP, AS_BACKJUMP, AS_FORWARDJUMP = 10, 28, 25, 3
BACK_SPEED = 99 << 16
UP, UP2, BACK, BACK2 = (LEDGE_BASE + sum(POSES[:k]) for k in range(4))
COMMANDS = [[5, UP + 10, 5, 5, UP + 19, 0, 3],                                  # sounds, hands free
            [5, UP2, 29, 1, 0, -703, 0, 2, -78, 2],                          # sound, move, jump velocity
            [5, BACK + 10, 5, 5, BACK + 19, 0, 5, BACK + 35, 29, 6, BACK + 36, 0, 3],   # sounds, turn round
            [1, 0, -603, 17]]                                                   # move
NCOMMANDS = (3, 3, 5, 1)
DATA = bytes.fromhex(
    "87ff72002bfdc200d2ff8300f8ffd6fe2d000f3e03fc6f02eb57f243403c04145002fd33ef43df3afdff0f03fceb1804"
    "f3694f01f9ff9c3f3744470401762f0106fc533eca870f05ffcb0000000086ff740033fdbc00ceff8500f8ffdefe1e00"
    "6f3e02fc3e05ece3b03af463bf3c03e42005ea736f3afffb7f3bfbd71f02fceb6803f871d000f6079c3f39487703fc71"
    "2f000ddc933ed6970f0401cc86ff950039fd9d00cbff9200f8ffe5fe1100cf3e02fc0e08e04f7136d1132f3d03b01008"
    "daabcf3500f00f3cf8af2f01fceb9802fc757000f313ac3f3b4cf702fc71ff3f0ec4933edbab0f0303d069ff9b003ffd"
    "6e00c7ff9e00f8ffebfe04002f3f02fc9d0abd637333561f9f3d0184100bcbd73f3109c4af3cf5873f00fcebd801007a"
    "1000ef1bbc3f3d54a702fc710f000ea8833edebfff0104d04cffad0042fd3400c3ff9700f8ffeefef9ff8f3f02fc1b0c"
    "68c3a534a6f69f3e0064000ebaeb0833f709ef3df2677f3ffcef8801017e0000f023ac3f3f549702fb710f000d94733e"
    "e4cfef0006d438ffb8003ffde6ffbcff8400f8ffe9fef2ffef3f02fc990bfad2a7375c1abf01fe77e90eb815d736fafd"
    "3f01ee531f3ffcef4802fb7df000ff1f3c3f4258e702fb790f000d94733eebcfef3f08d42effba003bfd9cffb9ff7700"
    "f8ffe1feedff3f0002fc280ac3ca573a3d867f04fe93790cb429c739fbf94f04ea43bf3efcef5803f37920020d1ccc3e"
    "445c3703fb81ff3f0ca0733ef1c7cf3e0ad02fffb4003dfd72ffb8ff7800f8ffe1feeaff6f0002fcf808a446e73b2ab6"
    "1f0600ac990aba4d873bfdf5ff05e83f6f3efcef2804f279e0020f18bc3e45604703fa810f000da8733ef3bbbf3d0cd0"
    "34ffac003dfd58ffb8ff7c00f8ffe0fee8ff9f0002fcd7078fee373d19d26f0703c8f908be65f73cfef56f07e63b1f3e"
    "fdeff804f079a0030f10ac3e45685703fa7d0f000db4733ef4abcf3c0ed036ffa70030fd4cffb8ff8700f8ffccfee7ff"
    "bf0002fc57088ae6473c0fde6f0706ec3909c975573c00fabf07e94fdf3dfdefc806e579a00515005c3e42806704097e"
    "2f01f0f3933df66b8f3c0dd03fffa50024fd76ffb8ffb100f9ff94fee7ffcf0001fcf90cedb6e73416dac00300148b0e"
    "4c1688350212ef04f287df3dfeef780ba609b00b3a201c3d3ccc2808703ac009742b4239e88bef3d07dc52ffb40005fd"
    "bfffb6ffa600f9ff57fee7ffcf0000fcbe0af41b3033013c803ff83f600aad1fef310bfc5f01f7bbdf3d00f0440d881c"
    "550ecbe51d3c222c0909faa2bb0d24e2a137087c9f3f01e85affab00c4fce9ffb3ff8a00f9ff19fee7ffbf00ffff7f04"
    "1914403b0720e03af94bb004ce23403800184f3df5ffef3d01f0a20809b0760c239a5d3ccdb31a077dab7a0b7d1df039"
    "5ca83f01fcef6eff86008afccfffa1ff8900f8ffdffde7ffbf0002fcaf001c842f3f02fcf037fb4bc000e107903cfc13"
    "e039ef2fdf3dfdefd204eba7f60c4bae9d3d8ce33a04bde3b9091869103d8c7cbf01fbfb5cff6f005dfca1ff8cff9600"
    "f6ffaefde7ffbf0010fcff0228a41f39fc9fc038f3533000dc2fd03cfc0b0f38f6efbf3dead78204f4b30d0e1dc0dd3e"
    "898bca01afcf58079174003f9294403f040c5cff6c0035fc71ff81ff9200f4ff7ffde7ffbf001ffc6f063bcc9e314a10"
    "703aec4f6000d467403cfd079f3606887f3dd5bfc20403b87e083a140d008f276a3f97cfb8032d40a00091b0703c0d1c"
    "4fff660012fc48ff81ff8800f3ff58fde7ffbf002bfc9009550cd834bf05103de94bf000cf877f3bfeff1f3612404f3d"
    "c4af020512c40e043d10ec0099d71a3e83df2801fa67f00181b4403b14284eff6300f9fb33ff8bff9300f2ff43fde7ff"
    "bf0030fc200c7460c736cfb98001ec47a001cf77a03afc03ef3607782f3dbcb3d20420d4bd0132ec0c01aeb79a3f84e3"
    "a801353860024164703d144053ff6200e1fb1fff9bff9700f2ff30fde8ffbf0034fc410ed5c02738d8914006f14f7002"
    "d063d039fa03ef37fdb72f3db5b792042ce4bd3f28d01c01c59f9a018ae768038024a00103184000155c5bff5f00c8fb"
    "07ffaaff9e00f2ff1dfde8ffbf0037fcf50dd0856739e071000bfd735003d04be038f8071f39f5eb1f3dafbf520438f4"
    "0d3e1fc02c01db87ca0395f36806ca7c603fe1231003187861ff6000adfbe4feb0ff8900f3ff07fde7ffbf0036fc760c"
    "1886473ae855120f8274c004c9332037fd03ef3aeffb2f3daec343043e0c1d3e1bd85c01e36f4a059dfbd807faf0203e"
    "da1fa0051d904bff67008efb9afeabff8a00f2ffe9fce3ffbf002bfc580e863e6739ff29b60d7f358007b00baf3323ac"
    "0f3ee8db5f3db9cf43043628de01243cac01d263aa0495e72805a9587f011790b00723a040ff7c0073fb32fea1ff8d00"
    "f2ffcafcdeffbf001ffc8b0e4023a63817f2840d36d55f0a84976b3133ad6f01deab8f3dc5df53042b4cde053ed8fc01"
    "bd57ea038ecb08026238ff045268b0092bb440ff7c0073fb32fea1ff8d00f2ffcafcdeffbf001ffc8b0e4023a63817f2"
    "840d36d55f0a84976b3133ad6f01deab8f3dc5df53042b4cde053ed8fc01bd57ea038ecb08026238ff045268b0092bb4"
    "3eff96005cfbb5fd95ff8e00f2ffabfcdaffbf0013fcfc0d9573c63731aa410c8df47e0c224fb934cd85af04cf67bf3d"
    "d2eb73041f6c4f097bf44c02a7474a0490afc8023f5cbf048aa4b00b39e03dff9e0044fb75fd8dff8a00f3ff8afcd6ff"
    "bf0007fc3c0eb6e3c636467e71095b28cc0cb0ae9937e5715f07b603df3de0f3a304178c610aeee48c02922f8a049093"
    "c803226c4f0388a4410d4d2442ffa40023fb92fd8aff9100f4ff65fcd3ffcf00f9ff0e0c0734b6325f62a1083d4c6e0c"
    "3dd78a35ca316e079eb7cf3df6e7a30532d852091fc57b0285e70a038097d8010b603f00cd9b010e4c2437ff9800fbfa"
    "adfd8aff8c00f6ff3cfcd2ffdf00ecff1e091a9c5f3213c481061b68df0a90bfda32b865be06c5f3bf3d0cd82406b9d4"
    "f107cc709a0189e3fa02738bc8014980ef3d49eb600e36d837ff8700adfad4fd8cff9f00f8ff0efcd2ffdf00e0ff0e06"
    "1ed4df3811a8c13dfe67b009c3173c311db9bf04e597bf3d21c8f50343a9c0048d68f90097d35a036677d80196982e3d"
    "0b70700e0e444cff74004dfad9fd8eff7700faffe3fbd6ffbf00dbff7f031b04cf3d08a0a136e5832008e537df3256a8"
    "4002f30b1f3e2bc43602a245ef0167f40901a7a7aa05522f6803d01cae01ac188f0df6ef3fff67001cfabafd93ff5400"
    "faffbffbdfff4000e6035f02112c8f3e05b44036f6f36005f0332f362ef8f03ff12f0f3f1ed86703d3b98f0144f07b01"
    "b883280bee86d507adec1e01a0a49f0afbfb56ff6000f1f997fd8cff3500faff9dfbeaffd03ff4076f0106548f3e02d0"
    "303709546002f91faf3919ecb03dec4b3f000ce85903097abf0121f81d01c9bf640901b6a207327c2f0073485007fe03"
    "88ff6100c9f970fd52ff3400faff7bfbf4ff503f02049f00fa836f3efff39f3813d4603f01043f3d07c4703be46b5f01"
    "faf78b013122f001fd03d0c36302c59d50020c1cff3f3ef80f0401fc00008aff6f0036fd0e01d1ff7d00f8ffd6fe2d00"
    "0f3e03fc6f02eb57f243403c04145002fd33ef43df3afdff0f03fceb1804f3694f01f9ff9c3f3744470401762f0106fc"
    "533eca870f05ffcb000000008dff73003efd0201d1ff8200f8ffdefe1e006f3e02fc3e05ece3b03af463bf3c03e42005"
    "ea736f3afffb7f3bfbd71f02fceb6803f871d000f6079c3f3948c703fb71ef0009e0733ec6970f0401cc79ff950043fd"
    "df00d4ff9c00f8ffe5fe1100cf3e02fc0e08e04f7136d1132f3d03b01008daabcf3500f00f3cf8af2f01fceb9802fc75"
    "7000f313ac3f3b4c3703f571bf000ec8933ec3a70f0303d058ff9b0048fda900d5ffa400f8ffebfe04002f3f02fc9d0a"
    "bd637333561f9f3d0184100bcbd73f3109c4af3cf5873f00fcebd801007a1000ef1bbc3f3d54a702f06d7f0012acb33e"
    "c0b7ff0104d03fffa6004cfd6f00ccff9c00f8ffeefef9ff8f3f02fc1b0c68c3a534a6f69f3e0064000ebaeb0833f709"
    "ef3df2677f3ffcef8801017e0000f023ac3f3f543702f0712f000d98833ec1c3ef0006d433ffb10048fd3900c4ff8900"
    "f8ffe9fef2ffef3f02fc990bfad2a7375c1abf01fe77e90eb815d736fafd3f01ee531f3ffcef4802fb7df000ff1f3c3f"
    "42582702037a0f3fe993533dd0cfef3f08d42fffb30044fd0200beff7300f8ffe1feedff3f0002fc280ac3ca573a3d86"
    "7f04fe93790cb429c739fbf94f04ea43bf3efcef5803f37920020d1ccc3e445c770215828f3ec8a3433cdccfcf3e0ad0"
    "33ffae0049fddfffbbff7600f8ffe1feeaff6f0002fcf808a446e73b2ab61f0600ac990aba4d873bfdf5ff05e83f6f3e"
    "fcef2804f279e0020f18bc3e45604703178a7f3fc5a7133cdbcbbf3d0cd036ffa6004efdbdffb9ff7c00f8ffe0fee8ff"
    "9f0002fcd7078fee373d19d26f0703c8f908be65f73cfef56f07e63b1f3efdeff804f079a0030f10ac3e456827041892"
    "bf00c7af133cd7c7cf3c0ed038ffa10046fdaaffb9ff8300f8ffccfee7ffbf0002fc57088ae6473c0fde6f0706ec3909"
    "c975573c00fabf07e94fdf3dfdefc806e579a00515005c3e4280a70529aedf02bcdb233cd99b8f3c0dd03bffa20034fd"
    "eaffb8ff9f00f9ff94fee7ffcf0001fcf90cedb6e73416dac00300148b0e4c1688350212ef04f287df3dfeef780ba609"
    "b00b3a201c3d3ccc38087d42c0078633c23ce2fbdf3d07ec4cffb30001fd2b00b9ff9100f9ff57fee7ffcf0000fcbe0a"
    "f41b3033013c803ff83f600aad1fef310bfc5f01f7bbdf3d00f0440d881c550ecbe51d3c222c1909ff822f0d2de3123d"
    "f943803f01045affaa00bffc3100b9ff8800f9ff19fee7ffbf00ffff7f041914403b0720e03af94bb004ce2340380018"
    "4f3df5ffef3d01f0a20809b0760c239a5d3ccdb33a07809b5a0d8015213c49401001fc1765ff850084fcf1ff77ff8100"
    "f8ffdffde7ffbf0002fcaf001c842f3f02fcf037fb4bc000e107903cfc13e039ef2fdf3dfdefd204eba7f60c4bae9d3d"
    "8ce33a04bde3b9091869103d8c7cbf01fbfb5aff6c0058fca0ff4fff8e00f6ffaefde7ffbf0010fcff0228a41f39fc9f"
    "c038f3533000dc2fd03cfc0b0f38f6efbf3de9db8204f4b30d0e1dc0dd3e898bba01afd338079488d03e9484ef3f0578"
    "49ff6c002cfc70ff57ff8a00f4ff7ffde7ffbf001ffc6f063bcc9e314a10703aec4f6000d467403cfd079f3606888f3d"
    "d4c7c20403b87e083a140d008f276a3f97cfa8032e48900092a80e3e18dc47ff640008fc47ff81ff8e00f3ff58fde7ff"
    "bf002bfc9009550cd834bf05103de94bf000cf877f3bfeff1f3612404f3dc4af020512c40e043d10ec0099d71a3e83df"
    "2801fa67f00181b44e3d274449ff5d00f3fb32ff8dff9e00f2ff43fde8ffb0003000400c7564e736cfb51002eb43b001"
    "cf77af3afcffef360774ef3cbe9fe20421d88d0133f43c01b1afca3f85dfb80137344002415c6d3e1fdc4eff5500dffb"
    "1eff9bffa000f1ff30fde9ffb0003404820ee90c7738d8896007ef437002cf63d039fa03ef37fdaf8f3cba8bb20430ec"
    "4d3f29e07c01cc939a018ae768038024a0010318bd3f107457ff4e00cbfb06ffabffbc00f1ff1dfde9ffb0003808c50d"
    "e5d5c739e06d700cf95f5003cf4be038f8031f39f4df2f3cb67782043efc4d3d20d4cc01e57bca018dfbc805b8987000"
    "dc3fed00fe0f59ff4800b1fbe3feaeffd500f3ff07fde7ffbf0036fc760c1886473ae855120f8274c004c9332037fd03"
    "ef3aeffb2f3cb93f43043e0c1d3e1bd85c01e36feb018e07d906d908f03fd24bbc01eebf44ff540091fb95fe95ffc800"
    "f7ffe7fcdeffdf0027e0160d27ba3739f83d320d6d107007b1179f3321c03f3ee8f71e3dc7c3b30312144d0827f09c3e"
    "86d7da028df7c8049a7c5f0213941c02e6a328ff7b0073fb26fe67ffb000fdffc5fcd3ffdf0017c0070e3e16e7370c12"
    "c10a5fc84f0a89b73b314091bf01e1e74e3ed2436303ec2b230e94ad7d3d2c57ea038ecb08026238ff0452685c02e08f"
    "3bff8d0058fbcbfd48ff90000100a4fcc9ffdf00079ce80e7b0aa63625d631085aa48e0c349b8934db793f05d8cb8d3f"
    "d9cfe303f953120c27595c3e34abda049b6f08013f5c80036f58ac02da7f3aff820043fbd5fd1cff890005008afcc2ff"
    "df00f7633a0f0e5bb6353a9a41065b9ced0cd31e3937f059df07d7c78d00db7b4304046ce109d88cfc3e41179a05a213"
    "f800258841017418fc02d87735ff570036fbd4fd09ff9a0005007ffcc1ffce00e8ef1b0f4b532635369e910568cc5d0c"
    "0b8f0937ee61c008fb4bad00dc73f303eb475109c9b05c3d3633b9059acff8011494a13f7c388c03e1832eff4c002dfb"
    "d0fd03ff9b00050077fcc1ffae00da6feb0e6bdfa6342bba110575f86d0b45f76936e871e00828e8ad00dd777303cd1b"
    "2109c0e87c3b2383a905909728030594313e85380c04eb9733ff590031fbd7fd09ff8e00050078fcc2ff4d00d0fb7c0e"
    "949356341be211056ffcdd0a59f75936dd89b108413c8d00db7bc202b5ff3209b0044c3a0df389058a7bf803f88f213d"
    "7a344c04f6c33eff54004dfbe1fd1bff9300040090fcc1ffcd3fce934d0db8575734032a110636788d0b1b2fe937cca5"
    "b0082ccc4d00d47fe201a8fb110988b8ed3a016f990592b7e803f197e13c3640fd03ff2f4aff68006efbd0fd16ffba00"
    "0400acfcc0ff4d3fcf2fed0bc2bb5734eb6d5006f1cb9c0bc216a939bfad9008103c0d00cb7f13019e03d1085d64ed3b"
    "fcde99059bfb9803eb9ff13cec536d03069c59ff7c0086fbb5fd11ffd6000500c2fcc2ffcc3ed0d7bd0ac4eb7734e189"
    "d005bb47fb0a855a393bb8a96f08fbdfdd3fc78b6300980b41093c1c3e3cfe3a2a059d0b6803e39f113dc237bd020af0"
    "67ff620086fbbdfd24ffd7000b00c7fcd0ff9c3ed1a77d0ac8d79734071ac005b84bcb0a834ee93bc19d1009ff172d00"
    "d0b303009d3b700d1fa48e39066b190287070703ddf3903de9bb6d010de06dff630081fbbffd37ffd7001300c5fcdcff"
    "4c3ed2635d0accdbb63424ce6005b9631b0b8e6af93bcc8d8009095c2d00dce70300a51fdd0e53671e381e87880188ff"
    "4803cd6bb03e1b44cd000ae83aff93007dfba2fd1effe1001b00bafcd0ff4b3dd39b4e0ad277b733e9b1e0029b7fdc0c"
    "c40a193ad05d30080c44de3ee6170201af2b0f0bdb83fe3a58c78f0156d0f0050708ef3f3ffc9e02f56333ffc3007cfb"
    "99fdeefee2002400a9fcbdff4a3cda83bf09d84789339119d000c0636f0d8b87193994a540071d705e3df07b5102b117"
    "bf05f2df1f3e9657c0005b58800307a00f0042f85e05e7ef3affd50097fba0fda9feeb0006008efcb9ff773909f07f0a"
    "da777732fff9e03df53f900bf763a834e3b5cf3e12f4ff3bfafb5003ab232f01fafbf03fd203bf0259fc00020600ef3f"
    "3ffc5f0900f445ffe0003efba3fdc4fe0301130045fc0000603aff036f08e59fb030fe03403cf5273009fa73fc30fc40"
    "1f3d19ecff3bfafb5003ab232f01fafbf03fd203bf0259fc00020600ef3f3ffcff07f6f725ffc900e5fa58fde6ffa701"
    "f8fff6fba6007f3a00fc6f08e49bb130a767d003f6074009f9773b313949c0031930ff3bfafb5003ab231f01fafbff3f"
    "d2ffaf0258f8ff0105fce03f3f00ff07f6f725ffc900e5fa58fde6ffa701f8fff6fba6007f3a00fc6f08e49bb130a767"
    "d003f6074009f9773b313949c0031930ff3bfafb5003ab231f01fafbff3fd2ffaf0258f8ff0105fce03f3f00ff07f6f7")


def add(d, s=None, stride=18):
    """s: offsets of the count fields (default: a PC level); stride: bytes per moveable (PSX: 20)."""
    s = s or sections(d)
    u = lambda f, o: struct.unpack_from(f, d, o)
    na, nst, nd, nc, nf, nm = (u("<I", s[k])[0] for k in ("anims", "states", "dispatch", "commands", "frames",
                                                            "moveables"))
    anim = lambda i: s["anims"] + 4 + 32 * i
    if na > INDEX and u("<H", anim(INDEX) + 16)[0] == LEDGE_BASE:
        return None
    movs = [u("<IHHIIH", s["moveables"] + 4 + stride * i) for i in range(nm)]
    second = min(m[5] for m in movs if m[0] != 0)
    assert second == INDEX and u("<H", anim(INDEX - 1) + 16)[0] == DROP_BASE + 8, \
        f"Lara has not {INDEX} animations ending with the controlled drop ({second})"
    assert len(DATA) == 2 * FRAME_SIZE * sum(POSES)
    state = lambda i: u("<H", anim(i) + 6)[0]
    assert (state(HANG), state(UPJUMP), state(FORWARD_START)) == (AS_HANG, AS_UPJUMP, AS_FORWARDJUMP), \
        "animations 96, 28 and 76 are not the hang, the up jump and the forward jump's start"
    d = bytearray(d)
    n = len(POSES)
    for i in range(na):                                          # next animations
        if u("<H", anim(i) + 20)[0] >= INDEX:
            struct.pack_into("<H", d, anim(i) + 20, u("<H", anim(i) + 20)[0] + n)
    for i in range(nd):                                          # state change targets
        o = s["dispatch"] + 4 + 8 * i + 4
        if u("<H", o)[0] >= INDEX:
            struct.pack_into("<H", d, o, u("<H", o)[0] + n)
    for i in range(nm):                                          # first animations of the moveables
        o = s["moveables"] + 4 + stride * i + 16
        if u("<H", o)[0] >= INDEX and u("<H", o)[0] != 0xFFFF:
            struct.pack_into("<H", d, o, u("<H", o)[0] + n)
    hang_end, (changes, first) = u("<H", anim(HANG) + 18)[0], u("<HH", anim(HANG) + 24)
    have = {u("<H", s["states"] + 4 + 6 * (first + c))[0] for c in range(changes)}
    assert changes and not have & {AS_UPJUMP, AS_BACKJUMP}, "the hang has state changes to the jumps already"
    at = first + changes                                         # the hang's state changes grow in place
    for i in range(na):
        if i != HANG and u("<H", anim(i) + 26)[0] >= at:
            struct.pack_into("<H", d, anim(i) + 26, u("<H", anim(i) + 26)[0] + 2)
    struct.pack_into("<H", d, anim(HANG) + 24, changes + 2)
    states = struct.pack("<HHHHHH", AS_UPJUMP, 1, nd, AS_BACKJUMP, 1, nd + 1)
    ranges = struct.pack("<hhhhhhhh", hang_end, hang_end + 1, INDEX, UP, hang_end, hang_end + 1, INDEX + 2, BACK)
    upjump_base = u("<H", anim(UPJUMP) + 16)[0]
    forward_end = u("<H", anim(FORWARD_START) + 18)[0]
    heads = [(AS_UPJUMP, 0, UP, UP2 - 1, INDEX + 1, UP2),
             (AS_UPJUMP, 0, UP2, BACK - 1, UPJUMP, upjump_base),
             (AS_BACKJUMP, 0, BACK, BACK2 - 1, INDEX + 3, BACK2),
             (AS_FORWARDJUMP, BACK_SPEED, BACK2, LEDGE_BASE + sum(POSES) - 1, FORWARD_START, forward_end)]
    new, offset, cmd = b"", 2 * nf, nc
    for k, (state, speed, base, end, nxt, nxt_frame) in enumerate(heads):
        new += struct.pack("<IBBHiiHHHHHHHH", offset, FRAME_RATE, FRAME_SIZE, state, speed, 0, base, end, nxt,
                           nxt_frame, 0, 0, NCOMMANDS[k], cmd)
        offset += 2 * FRAME_SIZE * POSES[k]
        cmd += len(COMMANDS[k])
    cmds = b"".join(struct.pack(f"<{len(c)}h", *c) for c in COMMANDS)
    a_end = anim(na)
    st_at = s["states"] + 4 + 6 * at
    st_end = s["states"] + 4 + 6 * nst
    dp_end = s["dispatch"] + 4 + 8 * nd
    c_end = s["commands"] + 4 + 2 * nc
    f_end = s["frames"] + 4 + 2 * nf
    out = (d[:s["anims"]] + struct.pack("<I", na + n) + d[s["anims"] + 4:anim(INDEX)] + new + d[anim(INDEX):a_end]
           + d[a_end:s["states"]] + struct.pack("<I", nst + 2) + d[s["states"] + 4:st_at] + states + d[st_at:st_end]
           + d[st_end:s["dispatch"]] + struct.pack("<I", nd + 2) + d[s["dispatch"] + 4:dp_end] + ranges
           + d[dp_end:s["commands"]] + struct.pack("<I", nc + len(cmds) // 2) + d[s["commands"] + 4:c_end] + cmds
           + d[c_end:s["frames"]] + struct.pack("<I", nf + len(DATA) // 2) + d[s["frames"] + 4:f_end] + DATA
           + d[f_end:])
    return bytes(out)


if __name__ == "__main__":
    path = sys.argv[1]
    out = add(open(path, "rb").read())
    if out is None:
        print(f"{path}: has the ledge jumps already")
    else:
        open(path, "wb").write(out)
        print(f"{path}: ledge jumps added as animations {INDEX} to {INDEX + len(POSES) - 1}")
