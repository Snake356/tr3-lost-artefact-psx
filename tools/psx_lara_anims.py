"""Adds the crawl flip (313), the crouch turns (314, 315), the controlled drop (316, 317), the ledge jumps (318 to 321), the corner shimmying (322 to 329), the climbing round corners (330 to 337), the smooth crawl pickup (292) and the crouch roll's arm (220) to a finished
Tomb Raider III PSX level, for Lara's home: the released HOUSE.PSX is used as it is, while our own levels
get them as PC levels (tools/add_crawl_flip.py, tools/add_crouch_turn.py, tools/add_crawl_pickup.py,
tools/fix_crouch_roll_arm.py, tools/add_controlled_drop.py, tools/add_ledge_jumps.py,
tools/add_corner_shimmy.py, tools/add_climb_corners.py). Then the animations only missing objects start
are pointed at a single pose (tools/strip_dead_anims.py).
The animation lists of a PSX level are those of
the PC level (same records, same frames); only a moveable takes 20 bytes instead of 18. The level
is padded to a multiple of 2048 again. A level that has them already is left as it is.

Usage: psx_lara_anims.py in.psx [out.psx]   (without out.psx it changes in.psx)"""
import os
import pathlib
import struct
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import add_climb_corners  # noqa: E402
import add_controlled_drop  # noqa: E402
import add_corner_shimmy  # noqa: E402
import add_crawl_flip  # noqa: E402
import add_crawl_pickup  # noqa: E402
import add_crouch_turn  # noqa: E402
import add_ledge_jumps  # noqa: E402
import fix_crouch_roll_arm  # noqa: E402
import psx_inventory  # noqa: E402
import strip_dead_anims  # noqa: E402

MODEL = 20


def sections(d, path):
    """Offsets of the count fields of the animation lists and the moveables, found as close_dome_tip.py
    finds the mesh data: behind the mesh data and the mesh pointers."""
    z = psx_inventory.inventory(pathlib.Path(path))[0]
    assert z["status"] == "ok", z["status"]
    u32 = lambda o: struct.unpack_from("<I", d, o)[0]
    nmd, nmp, na = z["meshdata"], z["meshptr"], z["anims"]
    hits = [o for o in range(0, len(d) - 12) if u32(o) == nmd and o + 8 + nmd * 2 + nmp * 4 < len(d)
            and u32(o + 4 + nmd * 2) == nmp and u32(o + 8 + nmd * 2 + nmp * 4) == na]
    assert len(hits) == 1, hits
    o = hits[0] + 8 + nmd * 2 + nmp * 4
    s = {}
    for name, size in (("anims", 32), ("states", 6), ("dispatch", 8), ("commands", 2), ("trees", 4),
                       ("frames", 2), ("moveables", MODEL)):
        s[name] = o
        o += 4 + size * u32(o)
    s["items"] = int(z["p_entities"], 16) - 4
    return s


def main(src, dst):
    d = open(src, "rb").read()
    n = len(d)
    for tool in (add_crawl_flip, add_crouch_turn, add_crawl_pickup, fix_crouch_roll_arm, add_controlled_drop,
                 add_ledge_jumps, add_corner_shimmy, add_climb_corners, strip_dead_anims):
        open(dst + ".tmp", "wb").write(d)                        # psx_inventory reads a file
        out = tool.add(d, sections(d, dst + ".tmp"), MODEL)
        os.remove(dst + ".tmp")
        if out is not None:
            d = out
    if len(d) != n:
        d += bytes(-len(d) % 2048)
    open(dst, "wb").write(d)
    print(f"{dst}: {len(d) - n:+d} bytes")


if __name__ == "__main__":
    main(sys.argv[1], sys.argv[2] if len(sys.argv) > 2 else sys.argv[1])
