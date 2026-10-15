; Definitions for the US-version

; Variables
Baddie6Ptr		equ		0x567C($gp) ; Pointer to Baddie6 Module Slot (Slot 10)
Baddie7Ptr		equ		0x5680($gp) ; Pointer to Baddie7 Module Slot (Slot 11)

; Functions (Memory Location)
M_main					equ		0x00010020
M_BaddieObjects			equ		0x00058D34
M_GF_InterpretSequence	equ		0x000381E8
M_LevelStats			equ		0x0006D450
M_GetSavedGamesList		equ		0x0006DF68
M_LOAD_VSyncHandler		equ		0x0006AEA8
M_LOAD_Stop				equ		0x0006B008
M_LOAD_Update			equ		0x0006B058

; Variables (Memory Locations)
M_LevelSecrets			equ		0x00097F38
M_SaveName				equ		0x00065EA8
M_LevelHorizonColours	equ		0x0008D77C

; Functions (File Offset)
F_main					equ		0x00000820
F_BaddieObjects			equ		0x00049534
F_GF_InterpretSequence	equ		0x000289E8
F_LevelStats			equ		0x0005DC50
F_GetSavedGamesList		equ		0x0005E768
F_LOAD_VSyncHandler		equ		0x0005B6A8
F_LOAD_Stop				equ		0x0005B808
F_LOAD_Update			equ		0x0005B858

; Variables (File Offsets)
F_LevelSecrets			equ		0x00088738
F_SaveName				equ		0x000566A8
F_LevelHorizonColours	equ		0x0007DF7C

; Positions in functions (Memory Locations)
M_BaddieObjects_Vulture			equ		(M_BaddieObjects+0x1A38)
M_BaddieObjects_Crow			equ		(M_BaddieObjects_Vulture+0x0080)
M_BaddieObjects_Croc			equ		(M_BaddieObjects+0x2074)
M_BaddieObjects_Lizard			equ		(M_BaddieObjects+0x13F8)
M_GF_InterpretSequence_FMV		equ		(M_GF_InterpretSequence+0x036C)
M_main_Initload					equ		(M_main+0x34)
M_LevelStats_LV_INDIABOSS		equ		(M_LevelStats+0x02D0)
M_GetSavedGamesList_strsofar	equ		(M_GetSavedGamesList+0x024C)
M_LOAD_VSyncHandler_WIDTH		equ		(M_LOAD_VSyncHandler+0x004C)
M_LOAD_VSyncHandler_LOADBAR_X	equ		(M_LOAD_VSyncHandler+0x0080)
M_LOAD_Update_LOADBAR_WIDTH		equ		(M_LOAD_Update+0x0048)

; Title for the game's save file
.macro	SAVENAME
.dh 0x7382 ; T
.dh 0x8F82 ; o
.dh 0x8D82 ; m
.dh 0x8282 ; b
.dh 0x4081 ; [space]
.dh 0x7182 ; R
.dh 0x8182 ; a
.dh 0x8982 ; i
.dh 0x8482 ; d
.dh 0x8582 ; e
.dh 0x9282 ; r
.dh 0x4681 ; :
.dh 0x4081 ; [space]
.dh 0x7382 ; T
.dh 0x8882 ; h
.dh 0x8582 ; e
.dh 0x4081 ; [space]
.dh 0x6B82 ; L
.dh 0x8F82 ; o
.dh 0x9382 ; s
.dh 0x9482 ; t
.dh 0x4081 ; [space]
.dh 0x6082 ; A
.dh 0x9282 ; r
.dh 0x9482 ; t
.dh 0x8982 ; i
.dh 0x8682 ; f
.dh 0x8182 ; a
.dh 0x8382 ; c
.dh 0x9482 ; t
.dh 0x0000
.endmacro
