"""Builds bin/patches/<version>/ngpchoice.ips: "New Game" asks for the game mode, "New Game" or "New Game+",
once the New Game+ is unlocked, as in the remastered Tomb Raider III.

The passport's "New Game" starts the game at once, or, with the level select of the gameflow (flag
0x400), opens a requester with the levels. This patch takes that requester for the choice:

  - the flag test there ('lw v0, flags(gp); nop; andi v0, v0, 0x400') calls a routine that returns the
    flags with 0x400 also set when the New Game+ is unlocked (and finds that out with the level select
    as well, for the filling): a New Game+ running (bonus byte set), or the memory card holds a New Game+
    savegame (the savegame's name with P as last letter, see make_ngpfile_ips.py; checked once with the
    BIOS functions open and close); the result is kept in a byte for the session,
  - the requester's filling (after its heading) calls a routine that, when unlocked, makes it the mode
    choice: heading MODE_TEXT, the items "New Game" (game text 6) and NGP_TEXT, 2 lines high instead of
    the level list's 5; when locked it clears the bonus byte and fills the level list as before,
  - the requester call returns the answer to a routine: in the mode choice it sets the bonus byte to 0 or
    1 (before the game's InitialiseStartInfo, which gives the New Game+ equipment when it is set); without
    the level select it answers 1, the first level, with it the requester becomes the level list (heading
    "Select Level", game text 11, the levels, 5 lines) and the level chosen there is answered,
  - the direct start (no requester) clears the bonus byte, so "New Game" is always a normal game then,
  - "Load Game" with both savegames on the card (normal and New Game+) asks first: its requester (heading
    "Load Game") gets the items "New Game" and NGP_TEXT, 2 lines high (phase 3); the answer sets the bonus
    byte, so the right file is loaded, and then the requester lists the savegame's positions as before. With
    only one of them it loads that one (the bonus byte set for the New Game+ savegame). It asks only on the
    title (inventory mode 1, the store 'addiu v0, zero, 2; sw s5, mode(gp); sw v0' in the inventory) and in
    Lara's home (the level running 0, as the death code tests it before its 'exit to title'); in a level it loads the file of the mode running (the bonus byte), so a load
    there never changes the mode (with no such file the card dialog says so).

Saving in the New Game+ (bonus byte set; also the savegame after the end credits) says so: the save
routine's call of the memory card dialog ('move a0, zero; move a1, s0; jal dialog; move a2, s1/s2') goes to a
routine that shows SAVE_TEXT instead of the dialog's first line ("Save game to" / "Overwrite game on", memory
card texts 24 and 25) for the time: a New Game+ savegame is created, or the one there overwritten (the
normal savegame is a file of its own and stays). Its title in the console's memory card screen (Shift-JIS in
the 0x200 byte header the save routine writes, 'lui a1; addiu a1, header' before its write; the title as
savename.ips sets it) reads TITLE_PLUS for the time: the title with a full width + at its end, or, where it
fills all 64 bytes (French: "Tomb Raider: Le Dernier Artefact"), "TR3" for "Tomb Raider" and the +. The bytes
that differ are switched with an XOR table, once before the dialog and once after it (that routine and its
table go into a game function nothing refers to, as the weapon clearing, or, where none is large enough, only the
routine and the table behind the routines in the appended block, or both there). The New Game+ load text goes
behind the routines too, or into an unused function where the block has no room for it; a text that ends
another one (French: "Nouvelle Partie+") shares its bytes.

The Lost Artefact ends with the gameflow's GAMECOMPLETE; newgameplus.ips sends it through a routine that sets
the bonus byte and jumps to the credits. Its first words are replaced here ('lui at; lbu t0, bonus; j gameend'):
the byte becomes 1 after a normal game and 2 after a New Game+, until the end statistics' save (GameStats, the
call behind 'lui v1, 0x100' and Tomb Raider III's bonus level 20 after all secrets) goes to a routine that sets
the current level (the halfword before the bonus byte) to the first and offers:

  - after a normal game first the normal savegame (every level it reached stays available), then, only while
    there is no New Game+ savegame on the card (a finished one keeps its levels), the New Game+ one:
    ModifyStartInfo for the first level with the bonus byte set and the weapon clearing below, which also
    makes the later levels unavailable (bit 0x1000000 of the word at 0x14 of the start infos),
  - after a New Game+ the New Game+ savegame, its levels kept.

Tomb Raider III's own end loops over the levels are never reached in this game.

A normal game must not take over New Game+ equipment of the same session: the bonus part of ModifyStartInfo
gives it to every level (InitialiseStartInfo of a New Game+ does that for all of them), and a normal
InitialiseStartInfo resets only the first level's weapons, so a normal game started in a later level with the
level select had the New Game+ weapons (and, without the New Game+, what a loaded or saved game left in the
start infos). Starting a normal game (the direct start, the choice "New Game" and the level list shown without
the mode choice) therefore first gives the start infos from level 2 on the first level's normal start: no weapons but the
pistols and not available (the bits 0xfc000000 and 0x1000000 of the word at 0x14 but the pistols' 0x2000000, bit 0 of the word at 0x18, the
ammunition halfwords 2 to 14), a small and a large medipack (bytes 0x10, 0x11), 2 flares (0x13) and a save
crystal (0x14); a level reached in the game gets what Lara has from the one before it anyway. That routine goes into a game function nothing refers
to (make_reunionview_ips.unused_routines, not written by the patches before this one).

Starting a level (the game's level start, 'beq s1, s2; nop; jal ModifyStartInfo; move a0, s0') calls
ModifyStartInfo for it; in the released bonus game that gave the unlimited ammunition back in every level,
in the New Game+ it would give the limited ammunition back. With the bonus byte set that call is left out
(a routine behind the weapon clearing), so Lara keeps what she has from the level before, as in a normal
game; Lara's house (level 0) always gets the call, without weapons, and ends the New Game+ of the session
(its doubled damage too); only the weapon in her hands becomes the pistols again (byte 0x16 of the start info, as the call does
at its start in every game); the New Game+ equipment comes from InitialiseStartInfo when it starts (in any level with the level
select) and from the game end routine above.

The memory card dialog clears every text (T_InitPrint) but not the inventory's handles of its button hints
("Select", "Go Back"); the load page therefore removes them before its load ('jal hints; sw zero, 0(s0)' before
the load call), and the inventory creates them again. That call is left out there and made right before the
load from the card instead (both in the load call routine and after the choice): with the choice the load comes
later, and handles left to freed texts kept the hints away until another page removed them; and without a load
at the page's start they were removed and created again at once, a flicker. After the choice the requester's own
texts are removed first as well (the function behind its init): the dialog would leave the requester holding
freed texts, and after a cancelled load it removed those slots later, by then the inventory's "Select" again.
So does everything else the page removes before its load (its title, the item name, the ring's heading, made
again meanwhile): the routine for that (the requester's removal and the page's own 15 words, copied) goes into
an unused function, as the weapon clearing (or behind it in its function where no other is left).

The memory card dialog keeps the result of its last card check (no file, file there, ...) and checks again
only after a card change; with two file names that result can belong to the other file, so every routine
that loads or saves here clears it first (the save routine's 'sw zero, status(gp)' before its open).

The texts MODE_TEXT, NGP_TEXT and SAVE_TEXT, in the version's language (glyph strings on the Japanese and
Asian Disc 1), are kept behind the routines; the US executables get ngpchoice_<language>.ips as well, for a
European disc built as NTSC (tools/builder/build.py). The New Game+ is
kept unlocked by the savegame the game offers after the end credits and the statistics: the bonus byte is
set then, so ngpfile.ips writes it as the New Game+ savegame (and it starts the New Game+ in the first level).

Found in every version: on the load page the load call ('jal load; nop; move s2, v0; bne s2, s3'), its
filling behind it ('lui v1; addiu v1, req + 0x12; addiu s0, v1, -0x12; ... jal init ... jal heading; move a3,
zero; jal fill; move a0, s0') and the requester call with that requester; the flag test before the requester call (lui a0; addiu a0; jal requester), the
filling ('jal heading; move a3, zero; jal fill; move a0, s0'), the requester's clear and add functions as
the first two calls in the filling, the direct start (the branch target of the flag test: jal
InitialiseStartInfo), the game texts (lw v0, texts(gp) before the heading text), the memory card texts and
the savegame name's index (make_ngpfile_ips.py's open calls: 'lw v0, mc(gp); nop; lw a0, name(v0)'), the
BIOS stub close ('addiu t2, zero, 0xb0; jr t2; addiu t1, zero, 0x36'), the bonus byte. The routines go
behind the New Game+ savegame routine into the block pickup3d.ips appends to the executable.

Usage: make_ngpchoice_ips.py [--versions us,ukrev,...] [--exe VERSION=PATH ...]"""
import argparse
import os
import struct

import make_jp_gameflow as jpg
import make_newgameplus_ips as ngp
import make_ngpfile_ips as nf
import make_pickup3d_ips as p3
import make_reunionview_ips as rv

HERE = os.path.dirname(os.path.abspath(__file__))
BIN = os.path.join(HERE, "..", "bin")
LOAD = rv.LOAD
NGP_TEXT = {"en": b"New Game+", "de": b"Neues Spiel+", "fr": b"Nouvelle Partie+", "it": b"Nuova partita+",
            "sp": b"Juego Nuevo+"}                    # as the game's own "New Game" in that language
MODE_TEXT = {"en": b"Game Mode", "de": b"Spielmodus", "fr": b"Mode de jeu", "it": b"Modo di gioco",
             "sp": b"Modo de juego"}
SAVE_TEXT = {"en": (b"Create New Game+ save on", b"Overwrite New Game+ on"),
             "de": (b"Spielstand f~ur Neues Spiel+ anlegen auf", b"Neues Spiel+ ~uberschreiben auf"),
             "fr": (b"Cr)eer sauvegarde Nouvelle Partie+ sur", b"Ecraser Nouvelle Partie+"),
             "it": (b"Crea salvataggio Nuova partita+ su", b"Sovrascrivi Nuova partita+ su"),
             "sp": (b"Crear partida guardada Juego Nuevo+ en", b"Escribir Juego Nuevo+ en")}
SAVE_TO, OVERWRITE_ON, LOAD_FROM = 24, 25, 26   # memory card texts
LOAD_TEXT = {"en": b"Load New Game+ from", "de": b"Neues Spiel+ laden von", "fr": b"Charger Nouvelle Partie+ de",
             "it": b"Carica Nuova partita+ da", "sp": b"Cargar Juego Nuevo+ de"}
TITLE_PLUS, TITLE_LONG, TITLE_SHORT = "＋", "Ｔｏｍｂ　Ｒａｉｄｅｒ", "ＴＲ３"   # savegame title (Shift-JIS)
# the Japanese Disc 1: glyph strings (KANJI.PSX cells, the + in cell 256, see make_kanji_la.py)
NGP_TEXT["jp"] = jpg.encode("ニューゲーム＋")
LOAD_TEXT["jp"] = jpg.encode("ニューゲーム＋をロードしますか？")
MODE_TEXT["jp"] = jpg.encode("ゲームモード")
SAVE_TEXT["jp"] = (jpg.encode("ニューゲーム＋をセーブしますか？"), jpg.encode("ニューゲーム＋に上書きしますか？"))
LANGUAGE = {"us": "en", "us10": "en", "uk": "en", "ukrev": "en", "jp2": "en", "as2": "en", "de": "de", "derev": "de",
            "fr": "fr", "frrev": "fr", "it": "it", "itrev": "it", "sp": "sp", "sprev": "sp", "jp": "jp", "as": "jp"}
NTSC_VERSIONS, NTSC_LANGUAGES = ("us", "us10"), ("de", "fr", "it", "sp", "en")   # a European disc as NTSC: the
                          # US executable with savename_<language>.ips (make_ntsc_savename_ips.py) and these
NEW_GAME, SELECT_LEVEL = 6, 11         # game texts
LEVEL_SELECT = 0x400
REQ_SELECTED, REQ_LINES, REQ_TOP = 4, 6, 8
MODE_LINES = 2


def word(d, o):
    return struct.unpack_from("<I", d, o)[0]


def target(w, o):
    return (w & 0x3FFFFFF) << 2 | (o + LOAD) & 0xF0000000 | 0x80000000


def sites(exe):
    w = lambda o: word(exe, o)
    hits = []
    for o in range(0x800, len(exe) - 0x40, 4):
        if w(o) >> 16 == 0x8F82 and w(o + 4) == 0 and w(o + 8) == 0x30420000 | LEVEL_SELECT and w(o + 12) >> 16 == 0x1040:
            for k in range(4, 16):
                if w(o + 4 * k) >> 16 == 0x3C04 and w(o + 4 * k + 4) >> 16 == 0x2484 and w(o + 4 * k + 8) >> 26 == 3:
                    hits.append((o, o + 4 * k + 8))
                    break
    assert len(hits) == 1, f"flag test before the requester found {len(hits)} times"
    flag_site, req_call = hits[0]
    flags = w(flag_site) & 0xFFFF
    direct = flag_site + 12 + 4 + (((w(flag_site + 12) & 0xFFFF) ^ 0x8000) - 0x8000) * 4
    assert w(direct) >> 26 == 3, "the direct start is no call"
    fills = [o for o in range(req_call, req_call + 0x100, 4)
             if w(o) >> 26 == 3 and w(o + 4) == 0x02002021 and w(o - 4) == 0x00003821 and w(o - 8) >> 26 == 3]
    assert len(fills) == 1, f"filling found {len(fills)} times"
    fill_site = fills[0]
    heading = target(w(fill_site - 8), fill_site - 8)
    texts = next(w(o) & 0xFFFF for o in range(fill_site - 0x20, fill_site, 4) if w(o) >> 16 == 0x8F82)
    fill = target(w(fill_site), fill_site)
    calls = [target(w(o), o) for o in range(fill - LOAD, fill - LOAD + 0x80, 4) if w(o) >> 26 == 3]
    clear, add = calls[0], calls[1]
    return dict(flag_site=flag_site, flags=flags, req_call=req_call, requester=target(w(req_call), req_call),
                direct=direct, start=target(w(direct), direct), fill_site=fill_site, fill=fill, heading=heading,
                clear=clear, add=add, texts=texts)


def load_sites(exe):
    w = lambda o: word(exe, o)
    hits = [o for o in range(0x800, len(exe) - 16, 4)
            if w(o) >> 26 == 3 and w(o + 4) == 0 and w(o + 8) == 0x00409021 and w(o + 12) >> 16 == 0x1653]
    assert len(hits) == 1, f"load call found {len(hits)} times"
    call = hits[0]
    assert w(call - 8) >> 26 == 3 and w(call - 4) == 0xAE000000, "button hint removal before the load not found"
    clear = [w(o) for o in range(call - 0x3C, call, 4)]          # the page's text removal before its load
    assert clear[0] >> 16 == 0x8F84 and sum(x >> 26 == 3 for x in clear) == 4, "text removal before the load not found"
    fills = [o for o in range(call, call + 0x80, 4)
             if w(o) >> 26 == 3 and w(o + 4) == 0x02002021 and w(o - 4) == 0x00003821 and w(o - 8) >> 26 == 3]
    assert len(fills) == 1, f"load filling found {len(fills)} times"
    fill_site = fills[0]
    lui = next(o for o in range(call, fill_site, 4) if w(o) >> 16 == 0x3C03 and w(o + 4) >> 16 == 0x2463)
    req = ((w(lui) & 0xFFFF) << 16) + (((w(lui + 4) & 0xFFFF) ^ 0x8000) - 0x8000) - 0x12 | 0x80000000
    y = ((w(lui + 8 - 12) & 0xFFFF) ^ 0x8000) - 0x8000                 # 'addiu v0, zero, y' before the lui
    init = next(target(w(o), o) for o in range(lui, fill_site, 4) if w(o) >> 26 == 3)
    ret = next(o for o in range(init - LOAD, init - LOAD + 0x100, 4) if w(o) == 0x03E00008)
    assert w(ret + 8) == 0x27BDFFE0, "the requester's text removal not found behind its init"
    disp = [o for o in range(0x800, len(exe) - 12, 4)
            if w(o) >> 16 == 0x3C04 and w(o + 4) >> 16 == 0x2484 and w(o + 12) >> 26 == 3
            and ((w(o) & 0xFFFF) << 16) + (((w(o + 4) & 0xFFFF) ^ 0x8000) - 0x8000) == req & 0x7FFFFFFF]
    assert len(disp) == 1, f"load requester call found {len(disp)} times"
    modes = [o for o in range(0x800, len(exe) - 8, 4)
             if w(o) == 0x24020002 and w(o + 4) >> 16 == 0xAF95 and w(o + 8) >> 16 == 0xAF82]
    assert len(modes) == 1, f"inventory mode found {len(modes)} times"
    levels = [o for o in range(0x800, len(exe) - 24, 4)          # 'lw v0, level(gp); nop; bnez v0; ...; j; addiu v0,
              if w(o) >> 16 == 0x8F82 and w(o + 4) == 0 and w(o + 8) >> 16 == 0x1440   # zero, 0x500': Lara's home
              and w(o + 16) >> 26 == 2 and w(o + 20) == 0x24020500]                     # ends with the title on death
    assert len(levels) == 1, f"level running found {len(levels)} times"
    return dict(load_call=call, load=target(w(call), call), mode=w(modes[0] + 4) & 0xFFFF, level=w(levels[0]) & 0xFFFF, hints=target(w(call - 8), call - 8), remove=ret + 8 + LOAD, clear=clear, load_fill_site=fill_site,
                load_fill=target(w(fill_site), fill_site), load_req=req, load_y=y, init=init,
                load_disp=disp[0] + 12)


def save_dialog(exe):
    """File offset of the save routine's 'jal dialog' and the dialog's address."""
    w = lambda o: word(exe, o)
    hits = [o for o in range(0x800, len(exe) - 8, 4)
            if w(o - 8) == 0x00002021 and w(o - 4) == 0x02002821 and w(o) >> 26 == 3
            and w(o + 4) in (0x02203021, 0x02403021)]                         # move a2, s1 or s2
    assert len(hits) == 1, f"save dialog call found {len(hits)} times"
    return hits[0], target(w(hits[0]), hits[0])


def load_dialog(exe, dialog):
    """File offset of the load routine's 'jal dialog' ('addiu a0, zero, 1' two words before: mode 1, load)."""
    call = 0x0C000000 | (dialog & 0x0FFFFFFF) >> 2
    hits = [o for o in range(0x800, len(exe) - 4, 4) if word(exe, o) == call and word(exe, o - 8) == 0x24040001]
    assert len(hits) == 1, f"load dialog call found {len(hits)} times"
    return hits[0]


def card_status(exe):
    """gp offset of the memory card dialog's last check result, cleared by the save routine before its open."""
    _, calls = nf.open_calls(exe)
    hits = {word(exe, c + 4 * k) & 0xFFFF for c in calls for k in range(-4, 3) if word(exe, c + 4 * k) >> 16 == 0xAF80}
    assert len(hits) == 1, f"card status found {len(hits)} times"
    return hits.pop()


def credits_jump(orig, ver):
    """File offset of newgameplus.ips's 'j credits' (in its routine the GAMECOMPLETE call goes to) and the
    credits' address."""
    site, credits = ngp.credits_call(orig)
    exe = rv.apply_ips(orig, open(os.path.join(BIN, "patches", ver, "newgameplus.ips"), "rb").read())
    cave = target(word(exe, site), site) - LOAD
    hits = [o for o in range(cave, cave + 0x20, 4) if word(exe, o) == 0x08000000 | (credits & 0x0FFFFFFF) >> 2]
    assert len(hits) == 1, "newgameplus.ips's jump to the credits not found"
    return hits[0], credits


def end_save_call(exe):
    """File offset of the end statistics' save call ('jal save' with 'addiu v0, zero, 20' and 'lui v1, 0x100'
    before it: the bonus level after all secrets), the save routine and the savegame size it is given."""
    w = lambda o: word(exe, o)
    hits = [o for o in range(0x880, len(exe) - 4, 4)
            if w(o) >> 26 == 3 and any(w(k) == 0x3C030100 for k in range(o - 0x40, o, 4))
            and any(w(k) == 0x24020014 for k in range(o - 0x50, o, 4))]
    assert len(hits) == 1, f"end statistics' save found {len(hits)} times"
    size = [w(k) & 0xFFFF for k in range(hits[0] - 0x20, hits[0], 4) if w(k) >> 16 == 0x2405]   # addiu a1, zero
    assert len(size) == 1, "savegame size not found"
    return hits[0], target(w(hits[0]), hits[0]), size[0]


def start_info_loops(exe):
    """ModifyStartInfo (the function that starts with 'move t5, a0' before the bonus part of the start
    equipment) and the file offsets of its calls in the loops over all levels."""
    w = lambda o: word(exe, o)
    _, start = ngp.bonus_checks(exe, ngp.bonus_byte(exe))
    fn = next(o for o in range(start, start - 0x400, -4) if w(o) == 0x00806821)      # move t5, a0
    call = 0x0C000000 | ((fn + LOAD) & 0x0FFFFFFF) >> 2
    sites = [o for o in range(0x800, len(exe) - 20, 4)
             if w(o) == call and w(o + 4) == 0x02002021 and w(o + 8) >> 16 == 0x8782
             and w(o + 12) == 0x26100001 and w(o + 20) >> 16 == 0x1040]
    assert len(sites) == 2, f"ModifyStartInfo loops found {len(sites)} times"
    return fn + LOAD, sites, word(exe, sites[0] + 8) & 0xFFFF                      # gp offset of the level count


def card_title(orig, ver, lang=None):
    """Address of the first byte of the savegame title that differs in the New Game+ and the XOR bytes from
    there on (the 64 byte Shift-JIS title at 4 in the header, with savename.ips applied; lang: the US executable
    of a European NTSC disc, with savename_<lang>.ips)."""
    name = f"savename_{lang}.ips" if lang else "savename.ips"
    exe = rv.apply_ips(orig, open(os.path.join(BIN, "patches", ver, name), "rb").read())
    _, calls = nf.open_calls(exe)
    w = lambda o: word(exe, o)
    hdr = next(((w(o) & 0xFFFF) << 16) + (((w(o + 4) & 0xFFFF) ^ 0x8000) - 0x8000)
               for o in range(calls[1], calls[1] + 0x60, 4) if w(o) >> 16 == 0x3C05 and w(o + 4) >> 16 == 0x24A5)
    off = (hdr | 0x80000000) - LOAD
    assert exe[off:off + 2] == b"SC", "no savegame header"
    title = exe[off + 4:off + 68]
    text = title.split(b"\0\0")[0].decode("shift_jis")
    if len(text) <= 30:
        plus = text + TITLE_PLUS
    else:
        assert text.startswith(TITLE_LONG), text
        plus = TITLE_SHORT + text[len(TITLE_LONG):] + TITLE_PLUS
    new = plus.encode("shift_jis").ljust(64, b"\0")
    assert len(new) == 64
    diff = [k for k in range(64) if new[k] != title[k]]
    first, last = diff[0], diff[-1] + 1
    return (hdr | 0x80000000) + 4 + first, bytes(a ^ b for a, b in zip(new[first:last], title[first:last]))


def start_infos(exe):
    """Address of the start info array (ModifyStartInfo: 'lui v1, hi; addiu v1, v1, lo; addu t4, v0, v1')."""
    fn, _, _ = start_info_loops(exe)
    o = fn - LOAD
    hits = [k for k in range(o, o + 0x40, 4) if word(exe, k) >> 16 == 0x3C03 and word(exe, k + 4) >> 16 == 0x2463]
    assert hits, "start info array not found"
    k = hits[0]
    return ((word(exe, k) & 0xFFFF) << 16) + (((word(exe, k + 4) & 0xFFFF) ^ 0x8000) - 0x8000) | 0x80000000


def clean_routine(levels, start):
    """Gives the start infos from level 2 on the first level's normal start; only t registers but t8."""
    a = p3.Asm()
    a.lh("t2", levels, "gp")
    a.lui("t4", hi(start + 2 * 0x34)); a.addiu("t4", "t4", (start + 2 * 0x34) & 0xFFFF)
    a.addiu("t3", "zero", 2)
    a.lui("t1", 0x02FF); a.ori("t1", "t1", 0xFFFF)                # all weapons but the pistols, not available
    a.addiu("t7", "zero", -2)
    a.addiu("t0", "zero", 1); a.addiu("t9", "zero", 2)
    a.label("loop")
    a.slt("t5", "t2", "t3")
    a.bnez("t5", "done"); a.nop()
    a.lw("t5", 0x14, "t4"); a.lw("t6", 0x18, "t4")
    a.r(0x24, "t5", "t5", "t1"); a.r(0x24, "t6", "t6", "t7")     # and
    a.sw("t5", 0x14, "t4"); a.sw("t6", 0x18, "t4")
    a.i(0x28, "t4", "t0", 0x10); a.i(0x28, "t4", "t0", 0x11)     # sb: one small, one large medipack
    a.i(0x28, "t4", "t9", 0x13); a.i(0x28, "t4", "t0", 0x14)     # 2 flares, 1 save crystal
    for k in range(2, 16, 2):
        a.sh("zero", k, "t4")                                     # ammunition
    a.addiu("t3", "t3", 1)
    a.bra("loop"); a.addiu("t4", "t4", 0x34)
    a.label("done")
    a.jr("ra"); a.nop()
    return a.code()


def cd_stop(exe):
    """Address of the function that stops the CD music, called when Lara dies ('lh v0, 0x22(s0); nop; bgtz v0;
    addiu v0, zero, -1; sh v0, 0x22(s0); lh v0, x(gp); nop; bnez v0; nop; jal stop')."""
    w = lambda o: word(exe, o)
    hits = [o for o in range(0x800, len(exe) - 40, 4)
            if w(o) == 0x86020022 and w(o + 4) == 0 and w(o + 8) >> 16 == 0x1C40 and w(o + 12) == 0x2402FFFF
            and w(o + 16) == 0xA6020022 and w(o + 20) >> 16 == 0x8782 and w(o + 24) == 0
            and w(o + 28) >> 16 == 0x1440 and w(o + 32) == 0 and w(o + 36) >> 26 == 3]
    assert len(hits) == 1, f"CD stop found {len(hits)} times"
    return target(w(hits[0] + 36), hits[0] + 36)


def ambient_track(exe):
    """gp offset of the byte with the level's background track (+2), which the music goes back to when a speech
    or another track ends, 0 for none: 'lbu v0, x(gp); nop; bnez v0; addiu v0, v0, -2; addiu a0, zero, 9'."""
    w = lambda o: word(exe, o)
    hits = [w(o) & 0xFFFF for o in range(0x800, len(exe) - 20, 4)
            if w(o) >> 16 == 0x9382 and w(o + 4) == 0 and w(o + 8) >> 16 == 0x1440 and w(o + 12) == 0x2442FFFE
            and w(o + 16) == 0x24040009]
    assert len(hits) == 1, f"background track found {len(hits)} times"
    return hits[0]


def keep_routine(msi, bonus, start, stop, ambient):
    """Instead of ModifyStartInfo(a0) at the level start: with the bonus byte set only the pistols in Lara's
    hands (gun type 1 at 0x16 of start info a0, 0x34 bytes each). Lara's house (level 0) always gets
    ModifyStartInfo (no weapons), ends a New Game+ of the session and stops the music: its track 0 plays
    nothing, left the last level's music running and kept it as the background track, which came back after
    every speech; that byte is cleared as well. ModifyStartInfo gives the house a save crystal (byte 0x14 of
    its start info); it is taken away: a savegame made there replaced the normal one, with no level reached
    after coming from the title."""
    a = p3.Asm()
    a.lui("t0", hi(bonus))
    a.beqz("a0", "house"); a.i(0x24, "t0", "t1", bonus & 0xFFFF)  # lbu t1, bonus
    a.nop()
    a.bnez("t1", "keep"); a.nop()
    a.j(msi); a.nop()
    a.label("house")
    a.i(0x28, "t0", "zero", bonus & 0xFFFF)                       # sb zero, bonus
    a.i(0x28, "gp", "zero", ambient)                              # sb zero, background track(gp)
    a.addiu("sp", "sp", -24); a.sw("ra", 20, "sp")
    a.jal(stop); a.nop()
    a.jal(msi); a.move("a0", "zero")
    a.lui("t0", hi(start + 0x14))
    a.lw("ra", 20, "sp")
    a.i(0x28, "t0", "zero", (start + 0x14) & 0xFFFF)              # sb zero: no save crystal in the house
    a.jr("ra"); a.addiu("sp", "sp", 24)
    a.label("keep")
    a.sll("t1", "a0", 5); a.sll("t2", "a0", 4); a.addu("t1", "t1", "t2")
    a.sll("t2", "a0", 2); a.addu("t1", "t1", "t2")              # a0 * 0x34
    a.lui("t2", hi(start)); a.addiu("t2", "t2", start & 0xFFFF)
    a.addu("t1", "t1", "t2")
    a.addiu("t2", "zero", 1)
    a.jr("ra"); a.i(0x28, "t1", "t2", 0x16)                     # sb: the pistols
    return a.code()


def level_start_call(exe, msi):
    """File offset of the level start's 'jal ModifyStartInfo; move a0, s0' behind 'beq s1, s2; nop'."""
    call = 0x0C000000 | (msi & 0x0FFFFFFF) >> 2
    hits = [o for o in range(0x800, len(exe) - 8, 4)
            if word(exe, o) == call and word(exe, o + 4) == 0x02002021 and word(exe, o - 8) >> 26 == 4
            and word(exe, o - 4) == 0]
    assert len(hits) == 1, f"level start call found {len(hits)} times"
    return hits[0]


def tflip_table(title):
    """The XOR table as words."""
    table = title[1] + bytes(-len(title[1]) % 4)
    return list(struct.unpack(f"<{len(table) // 4}I", table))


def tflip_routine(title, base, table=None):
    """Switches the savegame title with the XOR table behind the routine (base: the routine's address), or,
    with table (its address), only the routine."""
    def build(table):
        a = p3.Asm()
        a.lui("t0", hi(title[0])); a.addiu("t0", "t0", title[0] & 0xFFFF)
        a.lui("t1", hi(table)); a.addiu("t1", "t1", table & 0xFFFF)
        a.addiu("t2", "zero", len(title[1]))
        a.label("loop")
        a.i(0x24, "t0", "t3", 0); a.i(0x24, "t1", "t4", 0)       # lbu t3, 0(t0); lbu t4, 0(t1)
        a.addiu("t2", "t2", -1)
        a.r(0x26, "t3", "t3", "t4")                               # xor
        a.i(0x28, "t0", "t3", 0)                                  # sb t3, 0(t0)
        a.addiu("t0", "t0", 1)
        a.bnez("t2", "loop"); a.addiu("t1", "t1", 1)
        a.jr("ra"); a.nop()
        return a.code()
    if table is not None:
        return build(table)
    return build(base + 4 * len(build(0))) + tflip_table(title)


def patched_before(orig, ver):
    """The executable with the patches before this one in bin/patch.sh and the ranges they write."""
    sh = open(os.path.join(BIN, "patch.sh"), encoding="utf-8").read()
    names = [l.split('"')[1].split(",") for l in sh.splitlines() if l.startswith("patches=")][0]
    exe, used = orig, []
    for name in names[:names.index("ngpchoice")]:
        d = open(os.path.join(BIN, "patches", ver, f"{name}.ips"), "rb").read()
        used += p3.ips_ranges(d)
        exe = rv.apply_ips(exe, d)
    return exe, used


def clean_cave(orig, ver, size, taken=()):
    exe, used = patched_before(orig, ver)
    used = used + list(taken)
    free = [(a, b) for a, b in rv.unused_routines(rv.words(exe), ver)
            if b - a >= size and b <= len(orig) and not any(x < b and a < y for x, y in used)]
    assert free, "no unused function large enough for the weapon clearing"
    return min(free, key=lambda f: f[1] - f[0])[0]


def page_clear_routine(ls):
    """Removes the load requester's texts (a0) and does what the load page does before its load: removes its
    title, the item name, the ring's heading and the button hints, their handles cleared."""
    return ([0x27BDFFE8, 0xAFBF0010, 0xAFB00014, rv.jal(ls["remove"]), 0] + ls["clear"]
            + [0x8FBF0010, 0x8FB00014, 0x03E00008, 0x27BD0018])


def card_texts(exe):
    """gp offset of the memory card texts and the byte offset of the savegame's name among them."""
    _, calls = nf.open_calls(exe)
    for c in calls:
        for k in range(1, 4):
            x = word(exe, c - 4 * k)
            if x >> 16 == 0x8C44 and word(exe, c - 4 * k - 8) >> 16 == 0x8F82:
                return word(exe, c - 4 * k - 8) & 0xFFFF, x & 0xFFFF
    raise AssertionError("memory card texts not found")


def close_stub(exe):
    hits = [o + LOAD for o in range(0x800, len(exe) - 12, 4)
            if word(exe, o) == 0x240A00B0 and word(exe, o + 4) == 0x01400008 and word(exe, o + 8) == 0x24090036]
    assert len(hits) == 1, f"close found {len(hits)} times"
    return hits[0]


def hi(x):
    return ((x + 0x8000) >> 16) & 0xFFFF


def routines(s, ls, mc, name, open_, close, bonus, base, texts, dialog, status, end, tflip, clean, buf, load_at=None):
    """Code at base, then the data: state byte (0 unknown, 1 locked, 2 unlocked), phase byte (1 mode
    choice, 2 level list), the level list's line count, the name buffer and the texts.
    Returns (words, addresses)."""
    def build(d):
        a = p3.Asm()
        state, phase, lines, mode, ngp_text, save_to, overwrite_on, load_from = d
        lines2 = lines + 1                                        # the load requester's line count

        def la(r, x):
            a.lui(r, hi(x)); a.addiu(r, r, x & 0xFFFF)

        # --- flags: 'lw v0, flags(gp)' with 0x400 also set when unlocked
        a.label("flags")
        a.lw("v0", s["flags"], "gp")                             # checked with the level select too
        a.lui("t3", hi(bonus)); a.i(0x24, "t3", "t3", bonus & 0xFFFF)   # lbu t3, bonus: a New Game+ running
        a.lui("t1", hi(state))
        a.beqz("t3", "f_state"); a.addiu("t2", "zero", 2)
        a.i(0x28, "t1", "t2", state & 0xFFFF)                     # unlocks it for this session
        a.label("f_state")
        a.i(0x24, "t1", "t2", state & 0xFFFF)                     # lbu t2, state
        a.nop()
        a.bnez("t2", "f_have"); a.nop()
        a.addiu("sp", "sp", -24); a.sw("ra", 20, "sp"); a.sw("v0", 16, "sp")
        a.b(0x01, "zero", "s1", "exists"); a.addiu("a0", "zero", ord("P"))   # bal (bgezal zero)
        a.addiu("t2", "v0", 1)                                    # 1 locked, 2 unlocked
        a.lui("t1", hi(state)); a.i(0x28, "t1", "t2", state & 0xFFFF)   # sb t2, state
        a.lw("v0", 16, "sp"); a.lw("ra", 20, "sp"); a.addiu("sp", "sp", 24)
        a.label("f_have")
        a.addiu("t2", "t2", -2)
        a.bnez("t2", "f_ret"); a.nop()
        a.ori("v0", "v0", LEVEL_SELECT)
        a.label("f_ret")
        a.jr("ra"); a.nop()
        # --- exists(a0 = letter): is the savegame name with that last letter on the memory card?
        a.label("exists")
        a.addiu("sp", "sp", -24); a.sw("ra", 20, "sp"); a.sw("s0", 16, "sp")
        a.move("s0", "a0")
        a.lw("t0", mc, "gp"); a.nop()
        a.lw("a1", name, "t0")
        la("t1", buf)
        a.move("t2", "t1")
        a.label("e_copy")
        a.i(0x24, "a1", "t3", 0)                                  # lbu t3, 0(a1)
        a.addiu("a1", "a1", 1)
        a.i(0x28, "t2", "t3", 0)                                  # sb t3, 0(t2)
        a.bnez("t3", "e_copy"); a.addiu("t2", "t2", 1)
        a.i(0x28, "t2", "s0", -2)                                 # sb s0, -2(t2): the last letter
        a.move("a0", "t1")
        a.jal(open_); a.addiu("a1", "zero", 1)
        a.addiu("t0", "zero", -1)
        a.beq("v0", "t0", "e_out"); a.move("s0", "zero")
        a.jal(close); a.move("a0", "v0")
        a.addiu("s0", "zero", 1)
        a.label("e_out")
        a.move("v0", "s0")
        a.lw("ra", 20, "sp"); a.lw("s0", 16, "sp")
        a.jr("ra"); a.addiu("sp", "sp", 24)
        # --- filling (a0 = requester, its heading is set)
        a.label("fill")
        a.lui("t1", hi(lines))
        a.i(0x24, "t1", "t2", lines & 0xFFFF)                     # lbu t2, lines
        a.lhu("t3", REQ_LINES, "a0")
        a.bnez("t2", "fi_saved"); a.nop()
        a.i(0x28, "t1", "t3", lines & 0xFFFF)                     # keeps the level list's line count
        a.label("fi_saved")
        a.lui("t1", hi(state)); a.i(0x24, "t1", "t2", state & 0xFFFF)   # lbu t2, state
        a.nop()
        a.addiu("t2", "t2", -2)
        a.beqz("t2", "fi_mode"); a.lui("t1", hi(phase))
        a.i(0x28, "t1", "zero", phase & 0xFFFF)                   # locked: the level list, a normal game
        a.lui("t1", hi(bonus)); a.i(0x28, "t1", "zero", bonus & 0xFFFF)
        a.move("t8", "ra")
        a.jal(clean); a.nop()                                     # no New Game+ or savegame weapons
        a.move("ra", "t8")
        a.j(s["fill"]); a.nop()
        a.label("fi_mode")
        a.addiu("t2", "zero", 1); a.i(0x28, "t1", "t2", phase & 0xFFFF)   # phase 1: the mode choice
        a.addiu("sp", "sp", -32); a.sw("ra", 28, "sp"); a.sw("s0", 24, "sp")
        a.move("s0", "a0"); a.sw("zero", 16, "sp")
        a.addiu("t2", "zero", MODE_LINES); a.sh("t2", REQ_LINES, "s0")
        la("a1", mode); a.move("a2", "zero")
        a.jal(s["heading"]); a.move("a3", "zero")
        a.jal(s["clear"]); a.move("a0", "s0")
        a.lw("t0", s["texts"], "gp"); a.move("a0", "s0")
        a.lw("a1", 4 * NEW_GAME, "t0"); a.move("a2", "zero")
        a.jal(s["add"]); a.move("a3", "zero")
        a.move("a0", "s0")
        la("a1", ngp_text); a.move("a2", "zero")
        a.jal(s["add"]); a.move("a3", "zero")
        a.lw("ra", 28, "sp"); a.lw("s0", 24, "sp")
        a.jr("ra"); a.addiu("sp", "sp", 32)
        # --- choice: the requester's answer (a0 = requester)
        a.label("choice")
        a.addiu("sp", "sp", -32); a.sw("ra", 28, "sp"); a.sw("s0", 24, "sp")
        a.move("s0", "a0"); a.sw("zero", 16, "sp")
        a.jal(s["requester"]); a.nop()
        a.lui("t1", hi(phase)); a.i(0x24, "t1", "t2", phase & 0xFFFF)   # lbu t2, phase
        a.addiu("t3", "zero", 1)
        a.bne("t2", "t3", "c_out"); a.nop()                       # not the mode choice: as the game answers
        a.beqz("v0", "c_out"); a.nop()                            # still open
        a.bltz("v0", "c_out"); a.i(0x28, "t1", "zero", phase & 0xFFFF)   # cancelled
        a.addiu("t2", "v0", -2); a.sltiu("t2", "t2", 1)           # 1 for the New Game+
        a.lui("t1", hi(bonus)); a.i(0x28, "t1", "t2", bonus & 0xFFFF)   # sb t2, bonus
        a.bnez("t2", "c_ngp"); a.nop()
        a.jal(clean); a.nop()                                     # a normal game: no New Game+ weapons
        a.label("c_ngp")
        a.lw("t0", s["flags"], "gp"); a.nop()
        a.andi("t0", "t0", LEVEL_SELECT)
        a.bnez("t0", "c_levels"); a.nop()
        a.bra("c_out"); a.addiu("v0", "zero", 1)                  # without the level select: the first level
        a.label("c_levels")                                      # the level list in the same requester
        a.lui("t1", hi(phase)); a.addiu("t2", "zero", 2); a.i(0x28, "t1", "t2", phase & 0xFFFF)
        a.jal(s["clear"]); a.move("a0", "s0")
        a.lui("t1", hi(lines)); a.i(0x24, "t1", "t2", lines & 0xFFFF)   # lbu t2, lines
        a.sh("zero", REQ_SELECTED, "s0"); a.sh("zero", REQ_TOP, "s0")
        a.sh("t2", REQ_LINES, "s0")
        a.lw("t0", s["texts"], "gp"); a.move("a0", "s0")
        a.lw("a1", 4 * SELECT_LEVEL, "t0"); a.move("a2", "zero")
        a.jal(s["heading"]); a.move("a3", "zero")
        a.jal(s["fill"]); a.move("a0", "s0")
        a.move("v0", "zero")                                      # still open
        a.label("c_out")
        a.lw("ra", 28, "sp"); a.lw("s0", 24, "sp")
        a.jr("ra"); a.addiu("sp", "sp", 32)
        # --- load call (instead of the load from the card)
        a.label("lcall")
        a.addiu("sp", "sp", -24); a.sw("ra", 20, "sp"); a.sw("s0", 16, "sp")
        a.lw("t0", ls["mode"], "gp")
        a.lui("t1", hi(bonus))
        a.lw("t2", ls["level"], "gp")                             # the level running
        a.addiu("t0", "t0", -1)                                   # inventory mode 1: the title
        a.beqz("t0", "l_ask"); a.i(0x24, "t1", "v0", bonus & 0xFFFF)   # lbu v0, bonus
        a.bnez("t2", "l_load"); a.i(0x28, "t1", "v0", bonus & 0xFFFF)  # in a level: the file of the mode running
        a.label("l_ask")                                          # the title and Lara's home ask
        a.b(0x01, "zero", "s1", "exists"); a.addiu("a0", "zero", ord("G"))
        a.move("s0", "v0")
        a.b(0x01, "zero", "s1", "exists"); a.addiu("a0", "zero", ord("P"))
        a.beqz("s0", "l_one"); a.nop()
        a.beqz("v0", "l_one"); a.nop()
        a.lui("t1", hi(phase)); a.addiu("t2", "zero", 3); a.i(0x28, "t1", "t2", phase & 0xFFFF)
        a.bra("l_out"); a.addiu("v0", "zero", 1)                  # both: the requester asks
        a.label("l_one")
        a.lui("t1", hi(bonus)); a.i(0x28, "t1", "v0", bonus & 0xFFFF)   # only the New Game+ one: 1
        a.label("l_load")
        a.jal(ls["hints"]); a.sw("zero", status, "gp")            # check that file again
        a.jal(ls["load"]); a.nop()
        a.label("l_out")
        a.lw("ra", 20, "sp"); a.lw("s0", 16, "sp")
        a.jr("ra"); a.addiu("sp", "sp", 24)
        # --- load filling (a0 = requester, heading "Load Game")
        a.label("lfill")
        a.lui("t1", hi(phase)); a.i(0x24, "t1", "t2", phase & 0xFFFF)   # lbu t2, phase
        a.addiu("t3", "zero", 3)
        a.bne("t2", "t3", "lf_orig"); a.nop()
        a.addiu("sp", "sp", -32); a.sw("ra", 28, "sp"); a.sw("s0", 24, "sp")
        a.move("s0", "a0"); a.sw("zero", 16, "sp")
        a.lhu("t3", REQ_LINES, "s0")
        a.lui("t1", hi(lines2)); a.i(0x28, "t1", "t3", lines2 & 0xFFFF)   # keeps its line count
        a.addiu("t2", "zero", MODE_LINES); a.sh("t2", REQ_LINES, "s0")
        a.jal(s["clear"]); a.move("a0", "s0")
        a.lw("t0", s["texts"], "gp"); a.move("a0", "s0")
        a.lw("a1", 4 * NEW_GAME, "t0"); a.move("a2", "zero")
        a.jal(s["add"]); a.move("a3", "zero")
        a.move("a0", "s0")
        la("a1", ngp_text); a.move("a2", "zero")
        a.jal(s["add"]); a.move("a3", "zero")
        a.lw("ra", 28, "sp"); a.lw("s0", 24, "sp")
        a.jr("ra"); a.addiu("sp", "sp", 32)
        a.label("lf_orig")
        a.j(ls["load_fill"]); a.nop()
        # --- load requester call (a0 = requester)
        a.label("ldisp")
        a.addiu("sp", "sp", -32); a.sw("ra", 28, "sp"); a.sw("s0", 24, "sp")
        a.move("s0", "a0"); a.sw("zero", 16, "sp")
        a.jal(s["requester"]); a.nop()
        a.lui("t1", hi(phase)); a.i(0x24, "t1", "t2", phase & 0xFFFF)   # lbu t2, phase
        a.addiu("t3", "zero", 3)
        a.bne("t2", "t3", "ld_out"); a.nop()
        a.beqz("v0", "ld_out"); a.nop()                           # still open
        a.i(0x28, "t1", "zero", phase & 0xFFFF)                   # the choice is over
        a.lui("t1", hi(lines2)); a.i(0x24, "t1", "t2", lines2 & 0xFFFF)   # lbu t2, lines2
        a.nop()
        a.bltz("v0", "ld_out"); a.sh("t2", REQ_LINES, "s0")       # cancelled
        a.addiu("t2", "v0", -2); a.sltiu("t2", "t2", 1)           # 1 for the New Game+
        a.lui("t1", hi(bonus)); a.i(0x28, "t1", "t2", bonus & 0xFFFF)
        a.jal(ls["pclr"]); a.move("a0", "s0")                     # the texts, before the dialog clears all
        a.sw("zero", status, "gp")                                # check that file again
        a.jal(ls["load"]); a.nop()                                # the load from the card, as the game does it
        a.addiu("t0", "zero", 1)
        a.bne("v0", "t0", "ld_out"); a.addiu("v0", "zero", -1)    # failed: as cancelled
        a.addiu("t0", "zero", ls["load_y"]); a.sh("t0", 0x12, "s0")
        a.jal(ls["init"]); a.move("a0", "s0")
        a.lw("t0", s["texts"], "gp"); a.move("a0", "s0")
        a.lw("a1", 4 * 4, "t0"); a.move("a2", "zero")             # "Load Game"
        a.jal(s["heading"]); a.move("a3", "zero")
        a.jal(ls["load_fill"]); a.move("a0", "s0")
        a.move("v0", "zero")                                      # open: the savegame's positions
        a.label("ld_out")
        a.lw("ra", 28, "sp"); a.lw("s0", 24, "sp")
        a.jr("ra"); a.addiu("sp", "sp", 32)
        # --- the save routine's dialog: in the New Game+ with its own first line
        a.label("sdlg")
        a.addiu("sp", "sp", -40); a.sw("ra", 36, "sp"); a.sw("s0", 32, "sp"); a.sw("s1", 28, "sp")
        a.lui("t0", hi(bonus)); a.i(0x24, "t0", "t0", bonus & 0xFFFF)   # lbu t0, bonus
        a.sw("zero", status, "gp")                                # check the file of this mode again
        a.beqz("t0", "sd_plain"); a.lw("t0", mc, "gp")
        la("t1", save_to)
        a.lw("s0", 4 * SAVE_TO, "t0"); a.lw("s1", 4 * OVERWRITE_ON, "t0"); a.lw("t2", 4 * LOAD_FROM, "t0")
        a.sw("t1", 4 * SAVE_TO, "t0")
        la("t1", overwrite_on); a.sw("t1", 4 * OVERWRITE_ON, "t0")
        la("t1", load_from); a.sw("t1", 4 * LOAD_FROM, "t0")
        a.sw("t2", 24, "sp")
        a.jal(tflip); a.nop()                                     # the New Game+ title
        a.jal(dialog); a.nop()
        a.jal(tflip); a.nop()                                     # and back
        a.lw("t0", mc, "gp"); a.lw("t2", 24, "sp")
        a.sw("s0", 4 * SAVE_TO, "t0"); a.sw("s1", 4 * OVERWRITE_ON, "t0")
        a.sw("t2", 4 * LOAD_FROM, "t0")
        a.bra("sd_out"); a.nop()
        a.label("sd_plain")
        a.jal(dialog); a.nop()
        a.label("sd_out")
        a.lw("ra", 36, "sp"); a.lw("s1", 28, "sp"); a.lw("s0", 32, "sp")
        a.jr("ra"); a.addiu("sp", "sp", 40)
        # --- game completed (newgameplus.ips's routine: at = hi(bonus), t0 = the bonus byte before): the byte
        # becomes 1 after a normal game, 2 or more after a New Game+, until the end statistics' save
        a.label("gameend")
        a.addiu("t0", "t0", 1)
        a.j(end["credits"]); a.i(0x28, "at", "t0", bonus & 0xFFFF)      # sb t0, bonus
        # --- the end statistics' save (a0 for the save routine): a normal game is offered as a normal savegame
        # first (all levels it reached), then as the New Game+ (the first level only: the weapon clearing makes
        # the later ones unavailable), but only while there is no New Game+ savegame (a finished one keeps its
        # levels); a finished New Game+ is offered as the New Game+ with the levels it reached. Both start in
        # the first level (the statistics set level 20, Tomb Raider III's bonus level, when every secret was
        # found)
        a.label("endsave")
        a.addiu("sp", "sp", -32); a.sw("ra", 28, "sp"); a.sw("s0", 24, "sp")
        a.sw("a0", 16, "sp")
        a.lui("t0", hi(bonus)); a.i(0x24, "t0", "s0", bonus & 0xFFFF)     # lbu s0, bonus
        a.addiu("t1", "zero", 1)
        a.sh("t1", (bonus - 2) & 0xFFFF, "t0")                    # current level: the first
        a.addiu("s0", "s0", -1)                                   # 0 normal, 1 New Game+
        a.label("es_pass")                                        # s0 2: the New Game+ after a normal game
        a.r(0x2B, "t1", "zero", "s0")                             # sltu t1, zero, s0: the bonus byte
        a.lui("t0", hi(bonus)); a.i(0x28, "t0", "t1", bonus & 0xFFFF)
        a.jal(end["msi"]); a.addiu("a0", "zero", 1)               # the first level's start
        a.addiu("t1", "s0", -2)
        a.bnez("t1", "es_save"); a.nop()
        a.jal(clean); a.nop()                                     # the later levels: not available
        a.label("es_save")
        a.lw("a0", 16, "sp"); a.addiu("a1", "zero", end["size"])
        a.jal(end["save"]); a.move("a2", "zero")
        a.bnez("s0", "es_out"); a.nop()
        a.b(0x01, "zero", "s1", "exists"); a.addiu("a0", "zero", ord("P"))   # bal: a New Game+ savegame?
        a.bnez("v0", "es_out"); a.nop()                           # kept as it is
        a.bra("es_pass"); a.addiu("s0", "zero", 2)
        a.label("es_out")
        a.lw("ra", 28, "sp"); a.lw("s0", 24, "sp")
        a.jr("ra"); a.addiu("sp", "sp", 32)
        # --- direct start: always a normal game
        a.label("direct")
        a.move("t8", "ra")
        a.jal(clean); a.nop()                                     # a normal game: no New Game+ weapons
        a.move("ra", "t8")
        a.lui("t0", hi(bonus)); a.i(0x28, "t0", "zero", bonus & 0xFFFF)
        a.j(s["start"]); a.nop()
        return a

    n = 4 * len(build([0] * 8).code())
    mode, ngp_text, (save_to, overwrite_on), load_from = texts
    data = n + 4                                                  # state, phase, lines, lines2
    strings = [mode, ngp_text, save_to, overwrite_on] + ([load_from] if load_at is None else [])
    tail, at = b"", []
    for x in strings:                                             # a text that ends another one is not stored twice
        if any(y != x and y.endswith(x) for y in strings):
            at.append(None)
        else:
            at.append(base + data + len(tail)); tail += x + b"\0"
    for k, x in enumerate(strings):
        if at[k] is None:
            y = next(i for i, y in enumerate(strings) if y != x and y.endswith(x) and at[i] is not None)
            at[k] = at[y] + len(strings[y]) - len(x)
    d = [base + n, base + n + 1, base + n + 2] + at + ([load_at] if load_at is not None else [])
    a = build(d)
    code = a.code()
    assert 4 * len(code) == n
    tail += bytes(-len(tail) % 4)
    return (code + [0] + list(struct.unpack(f"<{len(tail) // 4}I", tail)),
            {k: base + 4 * v for k, v in a.labels.items()})


def build(orig, ver, lang=None):
    s = sites(orig)
    ls = load_sites(orig)
    mc, name = card_texts(orig)
    open_, _ = nf.open_calls(orig)
    close = close_stub(orig)
    bonus = ngp.bonus_byte(orig)
    off, addr, end = nf.appended_after(orig, ver)
    ntsc = lang                                                   # the NTSC language variant of a US executable
    lang = lang or LANGUAGE[ver]
    dialog_site, dialog = save_dialog(orig)
    msi, _, levels = start_info_loops(orig)
    clean_code = clean_routine(levels, start_infos(orig))
    keep_code = keep_routine(msi, ngp.bonus_byte(orig), start_infos(orig), cd_stop(orig), ambient_track(orig))
    title = card_title(orig, ver, ntsc)
    size = 4 * (len(clean_code) + len(keep_code))
    clean = clean_cave(orig, ver, size)
    keep = clean + 4 * len(clean_code)
    tflip, split = None, False
    for k, n in enumerate((len(tflip_routine(title, 0)), len(tflip_routine(title, 0, 0)))):
        try:
            tflip = clean_cave(orig, ver, 4 * n, [(clean, clean + size)])
            split = k == 1                                        # the table behind the routines in the block
            break
        except AssertionError:
            pass                                                  # else all of it behind the routines
    taken = [(clean, clean + size)] + ([] if tflip is None else [(tflip, tflip + 4 * n)])
    pclr_code = page_clear_routine(ls)
    try:
        pclr = clean_cave(orig, ver, 4 * len(pclr_code), taken)
    except AssertionError:                                        # else behind the weapon clearing, in its function
        exe, _ = patched_before(orig, ver)
        fend = next(e for f, e in rv.unused_routines(rv.words(exe), ver) if f <= clean < e)
        assert clean + size + 4 * len(pclr_code) <= fend, "no room for the page's text removal"
        pclr = clean + size
    taken.append((pclr, pclr + 4 * len(pclr_code)))
    ls["pclr"] = pclr + LOAD
    jump, credits = credits_jump(orig, ver)
    endsave_site, save, save_size = end_save_call(orig)
    assert hi(bonus) == hi(bonus - 2), "bonus byte and current level need two lui"

    load_text, load_cave = LOAD_TEXT[lang], None

    def block(tflip_addr):
        return routines(s, ls, mc, name, open_, close, bonus, addr,
                        (MODE_TEXT[lang], NGP_TEXT[lang], SAVE_TEXT[lang], load_text), dialog, card_status(orig),
                        dict(msi=msi, levels=levels, credits=credits, save=save, size=save_size), tflip_addr, clean + LOAD,
                        nf.name_buffer(orig, ver), None if load_cave is None else load_cave + LOAD)
    def layout():
        if tflip is None:
            words, lab = block(0)
            tflip_addr = addr + 4 * len(words)
            words, lab = block(tflip_addr)
            return words + tflip_routine(title, tflip_addr), lab, []
        if split:
            words, lab = block(tflip + LOAD)
            table = addr + 4 * len(words)
            return words + tflip_table(title), lab, [(tflip, tflip_routine(title, tflip + LOAD, table))]
        words, lab = block(tflip + LOAD)
        return words, lab, [(tflip, tflip_routine(title, tflip + LOAD))]
    words, lab, caves = layout()
    if off + 4 * len(words) > end:                                # the load text into an unused function
        text = load_text + bytes(4 - len(load_text) % 4)
        taken = taken + [(c, c + 4 * len(w)) for c, w in caves]
        load_cave = clean_cave(orig, ver, len(text), taken)
        words, lab, caves = layout()
        caves.append((load_cave, list(struct.unpack(f"<{len(text) // 4}I", text))))
    assert off + 4 * len(words) <= end, "no room left in the appended block"
    patches = [(off, words), (clean, clean_code + keep_code), (pclr, pclr_code)] + caves + [(level_start_call(orig, msi), [rv.jal(keep + LOAD)]),
               (s["flag_site"], [rv.jal(lab["flags"]), 0]),
               (s["req_call"], [rv.jal(lab["choice"])]),
               (s["fill_site"], [rv.jal(lab["fill"])]),
               (s["direct"], [rv.jal(lab["direct"])]),
               (ls["load_call"] - 8, [0]),                                 # the button hints: removed at the load
               (ls["load_call"], [rv.jal(lab["lcall"])]),
               (ls["load_fill_site"], [rv.jal(lab["lfill"])]),
               (ls["load_disp"], [rv.jal(lab["ldisp"])]),
               (dialog_site, [rv.jal(lab["sdlg"])]),
               (load_dialog(orig, dialog), [rv.jal(lab["sdlg"])]),
               (jump - 8, [0x3C010000 | hi(bonus), 0x90280000 | bonus & 0xFFFF,          # lui at; lbu t0, bonus
                           0x08000000 | (lab["gameend"] & 0x0FFFFFFF) >> 2, 0]),
               (endsave_site, [rv.jal(lab["endsave"])])]
    return patches, (f"flags gp+{s['flags']:#x} {s['flag_site'] + LOAD:#x}, requester {s['req_call'] + LOAD:#x}, "
                     f"fill {s['fill_site'] + LOAD:#x}, direct {s['direct'] + LOAD:#x}, load {ls['load_call'] + LOAD:#x} "
                     f"{ls['load_fill_site'] + LOAD:#x} {ls['load_disp'] + LOAD:#x} req {ls['load_req']:#x} "
                     f"y {ls['load_y']}, texts gp+{s['texts']:#x}, "
                     f"card texts gp+{mc:#x} name {name:#x}, close {close:#x}, bonus {bonus:#x} -> {addr:#x} "
                     f"({4 * len(words)} bytes, {end - off - 4 * len(words)} left)")


if __name__ == "__main__":
    ap = argparse.ArgumentParser()
    ap.add_argument("--versions", default="us,ukrev,derev,frrev,itrev,sprev,jp,jp2,as,as2")
    ap.add_argument("--exe", nargs="*", default=[], help="VERSION=PATH for executables not in bin/orig")
    a = ap.parse_args()
    paths = {v: os.path.join(BIN, "orig", f"{v}.exe") for v in a.versions.split(",") if v}
    paths.update(dict(e.split("=", 1) for e in a.exe))
    for ver, path in paths.items():
        p, desc = build(open(path, "rb").read(), ver)
        open(os.path.join(BIN, "patches", ver, "ngpchoice.ips"), "wb").write(rv.ips(p))
        print(f"{ver}: {desc}")
        if ver in NTSC_VERSIONS:
            for lang in NTSC_LANGUAGES:
                p, desc = build(open(path, "rb").read(), ver, lang)
                open(os.path.join(BIN, "patches", ver, f"ngpchoice_{lang}.ips"), "wb").write(rv.ips(p))
                print(f"{ver} ({lang}, NTSC): {desc}")
