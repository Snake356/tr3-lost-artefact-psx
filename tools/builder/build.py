"""Builds the Lost Artefact PS1 disc image from the player's own games:
a Tomb Raider III PS1 disc image (the language of the disc to build) and the Lost Artefact PC files
(Gold CD image or folder, Tomb Raider I-III Remastered folder).

Options:
  --ntsc <US image>   a European disc (German, French, Italian, Spanish, English UK) as NTSC (60 Hz):
                      the US executable with the language's speech and texts; needs the US Tomb Raider
                      III disc as well. The layout is the language's PAL layout turned to NTSC (ntsc_xml)
  --blood red|purple  blood colour in every language (default: as the game version has it; the German
                      versions show purple blood)
  --level-select      level select on New Game (and cheats, except on the Japanese and Asian Disc 1,
                      which get a save crystal at every level start instead); also --test
  --japan-weapons     Lara's weapons as strong as in the Japanese Disc 1: double damage, stronger rocket
                      and grenade explosions (japanweapons.ips; the Japanese and Asian Disc 1 have it)
  --japan-crystals    save crystals as in the Japanese Disc 1: a crystal in a level heals Lara, saving from the
                      crystal in the inventory does not use it up (japancrystals.ips; the Japanese and Asian
                      Disc 1 have it)

Steps: disc files from the image (disc_files.py), input levels (pc_levels.py), sounds (sounds.py), code
modules (modules.py), level conversion (convert.py with assets.py's hooks), executable (the IPS patches
of bin/patch.sh, then the options), texts (texts.py), pictures (the project's own, the unchanged
originals from the disc), the licence sectors of the disc, then mkpsxiso with disc/<layout>.xml. The cover
(covers/: PAL, USA or Japan by the disc's region) goes next to the disc image with the same name.

Usage: build.py <TR3 PS1 image> <Lost Artefact PC files> <out .bin> [--ntsc <US image>] [--blood red|purple] [--test]
                [--japan-weapons] [--japan-crystals]"""
import argparse
import json
import os
import re
import shutil
import subprocess
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, ".."))           # tools/ (in the EXE: bundled modules)
import assets           # noqa: E402
import convert          # noqa: E402
import disc_files       # noqa: E402
import extract          # noqa: E402
import inputs           # noqa: E402
import make_ntsc_exe    # noqa: E402
import modules          # noqa: E402
import pc_levels        # noqa: E402
import sounds           # noqa: E402
import texts            # noqa: E402
from make_reunionview_ips import apply_ips   # noqa: E402

PRJ = convert.PRJ
LAYOUT = {"us": "us", "ukrev": "uk", "derev": "de", "frrev": "fr", "itrev": "it", "sprev": "sp",
          "jp": "jp", "jp2": "jp2", "as": "as", "as2": "as2",
          "us10": "us", "uk": "uk", "de": "de", "fr": "fr", "it": "it", "sp": "sp"}     # Rev 0
NTSC_LANG = {"derev": "de", "frrev": "fr", "itrev": "it", "sprev": "sp", "ukrev": "en",    # speech/heading language
             "de": "de", "fr": "fr", "it": "it", "sp": "sp", "uk": "en"}                      # Rev 0
COVERS = {"us": "usa.png", "us10": "usa.png", "jp": "japan.png", "jp2": "japan.png", "as": "japan.png",
          "as2": "japan.png"}                               # every other (European) disc: pal.png, also as NTSC
PICTURES = json.load(open(os.path.join(HERE, "pictures_from_disc.json"), encoding="utf-8"))["from_player_disc"]


JAPAN_WEAPONS_BUILT_IN = ("jp", "as")                 # the Japanese and Asian Disc 1 have it themselves


def patched_exe(disc, info, blood=None, japan_weapons=False, ntsc_lang=None, japan_crystals=False):
    """japan_weapons: Lara's weapons as strong as in the Japanese Disc 1 (japanweapons.ips); ntsc_lang: the
    US executable of a European NTSC disc gets the savegame title and the New Game+ texts in that language
    (savename_<lang>.ips, ngpchoice_<lang>.ips)."""
    exe = disc.file(info["exe"])
    sh = open(os.path.join(PRJ, "bin", "patch.sh"), encoding="utf-8").read()
    for name in re.search(r'^patches="([^"]*)"', sh, re.M).group(1).split(","):
        if name in ("savename", "ngpchoice") and ntsc_lang:
            name = f"{name}_{ntsc_lang}"
        exe = apply_ips(exe, open(os.path.join(PRJ, "bin", "patches", info["version"], f"{name}.ips"), "rb").read())
    if blood:
        exe = apply_ips(exe, open(os.path.join(PRJ, "bin", "patches", info["version"], f"blood{blood}.ips"), "rb").read())
    if japan_weapons and info["version"] not in JAPAN_WEAPONS_BUILT_IN:
        exe = apply_ips(exe, open(os.path.join(PRJ, "bin", "patches", info["version"], "japanweapons.ips"), "rb").read())
    if japan_crystals and info["version"] not in JAPAN_WEAPONS_BUILT_IN:
        exe = apply_ips(exe, open(os.path.join(PRJ, "bin", "patches", info["version"], "japancrystals.ips"), "rb").read())
    return exe


NTSC_CARD = b"BASLUS-00691TR3G"
REV0 = ("de", "fr", "it", "sp", "uk", "us10")        # the first release of each disc (executable)


def ntsc_card_name(dat, lang):
    """The memory card file of a European NTSC disc: LANGUAGE AMERICAN picks the US name; it gets the
    language's letter instead of the 3 (BASLUS-00691TRFG for French), so its savegames are kept apart from
    the US disc's and the PAL disc's. The strings are XOR-ed with the cypher byte at 382."""
    key = dat[382]
    body = bytes(b ^ key for b in dat[384:])
    assert body.count(NTSC_CARD) == 1, "US memory card name not found"
    new = NTSC_CARD[:-2] + {"en": b"E", "de": b"D", "fr": b"F", "it": b"I", "sp": b"S"}[lang] + NTSC_CARD[-1:]
    return dat[:384] + bytes(b ^ key for b in body.replace(NTSC_CARD, new))


def rev0_card_names(dat):
    """The memory card files of a game run by a first release executable (Rev 0): T0 instead of TR in every
    card name (BESLES-01683T03G, BASLUS-00691T0DG for a German NTSC disc on US Rev 0). Rev 0 and Rev 1 share
    the names, but their level memory starts elsewhere (and the European savegames are 20 bytes apart), so a
    current position saved by the other release is read wrongly and can crash the game. The last letter stays,
    it tells the normal savegame (G) from the New Game+ one (P)."""
    key = dat[382]
    body = bytearray(b ^ key for b in dat[384:])
    names = list(re.finditer(rb"B[A-Z]S[A-Z]{2}[SM]-\d{5}TR[A-Z0-9]{2}", body))
    assert names, "memory card names not found"
    for m in names:
        body[m.start() + 13] = ord("0")
    return dat[:384] + bytes(b ^ key for b in body)


def ntsc_xml(xml, layout):
    """The NTSC layout of a European language's PAL layout:
    US licence, SYSTEM.CNF and executable name, PIXUS pictures, the US disc's data files and levels, and the title
    picture of 240 lines with black borders (the PAL one fills all 256 lines, as Tomb Raider III's PAL title)."""
    rep = [('<license file="./licenses/licensee.dat"/>', '<license file="./licenses/licensea.dat"/>'),
           ('source="./files/system/' + layout + '.cnf"', 'source="./files/system/us.cnf"'),
           ('<dir name="PIXUK">', '<dir name="PIXUS">'), ('name="TITLEUK.RAW"', 'name="TITLEUS.RAW"'),
           ('name="LEGALUK.RAW"', 'name="LEGALUS.RAW"'), ("./files/data/uk/", "./files/data/us/")]
    for old, new in rep:
        assert old in xml, f"disc/{layout}.xml: {old} not found"
        xml = xml.replace(old, new)
    xml = re.sub(r'<file name="SLES_\d{3}\.\d{2}" type="data"', '<file name="SLUS_006.91" type="data"', xml)
    xml = re.sub(r"(art/loadscr/\w+/title\w+)_pal\.raw", r"\1.raw", xml)   # NTSC shows 240 lines: the title with its borders
    xml = re.sub(r"\.\./levels/out/\w+rev/", "../levels/out/us/", xml)
    return xml


def licence(disc):
    """The licence sectors (0-11) in mkpsxiso's form: subheader, 2048 data bytes, empty EDC/ECC."""
    out = bytearray()
    for s in range(12):
        disc.f.seek(s * 2352 + 24)
        out += bytes.fromhex("0000080000000800") + disc.f.read(2048) + bytes(280)
    return bytes(out)


def build(ps1_image, pc_source, out_bin, ntsc_image=None, blood=None, work=None, log=print, test=False, keep_work=False,
          japan_weapons=False, japan_crystals=False):
    """The work folder (intermediate files, about 1 GB) is removed after a successful build unless keep_work."""
    try:
        return _build(ps1_image, pc_source, out_bin, ntsc_image, blood, work, log, test, keep_work, japan_weapons,
                      japan_crystals)
    finally:
        inputs.close_unpacked()


def _build(ps1_image, pc_source, out_bin, ntsc_image, blood, work, log, test, keep_work, japan_weapons=False,
           japan_crystals=False):
    work = os.path.abspath(work or os.path.join(os.path.dirname(os.path.abspath(out_bin)), "build_work"))
    if os.path.exists(work):
        shutil.rmtree(work)
    os.makedirs(work)
    w = lambda *p: os.path.join(work, *p)

    disc, how = inputs.open_image(ps1_image, w("images"))
    info = inputs.recognise_ps1(disc)
    if info is None:
        raise ValueError(f"{how}: not a known Tomb Raider III PS1 disc")
    if not info["supported"]:
        raise ValueError(f"{info['redump']}: this revision is not supported yet")
    layout = LAYOUT[info["version"]]
    main_disc, main_info = disc, info                      # the disc the game runs from
    if ntsc_image:
        if info["version"] not in NTSC_LANG:
            raise ValueError("NTSC is available for the European discs (German, French, Italian, Spanish, English UK)")
        main_disc, how2 = inputs.open_image(ntsc_image, w("images"))
        main_info = inputs.recognise_ps1(main_disc)
        if main_info is None or main_info["version"] not in ("us", "us10") or not main_info["supported"]:
            raise ValueError(f"{how2}: needs the US Tomb Raider III disc (the NTSC executable)")
    log(f"Tomb Raider III {info['language']} {info['revision']} -> layout {layout}" + (" (NTSC)" if ntsc_image else ""))

    log("disc files")
    extract.extract(main_disc, main_info, w("ex_main"))
    disc_files.build(w("ex_main"), main_info["version"], w("disc"))
    if ntsc_image:                                         # the language's own speech (F11-F15)
        extract.extract(disc, info, w("ex_lang"))
        disc_files.build(w("ex_lang"), info["version"], w("disc_lang"))
        speech = disc_files.LAYOUT[info["version"]][0]
        shutil.rmtree(w("disc", "files", "xa", speech), ignore_errors=True)
        shutil.copytree(w("disc_lang", "files", "xa", speech), w("disc", "files", "xa", speech))

    log("Lost Artefact PC files")
    pc = inputs.pc_files_from_folder(pc_source) if os.path.isdir(pc_source) else inputs.pc_files_from_disc(inputs.open_image(pc_source, w("images"))[0])
    missing = [n for n in [l + ".tr2" for l in inputs.LEVELS] + ["main.sfx"] if n not in pc]
    if missing:
        raise ValueError(f"{pc_source}: missing {', '.join(missing)}" + (" (MAIN.SFX holds the sounds)" if "main.sfx" in missing else ""))
    os.makedirs(w("in"))
    for name, data in pc_levels.prepare(pc).items():
        open(w("in", name), "wb").write(data)
    sounds.write(pc["main.sfx"], w("sfx"))

    conv_version = main_info["version"]
    open(w("modules.bin"), "wb").write(modules.module_file(main_disc, conv_version))
    log("converting the levels")
    c = convert.Converter(w("conv"), w("in"), w("sfx"), hooks=assets.HOOKS, versions=[conv_version],
                          module_files={conv_version: w("modules.bin")}, log=lambda m: log("  " + m))
    c.convert()

    log("executable, texts, pictures")
    exe = patched_exe(main_disc, main_info, blood, japan_weapons, NTSC_LANG[info["version"]] if ntsc_image else None,
                      japan_crystals)
    dat = bytearray(texts.tombpsx(layout, disc, test))
    if ntsc_image:
        exe = make_ntsc_exe.make(exe, patched_exe(disc, info), NTSC_LANG[info["version"]])
        dat[383] = 3                                       # LANGUAGE AMERICAN (no German blood censorship)
        dat = ntsc_card_name(dat, NTSC_LANG[info["version"]])
    if main_info["version"] in REV0:                       # the executable decides: an NTSC disc's is the US one
        dat = bytearray(rev0_card_names(dat))
    open(w("game.exe"), "wb").write(exe)
    open(w("TOMBPSX.DAT"), "wb").write(bytes(dat))
    if layout in texts.JAPANESE:
        open(w("KANJI.PSX"), "wb").write(texts.kanji(disc))
    open(w("licence.dat"), "wb").write(licence(main_disc))

    xml = open(os.path.join(PRJ, "disc", f"{layout}.xml"), encoding="utf-8").read()
    if ntsc_image:
        xml = ntsc_xml(xml, layout)
    pix_disc = disc                                        # the language's own pictures (INITLOAD.RAW)

    def source(m):
        src = m.group(1)
        if src.startswith("./files/system/"):
            path = os.path.join(PRJ, "disc", src[2:])
        elif src.startswith("./files/"):
            path = w("disc", src[2:])
        elif src.startswith("./licenses/"):
            path = w("licence.dat")
        elif src.startswith("../bin/mod/"):
            path = w("game.exe")
        elif src.startswith("../tombpsx/out/"):
            path = w("TOMBPSX.DAT")
        elif src == "../art/kanji/kanji_la.psx":
            path = w("KANJI.PSX")
        elif src.startswith("../levels/out/"):
            lv = src.split("/")[-1]
            path = w("conv", "out", conv_version, lv)     # the version converted (Rev 0 has its own)
        elif src[3:] in PICTURES:
            path = w("pix", os.path.basename(src))
            os.makedirs(w("pix"), exist_ok=True)
            open(path, "wb").write(pix_disc.file(PICTURES[src[3:]]))
        elif src.startswith("../art/"):
            path = os.path.join(PRJ, src[3:])
        else:
            raise ValueError(f"disc/{layout}.xml: unknown source {src}")
        if not os.path.exists(path):
            raise ValueError(f"missing {path}")
        return f'source="{os.path.abspath(path)}"' if m.group(0).startswith("source") else f'file="{os.path.abspath(path)}"'
    xml = re.sub(r'source="([^"]+)"', source, xml)
    xml = re.sub(r'<license file="([^"]+)"', lambda m: '<license ' + source(m), xml)
    out_bin = os.path.abspath(out_bin)
    xml = re.sub(r'image_name="[^"]*" cue_sheet="[^"]*"',
                 f'image_name="{os.path.basename(out_bin)}" cue_sheet="{os.path.basename(out_bin)[:-4]}.cue"', xml)
    open(w("disc.xml"), "w", encoding="utf-8").write(xml)
    log("writing the disc image")
    r = subprocess.run([os.path.join(convert.BIN, "mkpsxiso.exe"), "-y", w("disc.xml")], cwd=os.path.dirname(out_bin),
                       capture_output=True, text=True, errors="replace", creationflags=convert.NO_WINDOW)
    if r.returncode != 0 or not os.path.exists(out_bin):
        raise RuntimeError(f"mkpsxiso failed:\n{r.stdout[-1500:]}{r.stderr[-500:]}")
    cover = os.path.join(HERE, "covers", COVERS.get(info["version"], "pal.png"))
    shutil.copyfile(cover, out_bin[:-4] + ".png")         # found by frontends next to the disc image
    if not keep_work:
        inputs.close_unpacked()
        shutil.rmtree(work, ignore_errors=True)
    log(f"done: {out_bin}")
    return out_bin


def main(argv=None):
    ap = argparse.ArgumentParser()
    ap.add_argument("ps1_image")
    ap.add_argument("pc_files")
    ap.add_argument("out_bin")
    ap.add_argument("--ntsc", metavar="US_IMAGE")
    ap.add_argument("--blood", choices=("red", "purple"))
    ap.add_argument("--work")
    ap.add_argument("--level-select", "--test", dest="test", action="store_true")
    ap.add_argument("--keep-work", action="store_true", help="keep the intermediate files")
    ap.add_argument("--japan-weapons", action="store_true", help="weapons as strong as in the Japanese Disc 1")
    ap.add_argument("--japan-crystals", action="store_true", help="save crystals as in the Japanese Disc 1")
    a = ap.parse_args(argv)
    build(a.ps1_image, a.pc_files, a.out_bin, a.ntsc, a.blood, a.work, test=a.test, keep_work=a.keep_work,
          japan_weapons=a.japan_weapons, japan_crystals=a.japan_crystals)


if __name__ == "__main__":
    main()
