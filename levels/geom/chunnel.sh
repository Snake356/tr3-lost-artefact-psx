############################################
# CHUNNEL.TR2 Geometry Optimisation Script #
############################################

# Removes collision data from room 25 from chunnel.tr2
./trmod tr3pc chunnel.tr2 "removefloordata(25,*,*)" "resizeroom(25,1,1,6656)" "zone(25,0,0,-1)" "floor(25,0,0,wall)" "ceiling(25,0,0,wall)" "removelight(25,*)"

# Removes references to room 25 from floordata in chunnel.tr2
./trmod tr3pc chunnel.tr2 "removefloordata(22,1,9)" "removefloordata(22,2,9)" "removefloordata(22,3,9)" "floor(22,1,9,wall)" "ceiling(22,1,9,wall)" "floor(22,2,9,wall)" "ceiling(22,2,9,wall)" "floor(22,3,9,wall)" "ceiling(22,3,9,wall)"

# Removes collision data from room 26 in chunnel.tr2
./trmod tr3pc chunnel.tr2 "removefloordata(26,*,*)" "resizeroom(26,1,1,6656)" "zone(26,0,0,-1)" "floor(26,0,0,wall)" "ceiling(26,0,0,wall)" "removelight(26,*)"

# Removes collision data from room 29 in chunnel.tr2
./trmod tr3pc chunnel.tr2 "removefloordata(29,*,*)" "resizeroom(29,1,1,5376)" "zone(29,0,0,-1)" "floor(29,0,0,wall)" "ceiling(29,0,0,wall)" "removelight(29,*)"

# Removes references to room 29 from floordata in chunnel.tr2
./trmod tr3pc chunnel.tr2 "removefloordata(22,1,0)" "removefloordata(22,2,0)" "removefloordata(22,3,0)" "floor(22,1,0,wall)" "ceiling(22,1,0,wall)" "floor(22,2,0,wall)" "ceiling(22,2,0,wall)" "floor(22,3,0,wall)" "ceiling(22,3,0,wall)"

# Removes collision data from room 30 in chunnel.tr2
./trmod tr3pc chunnel.tr2 "removefloordata(30,*,*)" "resizeroom(30,1,1,5376)" "zone(30,0,0,-1)" "floor(30,0,0,wall)" "ceiling(30,0,0,wall)" "removelight(30,*)"

# Removes collision data from room 31 in chunnel.tr2
./trmod tr3pc chunnel.tr2 "removefloordata(31,*,*)" "resizeroom(31,1,1,2048)" "zone(31,0,0,-1)" "floor(31,0,0,wall)" "ceiling(31,0,0,wall)" "removelight(31,*)"

# Removes collision data from room 32 in chunnel.tr2
./trmod tr3pc chunnel.tr2 "removefloordata(32,*,*)" "resizeroom(32,1,1,2048)" "zone(32,0,0,-1)" "floor(32,0,0,wall)" "ceiling(32,0,0,wall)" "removelight(32,*)"

# Removes collision data from room 33 in chunnel.tr2
./trmod tr3pc chunnel.tr2 "removefloordata(33,*,*)" "resizeroom(33,1,1,2560)" "zone(33,0,0,-1)" "floor(33,0,0,wall)" "ceiling(33,0,0,wall)" "removelight(33,*)"

# Removes references to room 33 from floordata in chunnel.tr2
./trmod tr3pc chunnel.tr2 "removefloordata(23,*,0)" "floor(23,*,0,wall)" "ceiling(23,*,0,wall)"

# Removes collision data from room 34 in chunnel.tr2
./trmod tr3pc chunnel.tr2 "removefloordata(34,*,*)" "resizeroom(34,1,1,2560)" "zone(34,0,0,-1)" "floor(34,0,0,wall)" "ceiling(34,0,0,wall)" "removelight(34,*)"

# Removes references to room 34 from floordata in chunnel.tr2
./trmod tr3pc chunnel.tr2 "removefloordata(24,*,0)" "floor(24,*,0,wall)" "ceiling(24,*,0,wall)"

# Removes collision data from room 38 in chunnel.tr2
./trmod tr3pc chunnel.tr2 "removefloordata(38,*,*)" "resizeroom(38,1,1,1792)" "zone(38,0,0,-1)" "floor(38,0,0,wall)" "ceiling(38,0,0,wall)" "removelight(38,*)"

# Removes collision data from room 39 in chunnel.tr2
./trmod tr3pc chunnel.tr2 "removefloordata(39,*,*)" "resizeroom(39,1,1,1792)" "zone(39,0,0,-1)" "floor(39,0,0,wall)" "ceiling(39,0,0,wall)" "removelight(39,*)"

# Removes collision data from room 47 in chunnel.tr2
./trmod tr3pc chunnel.tr2 "removefloordata(47,*,*)" "resizeroom(47,1,1,3072)" "zone(47,0,0,-1)" "floor(47,0,0,wall)" "ceiling(47,0,0,wall)" "removelight(47,*)"

# Removes collision data from room 48 in chunnel.tr2
./trmod tr3pc chunnel.tr2 "removefloordata(48,*,*)" "resizeroom(48,1,1,3072)" "zone(48,0,0,-1)" "floor(48,0,0,wall)" "ceiling(48,0,0,wall)" "removelight(48,*)"

# Removes collision data from room 51 in chunnel.tr2
./trmod tr3pc chunnel.tr2 "removefloordata(51,*,*)" "resizeroom(51,1,1,3072)" "zone(51,0,0,-1)" "floor(51,0,0,wall)" "ceiling(51,0,0,wall)" "removelight(51,*)"

# Removes the geometry from room 73 in chunnel.tr2
./trmod tr3pc chunnel.tr2 "removegeometry(73)"

# Makes the sky invisible from deep within the tunnel
./trmod tr3pc chunnel.tr2 "nosky(3)" "nosky(6)" "nosky(9)" "nosky(17)" "nosky(20)" "nosky(80)" "nosky(106)" "nosky(109)" "nosky(111)" "nosky(112)" "nosky(113)" "inside(3)" "inside(6)" "inside(9)" "inside(17)" "inside(20)" "inside(80)" "inside(106)" "inside(109)" "inside(111)" "inside(112)" "inside(113)"
