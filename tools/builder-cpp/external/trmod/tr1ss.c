/*
 * Library of functions relating to Tomb Raider I on Sega Saturn
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
#include "util.h"     /* General functions for TRMOD */
#include "wrap.h"     /* Wrapper functions for called functions */
#include "trmodio.h"  /* TRMOD input and output */
#include "tr1pc.h"    /* Tomb Raider I PC (PHD/TUB) */

/* Navigation array */
static const uint32 tr1ss_nav[] =
{
	SKIP_BYTES, 0x0000001CU, /* ROOMFILE */
	NUMOBJECTTEXTURES, OBJECTTEXTURE, 
	SKIP_BYTES, 0x0000000CU, /* ROOMTQTR */
	SKIP_READ32, 0x00000001U,
	SKIP_BYTES, 0x0000000CU, /* ROOMTSUB */
	SKIP_READ32, 0x00000001U,
	SKIP_BYTES, 0x0000000CU, /* ROOMTPAL */
	SKIP_READ32, 0x00000003U,
	SKIP_BYTES, 0x0000000CU, /* ROOMSPAL */
	SKIP_READ32, 0x00000002U,
	SKIP_BYTES, 0x0000000EU, /* ROOMDATA */
	NUMROOMS,
	START_ROOM, 
		SKIP_BYTES, 0x00000034U,
		SKIP_READ32, 0x00000002U,
		SKIP_BYTES, 0x0000000EU, /* DOORDATA */
		R_NUMDOORS, R_DOOR,
		SKIP_BYTES, 0x0000000AU, /* FLOORDAT */
		R_NUMZSECTORS,
		SKIP_BYTES, 0x00000002U,
		R_NUMXSECTORS,
		SKIP_BYTES, 0x00000010U, /* FLOORSIZ */
		R_SECTOR,
		SKIP_BYTES, 0x00000010U, /* LIGHTAMB */
		N_IF_EQUAL, N_READ32, N_IMM, 0x4C494748U, N_THEN, 
			SKIP_BYTES, 0x0000000AU,
			R_NUMLIGHTS, R_LIGHT,
			SKIP_BYTES, 0x0000000EU,
		N_ELSE,
			SKIP_BYTES, 0x0000000AU,
		N_ENDIF,
		R_NUMSTATICMESHES, R_STATICMESH,
		SKIP_BYTES, 0x0000000EU, /* RM_FLIP */
		R_ALTROOM,
		SKIP_BYTES, 0x00000010U, /* RM_FLAGS */
	END_ROOM,
	SKIP_BYTES, 0x0000000CU, /* FLORDATA */
	NUMFLOORDATA, FLOORDATA,
	SKIP_BYTES, 0x0000000CU, /* CAMERAS */
	NUMCAMERAS, CAMERA,
	SKIP_BYTES, 0x0000000CU, /* SOUNDFX */
	NUMSOUNDSOURCES, SOUNDSOURCE,
	SKIP_BYTES, 0x0000000CU, /* BOXES */
	NUMBOXES, BOX,
	SKIP_BYTES, 0x0000000CU, /* OVERLAPS */
	NUMOVERLAPS, OVERLAP,
	ZONE,
	SKIP_BYTES, 0x00000060U, /* *_ZONE */
	SKIP_BYTES, 0x0000000CU, /* ARANGES */
	NUMANIMTEXTURES, ANIMATEDTEXTURE,
	SKIP_BYTES, 0x0000000CU, /* ITEMDATA */
	NUMENTITIES, ENTITY,
	SKIP_BYTES, 0x00000010U, /* ROOMEND */
	END_LEVEL
};

/*
 * Function that sets up the structs for navigating the level file
 * * Parameters:
 * * level = Pointer to the level struct
 */
void tr1ss_setup(struct level *level)
{
	/* Sets up the navigation array */
	level->nav = (uint32 *) tr1ss_nav;
	
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
	level->partsize[ZONE]               = 0x0000000CU;
	level->partsize[NUMANIMTEXTURES]    = 0x00000004U;
	level->partsize[NUMENTITIES]        = 0x00000004U;
	level->partsize[NUMROOMTEX]         = 0xFFFFFFFFU;
	level->partsize[NUMCINEMATICFRAMES] = 0x00000002U;
	level->partsize[NUMDEMODATA]        = 0x00000002U;
	level->partsize[NUMSOUNDDETAILS]    = 0x00000004U;
	level->partsize[NUMSAMPLES]         = 0x00000004U;
	level->partsize[NUMSAMPLEINDICES]   = 0x00000004U;
	level->partsize[CODEMODULE]         = 0xFFFFFFFFU;
	level->partsize[R_VERTEX]           = 0x00000008U;
	level->partsize[R_RECTANGLE]        = 0x0000000AU;
	level->partsize[R_TRIANGLE]         = 0x00000008U;
	level->partsize[R_SPRITE]           = 0x00000004U;
	level->partsize[R_DOOR]             = 0x00000020U;
	level->partsize[R_SECTOR]           = 0x00000008U;
	level->partsize[R_ROOMLIGHT]        = 0x00000002U;
	level->partsize[R_LIGHT]            = 0x00000014U;
	level->partsize[R_STATICMESH]       = 0x00000014U;
	level->partsize[TEXTILE]            = 0x00010000U;
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
	level->partsize[OBJECTTEXTURE]      = 0x00000010U;
	level->partsize[SPRITETEXTURE]      = 0x00000010U;
	level->partsize[SPRITESEQUENCE]     = 0x00000008U;
	level->partsize[CAMERA]             = 0x00000010U;
	level->partsize[SOUNDSOURCE]        = 0x00000010U;
	level->partsize[BOX]                = 0x00000014U;
	level->partsize[OVERLAP]            = 0x00000002U;
	level->partsize[ANIMATEDTEXTURE]    = 0x00000008U;
	level->partsize[ENTITY]             = 0x00000018U;
	level->partsize[ROOMTEXTURE]        = 0xFFFFFFFFU;
	level->partsize[CINEMATICFRAME]     = 0x00000010U;
	level->partsize[DEMODATA]           = 0x00000001U;
	level->partsize[SOUNDDETAIL]        = 0x00000008U;
	level->partsize[SAMPLE]             = 0x00000001U;
	level->partsize[SAMPLEINDEX]        = 0x00000004U;
}

/*
 * Function that needs to run after navigation
 * Parameters:
 * * level = Pointer to level struct
 * * error = Pointer to error struct
 */
void tr1ss_post_navigation(struct level *level, struct error *error)
{
	/* Variable Declarations */
	uint32 curroom = 0x0000U;    /* Current room */
	size_t readval = (size_t) 0; /* Return value for fread() */
	int seekval = 0;             /* Return value for fseek() */
	
	/* Gets NumVertices */
	for (curroom = 0x0000U; curroom < level->numRooms; ++curroom)
	{
		level->room[curroom].offset[R_NUMVERTICES] =
			(level->room[curroom].startpos + 0x00000038U);
		seekval = trmod_fseek(level->file, (long int)
		                      level->room[curroom].offset[R_NUMVERTICES],
		                      SEEK_SET);
		if (seekval != 0)
		{
			error->code = ERROR_FILE_READ_FAILED;
			error->string[0] = level->path;
			return;
		}
		readval = trmod_fread(&(level->room[curroom].value[R_NUMVERTICES]),
		                      (size_t) 1, (size_t) 2, level->file);
		if (readval != (size_t) 2)
		{
			error->code = ERROR_FILE_READ_FAILED;
			error->string[0] = level->path;
			return;
		}
		endian16(level->room[curroom].value[R_NUMVERTICES]);
	}
}

/*
 * Function that reads room header and flags from each room
 * Parameters:
 * * level = Pointer to level struct
 * * error = Pointer to error struct
 */
void tr1ss_readRoomHeaders(struct level *level, struct error *error)
{
	/* Variable Declarations */
	uint32 offset = 0x00000000;  /* Where to read/write to in file */
	uint16 curRoom = 0x0000;     /* Current room */
	size_t readval = (size_t) 0; /* Return value for fread() */
	int seekval = 0;             /* Return value for fseek() */
	
	/* Loops through every room */
	for (curRoom = 0x0000; curRoom < level->numRooms; ++curRoom)
	{
		offset = (level->room[curRoom].startpos + 0x00000018U);
		seekval = trmod_fseek(level->file, (long int) offset, SEEK_SET);
		if (seekval != 0)
		{
			error->code = ERROR_FILE_READ_FAILED;
			error->string[0] = level->path;
			return;
		}
		readval = trmod_fread(&(level->room[curRoom].x), (size_t) 1,
		                      (size_t) 4, level->file);
		if (readval != (size_t) 4)
		{
			error->code = ERROR_FILE_READ_FAILED;
			error->string[0] = level->path;
			return;
		}
		endian32(level->room[curRoom].x);
		readval = trmod_fread(&(level->room[curRoom].z), (size_t) 1,
		                      (size_t) 4, level->file);
		if (readval != (size_t) 4)
		{
			error->code = ERROR_FILE_READ_FAILED;
			error->string[0] = level->path;
			return;
		}
		endian32(level->room[curRoom].z);
		readval = trmod_fread(&(level->room[curRoom].yBottom),
		                      (size_t) 1, (size_t) 4, level->file);
		if (readval != (size_t) 4)
		{
			error->code = ERROR_FILE_READ_FAILED;
			error->string[0] = level->path;
			return;
		}
		endian32(level->room[curRoom].yBottom);
		readval = trmod_fread(&(level->room[curRoom].yTop),
		                      (size_t) 1, (size_t) 4, level->file);
		if (readval != (size_t) 4)
		{
			error->code = ERROR_FILE_READ_FAILED;
			error->string[0] = level->path;
			return;
		}
		endian32(level->room[curRoom].yTop);
		offset = (level->room[curRoom].offset[R_ALTROOM] + 0x00000002);
		seekval = trmod_fseek(level->file, (long int) offset, SEEK_SET);
		if (seekval != 0)
		{
			error->code = ERROR_FILE_READ_FAILED;
			error->string[0] = level->path;
			return;
		}
		readval = trmod_fread(&(level->room[curRoom].flags),
		                      (size_t) 1, (size_t) 2, level->file);
		if (readval != (size_t) 2)
		{
			error->code = ERROR_FILE_READ_FAILED;
			error->string[0] = level->path;
			return;
		}
		endian16(level->room[curRoom].flags);
	}
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
 * * intensity = Item's intensity1 value (8191-0)
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
void tr1ss_replaceitem(struct level *level, struct error *error, uint32 item,
                       uint16 id, uint16 room, int32 x, int32 y, int32 z,
                       int16 angle, int16 intensity, uint16 flags, uint16 rep)
{
	/* Variable Declarations */
	long int offset = 0l;          /* Where to read/write to */
	uint8 flagspace[2];            /* Space to store flags copy */
	int seekval = 0;               /* Return value of fseek() */
	size_t readval = (size_t) 0U;  /* Return value of fread() */
	size_t writeval = (size_t) 0U; /* Return value of fwrite() */

	/* Sets item number to the last one if max int */
	if (item == 0xFFFFFFFF)
	{
		item = level->value[NUMENTITIES];
		--item;
	}
	
	/* Replaces the item */
	tr1pc_replaceitem(level, error, item, id, room, x, y, z,
	                  angle, intensity, flags, rep);
	if (error->code != ERROR_NONE)
	{
		return;
	}
	
	/* Duplicates the flags if needed */
	if ((rep & 0x0080) != 0x0000) /* Flags */
	{
		offset = (long int) (item * level->partsize[ENTITY]);
		offset += (long int) (level->offset[NUMENTITIES] + 0x00000018U);
		seekval = trmod_fseek(level->file, (long int) offset, SEEK_SET);
		if (seekval != 0)
		{
			error->code = ERROR_FILE_READ_FAILED;
			error->string[0] = level->path;
			return;
		}
		readval = trmod_fread(flagspace, (size_t) 1, (size_t) 2, level->file);
		if (readval != (size_t) 2)
		{
			error->code = ERROR_FILE_READ_FAILED;
			error->string[0] = level->path;
			return;
		}
		writeval = trmod_fwrite(flagspace, (size_t) 1,
		                        (size_t) 2, level->file);
		if (writeval != (size_t) 2)
		{
			error->code = ERROR_FILE_READ_FAILED;
			error->string[0] = level->path;
			return;
		}
	}
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
 * * intensity = Static Mesh's intensity1 value (8191-0)
 * * rep = Bit-mask of elements to replace
 * * * 0 0 0 0  0 0 0 0   0 0 0 0  0 0 0 0
 * * * | | | |  | | | |   | | | |  | | | |
 * * * | | | |  | | | |   | | | |  | | | ID
 * * * | | | |  | | | |   | | | |  | | X
 * * * | | | |  | | | |   | | | |  | Y
 * * * | | | |  | | | |   | | | |  Z
 * * * | | | |  | | | |   | | | Angle
 * * * | | | |  | | | |   | | Intensity1
 * * * Unused
 */
void tr1ss_replacestaticmesh(struct level *level, struct error *error,
                             uint16 mesh, uint16 id, uint16 room, int32 x,
                             int32 y, int32 z, int16 angle, int16 intensity,
                             uint16 rep)
{
	/* Variable Declarations */
	long int offset = 0l;          /* Where to read/write to */
	uint8 idspace[2];              /* Space to store id copy */
	int seekval = 0;               /* Return value of fseek() */
	size_t readval = (size_t) 0U;  /* Return value of fread() */
	size_t writeval = (size_t) 0U; /* Return value of fwrite() */

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
	
	/* Replaces the static mesh */
	tr1pc_replacestaticmesh(level, error, mesh, id, room,
	                        x, y, z, angle, intensity, rep);
	if (error->code != ERROR_NONE)
	{
		return;
	}
	
	/* Duplicates the id if needed */
	if ((rep & 0x0001) != 0x0000) /* ID */
	{
		offset = (long int) (level->room[room].offset[R_NUMSTATICMESHES] +
		                     0x00000012 +
		                     ((uint32) (mesh * level->partsize[R_STATICMESH])));
		seekval = trmod_fseek(level->file, (long int) offset, SEEK_SET);
		if (seekval != 0)
		{
			error->code = ERROR_FILE_READ_FAILED;
			error->string[0] = level->path;
			return;
		}
		readval = trmod_fread(idspace, (size_t) 1, (size_t) 2, level->file);
		if (readval != (size_t) 2)
		{
			error->code = ERROR_FILE_READ_FAILED;
			error->string[0] = level->path;
			return;
		}
		writeval = trmod_fwrite(idspace, (size_t) 1,
		                        (size_t) 2, level->file);
		if (writeval != (size_t) 2)
		{
			error->code = ERROR_FILE_READ_FAILED;
			error->string[0] = level->path;
			return;
		}
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
void tr1ss_get_light(struct level *level, struct error *error,
                     uint16 room, uint16 light, uint16 print)
{
	/* Variable Declarations */
	uint8 memlight[20]; /* In-memory copy of the mesh */
	long int offset;    /* Offset of the mesh */
	int seekval;        /* Return value of fseek() */
	size_t readval;     /* Return value of fread() */
	int32 x;            /* The Light's X-Position */
	int32 y;            /* The Light's Y-Position */
	int32 z;            /* The Light's Z-Position */
	int16 inten;        /* The Light's Intensity */
	uint32 fade;        /* The Light's Fade */
	
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
	offset = (long int) (light * level->partsize[R_LIGHT]);
	offset += (long int) (level->room[room].offset[R_NUMLIGHTS] + 0x00000002);
	seekval = trmod_fseek(level->file, offset, SEEK_SET);
	if (seekval != 0)
	{
		error->code = ERROR_FILE_READ_FAILED;
		error->string[0] = level->path;
		return;
	}
	readval = trmod_fread(memlight, (size_t) 1, (size_t) 20, level->file);
	if (readval != (size_t) 20)
	{
		error->code = ERROR_FILE_READ_FAILED;
		error->string[0] = level->path;
		return;
	}
	
	/* Prints as hex dump if needed */
	if ((print & 0x0004) != 0x0000)
	{
		for (fade = 0x00000000U; fade < level->partsize[R_LIGHT]; ++fade)
		{
			trmod_printf("%02X ", memlight[fade]);
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
	inten = (int16) ((((uint16) memlight[13]) << 8U) |
	                 ((uint16) memlight[12]));
	fade = (uint32) ((((uint32) memlight[19]) << 24U) |
	                 (((uint32) memlight[18]) << 16U) |
	                 (((uint32) memlight[17]) << 8U) |
	                 ((uint32) memlight[16]));
	
	/* Adjusts for endianness */
	if (level->endian == TRUE)
	{
		x = reverse32(x);
		y = reverse32(y);
		z = reverse32(z);
		inten = reverse16(inten);
		fade = reverse32(fade);
	}
	
	/* Sets the coordinates to local coordinates */
	x -= level->room[room].x;
	y = (level->room[room].yBottom - y);
	z -= level->room[room].z;
	
	/* Determines how to print the light */
	if ((print & 0x0003) != 0x0000)
	{
		trmod_printf("%s %s %s ",
		             level->command, level->typestring, level->path);
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
	if (inten != (int16) 0xFFFF)
	{
		inten = (0x1FFF - inten);
	}
	
	/* Prints the light */
	trmod_printf(PRINT32 "," PRINT32 "," PRINT32 "," PRINT16 "%s," PRINTU32 ")",
	             x, z, y, inten,
	             ((inten != (int16) 0xFFFF) ? "/8191" : ""), fade);
	
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
 * * inten = Light's intensity1 value (8191-0)
 * * fade = Light's fade1 value
 * * rep = Bit-mask of elements to replace
 * * * 0 0 0 0  0 0 0 0   0 0 0 0  0 0 0 0
 * * * | | | |  | | | |   | | | |  | | | |
 * * * | | | |  | | | |   | | | |  | | | X
 * * * | | | |  | | | |   | | | |  | | Y
 * * * | | | |  | | | |   | | | |  | Z
 * * * | | | |  | | | |   | | | |  Intensity1
 * * * | | | |  | | | |   | | | Unused
 * * * | | | |  | | | |   | | Fade1
 * * * Unused
 */
void tr1ss_replacelight(struct level *level, struct error *error, uint16 light,
                        uint16 room, int32 x, int32 y, int32 z, int16 inten,
                        uint32 fade, uint16 rep)
{
	/* Variable Declarations */
	int seekval;        /* Return value for fseek() */
	size_t readval;     /* Return value for fread() */
	size_t writeval;    /* Return value for fwrite() */
	long int curpos;    /* Current position in the file */
	uint8 memlight[20]; /* In-memory copy of the item */
	
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
	                     0x00000002 + ((uint32) (light * 0x0014)));
	
	/* Reads the existing item into memory */
	seekval = trmod_fseek(level->file, curpos, SEEK_SET);
	if (seekval != 0)
	{
		error->code = ERROR_FILE_READ_FAILED;
		error->string[0] = level->path;
		return;
	}
	readval = trmod_fread(memlight, (size_t) 1, (size_t) 20, level->file);
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
	
	/* Adjusts for endianness */
	if (level->endian == TRUE)
	{
		x = reverse32(x);
		y = reverse32(y);
		z = reverse32(z);
		inten = reverse16(inten);
		fade = reverse32(fade);
	}
	
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
	if ((rep & 0x0008) != 0x0000) /* Intensity */
	{
		memlight[12] = (uint8) (inten & 0x00FF);
		memlight[13] = (uint8) ((inten & 0xFF00) >> 8U);
		memlight[14] = (uint8) (inten & 0x00FF);
		memlight[15] = (uint8) ((inten & 0xFF00) >> 8U);
	}
	if ((rep & 0x0020) != 0x0000) /* Fade */
	{
		memlight[16] = (uint8) (fade & 0x000000FF);
		memlight[17] = (uint8) ((fade & 0x0000FF00) >> 8U);
		memlight[18] = (uint8) ((fade & 0x00FF0000) >> 16U);
		memlight[19] = (uint8) ((fade & 0xFF000000) >> 24U);
	}
	
	/* Writes the light back to the level */
	seekval = trmod_fseek(level->file, curpos, SEEK_SET);
	if (seekval != 0)
	{
		error->code = ERROR_FILE_READ_FAILED;
		error->string[0] = level->path;
		return;
	}
	writeval = trmod_fwrite(memlight, (size_t) 1, (size_t) 20, level->file);
	if (writeval != (size_t) 20)
	{
		error->code = ERROR_FILE_WRITE_FAILED;
		error->string[0] = level->path;
		return;
	}
}
