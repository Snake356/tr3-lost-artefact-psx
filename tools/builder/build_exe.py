"""Builds the builder as one Windows EXE (PyInstaller, windowed) with everything it reads at run time:
the builder's own data (patches, covers, icon) next to its modules, and the project files it uses
(IPS patches, disc layouts, pictures, conversion scripts, the command line tools) under prj/; then the
download folder (package/: the EXE, package/README.txt, the licences, the covers).

Usage: build_exe.py <out folder> [--copy-to <folder>]   (--copy-to: also put the download folder there)"""
import glob
import json
import os
import re
import shutil
import subprocess
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
PRJ = os.path.normpath(os.path.join(HERE, "..", ".."))
NAME = "TR3 Lost Artefact PS1 Builder"
OWN = ["assets", "levels", "texts", "covers", "known_inputs.json", "levels.json", "modules.json", "sounds.json",
       "texts.json", "pictures_from_disc.json", "icon.ico", "icon.png"]
TOOLS = ["close_dome_tip.py", "bake_point_light.py", "optimise_floordata.py", "share_model.py", "restore_model.py",
         "strip_unused_tail.py", "psx_inventory.py", "add_crawl_flip.py", "add_crouch_turn.py", "add_crawl_pickup.py", "fix_crouch_roll_arm.py", "add_controlled_drop.py", "add_ledge_jumps.py", "add_corner_shimmy.py", "add_climb_corners.py", "strip_dead_anims.py", "compact_frames.py", "pack_rotations.py", "psx_lara_anims.py",
         "house_nose_fix.py", "nebel_raeume.py", "kaefig_fix.py", "passport_spine.py", "leer_ende.py", "split_room.py", "drop_object.py"]
# made from the player's disc (unchanged pictures, the Kanji font with texts.kanji), never bundled
FROM_DISC = set(json.load(open(os.path.join(HERE, "pictures_from_disc.json"), encoding="utf-8"))["from_player_disc"])     | {"art/kanji/kanji_la.psx"}
EXES = ["mkpsxiso", "tr3pcpsx_builder", "texmod", "trmod", "troi", "trsfx", "tr3mesh", "3texex", "3texrep"]


def project_files():
    """Project-relative paths of the files the builder reads from the project."""
    files = ["bin/patch.sh", "levels/convert.sh", "levels/sfx/empty.wav", "art/lara/nose.bin"]
    files += [os.path.relpath(p, PRJ) for p in glob.glob(os.path.join(PRJ, "bin", "patches", "*", "*.ips"))]
    files += [os.path.relpath(p, PRJ) for p in glob.glob(os.path.join(PRJ, "levels", "geom", "*.sh"))]
    files += [os.path.relpath(p, PRJ) for p in glob.glob(os.path.join(PRJ, "levels", "japan", "*.txt"))]
    files += [os.path.relpath(p, PRJ) for p in glob.glob(os.path.join(PRJ, "disc", "files", "system", "*.cnf"))]
    files += [os.path.relpath(p, PRJ) for p in glob.glob(os.path.join(PRJ, "art", "loadscr", "*", "title*.raw"))]   # NTSC titles
    files += [f"tools/{t}" for t in TOOLS] + [f"build/win/bin/{e}.exe" for e in EXES]
    for layout in ("us", "uk", "de", "fr", "it", "sp", "jp", "jp2", "as", "as2"):
        files.append(f"disc/{layout}.xml")
        xml = open(os.path.join(PRJ, "disc", f"{layout}.xml"), encoding="utf-8").read()
        for src in re.findall(r'source="\.\./(art/[^"]+)"', xml):
            if src not in FROM_DISC and os.path.exists(os.path.join(PRJ, src)):
                files.append(src)
    return sorted(set(f.replace("\\", "/") for f in files))


def main(out):
    out = os.path.abspath(out)
    stage = os.path.join(out, "stage")                  # one folder per bundle place (the command line stays short)
    if os.path.exists(stage):
        shutil.rmtree(stage)
    for o in OWN:
        src, dst = os.path.join(HERE, o), os.path.join(stage, "own", o)
        shutil.copytree(src, dst) if os.path.isdir(src) else (os.makedirs(os.path.dirname(dst), exist_ok=True), shutil.copy(src, dst))
    files = project_files()
    for f in files:
        os.makedirs(os.path.dirname(os.path.join(stage, "prj", f)), exist_ok=True)
        shutil.copy(os.path.join(PRJ, f), os.path.join(stage, "prj", f))
    data = [f"{os.path.join(stage, 'own')}{os.pathsep}.", f"{os.path.join(stage, 'prj')}{os.pathsep}prj"]
    args = [sys.executable, "-m", "PyInstaller", "--noconfirm", "--clean", "--onefile", "--windowed",
            "--name", NAME, "--icon", os.path.join(HERE, "icon.ico"), "--paths", os.path.join(HERE, ".."),
            "--distpath", out, "--workpath", os.path.join(out, "pyinstaller_work"), "--specpath", out,
            "--hidden-import", "math", "--hidden-import", "pathlib"]
    for d in data:
        args += ["--add-data", d]
    subprocess.run(args + [os.path.join(HERE, "gui.py")], check=True)
    package(out)
    print(f"{len(files)} project files; {os.path.join(out, NAME + '.exe')}")


def package(out):
    """The download folder: the EXE, the README, the licences and the covers (no archive: Google Drive and
    others block an .exe inside a .zip)."""
    pkg = os.path.join(out, "package", NAME)
    if os.path.exists(pkg):
        shutil.rmtree(pkg)
    shutil.copytree(os.path.join(HERE, "package"), pkg)
    shutil.copy(os.path.join(out, NAME + ".exe"), pkg)
    os.makedirs(os.path.join(pkg, "covers"))
    for name, title in (("pal", "PAL"), ("usa", "USA"), ("japan", "Japan")):
        shutil.copy(os.path.join(HERE, "covers", f"{name}.png"),
                    os.path.join(pkg, "covers", f"Tomb Raider III - The Lost Artefact ({title}).png"))
    print(f"package: {pkg}")


def copy_to(out, dest):
    """Overwrites the files in dest (nothing is deleted first, so a running builder there stops only its EXE)."""
    pkg = os.path.join(out, "package", NAME)
    try:
        shutil.copytree(pkg, dest, dirs_exist_ok=True)
    except (PermissionError, shutil.Error) as e:
        sys.exit(f"{dest}: could not replace every file - close the builder there and run again ({e})")
    print(f"copied to {dest}")


if __name__ == "__main__":
    main(sys.argv[1])
    if "--copy-to" in sys.argv:
        copy_to(os.path.abspath(sys.argv[1]), sys.argv[sys.argv.index("--copy-to") + 1])
