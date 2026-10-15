"""The 371 sound samples (000.wav..370.wav, the converter's levels/sfx/common) from the player's MAIN.SFX.

Gold CD: MAIN.SFX is the 371 WAV files one after the other. Tomb Raider I-III Remastered: a table of
740 bytes, then 414 WAV files, among them the same 371 (sounds.json: which one is which). Every sample
is checked against its CRC32 in sounds.json.

Usage: sounds.py <MAIN.SFX> <out folder>"""
import json
import os
import struct
import sys
import zlib

HERE = os.path.dirname(os.path.abspath(__file__))
TABLE = json.load(open(os.path.join(HERE, "sounds.json"), encoding="utf-8"))


def split(data, p=0):
    out = []
    while p < len(data):
        if data[p:p + 4] != b"RIFF":
            raise ValueError(f"MAIN.SFX: no WAV file at byte {p}")
        n = struct.unpack_from("<I", data, p + 4)[0] + 8
        out.append(data[p:p + n])
        p += n
    return out


def samples(main_sfx):
    """[wav bytes] * 371."""
    if main_sfx[:4] == b"RIFF":
        waves = split(main_sfx)
    else:
        rem = TABLE["remastered"]
        allw = split(main_sfx, rem["header_bytes"])
        waves = [allw[i] for i in rem["sample_index"]]
    if len(waves) != len(TABLE["samples_crc32"]):
        raise ValueError(f"MAIN.SFX: {len(waves)} samples instead of {len(TABLE['samples_crc32'])}")
    for i, (w, c) in enumerate(zip(waves, TABLE["samples_crc32"])):
        if "%08x" % zlib.crc32(w) != c:
            raise ValueError(f"MAIN.SFX: sample {i} differs from the Lost Artefact sample")
    return waves


def write(main_sfx, out):
    os.makedirs(out, exist_ok=True)
    for i, w in enumerate(samples(main_sfx)):
        open(os.path.join(out, f"{i:03d}.wav"), "wb").write(w)


if __name__ == "__main__":
    write(open(sys.argv[1], "rb").read(), sys.argv[2])
    print("done:", sys.argv[2])
