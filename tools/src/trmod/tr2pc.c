/*
 * Library of functions relating to Tomb Raider II on PC
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
#include <stdlib.h>   /* Standard library */
#include <string.h>   /* Functions relating to strings */
#include "fixedint.h" /* Definition of fixed-size integers */
#include "structs.h"  /* Definition of used structs */
#include "errors.h"   /* Definition of error codes */
#include "tr1pc.h"    /* Tomb Raider I PC (PHD) */
#include "tr2pc.h"    /* Tomb Raider II PC (TR2) */
#include "util.h"     /* General functions for TRMOD */
#include "wrap.h"     /* Wrapper functions for called functions */
#include "trmodio.h"  /* TRMOD input and output */

/* Navigation array */
static const uint32 tr2pc_nav[] =
{
	SKIP_BYTES, 0x00000704U,
	NUMTEXTILES, TEXTILE,
	SKIP_BYTES, 0x00000004U,
	NUMROOMS,
	START_ROOM,
		SKIP_BYTES, 0x00000014U,
		R_NUMVERTICES, R_VERTEX,
		R_NUMRECTANGLES, R_RECTANGLE,
		R_NUMTRIANGLES, R_TRIANGLE,
		R_NUMSPRITES, R_SPRITE,
		R_NUMDOORS, R_DOOR,
		R_NUMZSECTORS, R_NUMXSECTORS, R_SECTOR,
		SKIP_BYTES, 0x00000006U,
		R_NUMLIGHTS, R_LIGHT,
		R_NUMSTATICMESHES, R_STATICMESH,
		R_ALTROOM,
		SKIP_BYTES, 0x00000002U,
	END_ROOM,
	NUMFLOORDATA, FLOORDATA,
	NUMMESHDATA, MESHDATA,
	NUMMESHPOINTERS, MESHPOINTER,
	NUMANIMATIONS, ANIMATION,
	NUMSTATECHANGES, STATECHANGE,
	NUMANIMDISPATCHES, ANIMDISPATCH,
	NUMANIMCOMMANDS, ANIMCOMMAND,
	NUMMESHTREES, MESHTREE,
	NUMFRAMES, FRAME,
	NUMMOVEABLES, MOVEABLE,
	NUMSTATICMESHES, STATICMESH,
	NUMOBJECTTEXTURES, OBJECTTEXTURE,
	NUMSPRITETEXTURES, SPRITETEXTURE,
	NUMSPRITESEQUENCES, SPRITESEQUENCE,
	NUMCAMERAS, CAMERA,
	NUMSOUNDSOURCES, SOUNDSOURCE,
	NUMBOXES, BOX,
	NUMOVERLAPS, OVERLAP,
	ZONE,
	NUMANIMTEXTURES, ANIMATEDTEXTURE,
	NUMENTITIES, ENTITY,
	SKIP_BYTES, 0x00002000U,
	NUMCINEMATICFRAMES, CINEMATICFRAME,
	NUMDEMODATA, DEMODATA,
	SKIP_BYTES, 0x000002E4U,
	NUMSOUNDDETAILS, SOUNDDETAIL,
	NUMSAMPLEINDICES, SAMPLEINDEX,
	END_LEVEL
};

/*
 * Function that sets up the structs for navigating the level file
 * * Parameters:
 * * level = Pointer to the level struct
 */
void tr2pc_setup(struct level *level)
{
	/* Sets up the navigation array */
	level->nav = (uint32 *) tr2pc_nav;
	
	/* Sets up part sizes */
	level->partsize[R_NUMVERTICES]      = 0x00000002U;
	level->partsize[R_NUMRECTANGLES]    = 0x00000002U;
	level->partsize[R_NUMTRIANGLES]     = 0x00000002U;
	level->partsize[R_NUMSPRITES]       = 0x00000002U;
	level->partsize[R_NUMDOORS]         = 0x00000002U;
	level->partsize[R_NUMZSECTORS]      = 0x00000002U;
	level->partsize[R_NUMXSECTORS]      = 0x00000002U;
	level->partsize[R_NUMLIGHTS]        = 0x00000002U;
	level->partsize[R_NUMSTATICMESHES]  = 0x00000002U;
	level->partsize[R_ALTROOM]          = 0x00000002U;
	level->partsize[NUMTEXTILES]        = 0x00000004U;
	level->partsize[NUMPALETTES]        = 0xFFFFFFFFU;
	level->partsize[NUMROOMS]           = 0x00000002U;
	level->partsize[OUTROOMTABLE]       = 0xFFFFFFFFU;
	level->partsize[NUMROOMMESHBOXES]   = 0xFFFFFFFFU;
	level->partsize[NUMFLOORDATA]       = 0x00000004U;
	level->partsize[NUMMESHDATA]        = 0x00000004U;
	level->partsize[NUMMESHPOINTERS]    = 0x00000004U;
	level->partsize[NUMANIMATIONS]      = 0x00000004U;
	level->partsize[NUMSTATECHANGES]    = 0x00000004U;
	level->partsize[NUMANIMDISPATCHES]  = 0x00000004U;
	level->partsize[NUMANIMCOMMANDS]    = 0x00000004U;
	level->partsize[NUMMESHTREES]       = 0x00000004U;
	level->partsize[NUMFRAMES]          = 0x00000004U;
	level->partsize[NUMMOVEABLES]       = 0x00000004U;
	level->partsize[NUMSTATICMESHES]    = 0x00000004U;
	level->partsize[NUMOBJECTTEXTURES]  = 0x00000004U;
	level->partsize[NUMSPRITETEXTURES]  = 0x00000004U;
	level->partsize[NUMSPRITESEQUENCES] = 0x00000004U;
	level->partsize[NUMCAMERAS]         = 0x00000004U;
	level->partsize[NUMSOUNDSOURCES]    = 0x00000004U;
	level->partsize[NUMBOXES]           = 0x00000004U;
	level->partsize[NUMOVERLAPS]        = 0x00000004U;
	level->partsize[ZONE]               = 0x00000014U;
	level->partsize[NUMANIMTEXTURES]    = 0x00000004U;
	level->partsize[NUMENTITIES]        = 0x00000004U;
	level->partsize[NUMROOMTEX]         = 0xFFFFFFFFU;
	level->partsize[NUMCINEMATICFRAMES] = 0x00000002U;
	level->partsize[NUMDEMODATA]        = 0x00000002U;
	level->partsize[NUMSOUNDDETAILS]    = 0x00000004U;
	level->partsize[NUMSAMPLES]         = 0xFFFFFFFFU;
	level->partsize[NUMSAMPLEINDICES]   = 0x00000004U;
	level->partsize[CODEMODULE]         = 0xFFFFFFFFU;
	level->partsize[R_VERTEX]           = 0x0000000CU;
	level->partsize[R_RECTANGLE]        = 0x0000000AU;
	level->partsize[R_TRIANGLE]         = 0x00000008U;
	level->partsize[R_SPRITE]           = 0x00000004U;
	level->partsize[R_DOOR]             = 0x00000020U;
	level->partsize[R_SECTOR]           = 0x00000008U;
	level->partsize[R_ROOMLIGHT]        = 0xFFFFFFFFU;
	level->partsize[R_LIGHT]            = 0x00000018U;
	level->partsize[R_STATICMESH]       = 0x00000014U;
	level->partsize[TEXTILE]            = 0x00030000U;
	level->partsize[PALETTE]            = 0xFFFFFFFFU;
	level->partsize[ROOMMESHBOX]        = 0xFFFFFFFFU;
	level->partsize[FLOORDATA]          = 0x00000002U;
	level->partsize[MESHDATA]           = 0x00000002U;
	level->partsize[MESHPOINTER]        = 0x00000004U;
	level->partsize[ANIMATION]          = 0x00000020U;
	level->partsize[STATECHANGE]        = 0x00000006U;
	level->partsize[ANIMDISPATCH]       = 0x00000008U;
	level->partsize[ANIMCOMMAND]        = 0x00000002U;
	level->partsize[MESHTREE]           = 0x00000004U;
	level->partsize[FRAME]              = 0x00000002U;
	level->partsize[MOVEABLE]           = 0x00000012U;
	level->partsize[STATICMESH]         = 0x00000020U;
	level->partsize[OBJECTTEXTURE]      = 0x00000014U;
	level->partsize[SPRITETEXTURE]      = 0x00000010U;
	level->partsize[SPRITESEQUENCE]     = 0x00000008U;
	level->partsize[CAMERA]             = 0x00000010U;
	level->partsize[SOUNDSOURCE]        = 0x00000010U;
	level->partsize[BOX]                = 0x00000008U;
	level->partsize[OVERLAP]            = 0x00000002U;
	level->partsize[ANIMATEDTEXTURE]    = 0x00000002U;
	level->partsize[ENTITY]             = 0x00000018U;
	level->partsize[ROOMTEXTURE]        = 0xFFFFFFFFU;
	level->partsize[CINEMATICFRAME]     = 0x00000010U;
	level->partsize[DEMODATA]           = 0x00000001U;
	level->partsize[SOUNDDETAIL]        = 0x00000008U;
	level->partsize[SAMPLE]             = 0xFFFFFFFFU;
	level->partsize[SAMPLEINDEX]        = 0x00000004U;
}

/*
 * Function that prints an item
 * Parameters:
 * * level = Pointer to level struct
 * * error = Pointer to error struct
 * * item = Item number
 * * print = Printing flags
 * * * 0 0 0 0  0 0 0 0   0 0 0 0  0 0 0 0
 * * * | | | |  | | | |   | | | |  | | | |
 * * * | | | |  | | | |   | | | |  | | | As command
 * * * | | | |  | | | |   | | | |  | | As replace command
 * * * | | | |  | | | |   | | | |  | In Hexadecimal
 * * * Unused
 */
void tr2pc_get_item(struct level *level, struct error *error,
                    uint32 item, uint16 print)
{
	/* Variable Declarations */
	uint8 memitem[24]; /* In-memory copy of the item */
	long int offset;   /* Offset of the item */
	int seekval;       /* Return value of fseek() */
	size_t readval;    /* Return value of fread() */
	uint16 id;         /* The Item's ID */
	uint16 room;       /* The Item's Room */
	int32 x;           /* The Item's X-Position */
	int32 y;           /* The Item's Y-Position */
	int32 z;           /* The Item's Z-Position */
	int16 angle;       /* The Item's Angle */
	int16 inten1;      /* The Item's Intensity1 */
	int16 inten2;      /* The Item's Intensity2 */
	uint16 flags;      /* The Item's Flags */
	
	/* Checks whether the item exists */
	if (item >= level->value[NUMENTITIES])
	{
		error->code = ERROR_ITEM_DOESNT_EXIST;
		error->u32[0] = item;
		return;
	}
	
	/* Reads the item to be printed into memory */
	offset = (long int) (item * 0x00000018);
	offset += (long int) (level->offset[NUMENTITIES] + 0x00000004);
	seekval = trmod_fseek(level->file, offset, SEEK_SET);
	if (seekval != 0)
	{
		error->code = ERROR_FILE_READ_FAILED;
		error->string[0] = level->path;
		return;
	}
	readval = trmod_fread(memitem, (size_t) 1, (size_t) 24, level->file);
	if (readval != (size_t) 24)
	{
		error->code = ERROR_FILE_READ_FAILED;
		error->string[0] = level->path;
		return;
	}
	
	/* Prints as hex dump if needed */
	if ((print & 0x0004) != 0x0000)
	{
		for (id = 0x0000U; id < (uint16) level->partsize[ENTITY]; ++id)
		{
			trmod_printf("%02X ", memitem[id]);
		}
		trmod_printf("\n");
		return;
	}
	
	/* Interprets the read item */
	id = (uint16) ((((uint16) memitem[1]) << 8U) | ((uint16) memitem[0]));
	room = (uint16) ((((uint16) memitem[3]) << 8U) | ((uint16) memitem[2]));
	x = (int32) ((((uint32) memitem[7]) << 24U) |
	             (((uint32) memitem[6]) << 16U) |
	             (((uint32) memitem[5]) << 8U) | ((uint32) memitem[4]));
	y = (int32) ((((uint32) memitem[11]) << 24U) |
	             (((uint32) memitem[10]) << 16U) |
	             (((uint32) memitem[9]) << 8U) | ((uint32) memitem[8]));
	z = (int32) ((((uint32) memitem[15]) << 24U) |
	             (((uint32) memitem[14]) << 16U) |
	             (((uint32) memitem[13]) << 8U) | ((uint32) memitem[12]));
	angle = (int16) ((((uint16) memitem[17]) << 8U) | ((uint16) memitem[16]));
	inten1 = (int16) ((((uint16) memitem[19]) << 8U) | ((uint16) memitem[18]));
	inten2 = (int16) ((((uint16) memitem[21]) << 8U) | ((uint16) memitem[20]));
	flags = (uint16) ((((uint16) memitem[23]) << 8U) | ((uint16) memitem[22]));
	
	/* Checks that this room exists */
	if (room >= level->numRooms)
	{
		error->code = ERROR_ROOM_DOESNT_EXIST;
		error->u16[0] = room;
		return;
	}
	
	/* Sets the coordinates to local coordinates */
	x -= level->room[room].x;
	y = (level->room[room].yBottom - y);
	z -= level->room[room].z;
	
	/* Converts intensity if needed */
	if (inten1 != (int16) 0xFFFF)
	{
		inten1 = (0x1FFF - inten1);
	}
	if (inten2 != (int16) 0xFFFF)
	{
		inten2 = (0x1FFF - inten2);
	}
	
	/* Determines how to print the item */
	if ((print & 0x0003) != 0x0000)
	{
		trmod_printf("%s %s %s ", level->command,
		             level->typestring, level->path);
		if ((print & 0x0002) != 0x0000)
		{
			trmod_printf("\"replaceitem(" PRINTU32 ",", item);
		}
		else
		{
			trmod_printf("\"additem(");
		}
	}
	else
	{
		trmod_printf("Item(");
	}
	
	/* Prints the item */
	trmod_printf(PRINTU16 "," PRINTU16 "," PRINT32 "," PRINT32 "," PRINT32 ","
	             PRINT16 "," PRINT16 "%s," PRINT16 "%s,%04X)", id, room, x, z,
	             y, angle, inten1, ((inten1 != (int16) 0xFFFF) ? "/8191" : ""),
	             inten2, ((inten2 != (int16) 0xFFFF) ? "/8191" : ""), flags);
	
	/* Adds the closing quote if needed and prints the newline */
	if ((print & 0x0003) != 0x0000)
	{
		trmod_printf("\"");
	}
	trmod_printf("\n");
}

/*
 * Function that replaces an item in the level
 * Parameters:
 * * level = Pointer to level struct
 * * error = Pointer to error struct
 * * item = Number of the item ro be replaced (max int means the last one)
 * * id = Item's ID
 * * room = Item's room
 * * x = Item's X-position (in local coordnates)
 * * y = Item's Y-position (in local coordnates)
 * * z = Item's Z-position (in local coordnates)
 * * angle = Item's angle (Euler)
 * * inten1 = Item's intensity1 value (8191-0)
 * * inten2 = Item's intensity2 value (8191-0)
 * * flags = Item's flags
 * * rep = Bit-mask of elements to replace
 * * * 0 0 0 0  0 0 0 0   0 0 0 0  0 0 0 0
 * * * | | | |  | | | |   | | | |  | | | |
 * * * | | | |  | | | |   | | | |  | | | ID
 * * * | | | |  | | | |   | | | |  | | Room
 * * * | | | |  | | | |   | | | |  | X
 * * * | | | |  | | | |   | | | |  Y
 * * * | | | |  | | | |   | | | Z
 * * * | | | |  | | | |   | | Angle
 * * * | | | |  | | | |   | Intensity1
 * * * | | | |  | | | |   Flags
 * * * | | | |  | | | Intensity2
 * * * Unused
 */
void tr2pc_replaceitem(struct level *level, struct error *error,
                       uint32 item, uint16 id, uint16 room,
                       int32 x, int32 y, int32 z, int16 angle,
                       int16 inten1, int16 inten2, uint16 flags, uint16 rep)
{
	/* Variable Declarations */
	int seekval;       /* Return value for fseek() */
	size_t readval;    /* Return value for fread() */
	size_t writeval;   /* Return value for fwrite() */
	long int curpos;   /* Current position in the file */
	uint8 memitem[24]; /* In-memory copy of the item */
	
	/* Sets item number to the last one if max int */
	if (item == 0xFFFFFFFF)
	{
		item = level->value[NUMENTITIES];
		--item;
	}
	
	/* Checks whether the item exists */
	if (item >= level->value[NUMENTITIES])
	{
		error->code = ERROR_ITEM_DOESNT_EXIST;
		error->u32[0] = item;
		return;
	}
	/* Checks whether the room exists */
	if (room >= level->numRooms)
	{
		error->code = ERROR_ROOM_DOESNT_EXIST;
		error->u16[0] = room;
		return;
	}
	
	/* Determines offset of the item */
	curpos = (long int) (level->offset[NUMENTITIES] +
	                     0x00000004 + (item * 0x0018));
	
	/* Reads the existing item into memory */
	seekval = trmod_fseek(level->file, curpos, SEEK_SET);
	if (seekval != 0)
	{
		error->code = ERROR_FILE_READ_FAILED;
		error->string[0] = level->path;
		return;
	}
	readval = trmod_fread(memitem, (size_t) 1, (size_t) 24, level->file);
	if (readval != (size_t) 24)
	{
		error->code = ERROR_FILE_READ_FAILED;
		error->string[0] = level->path;
		return;
	}
	
	/* Determines what room to work with if no room as provided */
	if ((rep & 0x0002) == 0x0000)
	{
		room = (uint16) memitem[3];
		room <<= 8U;
		room |= (uint16) memitem[2];
	}
	
	/* Sets the coordinates to world coordinates */
	x += level->room[room].x;
	y = (level->room[room].yBottom - y);
	z += level->room[room].z;
	
	/* Overwrites elements of the the item as needed */
	if ((rep & 0x0001) != 0x0000) /* ID */
	{
		memitem[0] = (uint8) (id & 0x00FF);
		memitem[1] = (uint8) ((id & 0xFF00) >> 8U);
	}
	if ((rep & 0x0002) != 0x0000) /* Room */
	{
		memitem[2] = (uint8) (room & 0x00FF);
		memitem[3] = (uint8) ((room & 0xFF00) >> 8U);
	}
	if ((rep & 0x0004) != 0x0000) /* X */
	{
		memitem[4] = (uint8) (x & 0x000000FF);
		memitem[5] = (uint8) ((x & 0x0000FF00) >> 8U);
		memitem[6] = (uint8) ((x & 0x00FF0000) >> 16U);
		memitem[7] = (uint8) ((x & 0xFF000000) >> 24U);
	}
	if ((rep & 0x0008) != 0x0000) /* Y */
	{
		memitem[8] = (uint8) (y & 0x000000FF);
		memitem[9] = (uint8) ((y & 0x0000FF00) >> 8U);
		memitem[10] = (uint8) ((y & 0x00FF0000) >> 16U);
		memitem[11] = (uint8) ((y & 0xFF000000) >> 24U);
	}
	if ((rep & 0x0010) != 0x0000) /* Z */
	{
		memitem[12] = (uint8) (z & 0x000000FF);
		memitem[13] = (uint8) ((z & 0x0000FF00) >> 8U);
		memitem[14] = (uint8) ((z & 0x00FF0000) >> 16U);
		memitem[15] = (uint8) ((z & 0xFF000000) >> 24U);
	}
	if ((rep & 0x0020) != 0x0000) /* Angle */
	{
		memitem[16] = (uint8) (angle & 0x00FF);
		memitem[17] = (uint8) ((angle & 0xFF00) >> 8U);
	}
	if ((rep & 0x0040) != 0x0000) /* Intensity1 */
	{
		memitem[18] = (uint8) (inten1 & 0x00FF);
		memitem[19] = (uint8) ((inten1 & 0xFF00) >> 8U);
	}
	if ((rep & 0x0100) != 0x0000) /* Intensity1 */
	{
		memitem[20] = (uint8) (inten2 & 0x00FF);
		memitem[21] = (uint8) ((inten2 & 0xFF00) >> 8U);
	}
	if ((rep & 0x0080) != 0x0000) /* Flags */
	{
		memitem[22] = (uint8) (flags & 0x00FF);
		memitem[23] = (uint8) ((flags & 0xFF00) >> 8U);
	}
	
	/* Writes the item back to the level */
	seekval = trmod_fseek(level->file, curpos, SEEK_SET);
	if (seekval != 0)
	{
		error->code = ERROR_FILE_READ_FAILED;
		error->string[0] = level->path;
		return;
	}
	writeval = trmod_fwrite(memitem, (size_t) 1, (size_t) 24, level->file);
	if (writeval != (size_t) 24)
	{
		error->code = ERROR_FILE_WRITE_FAILED;
		error->string[0] = level->path;
		return;
	}
}

/*
 * Function that prints a static mesh
 * Parameters:
 * * level = Pointer to level struct
 * * error = Pointer to error struct
 * * room = Room number
 * * mesh = Mesh number
 * * print = Printing flags
 * * * 0 0 0 0  0 0 0 0   0 0 0 0  0 0 0 0
 * * * | | | |  | | | |   | | | |  | | | |
 * * * | | | |  | | | |   | | | |  | | | As command
 * * * | | | |  | | | |   | | | |  | | As replace command
 * * * | | | |  | | | |   | | | |  | In Hexadecimal
 * * * Unused
 */
void tr2pc_get_staticmesh(struct level *level, struct error *error,
                          uint16 room, uint16 mesh, uint16 print)
{
	/* Variable Declarations */
	uint8 memmesh[20]; /* In-memory copy of the mesh */
	long int offset;   /* Offset of the mesh */
	int seekval;       /* Return value of fseek() */
	size_t readval;    /* Return value of fread() */
	uint16 id;         /* The Static Mesh's ID */
	int32 x;           /* The Static Mesh's X-Position */
	int32 y;           /* The Static Mesh's Y-Position */
	int32 z;           /* The Static Mesh's Z-Position */
	int16 angle;       /* The Static Mesh's Angle */
	int16 inten1;      /* The Static Mesh's Intensity1 */
	int16 inten2;      /* The Static Mesh's Intensity2 */
	
	/* Checks whether the room exists */
	if (room >= level->numRooms)
	{
		error->code = ERROR_ROOM_DOESNT_EXIST;
		error->u16[0] = room;
		return;
	}
	/* Checks whether the static mesh exists */
	if (mesh >= level->room[room].value[R_NUMSTATICMESHES])
	{
		error->code = ERROR_STATICMESH_DOESNT_EXIST;
		error->u16[0] = room;
		error->u16[1] = mesh;
		return;
	}
	
	/* Reads the static mesh to be printed into memory */
	offset = (long int) (mesh * 0x0014);
	offset += (long int) (level->room[room].offset[R_NUMSTATICMESHES] +
	                      0x00000002);
	seekval = trmod_fseek(level->file, offset, SEEK_SET);
	if (seekval != 0)
	{
		error->code = ERROR_FILE_READ_FAILED;
		error->string[0] = level->path;
		return;
	}
	readval = trmod_fread(memmesh, (size_t) 1, (size_t) 20, level->file);
	if (readval != (size_t) 20)
	{
		error->code = ERROR_FILE_READ_FAILED;
		error->string[0] = level->path;
		return;
	}
	
	/* Prints as hex dump if needed */
	if ((print & 0x0004) != 0x0000)
	{
		for (id = 0x0000U; id < (uint16) level->partsize[R_STATICMESH]; ++id)
		{
			trmod_printf("%02X ", memmesh[id]);
		}
		trmod_printf("\n");
		return;
	}
	
	/* Interprets the read static mesh */
	x = (int32) ((((uint32) memmesh[3]) << 24U) |
	             (((uint32) memmesh[2]) << 16U) |
	             (((uint32) memmesh[1]) << 8U) | ((uint32) memmesh[0]));
	y = (int32) ((((uint32) memmesh[7]) << 24U) |
	             (((uint32) memmesh[6]) << 16U) |
	             (((uint32) memmesh[5]) << 8U) | ((uint32) memmesh[4]));
	z = (int32) ((((uint32) memmesh[11]) << 24U) |
	             (((uint32) memmesh[10]) << 16U) |
	             (((uint32) memmesh[9]) << 8U) | ((uint32) memmesh[8]));
	angle = (int16) ((((uint16) memmesh[13]) << 8U) | ((uint16) memmesh[12]));
	inten1 = (int16) ((((uint16) memmesh[15]) << 8U) | ((uint16) memmesh[14]));
	inten2 = (int16) ((((uint16) memmesh[17]) << 8U) | ((uint16) memmesh[16]));
	id = (uint16) ((((uint16) memmesh[19]) << 8U) | ((uint16) memmesh[18]));
	
	/* Sets the coordinates to local coordinates */
	x -= level->room[room].x;
	y = (level->room[room].yBottom - y);
	z -= level->room[room].z;
	
	/* Determines how to print the static mesh */
	if ((print & 0x0003) != 0x0000)
	{
		trmod_printf("%s %s %s ", level->command,
		             level->typestring, level->path);
		if ((print & 0x0002) != 0x0000)
		{
			trmod_printf("\"replacestaticmesh(" PRINTU16 ","
			             PRINTU16 "," PRINTU16 ",", room, mesh, id);
		}
		else
		{
			trmod_printf("\"addstaticmesh(" PRINTU16 "," PRINTU16 ",",
			             id, room);
		}
	}
	else
	{
		trmod_printf("Static Mesh(" PRINTU16 "," PRINTU16 ",", id, room);
	}
	
	/* Converts intensity if needed */
	if (inten1 != (int16) 0xFFFF)
	{
		inten1 = (0x1FFF - inten1);
	}
	if (inten2 != (int16) 0xFFFF)
	{
		inten2 = (0x1FFF - inten2);
	}
	
	/* Prints the static mesh */
	trmod_printf(PRINT32 "," PRINT32 "," PRINT32 "," PRINT16 "," PRINT16 "%s,"
	             PRINT16 "%s)", x, z, y, angle, inten1,
	             ((inten1 != (int16) 0xFFFF) ? "/8191" : ""), inten2,
	             ((inten2 != (int16) 0xFFFF) ? "/8191" : ""));
	
	/* Adds the closing quote if needed and prints the newline */
	if ((print & 0x0003) != 0x0000)
	{
		trmod_printf("\"");
	}
	trmod_printf("\n");
}

/*
 * Function that replaces a static mesh in the level
 * Parameters:
 * * level = Pointer to level struct
 * * error = Pointer to error struct
 * * mesh = Number of the mesh to be replaced (last if max int)
 * * id = Static Mesh's ID
 * * room = Static Mesh's room
 * * x = Static Mesh's X-position (in local coordnates)
 * * y = Static Mesh's Y-position (in local coordnates)
 * * z = Static Mesh's Z-position (in local coordnates)
 * * angle = Static Mesh's angle (Euler)
 * * intensity1 = Static Mesh's intensity1 value (8191-0)
 * * intensity2 = Static Mesh's intensity2 value (8191-0)
 * * rep = Bit-mask of elements to replace
 * * * 0 0 0 0  0 0 0 0   0 0 0 0  0 0 0 0
 * * * | | | |  | | | |   | | | |  | | | |
 * * * | | | |  | | | |   | | | |  | | | ID
 * * * | | | |  | | | |   | | | |  | | X
 * * * | | | |  | | | |   | | | |  | Y
 * * * | | | |  | | | |   | | | |  Z
 * * * | | | |  | | | |   | | | Angle
 * * * | | | |  | | | |   | | Intensity1
 * * * | | | |  | | | |   | Intensity2
 * * * Unused
 */
void tr2pc_replacestaticmesh(struct level *level, struct error *error,
                             uint16 mesh, uint16 id, uint16 room, int32 x,
                             int32 y, int32 z, int16 angle, int16 intensity1,
                             int16 intensity2, uint16 rep)
{
	/* Variable Declarations */
	int seekval;       /* Return value for fseek() */
	size_t readval;    /* Return value for fread() */
	size_t writeval;   /* Return value for fwrite() */
	long int curpos;   /* Current position in the file */
	uint8 memmesh[20]; /* In-memory copy of the item */
	
	/* Checks whether the room exists */
	if (room >= level->numRooms)
	{
		error->code = ERROR_ROOM_DOESNT_EXIST;
		error->u16[0] = room;
		return;
	}
	/* Sets mesh number to last one if max int */
	if (mesh == 0xFFFF)
	{
		mesh = level->room[room].value[R_NUMSTATICMESHES];
		--mesh;
	}
	/* Checks whether the static mesh exists */
	if (mesh >= level->room[room].value[R_NUMSTATICMESHES])
	{
		error->code = ERROR_STATICMESH_DOESNT_EXIST;
		error->u16[0] = room;
		error->u16[1] = mesh;
		return;
	}
	
	/* Determines offset of the static mesh */
	curpos = (long int) (level->room[room].offset[R_NUMSTATICMESHES] +
	                     0x00000002 + ((uint32) (mesh * 0x0014)));
	
	/* Reads the existing item into memory */
	seekval = trmod_fseek(level->file, curpos, SEEK_SET);
	if (seekval != 0)
	{
		error->code = ERROR_FILE_READ_FAILED;
		error->string[0] = level->path;
		return;
	}
	readval = trmod_fread(memmesh, (size_t) 1, (size_t) 20, level->file);
	if (readval != (size_t) 20)
	{
		error->code = ERROR_FILE_READ_FAILED;
		error->string[0] = level->path;
		return;
	}
	
	/* Sets the coordinates to world coordinates */
	x += level->room[room].x;
	y = (level->room[room].yBottom - y);
	z += level->room[room].z;
	
	/* Overwrites elements of the the item as needed */
	if ((rep & 0x0001) != 0x0000) /* ID */
	{
		memmesh[18] = (uint8) (id & 0x00FF);
		memmesh[19] = (uint8) ((id & 0xFF00) >> 8U);
	}
	if ((rep & 0x0002) != 0x0000) /* X */
	{
		memmesh[0] = (uint8) (x & 0x000000FF);
		memmesh[1] = (uint8) ((x & 0x0000FF00) >> 8U);
		memmesh[2] = (uint8) ((x & 0x00FF0000) >> 16U);
		memmesh[3] = (uint8) ((x & 0xFF000000) >> 24U);
	}
	if ((rep & 0x0004) != 0x0000) /* Y */
	{
		memmesh[4] = (uint8) (y & 0x000000FF);
		memmesh[5] = (uint8) ((y & 0x0000FF00) >> 8U);
		memmesh[6] = (uint8) ((y & 0x00FF0000) >> 16U);
		memmesh[7] = (uint8) ((y & 0xFF000000) >> 24U);
	}
	if ((rep & 0x0008) != 0x0000) /* Z */
	{
		memmesh[8] = (uint8) (z & 0x000000FF);
		memmesh[9] = (uint8) ((z & 0x0000FF00) >> 8U);
		memmesh[10] = (uint8) ((z & 0x00FF0000) >> 16U);
		memmesh[11] = (uint8) ((z & 0xFF000000) >> 24U);
	}
	if ((rep & 0x0010) != 0x0000) /* Angle */
	{
		memmesh[12] = (uint8) (angle & 0x00FF);
		memmesh[13] = (uint8) ((angle & 0xFF00) >> 8U);
	}
	if ((rep & 0x0020) != 0x0000) /* Intensity1 */
	{
		memmesh[14] = (uint8) (intensity1 & 0x00FF);
		memmesh[15] = (uint8) ((intensity1 & 0xFF00) >> 8U);
	}
	if ((rep & 0x0040) != 0x0000) /* Intensity2 */
	{
		memmesh[16] = (uint8) (intensity2 & 0x00FF);
		memmesh[17] = (uint8) ((intensity2 & 0xFF00) >> 8U);
	}
	
	/* Writes the static mesh back to the level */
	seekval = trmod_fseek(level->file, curpos, SEEK_SET);
	if (seekval != 0)
	{
		error->code = ERROR_FILE_READ_FAILED;
		error->string[0] = level->path;
		return;
	}
	writeval = trmod_fwrite(memmesh, (size_t) 1, (size_t) 20, level->file);
	if (writeval != (size_t) 20)
	{
		error->code = ERROR_FILE_WRITE_FAILED;
		error->string[0] = level->path;
		return;
	}
}

/*
 * Function that prints a light
 * Parameters:
 * * level = Pointer to level struct
 * * error = Pointer to error struct
 * * room = Room number
 * * light = Light number
 * * print = Printing flags
 * * * 0 0 0 0  0 0 0 0   0 0 0 0  0 0 0 0
 * * * | | | |  | | | |   | | | |  | | | |
 * * * | | | |  | | | |   | | | |  | | | As command
 * * * | | | |  | | | |   | | | |  | | As replace command
 * * * | | | |  | | | |   | | | |  | In Hexadecimal
 * * * Unused
 */
void tr2pc_get_light(struct level *level, struct error *error,
                     uint16 room, uint16 light, uint16 print)
{
	/* Variable Declarations */
	uint8 memlight[24]; /* In-memory copy of the mesh */
	long int offset;    /* Offset of the mesh */
	int seekval;        /* Return value of fseek() */
	size_t readval;     /* Return value of fread() */
	int32 x;            /* The Light's X-Position */
	int32 y;            /* The Light's Y-Position */
	int32 z;            /* The Light's Z-Position */
	int16 inten1;       /* The Light's Intensity1 */
	int16 inten2;       /* The Light's Intensity2 */
	uint32 fade1;       /* The Light's Fade1 */
	uint32 fade2;       /* The Light's Fade2 */
	
	/* Checks whether the room exists */
	if (room >= level->numRooms)
	{
		error->code = ERROR_ROOM_DOESNT_EXIST;
		error->u16[0] = room;
		return;
	}
	/* Checks whether the light exists */
	if (light >= level->room[room].value[R_NUMLIGHTS])
	{
		error->code = ERROR_LIGHT_DOESNT_EXIST;
		error->u16[0] = room;
		error->u16[1] = light;
		return;
	}
	
	/* Reads the light to be printed into memory */
	offset = (long int) (light * 0x0018);
	offset += (long int) (level->room[room].offset[R_NUMLIGHTS] + 0x00000002);
	seekval = trmod_fseek(level->file, offset, SEEK_SET);
	if (seekval != 0)
	{
		error->code = ERROR_FILE_READ_FAILED;
		error->string[0] = level->path;
		return;
	}
	readval = trmod_fread(memlight, (size_t) 1, (size_t) 24, level->file);
	if (readval != (size_t) 24)
	{
		error->code = ERROR_FILE_READ_FAILED;
		error->string[0] = level->path;
		return;
	}
	
	/* Prints as hex dump if needed */
	if ((print & 0x0004) != 0x0000)
	{
		for (fade1 = 0x00000000U; fade1 < level->partsize[R_LIGHT]; ++fade1)
		{
			trmod_printf("%02X ", memlight[fade1]);
		}
		trmod_printf("\n");
		return;
	}
	
	/* Interprets the read light */
	x = (int32) ((((uint32) memlight[3]) << 24U) |
	             (((uint32) memlight[2]) << 16U) |
	             (((uint32) memlight[1]) << 8U) | ((uint32) memlight[0]));
	y = (int32) ((((uint32) memlight[7]) << 24U) |
	             (((uint32) memlight[6]) << 16U) |
	             (((uint32) memlight[5]) << 8U) | ((uint32) memlight[4]));
	z = (int32) ((((uint32) memlight[11]) << 24U) |
	             (((uint32) memlight[10]) << 16U) |
	             (((uint32) memlight[9]) << 8U) | ((uint32) memlight[8]));
	inten1 = (int16) ((((uint16) memlight[13]) << 8U) |
	                  ((uint16) memlight[12]));
	inten2 = (int16) ((((uint16) memlight[15]) << 8U) |
	                  ((uint16) memlight[14]));
	fade1 = (uint32) ((((uint32) memlight[19]) << 24U) |
	                  (((uint32) memlight[18]) << 16U) |
	                  (((uint32) memlight[17]) << 8U) |
	                  ((uint32) memlight[16]));
	fade2 = (uint32) ((((uint32) memlight[23]) << 24U) |
	                  (((uint32) memlight[22]) << 16U) |
	                  (((uint32) memlight[21]) << 8U) |
	                  ((uint32) memlight[20]));
	
	/* Sets the coordinates to local coordinates */
	x -= level->room[room].x;
	y = (level->room[room].yBottom - y);
	z -= level->room[room].z;
	
	/* Determines how to print the light */
	if ((print & 0x0003) != 0x0000)
	{
		trmod_printf("%s %s %s ", level->command,
		             level->typestring, level->path);
		if ((print & 0x0002) != 0x0000)
		{
			trmod_printf("\"replacelight(" PRINTU16 "," PRINTU16 ",",
			             room, light);
		}
		else
		{
			trmod_printf("\"addlight(" PRINTU16 ",", room);
		}
	}
	else
	{
		trmod_printf("Light(" PRINTU16 ",", room);
	}
	
	/* Converts intensity if needed */
	if (inten1 != (int16) 0xFFFF)
	{
		inten1 = (0x1FFF - inten1);
	}
	if (inten2 != (int16) 0xFFFF)
	{
		inten2 = (0x1FFF - inten2);
	}
	
	/* Prints the light */
	trmod_printf(PRINT32 "," PRINT32 "," PRINT32 "," PRINT16 "%s," PRINT16 "%s,"
	             PRINTU32 "," PRINTU32 ")", x, z, y, inten1,
	             ((inten1 != (int16) 0xFFFF) ? "/8191" : ""), inten2,
	             ((inten2 != (int16) 0xFFFF) ? "/8191" : ""), fade1, fade2);
	
	/* Adds the closing quote if needed and prints the newline */
	if ((print & 0x0003) != 0x0000)
	{
		trmod_printf("\"");
	}
	trmod_printf("\n");
}

/*
 * Function that replaces a light in the level
 * Parameters:
 * * level = Pointer to level struct
 * * error = Pointer to error struct
 * * light = Number of the light to be replaced (max int means last one)
 * * room = Light's room
 * * x = Light's X-position (in local coordnates)
 * * y = Light's Y-position (in local coordnates)
 * * z = Light's Z-position (in local coordnates)
 * * inten1 = Light's intensity1 value (8191-0)
 * * inten2 = Light's intensity1 value (8191-0)
 * * fade1 = Light's fade1 value
 * * fade2 = Light's fade1 value
 * * rep = Bit-mask of elements to replace
 * * * 0 0 0 0  0 0 0 0   0 0 0 0  0 0 0 0
 * * * | | | |  | | | |   | | | |  | | | |
 * * * | | | |  | | | |   | | | |  | | | X
 * * * | | | |  | | | |   | | | |  | | Y
 * * * | | | |  | | | |   | | | |  | Z
 * * * | | | |  | | | |   | | | |  Intensity1
 * * * | | | |  | | | |   | | | Intensity2
 * * * | | | |  | | | |   | | Fade1
 * * * | | | |  | | | |   | Fade2
 * * * Unused
 */
void tr2pc_replacelight(struct level *level, struct error *error, uint16 light,
                        uint16 room, int32 x, int32 y, int32 z, int16 inten1,
                        int16 inten2, uint32 fade1, uint32 fade2, uint16 rep)
{
	/* Variable Declarations */
	int seekval;        /* Return value for fseek() */
	size_t readval;     /* Return value for fread() */
	size_t writeval;    /* Return value for fwrite() */
	long int curpos;    /* Current position in the file */
	uint8 memlight[24]; /* In-memory copy of the light */
	
	/* Checks whether the room exists */
	if (room >= level->numRooms)
	{
		error->code = ERROR_ROOM_DOESNT_EXIST;
		error->u16[0] = room;
		return;
	}
	/* Sets light to the last one if max int */
	if (light == 0xFFFF)
	{
		light = level->room[room].value[R_NUMLIGHTS];
		--light;
	}
	/* Checks whether the light exists */
	if (light >= level->room[room].value[R_NUMLIGHTS])
	{
		error->code = ERROR_LIGHT_DOESNT_EXIST;
		error->u16[0] = room;
		error->u16[1] = light;
		return;
	}
	
	/* Determines offset of the light */
	curpos = (long int) (level->room[room].offset[R_NUMLIGHTS] +
	                     0x00000002 + ((uint32) (light * 0x0018)));
	
	/* Reads the existing item into memory */
	seekval = trmod_fseek(level->file, curpos, SEEK_SET);
	if (seekval != 0)
	{
		error->code = ERROR_FILE_READ_FAILED;
		error->string[0] = level->path;
		return;
	}
	readval = trmod_fread(memlight, (size_t) 1, (size_t) 24, level->file);
	if (readval != (size_t) 24)
	{
		error->code = ERROR_FILE_READ_FAILED;
		error->string[0] = level->path;
		return;
	}
	
	/* Sets the coordinates to world coordinates */
	x += level->room[room].x;
	y = (level->room[room].yBottom - y);
	z += level->room[room].z;
	
	/* Overwrites the elements of the light if needed */
	if ((rep & 0x0001) != 0x0000) /* X */
	{
		memlight[0] = (uint8) (x & 0x000000FF);
		memlight[1] = (uint8) ((x & 0x0000FF00) >> 8U);
		memlight[2] = (uint8) ((x & 0x00FF0000) >> 16U);
		memlight[3] = (uint8) ((x & 0xFF000000) >> 24U);
	}
	if ((rep & 0x0002) != 0x0000) /* Y */
	{
		memlight[4] = (uint8) (y & 0x000000FF);
		memlight[5] = (uint8) ((y & 0x0000FF00) >> 8U);
		memlight[6] = (uint8) ((y & 0x00FF0000) >> 16U);
		memlight[7] = (uint8) ((y & 0xFF000000) >> 24U);
	}
	if ((rep & 0x0004) != 0x0000) /* Z */
	{
		memlight[8] = (uint8) (z & 0x000000FF);
		memlight[9] = (uint8) ((z & 0x0000FF00) >> 8U);
		memlight[10] = (uint8) ((z & 0x00FF0000) >> 16U);
		memlight[11] = (uint8) ((z & 0xFF000000) >> 24U);
	}
	if ((rep & 0x0008) != 0x0000) /* Intensity1 */
	{
		memlight[12] = (uint8) (inten1 & 0x00FF);
		memlight[13] = (uint8) ((inten1 & 0xFF00) >> 8U);
	}
	if ((rep & 0x0010) != 0x0000) /* Intensity2 */
	{
		memlight[14] = (uint8) (inten2 & 0x00FF);
		memlight[15] = (uint8) ((inten2 & 0xFF00) >> 8U);
	}
	if ((rep & 0x0020) != 0x0000) /* Fade1 */
	{
		memlight[16] = (uint8) (fade1 & 0x000000FF);
		memlight[17] = (uint8) ((fade1 & 0x0000FF00) >> 8U);
		memlight[18] = (uint8) ((fade1 & 0x00FF0000) >> 16U);
		memlight[19] = (uint8) ((fade1 & 0xFF000000) >> 24U);
	}
	if ((rep & 0x0040) != 0x0000) /* Fade2 */
	{
		memlight[20] = (uint8) (fade2 & 0x000000FF);
		memlight[21] = (uint8) ((fade2 & 0x0000FF00) >> 8U);
		memlight[22] = (uint8) ((fade2 & 0x00FF0000) >> 16U);
		memlight[23] = (uint8) ((fade2 & 0xFF000000) >> 24U);
	}
	
	/* Writes the light back to the level */
	seekval = trmod_fseek(level->file, curpos, SEEK_SET);
	if (seekval != 0)
	{
		error->code = ERROR_FILE_READ_FAILED;
		error->string[0] = level->path;
		return;
	}
	writeval = trmod_fwrite(memlight, (size_t) 1, (size_t) 24, level->file);
	if (writeval != (size_t) 24)
	{
		error->code = ERROR_FILE_WRITE_FAILED;
		error->string[0] = level->path;
		return;
	}
}

/*
 * Function that prints a vertex
 * Parameters:
 * * level = Pointer to level struct
 * * error = Pointer to error struct
 * * room = Room number
 * * vertex = Vertex number
 * * print = Printing flags
 * * * 0 0 0 0  0 0 0 0   0 0 0 0  0 0 0 0
 * * * | | | |  | | | |   | | | |  | | | |
 * * * | | | |  | | | |   | | | |  | | | As command
 * * * | | | |  | | | |   | | | |  | | As replace command
 * * * | | | |  | | | |   | | | |  | As part of a surface
 * * * | | | |  | | | |   | | | |  As part of a sprite
 * * * | | | |  | | | |   | | | In Hexadecimal
 * * * Unused
 */
void tr2pc_get_vertex(struct level *level, struct error *error,
                      uint16 room, uint16 vertex, uint16 print)
{
	/* Variable Declarations */
	int16 memvertex[6]; /* In-memory copy of the vertex */
	long int offset;    /* Offset of the vertex */
	int seekval;        /* Return value of fseek() */
	size_t readval;     /* Return value of fread() */
	int32 x;            /* The Vertex's X-Position */
	int32 y;            /* The Vertex's Y-Position */
	int32 z;            /* The Vertex's Z-Position */
	int16 inten1;       /* The Vertex's Intensity1 */
	int16 inten2;       /* The Vertex's Intensity2 */
	uint16 attrib;      /* The Vertex's Attributes */
	
	/* Checks whether the room exists */
	if (room >= level->numRooms)
	{
		error->code = ERROR_ROOM_DOESNT_EXIST;
		error->u16[0] = room;
		return;
	}
	/* Checks whether the vertex exists */
	if (vertex >= level->room[room].value[R_NUMVERTICES])
	{
		error->code = ERROR_VERTEX_DOESNT_EXIST;
		error->u16[0] = room;
		error->u16[1] = vertex;
		return;
	}
	
	/* Reads the vertex to be printed into memory */
	offset = (long int) (vertex * 0x000C);
	offset += (long int) (level->room[room].offset[R_NUMVERTICES] + 0x00000002);
	seekval = trmod_fseek(level->file, offset, SEEK_SET);
	if (seekval != 0)
	{
		error->code = ERROR_FILE_READ_FAILED;
		error->string[0] = level->path;
		return;
	}
	readval = trmod_fread(memvertex, (size_t) 1, (size_t) 12, level->file);
	if (readval != (size_t) 12)
	{
		error->code = ERROR_FILE_READ_FAILED;
		error->string[0] = level->path;
		return;
	}
	
	/* Prints as hex dump if needed */
	if ((print & 0x0010) != 0x0000)
	{
		for (inten1 = 0x0000; inten1 < (int16) level->partsize[R_VERTEX];
		     ++inten1)
		{
			trmod_printf("%02X ", ((uint8 *) memvertex)[inten1]);
		}
		trmod_printf("\n");
		return;
	}
	
	/* Adjusts for Big-Endian */
	if (level->endian == TRUE)
	{
		memvertex[0] = reverse16(memvertex[0]);
		memvertex[1] = reverse16(memvertex[1]);
		memvertex[2] = reverse16(memvertex[2]);
		memvertex[3] = reverse16(memvertex[3]);
		memvertex[4] = reverse16(memvertex[4]);
		memvertex[5] = reverse16(memvertex[5]);
	}
	
	/* Interprets the read the vertex */
	x = (int32) memvertex[0];
	y = (int32) memvertex[1];
	z = (int32) memvertex[2];
	inten1 = (int16) memvertex[3];
	inten2 = (int16) memvertex[5];
	attrib = (uint16) memvertex[4];
	
	/* Sets the y-coordinate to local */
	y = (level->room[room].yBottom - y);
	
	/* Determines how to print the static mesh */
	if ((print & 0x0003) != 0x0000)
	{
		trmod_printf("%s %s %s ", level->command,
		             level->typestring, level->path);
		if ((print & 0x0002) != 0x0000)
		{
			trmod_printf("\"replacevertex(" PRINTU16 "," PRINTU16 ",",
			             room, vertex);
		}
		else
		{
			trmod_printf("\"addvertex(" PRINTU16 ",", room);
		}
	}
	else if ((print & 0x0004) != 0x0000)
	{
		trmod_printf("(");
	}
	else if ((print & 0x0008) == 0x0000)
	{
		trmod_printf("Vertex(" PRINTU16 ",", room);
	}
	
	/* Converts intensity if needed */
	if (inten1 != (int16) 0xFFFF)
	{
		inten1 = (0x1FFF - inten1);
	}
	if (inten2 != (int16) 0xFFFF)
	{
		inten2 = (0x1FFF - inten2);
	}
	
	/* Prints the vertex */
	trmod_printf(PRINT32 "," PRINT32 "," PRINT32 "," PRINT16 "%s," PRINT16
	             "%s,%04X", x, z, y, inten1,
	             ((inten1 != (int16) 0xFFFF) ? "/8191" : ""), inten2,
	             ((inten2 != (int16) 0xFFFF) ? "/8191" : ""), attrib);
	
	/* Prints the closing bracket if needed */
	if ((print & 0x0008) == 0x0000)
	{
		trmod_printf(")");
	}
	
	/* Adds the closing quote if needed and prints the newline */
	if ((print & 0x0003) != 0x0000)
	{
		trmod_printf("\"");
	}
	if ((print & 0x000C) == 0x0000)
	{
		trmod_printf("\n");
	}
}

/*
 * Function that replaces a vertex
 * Parameters:
 * * level = Pointer to level struct
 * * error = Pointer to error struct
 * * room = Room number
 * * vertex = Vertex number (max int means last one)
 * * x = X-position of the vertex (in local coordinates)
 * * y = Y-position of the vertex (in local coordinates)
 * * z = Z-position of the vertex (in local coordinates)
 * * inten1 = Intensity1 of the vertex
 * * inten2 = Intensity2 of the vertex
 * * attrib = Attributes of the vertex
 * * rep = Bit-mask of elements to replace
 * * * 0 0 0 0  0 0 0 0   0 0 0 0  0 0 0 0
 * * * | | | |  | | | |   | | | |  | | | |
 * * * | | | |  | | | |   | | | |  | | | X
 * * * | | | |  | | | |   | | | |  | | Y
 * * * | | | |  | | | |   | | | |  | Z
 * * * | | | |  | | | |   | | | |  Intensity1
 * * * | | | |  | | | |   | | | Intensity2
 * * * | | | |  | | | |   | | Attributes
 * * * Unused
 */
void tr2pc_replace_vertex(struct level *level, struct error *error,
                          uint16 room, uint16 vertex,
                          int32 x, int32 y, int32 z, int16 inten1,
                          int16 inten2, uint32 attrib, uint16 rep)
{
	/* Variable Declarations */
	int16 memvertex[6]; /* In-memory copy of the vertex */
	long int offset;    /* Where to write the vertex */
	int seekval;        /* Return value of fseek() */
	size_t readval;     /* Return value of fread() */
	size_t writeval;    /* Return value of fwrite() */
	
	/* Checks whether the room exists */
	if (room >= level->numRooms)
	{
		error->code = ERROR_ROOM_DOESNT_EXIST;
		error->u16[0] = room;
		return;
	}
	/* Sets vertex to last one if max int */
	if (vertex == 0xFFFF)
	{
		vertex = level->room[room].value[R_NUMVERTICES];
		--vertex;
	}
	/* Checks whether the vertex exists */
	if (vertex >= level->room[room].value[R_NUMVERTICES])
	{
		error->code = ERROR_VERTEX_DOESNT_EXIST;
		error->u16[0] = room;
		error->u16[1] = vertex;
		return;
	}
	
	/* Reads the vertex to be altered into memory */
	offset = (long int) (vertex * 0x000C);
	offset += (long int) (level->room[room].offset[R_NUMVERTICES] + 0x00000002);
	seekval = trmod_fseek(level->file, offset, SEEK_SET);
	if (seekval != 0)
	{
		error->code = ERROR_FILE_READ_FAILED;
		error->string[0] = level->path;
		return;
	}
	readval = trmod_fread(memvertex, (size_t) 1, (size_t) 12, level->file);
	if (readval != (size_t) 12)
	{
		error->code = ERROR_FILE_READ_FAILED;
		error->string[0] = level->path;
		return;
	}
	
	/* Adjusts for Big-Endian */
	if (level->endian == TRUE)
	{
		memvertex[0] = reverse16(memvertex[0]);
		memvertex[1] = reverse16(memvertex[1]);
		memvertex[2] = reverse16(memvertex[2]);
		memvertex[3] = reverse16(memvertex[3]);
		memvertex[4] = reverse16(memvertex[4]);
		memvertex[5] = reverse16(memvertex[5]);
	}
	
	/* Replaces X-value if needed */
	if ((rep & 0x0001) != 0x0000)
	{
		memvertex[0] = (int16) x;
	}
	/* Replaces Y-value if needed */
	if ((rep & 0x0002) != 0x0000)
	{
		y = (level->room[room].yBottom - y);
		memvertex[1] = (int16) y;
	}
	/* Replaces Z-value if needed */
	if ((rep & 0x0004) != 0x0000)
	{
		memvertex[2] = (int16) z;
	}
	/* Replaces Intensity1 if needed */
	if ((rep & 0x0008) != 0x0000)
	{
		memvertex[3] = inten1;
	}
	/* Replaces Intensity2 if needed */
	if ((rep & 0x0010) != 0x0000)
	{
		memvertex[5] = inten2;
	}
	/* Replaces Intensity2 if needed */
	if ((rep & 0x0020) != 0x0000)
	{
		memvertex[4] = (int16) attrib;
	}
	
	/* Adjusts for Big-Endian */
	if (level->endian == TRUE)
	{
		memvertex[0] = reverse16(memvertex[0]);
		memvertex[1] = reverse16(memvertex[1]);
		memvertex[2] = reverse16(memvertex[2]);
		memvertex[3] = reverse16(memvertex[3]);
		memvertex[4] = reverse16(memvertex[4]);
		memvertex[5] = reverse16(memvertex[5]);
	}
	
	/* Writes the vertex to the level */
	seekval = trmod_fseek(level->file, offset, SEEK_SET);
	if (seekval != 0)
	{
		error->code = ERROR_FILE_READ_FAILED;
		error->string[0] = level->path;
		return;
	}
	writeval = trmod_fwrite(memvertex, (size_t) 1, (size_t) 12, level->file);
	if (writeval != (size_t) 12)
	{
		error->code = ERROR_FILE_WRITE_FAILED;
		error->string[0] = level->path;
		return;
	}
}

/*
 * Function that replaces a sprite
 * Parameters:
 * * level = Pointer to level struct
 * * error = Pointer to error struct
 * * room = Room number
 * * sprite = Sprite number (max int means last one)
 * * id = SpriteID
 * * x = X-position of the vertex (in local coordinates)
 * * y = Y-position of the vertex (in local coordinates)
 * * z = Z-position of the vertex (in local coordinates)
 * * inten1 = Intensity1 of the vertex
 * * inten2 = Intensity2 of the vertex
 * * attrib = Attributes of the vertex
 * * rep = Bit-mask of elements to replace
 * * * 0 0 0 0  0 0 0 0   0 0 0 0  0 0 0 0
 * * * | | | |  | | | |   | | | |  | | | |
 * * * | | | |  | | | |   | | | |  | | | ID
 * * * | | | |  | | | |   | | | |  | | X
 * * * | | | |  | | | |   | | | |  | Y
 * * * | | | |  | | | |   | | | |  Z
 * * * | | | |  | | | |   | | | Intensity1
 * * * | | | |  | | | |   | | Intensity2
 * * * | | | |  | | | |   | Attributes
 * * * Unused
 */
void tr2pc_replace_sprite(struct level *level, struct error *error,
                          uint16 room, uint16 sprite, uint32 id,
                          int32 x, int32 y, int32 z, int16 inten1,
                          int16 inten2, uint16 attrib, uint16 rep)
{
	/* Variable Declarations */
	uint16 memsprite[2]; /* In-memory copy of the sprite */
	uint32 cursprseq;    /* Current SpriteSequence */
	uint32 curID;        /* Current SpriteID */
	int16 ver_x;         /* X-position of the vertex */
	int16 ver_y;         /* Y-position of the vertex */
	int16 ver_z;         /* Z-position of the vertex */
	int16 ver_inten1;    /* Intensity1 of the vertex */
	int16 ver_inten2;    /* Intensity2 of the vertex */
	uint16 ver_attrib;   /* Attribute of the vertex */
	long int offset;     /* Position in file to read/write to */
	int seekval;         /* Return value of fseek() */
	size_t readval;      /* Return value of fread() */
	size_t writeval;     /* Return value of fwrite() */
	
	/* Checks whether the room exists */
	if (room >= level->numRooms)
	{
		error->code = ERROR_ROOM_DOESNT_EXIST;
		error->u16[0] = room;
		return;
	}
	/* Sets sprite to last one if max int */
	if (sprite == 0xFFFF)
	{
		sprite = level->room[room].value[R_NUMSPRITES];
		--sprite;
	}
	/* Checks whether the sprite exists */
	if (sprite >= level->room[room].value[R_NUMSPRITES])
	{
		error->code = ERROR_SPRITE_DOESNT_EXIST;
		error->u16[0] = room;
		error->u16[1] = sprite;
		return;
	}
	
	/* Reads the original sprite into memory */
	offset = (long int) (sprite * 0x0004);
	offset += (long int) (level->room[room].offset[R_NUMSPRITES] + 0x00000002);
	seekval = trmod_fseek(level->file, offset, SEEK_SET);
	if (seekval != 0)
	{
		error->code = ERROR_FILE_READ_FAILED;
		error->string[0] = level->path;
		return;
	}
	readval = trmod_fread(memsprite, (size_t) 1, (size_t) 4, level->file);
	if (readval != (size_t) 4)
	{
		error->code = ERROR_FILE_READ_FAILED;
		error->string[0] = level->path;
		return;
	}
	if (level->endian == TRUE)
	{
		memsprite[0] = reverse16(memsprite[0]);
		memsprite[1] = reverse16(memsprite[1]);
	}
	
	/* Reads the original vertex into memory */
	offset = (long int) (memsprite[0] * 0x000C);
	offset += (long int) (level->room[room].offset[R_NUMVERTICES] + 0x00000002);
	seekval = trmod_fseek(level->file, offset, SEEK_SET);
	if (seekval != 0)
	{
		error->code = ERROR_FILE_READ_FAILED;
		error->string[0] = level->path;
		return;
	}
	readval = trmod_fread(&ver_x, (size_t) 1, (size_t) 2, level->file);
	if (readval != (size_t) 2)
	{
		error->code = ERROR_FILE_READ_FAILED;
		error->string[0] = level->path;
		return;
	}
	readval = trmod_fread(&ver_y, (size_t) 1, (size_t) 2, level->file);
	if (readval != (size_t) 2)
	{
		error->code = ERROR_FILE_READ_FAILED;
		error->string[0] = level->path;
		return;
	}
	readval = trmod_fread(&ver_z, (size_t) 1, (size_t) 2, level->file);
	if (readval != (size_t) 2)
	{
		error->code = ERROR_FILE_READ_FAILED;
		error->string[0] = level->path;
		return;
	}
	readval = trmod_fread(&ver_inten1, (size_t) 1, (size_t) 2, level->file);
	if (readval != (size_t) 2)
	{
		error->code = ERROR_FILE_READ_FAILED;
		error->string[0] = level->path;
		return;
	}
	readval = trmod_fread(&ver_inten2, (size_t) 1, (size_t) 2, level->file);
	if (readval != (size_t) 2)
	{
		error->code = ERROR_FILE_READ_FAILED;
		error->string[0] = level->path;
		return;
	}
	readval = trmod_fread(&ver_attrib, (size_t) 1, (size_t) 2, level->file);
	if (readval != (size_t) 2)
	{
		error->code = ERROR_FILE_READ_FAILED;
		error->string[0] = level->path;
		return;
	}
	if (level->endian == TRUE)
	{
		ver_x = reverse16(ver_x);
		ver_y = reverse16(ver_y);
		ver_z = reverse16(ver_z);
		ver_inten1 = reverse16(ver_inten1);
		ver_inten2 = reverse16(ver_inten2);
		ver_attrib = reverse16(ver_attrib);
	}
	ver_y = (int16) (level->room[room].yBottom - (int32) ver_y);
	
	/* Replaces the needed parts of the vertex */
	if ((rep & 0x0002) != 0x0000)
	{
		ver_x = (int16) x;
	}
	if ((rep & 0x0004) != 0x0000)
	{
		ver_y = (int16) y;
	}
	if ((rep & 0x0008) != 0x0000)
	{
		ver_z = (int16) z;
	}
	if ((rep & 0x0010) != 0x0000)
	{
		ver_inten1 = inten1;
	}
	if ((rep & 0x0020) != 0x0000)
	{
		ver_inten2 = inten2;
	}
	if ((rep & 0x0040) != 0x0000)
	{
		ver_attrib = attrib;
	}
	
	/* Adds/Finds the needed vertex */
	memsprite[0] = tr2pc_find_vertex(level, error, room, ver_x, ver_y, ver_z,
	                                 ver_inten1, ver_inten2, ver_attrib);
	if (error->code != ERROR_NONE)
	{
		return;
	}
	
	/* Finds the correct texture offset */
	if ((rep & 0x0001) != 0x0000)
	{
		offset = (long int) (level->offset[NUMSPRITESEQUENCES] + 0x00000004);
		for (cursprseq = 0x00000000;
		     cursprseq < level->value[NUMSPRITESEQUENCES];
		     ++cursprseq)
		{
			/* Reads the SpriteID */
			seekval = trmod_fseek(level->file, offset, SEEK_SET);
			if (seekval != 0)
			{
				error->code = ERROR_FILE_READ_FAILED;
				error->string[0] = level->path;
				return;
			}
			readval = trmod_fread(&curID, (size_t) 1, (size_t) 4, level->file);
			if (readval != (size_t) 4)
			{
				error->code = ERROR_FILE_READ_FAILED;
				error->string[0] = level->path;
				return;
			}
			endian32(curID);
			
			/* Reads the offset if this is the correct SpriteID */
			if (curID == id)
			{
				offset += 6l;
				seekval = trmod_fseek(level->file, offset, SEEK_SET);
				if (seekval != 0)
				{
					error->code = ERROR_FILE_READ_FAILED;
					error->string[0] = level->path;
					return;
				}
				readval = trmod_fread(&memsprite[1], (size_t) 1,
				                      (size_t) 2, level->file);
				if (readval != (size_t) 2)
				{
					error->code = ERROR_FILE_READ_FAILED;
					error->string[0] = level->path;
					return;
				}
				endian16(memsprite[1]);
				break;
			}
			
			/* Moves on to the next Sprite Sequence */
			offset += 8l;
		}
		
		/* Returns an error if no fitting SpriteSequence was found */
		if (cursprseq == level->value[NUMSPRITESEQUENCES])
		{
			error->code = ERROR_SPRITESEQ_NOT_FOUND_ID;
			error->u32[0] = id;
			return;
		}
	}
	
	/* Adjusts for Big-Endian */
	if (level->endian == TRUE)
	{
		memsprite[0] = reverse16(memsprite[0]);
		memsprite[1] = reverse16(memsprite[1]);
	}
	
	/* Overwrites the sprite */
	offset = (long int) (sprite * 0x0004);
	offset += (long int) (level->room[room].offset[R_NUMSPRITES] + 0x00000002);
	seekval = trmod_fseek(level->file, offset, SEEK_SET);
	if (seekval != 0)
	{
		error->code = ERROR_FILE_READ_FAILED;
		error->string[0] = level->path;
		return;
	}
	writeval = trmod_fwrite(memsprite, (size_t) 1, (size_t) 4, level->file);
	if (writeval != (size_t) 4)
	{
		error->code = ERROR_FILE_WRITE_FAILED;
		error->string[0] = level->path;
		return;
	}
}

/*
 * Function that prints the roomlight value of a given room
 * Parameters:
 * * level = Pointer to level struct
 * * error = Pointer to error struct
 * * room = Room number
 * * print = Printing flags
 * * * 0 0 0 0  0 0 0 0   0 0 0 0  0 0 0 0
 * * * | | | |  | | | |   | | | |  | | | |
 * * * | | | |  | | | |   | | | |  | | | As command
 * * * Unused
 */
void tr2pc_get_roomlight(struct level *level, struct error *error,
                         uint16 room, uint16 print)
{
	/* Variable Declarations */
	long int offset;    /* Offset to read from */
	int16 roomlight[3]; /* Roomlight value */
	int seekval;        /* Return value of fseek() */
	size_t readval;     /* Return value of fread() */
	
	/* Checks whether the room exists */
	if (room >= level->numRooms)
	{
		error->code = ERROR_ROOM_DOESNT_EXIST;
		error->u16[0] = room;
		return;
	}
	
	/* Reads the roomlight value into memory */
	offset = (long int) (level->room[room].offset[R_NUMLIGHTS] - 0x00000006);
	seekval = trmod_fseek(level->file, offset, SEEK_SET);
	if (seekval != 0)
	{
		error->code = ERROR_FILE_READ_FAILED;
		error->string[0] = level->path;
		return;
	}
	readval = trmod_fread(roomlight, (size_t) 1, (size_t) 6, level->file);
	if (readval != (size_t) 6)
	{
		error->code = ERROR_FILE_READ_FAILED;
		error->string[0] = level->path;
		return;
	}
	if (level->endian == TRUE)
	{
		roomlight[0] = reverse16(roomlight[0]);
		roomlight[1] = reverse16(roomlight[1]);
		roomlight[2] = reverse16(roomlight[2]);
	}
	
	/* Converts intensity if needed */
	if (roomlight[0] != (int16) 0xFFFF)
	{
		roomlight[0] = (int16) (0x1FFF - roomlight[0]);
	}
	if (roomlight[1] != (int16) 0xFFFF)
	{
		roomlight[1] = (int16) (0x1FFF - roomlight[1]);
	}
	
	/* Determines how to print the roomlight */
	if ((print & 0x0001) != 0x0000)
	{
		trmod_printf("%s %s %s \"roomlight(" PRINTU16 "," PRINT16 "/8191,"
		             PRINT16 "/8191,%04X)\"\n", level->command,\
		             level->typestring, level->path, room, roomlight[0],
		             roomlight[1], roomlight[2]);
	}
	else
	{
		trmod_printf("RoomLight(" PRINTU16 "," PRINT16 "/8191," PRINT16
		             "/8191,%04X)\n", room, roomlight[0],
		             roomlight[1], roomlight[2]);
	}
}

/*
 * Function that changes the roomlight value of a given room
 * Parameters:
 * * level = Pointer to level struct
 * * error = Pointer to error struct
 * * room = Room number
 * * inten1 = Roomlight value (8191-0)
 * * inten2 = Roomlight value (8191-0)
 * * lightmode = Light Mode
 */
void tr2pc_roomlight(struct level *level, struct error *error, uint16 room,
                     int16 inten1, int16 inten2, uint16 lightmode)
{
	/* Variable Declarations */
	long int offset; /* Offset to read from */
	int seekval;     /* Return value of fseek() */
	size_t writeval; /* Return value of fwrite() */
	
	/* Checks whether the room exists */
	if (room >= level->numRooms)
	{
		error->code = ERROR_ROOM_DOESNT_EXIST;
		error->u16[0] = room;
		return;
	}
	
	/* Reads the roomlight value into memory */
	offset = (long int) (level->room[room].offset[R_NUMLIGHTS] - 0x00000006);
	seekval = trmod_fseek(level->file, offset, SEEK_SET);
	if (seekval != 0)
	{
		error->code = ERROR_FILE_READ_FAILED;
		error->string[0] = level->path;
		return;
	}
	if (level->endian == TRUE)
	{
		inten1 = reverse16(inten1);
		inten2 = reverse16(inten2);
		lightmode = reverse16(lightmode);
	}
	writeval = trmod_fwrite(&inten1, (size_t) 1, (size_t) 2, level->file);
	if (writeval != (size_t) 2)
	{
		error->code = ERROR_FILE_WRITE_FAILED;
		error->string[0] = level->path;
		return;
	}
	writeval = trmod_fwrite(&inten2, (size_t) 1, (size_t) 2, level->file);
	if (writeval != (size_t) 2)
	{
		error->code = ERROR_FILE_WRITE_FAILED;
		error->string[0] = level->path;
		return;
	}
	writeval = trmod_fwrite(&lightmode, (size_t) 1, (size_t) 2, level->file);
	if (writeval != (size_t) 2)
	{
		error->code = ERROR_FILE_WRITE_FAILED;
		error->string[0] = level->path;
		return;
	}
}

/*
 * Function that adds a room to the level
 * Parameters:
 * * level = Pointer to the level struct
 * * error = Pointer to the error struct
 */
void tr2pc_addroom(struct level *level, struct error *error)
{
	/* Variable Declarations */
	uint8 buffer[4]; /* Buffer for keeping bytes temporarily */
	long int curpos; /* Current position in the file */
	int seekval;     /* Return value of fseek() */
	size_t writeval; /* Return value of fwrite() */
	
	/* Makes space for this new room */
	insertbytes(level->file, level->path, level->offset[NUMFLOORDATA],
	            0x00000030, error);
	if (error->code != ERROR_NONE)
	{
		return;
	}
	
	/* Zeroes out the room */
	zerobytes(level->file, level->path, level->offset[NUMFLOORDATA],
	          0x00000030, error);
	if (error->code != ERROR_NONE)
	{
		return;
	}
	
	/* Sets NumData to 4 */
	buffer[0] = (uint8) 0x04;
	buffer[1] = (uint8) 0x00;
	buffer[2] = (uint8) 0x00;
	buffer[3] = (uint8) 0x00;
	curpos = (long int) (level->offset[NUMFLOORDATA] + 0x00000010);
	seekval = trmod_fseek(level->file, curpos, SEEK_SET);
	if (seekval != 0)
	{
		error->code = ERROR_FILE_READ_FAILED;
		error->string[0] = level->path;
		return;
	}
	writeval = trmod_fwrite(buffer, (size_t) 1, (size_t) 4, level->file);
	if (writeval != (size_t) 4)
	{
		error->code = ERROR_FILE_WRITE_FAILED;
		error->string[0] = level->path;
		return;
	}
	
	/* Sets alternate room to 0xFFFF */
	buffer[0] = (uint8) 0xFF;
	buffer[1] = (uint8) 0xFF;
	curpos += 28l;
	seekval = trmod_fseek(level->file, curpos, SEEK_SET);
	if (seekval != 0)
	{
		error->code = ERROR_FILE_READ_FAILED;
		error->string[0] = level->path;
		return;
	}
	writeval = trmod_fwrite(buffer, (size_t) 1, (size_t) 2, level->file);
	if (writeval != (size_t) 2)
	{
		error->code = ERROR_FILE_WRITE_FAILED;
		error->string[0] = level->path;
		return;
	}
	
	/* Adds one to NumRooms */
	++(level->numRooms);
	buffer[0] = (uint8) (level->numRooms & 0x00FF);
	buffer[1] = (uint8) ((level->numRooms & 0xFF00) >> 8U);
	seekval = trmod_fseek(level->file,
	                      (long int) level->offset[NUMROOMS], SEEK_SET);
	if (seekval != 0)
	{
		error->code = ERROR_FILE_READ_FAILED;
		error->string[0] = level->path;
		return;
	}
	writeval = trmod_fwrite(buffer, (size_t) 1, (size_t) 2, level->file);
	if (writeval != (size_t) 2)
	{
		error->code = ERROR_FILE_WRITE_FAILED;
		error->string[0] = level->path;
		return;
	}
	
	/* Frees allocated rooms */
	if (level->room != NULL)
	{
		free(level->room);
		level->room = NULL;
	}
	
	/* Re-navigates the level file */
	navigate(level, error);
}

/*
 * Function that changes the room's position
 * Parameters:
 * * level = Pointer to the level struct
 * * error = Pointer to the error struct
 * * room = Room number
 * * x = New X
 * * y = New YBottom
 * * z = New Z
 * * flags = Function flags
 * * * 0 0 0 0  0 0 0 0   0 0 0 0  0 0 0 0
 * * * | | | |  | | | |   | | | |  | | | |
 * * * | | | |  | | | |   | | | |  | | | Adjust vertices
 * * * | | | |  | | | |   | | | |  | | Adjust viewports
 * * * | | | |  | | | |   | | | |  | Adjust collision data
 * * * | | | |  | | | |   | | | |  Adjust lights
 * * * | | | |  | | | |   | | | Adjust static meshes
 * * * | | | |  | | | |   | | Adjust items
 * * * Unused
 */
void tr2pc_moveroom(struct level *level, struct error *error,
                    uint16 room, int32 x, int32 y, int32 z, uint16 flags)
{
	/* Variable Declarations */
	int32 diffx, diffy, diffz; /* Differences on each axis */
	int32 temp32;              /* Temporary 32-bit signed integer */
	int16 temp16;              /* Temporary 16-bit signed integer */
	uint16 tempu16;            /* Temporary 16-bit unsigned integer */
	uint8 tempu8[2];           /* Temporary 8-bit unsigned integers */
	uint16 i, j;               /* Counter variables */
	uint32 k;                  /* Counter variable */
	int seekval;               /* Return value of fseek() */
	size_t readval;            /* Return value of fread() */
	size_t writeval;           /* Return value of fwrite() */
	long int offset;           /* Offset to read/write to */
	
	/* Checks that this room exists */
	if (room >= level->numRooms)
	{
		error->code = ERROR_ROOM_DOESNT_EXIST;
		error->u16[0] = room;
		return;
	}
	
	/* Determines differences on each axis */
	diffx = (x - level->room[room].x);
	diffy = (y - level->room[room].yBottom);
	diffz = (z - level->room[room].z);
	
	/* Changes the room coordinates in the struct */
	level->room[room].x = x;
	level->room[room].yBottom = y;
	level->room[room].z = z;
	level->room[room].yTop += diffy;
	
	/* Changes the room header */
	offset = (long int) (level->room[room].offset[R_NUMVERTICES] - 0x00000014);
	seekval = trmod_fseek(level->file, offset, SEEK_SET);
	if (seekval != 0)
	{
		error->code = ERROR_FILE_READ_FAILED;
		error->string[0] = level->path;
		return;
	}
	temp32 = level->room[room].x;
	endian32(temp32);
	writeval = trmod_fwrite(&temp32, (size_t) 1, (size_t) 4, level->file);
	if (writeval != (size_t) 4)
	{
		error->code = ERROR_FILE_WRITE_FAILED;
		error->string[0] = level->path;
		return;
	}
	temp32 = level->room[room].z;
	endian32(temp32);
	writeval = trmod_fwrite(&temp32, (size_t) 1, (size_t) 4, level->file);
	if (writeval != (size_t) 4)
	{
		error->code = ERROR_FILE_WRITE_FAILED;
		error->string[0] = level->path;
		return;
	}
	temp32 = level->room[room].yBottom;
	endian32(temp32);
	writeval = trmod_fwrite(&temp32, (size_t) 1, (size_t) 4, level->file);
	if (writeval != (size_t) 4)
	{
		error->code = ERROR_FILE_WRITE_FAILED;
		error->string[0] = level->path;
		return;
	}
	temp32 = level->room[room].yTop;
	endian32(temp32);
	writeval = trmod_fwrite(&temp32, (size_t) 1, (size_t) 4, level->file);
	if (writeval != (size_t) 4)
	{
		error->code = ERROR_FILE_WRITE_FAILED;
		error->string[0] = level->path;
		return;
	}
	
	/* Adjusts vertices */
	if ((flags & 0x0001) != 0x0000)
	{
		offset = (long int) (level->room[room].offset[R_NUMVERTICES] +
		                     0x00000004);
		for (i = 0x0000; i < level->room[room].value[R_NUMVERTICES]; ++i)
		{
			/* Reads the original Y-coordinate */
			seekval = trmod_fseek(level->file, offset, SEEK_SET);
			if (seekval != 0)
			{
				error->code = ERROR_FILE_READ_FAILED;
				error->string[0] = level->path;
				return;
			}
			readval = trmod_fread(&temp16, (size_t) 1, (size_t) 2, level->file);
			if (readval != (size_t) 2)
			{
				error->code = ERROR_FILE_READ_FAILED;
				error->string[0] = level->path;
				return;
			}
			endian16(temp16);
			
			/* Adjusts the Y-coordinate */
			temp16 += (int16) diffy;
			
			/* Writes the modified Y-coordinate to the level */
			endian16(temp16);
			seekval = trmod_fseek(level->file, offset, SEEK_SET);
			if (seekval != 0)
			{
				error->code = ERROR_FILE_READ_FAILED;
				error->string[0] = level->path;
				return;
			}
			writeval = trmod_fwrite(&temp16, (size_t) 1,
			                        (size_t) 2, level->file);
			if (writeval != (size_t) 2)
			{
				error->code = ERROR_FILE_WRITE_FAILED;
				error->string[0] = level->path;
				return;
			}
			offset += 12l;
		}
	}
	
	/* Adjusts viewports */
	if ((flags & 0x0002) != 0x0000)
	{
		offset = (long int) (level->room[room].offset[R_NUMDOORS] + 0x00000006);
		for (i = 0x0000; i < level->room[room].value[R_NUMDOORS]; ++i)
		{
			for (j = 0x0000; j < 0x0005; ++j)
			{
				/* Reads the original Y-coordinate */
				seekval = trmod_fseek(level->file, offset, SEEK_SET);
				if (seekval != 0)
				{
					error->code = ERROR_FILE_READ_FAILED;
					error->string[0] = level->path;
					return;
				}
				readval = trmod_fread(&temp16, (size_t) 1,
				                      (size_t) 2, level->file);
				if (readval != (size_t) 2)
				{
					error->code = ERROR_FILE_READ_FAILED;
					error->string[0] = level->path;
					return;
				}
				endian16(temp16);
				
				/* Adjusts the Y-coordinate */
				temp16 += (int16) diffy;
				
				/* Writes the modified Y-coordinate to the level */
				endian16(temp16);
				seekval = trmod_fseek(level->file, offset, SEEK_SET);
				if (seekval != 0)
				{
					error->code = ERROR_FILE_READ_FAILED;
					error->string[0] = level->path;
					return;
				}
				writeval = trmod_fwrite(&temp16, (size_t) 1,
				                        (size_t) 2, level->file);
				if (writeval != (size_t) 2)
				{
					error->code = ERROR_FILE_WRITE_FAILED;
					error->string[0] = level->path;
					return;
				}
				offset += 6l;
			}
			offset += 2l;
		}
	}
	
	/* Adjusts collision data */
	if ((flags & 0x0004) != 0x0000)
	{
		tempu8[1] = (uint8) ((diffy & 0x0000FF00) >> 8U);
		offset = (long int) (level->room[room].offset[R_NUMZSECTORS] +
		                     0x00000009);
		for (i = 0x0000; i < level->room[room].value[R_NUMXSECTORS]; ++i)
		{
			for (j = 0x0000; j < level->room[room].value[R_NUMZSECTORS]; ++j)
			{
				/* Reads the original floor */
				seekval = trmod_fseek(level->file, offset, SEEK_SET);
				if (seekval != 0)
				{
					error->code = ERROR_FILE_READ_FAILED;
					error->string[0] = level->path;
					return;
				}
				readval = trmod_fread(&tempu8[0], (size_t) 1,
				                      (size_t) 1, level->file);
				if (readval != (size_t) 1)
				{
					error->code = ERROR_FILE_READ_FAILED;
					error->string[0] = level->path;
					return;
				}
				
				/* Adjusts the floor if needed */
				if (tempu8[0] != (uint8) 0x81)
				{
					tempu8[0] += tempu8[1];
					
					/* Writes the modified Y-coordinate to the level */
					seekval = trmod_fseek(level->file, offset, SEEK_SET);
					if (seekval != 0)
						{
						error->code = ERROR_FILE_READ_FAILED;
						error->string[0] = level->path;
						return;
					}
					writeval = trmod_fwrite(&tempu8[0], (size_t) 1,
					                        (size_t) 1, level->file);
					if (writeval != (size_t) 1)
					{
						error->code = ERROR_FILE_WRITE_FAILED;
						error->string[0] = level->path;
						return;
					}
				}
				offset += 2l;
				
				/* Reads the original ceiling */
				seekval = trmod_fseek(level->file, offset, SEEK_SET);
				if (seekval != 0)
				{
					error->code = ERROR_FILE_READ_FAILED;
					error->string[0] = level->path;
					return;
				}
				readval = trmod_fread(&tempu8[0], (size_t) 1,
				                      (size_t) 1, level->file);
				if (readval != (size_t) 1)
				{
					error->code = ERROR_FILE_READ_FAILED;
					error->string[0] = level->path;
					return;
				}
				
				/* Adjusts the ceiling if needed */
				if (tempu8[0] != (uint8) 0x81)
				{
					tempu8[0] += tempu8[1];
					
					/* Writes the modified Y-coordinate to the level */
					seekval = trmod_fseek(level->file, offset, SEEK_SET);
					if (seekval != 0)
						{
						error->code = ERROR_FILE_READ_FAILED;
						error->string[0] = level->path;
						return;
					}
					writeval = trmod_fwrite(&tempu8[0], (size_t) 1,
					                        (size_t) 1, level->file);
					if (writeval != (size_t) 1)
					{
						error->code = ERROR_FILE_WRITE_FAILED;
						error->string[0] = level->path;
						return;
					}
				}
				offset += 6l;
			}
		}
	}
	
	/* Adjusts lights */
	if ((flags & 0x0008) != 0x0000)
	{
		offset = (long int) (level->room[room].offset[R_NUMLIGHTS] +
		                     0x00000002);
		for (i = 0x0000; i < level->room[room].value[R_NUMLIGHTS]; ++i)
		{
			/* Reads the original X-coordinate */
			seekval = trmod_fseek(level->file, offset, SEEK_SET);
			if (seekval != 0)
			{
				error->code = ERROR_FILE_READ_FAILED;
				error->string[0] = level->path;
				return;
			}
			readval = trmod_fread(&temp32, (size_t) 1, (size_t) 4, level->file);
			if (readval != (size_t) 4)
			{
				error->code = ERROR_FILE_READ_FAILED;
				error->string[0] = level->path;
				return;
			}
			endian32(temp32);
			
			/* Adjusts the X-coordinate */
			temp32 += diffx;
			
			/* Writes the X-coordinate to the level */
			endian32(temp32);
			seekval = trmod_fseek(level->file, offset, SEEK_SET);
			if (seekval != 0)
			{
				error->code = ERROR_FILE_READ_FAILED;
				error->string[0] = level->path;
				return;
			}
			writeval = trmod_fwrite(&temp32, (size_t) 1,
			                        (size_t) 4, level->file);
			if (writeval != (size_t) 4)
			{
				error->code = ERROR_FILE_WRITE_FAILED;
				error->string[0] = level->path;
				return;
			}
			offset += 4l;
			
			/* Reads the original Y-coordinate */
			seekval = trmod_fseek(level->file, offset, SEEK_SET);
			if (seekval != 0)
			{
				error->code = ERROR_FILE_READ_FAILED;
				error->string[0] = level->path;
				return;
			}
			readval = trmod_fread(&temp32, (size_t) 1, (size_t) 4, level->file);
			if (readval != (size_t) 4)
			{
				error->code = ERROR_FILE_READ_FAILED;
				error->string[0] = level->path;
				return;
			}
			endian32(temp32);
			
			/* Adjusts the Y-coordinate */
			temp32 += diffy;
			
			/* Writes the Y-coordinate to the level */
			endian32(temp32);
			seekval = trmod_fseek(level->file, offset, SEEK_SET);
			if (seekval != 0)
			{
				error->code = ERROR_FILE_READ_FAILED;
				error->string[0] = level->path;
				return;
			}
			writeval = trmod_fwrite(&temp32, (size_t) 1,
			                        (size_t) 4, level->file);
			if (writeval != (size_t) 4)
			{
				error->code = ERROR_FILE_WRITE_FAILED;
				error->string[0] = level->path;
				return;
			}
			offset += 4l;
			
			/* Reads the original Z-coordinate */
			seekval = trmod_fseek(level->file, offset, SEEK_SET);
			if (seekval != 0)
			{
				error->code = ERROR_FILE_READ_FAILED;
				error->string[0] = level->path;
				return;
			}
			readval = trmod_fread(&temp32, (size_t) 1, (size_t) 4, level->file);
			if (readval != (size_t) 4)
			{
				error->code = ERROR_FILE_READ_FAILED;
				error->string[0] = level->path;
				return;
			}
			endian32(temp32);
			
			/* Adjusts the Z-coordinate */
			temp32 += diffz;
			
			/* Writes the Z-coordinate to the level */
			endian32(temp32);
			seekval = trmod_fseek(level->file, offset, SEEK_SET);
			if (seekval != 0)
			{
				error->code = ERROR_FILE_READ_FAILED;
				error->string[0] = level->path;
				return;
			}
			writeval = trmod_fwrite(&temp32, (size_t) 1,
			                        (size_t) 4, level->file);
			if (writeval != (size_t) 4)
			{
				error->code = ERROR_FILE_WRITE_FAILED;
				error->string[0] = level->path;
				return;
			}
			offset += 16l;
		}
	}
	
	/* Adjusts static mesh */
	if ((flags & 0x0010) != 0x0000)
	{
		offset = (long int) (level->room[room].offset[R_NUMSTATICMESHES] +
		                     0x00000002);
		for (i = 0x0000; i < level->room[room].value[R_NUMSTATICMESHES]; ++i)
		{
			/* Reads the original X-coordinate */
			seekval = trmod_fseek(level->file, offset, SEEK_SET);
			if (seekval != 0)
			{
				error->code = ERROR_FILE_READ_FAILED;
				error->string[0] = level->path;
				return;
			}
			readval = trmod_fread(&temp32, (size_t) 1, (size_t) 4, level->file);
			if (readval != (size_t) 4)
			{
				error->code = ERROR_FILE_READ_FAILED;
				error->string[0] = level->path;
				return;
			}
			endian32(temp32);
			
			/* Adjusts the X-coordinate */
			temp32 += diffx;
			
			/* Writes the X-coordinate to the level */
			endian32(temp32);
			seekval = trmod_fseek(level->file, offset, SEEK_SET);
			if (seekval != 0)
			{
				error->code = ERROR_FILE_READ_FAILED;
				error->string[0] = level->path;
				return;
			}
			writeval = trmod_fwrite(&temp32, (size_t) 1,
			                        (size_t) 4, level->file);
			if (writeval != (size_t) 4)
			{
				error->code = ERROR_FILE_WRITE_FAILED;
				error->string[0] = level->path;
				return;
			}
			offset += 4l;
			
			/* Reads the original Y-coordinate */
			seekval = trmod_fseek(level->file, offset, SEEK_SET);
			if (seekval != 0)
			{
				error->code = ERROR_FILE_READ_FAILED;
				error->string[0] = level->path;
				return;
			}
			readval = trmod_fread(&temp32, (size_t) 1, (size_t) 4, level->file);
			if (readval != (size_t) 4)
			{
				error->code = ERROR_FILE_READ_FAILED;
				error->string[0] = level->path;
				return;
			}
			endian32(temp32);
			
			/* Adjusts the Y-coordinate */
			temp32 += diffy;
			
			/* Writes the Y-coordinate to the level */
			endian32(temp32);
			seekval = trmod_fseek(level->file, offset, SEEK_SET);
			if (seekval != 0)
			{
				error->code = ERROR_FILE_READ_FAILED;
				error->string[0] = level->path;
				return;
			}
			writeval = trmod_fwrite(&temp32, (size_t) 1,
			                        (size_t) 4, level->file);
			if (writeval != (size_t) 4)
			{
				error->code = ERROR_FILE_WRITE_FAILED;
				error->string[0] = level->path;
				return;
			}
			offset += 4l;
			
			/* Reads the original Z-coordinate */
			seekval = trmod_fseek(level->file, offset, SEEK_SET);
			if (seekval != 0)
			{
				error->code = ERROR_FILE_READ_FAILED;
				error->string[0] = level->path;
				return;
			}
			readval = trmod_fread(&temp32, (size_t) 1, (size_t) 4, level->file);
			if (readval != (size_t) 4)
			{
				error->code = ERROR_FILE_READ_FAILED;
				error->string[0] = level->path;
				return;
			}
			endian32(temp32);
			
			/* Adjusts the Z-coordinate */
			temp32 += diffz;
			
			/* Writes the Z-coordinate to the level */
			endian32(temp32);
			seekval = trmod_fseek(level->file, offset, SEEK_SET);
			if (seekval != 0)
			{
				error->code = ERROR_FILE_READ_FAILED;
				error->string[0] = level->path;
				return;
			}
			writeval = trmod_fwrite(&temp32, (size_t) 1,
			                        (size_t) 4, level->file);
			if (writeval != (size_t) 4)
			{
				error->code = ERROR_FILE_WRITE_FAILED;
				error->string[0] = level->path;
				return;
			}
			offset += 12l;
		}
	}
	
	/* Adjusts items */
	if ((flags & 0x0020) != 0x0000)
	{
		offset = (long int) (level->offset[NUMENTITIES] + 0x00000006);
		for (k = 0x00000000; k < level->value[NUMENTITIES]; ++k)
		{
			/* Reads the room number */
			seekval = trmod_fseek(level->file, offset, SEEK_SET);
			if (seekval != 0)
			{
				error->code = ERROR_FILE_READ_FAILED;
				error->string[0] = level->path;
				return;
			}
			readval = trmod_fread(&tempu16, (size_t) 1,
			                      (size_t) 2, level->file);
			if (readval != (size_t) 2)
			{
				error->code = ERROR_FILE_READ_FAILED;
				error->string[0] = level->path;
				return;
			}
			endian16(tempu16);
			
			/* Skips this item if the item isn't in the moved room */
			if (tempu16 != room)
			{
				offset += level->partsize[ENTITY];
				continue;
			}
			else
			{
				offset += 2l;
			}
			
			/* Reads the original X-coordinate */
			seekval = trmod_fseek(level->file, offset, SEEK_SET);
			if (seekval != 0)
			{
				error->code = ERROR_FILE_READ_FAILED;
				error->string[0] = level->path;
				return;
			}
			readval = trmod_fread(&temp32, (size_t) 1, (size_t) 4, level->file);
			if (readval != (size_t) 4)
			{
				error->code = ERROR_FILE_READ_FAILED;
				error->string[0] = level->path;
				return;
			}
			endian32(temp32);
			
			/* Adjusts the X-coordinate */
			temp32 += diffx;
			
			/* Writes the X-coordinate to the level */
			endian32(temp32);
			seekval = trmod_fseek(level->file, offset, SEEK_SET);
			if (seekval != 0)
			{
				error->code = ERROR_FILE_READ_FAILED;
				error->string[0] = level->path;
				return;
			}
			writeval = trmod_fwrite(&temp32, (size_t) 1,
			                        (size_t) 4, level->file);
			if (writeval != (size_t) 4)
			{
				error->code = ERROR_FILE_WRITE_FAILED;
				error->string[0] = level->path;
				return;
			}
			offset += 4l;
			
			/* Reads the original Y-coordinate */
			seekval = trmod_fseek(level->file, offset, SEEK_SET);
			if (seekval != 0)
			{
				error->code = ERROR_FILE_READ_FAILED;
				error->string[0] = level->path;
				return;
			}
			readval = trmod_fread(&temp32, (size_t) 1, (size_t) 4, level->file);
			if (readval != (size_t) 4)
			{
				error->code = ERROR_FILE_READ_FAILED;
				error->string[0] = level->path;
				return;
			}
			endian32(temp32);
			
			/* Adjusts the Y-coordinate */
			temp32 += diffy;
			
			/* Writes the Y-coordinate to the level */
			endian32(temp32);
			seekval = trmod_fseek(level->file, offset, SEEK_SET);
			if (seekval != 0)
			{
				error->code = ERROR_FILE_READ_FAILED;
				error->string[0] = level->path;
				return;
			}
			writeval = trmod_fwrite(&temp32, (size_t) 1,
			                        (size_t) 4, level->file);
			if (writeval != (size_t) 4)
			{
				error->code = ERROR_FILE_WRITE_FAILED;
				error->string[0] = level->path;
				return;
			}
			offset += 4l;
			
			/* Reads the original Z-coordinate */
			seekval = trmod_fseek(level->file, offset, SEEK_SET);
			if (seekval != 0)
			{
				error->code = ERROR_FILE_READ_FAILED;
				error->string[0] = level->path;
				return;
			}
			readval = trmod_fread(&temp32, (size_t) 1, (size_t) 4, level->file);
			if (readval != (size_t) 4)
			{
				error->code = ERROR_FILE_READ_FAILED;
				error->string[0] = level->path;
				return;
			}
			endian32(temp32);
			
			/* Adjusts the Z-coordinate */
			temp32 += diffz;
			
			/* Writes the Z-coordinate to the level */
			endian32(temp32);
			seekval = trmod_fseek(level->file, offset, SEEK_SET);
			if (seekval != 0)
			{
				error->code = ERROR_FILE_READ_FAILED;
				error->string[0] = level->path;
				return;
			}
			writeval = trmod_fwrite(&temp32, (size_t) 1,
			                        (size_t) 4, level->file);
			if (writeval != (size_t) 4)
			{
				error->code = ERROR_FILE_WRITE_FAILED;
				error->string[0] = level->path;
				return;
			}
			offset += 14l;
		}
	}
}

/*
 * Function that prints a box.
 * Parameters:
 * * level = Pointer to level struct
 * * error = Pointer to error struct
 * * box = Box number
 * * print = Printing flags
 * * * 0 0 0 0  0 0 0 0   0 0 0 0  0 0 0 0
 * * * | | | |  | | | |   | | | |  | | | |
 * * * | | | |  | | | |   | | | |  | | | As command
 * * * | | | |  | | | |   | | | |  | | As replace command
 * * * Unused
 */
void tr2pc_getbox(struct level *level, struct error *error,
                  uint32 box, uint16 print)
{
	/* Variable Declarations */
	uint8 boxbytes[4];       /* Bytes from the box */
	int32 boxcrd[4];         /* Coordinates from the box */
	int16 truefloor;         /* TrueFloor from the box */
	uint16 overlapindex;     /* OverlapIndex from the box */
	int16 zone[10];          /* The zones */
	uint16 room;             /* The closest room to the box */
	int boxfound = 0;        /* Whether the needed box is found */
	uint16 curBoxIndex;      /* BoxIndex of the current sector */
	uint32 cursector;        /* Current sector in search */
	uint32 numsectors;       /* Total number of sectors */
	uint32 curOverlap;       /* Current overlap in processing */
	uint32 numOverlaps;      /* Number of overlaps in this box */
	uint16 *overlaps = NULL; /* In-memory overlaps */
	int blocked = 0;         /* Whether the blocked-flag is set */
	int blockable = 0;       /* Whether the blockable-flag is set */
	int i;                   /* Counter variable */
	int seekval;             /* Return value of fseek() */
	size_t readval;          /* Return value of fread() */
	long int offset;         /* Offset to read from */
	
	/* Checks whether the box exists */
	if (box >= level->value[NUMBOXES])
	{
		error->code = ERROR_BOX_DOESNT_EXIST;
		error->u32[0] = box;
		goto end;
	}
	
	/* Reads the box into memory */
	offset = (long int) (box * 0x00000008);
	offset += (long int) (level->offset[NUMBOXES] + 0x00000004);
	seekval = trmod_fseek(level->file, offset, SEEK_SET);
	if (seekval != 0)
	{
		error->code = ERROR_FILE_READ_FAILED;
		error->string[0] = level->path;
		goto end;
	}
	readval = trmod_fread(boxbytes, (size_t) 1, (size_t) 4, level->file);
	if (readval != (size_t) 4)
	{
		error->code = ERROR_FILE_READ_FAILED;
		error->string[0] = level->path;
		goto end;
	}
	boxcrd[0] = (uint32) boxbytes[0];
	boxcrd[1] = (uint32) boxbytes[1];
	boxcrd[2] = (uint32) boxbytes[2];
	boxcrd[3] = (uint32) boxbytes[3];
	boxcrd[0] <<= 10U;
	boxcrd[1] <<= 10U;
	boxcrd[2] <<= 10U;
	boxcrd[3] <<= 10U;
	if ((boxcrd[0] & 0x00020000) != 0x00000000)
	{
		boxcrd[0] |= 0xFFFC0000;
	}
	if ((boxcrd[1] & 0x00020000) != 0x00000000)
	{
		boxcrd[1] |= 0xFFFC0000;
	}
	if ((boxcrd[2] & 0x00020000) != 0x00000000)
	{
		boxcrd[2] |= 0xFFFC0000;
	}
	if ((boxcrd[3] & 0x00020000) != 0x00000000)
	{
		boxcrd[3] |= 0xFFFC0000;
	}
	readval = trmod_fread(&truefloor, (size_t) 1, (size_t) 2, level->file);
	if (readval != (size_t) 2)
	{
		error->code = ERROR_FILE_READ_FAILED;
		error->string[0] = level->path;
		goto end;
	}
	readval = trmod_fread(&overlapindex, (size_t) 1, (size_t) 2, level->file);
	if (readval != (size_t) 2)
	{
		error->code = ERROR_FILE_READ_FAILED;
		error->string[0] = level->path;
		goto end;
	}
	
	/* Reads the zone into memory */
	offset = (long int) (box * 0x00000002);
	offset += (long int) level->offset[ZONE];
	for (i = 0; i < 10; ++i)
	{
		seekval = trmod_fseek(level->file, offset, SEEK_SET);
		if (seekval != 0)
		{
			error->code = ERROR_FILE_READ_FAILED;
			error->string[0] = level->path;
			goto end;
		}
		readval = trmod_fread(&zone[i], (size_t) 1, (size_t) 2, level->file);
		if (readval != (size_t) 2)
		{
			error->code = ERROR_FILE_READ_FAILED;
			error->string[0] = level->path;
			goto end;
		}
		/* Moves to the next part of this zone */
		offset += (long int) level->value[NUMBOXES];
		offset += (long int) level->value[NUMBOXES];
	}
	
	/* Adjusts for Big-Endian */
	if (level->endian == TRUE)
	{
		truefloor = reverse16(truefloor);
		overlapindex = reverse16(overlapindex);
		zone[0] = reverse16(zone[0]);
		zone[1] = reverse16(zone[1]);
		zone[2] = reverse16(zone[2]);
		zone[3] = reverse16(zone[3]);
		zone[4] = reverse16(zone[4]);
		zone[5] = reverse16(zone[5]);
		zone[6] = reverse16(zone[6]);
		zone[7] = reverse16(zone[7]);
		zone[8] = reverse16(zone[8]);
		zone[9] = reverse16(zone[9]);
	}
	
	/* Reads overlaps for this zone into memory */
	if (overlapindex != 0xFFFF)
	{
		if ((overlapindex & 0x8000U) != 0x0000U)
		{
			blockable = 1;
		}
		if ((overlapindex & 0x4000U) != 0x0000U)
		{
			blocked = 1;
		}
		overlapindex &= 0x3FFFU;
	}
	else
	{
		curOverlap = numOverlaps = 0x00000000;
	}
	if (overlapindex < level->value[NUMOVERLAPS])
	{
		numOverlaps = (level->value[NUMOVERLAPS] - (uint32) overlapindex);
		overlaps = calloc((size_t) numOverlaps, (size_t) 2);
		if (overlaps == NULL)
		{
			error->code = ERROR_MEMORY;
			goto end;
		}
		offset = (long int) (overlapindex * 0x0002);
		offset += (long int) (level->offset[NUMOVERLAPS] + 0x00000004);
		seekval = trmod_fseek(level->file, offset, SEEK_SET);
		if (seekval != 0)
		{
			error->code = ERROR_FILE_READ_FAILED;
			error->string[0] = level->path;
			goto end;
		}
		readval = trmod_fread(overlaps, (size_t) 1,
		                      (size_t) (numOverlaps * 0x00000002), level->file);
		if (readval != (size_t) (numOverlaps * 0x00000002))
		{
			error->code = ERROR_FILE_READ_FAILED;
			error->string[0] = level->path;
			goto end;
		}
		
		/* Counts how long this list of overlaps is */
		for (curOverlap = 0x00000000; curOverlap < numOverlaps; ++curOverlap)
		{
			if ((overlaps[curOverlap] & 0x8000) != 0x0000)
			{
				numOverlaps = (curOverlap + 0x00000001);
				break;
			}
		}
	}
	else
	{
		curOverlap = numOverlaps = 0x00000000;
	}
	
	/* Finds which room this zone belongs to */
	room = 0x0000;
	while ((room < level->numRooms) && (boxfound == 0))
	{
		offset = (long int) (level->room[room].offset[R_NUMZSECTORS] +
		                     0x00000006);
		numsectors = (uint32) level->room[room].value[R_NUMXSECTORS];
		numsectors *= (uint32) level->room[room].value[R_NUMZSECTORS];
		for (cursector = 0x00000000; cursector < numsectors; ++cursector)
		{
			/* Reads the current BoxIndex into memory */
			seekval = trmod_fseek(level->file, offset, SEEK_SET);
			if (seekval != 0)
			{
				error->code = ERROR_FILE_READ_FAILED;
				error->string[0] = level->path;
				goto end;
			}
			readval = trmod_fread(&curBoxIndex, (size_t) 1,
			                      (size_t) 2, level->file);
			if (readval != (size_t) 2)
			{
				error->code = ERROR_FILE_READ_FAILED;
				error->string[0] = level->path;
				goto end;
			}
			endian16(curBoxIndex);
			
			/* Stops if the wanted box was found */
			if (curBoxIndex == (uint16) box)
			{
				boxfound = 1;
				break;
			}
			offset += 8l;
		}
		if (boxfound == 0)
		{
			++room;
		}
	}
	
	/* Finds the nearest room if the box isn't used (shouldn't happen) */
	if (boxfound == 0)
	{
		room = nearestRoom(level, error, boxcrd[2],
		                   boxcrd[0], (int32) truefloor);
	}
	
	/* Sets the coordinates to local coordinates */
	boxcrd[0] -= level->room[room].z;
	boxcrd[1] -= level->room[room].z;
	boxcrd[2] -= level->room[room].x;
	boxcrd[3] -= level->room[room].x;
	truefloor = (level->room[room].yBottom - truefloor);
	
	/* Determines how to print the box */
	if ((print & 0x0003) != 0x0000)
	{
		trmod_printf("%s %s %s ", level->command,
		             level->typestring, level->path);
		if ((print & 0x0002) != 0x0000)
		{
			trmod_printf("\"replacebox(" PRINTU32 ",", box);
		}
		else
		{
			trmod_printf("\"addbox(");
		}
	}
	else
	{
		trmod_printf("Box(");
	}
	
	/* Prints the box's coordinates */
	trmod_printf("(" PRINTU16 "," PRINT32 "," PRINT32 "," PRINT16 "),(" PRINTU16
	             "," PRINT32 "," PRINT32 "," PRINT16 "),", room, boxcrd[2],
	             boxcrd[0], truefloor, room, boxcrd[3], boxcrd[1], truefloor);
	
	/* Prints the zones */
	trmod_printf("(" PRINT16 "," PRINT16 "," PRINT16 "," PRINT16 "," PRINT16
	             "),(" PRINT16 "," PRINT16 "," PRINT16 "," PRINT16 "," PRINT16
	             "),", zone[0], zone[1], zone[2], zone[3], zone[4], zone[5],
	             zone[6], zone[7], zone[8], zone[9]);
	
	/* Prints the overlaps */
	trmod_printf("[");
	for (curOverlap = 0x00000000; curOverlap < numOverlaps; ++curOverlap)
	{
		trmod_printf(PRINTU16, (overlaps[curOverlap] & 0x7FFF));
		if (curOverlap < (numOverlaps - 0x00000001))
		{
			trmod_printf(",");
		}
	}
	trmod_printf("]");
	
	/* Prints the flags */
	if ((blocked == 1) && (blockable == 1))
	{
		trmod_printf(",both");
	}
	else if ((blocked == 1) && (blockable == 0))
	{
		trmod_printf(",blocked");
	}
	else if ((blocked == 0) && (blockable == 1))
	{
		trmod_printf(",blockable");
	}
	trmod_printf(")");
	
	/* Adds a closing quote mark if needed */
	if ((print & 0x0001) != 0x0000)
	{
		trmod_printf("\"");
	}
	trmod_printf("\n");
	
end:/* Ends allocated memory and ends the function */
	if (overlaps != NULL)
	{
		free(overlaps);
	}
}

/*
 * Function that adds a box to the level
 * Parameters:
 * * level = Pointer to level struct
 * * error = Pointer to error struct
 */
void tr2pc_add_box(struct level *level, struct error *error)
{
	/* Variable Declarations */
	int i;           /* Counter variable for loops */
	uint32 offset;   /* Offset where to read/write to */
	uint32 numBoxes; /* Copy of NumBoxes to use */
	int seekval;     /* Return value of fseek() */
	size_t writeval; /* Return value of fwrite() */
	
	/* Adds space for the zone */
	offset = level->offset[ZONE];
	offset += (level->value[NUMBOXES] * 0x00000002);
	(level->offset[ZONE])--; /* To keep zones separate from animated textures */
	for (i = 0; i < 10; ++i)
	{
		insertbytes(level->file, level->path, offset, 0x00000002, error);
		if (error->code != ERROR_NONE)
		{
			return;
		}
		level_struct_add(level, offset, 0x00000002);
		offset += (0x00000002 + (level->value[NUMBOXES] * 0x00000002));
	}
	(level->offset[ZONE])++; /* Adjusts for the earlier iteration */
	
	/* Makes space for the box */
	insertbytes(level->file, level->path, level->offset[NUMOVERLAPS],
	            0x00000008, error);
	if (error->code != ERROR_NONE)
	{
		return;
	}
	level_struct_add(level, level->offset[NUMOVERLAPS], 0x00000008);
	
	/* Adds one to NumBoxes */
	++(level->value[NUMBOXES]);
	numBoxes = level->value[NUMBOXES];
	endian32(numBoxes);
	seekval = trmod_fseek(level->file,
	                      (long int) level->offset[NUMBOXES], SEEK_SET);
	if (seekval != 0)
	{
		error->code = ERROR_FILE_READ_FAILED;
		error->string[0] = level->path;
		return;
	}
	writeval = trmod_fwrite(&numBoxes, (size_t) 1, (size_t) 4, level->file);
	if (writeval != (size_t) 4)
	{
		error->code = ERROR_FILE_WRITE_FAILED;
		error->string[0] = level->path;
		return;
	}
}

/*
 * Function that replaces a box
 * Parameters:
 * * level = Pointer to level struct
 * * error = Pointer to error struct
 * * box = Box number
 * * room = Room to which the coordinates relate
 * * xmin = Xmin for the box (in local coordinates)
 * * xmax = Xmax for the box (in local coordinates)
 * * zmin = Zmin for the box (in local coordinates)
 * * zmax = Zmax for the box (in local coordinates)
 * * floor = Truefloor for the box (in local coordinates)
 * * gz1 = Ground Zone 1
 * * gz2 = Ground Zone 2
 * * gz3 = Ground Zone 3
 * * gz4 = Ground Zone 4
 * * fz = Fly Zone
 * * agz1 = Alternate Ground Zone 1
 * * agz2 = Alternate Ground Zone 2
 * * agz3 = Alternate Ground Zone 3
 * * agz4 = Alternate Ground Zone 4
 * * afz = Alternate Fly Zone
 * * overlapstr = String-version of the overlaps
 * * blocked = Whether the blocked-flag is set
 * * blockable = Whether the blockable-flag is set
 * * rep = Bit-mask of elements to replace
 * * * 0 0 0 0  0 0 0 0  0 0 0 0  0 0 0 0  0 0 0 0  0 0 0 0  0 0 0 0  0 0 0 0
 * * * | | | |  | | | |  | | | |  | | | |  | | | |  | | | |  | | | |  | | | |
 * * * | | | |  | | | |  | | | |  | | | |  | | | |  | | | |  | | | |  | | | Xmin
 * * * | | | |  | | | |  | | | |  | | | |  | | | |  | | | |  | | | |  | | Xmax
 * * * | | | |  | | | |  | | | |  | | | |  | | | |  | | | |  | | | |  | Zmin
 * * * | | | |  | | | |  | | | |  | | | |  | | | |  | | | |  | | | |  Zmax
 * * * | | | |  | | | |  | | | |  | | | |  | | | |  | | | |  | | | TrueFloor
 * * * | | | |  | | | |  | | | |  | | | |  | | | |  | | | |  | | Ground Zone 1
 * * * | | | |  | | | |  | | | |  | | | |  | | | |  | | | |  | Ground Zone 2
 * * * | | | |  | | | |  | | | |  | | | |  | | | |  | | | |  Ground Zone 3
 * * * | | | |  | | | |  | | | |  | | | |  | | | |  | | | Ground Zone 4
 * * * | | | |  | | | |  | | | |  | | | |  | | | |  | | Fly Zone
 * * * | | | |  | | | |  | | | |  | | | |  | | | |  | Alt Ground Zone 1
 * * * | | | |  | | | |  | | | |  | | | |  | | | |  Alt Ground Zone 2
 * * * | | | |  | | | |  | | | |  | | | |  | | | Alt Ground Zone 3
 * * * | | | |  | | | |  | | | |  | | | |  | | Alt Ground Zone 4
 * * * | | | |  | | | |  | | | |  | | | |  | Alt Fly Zone
 * * * | | | |  | | | |  | | | |  | | | |  Overlaps
 * * * | | | |  | | | |  | | | |  | | | Blocked-flag
 * * * | | | |  | | | |  | | | |  | | Blockable-flag
 * * * Unused
 */
void tr2pc_replace_box(struct level *level, struct error *error, uint32 box,
                       uint16 room, int32 xmin, int32 xmax, int32 zmin,
                       int32 zmax, int16 floor, int16 gz1, int16 gz2,
                       int16 gz3, int16 gz4, int16 fz, int16 agz1, int16 agz2,
                       int16 agz3, int16 agz4, int16 afz, char *overlapstr,
                       int blocked, int blockable, uint32 rep)
{
	/* Variable Declarations */
	long int offset;         /* Offset to read/write from */
	int seekval;             /* Return value of fseek() */
	size_t readval;          /* Return value of fread() */
	size_t writeval;         /* Return value of fwrite() */
	uint16 *overlaps = NULL; /* In-memory copy of overlaps */
	uint16 curoverlap;       /* Current overlap in searching */
	uint16 *newol = NULL;    /* New overlap-entry */
	uint16 overlapindex;     /* OverlapIndex to write to box */
	char **olsubstr = NULL;  /* Array of overlap numbers */
	uint16 numolsubstr;      /* Number of overlap numbers */
	size_t numolsubbyt;      /* Number of overlap bytes */
	uint16 curolsubstr;      /* Current overlap number */
	uint32 numOverlaps;      /* Copy of NumOverlaps for writing */
	uint8 boxcrd[4];         /* Box coordinates */
	
	/* Sets box to last one if max int */
	if (box == 0xFFFFFFFF)
	{
		box = level->value[NUMBOXES];
		--box;
	}
	/* Checks whether the box exists */
	if (box >= level->value[NUMBOXES])
	{
		error->code = ERROR_BOX_DOESNT_EXIST;
		error->u32[0] = box;
		goto end;
	}
	
	/* Checks whether the room exists if needed */
	if (((rep & 0x0000001F) != 0x00000000) && (room >= level->numRooms))
	{
		error->code = ERROR_ROOM_DOESNT_EXIST;
		error->u16[0] = room;
		goto end;
	}
	
	/* Checks whether the overlapstr isn't NULL if it's needed */
	if (((rep & 0x00010000) != 0x00000000) && (overlapstr == NULL))
	{
		error->code = ERROR_NULL_POINTER;
		goto end;
	}
	
	/* Converts coordinates to local */
	if ((rep & 0x0000001F) != 0x00000000)
	{
		xmin += level->room[room].x;
		xmax += level->room[room].x;
		zmin += level->room[room].z;
		zmax += level->room[room].z;
		floor = (level->room[room].yBottom - floor);
	}
	boxcrd[0] = (uint8) ((zmin >> 10U) & 0x000000FF);
	boxcrd[1] = (uint8) ((zmax >> 10U) & 0x000000FF);
	boxcrd[2] = (uint8) ((xmin >> 10U) & 0x000000FF);
	boxcrd[3] = (uint8) ((xmax >> 10U) & 0x000000FF);
	
	/* Adjusts for Big-Endian */
	if (level->endian == TRUE)
	{
		floor = reverse16(floor);
		gz1 = reverse16(gz1);
		gz2 = reverse16(gz2);
		gz3 = reverse16(gz3);
		gz4 = reverse16(gz4);
		fz = reverse16(fz);
		agz1 = reverse16(agz1);
		agz2 = reverse16(agz2);
		agz3 = reverse16(agz3);
		agz4 = reverse16(agz4);
		afz = reverse16(afz);
	}
	
	/* Replaces coordinates if needed */
	offset = (long int) (box * 0x00000008);
	offset += (long int) (level->offset[NUMBOXES] + 0x00000004);
	if ((rep & 0x00000004) != 0x00000000)
	{
		seekval = trmod_fseek(level->file, offset, SEEK_SET);
		if (seekval != 0)
		{
			error->code = ERROR_FILE_READ_FAILED;
			error->string[0] = level->path;
			goto end;
		}
		writeval = trmod_fwrite(&boxcrd[0], (size_t) 1,
		                        (size_t) 1, level->file);
		if (writeval != (size_t) 1)
		{
			error->code = ERROR_FILE_WRITE_FAILED;
			error->string[0] = level->path;
			goto end;
		}
	}
	offset += 1l;
	if ((rep & 0x00000008) != 0x00000000)
	{
		seekval = trmod_fseek(level->file, offset, SEEK_SET);
		if (seekval != 0)
		{
			error->code = ERROR_FILE_READ_FAILED;
			error->string[0] = level->path;
			goto end;
		}
		writeval = trmod_fwrite(&boxcrd[1], (size_t) 1,
		                        (size_t) 1, level->file);
		if (writeval != (size_t) 1)
		{
			error->code = ERROR_FILE_WRITE_FAILED;
			error->string[0] = level->path;
			goto end;
		}
	}
	offset += 1l;
	if ((rep & 0x00000001) != 0x00000000)
	{
		seekval = trmod_fseek(level->file, offset, SEEK_SET);
		if (seekval != 0)
		{
			error->code = ERROR_FILE_READ_FAILED;
			error->string[0] = level->path;
			goto end;
		}
		writeval = trmod_fwrite(&boxcrd[2], (size_t) 1,
		                        (size_t) 1, level->file);
		if (writeval != (size_t) 1)
		{
			error->code = ERROR_FILE_WRITE_FAILED;
			error->string[0] = level->path;
			goto end;
		}
	}
	offset += 1l;
	if ((rep & 0x00000002) != 0x00000000)
	{
		seekval = trmod_fseek(level->file, offset, SEEK_SET);
		if (seekval != 0)
		{
			error->code = ERROR_FILE_READ_FAILED;
			error->string[0] = level->path;
			goto end;
		}
		writeval = trmod_fwrite(&boxcrd[3], (size_t) 1,
		                        (size_t) 1, level->file);
		if (writeval != (size_t) 1)
		{
			error->code = ERROR_FILE_WRITE_FAILED;
			error->string[0] = level->path;
			goto end;
		}
	}
	offset += 1l;
	if ((rep & 0x00000010) != 0x00000000)
	{
		seekval = trmod_fseek(level->file, offset, SEEK_SET);
		if (seekval != 0)
		{
			error->code = ERROR_FILE_READ_FAILED;
			error->string[0] = level->path;
			goto end;
		}
		writeval = trmod_fwrite(&floor, (size_t) 1, (size_t) 2, level->file);
		if (writeval != (size_t) 2)
		{
			error->code = ERROR_FILE_WRITE_FAILED;
			error->string[0] = level->path;
			goto end;
		}
	}
	
	/* Replaces zones if needed */
	offset = (long int) (box * 0x00000002);
	offset += (long int) level->offset[ZONE];
	if ((rep & 0x00000020) != 0x00000000)
	{
		seekval = trmod_fseek(level->file, offset, SEEK_SET);
		if (seekval != 0)
		{
			error->code = ERROR_FILE_READ_FAILED;
			error->string[0] = level->path;
			goto end;
		}
		writeval = trmod_fwrite(&gz1, (size_t) 1, (size_t) 2, level->file);
		if (writeval != (size_t) 2)
		{
			error->code = ERROR_FILE_WRITE_FAILED;
			error->string[0] = level->path;
			goto end;
		}
	}
	offset += (long int) (level->value[NUMBOXES] * 0x00000002);
	if ((rep & 0x00000040) != 0x00000000)
	{
		seekval = trmod_fseek(level->file, offset, SEEK_SET);
		if (seekval != 0)
		{
			error->code = ERROR_FILE_READ_FAILED;
			error->string[0] = level->path;
			goto end;
		}
		writeval = trmod_fwrite(&gz2, (size_t) 1, (size_t) 2, level->file);
		if (writeval != (size_t) 2)
		{
			error->code = ERROR_FILE_WRITE_FAILED;
			error->string[0] = level->path;
			goto end;
		}
	}
	offset += (long int) (level->value[NUMBOXES] * 0x00000002);
	if ((rep & 0x00000080) != 0x00000000)
	{
		seekval = trmod_fseek(level->file, offset, SEEK_SET);
		if (seekval != 0)
		{
			error->code = ERROR_FILE_READ_FAILED;
			error->string[0] = level->path;
			goto end;
		}
		writeval = trmod_fwrite(&gz3, (size_t) 1, (size_t) 2, level->file);
		if (writeval != (size_t) 2)
		{
			error->code = ERROR_FILE_WRITE_FAILED;
			error->string[0] = level->path;
			goto end;
		}
	}
	offset += (long int) (level->value[NUMBOXES] * 0x00000002);
	if ((rep & 0x00000100) != 0x00000000)
	{
		seekval = trmod_fseek(level->file, offset, SEEK_SET);
		if (seekval != 0)
		{
			error->code = ERROR_FILE_READ_FAILED;
			error->string[0] = level->path;
			goto end;
		}
		writeval = trmod_fwrite(&gz4, (size_t) 1, (size_t) 2, level->file);
		if (writeval != (size_t) 2)
		{
			error->code = ERROR_FILE_WRITE_FAILED;
			error->string[0] = level->path;
			goto end;
		}
	}
	offset += (long int) (level->value[NUMBOXES] * 0x00000002);
	if ((rep & 0x00000200) != 0x00000000)
	{
		seekval = trmod_fseek(level->file, offset, SEEK_SET);
		if (seekval != 0)
		{
			error->code = ERROR_FILE_READ_FAILED;
			error->string[0] = level->path;
			goto end;
		}
		writeval = trmod_fwrite(&fz, (size_t) 1, (size_t) 2, level->file);
		if (writeval != (size_t) 2)
		{
			error->code = ERROR_FILE_WRITE_FAILED;
			error->string[0] = level->path;
			goto end;
		}
	}
	offset += (long int) (level->value[NUMBOXES] * 0x00000002);
	if ((rep & 0x00000400) != 0x00000000)
	{
		seekval = trmod_fseek(level->file, offset, SEEK_SET);
		if (seekval != 0)
		{
			error->code = ERROR_FILE_READ_FAILED;
			error->string[0] = level->path;
			goto end;
		}
		writeval = trmod_fwrite(&agz1, (size_t) 1, (size_t) 2, level->file);
		if (writeval != (size_t) 2)
		{
			error->code = ERROR_FILE_WRITE_FAILED;
			error->string[0] = level->path;
			goto end;
		}
	}
	offset += (long int) (level->value[NUMBOXES] * 0x00000002);
	if ((rep & 0x00000800) != 0x00000000)
	{
		seekval = trmod_fseek(level->file, offset, SEEK_SET);
		if (seekval != 0)
		{
			error->code = ERROR_FILE_READ_FAILED;
			error->string[0] = level->path;
			goto end;
		}
		writeval = trmod_fwrite(&agz2, (size_t) 1, (size_t) 2, level->file);
		if (writeval != (size_t) 2)
		{
			error->code = ERROR_FILE_WRITE_FAILED;
			error->string[0] = level->path;
			goto end;
		}
	}
	offset += (long int) (level->value[NUMBOXES] * 0x00000002);
	if ((rep & 0x00001000) != 0x00000000)
	{
		seekval = trmod_fseek(level->file, offset, SEEK_SET);
		if (seekval != 0)
		{
			error->code = ERROR_FILE_READ_FAILED;
			error->string[0] = level->path;
			goto end;
		}
		writeval = trmod_fwrite(&agz3, (size_t) 1, (size_t) 2, level->file);
		if (writeval != (size_t) 2)
		{
			error->code = ERROR_FILE_WRITE_FAILED;
			error->string[0] = level->path;
			goto end;
		}
	}
	offset += (long int) (level->value[NUMBOXES] * 0x00000002);
	if ((rep & 0x00002000) != 0x00000000)
	{
		seekval = trmod_fseek(level->file, offset, SEEK_SET);
		if (seekval != 0)
		{
			error->code = ERROR_FILE_READ_FAILED;
			error->string[0] = level->path;
			goto end;
		}
		writeval = trmod_fwrite(&agz4, (size_t) 1, (size_t) 2, level->file);
		if (writeval != (size_t) 2)
		{
			error->code = ERROR_FILE_WRITE_FAILED;
			error->string[0] = level->path;
			goto end;
		}
	}
	offset += (long int) (level->value[NUMBOXES] * 0x00000002);
	if ((rep & 0x00004000) != 0x00000000)
	{
		seekval = trmod_fseek(level->file, offset, SEEK_SET);
		if (seekval != 0)
		{
			error->code = ERROR_FILE_READ_FAILED;
			error->string[0] = level->path;
			goto end;
		}
		writeval = trmod_fwrite(&afz, (size_t) 1, (size_t) 2, level->file);
		if (writeval != (size_t) 2)
		{
			error->code = ERROR_FILE_WRITE_FAILED;
			error->string[0] = level->path;
			goto end;
		}
	}
	
	/* Processes new overlaps if needed */
	if ((rep & 0x00008000) != 0x00000000)
	{
		if (strlen(overlapstr) == (size_t) 0U)
		{
			overlapstr = "0";
		}
		
		/* Separates the parameters */
		repchar(overlapstr, strlen(overlapstr), '[', ' ');
		repchar(overlapstr, strlen(overlapstr), ']', ' ');
		numolsubstr = splitString(overlapstr, &olsubstr, (uint16)
		                          strlen(overlapstr), ',', 1, 1, error);
		if (error->code != ERROR_NONE)
		{
			goto end;
		}
		
		/* Makes space for newol */
		newol = calloc((size_t) numolsubstr, (size_t) 2);
		if (newol == NULL)
		{
			error->code = ERROR_MEMORY;
			goto end;
		}
		
		/* Interprets the subparameters */
		for (curolsubstr = 0x0000; curolsubstr < numolsubstr; ++curolsubstr)
		{
			newol[curolsubstr] = (uint16) strtol(olsubstr[curolsubstr],
			                                     NULL, 10);
		}
		
		/* Sets the ending flag */
		--curolsubstr;
		newol[curolsubstr] |= 0x8000;
		
		/* Adjusts for Big-Endian */
		if (level->endian == TRUE)
		{
			for (curolsubstr = 0x0000; curolsubstr < numolsubstr; ++curolsubstr)
			{
				newol[curolsubstr] = reverse16(newol[curolsubstr]);
			}
		}
			
		/* Frees up no longer needed pointers */
		free(olsubstr);
		olsubstr = NULL;
		
		/* Calculates the number of new bytes */
		numolsubbyt = (size_t) (numolsubstr * 0x00000002);
		
		/* Determines whether there may already be needed overlaps */
		overlapindex = (uint16) level->value[NUMOVERLAPS];
		if (level->value[NUMOVERLAPS] >= numolsubstr)
		{
			/* Reads existing overlaps into memory */
			overlaps = calloc((size_t) level->value[NUMOVERLAPS],
			                  (size_t) 2);
			if (overlaps == NULL)
			{
				error->code = ERROR_MEMORY;
				goto end;
			}
			seekval = trmod_fseek(level->file, (long int)
			                      (level->offset[NUMOVERLAPS] + 0x00000004),
			                      SEEK_SET);
			if (seekval != 0)
			{
				error->code = ERROR_FILE_READ_FAILED;
				error->string[0] = level->path;
				goto end;
			}
			readval = trmod_fread(overlaps, (size_t) 1, (size_t)
			                      (level->value[NUMOVERLAPS] * 0x00000002),
			                      level->file);
			if (readval != (size_t) (level->value[NUMOVERLAPS] *
			                         0x00000002))
			{
				error->code = ERROR_FILE_READ_FAILED;
				error->string[0] = level->path;
				goto end;
			}
			
			/* Checks whether this overlap-entry already exists */
			for (curoverlap = 0x0000;
			     (curoverlap + numolsubstr) <= level->value[NUMOVERLAPS];
			     ++curoverlap)
			{
				/* Finds the needed overlap-entry if needed */
				if (memcmp(&overlaps[curoverlap], newol, numolsubbyt) == 0)
				{
					overlapindex = curoverlap;
					break;
				}
			}
		}
		
		/* Adds a new overlap-entry if needed */
		if (overlapindex == (uint16) level->value[NUMOVERLAPS])
		{
			/* Makes room for the new entry */
			insertbytes(level->file, level->path, level->offset[ZONE],
			            (uint32) numolsubbyt, error);
			if (error->code != ERROR_NONE)
			{
				return;
			}
			level_struct_add(level, level->offset[ZONE],
			                 (uint32) numolsubbyt);
			
			/* Writes the new entry to the level file */
			offset = (long int) overlapindex;
			offset *= 2l;
			offset += (long int) (level->offset[NUMOVERLAPS] + 0x00000004);
			seekval = trmod_fseek(level->file, offset, SEEK_SET);
			if (seekval != 0)
			{
				error->code = ERROR_FILE_READ_FAILED;
				error->string[0] = level->path;
				goto end;
			}
			writeval = trmod_fwrite(newol, (size_t) 1,
			                        numolsubbyt, level->file);
			if (writeval != numolsubbyt)
			{
				error->code = ERROR_FILE_WRITE_FAILED;
				error->string[0] = level->path;
				goto end;
			}
			
			/* Adjusts NumOverlaps */
			level->value[NUMOVERLAPS] += (uint32) numolsubstr;
			numOverlaps = level->value[NUMOVERLAPS];
			endian32(numOverlaps);
			seekval = trmod_fseek(level->file,
			                      (long int) level->offset[NUMOVERLAPS],
			                      SEEK_SET);
			if (seekval != 0)
			{
				error->code = ERROR_FILE_READ_FAILED;
				error->string[0] = level->path;
				goto end;
			}
			writeval = trmod_fwrite(&numOverlaps, (size_t) 1,
			                        (size_t) 4, level->file);
			if (writeval != (size_t) 4)
			{
				error->code = ERROR_FILE_WRITE_FAILED;
				error->string[0] = level->path;
				goto end;
			}
		}
		
		/* Frees up newol if needed */
		if (newol != NULL)
		{
			free(newol);
			newol = NULL;
		}
		
		/* Adjusts for Big-Endian */
		endian16(overlapindex);
		
		/* Writes the overlapindex */
		offset = (long int) (box * 0x00000008);
		offset += (long int) (level->offset[NUMBOXES] + 0x0000000A);
		seekval = trmod_fseek(level->file, offset, SEEK_SET);
		if (seekval != 0)
		{
			error->code = ERROR_FILE_READ_FAILED;
			error->string[0] = level->path;
			goto end;
		}
		writeval = trmod_fwrite(&overlapindex, (size_t) 1,
		                        (size_t) 2, level->file);
		if (writeval != (size_t) 2)
		{
			error->code = ERROR_FILE_WRITE_FAILED;
			error->string[0] = level->path;
			goto end;
		}
	}
	
	/* Replaces the "blocked/blockable"-flags if needed */
	if ((rep & 0x00010000) != 0x00000000)
	{
		/* Reads the overlapindex */
		offset = (long int) (box * 0x00000008);
		offset += (long int) (level->offset[NUMBOXES] + 0x0000000A);
		seekval = trmod_fseek(level->file, offset, SEEK_SET);
		if (seekval != 0)
		{
			error->code = ERROR_FILE_READ_FAILED;
			error->string[0] = level->path;
			goto end;
		}
		readval = trmod_fread(&overlapindex, (size_t) 1,
		                      (size_t) 2, level->file);
		if (readval != (size_t) 2)
		{
			error->code = ERROR_FILE_WRITE_FAILED;
			error->string[0] = level->path;
			goto end;
		}
		
		endian16(overlapindex);
		
		/* Replaces the flags */
		if (overlapindex != 0xFFFF)
		{
			if (blocked == 1)
			{
				overlapindex |= 0x4000U;
			}
			if (blockable == 1)
			{
				overlapindex |= 0x8000U;
			}
			
			endian16(overlapindex);
			
			/* Reads the overlapindex */
			seekval = trmod_fseek(level->file, offset, SEEK_SET);
			if (seekval != 0)
			{
				error->code = ERROR_FILE_READ_FAILED;
				error->string[0] = level->path;
				goto end;
			}
			writeval = trmod_fwrite(&overlapindex, (size_t) 1,
			                        (size_t) 2, level->file);
			if (writeval != (size_t) 2)
			{
				error->code = ERROR_FILE_WRITE_FAILED;
				error->string[0] = level->path;
				goto end;
			}
		}
	}
	
end:/* Frees allocated memory and ends the function */
	if (overlaps != NULL)
	{
		free(overlaps);
	}
	if (newol != NULL)
	{
		free(newol);
	}
	if (olsubstr != NULL)
	{
		free(olsubstr);
	}
}

/*
 * Function that finds whether a vertex exists, or adds it if it doesn't
 * Parameters:
 * * level = Pointer to the level struct
 * * error = Pointer to the error struct
 * * room = Room number
 * * x = X-position of the vertex (in local coordinates)
 * * y = Y-position of the vertex (in local coordinates)
 * * z = Z-position of the vertex (in local coordinates)
 * * inten1 = Intensity1 of the vertex
 * * inten2 = Intensity2 of the vertex
 * * attrib = Attribute of the vertex
 * Returns the vertex number
 */
uint16 tr2pc_find_vertex(struct level *level, struct error *error,
                         uint16 room, int32 x, int32 y, int32 z,
                         int16 inten1, int16 inten2, uint16 attrib)
{
	/* Variable Declarations */
	uint8 vertex[12];       /* In-memory new vertex */
	uint8 *vertices = NULL; /* In-memory copy of existing vertices */
	long int offset;        /* Where to read/write to */
	int seekval;            /* Return value of fseek() */
	size_t readval;         /* Return value of fread() */
	uint16 curvert;         /* Current vertex */
	uint32 numData;         /* Number of bytes to read */
	uint16 retval = 0x0000; /* Return value for this function */
	
	/* If there aren't any vertices, one definitely needs to be created */
	if (level->room[room].value[R_NUMVERTICES] == 0x0000)
	{
		retval = level->room[room].value[R_NUMVERTICES];
		tr1pc_add_vertex(level, error, room);
		if (error->code != ERROR_NONE)
		{
			goto end;
		}
		tr2pc_replace_vertex(level, error, room, 0xFFFF, x, y, z,
		                     inten1, inten2, attrib, 0xFFFF);
		if (error->code != ERROR_NONE)
		{
			goto end;
		}
		goto end;
	}
	
	/* Builds the needed vertex in memory */
	vertex[0] = (uint8) (x & 0x000000FF);
	vertex[1] = (uint8) ((x & 0x0000FF00) >> 8U);
	y = (level->room[room].yBottom - y);
	vertex[2] = (uint8) (y & 0x000000FF);
	vertex[3] = (uint8) ((y & 0x0000FF00) >> 8U);
	y = (level->room[room].yBottom - y);
	vertex[4] = (uint8) (z & 0x000000FF);
	vertex[5] = (uint8) ((z & 0x0000FF00) >> 8U);
	vertex[6] = (uint8) (inten1 & 0x00FF);
	vertex[7] = (uint8) ((inten1 & 0xFF00) >> 8U);
	vertex[8] = (uint8) (inten2 & 0x00FF);
	vertex[9] = (uint8) ((inten2 & 0xFF00) >> 8U);
	vertex[10] = (uint8) (attrib & 0x00FF);
	vertex[11] = (uint8) ((attrib & 0xFF00) >> 8U);
	
	/* Allocates vertices */
	vertices = malloc((size_t) (level->room[room].value[R_NUMVERTICES] *
	                            0x000C));
	if (vertices == NULL)
	{
		error->code = ERROR_MEMORY;
		goto end;
	}
	
	/* Reads the vertices into memory */
	offset = (long int) (level->room[room].offset[R_NUMVERTICES] + 0x00000002);
	numData = (uint32) level->room[room].value[R_NUMVERTICES];
	numData *= 0x0000000C;
	seekval = trmod_fseek(level->file, offset, SEEK_SET);
	if (seekval != 0)
	{
		error->code = ERROR_FILE_READ_FAILED;
		error->string[0] = level->path;
		goto end;
	}
	readval = trmod_fread(vertices, (size_t) 1, (size_t) numData, level->file);
	if (readval != (size_t) numData)
	{
		error->code = ERROR_FILE_READ_FAILED;
		error->string[0] = level->path;
		goto end;
	}
	
	/* Checks whether the needed vertex already exists */
	for (curvert = 0x0000; curvert < level->room[room].value[R_NUMVERTICES];
	     ++curvert)
	{
		if (memcmp(vertex, &vertices[(curvert * 0x000C)], (size_t) 8) == 0)
		{
			retval = curvert;
			break;
		}
	}
	
	/* Frees allocated memory, as it's no longer needed */
	free(vertices);
	vertices = NULL;
	
	/* Adds the new vertex if it doesn't exist */
	if (curvert == level->room[room].value[R_NUMVERTICES])
	{
		retval = level->room[room].value[R_NUMVERTICES];
		tr1pc_add_vertex(level, error, room);
		if (error->code != ERROR_NONE)
		{
			goto end;
		}
		tr2pc_replace_vertex(level, error, room, 0xFFFF, x, y, z,
		                     inten1, inten2, attrib, 0xFFFF);
		if (error->code != ERROR_NONE)
		{
			goto end;
		}
	}
	
end: /* Frees allocated memory and ends the function */
	if (vertices != NULL)
	{
		free(vertices);
		vertices = NULL;
	}
	return retval;
}
