/*
 * Library of functions relating to Tomb Raider I on PC
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
#include "tr1pc.h"    /* Tomb Raider I PC (PHD/TUB) */
#include "util.h"     /* General functions for TRMOD */
#include "wrap.h"     /* Wrapper functions for called functions */
#include "trmodio.h"  /* TRMOD input and output */

/* Navigation array */
static const uint32 tr1pc_nav[] =
{
	SKIP_BYTES, 0x00000004U,
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
		SKIP_BYTES, 0x00000002U,
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
	SKIP_BYTES, 0x00002300U,
	NUMCINEMATICFRAMES, CINEMATICFRAME,
	NUMDEMODATA, DEMODATA,
	SKIP_BYTES, 0x00000200U,
	NUMSOUNDDETAILS, SOUNDDETAIL,
	NUMSAMPLES, SAMPLE,
	NUMSAMPLEINDICES, SAMPLEINDEX,
	END_LEVEL
};
static const uint32 tr1gpc_nav[] =
{
	SKIP_BYTES, 0x00000004U,
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
		SKIP_BYTES, 0x00000002U,
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
	SKIP_BYTES, 0x00000300U,
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
	SKIP_BYTES, 0x00000200U,
	NUMSOUNDDETAILS, SOUNDDETAIL,
	NUMSAMPLES, SAMPLE,
	NUMSAMPLEINDICES, SAMPLEINDEX,
	END_LEVEL
};

/*
 * Function that sets up the structs for navigating the level file
 * * Parameters:
 * * level = Pointer to the level struct
 */
void tr1pc_setup(struct level *level)
{
	/* Sets up the navigation array */
	if (level->type == TRMOD_LEVTYPE_TR1_PC)
	{
		level->nav = (uint32 *) tr1pc_nav;
	}
	else if (level->type == TRMOD_LEVTYPE_TR1G_PC)
	{
		level->nav = (uint32 *) tr1gpc_nav;
	}
	
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
	level->partsize[R_LIGHT]            = 0x00000012U;
	level->partsize[R_STATICMESH]       = 0x00000012U;
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
	level->partsize[OBJECTTEXTURE]      = 0x00000014U;
	level->partsize[SPRITETEXTURE]      = 0x00000010U;
	level->partsize[SPRITESEQUENCE]     = 0x00000008U;
	level->partsize[CAMERA]             = 0x00000010U;
	level->partsize[SOUNDSOURCE]        = 0x00000010U;
	level->partsize[BOX]                = 0x00000014U;
	level->partsize[OVERLAP]            = 0x00000002U;
	level->partsize[ANIMATEDTEXTURE]    = 0x00000002U;
	level->partsize[ENTITY]             = 0x00000016U;
	level->partsize[ROOMTEXTURE]        = 0xFFFFFFFFU;
	level->partsize[CINEMATICFRAME]     = 0x00000010U;
	level->partsize[DEMODATA]           = 0x00000001U;
	level->partsize[SOUNDDETAIL]        = 0x00000008U;
	level->partsize[SAMPLE]             = 0x00000001U;
	level->partsize[SAMPLEINDEX]        = 0x00000004U;
}

/*
 * Function that navigates through the PC-file reading offsets and values
 * Parameters:
 * * level = Pointer to the level struct
 * * error = Pointer to the error struct
 */
void tr1pc_navigate(struct level *level, struct error *error)
{
	/* Variable Declarations */
	int    seekval = 0;           /* return value for fseek() */
	size_t readval = (size_t) 0;  /* Return value for fread() */
	uint32 curpos = 0x00000000;   /* Current position in the level */
	uint16 navcount = 0x0000;     /* Current position in the navigation array */
	uint16 startroom = 0x0000;    /* Where in the nav array rooms start */
	uint16 curRoom = 0x0000;      /* Current room in the level */
	uint32 read_u32 = 0x00000000; /* Used to read 32-bit integers */
	uint16 read_u16 = 0x0000;     /* Used to read 16-bit integers */
	uint32 iftype = 0x00000000;   /* Condition for the if-statement */
	uint32 c_param[4];            /* Parameters for the if-statment */
	uint16 c_filledparams;        /* Number of filled-in parameters */
	int16  ifdepth;               /* Current depth of condition */
	
	/* Parses the navigation array */
	for (navcount = 0x0000; level->nav[navcount] != END_LEVEL; ++navcount)
	{
		/* Speacial Cases */
		/* Special thing to skip. Get the length from the next element */
		if (level->nav[navcount] == SKIP_BYTES)
		{
			++navcount;
			curpos += level->nav[navcount];
		}
		/* Special case to skip bytes from variable */
		else if (level->nav[navcount] == SKIP_VAR)
		{
			++navcount;
			if (level->nav[navcount] < NUMKEEPPARTS)
			{
				curpos += level->value[level->nav[navcount]];
			}
			else
			{
				error->code = ERROR_INVALID_NAVARRAY;
				return;
			}
		}
		/* Special case to skip bytes from read 32-bit integer */
		else if (level->nav[navcount] == SKIP_READ32)
		{
			seekval = trmod_fseek(level->file, (long int) curpos, SEEK_SET);
			if (seekval != 0)
			{
				error->code = ERROR_FILE_READ_FAILED;
				error->string[0] = level->path;
				return;
			}
			readval = trmod_fread(&read_u32, (size_t) 1,
			                      (size_t) 4, level->file);
			if (readval != (size_t) 4)
			{
				error->code = ERROR_FILE_READ_FAILED;
				error->string[0] = level->path;
				return;
			}
			endian32(read_u32);
			curpos += 0x00000004U;
			++navcount;
			curpos += (level->nav[navcount] * read_u32);
		}
		/* Marker for the start of the room struct */
		else if (level->nav[navcount] == START_ROOM)
		{
			startroom = navcount;
			
			/* Special case to skip rooms if there are none */
			if (level->value[NUMROOMS] == 0x0000U)
			{
				for (; ((level->nav[navcount] != END_ROOM) &&
				        (level->nav[navcount] != END_LEVEL)); ++navcount);
				continue;
			}
			else
			{
				/* Remembers the offset of this room */
				level->room[curRoom].startpos = curpos;
			}
		}
		/* Marker for the end of the room struct */
		else if (level->nav[navcount] == END_ROOM)
		{
			/* Parse another room if needed */
			++curRoom;
			if (curRoom < level->numRooms)
			{
				/* Remembers the offset of this room */
				level->room[curRoom].startpos = curpos;
				
				navcount = startroom;
			}
		}
		/* Special case for sector */
		else if (level->nav[navcount] == R_SECTOR)
		{
			curpos += (level->partsize[R_SECTOR] *
			           level->room[curRoom].value[R_NUMZSECTORS] *
			           level->room[curRoom].value[R_NUMXSECTORS]);
		}
		/* Special case for zone */
		else if (level->nav[navcount] == ZONE)
		{
			level->offset[ZONE] = curpos;
			curpos += (level->value[NUMBOXES] * level->partsize[ZONE]);
		}
		/* Special case for set flag */
		else if (level->nav[navcount] == NAV_SET_FLAG)
		{
			++navcount;
			level->nav_flags |= level->nav[navcount];
		}
		/* Speacial case for VAR_TO_PARAM0 */
		else if (level->nav[navcount] == VAR_TO_PARAM0)
		{
			++navcount;
			level->value[level->nav[navcount]] = c_param[0];
		}
		/* Special case for conditionals */
		else if ((level->nav[navcount] == N_IF_EQUAL)   ||
		         (level->nav[navcount] == N_IF_GREATER) ||
		         (level->nav[navcount] == N_IF_SMALLER))
		{
			/* Sets up variables for the conditions */
			iftype = level->nav[navcount];
			c_param[0] = 0x00000000U;
			c_param[1] = 0x00000000U;
			c_param[2] = 0x00000000U;
			c_param[3] = 0x00000000U;
			c_filledparams = 0x0000U;
			++navcount;
			for (; level->nav[navcount] != N_THEN; ++navcount)
			{
				if (level->nav[navcount] == N_IMM)
				{
					++navcount;
					c_param[c_filledparams] = level->nav[navcount];
					++c_filledparams;
				}
				else if (level->nav[navcount] == N_AND)
				{
					if (c_filledparams < 0x0002U)
					{
						error->code = ERROR_INVALID_NAVARRAY;
						return;
					}
					c_param[(c_filledparams - 0x0002U)] &=
						c_param[(c_filledparams - 0x0001U)];
					--c_filledparams;
				}
				else if (level->nav[navcount] == N_OR)
				{
					if (c_filledparams < 0x0002U)
					{
						error->code = ERROR_INVALID_NAVARRAY;
						return;
					}
					c_param[(c_filledparams - 0x0002U)] |=
						c_param[(c_filledparams - 0x0001U)];
					--c_filledparams;
				}
				else if (level->nav[navcount] == N_READ32)
				{
					seekval = trmod_fseek(level->file,
					                      (long int) curpos, SEEK_SET);
					if (seekval != 0)
					{
						error->code = ERROR_FILE_READ_FAILED;
						error->string[0] = level->path;
						return;
					}
					readval = trmod_fread(&read_u32, (size_t) 1,
					                      (size_t) 4, level->file);
					if (readval != (size_t) 4)
					{
						error->code = ERROR_FILE_READ_FAILED;
						error->string[0] = level->path;
						return;
					}
					endian32(read_u32);
					curpos += 0x00000004U;
					c_param[c_filledparams] = read_u32;
					++c_filledparams;
				}
				else if (level->nav[navcount] == N_READ16)
				{
					seekval = trmod_fseek(level->file,
					                      (long int) curpos, SEEK_SET);
					if (seekval != 0)
					{
						error->code = ERROR_FILE_READ_FAILED;
						error->string[0] = level->path;
						return;
					}
					readval = trmod_fread(&read_u16, (size_t) 1,
					                      (size_t) 2, level->file);
					if (readval != (size_t) 2)
					{
						error->code = ERROR_FILE_READ_FAILED;
						error->string[0] = level->path;
						return;
					}
					endian16(read_u16);
					read_u32 = (uint32) read_u16;
					read_u32 &= 0x0000FFFFU;
					curpos += 0x00000002U;
					c_param[c_filledparams] = read_u32;
					++c_filledparams;
				}
				else if (level->nav[navcount] < NUMROOMPARTNUMS)
				{
					c_param[c_filledparams] = (uint32) (level->room[curRoom].
						value[level->nav[navcount]]);
					++c_filledparams;
				}
				else if (level->nav[navcount] < NUMLEVPARTS)
				{
					c_param[c_filledparams] =
						level->value[level->nav[navcount]];
					++c_filledparams;
				}
			}
			
			/* Checks if the condition applies */
			if (((iftype == N_IF_EQUAL) && (c_param[0] == c_param[1]))  ||
			    ((iftype == N_IF_GREATER) && (c_param[0] > c_param[1])) ||
			    ((iftype == N_IF_SMALLER) && (c_param[0] < c_param[1])))
			{
				/* The condition applies */
				continue;
			}
			else
			{
				/* Skips to the N_ELSE */
				ifdepth = 0x0000;
				for (; level->nav[navcount] != END_LEVEL; ++navcount)
				{
					if (((level->nav[navcount] == N_ELSE) ||
					     (level->nav[navcount] == N_ENDIF)) &&
					    (ifdepth == 0x0000))
					{
						break;
					}
					else if ((level->nav[navcount] == N_IF_EQUAL) ||
					         (level->nav[navcount] == N_IF_GREATER) ||
					         (level->nav[navcount] == N_IF_SMALLER))
					{
						++ifdepth;
					}
					else if (level->nav[navcount] == N_ENDIF)
					{
						--ifdepth;
					}
				}
				continue;
			}
		}
		/* Special case for else */
		else if (level->nav[navcount] == N_ELSE)
		{
			/* Skips to the N_ENDIF */
			ifdepth = 0x0000;
			for (; level->nav[navcount] != END_LEVEL; ++navcount)
			{
				if ((level->nav[navcount] == N_ENDIF) &&
				    (ifdepth == 0x0000))
				{
					break;
				}
				else if ((level->nav[navcount] == N_IF_EQUAL) ||
				         (level->nav[navcount] == N_IF_GREATER) ||
				         (level->nav[navcount] == N_IF_SMALLER))
				{
					++ifdepth;
				}
				else if (level->nav[navcount] == N_ENDIF)
				{
					--ifdepth;
				}
			}
			continue;
		}
		
		/* A Num-something to be read and kept */
		else if (level->nav[navcount] < NUMKEEPPARTS)
		{
			/* Read the value */
			seekval = trmod_fseek(level->file, (long int) curpos, SEEK_SET);
			if (seekval != 0)
			{
				error->code = ERROR_FILE_READ_FAILED;
				error->string[0] = level->path;
				return;
			}
			if (level->partsize[level->nav[navcount]] == 0x00000004)
			{
				readval = trmod_fread(&read_u32, (size_t) 1,
				                      (size_t) 4, level->file);
				if (readval != (size_t) 4)
				{
					error->code = ERROR_FILE_READ_FAILED;
					error->string[0] = level->path;
					return;
				}
				endian32(read_u32);
			}
			else if (level->partsize[level->nav[navcount]] == 0x00000003)
			{
				readval = trmod_fread(&read_u32, (size_t) 1,
				                      (size_t) 4, level->file);
				if (readval != (size_t) 4)
				{
					error->code = ERROR_FILE_READ_FAILED;
					error->string[0] = level->path;
					return;
				}
				endian32(read_u32);
				read_u32 &= 0x00FFFFFF;
			}
			else if (level->partsize[level->nav[navcount]] == 0x00000002)
			{
				readval = trmod_fread(&read_u16, (size_t) 1,
				                      (size_t) 2, level->file);
				if (readval != (size_t) 2)
				{
					error->code = ERROR_FILE_READ_FAILED;
					error->string[0] = level->path;
					return;
				}
				endian16(read_u16);
				read_u32 = (uint32) read_u16;
				read_u32 &= 0x0000FFFFU;
			}
			
			/* Stores the offset and value */
			if (level->nav[navcount] < NUMROOMPARTNUMS)
			{
				level->room[curRoom].offset[level->nav[navcount]] = curpos;
				level->room[curRoom].value[level->nav[navcount]] = read_u16;
			}
			else
			{
				level->offset[level->nav[navcount]] = curpos;
				level->value[level->nav[navcount]] = read_u32;
			}
			
			/* Moves curpos along */
			curpos += level->partsize[level->nav[navcount]];
		}
		
		/* An actual element to be skipped */
		else if (level->nav[navcount] < NUMLEVPARTS)
		{
			if (level->nav[navcount] < NUMROOMPARTS)
			{
				curpos += (level->partsize[level->nav[navcount]] *
				           level->room[curRoom].value[level->
				           partnum[level->nav[navcount]]]);
			}
			else
			{
				curpos += (level->partsize[level->nav[navcount]] *
				           level->value[level->partnum[level->nav[navcount]]]);
			}
		}
		/* Sets numRooms when it is read and allocates room structs */
		if (level->nav[navcount] == NUMROOMS)
		{
			level->numRooms = (uint16) level->value[NUMROOMS];
			if (level->room != NULL)
			{
				free(level->room);
				level->room = NULL;
			}
			level->room = calloc((size_t) level->numRooms, sizeof(struct room));
			if (level->room == NULL)
			{
				error->code = ERROR_MEMORY;
				return;
			}
		}
	}
	
	/* Runs whatever needs to be done after navigation */
	post_navigation(level, error);
	if (error->code != ERROR_NONE)
	{
		return;
	}

	/* Reads room headers and flags */
	readRoomHeaders(level, error);
}

/*
 * Function that reads room header and flags from each room
 * Parameters:
 * * level = Pointer to level struct
 * * error = Pointer to error struct
 */
void tr1pc_readRoomHeaders(struct level *level, struct error *error)
{
	/* Variable Declarations */
	uint32 offset = 0x00000000;  /* Where to read/write to in file */
	uint16 curRoom = 0x0000;     /* Current room */
	size_t readval = (size_t) 0; /* Return value for fread() */
	int seekval = 0;             /* Return value for fseek() */
	
	/* Loops through every room */
	for (curRoom = 0x0000; curRoom < level->numRooms; ++curRoom)
	{
		offset = (level->room[curRoom].offset[R_NUMVERTICES] - 0x00000014);
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
void tr1pc_get_item(struct level *level, struct error *error,
                    uint32 item, uint16 print)
{
	/* Variable Declarations */
	uint8 memitem[22]; /* In-memory copy of the item */
	long int offset;   /* Offset of the item */
	int seekval;       /* Return value of fseek() */
	size_t readval;    /* Return value of fread() */
	uint16 id;         /* The Item's ID */
	uint16 room;       /* The Item's Room */
	int32 x;           /* The Item's X-Position */
	int32 y;           /* The Item's Y-Position */
	int32 z;           /* The Item's Z-Position */
	int16 angle;       /* The Item's Angle */
	int16 inten;       /* The Item's Intensity */
	uint16 flags;      /* The Item's Flags */
	
	/* Checks whether the item exists */
	if (item >= level->value[NUMENTITIES])
	{
		error->code = ERROR_ITEM_DOESNT_EXIST;
		error->u32[0] = item;
		return;
	}
	
	/* Reads the item to be printed into memory */
	offset = (long int) (item * level->partsize[ENTITY]);
	offset += (long int) (level->offset[NUMENTITIES] + 0x00000004);
	seekval = trmod_fseek(level->file, offset, SEEK_SET);
	if (seekval != 0)
	{
		error->code = ERROR_FILE_READ_FAILED;
		error->string[0] = level->path;
		return;
	}
	readval = trmod_fread(memitem, (size_t) 1, (size_t) 22, level->file);
	if (readval != (size_t) 22)
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
	inten = (int16) ((((uint16) memitem[19]) << 8U) | ((uint16) memitem[18]));
	flags = (uint16) ((((uint16) memitem[21]) << 8U) | ((uint16) memitem[20]));
	
	/* Adjusts for endianness */
	if (level->endian == TRUE)
	{
		id = reverse16(id);
		room = reverse16(room);
		x = reverse32(x);
		y = reverse32(y);
		z = reverse32(z);
		angle = reverse16(angle);
		inten = reverse16(inten);
		flags = reverse16(flags);
	}
	
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
	if (inten != (int16) 0xFFFF)
	{
		inten = (0x1FFF - inten);
	}
	
	/* Determines how to print the item */
	if ((print & 0x0003) != 0x0000)
	{
		trmod_printf("%s %s %s ",
		             level->command, level->typestring, level->path);
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
	             PRINT16 "," PRINT16 "%s,%04X)", id, room, x, z, y, angle,
	             inten, ((inten != (int16) 0xFFFF) ? "/8191" : ""), flags);
	
	/* Adds the closing quote if needed and prints the newline */
	if ((print & 0x0003) != 0x0000)
	{
		trmod_printf("\"");
	}
	trmod_printf("\n");
}

/*
 * Function that adds an item to the level
 * Parameters:
 * * level = Pointer to level struct
 * * error = Pointer to error struct
 */
void tr1pc_additem(struct level *level, struct error *error)
{
	/* Variable Declarations */
	uint32 itemoffset; /* Offset to write the item to */
	uint32 itemnum;    /* Item number to write */
	int seekval;       /* Return value for fseek() */
	size_t writeval;   /* Return value for fwrite() */
	
	/* Determines the offset where to put the item */
	itemoffset = (level->offset[NUMENTITIES] + 0x00000004 +
	              (level->value[NUMENTITIES] * level->partsize[ENTITY]));
	
	/* Makes room for the new item */
	insertbytes(level->file, level->path, itemoffset,
	            level->partsize[ENTITY], error);
	if (error->code != ERROR_NONE)
	{
		return;
	}
	level_struct_add(level, itemoffset, level->partsize[ENTITY]);
	
	/* Adds one to NumEntities */
	++(level->value[NUMENTITIES]);
	
	/* Writes NumEntities to the level file */
	itemnum = level->value[NUMENTITIES];
	endian32(itemnum);
	seekval = trmod_fseek(level->file,
	                      (long int) level->offset[NUMENTITIES], SEEK_SET);
	if (seekval != 0)
	{
		error->code = ERROR_FILE_READ_FAILED;
		error->string[0] = level->path;
		return;
	}
	writeval = trmod_fwrite(&itemnum, (size_t) 1, (size_t) 4, level->file);
	if (writeval != (size_t) 4)
	{
		error->code = ERROR_FILE_WRITE_FAILED;
		error->string[0] = level->path;
		return;
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
void tr1pc_replaceitem(struct level *level, struct error *error, uint32 item,
                       uint16 id, uint16 room, int32 x, int32 y, int32 z,
                       int16 angle, int16 intensity, uint16 flags, uint16 rep)
{
	/* Variable Declarations */
	int seekval;       /* Return value for fseek() */
	size_t readval;    /* Return value for fread() */
	size_t writeval;   /* Return value for fwrite() */
	long int curpos;   /* Current position in the file */
	uint8 memitem[22]; /* In-memory copy of the item */
	
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
	curpos = (long int) (level->offset[NUMENTITIES] + 0x00000004 +
	                     (item * level->partsize[ENTITY]));
	
	/* Reads the existing item into memory */
	seekval = trmod_fseek(level->file, curpos, SEEK_SET);
	if (seekval != 0)
	{
		error->code = ERROR_FILE_READ_FAILED;
		error->string[0] = level->path;
		return;
	}
	readval = trmod_fread(memitem, (size_t) 1, (size_t) 22, level->file);
	if (readval != (size_t) 22)
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

	/* Adjusts for endianness */
	if (level->endian == TRUE)
	{
		id = reverse16(id);
		room = reverse16(room);
		x = reverse32(x);
		y = reverse32(y);
		z = reverse32(z);
		angle = reverse16(angle);
		intensity = reverse16(intensity);
		flags = reverse16(flags);
	}
	
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
	if ((rep & 0x0040) != 0x0000) /* Intensity */
	{
		memitem[18] = (uint8) (intensity & 0x00FF);
		memitem[19] = (uint8) ((intensity & 0xFF00) >> 8U);
	}
	if ((rep & 0x0080) != 0x0000) /* Flags */
	{
		memitem[20] = (uint8) (flags & 0x00FF);
		memitem[21] = (uint8) ((flags & 0xFF00) >> 8U);
	}
	
	/* Writes the item back to the level */
	seekval = trmod_fseek(level->file, curpos, SEEK_SET);
	if (seekval != 0)
	{
		error->code = ERROR_FILE_READ_FAILED;
		error->string[0] = level->path;
		return;
	}
	writeval = trmod_fwrite(memitem, (size_t) 1, (size_t) 22, level->file);
	if (writeval != (size_t) 22)
	{
		error->code = ERROR_FILE_WRITE_FAILED;
		error->string[0] = level->path;
		return;
	}
}

/*
 * Function that removes an item from the level
 * Parameters:
 * * level = Pointer to the level struct
 * * error = Pointer to the error struct
 * * item = Item to be removed
 * * flags = Bit-mask of flags
 * * * 0 0 0 0  0 0 0 0   0 0 0 0  0 0 0 0
 * * * | | | |  | | | |   | | | |  | | | |
 * * * | | | |  | | | |   | | | |  | | | Adjust FloorData
 * * * Unused
 */
void tr1pc_removeitem(struct level *level, struct error *error,
                      uint32 item, uint16 flags)
{
	/* Variable Declarations */
	int seekval;       /* Return value for fseek() */
	size_t readval;    /* Return value for fread() */
	size_t writeval;   /* Return value for fwrite() */
	uint32 offset;     /* Offset of the item to be removed */
	uint32 numItems;   /* New value for NumEntities */
	uint16 curroom;    /* Current room in processing */
	uint16 currow;     /* Current row in processing */
	uint16 curcol;     /* Current column in processing */
	uint16 fdIndex;    /* Current index in FloorData */
	uint16 fdSetup;    /* Current FloorData Setup */
	uint16 endData;    /* Whether the endData-bit was set */
	uint16 contbit;    /* Whether the continuation bit was set */
	uint16 *fd = NULL; /* In-memory copy of FloorData */
	uint8 *en = NULL;  /* Entries in FloorData */
	
	/* Checks whether the item exists */
	if (item >= level->value[NUMENTITIES])
	{
		error->code = ERROR_ITEM_DOESNT_EXIST;
		error->u32[0] = item;
		return;
	}
	
	/* Removes the item */
	offset = (item * level->partsize[ENTITY]);
	offset += 0x00000004;
	offset += level->offset[NUMENTITIES];
	removebytes(level->file, level->path, offset,
	            level->partsize[ENTITY], error);
	if (error->code != ERROR_NONE)
	{
		return;
	}
	level_struct_sub(level, offset, level->partsize[ENTITY]);
	
	/* Subtracts 1 from NumEntities */
	--(level->value[NUMENTITIES]);
	numItems = level->value[NUMENTITIES];
	endian32(numItems);
	
	/* Writes NumEntities back to the level file */
	seekval = trmod_fseek(level->file,
	                      (long int) level->offset[NUMENTITIES], SEEK_SET);
	if (seekval != 0)
	{
		error->code = ERROR_FILE_READ_FAILED;
		error->string[0] = level->path;
		return;
	}
	writeval = trmod_fwrite(&numItems, (size_t) 1, (size_t) 4, level->file);
	if (writeval != (size_t) 4)
	{
		error->code = ERROR_FILE_WRITE_FAILED;
		error->string[0] = level->path;
		return;
	}
	
	/* End the funtion here if FloorData is not to be modified */
	if ((flags & 0x0001) == 0x0000)
	{
		return;
	}
	
	/* Allocates space for floordata */
	fd = calloc((size_t) level->value[NUMFLOORDATA], (size_t) 2);
	if (fd == NULL)
	{
		error->code = ERROR_MEMORY;
		goto end;
	}
	en = calloc((size_t) level->value[NUMFLOORDATA], (size_t) 1);
	if (en == NULL)
	{
		error->code = ERROR_MEMORY;
		goto end;
	}
	
	/* Reads FloorData into memory */
	seekval = trmod_fseek(level->file, (long int) (level->offset[NUMFLOORDATA] +
	                                               0x00000004), SEEK_SET);
	if (seekval != 0)
	{
		error->code = ERROR_FILE_READ_FAILED;
		error->string[0] = level->path;
		goto end;
	}
	readval = trmod_fread(fd, (size_t) 1,
	                      (size_t) (level->value[NUMFLOORDATA] * 0x00000002),
	                      level->file);
	if (readval != (size_t) (level->value[NUMFLOORDATA] * 0x00000002))
	{
		error->code = ERROR_FILE_READ_FAILED;
		error->string[0] = level->path;
		goto end;
	}
	
	/* Adjusts for Big-Endian */
	if (level->endian == TRUE)
	{
		for (fdIndex = 0x0000; fdIndex < (uint16) level->value[NUMFLOORDATA];
		     ++fdIndex)
		{
			fd[fdIndex] = reverse16(fd[fdIndex]);
		}
	}
	
	/* Loops through every sector of every room */
	for (curroom = 0x0000; curroom < level->numRooms; ++curroom)
	{
		offset = (uint32) (level->room[curroom].offset[R_NUMXSECTORS] +
		                   0x00000002);
		for (currow = 0x0000;
		     currow < level->room[curroom].value[R_NUMZSECTORS];
		     ++currow)
		{
			for (curcol = 0x0000;
			     curcol < level->room[curroom].value[R_NUMXSECTORS];
			     ++curcol)
			{
				/* Gets FDIndex for this sector */
				seekval = trmod_fseek(level->file, (long int) offset, SEEK_SET);
				if (seekval != 0)
				{
					error->code = ERROR_FILE_READ_FAILED;
					error->string[0] = level->path;
					goto end;
				}
				readval = trmod_fread(&fdIndex, (size_t) 1,
				                      (size_t) 2, level->file);
				if (readval != (size_t) 2)
				{
					error->code = ERROR_FILE_READ_FAILED;
					error->string[0] = level->path;
					goto end;
				}
				endian16(fdIndex);

				if (fdIndex == (uint16) 0x0000U)
				{
					offset += 0x00000008;
					continue;
				}
				
				/* Checks fdIndex */
				if (fdIndex >= (uint16) level->value[NUMFLOORDATA])
				{
					error->code = ERROR_INVALID_FDINDEX;
					goto end;
				}
				
				/* Checks this FloorData entry for items */
				endData = 0x0000;
				while (endData == 0x0000)
				{
					/* Checks whether this entry has already been done */
					if (en[fdIndex] != (uint8) 0x00)
					{
						break;
					}
					
					/* Gets the FDSetup */
					fdSetup = fd[fdIndex];
					endData = (fd[fdIndex] & 0x8000);
					en[fdIndex] |= (uint8) 0xFF;
					++fdIndex;
					
					/* Determines FloorData Function */
					switch (fdSetup & 0x001F)
					{
						case 0x0002: /* Floor Slant */
						case 0x0003: /* Ceiling Slant */
						case 0x0007: /* Triangulation */
						case 0x0008: /* Triangulation */
						case 0x0009: /* Triangulation */
						case 0x000A: /* Triangulation */
						case 0x000B: /* Triangulation */
						case 0x000C: /* Triangulation */
						case 0x000D: /* Triangulation */
						case 0x000E: /* Triangulation */
						case 0x000F: /* Triangulation */
						case 0x0010: /* Triangulation */
						case 0x0011: /* Triangulation */
						case 0x0012: /* Triangulation */
							if (en[fdIndex] != (uint8) 0x00)
							{
								break;
							}
							en[fdIndex] |= (uint8) 0xFF;
							++fdIndex;
							break;
						case 0x0001: /* Portal */
						case 0x0005: /* Kill Lara */
						case 0x0006: /* Climbable Walls */
						case 0x0013: /* Monkey Swing */
						case 0x0014: /* Function 0x14 */
						case 0x0015: /* Function 0x15 */
							break;
						case 0x0004: /* Trigger */
							if (en[fdIndex] != (uint8) 0x00)
							{
								break;
							}
							en[fdIndex] |= (uint8) 0xFF;
							++fdIndex;
							/* Checks the item numbers if the
							 * trigger type is switch, key, or pickup */
							if (((fdSetup & 0x7F00) == 0x0200) ||
							    ((fdSetup & 0x7F00) == 0x0300) ||
							    ((fdSetup & 0x7F00) == 0x0400))
							{
								/* Adjusts the related item number if needed */
								if (en[fdIndex] != (uint8) 0x00)
								{
									break;
								}
								en[fdIndex] |= (uint8) 0xFF;
								if ((fd[fdIndex] & 0x03FF) >= (uint16) item)
								{
									if ((fd[fdIndex] & 0x03FF) > 0x0000)
									{
										--fd[fdIndex];
									}
								}
								++fdIndex;
							}
							/* Processes Trigger Actions */
							contbit = 0x0000;
							while (contbit == 0x0000)
							{
								if (en[fdIndex] != (uint8) 0x00)
								{
									break;
								}
								en[fdIndex] |= (uint8) 0xFF;
								switch (fd[fdIndex] & 0x7C00)
								{
									case 0x0000: /* Object */
									case 0x1800: /* Look at Item */
										/* Adjusts the item number if needed */
										if ((fd[fdIndex] & 0x03FF) >=
										    (uint16) item)
										{
											if ((fd[fdIndex] & 0x03FF) > 0x0000)
											{
												--fd[fdIndex];
											}
										}
										contbit |= (fd[fdIndex] & 0x8000);
										break;
									case 0x0400: /* Camera */
									case 0x3000: /* Flyby Camera */
										++fdIndex;
										contbit |= (fd[fdIndex] & 0x8000);
										break;
									case 0x0800: /* Current */
									case 0x0C00: /* Flip Map */
									case 0x1000: /* Flip On */
									case 0x1400: /* Flip Off */
									case 0x1C00: /* End Level */
									case 0x2000: /* Play Soundtrack */
									case 0x2400: /* Effect */
									case 0x2800: /* Secret */
									case 0x2C00: /* Clear Bodies */
									case 0x3400: /* Cutscene */
										contbit |= (fd[fdIndex] & 0x8000);
										break;
								}
								++fdIndex;
							}
							break;
						default:
							break;
					}
				}
				
				offset += 0x00000008;
			}
		}
	}
	
	/* Adjusts for Big-Endian */
	if (level->endian == TRUE)
	{
		for (fdIndex = 0x0000;
		     fdIndex < (uint16) level->value[NUMFLOORDATA]; ++fdIndex)
		{
			fd[fdIndex] = reverse16(fd[fdIndex]);
		}
	}
	
	/* Writes FloorData back to the level file */
	seekval = trmod_fseek(level->file, (long int) (level->offset[NUMFLOORDATA] +
	                                               0x00000004), SEEK_SET);
	if (seekval != 0)
	{
		error->code = ERROR_FILE_READ_FAILED;
		error->string[0] = level->path;
		goto end;
	}
	writeval = trmod_fwrite(fd, (size_t) 1, (size_t)
	                  (level->value[NUMFLOORDATA] * 0x00000002), level->file);
	if (writeval != (size_t) (level->value[NUMFLOORDATA] * 0x00000002))
	{
		error->code = ERROR_FILE_WRITE_FAILED;
		error->string[0] = level->path;
		goto end;
	}
	
end:/* Ends allocated memory and ends the function */
	if (fd != NULL)
	{
		free(fd);
	}
	if (en != NULL)
	{
		free(en);
	}
}

/*
 * Function that removes all items from the level
 * Parameters:
 * * level = Pointer to the level struct
 * * error = Pointer to the error struct
 */
void tr1pc_removeallitems(struct level *level, struct error *error)
{
	/* Variable Declarations */
	uint32 offset;   /* Offset of the data to be removed */
	uint32 size;     /* Size of the data to be removed */
	int seekval;     /* Return value for fseek() */
	size_t writeval; /* Return value for fwrite() */
	
	/* Does nothing if there are no items at all */
	if (level->value[NUMENTITIES] == 0x00000000)
	{
		return;
	}
	
	/* Removes items */
	offset = (level->offset[NUMENTITIES] + 0x00000004);
	size = (level->value[NUMENTITIES] * level->partsize[ENTITY]);
	removebytes(level->file, level->path, offset, size, error);
	if (error->code != ERROR_NONE)
	{
		return;
	}
	level_struct_sub(level, offset, size);
	
	/* Sets NumEntities to zero */
	level->value[NUMENTITIES] = 0x00000000;
	
	/* Writes NumEntities back to the level file */
	seekval = trmod_fseek(level->file,
	                      (long int) level->offset[NUMENTITIES], SEEK_SET);
	if (seekval != 0)
	{
		error->code = ERROR_FILE_READ_FAILED;
		error->string[0] = level->path;
		return;
	}
	writeval = trmod_fwrite(&level->value[NUMENTITIES], (size_t) 1,
	                        (size_t) 4, level->file);
	if (writeval != (size_t) 4)
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
void tr1pc_get_staticmesh(struct level *level, struct error *error,
                          uint16 room, uint16 mesh, uint16 print)
{
	/* Variable Declarations */
	uint8 memmesh[18]; /* In-memory copy of the mesh */
	long int offset;   /* Offset of the mesh */
	int seekval;       /* Return value of fseek() */
	size_t readval;    /* Return value of fread() */
	uint16 id;         /* The Static Mesh's ID */
	int32 x;           /* The Static Mesh's X-Position */
	int32 y;           /* The Static Mesh's Y-Position */
	int32 z;           /* The Static Mesh's Z-Position */
	int16 angle;       /* The Static Mesh's Angle */
	int16 inten;       /* The Static Mesh's Intensity */
	
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
	offset = (long int) (mesh * level->partsize[R_STATICMESH]);
	offset += (long int) (level->room[room].offset[R_NUMSTATICMESHES] +
	                      0x00000002);
	seekval = trmod_fseek(level->file, offset, SEEK_SET);
	if (seekval != 0)
	{
		error->code = ERROR_FILE_READ_FAILED;
		error->string[0] = level->path;
		return;
	}
	readval = trmod_fread(memmesh, (size_t) 1, (size_t) 18, level->file);
	if (readval != (size_t) 18)
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
	inten = (int16) ((((uint16) memmesh[15]) << 8U) | ((uint16) memmesh[14]));
	id = (uint16) ((((uint16) memmesh[17]) << 8U) | ((uint16) memmesh[16]));

	/* Adjusts for endianness */
	if (level->endian == TRUE)
	{
		x = reverse32(x);
		y = reverse32(y);
		z = reverse32(z);
		angle = reverse16(angle);
		inten = reverse16(inten);
		id = reverse16(id);
	}
	
	/* Sets the coordinates to local coordinates */
	x -= level->room[room].x;
	y = (level->room[room].yBottom - y);
	z -= level->room[room].z;
	
	/* Determines how to print the static mesh */
	if ((print & 0x0003) != 0x0000)
	{
		trmod_printf("%s %s %s ",
		             level->command, level->typestring, level->path);
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
	if (inten != (int16) 0xFFFF)
	{
		inten = (0x1FFF - inten);
	}
	
	/* Prints the static mesh */
	trmod_printf(PRINT32 "," PRINT32 "," PRINT32 "," PRINT16 "," PRINT16 "%s)",
	             x, z, y, angle, inten,
	             ((inten != (int16) 0xFFFF) ? "/8191" : ""));
	
	/* Adds the closing quote if needed and prints the newline */
	if ((print & 0x0003) != 0x0000)
	{
		trmod_printf("\"");
	}
	trmod_printf("\n");
}

/*
 * Function that adds a static mesh to the level
 * Parameters:
 * * level = Pointer to level struct
 * * error = Pointer to error struct
 * * room = Static Mesh's room
 */
void tr1pc_addstaticmesh(struct level *level, struct error *error, uint16 room)
{
	/* Variable Declarations */
	uint32 meshoffset; /* Offset to write the item to */
	uint16 meshnum;    /* Item number to write */
	int seekval;       /* Return value for fseek() */
	size_t writeval;   /* Return value for fwrite() */
	
	/* Checks that the room exists */
	if (room >= level->numRooms)
	{
		error->code = ERROR_ROOM_DOESNT_EXIST;
		error->u16[0] = room;
		return;
	}
	
	/* Determines the offset where to put the static mesh */
	meshoffset = level->room[room].offset[R_ALTROOM];
	
	/* Makes room for the new static mesh */
	insertbytes(level->file, level->path, meshoffset,
	            level->partsize[R_STATICMESH], error);
	if (error->code != ERROR_NONE)
	{
		return;
	}
	level_struct_add(level, meshoffset, level->partsize[R_STATICMESH]);
	
	/* Adds one to NumStaticMeshes */
	++(level->room[room].value[R_NUMSTATICMESHES]);
	
	/* Writes NumStaticMeshes to the level file */
	meshnum = level->room[room].value[R_NUMSTATICMESHES];
	endian16(meshnum);
	seekval = trmod_fseek(level->file, (long int)
	                      level->room[room].offset[R_NUMSTATICMESHES],
	                      SEEK_SET);
	if (seekval != 0)
	{
		error->code = ERROR_FILE_READ_FAILED;
		error->string[0] = level->path;
		return;
	}
	writeval = trmod_fwrite(&meshnum, (size_t) 1, (size_t) 2, level->file);
	if (writeval != (size_t) 2)
	{
		error->code = ERROR_FILE_WRITE_FAILED;
		error->string[0] = level->path;
		return;
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
void tr1pc_replacestaticmesh(struct level *level, struct error *error,
                             uint16 mesh, uint16 id, uint16 room, int32 x,
                             int32 y, int32 z, int16 angle, int16 intensity,
                             uint16 rep)
{
	/* Variable Declarations */
	int seekval;       /* Return value for fseek() */
	size_t readval;    /* Return value for fread() */
	size_t writeval;   /* Return value for fwrite() */
	long int curpos;   /* Current position in the file */
	uint8 memmesh[18]; /* In-memory copy of the item */
	
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
	                     0x00000002 +
	                     ((uint32) (mesh * level->partsize[R_STATICMESH])));
	
	/* Reads the existing item into memory */
	seekval = trmod_fseek(level->file, curpos, SEEK_SET);
	if (seekval != 0)
	{
		error->code = ERROR_FILE_READ_FAILED;
		error->string[0] = level->path;
		return;
	}
	readval = trmod_fread(memmesh, (size_t) 1, (size_t) 18, level->file);
	if (readval != (size_t) 18)
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
		id = reverse16(id);
		x = reverse32(x);
		y = reverse32(y);
		z = reverse32(z);
		angle = reverse16(angle);
		intensity = reverse16(intensity);
	}
	
	/* Overwrites elements of the the item as needed */
	if ((rep & 0x0001) != 0x0000) /* ID */
	{
		memmesh[16] = (uint8) (id & 0x00FF);
		memmesh[17] = (uint8) ((id & 0xFF00) >> 8U);
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
	if ((rep & 0x0020) != 0x0000) /* Intensity */
	{
		memmesh[14] = (uint8) (intensity & 0x00FF);
		memmesh[15] = (uint8) ((intensity & 0xFF00) >> 8U);
	}
	
	/* Writes the static mesh back to the level */
	seekval = trmod_fseek(level->file, curpos, SEEK_SET);
	if (seekval != 0)
	{
		error->code = ERROR_FILE_READ_FAILED;
		error->string[0] = level->path;
		return;
	}
	writeval = trmod_fwrite(memmesh, (size_t) 1, (size_t) 18, level->file);
	if (writeval != (size_t) 18)
	{
		error->code = ERROR_FILE_WRITE_FAILED;
		error->string[0] = level->path;
		return;
	}
}

/*
 * Function that removes a static mesh from the level
 * Parameters:
 * * level = Pointer to level struct
 * * error = Pointer to error struct
 * * room = Static Mesh's room
 * * mesh = Number of the mesh to be removed
 */
void tr1pc_removestaticmesh(struct level *level, struct error *error,
                            uint16 room, uint16 mesh)
{
	/* Variable Declarations */
	uint32 offset;    /* Offset of the static mesh to remove */
	uint16 numMeshes; /* Number of static meshes */
	int seekval;      /* Return value of fseek() */
	size_t writeval;  /* Return value of fwrite() */
	
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
	
	/* Removes the static mesh */
	offset = (level->room[room].offset[R_NUMSTATICMESHES] + 0x00000002);
	offset += (uint32) (mesh * level->partsize[R_STATICMESH]);
	removebytes(level->file, level->path, offset,
	            level->partsize[R_STATICMESH], error);
	if (error->code != ERROR_NONE)
	{
		return;
	}
	level_struct_sub(level, offset, level->partsize[R_STATICMESH]);
	
	/* Subtracts 1 from NumStaticMeshes */
	--(level->room[room].value[R_NUMSTATICMESHES]);
	numMeshes = level->room[room].value[R_NUMSTATICMESHES];
	endian16(numMeshes);
	
	/* Writes NumStaticMeshes back to the level file */
	seekval = trmod_fseek(level->file, (long int)
	                      level->room[room].offset[R_NUMSTATICMESHES],
	                      SEEK_SET);
	if (seekval != 0)
	{
		error->code = ERROR_FILE_READ_FAILED;
		error->string[0] = level->path;
		return;
	}
	writeval = trmod_fwrite(&numMeshes, (size_t) 1, (size_t) 2, level->file);
	if (writeval != (size_t) 2)
	{
		error->code = ERROR_FILE_WRITE_FAILED;
		error->string[0] = level->path;
		return;
	}
}

/*
 * Function that removes all static meshes from a room
 * Parameters:
 * * level = Pointer to level struct
 * * error = Pointer to error struct
 * * room = Room to remove all static meshes from
 */
void tr1pc_removeallstaticmeshes(struct level *level,
                                 struct error *error, uint16 room)
{
	/* Variable Declarations */
	uint32 offset;   /* Offset of the static mesh to remove */
	uint32 size;     /* Number of bytes to remove */
	int seekval;     /* Return value of fseek() */
	size_t writeval; /* Return value of fwrite() */
	
	/* Checks whether the room exists */
	if (room >= level->numRooms)
	{
		error->code = ERROR_ROOM_DOESNT_EXIST;
		error->u16[0] = room;
		return;
	}
	
	/* Stops if there are no static meshes in this room */
	if (level->room[room].value[R_NUMSTATICMESHES] == 0x0000)
	{
		return;
	}
	
	/* Removes the static meshes */
	offset = (level->room[room].offset[R_NUMSTATICMESHES] + 0x00000002);
	size = (level->room[room].offset[R_ALTROOM] -
	        level->room[room].offset[R_NUMSTATICMESHES] - 0x00000002);
	removebytes(level->file, level->path, offset, size, error);
	if (error->code != ERROR_NONE)
	{
		return;
	}
	level_struct_sub(level, offset, size);
	
	/* Sets NumStaticMeshes to zero */
	offset = 0x00000000;
	
	/* Writes NumStaticMeshes back to the level file */
	seekval = trmod_fseek(level->file, (long int)
	                      level->room[room].offset[R_NUMSTATICMESHES],
	                      SEEK_SET);
	if (seekval != 0)
	{
		error->code = ERROR_FILE_READ_FAILED;
		error->string[0] = level->path;
		return;
	}
	writeval = trmod_fwrite(&offset, (size_t) 1, (size_t) 2, level->file);
	if (writeval != (size_t) 2)
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
void tr1pc_get_light(struct level *level, struct error *error,
                     uint16 room, uint16 light, uint16 print)
{
	/* Variable Declarations */
	uint8 memlight[18]; /* In-memory copy of the mesh */
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
	offset = (long int) (light * 0x0012);
	offset += (long int) (level->room[room].offset[R_NUMLIGHTS] + 0x00000002);
	seekval = trmod_fseek(level->file, offset, SEEK_SET);
	if (seekval != 0)
	{
		error->code = ERROR_FILE_READ_FAILED;
		error->string[0] = level->path;
		return;
	}
	readval = trmod_fread(memlight, (size_t) 1, (size_t) 18, level->file);
	if (readval != (size_t) 18)
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
	fade = (uint32) ((((uint32) memlight[17]) << 24U) |
	                 (((uint32) memlight[16]) << 16U) |
	                 (((uint32) memlight[15]) << 8U) |
	                 ((uint32) memlight[14]));
	
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
 * Function that adds a light to the level
 * Parameters:
 * * level = Pointer to level struct
 * * error = Pointer to error struct
 * * room = Light's room
 */
void tr1pc_addlight(struct level *level, struct error *error, uint16 room)
{
	/* Variable Declarations */
	uint32 lightoffset; /* Offset to write the light to */
	uint16 lightnum;    /* Item number to write */
	int seekval;        /* Return value for fseek() */
	size_t writeval;    /* Return value for fwrite() */
	
	/* Checks that the room exists */
	if (room >= level->numRooms)
	{
		error->code = ERROR_ROOM_DOESNT_EXIST;
		error->u16[0] = room;
		return;
	}
	
	/* Determines the offset where to put the light */
	lightoffset = level->room[room].offset[R_NUMSTATICMESHES];
	
	/* Makes room for the new light */
	insertbytes(level->file, level->path, lightoffset,
	            level->partsize[R_LIGHT], error);
	if (error->code != ERROR_NONE)
	{
		return;
	}
	level_struct_add(level, lightoffset, level->partsize[R_LIGHT]);
	
	/* Adds one to NumLights */
	++(level->room[room].value[R_NUMLIGHTS]);
	
	/* Writes NumLights to the level file */
	lightnum = level->room[room].value[R_NUMLIGHTS];
	endian16(lightnum);
	seekval = trmod_fseek(level->file,
	                      (long int) level->room[room].offset[R_NUMLIGHTS],
	                      SEEK_SET);
	if (seekval != 0)
	{
		error->code = ERROR_FILE_READ_FAILED;
		error->string[0] = level->path;
		return;
	}
	writeval = trmod_fwrite(&lightnum, (size_t) 1, (size_t) 2, level->file);
	if (writeval != (size_t) 2)
	{
		error->code = ERROR_FILE_WRITE_FAILED;
		error->string[0] = level->path;
		return;
	}
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
void tr1pc_replacelight(struct level *level, struct error *error, uint16 light,
                        uint16 room, int32 x, int32 y, int32 z, int16 inten,
                        uint32 fade, uint16 rep)
{
	/* Variable Declarations */
	int seekval;        /* Return value for fseek() */
	size_t readval;     /* Return value for fread() */
	size_t writeval;    /* Return value for fwrite() */
	long int curpos;    /* Current position in the file */
	uint8 memlight[18]; /* In-memory copy of the item */
	
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
	                     0x00000002 + ((uint32) (light * 0x0012)));
	
	/* Reads the existing item into memory */
	seekval = trmod_fseek(level->file, curpos, SEEK_SET);
	if (seekval != 0)
	{
		error->code = ERROR_FILE_READ_FAILED;
		error->string[0] = level->path;
		return;
	}
	readval = trmod_fread(memlight, (size_t) 1, (size_t) 18, level->file);
	if (readval != (size_t) 18)
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
	}
	if ((rep & 0x0020) != 0x0000) /* Fade */
	{
		memlight[14] = (uint8) (fade & 0x000000FF);
		memlight[15] = (uint8) ((fade & 0x0000FF00) >> 8U);
		memlight[16] = (uint8) ((fade & 0x00FF0000) >> 16U);
		memlight[17] = (uint8) ((fade & 0xFF000000) >> 24U);
	}
	
	/* Writes the light back to the level */
	seekval = trmod_fseek(level->file, curpos, SEEK_SET);
	if (seekval != 0)
	{
		error->code = ERROR_FILE_READ_FAILED;
		error->string[0] = level->path;
		return;
	}
	writeval = trmod_fwrite(memlight, (size_t) 1, (size_t) 18, level->file);
	if (writeval != (size_t) 18)
	{
		error->code = ERROR_FILE_WRITE_FAILED;
		error->string[0] = level->path;
		return;
	}
}

/*
 * Function that removes a light from the level
 * Parameters:
 * * level = Pointer to level struct
 * * error = Pointer to error struct
 * * room = Light's room
 * * light = Number of the light to be removed
 */
void tr1pc_removelight(struct level *level, struct error *error,
                       uint16 room, uint16 light)
{
	/* Variable Declarations */
	uint32 offset;    /* Offset of the light to remove */
	uint16 numLights; /* Number of lights */
	int seekval;      /* Return value of fseek() */
	size_t writeval;  /* Return value of fwrite() */
	
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
	
	/* Removes the light */
	offset = (level->room[room].offset[R_NUMLIGHTS] + 0x00000002);
	offset += (uint32) (light * level->partsize[R_LIGHT]);
	removebytes(level->file, level->path, offset,
	            (uint32) level->partsize[R_LIGHT], error);
	if (error->code != ERROR_NONE)
	{
		return;
	}
	level_struct_sub(level, offset, (uint32) level->partsize[R_LIGHT]);
	
	/* Subtracts 1 from NumLights */
	--(level->room[room].value[R_NUMLIGHTS]);
	numLights = level->room[room].value[R_NUMLIGHTS];
	endian16(numLights);
	
	/* Writes NumLights back to the level file */
	seekval = trmod_fseek(level->file,
	                      (long int) level->room[room].offset[R_NUMLIGHTS],
	                      SEEK_SET);
	if (seekval != 0)
	{
		error->code = ERROR_FILE_READ_FAILED;
		error->string[0] = level->path;
		return;
	}
	writeval = trmod_fwrite(&numLights, (size_t) 1, (size_t) 2, level->file);
	if (writeval != (size_t) 2)
	{
		error->code = ERROR_FILE_WRITE_FAILED;
		error->string[0] = level->path;
		return;
	}
}

/*
 * Function that removes all lights from a room
 * Parameters:
 * * level = Pointer to level struct
 * * error = Pointer to error struct
 * * room = Room to remove all lights from
 */
void tr1pc_removealllights(struct level *level, struct error *error,
                           uint16 room)
{
	/* Variable Declarations */
	uint32 offset;   /* Offset of the lights to remove */
	uint32 size;     /* Number of bytes to remove */
	int seekval;     /* Return value of fseek() */
	size_t writeval; /* Return value of fwrite() */
	
	/* Checks whether the room exists */
	if (room >= level->numRooms)
	{
		error->code = ERROR_ROOM_DOESNT_EXIST;
		error->u16[0] = room;
		return;
	}
	
	/* Stops if there are no lights in this room */
	if (level->room[room].value[R_NUMLIGHTS] == 0x0000)
	{
		return;
	}
	
	/* Removes the lights */
	offset = (level->room[room].offset[R_NUMLIGHTS] + 0x00000002);
	size = (level->room[room].offset[R_NUMSTATICMESHES] -
	        level->room[room].offset[R_NUMLIGHTS] - 0x00000002);
	removebytes(level->file, level->path, offset, size, error);
	if (error->code != ERROR_NONE)
	{
		return;
	}
	level_struct_sub(level, offset, size);
	
	/* Sets NumLights to zero */
	offset = 0x00000000;
	
	/* Writes NumLights back to the level file */
	seekval = trmod_fseek(level->file,
	                      (long int) level->room[room].offset[R_NUMLIGHTS],
	                      SEEK_SET);
	if (seekval != 0)
	{
		error->code = ERROR_FILE_READ_FAILED;
		error->string[0] = level->path;
		return;
	}
	writeval = trmod_fwrite(&offset, (size_t) 1, (size_t) 2, level->file);
	if (writeval != (size_t) 2)
	{
		error->code = ERROR_FILE_WRITE_FAILED;
		error->string[0] = level->path;
		return;
	}
}

/*
 * Function that prints a sound source
 * Parameters:
 * * level = Pointer to level struct
 * * error = Pointer to error struct
 * * source = Sound source number
 * * print = Printing flags
 * * * 0 0 0 0  0 0 0 0   0 0 0 0  0 0 0 0
 * * * | | | |  | | | |   | | | |  | | | |
 * * * | | | |  | | | |   | | | |  | | | As command
 * * * | | | |  | | | |   | | | |  | | As replace command
 * * * | | | |  | | | |   | | | |  | In Hexadecimal
 * * * Unused
 */
void tr1pc_get_soundsource(struct level *level, struct error *error,
                           uint32 source, uint16 print)
{
	/* Variable Declarations */
	uint8 memsource[16]; /* In-memory copy of the item */
	long int offset;     /* Offset of the item */
	int seekval;         /* Return value of fseek() */
	size_t readval;      /* Return value of fread() */
	uint16 id;           /* The Sound Source's ID */
	uint16 room;         /* The Sound Source's Room */
	int32 x;             /* The Sound Source's X-Position */
	int32 y;             /* The Sound Source's Y-Position */
	int32 z;             /* The Sound Source's Z-Position */
	uint16 flags;        /* The Sound Source's Flags */
	
	/* Checks whether the sound source exists */
	if (source >= level->value[NUMSOUNDSOURCES])
	{
		error->code = ERROR_SOUNDSOURCE_DOESNT_EXIST;
		error->u32[0] = source;
		return;
	}
	
	/* Reads the sound source to be printed into memory */
	offset = (long int) (source * 0x00000010);
	offset += (long int) (level->offset[NUMSOUNDSOURCES] + 0x00000004);
	seekval = trmod_fseek(level->file, offset, SEEK_SET);
	if (seekval != 0)
	{
		error->code = ERROR_FILE_READ_FAILED;
		error->string[0] = level->path;
		return;
	}
	readval = trmod_fread(memsource, (size_t) 1, (size_t) 16, level->file);
	if (readval != (size_t) 16)
	{
		error->code = ERROR_FILE_READ_FAILED;
		error->string[0] = level->path;
		return;
	}
	
	/* Prints as hex dump if needed */
	if ((print & 0x0004) != 0x0000)
	{
		for (id = 0x0000U; id < (uint16) level->partsize[SOUNDSOURCE]; ++id)
		{
			trmod_printf("%02X ", memsource[id]);
		}
		trmod_printf("\n");
		return;
	}
	
	/* Interprets the read sound source */
	x = (int32) ((((uint32) memsource[3]) << 24U) |
	             (((uint32) memsource[2]) << 16U) |
	             (((uint32) memsource[1]) << 8U) | ((uint32) memsource[0]));
	y = (int32) ((((uint32) memsource[7]) << 24U) |
	             (((uint32) memsource[6]) << 16U) |
	             (((uint32) memsource[5]) << 8U) | ((uint32) memsource[4]));
	z = (int32) ((((uint32) memsource[11]) << 24U) |
	             (((uint32) memsource[10]) << 16U) |
	             (((uint32) memsource[9]) << 8U) | ((uint32) memsource[8]));
	id = (uint16) ((((uint16) memsource[13]) << 8U) |
	               ((uint16) memsource[12]));
	flags = (uint16) ((((uint16) memsource[15]) << 8U) |
	                  ((uint16) memsource[14]));
	
	if (level->endian == TRUE)
	{
		x = reverse32(x);
		y = reverse32(y);
		z = reverse32(z);
		id = reverse16(id);
		flags = reverse16(flags);
	}
	
	/* Finds the nearest room */
	room = nearestRoom(level, error, x, y, z);
	
	/* Sets the coordinates to local coordinates */
	x -= level->room[room].x;
	y = (level->room[room].yBottom - y);
	z -= level->room[room].z;
	
	/* Determines how to print the sound source */
	if ((print & 0x0003) != 0x0000)
	{
		trmod_printf("%s %s %s ",
		             level->command, level->typestring, level->path);
		if ((print & 0x0002) != 0x0000)
		{
			trmod_printf("\"replacesoundsource(" PRINTU32 ",", source);
		}
		else
		{
			trmod_printf("\"addsoundsource(");
		}
	}
	else
	{
		trmod_printf("SoundSource(");
	}
	
	/* Prints the SoundID and coordinates */
	trmod_printf(PRINTU16 "," PRINTU16 "," PRINT32 "," PRINT32 ","
	             PRINT32 ",", id, room, x, z, y);
	
	if ((print & 0x0003) == 0x0000)
	{
		switch (flags)
		{
			case 0x0080:
				trmod_printf("Room)");
				break;
			case 0x0040:
				trmod_printf("AltRoom)");
				break;
			case 0x00C0:
				trmod_printf("Both)");
				break;
			default:
				trmod_printf("%04X)", flags);
				break;
		}
	}
	else
	{
		switch (flags)
		{
			case 0x0080:
				trmod_printf("room)");
				break;
			case 0x0040:
				trmod_printf("altroom)");
				break;
			case 0x00C0:
				trmod_printf("both)");
				break;
			default:
				trmod_printf("%04X)", flags);
				break;
		}
	}
	
	/* Adds the closing quote if needed and prints the newline */
	if ((print & 0x0003) != 0x0000)
	{
		trmod_printf("\"");
	}
	trmod_printf("\n");
}

/*
 * Function that adds a sound source to the level
 * Parameters:
 * * level = Pointer to level struct
 * * error = Pointer to error struct
 */
void tr1pc_addsoundsource(struct level *level, struct error *error)
{
	/* Variable Declarations */
	uint32 sourceoffset; /* Offset to write the item to */
	uint32 sourcenum;    /* Item number to write */
	int seekval;         /* Return value for fseek() */
	size_t writeval;     /* Return value for fwrite() */
	
	/* Determines the offset where to put the sound source */
	sourceoffset = (level->offset[NUMSOUNDSOURCES] + 0x00000004 +
	                (level->value[NUMSOUNDSOURCES] *
	                 level->partsize[SOUNDSOURCE]));
	
	/* Makes room for the new sound source */
	insertbytes(level->file, level->path, sourceoffset,
	            level->partsize[SOUNDSOURCE], error);
	if (error->code != ERROR_NONE)
	{
		return;
	}
	level_struct_add(level, sourceoffset, level->partsize[SOUNDSOURCE]);
	
	/* Adds one to NumSoundSources */
	++(level->value[NUMSOUNDSOURCES]);
	
	/* Writes NumSoundSources to the level file */
	sourcenum = level->value[NUMSOUNDSOURCES];
	endian32(sourcenum);
	seekval = trmod_fseek(level->file,
	                      (long int) level->offset[NUMSOUNDSOURCES], SEEK_SET);
	if (seekval != 0)
	{
		error->code = ERROR_FILE_READ_FAILED;
		error->string[0] = level->path;
		return;
	}
	writeval = trmod_fwrite(&sourcenum, (size_t) 1, (size_t) 4, level->file);
	if (writeval != (size_t) 4)
	{
		error->code = ERROR_FILE_WRITE_FAILED;
		error->string[0] = level->path;
		return;
	}
}

/*
 * Function that replaces a sound source in the level
 * Parameters:
 * * level = Pointer to level struct
 * * error = Pointer to error struct
 * * source = Number of the sound source to be replaced (max int means last one)
 * * id = SoundID
 * * room = Room
 * * x = Sound Source's X-position (in local coordnates)
 * * y = Sound Source's Y-position (in local coordnates)
 * * z = Sound Source's Z-position (in local coordnates)
 * * flags = Sound Source's flags
 * * rep = Bit-mask of elements to replace
 * * * 0 0 0 0  0 0 0 0   0 0 0 0  0 0 0 0
 * * * | | | |  | | | |   | | | |  | | | |
 * * * | | | |  | | | |   | | | |  | | | ID
 * * * | | | |  | | | |   | | | |  | | Room
 * * * | | | |  | | | |   | | | |  | |
 * * * | | | |  | | | |   | | | |  | X
 * * * | | | |  | | | |   | | | |  Y
 * * * | | | |  | | | |   | | | Z
 * * * | | | |  | | | |   | | Flags
 * * * Unused
 */
void tr1pc_replacesoundsource(struct level *level, struct error *error,
                              uint32 source, uint16 id, uint16 room, int32 x,
                              int32 y, int32 z, uint16 flags, uint16 rep)
{
	/* Variable Declarations */
	int seekval;      /* Return value for fseek() */
	size_t readval;   /* Return value for fread() */
	size_t writeval;  /* Return value for fwrite() */
	long int curpos;  /* Current position in the file */
	uint8 memsrc[16]; /* In-memory copy of the sound source */
	
	/* Sets source to last one if max int */
	if (source == 0xFFFFFFFF)
	{
		source = level->value[NUMSOUNDSOURCES];
		--source;
	}
	
	/* Checks whether the item exists */
	if (source >= level->value[NUMSOUNDSOURCES])
	{
		error->code = ERROR_SOUNDSOURCE_DOESNT_EXIST;
		error->u32[0] = source;
		return;
	}
	/* Checks whether the room exists */
	if (((rep & 0x0002) != 0x0000) && (room >= level->numRooms))
	{
		error->code = ERROR_ROOM_DOESNT_EXIST;
		error->u16[0] = room;
		return;
	}
	
	/* Determines offset of the sound source */
	curpos = (long int) (level->offset[NUMSOUNDSOURCES] + 0x00000004 +
	                     (source * 0x0010));
	
	/* Reads the existing item into memory */
	seekval = trmod_fseek(level->file, curpos, SEEK_SET);
	if (seekval != 0)
	{
		error->code = ERROR_FILE_READ_FAILED;
		error->string[0] = level->path;
		return;
	}
	readval = trmod_fread(memsrc, (size_t) 1, (size_t) 16, level->file);
	if (readval != (size_t) 16)
	{
		error->code = ERROR_FILE_READ_FAILED;
		error->string[0] = level->path;
		return;
	}
	
	/* Determines what room to work with if no room as provided */
	if ((rep & 0x0002) == 0x0000)
	{
		/* Finds the nearest room */
		room = nearestRoom(level, error, x, y, z);
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
		id = reverse16(id);
		flags = reverse16(flags);
	}
	
	/* Overwrites elements of the the item as needed */
	if ((rep & 0x0001) != 0x0000) /* ID */
	{
		memsrc[12] = (uint8) (id & 0x00FF);
		memsrc[13] = (uint8) ((id & 0xFF00) >> 8U);
	}
	if ((rep & 0x0004) != 0x0000) /* X */
	{
		memsrc[0] = (uint8) (x & 0x000000FF);
		memsrc[1] = (uint8) ((x & 0x0000FF00) >> 8U);
		memsrc[2] = (uint8) ((x & 0x00FF0000) >> 16U);
		memsrc[3] = (uint8) ((x & 0xFF000000) >> 24U);
	}
	if ((rep & 0x0008) != 0x0000) /* Y */
	{
		memsrc[4] = (uint8) (y & 0x000000FF);
		memsrc[5] = (uint8) ((y & 0x0000FF00) >> 8U);
		memsrc[6] = (uint8) ((y & 0x00FF0000) >> 16U);
		memsrc[7] = (uint8) ((y & 0xFF000000) >> 24U);
	}
	if ((rep & 0x0010) != 0x0000) /* Z */
	{
		memsrc[8] = (uint8) (z & 0x000000FF);
		memsrc[9] = (uint8) ((z & 0x0000FF00) >> 8U);
		memsrc[10] = (uint8) ((z & 0x00FF0000) >> 16U);
		memsrc[11] = (uint8) ((z & 0xFF000000) >> 24U);
	}
	if ((rep & 0x0020) != 0x0000) /* Flags */
	{
		memsrc[14] = (uint8) (flags & 0x00FF);
		memsrc[15] = (uint8) ((flags & 0xFF00) >> 8U);
	}
	
	/* Writes the item back to the level */
	seekval = trmod_fseek(level->file, curpos, SEEK_SET);
	if (seekval != 0)
	{
		error->code = ERROR_FILE_READ_FAILED;
		error->string[0] = level->path;
		return;
	}
	writeval = trmod_fwrite(memsrc, (size_t) 1, (size_t) 16, level->file);
	if (writeval != (size_t) 16)
	{
		error->code = ERROR_FILE_WRITE_FAILED;
		error->string[0] = level->path;
		return;
	}
}

/*
 * Function that removes a sound source from the level
 * Parameters:
 * * level = Pointer to the level struct
 * * error = Pointer to the error struct
 * * source = Sound source to be removed
 */
void tr1pc_removesoundsource(struct level *level, struct error *error,
                             uint32 source)
{
	/* Variable Declarations */
	uint32 offset;     /* Offset of the data to be removed */
	uint32 numSources; /* Copy of NumSoundSources */
	int seekval;       /* Return value for fseek() */
	size_t writeval;   /* Return value for fwrite() */
	
	/* Checks whether the sound source exists */
	if (source >= level->value[NUMSOUNDSOURCES])
	{
		error->code = ERROR_SOUNDSOURCE_DOESNT_EXIST;
		error->u32[0] = source;
		return;
	}
	
	/* Removes the sound source */
	offset = (source * level->partsize[SOUNDSOURCE]);
	offset += 0x00000004;
	offset += level->offset[NUMSOUNDSOURCES];
	removebytes(level->file, level->path, offset,
	            level->partsize[SOUNDSOURCE], error);
	if (error->code != ERROR_NONE)
	{
		return;
	}
	level_struct_sub(level, offset, level->partsize[SOUNDSOURCE]);
	
	/* Subtracts 1 from NumSoundSources */
	--(level->value[NUMSOUNDSOURCES]);
	numSources = level->value[NUMSOUNDSOURCES];
	endian32(numSources);
	
	/* Writes NumSoundSources back to the level file */
	seekval = trmod_fseek(level->file,
	                      (long int) level->offset[NUMSOUNDSOURCES], SEEK_SET);
	if (seekval != 0)
	{
		error->code = ERROR_FILE_READ_FAILED;
		error->string[0] = level->path;
		return;
	}
	writeval = trmod_fwrite(&numSources, (size_t) 1, (size_t) 4, level->file);
	if (writeval != (size_t) 4)
	{
		error->code = ERROR_FILE_WRITE_FAILED;
		error->string[0] = level->path;
		return;
	}
}

/*
 * Function that removes all sound sources from the level
 * Parameters:
 * * level = Pointer to the level struct
 * * error = Pointer to the error struct
 */
void tr1pc_removeallsoundsources(struct level *level, struct error *error)
{
	/* Variable Declarations */
	uint32 offset;   /* Offset of the data to be removed */
	uint32 size;     /* Size of the data to be removed */
	int seekval;     /* Return value for fseek() */
	size_t writeval; /* Return value for fwrite() */
	
	/* Does nothing if there are no sound sources at all */
	if (level->value[NUMSOUNDSOURCES] == 0x00000000)
	{
		return;
	}
	
	/* Removes sound sources */
	offset = (level->offset[NUMSOUNDSOURCES] + 0x00000004);
	size = (level->value[NUMSOUNDSOURCES] * level->partsize[SOUNDSOURCE]);
	removebytes(level->file, level->path, offset, size, error);
	if (error->code != ERROR_NONE)
	{
		return;
	}
	level_struct_sub(level, offset, size);
	
	/* Sets NumSoundSources to zero */
	level->value[NUMSOUNDSOURCES] = 0x00000000;
	
	/* Writes NumSoundSources back to the level file */
	seekval = trmod_fseek(level->file,
	                      (long int) level->offset[NUMSOUNDSOURCES], SEEK_SET);
	if (seekval != 0)
	{
		error->code = ERROR_FILE_READ_FAILED;
		error->string[0] = level->path;
		return;
	}
	writeval = trmod_fwrite(&level->value[NUMSOUNDSOURCES], (size_t) 1,
	                        (size_t) 4, level->file);
	if (writeval != (size_t) 4)
	{
		error->code = ERROR_FILE_WRITE_FAILED;
		error->string[0] = level->path;
		return;
	}
}

/*
 * Function that prints a FloorData-entry
 * Parameters:
 * * level = Pointer to level struct
 * * error = Pointer to error struct
 * * room = Room number
 * * column = Column number
 * * row = Row number
 * * secpad = Padding between NumXSectors and the actual sectors (usually 0)
 * * print = Printing flags
 * * * 0 0 0 0  0 0 0 0   0 0 0 0  0 0 0 0
 * * * | | | |  | | | |   | | | |  | | | |
 * * * | | | |  | | | |   | | | |  | | | As command
 * * * Unused
 */
void tr1pc_get_floordata(struct level *level, struct error *error,
                         uint16 room, uint16 column, uint16 row,
                         uint32 secpad, uint16 print)
{
	/* Variable Declarations */
	long int offset;    /* Offset where to read from */
	long int camoffset; /* Offset where to read the camera from */
	uint8 camera[16];   /* In-memory copy of camera (if needed) */
	uint16 fdIndex;     /* FloorData-index of this sector */
	uint8 roombelow;    /* RoomBelow of this sector */
	uint8 roomabove;    /* RoomAbove of this sector */
	uint16 u16[3];      /* 16-bit unsigned integers */
	int16 s16[2];       /* 16-bit signed integers */
	int32 s32[3];       /* 32-bit signed integers */
	uint16 fdsetup;     /* FloorData Setup */
	uint16 endData;     /* End-bit of FloorData entry */
	uint16 trigsetup;   /* Trigger setup */
	uint16 trigaction;  /* Trigger action */
	uint16 contBit;     /* Continuation bit for trigger actions */
	int seekval;        /* Return value for fseek() */
	size_t readval;     /* Return value for fread() */
	
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
	offset = (long int) (level->room[room].offset[R_NUMXSECTORS] + 0x00000002);
	offset += (long int) (((level->room[room].value[R_NUMZSECTORS] * column) +
	                       row) * 0x0008);
	offset += (long int) secpad;
	
	/* Reads FDIndex, Roombelow and Roomabove */
	seekval = trmod_fseek(level->file, offset, SEEK_SET);
	if (seekval != 0)
	{
		error->code = ERROR_FILE_READ_FAILED;
		error->string[0] = level->path;
		return;
	}
	readval = trmod_fread(&fdIndex, (size_t) 1, (size_t) 2, level->file);
	if (readval != (size_t) 2)
	{
		error->code = ERROR_FILE_READ_FAILED;
		error->string[0] = level->path;
		return;
	}
	endian16(fdIndex);
	offset += 4l;
	seekval = trmod_fseek(level->file, offset, SEEK_SET);
	if (seekval != 0)
	{
		error->code = ERROR_FILE_READ_FAILED;
		error->string[0] = level->path;
		return;
	}
	readval = trmod_fread(&roombelow, (size_t) 1, (size_t) 1, level->file);
	if (readval != (size_t) 1)
	{
		error->code = ERROR_FILE_READ_FAILED;
		error->string[0] = level->path;
		return;
	}
	offset += 2l;
	seekval = trmod_fseek(level->file, offset, SEEK_SET);
	if (seekval != 0)
	{
		error->code = ERROR_FILE_READ_FAILED;
		error->string[0] = level->path;
		return;
	}
	readval = trmod_fread(&roomabove, (size_t) 1, (size_t) 1, level->file);
	if (readval != (size_t) 1)
	{
		error->code = ERROR_FILE_READ_FAILED;
		error->string[0] = level->path;
		return;
	}
	
	/* Ends the function if there's nothing to print */
	if ((fdIndex == 0x0000) && (roombelow == (uint8) 0xFF) &&
	    (roomabove == (uint8) 0xFF))
	{
		return;
	}
	
	/* Determines whether this FloorData-entry exists */
	if (fdIndex > (uint16) level->value[NUMFLOORDATA])
	{
		error->code = ERROR_FDINDEX_OUT_OF_RANGE;
		error->u16[0] = fdIndex;
		return;
	}
	
	/* Determines how to print the floordata entry */
	if ((print & 0x0001) != 0x0000)
	{
		trmod_printf("%s %s %s \"replacefloordata(" PRINTU16 "," PRINTU16 ","
		             PRINTU16 ":", level->command,
		             level->typestring, level->path, room, column, row);
	}
	else
	{
		trmod_printf("FloorData(" PRINTU16 "," PRINTU16 "," PRINTU16 "\n",
		             room, column, row);
	}
	
	/* Prints Floorportal and Celingportal */
	if ((print & 0x0001) != 0x0000)
	{
		if (roombelow != (uint8) 0xFF)
		{
			trmod_printf("floorportal(" PRINTU16 ")",
			             (((uint16) roombelow) & 0x00FF));
			if ((roomabove != (uint8) 0xFF) || (fdIndex != 0x0000))
			{
				trmod_printf(",");
			}
		}
		if (roomabove != (uint8) 0xFF)
		{
			trmod_printf("ceilingportal(" PRINTU16 ")",
			             (((uint16) roomabove) & 0x00FF));
			if (fdIndex != 0x0000)
			{
				trmod_printf(",");
			}
		}
	}
	else
	{
		if (roombelow != (uint8) 0xFF)
		{
			trmod_printf("    Floorportal(" PRINTU16 ")\n",
			             (((uint16) roombelow) & 0x00FF));
		}
		if (roomabove != (uint8) 0xFF)
		{
			trmod_printf("    Ceilingportal(" PRINTU16 ")\n",
			             (((uint16) roomabove) & 0x00FF));
		}
	}
	
	/* Prints the actual FloorData-entry */
	if (fdIndex != 0x0000)
	{
		/* Determines where the FloorData-entry starts */
		offset = (long int) fdIndex;
		offset *= 2l;
		offset += (long int) (level->offset[NUMFLOORDATA] + 0x00000004);
		
		/* Processes the FloorData-entries */
		do
		{
			/* Reads FDSetup */
			seekval = trmod_fseek(level->file, offset, SEEK_SET);
			if (seekval != 0)
			{
				error->code = ERROR_FILE_READ_FAILED;
				error->string[0] = level->path;
				return;
			}
			offset += 2l;
			readval = trmod_fread(&fdsetup, (size_t) 1,
			                      (size_t) 2, level->file);
			if (readval != (size_t) 2)
			{
				error->code = ERROR_FILE_READ_FAILED;
				error->string[0] = level->path;
				return;
			}
			endian16(fdsetup);
			endData = (fdsetup & 0x8000);
			
			/* Chooses the appropriate Function */
			switch (fdsetup & 0x001F)
			{
				case 0x0001: /* Portal */
					/* Reads the adjacent room into memory */
					seekval = trmod_fseek(level->file, offset, SEEK_SET);
					if (seekval != 0)
					{
						error->code = ERROR_FILE_READ_FAILED;
						error->string[0] = level->path;
						return;
					}
					offset += 2l;
					readval = trmod_fread(u16, (size_t) 1,
					                      (size_t) 2, level->file);
					if (readval != (size_t) 2)
					{
						error->code = ERROR_FILE_READ_FAILED;
						error->string[0] = level->path;
						return;
					}
					endian16(u16[0]);
					if ((print & 0x0001) != 0x0000)
					{
						trmod_printf("portal(" PRINTU16 ")", u16[0]);
					}
					else
					{
						trmod_printf("    Portal(" PRINTU16 ")\n", u16[0]);
					}
					break;
				case 0x0002: /* Floor Slant */
					/* Reads the slant values into memory */
					seekval = trmod_fseek(level->file, offset, SEEK_SET);
					if (seekval != 0)
					{
						error->code = ERROR_FILE_READ_FAILED;
						error->string[0] = level->path;
						return;
					}
					offset += 2l;
					readval = trmod_fread(u16, (size_t) 1,
					                      (size_t) 2, level->file);
					if (readval != (size_t) 2)
					{
						error->code = ERROR_FILE_READ_FAILED;
						error->string[0] = level->path;
						return;
					}
					endian16(u16[0]);
					
					/* Separates the X- and Z-slants, and extends signs */
					s16[0] = (int16) (0xFF00 | (u16[0] & 0x00FF));
					s16[1] = (int16) (0xFF00 | (u16[0] & 0xFF00) >> 8U);
					if ((s16[0] & 0x0080) == 0x0000)
					{
						s16[0] &= 0x00FF;
					}
					if ((s16[1] & 0x0080) == 0x0000)
					{
						s16[1] &= 0x00FF;
					}
					
					/* Prints the slant */
					if ((print & 0x0001) != 0x0000)
					{
						trmod_printf("floorslant(" PRINT16 "," PRINT16 ")",
						             s16[0], s16[1]);
					}
					else
					{
						trmod_printf("    Floorslant(" PRINT16 "," PRINT16
						             ")\n", s16[0], s16[1]);
					}
					break;
				case 0x0003: /* Ceiling Slant */
					/* Reads the slant values into memory */
					seekval = trmod_fseek(level->file, offset, SEEK_SET);
					if (seekval != 0)
					{
						error->code = ERROR_FILE_READ_FAILED;
						error->string[0] = level->path;
						return;
					}
					offset += 2l;
					readval = trmod_fread(u16, (size_t) 1,
					                      (size_t) 2, level->file);
					if (readval != (size_t) 2)
					{
						error->code = ERROR_FILE_READ_FAILED;
						error->string[0] = level->path;
						return;
					}
					endian16(u16[0]);
					
					/* Separates the X- and Z-slants, and extends signs */
					s16[0] = (int16) (0xFF00 | (u16[0] & 0x00FF));
					s16[1] = (int16) (0xFF00 | (u16[0] & 0xFF00) >> 8U);
					if ((s16[0] & 0x0080) == 0x0000)
					{
						s16[0] &= 0x00FF;
					}
					if ((s16[1] & 0x0080) == 0x0000)
					{
						s16[1] &= 0x00FF;
					}
					
					/* Prints the slant */
					if ((print & 0x0001) != 0x0000)
					{
						trmod_printf("ceilingslant(" PRINT16 "," PRINT16 ")",
						             s16[0], s16[1]);
					}
					else
					{
						trmod_printf("    Ceilingslant(" PRINT16 "," PRINT16
						             ")\n", s16[0], s16[1]);
					}
					break;
				case 0x0004: /* Trigger */
					if ((print & 0x0001) != 0x0000)
					{
						trmod_printf("trigger(");
					}
					else
					{
						trmod_printf("    Trigger(");
					}
					
					/* Reads TriggerSetup */
					seekval = trmod_fseek(level->file, offset, SEEK_SET);
					if (seekval != 0)
					{
						error->code = ERROR_FILE_READ_FAILED;
						error->string[0] = level->path;
						return;
					}
					offset += 2l;
					readval = trmod_fread(&trigsetup, (size_t) 1,
					                      (size_t) 2, level->file);
					if (readval != (size_t) 2)
					{
						error->code = ERROR_FILE_READ_FAILED;
						error->string[0] = level->path;
						return;
					}
					endian16(trigsetup);
					
					/* Prints trigger type */
					switch (fdsetup & 0x7F00)
					{
						case 0x0000: /* Trigger */
							trmod_printf("%crigger,",
							             (((print & 0x0001) != 0x0000) ?
							              't' : 'T'));
							break;
						case 0x0100: /* Pad */
							trmod_printf("%cad,",
							             (((print & 0x0001) != 0x0000) ?
							              'p' : 'P'));
							break;
						case 0x0200: /* Switch */
						case 0x0300: /* Key */
						case 0x0400: /* Pickup */
							/* Reads the item number into memory */
							seekval = trmod_fseek(level->file,
							                      offset, SEEK_SET);
							if (seekval != 0)
							{
								error->code = ERROR_FILE_READ_FAILED;
								error->string[0] = level->path;
								return;
							}
							offset += 2l;
							readval = trmod_fread(u16, (size_t) 1,
							                      (size_t) 2, level->file);
							if (readval != (size_t) 2)
							{
								error->code = ERROR_FILE_READ_FAILED;
								error->string[0] = level->path;
								return;
							}
							endian16(u16[0]);
							u16[0] &= 0x03FF;
							
							/* Prints the trigger type */
							if ((fdsetup & 0x7F00) == 0x0200)
							{
								trmod_printf("%cwitch(" PRINTU16 "),",
								             (((print & 0x0001) != 0x0000) ?
								              's' : 'S'), u16[0]);
							}
							else if ((fdsetup & 0x7F00) == 0x0300)
							{
								trmod_printf("%cey(" PRINTU16 "),",
								             (((print & 0x0001) != 0x0000) ?
								              'k' : 'K'), u16[0]);
							}
							else if ((fdsetup & 0x7F00) == 0x0400)
							{
								trmod_printf("%cickup(" PRINTU16 "),",
								             (((print & 0x0001) != 0x0000) ?
								              'p' : 'P'), u16[0]);
							}
							break;
						case 0x0500: /* Heavytrigger */
							trmod_printf("%ceavytrigger,",
							             (((print & 0x0001) != 0x0000) ?
							              'h' : 'H'));
							break;
						case 0x0600: /* Antipad */
							trmod_printf("%cntipad,",
							             (((print & 0x0001) != 0x0000) ?
							              'a' : 'A'));
							break;
						case 0x0700: /* Combat */
							trmod_printf("%combat,",
							             (((print & 0x0001) != 0x0000) ?
							              'c' : 'C'));
							break;
						case 0x0800: /* Dummy */
							trmod_printf("%cummy,",
							             (((print & 0x0001) != 0x0000) ?
							              'd' : 'D'));
							break;
						case 0x0900: /* Antitrigger */
							trmod_printf("%cntitrigger,",
							             (((print & 0x0001) != 0x0000) ?
							              'a' : 'A'));
							break;
						case 0x0A00: /* Heavyswitch */
							trmod_printf("%ceavyswitch,",
							             (((print & 0x0001) != 0x0000) ?
							              'h' : 'H'));
							break;
						case 0x0B00: /* Heavyantitrigger */
							trmod_printf("%ceavyantitrigger,",
							             (((print & 0x0001) != 0x0000) ?
							              'h' : 'H'));
							break;
						case 0x0C00: /* Monkey */
							trmod_printf("%conkey,",
							             (((print & 0x0001) != 0x0000) ?
							              'm' : 'M'));
							break;
						case 0x0D00: /* Skeleton */
							trmod_printf("%ckeleton,",
							             (((print & 0x0001) != 0x0000) ?
							              's' : 'S'));
							break;
						case 0x0E00: /* Tightrope */
							trmod_printf("%cightrope,",
							             (((print & 0x0001) != 0x0000) ?
							              't' : 'T'));
							break;
						case 0x0F00: /* Crawl */
							trmod_printf("%crawl,",
							             (((print & 0x0001) != 0x0000) ?
							              'c' : 'C'));
							break;
						case 0x1000: /* Climb */
							trmod_printf("%climb,",
							             (((print & 0x0001) != 0x0000) ?
							              'c' : 'C'));
							break;
					}
					
					/* Prints timer, oneshot and mask */
					trmod_printf(PRINTU16 ",%s,%02X%s",
					             (trigsetup & 0x00FF),         /* Timer */
					             (((trigsetup & 0x0100) != 0x0000) ?
					              "once" : ""),
					             ((trigsetup & 0x3E00) >> 9U), /* Mask */
					             (((print & 0x0001) == 0x0000) ? "\n" : ":"));
					
					/* Processes Trigger Actions */
					contBit = 0x0000;
					do
					{
						/* Reads Trigger Action */
						seekval = trmod_fseek(level->file, offset, SEEK_SET);
						if (seekval != 0)
						{
							error->code = ERROR_FILE_READ_FAILED;
							error->string[0] = level->path;
							return;
						}
						offset += 2l;
						readval = trmod_fread(&trigaction, (size_t) 1,
						                      (size_t) 2, level->file);
						if (readval != (size_t) 2)
						{
							error->code = ERROR_FILE_READ_FAILED;
							error->string[0] = level->path;
							return;
						}
						endian16(trigaction);
						contBit |= (trigaction & 0x8000);
						
						/* Determines Trigger Action */
						switch (trigaction & 0x7C00)
						{
							case 0x0000: /* Object */
								if ((print & 0x0001) == 0x0000)
								{
									trmod_printf("        Object(" PRINTU16
									             ")\n", (trigaction & 0x03FF));
								}
								else
								{
									trmod_printf("object(" PRINTU16 ")",
									             (trigaction & 0x03FF));
								}
								break;
							case 0x0400: /* Camera */
								if ((print & 0x0001) == 0x0000)
								{
									trmod_printf("        Camera(");
								}
								else
								{
									trmod_printf("camera(");
								}
								
								/* Reads the camera into memory */
								camoffset = (long int) (trigaction & 0x03FF);
								camoffset *= 16l;
								camoffset +=
									(long int) level->offset[NUMCAMERAS];
								camoffset += 4l;
								seekval = trmod_fseek(level->file, camoffset,
								                      SEEK_SET);
								if (seekval != 0)
								{
									error->code = ERROR_FILE_READ_FAILED;
									error->string[0] = level->path;
									return;
								}
								readval = trmod_fread(&camera, (size_t) 1,
								                      (size_t) 16, level->file);
								if (readval != (size_t) 16)
								{
									error->code = ERROR_FILE_READ_FAILED;
									error->string[0] = level->path;
									return;
								}
								
								/* Gets coordinates and flags */
								s32[0] = (int32)
									((((uint32) camera[3]) << 24U) |
									 (((uint32) camera[2]) << 16U) |
									 (((uint32) camera[1]) << 8U)  |
									 ((uint32) camera[0]));
								s32[1] = (int32)
									((((uint32) camera[7]) << 24U) |
									 (((uint32) camera[6]) << 16U) |
									 (((uint32) camera[5]) << 8U)  |
									 ((uint32) camera[4]));
								s32[2] = (int32)
									((((uint32) camera[11]) << 24U) |
									 (((uint32) camera[10]) << 16U) |
									 (((uint32) camera[9]) << 8U)   |
									 ((uint32) camera[8]));
								u16[1] = (uint16)
									((((uint16) camera[13]) << 8U) |
									 ((uint16) camera[12]));
								u16[0] = (uint16)
									((((uint16) camera[15]) << 8U) |
									 ((uint16) camera[14]));

								/* Adjusts for endianness */
								if (level->endian == TRUE)
								{
									s32[0] = reverse32(s32[0]);
									s32[1] = reverse32(s32[1]);
									s32[2] = reverse32(s32[2]);
									u16[0] = reverse16(u16[0]);
									u16[1] = reverse16(u16[1]);
								}
								
								/* Checks the existence of the room */
								if (u16[1] >= level->numRooms)
								{
									error->code = ERROR_ROOM_DOESNT_EXIST;
									error->u16[0] = u16[1];
									return;
								}
								
								/* Sets the coordinates to local coordinates */
								s32[0] -= level->room[u16[1]].x;
								s32[1] = (level->room[u16[1]].yBottom - s32[1]);
								s32[2] -= level->room[u16[1]].z;
								
								/* Prints coordinates */
								trmod_printf(PRINTU16 "," PRINT32 "," PRINT32
								             "," PRINT32 ",", u16[1], s32[0],
								             s32[2], s32[1]);
								
								/* Reads extra camera integer */
								seekval = trmod_fseek(level->file, offset,
								                      SEEK_SET);
								if (seekval != 0)
								{
									error->code = ERROR_FILE_READ_FAILED;
									error->string[0] = level->path;
									return;
								}
								offset += 2l;
								readval = trmod_fread(&u16[2], (size_t) 1,
								                      (size_t) 2, level->file);
								if (readval != (size_t) 2)
								{
									error->code = ERROR_FILE_READ_FAILED;
									error->string[0] = level->path;
									return;
								}
								endian16(u16[2]);
								contBit |= (u16[2] & 0x8000);
								
								/* Prints timer, speed, once and flags */
								trmod_printf(PRINTU16 "," PRINTU16 ",%s,%04X)",
								             (u16[2] & 0x00FF),
								             ((u16[2] & 0x3E00) >> 9U),
								             (((u16[2] & 0x0100) == 0x0100) ?
								              "once" : ""), u16[0]);
								if ((print & 0x0001) == 0x0000)
								{
									trmod_printf("\n");
								}
								break;
							case 0x0800: /* Current */
								if ((print & 0x0001) == 0x0000)
								{
									trmod_printf("        Underwater Current(");
								}
								else
								{
									trmod_printf("current(");
								}
								
								/* Reads the sink into memory */
								camoffset = (long int) (trigaction & 0x03FF);
								camoffset *= 16l;
								camoffset +=
									(long int) level->offset[NUMCAMERAS];
								camoffset += 4l;
								seekval = trmod_fseek(level->file, camoffset,
								                      SEEK_SET);
								if (seekval != 0)
								{
									error->code = ERROR_FILE_READ_FAILED;
									error->string[0] = level->path;
									return;
								}
								readval = trmod_fread(&camera, (size_t) 1,
								                      (size_t) 16, level->file);
								if (readval != (size_t) 16)
								{
									error->code = ERROR_FILE_READ_FAILED;
									error->string[0] = level->path;
									return;
								}
								
								/* Gets coordinates, strength and zone */
								s32[0] = (int32)
									((((uint32) camera[3]) << 24U) |
									 (((uint32) camera[2]) << 16U) |
									 (((uint32) camera[1]) << 8U) |
									 ((uint32) camera[0]));
								s32[1] = (int32)
									((((uint32) camera[7]) << 24U) |
									 (((uint32) camera[6]) << 16U) |
									 (((uint32) camera[5]) << 8U) |
									 ((uint32) camera[4]));
								s32[2] = (int32)
									((((uint32) camera[11]) << 24U) |
									 (((uint32) camera[10]) << 16U) |
									 (((uint32) camera[9]) << 8U) |
									 ((uint32) camera[8]));
								u16[0] = (uint16)
									((((uint16) camera[13]) << 8U) |
									 ((uint16) camera[12]));
								u16[2] = (uint16)
									((((uint16) camera[15]) << 8U) |
									 ((uint16) camera[14]));
								u16[1] = nearestRoom(level, error, s32[0],
								                     s32[1], s32[2]);
								
								/* Sets the coordinates to local coordinates */
								s32[0] -= level->room[u16[1]].x;
								s32[1] = (level->room[u16[1]].yBottom - s32[1]);
								s32[2] -= level->room[u16[1]].z;
								
								/* Prints the sink */
								trmod_printf(PRINTU16 "," PRINT32 "," PRINT32
								             "," PRINT32 "," PRINTU16 ","
								             PRINTU16 ")", u16[1], s32[0],
								             s32[2], s32[1], u16[0], u16[2]);
								if ((print & 0x0001) == 0x0000)
								{
									trmod_printf("\n");
								}
								break;
							case 0x0C00: /* Flip Map */
								if ((print & 0x0001) == 0x0000)
								{
									trmod_printf("        Flip Map(%03X)\n",
									       (trigaction & 0x03FF));
								}
								else
								{
									trmod_printf("flipmap(%03X)",
									       (trigaction & 0x03FF));
								}
								break;
							case 0x1000: /* Flip On */
								if ((print & 0x0001) == 0x0000)
								{
									trmod_printf("        Flip On(" PRINTU16
									             ")\n", (trigaction & 0x03FF));
								}
								else
								{
									trmod_printf("flipon(" PRINTU16 ")",
									             (trigaction & 0x03FF));
								}
								break;
							case 0x1400: /* Flip Off */
								if ((print & 0x0001) == 0x0000)
								{
									trmod_printf("        Flip Off(" PRINTU16
									             ")\n", (trigaction & 0x03FF));
								}
								else
								{
									trmod_printf("flipoff(" PRINTU16 ")",
									             (trigaction & 0x03FF));
								}
								break;
							case 0x1800: /* Look at Item */
								if ((print & 0x0001) == 0x0000)
								{
									trmod_printf("        Look at Item("
									             PRINTU16 ")\n",
									             (trigaction & 0x03FF));
								}
								else
								{
									trmod_printf("lookitem(" PRINTU16 ")",
									             (trigaction & 0x03FF));
								}
								break;
							case 0x1C00: /* End Level */
								if ((print & 0x0001) == 0x0000)
								{
									trmod_printf("        End Level\n");
								}
								else
								{
									trmod_printf("endlevel");
								}
								break;
							case 0x2000: /* Play Soundtrack */
								if ((print & 0x0001) == 0x0000)
								{
									trmod_printf("        Soundtrack(" PRINTU16
									             ")\n", (trigaction & 0x03FF));
								}
								else
								{
									trmod_printf("soundtrack(" PRINTU16 ")",
									             (trigaction & 0x03FF));
								}
								break;
							case 0x2400: /* Effect */
								if ((print & 0x0001) == 0x0000)
								{
									trmod_printf("        Effect(" PRINTU16
									             ")\n", (trigaction & 0x03FF));
								}
								else
								{
									trmod_printf("effect(" PRINTU16 ")",
									             (trigaction & 0x03FF));
								}
								break;
							case 0x2800: /* Secret Found */
								if ((print & 0x0001) == 0x0000)
								{
									trmod_printf("        Secret(" PRINTU16
									             ")\n", (trigaction & 0x03FF));
								}
								else
								{
									trmod_printf("secret(" PRINTU16 ")",
									             (trigaction & 0x03FF));
								}
								break;
							case 0x2C00: /* Clear Bodies */
								if ((print & 0x0001) == 0x0000)
								{
									trmod_printf("        Clear Bodies\n");
								}
								else
								{
									trmod_printf("clearbodies");
								}
								break;
							case 0x3000: /* FlyBy Camera */
								/* Reads extra flyby integer */
								seekval = trmod_fseek(level->file,
								                      offset, SEEK_SET);
								if (seekval != 0)
								{
									error->code = ERROR_FILE_READ_FAILED;
									error->string[0] = level->path;
									return;
								}
								offset += 2l;
								readval = trmod_fread(&u16[2], (size_t) 1,
								                      (size_t) 2, level->file);
								if (readval != (size_t) 2)
								{
									error->code = ERROR_FILE_READ_FAILED;
									error->string[0] = level->path;
									return;
								}
								endian16(u16[2]);
								contBit |= (u16[2] & 0x8000);
								if ((print & 0x0001) == 0x0000)
								{
									trmod_printf("        Flyby(" PRINTU16
									             ",%s)\n",
									             (trigaction & 0x03FF),
									             (((u16[2] & 0x0100) ==
									               0x0100) ? "Once" : ""));
								}
								else
								{
									trmod_printf("flyby(" PRINTU16 ",%s)",
									             (trigaction & 0x03FF),
									             (((u16[2] & 0x0100) ==
									               0x0100) ? "once" : ""));
								}
								break;
							case 0x3400: /* Cutscene */
								if ((print & 0x0001) == 0x0000)
								{
									trmod_printf("        Cutscene(" PRINTU16
									             ")\n", (trigaction & 0x03FF));
								}
								else
								{
									trmod_printf("cutscene(" PRINTU16 ")",
									             (trigaction & 0x03FF));
								}
								break;
							default:
								error->code = ERROR_UNKNOWN_TRIGGERACTION;
								error->u16[0] = ((trigaction & 0x7C00) >> 10U);
								return;
						}
						
						/* Prints a comma if another TrigAction is present */
						if ((contBit == 0x0000) &&
						    ((print & 0x0001) != 0x0000))
						{
							trmod_printf(",");
						}
					}
					while (contBit == 0x0000);
					
					/* Ends this trigger */
					if ((print & 0x0001) != 0x0000)
					{
						trmod_printf(")");
					}
					else
					{
						trmod_printf("    )\n");
					}
					break;
				case 0x0005: /* Kill Lara */
					if ((print & 0x0001) != 0x0000)
					{
						trmod_printf("kill");
					}
					else
					{
						trmod_printf("    Kill Lara\n");
					}
					break;
				case 0x0006: /* Climbable Walls */
					if ((print & 0x0001) != 0x0000)
					{
						trmod_printf("climbwalls(");
						if ((fdsetup & 0x0100) != 0x0000)
						{
							trmod_printf("z%s",
							             (((fdsetup & 0x0E00) != 0x0000) ?
							              "," : ""));
						}
						if ((fdsetup & 0x0200) != 0x0000)
						{
							trmod_printf("x%s",
							             (((fdsetup & 0x0C00) != 0x0000) ?
							              "," : ""));
						}
						if ((fdsetup & 0x0400) != 0x0000)
						{
							trmod_printf("-z%s",
							             (((fdsetup & 0x0800) != 0x0000) ?
							              "," : ""));
						}
						if ((fdsetup & 0x0800) != 0x0000)
						{
							trmod_printf("-x");
						}
						trmod_printf(")");
					}
					else
					{
						trmod_printf("    Climbable Walls(");
						if ((fdsetup & 0x0100) != 0x0000)
						{
							trmod_printf("Z%s",
							             (((fdsetup & 0x0E00) != 0x0000) ?
							              "," : ""));
						}
						if ((fdsetup & 0x0200) != 0x0000)
						{
							trmod_printf("X%s",
							             (((fdsetup & 0x0C00) != 0x0000) ?
							              "," : ""));
						}
						if ((fdsetup & 0x0400) != 0x0000)
						{
							trmod_printf("-Z%s",
							             (((fdsetup & 0x0800) != 0x0000) ?
							              "," : ""));
						}
						if ((fdsetup & 0x0800) != 0x0000)
						{
							trmod_printf("-X");
						}
						trmod_printf(")\n");
					}
					break;
				case 0x0007: /* TriFloor \\ solid */
				case 0x000B: /* TriFloor \\ leftportal */
				case 0x000C: /* TriFloor \\ rightportal */
				case 0x0008: /* TriFloor / solid */
				case 0x000D: /* TriFloor / leftportal */
				case 0x000E: /* TriFloor / rightportal */
				case 0x0009: /* TriCeiling \\ solid */
				case 0x000F: /* TriCeiling \\ leftportal */
				case 0x0010: /* TriCeiling \\ rightportal */
				case 0x000A: /* TriCeiling / solid */
				case 0x0011: /* TriCeiling / leftportal */
				case 0x0012: /* TriCeiling / rightportal */
					if (((fdsetup & 0x001F) == 0x0007) ||
					    ((fdsetup & 0x001F) == 0x000B) ||
					    ((fdsetup & 0x001F) == 0x000C))
					{
						if ((print & 0x0001) != 0x0000)
						{
							trmod_printf("trifloor(\\\\,");
						}
						else
						{
							trmod_printf("    Triangular Floor(\\,");
						}
					}
					else if (((fdsetup & 0x001F) == 0x0008) ||
					         ((fdsetup & 0x001F) == 0x000D) ||
					         ((fdsetup & 0x001F) == 0x000E))
					{
						if ((print & 0x0001) != 0x0000)
						{
							trmod_printf("trifloor(/,");
						}
						else
						{
							trmod_printf("    Triangular Floor(/,");
						}
					}
					else if (((fdsetup & 0x001F) == 0x0009) ||
					         ((fdsetup & 0x001F) == 0x000F) ||
					         ((fdsetup & 0x001F) == 0x0010))
					{
						if ((print & 0x0001) != 0x0000)
						{
							trmod_printf("triceiling(\\\\,");
						}
						else
						{
							trmod_printf("    Triangular Ceiling(\\,");
						}
					}
					else if (((fdsetup & 0x001F) == 0x000A) ||
					         ((fdsetup & 0x001F) == 0x0011) ||
					         ((fdsetup & 0x001F) == 0x0012))
					{
						if ((print & 0x0001) != 0x0000)
						{
							trmod_printf("triceiling(/,");
						}
						else
						{
							trmod_printf("    Triangular Ceiling(/,");
						}
					}
					
					/* Prints the portal part */
					if (((fdsetup & 0x001F) == 0x0007) ||
					    ((fdsetup & 0x001F) == 0x0008) ||
					    ((fdsetup & 0x001F) == 0x0009) ||
					    ((fdsetup & 0x001F) == 0x000A))
					{
						if ((print & 0x0001) != 0x0000)
						{
							trmod_printf(",");
						}
						else
						{
							trmod_printf("-,");
						}
					}
					else if (((fdsetup & 0x001F) == 0x000B) ||
					         ((fdsetup & 0x001F) == 0x000D) ||
					         ((fdsetup & 0x001F) == 0x000F) ||
					         ((fdsetup & 0x001F) == 0x0011))
					{
						if ((print & 0x0001) != 0x0000)
						{
							trmod_printf("l,");
						}
						else
						{
							trmod_printf("L,");
						}
					}
					else if (((fdsetup & 0x001F) == 0x000C) ||
					         ((fdsetup & 0x001F) == 0x000E) ||
					         ((fdsetup & 0x001F) == 0x0010) ||
					         ((fdsetup & 0x001F) == 0x0012))
					{
						if ((print & 0x0001) != 0x0000)
						{
							trmod_printf("r,");
						}
						else
						{
							trmod_printf("R,");
						}
					}
					
					/* Prints H1 and H2 */
					s16[0] = (int16) ((fdsetup & 0x03E0) >> 5U);
					if ((fdsetup & 0x0200) != 0x0000)
					{
						s16[0] |= 0xFFE0;
					}
					trmod_printf(PRINT16 ",", s16[0]);
					s16[0] = (int16) ((fdsetup & 0x7C00) >> 10U);
					if ((fdsetup & 0x4000) != 0x0000)
					{
						s16[0] |= 0xFFE0;
					}
					trmod_printf(PRINT16 ",", s16[0]);
					
					/* Reads the next integer into memory */
					seekval = trmod_fseek(level->file, offset, SEEK_SET);
					if (seekval != 0)
					{
						error->code = ERROR_FILE_READ_FAILED;
						error->string[0] = level->path;
						return;
					}
					offset += 2l;
					readval = trmod_fread(u16, (size_t) 1,
					                      (size_t) 2, level->file);
					if (readval != (size_t) 2)
					{
						error->code = ERROR_FILE_READ_FAILED;
						error->string[0] = level->path;
						return;
					}
					endian16(u16[0]);
					
					/* Prints the C-values */
					trmod_printf(PRINTU16 "," PRINTU16 ","
					             PRINTU16 "," PRINTU16 ")",
					             (u16[0] & 0x000F), ((u16[0] & 0x00F0) >> 4U),
					             ((u16[0] & 0x0F00) >> 8U),
					             ((u16[0] & 0xF000) >> 12U));
					if ((print & 0x0001) == 0x0000)
					{
						trmod_printf("\n");
					}
					break;
				case 0x0013: /* Monkeyswing */
					if ((print & 0x0001) != 0x0000)
					{
						trmod_printf("monkeyswing");
					}
					else
					{
						trmod_printf("    Monkeyswing\n");
					}
					break;
				case 0x0014: /* Function 14 */
					if ((print & 0x0001) != 0x0000)
					{
						trmod_printf("f14");
					}
					else
					{
						trmod_printf("    Function 14\n");
					}
					break;
				case 0x00015: /* Function 15 */
					if ((print & 0x0001) != 0x0000)
					{
						trmod_printf("f15");
					}
					else
					{
						trmod_printf("    Function 15\n");
					}
					break;
				default:
					error->code = ERROR_UNKNOWN_FDFUNCTION;
					error->u16[0] = (fdsetup & 0x001F);
					return;
			}
			
			/* Prints a comma if another FloorData-entry is present */
			if ((endData == 0x0000) && ((print & 0x0001) != 0x0000))
			{
				trmod_printf(",");
			}
		}
		while (endData == 0x0000);
	}
	
	/* Ends the printing entry */
	trmod_printf(")%s\n", (((print & 0x0001) == 0x0001) ? "\"" : ""));
}

/*
 * Function that replaces floordata in a given sector
 * Parameters:
 * * level = Pointer to level struct
 * * error = Pointer to error struct
 * * room = Room number
 * * column = Column number
 * * row = Row number
 * * string = String version of the floordata entry
 * * secpad = Padding between NumXSectors and the actual sectors (usually 0)
 * * flags = Flags relating to how to replace floordata:
 * * * 0 0 0 0  0 0 0 0   0 0 0 0  0 0 0 0
 * * * | | | |  | | | |   | | | |  | | | |
 * * * | | | |  | | | |   | | | |  | | | Optimise
 * * * Unused
 */
void tr1pc_replace_floordata(struct level *level, struct error *error,
                             uint16 room, uint16 column, uint16 row,
                             char *string, uint32 secpad, uint16 flags)
{
	/* Variable Declarations */
	uint8 camera[16];               /* Space to make a camera */
	uint16 *newfdentry = NULL;      /* New FloorData entry */
	uint16 *floordata = NULL;       /* Copy of existing FloorData */
	uint8 roombelow = (uint8) 0xFF; /* RoomBelow for this sector */
	uint8 roomabove = (uint8) 0xFF; /* RoomAbove for this sector */
	uint32 numFD = 0x00000000;      /* Copy of NumFloorData */
	uint16 outlength = 0x0000;      /* Length of the newfdentry */
	uint16 curpos = 0x0000;         /* Current position in FloorData */
	uint16 endpos = 0x0000;         /* Final position in FloorData */
	uint16 enddatpos = 0x0000;      /* Byte to place the EndData-bit in */
	uint16 contbitpos = 0x0000;     /* Byte to place the ContBit-bit in */
	char **fdfunc = NULL;           /* Array of FDFunctions */
	uint16 numFDFuncs = 0x0000;     /* Number of FDFunctions */
	uint16 curFDFunc = 0x0000;      /* Current FDFunction */
	char **trigact = NULL;          /* Array of Trigger Actions */
	uint16 numtrigact = 0x0000;     /* Number of Trigger Actions */
	uint16 curtrigact = 0x0000;     /* Current Trigger Action */
	char **subparam = NULL;         /* Array of subparameters */
	uint16 numSubparams = 0x0000;   /* Number of subparameters */
	uint16 curSubparam = 0x0000;    /* Current subparameter */
	int32 s32[3];                   /* Signed 32-bit integers for input */
	int16 s16[2];                   /* Signed 16-bit integers for input */
	uint16 u16[4];                  /* Unsigned 16-bit integers for input */
	long int writepos;              /* Position to write to */
	int seekval;                    /* Return value of fseek() */
	size_t readval;                 /* Return value of fread() */
	size_t writeval;                /* Return value of fwrite() */
	
	/* Checks that this sector exists */
	if (room >= level->numRooms)
	{
		error->code = ERROR_ROOM_DOESNT_EXIST;
		error->u16[0] = room;
		goto end;
	}
	if ((column >= level->room[room].value[R_NUMXSECTORS]) ||
	    (row >= level->room[room].value[R_NUMZSECTORS]))
	{
		error->code = ERROR_SECTOR_DOESNT_EXIST;
		error->u16[0] = room;
		error->u16[1] = column;
		error->u16[2] = row;
		goto end;
	}
	
	/* Allocates space for the floordata entry */
	newfdentry = calloc(strlen(string), (size_t) 2);
	if (newfdentry == NULL)
	{
		error->code = ERROR_MEMORY;
		goto end;
	}
	
	/* Separates the parameters */
	numFDFuncs = splitString(string, &fdfunc, (uint16) strlen(string),
	                         ',', 1, 1, error);
	if (error->code != ERROR_NONE)
	{
		goto end;
	}
	
	/* Loops through the FloorData Functions */
	for (curFDFunc = 0x0000; curFDFunc < numFDFuncs; ++curFDFunc)
	{
		if (compareString(fdfunc[curFDFunc], "floorportal", 11) == 0)
		{
			if (strlen(fdfunc[curFDFunc]) > (size_t) 12)
			{
				roombelow = (uint8) strtol(&fdfunc[curFDFunc][12], NULL, 10);
			}
		}
		else if (compareString(fdfunc[curFDFunc], "ceilingportal", 13) == 0)
		{
			if (strlen(fdfunc[curFDFunc]) > (size_t) 14)
			{
				roomabove = (uint8) strtol(&fdfunc[curFDFunc][14], NULL, 10);
			}
		}
		else if (compareString(fdfunc[curFDFunc], "portal", 6) == 0)
		{
			if (strlen(fdfunc[curFDFunc]) <= (size_t) 7)
			{
				error->code = ERROR_INVALID_SYNTAX;
				goto end;
			}
			enddatpos = outlength;
			newfdentry[outlength++] = 0x0001;
			newfdentry[outlength++] = (uint16) strtol(&fdfunc[curFDFunc][7],
			                                          NULL, 10);
		}
		else if (compareString(fdfunc[curFDFunc], "floorslant", 10) == 0)
		{
			if (strlen(fdfunc[curFDFunc]) <= (size_t) 11)
			{
				error->code = ERROR_INVALID_SYNTAX;
				goto end;
			}
			
			/* Separates the parameters */
			fdfunc[curFDFunc] = &fdfunc[curFDFunc][11];
			numSubparams = splitString(fdfunc[curFDFunc], &subparam,
			                           (uint16) strlen(fdfunc[curFDFunc]),
			                           ',', 1, 1, error);
			if (error->code != ERROR_NONE)
			{
				goto end;
			}
			if (numSubparams != 0x0002)
			{
				error->code = ERROR_INVALID_SYNTAX;
				goto end;
			}
			
			/* Reads the two angles */
			s16[0] = (int16) strtol(subparam[0], NULL, 10);
			s16[1] = (int16) strtol(subparam[1], NULL, 10);
			
			/* Adds the FDFunction */
			enddatpos = outlength;
			newfdentry[outlength++] = 0x0002;
			newfdentry[outlength++] = (uint16) (((s16[1] & 0x00FF) << 8U) |
			                                    (s16[0] & 0x00FF));
		}
		else if (compareString(fdfunc[curFDFunc], "ceilingslant", 12) == 0)
		{
			if (strlen(fdfunc[curFDFunc]) <= (size_t) 13)
			{
				error->code = ERROR_INVALID_SYNTAX;
				goto end;
			}
			
			/* Separates the parameters */
			fdfunc[curFDFunc] = &fdfunc[curFDFunc][13];
			numSubparams = splitString(fdfunc[curFDFunc], &subparam,
			                           (uint16) strlen(fdfunc[curFDFunc]),
			                           ',', 1, 1, error);
			if (error->code != ERROR_NONE)
			{
				goto end;
			}
			if (numSubparams != 0x0002)
			{
				error->code = ERROR_INVALID_SYNTAX;
				goto end;
			}
			
			/* Reads the two angles */
			s16[0] = (int16) strtol(subparam[1], NULL, 10);
			s16[1] = (int16) strtol(subparam[0], NULL, 10);
			
			/* Adds the FDFunction */
			enddatpos = outlength;
			newfdentry[outlength++] = 0x0003;
			newfdentry[outlength++] = (uint16) (((s16[0] & 0x00FF) << 8U) |
			                                    (s16[1] & 0x00FF));
		}
		else if (compareString(fdfunc[curFDFunc], "trigger", 7) == 0)
		{
			if (strlen(fdfunc[curFDFunc]) <= (size_t) 8)
			{
				error->code = ERROR_INVALID_SYNTAX;
				goto end;
			}
			
			/* Separates the trigger setup from trigger actions */
			fdfunc[curFDFunc] = &fdfunc[curFDFunc][8];
			numSubparams = splitString(fdfunc[curFDFunc], &subparam,
			                           (uint16) strlen(fdfunc[curFDFunc]),
			                           ':', 1, 1, error);
			if (error->code != ERROR_NONE)
			{
				goto end;
			}
			if (numSubparams != 0x0002)
			{
				error->code = ERROR_INVALID_SYNTAX;
				goto end;
			}
			
			/* Separates the trigger actions from each other */
			numtrigact = splitString(subparam[1], &trigact,
			                             (uint16) strlen(subparam[1]),
			                             ',', 1, 1, error);
			if (error->code != ERROR_NONE)
			{
				goto end;
			}
			
			/* Splits the elements of the trigger setup */
			fdfunc[curFDFunc] = subparam[0];
			numSubparams = splitString(fdfunc[curFDFunc], &subparam,
			                           (uint16) strlen(fdfunc[curFDFunc]),
			                           ',', 1, 1, error);
			if (error->code != ERROR_NONE)
			{
				goto end;
			}
			if (numSubparams < 0x0004)
			{
				error->code = ERROR_INVALID_SYNTAX;
				goto end;
			}
			
			/* Interprets Trigger Type */
			if (compareString(subparam[0], "trigger", 7) == 0)
			{
				enddatpos = outlength;
				newfdentry[outlength++] = 0x0004;
			}
			else if (compareString(subparam[0], "pad", 3) == 0)
			{
				enddatpos = outlength;
				newfdentry[outlength++] = 0x0104;
			}
			else if (compareString(subparam[0], "switch", 6) == 0)
			{
				enddatpos = outlength;
				newfdentry[outlength++] = 0x0204;
			}
			else if (compareString(subparam[0], "key", 3) == 0)
			{
				enddatpos = outlength;
				newfdentry[outlength++] = 0x0304;
			}
			else if (compareString(subparam[0], "pickup", 6) == 0)
			{
				enddatpos = outlength;
				newfdentry[outlength++] = 0x0404;
			}
			else if (compareString(subparam[0], "heavytrigger", 12) == 0)
			{
				enddatpos = outlength;
				newfdentry[outlength++] = 0x0504;
			}
			else if (compareString(subparam[0], "antipad", 7) == 0)
			{
				enddatpos = outlength;
				newfdentry[outlength++] = 0x0604;
			}
			else if (compareString(subparam[0], "combat", 6) == 0)
			{
				enddatpos = outlength;
				newfdentry[outlength++] = 0x0704;
			}
			else if (compareString(subparam[0], "dummy", 5) == 0)
			{
				enddatpos = outlength;
				newfdentry[outlength++] = 0x0804;
			}
			else if (compareString(subparam[0], "antitrigger", 11) == 0)
			{
				enddatpos = outlength;
				newfdentry[outlength++] = 0x0904;
			}
			else if (compareString(subparam[0], "heavyswitch", 11) == 0)
			{
				enddatpos = outlength;
				newfdentry[outlength++] = 0x0A04;
			}
			else if (compareString(subparam[0], "heavyantitrigger", 16) == 0)
			{
				enddatpos = outlength;
				newfdentry[outlength++] = 0x0B04;
			}
			else if (compareString(subparam[0], "monkey", 6) == 0)
			{
				enddatpos = outlength;
				newfdentry[outlength++] = 0x0C04;
			}
			else if (compareString(subparam[0], "skeleton", 8) == 0)
			{
				enddatpos = outlength;
				newfdentry[outlength++] = 0x0D04;
			}
			else if (compareString(subparam[0], "tightrope", 9) == 0)
			{
				enddatpos = outlength;
				newfdentry[outlength++] = 0x0E04;
			}
			else if (compareString(subparam[0], "crawl", 5) == 0)
			{
				enddatpos = outlength;
				newfdentry[outlength++] = 0x0F04;
			}
			else if (compareString(subparam[0], "climb", 5) == 0)
			{
				enddatpos = outlength;
				newfdentry[outlength++] = 0x1004;
			}
			else
			{
				error->code = ERROR_INVALID_TRIGTYPE;
				error->string[0] = subparam[0];
				goto end;
			}
			
			/* Adds timer, oneshot and mask */
			newfdentry[outlength] = (uint16) strtol(subparam[3], NULL, 16);
			newfdentry[outlength] &= 0x001F;
			newfdentry[outlength] <<= 9U;
			newfdentry[outlength] |= (uint16) ((strtol(subparam[1], NULL, 10)) &
			                                   0x00FF);
			if (compareString(subparam[2], "o", 1) == 0)
			{
				newfdentry[outlength] |= 0x0100;
			}
			++outlength;
			
			/* Adds trigger type object (if needed) */
			if ((compareString(subparam[0], "switch", 6) == 0) ||
			    (compareString(subparam[0], "key", 3) == 0)    ||
			    (compareString(subparam[0], "pickup", 6) == 0))
			{
				u16[0] = charindex(subparam[0], (uint16) strlen(subparam[0]),
				                   '(', 1);
				if (u16[0] == 0xFFFF)
				{
					error->code = ERROR_INVALID_SYNTAX;
					goto end;
				}
				++u16[0];
				newfdentry[outlength] = (uint16) strtol(&subparam[0][u16[0]],
				                                        NULL, 10);
				newfdentry[outlength] &= 0x03FF;
				++outlength;
			}
			
			/* Processes trigger actions */
			for (curtrigact = 0x0000; curtrigact < numtrigact; ++curtrigact)
			{
				if (compareString(trigact[curtrigact], "object", 6) == 0)
				{
					if (strlen(trigact[curtrigact]) < (size_t) 7)
					{
						error->code = ERROR_INVALID_SYNTAX;
						goto end;
					}
					trigact[curtrigact] = &trigact[curtrigact][7];
					contbitpos = outlength;
					newfdentry[outlength] = (uint16) strtol(trigact[curtrigact],
					                                        NULL, 10);
					newfdentry[outlength] &= 0x03FF;
					++outlength;
				}
				else if (compareString(trigact[curtrigact], "camera", 6) == 0)
				{
					if (strlen(trigact[curtrigact]) < (size_t) 7)
					{
						error->code = ERROR_INVALID_SYNTAX;
						goto end;
					}
					trigact[curtrigact] = &trigact[curtrigact][7];
					
					/* Splits the parameters */
					numSubparams = splitString(trigact[curtrigact], &subparam,
					                           (uint16)
					                           strlen(trigact[curtrigact]),
					                           ',', 1, 1, error);
					if (error->code != ERROR_NONE)
					{
						goto end;
					}
					if (numSubparams < 0x0008)
					{
						error->code = ERROR_INVALID_SYNTAX;
						goto end;
					}
					
					/* Gets the parameters from the input */
					u16[0] = (uint16) strtol(subparam[0], NULL, 10);
					s32[0] = (int32) strtol(subparam[1], NULL, 10);
					s32[1] = (int32) strtol(subparam[3], NULL, 10);
					s32[2] = (int32) strtol(subparam[2], NULL, 10);
					u16[1] = (uint16) strtol(subparam[4], NULL, 10);
					u16[2] = (uint16) strtol(subparam[5], NULL, 10);
					u16[3] = (uint16) strtol(subparam[7], NULL, 10);
					u16[1] &= 0x00FF;
					u16[2] &= 0x001F;
					u16[2] <<= 9U;
					
					/* Sets the coordinates to world coordinates */
					s32[0] += level->room[u16[0]].x;
					s32[1] = (level->room[u16[0]].yBottom - s32[1]);
					s32[2] += level->room[u16[0]].z;
					
					/* Puts the camera together in memory */
					camera[0] = (uint8) (s32[0] & 0x000000FF);
					camera[1] = (uint8) ((s32[0] & 0x0000FF00) >> 8U);
					camera[2] = (uint8) ((s32[0] & 0x00FF0000) >> 16U);
					camera[3] = (uint8) ((s32[0] & 0xFF000000) >> 24U);
					camera[4] = (uint8) (s32[1] & 0x000000FF);
					camera[5] = (uint8) ((s32[1] & 0x0000FF00) >> 8U);
					camera[6] = (uint8) ((s32[1] & 0x00FF0000) >> 16U);
					camera[7] = (uint8) ((s32[1] & 0xFF000000) >> 24U);
					camera[8] = (uint8) (s32[2] & 0x000000FF);
					camera[9] = (uint8) ((s32[2] & 0x0000FF00) >> 8U);
					camera[10] = (uint8) ((s32[2] & 0x00FF0000) >> 16U);
					camera[11] = (uint8) ((s32[2] & 0xFF000000) >> 24U);
					camera[12] = (uint8) (u16[0] & 0x00FF);
					camera[13] = (uint8) ((u16[0] & 0xFF00) >> 8U);
					camera[14] = (uint8) (u16[3] & 0x00FF);
					camera[15] = (uint8) ((u16[3] & 0xFF00) >> 8U);
					
					/* Adds the camera if needed */
					u16[0] = addcamera(level, error, camera);
					if (error->code != ERROR_NONE)
					{
						goto end;
					}
					
					/* Adds the trigger action */
					newfdentry[outlength] = u16[0];
					newfdentry[outlength] &= 0x03FF;
					newfdentry[outlength] |= 0x0400;
					++outlength;
					contbitpos = outlength;
					newfdentry[outlength] = u16[1];
					newfdentry[outlength] |= u16[2];
					if (compareString(subparam[6], "o", 1) == 0)
					{
						newfdentry[outlength] |= 0x0100;
					}
					++outlength;
				}
				else if (compareString(trigact[curtrigact], "current", 7) == 0)
				{
					if (strlen(trigact[curtrigact]) < (size_t) 8)
					{
						error->code = ERROR_INVALID_SYNTAX;
						goto end;
					}
					trigact[curtrigact] = &trigact[curtrigact][8];
					
					/* Splits the parameters */
					numSubparams = splitString(trigact[curtrigact], &subparam,
					                           (uint16)
					                           strlen(trigact[curtrigact]),
					                           ',', 1, 1, error);
					if (error->code != ERROR_NONE)
					{
						goto end;
					}
					if (numSubparams < 0x0006)
					{
						error->code = ERROR_INVALID_SYNTAX;
						goto end;
					}
					
					/* Gets the parameters from the input */
					u16[0] = (uint16) strtol(subparam[0], NULL, 10);
					s32[0] = (int32) strtol(subparam[1], NULL, 10);
					s32[1] = (int32) strtol(subparam[3], NULL, 10);
					s32[2] = (int32) strtol(subparam[2], NULL, 10);
					u16[1] = (uint16) strtol(subparam[4], NULL, 10);
					u16[2] = (uint16) strtol(subparam[5], NULL, 10);
					
					/* Sets the coordinates to world coordinates */
					s32[0] += level->room[u16[0]].x;
					s32[1] = (level->room[u16[0]].yBottom - s32[1]);
					s32[2] += level->room[u16[0]].z;
					
					/* Puts the sink together in memory */
					camera[0] = (uint8) (s32[0] & 0x000000FF);
					camera[1] = (uint8) ((s32[0] & 0x0000FF00) >> 8U);
					camera[2] = (uint8) ((s32[0] & 0x00FF0000) >> 16U);
					camera[3] = (uint8) ((s32[0] & 0xFF000000) >> 24U);
					camera[4] = (uint8) (s32[1] & 0x000000FF);
					camera[5] = (uint8) ((s32[1] & 0x0000FF00) >> 8U);
					camera[6] = (uint8) ((s32[1] & 0x00FF0000) >> 16U);
					camera[7] = (uint8) ((s32[1] & 0xFF000000) >> 24U);
					camera[8] = (uint8) (s32[2] & 0x000000FF);
					camera[9] = (uint8) ((s32[2] & 0x0000FF00) >> 8U);
					camera[10] = (uint8) ((s32[2] & 0x00FF0000) >> 16U);
					camera[11] = (uint8) ((s32[2] & 0xFF000000) >> 24U);
					camera[12] = (uint8) (u16[1] & 0x00FF);
					camera[13] = (uint8) ((u16[1] & 0xFF00) >> 8U);
					camera[14] = (uint8) (u16[2] & 0x00FF);
					camera[15] = (uint8) ((u16[2] & 0xFF00) >> 8U);
					
					/* Adds the sink if needed */
					u16[0] = addcamera(level, error, camera);
					if (error->code != ERROR_NONE)
					{
						goto end;
					}
					
					/* Adds the trigger action */
					contbitpos = outlength;
					newfdentry[outlength] = u16[0];
					newfdentry[outlength] &= 0x03FF;
					newfdentry[outlength] |= 0x0800;
					++outlength;
				}
				else if (compareString(trigact[curtrigact], "flipmap", 7) == 0)
				{
					if (strlen(trigact[curtrigact]) < (size_t) 8)
					{
						error->code = ERROR_INVALID_SYNTAX;
						goto end;
					}
					trigact[curtrigact] = &trigact[curtrigact][8];
					contbitpos = outlength;
					newfdentry[outlength] = (uint16) strtol(trigact[curtrigact],
					                                        NULL, 16);
					newfdentry[outlength] &= 0x03FF;
					newfdentry[outlength] |= 0x0C00;
					++outlength;
				}
				else if (compareString(trigact[curtrigact], "flipon", 6) == 0)
				{
					if (strlen(trigact[curtrigact]) < (size_t) 7)
					{
						error->code = ERROR_INVALID_SYNTAX;
						goto end;
					}
					trigact[curtrigact] = &trigact[curtrigact][7];
					contbitpos = outlength;
					newfdentry[outlength] = (uint16) strtol(trigact[curtrigact],
					                                        NULL, 10);
					newfdentry[outlength] &= 0x03FF;
					newfdentry[outlength] |= 0x1000;
					++outlength;
				}
				else if (compareString(trigact[curtrigact], "flipoff", 7) == 0)
				{
					if (strlen(trigact[curtrigact]) < (size_t) 8)
					{
						error->code = ERROR_INVALID_SYNTAX;
						goto end;
					}
					trigact[curtrigact] = &trigact[curtrigact][8];
					contbitpos = outlength;
					newfdentry[outlength] = (uint16) strtol(trigact[curtrigact],
					                                        NULL, 10);
					newfdentry[outlength] &= 0x03FF;
					newfdentry[outlength] |= 0x1400;
					++outlength;
				}
				else if (compareString(trigact[curtrigact], "lookite", 7) == 0)
				{
					if (strlen(trigact[curtrigact]) < (size_t) 9)
					{
						error->code = ERROR_INVALID_SYNTAX;
						goto end;
					}
					trigact[curtrigact] = &trigact[curtrigact][9];
					contbitpos = outlength;
					newfdentry[outlength] = (uint16) strtol(trigact[curtrigact],
					                                        NULL, 10);
					newfdentry[outlength] &= 0x03FF;
					newfdentry[outlength] |= 0x1800;
					++outlength;
				}
				else if (compareString(trigact[curtrigact], "endleve", 7) == 0)
				{
					contbitpos = outlength;
					newfdentry[outlength] = 0x1C00;
					++outlength;
				}
				else if (compareString(trigact[curtrigact], "soundtr", 7) == 0)
				{
					if (strlen(trigact[curtrigact]) < (size_t) 11)
					{
						error->code = ERROR_INVALID_SYNTAX;
						goto end;
					}
					trigact[curtrigact] = &trigact[curtrigact][11];
					contbitpos = outlength;
					newfdentry[outlength] = (uint16) strtol(trigact[curtrigact],
					                                        NULL, 10);
					newfdentry[outlength] &= 0x03FF;
					newfdentry[outlength] |= 0x2000;
					++outlength;
				}
				else if (compareString(trigact[curtrigact], "effect", 6) == 0)
				{
					if (strlen(trigact[curtrigact]) < (size_t) 7)
					{
						error->code = ERROR_INVALID_SYNTAX;
						goto end;
					}
					trigact[curtrigact] = &trigact[curtrigact][7];
					contbitpos = outlength;
					newfdentry[outlength] = (uint16) strtol(trigact[curtrigact],
					                                        NULL, 10);
					newfdentry[outlength] &= 0x03FF;
					newfdentry[outlength] |= 0x2400;
					++outlength;
				}
				else if (compareString(trigact[curtrigact], "secret", 6) == 0)
				{
					if (strlen(trigact[curtrigact]) < (size_t) 7)
					{
						error->code = ERROR_INVALID_SYNTAX;
						goto end;
					}
					trigact[curtrigact] = &trigact[curtrigact][7];
					contbitpos = outlength;
					newfdentry[outlength] = (uint16) strtol(trigact[curtrigact],
					                                        NULL, 10);
					newfdentry[outlength] &= 0x03FF;
					newfdentry[outlength] |= 0x2800;
					++outlength;
				}
				else if (compareString(trigact[curtrigact], "clearbo", 7) == 0)
				{
					contbitpos = outlength;
					newfdentry[outlength] = 0x2C00;
					++outlength;
				}
				else if (compareString(trigact[curtrigact], "flyby", 5) == 0)
				{
					if (strlen(trigact[curtrigact]) < (size_t) 6)
					{
						error->code = ERROR_INVALID_SYNTAX;
						goto end;
					}
					trigact[curtrigact] = &trigact[curtrigact][6];
					
					/* Splits the subparameters */
					numSubparams = splitString(trigact[curtrigact], &subparam,
					                           (uint16)
					                           strlen(trigact[curtrigact]),
					                           ',', 1, 1, error);
					if (error->code != ERROR_NONE)
					{
						goto end;
					}
					if (numSubparams < 0x0002)
					{
						error->code = ERROR_INVALID_SYNTAX;
						goto end;
					}
					
					/* Adds the trigger action */
					newfdentry[outlength] = (uint16) strtol(subparam[0],
					                                        NULL, 10);
					newfdentry[outlength] &= 0x03FF;
					newfdentry[outlength] |= 0x3000;
					++outlength;
					contbitpos = outlength;
					if (compareString(subparam[1], "o", 1) == 0)
					{
						newfdentry[outlength] = 0x0100;
					}
					else
					{
						newfdentry[outlength] = 0x0000;
					}
					++outlength;
				}
				else if (compareString(trigact[curtrigact], "cutscen", 7) == 0)
				{
					if (strlen(trigact[curtrigact]) < (size_t) 9)
					{
						error->code = ERROR_INVALID_SYNTAX;
						goto end;
					}
					trigact[curtrigact] = &trigact[curtrigact][9];
					contbitpos = outlength;
					newfdentry[outlength] = (uint16) strtol(trigact[curtrigact],
					                                        NULL, 10);
					newfdentry[outlength] &= 0x03FF;
					newfdentry[outlength] |= 0x3400;
					++outlength;
				}
				else
				{
					error->code = ERROR_INVALID_TRIGACTION;
					error->string[0] = trigact[curtrigact];
					goto end;
				}
			}
			
			/* Sets ContBit */
			newfdentry[contbitpos] |= 0x8000;
		}
		else if (compareString(fdfunc[curFDFunc], "kill", 4) == 0)
		{
			enddatpos = outlength;
			newfdentry[outlength++] = 0x0005;
		}
		else if (compareString(fdfunc[curFDFunc], "climbwalls", 10) == 0)
		{
			if (strlen(fdfunc[curFDFunc]) <= (size_t) 11)
			{
				error->code = ERROR_INVALID_SYNTAX;
				goto end;
			}
			/* Separates the parameters */
			fdfunc[curFDFunc] = &fdfunc[curFDFunc][11];
			numSubparams = splitString(fdfunc[curFDFunc], &subparam,
			                           (uint16) strlen(fdfunc[curFDFunc]),
			                           ',', 1, 1, error);
			if (error->code != ERROR_NONE)
			{
				goto end;
			}
			
			/* Interprets the directions */
			enddatpos = outlength;
			newfdentry[outlength] = 0x0006;
			for (curSubparam = 0x0000; curSubparam < numSubparams;
			     ++curSubparam)
			{
				if ((compareString(subparam[curSubparam], "z", 1) == 0) ||
				    (compareString(subparam[curSubparam], "+z", 2) == 0))
				{
					newfdentry[outlength] |= 0x0100;
				}
				else if ((compareString(subparam[curSubparam], "x", 1) == 0) ||
				         (compareString(subparam[curSubparam], "+x", 2) == 0))
				{
					newfdentry[outlength] |= 0x0200;
				}
				else if (compareString(subparam[curSubparam], "-z", 2) == 0)
				{
					newfdentry[outlength] |= 0x0400;
				}
				else if (compareString(subparam[curSubparam], "-x", 2) == 0)
				{
					newfdentry[outlength] |= 0x0800;
				}
			}
			++outlength;
		}
		else if ((compareString(fdfunc[curFDFunc], "trifloor", 8) == 0) ||
		         (compareString(fdfunc[curFDFunc], "triceiling", 10) == 0))
		{
			if (strlen(fdfunc[curFDFunc]) <= (size_t) 11)
			{
				error->code = ERROR_INVALID_SYNTAX;
				goto end;
			}
			
			/* Separates the parameters */
			u16[0] = charindex(fdfunc[curFDFunc], (uint16)
			                   strlen(fdfunc[curFDFunc]), '(', 1);
			if (u16[0] == 0xFFFF)
			{
				error->code = ERROR_INVALID_SYNTAX;
				goto end;
			}
			++u16[0];
			numSubparams = splitString(&(fdfunc[curFDFunc][u16[0]]), &subparam,
			                           (uint16)
			                           strlen(&(fdfunc[curFDFunc][u16[0]])),
			                           ',', 1, 1, error);
			if (error->code != ERROR_NONE)
			{
				goto end;
			}
			if (numSubparams < 0x0008)
			{
				error->code = ERROR_INVALID_SYNTAX;
				goto end;
			}
			
			/* Determines the FDFunction number */
			enddatpos = outlength;
			if (compareString(fdfunc[curFDFunc], "trifloor", 8) == 0)
			{
				if (compareString(subparam[0], "\\", 1) == 0)
				{
					if (compareString(subparam[1], "l", 1) == 0)
					{
						newfdentry[outlength] = 0x000B;
					}
					else if (compareString(subparam[1], "r", 1) == 0)
					{
						newfdentry[outlength] = 0x000C;
					}
					else
					{
						newfdentry[outlength] = 0x0007;
					}
				}
				else if (compareString(subparam[0], "/", 1) == 0)
				{
					if (compareString(subparam[1], "l", 1) == 0)
					{
						newfdentry[outlength] = 0x000D;
					}
					else if (compareString(subparam[1], "r", 1) == 0)
					{
						newfdentry[outlength] = 0x000E;
					}
					else
					{
						newfdentry[outlength] = 0x0008;
					}
				}
				else
				{
					error->code = ERROR_INVALID_SYNTAX;
					goto end;
				}
			}
			else
			{
				if (compareString(subparam[0], "\\", 1) == 0)
				{
					if (compareString(subparam[1], "l", 1) == 0)
					{
						newfdentry[outlength] = 0x000F;
					}
					else if (compareString(subparam[1], "r", 1) == 0)
					{
						newfdentry[outlength] = 0x0010;
					}
					else
					{
						newfdentry[outlength] = 0x0009;
					}
				}
				else if (compareString(subparam[0], "/", 1) == 0)
				{
					if (compareString(subparam[1], "l", 1) == 0)
					{
						newfdentry[outlength] = 0x0011;
					}
					else if (compareString(subparam[1], "r", 1) == 0)
					{
						newfdentry[outlength] = 0x0012;
					}
					else
					{
						newfdentry[outlength] = 0x000A;
					}
				}
				else
				{
					error->code = ERROR_INVALID_SYNTAX;
					goto end;
				}
			}
			
			/* Adds H1 and H2 */
			s16[0] = (int16) strtol(subparam[2], NULL, 10);
			s16[1] = (int16) strtol(subparam[3], NULL, 10);
			s16[0] &= 0x001F;
			s16[1] &= 0x001F;
			s16[0] <<= 5U;
			s16[1] <<= 10U;
			newfdentry[outlength] |= (uint16) s16[0];
			newfdentry[outlength] |= (uint16) s16[1];
			++outlength;
			
			/* Adds the C-values */
			u16[0] = (uint16) strtol(subparam[4], NULL, 10);
			u16[1] = (uint16) strtol(subparam[5], NULL, 10);
			u16[2] = (uint16) strtol(subparam[6], NULL, 10);
			u16[3] = (uint16) strtol(subparam[7], NULL, 10);
			u16[0] &= 0x000F;
			u16[1] &= 0x000F;
			u16[2] &= 0x000F;
			u16[3] &= 0x000F;
			u16[1] <<= 4U;
			u16[2] <<= 8U;
			u16[3] <<= 12U;
			newfdentry[outlength] = u16[0];
			newfdentry[outlength] |= u16[1];
			newfdentry[outlength] |= u16[2];
			newfdentry[outlength] |= u16[3];
			++outlength;
		}
		else if (compareString(fdfunc[curFDFunc], "monkeyswing", 11) == 0)
		{
			enddatpos = outlength;
			newfdentry[outlength++] = 0x0013;
		}
		else if (compareString(fdfunc[curFDFunc], "f14", 3) == 0)
		{
			enddatpos = outlength;
			newfdentry[outlength++] = 0x0014;
		}
		else if (compareString(fdfunc[curFDFunc], "f15", 3) == 0)
		{
			enddatpos = outlength;
			newfdentry[outlength++] = 0x0015;
		}
		else
		{
			error->code = ERROR_INVALID_FDFUNCTION;
			error->string[0] = fdfunc[curFDFunc];
			goto end;
		}
	}
	
	/* Adds FloorData Entry */
	if (outlength > 0x0000)
	{
		/* Sets EndData */
		newfdentry[enddatpos] |= 0x8000;
		
		/* Adjusts for Big-Endian if needed */
		if (level->endian == TRUE)
		{
			for (curpos = 0x0000; curpos < outlength; ++curpos)
			{
				newfdentry[curpos] = reverse16(newfdentry[curpos]);
			}
		}
		
		/* Converts outlength into bytes */
		outlength *= 0x0002;
		
		/* Optimises this floordata entry if needed */
		if (((flags & 0x0001) != 0x0000) &&
		    (outlength < (uint16) level->value[NUMFLOORDATA]))
		{
			/* Reads the existing floordata into memory */
			floordata = malloc((size_t)
			                   (level->value[NUMFLOORDATA] * 0x00000002));
			if (floordata == NULL)
			{
				error->code = ERROR_MEMORY;
				goto end;
			}
			seekval = trmod_fseek(level->file,
			                      (long int) (level->offset[NUMFLOORDATA] +
			                                  0x00000004), SEEK_SET);
			if (seekval != 0)
			{
				error->code = ERROR_FILE_READ_FAILED;
				error->string[0] = level->path;
				goto end;
			}
			readval = trmod_fread(floordata, (size_t) 1, (size_t)
			                      (level->value[NUMFLOORDATA] * 0x00000002),
			                      level->file);
			if (readval != (size_t) (level->value[NUMFLOORDATA] * 0x00000002))
			{
				error->code = ERROR_FILE_READ_FAILED;
				error->string[0] = level->path;
				goto end;
			}
			
			/* Checks whether this floordata entry already exists */
			endpos = (uint16) level->value[NUMFLOORDATA];
			endpos -= (outlength >> 1U);
			++endpos;
			if (endpos > level->value[NUMFLOORDATA])
			{
				endpos = 0x0000;
			}
			for (curpos = 0x0001; curpos < endpos; ++curpos)
			{
				if (memcmp(&floordata[curpos], newfdentry,
				           (size_t) outlength) == 0)
				{
					break;
				}
			}
			if (curpos == endpos)
			{
				curpos = (uint16) level->value[NUMFLOORDATA];
			}
		}
		else
		{
			curpos = (uint16) level->value[NUMFLOORDATA];
		}
		
		/* Adds the new floordata entry if needed */
		if (curpos == (uint16) level->value[NUMFLOORDATA])
		{
			/* Makes room for the new FloorData entry */
			insertbytes(level->file, level->path, level->offset[NUMMESHDATA],
			            (uint32) outlength, error);
			if (error->code != ERROR_NONE)
			{
				goto end;
			}
			writepos = (long int) level->offset[NUMMESHDATA];
			level_struct_add(level, level->offset[NUMMESHDATA],
			                 (uint32) outlength);
			
			/* Writes the new floordata-entry to the level file */
			seekval = trmod_fseek(level->file, writepos, SEEK_SET);
			if (seekval != 0)
			{
				error->code = ERROR_FILE_READ_FAILED;
				error->string[0] = level->path;
				goto end;
			}
			writeval = trmod_fwrite(newfdentry, (size_t) 1,
			                        (size_t) outlength, level->file);
			if (writeval != (size_t) outlength)
			{
				error->code = ERROR_FILE_WRITE_FAILED;
				error->string[0] = level->path;
				goto end;
			}
			outlength /= 2U;
			
			/* Updates NumFloorData */
			level->value[NUMFLOORDATA] += (uint32) outlength;
			numFD = level->value[NUMFLOORDATA];
			endian32(numFD);
			seekval = trmod_fseek(level->file,
			                      (long int) level->offset[NUMFLOORDATA],
			                      SEEK_SET);
			if (seekval != 0)
			{
				error->code = ERROR_FILE_READ_FAILED;
				error->string[0] = level->path;
				goto end;
			}
			writeval = trmod_fwrite(&numFD, (size_t) 1,
			                        (size_t) 4, level->file);
			if (writeval != (size_t) 4)
			{
				error->code = ERROR_FILE_WRITE_FAILED;
				error->string[0] = level->path;
				goto end;
			}
		}
	}
	
	/* Determines the offset to write the FDIndex to */
	writepos = (long int) (level->room[room].offset[R_NUMXSECTORS] +
	                       0x00000002);
	writepos += (long int) (((level->room[room].value[R_NUMZSECTORS] * column) +
	                         row) * 0x0008);
	writepos += (long int) secpad;
	
	/* Writes FDIndex to the level file */
	endian16(curpos);
	seekval = trmod_fseek(level->file, writepos, SEEK_SET);
	if (seekval != 0)
	{
		error->code = ERROR_FILE_READ_FAILED;
		error->string[0] = level->path;
		goto end;
	}
	writeval = trmod_fwrite(&curpos, (size_t) 1, (size_t) 2, level->file);
	if (writeval != (size_t) 2)
	{
		error->code = ERROR_FILE_WRITE_FAILED;
		error->string[0] = level->path;
		goto end;
	}
	
	/* Writes RoomBelow */
	writepos += 4l;
	seekval = trmod_fseek(level->file, writepos, SEEK_SET);
	if (seekval != 0)
	{
		error->code = ERROR_FILE_READ_FAILED;
		error->string[0] = level->path;
		goto end;
	}
	writeval = trmod_fwrite(&roombelow, (size_t) 1, (size_t) 1, level->file);
	if (writeval != (size_t) 1)
	{
		error->code = ERROR_FILE_WRITE_FAILED;
		error->string[0] = level->path;
		goto end;
	}
	
	/* Writes RoomAbove */
	writepos += 2l;
	seekval = trmod_fseek(level->file, writepos, SEEK_SET);
	if (seekval != 0)
	{
		error->code = ERROR_FILE_READ_FAILED;
		error->string[0] = level->path;
		goto end;
	}
	writeval = trmod_fwrite(&roomabove, (size_t) 1, (size_t) 1, level->file);
	if (writeval != (size_t) 1)
	{
		error->code = ERROR_FILE_WRITE_FAILED;
		error->string[0] = level->path;
		goto end;
	}
	
end:/* Frees allocated memory and ends the function */
	if (newfdentry != NULL)
	{
		free(newfdentry);
	}
	if (floordata != NULL)
	{
		free(floordata);
	}
	if (fdfunc != NULL)
	{
		free(fdfunc);
	}
	if (trigact != NULL)
	{
		free(trigact);
	}
	if (subparam != NULL)
	{
		free(subparam);
	}
}

/*
 * Function that removes floordata from a given sector
 * Parameters:
 * * level = Pointer to level struct
 * * error = Pointer to error struct
 * * room = Room number
 * * column = Column number
 * * row = Row number
 * * secpad = Padding between NumXSectors and the actual sectors (usually 0)
 */
void tr1pc_remove_floordata(struct level *level, struct error *error,
                            uint16 room, uint16 column,
                            uint16 row, uint32 secpad)
{
	/* Variable Declarations */
	long int offset; /* Offset to write to */
	uint16 zero;     /* Integer set to zero */
	uint16 maxint;   /* Integer set to max int */
	int seekval;     /* Return value of fseek() */
	size_t writeval; /* Return value of fwrite() */
	
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
	
	/* Sets fixed integers */
	zero = 0x0000;
	maxint = 0xFFFF;
	
	/* Calculates the offset to write to */
	offset = (long int) (level->room[room].offset[R_NUMXSECTORS] + 0x00000002);
	offset += (long int) (((level->room[room].value[R_NUMZSECTORS] * column) +
	                       row) * 0x0008);
	offset += (long int) secpad;
	
	/* Overwrites FDIndex */
	seekval = trmod_fseek(level->file, offset, SEEK_SET);
	if (seekval != 0)
	{
		error->code = ERROR_FILE_READ_FAILED;
		error->string[0] = level->path;
		return;
	}
	writeval = trmod_fwrite(&zero, (size_t) 1, (size_t) 2, level->file);
	if (writeval != (size_t) 2)
	{
		error->code = ERROR_FILE_WRITE_FAILED;
		error->string[0] = level->path;
		return;
	}
	offset += 4l;
	
	/* Overwrites RoomBelow */
	seekval = trmod_fseek(level->file, offset, SEEK_SET);
	if (seekval != 0)
	{
		error->code = ERROR_FILE_READ_FAILED;
		error->string[0] = level->path;
		return;
	}
	writeval = trmod_fwrite(&maxint, (size_t) 1, (size_t) 1, level->file);
	if (writeval != (size_t) 1)
	{
		error->code = ERROR_FILE_WRITE_FAILED;
		error->string[0] = level->path;
		return;
	}
	offset += 2l;
	
	/* Overwrites RoomAbove */
	seekval = trmod_fseek(level->file, offset, SEEK_SET);
	if (seekval != 0)
	{
		error->code = ERROR_FILE_READ_FAILED;
		error->string[0] = level->path;
		return;
	}
	writeval = trmod_fwrite(&maxint, (size_t) 1, (size_t) 1, level->file);
	if (writeval != (size_t) 1)
	{
		error->code = ERROR_FILE_WRITE_FAILED;
		error->string[0] = level->path;
		return;
	}
}

/*
 * Function that removes all floordata
 * Parameters:
 * * level = Pointer to level struct
 * * error = Pointer to error struct
 */
void tr1pc_remove_all_floordata(struct level *level, struct error *error)
{
	/* Variable Declarations */
	uint16 room;     /* Current room */
	uint16 column;   /* Current column */
	uint16 row;      /* Current row */
	uint32 offset;   /* Offset to remove bytes from */
	uint32 length;   /* Number of bytes to remove */
	int seekval;     /* Return value for fseek() */
	size_t writeval; /* Return value for fwrite() */
	
	/* Remove all cameras */
	if (level->value[NUMCAMERAS] > 0x00000000)
	{
		/* Removes the cameras */
		offset = (level->offset[NUMCAMERAS] + 0x00000002);
		length = (level->value[NUMCAMERAS] * 0x00000010);
		removebytes(level->file, level->path, offset, length, error);
		if (error->code != ERROR_NONE)
		{
			return;
		}
		level_struct_sub(level, offset, length);
		
		/* Sets NumCameras to zero */
		level->value[NUMCAMERAS] = 0x00000000;
		seekval = trmod_fseek(level->file,
		                      (long int) level->offset[NUMCAMERAS], SEEK_SET);
		if (seekval != 0)
		{
			error->code = ERROR_FILE_READ_FAILED;
			error->string[0] = level->path;
		}
		writeval = trmod_fwrite(&level->value[NUMCAMERAS], (size_t) 1,
		                        (size_t) 4, level->file);
		if (writeval != (size_t) 4)
		{
			error->code = ERROR_FILE_WRITE_FAILED;
			error->string[0] = level->path;
		}
	}
	
	/* Removes all FloorData */
	if (level->value[NUMFLOORDATA] > 0x00000001)
	{
		offset = (level->offset[NUMFLOORDATA] + 0x00000002);
		length = (uint32) (level->value[NUMFLOORDATA] * 0x00000002);
		length -= 0x00000002;
		removebytes(level->file, level->path, offset, length, error);
		if (error->code != ERROR_NONE)
		{
			return;
		}
		level_struct_sub(level, offset, length);
	}
	else if (level->value[NUMFLOORDATA] < 0x00000001)
	{
		offset = (level->offset[NUMFLOORDATA] + 0x00000002);
		length = (uint32) (level->value[NUMFLOORDATA] * 0x00000002);
		length = (0x00000002 - length);
		insertbytes(level->file, level->path, offset, length, error);
		if (error->code != ERROR_NONE)
		{
			return;
		}
		level_struct_add(level, offset, length);
	}
	
	/* Sets NumFloorData to one */
	level->value[NUMFLOORDATA] = 0x00000001;
	length = 0x00000001;
	endian32(length);
	seekval = trmod_fseek(level->file, (long int) level->offset[NUMFLOORDATA],
	                      SEEK_SET);
	if (seekval != 0)
	{
		error->code = ERROR_FILE_READ_FAILED;
		error->string[0] = level->path;
	}
	writeval = trmod_fwrite(&length, (size_t) 1, (size_t) 4, level->file);
	if (writeval != (size_t) 4)
	{
		error->code = ERROR_FILE_WRITE_FAILED;
		error->string[0] = level->path;
	}
	
	/* Removes all FloorData-references from sectors */
	for (room = 0x0000; room < level->numRooms; ++room)
	{
		for (column = 0x0000; column < level->room[room].value[R_NUMXSECTORS];
		     ++column)
		{
			for (row = 0x0000;
			     row < level->room[room].value[R_NUMZSECTORS]; ++row)
			{
				remove_floordata(level, error, room, column, row);
				if (error->code != ERROR_NONE)
				{
					return;
				}
			}
		}
	}
}

/*
 * Function that prints an alternate room
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
void tr1pc_get_altroom(struct level *level, struct error *error,
                       uint16 room, uint16 print)
{
	/* Variable Declarations */
	long int offset; /* Offset where to read from */
	size_t readval;  /* Return value of fread() */
	int seekval;     /* Return value of fseek() */
	int16 altroom;   /* Alternate room */
	
	/* Checks whether that room exists */
	if (room >= level->numRooms)
	{
		error->code = ERROR_ROOM_DOESNT_EXIST;
		error->u16[0] = room;
		return;
	}
	
	/* Moves to the offset */
	offset = (long int) level->room[room].offset[R_ALTROOM];
	seekval = trmod_fseek(level->file, offset, SEEK_SET);
	if (seekval != 0)
	{
		error->code = ERROR_FILE_READ_FAILED;
		error->string[0] = level->path;
		return;
	}
	
	/* Reads the alternate room */
	readval = trmod_fread(&altroom, (size_t) 1, (size_t) 2, level->file);
	if (readval != (size_t) 2)
	{
		error->code = ERROR_FILE_READ_FAILED;
		error->string[0] = level->path;
		return;
	}
	endian16(altroom);
	
	/* Determines how to print the alternate room */
	if ((print & 0x0001) != 0x0000)
	{
		trmod_printf("%s %s %s \"altroom(" PRINTU16 "," PRINT16 ")\"\n",
		             level->command, level->typestring,
		             level->path, room, altroom);
	}
	else
	{
		if (altroom >= 0x0000)
		{
			trmod_printf("Alternate Room(" PRINTU16 "," PRINT16 ")\n",
			             room, altroom);
		}
	}
}

/*
 * Function that sets a room's alternate room
 * Parameters:
 * * level = Pointer to level struct
 * * error = Pointer to error struct
 * * room = Room number
 * * altroom = Alternate Room
 */
void tr1pc_altroom(struct level *level, struct error *error,
                   uint16 room, int16 altroom)
{
	/* Variable Declarations */
	long int offset; /* Offset to write to */
	int seekval;     /* Return value of fseek() */
	size_t writeval; /* Return value of fwrite() */
	
	/* Checks whether that room exists */
	if (room >= level->numRooms)
	{
		error->code = ERROR_ROOM_DOESNT_EXIST;
		error->u16[0] = room;
		return;
	}
	
	/* Moves to the offset */
	offset = (long int) level->room[room].offset[R_ALTROOM];
	seekval = trmod_fseek(level->file, offset, SEEK_SET);
	if (seekval != 0)
	{
		error->code = ERROR_FILE_READ_FAILED;
		error->string[0] = level->path;
		return;
	}
	
	/* Adjusts for Big-Endian */
	endian16(altroom);
	
	/* Writes the alternate room */
	writeval = trmod_fwrite(&altroom, (size_t) 1, (size_t) 2, level->file);
	if (writeval != (size_t) 2)
	{
		error->code = ERROR_FILE_WRITE_FAILED;
		error->string[0] = level->path;
		return;
	}
}

/*
 * Function that prints a room's flags
 * Parameters:
 * * level = Pointer to level struct
 * * error = Pointer to error struct
 * * room = Room number
 * * flagpad = Bytes between altroom and room flags
 * * flagbits = Bits from the flags we care about
 * * print = Printing flags
 * * * 0 0 0 0  0 0 0 0   0 0 0 0  0 0 0 0
 * * * | | | |  | | | |   | | | |  | | | |
 * * * | | | |  | | | |   | | | |  | | | As command
 * * * | | | |  | | | |   | | | |  | | Also print negative
 * * * Unused
 */
void tr1pc_get_roomflags(struct level *level, struct error *error, uint16 room,
                         uint32 flagpad, uint16 flagbits, uint16 print)
{
	/* Variable Declarations */
	long int flagpos; /* Where the flags are in the level */
	int seekval;      /* Return value for fseek() */
	size_t readval;   /* Return value for fread() */
	uint16 flags;     /* Flags as read from the level file */
	
	/* Checks whether that room exists */
	if (room >= level->numRooms)
	{
		error->code = ERROR_ROOM_DOESNT_EXIST;
		error->u16[0] = room;
		return;
	}
	
	/* Reads the room flags from the level */
	flagpos = (long int) (level->room[room].offset[R_ALTROOM] + 0x00000002);
	flagpos += (long int) flagpad;
	seekval = trmod_fseek(level->file, flagpos, SEEK_SET);
	if (seekval != 0)
	{
		error->code = ERROR_FILE_READ_FAILED;
		error->string[0] = level->path;
		return;
	}
	readval = trmod_fread(&flags, (size_t) 1, (size_t) 2, level->file);
	if (readval != (size_t) 2)
	{
		error->code = ERROR_FILE_READ_FAILED;
		error->string[0] = level->path;
		return;
	}
	endian16(flags);
	
	/* Prints the flags where they exist */
	if ((flagbits & 0x0001) != 0x0000)
	{
		if ((print & 0x0001) == 0x0000)
		{
			if ((flags & 0x0001) != 0x0000)
			{
				trmod_printf("Water(" PRINTU16 ")\n", room);
			}
			else if ((print & 0x0002) != 0x0000)
			{
				trmod_printf("No water(" PRINTU16 ")\n", room);
			}
		}
		else
		{
			if ((flags & 0x0001) != 0x0000)
			{
				trmod_printf("%s %s %s \"water(" PRINTU16 ")\"\n",
				             level->command, level->typestring,
				             level->path, room);
			}
			else if ((print & 0x0002) != 0x0000)
			{
				trmod_printf("%s %s %s \"nowater(" PRINTU16 ")\"\n",
				             level->command, level->typestring,
				             level->path, room);
			}
		}
	}
	if ((flagbits & 0x0002) != 0x0000)
	{
		if ((print & 0x0001) == 0x0000)
		{
			if ((flags & 0x0002) != 0x0000)
			{
				trmod_printf("SFXAlways(" PRINTU16 ")\n", room);
			}
			else if ((print & 0x0002) != 0x0000)
			{
				trmod_printf("Not SFXAlways(" PRINTU16 ")\n", room);
			}
		}
		else
		{
			if ((flags & 0x0002) != 0x0000)
			{
				trmod_printf("%s %s %s \"sfxalways(" PRINTU16 ")\"\n",
				             level->command, level->typestring,
				             level->path, room);
			}
			else if ((print & 0x0002) != 0x0000)
			{
				trmod_printf("%s %s %s \"notsfxalways(" PRINTU16 ")\"\n",
				             level->command, level->typestring,
				             level->path, room);
			}
		}
	}
	if ((flagbits & 0x0004) != 0x0000)
	{
		if ((print & 0x0001) == 0x0000)
		{
			if ((flags & 0x0004) != 0x0000)
			{
				trmod_printf("Pitchshift(" PRINTU16 ")\n", room);
			}
			else if ((print & 0x0002) != 0x0000)
			{
				trmod_printf("No Pitchshift(" PRINTU16 ")\n", room);
			}
		}
		else
		{
			if ((flags & 0x0004) != 0x0000)
			{
				trmod_printf("%s %s %s \"pitchshift(" PRINTU16 ")\"\n",
				             level->command, level->typestring,
				             level->path, room);
			}
			else if ((print & 0x0002) != 0x0000)
			{
				trmod_printf("%s %s %s \"nopitchshift(" PRINTU16 ")\"\n",
				             level->command, level->typestring,
				             level->path, room);
			}
		}
	}
	if ((flagbits & 0x0008) != 0x0000)
	{
		if ((print & 0x0001) == 0x0000)
		{
			if ((flags & 0x0008) != 0x0000)
			{
				trmod_printf("Sky(" PRINTU16 ")\n", room);
			}
			else if ((print & 0x0002) != 0x0000)
			{
				trmod_printf("No sky(" PRINTU16 ")\n", room);
			}
		}
		else
		{
			if ((flags & 0x0008) != 0x0000)
			{
				trmod_printf("%s %s %s \"sky(" PRINTU16 ")\"\n",
				             level->command, level->typestring,
				             level->path, room);
			}
			else if ((print & 0x0002) != 0x0000)
			{
				trmod_printf("%s %s %s \"nosky(" PRINTU16 ")\"\n",
				             level->command, level->typestring,
				             level->path, room);
			}
		}
	}
	if ((flagbits & 0x0010) != 0x0000)
	{
		if ((print & 0x0001) == 0x0000)
		{
			if ((flags & 0x0010) != 0x0000)
			{
				trmod_printf("Dynamic(" PRINTU16 ")\n", room);
			}
			else if ((print & 0x0002) != 0x0000)
			{
				trmod_printf("No Dynamic(" PRINTU16 ")\n", room);
			}
		}
		else
		{
			if ((flags & 0x0010) != 0x0000)
			{
				trmod_printf("%s %s %s \"dynamic(" PRINTU16 ")\"\n",
				             level->command, level->typestring,
				             level->path, room);
			}
			else if ((print & 0x0002) != 0x0000)
			{
				trmod_printf("%s %s %s \"nodynamic(" PRINTU16 ")\"\n",
				             level->command, level->typestring,
				             level->path, room);
			}
		}
	}
	if ((flagbits & 0x0020) != 0x0000)
	{
		if ((print & 0x0001) == 0x0000)
		{
			if ((flags & 0x0020) != 0x0000)
			{
				trmod_printf("Wind(" PRINTU16 ")\n", room);
			}
			else if ((print & 0x0002) != 0x0000)
			{
				trmod_printf("No wind(" PRINTU16 ")\n", room);
			}
		}
		else
		{
			if ((flags & 0x0020) != 0x0000)
			{
				trmod_printf("%s %s %s \"wind(" PRINTU16 ")\"\n",
				             level->command, level->typestring,
				             level->path, room);
			}
			else if ((print & 0x0002) != 0x0000)
			{
				trmod_printf("%s %s %s \"nowind(" PRINTU16 ")\"\n",
				             level->command, level->typestring,
				             level->path, room);
			}
		}
	}
	if ((flagbits & 0x0040) != 0x0000)
	{
		if ((print & 0x0001) == 0x0000)
		{
			if ((flags & 0x0040) != 0x0000)
			{
				trmod_printf("Inside(" PRINTU16 ")\n", room);
			}
			else if ((print & 0x0002) != 0x0000)
			{
				trmod_printf("Not inside(" PRINTU16 ")\n", room);
			}
		}
		else
		{
			if ((flags & 0x0040) != 0x0000)
			{
				trmod_printf("%s %s %s \"inside(" PRINTU16 ")\"\n",
				             level->command, level->typestring,
				             level->path, room);
			}
			else if ((print & 0x0002) != 0x0000)
			{
				trmod_printf("%s %s %s \"notinside(" PRINTU16 ")\"\n",
				             level->command, level->typestring,
				             level->path, room);
			}
		}
	}
	if ((flagbits & 0x0080) != 0x0000)
	{
		if ((print & 0x0001) == 0x0000)
		{
			if ((flags & 0x0080) != 0x0000)
			{
				trmod_printf("Swamp(" PRINTU16 ")\n", room);
			}
			else if ((print & 0x0002) != 0x0000)
			{
				trmod_printf("No swamp(" PRINTU16 ")\n", room);
			}
		}
		else
		{
			if ((flags & 0x0080) != 0x0000)
			{
				trmod_printf("%s %s %s \"swamp(" PRINTU16 ")\"\n",
				             level->command, level->typestring,
				             level->path, room);
			}
			else if ((print & 0x0002) != 0x0000)
			{
				trmod_printf("%s %s %s \"noswamp(" PRINTU16 ")\"\n",
				             level->command, level->typestring,
				             level->path, room);
			}
		}
	}
}

/*
 * Function that modifies a room's flags based on a given integer
 * Parameters:
 * * level = Pointer to level struct
 * * error = Pointer to error struct
 * * room = Room number
 * * flagpad = Bytes between altroom and room flags
 * * mask = Integer holding the flags to be set or unset
 * * operation = (0 = AND, 1 = OR, 2 = ASSIGN)
 */
void tr1pc_setroomflags(struct level *level, struct error *error, uint16 room,
                        uint32 flagpad, uint16 mask, int operation)
{
	/* Variable Declarations */
	long int flagpos; /* Where the flags are in the level */
	int seekval;      /* Return value for fseek() */
	size_t readval;   /* Return value for fread() */
	size_t writeval;  /* Return value for fwrite() */
	uint16 flags;     /* Flags as read from the level file */
	
	/* Adjusts for Big-Endian */
	endian16(mask);
	
	/* Checks whether that room exists */
	if (room >= level->numRooms)
	{
		error->code = ERROR_ROOM_DOESNT_EXIST;
		error->u16[0] = room;
		return;
	}
	
	/* Reads the original room flags from the level */
	flagpos = (long int) (level->room[room].offset[R_ALTROOM] + 0x00000002);
	flagpos += (long int) flagpad;
	seekval = trmod_fseek(level->file, flagpos, SEEK_SET);
	if (seekval != 0)
	{
		error->code = ERROR_FILE_READ_FAILED;
		error->string[0] = level->path;
		return;
	}
	readval = trmod_fread(&flags, (size_t) 1, (size_t) 2, level->file);
	if (readval != (size_t) 2)
	{
		error->code = ERROR_FILE_READ_FAILED;
		error->string[0] = level->path;
		return;
	}
	
	/* Performs the needed modification */
	if (operation == 0)
	{
		flags &= mask;
	}
	else if (operation == 1)
	{
		flags |= mask;
	}
	else if (operation == 2)
	{
		flags = mask;
	}
	
	/* Writes the flags back to the level file */
	seekval = trmod_fseek(level->file, flagpos, SEEK_SET);
	if (seekval != 0)
	{
		error->code = ERROR_FILE_READ_FAILED;
		error->string[0] = level->path;
		return;
	}
	writeval = trmod_fwrite(&flags, (size_t) 1, (size_t) 2, level->file);
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
void tr1pc_addroom(struct level *level, struct error *error)
{
	/* Variable Declarations */
	uint8 buffer[4]; /* Buffer for keeping bytes temporarily */
	long int curpos; /* Current position in the file */
	int seekval;     /* Return value of fseek() */
	size_t writeval; /* Return value of fwrite() */
	
	/* Makes space for this new room */
	insertbytes(level->file, level->path,
	            level->offset[NUMFLOORDATA], 0x0000002C, error);
	if (error->code != ERROR_NONE)
	{
		return;
	}
	
	/* Zeroes out the room */
	zerobytes(level->file, level->path,
	          level->offset[NUMFLOORDATA], 0x0000002C, error);
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
	curpos += 24l;
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
	seekval = trmod_fseek(level->file, (long int) level->offset[NUMROOMS],
	                      SEEK_SET);
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
 * Function that prints the room's position
 * Parameters:
 * * level = Pointer to the level struct
 * * error = Pointer to the error struct
 * * room = Room number
 * * print = Printing flags
 * * * 0 0 0 0  0 0 0 0   0 0 0 0  0 0 0 0
 * * * | | | |  | | | |   | | | |  | | | |
 * * * | | | |  | | | |   | | | |  | | | As command
 * * * Unused
 */
void tr1pc_get_roompos(struct level *level, struct error *error,
                       uint16 room, uint16 print)
{
	/* Checks that this room exists */
	if (room >= level->numRooms)
	{
		error->code = ERROR_ROOM_DOESNT_EXIST;
		error->u16[0] = room;
		return;
	}
	
	/* Determines whether to print it as a command */
	if ((print & 0x0001) == 0x0000)
	{
		trmod_printf("Room Position(" PRINTU16 "," PRINT32 "," PRINT32
		             "," PRINT32 ")\n", room, level->room[room].x,
		             level->room[room].z, level->room[room].yBottom);
	}
	else
	{
		trmod_printf("%s %s %s \"moveroom(" PRINTU16 "," PRINT32 ","
		             PRINT32 "," PRINT32 ")\"\n", level->command,
		             level->typestring, level->path, room, level->room[room].x,
		             level->room[room].z, level->room[room].yBottom);
	}
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
void tr1pc_moveroom(struct level *level, struct error *error,
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
			offset += 8l;
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
			offset += 10l;
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
			offset += 10l;
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
				offset += 22l;
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
			offset += 12l;
		}
	}
}

/*
 * Function that prints the room's size
 * Parameters:
 * * level = Pointer to the level struct
 * * error = Pointer to the error struct
 * * room = Room number
 * * print = Printing flags
 * * * 0 0 0 0  0 0 0 0   0 0 0 0  0 0 0 0
 * * * | | | |  | | | |   | | | |  | | | |
 * * * | | | |  | | | |   | | | |  | | | As command
 * * * Unused
 */
void tr1pc_get_roomsize(struct level *level, struct error *error,
                        uint16 room, uint16 print)
{
	/* Variable Declarations */
	int32 roomheight; /* Height of the room */
	
	/* Checks that this room exists */
	if (room >= level->numRooms)
	{
		error->code = ERROR_ROOM_DOESNT_EXIST;
		error->u16[0] = room;
		return;
	}
	
	/* Calculates roomheight */
	roomheight = (level->room[room].yBottom - level->room[room].yTop);
	
	/* Determines whether to print it as a command */
	if ((print & 0x0001) == 0x0000)
	{
		trmod_printf("Room Size(" PRINTU16 "," PRINTU16 "," PRINTU16 "," PRINT32
		             ")\n", room, level->room[room].value[R_NUMXSECTORS],
		             level->room[room].value[R_NUMZSECTORS], roomheight);
	}
	else
	{
		trmod_printf("%s %s %s \"resizeroom(" PRINTU16 "," PRINTU16 "," PRINTU16
		             "," PRINT32 ")\"\n", level->command, level->typestring,
		             level->path, room, level->room[room].value[R_NUMXSECTORS],
		             level->room[room].value[R_NUMZSECTORS], roomheight);
	}
}

/*
 * Function that resizes a given room
 * Parameters:
 * * level = Pointer to the level struct
 * * error = Pointer to the error struct
 * * room = Room number
 * * columns = How many columns to change the room's width to
 * * rows = How many rows to change the room's length to
 * * height = How high the room is (in coordinates)
 */
void tr1pc_resize_room(struct level *level, struct error *error, uint16 room,
                       uint16 columns, uint16 rows, int32 height)
{
	/* Variable Declarations */
	uint8 emptysector[8]; /* Empty sector */
	long int offset;      /* Offset to read/write to */
	size_t writeval;      /* Return value of fwrite() */
	int seekval;          /* Return value of fseek() */
	uint16 curcol;        /* Current column */
	uint16 currow;        /* Current row */
	uint16 cursector;     /* Current sector */
	uint32 addbytes;      /* Number of bytes to insert */
	uint32 rembytes;      /* Number of bytes to remove */
	uint32 skipbytes;     /* Number of bytes to skip */
	uint32 curpos;        /* Current position in the file */
	
	/* Sets up emptysector */
	emptysector[0] = (uint8) 0x00;
	emptysector[1] = (uint8) 0x00;
	emptysector[2] = (uint8) 0xFF;
	emptysector[3] = (uint8) 0xFF;
	emptysector[4] = (uint8) 0xFF;
	emptysector[5] = (uint8) 0x81;
	emptysector[6] = (uint8) 0xFF;
	emptysector[7] = (uint8) 0x81;
	
	/* Checks that this room exists */
	if (room >= level->numRooms)
	{
		error->code = ERROR_ROOM_DOESNT_EXIST;
		error->u16[0] = room;
		return;
	}
	
	/* Stops if the number of columns or rows is zero */
	if ((columns == 0x0000) || (rows == 0x0000))
	{
		error->code = ERROR_ROOM_RESIZE_TO_ZERO;
		return;
	}
	
	/* Changes the room's height */
	level->room[room].yTop = (level->room[room].yBottom - height);
	height = level->room[room].yTop;
	endian32(height);
	offset = (long int) (level->room[room].offset[R_NUMVERTICES] - 0x00000008);
	seekval = trmod_fseek(level->file, offset, SEEK_SET);
	if (seekval != 0)
	{
		error->code = ERROR_FILE_READ_FAILED;
		error->string[0] = level->path;
		return;
	}
	writeval = trmod_fwrite(&height, (size_t) 1, (size_t) 4, level->file);
	if (writeval != (size_t) 4)
	{
		error->code = ERROR_FILE_WRITE_FAILED;
		error->string[0] = level->path;
		return;
	}
	
	/* Adjusts for there being no sectors */
	if ((level->room[room].value[R_NUMXSECTORS] == 0x0000) ||
	    (level->room[room].value[R_NUMZSECTORS] == 0x0000))
	{
		/* Adds a single sector */
		curpos = (level->room[room].offset[R_NUMZSECTORS] + 0x00000004);
		insertbytes(level->file, level->path, curpos, 0x00000008, error);
		if (error->code != ERROR_NONE)
		{
			return;
		}
		level_struct_add(level, curpos, 0x00000008);
		
		/* Overwrites the new sector */
		seekval = trmod_fseek(level->file, (long int) curpos, SEEK_SET);
		if (seekval != 0)
		{
			error->code = ERROR_FILE_READ_FAILED;
			error->string[0] = level->path;
			return;
		}
		writeval = trmod_fwrite(emptysector, (size_t) 1,
		                        (size_t) 8, level->file);
		if (writeval != (size_t) 8)
		{
			error->code = ERROR_FILE_WRITE_FAILED;
			error->string[0] = level->path;
			return;
		}
		
		/* Sets NumXSectors and NumZSectors to 1 */
		level->room[room].value[R_NUMXSECTORS] = 0x0001;
		level->room[room].value[R_NUMZSECTORS] = 0x0001;
		curcol = 0x0001;
		endian16(curcol);
		offset = (long int) level->room[room].offset[R_NUMZSECTORS];
		seekval = trmod_fseek(level->file, offset, SEEK_SET);
		if (seekval != 0)
		{
			error->code = ERROR_FILE_READ_FAILED;
			error->string[0] = level->path;
			return;
		}
		writeval = trmod_fwrite(&curcol, (size_t) 1, (size_t) 2, level->file);
		if (writeval != (size_t) 2)
		{
			error->code = ERROR_FILE_WRITE_FAILED;
			error->string[0] = level->path;
			return;
		}
		writeval = trmod_fwrite(&curcol, (size_t) 1, (size_t) 2, level->file);
		if (writeval != (size_t) 2)
		{
			error->code = ERROR_FILE_WRITE_FAILED;
			error->string[0] = level->path;
			return;
		}
	}
	
	/* Adjusts the number of columns */
	if (columns > level->room[room].value[R_NUMXSECTORS])
	{
		/* Adds the needed columns */
		offset = (long int) (level->room[room].offset[R_NUMZSECTORS] +
		                     0x00000004);
		offset += (long int) ((level->room[room].value[R_NUMXSECTORS] *
		                       level->room[room].value[R_NUMZSECTORS]) *
		                      0x0008);
		addbytes = (uint32) (columns - level->room[room].value[R_NUMXSECTORS]);
		addbytes *= level->room[room].value[R_NUMZSECTORS];
		addbytes *= 0x00000008;
		insertbytes(level->file, level->path, (uint32) offset, addbytes, error);
		if (error->code != ERROR_NONE)
		{
			return;
		}
		level_struct_add(level, (uint32) offset, addbytes);
		
		/* Fills up the added sectors with new sectors */
		for (curcol = level->room[room].value[R_NUMXSECTORS]; curcol < columns;
		     ++curcol)
		{
			for (currow = 0x0000;
			     currow < level->room[room].value[R_NUMZSECTORS];
			     ++currow)
			{
				seekval = trmod_fseek(level->file, offset, SEEK_SET);
				if (seekval != 0)
				{
					error->code = ERROR_FILE_READ_FAILED;
					error->string[0] = level->path;
					return;
				}
				writeval = trmod_fwrite(emptysector, (size_t) 1,
				                        (size_t) 8, level->file);
				if (writeval != (size_t) 8)
				{
					error->code = ERROR_FILE_WRITE_FAILED;
					error->string[0] = level->path;
					return;
				}
				offset += 8l;
			}
		}
		
		/* Adjusts NumXSectors */
		level->room[room].value[R_NUMXSECTORS] = columns;
		seekval = trmod_fseek(level->file, (long int)
		                      level->room[room].offset[R_NUMXSECTORS],
		                      SEEK_SET);
		if (seekval != 0)
		{
			error->code = ERROR_FILE_READ_FAILED;
			error->string[0] = level->path;
			return;
		}
		endian16(columns);
		writeval = trmod_fwrite(&columns, (size_t) 1, (size_t) 2, level->file);
		if (writeval != (size_t) 2)
		{
			error->code = ERROR_FILE_WRITE_FAILED;
			error->string[0] = level->path;
			return;
		}
		endian16(columns);
	}
	else if (columns < level->room[room].value[R_NUMXSECTORS])
	{
		/* Removes the unneeded columns */
		offset = (long int) (level->room[room].offset[R_NUMZSECTORS] +
		                     0x00000004);
		offset += (long int) ((columns *
		                       level->room[room].value[R_NUMZSECTORS]) *
		                      0x0008);
		rembytes = (uint32) (level->room[room].value[R_NUMXSECTORS] - columns);
		rembytes *= level->room[room].value[R_NUMZSECTORS];
		rembytes *= 0x00000008;
		removebytes(level->file, level->path, (uint32) offset, rembytes, error);
		if (error->code != ERROR_NONE)
		{
			return;
		}
		level_struct_sub(level, (uint32) offset, rembytes);
		
		/* Adjusts NumXSectors */
		level->room[room].value[R_NUMXSECTORS] = columns;
		seekval = trmod_fseek(level->file, (long int)
		                      level->room[room].offset[R_NUMXSECTORS],
		                      SEEK_SET);
		if (seekval != 0)
		{
			error->code = ERROR_FILE_READ_FAILED;
			error->string[0] = level->path;
			return;
		}
		endian16(columns);
		writeval = trmod_fwrite(&columns, (size_t) 1, (size_t) 2, level->file);
		if (writeval != (size_t) 2)
		{
			error->code = ERROR_FILE_WRITE_FAILED;
			error->string[0] = level->path;
			return;
		}
		endian16(columns);
	}
	
	/* Adjusts the number of rows if needed */
	if (rows > level->room[room].value[R_NUMZSECTORS])
	{
		/* Goes through the columns */
		curpos = (level->room[room].offset[R_NUMZSECTORS] + 0x00000004);
		skipbytes = (level->room[room].value[R_NUMZSECTORS] * 0x0008);
		addbytes = ((rows - level->room[room].value[R_NUMZSECTORS]) * 0x0008);
		for (curcol = 0x0000; curcol < level->room[room].value[R_NUMXSECTORS];
		     ++curcol)
		{
			/* Removes the unneeded sectors */
			curpos += skipbytes;
			insertbytes(level->file, level->path, curpos, addbytes, error);
			if (error->code != ERROR_NONE)
			{
				return;
			}
			level_struct_add(level, curpos, addbytes);
			
			/* Overwrites the new sectors with an empty sector */
			for (cursector = (rows - level->room[room].value[R_NUMZSECTORS]);
			     cursector > 0x0000; --cursector)
			{
				seekval = trmod_fseek(level->file, (long int) curpos, SEEK_SET);
				if (seekval != 0)
				{
					error->code = ERROR_FILE_READ_FAILED;
					error->string[0] = level->path;
					return;
				}
				writeval = trmod_fwrite(emptysector, (size_t) 1,
				                        (size_t) 8, level->file);
				if (writeval != (size_t) 8)
				{
					error->code = ERROR_FILE_WRITE_FAILED;
					error->string[0] = level->path;
					return;
				}
				curpos += 0x00000008;
			}
		}
		
		/* Adjusts NumZSectors */
		level->room[room].value[R_NUMZSECTORS] = rows;
		seekval = trmod_fseek(level->file, (long int)
		                      level->room[room].offset[R_NUMZSECTORS],
		                      SEEK_SET);
		if (seekval != 0)
		{
			error->code = ERROR_FILE_READ_FAILED;
			error->string[0] = level->path;
			return;
		}
		endian16(rows);
		writeval = trmod_fwrite(&rows, (size_t) 1, (size_t) 2, level->file);
		if (writeval != (size_t) 2)
		{
			error->code = ERROR_FILE_WRITE_FAILED;
			error->string[0] = level->path;
			return;
		}
		endian16(rows);
	}
	else if (rows < level->room[room].value[R_NUMZSECTORS])
	{
		/* Goes through the columns */
		curpos = (level->room[room].offset[R_NUMZSECTORS] + 0x00000004);
		rembytes = ((level->room[room].value[R_NUMZSECTORS] - rows) *
		            0x00000008);
		skipbytes = (rows * 0x00000008);
		for (curcol = 0x0000; curcol < level->room[room].value[R_NUMXSECTORS];
		     ++curcol)
		{
			/* Removes the unneeded sectors */
			curpos += skipbytes;
			removebytes(level->file, level->path, curpos, rembytes, error);
			if (error->code != ERROR_NONE)
			{
				return;
			}
			level_struct_sub(level, curpos, rembytes);
		}
		
		/* Adjusts NumZSectors */
		level->room[room].value[R_NUMZSECTORS] = rows;
		seekval = trmod_fseek(level->file, (long int)
		                      level->room[room].offset[R_NUMZSECTORS],
		                      SEEK_SET);
		if (seekval != 0)
		{
			error->code = ERROR_FILE_READ_FAILED;
			error->string[0] = level->path;
			return;
		}
		endian16(rows);
		writeval = trmod_fwrite(&rows, (size_t) 1, (size_t) 2, level->file);
		if (writeval != (size_t) 2)
		{
			error->code = ERROR_FILE_WRITE_FAILED;
			error->string[0] = level->path;
			return;
		}
		endian16(rows);
	}
	
}

/*
 * Function that removes all rooms from the level
 * Parameters:
 * * level = Pointer to level struct
 * * error = Pointer to error struct
 */
void tr1pc_remove_all_rooms(struct level *level, struct error *error)
{
	/* Variable Declarations */
	uint32 remdata;   /* Number of bytes to be removed */
	uint32 remoffset; /* Offset to remove bytes from */
	uint16 numRooms;  /* Copy of NumRooms */
	int seekval;      /* Return value of fseek() */
	size_t writeval;  /* Return value of fwrite() */
	
	/* Ends the function if there are no rooms */
	if (level->numRooms == 0x0000)
	{
		return;
	}
	
	/* Determines how many bytes to remove and where from */
	remdata = (level->offset[NUMFLOORDATA] - level->offset[NUMROOMS]);
	remdata -= 0x00000002;
	remoffset = (level->offset[NUMROOMS] + 0x00000002);
	
	/* Removes the bytes from the level file */
	removebytes(level->file, level->path, remoffset, remdata, error);
	if (error->code != ERROR_NONE)
	{
		return;
	}
	
	/* Sets NumRooms to 0 */
	level->numRooms = 0x0000;
	numRooms = 0x0000;
	seekval = trmod_fseek(level->file,
	                      (long int) level->offset[NUMROOMS], SEEK_SET);
	if (seekval != 0)
	{
		error->code = ERROR_FILE_READ_FAILED;
		error->string[0] = level->path;
		return;
	}
	writeval = trmod_fwrite(&numRooms, (size_t) 1, (size_t) 2, level->file);
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
 * Function that prints the floor or ceiling of a given sector
 * Parameters:
 * * level = Pointer to level struct
 * * error = Pointer to error struct
 * * room = Room number
 * * column = Column number
 * * row = Row number
 * * secpad = Padding between NumXSectors and the actual sectors (usually 0)
 * * type = What to print (0 = Floor, 1 = Ceiling)
 * * print = Printing flags
 * * * 0 0 0 0  0 0 0 0   0 0 0 0  0 0 0 0
 * * * | | | |  | | | |   | | | |  | | | |
 * * * | | | |  | | | |   | | | |  | | | As command
 * * * | | | |  | | | |   | | | |  | | In Hexadecimal
 * * * Unused
 */
void tr1pc_getfloorceil(struct level *level, struct error *error,
                        uint16 room, uint16 column, uint16 row,
                        uint32 secpad, int type, uint16 print)
{
	/* Variable Declarations */
	long int offset; /* Offset to read the height from */
	int8 height8;    /* 8-bit version of the fheight */
	int32 height32;  /* 32-bit version of the height */
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
	offset = (long int) (level->room[room].offset[R_NUMXSECTORS] + 0x00000007);
	offset += (long int) secpad;
	offset += (long int) (((level->room[room].value[R_NUMZSECTORS] * column) +
	                       row) * 0x0008);
	
	/* Adjusts for ceiling if needed */
	if (type == 1)
	{
		offset += 2l;
	}
	
	/* Reads the height into memory */
	seekval = trmod_fseek(level->file, offset, SEEK_SET);
	if (seekval != 0)
	{
		error->code = ERROR_FILE_READ_FAILED;
		error->string[0] = level->path;
		return;
	}
	readval = trmod_fread(&height8, (size_t) 1, (size_t) 1, level->file);
	if (readval != (size_t) 1)
	{
		error->code = ERROR_FILE_READ_FAILED;
		error->string[0] = level->path;
		return;
	}
	
	/* Prints it as hexadecimal if needed */
	if ((print & 0x0002U) != 0x0000U)
	{
		trmod_printf("%02X\n", (uint8) height8);
		return;
	}
	
	/* Determines how to print the height */
	if (type == 0)
	{
		if ((print & 0x0001) != 0x0000)
		{
			trmod_printf("%s %s %s \"floor(", level->command,
			             level->typestring, level->path);
		}
		else
		{
			trmod_printf("Floor(");
		}
	}
	else
	{
		if ((print & 0x0001) != 0x0000)
		{
			trmod_printf("%s %s %s \"ceiling(", level->command,
			             level->typestring, level->path);
		}
		else
		{
			trmod_printf("Ceiling(");
		}
	}
	
	/* Prints the sector */
	trmod_printf(PRINTU16 "," PRINTU16 "," PRINTU16 ",", room, column, row);
	if (height8 == (int8) 0x81)
	{
		/* Prints if it is a wall */
		if ((print & 0x0001) == 0x0000)
		{
			trmod_printf("Wall)");
		}
		else
		{
			trmod_printf("wall)");
		}
	}
	else
	{
		/* Calculates the height in coordinates */
		height32 = (int32) height8;
		height32 *= 0x00000100;
		height32 = (level->room[room].yBottom - height32);
		
		/* Prints the height */
		trmod_printf(PRINT32 ")", height32);
	}
	
	/* Prints closing quote if needed */
	if ((print & 0x0001) != 0x0000)
	{
		trmod_printf("\"");
	}
	trmod_printf("\n");
}

/*
 * Function that sets the floor or ceiling of a given sector
 * Parameters:
 * * level = Pointer to level struct
 * * error = Pointer to error struct
 * * room = Room number
 * * column = Column number
 * * row = Row number
 * * secpad = Padding between NumXSectors and the actual sectors (usually 0)
 * * height = Height (in local coordinates)
 * * type = Type of floor or ceiling:
 * * * 0 0 0 0  0 0 0 0   0 0 0 0  0 0 0 0
 * * * | | | |  | | | |   | | | |  | | | |
 * * * | | | |  | | | |   | | | |  | | | Surface (0 = Floor, 1 = Ceiling)
 * * * | | | |  | | | |   | | | |  | | Type (0 = Value, 1 = Wall)
 * * * Unused
 */
void tr1pc_setfloorceil(struct level *level, struct error *error,
                        uint16 room, uint16 column, uint16 row,
                        uint32 secpad, int32 height, uint16 type)
{
	/* Variable Declarations */
	long int offset; /* Offset to write the height to */
	int8 height8;    /* 8-bit version of the fheight */
	int seekval;     /* Return value of fseek() */
	size_t writeval; /* Return value of fwrite() */
	
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
	offset = (long int) (level->room[room].offset[R_NUMXSECTORS] + 0x00000007);
	offset += (long int) secpad;
	offset += (long int) (((level->room[room].value[R_NUMZSECTORS] * column) +
	                       row) * 0x0008);
	
	/* Adjusts for ceiling if needed */
	if ((type & 0x0001) != 0x0000)
	{
		offset += 2l;
	}
	
	/* Calculates the height in "clicks" */
	height = (level->room[room].yBottom - height);
	height /= 0x00000100;
	
	/* Sets the floor8 value */
	if ((type & 0x0002) != 0x0000)
	{
		height8 = (int8) 0x81;
	}
	else
	{
		height8 = (int8) height;
	}
	
	/* Writes the height to the level file */
	seekval = trmod_fseek(level->file, offset, SEEK_SET);
	if (seekval != 0)
	{
		error->code = ERROR_FILE_READ_FAILED;
		error->string[0] = level->path;
		return;
	}
	writeval = trmod_fwrite(&height8, (size_t) 1, (size_t) 1, level->file);
	if (writeval != (size_t) 1)
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
 * * secpad = Padding between NumXSectors and the actual sectors (usually 0)
 * * print = Printing flags
 * * * 0 0 0 0  0 0 0 0   0 0 0 0  0 0 0 0
 * * * | | | |  | | | |   | | | |  | | | |
 * * * | | | |  | | | |   | | | |  | | | As command
 * * * Unused
 */
void tr1pc_getzone(struct level *level, struct error *error, uint16 room,
                   uint16 column, uint16 row, uint32 secpad, uint16 print)
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
	offset += (long int) secpad;
	
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
	
	/* Determines how to print the zone */
	if ((print & 0x0001) != 0x0000)
	{
		trmod_printf("%s %s %s \"zone(", level->command,
		             level->typestring, level->path);
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
 * * secpad = Padding between NumXSectors and the actual sectors (usually 0)
 * * boxIndex = BoxIndex to write to the sector
 */
void tr1pc_setzone(struct level *level, struct error *error, uint16 room,
                   uint16 column, uint16 row, uint32 secpad, int16 boxIndex)
{
	/* Variable Declarations */
	long int offset; /* Offset to write the height to */
	int seekval;     /* Return value of fseek() */
	size_t writeval; /* Return value of fwrite() */
	
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
	offset += (long int) secpad;
	
	/* Writes the boxIndex to the level file */
	seekval = trmod_fseek(level->file, offset, SEEK_SET);
	if (seekval != 0)
	{
		error->code = ERROR_FILE_READ_FAILED;
		error->string[0] = level->path;
		return;
	}
	endian16(boxIndex);
	writeval = trmod_fwrite(&boxIndex, (size_t) 1, (size_t) 2, level->file);
	if (writeval != (size_t) 2)
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
void tr1pc_get_vertex(struct level *level, struct error *error,
                      uint16 room, uint16 vertex, uint16 print)
{
	/* Variable Declarations */
	int16 memvertex[4]; /* In-memory copy of the vertex */
	long int offset;    /* Offset of the vertex */
	int seekval;        /* Return value of fseek() */
	size_t readval;     /* Return value of fread() */
	int32 x;            /* The Vertex's X-Position */
	int32 y;            /* The Vertex's Y-Position */
	int32 z;            /* The Vertex's Z-Position */
	int16 inten;        /* The Vertex's Intensity */
	
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
	offset = (long int) (vertex * 0x0008);
	offset += (long int) (level->room[room].offset[R_NUMVERTICES] + 0x00000002);
	seekval = trmod_fseek(level->file, offset, SEEK_SET);
	if (seekval != 0)
	{
		error->code = ERROR_FILE_READ_FAILED;
		error->string[0] = level->path;
		return;
	}
	readval = trmod_fread(memvertex, (size_t) 1, (size_t) 8, level->file);
	if (readval != (size_t) 8)
	{
		error->code = ERROR_FILE_READ_FAILED;
		error->string[0] = level->path;
		return;
	}
	
	/* Prints as hex dump if needed */
	if ((print & 0x0010) != 0x0000)
	{
		for (inten = 0x0000; inten < (int16) level->partsize[R_VERTEX];
		     ++inten)
		{
			trmod_printf("%02X ", ((uint8 *) memvertex)[inten]);
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
	}
	
	/* Interprets the read the vertex */
	x = (int32) memvertex[0];
	y = (int32) memvertex[1];
	z = (int32) memvertex[2];
	inten = (int16) memvertex[3];
	
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
	if (inten != (int16) 0xFFFF)
	{
		inten = (0x1FFF - inten);
	}
	
	/* Prints the vertex */
	trmod_printf(PRINT32 "," PRINT32 "," PRINT32 "," PRINT16 "%s",
	             x, z, y, inten, ((inten != (int16) 0xFFFF) ? "/8191" : ""));
	
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
 * Function that adds a vertex
 * Parameters:
 * * level = Pointer to level struct
 * * error = Pointer to error struct
 * * room = Room number
 */
void tr1pc_add_vertex(struct level *level, struct error *error, uint16 room)
{
	/* Variable Declarations */
	long int offset; /* Where to read/write to */
	int seekval;     /* Return value of fseek() */
	size_t readval;  /* Return value of fread() */
	size_t writeval; /* Return value of fwrite() */
	uint32 numData;  /* Copy of NumData to modify */
	uint16 numVerts; /* Copy of NumVertices to modify */
	
	/* Checks whether the room exists */
	if (room >= level->numRooms)
	{
		error->code = ERROR_ROOM_DOESNT_EXIST;
		error->u16[0] = room;
		return;
	}
	
	/* Makes room for the new vertex */
	insertbytes(level->file, level->path,
	            level->room[room].offset[R_NUMRECTANGLES],
	            level->partsize[R_VERTEX], error);
	if (error->code != ERROR_NONE)
	{
		return;
	}
	level_struct_add(level, level->room[room].offset[R_NUMRECTANGLES],
	                 level->partsize[R_VERTEX]);
	
	/* Reads NumData into memory, adds 4 to it, and writes it back */
	offset = (long int) (level->room[room].offset[R_NUMVERTICES] - 0x00000004);
	seekval = trmod_fseek(level->file, offset, SEEK_SET);
	if (seekval != 0)
	{
		error->code = ERROR_FILE_READ_FAILED;
		error->string[0] = level->path;
		return;
	}
	readval = trmod_fread(&numData, (size_t) 1, (size_t) 4, level->file);
	if (readval != (size_t) 4)
	{
		error->code = ERROR_FILE_READ_FAILED;
		error->string[0] = level->path;
		return;
	}
	endian32(numData);
	numData += (level->partsize[R_VERTEX] / 0x00000002);
	endian32(numData);
	seekval = trmod_fseek(level->file, offset, SEEK_SET);
	if (seekval != 0)
	{
		error->code = ERROR_FILE_READ_FAILED;
		error->string[0] = level->path;
		return;
	}
	writeval = trmod_fwrite(&numData, (size_t) 1, (size_t) 4, level->file);
	if (writeval != (size_t) 4)
	{
		error->code = ERROR_FILE_WRITE_FAILED;
		error->string[0] = level->path;
		return;
	}
	
	/* Adds 1 to NumVertices and writes it to the level */
	++(level->room[room].value[R_NUMVERTICES]);
	numVerts = level->room[room].value[R_NUMVERTICES];
	endian16(numVerts);
	seekval = trmod_fseek(level->file,
	                      (long int) level->room[room].offset[R_NUMVERTICES],
	                      SEEK_SET);
	if (seekval != 0)
	{
		error->code = ERROR_FILE_READ_FAILED;
		error->string[0] = level->path;
		return;
	}
	writeval = trmod_fwrite(&numVerts, (size_t) 1, (size_t) 2, level->file);
	if (writeval != (size_t) 2)
	{
		error->code = ERROR_FILE_WRITE_FAILED;
		error->string[0] = level->path;
		return;
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
 * * inten = Intensity of the vertex
 * * rep = Bit-mask of elements to replace
 * * * 0 0 0 0  0 0 0 0   0 0 0 0  0 0 0 0
 * * * | | | |  | | | |   | | | |  | | | |
 * * * | | | |  | | | |   | | | |  | | | X
 * * * | | | |  | | | |   | | | |  | | Y
 * * * | | | |  | | | |   | | | |  | Z
 * * * | | | |  | | | |   | | | |  Intensity1
 * * * Unused
 */
void tr1pc_replace_vertex(struct level *level, struct error *error,
                          uint16 room, uint16 vertex, int32 x,
                          int32 y, int32 z, int16 inten, uint16 rep)
{
	/* Variable Declarations */
	int16 memvertex[4]; /* In-memory copy of the vertex */
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
	offset = (long int) (vertex * 0x0008);
	offset += (long int) (level->room[room].offset[R_NUMVERTICES] + 0x00000002);
	seekval = trmod_fseek(level->file, offset, SEEK_SET);
	if (seekval != 0)
	{
		error->code = ERROR_FILE_READ_FAILED;
		error->string[0] = level->path;
		return;
	}
	readval = trmod_fread(memvertex, (size_t) 1, (size_t) 8, level->file);
	if (readval != (size_t) 8)
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
		memvertex[3] = inten;
	}
	
	/* Adjusts for Big-Endian */
	if (level->endian == TRUE)
	{
		memvertex[0] = reverse16(memvertex[0]);
		memvertex[1] = reverse16(memvertex[1]);
		memvertex[2] = reverse16(memvertex[2]);
		memvertex[3] = reverse16(memvertex[3]);
	}
	
	/* Writes the vertex to the level */
	seekval = trmod_fseek(level->file, offset, SEEK_SET);
	if (seekval != 0)
	{
		error->code = ERROR_FILE_READ_FAILED;
		error->string[0] = level->path;
		return;
	}
	writeval = trmod_fwrite(memvertex, (size_t) 1, (size_t) 8, level->file);
	if (writeval != (size_t) 8)
	{
		error->code = ERROR_FILE_WRITE_FAILED;
		error->string[0] = level->path;
		return;
	}
}

/*
 * Function that removes a vertex
 * Parameters:
 * * level = Pointer to level struct
 * * error = Pointer to error struct
 * * room = Room number
 * * vertex = Vertex number
 */
void tr1pc_remove_vertex(struct level *level, struct error *error,
                         uint16 room, uint16 vertex)
{
	/* Variable Declarations */
	long int offset; /* Position where to read/write to */
	uint16 numVerts; /* Copy of NumVertices */
	uint16 curVert;  /* Current vertex */
	uint16 currect;  /* Current rectangle */
	uint32 numData;  /* Copy of NumData */
	int counter;     /* Counter variable for loops */
	int seekval;     /* Return value of fseek() */
	size_t readval;  /* Return value of fread() */
	size_t writeval; /* Return value of fwrite() */
	
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
	
	/* Removes the vertex from the file */
	offset = (long int) (level->partsize[R_VERTEX] * (uint32) vertex);
	offset += (long int) (level->room[room].offset[R_NUMVERTICES] + 0x00000002);
	removebytes(level->file, level->path, (uint32) offset,
	            level->partsize[R_VERTEX], error);
	if (error->code != ERROR_NONE)
	{
		return;
	}
	level_struct_sub(level, (uint32) offset, level->partsize[R_VERTEX]);
	
	/* Subtracts half the vertex size from NumData */
	offset = (long int) (level->room[room].offset[R_NUMVERTICES] - 0x00000004);
	seekval = trmod_fseek(level->file, offset, SEEK_SET);
	if (seekval != 0)
	{
		error->code = ERROR_FILE_READ_FAILED;
		error->string[0] = level->path;
		return;
	}
	readval = trmod_fread(&numData, (size_t) 1, (size_t) 4, level->file);
	if (readval != (size_t) 4)
	{
		error->code = ERROR_FILE_READ_FAILED;
		error->string[0] = level->path;
		return;
	}
	endian32(numData);
	numData -= (level->partsize[R_VERTEX] / 0x00000002);
	endian32(numData);
	seekval = trmod_fseek(level->file, offset, SEEK_SET);
	if (seekval != 0)
	{
		error->code = ERROR_FILE_READ_FAILED;
		error->string[0] = level->path;
		return;
	}
	writeval = trmod_fwrite(&numData, (size_t) 1, (size_t) 4, level->file);
	if (writeval != (size_t) 4)
	{
		error->code = ERROR_FILE_WRITE_FAILED;
		error->string[0] = level->path;
		return;
	}
	
	/* Subtracts one from NumVertices */
	--(level->room[room].value[R_NUMVERTICES]);
	numVerts = level->room[room].value[R_NUMVERTICES];
	endian16(numVerts);
	seekval = trmod_fseek(level->file,
	                      (long int) level->room[room].offset[R_NUMVERTICES],
	                      SEEK_SET);
	if (seekval != 0)
	{
		error->code = ERROR_FILE_READ_FAILED;
		error->string[0] = level->path;
		return;
	}
	writeval = trmod_fwrite(&numVerts, (size_t) 1, (size_t) 2, level->file);
	if (writeval != (size_t) 2)
	{
		error->code = ERROR_FILE_WRITE_FAILED;
		error->string[0] = level->path;
		return;
	}
	
	/* Adjusts rectangles */
	offset = (long int) (level->room[room].offset[R_NUMRECTANGLES] +
	                     0x00000002);
	for (currect = 0x0000; currect < level->room[room].value[R_NUMRECTANGLES];
	     ++currect)
	{
		/* Goes through the four vertices for this rectangle */
		for (counter = 0; counter < 4; ++counter)
		{
			seekval = trmod_fseek(level->file, (long int) offset, SEEK_SET);
			if (seekval != 0)
			{
				error->code = ERROR_FILE_READ_FAILED;
				error->string[0] = level->path;
				return;
			}
			readval = trmod_fread(&curVert, (size_t) 1,
			                      (size_t) 2, level->file);
			if (readval != (size_t) 2)
			{
				error->code = ERROR_FILE_READ_FAILED;
				error->string[0] = level->path;
				return;
			}
			endian16(curVert);
			if ((curVert >= vertex) && (curVert != 0x0000))
			{
				--curVert;
				endian16(curVert);
				seekval = trmod_fseek(level->file, (long int) offset, SEEK_SET);
				if (seekval != 0)
				{
					error->code = ERROR_FILE_READ_FAILED;
					error->string[0] = level->path;
					return;
				}
				writeval = trmod_fwrite(&curVert, (size_t) 1,
				                        (size_t) 2, level->file);
				if (writeval != (size_t) 2)
				{
					error->code = ERROR_FILE_WRITE_FAILED;
					error->string[0] = level->path;
					return;
				}
			}
			offset += 2l;
		}
		offset += 2l;
	}
	
	/* Adjusts triangles */
	offset = (long int) (level->room[room].offset[R_NUMTRIANGLES] + 0x00000002);
	for (currect = 0x0000; currect < level->room[room].value[R_NUMTRIANGLES];
	     ++currect)
	{
		/* Goes through the four vertices for this triangle */
		for (counter = 0; counter < 3; ++counter)
		{
			seekval = trmod_fseek(level->file, (long int) offset, SEEK_SET);
			if (seekval != 0)
			{
				error->code = ERROR_FILE_READ_FAILED;
				error->string[0] = level->path;
				return;
			}
			readval = trmod_fread(&curVert, (size_t) 1,
			                      (size_t) 2, level->file);
			if (readval != (size_t) 2)
			{
				error->code = ERROR_FILE_READ_FAILED;
				error->string[0] = level->path;
				return;
			}
			endian16(curVert);
			if ((curVert >= vertex) && (curVert != 0x0000))
			{
				--curVert;
				endian16(curVert);
				seekval = trmod_fseek(level->file, (long int) offset, SEEK_SET);
				if (seekval != 0)
				{
					error->code = ERROR_FILE_READ_FAILED;
					error->string[0] = level->path;
					return;
				}
				writeval = trmod_fwrite(&curVert, (size_t) 1,
				                        (size_t) 2, level->file);
				if (writeval != (size_t) 2)
				{
					error->code = ERROR_FILE_WRITE_FAILED;
					error->string[0] = level->path;
					return;
				}
			}
			offset += 2l;
		}
		offset += 2l;
	}
	
	/* Adjusts sprites */
	offset = (long int) (level->room[room].offset[R_NUMSPRITES] + 0x00000002);
	for (currect = 0x0000;
	     currect < level->room[room].value[R_NUMSPRITES]; ++currect)
	{
		seekval = trmod_fseek(level->file, (long int) offset, SEEK_SET);
		if (seekval != 0)
		{
			error->code = ERROR_FILE_READ_FAILED;
			error->string[0] = level->path;
			return;
		}
		readval = trmod_fread(&curVert, (size_t) 1, (size_t) 2, level->file);
		if (readval != (size_t) 2)
		{
			error->code = ERROR_FILE_READ_FAILED;
			error->string[0] = level->path;
			return;
		}
		endian16(curVert);
		if ((curVert >= vertex) && (curVert != 0x0000))
		{
			--curVert;
			endian16(curVert);
			seekval = trmod_fseek(level->file, (long int) offset, SEEK_SET);
			if (seekval != 0)
			{
				error->code = ERROR_FILE_READ_FAILED;
				error->string[0] = level->path;
				return;
			}
			writeval = trmod_fwrite(&curVert, (size_t) 1,
			                        (size_t) 2, level->file);
			if (writeval != (size_t) 2)
			{
				error->code = ERROR_FILE_WRITE_FAILED;
				error->string[0] = level->path;
				return;
			}
		}
		offset += 4l;
	}
}

/*
 * Function that removes all geometry from a room
 * Parameters:
 * * level = Pointer to level struct
 * * error = Pointer to error struct
 * * room = Room number
 */
void tr1pc_remove_geometry(struct level *level, struct error *error,
                           uint16 room)
{
	/* Variable Declarations */
	uint32 numData;  /* Copy of NumData */
	int seekval;     /* Return value of fseek() */
	size_t writeval; /* Return value of fwrite() */
	
	/* Checks whether the room exists */
	if (room >= level->numRooms)
	{
		error->code = ERROR_ROOM_DOESNT_EXIST;
		error->u16[0] = room;
		return;
	}
	
	/* Removes all the geometry */
	numData = ((level->room[room].offset[R_NUMZSECTORS] -
	            level->room[room].offset[R_NUMVERTICES]) - 0x0000000A);
	removebytes(level->file, level->path,
	            level->room[room].offset[R_NUMVERTICES], numData, error);
	if (error->code != ERROR_NONE)
	{
		return;
	}
	level_struct_sub(level, level->room[room].offset[R_NUMVERTICES], numData);
	
	/* Adjusts variables */
	level->room[room].value[R_NUMVERTICES] = 0x0000;
	level->room[room].value[R_NUMRECTANGLES] = 0x0000;
	level->room[room].value[R_NUMTRIANGLES] = 0x0000;
	level->room[room].value[R_NUMSPRITES] = 0x0000;
	level->room[room].value[R_NUMDOORS] = 0x0000;
	level->room[room].offset[R_NUMRECTANGLES] =
		(level->room[room].offset[R_NUMVERTICES] + 0x00000002);
	level->room[room].offset[R_NUMTRIANGLES] =
		(level->room[room].offset[R_NUMRECTANGLES] + 0x00000002);
	level->room[room].offset[R_NUMSPRITES] =
		(level->room[room].offset[R_NUMTRIANGLES] + 0x00000002);
	level->room[room].offset[R_NUMDOORS] =
		(level->room[room].offset[R_NUMSPRITES] + 0x00000002);
	
	/* Zeroes out these variables in the level file */
	zerobytes(level->file, level->path,
	          level->room[room].offset[R_NUMVERTICES], 0x0000000A, error);
	if (error->code != ERROR_NONE)
	{
		return;
	}
	
	/* Sets NumData to 4 */
	numData = 0x00000004;
	endian32(numData);
	seekval = trmod_fseek(level->file, (long int)
	                      (level->room[room].offset[R_NUMVERTICES] -
	                       0x00000004), SEEK_SET);
	if (seekval != 0)
	{
		error->code = ERROR_FILE_READ_FAILED;
		error->string[0] = level->path;
		return;
	}
	writeval = trmod_fwrite(&numData, (size_t) 1, (size_t) 4, level->file);
	if (writeval != (size_t) 4)
	{
		error->code = ERROR_FILE_WRITE_FAILED;
		error->string[0] = level->path;
		return;
	}
}

/*
 * Function that prints a rectangle
 * Parameters:
 * * level = Pointer to level struct
 * * error = Pointer to error struct
 * * room = Room number
 * * rectangle = Rectangle number
 * * print = Printing flags
 * * * 0 0 0 0  0 0 0 0   0 0 0 0  0 0 0 0
 * * * | | | |  | | | |   | | | |  | | | |
 * * * | | | |  | | | |   | | | |  | | | As command
 * * * | | | |  | | | |   | | | |  | | As replace command
 * * * | | | |  | | | |   | | | |  | In Hexadecimal
 * * * Unused
 */
void tr1pc_get_rectangle(struct level *level, struct error *error,
                         uint16 room, uint16 rectangle, uint16 print)
{
	/* Variable Declarations */
	uint16 memrect[5];  /* In-memory copy of the rectangle */
	uint16 doublesided; /* Whether the rectangle is double-sided */
	long int offset;    /* Offset of the rectangle */
	int seekval;        /* Return value of fseek() */
	size_t readval;     /* Return value of fread() */
	
	/* Checks whether the room exists */
	if (room >= level->numRooms)
	{
		error->code = ERROR_ROOM_DOESNT_EXIST;
		error->u16[0] = room;
		return;
	}
	/* Checks whether the rectangle exists */
	if (rectangle >= level->room[room].value[R_NUMRECTANGLES])
	{
		error->code = ERROR_RECTANGLE_DOESNT_EXIST;
		error->u16[0] = room;
		error->u16[1] = rectangle;
		return;
	}
	
	/* Reads the rectangle to be printed into memory */
	offset = (long int) (rectangle * 0x000A);
	offset += (long int) (level->room[room].offset[R_NUMRECTANGLES] +
	                      0x00000002);
	seekval = trmod_fseek(level->file, offset, SEEK_SET);
	if (seekval != 0)
	{
		error->code = ERROR_FILE_READ_FAILED;
		error->string[0] = level->path;
		return;
	}
	readval = trmod_fread(memrect, (size_t) 1, (size_t) 10, level->file);
	if (readval != (size_t) 10)
	{
		error->code = ERROR_FILE_READ_FAILED;
		error->string[0] = level->path;
		return;
	}
	
	/* Prints as hex dump if needed */
	if ((print & 0x0004) != 0x0000)
	{
		for (rectangle = 0x0000U;
		     rectangle < (uint16) level->partsize[R_RECTANGLE]; ++rectangle)
		{
			trmod_printf("%02X ", ((uint8 *) memrect)[rectangle]);
		}
		trmod_printf("\n");
		return;
	}
	
	/* Adjusts for Big-Endian */
	if (level->endian == TRUE)
	{
		memrect[0] = reverse16(memrect[0]);
		memrect[1] = reverse16(memrect[1]);
		memrect[2] = reverse16(memrect[2]);
		memrect[3] = reverse16(memrect[3]);
		memrect[4] = reverse16(memrect[4]);
	}
	doublesided = ((memrect[4] & 0x8000) >> 15U);
	memrect[4] &= 0x7FFF;
	
	/* Determines how to print the rectangle */
	if ((print & 0x0003) != 0x0000)
	{
		trmod_printf("%s %s %s ", level->command,
		             level->typestring, level->path);
		if ((print & 0x0002) != 0x0000)
		{
			trmod_printf("\"replacerectangle(" PRINTU16 "," PRINTU16 ",",
			             room, rectangle);
		}
		else
		{
			trmod_printf("\"addrectangle(" PRINTU16 ",", room);
		}
	}
	else
	{
		trmod_printf("Rectangle(" PRINTU16 ",", room);
	}
	
	/* Prints the rectangle */
	trmod_printf(PRINTU16 "," PRINTU16 "," PRINTU16 "," PRINTU16 "," PRINTU16,
	             memrect[0], memrect[1], memrect[2], memrect[3], memrect[4]);
	
	/* Prints whether the rectangle is double-sided */
	if (doublesided == 0x0000)
	{
		trmod_printf(")");
	}
	else
	{
		trmod_printf(",double)");
	}
	
	/* Adds the closing quote if needed and prints the newline */
	if ((print & 0x0003) != 0x0000)
	{
		trmod_printf("\"");
	}
	if ((print & 0x0004) == 0x0000)
	{
		trmod_printf("\n");
	}
}

/*
 * Function that adds a rectangle
 * Parameters:
 * * level = Pointer to level struct
 * * error = Pointer to error struct
 * * room = Room number
 */
void tr1pc_add_rectangle(struct level *level, struct error *error, uint16 room)
{
	/* Variable Declarations */
	long int offset; /* Where to read/write to */
	int seekval;     /* Return value of fseek() */
	size_t readval;  /* Return value of fread() */
	size_t writeval; /* Return value of fwrite() */
	uint32 numData;  /* Copy of NumData to modify */
	uint16 numRects; /* Copy of NumRectangles to modify */
	
	/* Checks whether the room exists */
	if (room >= level->numRooms)
	{
		error->code = ERROR_ROOM_DOESNT_EXIST;
		error->u16[0] = room;
		return;
	}
	
	/* Makes room for the new rectangle */
	insertbytes(level->file, level->path,
	            level->room[room].offset[R_NUMTRIANGLES],
	            level->partsize[R_RECTANGLE], error);
	if (error->code != ERROR_NONE)
	{
		return;
	}
	level_struct_add(level, level->room[room].offset[R_NUMTRIANGLES],
	                 level->partsize[R_RECTANGLE]);
	
	/* Reads NumData into memory, adds 5 to it, and writes it back */
	offset = (long int) (level->room[room].offset[R_NUMVERTICES] - 0x00000004);
	seekval = trmod_fseek(level->file, offset, SEEK_SET);
	if (seekval != 0)
	{
		error->code = ERROR_FILE_READ_FAILED;
		error->string[0] = level->path;
		return;
	}
	readval = trmod_fread(&numData, (size_t) 1, (size_t) 4, level->file);
	if (readval != (size_t) 4)
	{
		error->code = ERROR_FILE_READ_FAILED;
		error->string[0] = level->path;
		return;
	}
	endian32(numData);
	numData += (level->partsize[R_RECTANGLE] / 0x00000002);
	endian32(numData);
	seekval = trmod_fseek(level->file, offset, SEEK_SET);
	if (seekval != 0)
	{
		error->code = ERROR_FILE_READ_FAILED;
		error->string[0] = level->path;
		return;
	}
	writeval = trmod_fwrite(&numData, (size_t) 1, (size_t) 4, level->file);
	if (writeval != (size_t) 4)
	{
		error->code = ERROR_FILE_WRITE_FAILED;
		error->string[0] = level->path;
		return;
	}
	
	/* Adds 1 to NumRectangles and writes it to the level */
	++(level->room[room].value[R_NUMRECTANGLES]);
	numRects = level->room[room].value[R_NUMRECTANGLES];
	endian16(numRects);
	seekval = trmod_fseek(level->file,
	                      (long int) level->room[room].offset[R_NUMRECTANGLES],
	                      SEEK_SET);
	if (seekval != 0)
	{
		error->code = ERROR_FILE_READ_FAILED;
		error->string[0] = level->path;
		return;
	}
	writeval = trmod_fwrite(&numRects, (size_t) 1, (size_t) 2, level->file);
	if (writeval != (size_t) 2)
	{
		error->code = ERROR_FILE_WRITE_FAILED;
		error->string[0] = level->path;
		return;
	}
}

/*
 * Function that replaces a rectangle
 * Parameters:
 * * level = Pointer to level struct
 * * error = Pointer to error struct
 * * room = Room number
 * * rectangle = Rectangle number (max int means last one)
 * * vertex1 = Vertex 1 of the rectangle
 * * vertex2 = Vertex 2 of the rectangle
 * * vertex3 = Vertex 3 of the rectangle
 * * vertex4 = Vertex 4 of the rectangle
 * * texture = Texture of the rectangle
 * * doublesided = Whether the rectangle is double-sided
 * * rep = Bit-mask of elements to replace
 * * * 0 0 0 0  0 0 0 0   0 0 0 0  0 0 0 0
 * * * | | | |  | | | |   | | | |  | | | |
 * * * | | | |  | | | |   | | | |  | | | Vertex 1
 * * * | | | |  | | | |   | | | |  | | Vertex 2
 * * * | | | |  | | | |   | | | |  | Vertex 3
 * * * | | | |  | | | |   | | | |  Vertex 4
 * * * | | | |  | | | |   | | | Texture
 * * * | | | |  | | | |   | | Doublesided-ness
 * * * Unused
 */
void tr1pc_replace_rectangle(struct level *level, struct error *error,
                             uint16 room, uint16 rectangle, uint16 vertex1,
                             uint16 vertex2, uint16 vertex3, uint16 vertex4,
                             uint16 texture, uint16 doublesided, uint16 rep)
{
	/* Variable Declarations */
	uint16 memrect[5]; /* In-memory copy of the rectangle */
	long int offset;   /* Where to read/write to */
	int seekval;       /* Return value of fseek() */
	size_t readval;    /* Return value of fread() */
	size_t writeval;   /* Return value of fwrite() */
	
	/* Checks whether the room exists */
	if (room >= level->numRooms)
	{
		error->code = ERROR_ROOM_DOESNT_EXIST;
		error->u16[0] = room;
		return;
	}
	/* Sets rectangle to last one if max int */
	if (rectangle == 0xFFFF)
	{
		rectangle = level->room[room].value[R_NUMRECTANGLES];
		--rectangle;
	}
	/* Checks whether the rectangle exists */
	if (rectangle >= level->room[room].value[R_NUMRECTANGLES])
	{
		error->code = ERROR_RECTANGLE_DOESNT_EXIST;
		error->u16[0] = room;
		error->u16[1] = rectangle;
		return;
	}
	
	/* Loads the existing rectangle into memory */
	offset = (long int) (rectangle * 0x000A);
	offset += (long int) (level->room[room].offset[R_NUMRECTANGLES] +
	                      0x00000002);
	seekval = trmod_fseek(level->file, offset, SEEK_SET);
	if (seekval != 0)
	{
		error->code = ERROR_FILE_READ_FAILED;
		error->string[0] = level->path;
		return;
	}
	readval = trmod_fread(memrect, (size_t) 1, (size_t) 10, level->file);
	if (readval != (size_t) 10)
	{
		error->code = ERROR_FILE_READ_FAILED;
		error->string[0] = level->path;
		return;
	}
	
	/* Adjusts for Big-Endian */
	if (level->endian == TRUE)
	{
		memrect[0] = reverse16(memrect[0]);
		memrect[1] = reverse16(memrect[1]);
		memrect[2] = reverse16(memrect[2]);
		memrect[3] = reverse16(memrect[3]);
		memrect[4] = reverse16(memrect[4]);
	}
	
	/* Replaces the needed elements */
	if ((rep & 0x0001) != 0x0000)
	{
		memrect[0] = vertex1;
	}
	if ((rep & 0x0002) != 0x0000)
	{
		memrect[1] = vertex2;
	}
	if ((rep & 0x0004) != 0x0000)
	{
		memrect[2] = vertex3;
	}
	if ((rep & 0x0008) != 0x0000)
	{
		memrect[3] = vertex4;
	}
	if ((rep & 0x0010) != 0x0000)
	{
		memrect[4] = texture;
	}
	if ((rep & 0x0020) != 0x0000)
	{
		memrect[4] &= 0x7FFF;
		doublesided <<= 15U;
		memrect[4] |= doublesided;
	}
	
	/* Adjusts for Big-Endian */
	if (level->endian == TRUE)
	{
		memrect[0] = reverse16(memrect[0]);
		memrect[1] = reverse16(memrect[1]);
		memrect[2] = reverse16(memrect[2]);
		memrect[3] = reverse16(memrect[3]);
		memrect[4] = reverse16(memrect[4]);
	}
	
	/* Writes the rectangle back to the file */
	offset = (long int) (rectangle * 0x000A);
	offset += (long int) (level->room[room].offset[R_NUMRECTANGLES] +
	                      0x00000002);
	seekval = trmod_fseek(level->file, offset, SEEK_SET);
	if (seekval != 0)
	{
		error->code = ERROR_FILE_READ_FAILED;
		error->string[0] = level->path;
		return;
	}
	writeval = trmod_fwrite(memrect, (size_t) 1, (size_t) 10, level->file);
	if (writeval != (size_t) 10)
	{
		error->code = ERROR_FILE_WRITE_FAILED;
		error->string[0] = level->path;
		return;
	}
}

/*
 * Function that removes a rectangle
 * Parameters:
 * * level = Pointer to level struct
 * * error = Pointer to error struct
 * * room = Room number
 * * rectangle = Rectangle number
 */
void tr1pc_remove_rectangle(struct level *level, struct error *error,
                            uint16 room, uint16 rectangle)
{
	/* Variable Declarations */
	long int offset; /* Position where to read/write to */
	uint16 numRects; /* Copy of NumVertices */
	uint32 numData;  /* Copy of NumData */
	int seekval;     /* Return value of fseek() */
	size_t readval;  /* Return value of fread() */
	size_t writeval; /* Return value of fwrite() */
	
	/* Checks whether the room exists */
	if (room >= level->numRooms)
	{
		error->code = ERROR_ROOM_DOESNT_EXIST;
		error->u16[0] = room;
		return;
	}
	/* Checks whether the rectangle exists */
	if (rectangle >= level->room[room].value[R_NUMRECTANGLES])
	{
		error->code = ERROR_RECTANGLE_DOESNT_EXIST;
		error->u16[0] = room;
		error->u16[1] = rectangle;
		return;
	}
	
	/* Removes the rectangle from the file */
	offset = (long int) (level->partsize[R_RECTANGLE] * (uint32) rectangle);
	offset += (long int) (level->room[room].offset[R_NUMRECTANGLES] +
	                      0x00000002);
	removebytes(level->file, level->path, (uint32) offset,
	            level->partsize[R_RECTANGLE], error);
	if (error->code != ERROR_NONE)
	{
		return;
	}
	level_struct_sub(level, (uint32) offset, level->partsize[R_RECTANGLE]);
	
	/* Subtracts half the rectangle size from NumData */
	offset = (long int) (level->room[room].offset[R_NUMVERTICES] - 0x00000004);
	seekval = trmod_fseek(level->file, offset, SEEK_SET);
	if (seekval != 0)
	{
		error->code = ERROR_FILE_READ_FAILED;
		error->string[0] = level->path;
		return;
	}
	readval = trmod_fread(&numData, (size_t) 1, (size_t) 4, level->file);
	if (readval != (size_t) 4)
	{
		error->code = ERROR_FILE_READ_FAILED;
		error->string[0] = level->path;
		return;
	}
	endian32(numData);
	numData -= (level->partsize[R_RECTANGLE] / 0x00000002);
	endian32(numData);
	seekval = trmod_fseek(level->file, offset, SEEK_SET);
	if (seekval != 0)
	{
		error->code = ERROR_FILE_READ_FAILED;
		error->string[0] = level->path;
		return;
	}
	writeval = trmod_fwrite(&numData, (size_t) 1, (size_t) 4, level->file);
	if (writeval != (size_t) 4)
	{
		error->code = ERROR_FILE_WRITE_FAILED;
		error->string[0] = level->path;
		return;
	}
	
	/* Subtracts one from NumRectangles */
	--(level->room[room].value[R_NUMRECTANGLES]);
	numRects = level->room[room].value[R_NUMRECTANGLES];
	endian16(numRects);
	seekval = trmod_fseek(level->file,
	                      (long int) level->room[room].offset[R_NUMRECTANGLES],
	                      SEEK_SET);
	if (seekval != 0)
	{
		error->code = ERROR_FILE_READ_FAILED;
		error->string[0] = level->path;
		return;
	}
	writeval = trmod_fwrite(&numRects, (size_t) 1, (size_t) 2, level->file);
	if (writeval != (size_t) 2)
	{
		error->code = ERROR_FILE_WRITE_FAILED;
		error->string[0] = level->path;
		return;
	}
}

/*
 * Function that removes all rectangles
 * Parameters:
 * * level = Pointer to level struct
 * * error = Pointer to error struct
 * * room = Room number
 */
void tr1pc_remove_all_rectangles(struct level *level,
                                 struct error *error, uint16 room)
{
	/* Variable Declarations */
	long int offset; /* Position where to read/write to */
	uint32 remRects; /* Number of data to remove */
	uint32 numData;  /* Copy of NumData */
	int seekval;     /* Return value of fseek() */
	size_t readval;  /* Return value of fread() */
	size_t writeval; /* Return value of fwrite() */
	
	/* Checks whether the room exists */
	if (room >= level->numRooms)
	{
		error->code = ERROR_ROOM_DOESNT_EXIST;
		error->u16[0] = room;
		return;
	}
	
	/* Determines how many bytes to remove */
	remRects = ((level->room[room].offset[R_NUMTRIANGLES] -
	             level->room[room].offset[R_NUMRECTANGLES]) - 0x00000002);
	if (remRects == 0x00000000)
	{
		return;
	}
	
	/* Removes the rectangles from the file */
	removebytes(level->file, level->path,
	            level->room[room].offset[R_NUMRECTANGLES], remRects, error);
	if (error->code != ERROR_NONE)
	{
		return;
	}
	level_struct_sub(level, (uint32) level->room[room].offset[R_NUMRECTANGLES],
	                 remRects);
	
	/* Subtracts half the rectangle size from NumData */
	offset = (long int) (level->room[room].offset[R_NUMVERTICES] - 0x00000004);
	seekval = trmod_fseek(level->file, offset, SEEK_SET);
	if (seekval != 0)
	{
		error->code = ERROR_FILE_READ_FAILED;
		error->string[0] = level->path;
		return;
	}
	readval = trmod_fread(&numData, (size_t) 1, (size_t) 4, level->file);
	if (readval != (size_t) 4)
	{
		error->code = ERROR_FILE_READ_FAILED;
		error->string[0] = level->path;
		return;
	}
	endian32(numData);
	numData -= (remRects / 0x00000002);
	endian32(numData);
	seekval = trmod_fseek(level->file, offset, SEEK_SET);
	if (seekval != 0)
	{
		error->code = ERROR_FILE_READ_FAILED;
		error->string[0] = level->path;
		return;
	}
	writeval = trmod_fwrite(&numData, (size_t) 1, (size_t) 4, level->file);
	if (writeval != (size_t) 4)
	{
		error->code = ERROR_FILE_WRITE_FAILED;
		error->string[0] = level->path;
		return;
	}
	
	/* Sets NumRectangles to zero */
	level->room[room].value[R_NUMRECTANGLES] = 0x0000;
	remRects = 0x00000000;
	seekval = trmod_fseek(level->file,
	                      (long int) level->room[room].offset[R_NUMRECTANGLES],
	                      SEEK_SET);
	if (seekval != 0)
	{
		error->code = ERROR_FILE_READ_FAILED;
		error->string[0] = level->path;
		return;
	}
	writeval = trmod_fwrite(&remRects, (size_t) 1, (size_t) 2, level->file);
	if (writeval != (size_t) 2)
	{
		error->code = ERROR_FILE_WRITE_FAILED;
		error->string[0] = level->path;
		return;
	}
}

/*
 * Function that prints a triangle
 * Parameters:
 * * level = Pointer to level struct
 * * error = Pointer to error struct
 * * room = Room number
 * * triangle = Triangle number
 * * print = Printing flags
 * * * 0 0 0 0  0 0 0 0   0 0 0 0  0 0 0 0
 * * * | | | |  | | | |   | | | |  | | | |
 * * * | | | |  | | | |   | | | |  | | | As command
 * * * | | | |  | | | |   | | | |  | | As replace command
 * * * | | | |  | | | |   | | | |  | In Hexadecimal
 * * * Unused
 */
void tr1pc_get_triangle(struct level *level, struct error *error,
                        uint16 room, uint16 triangle, uint16 print)
{
	/* Variable Declarations */
	uint16 memtri[4];   /* In-memory copy of the triangle */
	uint16 doublesided; /* Whether the triangle is double-sided */
	long int offset;    /* Offset of the triangle */
	int seekval;        /* Return value of fseek() */
	size_t readval;     /* Return value of fread() */
	
	/* Checks whether the room exists */
	if (room >= level->numRooms)
	{
		error->code = ERROR_ROOM_DOESNT_EXIST;
		error->u16[0] = room;
		return;
	}
	/* Checks whether the triangle exists */
	if (triangle >= level->room[room].value[R_NUMTRIANGLES])
	{
		error->code = ERROR_TRIANGLE_DOESNT_EXIST;
		error->u16[0] = room;
		error->u16[1] = triangle;
		return;
	}
	
	/* Reads the triangle to be printed into memory */
	offset = (long int) (triangle * 0x0008);
	offset += (long int) (level->room[room].offset[R_NUMTRIANGLES] +
	                      0x00000002);
	seekval = trmod_fseek(level->file, offset, SEEK_SET);
	if (seekval != 0)
	{
		error->code = ERROR_FILE_READ_FAILED;
		error->string[0] = level->path;
		return;
	}
	readval = trmod_fread(memtri, (size_t) 1, (size_t) 8, level->file);
	if (readval != (size_t) 8)
	{
		error->code = ERROR_FILE_READ_FAILED;
		error->string[0] = level->path;
		return;
	}
	
	/* Prints as hex dump if needed */
	if ((print & 0x0004) != 0x0000)
	{
		for (triangle = 0x0000U;
		     triangle < (uint16) level->partsize[R_TRIANGLE]; ++triangle)
		{
			trmod_printf("%02X ", ((uint8 *) memtri)[triangle]);
		}
		trmod_printf("\n");
		return;
	}
	
	/* Adjusts for Big-Endian */
	if (level->endian == TRUE)
	{
		memtri[0] = reverse16(memtri[0]);
		memtri[1] = reverse16(memtri[1]);
		memtri[2] = reverse16(memtri[2]);
		memtri[3] = reverse16(memtri[3]);
	}
	doublesided = ((memtri[3] & 0x8000) >> 15U);
	memtri[3] &= 0x7FFF;
	
	/* Determines how to print the triangle */
	if ((print & 0x0003) != 0x0000)
	{
		trmod_printf("%s %s %s ", level->command,
		             level->typestring, level->path);
		if ((print & 0x0002) != 0x0000)
		{
			trmod_printf("\"replacetriangle(" PRINTU16 "," PRINTU16 ",",
			             room, triangle);
		}
		else
		{
			trmod_printf("\"addtriangle(" PRINTU16 ",", room);
		}
	}
	else
	{
		trmod_printf("Triangle(" PRINTU16 ",", room);
	}
	
	/* Prints the triangle */
	trmod_printf(PRINTU16 "," PRINTU16 "," PRINTU16 "," PRINTU16,
	             memtri[0], memtri[1], memtri[2], memtri[3]);
	
	/* Prints whether the rectangle is double-sided */
	if (doublesided == 0x0000)
	{
		trmod_printf(")");
	}
	else
	{
		trmod_printf(",double)");
	}
	
	/* Adds the closing quote if needed and prints the newline */
	if ((print & 0x0003) != 0x0000)
	{
		trmod_printf("\"");
	}
	if ((print & 0x0004) == 0x0000)
	{
		trmod_printf("\n");
	}
}

/*
 * Function that adds a triangle
 * Parameters:
 * * level = Pointer to level struct
 * * error = Pointer to error struct
 * * room = Room number
 */
void tr1pc_add_triangle(struct level *level, struct error *error, uint16 room)
{
	/* Variable Declarations */
	long int offset; /* Where to read/write to */
	int seekval;     /* Return value of fseek() */
	size_t readval;  /* Return value of fread() */
	size_t writeval; /* Return value of fwrite() */
	uint32 numData;  /* Copy of NumData to modify */
	uint16 numTris;  /* Copy of NumTriangles to modify */
	
	/* Checks whether the room exists */
	if (room >= level->numRooms)
	{
		error->code = ERROR_ROOM_DOESNT_EXIST;
		error->u16[0] = room;
		return;
	}
	
	/* Makes room for the new triangle */
	insertbytes(level->file, level->path,
	            level->room[room].offset[R_NUMSPRITES],
	            level->partsize[R_TRIANGLE], error);
	if (error->code != ERROR_NONE)
	{
		return;
	}
	level_struct_add(level, level->room[room].offset[R_NUMSPRITES],
	                 level->partsize[R_TRIANGLE]);
	
	/* Reads NumData into memory, adds 4 to it, and writes it back */
	offset = (long int) (level->room[room].offset[R_NUMVERTICES] - 0x00000004);
	seekval = trmod_fseek(level->file, offset, SEEK_SET);
	if (seekval != 0)
	{
		error->code = ERROR_FILE_READ_FAILED;
		error->string[0] = level->path;
		return;
	}
	readval = trmod_fread(&numData, (size_t) 1, (size_t) 4, level->file);
	if (readval != (size_t) 4)
	{
		error->code = ERROR_FILE_READ_FAILED;
		error->string[0] = level->path;
		return;
	}
	endian32(numData);
	numData += (level->partsize[R_TRIANGLE] / 0x00000002);
	endian32(numData);
	seekval = trmod_fseek(level->file, offset, SEEK_SET);
	if (seekval != 0)
	{
		error->code = ERROR_FILE_READ_FAILED;
		error->string[0] = level->path;
		return;
	}
	writeval = trmod_fwrite(&numData, (size_t) 1, (size_t) 4, level->file);
	if (writeval != (size_t) 4)
	{
		error->code = ERROR_FILE_WRITE_FAILED;
		error->string[0] = level->path;
		return;
	}
	
	/* Adds 1 to numTriangles and writes it to the level */
	++(level->room[room].value[R_NUMTRIANGLES]);
	numTris = level->room[room].value[R_NUMTRIANGLES];
	endian16(numTris);
	seekval = trmod_fseek(level->file,
	                      (long int) level->room[room].offset[R_NUMTRIANGLES],
	                      SEEK_SET);
	if (seekval != 0)
	{
		error->code = ERROR_FILE_READ_FAILED;
		error->string[0] = level->path;
		return;
	}
	writeval = trmod_fwrite(&numTris, (size_t) 1, (size_t) 2, level->file);
	if (writeval != (size_t) 2)
	{
		error->code = ERROR_FILE_WRITE_FAILED;
		error->string[0] = level->path;
		return;
	}
}

/*
 * Function that replaces a triangle
 * Parameters:
 * * level = Pointer to level struct
 * * error = Pointer to error struct
 * * room = Room number
 * * triangle = Triangle number (max int means last one)
 * * vertex1 = Vertex 1 of the triangle
 * * vertex2 = Vertex 2 of the triangle
 * * vertex3 = Vertex 3 of the triangle
 * * texture = Texture of the triangle
 * * doublesided = Whether the triangle is double-sided
 * * rep = Bit-mask of elements to replace
 * * * 0 0 0 0  0 0 0 0   0 0 0 0  0 0 0 0
 * * * | | | |  | | | |   | | | |  | | | |
 * * * | | | |  | | | |   | | | |  | | | Vertex 1
 * * * | | | |  | | | |   | | | |  | | Vertex 2
 * * * | | | |  | | | |   | | | |  | Vertex 3
 * * * | | | |  | | | |   | | | |  Texture
 * * * | | | |  | | | |   | | | Doublesided-ness
 * * * Unused
 */
void tr1pc_replace_triangle(struct level *level, struct error *error,
                            uint16 room, uint16 triangle, uint16 vertex1,
                            uint16 vertex2, uint16 vertex3, uint16 texture,
                            uint16 doublesided, uint16 rep)
{
	/* Variable Declarations */
	uint16 memtri[5]; /* In-memory copy of the triangle */
	long int offset;  /* Where to read/write to */
	int seekval;      /* Return value of fseek() */
	size_t readval;   /* Return value of fread() */
	size_t writeval;  /* Return value of fwrite() */
	
	/* Checks whether the room exists */
	if (room >= level->numRooms)
	{
		error->code = ERROR_ROOM_DOESNT_EXIST;
		error->u16[0] = room;
		return;
	}
	/* Sets triangle to last one if max int */
	if (triangle == 0xFFFF)
	{
		triangle = level->room[room].value[R_NUMTRIANGLES];
		--triangle;
	}
	/* Checks whether the triangle exists */
	if (triangle >= level->room[room].value[R_NUMTRIANGLES])
	{
		error->code = ERROR_TRIANGLE_DOESNT_EXIST;
		error->u16[0] = room;
		error->u16[1] = triangle;
		return;
	}
	
	/* Loads the existing triangle into memory */
	offset = (long int) (triangle * 0x0008);
	offset += (long int) (level->room[room].offset[R_NUMTRIANGLES] +
	                      0x00000002);
	seekval = trmod_fseek(level->file, offset, SEEK_SET);
	if (seekval != 0)
	{
		error->code = ERROR_FILE_READ_FAILED;
		error->string[0] = level->path;
		return;
	}
	readval = trmod_fread(memtri, (size_t) 1, (size_t) 8, level->file);
	if (readval != (size_t) 8)
	{
		error->code = ERROR_FILE_READ_FAILED;
		error->string[0] = level->path;
		return;
	}
	
	/* Adjusts for Big-Endian */
	if (level->endian == TRUE)
	{
		memtri[0] = reverse16(memtri[0]);
		memtri[1] = reverse16(memtri[1]);
		memtri[2] = reverse16(memtri[2]);
		memtri[3] = reverse16(memtri[3]);
	}
	
	/* Replaces the needed elements */
	if ((rep & 0x0001) != 0x0000)
	{
		memtri[0] = vertex1;
	}
	if ((rep & 0x0002) != 0x0000)
	{
		memtri[1] = vertex2;
	}
	if ((rep & 0x0004) != 0x0000)
	{
		memtri[2] = vertex3;
	}
	if ((rep & 0x0008) != 0x0000)
	{
		memtri[3] = texture;
	}
	if ((rep & 0x0010) != 0x0000)
	{
		memtri[3] &= 0x7FFF;
		doublesided <<= 15U;
		memtri[3] |= doublesided;
	}
	
	/* Adjusts for Big-Endian */
	if (level->endian == TRUE)
	{
		memtri[0] = reverse16(memtri[0]);
		memtri[1] = reverse16(memtri[1]);
		memtri[2] = reverse16(memtri[2]);
		memtri[3] = reverse16(memtri[3]);
	}
	
	/* Writes the triangle back to the file */
	offset = (long int) (triangle * 0x0008);
	offset += (long int) (level->room[room].offset[R_NUMTRIANGLES] +
	                      0x00000002);
	seekval = trmod_fseek(level->file, offset, SEEK_SET);
	if (seekval != 0)
	{
		error->code = ERROR_FILE_READ_FAILED;
		error->string[0] = level->path;
		return;
	}
	writeval = trmod_fwrite(memtri, (size_t) 1, (size_t) 8, level->file);
	if (writeval != (size_t) 8)
	{
		error->code = ERROR_FILE_WRITE_FAILED;
		error->string[0] = level->path;
		return;
	}
}

/*
 * Function that removes a triangle
 * Parameters:
 * * level = Pointer to level struct
 * * error = Pointer to error struct
 * * room = Room number
 * * triangle = Triangle number
 */
void tr1pc_remove_triangle(struct level *level, struct error *error,
                           uint16 room, uint16 triangle)
{
	/* Variable Declarations */
	long int offset; /* Position where to read/write to */
	uint16 numTris;  /* Copy of NumTriangles */
	uint32 numData;  /* Copy of NumData */
	int seekval;     /* Return value of fseek() */
	size_t readval;  /* Return value of fread() */
	size_t writeval; /* Return value of fwrite() */
	
	/* Checks whether the room exists */
	if (room >= level->numRooms)
	{
		error->code = ERROR_ROOM_DOESNT_EXIST;
		error->u16[0] = room;
		return;
	}
	/* Checks whether the triangle exists */
	if (triangle >= level->room[room].value[R_NUMTRIANGLES])
	{
		error->code = ERROR_TRIANGLE_DOESNT_EXIST;
		error->u16[0] = room;
		error->u16[1] = triangle;
		return;
	}
	
	/* Removes the triangle from the file */
	offset = (long int) (level->partsize[R_TRIANGLE] * (uint32) triangle);
	offset += (long int) (level->room[room].offset[R_NUMTRIANGLES] +
	                      0x00000002);
	removebytes(level->file, level->path,
	            (uint32) offset, level->partsize[R_TRIANGLE], error);
	if (error->code != ERROR_NONE)
	{
		return;
	}
	level_struct_sub(level, (uint32) offset, level->partsize[R_TRIANGLE]);
	
	/* Subtracts half the triangle size from NumData */
	offset = (long int) (level->room[room].offset[R_NUMVERTICES] - 0x00000004);
	seekval = trmod_fseek(level->file, offset, SEEK_SET);
	if (seekval != 0)
	{
		error->code = ERROR_FILE_READ_FAILED;
		error->string[0] = level->path;
		return;
	}
	readval = trmod_fread(&numData, (size_t) 1, (size_t) 4, level->file);
	if (readval != (size_t) 4)
	{
		error->code = ERROR_FILE_READ_FAILED;
		error->string[0] = level->path;
		return;
	}
	endian32(numData);
	numData -= (level->partsize[R_TRIANGLE] / 0x00000002);
	endian32(numData);
	seekval = trmod_fseek(level->file, offset, SEEK_SET);
	if (seekval != 0)
	{
		error->code = ERROR_FILE_READ_FAILED;
		error->string[0] = level->path;
		return;
	}
	writeval = trmod_fwrite(&numData, (size_t) 1, (size_t) 4, level->file);
	if (writeval != (size_t) 4)
	{
		error->code = ERROR_FILE_WRITE_FAILED;
		error->string[0] = level->path;
		return;
	}
	
	/* Subtracts one from NumTriangles */
	--(level->room[room].value[R_NUMTRIANGLES]);
	numTris = level->room[room].value[R_NUMTRIANGLES];
	endian16(numTris);
	seekval = trmod_fseek(level->file,
	                      (long int) level->room[room].offset[R_NUMTRIANGLES],
	                      SEEK_SET);
	if (seekval != 0)
	{
		error->code = ERROR_FILE_READ_FAILED;
		error->string[0] = level->path;
		return;
	}
	writeval = trmod_fwrite(&numTris, (size_t) 1, (size_t) 2, level->file);
	if (writeval != (size_t) 2)
	{
		error->code = ERROR_FILE_WRITE_FAILED;
		error->string[0] = level->path;
		return;
	}
}

/*
 * Function that removes all triangles
 * Parameters:
 * * level = Pointer to level struct
 * * error = Pointer to error struct
 * * room = Room number
 */
void tr1pc_remove_all_triangles(struct level *level,
                                struct error *error, uint16 room)
{
	/* Variable Declarations */
	long int offset; /* Position where to read/write to */
	uint32 remTris;  /* Number of data to remove */
	uint32 numData;  /* Copy of NumData */
	int seekval;     /* Return value of fseek() */
	size_t readval;  /* Return value of fread() */
	size_t writeval; /* Return value of fwrite() */
	
	/* Checks whether the room exists */
	if (room >= level->numRooms)
	{
		error->code = ERROR_ROOM_DOESNT_EXIST;
		error->u16[0] = room;
		return;
	}
	
	/* Determines how many bytes to remove */
	remTris = ((level->room[room].offset[R_NUMSPRITES] -
	            level->room[room].offset[R_NUMTRIANGLES]) - 0x00000002);
	if (remTris == 0x00000000)
	{
		return;
	}
	
	/* Removes the triangles from the file */
	removebytes(level->file, level->path,
	            level->room[room].offset[R_NUMTRIANGLES], remTris, error);
	if (error->code != ERROR_NONE)
	{
		return;
	}
	level_struct_sub(level, level->room[room].offset[R_NUMTRIANGLES], remTris);
	
	/* Subtracts half the triangle size from NumData */
	offset = (long int) (level->room[room].offset[R_NUMVERTICES] - 0x00000004);
	seekval = trmod_fseek(level->file, offset, SEEK_SET);
	if (seekval != 0)
	{
		error->code = ERROR_FILE_READ_FAILED;
		error->string[0] = level->path;
		return;
	}
	readval = trmod_fread(&numData, (size_t) 1, (size_t) 4, level->file);
	if (readval != (size_t) 4)
	{
		error->code = ERROR_FILE_READ_FAILED;
		error->string[0] = level->path;
		return;
	}
	endian32(numData);
	numData -= (remTris / 0x00000002);
	endian32(numData);
	seekval = trmod_fseek(level->file, offset, SEEK_SET);
	if (seekval != 0)
	{
		error->code = ERROR_FILE_READ_FAILED;
		error->string[0] = level->path;
		return;
	}
	writeval = trmod_fwrite(&numData, (size_t) 1, (size_t) 4, level->file);
	if (writeval != (size_t) 4)
	{
		error->code = ERROR_FILE_WRITE_FAILED;
		error->string[0] = level->path;
		return;
	}
	
	/* Sets NumTriangles to zero */
	level->room[room].value[R_NUMTRIANGLES] = 0x0000;
	remTris = 0x00000000;
	seekval = trmod_fseek(level->file,
	                      (long int) level->room[room].offset[R_NUMTRIANGLES],
	                      SEEK_SET);
	if (seekval != 0)
	{
		error->code = ERROR_FILE_READ_FAILED;
		error->string[0] = level->path;
		return;
	}
	writeval = trmod_fwrite(&remTris, (size_t) 1, (size_t) 2, level->file);
	if (writeval != (size_t) 2)
	{
		error->code = ERROR_FILE_WRITE_FAILED;
		error->string[0] = level->path;
		return;
	}
}

/*
 * Function that prints a sprite
 * Parameters:
 * * level = Pointer to level struct
 * * error = Pointer to error struct
 * * room = Room number
 * * sprite = Sprite number
 * * print = Printing flags
 * * * 0 0 0 0  0 0 0 0   0 0 0 0  0 0 0 0
 * * * | | | |  | | | |   | | | |  | | | |
 * * * | | | |  | | | |   | | | |  | | | As command
 * * * | | | |  | | | |   | | | |  | | As replace command
 * * * | | | |  | | | |   | | | |  | In Hexadecimal
 * * * Unused
 */
void tr1pc_get_sprite(struct level *level, struct error *error,
                      uint16 room, uint16 sprite, uint16 print)
{
	/* Variable Declarations */
	uint16 memsprite[2]; /* In-memory copy of the sprite */
	uint32 spriteID;     /* SpriteID from SpriteSequence */
	uint32 curSpriteSeq; /* Current SpriteSequence */
	uint16 curOffset;    /* Offset from SpriteSequence */
	long int offset;     /* Offset of the sprite */
	int seekval;         /* Return value of fseek() */
	size_t readval;      /* Return value of fread() */
	
	/* Checks whether the room exists */
	if (room >= level->numRooms)
	{
		error->code = ERROR_ROOM_DOESNT_EXIST;
		error->u16[0] = room;
		return;
	}
	/* Checks whether the sprite exists */
	if (sprite >= level->room[room].value[R_NUMSPRITES])
	{
		error->code = ERROR_SPRITE_DOESNT_EXIST;
		error->u16[0] = room;
		error->u16[1] = sprite;
		return;
	}
	
	/* Reads the sprite to be printed into memory */
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
	
	/* Prints as hex dump if needed */
	if ((print & 0x0004) != 0x0000)
	{
		for (spriteID = 0x00000000U;
		     spriteID < level->partsize[R_SPRITE]; ++spriteID)
		{
			trmod_printf("%02X ", ((uint8 *) memsprite)[spriteID]);
		}
		trmod_printf("\n");
		return;
	}
	
	/* Adjusts for Big-Endian */
	if (level->endian == TRUE)
	{
		memsprite[0] = reverse16(memsprite[0]);
		memsprite[1] = reverse16(memsprite[1]);
	}
	
	/* Finds the appropriate Sprite Sequence */
	offset = (long int) (level->offset[NUMSPRITESEQUENCES] + 0x0000000A);
	for (curSpriteSeq = 0x00000000;
	     curSpriteSeq < level->value[NUMSPRITESEQUENCES];
	     ++curSpriteSeq)
	{
		/* Reads the offset of the current sprite sequence into memory */
		seekval = trmod_fseek(level->file, offset, SEEK_SET);
		if (seekval != 0)
		{
			error->code = ERROR_FILE_READ_FAILED;
			error->string[0] = level->path;
			return;
		}
		readval = trmod_fread(&curOffset, (size_t) 1, (size_t) 2, level->file);
		if (readval != (size_t) 2)
		{
			error->code = ERROR_FILE_READ_FAILED;
			error->string[0] = level->path;
			return;
		}
		endian16(curOffset);
		
		/* Checks whether this is the correct sequence */
		if (curOffset == memsprite[1])
		{
			/* Reads the SpriteID into memory */
			offset -= 6l;
			seekval = trmod_fseek(level->file, offset, SEEK_SET);
			if (seekval != 0)
			{
				error->code = ERROR_FILE_READ_FAILED;
				error->string[0] = level->path;
				return;
			}
			readval = trmod_fread(&spriteID, (size_t) 1,
			                      (size_t) 4, level->file);
			if (readval != (size_t) 4)
			{
				error->code = ERROR_FILE_READ_FAILED;
				error->string[0] = level->path;
				return;
			}
			endian32(spriteID);
			break;
		}
		
		offset += 8l;
	}
	
	/* Checks whether the correct sprite sequence was found */
	if (curSpriteSeq == level->value[NUMSPRITESEQUENCES])
	{
		error->code = ERROR_SPRITESEQ_NOT_FOUND;
		error->u16[0] = memsprite[1];
		return;
	}
	
	/* Determines how to print the sprite */
	if ((print & 0x0003) != 0x0000)
	{
		trmod_printf("%s %s %s ", level->command,
		             level->typestring, level->path);
		if ((print & 0x0002) != 0x0000)
		{
			trmod_printf("\"replacesprite(" PRINTU16 "," PRINTU16
			             "," PRINTU32 ",", room, sprite, spriteID);
		}
		else
		{
			trmod_printf("\"addsprite(" PRINTU32 "," PRINTU16 ",",
			             spriteID, room);
		}
	}
	else
	{
		trmod_printf("Sprite(" PRINTU32 "," PRINTU16 ",", spriteID, room);
	}
	
	/* Prints the coordinates for the sprite */
	get_vertex(level, error, room, memsprite[0], 0x0008);
	if (error->code != ERROR_NONE)
	{
		return;
	}
	
	/* Ends the printing */
	if ((print & 0x0003) != 0x0000)
	{
		trmod_printf(")\"\n");
	}
	else
	{
		trmod_printf(")\n");
	}
}

/*
 * Function that adds a sprite
 * Parameters:
 * * level = Pointer to level struct
 * * error = Pointer to error struct
 * * room = Room number
 */
void tr1pc_add_sprite(struct level *level, struct error *error, uint16 room)
{
	/* Variable Declarations */
	long int offset;   /* Where to read/write to */
	int seekval;       /* Return value of fseek() */
	size_t readval;    /* Return value of fread() */
	size_t writeval;   /* Return value of fwrite() */
	uint32 numData;    /* Copy of NumData to modify */
	uint16 numSprites; /* Copy of NumSprites to modify */
	
	/* Checks whether the room exists */
	if (room >= level->numRooms)
	{
		error->code = ERROR_ROOM_DOESNT_EXIST;
		error->u16[0] = room;
		return;
	}
	
	/* Makes room for the new sprite */
	insertbytes(level->file, level->path, level->room[room].offset[R_NUMDOORS],
	            level->partsize[R_SPRITE], error);
	if (error->code != ERROR_NONE)
	{
		return;
	}
	level_struct_add(level, level->room[room].offset[R_NUMDOORS],
	                 level->partsize[R_SPRITE]);
	
	/* Reads NumData into memory, adds 2 to it, and writes it back */
	offset = (long int) (level->room[room].offset[R_NUMVERTICES] - 0x00000004);
	seekval = trmod_fseek(level->file, offset, SEEK_SET);
	if (seekval != 0)
	{
		error->code = ERROR_FILE_READ_FAILED;
		error->string[0] = level->path;
		return;
	}
	readval = trmod_fread(&numData, (size_t) 1, (size_t) 4, level->file);
	if (readval != (size_t) 4)
	{
		error->code = ERROR_FILE_READ_FAILED;
		error->string[0] = level->path;
		return;
	}
	endian32(numData);
	numData += (level->partsize[R_SPRITE] / 0x00000002);
	endian32(numData);
	seekval = trmod_fseek(level->file, offset, SEEK_SET);
	if (seekval != 0)
	{
		error->code = ERROR_FILE_READ_FAILED;
		error->string[0] = level->path;
		return;
	}
	writeval = trmod_fwrite(&numData, (size_t) 1, (size_t) 4, level->file);
	if (writeval != (size_t) 4)
	{
		error->code = ERROR_FILE_WRITE_FAILED;
		error->string[0] = level->path;
		return;
	}
	
	/* Adds 1 to NumSprites and writes it to the level */
	++(level->room[room].value[R_NUMSPRITES]);
	numSprites = level->room[room].value[R_NUMSPRITES];
	endian16(numSprites);
	seekval = trmod_fseek(level->file,
	                      (long int) level->room[room].offset[R_NUMSPRITES],
	                      SEEK_SET);
	if (seekval != 0)
	{
		error->code = ERROR_FILE_READ_FAILED;
		error->string[0] = level->path;
		return;
	}
	writeval = trmod_fwrite(&numSprites, (size_t) 1, (size_t) 2, level->file);
	if (writeval != (size_t) 2)
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
 * * inten = Intensity of the vertex
 * * rep = Bit-mask of elements to replace
 * * * 0 0 0 0  0 0 0 0   0 0 0 0  0 0 0 0
 * * * | | | |  | | | |   | | | |  | | | |
 * * * | | | |  | | | |   | | | |  | | | ID
 * * * | | | |  | | | |   | | | |  | | X
 * * * | | | |  | | | |   | | | |  | Y
 * * * | | | |  | | | |   | | | |  Z
 * * * | | | |  | | | |   | | | Intensity
 * * * Unused
 */
void tr1pc_replace_sprite(struct level *level, struct error *error,
                          uint16 room, uint16 sprite, uint32 id,
                          int32 x, int32 y, int32 z, int16 inten, uint16 rep)
{
	/* Variable Declarations */
	uint16 memsprite[2]; /* In-memory copy of the sprite */
	uint32 cursprseq;    /* Current SpriteSequence */
	uint32 curID;        /* Current SpriteID */
	int16 ver_x;         /* X-position of the vertex */
	int16 ver_y;         /* Y-position of the vertex */
	int16 ver_z;         /* Z-position of the vertex */
	int16 ver_inten;     /* Intensity of the vertex */
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
	offset = (long int) (memsprite[0] * 0x0008);
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
	readval = trmod_fread(&ver_inten, (size_t) 1, (size_t) 2, level->file);
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
		ver_inten = reverse16(ver_inten);
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
		ver_inten = (int16) inten;
	}
	
	/* Adds/Finds the needed vertex */
	memsprite[0] = tr1pc_find_vertex(level, error, room,
	                                 ver_x, ver_y, ver_z, ver_inten);
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
 * Function that removes a sprite
 * Parameters:
 * * level = Pointer to level struct
 * * error = Pointer to error struct
 * * room = Room number
 * * sprite = Sprite number
 */
void tr1pc_remove_sprite(struct level *level, struct error *error,
                         uint16 room, uint16 sprite)
{
	/* Variable Declarations */
	long int offset;   /* Position where to read/write to */
	uint16 numSprites; /* Copy of NumSprites */
	uint32 numData;    /* Copy of NumData */
	int seekval;       /* Return value of fseek() */
	size_t readval;    /* Return value of fread() */
	size_t writeval;   /* Return value of fwrite() */
	
	/* Checks whether the room exists */
	if (room >= level->numRooms)
	{
		error->code = ERROR_ROOM_DOESNT_EXIST;
		error->u16[0] = room;
		return;
	}
	/* Checks whether the sprite exists */
	if (sprite >= level->room[room].value[R_NUMSPRITES])
	{
		error->code = ERROR_SPRITE_DOESNT_EXIST;
		error->u16[0] = room;
		error->u16[1] = sprite;
		return;
	}
	
	/* Removes the sprite from the file */
	offset = (long int) (0x00000004 * (uint32) sprite);
	offset += (long int) (level->room[room].offset[R_NUMSPRITES] + 0x00000002);
	removebytes(level->file, level->path, (uint32) offset, 0x00000004, error);
	if (error->code != ERROR_NONE)
	{
		return;
	}
	level_struct_sub(level, (uint32) offset, 0x00000004);
	
	/* Subtracts half the sprite size from NumData */
	offset = (long int) (level->room[room].offset[R_NUMVERTICES] - 0x00000004);
	seekval = trmod_fseek(level->file, offset, SEEK_SET);
	if (seekval != 0)
	{
		error->code = ERROR_FILE_READ_FAILED;
		error->string[0] = level->path;
		return;
	}
	readval = trmod_fread(&numData, (size_t) 1, (size_t) 4, level->file);
	if (readval != (size_t) 4)
	{
		error->code = ERROR_FILE_READ_FAILED;
		error->string[0] = level->path;
		return;
	}
	endian32(numData);
	numData -= 0x00000002;
	endian32(numData);
	seekval = trmod_fseek(level->file, offset, SEEK_SET);
	if (seekval != 0)
	{
		error->code = ERROR_FILE_READ_FAILED;
		error->string[0] = level->path;
		return;
	}
	writeval = trmod_fwrite(&numData, (size_t) 1, (size_t) 4, level->file);
	if (writeval != (size_t) 4)
	{
		error->code = ERROR_FILE_WRITE_FAILED;
		error->string[0] = level->path;
		return;
	}
	
	/* Subtracts one from NumSprites */
	--(level->room[room].value[R_NUMSPRITES]);
	numSprites = level->room[room].value[R_NUMSPRITES];
	endian16(numSprites);
	seekval = trmod_fseek(level->file,
	                      (long int) level->room[room].offset[R_NUMSPRITES],
	                      SEEK_SET);
	if (seekval != 0)
	{
		error->code = ERROR_FILE_READ_FAILED;
		error->string[0] = level->path;
		return;
	}
	writeval = trmod_fwrite(&numSprites, (size_t) 1, (size_t) 2, level->file);
	if (writeval != (size_t) 2)
	{
		error->code = ERROR_FILE_WRITE_FAILED;
		error->string[0] = level->path;
		return;
	}
}

/*
 * Function that removes all sprites
 * Parameters:
 * * level = Pointer to level struct
 * * error = Pointer to error struct
 * * room = Room number
 */
void tr1pc_remove_all_sprites(struct level *level,
                              struct error *error, uint16 room)
{
	/* Variable Declarations */
	long int offset;   /* Position where to read/write to */
	uint32 remSprites; /* Number of data to remove */
	uint32 numData;    /* Copy of NumData */
	int seekval;       /* Return value of fseek() */
	size_t readval;    /* Return value of fread() */
	size_t writeval;   /* Return value of fwrite() */
	
	/* Checks whether the room exists */
	if (room >= level->numRooms)
	{
		error->code = ERROR_ROOM_DOESNT_EXIST;
		error->u16[0] = room;
		return;
	}
	
	/* Determines how many bytes to remove */
	remSprites = ((level->room[room].offset[R_NUMDOORS] -
	               level->room[room].offset[R_NUMSPRITES]) - 0x00000002);
	if (remSprites == 0x00000000)
	{
		return;
	}
	
	/* Removes the sprites from the file */
	removebytes(level->file, level->path,
	            level->room[room].offset[R_NUMSPRITES], remSprites, error);
	if (error->code != ERROR_NONE)
	{
		return;
	}
	level_struct_sub(level, (uint32) level->room[room].offset[R_NUMSPRITES],
	                 remSprites);
	
	/* Subtracts half the removed bytes from NumData */
	offset = (long int) (level->room[room].offset[R_NUMVERTICES] - 0x00000004);
	seekval = trmod_fseek(level->file, offset, SEEK_SET);
	if (seekval != 0)
	{
		error->code = ERROR_FILE_READ_FAILED;
		error->string[0] = level->path;
		return;
	}
	readval = trmod_fread(&numData, (size_t) 1, (size_t) 4, level->file);
	if (readval != (size_t) 4)
	{
		error->code = ERROR_FILE_READ_FAILED;
		error->string[0] = level->path;
		return;
	}
	endian32(numData);
	numData -= (remSprites / 0x00000002);
	endian32(numData);
	seekval = trmod_fseek(level->file, offset, SEEK_SET);
	if (seekval != 0)
	{
		error->code = ERROR_FILE_READ_FAILED;
		error->string[0] = level->path;
		return;
	}
	writeval = trmod_fwrite(&numData, (size_t) 1, (size_t) 4, level->file);
	if (writeval != (size_t) 4)
	{
		error->code = ERROR_FILE_WRITE_FAILED;
		error->string[0] = level->path;
		return;
	}
	
	/* Sets NumSprites to zero */
	level->room[room].value[R_NUMSPRITES] = 0x0000;
	remSprites = 0x00000000;
	seekval = trmod_fseek(level->file,
	                      (long int) level->room[room].offset[R_NUMSPRITES],
	                      SEEK_SET);
	if (seekval != 0)
	{
		error->code = ERROR_FILE_READ_FAILED;
		error->string[0] = level->path;
		return;
	}
	writeval = trmod_fwrite(&remSprites, (size_t) 1, (size_t) 2, level->file);
	if (writeval != (size_t) 2)
	{
		error->code = ERROR_FILE_WRITE_FAILED;
		error->string[0] = level->path;
		return;
	}
}

/*
 * Function that prints a viewport
 * Parameters:
 * * level = Pointer to level struct
 * * error = Pointer to error struct
 * * room = Room number
 * * viewport = Viewport number
 * * print = Printing flags
 * * * 0 0 0 0  0 0 0 0   0 0 0 0  0 0 0 0
 * * * | | | |  | | | |   | | | |  | | | |
 * * * | | | |  | | | |   | | | |  | | | As command
 * * * | | | |  | | | |   | | | |  | | As replace command
 * * * Unused
 */
void tr1pc_get_viewport(struct level *level, struct error *error,
                        uint16 room, uint16 viewport, uint16 print)
{
	/* Variable Declarations */
	int16 memviewport[16]; /* In-memory copy of the viewport */
	long int offset;       /* Offset of the viewport */
	int seekval;           /* Return value of fseek() */
	size_t readval;        /* Return value of fread() */
	
	/* Checks whether the room exists */
	if (room >= level->numRooms)
	{
		error->code = ERROR_ROOM_DOESNT_EXIST;
		error->u16[0] = room;
		return;
	}
	/* Checks whether the viewport exists */
	if (viewport >= level->room[room].value[R_NUMDOORS])
	{
		error->code = ERROR_VIEWPORT_DOESNT_EXIST;
		error->u16[0] = room;
		error->u16[1] = viewport;
		return;
	}
	
	/* Reads the viewport to be printed into memory */
	offset = (long int) (viewport * 0x0020);
	offset += (long int) (level->room[room].offset[R_NUMDOORS] + 0x00000002);
	seekval = trmod_fseek(level->file, offset, SEEK_SET);
	if (seekval != 0)
	{
		error->code = ERROR_FILE_READ_FAILED;
		error->string[0] = level->path;
		return;
	}
	readval = trmod_fread(memviewport, (size_t) 1, (size_t) 32, level->file);
	if (readval != (size_t) 32)
	{
		error->code = ERROR_FILE_READ_FAILED;
		error->string[0] = level->path;
		return;
	}
	
	/* Adjusts for Big-Endian */
	if (level->endian == TRUE)
	{
		memviewport[0] = reverse16(memviewport[0]);
		memviewport[1] = reverse16(memviewport[1]);
		memviewport[2] = reverse16(memviewport[2]);
		memviewport[3] = reverse16(memviewport[3]);
		memviewport[4] = reverse16(memviewport[4]);
		memviewport[5] = reverse16(memviewport[5]);
		memviewport[6] = reverse16(memviewport[6]);
		memviewport[7] = reverse16(memviewport[7]);
		memviewport[8] = reverse16(memviewport[8]);
		memviewport[9] = reverse16(memviewport[9]);
		memviewport[10] = reverse16(memviewport[10]);
		memviewport[11] = reverse16(memviewport[11]);
		memviewport[12] = reverse16(memviewport[12]);
		memviewport[13] = reverse16(memviewport[13]);
		memviewport[14] = reverse16(memviewport[14]);
		memviewport[15] = reverse16(memviewport[15]);
	}
	
	/* Converts the Y-coordinates to local */
	memviewport[2] = (level->room[room].yBottom - memviewport[2]);
	memviewport[5] = (level->room[room].yBottom - memviewport[5]);
	memviewport[8] = (level->room[room].yBottom - memviewport[8]);
	memviewport[11] = (level->room[room].yBottom - memviewport[11]);
	memviewport[14] = (level->room[room].yBottom - memviewport[14]);
	
	/* Determines how to print the viewport */
	if ((print & 0x0003) != 0x0000)
	{
		trmod_printf("%s %s %s ", level->command,
		             level->typestring, level->path);
		if ((print & 0x0002) != 0x0000)
		{
			trmod_printf("\"replaceviewport(" PRINTU16 "," PRINTU16 ",",
			             room, viewport);
		}
		else
		{
			trmod_printf("\"addviewport(" PRINTU16 ",", room);
		}
	}
	else
	{
		trmod_printf("Viewport(" PRINTU16 ",", room);
	}
	
	/* Prints the adjoining room and coordinates */
	trmod_printf(PRINT16 ",(" PRINT16 "," PRINT16 "," PRINT16 "),("
	             PRINT16 "," PRINT16 "," PRINT16 "),(" PRINT16 ","
	             PRINT16 "," PRINT16 "),(" PRINT16 "," PRINT16 ","
	             PRINT16 "),(" PRINT16 "," PRINT16 "," PRINT16 "))",
	             memviewport[0], memviewport[1], memviewport[3], memviewport[2],
	             memviewport[4], memviewport[6], memviewport[5], memviewport[7],
	             memviewport[9], memviewport[8], memviewport[10],
	             memviewport[12], memviewport[11], memviewport[13],
	             memviewport[15], memviewport[14]);
	
	/* Ends the printing */
	if ((print & 0x0003) != 0x0000)
	{
		trmod_printf("\"\n");
	}
	else
	{
		trmod_printf("\n");
	}
}

/*
 * Function that adds a viewport
 * Parameters:
 * * level = Pointer to level struct
 * * error = Pointer to error struct
 * * room = Room number
 */
void tr1pc_add_viewport(struct level *level, struct error *error, uint16 room)
{
	/* Variable Declarations */
	int seekval;     /* Return value of fseek() */
	size_t writeval; /* Return value of fwrite() */
	uint16 numDoors; /* Copy of NumDoors to modify */
	
	/* Checks whether the room exists */
	if (room >= level->numRooms)
	{
		error->code = ERROR_ROOM_DOESNT_EXIST;
		error->u16[0] = room;
		return;
	}
	
	/* Makes room for the new viewport */
	insertbytes(level->file, level->path,
	            level->room[room].offset[R_NUMZSECTORS],
	            level->partsize[R_DOOR], error);
	if (error->code != ERROR_NONE)
	{
		return;
	}
	level_struct_add(level, level->room[room].offset[R_NUMZSECTORS],
	                 level->partsize[R_DOOR]);
	
	/* Adds 1 to NumDoors and writes it to the level */
	++(level->room[room].value[R_NUMDOORS]);
	numDoors = level->room[room].value[R_NUMDOORS];
	endian16(numDoors);
	seekval = trmod_fseek(level->file,
	                      (long int) level->room[room].offset[R_NUMDOORS],
	                      SEEK_SET);
	if (seekval != 0)
	{
		error->code = ERROR_FILE_READ_FAILED;
		error->string[0] = level->path;
		return;
	}
	writeval = trmod_fwrite(&numDoors, (size_t) 1, (size_t) 2, level->file);
	if (writeval != (size_t) 2)
	{
		error->code = ERROR_FILE_WRITE_FAILED;
		error->string[0] = level->path;
		return;
	}
}

/*
 * Function that replaces a viewport
 * Parameters:
 * * level = Pointer to level struct
 * * error = Pointer to error struct
 * * room = Room number
 * * viewport = Viewport number
 * * adjroom = Adjoining room
 * * n_x = X-coordinate of the normal (local)
 * * n_y = Y-coordinate of the normal (local)
 * * n_z = Z-coordinate of the normal (local)
 * * v1_x = X-coordinate of the first vertex (local)
 * * v1_y = Y-coordinate of the first vertex (local)
 * * v1_z = Z-coordinate of the first vertex (local)
 * * v2_x = X-coordinate of the second vertex (local)
 * * v2_y = Y-coordinate of the second vertex (local)
 * * v2_z = Z-coordinate of the second vertex (local)
 * * v3_x = X-coordinate of the third vertex (local)
 * * v3_y = Y-coordinate of the third vertex (local)
 * * v3_z = Z-coordinate of the third vertex (local)
 * * v4_x = X-coordinate of the fourth vertex (local)
 * * v4_y = Y-coordinate of the fourth vertex (local)
 * * v4_z = Z-coordinate of the fourth vertex (local)
 * * rep = Flags what to be replaced:
 * * * 0 0 0 0  0 0 0 0   0 0 0 0  0 0 0 0
 * * * | | | |  | | | |   | | | |  | | | |
 * * * | | | |  | | | |   | | | |  | | | Adjoining room
 * * * | | | |  | | | |   | | | |  | | X-coordinate of the normal
 * * * | | | |  | | | |   | | | |  | Y-coordinate of the normal
 * * * | | | |  | | | |   | | | |  Z-coordinate of the normal
 * * * | | | |  | | | |   | | | X-coordinate of the first vertex
 * * * | | | |  | | | |   | | Y-coordinate of the first vertex
 * * * | | | |  | | | |   | Z-coordinate of the first vertex
 * * * | | | |  | | | |   X-coordinate of the second vertex
 * * * | | | |  | | | Y-coordinate of the second vertex
 * * * | | | |  | | Z-coordinate of the second vertex
 * * * | | | |  | X-coordinate of the third vertex
 * * * | | | |  Y-coordinate of the third vertex
 * * * | | | Z-coordinate of the third vertex
 * * * | | X-coordinate of the fourth vertex
 * * * | Y-coordinate of the fourth vertex
 * * * Z-coordinate of the fourth vertex
 */
void tr1pc_replace_viewport(struct level *level, struct error *error,
                            uint16 room, uint16 viewport, uint16 adjroom,
                            int16 n_x, int16 n_y, int16 n_z, int16 v1_x,
                            int16 v1_y, int16 v1_z, int16 v2_x, int16 v2_y,
                            int16 v2_z, int16 v3_x, int16 v3_y, int16 v3_z,
                            int16 v4_x, int16 v4_y, int16 v4_z, uint16 rep)
{
	/* Variable Declarations */
	int16 memviewport[16]; /* In-memory copy of the viewport */
	long int offset;        /* Offset of the viewport */
	int seekval;            /* Return value of fseek() */
	size_t readval;         /* Return value of fread() */
	size_t writeval;        /* Return value of fwrite() */
	
	/* Checks whether the room exists */
	if (room >= level->numRooms)
	{
		error->code = ERROR_ROOM_DOESNT_EXIST;
		error->u16[0] = room;
		return;
	}
	/* Sets viewport to the last one if max int */
	if (viewport == 0xFFFF)
	{
		viewport = level->room[room].value[R_NUMDOORS];
		--viewport;
	}
	/* Checks whether the viewport exists */
	if (viewport >= level->room[room].value[R_NUMDOORS])
	{
		error->code = ERROR_VIEWPORT_DOESNT_EXIST;
		error->u16[0] = room;
		error->u16[1] = viewport;
		return;
	}
	
	/* Reads the viewport to be modified into memory */
	offset = (long int) (viewport * 0x0020);
	offset += (long int) (level->room[room].offset[R_NUMDOORS] + 0x00000002);
	seekval = trmod_fseek(level->file, offset, SEEK_SET);
	if (seekval != 0)
	{
		error->code = ERROR_FILE_READ_FAILED;
		error->string[0] = level->path;
		return;
	}
	readval = trmod_fread(memviewport, (size_t) 1, (size_t) 32, level->file);
	if (readval != (size_t) 32)
	{
		error->code = ERROR_FILE_READ_FAILED;
		error->string[0] = level->path;
		return;
	}
	
	/* Adjusts for Big-Endian */
	if (level->endian == TRUE)
	{
		memviewport[0] = reverse16(memviewport[0]);
		memviewport[1] = reverse16(memviewport[1]);
		memviewport[2] = reverse16(memviewport[2]);
		memviewport[3] = reverse16(memviewport[3]);
		memviewport[4] = reverse16(memviewport[4]);
		memviewport[5] = reverse16(memviewport[5]);
		memviewport[6] = reverse16(memviewport[6]);
		memviewport[7] = reverse16(memviewport[7]);
		memviewport[8] = reverse16(memviewport[8]);
		memviewport[9] = reverse16(memviewport[9]);
		memviewport[10] = reverse16(memviewport[10]);
		memviewport[11] = reverse16(memviewport[11]);
		memviewport[12] = reverse16(memviewport[12]);
		memviewport[13] = reverse16(memviewport[13]);
		memviewport[14] = reverse16(memviewport[14]);
		memviewport[15] = reverse16(memviewport[15]);
	}
	
	/* Converts the Y-coordinates to local */
	memviewport[2] = (level->room[room].yBottom - memviewport[2]);
	memviewport[5] = (level->room[room].yBottom - memviewport[5]);
	memviewport[8] = (level->room[room].yBottom - memviewport[8]);
	memviewport[11] = (level->room[room].yBottom - memviewport[11]);
	memviewport[14] = (level->room[room].yBottom - memviewport[14]);
	
	/* Replaces which elements that need to be replaced */
	if ((rep & 0x0001) != 0x0000)
	{
		memviewport[0] = (int16) adjroom;
	}
	if ((rep & 0x0002) != 0x0000)
	{
		memviewport[1] = n_x;
	}
	if ((rep & 0x0004) != 0x0000)
	{
		memviewport[2] = n_y;
	}
	if ((rep & 0x0008) != 0x0000)
	{
		memviewport[3] = n_z;
	}
	if ((rep & 0x0010) != 0x0000)
	{
		memviewport[4] = v1_x;
	}
	if ((rep & 0x0020) != 0x0000)
	{
		memviewport[5] = v1_y;
	}
	if ((rep & 0x0040) != 0x0000)
	{
		memviewport[6] = v1_z;
	}
	if ((rep & 0x0080) != 0x0000)
	{
		memviewport[7] = v2_x;
	}
	if ((rep & 0x0100) != 0x0000)
	{
		memviewport[8] = v2_y;
	}
	if ((rep & 0x0200) != 0x0000)
	{
		memviewport[9] = v2_z;
	}
	if ((rep & 0x0400) != 0x0000)
	{
		memviewport[10] = v3_x;
	}
	if ((rep & 0x0800) != 0x0000)
	{
		memviewport[11] = v3_y;
	}
	if ((rep & 0x1000) != 0x0000)
	{
		memviewport[12] = v3_z;
	}
	if ((rep & 0x2000) != 0x0000)
	{
		memviewport[13] = v4_x;
	}
	if ((rep & 0x4000) != 0x0000)
	{
		memviewport[14] = v4_y;
	}
	if ((rep & 0x8000) != 0x0000)
	{
		memviewport[15] = v4_z;
	}
	
	/* Converts the Y-coordinates to local */
	memviewport[2] = (level->room[room].yBottom - memviewport[2]);
	memviewport[5] = (level->room[room].yBottom - memviewport[5]);
	memviewport[8] = (level->room[room].yBottom - memviewport[8]);
	memviewport[11] = (level->room[room].yBottom - memviewport[11]);
	memviewport[14] = (level->room[room].yBottom - memviewport[14]);
	
	/* Adjusts for Big-Endian */
	if (level->endian == TRUE)
	{
		memviewport[0] = reverse16(memviewport[0]);
		memviewport[1] = reverse16(memviewport[1]);
		memviewport[2] = reverse16(memviewport[2]);
		memviewport[3] = reverse16(memviewport[3]);
		memviewport[4] = reverse16(memviewport[4]);
		memviewport[5] = reverse16(memviewport[5]);
		memviewport[6] = reverse16(memviewport[6]);
		memviewport[7] = reverse16(memviewport[7]);
		memviewport[8] = reverse16(memviewport[8]);
		memviewport[9] = reverse16(memviewport[9]);
		memviewport[10] = reverse16(memviewport[10]);
		memviewport[11] = reverse16(memviewport[11]);
		memviewport[12] = reverse16(memviewport[12]);
		memviewport[13] = reverse16(memviewport[13]);
		memviewport[14] = reverse16(memviewport[14]);
		memviewport[15] = reverse16(memviewport[15]);
	}
	
	/* Overwrites the viewport */
	seekval = trmod_fseek(level->file, offset, SEEK_SET);
	if (seekval != 0)
	{
		error->code = ERROR_FILE_READ_FAILED;
		error->string[0] = level->path;
		return;
	}
	writeval = trmod_fwrite(memviewport, (size_t) 1, (size_t) 32, level->file);
	if (writeval != (size_t) 32)
	{
		error->code = ERROR_FILE_WRITE_FAILED;
		error->string[0] = level->path;
		return;
	}
}

/*
 * Function that removes a viewport
 * Parameters:
 * * level = Pointer to level struct
 * * error = Pointer to error struct
 * * room = Room number
 * * viewport = Viewport number
 */
void tr1pc_remove_viewport(struct level *level, struct error *error,
                           uint16 room, uint16 viewport)
{
	/* Variable Declarations */
	uint32 offset;   /* Offset of the viewport */
	int seekval;     /* Return value of fseek() */
	size_t writeval; /* Return value of fwrite() */
	uint16 numDoors; /* Copy of NumDoors to modify */
	
	/* Checks whether the room exists */
	if (room >= level->numRooms)
	{
		error->code = ERROR_ROOM_DOESNT_EXIST;
		error->u16[0] = room;
		return;
	}
	/* Checks whether the viewport exists */
	if (viewport >= level->room[room].value[R_NUMDOORS])
	{
		error->code = ERROR_VIEWPORT_DOESNT_EXIST;
		error->u16[0] = room;
		error->u16[1] = viewport;
		return;
	}
	
	/* Makes room for the new viewport */
	offset = (uint32) (viewport * 0x0020);
	offset += level->room[room].offset[R_NUMDOORS];
	offset += 0x00000002;
	removebytes(level->file, level->path, offset, 0x00000020, error);
	if (error->code != ERROR_NONE)
	{
		return;
	}
	level_struct_sub(level, offset, 0x00000020);
	
	/* Subtracts 1 from NumDoors and writes it to the level */
	--(level->room[room].value[R_NUMDOORS]);
	numDoors = level->room[room].value[R_NUMDOORS];
	endian16(numDoors);
	seekval = trmod_fseek(level->file,
	                      (long int) level->room[room].offset[R_NUMDOORS],
	                      SEEK_SET);
	if (seekval != 0)
	{
		error->code = ERROR_FILE_READ_FAILED;
		error->string[0] = level->path;
		return;
	}
	writeval = trmod_fwrite(&numDoors, (size_t) 1, (size_t) 2, level->file);
	if (writeval != (size_t) 2)
	{
		error->code = ERROR_FILE_WRITE_FAILED;
		error->string[0] = level->path;
		return;
	}
}

/*
 * Function that removes all viewports from a room
 * Parameters:
 * * level = Pointer to level struct
 * * error = Pointer to error struct
 * * room = Room to remove all viewports from
 */
void tr1pc_remove_all_viewports(struct level *level,
                                struct error *error, uint16 room)
{
	/* Variable Declarations */
	uint32 offset;   /* Offset of the viewports to remove */
	uint32 size;     /* Number of bytes to remove */
	int seekval;     /* Return value of fseek() */
	size_t writeval; /* Return value of fwrite() */
	
	/* Checks whether the room exists */
	if (room >= level->numRooms)
	{
		error->code = ERROR_ROOM_DOESNT_EXIST;
		error->u16[0] = room;
		return;
	}
	
	/* Stops if there are no viewports in this room */
	if (level->room[room].value[R_NUMDOORS] == 0x0000)
	{
		return;
	}
	
	/* Removes the viewports */
	offset = (level->room[room].offset[R_NUMDOORS] + 0x00000002);
	size = (level->room[room].offset[R_NUMZSECTORS] -
	        level->room[room].offset[R_NUMDOORS] - 0x00000002);
	removebytes(level->file, level->path, offset, size, error);
	if (error->code != ERROR_NONE)
	{
		return;
	}
	level_struct_sub(level, offset, size);
	
	/* Sets NumDoors to zero */
	offset = 0x00000000;
	
	/* Writes NumDoors back to the level file */
	seekval = trmod_fseek(level->file,
	                      (long int) level->room[room].offset[R_NUMDOORS],
	                      SEEK_SET);
	if (seekval != 0)
	{
		error->code = ERROR_FILE_READ_FAILED;
		error->string[0] = level->path;
		return;
	}
	writeval = trmod_fwrite(&offset, (size_t) 1, (size_t) 2, level->file);
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
void tr1pc_get_roomlight(struct level *level, struct error *error,
                         uint16 room, uint16 print)
{
	/* Variable Declarations */
	long int offset; /* Offset to read from */
	int16 roomlight; /* Roomlight value */
	int seekval;     /* Return value of fseek() */
	size_t readval;  /* Return value of fread() */
	
	/* Checks whether the room exists */
	if (room >= level->numRooms)
	{
		error->code = ERROR_ROOM_DOESNT_EXIST;
		error->u16[0] = room;
		return;
	}
	
	/* Reads the roomlight value into memory */
	offset = (long int) (level->room[room].offset[R_NUMLIGHTS] - 0x00000002);
	seekval = trmod_fseek(level->file, offset, SEEK_SET);
	if (seekval != 0)
	{
		error->code = ERROR_FILE_READ_FAILED;
		error->string[0] = level->path;
		return;
	}
	readval = trmod_fread(&roomlight, (size_t) 1, (size_t) 2, level->file);
	if (readval != (size_t) 2)
	{
		error->code = ERROR_FILE_READ_FAILED;
		error->string[0] = level->path;
		return;
	}
	endian16(roomlight);
	
	/* Converts intensity if needed */
	if (roomlight != (int16) 0xFFFF)
	{
		roomlight = (int16) (0x1FFF - roomlight);
	}
	
	/* Determines how to print the roomlight */
	if ((print & 0x0001) != 0x0000)
	{
		trmod_printf("%s %s %s \"roomlight(" PRINTU16 "," PRINT16 "/8191)\"\n",
		             level->command, level->typestring,
		             level->path, room, roomlight);
	}
	else
	{
		trmod_printf("RoomLight(" PRINTU16 "," PRINT16 "/8191)\n",
		             room, roomlight);
	}
}

/*
 * Function that changes the roomlight value of a given room
 * Parameters:
 * * level = Pointer to level struct
 * * error = Pointer to error struct
 * * room = Room number
 * * inten = Roomlight value (8191-0)
 */
void tr1pc_roomlight(struct level *level, struct error *error,
                     uint16 room, int16 roomlight)
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
	offset = (long int) (level->room[room].offset[R_NUMLIGHTS] - 0x00000002);
	seekval = trmod_fseek(level->file, offset, SEEK_SET);
	if (seekval != 0)
	{
		error->code = ERROR_FILE_READ_FAILED;
		error->string[0] = level->path;
		return;
	}
	endian16(roomlight);
	writeval = trmod_fwrite(&roomlight, (size_t) 1, (size_t) 2, level->file);
	if (writeval != (size_t) 2)
	{
		error->code = ERROR_FILE_WRITE_FAILED;
		error->string[0] = level->path;
		return;
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
void tr1pc_getbox(struct level *level, struct error *error,
                  uint32 box, uint16 print)
{
	/* Variable Declarations */
	int32 boxcrd[4];         /* Coordinates from the box */
	int16 truefloor;         /* TrueFloor from the box */
	uint16 overlapindex;     /* OverlapIndex from the box */
	int16 zone[6];           /* The zones */
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
	offset = (long int) (box * 0x00000014);
	offset += (long int) (level->offset[NUMBOXES] + 0x00000004);
	seekval = trmod_fseek(level->file, offset, SEEK_SET);
	if (seekval != 0)
	{
		error->code = ERROR_FILE_READ_FAILED;
		error->string[0] = level->path;
		goto end;
	}
	readval = trmod_fread(boxcrd, (size_t) 1, (size_t) 16, level->file);
	if (readval != (size_t) 16)
	{
		error->code = ERROR_FILE_READ_FAILED;
		error->string[0] = level->path;
		goto end;
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
	for (i = 0; i < 6; ++i)
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
		boxcrd[0] = reverse32(boxcrd[0]);
		boxcrd[1] = reverse32(boxcrd[1]);
		boxcrd[2] = reverse32(boxcrd[2]);
		boxcrd[3] = reverse32(boxcrd[3]);
		truefloor = reverse16(truefloor);
		overlapindex = reverse16(overlapindex);
		zone[0] = reverse16(zone[0]);
		zone[1] = reverse16(zone[1]);
		zone[2] = reverse16(zone[2]);
		zone[3] = reverse16(zone[3]);
		zone[4] = reverse16(zone[4]);
		zone[5] = reverse16(zone[5]);
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
		room = tr1pc_nearestRoom(level, boxcrd[2], boxcrd[0],
		                         (int32) truefloor);
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
	trmod_printf("(" PRINT16 "," PRINT16 ",0,0," PRINT16 "),(" PRINT16 ","
	             PRINT16 ",0,0," PRINT16 "),", zone[0], zone[1], zone[2],
	             zone[3], zone[4], zone[5]);
	
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
void tr1pc_add_box(struct level *level, struct error *error)
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
	for (i = 0; i < 6; ++i)
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
	            0x00000014, error);
	if (error->code != ERROR_NONE)
	{
		return;
	}
	level_struct_add(level, level->offset[NUMOVERLAPS], 0x00000014);
	
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
 * * fz = Fly Zone
 * * agz1 = Alternate Ground Zone 1
 * * agz2 = Alternate Ground Zone 2
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
 * * * | | | |  | | | |  | | | |  | | | |  | | | |  | | | |  Unused
 * * * | | | |  | | | |  | | | |  | | | |  | | | |  | | | Unused
 * * * | | | |  | | | |  | | | |  | | | |  | | | |  | | Fly Zone
 * * * | | | |  | | | |  | | | |  | | | |  | | | |  | Alt Ground Zone 1
 * * * | | | |  | | | |  | | | |  | | | |  | | | |  Alt Ground Zone 2
 * * * | | | |  | | | |  | | | |  | | | |  | | | Unused
 * * * | | | |  | | | |  | | | |  | | | |  | | Unused
 * * * | | | |  | | | |  | | | |  | | | |  | Alt Fly Zone
 * * * | | | |  | | | |  | | | |  | | | |  Overlaps
 * * * | | | |  | | | |  | | | |  | | | Blocked-flag
 * * * | | | |  | | | |  | | | |  | | Blockable-flag
 * * * Unused
 */
void tr1pc_replace_box(struct level *level, struct error *error, uint32 box,
                       uint16 room, int32 xmin, int32 xmax, int32 zmin,
                       int32 zmax, int16 floor, int16 gz1, int16 gz2, int16 fz,
                       int16 agz1, int16 agz2, int16 afz, char *overlapstr,
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
	
	/* Adjusts for Big-Endian */
	if (level->endian == TRUE)
	{
		xmin = reverse32(xmin);
		xmax = reverse32(xmax);
		zmin = reverse32(zmin);
		zmax = reverse32(zmax);
		floor = reverse16(floor);
		gz1 = reverse16(gz1);
		gz2 = reverse16(gz2);
		fz = reverse16(fz);
		agz1 = reverse16(agz1);
		agz2 = reverse16(agz2);
		afz = reverse16(afz);
	}
	
	/* Replaces coordinates if needed */
	offset = (long int) (box * 0x00000014);
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
		writeval = trmod_fwrite(&zmin, (size_t) 1, (size_t) 4, level->file);
		if (writeval != (size_t) 4)
		{
			error->code = ERROR_FILE_WRITE_FAILED;
			error->string[0] = level->path;
			goto end;
		}
	}
	offset += 4l;
	if ((rep & 0x00000008) != 0x00000000)
	{
		seekval = trmod_fseek(level->file, offset, SEEK_SET);
		if (seekval != 0)
		{
			error->code = ERROR_FILE_READ_FAILED;
			error->string[0] = level->path;
			goto end;
		}
		writeval = trmod_fwrite(&zmax, (size_t) 1, (size_t) 4, level->file);
		if (writeval != (size_t) 4)
		{
			error->code = ERROR_FILE_WRITE_FAILED;
			error->string[0] = level->path;
			goto end;
		}
	}
	offset += 4l;
	if ((rep & 0x00000001) != 0x00000000)
	{
		seekval = trmod_fseek(level->file, offset, SEEK_SET);
		if (seekval != 0)
		{
			error->code = ERROR_FILE_READ_FAILED;
			error->string[0] = level->path;
			goto end;
		}
		writeval = trmod_fwrite(&xmin, (size_t) 1, (size_t) 4, level->file);
		if (writeval != (size_t) 4)
		{
			error->code = ERROR_FILE_WRITE_FAILED;
			error->string[0] = level->path;
			goto end;
		}
	}
	offset += 4l;
	if ((rep & 0x00000002) != 0x00000000)
	{
		seekval = trmod_fseek(level->file, offset, SEEK_SET);
		if (seekval != 0)
		{
			error->code = ERROR_FILE_READ_FAILED;
			error->string[0] = level->path;
			goto end;
		}
		writeval = trmod_fwrite(&xmax, (size_t) 1, (size_t) 4, level->file);
		if (writeval != (size_t) 4)
		{
			error->code = ERROR_FILE_WRITE_FAILED;
			error->string[0] = level->path;
			goto end;
		}
	}
	offset += 4l;
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
			overlaps = calloc((size_t) level->value[NUMOVERLAPS], (size_t) 2);
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
			if (readval != (size_t) (level->value[NUMOVERLAPS] * 0x00000002))
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
			level_struct_add(level, level->offset[ZONE], (uint32) numolsubbyt);
			
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
		offset = (long int) (box * 0x00000014);
		offset += (long int) (level->offset[NUMBOXES] + 0x00000016);
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
	if ((rep & 0x00030000) != 0x00000000)
	{
		/* Reads the overlapindex */
		offset = (long int) (box * 0x00000014);
		offset += (long int) (level->offset[NUMBOXES] + 0x00000016);
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
			overlapindex &= 0x3FFFU;
			if (blocked == 1)
			{
				overlapindex |= 0x4000U;
			}
			if (blockable == 1)
			{
				overlapindex |= 0x8000U;
			}
			endian16(overlapindex);
			
			/* Writes the overlapindex */
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
 * Function that removes all boxes and overlaps from the level
 * Parameters:
 * * level = Pointer to level struct
 * * error = Pointer to error struct
 */
void tr1pc_remove_all_boxes(struct level *level, struct error *error)
{
	/* Variable Declarations */
	uint32 offset;   /* Where to remove bytes from */
	uint32 numbytes; /* Number of bytes to remove */
	int seekval;     /* Return value of fseek() */
	size_t writeval; /* Return value of fwrite() */
	
	/* Determines how many bytes to remove */
	numbytes = (level->offset[NUMANIMTEXTURES] - level->offset[NUMBOXES]);
	numbytes -= 0x00000008;
	if (numbytes == 0x00000000)
	{
		return;
	}
	
	/* Removes the boxes and overlaps */
	removebytes(level->file, level->path,
	            level->offset[NUMBOXES], numbytes, error);
	if (error->code != ERROR_NONE)
	{
		return;
	}
	level_struct_sub(level, level->offset[NUMBOXES], numbytes);
	
	/* Sets NumBoxes and NumOverlaps to zero */
	level->value[NUMBOXES] = 0x00000000;
	level->value[NUMOVERLAPS] = 0x00000000;
	level->offset[NUMOVERLAPS] = (level->offset[NUMBOXES] + 0x00000004);
	level->offset[ZONE] = (level->offset[NUMOVERLAPS] + 0x00000004);
	
	/* Writes zeroes to NumBoxes and NumOverlaps */
	seekval = trmod_fseek(level->file,
	                      (long int) level->offset[NUMBOXES], SEEK_SET);
	if (seekval != 0)
	{
		error->code = ERROR_FILE_READ_FAILED;
		error->string[0] = level->path;
		return;
	}
	offset = 0x00000000;
	writeval = trmod_fwrite(&offset, (size_t) 1, (size_t) 4, level->file);
	if (writeval != (size_t) 4)
	{
		error->code = ERROR_FILE_WRITE_FAILED;
		error->string[0] = level->path;
		return;
	}
	writeval = trmod_fwrite(&offset, (size_t) 1, (size_t) 4, level->file);
	if (writeval != (size_t) 4)
	{
		error->code = ERROR_FILE_WRITE_FAILED;
		error->string[0] = level->path;
		return;
	}
}

/*
 * Function that prints a hex dump of the level
 * Parameters:
 * * level = Pointer to the level struct
 * * error = Pointer to the error struct
 * * print = Printing flags
 * * * 0 0 0 0  0 0 0 0   0 0 0 0  0 0 0 0
 * * * | | | |  | | | |   | | | |  | | | |
 * * * | | | |  | | | |   | | | |  | | | Add titles to Num-something
 * * * | | | |  | | | |   | | | |  | | Add titles to everything
 * * * | | | |  | | | |   | | | |  | Add offsets
 * * * Unused
 */
void tr1pc_hexdump(struct level *level, struct error *error, uint16 print)
{
	/* Macro that applies the print flags */
#define hexdump_trmod_printflags(itemnum)                           \
	if ((print & 0x0004U) != 0x0000U)                               \
	{                                                               \
		trmod_printf("[%08lX] ", curpos);                           \
	}                                                               \
	if (level->nav[navcount] < NUMKEEPPARTS)                        \
	{                                                               \
		if ((print & 0x0003U) != 0x0000U)                           \
		{                                                           \
			trmod_printf("%s: ", partnames[level->nav[navcount]]);  \
		}                                                           \
	}                                                               \
	else if (level->nav[navcount] < NUMLEVPARTS)                    \
	{                                                               \
		if ((print & 0x0002U) != 0x0000U)                           \
		{                                                           \
			trmod_printf("%s(" PRINTU32 "): ",                      \
			             partnames[level->nav[navcount]], itemnum); \
		}                                                           \
	}                                                               \
	else if (((level->nav[navcount] == SKIP_BYTES) ||               \
	          (level->nav[navcount] == SKIP_VAR) ||                 \
	          (level->nav[navcount] == SKIP_READ32)) &&             \
	          (print & 0x0003U) != 0x0000U)                         \
	{                                                               \
		trmod_printf("Skipped bytes(" PRINTU32 "): ", itemnum);     \
	}
	
	/* Variable Declarations */
	long int curpos = 0l;         /* Current position in the level file */
	long int curpostarget = 0l;   /* Next position in the level file */
	long int levelsize = 0l;      /* Size of the level file */
	int seekval = 0;              /* Return value of fseek() */
	size_t readval = (size_t) 0U; /* Return value of fread() */
	uint16 navcount = 0x0000U;    /* Current position in the navigation array */
	uint16 startroom = 0x0000U;   /* Where in the nav array rooms start */
	uint16 curRoom = 0x0000U;     /* Current room */
	uint32 iftype = 0x00000000;   /* Condition for the if-statement */
	uint32 c_param[4];            /* Parameters for the if-statment */
	uint16 c_filledparams;        /* Number of filled-in parameters */
	int16  ifdepth;               /* Current depth of condition */
	uint16 i = 0x0000U;           /* Counter variables */
	uint16 j = 0x0000U;           /* Counter variables */
	uint32 k = 0x00000000U;       /* Counter variable */
	
	/* Determines level size */
	seekval = trmod_fseek(level->file, 0l, SEEK_END);
	if (seekval != 0)
	{
		error->code = ERROR_FILE_READ_FAILED;
		error->string[0] = level->path;
		goto end;
	}
	levelsize = trmod_ftell(level->file);
	
	/* Allocates inmem */
	level->inmem = calloc((size_t) levelsize, (size_t) 1U);
	if (level->inmem == NULL)
	{
		error->code = ERROR_MEMORY;
		goto end;
	}
	
	/* Reads the level file into memory */
	seekval = trmod_fseek(level->file, 0l, SEEK_SET);
	if (seekval != 0)
	{
		error->code = ERROR_FILE_READ_FAILED;
		error->string[0] = level->path;
		goto end;
	}
	readval = trmod_fread(level->inmem, (size_t) 1U,
	                      (size_t) levelsize, level->file);
	if (readval != (size_t) levelsize)
	{
		error->code = ERROR_FILE_READ_FAILED;
		error->string[0] = level->path;
		goto end;
	}
	
	/* Goes through the level */
	curpos = 0l;
	for (navcount = 0x0000U; level->nav[navcount] != END_LEVEL; ++navcount)
	{
		/* Speacial Cases */
		/* Special thing to skip. Get the length from the next element */
		if (level->nav[navcount] == SKIP_BYTES)
		{
			hexdump_trmod_printflags(level->nav[(navcount + 0x0001U)]);
			++navcount;
			curpostarget = (curpos + level->nav[navcount]);
			for (; curpos < curpostarget; ++curpos)
			{
				trmod_printf("%02X ", level->inmem[curpos]);
			}
			trmod_printf("\n");
		}
		else if (level->nav[navcount] == SKIP_READ32)
		{
			hexdump_trmod_printflags(level->nav[(navcount + 0x0001U)]);
			memcpy(&k, &(level->inmem[curpos]), (size_t) 4);
			endian32(k);
			++navcount;
			curpostarget = (curpos + 0x00000004U + (k * level->nav[navcount]));
			for (; curpos < curpostarget; ++curpos)
			{
				trmod_printf("%02X ", level->inmem[curpos]);
			}
			trmod_printf("\n");
		}
		else if (level->nav[navcount] == SKIP_VAR)
		{
			++navcount;
			if (level->nav[navcount] < NUMKEEPPARTS)
			{
				hexdump_trmod_printflags(0x00000000U);
				curpostarget = (curpos + level->value[level->nav[navcount]]);
				for (; curpos < curpostarget; ++curpos)
				{
					trmod_printf("%02X ", level->inmem[curpos]);
				}
				trmod_printf("\n");
			}
			else
			{
				error->code = ERROR_INVALID_NAVARRAY;
				goto end;
			}
		}
		/* Marker for the start of the room struct */
		else if (level->nav[navcount] == START_ROOM)
		{
			hexdump_trmod_printflags(0);
			startroom = navcount;
			if ((print & 0x0003U) != 0x0000U)
			{
				trmod_printf("Room(0)\n");
			}
		}
		/* Marker for the end of the room struct */
		else if (level->nav[navcount] == END_ROOM)
		{
			/* Parse another room if needed */
			++curRoom;
			if (curRoom < level->numRooms)
			{
				navcount = startroom;
				hexdump_trmod_printflags(0);
				if ((print & 0x0003U) != 0x0000U)
				{
					trmod_printf("Room(" PRINTU16 ")\n", curRoom);
				}
			}
		}
		/* Special case for sector */
		else if (level->nav[navcount] == R_SECTOR)
		{
			for (i = 0x0000U; i < level->room[curRoom].value[R_NUMXSECTORS];
			     ++i)
			{
				for (j = 0x0000U; j < level->room[curRoom].value[R_NUMZSECTORS];
				     ++j)
				{
					if ((print & 0x0004U) != 0x0000U)
					{
						trmod_printf("[%08lX] ", curpos);
					}
					if ((print & 0x0003U) != 0x0000U)
					{
						trmod_printf("Sector(" PRINTU16 "," PRINTU16 "): ",
						             i, j);
					}
					curpostarget = (curpos + level->partsize[R_SECTOR]);
					for (;curpos < curpostarget; ++curpos)
					{
						trmod_printf("%02X ", level->inmem[curpos]);
					}
					trmod_printf("\n");
				}
			}
		}
		/* Special case for zone */
		else if (level->nav[navcount] == ZONE)
		{
			level->offset[ZONE] = curpos;
			curpostarget = (curpos + (level->value[NUMBOXES] *
			                          level->partsize[ZONE]));
			if ((print & 0x0004U) != 0x0000U)
			{
				trmod_printf("[%08lX] ", curpos);
			}
			if ((print & 0x0003U) != 0x0000U)
			{
				trmod_printf("Zones: ");
			}
			for (; curpos < curpostarget; ++curpos)
			{
				trmod_printf("%02X ", level->inmem[curpos]);
			}
			trmod_printf("\n");
		}
		/* Special case for set flag */
		else if (level->nav[navcount] == NAV_SET_FLAG)
		{
			++navcount;
		}
		/* Speacial case for VAR_TO_PARAM0 */
		else if (level->nav[navcount] == VAR_TO_PARAM0)
		{
			++navcount;
			level->value[level->nav[navcount]] = c_param[0];
		}
		/* Special case for conditionals */
		else if ((level->nav[navcount] == N_IF_EQUAL)    ||
		         (level->nav[navcount] == N_IF_GREATER)  ||
		         (level->nav[navcount] == N_IF_SMALLER))
		{
			/* Sets up variables for the conditions */
			iftype = level->nav[navcount];
			c_param[0] = 0x00000000U;
			c_param[1] = 0x00000000U;
			c_filledparams = 0;
			++navcount;
			for (; level->nav[navcount] != N_THEN; ++navcount)
			{
				if (level->nav[navcount] == N_IMM)
				{
					++navcount;
					c_param[c_filledparams] = level->nav[navcount];
					++c_filledparams;
				}
				else if (level->nav[navcount] == N_AND)
				{
					if (c_filledparams < 0x0002U)
					{
						error->code = ERROR_INVALID_NAVARRAY;
						goto end;
					}
					c_param[(c_filledparams - 0x0002U)] &=
						c_param[(c_filledparams - 0x0001U)];
					--c_filledparams;
				}
				else if (level->nav[navcount] == N_OR)
				{
					if (c_filledparams < 0x0002U)
					{
						error->code = ERROR_INVALID_NAVARRAY;
						goto end;
					}
					c_param[(c_filledparams - 0x0002U)] |=
						c_param[(c_filledparams - 0x0001U)];
					--c_filledparams;
				}
				else if (level->nav[navcount] == N_READ32)
				{
					memcpy(&(c_param[c_filledparams]),
					       &(level->inmem[curpos]), (size_t) 4);
					endian32(c_param[c_filledparams]);
					if ((print & 0x0004U) != 0x0000U)
					{
						trmod_printf("[%08lX] ", curpos);
					}
					if ((print & 0x0003U) != 0x0000U)
					{
						trmod_printf("Read bytes(4): ");
					}
					curpostarget = (curpos + 0x00000004U);
					for (; curpos < curpostarget; ++curpos)
					{
						trmod_printf("%02X ", level->inmem[curpos]);
					}
					trmod_printf("\n");
					++c_filledparams;
				}
				else if (level->nav[navcount] == N_READ16)
				{
					memcpy(&(c_param[c_filledparams]),
					       &(level->inmem[curpos]), (size_t) 2);
					endian32(c_param[c_filledparams]);
					c_param[c_filledparams] &= 0x0000FFFFU;
					if ((print & 0x0004U) != 0x0000U)
					{
						trmod_printf("[%08lX] ", curpos);
					}
					if ((print & 0x0003U) != 0x0000U)
					{
						trmod_printf("Read bytes(2): ");
					}
					curpostarget = (curpos + 0x00000002U);
					for (; curpos < curpostarget; ++curpos)
					{
						trmod_printf("%02X ", level->inmem[curpos]);
					}
					trmod_printf("\n");
					++c_filledparams;
				}
				else if (level->nav[navcount] < NUMROOMPARTNUMS)
				{
					c_param[c_filledparams] = (uint32) (level->room[curRoom].
						value[level->nav[navcount]]);
					++c_filledparams;
				}
				else if (level->nav[navcount] < NUMLEVPARTS)
				{
					c_param[c_filledparams] =
						level->value[level->nav[navcount]];
					++c_filledparams;
				}
			}
			
			/* Checks if the condition applies */
			if (((iftype == N_IF_EQUAL) && (c_param[0] == c_param[1]))  ||
			    ((iftype == N_IF_GREATER) && (c_param[0] > c_param[1])) ||
			    ((iftype == N_IF_SMALLER) && (c_param[0] < c_param[1])))
			{
				/* The condition applies */
				continue;
			}
			else
			{
				/* Skips to the N_ELSE */
				ifdepth = 0x0000;
				for (; level->nav[navcount] != END_LEVEL; ++navcount)
				{
					if (((level->nav[navcount] == N_ELSE) ||
					     (level->nav[navcount] == N_ENDIF)) &&
					    (ifdepth == 0x0000))
					{
						break;
					}
					else if ((level->nav[navcount] == N_IF_EQUAL) ||
					         (level->nav[navcount] == N_IF_GREATER) ||
					         (level->nav[navcount] == N_IF_SMALLER))
					{
						++ifdepth;
					}
					else if (level->nav[navcount] == N_ENDIF)
					{
						--ifdepth;
					}
				}
				continue;
			}
		}
		/* Special case for else */
		else if (level->nav[navcount] == N_ELSE)
		{
			/* Skips to the N_ENDIF */
			ifdepth = 0x0000;
			for (; level->nav[navcount] != END_LEVEL; ++navcount)
			{
				if ((level->nav[navcount] == N_ENDIF) &&
				    (ifdepth == 0x0000))
				{
					break;
				}
				else if ((level->nav[navcount] == N_IF_EQUAL) ||
				         (level->nav[navcount] == N_IF_GREATER) ||
				         (level->nav[navcount] == N_IF_SMALLER))
				{
					++ifdepth;
				}
				else if (level->nav[navcount] == N_ENDIF)
				{
					--ifdepth;
				}
			}
			continue;
		}
		
		/* A Num-something to be read and kept */
		else if (level->nav[navcount] < NUMKEEPPARTS)
		{
			hexdump_trmod_printflags(0);
			curpostarget = (curpos+ level->partsize[level->nav[navcount]]);
			for (; curpos < curpostarget; ++curpos)
			{
				trmod_printf("%02X ", level->inmem[curpos]);
			}
			trmod_printf("\n");
		}
		
		/* An actual element */
		else if (level->nav[navcount] < NUMLEVPARTS)
		{
			if (level->nav[navcount] < NUMROOMPARTS)
			{
				if (level->partsize[level->nav[navcount]] <= 0x00000002U)
				{
					curpostarget = ((level->partsize[level->nav[navcount]] *
					                 level->room[curRoom].value[level->
					                 partnum[level->nav[navcount]]]) + curpos);
					hexdump_trmod_printflags(0);
					for (; curpos < curpostarget; ++curpos)
					{
						trmod_printf("%02X ", level->inmem[curpos]);
					}
					trmod_printf("\n");
				}
				else
				{
					for (i = 0x0000U; i < level->room[curRoom].value[level->
					                      partnum[level->nav[navcount]]]; ++i)
					{
						hexdump_trmod_printflags((uint32) i);
						curpostarget = (curpos +
						                level->partsize[level->nav[navcount]]);
						for (; curpos < curpostarget; ++curpos)
						{
							trmod_printf("%02X ", level->inmem[curpos]);
						}
						trmod_printf("\n");
					}
				}
			}
			else
			{
				if (level->partsize[level->nav[navcount]] <= 0x00000002U)
				{
					curpostarget = ((level->partsize[level->nav[navcount]] *
					                 level->value[level->
					                 partnum[level->nav[navcount]]]) + curpos);
					hexdump_trmod_printflags(0);
					for (; curpos < curpostarget; ++curpos)
					{
						trmod_printf("%02X ", level->inmem[curpos]);
					}
					trmod_printf("\n");
				}
				else
				{
					for (k = 0x00000000U;
					     k < level->value[level->partnum[level->nav[navcount]]];
					     ++k)
					{
						hexdump_trmod_printflags(k);
						curpostarget = (curpos +
						                level->partsize[level->nav[navcount]]);
						for (; curpos < curpostarget; ++curpos)
						{
							trmod_printf("%02X ", level->inmem[curpos]);
						}
						trmod_printf("\n");
					}
				}
			}
		}
	}
	if (curpos < levelsize)
	{
		if ((print & 0x0004U) != 0x0000U)
		{
			trmod_printf("[%08lX] ", curpos);
		}
		if ((print & 0x0003U) != 0x0000U)
		{
			trmod_printf("Unknown: ");
		}
		i = 0x0000U;
		for (; curpos < levelsize; ++curpos)
		{
			trmod_printf("%02X ", level->inmem[curpos]);
			++i;
			if (i == 0x0010U)
			{
				trmod_printf("\n");
				i = 0x0000U;
			}
		}
		trmod_printf("\n");
	}
	
end:/* Frees allocated memory and ends the function */
	if (level->inmem != NULL)
	{
		free(level->inmem);
		level->inmem = NULL;
	}
}

/*
 * Function that finds the nearest room to a given point
 * Parameters:
 * * level = Pointer to the level struct
 * * x = X-coordinate of the point (in world coordinates)
 * * y = Y-coordinate of the point (in world coordinates)
 * * z = Z-coordinate of the point (in world coordinates)
 * Returns the nearest room
 */
uint16 tr1pc_nearestRoom(struct level *level, int32 x, int32 y, int32 z)
{
	/* Variable Declarations */
	uint16 curroom;               /* Current room */
	uint16 bestroom = 0x0000;     /* Nearest room yet */
	uint32 curdist;               /* Current distance */
	uint32 bestdist = 0xFFFFFFFF; /* Best distance yet */
	uint32 diffx, diffy, diffz;   /* Distance on each axis */
	int32 curx, cury, curz;       /* Nearest position in the current room */
	int32 roommaxx, roommaxz;     /* Highest X- and Z-values in this room */
	uint32 res, bit, tmp;         /* Used to calculate square root */
	
	/* Loops through the rooms */
	for (curroom = 0x0000; curroom < level->numRooms; ++curroom)
	{
		/* Finds closest point in the room to the point */
		roommaxx = (int32) (level->room[curroom].value[R_NUMXSECTORS]);
		roommaxx *= 0x00000400;
		roommaxx += level->room[curroom].x;
		roommaxz = (int32) (level->room[curroom].value[R_NUMZSECTORS]);
		roommaxz *= 0x00000400;
		roommaxz += level->room[curroom].z;
		if (x < level->room[curroom].x)
		{
			curx = level->room[curroom].x;
		}
		else if (x > roommaxx)
		{
			curx = roommaxx;
		}
		else
		{
			curx = x;
		}
		if (y < level->room[curroom].yTop)
		{
			cury = level->room[curroom].yTop;
		}
		else if (y > level->room[curroom].yBottom)
		{
			cury = level->room[curroom].yBottom;
		}
		else
		{
			cury = y;
		}
		if (z < level->room[curroom].z)
		{
			curz = level->room[curroom].z;
		}
		else if (z > roommaxz)
		{
			curz = roommaxz;
		}
		else
		{
			curz = z;
		}
		
		/* Stops if the point is inside this room */
		if ((curx == x) && (cury == y) && (curz == z))
		{
			return curroom;
		}
		
		/* Calculates the distance between the points */
		if (curx > x)
		{
			diffx = (uint32) (curx - x);
		}
		else
		{
			diffx = (uint32) (x - curx);
		}
		if (cury > y)
		{
			diffy = (uint32) (cury - y);
		}
		else
		{
			diffy = (uint32) (y - cury);
		}
		if (curz > z)
		{
			diffz = (uint32) (curz - z);
		}
		else
		{
			diffz = (uint32) (z - curz);
		}
		diffx *= diffx;
		diffy *= diffy;
		diffz *= diffz;
		diffx += diffy;
		diffx += diffz;
		res = 0x00000000;
		bit = 0x40000000;
		while (bit > diffx)
		{
			bit >>= 2U;
		}
		while (bit != 0x00000000)
		{
			tmp = (res + bit);
			if (diffx >= tmp)
			{
				diffx -= tmp;
				res >>= 1U;
				res += bit;
			}
			else
			{
				res >>= 1U;
			}
			bit >>= 2U;
		}
		curdist = res;
		
		/* Sets the closest room yet */
		if (curdist < bestdist)
		{
			bestdist = curdist;
			bestroom = curroom;
		}
	}
	
	/* Returns the closest room */
	return bestroom;
}

/*
 * Function that finds whether a vertex exists, or adds it if it doesn't.
 * Parameters:
 * * level = Pointer to the level struct
 * * error = Pointer to the error struct
 * * room = Room number
 * * x = X-position of the vertex (in local coordinates)
 * * y = Y-position of the vertex (in local coordinates)
 * * z = Z-position of the vertex (in local coordinates)
 * * inten = Intensity of the vertex
 * Returns the vertex number
 */
uint16 tr1pc_find_vertex(struct level *level, struct error *error,
                         uint16 room, int32 x, int32 y, int32 z, int16 inten)
{
	/* Variable Declarations */
	uint8 vertex[8];        /* In-memory new vertex */
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
		tr1pc_replace_vertex(level, error, room, 0xFFFF,
		                     x, y, z, inten, 0xFFFF);
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
	vertex[6] = (uint8) (inten & 0x00FF);
	vertex[7] = (uint8) ((inten & 0xFF00) >> 8U);
	
	/* Allocates vertices */
	vertices = malloc((size_t) (level->room[room].value[R_NUMVERTICES] *
	                            0x0008));
	if (vertices == NULL)
	{
		error->code = ERROR_MEMORY;
		goto end;
	}
	
	/* Reads the vertices into memory */
	offset = (long int) (level->room[room].offset[R_NUMVERTICES] + 0x00000002);
	numData = (uint32) level->room[room].value[R_NUMVERTICES];
	numData *= 0x00000008;
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
		if (memcmp(vertex, &vertices[(curvert * 0x0008)], (size_t) 8) == 0)
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
		tr1pc_replace_vertex(level, error, room, 0xFFFF,
		                     x, y, z, inten, 0xFFFF);
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

/*
 * Function that adds a cameara to the level, or finds it if it exists.
 * Parameters:
 * * level = Pointer to the level struct
 * * error = Pointer to the error struct
 * * camera = In-memory copy of the camera to be found or added
 * Returns the camera number
 */
uint16 tr1pc_addcamera(struct level *level, struct error *error, uint8 *camera)
{
	/* Variable Declarations */
	uint8 *cams = NULL;         /* In-memory copy of the existing cameras */
	uint32 curcam = 0x00000000; /* Current camera in processing */
	uint16 camnum = 0xFFFF;     /* Output camera number */
	int seekval;                /* Return value of fseek() */
	size_t readval;             /* Return value of fread() */
	size_t writeval;            /* Return value of fwrite() */
	
	/* Reads the existing cameras into memory */
	if (level->value[NUMCAMERAS] > 0x00000000)
	{
		/* Allocates space for the cameras */
		cams = malloc((size_t) (level->value[NUMCAMERAS] * 0x00000010));
		if (cams == NULL)
		{
			error->code = ERROR_MEMORY;
			goto end;
		}
		
		/* Reads the cameras into memory */
		seekval = trmod_fseek(level->file, (long int)
		                      (level->offset[NUMCAMERAS] + 0x00000004),
		                      SEEK_SET);
		if (seekval != 0)
		{
			error->code = ERROR_FILE_READ_FAILED;
			error->string[0] = level->path;
			goto end;
		}
		readval = trmod_fread(cams, (size_t) 1, (size_t)
		                      (level->value[NUMCAMERAS] * 0x00000010),
		                      level->file);
		if (readval != (size_t) (level->value[NUMCAMERAS] * 0x00000010))
		{
			error->code = ERROR_FILE_READ_FAILED;
			error->string[0] = level->path;
			goto end;
		}
	}
	
	/* Finds the camera if it already exists */
	for (curcam = 0x00000000; curcam < level->value[NUMCAMERAS]; ++curcam)
	{
		if (memcmp(camera, &cams[(curcam * 0x00000010)], (size_t) 16) == 0)
		{
			camnum = (uint16) curcam;
			break;
		}
	}
	
	/* Adds the camera if it doesn't exist yet */
	if (camnum == 0xFFFF)
	{
		/* Makes room for the new camera */
		curcam = ((level->value[NUMCAMERAS] * 0x00000010) + 0x00000004 +
		          level->offset[NUMCAMERAS]);
		insertbytes(level->file, level->path, curcam, 0x00000010, error);
		if (error->code != ERROR_NONE)
		{
			goto end;
		}
		level_struct_add(level, curcam, 0x00000010);
		
		/* Writes the camera to the level file */
		seekval = trmod_fseek(level->file, (long int) curcam, SEEK_SET);
		if (seekval != 0)
		{
			error->code = ERROR_FILE_READ_FAILED;
			error->string[0] = level->path;
			goto end;
		}
		writeval = trmod_fwrite(camera, (size_t) 1, (size_t) 16, level->file);
		if (writeval != (size_t) 16)
		{
			error->code = ERROR_FILE_WRITE_FAILED;
			error->string[0] = level->path;
			goto end;
		}
		
		/* Adds one to NumCameras */
		camnum = (uint16) level->value[NUMCAMERAS];
		++(level->value[NUMCAMERAS]);
		curcam = level->value[NUMCAMERAS];
		endian32(curcam);
		seekval = trmod_fseek(level->file,
		                      (long int) level->offset[NUMCAMERAS], SEEK_SET);
		if (seekval != 0)
		{
			error->code = ERROR_FILE_READ_FAILED;
			error->string[0] = level->path;
			goto end;
		}
		writeval = trmod_fwrite(&curcam, (size_t) 1, (size_t) 4, level->file);
		if (writeval != (size_t) 4)
		{
			error->code = ERROR_FILE_WRITE_FAILED;
			error->string[0] = level->path;
			goto end;
		}
	}
	
end:/* Frees allocated memory and ends the function */
	if (cams != NULL)
	{
		free(cams);
	}
	return camnum;
}

/*
 * Function that prints the program's internal variables
 * Parameters:
 * * level = Pointer to level struct
 */
void tr1pc_vardump(struct level *level)
{
	/* Variable Declarations */
	uint32 curelement = 0x00000000U;
	uint16 curroom = 0x0000U;

	/* Prints level-wide variables */
	printf("file: %08lX\n"
	       "path: %s\n"
	       "inmem: %08lX\n"
	       "type: %08X\n"
	       "endian: %i\n"
	       "typestring: %s\n"
	       "command: %s\n"
	       "nav: %08lX\n"
	       "nav_flags: %08X\n"
	       "numRooms: " PRINTU16 "\n",
	       (unsigned long int) level->file,
	       level->path,
	       (unsigned long int) level->inmem,
	       level->type,
	       level->endian,
	       level->typestring,
	       level->command,
	       (unsigned long int) level->nav,
	       level->nav_flags,
	       level->numRooms);
	
	/* Prints offsets */
	for (curelement = 0x00000000U; curelement < NUMKEEPPARTS; ++curelement)
	{
		printf("offset[%s]: %06X\n",
		       partnames[curelement],
		       level->offset[curelement]);
	}
	
	/* Prints values */
	for (curelement = 0x00000000U; curelement < NUMKEEPPARTS; ++curelement)
	{
		printf("value[%s]: " PRINTU32 "\n",
		       partnames[curelement],
		       level->value[curelement]);
	}
	
	/* Prints part sizes */
	for (curelement = 0x00000000U; curelement < NUMLEVPARTS; ++curelement)
	{
		printf("partsize[%s]: " PRINTU32 "\n",
		       partnames[curelement],
		       level->partsize[curelement]);
	}
	
	/* Prints part-num-connections */
	for (curelement = 0x00000000U; curelement < NUMLEVPARTS; ++curelement)
	{
		printf("partnum[%s]: %s\n",
		       partnames[curelement],
		       partnames[level->partnum[curelement]]);
	}
	
	/* Prints the rooms */
	for (curroom = 0x0000U; curroom < level->numRooms; ++curroom)
	{
		printf("room[" PRINTU16 "].startpos: %06X\n"
		       "room[" PRINTU16 "].x: " PRINT32 "\n"
		       "room[" PRINTU16 "].z: " PRINT32 "\n"
		       "room[" PRINTU16 "].yBottom: " PRINT32 "\n"
		       "room[" PRINTU16 "].yTop: " PRINT32 "\n",
		       curroom,
		       level->room[curroom].startpos,
		       curroom,
		       level->room[curroom].x,
		       curroom,
		       level->room[curroom].z,
		       curroom,
		       level->room[curroom].yBottom,
		       curroom,
		       level->room[curroom].yTop);
		for (curelement = 0x00000000U; curelement < NUMROOMPARTNUMS;
		     ++curelement)
		{
			printf("room[" PRINTU16 "].offset[%s]: %08X\n",
			       curroom,
			       partnames[curelement],
			       level->room[curroom].offset[curelement]);
		}
		for (curelement = 0x00000000U; curelement < NUMROOMPARTNUMS;
		     ++curelement)
		{
			printf("room[" PRINTU16 "].value[%s]: " PRINTU16 "\n",
			       curroom,
			       partnames[curelement],
			       level->room[curroom].value[curelement]);
		}
		printf("room[" PRINTU16 "].flags: %04X\n",
		       curroom,
		       level->room[curroom].flags);
	}
}
