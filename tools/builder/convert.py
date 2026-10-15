"""levels/convert.sh in Python, so the builder runs without a Unix shell.

The command lines themselves are read from levels/convert.sh (texmod/trmod/bake_point_light lines of the level
customisation, the level/version lists, conversion parameters and removed sound samples) and from
levels/geom/*.sh and levels/japan/*.txt, so convert.sh stays the one place to change the conversion.
The loops of convert.sh are done here in the same order, each level in its own folder and the levels side by side.

Three inputs are hooks, because the project's files are edited Core data:
  textures(level, extracted_bmp) -> bmp     levels/textures/<level>.bmp (3texrep)
  anims(level, exported_block) -> dat       levels/anims/<level>.dat (troi export / import)
  mesh(level, mesh_text) -> text             levels/mesh/<level>.txt (tr3mesh)
REPO hooks return the repository's files; assets.py has hooks that rebuild them from the player's level.

Usage (check against levels/out): convert.py <work folder> [--versions us,jp] [--levels scotland]"""
import argparse
import concurrent.futures
import os
import re
import shlex
import shutil
import subprocess
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
FROZEN = getattr(sys, "frozen", False)           # the builder EXE: the project files it needs are in prj/
PRJ = os.path.join(sys._MEIPASS, "prj") if FROZEN else os.path.normpath(os.path.join(HERE, "..", ".."))
TOOLS = os.path.join(PRJ, "tools")
BIN = os.path.join(PRJ, "build", "win", "bin")
NO_WINDOW = 0x08000000 if os.name == "nt" else 0    # no console window per tool run from the builder window


def tool_command(tool):
    """Command line of a Python tool in tools/ (the EXE runs it itself, see gui.run_tool)."""
    return [sys.executable, "--run-tool", tool] if FROZEN else [sys.executable, os.path.join(TOOLS, tool)]


def script():
    return open(os.path.join(PRJ, "levels", "convert.sh"), encoding="utf-8").read().replace("\r", "")


def setting(s, name):
    return re.search(rf'^{name}="([^"]*)"', s, re.M).group(1)


def customisation_lines(s):
    """The single command lines of the LEVEL CUSTOMISATION block, in order, with their position relative to
    the loops: ("after", loop name) markers are returned as ("loop", name)."""
    block = s[s.index("### LEVEL CUSTOMISATION ###"):s.index("### JAPAN / ASIA DISC 1 ###")]
    out = []
    for line in block.split("\n"):
        t = line.strip()
        if t.startswith(("./texmod ", "./trmod ", 'py -3 "${prjdir}/../tools/bake_point_light.py"')) and "${level}" not in t:
            out.append(("cmd", t))
        elif t.startswith("./3texrep"):
            out.append(("loop", "textures"))
        elif t.startswith("./troi"):
            out.append(("loop", "anims"))
        elif t.startswith('sh "./geom/'):
            out.append(("loop", "geom"))
        elif t.startswith("./tr3mesh"):
            out.append(("loop", "mesh"))
        elif t.startswith('fd="$(./trmod'):
            out.append(("loop", "floordata"))
    return out


REPO = {
    "textures": lambda lv, cur: open(os.path.join(PRJ, "levels", "textures", f"{lv}.bmp"), "rb").read(),
    "anims": lambda lv, cur: open(os.path.join(PRJ, "levels", "anims", f"{lv}.dat"), "rb").read(),
    "mesh": lambda lv, cur: open(os.path.join(PRJ, "levels", "mesh", f"{lv}.txt"), "rb").read(),
}


class Converter:
    def __init__(self, work, inputs, sfx_dir, hooks=REPO, levels=None, versions=None, log=print, bin_dir=BIN,
                 module_files=None):
        """inputs: folder with the seven input levels (levels/in); sfx_dir: folder with 000.wav..370.wav;
        module_files: {version: code module file} to use the converter without built-in code modules
        (tr3pcpsx_builder, see modules.py), None for the normal tr3pcpsx."""
        self.module_files = module_files
        self.s = script()
        self.work, self.inputs, self.sfx_dir, self.hooks, self.log, self.bin = work, inputs, sfx_dir, hooks, log, bin_dir
        self.levels = levels or setting(self.s, "levels").split(",")
        self.versions = versions or setting(self.s, "versions").split(",")
        self.params = dict(p.split(":", 1) for p in setting(self.s, "convparams").split(";"))
        self.nosfx = {k: [x for x in v.split(",") if x] for k, v in (p.split(":", 1) for p in setting(self.s, "nosfx").split(";"))}
        self.seen = {}                                  # hook inputs, for making the patches

    def run(self, lv, exe, *args, check=True, env=None):
        r = subprocess.run([os.path.join(self.bin, exe + ".exe")] + list(args), cwd=self.path(lv),
                           capture_output=True, text=True, errors="replace", env=env, creationflags=NO_WINDOW)
        if check and r.returncode != 0:
            raise RuntimeError(f"{exe} {' '.join(args[:3])}: exit {r.returncode}\n{r.stdout[-500:]}{r.stderr[-500:]}")
        return r.stdout

    def py(self, lv, tool, *args):
        r = subprocess.run(tool_command(tool) + list(args), cwd=self.path(lv),
                           capture_output=True, text=True, errors="replace", creationflags=NO_WINDOW)
        if r.returncode != 0:
            raise RuntimeError(f"{tool}: {r.stdout[-500:]}{r.stderr[-500:]}")

    def shell_line(self, lv, line):
        """One command line from convert.sh or a geom script."""
        argv = shlex.split(line, posix=True)
        if argv[0] == "py":
            assert argv[1] == "-3" and "bake_point_light.py" in argv[2]
            rest = [a for a in argv[3:] if a not in ("||", "exit", "1")]
            self.py(lv, "bake_point_light.py", *rest)
        else:
            self.run(lv, argv[0][2:], *argv[1:])

    def path(self, *p):
        return os.path.join(self.work, *p)

    def convert(self):
        """Each level goes through all steps in its own folder (<work>/<level>), the levels side by side;
        the results are collected in <work>/out/<version>/<level>.psx."""
        if os.path.exists(self.work):
            shutil.rmtree(self.work)
        for ver in self.versions:
            os.makedirs(self.path("out", ver))
        self.log(f"converting {len(self.levels)} levels side by side")
        with concurrent.futures.ThreadPoolExecutor(min(len(self.levels), os.cpu_count() or 1)) as pool:
            for f in concurrent.futures.as_completed([pool.submit(self.level, lv) for lv in self.levels]):
                self.log(f"  {f.result()} done")

    def level(self, lv):
        os.makedirs(self.path(lv))
        shutil.copy(os.path.join(self.inputs, f"{lv}.tr2"), self.path(lv, f"{lv}.tr2"))
        self.sfx(lv)
        for kind, what in customisation_lines(self.s):
            if kind == "cmd":
                if re.search(r"(\w+)\.tr2", what).group(1) == lv:
                    self.shell_line(lv, what)
            else:
                getattr(self, "step_" + what)(lv)
        self.japan(lv)
        for ver in self.versions:
            os.makedirs(self.path(lv, "out", ver), exist_ok=True)
            src = f"{lv}_jp.tr2" if ver in ("jp", "as") else f"{lv}.tr2"
            exe, env = "tr3pcpsx", None
            if self.module_files:
                exe, env = "tr3pcpsx_builder", dict(os.environ, TR3PCPSX_MODULES=os.path.abspath(self.module_files[ver]))
            self.run(lv, exe, src, f"./out/{ver}/{lv}.psx", f"./outsfx/{ver}/{lv}.sfx", "-v",
                     *self.params[lv].split(), f"-{ver}", env=env)
            shutil.move(self.path(lv, "out", ver, f"{lv}.psx"), self.path("out", ver, f"{lv}.psx"))
        return lv

    def sfx(self, lv):
        empty = os.path.join(PRJ, "levels", "sfx", "empty.wav")
        tmp = self.path(lv, "tmpsfx")
        shutil.copytree(self.sfx_dir, tmp)
        for sample in self.nosfx.get(lv, []):
            shutil.copy(empty, os.path.join(tmp, f"{sample}.wav"))
        for ver in self.versions:
            os.makedirs(self.path(lv, "outsfx", ver), exist_ok=True)
            self.run(lv, "trsfx", "./tmpsfx", f"./outsfx/{ver}/{lv}.sfx")
        shutil.rmtree(tmp)

    def step_textures(self, lv):
        self.run(lv, "3texex", f"{lv}.tr2", "cur.bmp")
        cur = open(self.path(lv, "cur.bmp"), "rb").read()
        self.seen[("textures", lv)] = cur
        open(self.path(lv, "tex.bmp"), "wb").write(self.hooks["textures"](lv, cur))
        self.run(lv, "3texrep", f"{lv}.tr2", "tex.bmp")
        for f in ("cur.bmp", "tex.bmp"):
            os.remove(self.path(lv, f))

    def step_anims(self, lv):
        self.run(lv, "troi", f"{lv}.tr2", "e", "cur.dat")
        cur = open(self.path(lv, "cur.dat"), "rb").read()
        os.remove(self.path(lv, "cur.dat"))
        self.seen[("anims", lv)] = cur
        open(self.path(lv, "anim.dat"), "wb").write(self.hooks["anims"](lv, cur))
        self.run(lv, "troi", f"{lv}.tr2", "i", "anim.dat")
        os.remove(self.path(lv, "anim.dat"))

    def step_geom(self, lv):
        for line in open(os.path.join(PRJ, "levels", "geom", f"{lv}.sh"), encoding="utf-8"):
            if line.strip().startswith("./"):
                self.shell_line(lv, line.strip())

    def step_mesh(self, lv):
        cur = subprocess.run([os.path.join(self.bin, "tr3mesh.exe"), f"{lv}.tr2"], cwd=self.path(lv),
                             capture_output=True, check=True, creationflags=NO_WINDOW).stdout
        self.seen[("mesh", lv)] = cur
        open(self.path(lv, "mesh.txt"), "wb").write(self.hooks["mesh"](lv, cur))
        self.run(lv, "tr3mesh", f"{lv}.tr2", "mesh.txt")
        os.remove(self.path(lv, "mesh.txt"))

    def step_floordata(self, lv):
        self.py(lv, "optimise_floordata.py", os.path.join(self.bin, "trmod.exe"), f"{lv}.tr2")

    def japan(self, lv):
        """The Japan/Asia Disc 1 level (<level>_jp.tr2)."""
        if not {"jp", "as"} & set(self.versions):
            return
        shutil.copy(self.path(lv, f"{lv}.tr2"), self.path(lv, f"{lv}_jp.tr2"))
        if lv in ("scotland", "chunnel"):
            self.py(lv, "share_model.py", f"{lv}_jp.tr2", f"{lv}_jp.tr2", "167:192")
        if lv == "chunnel":
            self.py(lv, "restore_model.py", os.path.join(self.inputs, "chunnel.tr2"), "chunnel_jp.tr2", "chunnel_jp.tr2", "162")
        p = os.path.join(PRJ, "levels", "japan", f"{lv}.txt")
        if os.path.exists(p):
            cmds = [l.rstrip("\r\n") for l in open(p, encoding="utf-8") if l.strip() and not l.startswith("#")]
            for i in range(0, len(cmds), 60):
                self.run(lv, "trmod", "tr3pc", f"{lv}_jp.tr2", *cmds[i:i + 60])

if __name__ == "__main__":
    ap = argparse.ArgumentParser()
    ap.add_argument("work")
    ap.add_argument("--levels")
    ap.add_argument("--versions")
    a = ap.parse_args()
    c = Converter(a.work, os.path.join(PRJ, "levels", "in"), os.path.join(PRJ, "levels", "sfx", "common"),
                  levels=a.levels.split(",") if a.levels else None, versions=a.versions.split(",") if a.versions else None)
    c.convert()
    same = diff = 0
    for ver in c.versions:
        for lv in c.levels:
            mine = open(c.path("out", ver, f"{lv}.psx"), "rb").read()
            ref = open(os.path.join(PRJ, "levels", "out", ver, f"{lv}.psx"), "rb").read()
            if mine == ref:
                same += 1
            else:
                diff += 1
                print(f"DIFFERENT: {ver}/{lv}.psx")
    print(f"{same} same as levels/out, {diff} different")
