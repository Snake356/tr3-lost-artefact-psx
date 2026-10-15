"""The game texts (TOMBPSX.DAT: gameflow and all strings) and, for the Japanese and Asian discs, the Kanji
font (KANJI.PSX) from the player's own Tomb Raider III PS1 disc.

The project's files (tombpsx/out/<layout>.dat, art/kanji/kanji_la.psx) are the disc's own files with the
Lost Artefact's changes (level list, level names, the 13 added Kanji); texts.json lists the patch
(bindiff) for each layout.

Usage: texts.py <layout> <PS1 image> <out folder>"""
import json
import os
import sys

import bindiff
import inputs

HERE = os.path.dirname(os.path.abspath(__file__))
TABLE = json.load(open(os.path.join(HERE, "texts.json"), encoding="utf-8"))
JAPANESE = ("jp", "jp2", "as", "as2")


# gameflow flags (u16 at byte 374): 0x0400 level select on New Game, 0x0800 cheat codes, 0x0080 DOZY (fly)
TEST_FLAGS = 0x0C80
SAVE_FROM_INVENTORY = ("jp", "as")    # Japanese/Asian Disc 1: the game saves from a crystal in the inventory


def tombpsx(layout, disc, test=False):
    """test: level select on New Game (and cheats). The Japanese and Asian Disc 1 take their own gameflow
    (tombpsx/out/jptest.dat, astest.dat: level select and a save crystal in the inventory at every level
    start, without cheats); the others get the flags."""
    key = layout + "test" if test and layout in SAVE_FROM_INVENTORY else layout
    e = TABLE[key]
    dat = bytearray(bindiff.apply(disc.file(e["base_file"]), open(os.path.join(HERE, "texts", e["patch"]), "rb").read()))
    if test and layout not in SAVE_FROM_INVENTORY:
        flags = int.from_bytes(dat[374:376], "little") | TEST_FLAGS
        dat[374:376] = flags.to_bytes(2, "little")
    return bytes(dat)


def kanji(disc):
    e = TABLE["kanji"]
    return bindiff.apply(disc.file(e["base_file"]), open(os.path.join(HERE, "texts", e["patch"]), "rb").read())


if __name__ == "__main__":
    layout, image, out = sys.argv[1:4]
    disc, _ = inputs.open_image(image)
    os.makedirs(out, exist_ok=True)
    open(os.path.join(out, "TOMBPSX.DAT"), "wb").write(tombpsx(layout, disc))
    if layout in JAPANESE:
        open(os.path.join(out, "KANJI.PSX"), "wb").write(kanji(disc))
    print("done:", out)
