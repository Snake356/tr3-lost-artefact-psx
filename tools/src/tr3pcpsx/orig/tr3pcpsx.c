/*
 * Tomb Raider III PC to PSX Level Converter
 * Copyright (C) 2021 b122251
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not,see <http://www.gnu.org/licenses/>.
 */

/* File Inclusions */
#include <stdio.h>    /* Standard I/O */
#include <stdlib.h>   /* Standard Library */
#include <string.h>   /* Functions relating to strings */
#include "params.h"   /* Handles command-line parameters */
#include "fixedint.h" /* Definition of fixed-size integers */
#include "structs.h"  /* Definition of structs used by TR3PCPSX */
#include "util.h"     /* General-purpose code used by TR3PCPSX */
#include "osdep.h"    /* Library of OS-dependant functions */
#include "textures.h" /* Library for converting textures */
#include "colour.h"   /* Library for converting colours */
#include "audio.h"    /* Library for converting audio samples */
#include "modules.h"  /* Library for adding code modules */
#include "errors.h"   /* Error Handling for TR3PCPSX */
#include "license.h"  /* Prints the GNU General Public License 3 */
#include "version.h"  /* Program name and version number */

/* Static function declarations */
static int convert(char *pcpath, char *psxpath, INTU32 flags,
                   struct water_colour *waterColour, INTU32 skyColour,
                   char *sfxPath);
static int tr3pc_navigate(FILE *pc, struct level_info *pc_info);
static int tr3psx_navigate(FILE *psx, struct level_info *psx_info,
                           INTU32 flags);
static int tr3psx_create(FILE *psx, int jap);
static int convertRooms(FILE *pc, FILE *psx, INTU32 pc_p_NumRooms,
                        struct level_info *psx_info, char *psxpath,
                        INTU16 numRooms, struct room_info *room,
                        INTU32 flags, INTU16 *roomTextures);
static int buildOutsideTable(FILE *psx, INTU16 numRooms, struct room_info *room,
                             struct level_info *psx_info);
static int convertMeshPointers(FILE *pc, FILE *psx, char *psxpath,
                               struct level_info *pc_info,
                               struct level_info *psx_info, INTU32 flags,
                               INTU16 numMisorientedTextures,
                               INTU16 *misorientedTextures,
                               INTU16 *objectTextures);
static int convertMesh(FILE *pc, FILE *psx, struct level_info *psx_info,
                       INT32 pc_MeshPos, INTU32 *psx_curPointer, INTU32 flags,
                       int skybox, INTU16 numMisorientedTextures,
                       INTU16 *misorientedTextures,
                       INTU16 *objectTextures);
static int convertSounds(FILE *pc, FILE *psx, struct level_info *pc_info,
                         struct level_info *psx_info, char *psxPath,
                         char *sfxPath);
static INTU16 findMesh(FILE *pc, struct level_info *pc_info, INTU32 objectID);
#ifdef FIXEDINT_CHECK_ON_STARTUP
static int fixedint_check(void);
#endif

/*
 * Main function of the program.
 * Parameters:
 * * argc = Number of command-line parameters
 * * argc = Command-line parameters
 */
int main(int argc, char *argv[])
{
	/* Variable Declarations */
	INTU32 flags = 0x00000000;       /* Bit-mask of command-line parameters */
	struct water_colour waterColour; /* Struct for calculating water colour */
	INTU32 skyColour = 0x00000000;   /* Colour of the sky */
	int pcparam = 0;                 /* Parameter number of the PC-file */
	int psxparam = 0;                /* Parameter number of the PSX-file */
	char *sfxPath = "MAIN.SFX";      /* Path to the MAIN.SFX file */
	int errorNumber;                 /* Return value for the program */
	errorNumber = ERROR_NONE;
	
	/* Checks the sizes of fixed-size integers */
#ifdef FIXEDINT_CHECK_ON_STARTUP
	errorNumber = fixedint_check();
	if (errorNumber != ERROR_NONE)
	{
		return ERROR_FIXED_INTEGERS;
	}
#endif
	
	/* Sets water colour to default value */
	waterColour.instruction = 0x0DDD;
	waterColour.red_operand = 0x000E;
	waterColour.green_operand = 0x0019;
	waterColour.blue_operand = 0x0013;
	
	/* Reads command-line parameters */
	errorNumber = interpretParameters(argc, argv, &pcparam, &psxparam, &flags,
	                                  &waterColour, &skyColour, &sfxPath);
	
	/* Checks whether "show license"-flag was set */
	if (isflag(FLAGS_LICENSE))
	{
		showLicense();
		return ERROR_NONE;
	}
	
	/* Checks that PC and PSX files were specified */
	if ((errorNumber == ERROR_NONE) && ((pcparam == 0) || (psxparam == 0)))
	{
		errorNumber = ERROR_INVALID_PARAMETERS;
	}
	
	/* Converts the level */
	if (errorNumber == ERROR_NONE)
	{
		errorNumber = convert(argv[pcparam], argv[psxparam], flags,
		                      &waterColour, skyColour, sfxPath);
	}
	
	/* Prints error message if needed */
	printError(errorNumber, argv, pcparam, psxparam, sfxPath);
	
	/* Ends the program */
	return errorNumber;
}

/*
 * Function that converts a level from PC to PSX.
 * Parameters:
 * * pcpath = Path to the PC-file
 * * psxpath = Path to the PSX-file
 * * flags = Bit-mask of command-line flags
 * * waterColour = Colour of the water
 * * skyColour = Colour of the sky
 * * sfxPath = Path to MAIN.SFX
 */
static int convert(char *pcpath, char *psxpath, INTU32 flags,
                   struct water_colour *waterColour, INTU32 skyColour,
                   char *sfxPath)
{
	/* Variable Declatations */
	FILE *pc = NULL;  /* Pointer to PC-file */
	FILE *psx = NULL; /* Pointer to PSX-file */
	struct level_info pc_info;  /* Offsets and values in PC-file */
	struct level_info psx_info; /* Offsets and values in PSX-file */
	INTU16 numMisorientedTextures = 0;  /* Number of misoriented textures */
	INTU16 *misorientedTextures = NULL; /* Array of misoriented textures */
	INTU16 *roomTextures = NULL;   /* Array of room textures */
	INTU16 *objectTextures = NULL; /* Array of object textures */
	int freeint1;       /* Integer for general use */
	size_t freesizet1;  /* size_t for general use */
	INTU32 freeuint321,
	       freeuint322,
	       freeuint323; /* General-purpose unsigned 32-bit integers */
	INTU16 freeuint161; /* General-purpose unsigned 16-bit integers */
	long int freelong1; /* Long integer for general use */
	int errorNumber; /* Return value for this program */
	
	/* Sets level_info structs to NULL before we start */
	level_info_wipe(pc_info);
	level_info_wipe(psx_info);
	
	/* Opens PC-file */
	pc = fopen(pcpath, "rb");
	if (pc == NULL)
	{
		errorNumber = ERROR_PC_OPEN_FAILED;
		goto end;
	}
	
	/* Tries to open the PSX-file as host file */
	psx = fopen(psxpath, "r+b");
	if (psx == NULL)
	{
		/* Tries to open a new PSX-file */
		psx = fopen(psxpath, "w+b");
		if (psx == NULL)
		{
			errorNumber = ERROR_PSX_OPEN_FAILED;
			goto end;
		}
		setflag(FLAGS_NEW_LEVEL);
	}
	
	/* Determines number of rooms in the level */
	freeint1 = fseek(pc, 1796, SEEK_SET);
	if (freeint1 != 0)
	{
		errorNumber = ERROR_PC_READ_FAILED;
		goto end;
	}
	freesizet1 = fread(&pc_info.v_NumTexTiles, 1, 4, pc);
	if (freesizet1 != 4)
	{
		errorNumber = ERROR_PC_READ_FAILED;
		goto end;
	}
#ifdef __BIG_ENDIAN__
	pc_info.v_NumTexTiles = reverse32(pc_info.v_NumTexTiles);
#endif
	pc_info.p_NumRooms = (1804 + (pc_info.v_NumTexTiles * 196608));
	freeint1 = fseek(pc, (long int) pc_info.p_NumRooms, SEEK_SET);
	if (freeint1 != 0)
	{
		errorNumber = ERROR_PC_READ_FAILED;
		goto end;
	}
	freesizet1 = fread(&pc_info.v_NumRooms, 1, 2, pc);
	if (freesizet1 != 2)
	{
		errorNumber = ERROR_PC_READ_FAILED;
		goto end;
	}
#ifdef __BIG_ENDIAN__
	pc_info.v_NumRooms = reverse16(pc_info.v_NumRooms);
#endif
	
	/* Allocates space for room-related variables */
	pc_info.room = calloc((size_t) pc_info.v_NumRooms,
	                      sizeof(struct room_info));
	if (pc_info.room == NULL)
	{
		errorNumber = ERROR_MEMORY;
		goto end;
	}
	psx_info.room = calloc((size_t) pc_info.v_NumRooms,
	                       sizeof(struct room_info));
	if (psx_info.room == NULL)
	{
		errorNumber = ERROR_MEMORY;
		goto end;
	}
	
	/* Navigates through PC-file */
	if (isflag(FLAGS_VERBOSE))
	{
#ifdef CLEARTERMINAL
		CLEARTERMINAL
#endif
		printf("Navigating PC-file %s... ", pcpath);
		(void) fflush(stdout);
	}
	errorNumber = tr3pc_navigate(pc, &pc_info);
	if (errorNumber != ERROR_NONE)
	{
		PRINT_FAILED;
		goto end;
	}
	PRINT_DONE;
	
	if (notflag(FLAGS_NEW_LEVEL))
	{
		/* Navigates through PSX-file */
		if (isflag(FLAGS_VERBOSE))
		{
			printf("Navigating PSX-file %s... ", psxpath);
			(void) fflush(stdout);
		}
		errorNumber = tr3psx_navigate(psx, &psx_info, flags);
		if (errorNumber != ERROR_NONE)
		{
			PRINT_FAILED;
			goto end;
		}
		PRINT_DONE;
	}
	else
	{
		/* Creates PSX-file */
		if (isflag(FLAGS_VERBOSE))
		{
			printf("Creating PSX-file %s... ", psxpath);
			(void) fflush(stdout);
		}
		if (((flags & FLAGS_LANGUAGE) == FLAGS_LANGUAGE_JP_JP) ||
		    ((flags & FLAGS_LANGUAGE) == FLAGS_LANGUAGE_ASIA_JP))
		{
			freeint1 = 1;
		}
		else
		{
			freeint1 = 0;
		}
		errorNumber = tr3psx_create(psx, freeint1);
		if (errorNumber != ERROR_NONE)
		{
			PRINT_FAILED;
			goto end;
		}
		errorNumber = tr3psx_navigate(psx, &psx_info, flags);
		if (errorNumber != ERROR_NONE)
		{
			PRINT_FAILED;
			goto end;
		}
		PRINT_DONE;
	}
	
	/* Sets PSX NumRooms to equal PC */
	psx_info.v_NumRooms = pc_info.v_NumRooms;
	
	/* Converts the audio samples */
	if (notflag(FLAGS_SKIP_AUDIO))
	{
		if (isflag(FLAGS_VERBOSE))
		{
			printf("Converting Sound Samples... ");
			(void) fflush(stdout);
		}
		
		/* Calls the sound conversion routine */
		errorNumber = convertSounds(pc, psx, &pc_info, &psx_info,
		                            psxpath, sfxPath);
		if (errorNumber != ERROR_NONE)
		{
			PRINT_FAILED;
			goto end;
		}
		PRINT_DONE;
	}
	
	/* Converts Code Modules */
	if (notflag(FLAGS_SKIP_MODULES))
	{
		if (isflag(FLAGS_VERBOSE))
		{
			printf("Adding Code Modules... ");
			(void) fflush(stdout);
		}
		errorNumber = addModules(pc, &pc_info, psxpath, psx, &psx_info, flags);
		if (errorNumber != ERROR_NONE)
		{
			PRINT_FAILED;
			goto end;
		}
		PRINT_DONE;
	}
	
	/* Converts Textures */
	if (notflag(FLAGS_SKIP_TEXTURES))
	{
		/* Counts the number of misoriented textures */
		errorNumber = findMisorientedTextures(pc, &pc_info,
		                                      &numMisorientedTextures);
		if (errorNumber != ERROR_NONE)
		{
			goto end;
		}
		/* Allocates the list of misoriented textures */
		if (numMisorientedTextures > 0x0000)
		{
			misorientedTextures = calloc((size_t) numMisorientedTextures, 2);
			if (misorientedTextures == NULL)
			{
				errorNumber = ERROR_MEMORY;
				goto end;
			}
		}
		/* Converts the textures */
		errorNumber = convertAllTextures(pc, psx, &pc_info, &psx_info,
		                                 psxpath, flags, misorientedTextures,
		                                 &roomTextures, &objectTextures,
		                                 waterColour);
		if (errorNumber != ERROR_NONE)
		{
			goto end;
		}
	
		/* Adds support for Kanji */
		if (((flags & FLAGS_LANGUAGE) == FLAGS_LANGUAGE_JP_JP) ||
		    ((flags & FLAGS_LANGUAGE) == FLAGS_LANGUAGE_ASIA_JP))
		{
			if (isflag(FLAGS_VERBOSE))
			{
				printf("Adding Kanji... ");
				(void) fflush(stdout);
			}
			errorNumber = addKanji(psx, &psx_info);
			if (errorNumber != ERROR_NONE)
			{
				PRINT_FAILED;
				goto end;
			}
			PRINT_DONE;
		}
	
		/* Converts animated textures */
		if (isflag(FLAGS_VERBOSE))
		{
			printf("Converting Animated Textures... ");
			(void) fflush(stdout);
		}
		errorNumber = convertAnimatedTextures(pc, psx, &pc_info, &psx_info,
		                                      psxpath, roomTextures);
		if (errorNumber != ERROR_NONE)
		{
			PRINT_FAILED;
			goto end;
		}
		PRINT_DONE;
	}
	
	/* Converts Rooms */
	if (notflag(FLAGS_SKIP_ROOMS))
	{
		if (isflag(FLAGS_VERBOSE))
		{
			printf("Converting Rooms... ");
			(void) fflush(stdout);
		}
		
		/* Converts rooms */
		errorNumber = convertRooms(pc, psx, pc_info.p_NumRooms, &psx_info,
		                           psxpath, pc_info.v_NumRooms, pc_info.room,
		                           flags, roomTextures);
		if (errorNumber != ERROR_NONE)
		{
			PRINT_FAILED;
			goto end;
		}
		
		/* Removes the old OutsideRoomTable */
		errorNumber = removebytes(psx, psxpath, psx_info.p_OutRoomTableLen,
		                          psx_info.v_OutRoomTableLen);
		if (errorNumber != ERROR_NONE)
		{
			PRINT_FAILED;
			goto end;
		}
		level_info_sub(&psx_info, psx_info.p_OutRoomTableLen,
		               psx_info.v_OutRoomTableLen);
		
		/* Builds the OutsideRoomTable */
		errorNumber = buildOutsideTable(psx, pc_info.v_NumRooms,
		                                pc_info.room, &psx_info);
		if (errorNumber != ERROR_NONE)
		{
			PRINT_FAILED;
			goto end;
		}
		PRINT_DONE;
	}
	
	/* Converts FloorData */
	if (notflag(FLAGS_SKIP_FLOORDATA))
	{
		if (isflag(FLAGS_VERBOSE))
		{
			printf("Converting Floor Data... ");
			(void) fflush(stdout);
		}
		
		/* Removes old FloorData */
		freeuint321 = (psx_info.v_NumFloorData * 2);
		errorNumber = removebytes(psx, psxpath, psx_info.p_NumFloorData,
		                          freeuint321);
		if (errorNumber != ERROR_NONE)
		{
			PRINT_FAILED;
			goto end;
		}
		level_info_sub(&psx_info, psx_info.p_NumFloorData, freeuint321);
		
		/* Makes room for new FloorData */
		freeuint321 = (pc_info.v_NumFloorData * 2);
		errorNumber = insertbytes(psx, psx_info.p_NumFloorData, freeuint321);
		if (errorNumber != ERROR_NONE)
		{
			PRINT_FAILED;
			goto end;
		}
		level_info_add(&psx_info, psx_info.p_NumFloorData, freeuint321);
		freeuint321 += 4;
		
		/* Copies FloorData */
		errorNumber = copybytes(pc, psx, pc_info.p_NumFloorData,
		                        psx_info.p_NumFloorData, freeuint321);
		if (errorNumber != ERROR_NONE)
		{
			PRINT_FAILED;
			goto end;
		}
		psx_info.v_NumFloorData = pc_info.v_NumFloorData;
		PRINT_DONE;
	}
	
	/* Converts Mesh Data and Mesh Pointers */
	if (notflag(FLAGS_SKIP_MESHES))
	{
		if (isflag(FLAGS_VERBOSE))
		{
			printf("Converting Mesh Data... ");
			(void) fflush(stdout);
		}
		errorNumber = convertMeshPointers(pc, psx, psxpath, &pc_info, &psx_info,
		                                  flags, numMisorientedTextures,
		                                  misorientedTextures,
		                                  objectTextures);
		if (errorNumber != ERROR_NONE)
		{
			PRINT_FAILED;
			goto end;
		}
		PRINT_DONE;
	}
	
	/* Converts Animation Data */
	if (notflag(FLAGS_SKIP_ANIMATIONS))
	{
		if (isflag(FLAGS_VERBOSE))
		{
			printf("Converting Animations... ");
			(void) fflush(stdout);
		}
		
		/* Removes old animations */
		freeuint321 = (psx_info.v_NumAnimations * 32);
		errorNumber = removebytes(psx, psxpath, psx_info.p_NumAnimations,
		                          freeuint321);
		if (errorNumber != ERROR_NONE)
		{
			PRINT_FAILED;
			goto end;
		}
		level_info_sub(&psx_info, psx_info.p_NumAnimations, freeuint321);
		
		/* Makes room for new animations */
		freeuint321 = (pc_info.v_NumAnimations * 32);
		errorNumber = insertbytes(psx, psx_info.p_NumAnimations, freeuint321);
		if (errorNumber != ERROR_NONE)
		{
			PRINT_FAILED;
			goto end;
		}
		level_info_add(&psx_info, psx_info.p_NumAnimations, freeuint321);
		freeuint321 += 4;
		
		/* Copies animations */
		errorNumber = copybytes(pc, psx, pc_info.p_NumAnimations,
		                        psx_info.p_NumAnimations, freeuint321);
		if (errorNumber != ERROR_NONE)
		{
			PRINT_FAILED;
			goto end;
		}
		psx_info.v_NumAnimations = pc_info.v_NumAnimations;
		PRINT_DONE;
		
		if (isflag(FLAGS_VERBOSE))
		{
			printf("Converting State Changes... ");
			(void) fflush(stdout);
		}
		
		/* Removes old state changes */
		freeuint321 = (psx_info.v_NumStateChanges * 6);
		errorNumber = removebytes(psx, psxpath, psx_info.p_NumStateChanges,
		                          freeuint321);
		if (errorNumber != ERROR_NONE)
		{
			PRINT_FAILED;
			goto end;
		}
		level_info_sub(&psx_info, psx_info.p_NumStateChanges, freeuint321);
		
		/* Makes room for new state changes */
		freeuint321 = (pc_info.v_NumStateChanges * 6);
		errorNumber = insertbytes(psx, psx_info.p_NumStateChanges, freeuint321);
		if (errorNumber != ERROR_NONE)
		{
			PRINT_FAILED;
			goto end;
		}
		level_info_add(&psx_info, psx_info.p_NumStateChanges, freeuint321);
		freeuint321 += 4;
		
		/* Copies state changes */
		errorNumber = copybytes(pc, psx, pc_info.p_NumStateChanges,
		                        psx_info.p_NumStateChanges, freeuint321);
		if (errorNumber != ERROR_NONE)
		{
			PRINT_FAILED;
			goto end;
		}
		psx_info.v_NumStateChanges = pc_info.v_NumStateChanges;
		PRINT_DONE;
		
		if (isflag(FLAGS_VERBOSE))
		{
			printf("Converting Animation Dispatches... ");
			(void) fflush(stdout);
		}
		
		/* Removes old anim dispatches */
		freeuint321 = (psx_info.v_NumAnimDispatches * 8);
		errorNumber = removebytes(psx, psxpath, psx_info.p_NumAnimDispatches,
		                          freeuint321);
		if (errorNumber != ERROR_NONE)
		{
			PRINT_FAILED;
			goto end;
		}
		level_info_sub(&psx_info, psx_info.p_NumAnimDispatches, freeuint321);
		
		/* Makes room for new anim dispatches */
		freeuint321 = (pc_info.v_NumAnimDispatches * 8);
		errorNumber = insertbytes(psx, psx_info.p_NumAnimDispatches,
		                          freeuint321);
		if (errorNumber != ERROR_NONE)
		{
			PRINT_FAILED;
			goto end;
		}
		level_info_add(&psx_info, psx_info.p_NumAnimDispatches, freeuint321);
		freeuint321 += 4;
		
		/* Copies anim dispatches */
		errorNumber = copybytes(pc, psx, pc_info.p_NumAnimDispatches,
		                        psx_info.p_NumAnimDispatches, freeuint321);
		if (errorNumber != ERROR_NONE)
		{
			PRINT_FAILED;
			goto end;
		}
		psx_info.v_NumAnimDispatches = pc_info.v_NumAnimDispatches;
		PRINT_DONE;
		
		if (isflag(FLAGS_VERBOSE))
		{
			printf("Converting Animation Commands... ");
			(void) fflush(stdout);
		}
		
		/* Removes old anim commands */
		freeuint321 = (psx_info.v_NumAnimCommands * 2);
		errorNumber = removebytes(psx, psxpath, psx_info.p_NumAnimCommands,
		                          freeuint321);
		if (errorNumber != ERROR_NONE)
		{
			PRINT_FAILED;
			goto end;
		}
		level_info_sub(&psx_info, psx_info.p_NumAnimCommands, freeuint321);
		
		/* Makes room for new anim commands */
		freeuint321 = (pc_info.v_NumAnimCommands * 2);
		errorNumber = insertbytes(psx, psx_info.p_NumAnimCommands, freeuint321);
		if (errorNumber != ERROR_NONE)
		{
			PRINT_FAILED;
			goto end;
		}
		level_info_add(&psx_info, psx_info.p_NumAnimCommands, freeuint321);
		freeuint321 += 4;
		
		/* Copies anim commands */
		errorNumber = copybytes(pc, psx, pc_info.p_NumAnimCommands,
		                        psx_info.p_NumAnimCommands, freeuint321);
		if (errorNumber != ERROR_NONE)
		{
			PRINT_FAILED;
			goto end;
		}
		psx_info.v_NumAnimCommands = pc_info.v_NumAnimCommands;
		PRINT_DONE;
		
		if (isflag(FLAGS_VERBOSE))
		{
			printf("Converting Mesh Trees... ");
			(void) fflush(stdout);
		}
		
		/* Removes old mesh trees */
		freeuint321 = (psx_info.v_NumMeshTrees * 4);
		errorNumber = removebytes(psx, psxpath, psx_info.p_NumMeshTrees,
		                          freeuint321);
		if (errorNumber != ERROR_NONE)
		{
			PRINT_FAILED;
			goto end;
		}
		level_info_sub(&psx_info, psx_info.p_NumMeshTrees, freeuint321);
		
		/* Makes room for new mesh trees */
		freeuint321 = (pc_info.v_NumMeshTrees * 4);
		errorNumber = insertbytes(psx, psx_info.p_NumMeshTrees, freeuint321);
		if (errorNumber != ERROR_NONE)
		{
			PRINT_FAILED;
			goto end;
		}
		level_info_add(&psx_info, psx_info.p_NumMeshTrees, freeuint321);
		freeuint321 += 4;
		
		/* Copies mesh trees */
		errorNumber = copybytes(pc, psx, pc_info.p_NumMeshTrees,
		                        psx_info.p_NumMeshTrees, freeuint321);
		if (errorNumber != ERROR_NONE)
		{
			PRINT_FAILED;
			goto end;
		}
		psx_info.v_NumMeshTrees = pc_info.v_NumMeshTrees;
		PRINT_DONE;
		
		if (isflag(FLAGS_VERBOSE))
		{
			printf("Converting Frames... ");
			(void) fflush(stdout);
		}
		
		/* Removes old frames */
		freeuint321 = (psx_info.v_NumFrames * 2);
		errorNumber = removebytes(psx, psxpath, psx_info.p_NumFrames,
		                          freeuint321);
		if (errorNumber != ERROR_NONE)
		{
			PRINT_FAILED;
			goto end;
		}
		level_info_sub(&psx_info, psx_info.p_NumFrames, freeuint321);
		
		/* Makes room for new frames */
		freeuint321 = (pc_info.v_NumFrames * 2);
		errorNumber = insertbytes(psx, psx_info.p_NumFrames, freeuint321);
		if (errorNumber != ERROR_NONE)
		{
			PRINT_FAILED;
			goto end;
		}
		level_info_add(&psx_info, psx_info.p_NumFrames, freeuint321);
		freeuint321 += 4;
		
		/* Copies frames */
		errorNumber = copybytes(pc, psx, pc_info.p_NumFrames,
		                        psx_info.p_NumFrames, freeuint321);
		if (errorNumber != ERROR_NONE)
		{
			PRINT_FAILED;
			goto end;
		}
		psx_info.v_NumFrames = pc_info.v_NumFrames;
		PRINT_DONE;
	}
	
	/* Converts Moveables */
	if (notflag(FLAGS_SKIP_MOVEABLES))
	{
		if (isflag(FLAGS_VERBOSE))
		{
			printf("Converting Moveables... ");
			(void) fflush(stdout);
		}
		
		/* Removes old moveables */
		freeuint321 = (psx_info.v_NumMoveables * 20);
		errorNumber = removebytes(psx, psxpath, psx_info.p_NumMoveables,
		                          freeuint321);
		if (errorNumber != ERROR_NONE)
		{
			PRINT_FAILED;
			goto end;
		}
		level_info_sub(&psx_info, psx_info.p_NumMoveables, freeuint321);
		
		/* Makes room for new moveables */
		freeuint321 = (pc_info.v_NumMoveables * 20);
		errorNumber = insertbytes(psx, psx_info.p_NumMoveables, freeuint321);
		if (errorNumber != ERROR_NONE)
		{
			PRINT_FAILED;
			goto end;
		}
		level_info_add(&psx_info, psx_info.p_NumMoveables, freeuint321);
		
		/* Copies NumMoveables */
		freeuint322 = pc_info.v_NumMoveables;
#ifdef __BIG_ENDIAN__
		freeuint322 = reverse32(freeuint322);
#endif
		freeint1 = fseek(psx, (long int) psx_info.p_NumMoveables, SEEK_SET);
		if (freeint1 != 0)
		{
			errorNumber = ERROR_PSX_READ_FAILED;
			PRINT_FAILED;
			goto end;
		}
		freesizet1 = fwrite(&freeuint322, 1, 4, psx);
		if (freesizet1 != 4)
		{
			errorNumber = ERROR_PSX_WRITE_FAILED;
			PRINT_FAILED;
			goto end;
		}
		
		/* Copies moveables */
		freeuint161 = 0x834F; /* This value is appended to each moveable */
#ifdef __BIG_ENDIAN__
		freeuint161 = reverse16(freeuint161);
#endif
		freeuint322 = pc_info.p_NumMoveables + 4;
		freeuint323 = psx_info.p_NumMoveables + 4;
		for (freeuint321 = 0; freeuint321 < pc_info.v_NumMoveables;
		     ++freeuint321)
		{
			errorNumber = copybytes(pc, psx, freeuint322, freeuint323, 18);
			if (errorNumber != ERROR_NONE)
			{
				PRINT_FAILED;
				goto end;
			}
			freeint1 = fseek(psx, (long int) (freeuint323 + 18), SEEK_SET);
			if (freeint1 != 0)
			{
				errorNumber = ERROR_PSX_READ_FAILED;
				PRINT_FAILED;
				goto end;
			}
			freesizet1 = fwrite(&freeuint161, 1, 2, psx);
			if (freesizet1 != 2)
			{
				errorNumber = ERROR_PSX_WRITE_FAILED;
				PRINT_FAILED;
				goto end;
			}
			
			freeuint322 += 18;
			freeuint323 += 20;
		}
		psx_info.v_NumMoveables = pc_info.v_NumMoveables;
		PRINT_DONE;
	}
	
	/* Converts Static Meshes */
	if (notflag(FLAGS_SKIP_STATICMESHES))
	{
		if (isflag(FLAGS_VERBOSE))
		{
			printf("Converting Static Meshes... ");
			(void) fflush(stdout);
		}
		
		/* Removes old static meshes */
		freeuint321 = (psx_info.v_NumStaticMeshes * 32);
		errorNumber = removebytes(psx, psxpath, psx_info.p_NumStaticMeshes,
		                          freeuint321);
		if (errorNumber != ERROR_NONE)
		{
			PRINT_FAILED;
			goto end;
		}
		level_info_sub(&psx_info, psx_info.p_NumStaticMeshes, freeuint321);
		
		/* Makes room for new static meshes */
		freeuint321 = (pc_info.v_NumStaticMeshes * 32);
		errorNumber = insertbytes(psx, psx_info.p_NumStaticMeshes, freeuint321);
		if (errorNumber != ERROR_NONE)
		{
			PRINT_FAILED;
			goto end;
		}
		level_info_add(&psx_info, psx_info.p_NumStaticMeshes, freeuint321);
		freeuint321 += 4;
		
		/* Copies static meshes */
		errorNumber = copybytes(pc, psx, pc_info.p_NumStaticMeshes,
		                        psx_info.p_NumStaticMeshes, freeuint321);
		if (errorNumber != ERROR_NONE)
		{
			PRINT_FAILED;
			goto end;
		}
		psx_info.v_NumStaticMeshes = pc_info.v_NumStaticMeshes;
		PRINT_DONE;
	}
	
	/* Converts Cameras */
	if (notflag(FLAGS_SKIP_CAMERAS))
	{
		if (isflag(FLAGS_VERBOSE))
		{
			printf("Converting Cameras... ");
			(void) fflush(stdout);
		}
		
		/* Removes old cameras */
		freeuint321 = (psx_info.v_NumCameras * 16);
		errorNumber = removebytes(psx, psxpath, psx_info.p_NumCameras,
		                          freeuint321);
		if (errorNumber != ERROR_NONE)
		{
			PRINT_FAILED;
			goto end;
		}
		level_info_sub(&psx_info, psx_info.p_NumCameras, freeuint321);
		
		/* Makes room for new cameras */
		freeuint321 = (pc_info.v_NumCameras * 16);
		errorNumber = insertbytes(psx, psx_info.p_NumCameras, freeuint321);
		if (errorNumber != ERROR_NONE)
		{
			PRINT_FAILED;
			goto end;
		}
		level_info_add(&psx_info, psx_info.p_NumCameras, freeuint321);
		freeuint321 += 4;
		
		/* Copies cameras */
		errorNumber = copybytes(pc, psx, pc_info.p_NumCameras,
		                        psx_info.p_NumCameras, freeuint321);
		if (errorNumber != ERROR_NONE)
		{
			PRINT_FAILED;
			goto end;
		}
		psx_info.v_NumCameras = pc_info.v_NumCameras;
		PRINT_DONE;
	}
	
	/* Converts Sound Sources */
	if (notflag(FLAGS_SKIP_SOUNDSOURCES))
	{
		if (isflag(FLAGS_VERBOSE))
		{
			printf("Converting Sound Sources... ");
			(void) fflush(stdout);
		}
		
		/* Removes old sound sources */
		freeuint321 = (psx_info.v_NumSoundSources * 16);
		errorNumber = removebytes(psx, psxpath, psx_info.p_NumSoundSources,
		                          freeuint321);
		if (errorNumber != ERROR_NONE)
		{
			PRINT_FAILED;
			goto end;
		}
		level_info_sub(&psx_info, psx_info.p_NumSoundSources, freeuint321);
		
		/* Makes room for new sound sources */
		freeuint321 = (pc_info.v_NumSoundSources * 16);
		errorNumber = insertbytes(psx, psx_info.p_NumSoundSources, freeuint321);
		if (errorNumber != ERROR_NONE)
		{
			PRINT_FAILED;
			goto end;
		}
		level_info_add(&psx_info, psx_info.p_NumSoundSources, freeuint321);
		freeuint321 += 4;
		
		/* Copies sound sources */
		errorNumber = copybytes(pc, psx, pc_info.p_NumSoundSources,
		                        psx_info.p_NumSoundSources, freeuint321);
		if (errorNumber != ERROR_NONE)
		{
			PRINT_FAILED;
			goto end;
		}
		psx_info.v_NumSoundSources = pc_info.v_NumSoundSources;
		PRINT_DONE;
	}
	
	/* Converts Boxes, Zones and Overlaps */
	if (notflag(FLAGS_SKIP_BOXES))
	{
		if (isflag(FLAGS_VERBOSE))
		{
			printf("Converting Boxes, Zones and Overlaps... ");
			(void) fflush(stdout);
		}
		
		/* Removes old Boxes, Zones and Overlaps */
		freeuint321 = (psx_info.p_NumAnimatedTextures - psx_info.p_NumBoxes);
		--freeuint321;
		errorNumber = removebytes(psx, psxpath, psx_info.p_NumBoxes,
		                          freeuint321);
		if (errorNumber != ERROR_NONE)
		{
			PRINT_FAILED;
			goto end;
		}
		level_info_sub(&psx_info, psx_info.p_NumBoxes, freeuint321);
		
		/* Makes room for new Boxes, Zones and Overlaps */
		freeuint321 = (pc_info.p_NumAnimatedTextures - pc_info.p_NumBoxes);
		--freeuint321;
		errorNumber = insertbytes(psx, psx_info.p_NumBoxes, freeuint321);
		if (errorNumber != ERROR_NONE)
		{
			PRINT_FAILED;
			goto end;
		}
		level_info_add(&psx_info, psx_info.p_NumBoxes, freeuint321);
		
		/* Copies Boxes, Zones and Overlaps */
		++freeuint321;
		errorNumber = copybytes(pc, psx, pc_info.p_NumBoxes,
		                        psx_info.p_NumBoxes, freeuint321);
		if (errorNumber != ERROR_NONE)
		{
			PRINT_FAILED;
			goto end;
		}
		psx_info.v_NumBoxes = pc_info.v_NumBoxes;
		psx_info.v_NumOverlaps = pc_info.v_NumOverlaps;
		psx_info.p_NumOverlaps = (psx_info.p_NumBoxes + 4 +
		                          (psx_info.v_NumBoxes * 8));
		PRINT_DONE;
	}
	
	/* Converts Entities */
	if (notflag(FLAGS_SKIP_ENTITIES))
	{
		if (isflag(FLAGS_VERBOSE))
		{
			printf("Converting Entities... ");
			(void) fflush(stdout);
		}
		
		/* Removes old entities */
		freeuint321 = (psx_info.v_NumEntities * 24);
		errorNumber = removebytes(psx, psxpath,
		                          psx_info.p_NumEntities, freeuint321);
		if (errorNumber != ERROR_NONE)
		{
			PRINT_FAILED;
			goto end;
		}
		level_info_sub(&psx_info, psx_info.p_NumEntities, freeuint321);
		
		/* Makes room for new entities */
		freeuint321 = (pc_info.v_NumEntities * 24);
		errorNumber = insertbytes(psx, psx_info.p_NumEntities, freeuint321);
		if (errorNumber != ERROR_NONE)
		{
			PRINT_FAILED;
			goto end;
		}
		level_info_add(&psx_info, psx_info.p_NumEntities, freeuint321);
		freeuint321 += 4;
		
		/* Copies entities */
		errorNumber = copybytes(pc, psx, pc_info.p_NumEntities,
		                        psx_info.p_NumEntities, freeuint321);
		if (errorNumber != ERROR_NONE)
		{
			PRINT_FAILED;
			goto end;
		}
		psx_info.v_NumEntities = pc_info.v_NumEntities;
		PRINT_DONE;
	}
	
	/* Converts Cinematic Frames */
	if (notflag(FLAGS_SKIP_CINEMATICS))
	{
		if (isflag(FLAGS_VERBOSE))
		{
			printf("Converting Cinematic Frames... ");
			(void) fflush(stdout);
		}
		
		/* Removes old cinematic frames */
		freeuint321 = (psx_info.v_NumCinematicFrames * 16);
		errorNumber = removebytes(psx, psxpath,
		                          psx_info.p_NumCinematicFrames, freeuint321);
		if (errorNumber != ERROR_NONE)
		{
			PRINT_FAILED;
			goto end;
		}
		level_info_sub(&psx_info, psx_info.p_NumCinematicFrames, freeuint321);
		
		/* Makes room for new cinematic frames */
		freeuint321 = (pc_info.v_NumCinematicFrames * 16);
		errorNumber = insertbytes(psx, psx_info.p_NumCinematicFrames,
		                          freeuint321);
		if (errorNumber != ERROR_NONE)
		{
			PRINT_FAILED;
			goto end;
		}
		level_info_add(&psx_info, psx_info.p_NumCinematicFrames, freeuint321);
		freeuint321 += 0x00000002;
		
		/* Copies cinematic frames */
		errorNumber = copybytes(pc, psx, pc_info.p_NumCinematicFrames,
		                        psx_info.p_NumCinematicFrames, freeuint321);
		if (errorNumber != ERROR_NONE)
		{
			PRINT_FAILED;
			goto end;
		}
		psx_info.v_NumCinematicFrames = pc_info.v_NumCinematicFrames;
		PRINT_DONE;
	}
	
	/* Sets Sky Colour */
	if (isflag(FLAGS_SKY_COLOUR))
	{
		if (isflag(FLAGS_VERBOSE))
		{
			printf("Setting Sky Colour... ");
			(void) fflush(stdout);
		}
		
		/* Moves to the offset to write the sky colour */
		freelong1 = (long int) (psx_info.p_NumSoundDetails - 0x000002E8);
		freeint1 = fseek(psx, freelong1, SEEK_SET);
		if (freeint1 != 0)
		{
			errorNumber = ERROR_PSX_READ_FAILED;
			PRINT_FAILED;
			goto end;
		}
		
		/* Sets up the sky colour in memory */
#ifdef __BIG_ENDIAN__
		skyColour <<= 8;
#else
		skyColour = ((skyColour & 0x0000FF00) |
		             ((skyColour & 0x00FF0000) >> 16) |
		             ((skyColour & 0x000000FF) << 16));
#endif
		
		/* Writes the sky colour to PSX */
		freesizet1 = fwrite(&skyColour, 1, 4, psx);
		if (freesizet1 != 4)
		{
			errorNumber = ERROR_PSX_WRITE_FAILED;
			PRINT_FAILED;
			goto end;
		}
		
		PRINT_DONE;
	}
	
end:/* Closes files, frees memory and returns the function */
	if (pc != NULL)
	{
		freeint1 = fclose(pc);
		if (freeint1 != 0)
		{
			errorNumber = ERROR_PC_CLOSE_FAILED;
		}
	}
	if (psx != NULL)
	{
		freeint1 = fclose(psx);
		if (freeint1 != 0)
		{
			errorNumber = ERROR_PSX_CLOSE_FAILED;
		}
	}
	if (pc_info.room != NULL)
	{
		free(pc_info.room);
	}
	if (psx_info.room != NULL)
	{
		free(psx_info.room);
	}
	if (misorientedTextures != NULL)
	{
		free(misorientedTextures);
	}
	if (roomTextures != NULL)
	{
		free(roomTextures);
	}
	if (objectTextures != NULL)
	{
		free(objectTextures);
	}
	
	return errorNumber;
}

/*
 * Function that navigates through the PC-file reading offsets and values.
 * Parameters:
 * * pc = Pointer to the level file
 * * pc_info = Pointer to the level_info struct where the data is to be stored
 * Nota bene:
 * * Expects v_NumTexTiles,p_NumRooms,and v_Numrooms inside pc_info to be set
 * * and all room-related variables to be already allocated.
 */
static int tr3pc_navigate(FILE *pc, struct level_info *pc_info)
{
	/* Variable Declarations */
	INTU16 freeuint161; /* 16-bit unsigned integer for general use */
	int freeint1;       /* Integer for general use */
	size_t freesizet1;  /* size_t fo general use */
	
	/* Loops through rooms */
	freeint1 = fseek(pc, (long int) ((pc_info->p_NumRooms) + 2), SEEK_SET);
	if (freeint1 != 0)
	{
		return ERROR_PC_READ_FAILED;
	}
	for (freeuint161 = 0; freeuint161 < (pc_info->v_NumRooms); ++freeuint161)
	{
		/* Room Header */
		freesizet1 = fread(&(pc_info->room[freeuint161].x), 1, 4, pc);
		if (freesizet1 != 4)
		{
			return ERROR_PC_READ_FAILED;
		}
#ifdef __BIG_ENDIAN__
		pc_info->room[freeuint161].x = reverse32(pc_info->room[freeuint161].x);
#endif
		freesizet1 = fread(&(pc_info->room[freeuint161].z), 1, 4, pc);
		if (freesizet1 != 4)
		{
			return ERROR_PC_READ_FAILED;
		}
#ifdef __BIG_ENDIAN__
		pc_info->room[freeuint161].z = reverse32(pc_info->room[freeuint161].z);
#endif
		freesizet1 = fread(&(pc_info->room[freeuint161].yBottom), 1, 4, pc);
		if (freesizet1 != 4)
		{
			return ERROR_PC_READ_FAILED;
		}
#ifdef __BIG_ENDIAN__
		pc_info->room[freeuint161].yBottom =
			reverse32(pc_info->room[freeuint161].yBottom);
#endif
		freesizet1 = fread(&(pc_info->room[freeuint161].yTop), 1, 4, pc);
		if (freesizet1 != 4)
		{
			return ERROR_PC_READ_FAILED;
		}
#ifdef __BIG_ENDIAN__
		pc_info->room[freeuint161].yTop =
			reverse32(pc_info->room[freeuint161].yTop);
#endif
		/* NumVertices */
		freeint1 = fseek(pc, 4, SEEK_CUR);
		if (freeint1 != 0)
		{
			return ERROR_PC_READ_FAILED;
		}
		pc_info->room[freeuint161].p_NumVertices = ((INTU32) ftell(pc));
		freesizet1 = fread(&(pc_info->room[freeuint161].v_NumVertices),
		                   1, 2, pc);
		if (freesizet1 != 2)
		{
			return ERROR_PC_READ_FAILED;
		}
#ifdef __BIG_ENDIAN__
		pc_info->room[freeuint161].v_NumVertices =
			reverse16(pc_info->room[freeuint161].v_NumVertices);
#endif
		/* NumRectangles */
		pc_info->room[freeuint161].p_NumRectangles =
			(pc_info->room[freeuint161].p_NumVertices + 2 +
			 (pc_info->room[freeuint161].v_NumVertices * 12));
		freeint1 = fseek(pc, (long int)
		                 pc_info->room[freeuint161].p_NumRectangles,
		                 SEEK_SET);
		if (freeint1 != 0)
		{
			return ERROR_PC_READ_FAILED;
		}
		freesizet1 = fread(&(pc_info->room[freeuint161].v_NumRectangles),
		                   1, 2, pc);
		if (freesizet1 != 2)
		{
			return ERROR_PC_READ_FAILED;
		}
#ifdef __BIG_ENDIAN__
		pc_info->room[freeuint161].v_NumRectangles =
			reverse16(pc_info->room[freeuint161].v_NumRectangles);
#endif
		/* NumTriangles */
		pc_info->room[freeuint161].p_NumTriangles =
			(pc_info->room[freeuint161].p_NumRectangles + 2 +
			 (pc_info->room[freeuint161].v_NumRectangles * 10));
		freeint1 = fseek(pc, (long int)
		                 pc_info->room[freeuint161].p_NumTriangles, SEEK_SET);
		if (freeint1 != 0)
		{
			return ERROR_PC_READ_FAILED;
		}
		freesizet1 = fread(&(pc_info->room[freeuint161].v_NumTriangles),
		                   1, 2, pc);
		if (freesizet1 != 2)
		{
			return ERROR_PC_READ_FAILED;
		}
#ifdef __BIG_ENDIAN__
		pc_info->room[freeuint161].v_NumTriangles =
			reverse16(pc_info->room[freeuint161].v_NumTriangles);
#endif
		/* NumSprites */
		pc_info->room[freeuint161].p_NumSprites =
		    (pc_info->room[freeuint161].p_NumTriangles + 2 +
		     (pc_info->room[freeuint161].v_NumTriangles * 8));
		freeint1 = fseek(pc, (long int)
		                 pc_info->room[freeuint161].p_NumSprites,
		                 SEEK_SET);
		if (freeint1 != 0)
		{
			return ERROR_PC_READ_FAILED;
		}
		freesizet1 = fread(&(pc_info->room[freeuint161].v_NumSprites),
		                   1, 2, pc);
		if (freesizet1 != 2)
		{
			return ERROR_PC_READ_FAILED;
		}
#ifdef __BIG_ENDIAN__
		pc_info->room[freeuint161].v_NumSprites =
			reverse16(pc_info->room[freeuint161].v_NumSprites);
#endif
		/* NumDoors */
		pc_info->room[freeuint161].p_NumDoors =
			(pc_info->room[freeuint161].p_NumSprites + 2 +
			 (pc_info->room[freeuint161].v_NumSprites * 4));
		freeint1 = fseek(pc, (long int)
		                 pc_info->room[freeuint161].p_NumDoors,
		                 SEEK_SET);
		if (freeint1 != 0)
		{
			return ERROR_PC_READ_FAILED;
		}
		freesizet1 = fread(&(pc_info->room[freeuint161].v_NumDoors), 1, 2, pc);
		if (freesizet1 != 2)
		{
			return ERROR_PC_READ_FAILED;
		}
#ifdef __BIG_ENDIAN__
		pc_info->room[freeuint161].v_NumDoors =
			reverse16(pc_info->room[freeuint161].v_NumDoors);
#endif
		/* NumZSectors */
		pc_info->room[freeuint161].p_NumZSectors =
			(pc_info->room[freeuint161].p_NumDoors + 2 +
			 (pc_info->room[freeuint161].v_NumDoors * 32));
		freeint1 = fseek(pc, (long int)
		                 pc_info->room[freeuint161].p_NumZSectors,
		                 SEEK_SET);
		if (freeint1 != 0)
		{
			return ERROR_PC_READ_FAILED;
		}
		freesizet1 = fread(&(pc_info->room[freeuint161].v_NumZSectors),
		                   1, 2, pc);
		if (freesizet1 != 2)
		{
			return ERROR_PC_READ_FAILED;
		}
#ifdef __BIG_ENDIAN__
		pc_info->room[freeuint161].v_NumZSectors =
			reverse16(pc_info->room[freeuint161].v_NumZSectors);
#endif
		/* NumXSectors */
		pc_info->room[freeuint161].p_NumXSectors =
			(pc_info->room[freeuint161].p_NumZSectors + 2);
		freeint1 = fseek(pc, (long int)
		                 pc_info->room[freeuint161].p_NumXSectors,
		                 SEEK_SET);
		if (freeint1 != 0)
		{
			return ERROR_PC_READ_FAILED;
		}
		freesizet1 = fread(&(pc_info->room[freeuint161].v_NumXSectors),
		                   1, 2, pc);
		if (freesizet1 != 2)
		{
			return ERROR_PC_READ_FAILED;
		}
#ifdef __BIG_ENDIAN__
		pc_info->room[freeuint161].v_NumXSectors =
			reverse16(pc_info->room[freeuint161].v_NumXSectors);
#endif
		/* NumLights */
		pc_info->room[freeuint161].p_NumLights =
			(pc_info->room[freeuint161].p_NumXSectors + 6 +
			 (pc_info->room[freeuint161].v_NumZSectors *
			  pc_info->room[freeuint161].v_NumXSectors * 8));
		freeint1 = fseek(pc, (long int)
		                 pc_info->room[freeuint161].p_NumLights,
		                 SEEK_SET);
		if (freeint1 != 0)
		{
			return ERROR_PC_READ_FAILED;
		}
		freesizet1 = fread(&(pc_info->room[freeuint161].v_NumLights), 1, 2, pc);
		if (freesizet1 != 2)
		{
			return ERROR_PC_READ_FAILED;
		}
#ifdef __BIG_ENDIAN__
		pc_info->room[freeuint161].v_NumLights =
			reverse16(pc_info->room[freeuint161].v_NumLights);
#endif
		/* NumStaticMeshes */
		pc_info->room[freeuint161].p_NumStaticMeshes =
			(pc_info->room[freeuint161].p_NumLights + 2 +
			 (pc_info->room[freeuint161].v_NumLights * 24));
		freeint1 = fseek(pc, (long int)
		                 pc_info->room[freeuint161].p_NumStaticMeshes,
		                 SEEK_SET);
		if (freeint1 != 0)
		{
			return ERROR_PC_READ_FAILED;
		}
		freesizet1 = fread(&(pc_info->room[freeuint161].v_NumStaticMeshes),
		                   1, 2, pc);
		if (freesizet1 != 2)
		{
			return ERROR_PC_READ_FAILED;
		}
#ifdef __BIG_ENDIAN__
		pc_info->room[freeuint161].v_NumStaticMeshes =
			reverse16(pc_info->room[freeuint161].v_NumStaticMeshes);
#endif
		/* AlternateRoom */
		pc_info->room[freeuint161].p_AlternateRoom =
			(pc_info->room[freeuint161].p_NumStaticMeshes + 2 +
			 (pc_info->room[freeuint161].v_NumStaticMeshes * 20));
		freeint1 = fseek(pc, (long int)
		                 pc_info->room[freeuint161].p_AlternateRoom + 2,
		                 SEEK_SET);
		if (freeint1 != 0)
		{
			return ERROR_PC_READ_FAILED;
		}
		freesizet1 = fread(&(pc_info->room[freeuint161].flags), 1, 2, pc);
		if (freesizet1 != (size_t) 2)
		{
			return ERROR_PC_READ_FAILED;
		}
#ifdef __BIG_ENDIAN__
		pc_info->room[freeuint161].flags =
			reverse16(pc_info->room[freeuint161].flags);
#endif
		freeint1 = fseek(pc, (long int)
		                 pc_info->room[freeuint161].p_AlternateRoom + 7,
		                 SEEK_SET);
		if (freeint1 != 0)
		{
			return ERROR_PC_READ_FAILED;
		}
	}
	/* NumFloorData */
	pc_info->p_NumFloorData = ((INTU32) ftell(pc));
	freesizet1 = fread(&(pc_info->v_NumFloorData), 1, 4, pc);
	if (freesizet1 != 4)
	{
		return ERROR_PC_READ_FAILED;
	}
#ifdef __BIG_ENDIAN__
	pc_info->v_NumFloorData = reverse32(pc_info->v_NumFloorData);
#endif
	/* NumMeshData */
	pc_info->p_NumMeshData = (pc_info->p_NumFloorData + 4 +
	                          (pc_info->v_NumFloorData * 2));
	freeint1 = fseek(pc, (long int) pc_info->p_NumMeshData, SEEK_SET);
	if (freeint1 != 0)
	{
		return ERROR_PC_READ_FAILED;
	}
	freesizet1 = fread(&(pc_info->v_NumMeshData), 1, 4, pc);
	if (freesizet1 != 4)
	{
		return ERROR_PC_READ_FAILED;
	}
#ifdef __BIG_ENDIAN__
	pc_info->v_NumMeshData = reverse32(pc_info->v_NumMeshData);
#endif
	/* NumMeshPointers */
	pc_info->p_NumMeshPointers = (pc_info->p_NumMeshData + 4 +
	                              (pc_info->v_NumMeshData * 2));
	freeint1 = fseek(pc, (long int) pc_info->p_NumMeshPointers, SEEK_SET);
	if (freeint1 != 0)
	{
		return ERROR_PC_READ_FAILED;
	}
	freesizet1 = fread(&(pc_info->v_NumMeshPointers), 1, 4, pc);
	if (freesizet1 != 4)
	{
		return ERROR_PC_READ_FAILED;
	}
#ifdef __BIG_ENDIAN__
	pc_info->v_NumMeshPointers = reverse32(pc_info->v_NumMeshPointers);
#endif
	/* NumAnimations */
	pc_info->p_NumAnimations = (pc_info->p_NumMeshPointers + 4 +
	                            (pc_info->v_NumMeshPointers * 4));
	freeint1 = fseek(pc, (long int) pc_info->p_NumAnimations, SEEK_SET);
	if (freeint1 != 0)
	{
		return ERROR_PC_READ_FAILED;
	}
	freesizet1 = fread(&(pc_info->v_NumAnimations), 1, 4, pc);
	if (freesizet1 != 4)
	{
		return ERROR_PC_READ_FAILED;
	}
#ifdef __BIG_ENDIAN__
	pc_info->v_NumAnimations = reverse32(pc_info->v_NumAnimations);
#endif
	/* NumStateChanges */
	pc_info->p_NumStateChanges = (pc_info->p_NumAnimations + 4 +
	                              (pc_info->v_NumAnimations * 32));
	freeint1 = fseek(pc, (long int) pc_info->p_NumStateChanges, SEEK_SET);
	if (freeint1 != 0)
	{
		return ERROR_PC_READ_FAILED;
	}
	freesizet1 = fread(&(pc_info->v_NumStateChanges), 1, 4, pc);
	if (freesizet1 != 4)
	{
		return ERROR_PC_READ_FAILED;
	}
#ifdef __BIG_ENDIAN__
	pc_info->v_NumStateChanges = reverse32(pc_info->v_NumStateChanges);
#endif
	/* NumAnimDispatches */
	pc_info->p_NumAnimDispatches = (pc_info->p_NumStateChanges + 4 +
	                                (pc_info->v_NumStateChanges * 6));
	freeint1 = fseek(pc, (long int) pc_info->p_NumAnimDispatches, SEEK_SET);
	if (freeint1 != 0)
	{
		return ERROR_PC_READ_FAILED;
	}
	freesizet1 = fread(&(pc_info->v_NumAnimDispatches), 1, 4, pc);
	if (freesizet1 != 4)
	{
		return ERROR_PC_READ_FAILED;
	}
#ifdef __BIG_ENDIAN__
	pc_info->v_NumAnimDispatches = reverse32(pc_info->v_NumAnimDispatches);
#endif
	/* NumAnimCommands */
	pc_info->p_NumAnimCommands = (pc_info->p_NumAnimDispatches + 4 +
	                              (pc_info->v_NumAnimDispatches * 8));
	freeint1 = fseek(pc, (long int) pc_info->p_NumAnimCommands, SEEK_SET);
	if (freeint1 != 0)
	{
		return ERROR_PC_READ_FAILED;
	}
	freesizet1 = fread(&(pc_info->v_NumAnimCommands), 1, 4, pc);
	if (freesizet1 != 4)
	{
		return ERROR_PC_READ_FAILED;
	}
#ifdef __BIG_ENDIAN__
	pc_info->v_NumAnimCommands = reverse32(pc_info->v_NumAnimCommands);
#endif
	/* NumMeshTrees */
	pc_info->p_NumMeshTrees = (pc_info->p_NumAnimCommands + 4 +
	                           (pc_info->v_NumAnimCommands * 2));
	freeint1 = fseek(pc, (long int) pc_info->p_NumMeshTrees, SEEK_SET);
	if (freeint1 != 0)
	{
		return ERROR_PC_READ_FAILED;
	}
	freesizet1 = fread(&(pc_info->v_NumMeshTrees), 1, 4, pc);
	if (freesizet1 != 4)
	{
		return ERROR_PC_READ_FAILED;
	}
#ifdef __BIG_ENDIAN__
	pc_info->v_NumMeshTrees = reverse32(pc_info->v_NumMeshTrees);
#endif
	/* NumFrames */
	pc_info->p_NumFrames = (pc_info->p_NumMeshTrees + 4 +
	                        (pc_info->v_NumMeshTrees * 4));
	freeint1 = fseek(pc, (long int) pc_info->p_NumFrames, SEEK_SET);
	if (freeint1 != 0)
	{
		return ERROR_PC_READ_FAILED;
	}
	freesizet1 = fread(&(pc_info->v_NumFrames), 1, 4, pc);
	if (freesizet1 != 4)
	{
		return ERROR_PC_READ_FAILED;
	}
#ifdef __BIG_ENDIAN__
	pc_info->v_NumFrames = reverse32(pc_info->v_NumFrames);
#endif
	/* NumMoveables */
	pc_info->p_NumMoveables = (pc_info->p_NumFrames +
	                           4 + (pc_info->v_NumFrames * 2));
	freeint1 = fseek(pc, (long int) pc_info->p_NumMoveables, SEEK_SET);
	if (freeint1 != 0)
	{
		return ERROR_PC_READ_FAILED;
	}
	freesizet1 = fread(&(pc_info->v_NumMoveables), 1, 4, pc);
	if (freesizet1 != 4)
	{
		return ERROR_PC_READ_FAILED;
	}
#ifdef __BIG_ENDIAN__
	pc_info->v_NumMoveables = reverse32(pc_info->v_NumMoveables);
#endif
	/* NumStaticMeshes */
	pc_info->p_NumStaticMeshes = (pc_info->p_NumMoveables + 4 +
	                              (pc_info->v_NumMoveables * 18));
	freeint1 = fseek(pc, (long int) pc_info->p_NumStaticMeshes, SEEK_SET);
	if (freeint1 != 0)
	{
		return ERROR_PC_READ_FAILED;
	}
	freesizet1 = fread(&(pc_info->v_NumStaticMeshes), 1, 4, pc);
	if (freesizet1 != 4)
	{
		return ERROR_PC_READ_FAILED;
	}
#ifdef __BIG_ENDIAN__
	pc_info->v_NumStaticMeshes = reverse32(pc_info->v_NumStaticMeshes);
#endif
	/* NumSpriteTextures */
	pc_info->p_NumSpriteTextures = (pc_info->p_NumStaticMeshes + 4 +
	                                (pc_info->v_NumStaticMeshes * 32));
	freeint1 = fseek(pc, (long int) pc_info->p_NumSpriteTextures, SEEK_SET);
	if (freeint1 != 0)
	{
		return ERROR_PC_READ_FAILED;
	}
	freesizet1 = fread(&(pc_info->v_NumSpriteTextures), 1, 4, pc);
	if (freesizet1 != 4)
	{
		return ERROR_PC_READ_FAILED;
	}
#ifdef __BIG_ENDIAN__
	pc_info->v_NumSpriteTextures = reverse32(pc_info->v_NumSpriteTextures);
#endif
	/* NumSpriteSequences */
	pc_info->p_NumSpriteSequences = (pc_info->p_NumSpriteTextures + 4 +
	                                 (pc_info->v_NumSpriteTextures * 16));
	freeint1 = fseek(pc, (long int) pc_info->p_NumSpriteSequences, SEEK_SET);
	if (freeint1 != 0)
	{
		return ERROR_PC_READ_FAILED;
	}
	freesizet1 = fread(&(pc_info->v_NumSpriteSequences), 1, 4, pc);
	if (freesizet1 != 4)
	{
		return ERROR_PC_READ_FAILED;
	}
#ifdef __BIG_ENDIAN__
	pc_info->v_NumSpriteSequences = reverse32(pc_info->v_NumSpriteSequences);
#endif
	/* NumCameras */
	pc_info->p_NumCameras = (pc_info->p_NumSpriteSequences + 4 +
	                         (pc_info->v_NumSpriteSequences * 8));
	freeint1 = fseek(pc, (long int) pc_info->p_NumCameras, SEEK_SET);
	if (freeint1 != 0)
	{
		return ERROR_PC_READ_FAILED;
	}
	freesizet1 = fread(&(pc_info->v_NumCameras), 1, 4, pc);
	if (freesizet1 != 4)
	{
		return ERROR_PC_READ_FAILED;
	}
#ifdef __BIG_ENDIAN__
	pc_info->v_NumCameras = reverse32(pc_info->v_NumCameras);
#endif
	/* NumSoundSources */
	pc_info->p_NumSoundSources = (pc_info->p_NumCameras + 4 +
	                              (pc_info->v_NumCameras * 16));
	freeint1 = fseek(pc, (long int) pc_info->p_NumSoundSources, SEEK_SET);
	if (freeint1 != 0)
	{
		return ERROR_PC_READ_FAILED;
	}
	freesizet1 = fread(&(pc_info->v_NumSoundSources), 1, 4, pc);
	if (freesizet1 != 4)
	{
		return ERROR_PC_READ_FAILED;
	}
#ifdef __BIG_ENDIAN__
	pc_info->v_NumSoundSources = reverse32(pc_info->v_NumSoundSources);
#endif
	/* NumBoxes */
	pc_info->p_NumBoxes = (pc_info->p_NumSoundSources + 4 +
	                       (pc_info->v_NumSoundSources * 16));
	freeint1 = fseek(pc, (long int) pc_info->p_NumBoxes, SEEK_SET);
	if (freeint1 != 0)
	{
		return ERROR_PC_READ_FAILED;
	}
	freesizet1 = fread(&(pc_info->v_NumBoxes), 1, 4, pc);
	if (freesizet1 != 4)
	{
		return ERROR_PC_READ_FAILED;
	}
#ifdef __BIG_ENDIAN__
	pc_info->v_NumBoxes = reverse32(pc_info->v_NumBoxes);
#endif
	/* NumOverlaps */
	pc_info->p_NumOverlaps = (pc_info->p_NumBoxes +
	                          4 + (pc_info->v_NumBoxes * 8));
	freeint1 = fseek(pc, (long int) pc_info->p_NumOverlaps, SEEK_SET);
	if (freeint1 != 0)
	{
		return ERROR_PC_READ_FAILED;
	}
	freesizet1 = fread(&(pc_info->v_NumOverlaps), 1, 4, pc);
	if (freesizet1 != 4)
	{
		return ERROR_PC_READ_FAILED;
	}
#ifdef __BIG_ENDIAN__
	pc_info->v_NumOverlaps = reverse32(pc_info->v_NumOverlaps);
#endif
	/* NumAnimatedTextures */
	pc_info->p_Zones = (pc_info->p_NumOverlaps +
	                    4 + (pc_info->v_NumOverlaps * 2));
	pc_info->p_NumAnimatedTextures = (pc_info->p_Zones +
	                                  (pc_info->v_NumBoxes * 20));
	freeint1 = fseek(pc, (long int) pc_info->p_NumAnimatedTextures, SEEK_SET);
	if (freeint1 != 0)
	{
		return ERROR_PC_READ_FAILED;
	}
	freesizet1 = fread(&(pc_info->v_NumAnimatedTextures), 1, 4, pc);
	if (freesizet1 != 4)
	{
		return ERROR_PC_READ_FAILED;
	}
#ifdef __BIG_ENDIAN__
	pc_info->v_NumAnimatedTextures = reverse32(pc_info->v_NumAnimatedTextures);
#endif
	/* NumObjectTextures */
	pc_info->p_NumObjectTextures = (pc_info->p_NumAnimatedTextures + 4 +
	                                (pc_info->v_NumAnimatedTextures * 2));
	freeint1 = fseek(pc, (long int) pc_info->p_NumObjectTextures, SEEK_SET);
	if (freeint1 != 0)
	{
		return ERROR_PC_READ_FAILED;
	}
	freesizet1 = fread(&(pc_info->v_NumObjectTextures), 1, 4, pc);
	if (freesizet1 != 4)
	{
		return ERROR_PC_READ_FAILED;
	}
#ifdef __BIG_ENDIAN__
	pc_info->v_NumObjectTextures = reverse32(pc_info->v_NumObjectTextures);
#endif
	/* NumEntities */
	pc_info->p_NumEntities = (pc_info->p_NumObjectTextures + 4 +
	                          (pc_info->v_NumObjectTextures * 20));
	freeint1 = fseek(pc, (long int) pc_info->p_NumEntities, SEEK_SET);
	if (freeint1 != 0)
	{
		return ERROR_PC_READ_FAILED;
	}
	freesizet1 = fread(&(pc_info->v_NumEntities), 1, 4, pc);
	if (freesizet1 != 4)
	{
		return ERROR_PC_READ_FAILED;
	}
#ifdef __BIG_ENDIAN__
	pc_info->v_NumEntities = reverse32(v_NumEntities->v_NumEntities);
#endif
	/* NumCinematicFrames */
	pc_info->p_NumCinematicFrames = (pc_info->p_NumEntities + 8196 +
	                                 (pc_info->v_NumEntities * 24));
	freeint1 = fseek(pc, (long int) pc_info->p_NumCinematicFrames, SEEK_SET);
	if (freeint1 != 0)
	{
		return ERROR_PC_READ_FAILED;
	}
	freesizet1 = fread(&(pc_info->v_NumCinematicFrames), 1, 2, pc);
	if (freesizet1 != 2)
	{
		return ERROR_PC_READ_FAILED;
	}
#ifdef __BIG_ENDIAN__
	pc_info->v_NumCinematicFrames = reverse16(pc_info->v_NumCinematicFrames);
#endif
	/* NumDemoData */
	pc_info->p_NumDemoData = (pc_info->p_NumCinematicFrames + 2 +
	                          (pc_info->v_NumCinematicFrames * 16));
	freeint1 = fseek(pc, (long int) pc_info->p_NumDemoData, SEEK_SET);
	if (freeint1 != 0)
	{
		return ERROR_PC_READ_FAILED;
	}
	freesizet1 = fread(&freeuint161, 1, 2, pc);
	if (freesizet1 != 2)
	{
		return ERROR_PC_READ_FAILED;
	}
#ifdef __BIG_ENDIAN__
	freeuint161 = reverse16(freeuint161);
#endif
	pc_info->v_NumDemoData = (INTU32) freeuint161;
	/* NumSoundDetails */
	pc_info->p_NumSoundDetails = (pc_info->p_NumDemoData + 742 +
	                              pc_info->v_NumDemoData);
	freeint1 = fseek(pc, (long int) pc_info->p_NumSoundDetails, SEEK_SET);
	if (freeint1 != 0)
	{
		return ERROR_PC_READ_FAILED;
	}
	freesizet1 = fread(&(pc_info->v_NumSoundDetails), 1, 4, pc);
	if (freesizet1 != 4)
	{
		return ERROR_PC_READ_FAILED;
	}
#ifdef __BIG_ENDIAN__
	pc_info->v_NumSoundDetails = reverse32(pc_info->v_NumSoundDetails);
#endif
	/* NumSampleIndices */
	pc_info->p_NumSampleIndices = (pc_info->p_NumSoundDetails + 4 +
	                               (pc_info->v_NumSoundDetails * 8));
	freeint1 = fseek(pc, (long int) pc_info->p_NumSampleIndices, SEEK_SET);
	if (freeint1 != 0)
	{
		return ERROR_PC_READ_FAILED;
	}
	freesizet1 = fread(&(pc_info->v_NumSampleIndices), 1, 4, pc);
	if (freesizet1 != 4)
	{
		return ERROR_PC_READ_FAILED;
	}
#ifdef __BIG_ENDIAN__
	pc_info->v_NumSampleIndices = reverse32(pc_info->v_NumSampleIndices);
#endif

	return ERROR_NONE;
}

/*
 * Function that navigates through the PSX-file reading offsets and values.
 * Parameters:
 * * psx = Pointer to the level file
 * * psx_info = Pointer to the level_info struct where the data is to be stored
 * * flags = Bit-mask of command-line flags
 * Nota bene:
 * * Expects all room-related variables to be already allocated.
 */
static int tr3psx_navigate(FILE *psx, struct level_info *psx_info, INTU32 flags)
{
	/* Variable Declarations */
	INTU32 freeuint321, freeuint322; /* 32-bit unsigned ints for general use */
	INTU16 freeuint161, freeuint162,
	       freeuint163;              /* 16-bit unsigned ints for general use */
	int freeint1;                    /* Integer for general use */
	size_t freesizet1;               /* size_t fo general use */
	
	/* NumSampleIndices */
	psx_info->p_NumSampleIndices = 0x00000004;
	freeint1 = fseek(psx, (long int) psx_info->p_NumSampleIndices, SEEK_SET);
	if (freeint1 != 0)
	{
		return ERROR_PSX_READ_FAILED;
	}
	freesizet1 = fread(&psx_info->v_NumSampleIndices, 1, 4, psx);
	if (freesizet1 != 4)
	{
		return ERROR_PSX_READ_FAILED;
	}
#ifdef __BIG_ENDIAN__
	psx_info->v_NumSampleIndices = reverse32(psx_info->v_NumSampleIndices);
#endif
	/* NumSamples */
	psx_info->p_NumSamples = (psx_info->p_NumSampleIndices + 0x00000004 +
	                          (psx_info->v_NumSampleIndices * 0x00000004));
	freeint1 = fseek(psx, (long int) psx_info->p_NumSamples, SEEK_SET);
	if (freeint1 != 0)
	{
		return ERROR_PSX_READ_FAILED;
	}
	freesizet1 = fread(&psx_info->v_NumSamples, 1, 4, psx);
	if (freesizet1 != 4)
	{
		return ERROR_PSX_READ_FAILED;
	}
#ifdef __BIG_ENDIAN__
	psx_info->v_NumSamples = reverse32(psx_info->v_NumSamples);
#endif
	/* First Code Module */
	psx_info->p_FirstCodeModule = (psx_info->p_NumSamples + 0x00000004 +
	                               psx_info->v_NumSamples);
	/* NumRooms */
	freeuint321 = psx_info->p_FirstCodeModule;
	for (freeuint161 = 0x0000; freeuint161 < 0x000D; ++freeuint161)
	{
		freeint1 = fseek(psx, (long int) freeuint321, SEEK_SET);
		if (freeint1 != 0)
		{
			return ERROR_PSX_READ_FAILED;
		}
		freesizet1 = fread(&freeuint322, 1, 4, psx);
		if (freesizet1 != 4)
		{
			return ERROR_PSX_READ_FAILED;
		}
#ifdef __BIG_ENDIAN__
		freeuint322 = reverse32(freeuint322);
#endif
		if (freeuint322 != 0x00000000)
		{
			freeuint321 += (freeuint322 + 0x00000004);
			freeint1 = fseek(psx, (long int) freeuint321, SEEK_SET);
			if (freeint1 != 0)
			{
				return ERROR_PSX_READ_FAILED;
			}
			freesizet1 = fread(&freeuint322, 1, 4, psx);
			if (freesizet1 != 4)
			{
				return ERROR_PSX_READ_FAILED;
			}
#ifdef __BIG_ENDIAN__
			freeuint322 = reverse32(freeuint322);
#endif
			freeuint321 += (freeuint322 + 0x00000004);
		}
		else
		{
			freeuint321 += 0x00000004;
		}
	}
	psx_info->p_NumRooms = freeuint321;
	/* Reads the value of NumRooms */
	freeint1 = fseek(psx, (long int) psx_info->p_NumRooms, SEEK_SET);
	if (freeint1 != 0)
	{
		return ERROR_PSX_READ_FAILED;
	}
	freesizet1 = fread(&(psx_info->v_NumRooms), 1, 2, psx);
	if (freesizet1 != 2)
	{
		return ERROR_PSX_READ_FAILED;
	}
#ifdef __BIG_ENDIAN__
	psx_info->v_NumRooms = reverse16(psx_info->v_NumRooms);
#endif
	
	/* Loops through rooms */
	freeint1 = fseek(psx, (long int) ((psx_info->p_NumRooms) + 2), SEEK_SET);
	if (freeint1 != 0)
	{
		return ERROR_PSX_READ_FAILED;
	}
	for (freeuint161 = 0; freeuint161 < (psx_info->v_NumRooms); ++freeuint161)
	{
		/* NumDoors */
		freeint1 = fseek(psx, 16, SEEK_CUR);
		if (freeint1 != 0)
		{
			return ERROR_PSX_READ_FAILED;
		}
		freesizet1 = fread(&freeuint321, 1, 4, psx);
		if (freesizet1 != 4)
		{
			return ERROR_PSX_READ_FAILED;
		}
#ifdef __BIG_ENDIAN__
		freeuint321 = reverse32(freeuint321);
#endif
		freeuint322 = (INTU32) (ftell(psx) + (freeuint321 * 2));
		freeint1 = fseek(psx, (long int) freeuint322, SEEK_SET);
		if (freeint1 != 0)
		{
			return ERROR_PSX_READ_FAILED;
		}
		freesizet1 = fread(&freeuint162, 1, 2, psx);
		if (freesizet1 != 2)
		{
			return ERROR_PSX_READ_FAILED;
		}
#ifdef __BIG_ENDIAN__
		freeuint162 = reverse16(freeuint162);
#endif
		/* NumZSectors */
		freeuint322 += (2 + (freeuint162 * 32));
		freeint1 = fseek(psx, (long int) freeuint322, SEEK_SET);
		if (freeint1 != 0)
		{
			return ERROR_PSX_READ_FAILED;
		}
		freesizet1 = fread(&freeuint162, 1, 2, psx);
		if (freesizet1 != 2)
		{
			return ERROR_PSX_READ_FAILED;
		}
		freesizet1 = fread(&freeuint163, 1, 2, psx);
		if (freesizet1 != 2)
		{
			return ERROR_PSX_READ_FAILED;
		}
#ifdef __BIG_ENDIAN__
		freeuint162 = reverse16(freeuint162);
		freeuint163 = reverse16(freeuint163);
#endif
		/* NumLights */
		freeuint322 += (8 + (freeuint162 * freeuint163 * 8));
		freeint1 = fseek(psx, (long int) freeuint322, SEEK_SET);
		if (freeint1 != 0)
		{
			return ERROR_PSX_READ_FAILED;
		}
		freesizet1 = fread(&freeuint162, 1, 2, psx);
		if (freesizet1 != 2)
		{
			return ERROR_PSX_READ_FAILED;
		}
#ifdef __BIG_ENDIAN__
		freeuint162 = reverse16(freeuint162);
#endif
		/* NumStaticMeshes */
		freeuint322 += 2 + (freeuint162 * 24);
		freeint1 = fseek(psx, (long int) freeuint322, SEEK_SET);
		if (freeint1 != 0)
		{
			return ERROR_PSX_READ_FAILED;
		}
		freesizet1 = fread(&freeuint162, 1, 2, psx);
		if (freesizet1 != 2)
		{
			return ERROR_PSX_READ_FAILED;
		}
#ifdef __BIG_ENDIAN__
		freeuint162 = reverse16(freeuint162);
#endif
		/* Next Room */
		freeuint322 += 9 + (freeuint162 * 20);
		freeint1 = fseek(psx, (long int) freeuint322, SEEK_SET);
		if (freeint1 != 0)
		{
			return ERROR_PSX_READ_FAILED;
		}
	}
	/* NumFloorData */
	psx_info->p_NumFloorData = ((INTU32) ftell(psx));
	freesizet1 = fread(&(psx_info->v_NumFloorData), 1, 4, psx);
	if (freesizet1 != 4)
	{
		return ERROR_PSX_READ_FAILED;
	}
#ifdef __BIG_ENDIAN__
	psx_info->v_NumFloorData = reverse32(psx_info->v_NumFloorData);
#endif
	/* OutRoomTableLen */
	psx_info->p_OutRoomTableLen = (psx_info->p_NumFloorData + 1462 +
	                               (psx_info->v_NumFloorData * 2));
	freeint1 = fseek(psx, (long int) psx_info->p_OutRoomTableLen, SEEK_SET);
	if (freeint1 != 0)
	{
		return ERROR_PSX_READ_FAILED;
	}
	freesizet1 = fread(&(psx_info->v_OutRoomTableLen), 1, 4, psx);
	if (freesizet1 != 4)
	{
		return ERROR_PSX_READ_FAILED;
	}
#ifdef __BIG_ENDIAN__
	psx_info->v_OutRoomTableLen = reverse32(psx_info->v_OutRoomTableLen);
#endif
	/* NumRoomMeshBoundingBoxes */
	psx_info->p_NumRoomMeshBoxes = (psx_info->p_OutRoomTableLen + 4 +
	                                psx_info->v_OutRoomTableLen);
	freeint1 = fseek(psx, (long int) psx_info->p_NumRoomMeshBoxes, SEEK_SET);
	if (freeint1 != 0)
	{
		return ERROR_PSX_READ_FAILED;
	}
	freesizet1 = fread(&(psx_info->v_NumRoomMeshBoxes), 1, 4, psx);
	if (freesizet1 != 4)
	{
		return ERROR_PSX_READ_FAILED;
	}
#ifdef __BIG_ENDIAN__
	psx_info->v_NumRoomMeshBoxes = reverse32(psx_info->v_NumRoomMeshBoxes);
#endif
	/* NumMeshData */
	psx_info->p_NumMeshData = (psx_info->p_NumRoomMeshBoxes + 4 +
	                           (psx_info->v_NumRoomMeshBoxes * 8));
	freeint1 = fseek(psx, (long int) psx_info->p_NumMeshData, SEEK_SET);
	if (freeint1 != 0)
	{
		return ERROR_PSX_READ_FAILED;
	}
	freesizet1 = fread(&(psx_info->v_NumMeshData), 1, 4, psx);
	if (freesizet1 != 4)
	{
		return ERROR_PSX_READ_FAILED;
	}
#ifdef __BIG_ENDIAN__
	psx_info->v_NumMeshData = reverse32(psx_info->v_NumMeshData);
#endif
	/* NumMeshPointers */
	psx_info->p_NumMeshPointers = (psx_info->p_NumMeshData + 4 +
	                               (psx_info->v_NumMeshData * 2));
	freeint1 = fseek(psx, (long int) psx_info->p_NumMeshPointers, SEEK_SET);
	if (freeint1 != 0)
	{
		return ERROR_PSX_READ_FAILED;
	}
	freesizet1 = fread(&(psx_info->v_NumMeshPointers), 1, 4, psx);
	if (freesizet1 != 4)
	{
		return ERROR_PSX_READ_FAILED;
	}
#ifdef __BIG_ENDIAN__
	psx_info->v_NumMeshPointers = reverse32(psx_info->v_NumMeshPointers);
#endif
	/* NumAnimations */
	psx_info->p_NumAnimations = (psx_info->p_NumMeshPointers + 4 +
	                             (psx_info->v_NumMeshPointers * 4));
	freeint1 = fseek(psx, (long int) psx_info->p_NumAnimations, SEEK_SET);
	if (freeint1 != 0)
	{
		return ERROR_PSX_READ_FAILED;
	}
	freesizet1 = fread(&(psx_info->v_NumAnimations), 1, 4, psx);
	if (freesizet1 != 4)
	{
		return ERROR_PSX_READ_FAILED;
	}
#ifdef __BIG_ENDIAN__
	psx_info->v_NumAnimations = reverse32(psx_info->v_NumAnimations);
#endif
	/* NumStateChanges */
	psx_info->p_NumStateChanges = (psx_info->p_NumAnimations + 4 +
	                               (psx_info->v_NumAnimations * 32));
	freeint1 = fseek(psx, (long int) psx_info->p_NumStateChanges, SEEK_SET);
	if (freeint1 != 0)
	{
		return ERROR_PSX_READ_FAILED;
	}
	freesizet1 = fread(&(psx_info->v_NumStateChanges), 1, 4, psx);
	if (freesizet1 != 4)
	{
		return ERROR_PSX_READ_FAILED;
	}
#ifdef __BIG_ENDIAN__
	psx_info->v_NumStateChanges = reverse32(psx_info->v_NumStateChanges);
#endif
	/* NumAnimDispatches */
	psx_info->p_NumAnimDispatches = (psx_info->p_NumStateChanges + 4 +
	                                 (psx_info->v_NumStateChanges * 6));
	freeint1 = fseek(psx, (long int) psx_info->p_NumAnimDispatches, SEEK_SET);
	if (freeint1 != 0)
	{
		return ERROR_PSX_READ_FAILED;
	}
	freesizet1 = fread(&(psx_info->v_NumAnimDispatches), 1, 4, psx);
	if (freesizet1 != 4)
	{
		return ERROR_PSX_READ_FAILED;
	}
#ifdef __BIG_ENDIAN__
	psx_info->v_NumAnimDispatches = reverse32(psx_info->v_NumAnimDispatches);
#endif
	/* NumAnimCommands */
	psx_info->p_NumAnimCommands = (psx_info->p_NumAnimDispatches + 4 +
	                               (psx_info->v_NumAnimDispatches * 8));
	freeint1 = fseek(psx, (long int) psx_info->p_NumAnimCommands, SEEK_SET);
	if (freeint1 != 0)
	{
		return ERROR_PSX_READ_FAILED;
	}
	freesizet1 = fread(&(psx_info->v_NumAnimCommands), 1, 4, psx);
	if (freesizet1 != 4)
	{
		return ERROR_PSX_READ_FAILED;
	}
#ifdef __BIG_ENDIAN__
	psx_info->v_NumAnimCommands = reverse32(psx_info->v_NumAnimCommands);
#endif
	/* NumMeshTrees */
	psx_info->p_NumMeshTrees = (psx_info->p_NumAnimCommands + 4 +
	                            (psx_info->v_NumAnimCommands * 2));
	freeint1 = fseek(psx, (long int) psx_info->p_NumMeshTrees, SEEK_SET);
	if (freeint1 != 0)
	{
		return ERROR_PSX_READ_FAILED;
	}
	freesizet1 = fread(&(psx_info->v_NumMeshTrees), 1, 4, psx);
	if (freesizet1 != 4)
	{
		return ERROR_PSX_READ_FAILED;
	}
#ifdef __BIG_ENDIAN__
	psx_info->v_NumMeshTrees = reverse32(psx_info->v_NumMeshTrees);
#endif
	/* NumFrames */
	psx_info->p_NumFrames = (psx_info->p_NumMeshTrees + 4 +
	                         (psx_info->v_NumMeshTrees * 4));
	freeint1 = fseek(psx, (long int) psx_info->p_NumFrames, SEEK_SET);
	if (freeint1 != 0)
	{
		return ERROR_PSX_READ_FAILED;
	}
	freesizet1 = fread(&(psx_info->v_NumFrames), 1, 4, psx);
	if (freesizet1 != 4)
	{
		return ERROR_PSX_READ_FAILED;
	}
#ifdef __BIG_ENDIAN__
	psx_info->v_NumFrames = reverse32(psx_info->v_NumFrames);
#endif
	/* NumMoveables */
	psx_info->p_NumMoveables =
	    (psx_info->p_NumFrames + 4 + (psx_info->v_NumFrames * 2));
	freeint1 = fseek(psx, (long int) psx_info->p_NumMoveables, SEEK_SET);
	if (freeint1 != 0)
	{
		return ERROR_PSX_READ_FAILED;
	}
	freesizet1 = fread(&(psx_info->v_NumMoveables), 1, 4, psx);
	if (freesizet1 != 4)
	{
		return ERROR_PSX_READ_FAILED;
	}
#ifdef __BIG_ENDIAN__
	psx_info->v_NumMoveables = reverse32(psx_info->v_NumMoveables);
#endif
	/* NumStaticMeshes */
	psx_info->p_NumStaticMeshes = (psx_info->p_NumMoveables + 4 +
	                               (psx_info->v_NumMoveables * 20));
	freeint1 = fseek(psx, (long int) psx_info->p_NumStaticMeshes, SEEK_SET);
	if (freeint1 != 0)
	{
		return ERROR_PSX_READ_FAILED;
	}
	freesizet1 = fread(&(psx_info->v_NumStaticMeshes), 1, 4, psx);
	if (freesizet1 != 4)
	{
		return ERROR_PSX_READ_FAILED;
	}
#ifdef __BIG_ENDIAN__
	psx_info->v_NumStaticMeshes = reverse32(psx_info->v_NumStaticMeshes);
#endif
	/* NumTextureTiles */
	psx_info->p_NumTexTiles = (psx_info->p_NumStaticMeshes + 4 +
	                           (psx_info->v_NumStaticMeshes * 32));
	freeint1 = fseek(psx, (long int) psx_info->p_NumTexTiles, SEEK_SET);
	if (freeint1 != 0)
	{
		return ERROR_PSX_READ_FAILED;
	}
	freesizet1 = fread(&(psx_info->v_NumTexTiles), 1, 4, psx);
	if (freesizet1 != 4)
	{
		return ERROR_PSX_READ_FAILED;
	}
#ifdef __BIG_ENDIAN__
	psx_info->v_NumTexTiles = reverse32(psx_info->v_NumTexTiles);
#endif
	/* NumPalettes */
	psx_info->p_NumPalettes = (psx_info->p_NumTexTiles + 4 +
	                           (psx_info->v_NumTexTiles * 32768));
	/* Skips the Kanji-sprite if this is the Japanese version */
	if (((flags & FLAGS_LANGUAGE) == FLAGS_LANGUAGE_JP_JP) ||
	    ((flags & FLAGS_LANGUAGE) == FLAGS_LANGUAGE_ASIA_JP))
	{
		psx_info->p_NumPalettes += 0x00000002;
	}
	freeint1 = fseek(psx, (long int) psx_info->p_NumPalettes, SEEK_SET);
	if (freeint1 != 0)
	{
		return ERROR_PSX_READ_FAILED;
	}
	freesizet1 = fread(&(psx_info->v_NumPalettes), 1, 4, psx);
	if (freesizet1 != 4)
	{
		return ERROR_PSX_READ_FAILED;
	}
#ifdef __BIG_ENDIAN__
	psx_info->v_NumPalettes = reverse32(psx_info->v_NumPalettes);
#endif
	/* NumObjectTextures */
	psx_info->p_NumObjectTextures = (psx_info->p_NumPalettes + 4 +
	                                 (psx_info->v_NumPalettes * 64));
	freeint1 = fseek(psx, (long int) psx_info->p_NumObjectTextures, SEEK_SET);
	if (freeint1 != 0)
	{
		return ERROR_PSX_READ_FAILED;
	}
	freesizet1 = fread(&(psx_info->v_NumObjectTextures), 1, 4, psx);
	if (freesizet1 != 4)
	{
		return ERROR_PSX_READ_FAILED;
	}
#ifdef __BIG_ENDIAN__
	psx_info->v_NumObjectTextures = reverse32(psx_info->v_NumObjectTextures);
#endif
	/* NumSpriteTextures */
	psx_info->p_NumSpriteTextures = (psx_info->p_NumObjectTextures + 4 +
	                                 (psx_info->v_NumObjectTextures * 16));
	freeint1 = fseek(psx, (long int) psx_info->p_NumSpriteTextures, SEEK_SET);
	if (freeint1 != 0)
	{
		return ERROR_PSX_READ_FAILED;
	}
	freesizet1 = fread(&(psx_info->v_NumSpriteTextures), 1, 4, psx);
	if (freesizet1 != 4)
	{
		return ERROR_PSX_READ_FAILED;
	}
#ifdef __BIG_ENDIAN__
	psx_info->v_NumSpriteTextures = reverse32(psx_info->v_NumSpriteTextures);
#endif
	/* NumSpriteSequences */
	psx_info->p_NumSpriteSequences = (psx_info->p_NumSpriteTextures + 4 +
	                                  (psx_info->v_NumSpriteTextures * 16));
	freeint1 = fseek(psx, (long int) psx_info->p_NumSpriteSequences, SEEK_SET);
	if (freeint1 != 0)
	{
		return ERROR_PSX_READ_FAILED;
	}
	freesizet1 = fread(&(psx_info->v_NumSpriteSequences), 1, 4, psx);
	if (freesizet1 != 4)
	{
		return ERROR_PSX_READ_FAILED;
	}
#ifdef __BIG_ENDIAN__
	psx_info->v_NumSpriteSequences = reverse32(psx_info->v_NumSpriteSequences);
#endif
	/* NumCameras */
	psx_info->p_NumCameras = (psx_info->p_NumSpriteSequences + 4 +
	                          (psx_info->v_NumSpriteSequences * 8));
	freeint1 = fseek(psx, (long int) psx_info->p_NumCameras, SEEK_SET);
	if (freeint1 != 0)
	{
		return ERROR_PSX_READ_FAILED;
	}
	freesizet1 = fread(&(psx_info->v_NumCameras), 1, 4, psx);
	if (freesizet1 != 4)
	{
		return ERROR_PSX_READ_FAILED;
	}
#ifdef __BIG_ENDIAN__
	psx_info->v_NumCameras = reverse32(psx_info->v_NumCameras);
#endif
	/* NumSoundSources */
	psx_info->p_NumSoundSources = (psx_info->p_NumCameras + 4 +
	                               (psx_info->v_NumCameras * 16));
	freeint1 = fseek(psx, (long int) psx_info->p_NumSoundSources, SEEK_SET);
	if (freeint1 != 0)
	{
		return ERROR_PSX_READ_FAILED;
	}
	freesizet1 = fread(&(psx_info->v_NumSoundSources), 1, 4, psx);
	if (freesizet1 != 4)
	{
		return ERROR_PSX_READ_FAILED;
	}
#ifdef __BIG_ENDIAN__
	psx_info->v_NumSoundSources = reverse32(psx_info->v_NumSoundSources);
#endif
	/* NumBoxes */
	psx_info->p_NumBoxes = (psx_info->p_NumSoundSources + 4 +
	                        (psx_info->v_NumSoundSources * 16));
	freeint1 = fseek(psx, (long int) psx_info->p_NumBoxes, SEEK_SET);
	if (freeint1 != 0)
	{
		return ERROR_PSX_READ_FAILED;
	}
	freesizet1 = fread(&(psx_info->v_NumBoxes), 1, 4, psx);
	if (freesizet1 != 4)
	{
		return ERROR_PSX_READ_FAILED;
	}
#ifdef __BIG_ENDIAN__
	psx_info->v_NumBoxes = reverse32(psx_info->v_NumBoxes);
#endif
	/* NumOverlaps */
	psx_info->p_NumOverlaps = (psx_info->p_NumBoxes +
	                           4 + (psx_info->v_NumBoxes * 8));
	freeint1 = fseek(psx, (long int) psx_info->p_NumOverlaps, SEEK_SET);
	if (freeint1 != 0)
	{
		return ERROR_PSX_READ_FAILED;
	}
	freesizet1 = fread(&(psx_info->v_NumOverlaps), 1, 4, psx);
	if (freesizet1 != 4)
	{
		return ERROR_PSX_READ_FAILED;
	}
#ifdef __BIG_ENDIAN__
	psx_info->v_NumOverlaps = reverse32(psx_info->v_NumOverlaps);
#endif
	/* NumAnimatedTextures */
	psx_info->p_Zones = (psx_info->p_NumOverlaps +
	                     4 + (psx_info->v_NumOverlaps * 2));
	psx_info->p_NumAnimatedTextures =
		(psx_info->p_Zones + (psx_info->v_NumBoxes * 20));
	freeint1 = fseek(psx, (long int) psx_info->p_NumAnimatedTextures, SEEK_SET);
	if (freeint1 != 0)
	{
		return ERROR_PSX_READ_FAILED;
	}
	freesizet1 = fread(&(psx_info->v_NumAnimatedTextures), 1, 4, psx);
	if (freesizet1 != 4)
	{
		return ERROR_PSX_READ_FAILED;
	}
#ifdef __BIG_ENDIAN__
	psx_info->v_NumAnimatedTextures =
		reverse32(psx_info->v_NumAnimatedTextures);
#endif
	/* NumEntities */
	psx_info->p_NumEntities = (psx_info->p_NumAnimatedTextures + 4 +
	                           (psx_info->v_NumAnimatedTextures * 2));
	freeint1 = fseek(psx, (long int) psx_info->p_NumEntities, SEEK_SET);
	if (freeint1 != 0)
	{
		return ERROR_PSX_READ_FAILED;
	}
	freesizet1 = fread(&(psx_info->v_NumEntities), 1, 4, psx);
	if (freesizet1 != 4)
	{
		return ERROR_PSX_READ_FAILED;
	}
#ifdef __BIG_ENDIAN__
	psx_info->v_NumEntities = reverse32(psx_info->v_NumEntities);
#endif
	/* NumRoomTextures */
	psx_info->p_NumRoomTextures = (psx_info->p_NumEntities + 8 +
	                               (psx_info->v_NumEntities * 24));
	freeint1 = fseek(psx, (long int) psx_info->p_NumRoomTextures, SEEK_SET);
	if (freeint1 != 0)
	{
		return ERROR_PSX_READ_FAILED;
	}
	freesizet1 = fread(&(psx_info->v_NumRoomTextures), 1, 4, psx);
	if (freesizet1 != 4)
	{
		return ERROR_PSX_READ_FAILED;
	}
#ifdef __BIG_ENDIAN__
	psx_info->v_NumRoomTextures = reverse32(psx_info->v_NumRoomTextures);
#endif
	/* NumSoundDetails */
	psx_info->p_NumSoundDetails = (psx_info->p_NumRoomTextures + 744 +
	                               (psx_info->v_NumRoomTextures * 48));
	freeint1 = fseek(psx, (long int) psx_info->p_NumSoundDetails, SEEK_SET);
	if (freeint1 != 0)
	{
		return ERROR_PSX_READ_FAILED;
	}
	freesizet1 = fread(&(psx_info->v_NumSoundDetails), 1, 4, psx);
	if (freesizet1 != 4)
	{
		return ERROR_PSX_READ_FAILED;
	}
#ifdef __BIG_ENDIAN__
	psx_info->v_NumSoundDetails = reverse32(psx_info->v_NumSoundDetails);
#endif
	/* NumCinematicFrames */
	psx_info->p_NumCinematicFrames = (psx_info->p_NumSoundDetails + 8 +
	                                  (psx_info->v_NumSoundDetails * 8));
	freeint1 = fseek(psx, (long int) psx_info->p_NumCinematicFrames, SEEK_SET);
	if (freeint1 != 0)
	{
		return ERROR_PSX_READ_FAILED;
	}
	freesizet1 = fread(&(psx_info->v_NumCinematicFrames), 1, 2, psx);
	if (freesizet1 != 2)
	{
		return ERROR_PSX_READ_FAILED;
	}
#ifdef __BIG_ENDIAN__
	psx_info->v_NumCinematicFrames = reverse16(psx_info->v_NumCinematicFrames);
#endif
	/* NumDemoData */
	psx_info->p_NumDemoData = (psx_info->p_NumCinematicFrames + 2 +
	                           (psx_info->v_NumCinematicFrames * 16));
	freeint1 = fseek(psx, (long int) psx_info->p_NumDemoData, SEEK_SET);
	if (freeint1 != 0)
	{
		return ERROR_PSX_READ_FAILED;
	}
	freesizet1 = fread(&(psx_info->v_NumDemoData), 1, 4, psx);
	if (freesizet1 != 4)
	{
		return ERROR_PSX_READ_FAILED;
	}
#ifdef __BIG_ENDIAN__
	psx_info->v_NumDemoData = reverse32(psx_info->v_NumDemoData);
#endif
	
	return ERROR_NONE;
}

/*
 * Function that creates a new empty PSX-file from scratch.
 * Parameters:
 * * psx = Pointer to the level file
 * * jap = Whether this is a japanese level (0 = No, 1 = Yes)
 */
static int tr3psx_create(FILE *psx, int jap)
{
	/* Variable Declarations */
	INTU8 buffer[64];
	size_t freesizet1;
	int freeint1;
	
	/* Writes an empty level file */
	if (jap == 0)
	{
		freeint1 = zerobytes(psx, 0, 2382);
	}
	else
	{
		freeint1 = zerobytes(psx, 0, 2384);
	}
	if (freeint1 != ERROR_NONE)
	{
		return freeint1;
	}
	
	/* Adds item list version */
	buffer[0] = (INTU8) 0xC8;
	buffer[1] = (INTU8) 0xFF;
	buffer[2] = (INTU8) 0xFF;
	buffer[3] = (INTU8) 0xFF;
	freeint1 = fseek(psx, (long int) 0, SEEK_SET);
	if (freeint1 != 0)
	{
		return ERROR_PSX_READ_FAILED;
	}
	freesizet1 = fwrite(buffer, 1, 4, psx);
	if (freesizet1 != (size_t) 4)
	{
		return ERROR_PSX_WRITE_FAILED;
	}
	
	/* Writes an empty soundmap */
	(void) memset(buffer, 0xFF, 64);
	if (jap == 0)
	{
		freeint1 = fseek(psx, (long int) 1628, SEEK_SET);
	}
	else
	{
		freeint1 = fseek(psx, (long int) 1630, SEEK_SET);
	}
	if (freeint1 != 0)
	{
		return ERROR_PSX_READ_FAILED;
	}
	for (freeint1 = 11; freeint1 > 0; --freeint1)
	{
		freesizet1 = fwrite(buffer, 1, 64, psx);
		if (freesizet1 != 64)
		{
			return ERROR_PSX_WRITE_FAILED;
		}
	}
	freesizet1 = fwrite(buffer, 1, 36, psx);
	if (freesizet1 != 36)
	{
		return ERROR_PSX_WRITE_FAILED;
	}
	
	return ERROR_NONE;
}

/*
 * Function that converts the rooms from PC to PSX.
 * Parameters:
 * * pc = Pointer to the PC-file
 * * psx = Pointer to the PSX-file
 * * pc_p_NumRooms = Offset of NumRooms in pc
 * * psx_info = Offsets and values from PSX
 * * psxpath = Path to the PSX-file
 * * numRooms = Number of rooms in pc
 * * room = Pointer to room struct in pc_info (only read from)
 * * flags = Bit-mask of program-wide flags
 * * roomTextures = Array of room textures
 */
static int convertRooms(FILE *pc, FILE *psx, INTU32 pc_p_NumRooms,
                        struct level_info *psx_info, char *psxpath,
                        INTU16 numRooms, struct room_info *room,
                        INTU32 flags, INTU16 *roomTextures)
{
	/* Variable Declarations */
	INTU8 *pcVerts = NULL;    /* Space in memory to keep vertices from PC */
	INTU8 *pcRects = NULL;    /* Space in memory to keep rectangles from PC */
	INTU8 *pcTris = NULL;     /* Space in memory to keep triangles from PC */
	INTU32 *psVerts = NULL;   /* Space in memory to keep vertices for PSX */
	INTU32 *psRects = NULL;   /* Space in memory to keep rectangles for PSX */
	INTU32 *psTris = NULL;    /* Space in memory to keep triangles for PSX */
	INTU8 *vertices = NULL;   /* Array to keep track of converter vertices */
	INTU8 *rectangles = NULL; /* Array to keep track of converted rectangles */
	INTU8 *triangles = NULL;  /* Array to keep track of converter triangles */
	INTU16 *roomboxes = NULL; /* Array to store the room bounding boxes */
	INTU32 candidates[5];     /* Possible candidates for the next polygon */
	INTU32 chunkOffset[16];   /* Offsets of the PSX room-chunks */
	INTU16 usedVertices[4];   /* Used vertices in this polygon */
	INTU32 numroomboxes;      /* Number of RoomBoundingBoxes */
	INTU16 boxminx;           /* Lowest X-position in RoomBoundingBox */
	INTU16 boxminy;           /* Lowest Y-position in RoomBoundingBox */
	INTU16 boxminz;           /* Lowest Z-position in RoomBoundingBox */
	INTU16 boxmaxx;           /* Highest X-position in RoomBoundingBox */
	INTU16 boxmaxy;           /* Highest Y-position in RoomBoundingBox */
	INTU16 boxmaxz;           /* Highest Z-position in RoomBoundingBox */
	INTU32 numChunks;         /* Number of PSX room-chunks */
	INTU16 rectodo;           /* Number of rectangles left to convert */
	INTU16 tritodo;           /* Number of triangles left to convert */
	INTU8 chunkverts;         /* Number of vertices in this chunk */
	INTU8 chunktris;          /* Number of triangles in this chunk */
	INTU16 chunkrects;        /* Number of rectangles in this chunk */
	INTU16 curroom;           /* Current room in processing */
	INT32 roomY;              /* Y-coordinate of the current room */
	INTU32 numData;           /* NumData to be written to PSX */
	INTU32 chunkSize;         /* Size of the current chunk */
	INTU8 water;              /* Whether this room is underwater */
	long int pc_curpos;       /* Current position in pc */
	long int psx_curpos;      /* Current position in psx */
	long int psx_curpos2;     /* Second current position in psx */
	size_t freesizet1,
	       freesizet2;        /* size_t for general use */
	int freeint1;             /* Integer for general use */
	INTU8 freeuint81;         /* Unsigned 8-bit integer for general use */
	INTU16 freeuint161,
	       freeuint162,
	       freeuint163,
	       freeuint164,
	       freeuint165;       /* Unsigned 16-bit integers for general use */
	INT16 freeint161;         /* Signed 16-bit integer for general use */
	INTU32 freeuint321,
	       freeuint322,
	       freeuint323;       /* Unsigned 32-bit integers for general use */
	int errorNumber = ERROR_NONE;
	
	/* Allocates needed memory */
	freeuint161 = 0x0003;
	freeuint162 = 0x0001;
	freeuint163 = 0x0001;
	for (curroom = 0x0000; curroom < numRooms; ++curroom)
	{
		if (room[curroom].v_NumVertices > freeuint161)
		{
			freeuint161 = room[curroom].v_NumVertices;
		}
		if (room[curroom].v_NumRectangles > freeuint162)
		{
			freeuint162 = room[curroom].v_NumRectangles;
		}
		if (room[curroom].v_NumTriangles > freeuint163)
		{
			freeuint163 = room[curroom].v_NumTriangles;
		}
	}
	vertices = calloc((size_t) freeuint161, (size_t) 1);
	rectangles = calloc((size_t) freeuint162, (size_t) 1);
	triangles = calloc((size_t) freeuint163, (size_t) 1);
	pcVerts = calloc((size_t) freeuint161, (size_t) 12);
	pcRects = calloc((size_t) freeuint162, (size_t) 10);
	pcTris = calloc((size_t) freeuint163, (size_t) 8);
	psVerts = calloc((size_t) freeuint161, (size_t) 4);
	psRects = calloc((size_t) freeuint162, (size_t) 8);
	psTris = calloc((size_t) freeuint163, (size_t) 4);
	freeuint161 = (pc_p_NumRooms * 0x0020);
	roomboxes = calloc((size_t) freeuint161, (size_t) 8);
	if ((vertices == NULL) || (rectangles == NULL) || (triangles == NULL) ||
	    (pcVerts == NULL) || (pcRects == NULL) || (pcTris == NULL) ||
	    (psVerts == NULL) || (psRects == NULL) || (psTris == NULL) ||
	    (roomboxes == NULL))
	{
		errorNumber = ERROR_MEMORY;
		goto end;
	}
	
	/* Removes original rooms */
	freeuint321 = (psx_info->p_NumFloorData - psx_info->p_NumRooms);
	freeuint321 -= 0x00000002;
	errorNumber = removebytes(psx, psxpath, psx_info->p_NumRooms, freeuint321);
	if (errorNumber != ERROR_NONE)
	{
		PRINT_FAILED;
		goto end;
	}
	level_info_sub(psx_info, psx_info->p_NumRooms, freeuint321);
	
	/*
	 * Note to self:
	 * We add one to the position of NumFloorData here, so level_info_add
	 * doesn't get confused about the position of NumFloorData, this one is
	 * subtracted from the position at the end of this function.
	 */
	++(psx_info->p_NumFloorData);
	
	/* Copies NumRooms */
	pc_curpos = (long int) pc_p_NumRooms;
	psx_curpos = (long int) psx_info->p_NumRooms;
	freeint1 = fseek(pc, pc_curpos, SEEK_SET);
	if (freeint1 != 0)
	{
		errorNumber = ERROR_PC_READ_FAILED;
		goto end;
	}
	freesizet1 = fread(&freeuint161, 1, 2, pc);
	if (freesizet1 != 2)
	{
		errorNumber = ERROR_PC_READ_FAILED;
		goto end;
	}
	freeint1 = fseek(psx, psx_curpos, SEEK_SET);
	if (freeint1 != 0)
	{
		errorNumber = ERROR_PSX_READ_FAILED;
		goto end;
	}
	freesizet1 = fwrite(&freeuint161, 1, 2, psx);
	if (freesizet1 != 2)
	{
		errorNumber = ERROR_PSX_WRITE_FAILED;
		goto end;
	}
	pc_curpos += 2;
	psx_curpos += 2;
	
	/* Sets NumRoomBoxes to 0 before we start */
	numroomboxes = 0x00000000;
	
	/* Converts rooms */
	for (curroom = 0; curroom < numRooms; ++curroom)
	{
		/* Reads whether this room is underwater */
		pc_curpos = (long int) (room[curroom].p_AlternateRoom + 0x00000002);
		freeint1 = fseek(pc, pc_curpos, SEEK_SET);
		if (freeint1 != 0)
		{
			errorNumber = ERROR_PC_READ_FAILED;
			goto end;
		}
		freesizet1 = fread(&water, (size_t) 1, (size_t) 1, pc);
		if (freesizet1 != 1)
		{
			errorNumber = ERROR_PC_READ_FAILED;
			goto end;
		}
		water &= (INTU8) 0x01;
		
		/* Allocates room for the header and indices */
		errorNumber = insertbytes(psx, (INTU32) psx_curpos, (INTU32) 88);
		if (errorNumber != ERROR_NONE)
		{
			goto end;
		}
		level_info_add(psx_info, (INTU32) psx_curpos, (INTU32) 88);
		errorNumber = zerobytes(psx, (INTU32) psx_curpos, (INTU32) 88);
		if (errorNumber != ERROR_NONE)
		{
			goto end;
		}
		
		/* Copies room header */
		pc_curpos = (long int) (room[curroom].p_NumVertices - 20);
		errorNumber = copybytes(pc, psx, (INTU32) pc_curpos,
		                        (INTU32) psx_curpos, (INTU32) 16);
		if (errorNumber != ERROR_NONE)
		{
			goto end;
		}
		psx_curpos += 16;
		psx_curpos2 = psx_curpos;
		psx_curpos += 72;
		numData = 0x00000044;
		
		/* Reads roomY */
		pc_curpos += 12;
		freeint1 = fseek(pc, pc_curpos, SEEK_SET);
		if (freeint1 != 0)
		{
			errorNumber = ERROR_PC_READ_FAILED;
			goto end;
		}
		freesizet1 = fread(&roomY, 1, 4, pc);
		if (freesizet1 != 4)
		{
			errorNumber = ERROR_PC_READ_FAILED;
			goto end;
		}
#ifdef __BIG_ENDIAN__
		roomY = reverse32(roomY);
#endif
		
		/* Reads vertices into memory */
		pc_curpos = (long int) (room[curroom].p_NumVertices + 0x00000002);
		freeint1 = fseek(pc, pc_curpos, SEEK_SET);
		if (freeint1 != 0)
		{
			errorNumber = ERROR_PC_READ_FAILED;
			goto end;
		}
		freesizet2 = (size_t) (room[curroom].v_NumVertices * 12);
		freesizet1 = fread(pcVerts, (size_t) 1, freesizet2, pc);
		if (freesizet1 != freesizet2)
		{
			errorNumber = ERROR_PC_READ_FAILED;
			goto end;
		}
		
		/* Reads rectangles into memory */
		pc_curpos = (long int) (room[curroom].p_NumRectangles + 0x00000002);
		freeint1 = fseek(pc, pc_curpos, SEEK_SET);
		if (freeint1 != 0)
		{
			errorNumber = ERROR_PC_READ_FAILED;
			goto end;
		}
		freesizet2 = (size_t) (room[curroom].v_NumRectangles * 10);
		freesizet1 = fread(pcRects, (size_t) 1, (size_t) freesizet2, pc);
		if (freesizet1 != freesizet2)
		{
			errorNumber = ERROR_PC_READ_FAILED;
			goto end;
		}
		
		/* Reads triangles into memory */
		pc_curpos = (long int) (room[curroom].p_NumTriangles + 0x00000002);
		freeint1 = fseek(pc, pc_curpos, SEEK_SET);
		if (freeint1 != 0)
		{
			errorNumber = ERROR_PC_READ_FAILED;
			goto end;
		}
		freesizet2 = (size_t) (room[curroom].v_NumTriangles * 8);
		freesizet1 = fread(pcTris, (size_t) 1, (size_t) freesizet2, pc);
		if (freesizet1 != freesizet2)
		{
			errorNumber = ERROR_PC_READ_FAILED;
			goto end;
		}
		
		/* Sets Y-coordinates to local */
		freeuint321 = 0x00000002;
		for (freeuint161 = 0x0000; freeuint161 < room[curroom].v_NumVertices;
		     ++freeuint161)
		{
			memcpy(&freeint161, &pcVerts[freeuint321], 2);
#ifdef __BIG_ENDIAN__
			freeint161 = reverse16(freeint161);
#endif
			freeint161 -= roomY;
#ifdef __BIG_ENDIAN__
			freeint161 = reverse16(freeint161);
#endif
			memcpy(&pcVerts[freeuint321], &freeint161, 2);
			freeuint321 += 0x0000000C;
		}
		
		/* Sets variables at the start */
		(void) memset(&chunkOffset[0], 0, 64);
		chunkOffset[0] = 0x00000044;
		(void) memset(psRects, 0, (room[curroom].v_NumRectangles * 8));
		(void) memset(vertices, 0, room[curroom].v_NumVertices);
		(void) memset(rectangles, 0, room[curroom].v_NumRectangles);
		(void) memset(triangles, 0, room[curroom].v_NumTriangles);
		numChunks = 0x00000000;
		chunkSize = 0x00000018;
		chunkverts = (INTU8) 0x00;
		chunktris = (INTU8) 0x00;
		chunkrects = (INTU16) 0x0000;
		rectodo = room[curroom].v_NumRectangles;
		tritodo = room[curroom].v_NumTriangles;
		boxminx = boxminy = boxminz = (INTU16) 0xFFFF;
		boxmaxx = boxmaxy = boxmaxz = (INTU16) 0x0000;
		
		/* Adds polygons one at a time */
		while ((rectodo > 0x0000) || (tritodo > 0x0000))
		{
			(void) memset(&candidates[0], 0, 20);
			/* Looks for a rectangle to add */
			for (freeuint161 = 0x0000;
			     freeuint161 < room[curroom].v_NumRectangles; ++freeuint161)
			{
				/* Skips an already added rectangle */
				if (rectangles[freeuint161] == (INTU8) 0x01)
				{
					continue;
				}
				
				/* Counts the number of new vertices needed */
				freeuint321 = (0x0000000A * ((INTU32) freeuint161));
				memcpy(&usedVertices[0], &pcRects[freeuint321], 8);
#ifdef __BIG_ENDIAN__
				usedVertices[0] = reverse16(usedVertices[0]);
				usedVertices[1] = reverse16(usedVertices[1]);
				usedVertices[2] = reverse16(usedVertices[2]);
				usedVertices[3] = reverse16(usedVertices[3]);
#endif
				freeuint162 = 0x0000;
				if ((vertices[usedVertices[0]] & 0x80) == (INTU8) 0x00)
				{
					++freeuint162;
				}
				if ((vertices[usedVertices[1]] & 0x80) == (INTU8) 0x00)
				{
					++freeuint162;
				}
				if ((vertices[usedVertices[2]] & 0x80) == (INTU8) 0x00)
				{
					++freeuint162;
				}
				if ((vertices[usedVertices[3]] & 0x80) == (INTU8) 0x00)
				{
					++freeuint162;
				}
				
				/* Remembers this rectangle as a candidate */
				if ((candidates[freeuint162] & 0x80000000) == 0x00000000)
				{
					candidates[freeuint162] = (INTU32) freeuint161;
					candidates[freeuint162] |= 0x80000000;
					/* Takes note of whether this rectangle is double sided */
					if ((pcRects[(freeuint321 + 0x00000009)] & (INTU8) 0x80) ==
					    (INTU8) 0x80)
					{
						candidates[freeuint162] |= 0x20000000;
					}
				}
			}
			
			/* Looks for a triangle to add */
			for (freeuint161 = 0x0000;
			     freeuint161 < room[curroom].v_NumTriangles; ++freeuint161)
			{
				/* Skips an already added triangle */
				if (triangles[freeuint161] == (INTU8) 0x01)
				{
					continue;
				}
				
				/* Counts the number of new vertices needed */
				freeuint321 = (0x00000008 * ((INTU32) freeuint161));
				memcpy(&usedVertices[0], &pcTris[freeuint321], 6);
#ifdef __BIG_ENDIAN__
				usedVertices[0] = reverse16(usedVertices[0]);
				usedVertices[1] = reverse16(usedVertices[1]);
				usedVertices[2] = reverse16(usedVertices[2]);
#endif
				freeuint162 = 0x0000;
				if ((vertices[usedVertices[0]] & 0x80) == (INTU8) 0x00)
				{
					++freeuint162;
				}
				if ((vertices[usedVertices[1]] & 0x80) == (INTU8) 0x00)
				{
					++freeuint162;
				}
				if ((vertices[usedVertices[2]] & 0x80) == (INTU8) 0x00)
				{
					++freeuint162;
				}
				
				/* Remembers this triangle as a candidate */
				if ((candidates[freeuint162] & 0xC0000000) == 0x00000000)
				{
					candidates[freeuint162] = (INTU32) freeuint161;
					candidates[freeuint162] |= 0x40000000;
					/* Takes note of whether this triangle is double sided */
					if ((pcTris[(freeuint321 + 0x00000007)] & (INTU8) 0x80) ==
					    (INTU8) 0x80)
					{
						candidates[freeuint162] |= 0x20000000;
					}
				}
			}
			
			/* Picks a candidate */
			for (freeuint161 = 0x0000; freeuint161 < 0x0005; ++freeuint161)
			{
				if (candidates[freeuint161] != 0x00000000)
				{
					break;
				}
			}
			
			/* Sees if the selected polygon will fit in the current chunk */
			freeuint81 = (chunkverts + (INTU8) freeuint161);
			/* If the candidate is a triangle,
			 * and there's already 255 triangles in this chunk */
			if ((((candidates[freeuint161] & 0x60000000) == 0x40000000) &&
			     (chunktris == (INTU8) 0xFF)) ||
			/* If the candidate is a double-sided triangle,
			 * and there's no space for two triangles in this chunk */
			    (((candidates[freeuint161] & 0x60000000) == 0x60000000) &&
			     (chunktris >= (INTU8) 0xFE)) ||
			/* If the number of vertices to be added
			 * will bring the total above 93 */
			    (freeuint81 >= (INTU8) 0x5D))
			{
				/* Checks there is space for another chunk */
				if (numChunks == 0x00000010)
				{
					/* Prints error message here, to include the room number */
					printf(PROGNAME ": Room " PRINTU16
					       " too big to be converted\n", curroom);
					errorNumber = ERROR_TOO_MANY_CHUNKS;
					goto end;
				}
				
				/* Ends this chunk */
				freeuint321 = 0x000003FF;
				freeuint164 = (INTU16) chunkrects;
				freeuint164 %= 0x0003;
				freeuint321 <<= (freeuint164 * 0x000A);
				freeuint164 = (INTU16) chunkrects;
				freeuint164 -= (freeuint164 % 0x0003);
				freeuint164 /= 0x0003;
				freeuint164 *= 0x0004;
				psRects[freeuint164] |= freeuint321;
				
				/* Makes room for the chunk */
				errorNumber = insertbytes(psx, (INTU32) psx_curpos, chunkSize);
				if (errorNumber != ERROR_NONE)
				{
					goto end;
				}
				level_info_add(psx_info, (INTU32) psx_curpos, chunkSize);
				numData += chunkSize;
				
				/* Remembers that the chunk was added */
				chunkSize += chunkOffset[numChunks];
				++numChunks;
				if (numChunks < 0x00000010)
				{
					chunkOffset[numChunks] = chunkSize;
				}
				
				/* Adjusts for Big-Endian */
#ifdef __BIG_ENDIAN__
				freeuint164 = (INTU16) chunkrects;
				freeuint164 += ((freeuint164 - (freeuint164 % 0x0003)) /
				                0x0003);
				++freeuint164;
				for (freeuint163 = 0x0000; freeuint163 < freeuint164;
				     ++freeuint163)
				{
					psRects[freeuint163] = reverse32(psRects[freeuint163]);
				}
#endif
				
				/* Makes the bounding box */
				boxminx <<= 8U;
				boxminy <<= 8U;
				boxminz <<= 8U;
				boxmaxx <<= 8U;
				boxmaxy <<= 8U;
				boxmaxz <<= 8U;
				/* Checks whether this bounding box's min already exists */
				freeuint321 = 0x00000000;
				freeuint322 = (numroomboxes * 0x00000004);
				while (freeuint321 < freeuint322)
				{
					if ((roomboxes[freeuint321] == boxminx) &&
					    (roomboxes[(freeuint321 + 0x00000001)] == boxminy) &&
					    (roomboxes[(freeuint321 + 0x00000002)] == boxminz))
					{
						break;
					}
					
					/* Moves to the next vertex */
					freeuint321 += 0x00000004;
				}
				if (freeuint321 == freeuint322)
				{
					roomboxes[freeuint321] = boxminx;
					roomboxes[(freeuint321 + 0x00000001)] = boxminy;
					roomboxes[(freeuint321 + 0x00000002)] = boxminz;
					++numroomboxes;
				}
				freeuint164 = (INTU16) (freeuint321 >> 2U);
				
				/* Writes BoundMinIndex */
#ifdef __BIG_ENDIAN__
				freeuint164 = reverse16(freeuint164);
#endif
				freeint1 = fseek(psx, psx_curpos, SEEK_SET);
				if (freeint1 != 0)
				{
					errorNumber = ERROR_PSX_READ_FAILED;
					goto end;
				}
				freesizet1 = fwrite(&freeuint164, (size_t) 1, (size_t) 2, psx);
				if (freesizet1 != (size_t) 2)
				{
					errorNumber = ERROR_PSX_WRITE_FAILED;
					goto end;
				}
				psx_curpos += 2;
				
				/* Checks whether this bounding box's max already exists */
				freeuint321 = 0x00000000;
				freeuint322 = (numroomboxes * 0x00000004);
				while (freeuint321 < freeuint322)
				{
					if ((roomboxes[freeuint321] == boxmaxx) &&
					    (roomboxes[(freeuint321 + 0x00000001)] == boxmaxy) &&
					    (roomboxes[(freeuint321 + 0x00000002)] == boxmaxz))
					{
						break;
					}
					
					/* Moves to the next vertex */
					freeuint321 += 0x00000004;
				}
				if (freeuint321 == freeuint322)
				{
					roomboxes[freeuint321] = boxmaxx;
					roomboxes[(freeuint321 + 0x00000001)] = boxmaxy;
					roomboxes[(freeuint321 + 0x00000002)] = boxmaxz;
					++numroomboxes;
				}
				freeuint164 = (INTU16) (freeuint321 >> 2U);
				
				/* Writes BoundMaxIndex */
#ifdef __BIG_ENDIAN__
				freeuint164 = reverse16(freeuint164);
#endif
				freeint1 = fseek(psx, psx_curpos, SEEK_SET);
				if (freeint1 != 0)
				{
					errorNumber = ERROR_PSX_READ_FAILED;
					goto end;
				}
				freesizet1 = fwrite(&freeuint164, (size_t) 1, (size_t) 2, psx);
				if (freesizet1 != (size_t) 2)
				{
					errorNumber = ERROR_PSX_WRITE_FAILED;
					goto end;
				}
				psx_curpos += 2;
				
				/* Writes Room Number */
				freeuint81 = (INTU8) curroom;
				freesizet1 = fwrite(&freeuint81, (size_t) 1, (size_t) 1, psx);
				if (freesizet1 != (size_t) 1)
				{
					errorNumber = ERROR_PSX_WRITE_FAILED;
					goto end;
				}
				++psx_curpos;
				
				/* Writes room flags */
				freeuint81 = (INTU8) 0x00;
				freesizet1 = fwrite(&freeuint81, (size_t) 1, (size_t) 1, psx);
				if (freesizet1 != (size_t) 1)
				{
					errorNumber = ERROR_PSX_WRITE_FAILED;
					goto end;
				}
				++psx_curpos;
				
				/* Writes NumVertices */
				freesizet1 = fwrite(&chunkverts, (size_t) 1, (size_t) 1, psx);
				if (freesizet1 != (size_t) 1)
				{
					errorNumber = ERROR_PSX_WRITE_FAILED;
					goto end;
				}
				++psx_curpos;
				
				/* Writes NumTriangles */
				freesizet1 = fwrite(&chunktris, (size_t) 1, (size_t) 1, psx);
				if (freesizet1 != (size_t) 1)
				{
					errorNumber = ERROR_PSX_WRITE_FAILED;
					goto end;
				}
				++psx_curpos;
				
				/* Writes Vertices */
				freeuint321 = (INTU32) chunkverts;
				freeuint321 &= 0x0000007F;
				freeuint321 *= 0x00000004;
				freesizet1 = fwrite(psVerts, (size_t) 1, (size_t) freeuint321,
				                    psx);
				if (freesizet1 != (size_t) freeuint321)
				{
					errorNumber = ERROR_PSX_WRITE_FAILED;
					goto end;
				}
				psx_curpos += (long int) freeuint321;
				
				/* Writes Triangles */
				freeuint321 = (INTU32) chunktris;
				freeuint321 &= 0x000000FF;
				freeuint321 *= 0x00000004;
				freesizet1 = fwrite(psTris, (size_t) 1, (size_t) freeuint321,
				                    psx);
				if (freesizet1 != (size_t) freeuint321)
				{
					errorNumber = ERROR_PSX_WRITE_FAILED;
					goto end;
				}
				psx_curpos += (long int) freeuint321;
				
				/* Writes Rectangles */
				freeuint164 = (INTU16) chunkrects;
				freeuint164 -= (freeuint164 % 0x0003);
				freeuint164 /= 0x0003;
				++freeuint164;
				freeuint321 = (((INTU32) freeuint164) * 0x00000010);
				freesizet1 = fwrite(psRects, (size_t) 1, (size_t) freeuint321,
				                    psx);
				if (freesizet1 != (size_t) freeuint321)
				{
					errorNumber = ERROR_PSX_WRITE_FAILED;
					goto end;
				}
				psx_curpos += (long int) freeuint321;
				
				/* Resets the chunk variables to 0 */
				chunkverts = (INTU8) 0x00;
				chunktris = (INTU8) 0x00;
				chunkrects = (INTU16) 0x0000;
				chunkSize = 0x00000018;
				boxminx = boxminy = boxminz = (INTU16) 0xFFFF;
				boxmaxx = boxmaxy = boxmaxz = (INTU16) 0x0000;
				(void) memset(vertices, 0, room[curroom].v_NumVertices);
				(void) memset(psRects, 0, (room[curroom].v_NumRectangles * 8));
			}
			
			/* Adds the needed vertices */
			freeuint162 = (INTU16) (candidates[freeuint161] & 0x0000FFFF);
			if ((candidates[freeuint161] & 0x80000000) == 0x80000000)
			{
				freeuint321 = (0x0000000A * ((INTU32) freeuint162));
				memcpy(&usedVertices[0], &pcRects[freeuint321], 8);
#ifdef __BIG_ENDIAN__
				usedVertices[0] = reverse16(usedVertices[0]);
				usedVertices[1] = reverse16(usedVertices[1]);
				usedVertices[2] = reverse16(usedVertices[2]);
				usedVertices[3] = reverse16(usedVertices[3]);
#endif
				freeuint163 = 0x0004;
			}
			else
			{
				freeuint321 = (0x00000008 * ((INTU32) freeuint162));
				memcpy(&usedVertices[0], &pcTris[freeuint321], 6);
#ifdef __BIG_ENDIAN__
				usedVertices[0] = reverse16(usedVertices[0]);
				usedVertices[1] = reverse16(usedVertices[1]);
				usedVertices[2] = reverse16(usedVertices[2]);
#endif
				freeuint163 = 0x0003;
			}
			do
			{
				--freeuint163;
				if ((vertices[usedVertices[freeuint163]] & 0x80) != 0x00)
				{
					continue;
				}
				/* Converts the vertex from PC to PSX */
				freeuint321 = 0x00000000;
				freeuint322 = 0x0000000C;
				freeuint322 *= (INTU32) usedVertices[freeuint163];
				++freeuint322;
				/* X */
				freeuint321 = (INTU32) pcVerts[freeuint322];
				freeuint321 &= 0x0000007C;
				if ((INTU16) freeuint321 < boxminx)
				{
					boxminx = (INTU16) freeuint321;
				}
				if ((INTU16) freeuint321 > boxmaxx)
				{
					boxmaxx = (INTU16) freeuint321;
				}
				freeuint321 <<= 8U;
				freeuint322 += 0x00000002;
				/* Y */
				freeuint323 = (INTU32) pcVerts[freeuint322];
				freeuint323 &= 0x0000001F;
				if ((INTU16) freeuint323 < boxminy)
				{
					boxminy = (INTU16) freeuint323;
				}
				if ((INTU16) freeuint323 > boxmaxy)
				{
					boxmaxy = (INTU16) freeuint323;
				}
				freeuint323 <<= 5U;
				freeuint321 |= freeuint323;
				freeuint322 += 0x00000002;
				/* Z */
				freeuint323 = (INTU32) pcVerts[freeuint322];
				freeuint323 &= 0x0000007C;
				if ((INTU16) freeuint323 < boxminz)
				{
					boxminz = (INTU16) freeuint323;
				}
				if ((INTU16) freeuint323 > boxmaxz)
				{
					boxmaxz = (INTU16) freeuint323;
				}
				freeuint323 >>= 2U;
				freeuint321 |= freeuint323;
				freeuint322 += 0x00000004;
				/* Flags */
				freeuint323 = (INTU32) pcVerts[freeuint322];
				freeuint323 &= 0x00000020;
				freeuint323 <<= 26U;
				freeuint321 |= freeuint323;
				freeuint323 = (INTU32) pcVerts[freeuint322];
				freeuint323 &= 0x00000040;
				freeuint323 <<= 24U;
				freeuint321 |= freeuint323;
				++freeuint322;
				/* Colour */
				memcpy(&freeuint164, &pcVerts[freeuint322], 2);
#ifdef __BIG_ENDIAN__
				freeuint164 = reverse16(freeuint164);
#endif
				freeuint164 = rgbbgr1616(freeuint164);
				freeuint323 = (INTU32) freeuint164;
				freeuint323 <<= 15U;
				freeuint321 |= freeuint323;
				
				/* Adds this vertex to the chunk */
#ifdef __BIG_ENDIAN__
				freeuint321 = reverse32(freeuint321);
#endif
				psVerts[chunkverts] = freeuint321;
				vertices[usedVertices[freeuint163]] = chunkverts;
				vertices[usedVertices[freeuint163]] |= 0x80;
				chunkSize += 0x00000004;
				++chunkverts;
			}
			while (freeuint163 > 0x0000);
			
			/* Adds the polygon */
			if ((candidates[freeuint161] & 0x40000000) == 0x40000000)
			{
				/* Adds the triangle */
				freeuint322 = (0x00000008 * (INTU32) freeuint162);
				/* Vertex 1 */
				memcpy(&freeuint164, &pcTris[freeuint322], 2);
#ifdef __BIG_ENDIAN__
				freeuint164 = reverse16(freeuint164);
#endif
				freeuint321 = (INTU32) vertices[freeuint164];
				freeuint321 &= 0x0000007F;
				freeuint322 += 0x00000002;
				/* Vertex 2 */
				memcpy(&freeuint164, &pcTris[freeuint322], 2);
#ifdef __BIG_ENDIAN__
				freeuint164 = reverse16(freeuint164);
#endif
				freeuint323 = (INTU32) vertices[freeuint164];
				freeuint323 &= 0x0000007F;
				freeuint323 <<= 7U;
				freeuint321 |= freeuint323;
				freeuint322 += 0x00000002;
				/* Vertex 3 */
				memcpy(&freeuint164, &pcTris[freeuint322], 2);
#ifdef __BIG_ENDIAN__
				freeuint164 = reverse16(freeuint164);
#endif
				freeuint323 = (INTU32) vertices[freeuint164];
				freeuint323 &= 0x0000007F;
				freeuint323 <<= 14U;
				freeuint321 |= freeuint323;
				freeuint322 += 0x00000002;
				/* Texture */
				memcpy(&freeuint164, &pcTris[freeuint322], 2);
#ifdef __BIG_ENDIAN__
				freeuint164 = reverse16(freeuint164);
#endif
				freeuint164 &= 0x3FFF;
				freeuint164 <<= 2U;
				if (water == (INTU8) 0x01)
				{
					freeuint164 += 0x0002;
				}
				freeuint323 = (INTU32) roomTextures[freeuint164];
				if (freeuint323 > 0x000003FF)
				{
					PRINT_WARNING;
					printf("Room Texture index " PRINTU32 " is above 1023\n",
					       freeuint323);
				}
				freeuint323 &= 0x000003FF;
				freeuint323 <<= 21U;
				freeuint321 |= freeuint323;
				
				/* Remembers that this triangle was converted */
#ifdef __BIG_ENDIAN__
				freeuint321 = reverse32(freeuint321);
#endif
				psTris[chunktris] = freeuint321;
				triangles[freeuint162] = (INTU8) 0x01;
				chunkSize += 0x00000004;
				++chunktris;
				--tritodo;
				if ((candidates[freeuint161] & 0x20000000) == 0x20000000)
				{
					/* Makes the needed duplicate for double-sided */
#ifdef __BIG_ENDIAN__
					freeuint321 = reverse32(freeuint321);
#endif
					freeuint321 = (INTU32)
					              ((freeuint321 & 0x001FC000) |
					               ((freeuint321 & 0x00003F80) >> 7U) |
					               ((freeuint321 & 0x0000007F) << 7U));
					++freeuint164;
					freeuint323 = (INTU32) roomTextures[freeuint164];
					if (freeuint323 > 0x000003FF)
					{
						PRINT_WARNING;
						printf("Room Texture index " PRINTU32 " is above 1023\n",
						       freeuint323);
					}
					freeuint323 &= 0x000003FF;
					freeuint323 <<= 21U;
					freeuint321 |= freeuint323;
#ifdef __BIG_ENDIAN__
					freeuint321 = reverse32(freeuint321);
#endif
					psTris[chunktris] = freeuint321;
					chunkSize += 0x00000004;
					++chunktris;
				}
			}
			else
			{
				/* Adds the rectangle */
				freeuint322 = (0x0000000A * (INTU32) freeuint162);
				/* Vertex 1 */
				memcpy(&freeuint164, &pcRects[freeuint322], 2);
#ifdef __BIG_ENDIAN__
				freeuint164 = reverse16(freeuint164);
#endif
				freeuint321 = (INTU32) vertices[freeuint164];
				freeuint321 &= 0x0000007F;
				freeuint322 += 0x00000002;
				/* Vertex 2 */
				memcpy(&freeuint164, &pcRects[freeuint322], 2);
#ifdef __BIG_ENDIAN__
				freeuint164 = reverse16(freeuint164);
#endif
				freeuint323 = (INTU32) vertices[freeuint164];
				freeuint323 &= 0x0000007F;
				freeuint323 <<= 7U;
				freeuint321 |= freeuint323;
				freeuint322 += 0x00000002;
				/* Vertex 3 */
				memcpy(&freeuint164, &pcRects[freeuint322], 2);
#ifdef __BIG_ENDIAN__
				freeuint164 = reverse16(freeuint164);
#endif
				freeuint323 = (INTU32) vertices[freeuint164];
				freeuint323 &= 0x0000007F;
				freeuint323 <<= 21U;
				freeuint321 |= freeuint323;
				freeuint322 += 0x00000002;
				/* Vertex 4 */
				memcpy(&freeuint164, &pcRects[freeuint322], 2);
#ifdef __BIG_ENDIAN__
				freeuint164 = reverse16(freeuint164);
#endif
				freeuint323 = (INTU32) vertices[freeuint164];
				freeuint323 &= 0x0000007F;
				freeuint323 <<= 14U;
				freeuint321 |= freeuint323;
				/* Stores the vertex indices */
				freeuint164 = (INTU16) chunkrects;
				freeuint164 += ((freeuint164 - (freeuint164 % 0x0003)) /
				                0x0003);
				++freeuint164;
				psRects[freeuint164] = freeuint321;
				freeuint322 += 0x00000002;
				/* Texture */
				memcpy(&freeuint164, &pcRects[freeuint322], 2);
#ifdef __BIG_ENDIAN__
				freeuint164 = reverse16(freeuint164);
#endif
				freeuint164 &= 0x3FFF;
				freeuint164 <<= 2U;
				if (water == (INTU8) 0x01)
				{
					freeuint164 += 0x0002;
				}
				freeuint323 = (INTU32) roomTextures[freeuint164];
				/* Prints an error if the index is too high */
				if (freeuint323 >= 0x000003FF)
				{
					PRINT_WARNING;
					printf("Room Texture index " PRINTU32 " is above 1022\n",
					       freeuint323);
					/* Escapes corruption of the room data */
					if (freeuint323 == 0x000003FF)
					{
						freeuint323 = 0x00000000;
					}
				}
				
				/* Stores the texture */
				freeuint323 &= 0x000003FF;
				freeuint164 = (INTU16) chunkrects;
				freeuint164 -= (freeuint164 % 0x0003);
				freeuint164 /= 0x0003;
				freeuint164 *= 0x0004;
				freeuint165 = (INTU16) chunkrects;
				freeuint165 %= 0x0003;
				freeuint323 <<= (freeuint165 * 0x000A);
				psRects[freeuint164] |= freeuint323;
				
				/* Remembers that this rectangle was converted */
				rectangles[freeuint162] = (INTU8) 0x01;
				freeuint164 = (INTU16) chunkrects;
				if ((freeuint164 % 0x0003) == 0x0002)
				{
					chunkSize += 0x00000010;
				}
				++chunkrects;
				--rectodo;
				
				if ((candidates[freeuint161] & 0x20000000) == 0x20000000)
				{
					/* Makes the needed duplicate for double-sided */
					/* Vertices */
					freeuint164 = (INTU16) chunkrects;
					--freeuint164;
					freeuint164 += ((freeuint164 - (freeuint164 % 0x0003)) /
					                0x0003);
					++freeuint164;
					freeuint321 = psRects[freeuint164];
#ifdef __BIG_ENDIAN__
					freeuint321 = reverse32(freeuint321);
#endif
					freeuint321 = (INTU32)
					              ((freeuint321 & 0xF0000000) |
					               ((freeuint321 & 0x0FE03F80) >> 7U) |
					               ((freeuint321 & 0x001FC07F) << 7U));
#ifdef __BIG_ENDIAN__
					freeuint321 = reverse32(freeuint321);
#endif
					freeuint164 = (INTU16) chunkrects;
					freeuint164 += ((freeuint164 - (freeuint164 % 0x0003)) /
					                0x0003);
					++freeuint164;
					psRects[freeuint164] = freeuint321;
					/* Texture */
					memcpy(&freeuint164, &pcRects[freeuint322], 2);
#ifdef __BIG_ENDIAN__
					freeuint164 = reverse16(freeuint164);
#endif
					freeuint164 &= 0x3FFF;
					freeuint164 <<= 2U;
					++freeuint164;
					if (water == (INTU8) 0x01)
					{
						freeuint164 += 0x0002;
					}
					freeuint323 = (INTU32) roomTextures[freeuint164];
					/* Prints an error if the index is too high */
					if (freeuint323 >= 0x000003FF)
					{
						PRINT_WARNING;
						printf("Room Texture index " PRINTU32 " is above 1022\n",
						       freeuint323);
						/* Escapes corruption of the room data */
						if (freeuint323 == 0x000003FF)
						{
							freeuint323 = 0x00000000;
						}
					}
					
					/* Stores the texture */
					freeuint323 &= 0x000003FF;
					freeuint164 = (INTU16) chunkrects;
					freeuint164 -= (freeuint164 % 0x0003);
					freeuint164 /= 0x0003;
					freeuint164 *= 0x0004;
					freeuint165 = (INTU16) chunkrects;
					freeuint165 %= 0x0003;
					freeuint323 <<= (freeuint165 * 0x000A);
					psRects[freeuint164] |= freeuint323;
					freeuint164 = (INTU16) chunkrects;
					if ((freeuint164 % 0x0003) == 0x0002)
					{
						chunkSize += 0x00000010;
					}
					++chunkrects;
				}
			}
		}
		
		/* Checks there is space for another chunk */
		if (numChunks == 0x00000010)
		{
			/* Prints error message here, to include the room number */
			printf(PROGNAME ": Room " PRINTU16 " too big to be converted\n",
			       curroom);
			errorNumber = ERROR_TOO_MANY_CHUNKS;
			goto end;
		}
		
		/* Ends this chunk if there was one */
		if (chunkverts != (INTU8) 0x00)
		{
			freeuint321 = 0x000003FF;
			freeuint164 = (INTU16) chunkrects;
			freeuint164 %= 0x0003;
			freeuint321 <<= (freeuint164 * 0x000A);
			freeuint164 = (INTU16) chunkrects;
			freeuint164 -= (freeuint164 % 0x0003);
			freeuint164 /= 0x0003;
			freeuint164 *= 0x0004;
			psRects[freeuint164] |= freeuint321;
			
			/* Makes room for the chunk */
			errorNumber = insertbytes(psx, (INTU32) psx_curpos, chunkSize);
			if (errorNumber != ERROR_NONE)
			{
				goto end;
			}
			level_info_add(psx_info, (INTU32) psx_curpos, chunkSize);
			numData += chunkSize;
			
			/* Remembers that the chunk was added */
			chunkSize += chunkOffset[numChunks];
			++numChunks;
			if (numChunks < 0x00000010)
			{
				chunkOffset[numChunks] = chunkSize;
			}
			
			/* Adjusts for Big-Endian */
#ifdef __BIG_ENDIAN__
			freeuint164 = (INTU16) chunkrects;
			freeuint164 += ((freeuint164 - (freeuint164 % 0x0003)) / 0x0003);
			++freeuint164;
			for (freeuint163 = 0x0000; freeuint163 < freeuint164; ++freeuint163)
			{
				psRects[freeuint163] = reverse32(psRects[freeuint163]);
			}
#endif
			
			/* Makes the bounding box */
			boxminx <<= 8U;
			boxminy <<= 8U;
			boxminz <<= 8U;
			boxmaxx <<= 8U;
			boxmaxy <<= 8U;
			boxmaxz <<= 8U;
			/* Checks whether this bounding box's min already exists */
			freeuint321 = 0x00000000;
			freeuint322 = (numroomboxes * 0x00000004);
			while (freeuint321 < freeuint322)
			{
				if ((roomboxes[freeuint321] == boxminx) &&
				    (roomboxes[(freeuint321 + 0x00000001)] == boxminy) &&
				    (roomboxes[(freeuint321 + 0x00000002)] == boxminz))
				{
					break;
				}
				
				/* Moves to the next vertex */
				freeuint321 += 0x00000004;
			}
			if (freeuint321 == freeuint322)
			{
				roomboxes[freeuint321] = boxminx;
				roomboxes[(freeuint321 + 0x00000001)] = boxminy;
				roomboxes[(freeuint321 + 0x00000002)] = boxminz;
				++numroomboxes;
			}
			freeuint164 = (INTU16) (freeuint321 >> 2U);
			
			/* Writes BoundMinIndex */
#ifdef __BIG_ENDIAN__
			freeuint164 = reverse16(freeuint164);
#endif
			freeint1 = fseek(psx, psx_curpos, SEEK_SET);
			if (freeint1 != 0)
			{
				errorNumber = ERROR_PSX_READ_FAILED;
				goto end;
			}
			freesizet1 = fwrite(&freeuint164, (size_t) 1, (size_t) 2, psx);
			if (freesizet1 != (size_t) 2)
			{
				errorNumber = ERROR_PSX_WRITE_FAILED;
				goto end;
			}
			psx_curpos += 2;
			
			/* Checks whether this bounding box's max already exists */
			freeuint321 = 0x00000000;
			freeuint322 = (numroomboxes * 0x00000004);
			while (freeuint321 < freeuint322)
			{
				if ((roomboxes[freeuint321] == boxmaxx) &&
				    (roomboxes[(freeuint321 + 0x00000001)] == boxmaxy) &&
				    (roomboxes[(freeuint321 + 0x00000002)] == boxmaxz))
				{
					break;
				}
				
				/* Moves to the next vertex */
				freeuint321 += 0x00000004;
			}
			if (freeuint321 == freeuint322)
			{
				roomboxes[freeuint321] = boxmaxx;
				roomboxes[(freeuint321 + 0x00000001)] = boxmaxy;
				roomboxes[(freeuint321 + 0x00000002)] = boxmaxz;
				++numroomboxes;
			}
			freeuint164 = (INTU16) (freeuint321 >> 2U);
			
			/* Writes BoundMaxIndex */
#ifdef __BIG_ENDIAN__
			freeuint164 = reverse16(freeuint164);
#endif
			freeint1 = fseek(psx, psx_curpos, SEEK_SET);
			if (freeint1 != 0)
			{
				errorNumber = ERROR_PSX_READ_FAILED;
				goto end;
			}
			freesizet1 = fwrite(&freeuint164, (size_t) 1, (size_t) 2, psx);
			if (freesizet1 != (size_t) 2)
			{
				errorNumber = ERROR_PSX_WRITE_FAILED;
				goto end;
			}
			psx_curpos += 2;
			
			/* Writes Room Number */
			freeuint81 = (INTU8) curroom;
			freesizet1 = fwrite(&freeuint81, (size_t) 1, (size_t) 1, psx);
			if (freesizet1 != (size_t) 1)
			{
				errorNumber = ERROR_PSX_WRITE_FAILED;
				goto end;
			}
			++psx_curpos;
			
			/* Writes room flags */
			freeuint81 = (INTU8) 0x00;
			freesizet1 = fwrite(&freeuint81, (size_t) 1, (size_t) 1, psx);
			if (freesizet1 != (size_t) 1)
			{
				errorNumber = ERROR_PSX_WRITE_FAILED;
				goto end;
			}
			++psx_curpos;
			
			/* Writes NumVertices */
			freesizet1 = fwrite(&chunkverts, (size_t) 1, (size_t) 1, psx);
			if (freesizet1 != (size_t) 1)
			{
				errorNumber = ERROR_PSX_WRITE_FAILED;
				goto end;
			}
			++psx_curpos;
			
			/* Writes NumTriangles */
			freesizet1 = fwrite(&chunktris, (size_t) 1, (size_t) 1, psx);
			if (freesizet1 != (size_t) 1)
			{
				errorNumber = ERROR_PSX_WRITE_FAILED;
				goto end;
			}
			++psx_curpos;
			
			/* Writes Vertices */
			freeuint321 = (INTU32) chunkverts;
			freeuint321 &= 0x0000007F;
			freeuint321 *= 0x00000004;
			freesizet1 = fwrite(psVerts, (size_t) 1, (size_t) freeuint321, psx);
			if (freesizet1 != (size_t) freeuint321)
			{
				errorNumber = ERROR_PSX_WRITE_FAILED;
				goto end;
			}
			psx_curpos += (long int) freeuint321;
			
			/* Writes Triangles */
			freeuint321 = (INTU32) chunktris;
			freeuint321 &= 0x000000FF;
			freeuint321 *= 0x00000004;
			freesizet1 = fwrite(psTris, (size_t) 1, (size_t) freeuint321, psx);
			if (freesizet1 != (size_t) freeuint321)
			{
				errorNumber = ERROR_PSX_WRITE_FAILED;
				goto end;
			}
			psx_curpos += (long int) freeuint321;
			
			/* Writes Rectangles */
			freeuint164 = (INTU16) chunkrects;
			freeuint164 -= (freeuint164 % 0x0003);
			freeuint164 /= 0x0003;
			++freeuint164;
			freeuint321 = (((INTU32) freeuint164) * 0x00000010);
			freesizet1 = fwrite(psRects, (size_t) 1, (size_t) freeuint321, psx);
			if (freesizet1 != (size_t) freeuint321)
			{
				errorNumber = ERROR_PSX_WRITE_FAILED;
				goto end;
			}
			psx_curpos += (long int) freeuint321;
		}
		
		/* Adjusts for Big Endian */
#ifdef __BIG_ENDIAN__
		numData = reverse32(numData);
		numChunks = reverse32(numChunks);
		for (freeuint81 = (INTU8) 0x00; freeuint81 < (INTU8) 0x10; ++freeuint81)
		{
			chunkOffset[freeuint81] = reverse32(chunkOffset[freeuint81]);
		}
#endif
		
		/* Writes NumData */
		numData >>= 1U;
		freeint1 = fseek(psx, psx_curpos2, SEEK_SET);
		if (freeint1 != 0)
		{
			errorNumber = ERROR_PSX_READ_FAILED;
			goto end;
		}
		freesizet1 = fwrite(&numData, (size_t) 1, (size_t) 4, psx);
		if (freesizet1 != (size_t) 4)
		{
			errorNumber = ERROR_PSX_WRITE_FAILED;
			goto end;
		}
		
		/* Writes NumChunks */
		freesizet1 = fwrite(&numChunks, (size_t) 1, (size_t) 4, psx);
		if (freesizet1 != (size_t) 4)
		{
			errorNumber = ERROR_PSX_WRITE_FAILED;
			goto end;
		}
		
		/* Writes ChunkIndices */
		freesizet1 = fwrite(&chunkOffset, (size_t) 1, (size_t) 64, psx);
		if (freesizet1 != (size_t) 64)
		{
			errorNumber = ERROR_PSX_WRITE_FAILED;
			goto end;
		}
		
		/* Makes space for the rest of the room */
		freeuint321 = ((room[curroom].p_AlternateRoom -
		                room[curroom].p_NumDoors) + 0x00000007);
		errorNumber = insertbytes(psx, (INTU32) psx_curpos, freeuint321);
		if (errorNumber != ERROR_NONE)
		{
			goto end;
		}
		level_info_add(psx_info, (INTU32) psx_curpos, freeuint321);
		
		/* Copies NumDoors */
		errorNumber = copybytes(pc, psx, room[curroom].p_NumDoors,
		                        (INTU32) psx_curpos, (INTU32) 0x00000002);
		if (errorNumber != ERROR_NONE)
		{
			goto end;
		}
		pc_curpos = (long int) (room[curroom].p_NumDoors + 0x00000002);
		psx_curpos += 2;
		
		/* Converts doors */
		for (freeuint161 = 0; freeuint161 < room[curroom].v_NumDoors;
		     ++freeuint161)
		{
			/* Reads door into memory */
			freeint1 = fseek(pc, pc_curpos, SEEK_SET);
			if (freeint1 != 0)
			{
				errorNumber = ERROR_PC_READ_FAILED;
				goto end;
			}
			freesizet1 = fread(pcVerts, 1, 32, pc);
			if (freesizet1 != 32)
			{
				errorNumber = ERROR_PC_READ_FAILED;
				goto end;
			}
			
			/* Makes Y-coordinates local */
			memcpy(&freeint161, &pcVerts[10], 2);
#ifdef __BIG_ENDIAN__
			freeint161 = reverse16(freeint161);
#endif
			freeint161 -= roomY;
#ifdef __BIG_ENDIAN__
			freeint161 = reverse16(freeint161);
#endif
			memcpy(&pcVerts[10], &freeint161, 2);
			memcpy(&freeint161, &pcVerts[16], 2);
#ifdef __BIG_ENDIAN__
			freeint161 = reverse16(freeint161);
#endif
			freeint161 -= roomY;
#ifdef __BIG_ENDIAN__
			freeint161 = reverse16(freeint161);
#endif
			memcpy(&pcVerts[16], &freeint161, 2);
			memcpy(&freeint161, &pcVerts[22], 2);
#ifdef __BIG_ENDIAN__
			freeint161 = reverse16(freeint161);
#endif
			freeint161 -= roomY;
#ifdef __BIG_ENDIAN__
			freeint161 = reverse16(freeint161);
#endif
			memcpy(&pcVerts[22], &freeint161, 2);
			memcpy(&freeint161, &pcVerts[28], 2);
#ifdef __BIG_ENDIAN__
			freeint161 = reverse16(freeint161);
#endif
			freeint161 -= roomY;
#ifdef __BIG_ENDIAN__
			freeint161 = reverse16(freeint161);
#endif
			memcpy(&pcVerts[28], &freeint161, 2);
			
			/* Writes the door to PSX */
			freeint1 = fseek(psx, psx_curpos, SEEK_SET);
			if (freeint1 != 0)
			{
				errorNumber = ERROR_PSX_READ_FAILED;
				goto end;
			}
			freesizet1 = fwrite(pcVerts, 1, 32, psx);
			if (freesizet1 != 32)
			{
				errorNumber = ERROR_PSX_WRITE_FAILED;
				goto end;
			}
			
			/* Moves on to next triangle */
			pc_curpos += 32;
			psx_curpos += 32;
		}
		
		/* Copies Sector Data, Lights, Static Meshes and Flags */
		freeuint321 = ((room[curroom].p_AlternateRoom -
		                room[curroom].p_NumZSectors) + 0x00000007);
		errorNumber = copybytes(pc, psx, (INTU32) room[curroom].p_NumZSectors,
		                        (INTU32) psx_curpos, freeuint321);
		if (errorNumber != ERROR_NONE)
		{
			goto end;
		}
		
		/* Converts RoomLight */
		psx_curpos += ((room[curroom].p_NumLights -
		                room[curroom].p_NumZSectors) - 0x00000004);
		freeint1 = fseek(pc, (((long int) room[curroom].p_NumLights) - 4l),
		                 SEEK_SET);
		if (freeint1 != 0)
		{
			errorNumber = ERROR_PC_READ_FAILED;
			goto end;
		}
		freesizet1 = fread(&freeuint161, 1, 2, pc);
		if (freesizet1 != (size_t) 2)
		{
			errorNumber = ERROR_PC_READ_FAILED;
			goto end;
		}
#ifdef __BIG_ENDIAN__
		freeuint161 = reverse16(freeuint161);
#endif
		freeuint161 = (0x1FFF - freeuint161);
		freeuint161 >>= 1U;
#ifdef __BIG_ENDIAN__
		freeuint161 = reverse16(freeuint161);
#endif
		freeint1 = fseek(psx, (long int) psx_curpos, SEEK_SET);
		if (freeint1 != 0)
		{
			errorNumber = ERROR_PSX_READ_FAILED;
			goto end;
		}
		freesizet1 = fwrite(&freeuint161, 1, 2, psx);
		if (freesizet1 != (size_t) 2)
		{
			errorNumber = ERROR_PSX_WRITE_FAILED;
			goto end;
		}
		
		/* Converts the falloff-values for the light structs */
		psx_curpos2 = (psx_curpos + 4l);
		freeint1 = fseek(psx, psx_curpos2, SEEK_SET);
		if (freeint1 != 0)
		{
			errorNumber = ERROR_PSX_READ_FAILED;
			goto end;
		}
		freesizet1 = fread(&freeuint161, 1, 2, psx);
		if (freesizet1 != (size_t) 2)
		{
			errorNumber = ERROR_PSX_READ_FAILED;
			goto end;
		}
#ifdef __BIG_ENDIAN__
		freeuint161 = reverse16(freeuint161);
#endif
		psx_curpos2 += 17l;
		for (; freeuint161 > 0x0000; --freeuint161)
		{
			/* Checks the light type */
			freeint1 = fseek(psx, psx_curpos2, SEEK_SET);
			if (freeint1 != 0)
			{
				errorNumber = ERROR_PSX_READ_FAILED;
				goto end;
			}
			freesizet1 = fread(&freeuint81, 1, 1, psx);
			if (freesizet1 != (size_t) 1)
			{
				errorNumber = ERROR_PSX_READ_FAILED;
				goto end;
			}
			psx_curpos2 += 5l;
			if (freeuint81 == (INTU8) 0x00)
			{
				freeint1 = fseek(psx, psx_curpos2, SEEK_SET);
				if (freeint1 != 0)
				{
					errorNumber = ERROR_PSX_READ_FAILED;
					goto end;
				}
				freesizet1 = fread(&freeuint321, 1, 4, psx);
				if (freesizet1 != (size_t) 4)
				{
					errorNumber = ERROR_PSX_READ_FAILED;
					goto end;
				}
#ifdef __BIG_ENDIAN__
				freeuint321 = reverse32(freeuint321);
#endif
				freeuint321 <<= 2U;
#ifdef __BIG_ENDIAN__
				freeuint321 = reverse32(freeuint321);
#endif
				freeint1 = fseek(psx, psx_curpos2, SEEK_SET);
				if (freeint1 != 0)
				{
					errorNumber = ERROR_PSX_READ_FAILED;
					goto end;
				}
				freesizet1 = fwrite(&freeuint321, 1, 4, psx);
				if (freesizet1 != (size_t) 4)
				{
					errorNumber = ERROR_PSX_WRITE_FAILED;
					goto end;
				}
			}
			psx_curpos2 += 19l;
		}
		
		/* Moves on to the next room */
		psx_curpos += ((room[curroom].p_AlternateRoom -
		                room[curroom].p_NumLights) + 0x0000000B);
	}
	
	/* Look at the earlier note to self */
	--(psx_info->p_NumFloorData);
	
	/* Adjusts for Big-Endian */
#ifdef __BIG_ENDIAN__
	freeuint321 = 0x00000000;
	freeuint322 = (numroomboxes * 0x00000004);
	while (freeuint321 < freeuint322)
	{
		roomboxes[freeuint321] = reverse16(roomboxes[freeuint321]);
		++freeuint321;
		roomboxes[freeuint321] = reverse16(roomboxes[freeuint321]);
		++freeuint321;
		roomboxes[freeuint321] = reverse16(roomboxes[freeuint321]);
		freeuint321 += 0x00000002;
	}
#endif
	
	/* Adjusts the space for the RoomMeshBoxes */
	if (psx_info->v_NumRoomMeshBoxes > numroomboxes)
	{
		freeuint321 = (psx_info->v_NumRoomMeshBoxes - numroomboxes);
		freeuint321 *= 0x00000008;
		errorNumber = removebytes(psx, psxpath, psx_info->p_NumRoomMeshBoxes,
		                          freeuint321);
		if (errorNumber != ERROR_NONE)
		{
			goto end;
		}
		level_info_sub(psx_info, psx_info->p_NumRoomMeshBoxes, freeuint321);
	}
	else if (psx_info->v_NumRoomMeshBoxes < numroomboxes)
	{
		freeuint321 = (numroomboxes - psx_info->v_NumRoomMeshBoxes);
		freeuint321 *= 0x00000008;
		errorNumber = insertbytes(psx, psx_info->p_NumRoomMeshBoxes,
		                          freeuint321);
		if (errorNumber != ERROR_NONE)
		{
			goto end;
		}
		level_info_add(psx_info, psx_info->p_NumRoomMeshBoxes, freeuint321);
	}
	psx_info->v_NumRoomMeshBoxes = numroomboxes;
	
	/* Writes NumRoomMeshBoxes */
	freeuint321 = numroomboxes;
#ifdef __BIG_ENDIAN__
	freeuint321 = reverse32(freeuint321);
#endif
	freeint1 = fseek(psx, (long int) psx_info->p_NumRoomMeshBoxes, SEEK_SET);
	if (freeint1 != 0)
	{
		errorNumber = ERROR_PSX_READ_FAILED;
		goto end;
	}
	freesizet1 = fwrite(&freeuint321, (size_t) 1, (size_t) 4, psx);
	if (freesizet1 != (size_t) 4)
	{
		errorNumber = ERROR_PSX_WRITE_FAILED;
		goto end;
	}
	
	/* Writes RoomMeshBoxes */
	freeuint321 = (numroomboxes * 0x00000008);
	freesizet1 = fwrite(roomboxes, (size_t) 1, (size_t) freeuint321, psx);
	if (freesizet1 != (size_t) freeuint321)
	{
		errorNumber = ERROR_PSX_WRITE_FAILED;
		goto end;
	}
	
end:/* Frees allocated memory and ends the function */
	if (vertices != NULL)
	{
		free(vertices);
	}
	if (rectangles != NULL)
	{
		free(rectangles);
	}
	if (triangles != NULL)
	{
		free(triangles);
	}
	if (pcVerts != NULL)
	{
		free(pcVerts);
	}
	if (pcRects != NULL)
	{
		free(pcRects);
	}
	if (pcTris != NULL)
	{
		free(pcTris);
	}
	if (psVerts != NULL)
	{
		free(psVerts);
	}
	if (psRects != NULL)
	{
		free(psRects);
	}
	if (psTris != NULL)
	{
		free(psTris);
	}
	if (roomboxes != NULL)
	{
		free(roomboxes);
	}
	
	return errorNumber;
}

/*
 * Function that builds the OutsideRoomTable
 * Parameters:
 * * psx = Pointer to the PSX-file
 * * numRooms = Number of rooms in pc
 * * room = Pointer to room struct in pc_info (only read from)
 * * psx_info = Struct containing values and offsets from PSX
 */
static int buildOutsideTable(FILE *psx, INTU16 numRooms, struct room_info *room,
                             struct level_info *psx_info)
{
	/* Variable Declarations */
	INTU8 *outsideRoomTable = NULL;
	INTU16 *outsideRoomOffsets = NULL;
	INT32 x, y;
	INT32 rx, ry, j, xl, yl;
	INT32 z;
	INT32 z2;
	INTU16 i;
	INTU8 *d = NULL;
	INTU32 p = 0x00000000;
	INTU32 s = 0x00000000;
	INT32 max_slots = 0x00000000;
	INTU32 freeuint321;
	size_t freesizet1;
	int freeint1;
	int errorNumber = ERROR_NONE;
	
	/* Allows only 255 rooms */
	if (numRooms > 0x00FF)
	{
		PRINT_WARNING;
		printf("All rooms above 254 will be excluded from the OutsideTable\n");
		numRooms = 0x00FF;
	}
	
	/* Allocates outsideRoomTable and outsideRoomOffsets */
	outsideRoomTable = malloc(46656);
	if (outsideRoomTable == NULL)
	{
		errorNumber = ERROR_MEMORY;
		goto end;
	}
	outsideRoomOffsets = malloc(1458);
	if (outsideRoomOffsets == NULL)
	{
		errorNumber = ERROR_MEMORY;
		goto end;
	}
	
	/* Wipes outsideRoomTable and outsideRoomOffsets */
	(void) memset(outsideRoomTable, 0xFF, 46656);
	(void) memset(outsideRoomOffsets, 0x00, 1458);
	
	for (y = 0x00000000; y < 0x0000006C; y += 0x00000004)
	{
		for (x = 0x00000000; x < 0x0000006C; x += 0x00000004)
		{
			for (i = 0x0000; i < numRooms; ++i)
			{
				rx = ((room[i].z >> 10U) + 0x00000001);
				ry = ((room[i].x >> 10U) + 0x00000001);
				
				j = 0x00000000;
				for (yl = 0x00000000; yl < 0x00000004; ++yl)
				{
					for (xl = 0x00000000; xl < 0x00000004; ++xl)
					{
						if (((x + xl) >= rx) &&
						    ((x + xl) <
						     (rx + (INT32) (room[i].v_NumZSectors - 0x0002))) &&
						    ((y + yl) >= ry) &&
						    ((y + yl) <
						     (ry + (INT32) (room[i].v_NumXSectors - 0x0002))))
						{
							j = 0x00000001;
							break;
						}
					}
				}
				
				if (j == 0x00000000)
				{
					continue;
				}
				
				j = (((x >> 2U) * 0x00000040) + ((y >> 2U) * 0x000006C0));
				d = &outsideRoomTable[j];
				for (j = 0x00000000; j < 0x00000040; ++j)
				{
					if (d[j] == (INTU8) 0xFF)
					{
						d[j] = (INTU8) i;
						if (j < max_slots)
						{
							max_slots = j;
						}
						break;
					}
				}
				
				if (j == 0x00000040)
				{
					errorNumber = ERROR_ROOM_TABLE_CLASH;
					goto end;
				}
			}
		}
	}
	d = NULL;
	
	s = 0x00000000;
	for (y = 0x00000000; y < 0x0000001B; ++y)
	{
		for (x = 0x00000000; x < 0x0000001B; ++x)
		{
			z = 0x00000000;
			i = (INTU16) ((y * 0x0000001B) + x);
			j = ((x * 0x00000040) + (y * 0x000006C0));
			d = &outsideRoomTable[j];
			
			while (d[z] != (INTU8) 0xFF)
			{
				++z;
			}
			
			if (z == 0x00000000)
			{
				outsideRoomOffsets[i] = 0xFFFF;
			}
			else if (z == 0x00000001)
			{
				outsideRoomOffsets[i] = (INTU16) *d;
				outsideRoomOffsets[i] &= 0x00FF;
				outsideRoomOffsets[i] |= 0x8000;
			}
			else
			{
				p = 0x00000000;
				while (p < s)
				{
					if (memcmp(&outsideRoomTable[p], d, (size_t) z) == 0)
					{
						outsideRoomOffsets[i] = (INTU16) p;
						break;
					}
					else
					{
						z2 = 0x00000000;
						while (outsideRoomTable[(p + z2)] != (INTU8) 0xFF)
						{
							++z2;
						}
						p += z2;
						++p;
					}
				}
				
				if (p >= s)
				{
					outsideRoomOffsets[i] = (INTU16) s;
					for (; z != 0x00000000; z--)
					{
						outsideRoomTable[s] = *d;
						++d;
						++s;
					}
					outsideRoomTable[s] = (INTU8) 0xFF;
					++s;
				}
			}
		}
	}
	
	/* Adjusts for Big-Endian if needed */
#ifdef __BIG_ENDIAN__
	for (freeuint321 = 0x00000000; freeuint321 < 0x000002D9; ++freeuint321)
	{
		outsideRoomOffsets[freeuint321] =
			reverse16(outsideRoomOffsets[freeuint321]);
	}
#endif
	
	/* Makes room for the OutsideRoomTable */
	errorNumber = insertbytes(psx, psx_info->p_OutRoomTableLen, s);
	if (errorNumber != ERROR_NONE)
	{
		goto end;
	}
	level_info_add(psx_info, psx_info->p_OutRoomTableLen, s);
	
	/* Writes the OutsideRoomOffsets to PSX */
	freeuint321 = (psx_info->p_OutRoomTableLen - 0x000005B2);
	freeint1 = fseek(psx, (long int) freeuint321, SEEK_SET);
	if (freeint1 != 0)
	{
		errorNumber = ERROR_PSX_READ_FAILED;
		goto end;
	}
	freesizet1 = fwrite(outsideRoomOffsets, (size_t) 1, (size_t) 0x05B2, psx);
	if (freesizet1 != (size_t) 0x05B2)
	{
		errorNumber = ERROR_PSX_WRITE_FAILED;
		goto end;
	}
	
	/* Writes OutRoomTableLen */
	psx_info->v_OutRoomTableLen = s;
#ifdef __BIG_ENDIAN__
	s = reverse32(s);
#endif
	freesizet1 = fwrite(&s, (size_t) 1, (size_t) 4, psx);
	if (freesizet1 != (size_t) 4)
	{
		errorNumber = ERROR_PSX_WRITE_FAILED;
		goto end;
	}
	
	/* Writes OutsideRoomTable */
	freesizet1 = fwrite(outsideRoomTable, (size_t) 1,
	                    (size_t) psx_info->v_OutRoomTableLen, psx);
	if (freesizet1 != (size_t) psx_info->v_OutRoomTableLen)
	{
		errorNumber = ERROR_PSX_WRITE_FAILED;
		goto end;
	}
	
end:/* Frees allocated memory and ends the function */
	if (outsideRoomTable != NULL)
	{
		free(outsideRoomTable);
	}
	if (outsideRoomOffsets != NULL)
	{
		free(outsideRoomOffsets);
	}
	return errorNumber;
}

/*
 * Function that converts Mesh Pointers from PC to PSX.
 * (And it causes conversion of Mesh Data)
 * Parameters:
 * * pc = Pointer to PC-file
 * * psx = Pointer to PSX-file
 * * psxpath = Path to PSX-file
 * * pc_info = Pointer to the level_info struct relating to PC
 * * psx_info = Pointer to the level_info struct relating to PSX
 * * flags = Bit-mask of program-wide flags
 * * numMisorientedTextures = Number of misoriented textures
 * * misorientedTextures = List of misoriented textures
 * * objectTextures = Conversion table of object textures
 */
static int convertMeshPointers(FILE *pc, FILE *psx, char *psxpath,
                               struct level_info *pc_info,
                               struct level_info *psx_info, INTU32 flags,
                               INTU16 numMisorientedTextures,
                               INTU16 *misorientedTextures,
                               INTU16 *objectTextures)
{
	/* Variable Declarations */
	INTU32 *pc_meshPointers = NULL; /* Mesh pointers from pc in memory */
	INTU32 curPointer;              /* Current Mesh Pointer in the conversion */
	INTU32 pointTodo;               /* Number of pointers to be converted */
	INT32 highestDone;              /* Highest converted value */
	INT32 lowestFound;              /* Lowest found unconverted mesh pointer */
	INTU32 psx_curPointer;          /* Current mesh offset in PSX */
	INTU32 skybox;                  /* Which mesh pointer is the skybox */
	INTU32 freeuint321;             /* 32-bit unsigned integer (general use) */
	INT32 freeint321;               /* 32-bit signed integer (general use) */
	INTU16 freeuint161;             /* 16-bit unsigned integer (general use) */
	int freeint1;                   /* Integer (general use) */
	long int freelong1;             /* Long integer (general use) */
	size_t freesizet1;              /* size_t (general use) */
	int errorNumber;                /* Return value for this function */
	errorNumber = ERROR_NONE;
	
	/* Removes all mesh data from PSX */
	freeuint321 = (psx_info->v_NumMeshData * 2);
	errorNumber = removebytes(psx, psxpath, psx_info->p_NumMeshData,
	                          freeuint321);
	if (errorNumber != ERROR_NONE)
	{
		return errorNumber;
	}
	level_info_sub(psx_info, psx_info->p_NumMeshData, freeuint321);
	
	/* Adjusts space for Mesh Pointers in PSX to equal PC */
	if (pc_info->v_NumMeshPointers > psx_info->v_NumMeshPointers)
	{
		freeuint321 = (pc_info->v_NumMeshPointers -
		               psx_info->v_NumMeshPointers);
		freeuint321 *= 0x00000004;
		errorNumber = insertbytes(psx, psx_info->p_NumMeshPointers,
		                          freeuint321);
		if (errorNumber != ERROR_NONE)
		{
			return errorNumber;
		}
		level_info_add(psx_info, psx_info->p_NumMeshPointers, freeuint321);
	}
	else if (pc_info->v_NumMeshPointers < psx_info->v_NumMeshPointers)
	{
		freeuint321 = (psx_info->v_NumMeshPointers -
		               pc_info->v_NumMeshPointers);
		freeuint321 *= 4;
		errorNumber = removebytes(psx, psxpath, psx_info->p_NumMeshPointers,
		                          freeuint321);
		if (errorNumber != ERROR_NONE)
		{
			return errorNumber;
		}
		level_info_sub(psx_info, psx_info->p_NumMeshPointers, freeuint321);
	}
	
	/* Attempts to allocate space in memory for mesh pointers from PC */
	pc_meshPointers = calloc((size_t) pc_info->v_NumMeshPointers, 4);
	if (pc_meshPointers != NULL)
	{
		/* Loads mesh pointers from PC into memory */
		freelong1 = (long int) (pc_info->p_NumMeshPointers + 4);
		for (curPointer = 0; curPointer < pc_info->v_NumMeshPointers;
		     ++curPointer)
		{
			freeint1 = fseek(pc, freelong1, SEEK_SET);
			if (freeint1 != 0)
			{
				errorNumber = ERROR_PC_READ_FAILED;
				goto end;
			}
			freesizet1 = fread(&pc_meshPointers[curPointer], 1, 4, pc);
			if (freesizet1 != 4)
			{
				errorNumber = ERROR_PC_READ_FAILED;
				goto end;
			}
#ifdef __BIG_ENDIAN__
			pc_meshPointers[curPointer] =
				reverse32(pc_meshPointers[curPointer]);
#endif
			
			freelong1 += 4;
		}
	}
	else
	{
		errorNumber = ERROR_MEMORY;
		goto end;
	}
	
	/* Finds out which mesh is the skybox */
	freeuint161 = findMesh(pc, pc_info, 0x00000163);
	if (freeuint161 != 0xFFFF)
	{
		skybox = pc_meshPointers[freeuint161];
	}
	else
	{
		skybox = 0xFFFF;
	}
	
	/* Converts mesh pointers */
	psx_curPointer = 0;
	pointTodo = pc_info->v_NumMeshPointers;
	highestDone = -1;
	while (pointTodo > 0)
	{
		/* Finds lowest unconverted mesh pointer */
		lowestFound = 0x7FFFFFFF;
		for (curPointer = 0; curPointer < pc_info->v_NumMeshPointers;
		     ++curPointer)
		{
			/* Loads current mesh pointer into memory */
			freeint321 = (INT32) pc_meshPointers[curPointer];
			
			if ((freeint321 > highestDone) && (freeint321 < lowestFound))
			{
				lowestFound = freeint321;
			}
		}
		
		/* Converts all mesh pointers equal to the lowest found mesh pointer */
		for (curPointer = 0; curPointer < pc_info->v_NumMeshPointers;
		     ++curPointer)
		{
			/* Loads current mesh pointer into memory */
			freeint321 = (INT32) pc_meshPointers[curPointer];
			
			/* Converts the mesh pointer if equal to the lowest unconverted */
			if (freeint321 == lowestFound)
			{
				/* Determines value the mesh pointer should be converted to */
#ifdef __BIG_ENDIAN__
				psx_curPointer = reverse32(psx_curPointer);
#endif
				
				/* Writes value to correct location in PSX */
				freelong1 = (long int) (psx_info->p_NumMeshPointers + 4 +
				                        (curPointer * 4));
				freeint1 = fseek(psx, freelong1, SEEK_SET);
				if (freeint1 != 0)
				{
					errorNumber = ERROR_PSX_READ_FAILED;
					goto end;
				}
				freesizet1 = fwrite(&psx_curPointer, 1, 4, psx);
				if (freesizet1 != 4)
				{
					errorNumber = ERROR_PSX_WRITE_FAILED;
					goto end;
				}
				
#ifdef __BIG_ENDIAN__
				psx_curPointer = reverse32(psx_curPointer);
#endif
				
				--pointTodo;
			}
		}
		
		/* Converts mesh */
		freelong1 = (long int) (pc_info->p_NumMeshData + 4 + lowestFound);
		if (lowestFound == (INT32) skybox)
		{
			freeint1 = 1;
		}
		else
		{
			freeint1 = 0;
		}
		errorNumber = convertMesh(pc, psx, psx_info, (INT32) freelong1,
		                          &psx_curPointer, flags, freeint1,
		                          numMisorientedTextures, misorientedTextures,
		                          objectTextures);
		if (errorNumber != ERROR_NONE)
		{
			goto end;
		}
		
		/* Moves on to the next mesh pointer */
		highestDone = lowestFound;
	}
	
	/* Changes NumMeshPointers in PSX to correct value */
	freeuint321 = pc_info->v_NumMeshPointers;
#ifdef __BIG_ENDIAN__
	freeuint321 = reverse32(freeuint321);
#endif
	freeint1 = fseek(psx, (long int) psx_info->p_NumMeshPointers, SEEK_SET);
	if (freeint1 != 0)
	{
		errorNumber = ERROR_PSX_READ_FAILED;
		goto end;
	}
	freesizet1 = fwrite(&freeuint321, 1, 4, psx);
	if (freesizet1 != 4)
	{
		errorNumber = ERROR_PSX_WRITE_FAILED;
		goto end;
	}
	
	/* Changes NumMeshData in PSX to correct value */
	freeuint321 = (psx_curPointer / 2);
#ifdef __BIG_ENDIAN__
	freeuint321 = reverse32(freeuint321);
#endif
	freeint1 = fseek(psx, (long int) psx_info->p_NumMeshData, SEEK_SET);
	if (freeint1 != 0)
	{
		errorNumber = ERROR_PSX_READ_FAILED;
		goto end;
	}
	freesizet1 = fwrite(&freeuint321, 1, 4, psx);
	if (freesizet1 != 4)
	{
		errorNumber = ERROR_PSX_WRITE_FAILED;
		goto end;
	}
	
end:/* Frees memory and returns error number */
	if (pc_meshPointers != NULL)
	{
		free(pc_meshPointers);
	}
	return errorNumber;
}

/*
 * Function that converts a mesh from PC to PSX.
 * Parameters:
 * * pc = Pointer to PC-file
 * * psx = Pointer to PSX-file
 * * psx_info = Pointer to info about PSX-file
 * * pc_MeshPos = Offset of the mesh to be converted in PC
 * * psx_curPointer = Pointer to current offset of next mesh in PSX
 * * flags = Bit-mask of program-wide flags
 * * skybox = If 1, the current mesh is the skybox, else 0
 * * numMisorientedTextures = Number of misoriented textures
 * * misorientedTextures = List of misoriented textures
 * * objectTextures = Conversion table of object textures
 */
static int convertMesh(FILE *pc, FILE *psx, struct level_info *psx_info,
                       INT32 pc_MeshPos, INTU32 *psx_curPointer, INTU32 flags,
                       int skybox, INTU16 numMisorientedTextures,
                       INTU16 *misorientedTextures,
                       INTU16 *objectTextures)
{
	/* Variable Declarations */
	INTU8 buffer[32];     /* Buffer space for conversions */
	INT16 pc_NumVertices; /* Number of vertices in PC-mesh */
	INT16 pc_NumNormals;  /* Number of normals in PC-mesh */
	INT16 pc_NumTexturedRectangles; /* Number of textured rectangles */
	INT16 pc_NumTexturedTriangles;  /* Number of textured triangles */
	INT16 pc_NumColouredRectangles; /* Number of coloured rectangles */
	INT16 pc_NumColouredTriangles;  /* Number of coloured triangles */
	INT16 psx_NumNormals;    /* Number of normals in PSX-mesh */
	INT16 psx_NumRectangles; /* Number of rectangles in PSX-mesh */
	INT16 psx_NumTriangles;  /* Number of triangles in PSX-mesh */
	long int pc_PosTexturedTriangles; /* Offset of textured triangles */
	long int pc_PosColouredTriangles; /* Offset of coloured triangles */
	long int pc_PosTexturedRectangles; /* Offset of textured rectangles */
	long int pc_PosColouredRectangles; /* Offset of coloured rectangles */
	long int pc_offset;  /* Current offset in PC */
	long int psx_offset; /* Current offset in PSX */
	long int pc_curlight; /* Offset of the current light value to use */
	INTU16 curfaces;     /* Number of current faces */
	int doublesided = 0; /* Whether this polygon is double sided */
	INTU32 freeuint321;  /* Unsigned 32-bit integer for general use */
	INTU16 freeuint161,
	       freeuint162,
	       freeuint163,
	       freeuint165; /* Unsigned 16-bit integers for general use */
	long int freelong1; /* Long integer for general use */
	size_t freesizet1;  /* size_t for general use */
	int freeint1,
	    freeint2;       /* Integer for general use */
	
	/* Reads info about the mesh into memory */
	/* NumVertices */
	freelong1 = (long int) (pc_MeshPos + 0x0000000A);
	freeint1 = fseek(pc, freelong1, SEEK_SET);
	if (freeint1 != 0)
	{
		return ERROR_PC_READ_FAILED;
	}
	freesizet1 = fread(&pc_NumVertices, 1, 2, pc);
	if (freesizet1 != 2)
	{
		return ERROR_PC_READ_FAILED;
	}
#ifdef __BIG_ENDIAN__
	pc_NumVertices = reverse16(pc_NumVertices);
#endif
	/* NumNormals */
	freelong1 += (2 + (pc_NumVertices * 6));
	freeint1 = fseek(pc, freelong1, SEEK_SET);
	if (freeint1 != 0)
	{
		return ERROR_PC_READ_FAILED;
	}
	freesizet1 = fread(&pc_NumNormals, 1, 2, pc);
	if (freesizet1 != 2)
	{
		return ERROR_PC_READ_FAILED;
	}
#ifdef __BIG_ENDIAN__
	pc_NumNormals = reverse16(pc_NumNormals);
#endif
	/* NumTexturedRectangles */
	if (pc_NumNormals < 0)
	{
		freelong1 += (2 + (abs((int) pc_NumNormals) * 2));
	}
	else
	{
		freelong1 += (2 + (pc_NumNormals * 6));
	}
	pc_PosTexturedRectangles = (freelong1 + 2l);
	freeint1 = fseek(pc, freelong1, SEEK_SET);
	if (freeint1 != 0)
	{
		return ERROR_PC_READ_FAILED;
	}
	freesizet1 = fread(&pc_NumTexturedRectangles, 1, 2, pc);
	if (freesizet1 != 2)
	{
		return ERROR_PC_READ_FAILED;
	}
#ifdef __BIG_ENDIAN__
	pc_NumTexturedRectangles = reverse16(pc_NumTexturedRectangles);
#endif
	/* NumTexturedTriangles */
	freelong1 += (2 + (pc_NumTexturedRectangles * 10));
	pc_PosTexturedTriangles = (freelong1 + 2l);
	freeint1 = fseek(pc, freelong1, SEEK_SET);
	if (freeint1 != 0)
	{
		return ERROR_PC_READ_FAILED;
	}
	freesizet1 = fread(&pc_NumTexturedTriangles, 1, 2, pc);
	if (freesizet1 != 2)
	{
		return ERROR_PC_READ_FAILED;
	}
#ifdef __BIG_ENDIAN__
	pc_NumTexturedTriangles = reverse16(pc_NumTexturedTriangles);
#endif
	/* NumColouredRectangles */
	freelong1 += (2 + (pc_NumTexturedTriangles * 8));
	pc_PosColouredRectangles = (freelong1 + 2l);
	freeint1 = fseek(pc, freelong1, SEEK_SET);
	if (freeint1 != 0)
	{
		return ERROR_PC_READ_FAILED;
	}
	freesizet1 = fread(&pc_NumColouredRectangles, 1, 2, pc);
	if (freesizet1 != 2)
	{
		return ERROR_PC_READ_FAILED;
	}
#ifdef __BIG_ENDIAN__
	pc_NumColouredRectangles = reverse16(pc_NumColouredRectangles);
#endif
	/* NumColouredTriangles */
	freelong1 += (2 + (pc_NumColouredRectangles * 10));
	pc_PosColouredTriangles = (freelong1 + 2l);
	freeint1 = fseek(pc, freelong1, SEEK_SET);
	if (freeint1 != 0)
	{
		return ERROR_PC_READ_FAILED;
	}
	freesizet1 = fread(&pc_NumColouredTriangles, 1, 2, pc);
	if (freesizet1 != 2)
	{
		return ERROR_PC_READ_FAILED;
	}
#ifdef __BIG_ENDIAN__
	pc_NumColouredTriangles = reverse16(pc_NumColouredTriangles);
#endif
	
	/* Checks there aren't more than 255 vertices */
	if (abs((int) pc_NumNormals) >= 255)
	{
		return ERROR_MESH_TOO_MANY_VERTICES;
	}
	
	/* Counts rectangles and Triangles */
	psx_NumNormals = pc_NumNormals;
	psx_NumRectangles = 0x0000;
	psx_NumTriangles = 0x0000;
	freelong1 = (long int) (pc_PosTexturedRectangles + 0x00000008);
	for (freeuint161 = 0x0000; freeuint161 < (INTU16) pc_NumTexturedRectangles;
	     ++freeuint161)
	{
		freeint1 = fseek(pc, freelong1, SEEK_SET);
		if (freeint1 != 0)
		{
			return ERROR_PC_READ_FAILED;
		}
		freesizet1 = fread(&freeuint162, (size_t) 1, (size_t) 2, pc);
		if (freesizet1 != (size_t) 2)
		{
			return ERROR_PC_READ_FAILED;
		}
#ifdef __BIG_ENDIAN__
		freeuint162 = reverse16(freeuint162);
#endif
		++psx_NumRectangles;
		/* Adjusts for double-sided texture */
		if ((freeuint162 & 0x8000) != 0x0000)
		{
			++psx_NumRectangles;
		}
		freelong1 += 10l;
	}
	psx_NumRectangles += pc_NumColouredRectangles;
	freelong1 = (long int) (pc_PosTexturedTriangles + 0x00000006);
	for (freeuint161 = 0x0000; freeuint161 < (INTU16) pc_NumTexturedTriangles;
	     ++freeuint161)
	{
		freeint1 = fseek(pc, freelong1, SEEK_SET);
		if (freeint1 != 0)
		{
			return ERROR_PC_READ_FAILED;
		}
		freesizet1 = fread(&freeuint162, (size_t) 1, (size_t) 2, pc);
		if (freesizet1 != (size_t) 2)
		{
			return ERROR_PC_READ_FAILED;
		}
#ifdef __BIG_ENDIAN__
		freeuint162 = reverse16(freeuint162);
#endif
		++psx_NumTriangles;
		/* Adjusts for double-sided texture */
		if ((freeuint162 & 0x8000) != 0x0000)
		{
			++psx_NumTriangles;
		}
		freelong1 += 8l;
	}
	psx_NumTriangles += pc_NumColouredTriangles;
	
	/* Adjusts for the skybox-object if needed */
	if (skybox == 1)
	{
		psx_NumTriangles = 0x0000;
	}
	
	/* Determines size of mesh in PSX */
	freeuint321 = (INTU32) (16 + (psx_NumRectangles * 6) +
	                        (psx_NumTriangles * 5));
	freeuint321 += ((psx_NumRectangles & 0x0001) * 0x00000002);
	if ((psx_NumTriangles & 0x0003) != 0x0000)
	{
		freeuint321 += (0x0004 - (psx_NumTriangles & 0x0003));
	}
	if (psx_NumNormals < 0)
	{
		freeuint321 += (abs((int) psx_NumNormals) * 8);
	}
	else
	{
		freeuint321 += (psx_NumNormals * 16);
	}
	
	/* Adjusts for the skybox-object if needed */
	if (skybox == 1)
	{
		freeuint321 -= ((psx_NumNormals * 8) + 12);
	}
	
	/* Sets starting offsets of the mesh in both files */
	pc_offset = (long int) pc_MeshPos;
	psx_offset = (long int) psx_info->p_NumMeshPointers;
	
	/* Makes room for mesh in PSX */
	freeint1 = insertbytes(psx, psx_info->p_NumMeshPointers, freeuint321);
	if (freeint1 != ERROR_NONE)
	{
		return freeint1;
	}
	level_info_add(psx_info, psx_info->p_NumMeshPointers, freeuint321);
	psx_info->p_NumMeshPointers += freeuint321;
	*psx_curPointer += freeuint321;
	
	/* Adjusts for the skybox-object */
	if (skybox == 1)
	{
		/* Writes NumVertices to PSX */
		buffer[0] = (INTU8) abs((int) psx_NumNormals);
		buffer[1] = (INTU8) 0x00;
		freeint1 = fseek(psx, psx_offset, SEEK_SET);
		if (freeint1 != 0)
		{
			return ERROR_PSX_READ_FAILED;
		}
		freesizet1 = fwrite(buffer, 1, 2, psx);
		if (freesizet1 != 2)
		{
			return ERROR_PSX_WRITE_FAILED;
		}
		pc_offset += 12;
		psx_offset += 2;
		/* Writes NumRectangles to PSX */
#ifdef __BIG_ENDIAN__
		psx_NumRectangles = reverse16(psx_NumRectangles);
#endif
		freeint1 = fseek(psx, psx_offset, SEEK_SET);
		if (freeint1 != 0)
		{
			return ERROR_PSX_READ_FAILED;
		}
		freesizet1 = fwrite(&psx_NumRectangles, 1, 2, psx);
		if (freesizet1 != 2)
		{
			return ERROR_PSX_WRITE_FAILED;
		}
#ifdef __BIG_ENDIAN__
		psx_NumRectangles = reverse16(psx_NumRectangles);
#endif
		psx_offset += 2;
	}
	else
	{
		/* Copies Centre and Radius */
		freeint1 = fseek(pc, pc_offset, SEEK_SET);
		if (freeint1 != 0)
		{
			return ERROR_PC_READ_FAILED;
		}
		freesizet1 = fread(buffer, 1, 8, pc);
		if (freesizet1 != 8)
		{
			return ERROR_PC_READ_FAILED;
		}
		freeint1 = fseek(psx, psx_offset, SEEK_SET);
		if (freeint1 != 0)
		{
			return ERROR_PSX_READ_FAILED;
		}
		freesizet1 = fwrite(buffer, 1, 8, psx);
		if (freesizet1 != 8)
		{
			return ERROR_PSX_WRITE_FAILED;
		}
		pc_offset += 12;
		psx_offset += 8;
		
		/* Writes NumVertices to PSX */
		if (psx_NumNormals < 0)
		{
			buffer[0] = (INTU8) (abs((int) psx_NumNormals));
			buffer[1] = (INTU8) 0x80;
		}
		else
		{
			buffer[0] = (INTU8) (psx_NumNormals);
			buffer[1] = (INTU8) 0x00;
		}
		freeint1 = fseek(psx, psx_offset, SEEK_SET);
		if (freeint1 != 0)
		{
			return ERROR_PSX_READ_FAILED;
		}
		freesizet1 = fwrite(buffer, 1, 2, psx);
		if (freesizet1 != 2)
		{
			return ERROR_PSX_WRITE_FAILED;
		}
		psx_offset += 2;
		
		/* Calculates and writes fOffset to PSX */
		if (psx_NumNormals < 0)
		{
			freeuint162 = (INTU16) (abs((int) psx_NumNormals) * 8);
		}
		else
		{
			freeuint162 = (INTU16) (psx_NumNormals * 16);
		}
		freesizet1 = fwrite(&freeuint162, 1, 2, psx);
		if (freesizet1 != 2)
		{
			return ERROR_PSX_WRITE_FAILED;
		}
		psx_offset += 2;
	}
	
	/* Copies vertices */
	(void) memset(&buffer[6], 0, 2);
	pc_curlight = (pc_offset + 2 + (pc_NumVertices * 6));
	for (; pc_NumVertices > 0; pc_NumVertices--)
	{
		/* Reads the vertex from PC */
		freeint1 = fseek(pc, pc_offset, SEEK_SET);
		if (freeint1 != 0)
		{
			return ERROR_PC_READ_FAILED;
		}
		freesizet1 = fread(buffer, 1, 6, pc);
		if (freesizet1 != 6)
		{
			return ERROR_PC_READ_FAILED;
		}
		/* Adds the light value if needed */
		if (psx_NumNormals < 0)
		{
			freeint1 = fseek(pc, pc_curlight, SEEK_SET);
			if (freeint1 != 0)
			{
				return ERROR_PC_READ_FAILED;
			}
			freesizet1 = fread(&buffer[6], 1, 2, pc);
			if (freesizet1 != 2)
			{
				return ERROR_PC_READ_FAILED;
			}
			/* Converts the light value from 0-1FFF to RGB */
			buffer[7] = (INTU8) ~buffer[7];
			buffer[7] &= (INTU8) 0x1F;
			buffer[6] = buffer[7];
			buffer[6] |= (INTU8) (buffer[6] << 5U);
			buffer[7] <<= 2U;
			buffer[7] |= (INTU8) (buffer[7] >> 5U);
			pc_curlight += 2;
		}
		/* Writes the vertex to PSX */
		freeint1 = fseek(psx, psx_offset, SEEK_SET);
		if (freeint1 != 0)
		{
			return ERROR_PSX_READ_FAILED;
		}
		freesizet1 = fwrite(buffer, 1, 8, psx);
		if (freesizet1 != 8)
		{
			return ERROR_PSX_WRITE_FAILED;
		}
		pc_offset += 6;
		psx_offset += 8;
	}
	
	/* Determines whether to use normals or lights */
	pc_offset += 2;
	if ((pc_NumNormals > 0) && (skybox == 0))
	{
		/* Copies Normals */
		for (; pc_NumNormals > 0; pc_NumNormals--)
		{
			freeint1 = fseek(pc, pc_offset, SEEK_SET);
			if (freeint1 != 0)
			{
				return ERROR_PC_READ_FAILED;
			}
			freesizet1 = fread(buffer, 1, 6, pc);
			if (freesizet1 != 6)
			{
				return ERROR_PC_READ_FAILED;
			}
			
			/* Shifts every coordinate two bits to the right */
			buffer[0] >>= 2U;
			buffer[0] |= ((buffer[1] & (INTU8) 0x03) << 6U);
			buffer[1] >>= 2U;
			if ((buffer[1] & (INTU8) 0x20) != (INTU8) 0x00)
			{
				buffer[1] |= (INTU8) 0xC0;
			}
			buffer[2] >>= 2U;
			buffer[2] |= ((buffer[3] & (INTU8) 0x03) << 6U);
			buffer[3] >>= 2U;
			if ((buffer[3] & (INTU8) 0x20) != (INTU8) 0x00)
			{
				buffer[3] |= (INTU8) 0xC0;
			}
			buffer[4] >>= 2U;
			buffer[4] |= ((buffer[5] & (INTU8) 0x03) << 6U);
			buffer[5] >>= 2U;
			if ((buffer[5] & (INTU8) 0x20) != (INTU8) 0x00)
			{
				buffer[5] |= (INTU8) 0xC0;
			}
			
			/* Writes the normal to the PSX */
			freeint1 = fseek(psx, psx_offset, SEEK_SET);
			if (freeint1 != 0)
			{
				return ERROR_PSX_READ_FAILED;
			}
			freesizet1 = fwrite(buffer, 1, 8, psx);
			if (freesizet1 != 8)
			{
				return ERROR_PSX_WRITE_FAILED;
			}
			pc_offset += 6;
			psx_offset += 8;
		}
	}
	
	if (skybox != 1)
	{
		/* Writes NumTriangles to PSX */
#ifdef __BIG_ENDIAN__
		psx_NumTriangles = reverse16(psx_NumTriangles);
#endif
		freeint1 = fseek(psx, psx_offset, SEEK_SET);
		if (freeint1 != 0)
		{
			return ERROR_PSX_READ_FAILED;
		}
		freesizet1 = fwrite(&psx_NumTriangles, 1, 2, psx);
		if (freesizet1 != 2)
		{
			return ERROR_PSX_WRITE_FAILED;
		}
#ifdef __BIG_ENDIAN__
		psx_NumTriangles = reverse16(psx_NumTriangles);
#endif
		psx_offset += 2;
		
		/* Writes NumRectangles to PSX */
#ifdef __BIG_ENDIAN__
		psx_NumRectangles = reverse16(psx_NumRectangles);
#endif
		freeint1 = fseek(psx, psx_offset, SEEK_SET);
		if (freeint1 != 0)
		{
			return ERROR_PSX_READ_FAILED;
		}
		freesizet1 = fwrite(&psx_NumRectangles, 1, 2, psx);
		if (freesizet1 != 2)
		{
			return ERROR_PSX_WRITE_FAILED;
		}
#ifdef __BIG_ENDIAN__
		psx_NumRectangles = reverse16(psx_NumRectangles);
#endif
		psx_offset += 2;
		
		curfaces = 0x0000;
		
		/* Converts coloured triangles to PSX */
		pc_offset = pc_PosColouredTriangles;
		for (; pc_NumColouredTriangles > 0; pc_NumColouredTriangles--)
		{
			/* Reads the triangle from PC */
			freeint1 = fseek(pc, pc_offset, SEEK_SET);
			if (freeint1 != 0)
			{
				return ERROR_PC_READ_FAILED;
			}
			freesizet1 = fread(&buffer[20], 1, 8, pc);
			if (freesizet1 != 8)
			{
				return ERROR_PC_READ_FAILED;
			}
			
			/* Sets the correct colour index  */
			buffer[26] = buffer[27];
			buffer[27] = (INTU8) 0x00;
			
			/* Converts the triangle to PSX-format */
			freeint1 = (int) ((curfaces + 0x0001) * 0x0004);
			buffer[freeint1++] = buffer[20];
			buffer[freeint1++] = buffer[22];
			buffer[freeint1++] = buffer[24];
			buffer[freeint1++] = buffer[27];
			buffer[curfaces] = buffer[26];
			++curfaces;
			
			/* Writes the triangles to PSX if needed */
			if (curfaces == 0x0004)
			{
				freeint1 = fseek(psx, psx_offset, SEEK_SET);
				if (freeint1 != 0)
				{
					return ERROR_PSX_READ_FAILED;
				}
				freesizet1 = fwrite(buffer, 1, 20, psx);
				if (freesizet1 != 20)
				{
					return ERROR_PSX_WRITE_FAILED;
				}
				psx_offset += 20;
				curfaces = 0x0000;
			}
			pc_offset += 8;
		}
		
		/* Converts textured triangles to PSX */
		pc_offset = pc_PosTexturedTriangles;
		for (; pc_NumTexturedTriangles > 0; pc_NumTexturedTriangles--)
		{
			freeint1 = fseek(pc, pc_offset, SEEK_SET);
			if (freeint1 != 0)
			{
				return ERROR_PC_READ_FAILED;
			}
			freesizet1 = fread(&buffer[20], 1, 8, pc);
			if (freesizet1 != 8)
			{
				return ERROR_PC_READ_FAILED;
			}
			
			/* Checks whether the texture was misoriented */
			freeuint162 = (INTU16) (((INTU16) buffer[26]) |
			                        (((INTU16) buffer[27]) << 8));
			if ((freeuint162 & 0x8000) != 0x0000)
			{
				doublesided = 1;
			}
			else
			{
				doublesided = 0;
			}
			freeuint162 &= 0x7FFF;
			
			if (notflag(FLAGS_DONT_FIX_TEXTURES))
			{
				for (freeuint163 = 0x0000; freeuint163 < numMisorientedTextures;
				     ++freeuint163)
				{
					if (freeuint162 ==
					    ((INTU16) (misorientedTextures[freeuint163] & 0x7FFF)))
					{
						if ((misorientedTextures[freeuint163] & 0x8000) !=
						    0x0000)
						{
							/* Rotates the triangle to the left */
							buffer[30] = buffer[20];
							buffer[31] = buffer[21];
							buffer[20] = buffer[22];
							buffer[21] = buffer[23];
							buffer[22] = buffer[24];
							buffer[23] = buffer[25];
							buffer[24] = buffer[30];
							buffer[25] = buffer[31];
						}
						else
						{
							/* Rotates the triangle to the right */
							buffer[30] = buffer[20];
							buffer[31] = buffer[21];
							buffer[20] = buffer[24];
							buffer[21] = buffer[25];
							buffer[24] = buffer[22];
							buffer[25] = buffer[23];
							buffer[22] = buffer[30];
							buffer[23] = buffer[31];
						}
					}
				}
			}
			
			/* Converts the texture via lookup table */
			freeuint165 = objectTextures[(freeuint162 << 1U)];
			buffer[27] = (INTU8) ((freeuint165 & 0xFF00) >> 8U);
			buffer[26] = (INTU8) (freeuint165 & 0x00FF);
			
			/* Adds 256 to the texture number */
			++buffer[27];
			
			/* Converts the triangle to PSX-format */
			freeint1 = (int) ((curfaces + 0x0001) * 0x0004);
			buffer[freeint1++] = buffer[20];
			buffer[freeint1++] = buffer[22];
			buffer[freeint1++] = buffer[24];
			buffer[freeint1++] = buffer[27];
			buffer[curfaces] = buffer[26];
			++curfaces;
			
			/* Writes the triangles to PSX if needed */
			if (curfaces == 0x0004)
			{
				freeint1 = fseek(psx, psx_offset, SEEK_SET);
				if (freeint1 != 0)
				{
					return ERROR_PSX_READ_FAILED;
				}
				freesizet1 = fwrite(buffer, 1, 20, psx);
				if (freesizet1 != 20)
				{
					return ERROR_PSX_WRITE_FAILED;
				}
				psx_offset += 20;
				curfaces = 0x0000;
			}
			
			/* Makes a duplicate if needed */
			if (doublesided == 1)
			{
				/* Refers to the mirrored texture */
				freeuint165 = objectTextures[((freeuint162 << 1U) + 0x0001)];
				buffer[27] = (INTU8) ((freeuint165 & 0xFF00) >> 8U);
				buffer[26] = (INTU8) (freeuint165 & 0x00FF);
				
				/* Adds 256 to the texture number */
				++buffer[27];
				
				/* Converts the triangle to PSX-format */
				freeint1 = (int) ((curfaces + 0x0001) * 0x0004);
				buffer[freeint1++] = buffer[22];
				buffer[freeint1++] = buffer[20];
				buffer[freeint1++] = buffer[24];
				buffer[freeint1++] = buffer[27];
				buffer[curfaces] = buffer[26];
				++curfaces;
				
				/* Writes the triangles to PSX if needed */
				if (curfaces == 0x0004)
				{
					freeint1 = fseek(psx, psx_offset, SEEK_SET);
					if (freeint1 != 0)
					{
						return ERROR_PSX_READ_FAILED;
					}
					freesizet1 = fwrite(buffer, 1, 20, psx);
					if (freesizet1 != 20)
					{
						return ERROR_PSX_WRITE_FAILED;
					}
					psx_offset += 20;
					curfaces = 0x0000;
				}
			}
			pc_offset += 8;
		}
		
		/* Writes any left-over triangles to PSX */
		if (curfaces != 0x0000)
		{
			freeint2 = (int) curfaces;
			for (freeint1 = 3; freeint1 > freeint2; --freeint1)
			{
				buffer[freeint1] = (INTU8) 0x00;
			}
			++curfaces;
			curfaces *= 0x0004;
			freeint1 = fseek(psx, psx_offset, SEEK_SET);
			if (freeint1 != 0)
			{
				return ERROR_PSX_READ_FAILED;
			}
			freesizet1 = fwrite(buffer, 1, (size_t) curfaces, psx);
			if (freesizet1 != (size_t) curfaces)
			{
				return ERROR_PSX_WRITE_FAILED;
			}
			psx_offset += (long int) curfaces;
			curfaces = 0x0000;
		}
	}
	
	/* Converts coloured rectangles to PSX */
	pc_offset = pc_PosColouredRectangles;
	for (; pc_NumColouredRectangles > 0; pc_NumColouredRectangles--)
	{
		freeint1 = fseek(pc, pc_offset, SEEK_SET);
		if (freeint1 != 0)
		{
			return ERROR_PC_READ_FAILED;
		}
		freesizet1 = fread(&buffer[20], 1, 10, pc);
		if (freesizet1 != 10)
		{
			return ERROR_PC_READ_FAILED;
		}
		
		/* Sets the correct colour index  */
		buffer[28] = buffer[29];
		buffer[29] = (INTU8) 0x00;
		
		/* Converts the rectangle to PSX-format */
		freeint1 = (int) ((curfaces + 0x0001) * 0x0004);
		buffer[freeint1++] = buffer[20];
		buffer[freeint1++] = buffer[22];
		buffer[freeint1++] = buffer[26];
		buffer[freeint1++] = buffer[24];
		freeint1 = (int) (curfaces * 0x0002);
		buffer[freeint1++] = buffer[28];
		buffer[freeint1++] = buffer[29];
		++curfaces;
		
		/* Writes the rectangles to PSX if needed */
		if (curfaces == 0x0002)
		{
			freeint1 = fseek(psx, psx_offset, SEEK_SET);
			if (freeint1 != 0)
			{
				return ERROR_PSX_READ_FAILED;
			}
			freesizet1 = fwrite(buffer, 1, 12, psx);
			if (freesizet1 != 12)
			{
				return ERROR_PSX_WRITE_FAILED;
			}
			psx_offset += 12;
			curfaces = 0x0000;
		}
		pc_offset += 10;
	}
	
	/* Converts textured rectangles to PSX */
	pc_offset = pc_PosTexturedRectangles;
	for (; pc_NumTexturedRectangles > 0; pc_NumTexturedRectangles--)
	{
		freeint1 = fseek(pc, pc_offset, SEEK_SET);
		if (freeint1 != 0)
		{
			return ERROR_PC_READ_FAILED;
		}
		freesizet1 = fread(&buffer[20], 1, 10, pc);
		if (freesizet1 != 10)
		{
			return ERROR_PC_READ_FAILED;
		}
		
		/* Checks whether the texture was misoriented */
		freeuint162 = (INTU16) (((INTU16) buffer[28]) |
		                        (((INTU16) buffer[29]) << 8));
		if ((freeuint162 & 0x8000) != 0x0000)
		{
			doublesided = 1;
		}
		else
		{
			doublesided = 0;
		}
		freeuint162 &= 0x7FFF;
		
		if (notflag(FLAGS_DONT_FIX_TEXTURES))
		{
			for (freeuint163 = 0x0000; freeuint163 < numMisorientedTextures;
			     ++freeuint163)
			{
				if (((INTU16) (misorientedTextures[freeuint163] & 0x7FFF)) ==
				    freeuint162)
				{
					if ((misorientedTextures[freeuint163] & 0x8000) != 0x0000)
					{
						/* Rotates the rectangle to the left */
						buffer[30] = buffer[20];
						buffer[31] = buffer[21];
						buffer[20] = buffer[22];
						buffer[21] = buffer[23];
						buffer[22] = buffer[24];
						buffer[23] = buffer[25];
						buffer[24] = buffer[26];
						buffer[25] = buffer[27];
						buffer[26] = buffer[30];
						buffer[27] = buffer[31];
					}
					else
					{
						/* Rotates the rectangle to the right */
						buffer[30] = buffer[20];
						buffer[31] = buffer[21];
						buffer[20] = buffer[26];
						buffer[21] = buffer[27];
						buffer[26] = buffer[24];
						buffer[27] = buffer[25];
						buffer[24] = buffer[22];
						buffer[25] = buffer[23];
						buffer[22] = buffer[30];
						buffer[23] = buffer[31];
					}
				}
			}
		}
		
		/* Converts the texture via lookup table */
		freeuint165 = objectTextures[(freeuint162 << 1U)];
		buffer[29] = (INTU8) ((freeuint165 & 0xFF00) >> 8U);
		buffer[28] = (INTU8) (freeuint165 & 0x00FF);
		
		/* Adds 256 to the texture number */
		++buffer[29];
		
		/* Converts the rectangle to PSX-format */
		freeint1 = (int) ((curfaces + 0x0001) * 0x0004);
		buffer[freeint1++] = buffer[20];
		buffer[freeint1++] = buffer[22];
		buffer[freeint1++] = buffer[26];
		buffer[freeint1++] = buffer[24];
		freeint1 = (int) (curfaces * 0x0002);
		buffer[freeint1++] = buffer[28];
		buffer[freeint1++] = buffer[29];
		++curfaces;
		
		/* Writes the rectangles to PSX if needed */
		if (curfaces == 0x0002)
		{
			freeint1 = fseek(psx, psx_offset, SEEK_SET);
			if (freeint1 != 0)
			{
				return ERROR_PSX_READ_FAILED;
			}
			freesizet1 = fwrite(buffer, 1, 12, psx);
			if (freesizet1 != 12)
			{
				return ERROR_PSX_WRITE_FAILED;
			}
			psx_offset += 12;
			curfaces = 0x0000;
		}
		
		/* Makes duplicate for the other side if needed */
		if (doublesided == 1)
		{
			/* Refers to the mirrored texture */
			freeuint165 = objectTextures[((freeuint162 << 1U) + 0x0001)];
			buffer[29] = (INTU8) ((freeuint165 & 0xFF00) >> 8U);
			buffer[28] = (INTU8) (freeuint165 & 0x00FF);
			
			/* Adds 256 to the texture number */
			++buffer[29];
			
			/* Converts the rectangle to PSX-format */
			freeint1 = (int) ((curfaces + 0x0001) * 0x0004);
			buffer[freeint1++] = buffer[22];
			buffer[freeint1++] = buffer[20];
			buffer[freeint1++] = buffer[24];
			buffer[freeint1++] = buffer[26];
			freeint1 = (int) (curfaces * 0x0002);
			buffer[freeint1++] = buffer[28];
			buffer[freeint1++] = buffer[29];
			++curfaces;
			
			/* Writes the rectangles to PSX if needed */
			if (curfaces == 0x0002)
			{
				freeint1 = fseek(psx, psx_offset, SEEK_SET);
				if (freeint1 != 0)
				{
					return ERROR_PSX_READ_FAILED;
				}
				freesizet1 = fwrite(buffer, 1, 12, psx);
				if (freesizet1 != 12)
				{
					return ERROR_PSX_WRITE_FAILED;
				}
				psx_offset += 12;
				curfaces = 0x0000;
			}
		}
		
		pc_offset += 10;
	}
	
	/* Writes any left-over rectangles to PSX */
	if (curfaces != 0x0000)
	{
		buffer[2] = (INTU8) 0x00;
		buffer[3] = (INTU8) 0x00;
		freeint1 = fseek(psx, psx_offset, SEEK_SET);
		if (freeint1 != 0)
		{
			return ERROR_PSX_READ_FAILED;
		}
		freesizet1 = fwrite(buffer, 1, (size_t) 8, psx);
		if (freesizet1 != (size_t) 8)
		{
			return ERROR_PSX_WRITE_FAILED;
		}
		psx_offset += (long int) 8;
	}
	
	/* Ends the function */
	return ERROR_NONE;
}

/*
 * Function that converts the sounds from PC to PSX.
 * Parameters:
 * * pc = Pointer to PC-file
 * * psx = Pointer to PSX-file
 * * pc_info = Pointer to the level_info struct relating to PC
 * * psx_info = Pointer to the level_info struct relating to PSX
 * * psxPath = Path to the PSX-file
 * * sfxPath = Path to the MAIN.SFX file
 */
static int convertSounds(FILE *pc, FILE *psx, struct level_info *pc_info,
                         struct level_info *psx_info, char *psxPath,
                         char *sfxPath)
{
	/* Variable Declarations */
	FILE *sfx = NULL;             /* MAIN.SFX file to read from */
	INTU32 *sfx_indices = NULL;   /* List of indices in main.sfx */
	INTU32 *sampleIndices = NULL; /* List of SampleIndices from PC */
	INTU32 numSampleData;         /* Number of total bytes of the samples */
	INTU32 freeuint321,
	       freeuint322,
	       freeuint323;           /* Unsigned 32-bit integers for general use */
	INTU16 freeuint161,
	       freeuint162;           /* Unsigned 16-bit integers for general use */
	int freeint1;                 /* Integer for general use */
	size_t freesizet1;            /* size_t for general use */
	long int freelong1;           /* Long integer for general use */
	int errorNumber;              /* Return value for this function */
	errorNumber = ERROR_NONE;
	
	/* Removes old sample indices and samples from PSX */
	freeuint321 = (psx_info->p_FirstCodeModule - 0x0000000C);
	errorNumber = removebytes(psx, psxPath, 0x00000004, freeuint321);
	if (errorNumber != ERROR_NONE)
	{
		goto end;
	}
	errorNumber = zerobytes(psx, 0x00000004, 0x00000008);
	if (errorNumber != ERROR_NONE)
	{
		goto end;
	}
	level_info_sub(psx_info, 0x00000004, freeuint321);
	psx_info->p_NumSampleIndices = 0x00000004;
	psx_info->p_NumSamples = 0x00000008;
	psx_info->v_NumSampleIndices = 0x00000000;
	psx_info->v_NumSamples = 0x00000000;

	/* Removes old sound details from PSX */
	freeuint321 = (psx_info->v_NumSoundDetails * 0x00000008);
	errorNumber = removebytes(psx, psxPath, psx_info->p_NumSoundDetails,
	                          freeuint321);
	if (errorNumber != ERROR_NONE)
	{
		goto end;
	}
	errorNumber = zerobytes(psx, psx_info->p_NumSoundDetails, 0x00000004);
	if (errorNumber != ERROR_NONE)
	{
		goto end;
	}
	psx_info->v_NumSoundDetails = 0x00000000;
	level_info_sub(psx_info, psx_info->p_NumSoundDetails, freeuint321);
	
	/* Copies the soundmap */
	freeuint321 = (pc_info->p_NumSoundDetails - 740);
	freeuint322 = (psx_info->p_NumSoundDetails - 740);
	errorNumber = copybytes(pc, psx, freeuint321, freeuint322, 740);
	if (errorNumber != ERROR_NONE)
	{
		goto end;
	}
	
	/* Makes room for the sound details */
	freeuint321 = (pc_info->v_NumSoundDetails * 8);
	errorNumber = insertbytes(psx, psx_info->p_NumSoundDetails, freeuint321);
	if (errorNumber != ERROR_NONE)
	{
		goto end;
	}
	level_info_add(psx_info, psx_info->p_NumSoundDetails, freeuint321);
	
	/* Copies sound details */
	freeuint321 += 0x00000004;
	errorNumber = copybytes(pc, psx, pc_info->p_NumSoundDetails,
	                        psx_info->p_NumSoundDetails, freeuint321);
	
	/* Opens main.sfx */
	sfx = fopen(sfxPath, "rb");
	if (sfx == NULL)
	{
		errorNumber = ERROR_SFX_OPEN_FAILED;
		goto end;
	}
	
	/* Allocates sfx_indices */
	sfx_indices = calloc(2048, 4);
	if (sfx_indices == NULL)
	{
		errorNumber = ERROR_MEMORY;
		goto end;
	}
	
	/* Determines the size of main.sfx */
	freeint1 = fseek(sfx, 0, SEEK_END);
	if (freeint1 != 0)
	{
		errorNumber = ERROR_SFX_READ_FAILED;
		goto end;
	}
	freeuint321 = (INTU32) ftell(sfx);
	freeuint322 = 0x00000000;
	freeuint161 = 0x0000;
	
	/* Determines sfx_indices */
	while (freeuint322 < freeuint321)
	{
		if (freeuint161 == 0x0800)
		{
			errorNumber = ERROR_SFX_TOO_MANY_SAMPLES;
			goto end;
		}
		sfx_indices[freeuint161++] = freeuint322;
		freeuint322 += 4;
		freeint1 = fseek(sfx, (long int) freeuint322, SEEK_SET);
		if (freeint1 != 0)
		{
			errorNumber = ERROR_SFX_READ_FAILED;
			goto end;
		}
		freesizet1 = fread(&freeuint323, 1, 4, sfx);
		if (freesizet1 != (size_t) 4)
		{
			errorNumber = ERROR_SFX_READ_FAILED;
			goto end;
		}
#ifdef __BIG_ENDIAN__
		freeuint323 = reverse32(freeuint323);
#endif
		freeuint322 += 0x00000004;
		freeuint322 += freeuint323;
	}
	
	/* Gets sampleIndices from PC */
	sampleIndices = calloc((size_t) pc_info->v_NumSampleIndices, 4);
	if (sampleIndices == NULL)
	{
		errorNumber = ERROR_MEMORY;
		goto end;
	}
	freelong1 = (long int) (pc_info->p_NumSampleIndices + 0x00000004);
	freeint1 = fseek(pc, freelong1, SEEK_SET);
	if (freeint1 != 0)
	{
		errorNumber = ERROR_PC_READ_FAILED;
		goto end;
	}
	freeuint321 = (pc_info->v_NumSampleIndices * 0x00000004);
	freesizet1 = fread(sampleIndices, 1, (size_t) freeuint321, pc);
	if (freesizet1 != (size_t) freeuint321)
	{
		errorNumber = ERROR_PC_READ_FAILED;
		goto end;
	}
#ifdef __BIG_ENDIAN__
	freeuint321 /= 0x00000004;
	do
	{
		--freeuint321;
		sampleIndices[freeuint321] = reverse32(sampleIndices[freeuint321]);
	}
	while (freeuint321 > 0x00000000);
#endif
	
	/* Dereferences the sampleIndices */
	for (freeuint321 = 0x00000000; freeuint321 < pc_info->v_NumSampleIndices;
	     ++freeuint321)
	{
		sampleIndices[freeuint321] = sfx_indices[sampleIndices[freeuint321]];
	}
	
	/* Frees up sfx_indices */
	free(sfx_indices);
	sfx_indices = NULL;
	
	/* Determines which samples loop */
	for (freeuint321 = 0x00000000; freeuint321 < pc_info->v_NumSoundDetails;
	     ++freeuint321)
	{
		/* Reads relevant data from sound details entry */
		freelong1 = (long int) (pc_info->p_NumSoundDetails +
		                        4 + (freeuint321 * 8));
		freeint1 = fseek(pc, freelong1, SEEK_SET);
		if (freeint1 != 0)
		{
			errorNumber = ERROR_PC_READ_FAILED;
			goto end;
		}
		freesizet1 = fread(&freeuint161, 1, 2, pc);
		if (freesizet1 != 2)
		{
			errorNumber = ERROR_PC_READ_FAILED;
			goto end;
		}
#ifdef __BIG_ENDIAN__
		freeuint161 = reverse16(freeuint161);
#endif
		freelong1 += 6;
		freeint1 = fseek(pc, freelong1, SEEK_SET);
		if (freeint1 != 0)
		{
			errorNumber = ERROR_PC_READ_FAILED;
			goto end;
		}
		freesizet1 = fread(&freeuint162, 1, 2, pc);
		if (freesizet1 != 2)
		{
			errorNumber = ERROR_PC_READ_FAILED;
			goto end;
		}
#ifdef __BIG_ENDIAN__
		freeuint162 = reverse16(freeuint162);
#endif
		
		/* Sets the loop-flag if needed */
		if ((freeuint162 & 0x0003) == 0x0003)
		{
			freeuint162 &= 0x003C;
			freeuint162 >>= 2;
			for (; freeuint162 > 0x0000; --freeuint162)
			{
				sampleIndices[freeuint161++] |= 0x80000000;
			}
		}
	}
	
	/* Makes room for sample indices in PSX */
	freeuint321 = (INTU32) (psx_info->p_NumSoundDetails + 4 +
	                        (pc_info->v_NumSoundDetails * 8));
	errorNumber = copybytes(pc, psx, pc_info->p_NumSampleIndices,
	                        freeuint321, 0x00000004);
	if (errorNumber != ERROR_NONE)
	{
		goto end;
	}
	errorNumber = copybytes(pc, psx, pc_info->p_NumSampleIndices,
	                        0x00000004, 0x00000004);
	if (errorNumber != ERROR_NONE)
	{
		goto end;
	}
	freeuint321 = (INTU32) (pc_info->v_NumSampleIndices * 0x00000004);
	errorNumber = insertbytes(psx, 0x00000008, freeuint321);
	if (errorNumber != ERROR_NONE)
	{
		goto end;
	}
	level_info_add(psx_info, 0x00000008, freeuint321);
	
	/* Sets counting variables to 0 before starting */
	numSampleData = 0x00000000;
	
	/* Makes sure p_FirstCodeModule doesn't get corrupted */
	++psx_info->p_FirstCodeModule;
	
	/* Converts the samples */
	for (freeuint321 = 0x00000000; freeuint321 < pc_info->v_NumSampleIndices;
	     ++freeuint321)
	{
		/* Sets the offset of a new sampleindex in PSX */
		freeuint323 = (INTU32) (0x00000008 + (freeuint321 * 0x00000004));
		freeuint322 = numSampleData;
#ifdef __BIG_ENDIAN__
		freeuint322 = reverse32(freeuint322);
#endif
		freeint1 = fseek(psx, (long int) freeuint323, SEEK_SET);
		if (freeint1 != 0)
		{
			errorNumber = ERROR_PSX_READ_FAILED;
			goto end;
		}
		freesizet1 = fwrite(&freeuint322, 1, 4, psx);
		if (freesizet1 != 4)
		{
			errorNumber = ERROR_PSX_WRITE_FAILED;
			goto end;
		}
		
		/* Determines the size of the sample after conversion */
		freeuint322 = outVagSize(sfx,
		                         (sampleIndices[freeuint321] & 0x7FFFFFFF));
		if ((sampleIndices[freeuint321] & 0x80000000) == 0x00000000)
		{
			freeuint322 += 0x00000010;
		}
		
		/* Inserts room for the new sample */
		freeuint323 = (INTU32) (0x0000000C +
		                        (pc_info->v_NumSampleIndices * 0x00000004) +
		                        numSampleData);
		errorNumber = insertbytes(psx, freeuint323, freeuint322);
		if (errorNumber != ERROR_NONE)
		{
			goto end;
		}
		level_info_add(psx_info, freeuint323, freeuint322);
		
		/* Zeroes out sixteen bytes before the sample */
		errorNumber = zerobytes(psx, freeuint323, 16);
		
		/* Adds the added length to numSampleData */
		numSampleData += freeuint322;
		
		/* Remembers whether this sample loops */
		if ((sampleIndices[freeuint321] & 0x80000000) != 0x00000000)
		{
			freeint1 = 1;
		}
		else
		{
			freeint1 = 0;
		}
		
		/* Converts the sample */
		freeuint323 += 0x00000010;
		errorNumber = convertSample(sfx, psx,
		                            (sampleIndices[freeuint321] & 0x7FFFFFFF),
		                            freeuint323, freeint1);
		if (errorNumber != ERROR_NONE)
		{
			goto end;
		}
	}
	
	/* Sets p_FirstCodeModule to what it should be */
	--psx_info->p_FirstCodeModule;
	
	/* Writes NumSampleData to PSX */
	freelong1 = (long int) ((freeuint321 * 0x00000004) + 0x00000008);
	freeint1 = fseek(psx, freelong1, SEEK_SET);
	if (freeint1 != 0)
	{
		errorNumber = ERROR_PSX_READ_FAILED;
		goto end;
	}
#ifdef __BIG_ENDIAN__
	numSampleData = reverse32(numSampleData);
#endif
	freesizet1 = fwrite(&numSampleData, 1, 4, psx);
	if (freesizet1 != 4)
	{
		errorNumber = ERROR_PSX_WRITE_FAILED;
		goto end;
	}
	
end:/* Frees allocated memory and ends the function */
	if (sfx_indices != NULL)
	{
		free(sfx_indices);
	}
	if (sampleIndices != NULL)
	{
		free(sampleIndices);
	}
	if (sfx != NULL)
	{
		(void) fclose(sfx);
	}
	return errorNumber;
}

/*
 * Function that finds the corresponding mesh pointer to an ObjectID
 * Parameters:
 * * pc = Pointer to PC-file
 * * pc_info = Struct holding offsets and values from PC
 * * objectID = ObjectID to find the corresponding mesh on
 * Return values:
 * * 0xFFFF = The mesh could not be found
 * * else = The number of the mesh pointer
 */
static INTU16 findMesh(FILE *pc, struct level_info *pc_info, INTU32 objectID)
{
	/* Variable Declarations */
	INTU8 buffer[8];    /* Buffer space to load the Moveables into */
	INTU32 curID;       /* ObjectID of the current Moveable */
	INTU32 curMoveable; /* Current Moveable */
	INTU16 retval;      /* Return value for this function */
	long int curpos;    /* Current position in PC */
	int freeint1;       /* Integer for general use */
	size_t freesizet1;  /* Size_t for general use */
	
	/* Loops through the Movables looking for the mesh */
	curpos = (long int) (pc_info->p_NumMoveables + 0x00000004);
	for (curMoveable = 0x00000000; curMoveable < pc_info->v_NumMoveables;
	     ++curMoveable)
	{
		/* Goes to the Moveable */
		freeint1 = fseek(pc, curpos, SEEK_SET);
		if (freeint1 != 0)
		{
			return 0xFFFF;
		}
		
		/* Reads the needed parts into memory */
		freesizet1 = fread(buffer, 1, 8, pc);
		if (freesizet1 != (size_t) 8)
		{
			return 0xFFFF;
		}
		
		/* Determines curID */
		memcpy(&curID, buffer, 4);
#ifdef __BIG_ENDIAN__
		curID = reverse32(curID);
#endif
		
		/* Sees if they match */
		if (curID == objectID)
		{
			/* Returns the correct mesh pointer */
			memcpy(&retval, &buffer[6], 2);
#ifdef __BIG_ENDIAN__
			retval = reverse16(retval);
#endif
			return retval;
		}
		
		/* Moves to the next moveable */
		curpos += 18;
	}
	
	/* No Moveable with this objectID exists */
	return 0xFFFF;
}

/*
 * Function that checks the sizes of fixed-size integers.
 * Return values:
 * * 0 = All integers are the correct sizes
 * * 1 = Some integers are not the correct sizes
 */
#ifdef FIXEDINT_CHECK_ON_STARTUP
static int fixedint_check(void)
{
	/* Variable Declarations */
	int retval = 0; /* Return value for this function */
	
	/* Checks the sizes of the integers */
	if (sizeof(INT8) != 1)
	{
		printf("INT8 should be an 8-bit signed integer.\n"
		       "Instead it is %i-bits.\n", (int) (sizeof(INT8) * 8));
		retval = 1;
	}
	if (sizeof(INTU8) != 1)
	{
		printf("INTU8 should be an 8-bit unsigned integer.\n"
		       "Instead it is %i-bits.\n", (int) (sizeof(INTU8) * 8));
		retval = 1;
	}
	if (sizeof(INT16) != 2)
	{
		printf("INT16 should be a 16-bit signed integer.\n"
		       "Instead it is %i-bits.\n", (int) (sizeof(INT16) * 8));
		retval = 1;
	}
	if (sizeof(INTU16) != 2)
	{
		printf("INTU16 should be a 16-bit unsigned integer.\n"
		       "Instead it is %i-bits.\n", (int) (sizeof(INTU16) * 8));
		retval = 1;
	}
	if (sizeof(INT32) != 4)
	{
		printf("INT32 should be a 32-bit signed integer.\n"
		       "Instead it is %i-bits.\n", (int) (sizeof(INT32) * 8));
		retval = 1;
	}
	if (sizeof(INTU32) != 4)
	{
		printf("INTU32 should be a 32-bit unsigned integer.\n"
		       "Instead it is %i-bits.\n", (int) (sizeof(INTU32) * 8));
		retval = 1;
	}
	if (retval == 1)
	{
		printf("Please modify fixedint.h and recompile the program.\n");
	}
	
	return retval;
}
#endif
