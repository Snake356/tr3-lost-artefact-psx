"""Smoother jump twists in a Tomb Raider III PC level, as TRX's option enable_jump_twists (LostArtefacts/TRX,
TRXInjectionTool): the twist out of the running jump (animations 207, 209) and out of the back jump (212,
213) carry 4, 2, 3 and 3 poses at frame rate 4, so Lara snaps through the turn; TRX's versions have one pose
per frame (14, 6, 11, 11) over the same frames. These poses are added at the end of the frame list and the four
animations point to them with frame rate 1; their frame numbers, state changes, next animations and commands
stay as they are. The old poses stay unused in the list. 3052 bytes per level. A level that has them already
is left as it is.

Usage: add_jump_twists.py <level.tr2>   (changes the file in place)"""
import struct
import sys

from add_crawl_flip import sections

OLD_RATE, NEW_RATE = 4, 1
TWISTS = {
    207: (35, 14, bytes.fromhex(
    "9cff6d0014fd0f00defeea00040063fefbff703dff03270dffe54837ff01a5438f3d03e8d843cd431001fe3b4e0cb85b"
    "5f0bf4d7fdc35003216c900c17580ac04f0203d0000097ff6d000dfd0400f7fe0501030054feffff503dfd03670cf2b5"
    "0738fffda643103f0600bc43ba431000ff07ae0c994f4f0bf4dbfdc330053c90400c16500ac05f0109f0000085ff6e00"
    "09fdfcff22ff2101030045fe0400503dfa0b870be999b73800fead432001091c9b43ab43ef3e01ccfe0c72433f0bf5db"
    "fdc3e00661d0200c154c0ac050000e1000006eff6e0007fdeaff53ff3b01030035fe0900503df617b70ae2854839ff01"
    "b94370030e4075439e43af3d068c3e0d47273f0bf5dbfdc331089138200c154c0ac0603f133400005dff6e0009fdccff"
    "81ff5201030026fe0d00403df21ff709dc75d739fffdc643d005166c4d4394435f3c0d486d0d16fb3f0bf4dbfdc3f108"
    "cbc4300c15500ac0703e1754000055ff700017fda2ffa6ff6901030016fe1200103ded276709d561473a00fed4430008"
    "24ac25438e43fe3a17fc8d0de3b24f0bf4dbfdc332090769500c16500ac0903d1a7400004cff6e002bfd6fffb8ff7f01"
    "030006fe1600c03ce7232709ce45b73a00fee043c1093b04173000fe8943ae3924acad0dad4e5f0bf4d7fdc303093e09"
    "700c16540ac0d03c1c8c3fff6f0040fd37ffadff92010300f7fd1a00203ce1173709c515183b0002e943e10a5a785732"
    "00fe87437e3834509c0d7aca5f0bf4d7fdc383086a95800c17580ac0203c1d9c31ff700058fdf6fe8dff9d010300e9fd"
    "1d00203bda03b609b7c5673bfffdec43310b78d84834000286435d3746ec6c0d503a5f0bf4d7fdc3f4078b15900c1758"
    "0ac0a03b1ca011ff710080fd9ffe5aff87010300dcfd1f009f39c8b7060b9205873bfffde643410a7ec4b735fffd8b43"
    "3d365b841b0d337e5f0bf4d7fdc3b4079e9d900c17580ac0603b1a8cf6fe880093fddffe40ff13010400cffd1f001f37"
    "ac3f240c2815683bff01d94311087654c73600fe9943fd347510aa0c279a5f0bf4d7fdc31508a745900c17580ac0903b"
    "166423ff9d0054fd1dff6eff84000400c4fd1e00ce33a0b3910ab0e0383b0002c743400573dc8837ff01ab43dc339b68"
    "490c2ab65f0bf4d7fdc3c608ae05900c17580ac0e03b102c23ff9d0054fd1dff6eff84000400c4fd1e00ce33a0b3910a"
    "b0e0383b0002c743400573dc8837ff01ab43dc339b68490c2ab65f0bf4d7fdc3c608ae05900c17580ac0e03b102c56ff"
    "7b00e7fcfbfe68ff6d000400b9fd1e00db305890800783d4083b0002b4435002787438380002be430b33d868e80b33e6"
    "5f0bf4d7fdc36609bad1900c17580ac03f3c09f4")),
    209: (37, 6, bytes.fromhex(
    "99ff9d00d9fcb4fed5ff0a010400affd1d008032427990047850d73a00fea54300008220e838ff01cd43ea32213da80b"
    "3c465f0bf4d7fdc3a709c991900c17580ac06f3c04c80000000037ff1f016bfd50fed2ff0a010200d4fd2000023bcd60"
    "10066a7cc83900026f3affff20016a446838ff01df3b00fc9b32cff0380d081e2f0deebbfdc3370c01aa110f5b6809c0"
    "5f3c04c42affd70001fd91fe96ff32010000fffd2300d1026c30f0075aa8783800028f3affff80024e60b837ff01bf3a"
    "00fc4d3384a0660e70816e0faaabfec3190ec1ce670de18908c04f3c04c063ff780026fd28ff7fff3801feff22fe2200"
    "4f03fd93900947a44837ff01a543400335645835ff019c43ce3458d0030e8494d80e2cb2fec3ac0cab9fe70aefc508c0"
    "4f3c05c00000000052ff910021fd8bff8aff1001fcff38fe18002f02e65b800a293c6836ff0170390100c00222543830"
    "ff011039ff032f373f8c110ce6e3080f36dafdc3fd091888d70aefc50ac06f3c04c80dffe90015fdf0ff68ffd900faff"
    "46fe0a00cf3ff7b7ff0a01909835ff01303804009001124469438038fe038f3b1ef06006628bef0af5dbfcc39e058ddc"
    "a00d228410c0ff3c00e80000")),
    212: (37, 11, bytes.fromhex(
    "4cffb800f1fcfbffa0fffb00f7ff49fef8ffc00303084f01f5b3f8438038fa2770010064fa43f037ea3f903cfb031d0c"
    "21c30f03fafbd4c3820bdb6c1004080c3ec0244000000000000066ff6100f2fc090079ffec00f8ff47fef8ff2f05f2c3"
    "4f00f5b7d4437038fb2790020064aa436038eb375f3c01cc9d0b3ecb60090d60d6c3c20ab2402f09f3c33cc000040890"
    "0000000074ff6600f8fc07005fffc000faff41fef8ff2f06da735f3ff5bbab436038fb27100400644e43e038ed33bf3b"
    "088c4e0a731f250d6bf1d7c3010979f01c0e114f3bc0a1051c500000000073ff9f00fcfcdcff52ffd100ffff37fef8ff"
    "cf06bb170f00f5b781435038fc23f00500640a436039ee2f0f3b0d688e09892bb608a9c9d9c3f1075fe09a0b741239c0"
    "e10525b0000000006bffbe0001fd88ff45ffe300060029fef8ff3e0798af7f02f5ab57435038fc2310080064b73100fe"
    "e039f0276f3a0e6cce09830b06089e45dbc311085fe42a0b85b237c001051e90000053ffc00007fd1cff3cff1f011000"
    "17fef9ff6e0773377f05f49f2c434038fd23700a0064b73300fe603af123af390e782d0a76df05088781dcc3810864e8"
    "0b0baaae36c0b1031358000044ffa00014fda1fe2fff44011b0003fef9ff5d074dabdf08ef8701434038fd1fe00c0068"
    "68350002f03af21fef380c8cad0a63a724086385dec311096df48c0addea34c021020a1000003eff840027fd54fe1eff"
    "45012500effdf9ff0d0728076f0ce1439832ff014038fe1f500f067cd73600fe803bf31b2f3808a82d0b4857c3073765"
    "e0c3c2097b045e090a1332c0500003c843ff7d002efd3dfe03ff1c012c00dcfdf9ff6c0605435b0ffca63835ff014038"
    "ff1f380efd593838ff01003cf4177f3704cc8c0b28f3b206115de1c3720a8b1c9e0725f730c0703eff8343ff7d002efd"
    "3dfe03ff1c012c00dcfdf9ff6c0605435b0ffca63835ff014038ff1f380efd593838ff01003cf4177f3704cc8c0b28f3"
    "b206115de1c3720a8b1c9e0725f730c0703eff8356ff78002bfd2bfee4fed1002f00c9fdf9ff6b05e86a880c202eb837"
    "ff013038ff1be80bfe618839ff01903cf517cf36fef3cc0b06872105fa90e3c3120b9d40af052f972fc0a03cfe3f")),
    213: (37, 11, bytes.fromhex(
    "86ff770025fd14fe7bff2a01c7ffb6fdf9fff203d16e870912f2183aff0140380014b809fe61e73afffdc03df70f2036"
    "f817dc0be8229003f3fce6c3820bae6c10042d0f2bc0b24300001fffa10049fd33febbff2901c0ffc9fdf9ff9101035f"
    "e70912f6083aff0140380114680bea05f73900fe503ef80b7037f90ffc0a00afb003dce4e8c3a20ab04ce00342132ac0"
    "403bff03f4feaf0057fd51fec1ff0c01c3ffddfdf9ff603f3fd3670a13faf839ff0140380114170db21928390002e03e"
    "f80bf038fa0bed090f1bd003c3c4eac3b209b42cc003571728c0703bfe070bff8b004cfdb4fec3fffd00ccfff3fdf9ff"
    "603d7a87d70a13fed839ff0140380210d40d2ef9273800fe803ff907703afb07cd08186fd003aaa4ecc3b208b8109003"
    "6d1b26c0903bfe0734ff480032fd0cffa5fffc00d6ff07fef9ffc03bb13f580b160ab839ff0140380310030db610d736"
    "00fe1000fa07c03bfb07ad071db7c0039188edc3c107bdf47003831f24c0c03bfd0b38ff4a0021fd47ff93fffc00deff"
    "1afef9ff9f3bdfe7c80b1a1a6839ff0140380310c20b7f2c073500fea000fa03b03cfb036d061ef7a0037768efc3c106"
    "c3d44003991f22c0e03bfc0b20ff620013fd84ff9fff0101e2ff2bfef9ff9f3cf8ab380c2336e838ff014038040cb10a"
    "5ea4773200fe4001fb03303dfb033e051e3360035d4cf1c3d105c9b81003b01f21c0103cfc0f07ff970011fdb6ffc0ff"
    "f800e8ff3bfef9ffcf3d00ac980c30624738fffd4038040cd10943380a43df01fcff4f3dfcfffe031b6b20034430f3c3"
    "e104d098e002c61b1fc0403cfb0f0000f6fec70013fdcfffc5ffe100edff49fef9ffff3e00c8f80c429e973700fe4038"
    "050cf0082cdc41436f02fcff2f3dfcffce02179fd0022b1cf4c3f103d774b002dc171ec0703cfa0f0000eefef1001afd"
    "e8ffcbffe200f2ff58fef9ff1f00ffef380d58eae73600fe40380608000819947943ff02fdff0f3dfcffae0111d77002"
    "1208f6c30103de549002f20f1cc0903cfa130000ebfe0f0122fdf1ffc5ff2401f7ff65fef9ff40010304890d71463736"
    "00fe40380608e0060b5cae438f03fdfbff3cfcfb8f000a0f0f02fafbf8c32102e630700208041ac0c03cf9130000")),
}


def add(d, s=None, stride=18):
    """s: offsets of the count fields (default: a PC level); stride: bytes per moveable (PSX: 20)."""
    s = s or sections(d)
    u = lambda f, o: struct.unpack_from(f, d, o)
    nf = u("<I", s["frames"])[0]
    rates = {i: d[s["anims"] + 4 + 32 * i + 4] for i in TWISTS}
    if all(r == NEW_RATE for r in rates.values()):
        return None
    assert all(r == OLD_RATE for r in rates.values()), rates
    d = bytearray(d)
    data = b""
    for i, (size, poses, frames) in TWISTS.items():
        anim = s["anims"] + 4 + 32 * i
        first, last = u("<HH", anim + 16)
        assert last - first + 1 == poses and len(frames) == 2 * size * poses, (i, first, last)
        struct.pack_into("<IBB", d, anim, 2 * nf + len(data), NEW_RATE, size)
        data += frames
    f_end = s["frames"] + 4 + 2 * nf
    return bytes(d[:s["frames"]] + struct.pack("<I", nf + len(data) // 2) + d[s["frames"] + 4:f_end] + data
                 + d[f_end:])


if __name__ == "__main__":
    path = sys.argv[1]
    out = add(open(path, "rb").read())
    if out is not None:
        open(path, "wb").write(out)
    print(f"{path}: jump twists {'already set' if out is None else 'replaced'}")
