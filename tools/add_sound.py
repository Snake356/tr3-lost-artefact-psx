"""Adds a sound a level's sound map lacks (Tomb Raider III PC level): one sample index (the sample's number in
levels/sfx/common) and one sound details entry are appended and the sound map points the sound at them.
Default: Shakespeare Cliff's strobe light (room 68, switched on with the drill in room 21) calls SFX_ALARM_1 (208),
which chunnel.tr2 lacks (TRX adds it with alarm_sfx.bin); sample 215 with the details Sleeping with the Fishes has
for it (volume 76, radius 11, pitch 199, flags 0x0004). Its PS1 sample takes 5,056 bytes of sound memory.
Usage: add_sound.py level.tr2 [sound sample volume radius chance pitch flags]"""
import os
import struct
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import drop_object                     # noqa: E402

ALARM = (208, 215, 76, 11, 0, 199, 0x0004)


def patch(path, sound, sample, volume, radius, chance, pitch, flags):
    b = bytearray(open(path, "rb").read())
    u16 = lambda o: struct.unpack_from("<H", b, o)[0]
    u32 = lambda o: struct.unpack_from("<I", b, o)[0]
    p = drop_object.sections(bytes(b))["items"]
    p += 4 + u32(p) * 24
    p += 8192
    p += 2 + u16(p) * 16
    p += 2 + u16(p)
    smap = p
    if struct.unpack_from("<h", b, smap + 2 * sound)[0] >= 0:
        return "sound %d is there already" % sound
    pd = smap + 370 * 2
    nd = u32(pd)
    pi = pd + 4 + nd * 8
    ni = u32(pi)
    idx = list(struct.unpack_from("<%dI" % ni, b, pi + 4))
    if sample in idx:
        k = idx.index(sample)
    else:
        k = ni
        b[pi + 4 + ni * 4:pi + 4 + ni * 4] = struct.pack("<I", sample)
        struct.pack_into("<I", b, pi, ni + 1)
    b[pi:pi] = struct.pack("<HBBBBH", k, volume, radius, chance, pitch, flags)
    struct.pack_into("<I", b, pd, nd + 1)
    struct.pack_into("<h", b, smap + 2 * sound, nd)
    open(path, "wb").write(b)
    return "sound %d -> details %d, sample index %d (sample %03d)" % (sound, nd, k, sample)


if __name__ == "__main__":
    args = [int(x, 0) for x in sys.argv[2:]] or list(ALARM)
    print(sys.argv[1], patch(sys.argv[1], *args))
