/*
 * Library of functions relating to Tomb Raider III on PC
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
#include "tr2pc.h"    /* Tomb Raider II on PC */

/* Navigation array */
static const uint32 tr3pc_nav[] =
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
		SKIP_BYTES, 0x00000004U,
		R_NUMLIGHTS, R_LIGHT,
		R_NUMSTATICMESHES, R_STATICMESH,
		R_ALTROOM,
		SKIP_BYTES, 0x00000005U,
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
	NUMSPRITETEXTURES, SPRITETEXTURE,
	NUMSPRITESEQUENCES, SPRITESEQUENCE,
	NUMCAMERAS, CAMERA,
	NUMSOUNDSOURCES, SOUNDSOURCE,
	NUMBOXES, BOX,
	NUMOVERLAPS, OVERLAP,
	ZONE,
	NUMANIMTEXTURES, ANIMATEDTEXTURE,
	NUMOBJECTTEXTURES, OBJECTTEXTURE,
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
void tr3pc_setup(struct level *level)
{
	/* Sets up the navigation array */
	level->nav = (uint32 *) tr3pc_nav;
	
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
void tr3pc_get_item(struct level *level, struct error *error,
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
	uint8 r, g, b;     /* The Item's RGB-values */
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
	
	/* Determines how to print the item */
	if ((print & 0x0003) != 0x0000)
	{
		trmod_printf("%s %s %s ", level->command, level->typestring,
		             level->path);
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
	trmod_printf(PRINTU16 "," PRINTU16 "," PRINT32 "," PRINT32 ","
	             PRINT32 "," PRINT16 ",", id, room, x, z, y, angle);
	if (inten1 != (int16) 0xFFFF)
	{
		/* Separates the colour channels */
		b = (uint8) ((inten1 & 0x7C00) >> 7U);
		g = (uint8) ((inten1 & 0x03E0) >> 2U);
		r = (uint8) ((inten1 & 0x001F) << 3U);
		r |= (uint8) (r >> 5U);
		g |= (uint8) (g >> 5U);
		b |= (uint8) (b >> 5U);
		trmod_printf("%02X%02X%02X,", r, g, b);
	}
	else
	{
		trmod_printf("-1,");
	}
	if (inten2 != (int16) 0xFFFF)
	{
		/* Separates the colour channels */
		b = (uint8) ((inten2 & 0x7C00) >> 7U);
		g = (uint8) ((inten2 & 0x03E0) >> 2U);
		r = (uint8) ((inten2 & 0x001F) << 3U);
		r |= (uint8) (r >> 5U);
		g |= (uint8) (g >> 5U);
		b |= (uint8) (b >> 5U);
		trmod_printf("%02X%02X%02X,", r, g, b);
	}
	else
	{
		trmod_printf("-1,");
	}
	trmod_printf("%04X)", flags);
	
	/* Adds the closing quote if needed and prints the newline */
	if ((print & 0x0003) != 0x0000)
	{
		trmod_printf("\"");
	}
	trmod_printf("\n");
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
void tr3pc_get_staticmesh(struct level *level, struct error *error,
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
	uint8 r, g, b;     /* The Static Mesh's RGB-values */
	
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
	
	/* Prints the static mesh */
	trmod_printf(PRINT32 "," PRINT32 "," PRINT32 "," PRINT16 ",",
	             x, z, y, angle);
	if (inten1 != (int16) 0xFFFF)
	{
		/* Separates the colour channels */
		b = (uint8) ((inten1 & 0x7C00) >> 7U);
		g = (uint8) ((inten1 & 0x03E0) >> 2U);
		r = (uint8) ((inten1 & 0x001F) << 3U);
		r |= (uint8) (r >> 5U);
		g |= (uint8) (g >> 5U);
		b |= (uint8) (b >> 5U);
		trmod_printf("%02X%02X%02X,", r, g, b);
	}
	else
	{
		trmod_printf("-1,");
	}
	if (inten2 != (int16) 0xFFFF)
	{
		/* Separates the colour channels */
		b = (uint8) ((inten2 & 0x7C00) >> 7U);
		g = (uint8) ((inten2 & 0x03E0) >> 2U);
		r = (uint8) ((inten2 & 0x001F) << 3U);
		r |= (uint8) (r >> 5U);
		g |= (uint8) (g >> 5U);
		b |= (uint8) (b >> 5U);
		trmod_printf("%02X%02X%02X)", r, g, b);
	}
	else
	{
		trmod_printf("-1)");
	}
	
	/* Adds the closing quote if needed and prints the newline */
	if ((print & 0x0003) != 0x0000)
	{
		trmod_printf("\"");
	}
	trmod_printf("\n");
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
void tr3pc_get_light(struct level *level, struct error *error,
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
	uint32 colour;      /* The Light's colour value */
	uint32 inten;       /* The Light's Intensity */
	uint32 fade;        /* The Light's Fade */
	int16 nx, ny, nz;   /* Normals for sunlight */
	uint8 type;         /* Type of light */
	
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
	colour = (uint32) ((((uint32) memlight[12]) << 16U) |
	                   (((uint32) memlight[13]) << 8U) |
	                   ((uint32) memlight[14]));
	type = memlight[15];
	if (type == (uint8) 0x00)
	{
		/* Spotlight */
		inten = (uint32) ((((uint32) memlight[19]) << 24U) |
		                  (((uint32) memlight[18]) << 16U) |
		                  (((uint32) memlight[17]) << 8U) |
		                  ((uint32) memlight[16]));
		fade = (uint32) ((((uint32) memlight[23]) << 24U) |
		                 (((uint32) memlight[22]) << 16U) |
		                 (((uint32) memlight[21]) << 8U) |
		                 ((uint32) memlight[20]));
	}
	else
	{
		/* Sunlight */
		nx = (int16) ((((uint16) memlight[17]) << 8U) |
		               ((uint16) memlight[16]));
		ny = (int16) ((((uint16) memlight[19]) << 8U) |
		               ((uint16) memlight[18]));
		nz = (int16) ((((uint16) memlight[21]) << 8U) |
		               ((uint16) memlight[20]));
	}
	
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
	
	/* Prints the light's coordinates and colour */
	trmod_printf(PRINT32 "," PRINT32 "," PRINT32 ",%06X,", x, z, y, colour);
	
	/* Prints the rest of the light based on type */
	if (type == (uint8) 0x00)
	{
		trmod_printf("spotlight," PRINTU32 "," PRINTU32, inten, fade);
	}
	else
	{
		trmod_printf("sunlight," PRINT16 "," PRINT16 "," PRINT16, nx, nz, ny);
	}
	
	/* Adds the closing quote if needed and prints the newline */
	trmod_printf(")");
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
 * * colour = The colour in hexadecimal
 * * inten = Light's intensity value
 * * fade = Light's fade value
 * * rep = Bit-mask of elements to replace
 * * * 0 0 0 0  0 0 0 0   0 0 0 0  0 0 0 0
 * * * | | | |  | | | |   | | | |  | | | |
 * * * | | | |  | | | |   | | | |  | | | X
 * * * | | | |  | | | |   | | | |  | | Y
 * * * | | | |  | | | |   | | | |  | Z
 * * * | | | |  | | | |   | | | |  Intensity
 * * * | | | |  | | | |   | | | Unused
 * * * | | | |  | | | |   | | Fade
 * * * | | | |  | | | |   | Colour
 * * * | | | |  | | | |   Type
 * * * | | | |  | | | nx
 * * * | | | |  | | ny
 * * * | | | |  | nz
 * * * Unused
 */
void tr3pc_replacelight(struct level *level, struct error *error, uint16 light,
                        uint16 room, int32 x, int32 y, int32 z, uint32 colour,
                        uint8 type, uint32 inten, uint32 fade, int16 nx,
                        int16 ny, int16 nz, uint16 rep)
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
	if ((rep & 0x0040) != 0x0000) /* Colour */
	{
		memlight[12] = (uint8) ((colour & 0x00FF0000) >> 16U);
		memlight[13] = (uint8) ((colour & 0x0000FF00) >> 8U);
		memlight[14] = (uint8) (colour & 0x000000FF);
	}
	if ((rep & 0x0080) != 0x0000) /* Type */
	{
		memlight[15] = type;
	}
	else
	{
		type = memlight[15];
	}
	if (type == (uint8) 0x00)
	{
		/* Spotlight */
		if ((rep & 0x0008) != 0x0000) /* Intensity */
		{
			memlight[16] = (uint8) (inten & 0x000000FF);
			memlight[17] = (uint8) ((inten & 0x0000FF00) >> 8U);
			memlight[18] = (uint8) ((inten & 0x00FF0000) >> 16U);
			memlight[19] = (uint8) ((inten & 0xFF000000) >> 24U);
		}
		if ((rep & 0x0020) != 0x0000) /* Fade */
		{
			memlight[20] = (uint8) (fade & 0x000000FF);
			memlight[21] = (uint8) ((fade & 0x0000FF00) >> 8U);
			memlight[22] = (uint8) ((fade & 0x00FF0000) >> 16U);
			memlight[23] = (uint8) ((fade & 0xFF000000) >> 24U);
		}
	}
	else
	{
		/* Sunlight */
		if ((rep & 0x0100) != 0x0000) /* nX */
		{
			memlight[16] = (uint8) (nx & 0x00FF);
			memlight[17] = (uint8) ((nx & 0xFF00) >> 8U);
		}
		if ((rep & 0x0200) != 0x0000) /* nY */
		{
			memlight[18] = (uint8) (ny & 0x00FF);
			memlight[19] = (uint8) ((ny & 0xFF00) >> 8U);
		}
		if ((rep & 0x0400) != 0x0000) /* nZ */
		{
			memlight[20] = (uint8) (nz & 0x00FF);
			memlight[21] = (uint8) ((nz & 0xFF00) >> 8U);
		}
		memlight[22] = (uint8) 0x00U;
		memlight[23] = (uint8) 0x00U;
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
void tr3pc_get_vertex(struct level *level, struct error *error,
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
	uint8 r, g, b;      /* Colour values for the vertex */
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
	inten1 = (int16) memvertex[5];
	inten2 = (int16) memvertex[3];
	attrib = (uint16) memvertex[4];
	
	/* Sets the y-coordinate to local */
	y = (level->room[room].yBottom - y);
	
	/* Determines how to print the static mesh */
	if ((print & 0x0003) != 0x0000)
	{
		trmod_printf("%s %s %s ",
		             level->command, level->typestring, level->path);
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
	
	/* Prints the vertex coordinates */
	trmod_printf(PRINT32 "," PRINT32 "," PRINT32 ",", x, z, y);
	
	/* Converts intensity if needed */
	if (inten1 != (int16) 0xFFFF)
	{
		r = (uint8) ((inten1 & 0x7C00) >> 7U);
		g = (uint8) ((inten1 & 0x03E0) >> 2U);
		b = (uint8) ((inten1 & 0x001F) << 3U);
		r |= (uint8) (r >> 5U);
		g |= (uint8) (g >> 5U);
		b |= (uint8) (b >> 5U);
	}
	
	/* Prints the colours */
	if (inten1 != (int16) 0xFFFF)
	{
		trmod_printf("%02X%02X%02X,", r, g, b);
	}
	else
	{
		trmod_printf("-1,");
	}
	
	/* Converts intensity if needed */
	if (inten2 != (int16) 0xFFFF)
	{
		r = (uint8) ((inten2 & 0x7C00) >> 7U);
		g = (uint8) ((inten2 & 0x03E0) >> 2U);
		b = (uint8) ((inten2 & 0x001F) << 3U);
		r |= (uint8) (r >> 5U);
		g |= (uint8) (g >> 5U);
		b |= (uint8) (b >> 5U);
	}
	
	/* Prints the colours */
	if (inten2 != (int16) 0xFFFF)
	{
		trmod_printf("%02X%02X%02X,", r, g, b);
	}
	else
	{
		trmod_printf("-1,");
	}
	
	/* Prints the second intensity and attributes */
	trmod_printf("%04X", attrib);
	
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
 * * inten1 = Colour value of the vertex
 * * inten2 = Lighting value of the vertex
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
void tr3pc_replace_vertex(struct level *level, struct error *error,
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
	
	inten1 = (int16) (((inten1 & 0x001F) << 10U) |
	                  ((inten1 & 0x7C00) >> 10U) | (inten1 & 0x83E0));
	inten2 = (int16) (((inten2 & 0x001F) << 10U) |
	                  ((inten2 & 0x7C00) >> 10U) | (inten2 & 0x83E0));
	
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
		memvertex[5] = inten1;
	}
	/* Replaces Intensity2 if needed */
	if ((rep & 0x0010) != 0x0000)
	{
		memvertex[3] = inten2;
	}
	/* Replaces Attributes if needed */
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
void tr3pc_replace_sprite(struct level *level, struct error *error,
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
	
	inten1 = (int16) (((inten1 & 0x001F) << 10U) |
	                  ((inten1 & 0x7C00) >> 10U) | (inten1 & 0x83E0));
	inten2 = (int16) (((inten2 & 0x001F) << 10U) |
	                  ((inten2 & 0x7C00) >> 10U) | (inten2 & 0x83E0));
	
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
 * Function that prints the boxIndex of a given sector
 * Parameters:
 * * level = Pointer to level struct
 * * error = Pointer to error struct
 * * room = Room number
 * * column = Column number
 * * row = Row number
 * * print = Printing flags
 * * * 0 0 0 0  0 0 0 0   0 0 0 0  0 0 0 0
 * * * | | | |  | | | |   | | | |  | | | |
 * * * | | | |  | | | |   | | | |  | | | As command
 * * * Unused
 */
void tr3pc_getzone(struct level *level, struct error *error,
                   uint16 room, uint16 column, uint16 row, uint16 print)
{
	/* Variable Declarations */
	long int offset; /* Offset to read the height from */
	int16 boxIndex; /* Box Index of the sector */
	int seekval;     /* Return value of fseek() */
	size_t readval;  /* Return value of fread() */
	
	/* Checks that this sector exists */
	if (room >= level->numRooms)
	{
		error->code = ERROR_ROOM_DOESNT_EXIST;
		error->u16[0] = room;
		return;
	}
	if ((column >= level->room[room].value[R_NUMXSECTORS]) ||
	    (row >= level->room[room].value[R_NUMZSECTORS]))
	{
		error->code = ERROR_SECTOR_DOESNT_EXIST;
		error->u16[0] = room;
		error->u16[1] = column;
		error->u16[2] = row;
		return;
	}
	
	/* Calculates the offset of the sector data */
	offset = (long int) (level->room[room].offset[R_NUMXSECTORS] + 0x00000004);
	offset += (long int) (((level->room[room].value[R_NUMZSECTORS] * column) +
	                       row) * 0x0008);
	
	/* Reads the boxindex into memory */
	seekval = trmod_fseek(level->file, offset, SEEK_SET);
	if (seekval != 0)
	{
		error->code = ERROR_FILE_READ_FAILED;
		error->string[0] = level->path;
		return;
	}
	readval = trmod_fread(&boxIndex, (size_t) 1, (size_t) 2, level->file);
	if (readval != (size_t) 2)
	{
		error->code = ERROR_FILE_READ_FAILED;
		error->string[0] = level->path;
		return;
	}
	endian16(boxIndex);
	boxIndex >>= 4U;
	
	/* Sets value of -1 if needed */
	if (boxIndex == (int16) 0x07FF)
	{
		boxIndex = (int16) -1;
	}
	
	/* Determines how to print the zone */
	if ((print & 0x0001) != 0x0000)
	{
		trmod_printf("%s %s %s \"zone(", level->command, level->typestring,
		             level->path);
	}
	else
	{
		trmod_printf("Zone(");
	}
	
	/* Prints the sector */
	trmod_printf(PRINTU16 "," PRINTU16 "," PRINTU16 "," PRINT16 ")",
	       room, column, row, boxIndex);
	
	/* Prints closing quote if needed */
	if ((print & 0x0001) != 0x0000)
	{
		trmod_printf("\"");
	}
	trmod_printf("\n");
}

/*
 * Function that sets the boxIndex of a given sector
 * Parameters:
 * * level = Pointer to level struct
 * * error = Pointer to error struct
 * * room = Room number
 * * column = Column number
 * * row = Row number
 * * boxIndex = BoxIndex to write to the sector
 */
void tr3pc_setzone(struct level *level, struct error *error, uint16 room,
                   uint16 column, uint16 row, int16 boxIndex)
{
	/* Variable Declarations */
	long int offset; /* Offset to write the height to */
	int seekval;     /* Return value of fseek() */
	size_t readval;  /* Return value of fread() */
	size_t writeval; /* Return value of fwrite() */
	uint16 oldval;   /* Old value of the boxIndex */
	
	/* Checks that this sector exists */
	if (room >= level->numRooms)
	{
		error->code = ERROR_ROOM_DOESNT_EXIST;
		error->u16[0] = room;
		return;
	}
	if ((column >= level->room[room].value[R_NUMXSECTORS]) ||
	    (row >= level->room[room].value[R_NUMZSECTORS]))
	{
		error->code = ERROR_SECTOR_DOESNT_EXIST;
		error->u16[0] = room;
		error->u16[1] = column;
		error->u16[2] = row;
		return;
	}
	
	/* Calculates the offset of the sector data */
	offset = (long int) (level->room[room].offset[R_NUMXSECTORS] + 0x00000004);
	offset += (long int) (((level->room[room].value[R_NUMZSECTORS] * column) +
	                       row) * 0x0008);
	
	/* Reads the original boxIndex and StepSound */
	seekval = trmod_fseek(level->file, offset, SEEK_SET);
	if (seekval != 0)
	{
		error->code = ERROR_FILE_READ_FAILED;
		error->string[0] = level->path;
		return;
	}
	readval = trmod_fread(&oldval, (size_t) 1, (size_t) 2, level->file);
	if (readval != (size_t) 2)
	{
		error->code = ERROR_FILE_READ_FAILED;
		error->string[0] = level->path;
		return;
	}
	
	/* Keeps the stepsound */
	boxIndex <<= 4U;
	boxIndex &= 0x7FF0;
	oldval &= 0x000F;
	boxIndex |= oldval;
	endian16(boxIndex);
	
	/* Writes the boxIndex to the level file */
	seekval = trmod_fseek(level->file, offset, SEEK_SET);
	if (seekval != 0)
	{
		error->code = ERROR_FILE_READ_FAILED;
		error->string[0] = level->path;
		return;
	}
	writeval = trmod_fwrite(&boxIndex, (size_t) 1, (size_t) 2, level->file);
	if (writeval != (size_t) 2)
	{
		error->code = ERROR_FILE_WRITE_FAILED;
		error->string[0] = level->path;
		return;
	}
}

/*
 * Function that prints the step sound of a given sector
 * Parameters:
 * * level = Pointer to level struct
 * * error = Pointer to error struct
 * * room = Room number
 * * column = Column number
 * * row = Row number
 * * print = Printing flags
 * * * 0 0 0 0  0 0 0 0   0 0 0 0  0 0 0 0
 * * * | | | |  | | | |   | | | |  | | | |
 * * * | | | |  | | | |   | | | |  | | | As command
 * * * Unused
 */
void tr3pc_getstepsound(struct level *level, struct error *error,
                        uint16 room, uint16 column, uint16 row, uint16 print)
{
	/* Variable Declarations */
	long int offset;  /* Offset to read the height from */
	uint16 stepsound; /* Step Sound of the sector */
	int seekval;      /* Return value of fseek() */
	size_t readval;   /* Return value of fread() */
	
	/* Checks that this sector exists */
	if (room >= level->numRooms)
	{
		error->code = ERROR_ROOM_DOESNT_EXIST;
		error->u16[0] = room;
		return;
	}
	if ((column >= level->room[room].value[R_NUMXSECTORS]) ||
	    (row >= level->room[room].value[R_NUMZSECTORS]))
	{
		error->code = ERROR_SECTOR_DOESNT_EXIST;
		error->u16[0] = room;
		error->u16[1] = column;
		error->u16[2] = row;
		return;
	}
	
	/* Calculates the offset of the sector data */
	offset = (long int) (level->room[room].offset[R_NUMXSECTORS] + 0x00000004);
	offset += (long int) (((level->room[room].value[R_NUMZSECTORS] * column) +
	                       row) * 0x0008);
	
	/* Reads the step sound into memory */
	seekval = trmod_fseek(level->file, offset, SEEK_SET);
	if (seekval != 0)
	{
		error->code = ERROR_FILE_READ_FAILED;
		error->string[0] = level->path;
		return;
	}
	readval = trmod_fread(&stepsound, (size_t) 1, (size_t) 2, level->file);
	if (readval != (size_t) 2)
	{
		error->code = ERROR_FILE_READ_FAILED;
		error->string[0] = level->path;
		return;
	}
	endian16(stepsound);
	stepsound &= 0x000F;
	
	/* Determines how to print the zone */
	if ((print & 0x0001) != 0x0000)
	{
		trmod_printf("%s %s %s \"stepsound(", level->command,
		             level->typestring, level->path);
	}
	else
	{
		trmod_printf("Step Sound(");
	}
	
	/* Prints the sector */
	trmod_printf(PRINTU16 "," PRINTU16 "," PRINTU16 "," PRINTU16 ")",
	             room, column, row, stepsound);
	
	/* Prints closing quote if needed */
	if ((print & 0x0001) != 0x0000)
	{
		trmod_printf("\"");
	}
	trmod_printf("\n");
}

/*
 * Function that sets the step sound of a given sector
 * Parameters:
 * * level = Pointer to level struct
 * * error = Pointer to error struct
 * * room = Room number
 * * column = Column number
 * * row = Row number
 * * boxIndex = BoxIndex to write to the sector
 */
void tr3pc_setstepsound(struct level *level, struct error *error, uint16 room,
                        uint16 column, uint16 row, uint16 stepsound)
{
	/* Variable Declarations */
	long int offset; /* Offset to write the height to */
	int seekval;     /* Return value of fseek() */
	size_t readval;  /* Return value of fread() */
	size_t writeval; /* Return value of fwrite() */
	uint16 oldval;   /* Old value of the boxIndex */
	
	/* Checks that this sector exists */
	if (room >= level->numRooms)
	{
		error->code = ERROR_ROOM_DOESNT_EXIST;
		error->u16[0] = room;
		return;
	}
	if ((column >= level->room[room].value[R_NUMXSECTORS]) ||
	    (row >= level->room[room].value[R_NUMZSECTORS]))
	{
		error->code = ERROR_SECTOR_DOESNT_EXIST;
		error->u16[0] = room;
		error->u16[1] = column;
		error->u16[2] = row;
		return;
	}
	
	/* Calculates the offset of the sector data */
	offset = (long int) (level->room[room].offset[R_NUMXSECTORS] + 0x00000004);
	offset += (long int) (((level->room[room].value[R_NUMZSECTORS] * column) +
	                       row) * 0x0008);
	
	/* Reads the original boxIndex and StepSound */
	seekval = trmod_fseek(level->file, offset, SEEK_SET);
	if (seekval != 0)
	{
		error->code = ERROR_FILE_READ_FAILED;
		error->string[0] = level->path;
		return;
	}
	readval = trmod_fread(&oldval, (size_t) 1, (size_t) 2, level->file);
	if (readval != (size_t) 2)
	{
		error->code = ERROR_FILE_READ_FAILED;
		error->string[0] = level->path;
		return;
	}
	
	/* Keeps the boxIndex */
	oldval &= 0xFFF0;
	stepsound &= 0x000F;
	oldval |= stepsound;
	endian16(oldval);
	
	/* Writes the boxIndex to the level file */
	seekval = trmod_fseek(level->file, offset, SEEK_SET);
	if (seekval != 0)
	{
		error->code = ERROR_FILE_READ_FAILED;
		error->string[0] = level->path;
		return;
	}
	writeval = trmod_fwrite(&oldval, (size_t) 1, (size_t) 2, level->file);
	if (writeval != (size_t) 2)
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
void tr3pc_get_roomlight(struct level *level, struct error *error,
                         uint16 room, uint16 print)
{
	/* Variable Declarations */
	long int offset;    /* Offset to read from */
	int16 roomlight[2]; /* Roomlight value */
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
	offset = (long int) (level->room[room].offset[R_NUMLIGHTS] - 0x00000004);
	seekval = trmod_fseek(level->file, offset, SEEK_SET);
	if (seekval != 0)
	{
		error->code = ERROR_FILE_READ_FAILED;
		error->string[0] = level->path;
		return;
	}
	readval = trmod_fread(roomlight, (size_t) 1, (size_t) 4, level->file);
	if (readval != (size_t) 4)
	{
		error->code = ERROR_FILE_READ_FAILED;
		error->string[0] = level->path;
		return;
	}
	if (level->endian == TRUE)
	{
		roomlight[0] = reverse16(roomlight[0]);
		roomlight[1] = reverse16(roomlight[1]);
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
	if ((print & 0x0003) != 0x0000)
	{
		trmod_printf("%s %s %s \"roomlight(" PRINTU16 "," PRINT16 "/8191,"
		             PRINT16 "/8191)\"\n", level->command, level->typestring,
		             level->path, room, roomlight[0], roomlight[1]);
	}
	else
	{
		trmod_printf("RoomLight(" PRINTU16 "," PRINT16 "/8191," PRINT16
		             "/8191)\n", room, roomlight[0], roomlight[1]);
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
 */
void tr3pc_roomlight(struct level *level, struct error *error,
                     uint16 room, int16 inten1, int16 inten2)
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
	offset = (long int) (level->room[room].offset[R_NUMLIGHTS] - 0x00000004);
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
}

/*
 * Function that adds a room to the level
 * Parameters:
 * * level = Pointer to the level struct
 * * error = Pointer to the error struct
 */
void tr3pc_addroom(struct level *level, struct error *error)
{
	/* Variable Declarations */
	uint8 buffer[4]; /* Buffer for keeping bytes temporarily */
	long int curpos; /* Current position in the file */
	int seekval;     /* Return value of fseek() */
	size_t writeval; /* Return value of fwrite() */
	
	/* Makes space for this new room */
	insertbytes(level->file, level->path, level->offset[NUMFLOORDATA],
	            0x00000031, error);
	if (error->code != ERROR_NONE)
	{
		return;
	}
	
	/* Zeroes out the room */
	zerobytes(level->file, level->path, level->offset[NUMFLOORDATA],
	          0x00000031, error);
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
	curpos += 26l;
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
