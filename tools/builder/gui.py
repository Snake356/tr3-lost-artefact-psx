"""Tomb Raider III: The Lost Artefact - PS1 disc builder (window).

Pick your Tomb Raider III PS1 disc image and your Lost Artefact PC files; the builder recognises them and
writes the Lost Artefact PS1 disc image (.bin/.cue). Nothing of the games is included in the builder."""
import os
import platform
import queue
import re
import sys
import urllib.parse
import webbrowser
import threading
import time
import tkinter as tk
from tkinter import filedialog, ttk

import subprocess

if os.name == "nt":
    # every program started from the builder (also by the project's Python tools, which start trmod and
    # others themselves) runs without a console window: no windows popping up, none that can be closed
    # by mistake (that stopped the build), and no input from a console
    _popen_init = subprocess.Popen.__init__

    def _hidden_popen(self, *args, **kwargs):
        kwargs["creationflags"] = kwargs.get("creationflags", 0) | 0x08000000      # CREATE_NO_WINDOW
        if kwargs.get("stdin") is None:
            kwargs["stdin"] = subprocess.DEVNULL
        _popen_init(self, *args, **kwargs)
    subprocess.Popen.__init__ = _hidden_popen

import build  # noqa: E402
import inputs  # noqa: E402

TITLE = "Tomb Raider III: The Lost Artefact - PS1 Builder"
VERSION = "1.0"
ISSUES = "https://github.com/Snake356/tr3-lost-artefact-psx/issues/new"
HERE = getattr(sys, "_MEIPASS", os.path.dirname(os.path.abspath(__file__)))

BG, CARD, LINE = "#1b1d22", "#24272d", "#3a3d44"
TEXT, MUTED = "#e6e2d8", "#9a958a"
ACCENT, ACCENT_HOVER, ON_ACCENT = "#4a90d9", "#5ea0e6", "#0f1a26"
OK, BAD = "#a9d79a", "#e88a84"
FONT = "Segoe UI"

# build steps as build.build logs them: (log line, step name, typical seconds of the EXE; for the bar and the
# time left). NTSC reads a second disc, the Japanese/Asian Disc 1 converts a second set of levels (typical()).
STEPS = [("disc files", "Reading your PS1 disc", 13), ("Lost Artefact PC files", "Reading the PC files", 0.5),
         ("converting the levels", "Converting the levels", 35), ("executable, texts, pictures",
         "Executable, texts and pictures", 0.5), ("writing the disc image", "Writing the disc image", 3)]


def clock(seconds):
    seconds = int(round(seconds))
    return f"{seconds // 60}:{seconds % 60:02d}"
LEVEL_NAMES = {"scotland": "Highland Fling", "willsden": "Willard's Lair", "chunnel": "Shakespeare Cliff",
               "undersea": "Sleeping with the Fishes", "underse2": "Sleeping with the Fishes (part 2)",
               "zoo": "It's a Madhouse!", "slinc": "Reunion"}

CREDITS = [
    ("PS1 port", "b122251", ""),
    ("Carried the port on", "FindingTheLost", "Listed many of the bugs fixed since and came up with splitting "
                                              "Sleeping with the Fishes in two to stay within the PS1's limits."),
    ("Continued and finished by", "XeNoN3560", ""),
    ("Japanese/Asian Disc 1", "XeNoN3560", "The easier levels, adapted by hand as the original Japanese TR3 had them."),
    ("PS1 disc builder", "XeNoN3560", ""),
]
ORIGIN = ("Continues b122251's original project: gitlab.com/b122251/tr3gpsx\n\n"
          "Claude (Anthropic) was used as an AI assistant to help finish the project.")
LEGAL = ("Tomb Raider III and The Lost Artefact (c) Core Design / Eidos Interactive; the rights are now "
         "held by Crystal Dynamics. This is a fan project and not affiliated with them. No game data is "
         "included: the builder needs your own copies of both games.")


def style(root):
    s = ttk.Style(root)
    s.theme_use("clam")
    root.configure(background=BG)
    s.configure(".", background=BG, foreground=TEXT, fieldbackground=CARD, bordercolor=LINE, lightcolor=CARD,
                darkcolor=CARD, troughcolor=LINE, focuscolor=ACCENT, font=(FONT, 10))
    s.configure("TFrame", background=BG)
    s.configure("Card.TFrame", background=CARD)
    s.configure("Head.TFrame", background=CARD)
    s.configure("TLabel", background=BG, foreground=TEXT)
    s.configure("Card.TLabel", background=CARD)
    s.configure("Head.TLabel", background=CARD, font=(FONT, 14, "bold"), foreground="#f1ead6")
    s.configure("Sub.TLabel", background=CARD, foreground=MUTED, font=(FONT, 9))
    s.configure("Step.TLabel", background=CARD, font=(FONT, 10, "bold"))
    s.configure("Muted.TLabel", background=CARD, foreground=MUTED)
    s.configure("MutedBg.TLabel", background=BG, foreground=MUTED, font=(FONT, 9))
    s.configure("TButton", background=CARD, foreground=TEXT, bordercolor="#555a62", padding=(10, 4))
    s.map("TButton", background=[("active", LINE)], foreground=[("disabled", MUTED)])
    s.configure("Accent.TButton", background=ACCENT, foreground=ON_ACCENT, bordercolor=ACCENT,
                font=(FONT, 10, "bold"), padding=(18, 7))
    s.map("Accent.TButton", background=[("disabled", LINE), ("active", ACCENT_HOVER)],
          foreground=[("disabled", MUTED)])
    s.configure("Info.TButton", background=CARD, foreground=ACCENT, bordercolor=CARD, font=(FONT, 11, "bold"),
                padding=(8, 2))
    s.map("Info.TButton", background=[("active", LINE)])
    s.configure("Link.TButton", background=CARD, foreground=MUTED, bordercolor=CARD, padding=(0, 0))
    s.map("Link.TButton", background=[("active", CARD)], foreground=[("active", TEXT)])
    s.configure("TEntry", fieldbackground=BG, foreground=TEXT, insertcolor=TEXT, bordercolor=LINE, padding=4)
    s.map("TEntry", fieldbackground=[("disabled", CARD)], foreground=[("disabled", MUTED)])
    s.configure("Card.TCheckbutton", background=CARD, foreground=TEXT, indicatorbackground=BG,
                indicatorforeground=ACCENT, upperbordercolor=LINE, lowerbordercolor=LINE)
    s.map("Card.TCheckbutton", background=[("active", CARD)], foreground=[("disabled", MUTED)],
          indicatorbackground=[("disabled", CARD)])
    s.configure("Seg.Toolbutton", background=BG, foreground=TEXT, bordercolor=LINE, padding=(12, 3))
    s.map("Seg.Toolbutton", background=[("selected", ACCENT), ("active", LINE)],
          foreground=[("selected", ON_ACCENT)])
    s.configure("Horizontal.TProgressbar", background=ACCENT, troughcolor=LINE, bordercolor=LINE,
                lightcolor=ACCENT, darkcolor=ACCENT, thickness=8)


class App(tk.Tk):
    def __init__(self):
        super().__init__()
        self.title(TITLE)
        self.minsize(760, 470)
        try:
            self.iconbitmap(default=os.path.join(HERE, "icon.ico"))
        except tk.TclError:                     # not Windows
            self.iconphoto(True, tk.PhotoImage(file=os.path.join(HERE, "icon.png")))
        style(self)
        self.msgs = queue.Queue()
        self.ps1_info = None
        self.ps1, self.pc, self.us, self.out = tk.StringVar(), tk.StringVar(), tk.StringVar(), tk.StringVar()
        self.ntsc, self.blood, self.test = tk.BooleanVar(), tk.StringVar(value="default"), tk.BooleanVar()
        self.step_text, self.detail_text, self.time_text = tk.StringVar(value="Ready"), tk.StringVar(), tk.StringVar()
        self.t0 = None                                  # start of the running build (time.monotonic)

        head = ttk.Frame(self, style="Head.TFrame", padding=(16, 10))
        head.pack(fill="x")
        tk.Frame(self, background=ACCENT, height=2).pack(fill="x")
        ttk.Label(head, text="Tomb Raider III: The Lost Artefact", style="Head.TLabel").grid(row=0, column=0, sticky="w")
        ttk.Label(head, text="PS1 disc builder", style="Sub.TLabel").grid(row=1, column=0, sticky="w")
        head.columnconfigure(0, weight=1)
        ttk.Button(head, text="Report a problem", style="Info.TButton", command=self.report).grid(row=0, column=1, rowspan=2)
        ttk.Button(head, text="ⓘ  About", style="Info.TButton", command=self.about).grid(row=0, column=2, rowspan=2)

        body = ttk.Frame(self, padding=(16, 12))
        body.pack(fill="both", expand=True)
        body.columnconfigure(0, weight=1)

        def card(r, title):
            c = ttk.Frame(body, style="Card.TFrame", padding=(12, 10))
            c.grid(row=r, column=0, sticky="ew", pady=(0, 10))
            c.columnconfigure(1, weight=1)
            mark = ttk.Label(c, text="○", style="Muted.TLabel", font=(FONT, 13), width=2)
            mark.grid(row=0, column=0, rowspan=2, sticky="n")
            ttk.Label(c, text=title, style="Step.TLabel").grid(row=0, column=1, sticky="w")
            info = ttk.Label(c, text="", style="Muted.TLabel", wraplength=480)
            info.grid(row=1, column=1, sticky="w")
            return c, mark, info

        c, self.ps1_mark, self.ps1_label = card(0, "1   Your Tomb Raider III PS1 disc")
        ttk.Button(c, text="Choose...", command=self.pick_ps1).grid(row=0, column=2, rowspan=2, padx=(8, 0))
        self.ps1_label.config(text="Any version and revision; .bin/.cue, .iso or .zip")

        c, self.pc_mark, self.pc_label = card(1, "2   Lost Artefact PC files")
        ttk.Button(c, text="Disc image...", command=self.pick_pc).grid(row=0, column=2, rowspan=2, padx=(8, 0))
        ttk.Button(c, text="Folder...", command=self.pick_pc_folder).grid(row=0, column=3, rowspan=2, padx=(6, 0))
        self.pc_label.config(text="Gold CD image or its installed game folder, Tomb Raider I-III Remastered or TRX")

        opt = ttk.Frame(body, style="Card.TFrame", padding=(12, 10))
        opt.grid(row=2, column=0, sticky="ew", pady=(0, 10))
        opt.columnconfigure(2, weight=1)
        ttk.Label(opt, text="3   Options", style="Step.TLabel").grid(row=0, column=0, columnspan=4, sticky="w", pady=(0, 6))
        ttk.Label(opt, text="Blood", style="Muted.TLabel").grid(row=1, column=0, sticky="w", padx=(26, 10))
        seg = ttk.Frame(opt, style="Card.TFrame")
        seg.grid(row=1, column=1, columnspan=3, sticky="w")
        for text, val in (("As the game has it", "default"), ("Red", "red"), ("Purple", "purple")):
            ttk.Radiobutton(seg, text=text, value=val, variable=self.blood, style="Seg.Toolbutton").pack(side="left")
        ttk.Checkbutton(opt, text="Level select on New Game (leave off for a normal playthrough)", variable=self.test,
                        style="Card.TCheckbutton").grid(row=2, column=0, columnspan=4, sticky="w", padx=(22, 0), pady=(8, 0))
        self.ntsc_box = ttk.Checkbutton(opt, text="NTSC 60 Hz (European discs, needs the US disc, any revision):",
                                        variable=self.ntsc, command=self.update_state, style="Card.TCheckbutton")
        self.ntsc_box.grid(row=3, column=0, columnspan=2, sticky="w", padx=(22, 0), pady=(6, 0))
        self.us_entry = ttk.Entry(opt, textvariable=self.us)
        self.us_entry.grid(row=3, column=2, sticky="ew", padx=(8, 0), pady=(6, 0))
        self.us_button = ttk.Button(opt, text="Choose...", command=self.pick_us)
        self.us_button.grid(row=3, column=3, padx=(6, 0), pady=(6, 0))

        save = ttk.Frame(body)
        save.grid(row=3, column=0, sticky="ew", pady=(2, 10))
        save.columnconfigure(1, weight=1)
        ttk.Label(save, text="Save as").grid(row=0, column=0, padx=(0, 8))
        ttk.Entry(save, textvariable=self.out).grid(row=0, column=1, sticky="ew")
        ttk.Button(save, text="...", width=3, command=self.pick_out).grid(row=0, column=2, padx=(6, 10))
        self.build_button = ttk.Button(save, text="▶  Build disc image", style="Accent.TButton", command=self.start)
        self.build_button.grid(row=0, column=3)

        prog = ttk.Frame(body, style="Card.TFrame", padding=(12, 10))
        prog.grid(row=4, column=0, sticky="nsew")
        self.body = body
        prog.columnconfigure(0, weight=1)
        ttk.Label(prog, textvariable=self.step_text, style="Muted.TLabel").grid(row=0, column=0, sticky="w")
        ttk.Label(prog, textvariable=self.detail_text, style="Muted.TLabel").grid(row=0, column=1, sticky="e")
        self.progress = ttk.Progressbar(prog, mode="determinate", maximum=100)
        self.progress.grid(row=1, column=0, columnspan=2, sticky="ew", pady=(6, 6))
        self.details_button = ttk.Button(prog, text="▸ Details", style="Link.TButton", command=self.toggle_details)
        self.details_button.grid(row=2, column=0, sticky="w")
        self.report_button = ttk.Button(prog, text="Report this error on GitHub", style="Link.TButton",
                                        command=lambda: self.report(self.last_error))
        self.last_error, self.pc_rels = None, None
        self.log = tk.Text(prog, height=10, state="disabled", wrap="word", background=BG, foreground=MUTED,
                           insertbackground=TEXT, relief="flat", font=("Consolas", 9), highlightthickness=0)
        self.prog_frame = prog
        self.time_label = ttk.Label(prog, textvariable=self.time_text, style="Muted.TLabel")
        self.time_label.grid(row=2, column=1, sticky="e")
        self.update_state()
        self.after(100, self.poll)

    # about
    def about(self):
        w = tk.Toplevel(self)
        w.title("About")
        w.configure(background=CARD)
        w.resizable(False, False)
        w.transient(self)
        f = ttk.Frame(w, style="Card.TFrame", padding=(20, 16))
        f.pack(fill="both")
        ttk.Label(f, text="Tomb Raider III: The Lost Artefact - PS1", style="Head.TLabel").pack(anchor="w")
        tk.Frame(f, background=ACCENT, height=2).pack(fill="x", pady=(6, 12))
        for role, who, note in CREDITS:
            ttk.Label(f, text=role, style="Muted.TLabel").pack(anchor="w")
            ttk.Label(f, text=who, style="Card.TLabel", font=(FONT, 10, "bold")).pack(anchor="w", pady=(0, 0 if note else 8))
            if note:
                ttk.Label(f, text=note, style="Card.TLabel", wraplength=420).pack(anchor="w", pady=(0, 8))
        ttk.Label(f, text=ORIGIN, style="Muted.TLabel", wraplength=420, font=(FONT, 9)).pack(anchor="w", pady=(6, 0))
        ttk.Label(f, text=LEGAL, style="Muted.TLabel", wraplength=420, font=(FONT, 9)).pack(anchor="w", pady=(6, 12))
        ttk.Button(f, text="Close", command=w.destroy).pack(anchor="e")
        w.grab_set()

    # problem reports
    def report(self, error=None):
        """Opens a new GitHub issue, filled in with what the builder knows. File paths are left out."""
        ps1 = self.ps1_info
        lines = [f"Builder {VERSION}, Windows {platform.release()}",
                 f"PS1 disc: {ps1['redump'] if ps1 else 'not chosen or not recognised'}",
                 f"PC files: {self.pc_rels or 'not chosen or not recognised'}",
                 f"Options: blood {self.blood.get()}, level select {'on' if self.test.get() else 'off'}, "
                 f"NTSC {'on' if self.ntsc.get() else 'off'}"]
        if error:
            msg = self.without_paths(error)[:1200]
            title = "Builder error: " + msg.splitlines()[0][:80]
            body = "**What happened**\nThe build stopped with this error:\n```\n" + msg + "\n```\n\n"
        else:
            title = ""
            body = ("**What happened**\n(For a problem in the game: which disc version, which level, where - "
                    "room number from TRView if you can - and what you did just before.)\n\n")
        body += "**Setup**\n" + "\n".join("- " + l for l in lines) + "\n"
        webbrowser.open(ISSUES + "?" + urllib.parse.urlencode({"title": title, "body": body}))

    def without_paths(self, text):
        """The player's folders and file paths replaced by <path> (nothing personal goes into the report)."""
        known = [self.ps1.get(), self.pc.get(), self.us.get(), self.out.get(), os.path.expanduser("~")]
        known += [os.path.dirname(p) for p in known if p]
        for p in sorted({os.path.abspath(p) for p in known if p}, key=len, reverse=True):
            for form in (p, p.replace("\\", "/")):
                text = text.replace(form, "<path>")
        # any other path, up to the next separator (spaces included: rather too much than a folder name)
        return re.sub(r"(?:[A-Za-z]:)?[\\/][^\n:;,'\"]*[\\/][^\n:;,'\"]*", "<path>", text)

    # file pickers
    def pick_ps1(self):
        p = filedialog.askopenfilename(title="Tomb Raider III PS1 disc image",
                                       filetypes=[("Disc images", "*.cue *.bin *.iso *.zip"), ("All files", "*.*")])
        if p:
            self.ps1.set(p)
            self.recognise_ps1()

    def pick_us(self):
        p = filedialog.askopenfilename(title="Tomb Raider III PS1 disc image (USA)",
                                       filetypes=[("Disc images", "*.cue *.bin *.iso *.zip"), ("All files", "*.*")])
        if p:
            self.us.set(p)

    def pick_pc(self):
        p = filedialog.askopenfilename(title="Lost Artefact PC disc image",
                                       filetypes=[("Disc images", "*.iso *.cue *.bin *.zip"), ("All files", "*.*")])
        if p:
            self.pc.set(p)
            self.recognise_pc()

    def pick_pc_folder(self):
        p = filedialog.askdirectory(title="Lost Artefact game folder (or Tomb Raider I-III Remastered)")
        if p:
            self.pc.set(p)
            self.recognise_pc()

    def pick_out(self):
        p = filedialog.asksaveasfilename(title="Save disc image", defaultextension=".bin",
                                         filetypes=[("Disc image", "*.bin")])
        if p:
            self.out.set(p)

    # recognition
    def mark(self, mark, label, ok, text):
        mark.config(text="✓" if ok else "✗", foreground=OK if ok else BAD)
        label.config(text=text, foreground=OK if ok else BAD)

    def recognise_ps1(self):
        self.ps1_info = None
        try:
            disc, _ = inputs.open_image(self.ps1.get())
            info = inputs.recognise_ps1(disc)
        except (OSError, ValueError) as e:
            self.mark(self.ps1_mark, self.ps1_label, False, str(e))
            return self.update_state()
        if info is None:
            self.mark(self.ps1_mark, self.ps1_label, False, "Not a known Tomb Raider III PS1 disc.")
        elif not info["supported"]:
            self.mark(self.ps1_mark, self.ps1_label, False,
                      f"{info['language']}, {info['revision']}: this revision is not supported yet.")
        else:
            self.ps1_info = info
            self.mark(self.ps1_mark, self.ps1_label, True, f"Tomb Raider III {info['language']}, {info['revision']} - recognised")
            if not self.out.get():
                self.out.set(os.path.join(os.path.dirname(self.ps1.get()), f"tr3la_{build.LAYOUT[info['version']]}.bin"))
        self.update_state()

    def recognise_pc(self):
        src = self.pc.get()
        try:
            files = inputs.pc_files_from_folder(src) if os.path.isdir(src) else inputs.pc_files_from_disc(inputs.open_image(src)[0])
            res = inputs.recognise_pc(files)
        except (OSError, ValueError) as e:
            self.mark(self.pc_mark, self.pc_label, False, str(e))
            return
        missing = [n for n, (rel, crc) in res.items() if crc is None]
        unknown = [n for n, (rel, crc) in res.items() if crc is not None and rel is None]
        if missing or unknown:
            hint = " (MAIN.SFX holds the sounds, it is next to the level folder)" if "main.sfx" in missing else ""
            text = "; ".join(x for x in (f"missing: {', '.join(missing)}{hint}" if missing else "",
                                         f"unknown version: {', '.join(unknown)}" if unknown else "") if x)
            self.mark(self.pc_mark, self.pc_label, False, text)
        else:
            rels = sorted({rel for rel, _ in res.values()})
            self.pc_rels = ", ".join(rels)
            self.mark(self.pc_mark, self.pc_label, True, f"Lost Artefact found ({', '.join(rels)}) - recognised")

    def update_state(self):
        european = bool(self.ps1_info and self.ps1_info["version"] in build.NTSC_LANG)
        if not european:
            self.ntsc.set(False)
        self.ntsc_box.config(state="normal" if european else "disabled")
        st = "normal" if self.ntsc.get() else "disabled"
        self.us_entry.config(state=st)
        self.us_button.config(state=st)

    # building
    def toggle_details(self, show=None):
        show = not self.log.winfo_ismapped() if show is None else show
        if show:
            self.log.grid(row=3, column=0, columnspan=2, sticky="nsew", pady=(6, 0))
            self.prog_frame.rowconfigure(3, weight=1)
            self.body.rowconfigure(4, weight=1)
        else:
            self.log.grid_remove()
            self.body.rowconfigure(4, weight=0)
        self.details_button.config(text=("▾" if show else "▸") + " Details")

    def say(self, text):
        self.log.config(state="normal")
        self.log.insert("end", text + "\n")
        self.log.see("end")
        self.log.config(state="disabled")

    def follow(self, m):
        """Step name and progress bar from the build's log lines."""
        for k, (line, name, _) in enumerate(STEPS):
            if m == line or m.startswith(line):
                self.step, self.step_t0 = k, time.monotonic()
                self.levels_done = 0
                self.step_text.set(f"Step {k + 1} of {len(STEPS)}: {name}")
                self.detail_text.set("")
                return
        lv = m.strip().removesuffix(" done")
        if m.strip().endswith(" done") and lv in LEVEL_NAMES:
            self.levels_done += 1
            self.detail_text.set(f"{LEVEL_NAMES[lv]} ({self.levels_done}/{len(LEVEL_NAMES)})")

    def typical(self):
        t = [s[2] for s in STEPS]
        if self.ntsc.get():
            t[0] += 17
        if self.ps1_info and self.ps1_info["version"] in ("jp", "as"):
            t[2] += 8
        return t

    def tick(self):
        """The bar moves on smoothly within a step (by its typical time, at most to 97 % of the step, by the
        finished levels while converting). The time left counts down: the rest of the step and the later
        steps by their typical times, scaled by how long the finished steps took on this PC."""
        now = time.monotonic()
        typical, k = self.typical_times, self.step
        speed = min(max((self.step_t0 - self.t0) / sum(typical[:k]), 0.5), 4) if k else 1
        in_step = now - self.step_t0
        part = min(in_step / (typical[k] * speed), 0.97)
        if k == 2:
            part = max(part, self.levels_done / len(LEVEL_NAMES))
        done = sum(typical[:k]) + part * typical[k]
        self.progress["value"] = max(self.progress["value"], 100 * done / sum(typical))
        elapsed = now - self.t0
        left = max(typical[k] * speed - in_step, 1) + speed * sum(typical[k + 1:])
        self.time_text.set(f"{clock(elapsed)} - about {clock(left)} left")

    def start(self):
        if not (self.ps1.get() and self.pc.get() and self.out.get()):
            self.step_text.set("Choose the PS1 disc, the PC files and where to save the disc image.")
            return
        self.build_button.config(state="disabled")
        self.report_button.grid_remove()
        self.progress["value"] = 0
        self.step, self.levels_done = 0, 0
        self.t0 = self.step_t0 = time.monotonic()
        self.typical_times = self.typical()
        self.time_label.grid()
        self.step_text.set(f"Step 1 of {len(STEPS)}: {STEPS[0][1]}")
        self.detail_text.set("")
        args = (self.ps1.get(), self.pc.get(), self.out.get(), self.us.get() if self.ntsc.get() else None,
                None if self.blood.get() == "default" else self.blood.get())
        threading.Thread(target=self.run, args=args, kwargs={"test": self.test.get()}, daemon=True).start()

    def run(self, *args, test=False):
        try:
            build.build(*args, log=self.msgs.put, test=test)
            self.msgs.put(("done", None))
        except Exception as e:      # shown to the player, the build stops; the work folder stays for checking
            self.msgs.put(("error", f"{e}\n(The intermediate files were kept in the folder build_work next to the disc image.)"))

    def poll(self):
        while not self.msgs.empty():
            m = self.msgs.get()
            if isinstance(m, tuple):
                self.build_button.config(state="normal")
                took, self.t0 = clock(time.monotonic() - self.t0), None
                self.time_text.set("")
                if m[0] == "done":
                    self.progress["value"] = 100
                    self.step_text.set(f"Finished in {took} - the disc image is ready.")
                    self.detail_text.set("")
                    self.say(f"Finished in {took}.")
                else:
                    self.time_label.grid_remove()
                    self.step_text.set("The build stopped - see the details.")
                    self.say(f"Error: {m[1]}")
                    self.last_error = m[1]
                    self.report_button.grid(row=2, column=1, sticky="e")
                    self.toggle_details(True)
            else:
                self.say(m)
                self.follow(m)
        if self.t0 is not None:
            self.tick()
        self.after(100, self.poll)


def run_tool(tool, args):
    """The EXE runs the project's Python tools (tools/*.py) through itself: <exe> --run-tool <tool> <args>."""
    import runpy
    import convert
    sys.path.insert(0, convert.TOOLS)
    sys.argv = [tool] + args
    runpy.run_path(os.path.join(convert.TOOLS, tool), run_name="__main__")


if __name__ == "__main__":
    if len(sys.argv) > 2 and sys.argv[1] == "--run-tool":
        run_tool(sys.argv[2], sys.argv[3:])
    elif len(sys.argv) > 1 and sys.argv[1] == "--build":      # without the window, as build.py
        build.main(sys.argv[2:])
    else:
        App().mainloop()
