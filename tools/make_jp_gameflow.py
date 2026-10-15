"""Builds tombpsx/out/jp.dat, the Japanese TOMBPSX.DAT of The Lost Artefact.

Japanese TR3 PSX texts are glyph strings: 0x40, one u16 per character (0x8000 | cell*16 | 1, cell
in KANJI.PSX), 0xFFFF; the file has no string cypher and language 4. The game/PSX strings are
taken entry by entry from the Japanese TR3 TOMBPSX.DAT (same catalogue and order as our English
lists); level and item names come from TRANSLATIONS below and are encoded with the cell table
tools/tr3_japan_kanji_cells.json (242 original TR3 cells + 13 added ones in art/kanji/kanji_la.psx).

How: tombpsx/src/jp/ is generated from tombpsx/src/us/ with LANGUAGE JAPANESE, no CYPHER_CODE and
every Japanese text replaced by a unique ASCII placeholder of exactly the byte length of its glyph
string. The normal gameflow tool builds the file, then the placeholders are overwritten in place,
so all offsets stay valid.

Usage: make_jp_gameflow.py <Japanese TR3 TOMBPSX.DAT> <gameflow.exe> [--level-select]
       --level-select adds SELECT_ANY_LEVEL (level select on New Game)."""
import json
import os
import re
import shutil
import struct
import subprocess
import sys

PRJ = os.path.join(os.path.dirname(os.path.abspath(__file__)), "..")
SRC_US = os.path.join(PRJ, "tombpsx", "src", "us")
SRC_JP = os.path.join(PRJ, "tombpsx", "src", "jp")
OUT = os.path.join(PRJ, "tombpsx", "out", "jp.dat")

TRANSLATIONS = {
    "Lara's House": "ララの家",
    "Highland Fling": "ハイランド　フリング",
    "Willard's Lair": "ウィラードの隠れ家",
    "Shakespeare Cliff": "シェイクスピア　クリフ",
    "Sleeping with the Fishes I": "魚と眠る　１",
    "Sleeping with the Fishes II": "魚と眠る　２",
    "It's a Madhouse!": "マッドハウス",
    "Reunion": "リユニオン",
    "Racetrack Key": "レースコースの鍵",
    "Crowbar": "バール",
    "Thistle Stone": "シスルストーン",
    "Cairn Key": "ケルンの鍵",
    "Pump Access Disk": "ポンプのアクセスディスク",
    "Drill Activator Card": "ドリル起動カード",
    "Circuit Bulb": "回路の電球",
    "Mutant Sample": "ミュータントのサンプル",
    "The Hand of Rathmore": "ラスモアの手",
    "Hand of Rathmore": "ラスモアの手",
    "Zoo Key": "動物園の鍵",
    "Aviary Key": "鳥小屋の鍵",
}

cells = {v: int(k) for k, v in json.load(open(os.path.join(PRJ, "tools", "tr3_japan_kanji_cells.json"), encoding="utf-8")).items()}


def encode(text):
    codes = []
    for ch in text:
        if ch not in cells:
            raise SystemExit(f"character {ch!r} is missing from the glyph table ({text})")
        codes.append(0x8000 | cells[ch] << 4 | 1)
    return b"\x40" + struct.pack(f"<{len(codes)}H", *codes) + b"\xff\xff"


def jp_block(d, table, count):
    offs = struct.unpack_from(f"<{count + 1}H", d, table)
    data = table + 2 * (count + 1)
    out = []
    for i in range(count):
        s = d[data + offs[i]:data + offs[i + 1] - 1]
        if all(32 <= c < 127 for c in s) and b"\x40" not in s:
            out.append(("text", s.decode("ascii")))
        else:
            out.append(("glyph", s))  # glyph string, possibly after an ASCII button symbol
    return out


class Tokens:
    def __init__(self):
        self.n, self.map = 0, {}

    def make(self, raw):
        """ASCII placeholder of len(raw) bytes; raw is the final glyph string."""
        assert len(raw) >= 5 and b"\x00" not in raw and len(raw) < 64, raw
        tok = f"Z{self.n:03X}".ljust(len(raw), "Q")
        self.n += 1
        self.map[tok.encode()] = raw
        return tok


def main(jp_dat, gameflow_exe, level_select=False):
    d = open(jp_dat, "rb").read()
    assert struct.unpack_from("<H", d, 2701)[0] == 92
    game = jp_block(d, 2703, 92)
    psx = jp_block(d, 4245, 80)
    T = Tokens()

    # strings file: GAME_STRINGS and PSX_STRINGS entries
    lines = open(os.path.join(SRC_US, "psx_strings.txt"), encoding="latin-1").read().split("\n")
    out, section, idx = [], None, 0
    for line in lines:
        bare = line.split("//")[0].strip()
        if bare in ("GAME_STRINGS:", "PC_STRINGS:", "PSX_STRINGS:"):
            section, idx = bare, 0; out.append(line); continue
        if bare == "END:":
            section = None; out.append(line); continue
        if section in ("GAME_STRINGS:", "PSX_STRINGS:") and bare:
            kind, val = (game if section == "GAME_STRINGS:" else psx)[idx]
            if kind == "text":
                # plain entries (memory card IDs): keep ours, with the Japanese product code
                val = bare.replace("BASLUS-00691", "BISLPM-86196")
            if bare in TRANSLATIONS:
                kind, val = "glyph", encode(TRANSLATIONS[bare])
            out.append(T.make(val) if kind == "glyph" else val)
            idx += 1
            continue
        out.append(line)
    os.makedirs(SRC_JP, exist_ok=True)
    open(os.path.join(SRC_JP, "psx_strings.txt"), "w", encoding="latin-1").write("\n".join(out))
    shutil.copy(os.path.join(SRC_US, "pc_strings.txt"), os.path.join(SRC_JP, "pc_strings.txt"))

    # script: language, no cypher, level and item names
    s = open(os.path.join(SRC_US, "tr3_gold.txt"), encoding="latin-1").read().split("\n")
    res, in_level = [], False
    for line in s:
        m = re.match(r"(\s*)(GYM|LEVEL|PUZZLE\d|KEY\d|PICKUP\d)(:\s*)([^/\t]*?)(\s*(//.*)?)$", line)
        if re.match(r"\s*CYPHER_CODE:", line):
            continue
        if re.match(r"\s*LANGUAGE:", line):
            line = re.sub(r"LANGUAGE:\s*\w+", "LANGUAGE:\tJAPANESE", line)
            if level_select:
                line += "\n   SELECT_ANY_LEVEL:"
        elif m and m.group(4).strip() in TRANSLATIONS:
            line = m.group(1) + m.group(2) + m.group(3) + T.make(encode(TRANSLATIONS[m.group(4).strip()])) + (m.group(5) or "")
        elif m and m.group(4).strip():
            raise SystemExit(f"no translation for {m.group(4).strip()!r}")
        # with level select every level may be the first one played, and the Japanese
        # game saves from the crystal in the inventory: give each level one at its start.
        # Start inventory entries only count for the level when they come before GAME
        # (those after it are handed to the next level once this one is completed).
        if level_select and re.match(r"\s*GAME:\s*data\\(?!title)", line, re.I) and in_level:
            res.append("   PSXSTARTINV:\tCRYSTAL")
        res.append(line)
        if re.match(r"\s*LEVEL:", line):
            in_level = True
        elif re.match(r"\s*END:", line):
            in_level = False
    open(os.path.join(SRC_JP, "tr3_gold.txt"), "w", encoding="latin-1").write("\n".join(res))

    # build and replace the placeholders
    subprocess.run([gameflow_exe, "tr3_gold.txt"], cwd=SRC_JP, capture_output=True)
    raw = bytearray(open(os.path.join(SRC_JP, "tombPSX.dat"), "rb").read())
    for tok, val in T.map.items():
        assert raw.count(tok) == 1, (tok, raw.count(tok))
        p = raw.index(tok); raw[p:p + len(tok)] = val
    assert raw[382] == 0 and raw[383] == 4, (raw[382], raw[383])
    open(OUT, "wb").write(raw)
    for f in ("tombPSX.dat", "tr3_gold.log", "tr3_gold.str"):
        try:
            os.remove(os.path.join(SRC_JP, f))
        except FileNotFoundError:
            pass
    print(f"{OUT}: {len(raw)} Bytes, {len(T.map)} japanische Texte eingesetzt")


if __name__ == "__main__":
    main(sys.argv[1], sys.argv[2], "--level-select" in sys.argv[3:])
