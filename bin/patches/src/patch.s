; #############################################
; # SOURCE CODE FOR THE LOST ARTEFACT PATCHES #
; #############################################
.psx

; ###################
; # FILE INCLUSIONS #
; ###################
.include "./ver/INCLUDE_VERSION_VARIABLE.s" ; Definitions per version
.include "./inc/common.s" ; Common definitions

; #####################
; # MACRO DEFINITIONS #
; #####################
MEM_OFFSET				equ		0x00010000
MAX_SAVENAME_LENGTH		equ		0x00000040

; Macro that defines the entry for a patch
; Variables:
; * PE_SP = Starting offset of the code in code.bin
; * PE_EP = Ending offset of the code in code.bin
; * PE_FO = Offset to write the code to in binary
.macro PatchEntry, PE_SP, PE_EP, PE_FO
	dw		(PE_SP-MEM_OFFSET), (PE_EP-PE_SP), PE_FO
.endmacro

; #############
; # CODE FILE #
; #############
.create "./out/code.bin", MEM_OFFSET ; Code file
	; Main Code
	.org MEM_OFFSET

	; Bird Patch
		.org M_BaddieObjects_Vulture
		Bird_Vulture:
			lw		$v1, Baddie7Ptr
		Bird_Vulture_end:
		.org M_BaddieObjects_Crow
		Bird_Crow:
			lw		$v1, Baddie7Ptr
		Bird_Crow_end:

	; Croc Patch
		.org M_BaddieObjects_Croc
		Croc:
			lw		$v1, Baddie7Ptr
		Croc_end:

	; Lizard Patch
		.org M_BaddieObjects_Lizard
		Lizard:
			lw		$v1, Baddie6Ptr
		Lizard_end:
	
	; NoFMV Patch
		.org M_GF_InterpretSequence_FMV
		NoFMV:
			nop
		NoFMV_end:

	; InitLoad Patch
		.org M_main_Initload
		InitLoad:
			nop
		InitLoad_end:

	; Secrets Patch
		.org M_LevelSecrets
		Secrets:
			.dh 0x0000, 0x0003, 0x0003, 0x0003, 0x0003, 0x0003, 0x0000
			.dh 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000
			.dh 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000
		Secrets_end:

	; No Adventure Select Patch
		.org M_LevelStats_LV_INDIABOSS
		Adventure:
			addiu	$v0, $zero, 0xFF ; Loads adventure select on level 255 instead of 4
		Adventure_end:

	; Save Name Patch
		.org M_SaveName
		SaveName:
			SAVENAME

	; Story So Far Patch
		.org M_GetSavedGamesList_strsofar
		StorySoFar:
			; This simply wipes out the code that loads the "Story So Far"-
			; option at the bottom of the Level Select screen.
			nop
			nop
			nop
			nop
			nop
			nop
			nop
			nop
			nop
			nop
			nop
			nop
			nop
			nop
		StorySoFar_end:

	; Sky Colour Patch
		.org M_LevelHorizonColours
		SkyColour:
			.dw 0x28402828, 0x28583840, 0x28583840, 0x28D0A860
			.dw 0x28080000, 0x28000000, 0x28000000
		SkyColour_end:

	; Loadbar Patch
		.org M_LOAD_VSyncHandler_WIDTH
		LoadBar_first:
			slti	$v0, LOADBAR_WIDTH
		LoadBar_first_end:
			nop
		LoadBar_second:
			addiu	$v0, $zero, LOADBAR_WIDTH
		LoadBar_second_end:
		.org M_LOAD_VSyncHandler_LOADBAR_X
		LoadBar_third:
			addiu	$a0, $zero, LOADBAR_X
		LoadBar_third_end:
			nop
			nop
		LoadBar_fourth:
			addiu	$a2, $zero, (LOADBAR_WIDTH>>3)
		LoadBar_fourth_end:
		.org M_LOAD_Stop
			nop
		LoadBar_fifth:
			addiu	$v0, $zero, (LOADBAR_WIDTH-1)
		LoadBar_fifth_end:
		.org M_LOAD_Update_LOADBAR_WIDTH
		LoadBar_sixth:
			lui		$v1, LOADBAR_WIDTH
		LoadBar_sixth_end:
			nop
			nop
			nop
			nop
			nop
		LoadBar_seventh:
			slti	$v0, $v1, LOADBAR_WIDTH
		LoadBar_seventh_end:
			nop
		LoadBar_eighth:
			addiu	$v0, $zero, LOADBAR_WIDTH
		LoadBar_eighth_end:
.close

; ###############
; # PATCH FILES #
; ###############
.create "./out/bird.pat", 0x00000000
	PatchEntry Bird_Vulture, Bird_Vulture_end, F_BaddieObjects_Vulture
	PatchEntry Bird_Crow, Bird_Crow_end, F_BaddieObjects_Crow
.close
.create "./out/croc.pat", 0x00000000
	PatchEntry Croc, Croc_end, F_BaddieObjects_Croc
.close
.create "./out/lizard.pat", 0x00000000
	PatchEntry Lizard, Lizard_end, F_BaddieObjects_Lizard
.close
.create "./out/nofmv.pat", 0x00000000
	PatchEntry NoFMV, NoFMV_end, F_GF_InterpretSequence_FMV
.close
.create "./out/initload.pat", 0x00000000
	PatchEntry InitLoad, InitLoad_end, F_main_Initload
.close
.create "./out/secrets.pat", 0x00000000
	PatchEntry Secrets, Secrets_end, F_LevelSecrets
.close
.create "./out/adventur.pat", 0x00000000
	PatchEntry Adventure, Adventure_end, F_LevelStats_LV_INDIABOSS
.close
.create "./out/savename.pat", 0x00000000
	PatchEntry SaveName, (SaveName+MAX_SAVENAME_LENGTH), F_SaveName
.close
.create "./out/strsofar.pat", 0x00000000
	PatchEntry StorySoFar, StorySoFar_end, F_GetSavedGamesList_strsofar
.close
.create "./out/sky.pat", 0x00000000
	PatchEntry SkyColour, SkyColour_end, F_LevelHorizonColours
.close
.create "./out/loadbar.pat", 0x00000000
	PatchEntry LoadBar_first, LoadBar_first_end, F_LOAD_VSyncHandler_WIDTH
	PatchEntry LoadBar_second, LoadBar_second_end, (F_LOAD_VSyncHandler+(LoadBar_second-M_LOAD_VSyncHandler))
	PatchEntry LoadBar_third, LoadBar_third_end, F_LOAD_VSyncHandler_LOADBAR_X
	PatchEntry LoadBar_fourth, LoadBar_fourth_end, (F_LOAD_VSyncHandler+(LoadBar_fourth-M_LOAD_VSyncHandler))
	PatchEntry LoadBar_fifth, LoadBar_fifth_end, (F_LOAD_Stop+(LoadBar_fifth-M_LOAD_Stop))
	PatchEntry LoadBar_sixth, LoadBar_sixth_end, F_LOAD_Update_LOADBAR_WIDTH
	PatchEntry LoadBar_seventh, LoadBar_seventh_end, (F_LOAD_Update+(LoadBar_seventh-M_LOAD_Update))
	PatchEntry LoadBar_eighth, LoadBar_eighth_end, (F_LOAD_Update+(LoadBar_eighth-M_LOAD_Update))
.close
