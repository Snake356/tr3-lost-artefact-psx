######################################################################
# Tomb Raider : The Lost Artefact PS1 TOMBSPSX.DAT Conversion Script #
######################################################################

# Versions of the game
# Commented out due to unfinished: "jp" "jp2" "as" "as2"
versions=("us" "uk" "de" "fr" "it" "sp")

# Makes a new out-directory
rm -rf ./out
mkdir ./out

for ver in "${versions[@]}"
do
	cd "./src/$ver"
	../../../tools/bin/gameflow tr3_gold.txt
	mv tombPSX.dat ../../out/$ver.dat
	cd ../..
done

