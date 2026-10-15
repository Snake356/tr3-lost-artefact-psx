#!/usr/bin/env python3
"""Inventory of a TR3 PSX level file (header word -56): walks the file along the layout
that trview's Level_tr3_psx.cpp reads, and prints the counts per file.
Writes nothing. If the chain breaks somewhere, the position with offset is shown in the output.

Layout (trview, Level_tr3_psx.cpp, as of 24.08.2026):
  i32 Version | sound bank: u32 n, u32 off[n], u32 size, data
  13 x code overlay: i32 size [, data, i32 size2, data]  (0 = empty)
  u16 NumRooms, rooms (roomlets in the data block; portals, sectors, ambient u16,
      LightMode u16, lights 24 B, statics 20 B, alt i16, flags i16, water, hall, filler)
  Floordata | 729 x u16 outdoor map | u32 n, n x u8 outdoor room table | i32 n, n x 8 bbox
  MeshData | MeshPointer | Anims 32 | StateChanges 6 | Dispatches 8 | AnimCmds 2 |
  MeshTree 4 | Frames 2 | Models 20 (18+2) | Statics 32 |
  u32 n tiles (4 bit, 32768 B) | u32 n CLUT pairs (Japan: 2 extra bytes before) x 2 x 32 B |
  Objtex 16 | Sprite-Tex 16 | SpriteSeq 8 | Cameras 16 | Sound sources 16 | Boxes 8 |
  Overlaps 2 | Zones 20 per box | Animtex u32 n words | Entities 24 | i32 horizon colour |
  u32 n room textures x 48 (16 + 2 x 16 variants) | SoundMap 370 x i16 | SoundDetails 8

Usage:
    python psx_inventory.py <file.psx|folder> [--rooms]   (--rooms: counts per room)
"""
import struct
import sys
from pathlib import Path


class Reader:
    def __init__(self, data):
        self.d = data
        self.o = 0

    def u8(self):
        v = self.d[self.o]
        self.o += 1
        return v

    def u16(self):
        v = struct.unpack_from('<H', self.d, self.o)[0]
        self.o += 2
        return v

    def i16(self):
        v = struct.unpack_from('<h', self.d, self.o)[0]
        self.o += 2
        return v

    def u32(self):
        v = struct.unpack_from('<I', self.d, self.o)[0]
        self.o += 4
        return v

    def i32(self):
        v = struct.unpack_from('<i', self.d, self.o)[0]
        self.o += 4
        return v

    def skip(self, n):
        self.o += n


def read_room(r, per_room):
    x, z, yb, yt = struct.unpack_from('<iiii', r.d, r.o)
    r.skip(16)
    ndw = r.u32()
    start = r.o
    n_roomlets = r.u32()
    offs = [r.u32() for _ in range(n_roomlets)]
    nv = nt = nq = 0
    for off in offs:
        r.o = start + off
        r.skip(6)                       # bbox 6 bytes
        v = r.u8()
        t = r.u8()
        r.skip(v * 4 + t * 4)
        nv += v
        nt += t
        more = True
        while more:
            tex = r.u32()
            r.skip(12)                  # 3 face words
            for k in range(3):
                if ((tex >> (10 * k)) & 0x3ff) == 0x3ff:
                    more = False
                    break
                nq += 1
    r.o = start + ndw * 2
    npo = r.u16()
    r.skip(npo * 32)
    nz = r.u16()
    nx = r.u16()
    r.skip(nz * nx * 8)
    amb = r.i16()
    lm = r.i16()
    nl = r.u16()
    r.skip(nl * 24)
    ns = r.u16()
    r.skip(ns * 20)
    alt = r.i16()
    flags = r.u16()
    water = r.u8()
    hall = r.u8()
    r.skip(1)
    per_room.append(dict(x=x, z=z, yb=yb, yt=yt, ndw=ndw, roomlets=n_roomlets, v=nv, q=nq, t=nt,
                         portals=npo, nz=nz, nx=nx, amb=amb, lm=lm, lights=nl, statics=ns,
                         alt=alt, flags=flags, water=water, hall=hall))


def inventory(path):
    d = path.read_bytes()
    r = Reader(d)
    z = {'file': path.name, 'bytes': len(d)}
    per_room = []
    try:
        z['version'] = r.i32()
        n = r.u32()
        r.skip(n * 4)
        size = 0
        if n:                           # without samples there is NO size field (cutscenes)
            size = r.u32()
            r.skip(size)
        z['samples'] = n
        z['sample_bytes'] = size
        overlays = []
        for _ in range(13):
            s = r.i32()
            if s != 0:
                r.skip(s)
                s2 = r.i32()
                r.skip(s2)
                overlays.append((s, s2))
            else:
                overlays.append((0, 0))
        z['overlays'] = sum(1 for a, _ in overlays if a)
        z['overlay_bytes'] = sum(a + b for a, b in overlays)
        z['p_rooms'] = '%#x' % r.o
        nr = r.u16()
        z['rooms'] = nr
        for _ in range(nr):
            read_room(r, per_room)
        z['vertices'] = sum(x['v'] for x in per_room)
        z['quads'] = sum(x['q'] for x in per_room)
        z['triangles'] = sum(x['t'] for x in per_room)
        z['lights'] = sum(x['lights'] for x in per_room)
        z['room_statics'] = sum(x['statics'] for x in per_room)
        nfd = r.u32()
        r.skip(nfd * 2)
        z['floordata'] = nfd
        r.skip(729 * 2)
        nout = r.u32()
        r.skip(nout)
        z['outdoor_table'] = nout
        nbb = r.i32()
        r.skip(nbb * 8)
        z['bboxes'] = nbb
        nmd = r.u32()
        r.skip(nmd * 2)
        z['meshdata'] = nmd
        nmp = r.u32()
        r.skip(nmp * 4)
        z['meshptr'] = nmp
        na = r.u32()
        r.skip(na * 32)
        z['anims'] = na
        nsc = r.u32()
        r.skip(nsc * 6)
        z['statechg'] = nsc
        nad = r.u32()
        r.skip(nad * 8)
        z['dispatch'] = nad
        nac = r.u32()
        r.skip(nac * 2)
        z['animcmd'] = nac
        nmt = r.u32()
        r.skip(nmt * 4)
        z['meshtree'] = nmt
        nfr = r.u32()
        r.skip(nfr * 2)
        z['frames'] = nfr
        nmo = r.u32()
        r.skip(nmo * 20)
        z['models'] = nmo
        nst = r.u32()
        r.skip(nst * 32)
        z['statics'] = nst
        ntx = r.u32()
        r.skip(ntx * 32768)
        z['tiles4'] = ntx
        z['p_clut'] = '%#x' % r.o
        ncl = r.u32()
        z['japan_bytes'] = 0
        if ncl & 0xffff0000:
            r.o -= 2
            ncl = r.u32()
            z['japan_bytes'] = 2
        r.skip(ncl * 2 * 32)
        z['cluts'] = ncl * 2
        z['p_objtex'] = '%#x' % r.o
        not_ = r.u32()
        r.skip(not_ * 16)
        z['objtex'] = not_
        nsp = r.u32()
        r.skip(nsp * 16)
        z['spritetex'] = nsp
        nss = r.u32()
        r.skip(nss * 8)
        z['spriteseq'] = nss
        nca = r.u32()
        r.skip(nca * 16)
        z['cameras'] = nca
        nso = r.u32()
        r.skip(nso * 16)
        z['soundsources'] = nso
        nbx = r.u32()
        r.skip(nbx * 8)
        z['boxes'] = nbx
        nov = r.u32()
        r.skip(nov * 2)
        z['overlaps'] = nov
        r.skip(nbx * 20)
        nat = r.u32()
        r.skip(nat * 2)
        z['animtex'] = nat
        nen = r.u32()
        z['p_entities'] = '%#x' % r.o
        r.skip(nen * 24)
        z['entities'] = nen
        z['horizon'] = '%08x' % r.u32()
        z['p_roomtex'] = '%#x' % r.o
        nrt = r.u32()
        r.skip(nrt * 48)
        z['roomtex'] = nrt
        z['p_soundmap'] = '%#x' % r.o
        r.skip(370 * 2)
        nsd = r.u32()
        r.skip(nsd * 8)
        z['sounddetails'] = nsd
        z['rest'] = len(d) - r.o
        z['status'] = 'ok'
    except Exception as e:  # the inventory should name the position, not abort
        z['status'] = 'ERROR @%#x: %s' % (r.o, e)
    return z, per_room


COLUMNS = ['file', 'bytes', 'version', 'samples', 'sample_bytes', 'overlays', 'overlay_bytes', 'p_rooms',
           'rooms', 'vertices', 'quads', 'triangles', 'lights', 'room_statics', 'floordata',
           'outdoor_table', 'bboxes', 'meshdata', 'meshptr', 'anims', 'statechg', 'dispatch', 'animcmd',
           'meshtree', 'frames', 'models', 'statics', 'tiles4', 'p_clut', 'japan_bytes', 'cluts', 'p_objtex', 'objtex',
           'spritetex', 'spriteseq', 'cameras', 'soundsources', 'boxes', 'overlaps', 'animtex', 'entities',
           'horizon', 'p_roomtex', 'roomtex', 'p_soundmap', 'sounddetails', 'rest', 'status']


def main():
    args = [a for a in sys.argv[1:] if not a.startswith('--')]
    show_rooms = '--rooms' in sys.argv
    target = Path(args[0])
    if target.is_dir():
        files = sorted({p.resolve() for p in target.iterdir() if p.suffix.lower() == '.psx'})
    else:
        files = [target]
    print('\t'.join(COLUMNS))
    for p in files:
        z, per_room = inventory(p)
        print('\t'.join(str(z.get(s, '')) for s in COLUMNS))
        if show_rooms:
            for i, x in enumerate(per_room):
                print('  R%-3d bbox %6d %6d %6d %6d  roomlets %2d  v %3d q %3d t %3d  portals %2d  sect %2dx%-2d'
                      '  amb %5d lm %d  light %2d stat %2d  alt %3d flags %04x water %d hall %d'
                      % (i, x['x'], x['z'], x['yb'], x['yt'], x['roomlets'], x['v'], x['q'], x['t'],
                         x['portals'], x['nz'], x['nx'], x['amb'], x['lm'], x['lights'], x['statics'],
                         x['alt'], x['flags'], x['water'], x['hall']))


if __name__ == '__main__':
    main()
