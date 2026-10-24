#############################################
# UNDERSE2.TR2 Geometry Optimisation Script #
#############################################

# BE CAREFUL WHEN REMOVING ITEMS! SOME ITEMS MAY REMOVE OTHERS WHEN REMOVED. UNSURE IF IT'S THE MAP OR TRMOD ITSELF CAUSING THE ISSUE.

# Items to be removed: 1, 3, 4, 5, 6, 7, 8, 9, 10, 11, 14, 15, 16, 28, 29, 30, 31, 32, 38, 39, 40, 41, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 79, 80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 131, 132, 133, 145.

# Rooms to be removed: 1, 2, 3, 4, 5, 6, 7, 9, 10, 14, 19, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 42, 43, 45, 46, 47, 48, 49, 50, 51, 57, 58, 59, 60, 61, 62, 63, 64, 100, 112, 115.

# Commands: "removestaticmesh(room,*)" "removefloordata(room,*,*)" "removelight(room,*)" "resizeroom(room,1,1,height[in coords])" "zone(room,0,0,-1)" "floor(room,0,0,wall)" "ceiling(room,0,0,wall)" "removeitem(item)" "removegeometry(room)"

# In the end of the level, it is not possible to see anything outside of the glass in the uzis tank. To do so, rooms 26, 27 and 28's geometry must be restored. A price that is paid with performance and possibly a crash (not tested yet but would be good to have).

# Removes all static meshes in part 1 rooms:
# Static mesh from room 124 (yellow submarine) has to be removed due to corruption (textures are jumping to the room in front while the invisible collision remains in the original position), modifying position does not ammend the issue.
./trmod tr3pc underse2.tr2 "removestaticmesh(3,*)" "removestaticmesh(6,*)" "removestaticmesh(14,*)" "removestaticmesh(19,*)" "removestaticmesh(28,*)" "removestaticmesh(35,*)" "removestaticmesh(124,*)"

# Removes all floor data in part 1 rooms:
./trmod tr3pc underse2.tr2 "removefloordata(1,*,*)" "removefloordata(2,*,*)" "removefloordata(3,*,*)" "removefloordata(4,*,*)" "removefloordata(5,*,*)" "removefloordata(6,*,*)" "removefloordata(7,*,*)" "removefloordata(9,*,*)" "removefloordata(10,*,*)" "removefloordata(14,*,*)" "removefloordata(19,*,*)" "removefloordata(26,*,*)" "removefloordata(27,*,*)" "removefloordata(28,*,*)" "removefloordata(29,*,*)" "removefloordata(30,*,*)" "removefloordata(31,*,*)" "removefloordata(32,*,*)" "removefloordata(33,*,*)" "removefloordata(34,*,*)" "removefloordata(35,*,*)" "removefloordata(36,*,*)" "removefloordata(37,*,*)" "removefloordata(38,*,*)" "removefloordata(39,*,*)" "removefloordata(40,*,*)" "removefloordata(42,*,*)" "removefloordata(43,*,*)" "removefloordata(45,*,*)" "removefloordata(46,*,*)" "removefloordata(47,*,*)" "removefloordata(48,*,*)" "removefloordata(49,*,*)" "removefloordata(50,*,*)" "removefloordata(51,*,*)" "removefloordata(57,*,*)" "removefloordata(58,*,*)" "removefloordata(59,*,*)" "removefloordata(60,*,*)" "removefloordata(61,*,*)" "removefloordata(62,*,*)" "removefloordata(63,*,*)" "removefloordata(64,*,*)" "removefloordata(100,*,*)" "removefloordata(112,*,*)" "removefloordata(115,*,*)"

# Removes all lights in part 1 rooms:
./trmod tr3pc underse2.tr2 "removelight(1,*)" "removelight(2,*)" "removelight(3,*)" "removelight(4,*)" "removelight(5,*)" "removelight(6,*)" "removelight(7,*)" "removelight(9,*)" "removelight(10,*)" "removelight(14,*)" "removelight(19,*)" "removelight(26,*)" "removelight(27,*)" "removelight(28,*)" "removelight(30,*)" "removelight(31,*)" "removelight(34,*)" "removelight(35,*)" "removelight(36,*)" "removelight(37,*)" "removelight(38,*)" "removelight(39,*)" "removelight(40,*)" "removelight(42,*)" "removelight(43,*)" "removelight(45,*)" "removelight(46,*)" "removelight(47,*)" "removelight(48,*)" "removelight(49,*)" "removelight(50,*)" "removelight(51,*)" "removelight(57,*)" "removelight(58,*)" "removelight(59,*)" "removelight(60,*)" "removelight(61,*)" "removelight(62,*)" "removelight(63,*)" "removelight(64,*)" "removelight(100,*)" "removelight(112,*)" "removelight(115,*)"

# Resizes all part 1 rooms down to one square of varying heights (lots of bugs happen, either with textures and random freezes ("invalid byte sizes")):
#./trmod tr3pc underse2.tr2 "resizeroom(1,1,1,1792)" "resizeroom(2,1,1,2304)" "resizeroom(3,1,1,3584)" "resizeroom(4,1,1,1536)" "resizeroom(5,1,1,2304)" "resizeroom(6,1,1,2560)" "resizeroom(7,1,1,2816)" "resizeroom(9,1,1,5120)" "resizeroom(10,1,1,2304)" "resizeroom(14,1,1,8704)" "resizeroom(19,1,1,2304)" "resizeroom(26,1,1,4096)" "resizeroom(27,1,1,4096)" "resizeroom(28,1,1,5632)" "resizeroom(29,1,1,3072)" "resizeroom(30,1,1,3584)" "resizeroom(31,1,1,5632)" "resizeroom(32,1,1,2048)" "resizeroom(33,1,1,1024)" "resizeroom(34,1,1,5632)" "resizeroom(35,1,1,2304)" "resizeroom(36,1,1,3584)" "resizeroom(37,1,1,1792)" "resizeroom(38,1,1,1792)" "resizeroom(39,1,1,1280)" "resizeroom(40,1,1,1792)" "resizeroom(42,1,1,2304)" "resizeroom(43,1,1,2560)" "resizeroom(45,1,1,1280)" "resizeroom(46,1,1,2560)" "resizeroom(47,1,1,2560)" "resizeroom(48,1,1,1280)" "resizeroom(49,1,1,2560)" "resizeroom(50,1,1,1792)" "resizeroom(51,1,1,2048)" "resizeroom(57,1,1,2304)" "resizeroom(58,1,1,1280)" "resizeroom(59,1,1,1792)" "resizeroom(60,1,1,1536)" "resizeroom(61,1,1,1536)" "resizeroom(62,1,1,2048)" "resizeroom(63,1,1,3072)" "resizeroom(64,1,1,1280)" "resizeroom(100,1,1,1280)" "resizeroom(112,1,1,2048)" "resizeroom(115,1,1,1280)"

# Zones to 0 coordinates all part 1 rooms:
./trmod tr3pc underse2.tr2 "zone(1,0,0,-1)" "zone(2,0,0,-1)" "zone(3,0,0,-1)" "zone(4,0,0,-1)" "zone(5,0,0,-1)" "zone(6,0,0,-1)" "zone(7,0,0,-1)" "zone(9,0,0,-1)" "zone(10,0,0,-1)" "zone(14,0,0,-1)" "zone(19,0,0,-1)" "zone(26,0,0,-1)" "zone(27,0,0,-1)" "zone(28,0,0,-1)" "zone(29,0,0,-1)" "zone(30,0,0,-1)" "zone(31,0,0,-1)" "zone(32,0,0,-1)" "zone(33,0,0,-1)" "zone(34,0,0,-1)" "zone(35,0,0,-1)" "zone(36,0,0,-1)" "zone(37,0,0,-1)" "zone(38,0,0,-1)" "zone(39,0,0,-1)" "zone(40,0,0,-1)" "zone(42,0,0,-1)" "zone(43,0,0,-1)" "zone(45,0,0,-1)" "zone(46,0,0,-1)" "zone(47,0,0,-1)" "zone(48,0,0,-1)" "zone(49,0,0,-1)" "zone(50,0,0,-1)" "zone(51,0,0,-1)" "zone(57,0,0,-1)" "zone(58,0,0,-1)" "zone(59,0,0,-1)" "zone(60,0,0,-1)" "zone(61,0,0,-1)" "zone(62,0,0,-1)" "zone(63,0,0,-1)" "zone(64,0,0,-1)" "zone(100,0,0,-1)" "zone(112,0,0,-1)" "zone(115,0,0,-1)"

# Sets lowest height of the floor to all part 1 rooms:
./trmod tr3pc underse2.tr2 "floor(1,0,0,wall)" "floor(2,0,0,wall)" "floor(3,0,0,wall)" "floor(4,0,0,wall)" "floor(5,0,0,wall)" "floor(6,0,0,wall)" "floor(7,0,0,wall)" "floor(9,0,0,wall)" "floor(10,0,0,wall)" "floor(14,0,0,wall)" "floor(19,0,0,wall)" "floor(26,0,0,wall)" "floor(27,0,0,wall)" "floor(28,0,0,wall)" "floor(29,0,0,wall)" "floor(30,0,0,wall)" "floor(31,0,0,wall)" "floor(32,0,0,wall)" "floor(33,0,0,wall)" "floor(34,0,0,wall)" "floor(35,0,0,wall)" "floor(36,0,0,wall)" "floor(37,0,0,wall)" "floor(38,0,0,wall)" "floor(39,0,0,wall)" "floor(40,0,0,wall)" "floor(42,0,0,wall)" "floor(43,0,0,wall)" "floor(45,0,0,wall)" "floor(46,0,0,wall)" "floor(47,0,0,wall)" "floor(48,0,0,wall)" "floor(49,0,0,wall)" "floor(50,0,0,wall)" "floor(51,0,0,wall)" "floor(57,0,0,wall)" "floor(58,0,0,wall)" "floor(59,0,0,wall)" "floor(60,0,0,wall)" "floor(61,0,0,wall)" "floor(62,0,0,wall)" "floor(63,0,0,wall)" "floor(64,0,0,wall)" "floor(100,0,0,wall)" "floor(112,0,0,wall)" "floor(115,0,0,wall)"

# Sets lowest height of the ceiling to all part 1 rooms:
./trmod tr3pc underse2.tr2 "ceiling(1,0,0,wall)" "ceiling(2,0,0,wall)" "ceiling(3,0,0,wall)" "ceiling(4,0,0,wall)" "ceiling(5,0,0,wall)" "ceiling(6,0,0,wall)" "ceiling(7,0,0,wall)" "ceiling(9,0,0,wall)" "ceiling(10,0,0,wall)" "ceiling(14,0,0,wall)" "ceiling(19,0,0,wall)" "ceiling(26,0,0,wall)" "ceiling(27,0,0,wall)" "ceiling(28,0,0,wall)" "ceiling(29,0,0,wall)" "ceiling(30,0,0,wall)" "ceiling(31,0,0,wall)" "ceiling(32,0,0,wall)" "ceiling(33,0,0,wall)" "ceiling(34,0,0,wall)" "ceiling(35,0,0,wall)" "ceiling(36,0,0,wall)" "ceiling(37,0,0,wall)" "ceiling(38,0,0,wall)" "ceiling(39,0,0,wall)" "ceiling(40,0,0,wall)" "ceiling(42,0,0,wall)" "ceiling(43,0,0,wall)" "ceiling(45,0,0,wall)" "ceiling(46,0,0,wall)" "ceiling(47,0,0,wall)" "ceiling(48,0,0,wall)" "ceiling(49,0,0,wall)" "ceiling(50,0,0,wall)" "ceiling(51,0,0,wall)" "ceiling(57,0,0,wall)" "ceiling(58,0,0,wall)" "ceiling(59,0,0,wall)" "ceiling(60,0,0,wall)" "ceiling(61,0,0,wall)" "ceiling(62,0,0,wall)" "ceiling(63,0,0,wall)" "ceiling(64,0,0,wall)" "ceiling(100,0,0,wall)" "ceiling(112,0,0,wall)" "ceiling(115,0,0,wall)"

# Removes the part 1 rooms' items outright, instead of replacing them with a
# harmless placeholder (176) -- see undersea.sh for why descending order is
# what makes plain removeitem() safe here (verified empirically).
#
# Full list from the "Items to be removed" comment above (61 items), minus
# item 28 (object 24, sits in room 13 -- room 13 is deliberately shared/kept
# active by undersea.sh, so it must NOT be emptied here either) -- the
# earlier version only implemented 11 of these, leaving 49 items still
# sitting fully visible in the supposedly-emptied part-1 rooms. Item 2 (Lara
# herself, room 1) sits in a removed room too but is correctly left alone --
# never part of any documented removal list, don't add it.
./trmod tr3pc underse2.tr2 "removeitem(145,1)" "removeitem(133,1)" "removeitem(132,1)" "removeitem(131,1)" "removeitem(89,1)" "removeitem(88,1)" "removeitem(87,1)" "removeitem(86,1)" "removeitem(85,1)" "removeitem(84,1)" "removeitem(83,1)" "removeitem(82,1)" "removeitem(81,1)" "removeitem(80,1)" "removeitem(79,1)" "removeitem(74,1)" "removeitem(73,1)" "removeitem(72,1)" "removeitem(71,1)" "removeitem(70,1)" "removeitem(69,1)" "removeitem(68,1)" "removeitem(67,1)" "removeitem(66,1)" "removeitem(65,1)" "removeitem(64,1)" "removeitem(63,1)" "removeitem(62,1)" "removeitem(61,1)" "removeitem(60,1)" "removeitem(59,1)" "removeitem(58,1)" "removeitem(57,1)" "removeitem(56,1)" "removeitem(55,1)" "removeitem(54,1)" "removeitem(53,1)" "removeitem(52,1)" "removeitem(51,1)" "removeitem(41,1)" "removeitem(40,1)" "removeitem(39,1)" "removeitem(38,1)" "removeitem(32,1)" "removeitem(31,1)" "removeitem(30,1)" "removeitem(29,1)" "removeitem(16,1)" "removeitem(15,1)" "removeitem(14,1)" "removeitem(11,1)" "removeitem(10,1)" "removeitem(9,1)" "removeitem(8,1)" "removeitem(7,1)" "removeitem(6,1)" "removeitem(5,1)" "removeitem(4,1)" "removeitem(3,1)" "removeitem(1,1)"

# Removes all geometry in part 1 rooms: (rooms 26, 27 and 28 excluded so that the last aquarium doesn't appear black/empty)
./trmod tr3pc underse2.tr2 "removegeometry(1)" "removegeometry(2)" "removegeometry(3)" "removegeometry(4)" "removegeometry(5)" "removegeometry(6)" "removegeometry(7)" "removegeometry(9)" "removegeometry(10)" "removegeometry(14)" "removegeometry(19)" "removegeometry(29)" "removegeometry(30)" "removegeometry(31)" "removegeometry(32)" "removegeometry(33)" "removegeometry(34)" "removegeometry(35)" "removegeometry(36)" "removegeometry(37)" "removegeometry(38)" "removegeometry(39)" "removegeometry(40)" "removegeometry(42)" "removegeometry(43)" "removegeometry(45)" "removegeometry(46)" "removegeometry(47)" "removegeometry(48)" "removegeometry(49)" "removegeometry(50)" "removegeometry(51)" "removegeometry(57)" "removegeometry(58)" "removegeometry(59)" "removegeometry(60)" "removegeometry(61)" "removegeometry(62)" "removegeometry(63)" "removegeometry(64)" "removegeometry(100)" "removegeometry(112)" "removegeometry(115)"

# Removes floor data from the end of room 44 (unneeded "clearbodies" trigger):
./trmod tr3pc underse2.tr2 "removefloordata(44,1,2)" "removefloordata(44,1,3)"

# Removes floor data from the beginning of room 13 (unneeded "killalltriggers" trigger):
./trmod tr3pc underse2.tr2 "removefloordata(13,16,8)" "removefloordata(13,16,9)"

# Rotates the diving bells to hide the part that doesn't exist in underse2.tr2:
# NOTE: these item indices (originally 48, 50) are recomputed for the FULL
# 60-item removal above (was 27, 29 -- previously 37, 39 for the old,
# incomplete 11-item removal list; don't blindly reuse old numbers here if
# the removal list above ever changes again, recompute instead).
./trmod tr3pc underse2.tr2 "replaceitem(27,,,,,,16384,,,)"
./trmod tr3pc underse2.tr2 "replaceitem(29,,,,,,16384,,,)"
# The two submarines coming down (ANIMATING6) start in rooms 24 and 25, which have no light at the darkest
# ambient, so they were almost black: a light at each submarine (a fixed shade instead flickers on the PS1).
./trmod tr3pc underse2.tr2 "addlight(24,3584,6656,3584,D0E8E8,spotlight,4095,4096)" "addlight(25,3584,7680,2432,D0E8E8,spotlight,4095,4096)"

# Sets Lara's new position in the end of room 44:
# NOTE: original item index 2 -> now 1 after the full 60-item removal above
# (same reasoning as the diving-bell indices).
./trmod tr3pc underse2.tr2 "replaceitem(1,,13,16896,9216,3584,,,,)"

# Makes the skybox invisible for part 2 except in room 111:
./trmod tr3pc underse2.tr2 "nosky(*)" "sky(111)"

# Fixes rooms 44 and 53 being 34 clicks tall, 3 clicks over the PSX room
# format's 31-click vertex-height limit (tr3pcpsx would otherwise silently
# truncate these vertices' Y-value, corrupting the geometry -- listed as a
# known bug in split_info.txt). The affected vertices all belong to a
# purely decorative "abyss" floor sitting well below the room's real
# (FloorData-defined) walkable floor, so raising them by 768 units (3
# clicks) is invisible to gameplay/collision and only makes the decorative
# drop 3 clicks shallower. Y-parameter is height-above-yBottom (trmod
# convention), computed as yBottom - (original_Y - 768). Verified with a
# clean tr3pcpsx conversion (zero Y-value warnings).
./trmod tr3pc underse2.tr2 "replacevertex(44,76,,,1280)" "replacevertex(44,79,,,1280)" "replacevertex(44,81,,,1280)" "replacevertex(44,86,,,768)" "replacevertex(44,89,,,768)" "replacevertex(44,91,,,768)" "replacevertex(44,96,,,768)" "replacevertex(44,99,,,768)" "replacevertex(44,101,,,768)"
./trmod tr3pc underse2.tr2 "replacevertex(53,15,,,1280)" "replacevertex(53,27,,,1280)" "replacevertex(53,30,,,768)" "replacevertex(53,32,,,1024)" "replacevertex(53,51,,,1280)" "replacevertex(53,218,,,1280)" "replacevertex(53,277,,,1280)" "replacevertex(53,279,,,1280)" "replacevertex(53,281,,,1280)" "replacevertex(53,309,,,1280)" "replacevertex(53,361,,,1280)" "replacevertex(53,393,,,1280)" "replacevertex(53,395,,,1024)"
