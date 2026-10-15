#############################################
# WILLSDEN.TR2 Geometry Optimisation Script #
#############################################

# Fixes camera bug when inserting the "Cairn Key" by placing "lookitem(48)" as the last action.
./trmod tr3pc willsden.tr2 "replacefloordata(56,14,1:trigger(key(47),0,once,1F:soundtrack(12),flipmap(001),object(45),object(46),object(44),camera(56,13824,1536,6016,7,0,,0000),lookitem(48)))"
