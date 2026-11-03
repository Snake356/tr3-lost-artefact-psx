// The patch formats the builder applies to the player's files.
#pragma once
#include "bytes.h"

// IPS: the executable's patches (bin/patches/<version>/*.ips).
Bytes apply_ips(const Bytes &data, const Bytes &ips);

// LAD1 binary difference (xz-packed copy/add instructions); checks the CRC32 of source and result.
Bytes apply_bindiff(const Bytes &src, const Bytes &patch);

// Texture pages: per 16-bit pixel the 5-bit colour steps and the transparency bit (xz-packed, CRC32-checked).
Bytes apply_texture_patch(const Bytes &bmp, const Bytes &patch);

// The animation block troi exports, with every bone rotation in the two-word form (as the project's files).
Bytes rebuild_anim_block(const Bytes &block);
