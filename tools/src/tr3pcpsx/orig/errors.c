/*
 * Library that handles errors in TR3PCPSX.
 * Copyright (C) 2021 b122251
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation,either version 3 of the License,or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY;without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not,see <http://www.gnu.org/licenses/>.
 */

/* File Inclusions */
#include <stdio.h>    /* Standard I/O */
#include "fixedint.h" /* Definition of fixed-size integers */
#include "errors.h"   /* Error Handling for TR3PCPSX */
#include "osdep.h"    /* Library of OS-dependant functions */
#include "version.h"  /* Version number */

/*
 * Prints error messages if needed.
 * Parameters:
 * * errorNumber = The number of the error
 * * argv = Command-line parameters
 * * pcparam = Number of the parameter to PC-file
 * * psxparam = Number of the parameter to PSX-file
 * * sfxPath = Path to MAIN.SFX
 */
void printError(int errorNumber, char *argv[],
                int pcparam, int psxparam, char *sfxPath)
{
	switch (errorNumber)
	{
		case ERROR_INVALID_PARAMETERS:
#ifdef CLEARTERMINAL
			CLEARTERMINAL
#endif
			printf("Tomb Raider III PC to PSX Level Converter" PROGVER "\n"
			       "  Copyright (C) 2021  b122251\n\n"
			       "Usage: %s [PCFILE] [PSXFILE] [SFXFILE] [OPTION]...\n"
			       "  PCFILE is the TR3 file to be converted\n"
			       "  PSXFILE is the output PSX file\n"
			       "  SFXFILE is the MAIN.SFX file\n"
			       "  See the readme for options\n"
			       "\n", argv[0]);
			break;
		case ERROR_PC_FILE_INVALID:
			printf(PROGNAME ": %s is not a TR3-file\n", argv[pcparam]);
			break;
		case ERROR_PC_OPEN_FAILED:
			printf(PROGNAME ": %s could not be opened\n", argv[pcparam]);
			break;
		case ERROR_PC_READ_FAILED:
			printf(PROGNAME ": %s could not be read from\n", argv[pcparam]);
			break;
		case ERROR_PC_CLOSE_FAILED:
			printf(PROGNAME ": %s could not be closed\n", argv[pcparam]);
			break;
		case ERROR_PSX_OPEN_FAILED:
			printf(PROGNAME ": %s could not be opened\n", argv[psxparam]);
			break;
		case ERROR_PSX_READ_FAILED:
			printf(PROGNAME ": %s could not be read from\n", argv[psxparam]);
			break;
		case ERROR_PSX_WRITE_FAILED:
			printf(PROGNAME ": %s could not be written to\n", argv[psxparam]);
			break;
		case ERROR_PSX_CLOSE_FAILED:
			printf(PROGNAME ": %s could not be closed\n", argv[psxparam]);
			break;
		case ERROR_MEMORY:
			printf(PROGNAME ": Memory could not be allocated\n");
			break;
		case ERROR_REMOVE_DATA_INVALID:
			printf(PROGNAME ": Data to be removed doesn't exist\n");
			break;
		case ERROR_INVALID_WATER_COLOUR:
			printf(PROGNAME ": Invalid description of water colour\n");
			break;
		case ERROR_ARRAY_TOO_LONG:
			printf(PROGNAME ": Array is too long to be sorted\n"); /* Unused */
			break;
		case ERROR_COLOURS_INVALID:
			printf(PROGNAME ": Colours do not exist\n"); /* Unused */
			break;
		case ERROR_TOO_MANY_TEXTILES:
			printf(PROGNAME ": Output has more than 15 texture tiles\n");
			break;
		case ERROR_TEXTURE_CORNERS:
			printf(PROGNAME ": Texture corners are invalid\n");
			break;
		case ERROR_TOO_MANY_TEXTURES:
			printf(PROGNAME ": Level has too many textures to convert\n");
			break;
		case ERROR_SFX_OPEN_FAILED:
			printf(PROGNAME ": %s could not be opened\n", sfxPath);
			break;
		case ERROR_SFX_READ_FAILED:
			printf(PROGNAME ": %s could not be read from\n", sfxPath);
			break;
		case ERROR_SAMPLE_BITRATE:
			printf(PROGNAME ": Sample bitrate is not supported\n");
			break;
		case ERROR_WAVE_NON_PCM:
			printf(PROGNAME ": Input WAVE is not in PCM\n");
			break;
		case ERROR_WAVE_TOO_MANY_CHANNELS:
			printf(PROGNAME ": Input WAVE has more than two channels\n");
			break;
		case ERROR_TOO_MANY_CHUNKS:
			/* The error message has been printed in the function */
			break;
		case ERROR_SFX_TOO_MANY_SAMPLES:
			printf(PROGNAME ": %s has too many samples\n", sfxPath);
			break;
		case ERROR_MODULE_CLASH:
			printf(PROGNAME ": Code modules couldn't be combined\n");
			break;
		case ERROR_ROOM_TABLE_CLASH:
			printf(PROGNAME ": Outside table clash\n");
			break;
		case ERROR_MESH_TOO_MANY_VERTICES:
			printf(PROGNAME ": Mesh uses more than 255 vertices\n");
			break;
		case ERROR_TOO_MANY_PALETTES:
			printf(PROGNAME ": Too many palettes\n");
			break;
	}
}
