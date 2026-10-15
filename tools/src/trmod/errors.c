/*
 * Library that prints error codes for TRMOD
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
#include <stdio.h>   /* Standard I/O */
#include "errors.h"  /* Definition of error codes */
#include "version.h" /* Program version number */
#include "trmodio.h" /* TRMOD input and output */

/*
 * Function that prints the error codes
 * Parameters:
 * * error = Pointer to the error struct
 */
void printError(struct error *error)
{
	switch (error->code)
	{
		case ERROR_MEMORY:
			trmod_printf(PROGNAME ": Memory could not be allocated\n");
			break;
		case ERROR_INVALID_PARAMETERS:
			trmod_printf("Tomb Raider Level Modifier " PROGVER "\n"
			             "  Copyright (c) 2022, b122251\n"
			             "  Usage: %s [level type] [level file] [command]\n"
			             "  Please see the readme file for details\n",
			             error->string[0]);
			break;
		case ERROR_FILE_OPEN_FAILED:
			trmod_printf(PROGNAME ": %s could not be opened\n",
			             error->string[0]);
			break;
		case ERROR_FILE_READ_FAILED:
			trmod_printf(PROGNAME ": %s could not be read from\n",
			             error->string[0]);
			break;
		case ERROR_FILE_WRITE_FAILED:
			trmod_printf(PROGNAME ": %s could not be written to\n",
			             error->string[0]);
			break;
		case ERROR_FILE_CLOSE_FAILED:
			trmod_printf(PROGNAME ": %s could not be closed\n",
			             error->string[0]);
			break;
		case ERROR_REMOVE_DATA_INVALID:
			trmod_printf(PROGNAME ": Data to be removed doesn't exist\n");
			break;
		case ERROR_INVALID_TYPE:
			trmod_printf(PROGNAME ": %s is not a valid level type identifier\n",
			             error->string[0]);
			break;
		case ERROR_UNIMPLEMENTED_LEVTYPE:
			trmod_printf(PROGNAME ": Level type %s is unimplemented\n",
			             error->string[0]);
			break;
		case ERROR_UNIMPLEMENTED_FUNCTION:
			trmod_printf(PROGNAME ": Function has not been implemented yet\n");
			break;
		case ERROR_INVALID_COMMAND:
			trmod_printf(PROGNAME ": %s is not a valid command\n",
			             error->string[0]);
			break;
		case ERROR_ROOM_DOESNT_EXIST:
			trmod_printf(PROGNAME ": Room " PRINTU16 " does not exist\n",
			             error->u16[0]);
			break;
		case ERROR_ITEM_DOESNT_EXIST:
			trmod_printf(PROGNAME ": Item " PRINTU32 " does not exist\n",
			             error->u32[0]);
			break;
		case ERROR_TOO_MANY_SUBPARAMS:
			trmod_printf(PROGNAME ": Too many parameters\n");
			break;
		case ERROR_NOT_ENOUGH_SUBPARAMS:
			trmod_printf(PROGNAME ": Not enough parameters\n");
			break;
		case ERROR_INVALID_FDINDEX:
			trmod_printf(PROGNAME ": Invalid FDIndex\n");
			break;
		case ERROR_STATICMESH_DOESNT_EXIST:
			trmod_printf(PROGNAME ": Static Mesh " PRINTU16 ":" PRINTU16
			             " does not exist\n", error->u16[0], error->u16[1]);
			break;
		case ERROR_LIGHT_DOESNT_EXIST:
			trmod_printf(PROGNAME ": Light " PRINTU16 ":" PRINTU16
			             " does not exist\n", error->u16[0], error->u16[1]);
			break;
		case ERROR_SECTOR_DOESNT_EXIST:
			trmod_printf(PROGNAME ": Sector " PRINTU16 ": (" PRINTU16
			             "," PRINTU16 ") does not exist\n", error->u16[0],
			             error->u16[1], error->u16[2]);
			break;
		case ERROR_INVALID_SYNTAX:
			trmod_printf(PROGNAME ": Syntax error\n");
			break;
		case ERROR_FDINDEX_OUT_OF_RANGE:
			trmod_printf(PROGNAME ": FDIndex " PRINTU16 " out of range\n",
			             error->u16[0]);
			break;
		case ERROR_UNKNOWN_FDFUNCTION:
			trmod_printf(PROGNAME ": Unknown FDFunction " PRINTU16 "\n",
			             error->u16[0]);
			break;
		case ERROR_UNKNOWN_TRIGGERACTION:
			trmod_printf(PROGNAME ": Unknown TrigAction " PRINTU16 "\n",
			             error->u16[0]);
			break;
		case ERROR_NO_FLOORDATA_SEMICOLON:
			trmod_printf(PROGNAME ": No separating semicolon in floordata\n");
			break;
		case ERROR_NO_FLOORDATA_COORDINATES:
			trmod_printf(PROGNAME ": No room, column and row specification\n");
			break;
		case ERROR_INVALID_FDFUNCTION:
			trmod_printf(PROGNAME ": %s is not a valid FDFunction\n",
			             error->string[0]);
			break;
		case ERROR_INVALID_TRIGTYPE:
			trmod_printf(PROGNAME ": %s is not a valid trigger type\n",
			             error->string[0]);
			break;
		case ERROR_INVALID_TRIGACTION:
			trmod_printf(PROGNAME ": %s is not a valid trigger action\n",
			             error->string[0]);
			break;
		case ERROR_SOUNDSOURCE_DOESNT_EXIST:
			trmod_printf(PROGNAME ": Sound source " PRINTU32
			             " does not exist\n", error->u32[0]);
			break;
		case ERROR_VERTEX_DOESNT_EXIST:
			trmod_printf(PROGNAME ": Vertex " PRINTU16 ":" PRINTU16
			             " does not exist\n", error->u16[0], error->u16[1]);
			break;
		case ERROR_RECTANGLE_DOESNT_EXIST:
			trmod_printf(PROGNAME ": Rectangle " PRINTU16 ":" PRINTU16
			             " dos not exist\n", error->u16[0], error->u16[1]);
			break;
		case ERROR_TRIANGLE_DOESNT_EXIST:
			trmod_printf(PROGNAME ": Triangle " PRINTU16 ":" PRINTU16
			             " dos not exist\n", error->u16[0], error->u16[1]);
			break;
		case ERROR_SPRITE_DOESNT_EXIST:
			trmod_printf(PROGNAME ": Sprite " PRINTU16 ":" PRINTU16
			             " dos not exist\n", error->u16[0], error->u16[1]);
			break;
		case ERROR_SPRITESEQ_NOT_FOUND:
			trmod_printf(PROGNAME
			             ": There is no Sprite Sequence with the offset "
			             PRINTU16 "\n", error->u16[0]);
			break;
		case ERROR_SPRITESEQ_NOT_FOUND_ID:
			trmod_printf(PROGNAME
			             ": There is no Sprite Sequence with the SpriteID "
			             PRINTU32 "\n", error->u32[0]);
			break;
		case ERROR_ROOM_RESIZE_TO_ZERO:
			trmod_printf(PROGNAME
			             ": Cannot set a room's width or length to zero\n");
			break;
		case ERROR_VIEWPORT_DOESNT_EXIST:
			trmod_printf(PROGNAME ": Viewport " PRINTU16 ":" PRINTU16
			             " does not exist\n", error->u16[0], error->u16[1]);
			break;
		case ERROR_BOX_DOESNT_EXIST:
			trmod_printf(PROGNAME ": Box " PRINTU32 " does not exist\n",
			             error->u32[0]);
			break;
		case ERROR_NULL_POINTER:
			trmod_printf(PROGNAME ": NULL-pointer error (shouldn't happen)\n");
			break;
		case ERROR_SILENT:
			break;
		case ERROR_INVALID_NAVARRAY:
			trmod_printf(PROGNAME ": Navigation array is invalid\n");
			break;
	}
}
