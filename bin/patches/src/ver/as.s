; Definitions for the Asian version

; Variables
Baddie6Ptr		equ		0x522C($gp) ; Pointer to Baddie6 Module Slot (Slot 10)
Baddie7Ptr		equ		0x5230($gp) ; Pointer to Baddie7 Module Slot (Slot 11)

; Functions (Memory Location)
M_main					equ		0x00010020
M_BaddieObjects			equ		0x00058FDC
M_GF_InterpretSequence	equ		0x00038344
M_LevelStats			equ		0x0006DE6C
M_GetSavedGamesList		equ		0x0006E998
M_LOAD_VSyncHandler		equ		0x0006B8BC
M_LOAD_Stop				equ		0x0006BA1C
M_LOAD_Update			equ		0x0006BA6C

; Variables (Memory Locations)
M_LevelSecrets			equ		0x00098850
M_SaveName				equ		0x00066250
M_LevelHorizonColours	equ		0x0008DF64

; Functions (File Offset)
F_main					equ		0x00000820
F_BaddieObjects			equ		0x000497DC
F_GF_InterpretSequence	equ		0x00028B44
F_LevelStats			equ		0x0005E66C
F_GetSavedGamesList		equ		0x0005F198
F_LOAD_VSyncHandler		equ		0x0005C0BC
F_LOAD_Stop				equ		0x0005C21C
F_LOAD_Update			equ		0x0005C26C

; Variables (File Offsets)
F_LevelSecrets			equ		0x00089050
F_SaveName				equ		0x00056A50
F_LevelHorizonColours	equ		0x0007E764

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
.dh 0x8582 ; e
.dh 0x8682 ; f
.dh 0x8182 ; a
.dh 0x8382 ; c
.dh 0x9482 ; t
.dh 0x0000
.endmacro
