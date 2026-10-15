#!/bin/sh
cd ../../
patch obj.c patches/windows/obj.patch
patch wad.c patches/windows/wad.patch
patch raw.c patches/windows/raw.patch
cd patches/windows
