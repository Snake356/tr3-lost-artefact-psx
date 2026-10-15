/*
 * Library of functions relating to Tomb Raider I on PSX
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

/* Navigation array */
static const uint32 tr1psx_nav[] =
{
	SKIP_BYTES, 0x00000010U,
	NUMSAMPLES,
	SKIP_BYTES, 0x0006FFF9U,
	SAMPLE,
	NUMROOMS,
	START_ROOM,
		SKIP_BYTES, 0x00000014U,
		R_NUMVERTICES,
		SKIP_BYTES, 0x00000002U,
		R_VERTEX,
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
	SKIP_BYTES, 0x00000200U,
	NUMSOUNDDETAILS, SOUNDDETAIL,
	END_LEVEL
};

/*
 * Function that sets up the structs for navigating the level file
 * * Parameters:
 * * level = Pointer to the level struct
 */
void tr1psx_setup(struct level *level)
{
	/* Sets up the navigation array */
	level->nav = (uint32 *) tr1psx_nav;
	
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
	level->partsize[NUMSAMPLES]         = 0x00000003U;
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
	level->partsize[MOVEABLE]           = 0x00000014U;
	level->partsize[STATICMESH]         = 0x00000020U;
	level->partsize[OBJECTTEXTURE]      = 0x00000010U;
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
