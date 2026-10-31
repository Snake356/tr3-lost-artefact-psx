#!/bin/sh

###############################################################
# Tomb Raider : The Lost Artefact PS1 Level Conversion Script #
###############################################################

### VARIABLE DEFINITIONS ###

# Definition of levels and versions
levels="scotland,willsden,chunnel,undersea,underse2,zoo,slinc"
versions="us,ukrev,derev,frrev,itrev,sprev,jp,jp2,as,as2"

# Definition of extra conversion parameters per level
convparams="scotland:-wr -mo;willsden:-nm;chunnel:-nm;undersea:-wc==50A3F8 -nm;underse2:-wc==50A3F8 -nm;zoo:-nm;slinc:-nm"

# Definition of SFX-samples to be removed for each level
nosfx="scotland:;willsden:117,334,081,082,237,218,047,217,084,083,329,269,021,022,023,024,025,061;chunnel:021,022,023,024,025,031,047,052,095,166,223,274,275,276,277,278,279,280,331,334,335,341,342;undersea:021,022,023,024,025,068,072,117,144,218,281,308,309;underse2:021,022,023,024,025,068,072,117,144,218,281,308,309;zoo:043,083,084,165,220;slinc:"

# Length of the shell command buffer
shellbuffer=8000


### SETUP ###

# Replaces the comma's with newlines
versions="$(echo "${versions}" |
            tr ',' '\n')"
levels="$(echo "${levels}" |
          tr ',' '\n')"

# Copies work files into /tmp
prjdir="${PWD}"
rm -rf ./out
rm -rf /tmp/levels
cp -r ../levels /tmp/.
cp ../tools/bin/* /tmp/levels/.
cd /tmp/levels/.
mv ./in/* .

# Makes output directories
mkdir ./out
for ver in ${versions}; do
	mkdir "./out/${ver}"
done


### SFX ###

# Makes the sfx-files for each level and version
mkdir "./outsfx"
for version in ${versions}; do
	mkdir "./outsfx/${version}"
done
for level in ${levels}; do
	mkdir ./tmpsfx
	cp ./sfx/common/* ./tmpsfx/.
	remsfx="$(echo "${nosfx}"  |
	          tr   ';' '\n'    |
	          grep "${level}:" |
	          sed "s|.*:||g"   |
	          tr ',' '\n')"
	for sample in ${remsfx}; do
		rm "./tmpsfx/${sample}.wav"
		cp "./sfx/empty.wav" "./tmpsfx/${sample}.wav"
	done
	for version in ${versions}; do
		cp -r ./tmpsfx ./tmpsfx2
		cp -r ./sfx/${version}/* ./tmpsfx2/.
		./trsfx "./tmpsfx2" "./outsfx/${version}/${level}.sfx"
		rm -rf "./tmpsfx2"
	done
	rm -rf "./tmpsfx"
done


### LEVEL CUSTOMISATION ###

# Replaces the texture data in all the levels from the bitmap image
for level in ${levels}; do
	./3texrep "${level}.tr2" "./textures/${level}.bmp"
done

# Adds the object textures for controller and save crystal
./texmod scotland.tr2 a 12 255 31 255 255 255 31 1 240 1 0 255 255 0 0 0 0 0002
./texmod scotland.tr2 a 12 255 31 255 255 1 0 255 255 255 31 1 240 0 0 0 0 0002
./texmod scotland.tr2 a 12 1 32 1 240 255 47 1 240 255 47 255 255 1 32 255 255 0000
./texmod scotland.tr2 a 12 1 48 1 240 255 63 1 240 255 63 255 255 1 48 255 255 0000
./texmod scotland.tr2 a 12 1 64 1 240 255 79 1 240 255 79 255 255 1 64 255 255 0000
./texmod scotland.tr2 a 12 1 80 1 240 255 95 1 240 255 95 255 255 1 80 255 255 0000
./texmod scotland.tr2 a 12 1 96 1 240 255 98 1 240 255 98 255 242 0 0 0 0 0000
./texmod scotland.tr2 a 12 1 99 1 240 255 101 1 240 255 101 255 242 0 0 0 0 0000
./texmod scotland.tr2 a 12 1 102 1 240 255 104 1 240 255 104 255 242 0 0 0 0 0000
./texmod scotland.tr2 a 12 1 105 1 240 255 107 1 240 255 107 255 242 0 0 0 0 0000
./texmod scotland.tr2 a 12 1 108 1 240 255 110 1 240 255 110 255 242 0 0 0 0 0000
./texmod scotland.tr2 a 12 1 111 1 240 255 113 1 240 255 113 255 242 0 0 0 0 0000
./texmod scotland.tr2 a 12 1 114 1 240 255 116 1 240 255 116 255 242 0 0 0 0 0000
./texmod scotland.tr2 a 12 1 117 1 240 255 119 1 240 255 119 255 242 0 0 0 0 0000
./texmod willsden.tr2 a 14 255 31 255 255 255 31 1 240 1 0 255 255 0 0 0 0 0002
./texmod willsden.tr2 a 14 255 31 255 255 1 0 255 255 255 31 1 240 0 0 0 0 0002
./texmod willsden.tr2 a 14 1 32 1 240 255 47 1 240 255 47 255 255 1 32 255 255 0000
./texmod willsden.tr2 a 14 1 48 1 240 255 63 1 240 255 63 255 255 1 48 255 255 0000
./texmod willsden.tr2 a 14 1 64 1 240 255 79 1 240 255 79 255 255 1 64 255 255 0000
./texmod willsden.tr2 a 14 1 80 1 240 255 95 1 240 255 95 255 255 1 80 255 255 0000
./texmod willsden.tr2 a 14 1 96 1 240 255 98 1 240 255 98 255 242 0 0 0 0 0000
./texmod willsden.tr2 a 14 1 99 1 240 255 101 1 240 255 101 255 242 0 0 0 0 0000
./texmod willsden.tr2 a 14 1 102 1 240 255 104 1 240 255 104 255 242 0 0 0 0 0000
./texmod willsden.tr2 a 14 1 105 1 240 255 107 1 240 255 107 255 242 0 0 0 0 0000
./texmod willsden.tr2 a 14 1 108 1 240 255 110 1 240 255 110 255 242 0 0 0 0 0000
./texmod willsden.tr2 a 14 1 111 1 240 255 113 1 240 255 113 255 242 0 0 0 0 0000
./texmod willsden.tr2 a 14 1 114 1 240 255 116 1 240 255 116 255 242 0 0 0 0 0000
./texmod willsden.tr2 a 14 1 117 1 240 255 119 1 240 255 119 255 242 0 0 0 0 0000
./texmod chunnel.tr2 a 13 255 31 255 255 255 31 1 240 1 0 255 255 0 0 0 0 0002
./texmod chunnel.tr2 a 13 255 31 255 255 1 0 255 255 255 31 1 240 0 0 0 0 0002
./texmod chunnel.tr2 a 13 1 32 1 240 255 47 1 240 255 47 255 255 1 32 255 255 0000
./texmod chunnel.tr2 a 13 1 48 1 240 255 63 1 240 255 63 255 255 1 48 255 255 0000
./texmod chunnel.tr2 a 13 1 64 1 240 255 79 1 240 255 79 255 255 1 64 255 255 0000
./texmod chunnel.tr2 a 13 1 80 1 240 255 95 1 240 255 95 255 255 1 80 255 255 0000
./texmod chunnel.tr2 a 13 1 96 1 240 255 98 1 240 255 98 255 242 0 0 0 0 0000
./texmod chunnel.tr2 a 13 1 99 1 240 255 101 1 240 255 101 255 242 0 0 0 0 0000
./texmod chunnel.tr2 a 13 1 102 1 240 255 104 1 240 255 104 255 242 0 0 0 0 0000
./texmod chunnel.tr2 a 13 1 105 1 240 255 107 1 240 255 107 255 242 0 0 0 0 0000
./texmod chunnel.tr2 a 13 1 108 1 240 255 110 1 240 255 110 255 242 0 0 0 0 0000
./texmod chunnel.tr2 a 13 1 111 1 240 255 113 1 240 255 113 255 242 0 0 0 0 0000
./texmod chunnel.tr2 a 13 1 114 1 240 255 116 1 240 255 116 255 242 0 0 0 0 0000
./texmod chunnel.tr2 a 13 1 117 1 240 255 119 1 240 255 119 255 242 0 0 0 0 0000
./texmod chunnel.tr2 a 13 255 215 255 255 1 176 255 255 255 215 1 224 0 176 0 224 0000
./texmod chunnel.tr2 a 13 255 255 255 255 255 255 1 224 1 216 255 255 0 216 0 224 0000
./texmod undersea.tr2 a 12 255 31 255 255 255 31 1 240 1 0 255 255 0 0 0 0 0002
./texmod undersea.tr2 a 12 255 31 255 255 1 0 255 255 255 31 1 240 0 0 0 0 0002
./texmod undersea.tr2 a 12 1 32 1 240 255 47 1 240 255 47 255 255 1 32 255 255 0000
./texmod undersea.tr2 a 12 1 48 1 240 255 63 1 240 255 63 255 255 1 48 255 255 0000
./texmod undersea.tr2 a 12 1 64 1 240 255 79 1 240 255 79 255 255 1 64 255 255 0000
./texmod undersea.tr2 a 12 1 80 1 240 255 95 1 240 255 95 255 255 1 80 255 255 0000
./texmod undersea.tr2 a 12 1 96 1 240 255 98 1 240 255 98 255 242 0 0 0 0 0000
./texmod undersea.tr2 a 12 1 99 1 240 255 101 1 240 255 101 255 242 0 0 0 0 0000
./texmod undersea.tr2 a 12 1 102 1 240 255 104 1 240 255 104 255 242 0 0 0 0 0000
./texmod undersea.tr2 a 12 1 105 1 240 255 107 1 240 255 107 255 242 0 0 0 0 0000
./texmod undersea.tr2 a 12 1 108 1 240 255 110 1 240 255 110 255 242 0 0 0 0 0000
./texmod undersea.tr2 a 12 1 111 1 240 255 113 1 240 255 113 255 242 0 0 0 0 0000
./texmod undersea.tr2 a 12 1 114 1 240 255 116 1 240 255 116 255 242 0 0 0 0 0000
./texmod undersea.tr2 a 12 1 117 1 240 255 119 1 240 255 119 255 242 0 0 0 0 0000
./texmod undersea.tr2 a 12 255 215 255 255 1 176 255 255 255 215 1 224 0 176 0 224 0000
./texmod undersea.tr2 a 12 255 255 255 255 255 255 1 224 1 216 255 255 0 216 0 224 0000
./texmod underse2.tr2 a 12 255 31 255 255 255 31 1 240 1 0 255 255 0 0 0 0 0002
./texmod underse2.tr2 a 12 255 31 255 255 1 0 255 255 255 31 1 240 0 0 0 0 0002
./texmod underse2.tr2 a 12 1 32 1 240 255 47 1 240 255 47 255 255 1 32 255 255 0000
./texmod underse2.tr2 a 12 1 48 1 240 255 63 1 240 255 63 255 255 1 48 255 255 0000
./texmod underse2.tr2 a 12 1 64 1 240 255 79 1 240 255 79 255 255 1 64 255 255 0000
./texmod underse2.tr2 a 12 1 80 1 240 255 95 1 240 255 95 255 255 1 80 255 255 0000
./texmod underse2.tr2 a 12 1 96 1 240 255 98 1 240 255 98 255 242 0 0 0 0 0000
./texmod underse2.tr2 a 12 1 99 1 240 255 101 1 240 255 101 255 242 0 0 0 0 0000
./texmod underse2.tr2 a 12 1 102 1 240 255 104 1 240 255 104 255 242 0 0 0 0 0000
./texmod underse2.tr2 a 12 1 105 1 240 255 107 1 240 255 107 255 242 0 0 0 0 0000
./texmod underse2.tr2 a 12 1 108 1 240 255 110 1 240 255 110 255 242 0 0 0 0 0000
./texmod underse2.tr2 a 12 1 111 1 240 255 113 1 240 255 113 255 242 0 0 0 0 0000
./texmod underse2.tr2 a 12 1 114 1 240 255 116 1 240 255 116 255 242 0 0 0 0 0000
./texmod underse2.tr2 a 12 1 117 1 240 255 119 1 240 255 119 255 242 0 0 0 0 0000
./texmod underse2.tr2 a 12 255 215 255 255 1 176 255 255 255 215 1 224 0 176 0 224 0000
./texmod underse2.tr2 a 12 255 255 255 255 255 255 1 224 1 216 255 255 0 216 0 224 0000
./texmod zoo.tr2 a 14 255 31 255 255 255 31 1 240 1 0 255 255 0 0 0 0 0002
./texmod zoo.tr2 a 14 255 31 255 255 1 0 255 255 255 31 1 240 0 0 0 0 0002
./texmod zoo.tr2 a 14 1 32 1 240 255 47 1 240 255 47 255 255 1 32 255 255 0000
./texmod zoo.tr2 a 14 1 48 1 240 255 63 1 240 255 63 255 255 1 48 255 255 0000
./texmod zoo.tr2 a 14 1 64 1 240 255 79 1 240 255 79 255 255 1 64 255 255 0000
./texmod zoo.tr2 a 14 1 80 1 240 255 95 1 240 255 95 255 255 1 80 255 255 0000
./texmod zoo.tr2 a 14 1 96 1 240 255 98 1 240 255 98 255 242 0 0 0 0 0000
./texmod zoo.tr2 a 14 1 99 1 240 255 101 1 240 255 101 255 242 0 0 0 0 0000
./texmod zoo.tr2 a 14 1 102 1 240 255 104 1 240 255 104 255 242 0 0 0 0 0000
./texmod zoo.tr2 a 14 1 105 1 240 255 107 1 240 255 107 255 242 0 0 0 0 0000
./texmod zoo.tr2 a 14 1 108 1 240 255 110 1 240 255 110 255 242 0 0 0 0 0000
./texmod zoo.tr2 a 14 1 111 1 240 255 113 1 240 255 113 255 242 0 0 0 0 0000
./texmod zoo.tr2 a 14 1 114 1 240 255 116 1 240 255 116 255 242 0 0 0 0 0000
./texmod zoo.tr2 a 14 1 117 1 240 255 119 1 240 255 119 255 242 0 0 0 0 0000
./texmod zoo.tr2 a 14 255 215 255 255 1 176 255 255 255 215 1 224 0 176 0 224 0000
./texmod zoo.tr2 a 14 255 255 255 255 255 255 1 224 1 216 255 255 0 216 0 224 0000
./texmod slinc.tr2 a 11 255 31 255 255 255 31 1 240 1 0 255 255 0 0 0 0 0002
./texmod slinc.tr2 a 11 255 31 255 255 1 0 255 255 255 31 1 240 0 0 0 0 0002
./texmod slinc.tr2 a 11 1 32 1 240 255 47 1 240 255 47 255 255 1 32 255 255 0000
./texmod slinc.tr2 a 11 1 48 1 240 255 63 1 240 255 63 255 255 1 48 255 255 0000
./texmod slinc.tr2 a 11 1 64 1 240 255 79 1 240 255 79 255 255 1 64 255 255 0000
./texmod slinc.tr2 a 11 1 80 1 240 255 95 1 240 255 95 255 255 1 80 255 255 0000
./texmod slinc.tr2 a 11 1 96 1 240 255 98 1 240 255 98 255 242 0 0 0 0 0000
./texmod slinc.tr2 a 11 1 99 1 240 255 101 1 240 255 101 255 242 0 0 0 0 0000
./texmod slinc.tr2 a 11 1 102 1 240 255 104 1 240 255 104 255 242 0 0 0 0 0000
./texmod slinc.tr2 a 11 1 105 1 240 255 107 1 240 255 107 255 242 0 0 0 0 0000
./texmod slinc.tr2 a 11 1 108 1 240 255 110 1 240 255 110 255 242 0 0 0 0 0000
./texmod slinc.tr2 a 11 1 111 1 240 255 113 1 240 255 113 255 242 0 0 0 0 0000
./texmod slinc.tr2 a 11 1 114 1 240 255 116 1 240 255 116 255 242 0 0 0 0 0000
./texmod slinc.tr2 a 11 1 117 1 240 255 119 1 240 255 119 255 242 0 0 0 0 0000
./texmod slinc.tr2 a 11 255 215 255 255 1 176 255 255 255 215 1 224 0 176 0 224 0000
./texmod slinc.tr2 a 11 255 255 255 255 255 255 1 224 1 216 255 255 0 216 0 224 0000

# Replaces the animations to allow save crystals and controller mesh
for level in ${levels}; do
	./troi "${level}.tr2" i "./anims/${level}.dat"
done

# Adds the forward crawl exit (Lara flips out of a crawlspace; started by crawlflip.ips) as animation 313
for level in ${levels}; do
	py -3 "${prjdir}/../tools/add_crawl_flip.py" "${level}.tr2" || exit 1
done

# Adds the turns on the spot in the crouch (started by crouchturn.ips) as animations 314 and 315
for level in ${levels}; do
	py -3 "${prjdir}/../tools/add_crouch_turn.py" "${level}.tr2" || exit 1
done

# Smooths the pickup in a crawlspace (animation 292) with 14 poses instead of 6
for level in ${levels}; do
	py -3 "${prjdir}/../tools/add_crawl_pickup.py" "${level}.tr2" || exit 1
done

# Lara's right arm at the end of the crouch roll (animation 220) leads into the crouch (TRX's poses)
for level in ${levels}; do
	py -3 "${prjdir}/../tools/fix_crouch_roll_arm.py" "${level}.tr2" || exit 1
done

# Adds TRX's controlled drop (Lara turns round and catches the edge; started by controlleddrop.ips) as animations 316 and 317
for level in ${levels}; do
	py -3 "${prjdir}/../tools/add_controlled_drop.py" "${level}.tr2" || exit 1
done

# Adds TRX's ledge jumps (from hanging, jump up or push off backwards; started by ledgejump.ips) as animations 318 to 321
for level in ${levels}; do
	py -3 "${prjdir}/../tools/add_ledge_jumps.py" "${level}.tr2" || exit 1
done

# Adds TRX's corner shimmying (hanging, round an outer or inner corner; started by cornershimmy.ips) as animations 322 to 329
for level in ${levels}; do
	py -3 "${prjdir}/../tools/add_corner_shimmy.py" "${level}.tr2" || exit 1
done

# Adds TRX's climbing round corners (on a climbable wall, round an outer or inner corner; started by cornershimmy.ips) as animations 330 to 337
for level in ${levels}; do
	py -3 "${prjdir}/../tools/add_climb_corners.py" "${level}.tr2" || exit 1
done

# Frees the poses of Lara's animations that only objects missing from the level start (switches, key holes, zip line...)
for level in ${levels}; do
	py -3 "${prjdir}/../tools/strip_dead_anims.py" "${level}.tr2" || exit 1
done

# Frees level memory: poses and mesh trees in common are shared, unused poses and tree words removed
for level in ${levels}; do
	py -3 "${prjdir}/../tools/compact_frames.py" "${level}.tr2" || exit 1
done

# Frees level memory: rotations about one axis in one word, each animation with its own frame size (as Core's levels)
for level in ${levels}; do
	py -3 "${prjdir}/../tools/pack_rotations.py" "${level}.tr2" || exit 1
done

# Adds savegame crystals:
./trmod tr3pc scotland.tr2 "replaceitem(48,180,79,8704,4608,512,0,-1,-1,3E00)" "replaceitem(117,180,6,4608,4608,256,0,-1,-1,3E00)" "replacefloordata(84,3,1:ceilingslant(2,0),trigger(trigger,0,,1F:object(118)))" "replacefloordata(84,4,1:trigger(trigger,0,,1F:object(118)))" "replacefloordata(84,5,1:ceilingslant(-2,0),trigger(trigger,0,,1F:object(118)))" "replacefloordata(36,5,1:ceilingportal(6),trifloor(\\,,1,0,1,2,0,0),trigger(trigger,0,,1F:object(21)))" "replacefloordata(36,5,2:ceilingportal(6),trigger(trigger,0,,1F:object(21)))" "replacefloordata(36,5,3:ceilingportal(6),trifloor(/,,4,0,0,0,5,1),trigger(trigger,0,,1F:object(21)))" "replacefloordata(65,10,1:monkeyswing,trigger(pickup(78),0,,1F:lookitem(77),camera(65,1536,2176,3424,4,0,once,0000)))" "additem(180,48,5632,8704,512,0,-1,-1,3E00)" "additem(180,52,3584,2560,0,0,-1,-1,3E00)" "additem(180,19,6656,7680,0,0,-1,-1,3E00)" "additem(180,126,1536,1536,256,0,-1,-1,3E00)" "additem(180,87,7680,5632,256,0,-1,-1,3E00)" "additem(180,45,1536,9728,2560,0,-1,-1,3E00)" "additem(180,112,2560,4608,0,0,-1,-1,3E00)" "additem(180,77,12800,1536,256,0,-1,-1,3E00)"
./trmod tr3pc willsden.tr2 "additem(180,93,5632,2560,768,0,-1,-1,3E00)" "additem(180,41,1536,2560,256,0,-1,-1,3E00)" "additem(180,101,3584,1536,0,0,-1,-1,3E00)" "additem(180,76,2560,7680,0,0,-1,-1,3E00)" "additem(180,71,4608,1536,0,0,-1,-1,3E00)" "additem(180,107,5632,2560,0,0,-1,-1,3E00)" "additem(180,64,2560,2560,0,0,-1,-1,3E00)" "additem(180,78,5632,4608,1024,0,-1,-1,3E00)"
./trmod tr3pc chunnel.tr2 "additem(180,6,2560,5632,256,0,-1,-1,3E00)" "additem(180,91,1536,5632,1280,0,-1,-1,3E00)" "additem(180,0,2560,6656,0,0,-1,-1,3E00)" "additem(180,21,7680,3584,2688,0,-1,-1,3E00)" "additem(180,75,5632,6656,1024,0,-1,-1,3E00)" "additem(180,8,5632,4608,1152,0,-1,-1,3E00)" "additem(180,36,3584,1536,0,0,-1,-1,3E00)" "additem(180,79,7680,2560,256,0,-1,-1,3E00)" "additem(180,92,2560,2560,0,0,-1,-1,3E00)" "additem(180,112,7680,4608,1152,0,-1,-1,3E00)"
./trmod tr3pc undersea.tr2 "additem(180,32,1536,1536,256,0,-1,-1,3E00)" "additem(180,14,3584,3584,5376,0,-1,-1,3E00)" "additem(180,63,1536,2560,1280,0,-1,-1,3E00)" "additem(180,35,2560,5632,768,0,-1,-1,3E00)" "additem(180,6,1536,13824,768,0,-1,-1,3E00)"
./trmod tr3pc underse2.tr2 "additem(180,8,11264,8192,3840,0,-1,-1,3E00)" "additem(180,99,4608,4608,256,0,-1,-1,3E00)" "additem(180,12,4608,6656,1024,0,-1,-1,3E00)" "additem(180,65,1536,2560,384,0,-1,-1,3E00)" "additem(180,20,3584,2560,768,0,-1,-1,3E00)" "additem(180,105,1536,3584,0,0,-1,-1,3E00)" "additem(180,53,8704,8704,3584,0,-1,-1,3E00)" "additem(180,84,4608,2560,256,0,-1,-1,3E00)"
./trmod tr3pc zoo.tr2 "additem(180,85,2560,1536,768,0,-1,-1,3E00)" "additem(180,54,5632,7680,256,0,-1,-1,3E00)" "additem(180,0,10752,7680,768,0,-1,-1,3E00)" "additem(180,7,1536,3584,512,0,-1,-1,3E00)" "additem(180,133,2560,7680,512,0,-1,-1,3E00)" "additem(180,63,4608,3584,1280,0,-1,-1,3E00)" "additem(180,111,1536,5632,256,0,-1,-1,3E00)" "additem(180,99,1536,8704,0,0,-1,-1,3E00)" "additem(180,96,6656,4608,1536,0,-1,-1,3E00)" "additem(180,116,4608,6656,256,0,-1,-1,3E00)" "additem(180,128,2560,1536,768,0,-1,-1,3E00)"
./trmod tr3pc slinc.tr2 "additem(180,60,6656,1536,1280,0,-1,-1,3E00)" "additem(180,38,5632,13824,256,0,-1,-1,3E00)" "additem(180,38,5632,1536,768,0,-1,-1,3E00)" "additem(180,66,2560,3584,256,0,-1,-1,3E00)" "additem(180,7,1536,4608,256,0,-1,-1,3E00)" "additem(180,40,1536,2560,0,0,-1,-1,3E00)"
# Reunion: the punks lose their burning clubs (the flame emitters under all ten are removed; ten burning
# clubs meant up to ten dynamic lights and hundreds of flame sprites, which the PS1 cannot keep up with).
# To make up for it, a glow is baked into the room vertex colours around them (costs nothing) and a room
# light per group lights the punks themselves, both in the colour of the light nearest to the punks
# (FFFFBF rooms 2/23, 103F17 rooms 8/18, FF3F00 room 33, 0098CC room 40, 0098DC room 58). The PS1 shows
# vertex colours twice as bright as TRView (16 of 31 is full texture brightness). Room 2's punk by the hut
# gets no glow (the hut wall stays as in the released level).
./trmod tr3pc slinc.tr2 "removeitem(79,1)" "removeitem(71,1)" "removeitem(40,1)" "removeitem(38,1)" "removeitem(32,1)" "removeitem(29,1)" "removeitem(28,1)" "removeitem(20,1)" "removeitem(11,1)" "removeitem(4,1)"
py -3 "${prjdir}/../tools/bake_point_light.py" slinc.tr2 80384,-6912,69120 82432,-6912,67072 81408,-6912,68096 --radius 3072 --strength 12 --tint 1.0,1.0,0.75 || exit 1
py -3 "${prjdir}/../tools/bake_point_light.py" slinc.tr2 52736,3456,53760 52736,3840,49664 --radius 3072 --strength 12 --tint 0.25,1.0,0.37 || exit 1
py -3 "${prjdir}/../tools/bake_point_light.py" slinc.tr2 85504,-3840,57856 86528,-3584,57856 --radius 3072 --strength 12 --tint 1.0,0.25,0.0 || exit 1
py -3 "${prjdir}/../tools/bake_point_light.py" slinc.tr2 78336,-6144,46592 --radius 3072 --strength 12 --tint 0.0,0.75,1.0 || exit 1
py -3 "${prjdir}/../tools/bake_point_light.py" slinc.tr2 77312,-6144,57856 --radius 3072 --strength 12 --tint 0.0,0.69,1.0 || exit 1
# Dark spots: room 7's side walls (not its floor and platforms, which only Lara's flare is to show) and
# room 2's east wall, each in the colour of the room's light; room 2 gets a light there too.
py -3 "${prjdir}/../tools/bake_point_light.py" slinc.tr2 65536,-768,49536 65536,-768,54912 --radius 3072 --strength 10 --tint 0.25,1.0,0.5 --above 768 || exit 1
py -3 "${prjdir}/../tools/bake_point_light.py" slinc.tr2 83456,-7000,63300 --radius 4096 --strength 11 --tint 1.0,1.0,0.75 || exit 1
./trmod tr3pc slinc.tr2 "addlight(2,1536,1536,1024,FFFFBF,spotlight,6143,2048)" "addlight(8,5632,1536,1920,40FF5D,spotlight,8191,2048)" "addlight(18,5632,1536,1536,40FF5D,spotlight,8191,2048)" "addlight(23,4608,4608,1024,FFFFBF,spotlight,8191,2048)" "addlight(33,3072,3584,1664,FF3F00,spotlight,8191,2048)" "addlight(40,4608,3584,1024,00BFFF,spotlight,8191,2048)" "addlight(58,4608,10752,2304,00B2FF,spotlight,8191,2048)"
./trmod tr3pc slinc.tr2 "addlight(2,6656,4932,600,FFFFBF,spotlight,6143,3072)"
# Reunion rooms 20 and 21 (where enemies come in) are much darker than their neighbours 8, 18 and 19:
# a wide soft glow in their middle (fades out over 6 blocks, so it runs evenly into the neighbouring
# rooms without an edge; tint taken from the existing light) and a neutral room light each for the enemies.
py -3 "${prjdir}/../tools/bake_point_light.py" slinc.tr2 58368,1152,55296 58368,1152,49152 --radius 6144 --strength 5 --tint 0.65,1.0,0.7 || exit 1
./trmod tr3pc slinc.tr2 "addlight(20,6144,4096,3072,D8F0D8,spotlight,4095,4096)" "addlight(21,6144,4096,3200,D8F0D8,spotlight,4095,4096)"
# Room 19's orange-red light (FF3F20) and the patch it left on its wall (from the released level) in the
# room's green (light 1 is 3FFF7F; the patch keeps its brightness), room 21's vertex 23 shares a corner
# with it. Room 2: the hut wall's two light lower corners by the punk a little darker. Room 7: the middle
# under its light (path, platform edges, block walls) at half the original brightness, and the block
# walls' corners on the border to rooms 20/21 as dark as the neighbour's wall (no edge at the border).
./trmod tr3pc slinc.tr2 "replacelight(19,0,5632,1536,640,3FFF7F,spotlight,8191,872)" "replacevertex(19,29,,,,9799,9799,)" "replacevertex(19,33,,,,10888,10888,)" "replacevertex(19,36,,,,10888,10888,)" "replacevertex(19,37,,,,9799,9799,)" "replacevertex(19,38,,,,3266,3266,)" "replacevertex(19,39,,,,9799,9799,)" "replacevertex(19,40,,,,8711,8711,)" "replacevertex(19,42,,,,4388,4388,)" "replacevertex(19,43,,,,7622,7622,)" "replacevertex(21,23,,,,9799,9799,)" "replacevertex(2,12,,,,3204,3204,)" "replacevertex(2,15,,,,3204,3204,)" "replacevertex(7,24,,,,2209,2209,)" "replacevertex(7,25,,,,2209,2209,)" "replacevertex(7,20,,,,1056,1056,)" "replacevertex(7,29,,,,1056,1056,)" "replacevertex(20,206,,,,0,0,)" "replacevertex(21,216,,,,2113,2113,)" "replacevertex(7,4,,,,2113,2113,)" "replacevertex(7,21,,,,1088,1088,)" "replacevertex(7,28,,,,1088,1088,)" "replacevertex(7,22,,,,2177,2177,)" "replacevertex(7,27,,,,2177,2177,)" "replacevertex(7,23,,,,1120,1120,)" "replacevertex(7,26,,,,1120,1120,)" "replacevertex(7,44,,,,1088,1088,)" "replacevertex(7,7,,,,0,0,)" "replacevertex(7,9,,,,0,0,)"

# Optimises the world geometry
for level in ${levels}; do
	sh "./geom/${level}.sh"
done

# Optimises the mesh data
for level in ${levels}; do
	./tr3mesh "${level}.tr2" "./mesh/${level}.txt"
done

# Optimises the FloorData
for level in ${levels}; do
	fd="$(./trmod "tr3pc" "${level}.tr2" "cgetfloordata(*,*,*)" |
	      sed "s|^.*trmod tr3pc ${level}.tr2 ||g"               |
	      awk '{ print length, $0 }'                            |
	      sort -n -s -r                                         |
	      cut -d" " -f2-                                        |
	      tr '\n' ' '                                           |
	      fold -w "${shellbuffer}" -s                           |
	      sed "s|^|./trmod tr3pc ${level}.tr2 |g")"
	./trmod "tr3pc" "${level}.tr2" "removefloordata(*,*,*)"
	(
		IFS="$(printf '\n\v')"
		for fdline in ${fd}
		do
			(
				IFS=" "
				${fdline}
			)
		done
	)
done



./trmod tr3pc scotland.tr2 "addstaticmesh(30,31,6656,5632,3072,16384,848484,848484)" "addstaticmesh(20,53,9728,4608,1280,-24576,848484,848484)" "addstaticmesh(25,53,4608,4608,256,8192,848484,848484)" "addstaticmesh(25,52,9728,512,512,-16384,848484,848484)" "addstaticmesh(24,52,3584,512,512,-24576,848484,848484)" "addstaticmesh(24,64,8704,1536,-512,24576,848484,848484)" "addstaticmesh(20,59,12800,5632,-256,-8192,848484,848484)" "addstaticmesh(16,108,1536,4608,-1920,-8192,848484,848484)" "addstaticmesh(31,115,2560,4608,0,-32768,424242,848484)" "addstaticmesh(32,115,2560,4608,0,-32768,424242,848484)" "addstaticmesh(12,137,-512,2560,256,16384,848484,848484)" "addstaticmesh(12,139,2560,7680,0,16384,848484,848484)" "addstaticmesh(5,143,2560,512,256,-16384,848484,848484)"
./trmod tr3pc willsden.tr2 "addstaticmesh(10,26,512,3584,0,-32768,848484,848484)" "addstaticmesh(10,26,512,2560,0,-32768,848484,848484)" "addstaticmesh(31,95,4608,12800,0,-32768,424242,848484)" "addstaticmesh(32,95,4608,12800,0,-32768,424242,848484)" "addstaticmesh(10,26,512,5632,0,-32768,848484,848484)" "addstaticmesh(10,26,512,6656,0,-32768,848484,848484)" "addstaticmesh(10,26,512,7680,0,-32768,848484,848484)" "addstaticmesh(5,55,9728,1536,-256,0,848484,848484)" "addstaticmesh(20,26,3584,5632,-1664,-24576,848484,848484)" "addstaticmesh(20,25,11776,5632,512,8192,848484,848484)" "addstaticmesh(24,37,512,2560,2816,16384,848484,848484)" "addstaticmesh(20,35,8704,4608,512,24576,848484,848484)" "addstaticmesh(40,108,5632,3584,512,-16384,848484,848484)" "addstaticmesh(20,50,9728,4608,1280,-24576,848484,848484)" "addstaticmesh(25,37,3584,5632,1792,16384,5A5A84,848484)" "addstaticmesh(24,56,4608,5632,-256,24576,848484,848484)" "addstaticmesh(20,56,12800,5632,-256,-8192,848484,848484)" "addstaticmesh(16,80,1536,4608,-1920,-8192,848484,848484)" "addstaticmesh(6,83,6656,1536,2560,-32768,848484,848484)" "addstaticmesh(17,96,7680,3584,0,0,848484,848484)" "addstaticmesh(17,96,7680,2560,0,16384,848484,848484)" "addstaticmesh(21,1,3584,-512,0,-16384,848484,848484)" "addstaticmesh(17,95,512,2560,256,0,848484,848484)" "addstaticmesh(17,95,512,1536,256,16384,848484,848484)"
./trmod tr3pc chunnel.tr2 "addstaticmesh(17,3,512,6656,1024,0,848484,848484)" "addstaticmesh(17,3,512,1536,1024,0,848484,848484)" "addstaticmesh(32,6,512,4608,1536,0,636363,848484)" "addstaticmesh(33,6,512,4608,1536,0,4A4A4A,848484)" "addstaticmesh(32,4,6656,512,640,-16384,636363,848484)" "addstaticmesh(33,4,6656,512,640,-16384,4A4A4A,848484)" "addstaticmesh(45,3,3584,7680,1536,16384,848484,848484)" "addstaticmesh(46,3,2560,7680,1536,16384,848484,848484)" "addstaticmesh(32,3,1536,7680,1024,16384,636363,848484)" "addstaticmesh(33,3,1536,7680,1024,16384,4A4A4A,848484)" "addstaticmesh(32,3,5632,512,1536,-16384,848484,848484)" "addstaticmesh(33,3,5632,512,1536,-16384,848484,848484)" "addstaticmesh(12,83,512,512,0,16384,848484,848484)" "addstaticmesh(13,83,512,512,0,16384,848484,848484)" "addstaticmesh(17,97,5632,3584,0,0,848484,848484)" "addstaticmesh(39,99,1536,512,0,16384,848484,848484)" "addstaticmesh(22,23,-512,8704,256,16384,848484,848484)" "addstaticmesh(22,23,512,8704,256,16384,848484,848484)" "addstaticmesh(22,22,9728,8704,2560,16384,848484,848484)" "addstaticmesh(25,22,10752,3584,2304,0,63635A,848484)" "addstaticmesh(26,22,10752,3584,2304,0,636363,848484)" "addstaticmesh(31,1,4608,7680,6144,0,848484,848484)" "addstaticmesh(12,6,3584,13824,768,-32768,848484,848484)" "addstaticmesh(13,6,3584,13824,768,-32768,848484,848484)" "addstaticmesh(13,6,4608,13824,768,-32768,848484,848484)" "addstaticmesh(12,6,4608,13824,768,-32768,848484,848484)" "addstaticmesh(44,98,2560,1536,3072,-16384,5A5A5A,848484)" "addstaticmesh(44,98,2560,2560,3072,16384,5A5A5A,848484)"
./trmod tr3pc undersea.tr2 "addstaticmesh(16,6,4608,5632,-1024,-32768,848484,848484)" "addstaticmesh(37,4,512,2560,0,16384,6B6B73,848484)" "addstaticmesh(38,4,512,2560,0,16384,6B6B73,848484)"
./trmod tr3pc underse2.tr2 "addstaticmesh(33,11,10752,2560,2560,0,FFFFFF,848484)" "addstaticmesh(33,13,8704,512,4096,0,FFFFFF,848484)" "addstaticmesh(32,13,8704,512,4096,0,848484,848484)" "addstaticmesh(41,21,7680,14848,768,16384,848484,848484)" "addstaticmesh(41,21,8704,14848,768,16384,848484,848484)" "addstaticmesh(35,96,5632,512,768,0,848484,848484)" "addstaticmesh(35,96,5632,512,-256,-32768,848484,848484)" "addstaticmesh(11,110,15872,11776,3072,0,FF6B00,848484)"
./trmod tr3pc zoo.tr2 "addstaticmesh(30,72,7680,512,2816,-32768,848484,848484)" "addstaticmesh(11,4,5632,2560,-2048,0,848484,848484)" "addstaticmesh(12,4,5632,2560,-2048,0,848484,848484)" "addstaticmesh(45,18,2560,512,-1024,-16384,848484,848484)" "addstaticmesh(45,18,2560,512,-1024,-32768,848484,848484)" "addstaticmesh(45,18,2560,512,-1024,16384,848484,848484)" "addstaticmesh(11,3,512,2560,2304,0,DEDEDE,848484)" "addstaticmesh(11,6,5632,2560,-1024,0,DEDEDE,848484)" "addstaticmesh(12,6,5632,2560,-1024,0,848484,848484)" "addstaticmesh(34,6,3584,4608,-256,16384,ADADD6,848484)" "addstaticmesh(16,5,3584,7680,256,-16384,FFFFFF,848484)" "addstaticmesh(2,7,1536,2560,2304,-32768,848484,848484)" "addstaticmesh(45,10,1536,512,0,0,848484,848484)" "addstaticmesh(45,10,2560,512,0,0,848484,848484)" "addstaticmesh(45,10,3584,512,0,0,848484,848484)" "addstaticmesh(45,10,4608,512,0,16384,848484,848484)" "addstaticmesh(45,10,4608,512,0,0,848484,848484)" "addstaticmesh(7,17,8704,7680,256,8192,5A6B8C,848484)" "addstaticmesh(7,16,7680,512,0,0,52638C,848484)" "addstaticmesh(7,40,1536,7680,2048,8192,5A6B8C,848484)" "addstaticmesh(7,42,-2560,11776,256,0,52638C,848484)" "addstaticmesh(1,18,8704,3584,-2560,0,848484,848484)" "addstaticmesh(1,18,3584,3584,-2560,-32768,848484,848484)" "addstaticmesh(1,20,512,3584,1024,0,848484,848484)" "addstaticmesh(10,21,3584,3584,256,0,636363,848484)" "addstaticmesh(15,27,3584,6656,128,-24576,848484,848484)" "addstaticmesh(10,32,1536,2560,0,0,636363,848484)" "addstaticmesh(10,37,5632,512,0,16384,636363,848484)" "addstaticmesh(16,37,4608,512,384,-32768,FFFFFF,848484)" "addstaticmesh(34,34,5632,4608,-2304,-16384,848484,848484)" "addstaticmesh(34,49,2560,1536,1536,-16384,848484,848484)" "addstaticmesh(1,31,8704,6656,256,0,848484,848484)" "addstaticmesh(2,31,4608,5632,1024,0,848484,848484)" "addstaticmesh(1,38,8704,2560,-2304,0,848484,848484)" "addstaticmesh(0,65,2560,4608,-2304,16384,848484,848484)" "addstaticmesh(0,82,5632,3584,1536,16384,848484,848484)" "addstaticmesh(0,55,-512,2560,256,-16384,848484,848484)" "addstaticmesh(0,69,3584,7680,-1280,-16384,848484,848484)" "addstaticmesh(0,79,5632,2560,768,-16384,848484,848484)" "addstaticmesh(45,55,4608,1536,512,-32768,848484,848484)" "addstaticmesh(0,54,6656,7680,-768,16384,848484,848484)" "addstaticmesh(0,54,4608,6656,-1280,-32768,848484,848484)" "addstaticmesh(0,54,5632,13824,-768,-16384,848484,848484)" "addstaticmesh(30,60,512,3584,0,16384,848484,848484)" "addstaticmesh(30,60,-512,3584,256,16384,848484,848484)" "addstaticmesh(30,60,1536,3584,-256,16384,848484,848484)" "addstaticmesh(30,60,2560,3584,-512,16384,848484,848484)" "addstaticmesh(30,60,3584,3584,-768,16384,848484,848484)" "addstaticmesh(30,59,5632,3584,3840,16384,848484,848484)" "addstaticmesh(30,128,-512,5632,4096,16384,848484,848484)" "addstaticmesh(30,128,512,5632,3840,16384,848484,848484)" "addstaticmesh(7,56,12800,7680,256,0,848484,848484)" "addstaticmesh(7,58,3584,3584,-1792,0,848484,848484)" "addstaticmesh(30,0,7680,16896,3072,-32768,848484,848484)" "addstaticmesh(30,0,7680,17920,3328,-32768,848484,848484)" "addstaticmesh(30,76,2560,512,1280,-32768,848484,848484)" "addstaticmesh(30,72,7680,3584,3584,-32768,848484,848484)" "addstaticmesh(30,72,7680,4608,3840,-32768,848484,848484)" "addstaticmesh(30,77,1536,512,1280,-32768,848484,848484)" "addstaticmesh(30,76,2560,4608,2304,-32768,848484,848484)"
./trmod tr3pc zoo.tr2 "addstaticmesh(30,76,2560,5632,2560,-32768,848484,848484)" "addstaticmesh(30,112,2560,512,1536,-32768,848484,848484)" "addstaticmesh(2,87,1536,-512,256,16384,848484,848484)" "addstaticmesh(3,92,1536,10752,0,-16384,848484,848484)" "addstaticmesh(36,121,8704,-512,0,-8192,848484,848484)" "addstaticmesh(35,122,5632,13824,0,-8192,848484,848484)" "addstaticmesh(33,122,5632,6656,0,0,848484,848484)" "addstaticmesh(2,95,-512,1536,1280,24576,848484,848484)" "addstaticmesh(30,77,1536,5632,2560,-32768,848484,848484)" "addstaticmesh(30,77,1536,4608,2304,-32768,848484,848484)" "addstaticmesh(36,91,512,9728,512,-32768,848484,848484)" "addstaticmesh(30,58,6656,3584,1024,16384,848484,848484)" "addstaticmesh(40,134,3584,2560,3072,-16384,848484,848484)"
./trmod tr3pc slinc.tr2 "addstaticmesh(31,6,3584,1536,3584,16384,216331,848484)" "addstaticmesh(15,30,512,3584,512,-8192,848484,848484)" "addstaticmesh(7,17,8704,7680,256,8192,5A6B8C,848484)" "addstaticmesh(7,16,7680,512,0,0,52638C,848484)" "addstaticmesh(7,41,1536,7680,2048,8192,5A6B8C,848484)" "addstaticmesh(7,43,-2560,11776,256,0,52638C,848484)" "addstaticmesh(32,2,1536,7680,0,-32768,848484,848484)" "addstaticmesh(32,1,4608,2560,0,-32768,848484,848484)" "addstaticmesh(32,60,512,2560,0,-16384,5A3939,848484)"
./trmod tr3pc zoo.tr2 "stepsound(66,1,5,10)" "stepsound(66,1,6,10)" "stepsound(66,1,7,10)" "stepsound(66,1,8,10)" "stepsound(66,1,9,10)" "stepsound(69,1,3,10)" "stepsound(69,1,4,10)" "stepsound(69,1,5,10)" "stepsound(69,1,6,10)" "stepsound(69,1,8,10)" "stepsound(69,4,5,10)" "stepsound(69,4,6,10)" "stepsound(69,4,7,10)" "stepsound(69,4,8,10)" "stepsound(69,4,9,10)" "stepsound(133,4,4,10)"
py -3 "${prjdir}/../tools/nebel_raeume.py" scotland.tr2 || exit 1
py -3 "${prjdir}/../tools/nebel_raeume.py" willsden.tr2 || exit 1
py -3 "${prjdir}/../tools/nebel_raeume.py" chunnel.tr2 || exit 1
py -3 "${prjdir}/../tools/nebel_raeume.py" undersea.tr2 || exit 1
py -3 "${prjdir}/../tools/nebel_raeume.py" underse2.tr2 || exit 1
py -3 "${prjdir}/../tools/nebel_raeume.py" zoo.tr2 || exit 1
py -3 "${prjdir}/../tools/nebel_raeume.py" slinc.tr2 || exit 1
py -3 "${prjdir}/../tools/kaefig_fix.py" underse2.tr2 || exit 1
./trmod tr3pc underse2.tr2 "addlight(83,2048,4608,256,98CCCC,spotlight,4607,2600)"
py -3 "${prjdir}/../tools/passport_spine.py" scotland.tr2 || exit 1
py -3 "${prjdir}/../tools/passport_spine.py" willsden.tr2 || exit 1
py -3 "${prjdir}/../tools/passport_spine.py" chunnel.tr2 || exit 1
py -3 "${prjdir}/../tools/passport_spine.py" undersea.tr2 || exit 1
py -3 "${prjdir}/../tools/passport_spine.py" underse2.tr2 || exit 1
py -3 "${prjdir}/../tools/passport_spine.py" zoo.tr2 || exit 1
py -3 "${prjdir}/../tools/passport_spine.py" slinc.tr2 || exit 1
### JAPAN / ASIA DISC 1 ###

# The Japanese and Asian Disc 1 (jp, as) get easier levels like the original Japanese TR3 Disc 1:
# more pickups and save crystals, fewer traps. Each level is copied to <level>_jp.tr2 and changed with
# the trmod commands in ./japan/<level>.txt (made by tools/make_japan_commands.py from the finished
# Japan levels). The mesh optimisation above empties models a level does not use (they then show
# Lara's hips/leg). The Japan version has the grenade launcher already in Highland Fling (and carries it
# on) and the Desert Eagle in Willard's Lair. The grenade launcher pickup gets the model of its inventory
# object, which the level loads anyway (tools/share_model.py): putting its own mesh back made Highland Fling,
# which has the least memory left (rain; the Japanese discs keep the Kanji font in RAM), freeze on load.
for level in ${levels}; do
	cp "${level}.tr2" "${level}_jp.tr2"
done
if [ -f scotland_jp.tr2 ]; then
	py -3 "${prjdir}/../tools/share_model.py" scotland_jp.tr2 scotland_jp.tr2 167:192 || exit 1
fi
if [ -f chunnel_jp.tr2 ]; then
	py -3 "${prjdir}/../tools/share_model.py" chunnel_jp.tr2 chunnel_jp.tr2 167:192 || exit 1
	# the Desert Eagle's inventory model stands upright, so it gets its own mesh back (restore_model.py);
	# Shakespeare Cliff has the memory for it
	py -3 "${prjdir}/../tools/restore_model.py" "${prjdir}/in/chunnel.tr2" chunnel_jp.tr2 chunnel_jp.tr2 162 || exit 1
fi
for level in ${levels}; do
	if [ -f "./japan/${level}.txt" ]; then
		grep -v '^#' "./japan/${level}.txt" | tr -d '\r' | tr '\n' '\0' |
			xargs -0 -n 60 ./trmod tr3pc "${level}_jp.tr2" > /dev/null || exit 1
	fi
done


### LEVEL CONVERSION ###

# Converts the levels
for level in ${levels}; do
	var="$(echo "${convparams}" |
	       tr   ';' '\n'        |
	       grep "${level}"      |
	       sed "s|.*:||g")"
	for version in ${versions}; do
		# Performs the actual level conversion
		src="${level}.tr2"
		case "${version}" in
			jp|as) src="${level}_jp.tr2" ;;
		esac
		./tr3pcpsx "${src}" "./out/${version}/${level}.psx" "./outsfx/${version}/${level}.sfx" "-v" $var "-${version}"
		py -3 "${prjdir}/../tools/leer_ende.py" "./out/${version}/${level}.psx" || exit 1
	done
done

# Copies output back into the project directory
cd "${prjdir}"
cp -r /tmp/levels/out .

