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
nosfx="scotland:;willsden:117,334,081,082,237,218,047,217,084,083,329,269,021,022,023,024,025,061;chunnel:021,022,023,024,025,031,047,052,078,095,166,208,209,210,223,334,335,341,342;undersea:021,022,023,024,025,068,072,117,144,218,281;underse2:021,022,023,024,025,068,072,117,144,218,281;zoo:043,083,084,165,220;slinc:"

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
# To make up for it, a warm glow is baked into the room vertex colours around them (costs nothing) and a
# warm room light per group lights the punks themselves.
./trmod tr3pc slinc.tr2 "removeitem(79,1)" "removeitem(71,1)" "removeitem(40,1)" "removeitem(38,1)" "removeitem(32,1)" "removeitem(29,1)" "removeitem(28,1)" "removeitem(20,1)" "removeitem(11,1)" "removeitem(4,1)"
py -3 "${prjdir}/../tools/bake_point_light.py" slinc.tr2 78336,-6912,59904 52736,3456,53760 52736,3840,49664 80384,-6912,69120 82432,-6912,67072 81408,-6912,68096 85504,-3840,57856 86528,-3584,57856 78336,-6144,46592 77312,-6144,57856 --radius 3072 --strength 8 --tint 1.0,0.7,0.25 || exit 1
./trmod tr3pc slinc.tr2 "addlight(2,1536,1536,1024,FFB050,spotlight,6143,2048)" "addlight(8,5632,1536,1920,FFB050,spotlight,6143,2048)" "addlight(18,5632,1536,1536,FFB050,spotlight,6143,2048)" "addlight(23,4608,4608,1024,FFB050,spotlight,6143,2048)" "addlight(33,3072,3584,1664,FFB050,spotlight,6143,2048)" "addlight(40,4608,3584,1024,FFB050,spotlight,6143,2048)" "addlight(58,4608,10752,2304,FFB050,spotlight,6143,2048)"
# Reunion rooms 20 and 21 (where enemies come in) are much darker than their neighbours 8, 18 and 19:
# a wide soft glow in their middle (fades out over 6 blocks, so it runs evenly into the neighbouring
# rooms without an edge; tint taken from the existing light) and a neutral room light each for the enemies.
py -3 "${prjdir}/../tools/bake_point_light.py" slinc.tr2 58368,1152,55296 58368,1152,49152 --radius 6144 --strength 5 --tint 0.65,1.0,0.7 || exit 1
./trmod tr3pc slinc.tr2 "addlight(20,6144,4096,3072,D8F0D8,spotlight,4095,4096)" "addlight(21,6144,4096,3200,D8F0D8,spotlight,4095,4096)"

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
	done
done

# Copies output back into the project directory
cd "${prjdir}"
cp -r /tmp/levels/out .

