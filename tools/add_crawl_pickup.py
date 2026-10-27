"""Smooths Lara's pickup in a crawlspace (animation 292, the crawl pickup, started by crawlflip.ips) in a
Tomb Raider III PC level: Tomb Raider III has only 6 poses for it (frame rate 10), so Lara jerks from one to
the next. Later games carry the same motion with 14 poses (frame rate 4) on the same skeleton; they are
added at the end of the frame list with their single axis rotations turned from 12 to 10 bits (Tomb Raider
III's precision), and animation 292 points to them, the last one twice: in its last frames the game
interpolates towards the pose behind (otherwise whatever follows the frame list: Lara vanished for a moment
and the camera jumped). Its frame numbers, state changes and commands stay as they are. 1110 bytes per
level. A level that has them already is left as it is.

Usage: add_crawl_pickup.py <level.tr2>   (changes the file in place)"""
import struct
import sys

from add_crawl_flip import sections

INDEX = 292
FRAME_SIZE, FRAME_RATE = 37, 4


def add(d, s=None, stride=18):
    s = s or sections(d)
    u = lambda f, o: struct.unpack_from(f, d, o)
    nf = u("<I", s["frames"])[0]
    anim = s["anims"] + 4 + 32 * INDEX
    rate, _, state = u("<BBH", anim + 4)
    if rate == FRAME_RATE:
        return None
    start, end = u("<HH", anim + 16)
    assert state == 39 and end - start == 55, f"animation {INDEX} is not the crawl pickup"
    d = bytearray(d)
    struct.pack_into("<IBB", d, anim, 2 * nf, FRAME_RATE, FRAME_SIZE)
    f_end = s["frames"] + 4 + 2 * nf
    data = DATA + DATA[-2 * FRAME_SIZE:]
    return bytes(d[:s["frames"]] + struct.pack("<I", nf + len(data) // 2) + d[s["frames"] + 4:f_end] + data
                 + d[f_end:])


DATA = bytes.fromhex(
    "6cff9400bbfe1100dcfe1501000050ffdbff2b43ef0d46b0f833ff0140020f04e00db94ff73300fe4f02f0fbf543ac02"
    "3a871f407f3ffd96a302c5781f40703f0269ab400000000000006cff9400bcfe1100dcfe1101000050ffc6ff3a43e00d"
    "73640835ff0140020f04ef0d8c9b073500fe4f02f0fb503efb43ac0234cb60402f3e70ff6303d5482140203ff490af07"
    "fddf00006eff9200c4fe1100dcfe0401000050ffb0ff4d43910d9a0818360002300210049e0d65f7173600fe3f02effb"
    "203df68b7d02291bab406f3bfd77d303e70c2640d03edb9c0f02fcaf00006eff9200cbfe1100dbfefb00000050ffafff"
    "4f43710d97001836ff01300210047f0d68031736fffd3f02effbb03cf8673d011613a3404f3b04c8d303eb302740d03e"
    "da98df3dfb8f00006eff9300d0fe1000dcfefc00000050ffb1ff4d43800d97fc08360002300210048f0d6803173600fe"
    "3f02effba03cfb3b3c000cff8e40e03beb476304ec582740d03edab8ef3cfc6f00006eff9300cbfe1000dbfefb000100"
    "50ffb0ff4d43810d99000836ff01300211088f0d6907173600fe3f02effbe03cf76b6d011b0f9d408f3bf4dbf303ea2c"
    "2740d03edda87f3dfe5700006eff9200c6fe1100dcfefb00000050ffb0ff4d43910d9a081836ff01300211049e0d65f7"
    "173600fe3f02eefb203df68b7d02291bab406f3bfd77d303e70c2640d03edb9cdf3dfb8f00006cff8f00ccfe1100dbfe"
    "fd00f9ff50ffacff4e43d00d91fc5836ff014f0210e85e0d4a83173600feff01f4f3c03c0820dc09fd7eef40af3d0394"
    "c304ea702340003fcaa0603deb7700006aff8e00d8fe1100dbfef100eeff50ffa8ff4f43300e7ac4b836ff015f020ebc"
    "cd0c2cf7173600fe8f0100eccf3c2370680bc2b9c709fffdb00106043405f41c2040403fad70f13dddaf6aff9200dffe"
    "1100dbfeef00e8ff50ffa6ff4d43800e6584d83600026f020d9c7d0c1eaf073600fe4f0109e85e3d33fc580a77a92705"
    "fffd40040854b404fa881e40603f9724423fde6769ff9500ddfe1100dbfefc00eaff50ffadff4743800e606488360002"
    "6f020da8bd0c24c7c735fffd5f0109f0bf3d3104580a77a92705fffd40040854d403f16824408f3fa5f45201df376aff"
    "9100d7fe1100dbfe1101f1ff50ffbcff3d43400e5a34b83500027f020dc84e0d434f373500feb00103001f3e2354880a"
    "a4c9c70afffd6f03eee7f403de042940903fc62c3104e0676bff8f00c7fe1100dbfe5201f9ff50ffcdff33431f0e51f0"
    "c83400026f020ee8cf0d7a3f8734fffd1002fa079f3e10b0290b33eab640af01b2772304cca02540903fe7887007eb83"
    "00006cff9200bcfe1100dcfe1701ffff50ffd9ff2c43ff0d48b81834000250020f00e00dae1f0734fffd4f02f2ff2f3f"
    "02f4ac051c3f3640cf3f27874303c5741f40803ffd88200afb0f0000"
)


if __name__ == "__main__":
    path = sys.argv[1]
    out = add(open(path, "rb").read())
    if out is None:
        print(f"{path}: has the smooth crawl pickup already")
    else:
        open(path, "wb").write(out)
        print(f"{path}: crawl pickup smoothed (animation {INDEX})")
