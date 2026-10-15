; Configuration common for all PAL-versions

; Variables
Baddie6Ptr		equ		0x567C($gp) ; Pointer to Baddie6 Module Slot (Slot 10)
Baddie7Ptr		equ		0x5680($gp) ; Pointer to Baddie7 Module Slot (Slot 11)

; Functions (Memory Location)
M_main					equ		0x00010020
M_BaddieObjects			equ		0x00058D3C
M_GF_InterpretSequence	equ		0x000381F0
M_LevelStats			equ		0x0006D468
M_GetSavedGamesList		equ		0x0006DF80
M_LOAD_VSyncHandler		equ		0x0006AEC0
M_LOAD_Stop				equ		0x0006B020
M_LOAD_Update			equ		0x0006B070

; Variables (Memory Locations)
M_LevelSecrets			equ		0x00097DE4
M_SaveName				equ		0x00065ECC
M_LevelHorizonColours	equ		0x0008D628

; Functions (File Offset)
F_main					equ		0x00000820
F_BaddieObjects			equ		0x0004953C
F_GF_InterpretSequence	equ		0x000289F0
F_LevelStats			equ		0x0005DC68
F_GetSavedGamesList		equ		0x0005E780
F_LOAD_VSyncHandler		equ		0x0005B6C0
F_LOAD_Stop				equ		0x0005B820
F_LOAD_Update			equ		0x0005B870

; Variables (File Offsets)
F_LevelSecrets			equ		0x000885E4
F_SaveName				equ		0x000566CC
F_LevelHorizonColours	equ		0x0007DE28

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
