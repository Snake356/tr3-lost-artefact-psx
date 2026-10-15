#!/bin/sh

# Versions of the game
versions="$(ls "./ver/." | sed 's|^\(.*\)\.s$|\1|g')"

# Builds the patches for each version
rm -rf "./out"
mkdir "./out"
for version in ${versions}; do
	rm -rf "../${version}"
	mkdir "../${version}"
	sed "s|INCLUDE_VERSION_VARIABLE|${version}|g" "patch.s" > "${version}.s"
	# Runs the Assembler
	./tools/bin/armips "${version}.s"
	rm "${version}.s"
	# Converts the binary files to patches
	patfiles="$(ls "./out" | grep "\.pat$")"
	for patfile in ${patfiles}; do
		./tools/bin/bin2ips "./out/${patfile}" "./out/code.bin" "../${version}/${patfile%'.pat'}.ips"
	done
done
rm -rf "./out"
