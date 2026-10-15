#!/bin/sh
cd ../../
patch bin.c patches/be/bin.patch
patch imgtools.c patches/be/imgtools.patch
patch obj.c patches/be/obj.patch
patch pcx.c patches/be/pcx.patch
patch raw.c patches/be/raw.patch
patch str.c patches/be/str.patch
patch wad.c patches/be/wad.patch
cd patches/be
