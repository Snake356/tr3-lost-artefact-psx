/*
 *  __________  _    _  ___  ____
 * |_   _  ,. \| \  / |/ _ \|  _  \
 *   | | | `' /|  \/  | | | | | | |
 *   | | | |\ \|      | |_| | |_| |
 *   |_| |_| \_\_|\/|_|\___/|____/
 *
 * Tomb Raider Level Modifier
 * Copyright (C) 2022 b122251
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
#include "license.h"  /* Prints the GNU GPL3 */
#include "fixedint.h" /* Definition of fixed-size integers */
#include "structs.h"  /* Definition of used structs */
#include "command.h"  /* COmmand Interpretation */
#include "errors.h"   /* Definition of error codes */
#include "wrap.h"     /* Wrapper functions for actual functionality */
#include "util.h"     /* General functions for TRMOD */
#include "trmodio.h"  /* TRMOD input and output */

/* Internal Function Declarations */
static void leveltype_identify(struct level *level, struct error *error,
                               char *idstr);
#ifndef _STDINT_H
static int checkFixedInt(void);
#endif

#ifdef TRMOD_GUI
#else
/*
 * Main Function of the Program (Terminal Mode)
 */
int main(int argc, char *argv[])
{
	/* Variable Declarations */
	struct level level; /* Struct for the level */
	struct error error; /* Struct for error handling */
	int curparam = 0;   /* Current parameter */
	int retval = 0;     /* Return value for called functions */
	
	/* Wipes the level and error structs before starting */
	memset(&level, 0, sizeof(struct level));
	memset(&error, 0, sizeof(struct error));
	
	/* Checks fixed-size integers */
#ifndef _STDINT_H
	retval = checkFixedInt();
	if (retval != 0)
	{
		goto end;
	}
#endif
	
	/* Checks the number of command-line parameters */
	if (argc < 4)
	{
		if (argc >= 2)
		{
			/* Special commands */
			if ((compareString(argv[1], "--l", 3) == 0) ||
			    (compareString(argv[1], "-l", 2) == 0)  ||
			    (compareString(argv[1], "l", 1) == 0))
			{
				showLicense();
				goto end;
			}
			else if ((compareString(argv[1], "--u", 3) == 0) ||
			         (compareString(argv[1], "-u", 2) == 0)  ||
			         (compareString(argv[1], "u", 1) == 0)   ||
			         (compareString(argv[1], "--h", 3) == 0) ||
			         (compareString(argv[1], "-h", 2) == 0)  ||
			         (compareString(argv[1], "h", 1) == 0)   ||
			         (compareString(argv[1], "/?", 2) == 0)  ||
			         (compareString(argv[1], "\\?", 2) == 0))
			{
				showUsage(argv[0]);
				goto end;
			}
		}
		
		error.code = ERROR_INVALID_PARAMETERS;
		error.string[0] = argv[0];
		goto end;
	}
	
	/* Sets typestring and command */
	level.typestring = argv[1];
	level.command = argv[0];
	
	/* Interprets leveltype identifier */
	leveltype_identify(&level, &error, argv[1]);
	if (error.code != ERROR_NONE)
	{
		goto end;
	}
	if (level.type == 0x00000000)
	{
		error.code = ERROR_INVALID_TYPE;
		error.string[0] = argv[1];
		goto end;
	}

	/* Determines endianness */
#ifdef __BIG_ENDIAN__
	if (level.type == TRMOD_LEVTYPE_TR1_SS)
	{
		level.endian = FALSE;
	}
	else
	{
		level.endian = TRUE;
	}
#else
	if (level.type == TRMOD_LEVTYPE_TR1_SS)
	{
		level.endian = TRUE;
	}
	else
	{
		level.endian = FALSE;
	}
#endif
	
	/* Sets up navigation structs */
	setup(&level, &error);
	
	/* Sets level.path */
	level.path = argv[2];
	
	/* Opens the level file */
	level.file = trmod_fopen(level.path, "r+b");
	if (level.file == NULL)
	{
		error.code = ERROR_FILE_OPEN_FAILED;
		error.string[0] = level.path;
		goto end;
	}
	
	/* Navigates through the level file */
	navigate(&level, &error);
	if (error.code != ERROR_NONE)
	{
		goto end;
	}
	
	/* Processes the command parameters */
	for (curparam = 3; curparam < argc; ++curparam)
	{
		interpretCommand(&level, &error, argv[curparam]);
		if (error.code != ERROR_NONE)
		{
			goto end;
		}
	}
	
end:/* The end of the program */
	/* Closes the level file */
	if (level.file != NULL)
	{
		retval = trmod_fclose(level.file, level.path);
		if (retval != 0)
		{
			error.code = ERROR_FILE_CLOSE_FAILED;
			error.string[0] = level.path;
		}
	}
	
	/* Frees allocated room variables */
	if (level.room != NULL)
	{
		free(level.room);
		level.room = NULL;
	}
	
	/* Prints an error code if needed */
	if (error.code != ERROR_NONE)
	{
		printError(&error);
	}
	
	/* Ends the program */
	return error.code;
}
#endif

/*
 * Function that returns a level-type identifier
 * Parameters:
 * * level = Pointer to the level struct
 * * error = Pointer to the error struct
 * * idstr = String of the level type identifier
 */
static void leveltype_identify(struct level *level, struct error *error,
                               char *idstr)
{
	/* Checks type identifier */
	if (compareString(idstr, "tr1pc", (size_t) 5) == 0)
	{
		level->type = TRMOD_LEVTYPE_TR1_PC;
	}
	else if (compareString(idstr, "tr1gpc", (size_t) 6) == 0)
	{
		level->type = TRMOD_LEVTYPE_TR1G_PC;
	}
	else if (compareString(idstr, "tr1ps", (size_t) 5) == 0)
	{
		level->type = TRMOD_LEVTYPE_TR1_PS;
		error->code = ERROR_UNIMPLEMENTED_LEVTYPE;
		error->string[0] = idstr;
	}
	else if (compareString(idstr, "tr1s", (size_t) 4) == 0)
	{
		level->type = TRMOD_LEVTYPE_TR1_SS;
		error->string[0] = idstr;
	}
	else if (compareString(idstr, "tr2pc", (size_t) 5) == 0)
	{
		level->type = TRMOD_LEVTYPE_TR2_PC;
	}
	else if (compareString(idstr, "tr2ps", (size_t) 5) == 0)
	{
		level->type = TRMOD_LEVTYPE_TR2_PS;
		error->code = ERROR_UNIMPLEMENTED_LEVTYPE;
		error->string[0] = idstr;
	}
	else if (compareString(idstr, "tr3pc", (size_t) 5) == 0)
	{
		level->type = TRMOD_LEVTYPE_TR3_PC;
	}
	else if (compareString(idstr, "tr3ps", (size_t) 5) == 0)
	{
		level->type = TRMOD_LEVTYPE_TR3_PS;
		error->code = ERROR_UNIMPLEMENTED_LEVTYPE;
		error->string[0] = idstr;
	}
	else if (compareString(idstr, "tr4pc", (size_t) 5) == 0)
	{
		level->type = TRMOD_LEVTYPE_TR4_PC;
		error->code = ERROR_UNIMPLEMENTED_LEVTYPE;
		error->string[0] = idstr;
	}
	else if (compareString(idstr, "tr4ps", (size_t) 5) == 0)
	{
		level->type = TRMOD_LEVTYPE_TR4_PS;
		error->code = ERROR_UNIMPLEMENTED_LEVTYPE;
		error->string[0] = idstr;
	}
	else if (compareString(idstr, "tr5pc", (size_t) 5) == 0)
	{
		level->type = TRMOD_LEVTYPE_TR5_PC;
		error->code = ERROR_UNIMPLEMENTED_LEVTYPE;
		error->string[0] = idstr;
	}
	else if (compareString(idstr, "tr5ps", (size_t) 5) == 0)
	{
		level->type = TRMOD_LEVTYPE_TR5_PS;
		error->code = ERROR_UNIMPLEMENTED_LEVTYPE;
		error->string[0] = idstr;
	}
	else if (compareString(idstr, "tr5dc", (size_t) 5) == 0)
	{
		level->type = TRMOD_LEVTYPE_TR5_PC;
		error->code = ERROR_UNIMPLEMENTED_LEVTYPE;
		error->string[0] = idstr;
	}
}

#ifndef _STDINT_H
/*
 * Function that checks whether the integers in fixedint.h are correct
 * Return values: 0 = Everything is correct, 1 = Something is incorrect
 */
static int checkFixedInt(void)
{
	/* Variable Declarations */
	uint8 uint8_v[2];
	int8 int8_v[2];
	uint16 uint16_v[2];
	int16 int16_v[2];
	uint32 uint32_v[2];
	int32 int32_v[2];
	unsigned int size;
	
	/* Checks sizes */
	size = (unsigned int) sizeof(uint8);
	size <<= 3U;
	if (size != 8U)
	{
		printf("uint8 should be 8 bits, instead it is %u bits\n"
		       "Please adjust fixedint.h and recompile\n", size);
		return 1;
	}
	size = (unsigned int) sizeof(int8);
	size <<= 3U;
	if (size != 8U)
	{
		printf("int8 should be 8 bits, instead it is %u bits\n"
		       "Please adjust fixedint.h and recompile\n", size);
		return 1;
	}
	size = (unsigned int) sizeof(uint16);
	size <<= 3U;
	if (size != 16U)
	{
		printf("uint16 should be 16 bits, instead it is %u bits\n"
		       "Please adjust fixedint.h and recompile\n", size);
		return 1;
	}
	size = (unsigned int) sizeof(int16);
	size <<= 3U;
	if (size != 16U)
	{
		printf("int16 should be 16 bits, instead it is %u bits\n"
		       "Please adjust fixedint.h and recompile\n", size);
		return 1;
	}
	size = (unsigned int) sizeof(uint32);
	size <<= 3U;
	if (size != 32U)
	{
		printf("uint32 should be 32 bits, instead it is %u bits\n"
		       "Please adjust fixedint.h and recompile\n", size);
		return 1;
	}
	size = (unsigned int) sizeof(int32);
	size <<= 3U;
	if (size != 32U)
	{
		printf("int32 should be 32 bits, instead it is %u bits\n"
		       "Please adjust fixedint.h and recompile\n", size);
		return 1;
	}
	
	/* Checks sign */
	memset(&uint8_v[0], 0, sizeof(uint8_v[0]));
	memset(&int8_v[0], 0, sizeof(int8_v[0]));
	memset(&uint16_v[0], 0, sizeof(uint16_v[0]));
	memset(&int16_v[0], 0, sizeof(int16_v[0]));
	memset(&uint32_v[0], 0, sizeof(uint32_v[0]));
	memset(&int32_v[0], 0, sizeof(int32_v[0]));
	memset(&uint8_v[1], 255, sizeof(uint8_v[1]));
	memset(&int8_v[1], 255, sizeof(int8_v[1]));
	memset(&uint16_v[1], 255, sizeof(uint16_v[1]));
	memset(&int16_v[1], 255, sizeof(int16_v[1]));
	memset(&uint32_v[1], 255, sizeof(uint32_v[1]));
	memset(&int32_v[1], 255, sizeof(int32_v[1]));
	if (uint8_v[1] < uint8_v[0])
	{
		printf("uint8 should be unsigned, instead it is signed\n"
		       "Please adjust fixedint.h and recompile\n");
		return 1;
	}
	if (uint16_v[1] < uint16_v[0])
	{
		printf("uint16 should be unsigned, instead it is signed\n"
		       "Please adjust fixedint.h and recompile\n");
		return 1;
	}
	if (uint32_v[1] < uint32_v[0])
	{
		printf("uint32 should be unsigned, instead it is signed\n"
		       "Please adjust fixedint.h and recompile\n");
		return 1;
	}
	if (int8_v[1] > int8_v[0])
	{
		printf("int8 should be signed, instead it is unsigned\n"
		       "Please adjust fixedint.h and recompile\n");
		return 1;
	}
	if (int16_v[1] > int16_v[0])
	{
		printf("int16 should be signed, instead it is unsigned\n"
		       "Please adjust fixedint.h and recompile\n");
		return 1;
	}
	if (int32_v[1] > int32_v[0])
	{
		printf("int32 should be signed, instead it is unsigned\n"
		       "Please adjust fixedint.h and recompile\n");
		return 1;
	}
	
	/* All tests have passed */
	return 0;
}
#endif
