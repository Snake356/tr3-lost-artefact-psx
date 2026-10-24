################################################################
# Tomb Raider : The Lost Artefact PS1 Binary Conversion Script #
################################################################

# Versions of the game
versions="us,ukrev,derev,frrev,itrev,sprev,jp,jp2,as,as2"

# Patches for the game
patches="bird,croc,lizard,secrets,adventur,savename,strsofar,sky,loadbar,reunionlava,noicons,diverhide,credits10,reunionview,roomguard,duckroll,newgameplus,pickup3d,crawlflip,crouchturn,ngpfile,ngpchoice,lotslots4"

# Makes the variables loopable
versions="$(echo "${versions}" | tr ',' '\n')"
patches="$(echo "${patches}" | tr ',' '\n')"

# Makes output directories
rm -rf ./mod
cp -r ./orig ./mod

# Applies the patches
for ver in ${versions}
do
	for pat in ${patches}
	do
		../tools/bin/applyips ./mod/$ver.exe ./patches/$ver/$pat.ips
	done
done

