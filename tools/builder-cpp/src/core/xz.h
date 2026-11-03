// .xz decompression (LZMA2), the format of the project's patches (Python lzma.compress).
#pragma once
#include "bytes.h"

Bytes xz_decompress(const Bytes &in);
