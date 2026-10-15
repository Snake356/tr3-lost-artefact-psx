/*
 * Library that prints error codes for TRMOD
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
#include <stdio.h>   /* Standard I/O */
#include "errors.h"  /* Definition of error codes */

#define PROGNAME "troi"

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
			printf(PROGNAME ": Memory could not be allocated\n");
			break;
		case ERROR_INVALID_PARAMETERS:
			printf("Tomb Raider III Object Importer/Exporter\n"
			       "  Copyright (c) 2021, b122251\n"
			       "  Usage: %s [level file] [import/export] [object ID] [data file]\n",
			       error->string[0]);
			break;
		case ERROR_FILE_OPEN_FAILED:
			printf(PROGNAME ": %s could not be opened\n", error->string[0]);
			break;
		case ERROR_FILE_READ_FAILED:
			printf(PROGNAME ": %s could not be read from\n", error->string[0]);
			break;
		case ERROR_FILE_WRITE_FAILED:
			printf(PROGNAME ": %s could not be written to\n",
			       error->string[0]);
			break;
		case ERROR_FILE_CLOSE_FAILED:
			printf(PROGNAME ": %s could not be closed\n", error->string[0]);
			break;
		case ERROR_REMOVE_DATA_INVALID:
			printf(PROGNAME ": Data to be removed doesn't exist\n");
			break;
		case ERROR_INVALID_TYPE:
			printf(PROGNAME ": %s is not a valid level type identifier\n",
			       error->string[0]);
			break;
		case ERROR_UNIMPLEMENTED_LEVTYPE:
			printf(PROGNAME ": Level type %s is unimplemented\n",
			       error->string[0]);
			break;
		case ERROR_INVALID_COMMAND:
			printf(PROGNAME ": %s is not a valid command\n", error->string[0]);
			break;
		case ERROR_ROOM_DOESNT_EXIST:
			printf(PROGNAME ": Room " PRINTU16 " does not exist\n",
			       error->u16[0]);
			break;
		case ERROR_ITEM_DOESNT_EXIST:
			printf(PROGNAME ": Item " PRINTU32 " does not exist\n",
			       error->u32[0]);
			break;
		case ERROR_TOO_MANY_SUBPARAMS:
			printf(PROGNAME ": Too many parameters\n");
			break;
		case ERROR_NOT_ENOUGH_SUBPARAMS:
			printf(PROGNAME ": Not enough parameters\n");
			break;
		case ERROR_INVALID_FDINDEX:
			printf(PROGNAME ": Invalid FDIndex\n");
			break;
		case ERROR_STATICMESH_DOESNT_EXIST:
			printf(PROGNAME ": Static Mesh " PRINTU16 ":" PRINTU16
			       " does not exist\n", error->u16[0], error->u16[1]);
			break;
		case ERROR_LIGHT_DOESNT_EXIST:
			printf(PROGNAME ": Light " PRINTU16 ":" PRINTU16
			       " does not exist\n", error->u16[0], error->u16[1]);
			break;
		case ERROR_SECTOR_DOESNT_EXIST:
			printf(PROGNAME ": Sector " PRINTU16 ": (" PRINTU16
			       "," PRINTU16 ") does not exist\n", error->u16[0],
			       error->u16[1], error->u16[2]);
			break;
		case ERROR_INVALID_SYNTAX:
			printf(PROGNAME ": Syntax error\n");
			break;
		case ERROR_FDINDEX_OUT_OF_RANGE:
			printf(PROGNAME ": FDIndex " PRINTU16 " out of range\n",
			       error->u16[0]);
			break;
		case ERROR_UNKNOWN_FDFUNCTION:
			printf(PROGNAME ": Unknown FDFunction " PRINTU16 "\n",
			       error->u16[0]);
			break;
		case ERROR_UNKNOWN_TRIGGERACTION:
			printf(PROGNAME ": Unknown TrigAction " PRINTU16 "\n",
			       error->u16[0]);
			break;
		case ERROR_NO_FLOORDATA_SEMICOLON:
			printf(PROGNAME ": No separating semicolon in floordata\n");
			break;
		case ERROR_NO_FLOORDATA_COORDINATES:
			printf(PROGNAME ": No room, column and row specification\n");
			break;
		case ERROR_INVALID_FDFUNCTION:
			printf(PROGNAME ": %s is not a valid FDFunction\n",
			       error->string[0]);
			break;
		case ERROR_INVALID_TRIGTYPE:
			printf(PROGNAME ": %s is not a valid trigger type\n",
			       error->string[0]);
			break;
		case ERROR_INVALID_TRIGACTION:
			printf(PROGNAME ": %s is not a valid trigger action\n",
			       error->string[0]);
			break;
		case ERROR_SOUNDSOURCE_DOESNT_EXIST:
			printf(PROGNAME ": Sound source " PRINTU32 " does not exist\n",
			       error->u32[0]);
			break;
		case ERROR_VERTEX_DOESNT_EXIST:
			printf(PROGNAME ": Vertex " PRINTU16 ":" PRINTU16
			       " does not exist\n", error->u16[0], error->u16[1]);
			break;
		case ERROR_RECTANGLE_DOESNT_EXIST:
			printf(PROGNAME ": Rectangle " PRINTU16 ":" PRINTU16
			       " dos not exist\n", error->u16[0], error->u16[1]);
			break;
		case ERROR_TRIANGLE_DOESNT_EXIST:
			printf(PROGNAME ": Triangle " PRINTU16 ":" PRINTU16
			       " dos not exist\n", error->u16[0], error->u16[1]);
			break;
		case ERROR_SPRITE_DOESNT_EXIST:
			printf(PROGNAME ": Sprite " PRINTU16 ":" PRINTU16
			       " dos not exist\n", error->u16[0], error->u16[1]);
			break;
		case ERROR_SPRITESEQ_NOT_FOUND:
			printf(PROGNAME ": There is no Sprite Sequence with the offset "
			       PRINTU16 "\n", error->u16[0]);
			break;
		case ERROR_SPRITESEQ_NOT_FOUND_ID:
			printf(PROGNAME ": There is no Sprite Sequence with the SpriteID "
			       PRINTU32 "\n", error->u32[0]);
			break;
		case ERROR_ROOM_RESIZE_TO_ZERO:
			printf(PROGNAME ": Cannot set a room's width or length to zero\n");
			break;
		case ERROR_VIEWPORT_DOESNT_EXIST:
			printf(PROGNAME ": Viewport " PRINTU16 ":" PRINTU16
			       " does not exist\n", error->u16[0], error->u16[1]);
			break;
		case ERROR_BOX_DOESNT_EXIST:
			printf(PROGNAME ": Box " PRINTU32 " does not exist\n",
			       error->u32[0]);
			break;
		case ERROR_NULL_POINTER:
			printf(PROGNAME ": NULL-pointer error (shouldn't happen)\n");
			break;
		case ERROR_MOVABLE_DOESNT_EXIST:
			printf(PROGNAME ": Movable with ObjectID " PRINTU32
			       " does not exist\n", error->u32[0]);
			break;
	}
}
