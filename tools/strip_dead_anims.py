"""Frees the poses of Lara's animations that a Tomb Raider III level can never play, because only the code of an
object starts them and the level has no item of that object: the underwater switch (129, object 130), the key
(131, key holes 232 to 235), the spike death (149, spikes 87), the small switch (195, 196, object 118), the push
button (197, object 128), the zip line (214 to 216, object 127), pushing and pulling a block (120 to 123,
movable blocks 97 to 100), the puzzle item (134, puzzle holes 213 to 216) and the rolling ball death (139,
rolling balls 88, 89, 112 and 126). These objects are the ones the executable gives SwitchCollision2,
KeyHoleCollision, SpikeCollision, SwitchCollision (per object number), DeathSlideCollision,
MovableBlockCollision, PuzzleHoleCollision and RollingBallCollision. The level's own item list decides.

The animation numbers are fixed in the executable, so the records stay. Such an animation takes the poses of
animation 11 (Lara standing, two poses: its frame offset and pose size) and frame rate 255, so it uses two
poses at most (its frames are 146 at most); its frame numbers, next animation, state changes and commands
stay, and nothing reads behind the poses. compact_frames.py, which runs next, then removes the poses no animation uses any more.

Before that every animation freed is checked to be reached only from animations freed with it or by a state
change to a state that only the missing object sets (40 underwater switch, 42 key, 70 zip line, 36 to 38 block,
43 puzzle item). A level that has them freed already is left as it is.

Usage: strip_dead_anims.py <level.tr2>   (changes the file in place)"""
import struct
import sys

from drop_object import sections

DONOR, RATE = 11, 255
GROUPS = [((130,), (40,), (129,)),
          ((232, 233, 234, 235), (42,), (131,)),
          ((87,), (), (149,)),
          ((118,), (), (195, 196)),
          ((128,), (), (197,)),
          ((127,), (70,), (214, 215, 216)),
          ((97, 98, 99, 100), (36, 37, 38), (120, 121, 122, 123)),
          ((213, 214, 215, 216), (43,), (134,)),
          ((88, 89, 112, 126), (), (139,))]


def dead(d, s):
    """The animations and the states of the objects the level has no item of."""
    u = lambda f, o: struct.unpack_from(f, d, o)
    types = {u("<h", s["items"] + 4 + 24 * i)[0] for i in range(u("<I", s["items"])[0])}
    assert 0 in types, "no Lara in the item list"
    anims, states = [], set()
    for objects, st, an in GROUPS:
        if not types & set(objects):
            anims += an
            states |= set(st)
    return sorted(anims), states


def check_unreached(d, s, stride, anims, states):
    u = lambda f, o: struct.unpack_from(f, d, o)
    movs = [u("<IHHIIH", s["moveables"] + 4 + stride * i) for i in range(u("<I", s["moveables"])[0])]
    lara = min(m[5] for m in movs if m[0] != 0)
    assert movs[0][0] == 0 and movs[0][5] == 0 and lara > max(anims), f"Lara has {lara} animations"
    for i in range(lara):
        if i in anims:
            continue
        a = s["anims"] + 4 + 32 * i
        assert u("<H", a + 20)[0] not in anims, f"animation {i} goes on with {u('<H', a + 20)[0]}"
        changes, first = u("<HH", a + 24)
        for c in range(changes):
            state, ranges, r = u("<HHH", s["states"] + 4 + 6 * (first + c))
            for k in range(ranges):
                target = u("<H", s["dispatch"] + 4 + 8 * (r + k) + 4)[0]
                assert target not in anims or state in states, f"animation {i} changes to {target} (state {state})"


def add(d, s=None, stride=18, report=None):
    """s: offsets of the count fields, with the item list (default: a PC level); stride: bytes per moveable
    (PSX: 20). report (a dict) gets the animations freed."""
    s = s or sections(d)
    report = {} if report is None else report
    anims, states = dead(d, s)
    report["anims"] = anims
    donor = s["anims"] + 4 + 32 * DONOR
    offset, rate, size = struct.unpack_from("<IBB", d, donor)
    base, end = struct.unpack_from("<HH", d, donor + 16)
    assert (end - base) // rate + 1 + ((end - base) % rate != 0) >= 2, "animation 11 uses one pose"
    todo = [i for i in anims if struct.unpack_from("<IBB", d, s["anims"] + 4 + 32 * i) != (offset, RATE, size)]
    if not todo:
        return None
    check_unreached(d, s, stride, anims, states)
    d = bytearray(d)
    for i in todo:
        a = s["anims"] + 4 + 32 * i
        base, end = struct.unpack_from("<HH", d, a + 16)
        assert 0 <= end - base < RATE, (i, base, end)
        struct.pack_into("<IBB", d, a, offset, RATE, size)
    return bytes(d)


if __name__ == "__main__":
    path = sys.argv[1]
    report = {}
    out = add(open(path, "rb").read(), report=report)
    if out is not None:
        open(path, "wb").write(out)
    names = ", ".join(map(str, report["anims"])) or "none"
    print(f"{path}: animations {names} {'freed already' if out is None else 'freed'}")
