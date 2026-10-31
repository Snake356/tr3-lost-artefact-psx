"""Lara's right arm at the end of the crouch roll (animation 220, back into the crouch) in a Tomb Raider III PC
level: Core's animation lays the hand on the knee, the crouch (222) that follows begins with the arm down, so
the arm jumps from one pose to the other. TRX (LostArtefacts/TRX #5531, TRXInjectionTool #271) replaced the
right arm (bones 8 to 10: upper arm, forearm, hand) in all its poses with hand-made ones that lead into the
crouch; these are their rotations. The animation plays 9 poses, but Core stored a 10th behind them (before
animation 221), and in its last frame the game blends towards it: with the old arm left there, the new one
twitched once at the end. The rest of the animation stays, each pose keeps its size (39 words in our
levels, 37 in the released HOUSE.PSX, which keeps a single axis rotation in one word; the new arm takes as
many words or fewer, zeros fill the rest). A level that has them already is left as it is.

Usage: fix_crouch_roll_arm.py <level.tr2>   (changes the file in place)"""
import struct
import sys

from add_crawl_flip import sections

ANIM, POSES, BONES = 220, 10, (8, 9, 10)          # 9 played, 1 stored behind
OLD = [(0x002C, 0xAEFB, 0x0700, 0x0000, 0x3BC0, 0x1B3E), (0x3F6C, 0xE6EC, 0x0970, 0x0000, 0x3A90, 0x86ED),
       (0x007D, 0x06E7, 0x0B50, 0x0000, 0x39C0, 0x5EED), (0x031D, 0x32E9, 0x0D60, 0x0000, 0x396F, 0xFF17),
       (0x06CD, 0x92FB, 0x0F90, 0x0000, 0x39DF, 0x875E), (0x0A3E, 0xA33A, 0x0E47, 0xFDFF, 0x3ABF, 0x23B2),
       (0x0AE0, 0xC7BE, 0x0C67, 0xFDFF, 0x3B9E, 0xC804), (0x08E2, 0x140E, 0x0AF7, 0xFDFF, 0x3C8E, 0x7C49),
       (0x0722, 0x8027, 0x0A37, 0xFDFF, 0x3D3E, 0x4872), (0x06E2, 0x8C29, 0x0A17, 0xFDFF, 0x3D5E, 0x4078)]
NEW = [(0x3FCC, 0xBAF7, 0x4080, 0x3B30, 0x5B01), (0x3FCC, 0xF6E9, 0x40A6, 0x3A20, 0x76E8),
       (0x019D, 0x1AE6, 0x40C5, 0x3970, 0x36FC), (0x034D, 0x36EE, 0x40D6, 0x39CF, 0xEF33),
       (0x03FD, 0x4EF4, 0x40DA, 0x3A4F, 0xC369), (0x031D, 0x46FA, 0x40CD, 0x3B1F, 0xC39A),
       (0x024D, 0x42FF, 0x40BF, 0x3BDF, 0xC3CC), (0x017D, 0x3B04, 0x40B2, 0x3C8F, 0xC3FA),
       (0x00CD, 0x3308, 0x40A8, 0x3D3F, 0xC022), (0x008D, 0x330A, 0x40A4, 0x3D6F, 0xC02E)]


def split(words):
    """The 15 rotations of a pose (behind the box and the offset), each one or two words."""
    out, j = [], 9
    for _ in range(15):
        n = 1 if words[j] & 0xC000 else 2
        out.append(tuple(words[j:j + n]))
        j += n
    return out


def angles(rots):
    """x, y, z (10 bits each) of rotations given as one or two words."""
    out = []
    for r in rots:
        if len(r) == 2:
            out.append((r[0] >> 4 & 0x3FF, (r[0] & 0xF) << 6 | r[1] >> 10, r[1] & 0x3FF))
        else:
            v, axis = r[0] & 0x3FF, r[0] >> 14
            out.append(tuple(v if axis == i + 1 else 0 for i in range(3)))
    return out


def as_rots(words):
    return [tuple(words[0:2]), tuple(words[2:3]) if len(words) == 5 else tuple(words[2:4]), tuple(words[-2:])]


def add(d, s=None, stride=18):
    """s: offsets of the count fields (default: a PC level); stride: bytes per moveable (PSX: 20)."""
    s = s or sections(d)
    a = s["anims"] + 4 + 32 * ANIM
    offset, _, size = struct.unpack_from("<IBB", d, a)
    first, last = struct.unpack_from("<HH", d, a + 16)
    rate = d[a + 4]
    assert (last - first) // rate + 1 == POSES - 1, (first, last, rate)
    assert struct.unpack_from("<I", d, a + 32)[0] == offset + 2 * size * POSES, "no pose behind"
    d = bytearray(d)
    done = 0
    for k in range(POSES):
        o = s["frames"] + 4 + offset + 2 * size * k
        words = list(struct.unpack_from(f"<{size}H", d, o))
        rots = split(words + [0] * 4)
        arm = angles([rots[b] for b in BONES])
        if arm == angles(as_rots(NEW[k])):
            done += 1
            continue
        assert arm == angles(as_rots(OLD[k])), (k, arm)
        new = NEW[k]
        rots[8], rots[9], rots[10] = new[0:2], new[2:3], new[3:5]
        out = words[:9] + [x for r in rots for x in r]
        assert len(out) <= size, (k, len(out), size)
        out += [0] * (size - len(out))
        struct.pack_into(f"<{size}H", d, o, *out)
    assert done in (0, POSES), done
    return None if done else bytes(d)


if __name__ == "__main__":
    path = sys.argv[1]
    out = add(open(path, "rb").read())
    if out is not None:
        open(path, "wb").write(out)
    print(f"{path}: crouch roll arm {'already set' if out is None else 'replaced'}")
