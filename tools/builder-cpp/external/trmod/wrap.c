/*
 * Wrapper Functions for different Level Formats
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
#include <stdlib.h>   /* Standard library */
#include "fixedint.h" /* Definition of fixed-size integers */
#include "structs.h"  /* Definition of used structs */
#include "errors.h"   /* Definition of error codes */
#include "tr1pc.h"    /* Tomb Raider I PC (PHD/TUB) */
#include "tr1psx.h"   /* Tomb Raider I PSX (PSX) */
#include "tr1ss.h"    /* Tomb Raider I Saturn (SAT) */
#include "tr2pc.h"    /* Tomb Raider II PC (TR2) */
#include "tr2psx.h"   /* Tomb Raider II PSX (PSX) */
#include "tr3pc.h"    /* Tomb Raider III PC (TR2) */
#include "tr3psx.h"   /* Tomb Raider III PSX (PSX) */
#include "util.h"     /* General functions for TRMOD */
#include "conv.h"     /* Format conversions */
#include "wrap.h"     /* Wrapper functions for called functions */

/*
 * Function that sets up the navigation structs
 */
void setup(struct level *level, struct error *error)
{
	switch (level->type)
	{
		case TRMOD_LEVTYPE_TR1_PC:
		case TRMOD_LEVTYPE_TR1G_PC:
			tr1pc_setup(level);
			break;
		case TRMOD_LEVTYPE_TR1_PS:
			tr1psx_setup(level);
			break;
		case TRMOD_LEVTYPE_TR1_SS:
			tr1ss_setup(level);
			break;
		case TRMOD_LEVTYPE_TR2_PC:
			tr2pc_setup(level);
			break;
		case TRMOD_LEVTYPE_TR2_PS:
			tr2psx_setup(level);
			break;
		case TRMOD_LEVTYPE_TR3_PC:
			tr3pc_setup(level);
			break;
		case TRMOD_LEVTYPE_TR3_PS:
			tr3psx_setup(level);
			break;
		default:
			error->code = ERROR_UNIMPLEMENTED_FUNCTION;
			break;
	}
	
	/* Associates elements with their numbers */
	level->partnum[R_VERTEX] = R_NUMVERTICES;
	level->partnum[R_RECTANGLE] = R_NUMRECTANGLES;
	level->partnum[R_RECTTEX] = R_NUMRECTANGLES;
	level->partnum[R_TRIANGLE] = R_NUMTRIANGLES;
	level->partnum[R_SPRITE] = R_NUMSPRITES;
	level->partnum[R_DOOR] = R_NUMDOORS;
	level->partnum[R_LIGHT] = R_NUMLIGHTS;
	level->partnum[R_STATICMESH] = R_NUMSTATICMESHES;
	level->partnum[TEXTILE] = NUMTEXTILES;
	level->partnum[PALETTE] = NUMPALETTES;
	level->partnum[ROOMMESHBOX] = NUMROOMMESHBOXES;
	level->partnum[FLOORDATA] = NUMFLOORDATA;
	level->partnum[MESHDATA] = NUMMESHDATA;
	level->partnum[MESHPOINTER] = NUMMESHPOINTERS;
	level->partnum[ANIMATION] = NUMANIMATIONS;
	level->partnum[STATECHANGE] = NUMSTATECHANGES;
	level->partnum[ANIMDISPATCH] = NUMANIMDISPATCHES;
	level->partnum[ANIMCOMMAND] = NUMANIMCOMMANDS;
	level->partnum[MESHTREE] = NUMMESHTREES;
	level->partnum[FRAME] = NUMFRAMES;
	level->partnum[MOVEABLE] = NUMMOVEABLES;
	level->partnum[STATICMESH] = NUMSTATICMESHES;
	level->partnum[OBJECTTEXTURE] = NUMOBJECTTEXTURES;
	level->partnum[SPRITETEXTURE] = NUMSPRITETEXTURES;
	level->partnum[SPRITESEQUENCE] = NUMSPRITESEQUENCES;
	level->partnum[CAMERA] = NUMCAMERAS;
	level->partnum[SOUNDSOURCE] = NUMSOUNDSOURCES;
	level->partnum[BOX] = NUMBOXES;
	level->partnum[OVERLAP] = NUMOVERLAPS;
	level->partnum[ANIMATEDTEXTURE] = NUMANIMTEXTURES;
	level->partnum[ENTITY] = NUMENTITIES;
	level->partnum[ROOMTEXTURE] = NUMROOMTEX;
	level->partnum[CINEMATICFRAME] = NUMCINEMATICFRAMES;
	level->partnum[DEMODATA] = NUMDEMODATA;
	level->partnum[SOUNDDETAIL] = NUMSOUNDDETAILS;
	level->partnum[SAMPLE] = NUMSAMPLES;
	level->partnum[SAMPLEINDEX] = NUMSAMPLEINDICES;
}

/*
 * Wrapper function for navigate
 * Parameters:
 * * level = Pointer to level struct
 * * error = Pointer to error struct
 */
void navigate(struct level *level, struct error *error)
{
	/* Determines how to run the command based on leveltype */
	switch (level->type)
	{
		case TRMOD_LEVTYPE_TR1_PC:
		case TRMOD_LEVTYPE_TR1G_PC:
		case TRMOD_LEVTYPE_TR1_PS:
		case TRMOD_LEVTYPE_TR1_SS:
		case TRMOD_LEVTYPE_TR2_PC:
		case TRMOD_LEVTYPE_TR2_PS:
		case TRMOD_LEVTYPE_TR3_PC:
		case TRMOD_LEVTYPE_TR3_PS:
			tr1pc_navigate(level, error);
			break;
		default:
			error->code = ERROR_UNIMPLEMENTED_FUNCTION;
			break;
	}
}

/*
 * Function that needs to run after navigation
 * Parameters:
 * * level = Pointer to level struct
 * * error = Pointer to error struct
 */
void post_navigation(struct level *level, struct error *error)
{
	/* Determines how to run the command based on leveltype */
	switch (level->type)
	{
		case TRMOD_LEVTYPE_TR1_PC:
		case TRMOD_LEVTYPE_TR1G_PC:
		case TRMOD_LEVTYPE_TR1_PS:
		case TRMOD_LEVTYPE_TR2_PC:
		case TRMOD_LEVTYPE_TR2_PS:
		case TRMOD_LEVTYPE_TR3_PC:
			/* Nothing to do here */
			break;
		case TRMOD_LEVTYPE_TR1_SS:
			tr1ss_post_navigation(level, error);
			break;
		case TRMOD_LEVTYPE_TR3_PS:
			tr3psx_post_navigation(level, error);
			break;
		default:
			error->code = ERROR_UNIMPLEMENTED_FUNCTION;
			break;
	}
}

/*
 * Wrapper function for navigate
 * Parameters:
 * * level = Pointer to level struct
 * * error = Pointer to error struct
 */
void readRoomHeaders(struct level *level, struct error *error)
{
	/* Determines how to run the command based on leveltype */
	switch (level->type)
	{
		case TRMOD_LEVTYPE_TR1_PC:
		case TRMOD_LEVTYPE_TR1G_PC:
		case TRMOD_LEVTYPE_TR1_PS:
		case TRMOD_LEVTYPE_TR2_PC:
		case TRMOD_LEVTYPE_TR2_PS:
		case TRMOD_LEVTYPE_TR3_PC:
		case TRMOD_LEVTYPE_TR3_PS:
			tr1pc_readRoomHeaders(level, error);
			break;
		case TRMOD_LEVTYPE_TR1_SS:
			tr1ss_readRoomHeaders(level, error);
			break;
		default:
			error->code = ERROR_UNIMPLEMENTED_FUNCTION;
			break;
	}
}


/*
 * Wrapper function for get item
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
void get_item(struct level *level, struct error *error,
              uint32 item, uint16 print)
{
	/* Determines how to run the command based on leveltype */
	switch (level->type)
	{
		case TRMOD_LEVTYPE_TR1_PC:
		case TRMOD_LEVTYPE_TR1G_PC:
		case TRMOD_LEVTYPE_TR1_PS:
		case TRMOD_LEVTYPE_TR1_SS:
			tr1pc_get_item(level, error, item, print);
			break;
		case TRMOD_LEVTYPE_TR2_PC:
			tr2pc_get_item(level, error, item, print);
			break;
		case TRMOD_LEVTYPE_TR3_PC:
			tr3pc_get_item(level, error, item, print);
			break;
		default:
			error->code = ERROR_UNIMPLEMENTED_FUNCTION;
			break;
	}
}

/*
 * Wrapper function for additem
 * Parameters:
 * * level = Pointer to level struct
 * * error = Pointer to error struct
 * * id = Item ID
 * * room = Room the item is in
 * * x = X-Position for the item (local)
 * * y = Y-Position for the item (local)
 * * z = Z-Position for the item (local)
 * * angle = Item's angle
 * * intensity1 = String of itensity1
 * * intensity2 = String of itensity2
 * * flags = Item's flags
 */
void add_item(struct level *level, struct error *error, uint16 id, uint16 room,
              int32 x, int32 y, int32 z, int16 angle, char *intensity1,
              char *intensity2, uint16 flags)
{
	/* Determines how to run the command based on leveltype */
	switch (level->type)
	{
		case TRMOD_LEVTYPE_TR1_PC:
		case TRMOD_LEVTYPE_TR1G_PC:
		case TRMOD_LEVTYPE_TR1_SS:
		case TRMOD_LEVTYPE_TR2_PC:
		case TRMOD_LEVTYPE_TR3_PC:
			tr1pc_additem(level, error);
			if (error->code != ERROR_NONE)
			{
				return;
			}
			replace_item(level, error, 0xFFFFFFFF, id, room, x, y, z,
			             angle, intensity1, intensity2, flags, 0x01FF);
			break;
		default:
			error->code = ERROR_UNIMPLEMENTED_FUNCTION;
			break;
	}
}

/*
 * Wrapper function for replace item
 * Parameters:
 * * level = Pointer to level struct
 * * error = Pointer to error struct
 * * item = Item number (max int means the last one)
 * * id = Item ID
 * * room = Room the item is in
 * * x = X-Position for the item (local)
 * * y = Y-Position for the item (local)
 * * z = Z-Position for the item (local)
 * * angle = Item's angle
 * * intensity1 = String of itensity1
 * * intensity2 = String of itensity2
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
void replace_item(struct level *level, struct error *error, uint32 item,
                  uint16 id, uint16 room, int32 x, int32 y, int32 z,
                  int16 angle, char *intensity1, char *intensity2, uint16 flags,
                  uint16 rep)
{
	/* Variable Declarations */
	int16 s16[2] = {0x0000, 0x0000}; /* 16-bit signed integers */
	
	/* Determines how to run the command based on leveltype */
	switch (level->type)
	{
		case TRMOD_LEVTYPE_TR1_PC:
		case TRMOD_LEVTYPE_TR1G_PC:
			s16[0] = convertLight(intensity1, 0);
			if (error->code == ERROR_NONE)
			{
				tr1pc_replaceitem(level, error, item, id, room,
				                  x, y, z, angle, s16[0], flags, rep);
			}
			break;
		case TRMOD_LEVTYPE_TR1_SS:
			s16[0] = convertLight(intensity1, 0);
			if (error->code == ERROR_NONE)
			{
				tr1ss_replaceitem(level, error, item, id, room,
				                  x, y, z, angle, s16[0], flags, rep);
			}
			break;
		case TRMOD_LEVTYPE_TR2_PC:
			s16[0] = convertLight(intensity1, 0);
			s16[1] = convertLight(intensity2, 0);
			if (error->code == ERROR_NONE)
			{
				tr2pc_replaceitem(level, error, item, id, room, x, y, z,
				                  angle, s16[0], s16[1], flags, rep);
			}
			break;
		case TRMOD_LEVTYPE_TR3_PC:
			s16[0] = convertLight(intensity1, 1);
			s16[1] = convertLight(intensity2, 1);
			if (error->code == ERROR_NONE)
			{
				tr2pc_replaceitem(level, error, item, id, room, x, y, z,
				                  angle, s16[0], s16[1], flags, rep);
			}
			break;
		default:
			error->code = ERROR_UNIMPLEMENTED_FUNCTION;
			break;
	}
}

/*
 * Wrapper function for remove item
 * Parameters:
 * * level = Pointer to level struct
 * * error = Pointer to error struct
 * * item = Item number
 * * flags = Bit-mask of flags
 * * * 0 0 0 0  0 0 0 0   0 0 0 0  0 0 0 0
 * * * | | | |  | | | |   | | | |  | | | |
 * * * | | | |  | | | |   | | | |  | | | Adjust FloorData
 * * * Unused
 */
void remove_item(struct level *level, struct error *error,
                 uint32 item, uint16 flags)
{
	/* Determines how to run the command based on leveltype */
	switch (level->type)
	{
		case TRMOD_LEVTYPE_TR1_PC:
		case TRMOD_LEVTYPE_TR1G_PC:
		case TRMOD_LEVTYPE_TR1_SS:
		case TRMOD_LEVTYPE_TR2_PC:
		case TRMOD_LEVTYPE_TR3_PC:
			tr1pc_removeitem(level, error, item, flags);
			break;
		default:
			error->code = ERROR_UNIMPLEMENTED_FUNCTION;
			break;
	}
}

/*
 * Wrapper function for remove item
 * Parameters:
 * * level = Pointer to level struct
 * * error = Pointer to error struct
 */
void remove_all_items(struct level *level, struct error *error)
{
	/* Determines how to run the command based on leveltype */
	switch (level->type)
	{
		case TRMOD_LEVTYPE_TR1_PC:
		case TRMOD_LEVTYPE_TR1G_PC:
		case TRMOD_LEVTYPE_TR1_SS:
		case TRMOD_LEVTYPE_TR2_PC:
		case TRMOD_LEVTYPE_TR3_PC:
			tr1pc_removeallitems(level, error);
			break;
		default:
			error->code = ERROR_UNIMPLEMENTED_FUNCTION;
			break;
	}
}

/*
 * Wrapper function for get static mesh
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
void get_staticmesh(struct level *level, struct error *error,
                    uint16 room, uint16 mesh, uint16 print)
{
	/* Determines how to run the command based on leveltype */
	switch (level->type)
	{
		case TRMOD_LEVTYPE_TR1_PC:
		case TRMOD_LEVTYPE_TR1G_PC:
		case TRMOD_LEVTYPE_TR1_SS:
			tr1pc_get_staticmesh(level, error, room, mesh, print);
			break;
		case TRMOD_LEVTYPE_TR2_PC:
			tr2pc_get_staticmesh(level, error, room, mesh, print);
			break;
		case TRMOD_LEVTYPE_TR3_PC:
			tr3pc_get_staticmesh(level, error, room, mesh, print);
			break;
		default:
			error->code = ERROR_UNIMPLEMENTED_FUNCTION;
			break;
	}
}

/*
 * Wrapper function for addstaticmesh
 * Parameters:
 * * level = Pointer to level struct
 * * error = Pointer to error struct
 * * id = Static Mesh ID
 * * room = Room the static mesh is in
 * * x = X-Position for the static mesh (local)
 * * y = Y-Position for the static mesh (local)
 * * z = Z-Position for the static mesh (local)
 * * angle = Static mesh's angle
 * * intensity1 = String of itensity1
 * * intensity2 = String of itensity2
 */
void add_staticmesh(struct level *level, struct error *error, uint16 id,
                    uint16 room, int32 x, int32 y, int32 z, int16 angle,
                    char *intensity1, char *intensity2)
{
	/* Determines how to run the command based on leveltype */
	switch (level->type)
	{
		case TRMOD_LEVTYPE_TR1_PC:
		case TRMOD_LEVTYPE_TR1G_PC:
		case TRMOD_LEVTYPE_TR1_SS:
		case TRMOD_LEVTYPE_TR2_PC:
		case TRMOD_LEVTYPE_TR3_PC:
			tr1pc_addstaticmesh(level, error, room);
			if (error->code != ERROR_NONE)
			{
				return;
			}
			replace_staticmesh(level, error, 0xFFFF, id, room, x, y, z,
			                   angle, intensity1, intensity2, 0x007F);
			break;
		default:
			error->code = ERROR_UNIMPLEMENTED_FUNCTION;
			break;
	}
}

/*
 * Wrapper function for replace static mesh
 * Parameters:
 * * level = Pointer to level struct
 * * error = Pointer to error struct
 * * mesh = Mesh number (max int means last one)
 * * id = Mesh ID
 * * room = Room the static mesh is in
 * * x = X-Position for the static mesh (local)
 * * y = Y-Position for the static mesh (local)
 * * z = Z-Position for the static mesh (local)
 * * angle = Static Mesh's angle
 * * intensity1 = String of itensity1
 * * intensity2 = String of itensity2
 * * flags = Item's flags
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
void replace_staticmesh(struct level *level, struct error *error, uint16 mesh,
                        uint16 id, uint16 room, int32 x, int32 y, int32 z,
                        int16 angle, char *intensity1, char *intensity2,
                        uint16 rep)
{
	/* Variable Declarations */
	int16 s16[2] = {0x0000, 0x000}; /* 16-bit signed integer */
	
	/* Determines how to run the command based on leveltype */
	switch (level->type)
	{
		case TRMOD_LEVTYPE_TR1_PC:
		case TRMOD_LEVTYPE_TR1G_PC:
			s16[0] = convertLight(intensity1, 0);
			if (error->code == ERROR_NONE)
			{
				tr1pc_replacestaticmesh(level, error, mesh, id,
				                        room, x, y, z, angle, s16[0], rep);
			}
			break;
		case TRMOD_LEVTYPE_TR1_SS:
			s16[0] = convertLight(intensity1, 0);
			if (error->code == ERROR_NONE)
			{
				tr1ss_replacestaticmesh(level, error, mesh, id,
				                        room, x, y, z, angle, s16[0], rep);
			}
			break;
		case TRMOD_LEVTYPE_TR2_PC:
			s16[0] = convertLight(intensity1, 0);
			s16[1] = convertLight(intensity2, 0);
			if (error->code == ERROR_NONE)
			{
				tr2pc_replacestaticmesh(level, error, mesh, id,
				                        room, x, y, z, angle,
				                        s16[0], s16[1], rep);
			}
			break;
		case TRMOD_LEVTYPE_TR3_PC:
			s16[0] = convertLight(intensity1, 1);
			s16[1] = convertLight(intensity2, 1);
			if (error->code == ERROR_NONE)
			{
				tr2pc_replacestaticmesh(level, error, mesh, id,
				                        room, x, y, z, angle,
				                        s16[0], s16[1], rep);
			}
			break;
		default:
			error->code = ERROR_UNIMPLEMENTED_FUNCTION;
			break;
	}
}

/*
 * Wrapper function for remove static mesh
 * Parameters:
 * * level = Pointer to level struct
 * * error = Pointer to error struct
 * * room = Room the static mesh is in
 * * mesh = Number of the static mesh to remove
 */
void remove_staticmesh(struct level *level, struct error *error,
                       uint16 room, uint16 mesh)
{
	/* Determines how to run the command based on leveltype */
	switch (level->type)
	{
		case TRMOD_LEVTYPE_TR1_PC:
		case TRMOD_LEVTYPE_TR1G_PC:
		case TRMOD_LEVTYPE_TR1_SS:
		case TRMOD_LEVTYPE_TR2_PC:
		case TRMOD_LEVTYPE_TR3_PC:
			tr1pc_removestaticmesh(level, error, room, mesh);
			break;
		default:
			error->code = ERROR_UNIMPLEMENTED_FUNCTION;
			break;
	}
}

/*
 * Wrapper function for remove all static meshes
 * Parameters:
 * * level = Pointer to level struct
 * * error = Pointer to error struct
 * * room = Room to remove the static meshes from
 */
void remove_all_staticmeshes(struct level *level,
                             struct error *error, uint16 room)
{
	/* Determines how to run the command based on leveltype */
	switch (level->type)
	{
		case TRMOD_LEVTYPE_TR1_PC:
		case TRMOD_LEVTYPE_TR1G_PC:
		case TRMOD_LEVTYPE_TR1_SS:
		case TRMOD_LEVTYPE_TR2_PC:
		case TRMOD_LEVTYPE_TR3_PC:
			tr1pc_removeallstaticmeshes(level, error, room);
			break;
		default:
			error->code = ERROR_UNIMPLEMENTED_FUNCTION;
			break;
	}
}

/*
 * Wrapper function for get light
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
void get_light(struct level *level, struct error *error,
               uint16 room, uint16 light, uint16 print)
{
	/* Determines how to run the command based on leveltype */
	switch (level->type)
	{
		case TRMOD_LEVTYPE_TR1_PC:
		case TRMOD_LEVTYPE_TR1G_PC:
			tr1pc_get_light(level, error, room, light, print);
			break;
		case TRMOD_LEVTYPE_TR1_SS:
			tr1ss_get_light(level, error, room, light, print);
			break;
		case TRMOD_LEVTYPE_TR2_PC:
			tr2pc_get_light(level, error, room, light, print);
			break;
		case TRMOD_LEVTYPE_TR3_PC:
			tr3pc_get_light(level, error, room, light, print);
			break;
		default:
			error->code = ERROR_UNIMPLEMENTED_FUNCTION;
			break;
	}
}

/*
 * Wrapper function for addlight
 * Parameters:
 * * level = Pointer to level struct
 * * error = Pointer to error struct
 * * room = Room the light is in
 * * x = X-Position for the light (local)
 * * y = Y-Position for the light (local)
 * * z = Z-Position for the light (local)
 * * intensity1 = String of itensity1
 * * intensity2 = String of itensity2
 * * fade1 = Fade1 value for the light
 * * fade2 = Fade2 value for the light
 * * type = Type of the light
 * * nx = Normal X
 * * ny = Normal Y
 * * nz = Normal Z
 */
void add_light(struct level *level, struct error *error,
               uint16 room, int32 x, int32 y, int32 z, char *intensity1,
               char *intensity2, uint32 fade1, uint32 fade2, uint8 type,
               int16 nx, int16 ny, int16 nz)
{
	/* Determines how to run the command based on leveltype */
	switch (level->type)
	{
		case TRMOD_LEVTYPE_TR1_PC:
		case TRMOD_LEVTYPE_TR1G_PC:
		case TRMOD_LEVTYPE_TR1_SS:
		case TRMOD_LEVTYPE_TR2_PC:
		case TRMOD_LEVTYPE_TR3_PC:
			tr1pc_addlight(level, error, room);
			if (error->code != ERROR_NONE)
			{
				return;
			}
			replace_light(level, error, room, 0xFFFF, x, y, z, intensity1,
			              intensity2, fade1, fade2, type, nx, ny, nz, 0xFFFF);
			break;
		default:
			error->code = ERROR_UNIMPLEMENTED_FUNCTION;
			break;
	}
}

/*
 * Wrapper function for replace light
 * Parameters:
 * * level = Pointer to level struct
 * * error = Pointer to error struct
 * * room = Room the light is in
 * * light = Light number to remove (max int means last one)
 * * x = X-Position for the light (local)
 * * y = Y-Position for the light (local)
 * * z = Z-Position for the light (local)
 * * intensity1 = String of itensity1
 * * intensity2 = String of itensity2
 * * fade1 = Fade1 value for the light
 * * fade2 = Fade2 value for the light
 * * type = Type of light (spotlight/sunlight)
 * * nx = Normal X
 * * ny = Normal Y
 * * nz = Normal Z
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
 * * * | | | |  | | | |   Type
 * * * | | | |  | | | nx
 * * * | | | |  | | ny
 * * * | | | |  | nz
 * * * Unused
 */
void replace_light(struct level *level, struct error *error, uint16 room,
                   uint16 light, int32 x, int32 y, int32 z, char *intensity1,
                   char *intensity2, uint32 fade1, uint32 fade2, uint8 type,
                   int16 nx, int16 ny, int16 nz, uint16 rep)
{
	/* Variable Declarations */
	int16 s16[2] = {0x0000, 0x0000}; /* 16-bit signed integer */
	uint32 u32[1] = {0x00000000U};   /* 32-bit unsigned integer */
	
	/* Determines how to run the command based on leveltype */
	switch (level->type)
	{
		case TRMOD_LEVTYPE_TR1_PC:
		case TRMOD_LEVTYPE_TR1G_PC:
			s16[0] = convertLight(intensity1, 0);
			if (error->code == ERROR_NONE)
			{
				tr1pc_replacelight(level, error, light, room,
				                   x, y, z, s16[0], fade1, rep);
			}
			break;
		case TRMOD_LEVTYPE_TR1_SS:
			s16[0] = convertLight(intensity1, 0);
			if (error->code == ERROR_NONE)
			{
				tr1ss_replacelight(level, error, light, room,
				                   x, y, z, s16[0], fade1, rep);
			}
			break;
		case TRMOD_LEVTYPE_TR2_PC:
			s16[0] = convertLight(intensity1, 0);
			s16[1] = convertLight(intensity2, 0);
			if (error->code == ERROR_NONE)
			{
				tr2pc_replacelight(level, error, light, room, x, y, z,
				                   s16[0], s16[1], fade1, fade2, rep);
			}
			break;
		case TRMOD_LEVTYPE_TR3_PC:
			if (intensity1 != NULL)
			{
				u32[0] = (uint32) strtol(intensity1, NULL, 10);
			}
			tr3pc_replacelight(level, error, light, room, x, y, z, fade2,
			                   type, u32[0], fade1, nx, ny, nz, rep);
			break;
		default:
			error->code = ERROR_UNIMPLEMENTED_FUNCTION;
			break;
	}
}

/*
 * Wrapper function for remove light
 * Parameters:
 * * level = Pointer to level struct
 * * error = Pointer to error struct
 * * room = Room the light is in
 * * light = Number of the light to remove
 */
void remove_light(struct level *level, struct error *error,
                  uint16 room, uint16 light)
{
	/* Determines how to run the command based on leveltype */
	switch (level->type)
	{
		case TRMOD_LEVTYPE_TR1_PC:
		case TRMOD_LEVTYPE_TR1G_PC:
		case TRMOD_LEVTYPE_TR1_SS:
		case TRMOD_LEVTYPE_TR2_PC:
		case TRMOD_LEVTYPE_TR3_PC:
			tr1pc_removelight(level, error, room, light);
			break;
		default:
			error->code = ERROR_UNIMPLEMENTED_FUNCTION;
			break;
	}
}

/*
 * Wrapper function for remove all lights
 * Parameters:
 * * level = Pointer to level struct
 * * error = Pointer to error struct
 * * room = Room to remove the lights from
 */
void remove_all_lights(struct level *level, struct error *error, uint16 room)
{
	/* Determines how to run the command based on leveltype */
	switch (level->type)
	{
		case TRMOD_LEVTYPE_TR1_PC:
		case TRMOD_LEVTYPE_TR1G_PC:
		case TRMOD_LEVTYPE_TR1_SS:
		case TRMOD_LEVTYPE_TR2_PC:
		case TRMOD_LEVTYPE_TR3_PC:
			tr1pc_removealllights(level, error, room);
			break;
		default:
			error->code = ERROR_UNIMPLEMENTED_FUNCTION;
			break;
	}
}

/*
 * Wrapper function for get sound source
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
void get_soundsource(struct level *level, struct error *error,
                     uint32 source, uint16 print)
{
	/* Determines how to run the command based on leveltype */
	switch (level->type)
	{
		case TRMOD_LEVTYPE_TR1_PC:
		case TRMOD_LEVTYPE_TR1G_PC:
		case TRMOD_LEVTYPE_TR1_SS:
		case TRMOD_LEVTYPE_TR2_PC:
		case TRMOD_LEVTYPE_TR3_PC:
			tr1pc_get_soundsource(level, error, source, print);
			break;
		default:
			error->code = ERROR_UNIMPLEMENTED_FUNCTION;
			break;
	}
}

/*
 * Wrapper function for add sound source
 * Parameters:
 * * level = Pointer to level struct
 * * error = Pointer to error struct
 * * id = SoundID
 * * room = Room
 * * x = Sound Source's X-position (in local coordnates)
 * * y = Sound Source's Y-position (in local coordnates)
 * * z = Sound Source's Z-position (in local coordnates)
 * * flags = Sound Source's flags
 */
void add_soundsource(struct level *level, struct error *error, uint16 id,
                     uint16 room, int32 x, int32 y, int32 z, uint16 flags)
{
	/* Determines how to run the command based on leveltype */
	switch (level->type)
	{
		case TRMOD_LEVTYPE_TR1_PC:
		case TRMOD_LEVTYPE_TR1G_PC:
		case TRMOD_LEVTYPE_TR1_SS:
		case TRMOD_LEVTYPE_TR2_PC:
		case TRMOD_LEVTYPE_TR3_PC:
			tr1pc_addsoundsource(level, error);
			if (error->code != ERROR_NONE)
			{
				return;
			}
			replace_soundsource(level, error, 0xFFFFFFFF, id,
			                    room, x, y, z, flags, 0x003F);
			break;
		default:
			error->code = ERROR_UNIMPLEMENTED_FUNCTION;
			break;
	}
}

/*
 * Wrapper function for replace sound source
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
void replace_soundsource(struct level *level, struct error *error,
                         uint32 source, uint16 id, uint16 room, int32 x,
                         int32 y, int32 z, uint16 flags, uint16 rep)
{
	/* Determines how to run the command based on leveltype */
	switch (level->type)
	{
		case TRMOD_LEVTYPE_TR1_PC:
		case TRMOD_LEVTYPE_TR1G_PC:
		case TRMOD_LEVTYPE_TR1_SS:
		case TRMOD_LEVTYPE_TR2_PC:
		case TRMOD_LEVTYPE_TR3_PC:
			tr1pc_replacesoundsource(level, error, source, id, room,
			                         x, y, z, flags, rep);
			break;
		default:
			error->code = ERROR_UNIMPLEMENTED_FUNCTION;
			break;
	}
}

/*
 * Wrapper function for remove sound source
 * Parameters:
 * * level = Pointer to level struct
 * * error = Pointer to error struct
 * * source = Number of the sound source to be replaced
 */
void remove_soundsource(struct level *level, struct error *error, uint32 source)
{
	/* Determines how to run the command based on leveltype */
	switch (level->type)
	{
		case TRMOD_LEVTYPE_TR1_PC:
		case TRMOD_LEVTYPE_TR1G_PC:
		case TRMOD_LEVTYPE_TR1_SS:
		case TRMOD_LEVTYPE_TR2_PC:
		case TRMOD_LEVTYPE_TR3_PC:
			tr1pc_removesoundsource(level, error, source);
			break;
		default:
			error->code = ERROR_UNIMPLEMENTED_FUNCTION;
			break;
	}
}

/*
 * Wrapper function for remove all sound sources
 * Parameters:
 * * level = Pointer to level struct
 * * error = Pointer to error struct
 */
void remove_all_soundsources(struct level *level, struct error *error)
{
	/* Determines how to run the command based on leveltype */
	switch (level->type)
	{
		case TRMOD_LEVTYPE_TR1_PC:
		case TRMOD_LEVTYPE_TR1G_PC:
		case TRMOD_LEVTYPE_TR1_SS:
		case TRMOD_LEVTYPE_TR2_PC:
		case TRMOD_LEVTYPE_TR3_PC:
			tr1pc_removeallsoundsources(level, error);
			break;
		default:
			error->code = ERROR_UNIMPLEMENTED_FUNCTION;
			break;
	}
}

/*
 * Wrapper function for get floordata
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
void get_floordata(struct level *level, struct error *error, uint16 room,
                   uint16 column, uint16 row, uint16 print)
{
	/* Determines how to run the command based on leveltype */
	switch (level->type)
	{
		case TRMOD_LEVTYPE_TR1_PC:
		case TRMOD_LEVTYPE_TR1G_PC:
		case TRMOD_LEVTYPE_TR2_PC:
		case TRMOD_LEVTYPE_TR3_PC:
			tr1pc_get_floordata(level, error, room, column, row,
			                    0x00000000U, print);
			break;
		case TRMOD_LEVTYPE_TR1_SS:
			tr1pc_get_floordata(level, error, room, column, row,
			                    0x00000010U, print);
			break;
		default:
			error->code = ERROR_UNIMPLEMENTED_FUNCTION;
			break;
	}
}

/*
 * Wrapper function for replace floordata
 * Parameters:
 * * level = Pointer to level struct
 * * error = Pointer to error struct
 * * room = Room number
 * * column = Column number
 * * row = Row number
 * * string = String-version of the floordata
 * * flags = Flags relating to how to replace floordata:
 * * * 0 0 0 0  0 0 0 0   0 0 0 0  0 0 0 0
 * * * | | | |  | | | |   | | | |  | | | |
 * * * | | | |  | | | |   | | | |  | | | Optimise
 * * * Unused
 */
void replace_floordata(struct level *level, struct error *error, uint16 room,
                       uint16 column, uint16 row, char *string, uint16 flags)
{
	/* Determines how to run the command based on leveltype */
	switch (level->type)
	{
		case TRMOD_LEVTYPE_TR1_PC:
		case TRMOD_LEVTYPE_TR1G_PC:
		case TRMOD_LEVTYPE_TR2_PC:
		case TRMOD_LEVTYPE_TR3_PC:
			tr1pc_replace_floordata(level, error, room, column,
			                        row, string, 0x00000000U, flags);
			break;
		case TRMOD_LEVTYPE_TR1_SS:
			tr1pc_replace_floordata(level, error, room, column,
			                        row, string, 0x00000010U, flags);
			break;
		default:
			error->code = ERROR_UNIMPLEMENTED_FUNCTION;
			break;
	}
}

/*
 * Wrapper function for remove floordata
 * Parameters:
 * * level = Pointer to level struct
 * * error = Pointer to error struct
 * * room = Room number
 * * column = Column number
 * * row = Row number
 */
void remove_floordata(struct level *level, struct error *error,
                      uint16 room, uint16 column, uint16 row)
{
	/* Determines how to run the command based on leveltype */
	switch (level->type)
	{
		case TRMOD_LEVTYPE_TR1_PC:
		case TRMOD_LEVTYPE_TR1G_PC:
		case TRMOD_LEVTYPE_TR2_PC:
		case TRMOD_LEVTYPE_TR3_PC:
			tr1pc_remove_floordata(level, error, room,
			                       column, row, 0x00000000U);
			break;
		case TRMOD_LEVTYPE_TR1_SS:
			tr1pc_remove_floordata(level, error, room,
			                       column, row, 0x00000010U);
			break;
		default:
			error->code = ERROR_UNIMPLEMENTED_FUNCTION;
			break;
	}
}

/*
 * Wrapper function for remove all floordata
 * Parameters:
 * * level = Pointer to level struct
 * * error = Pointer to error struct
 */
void remove_all_floordata(struct level *level, struct error *error)
{
	/* Determines how to run the command based on leveltype */
	switch (level->type)
	{
		case TRMOD_LEVTYPE_TR1_PC:
		case TRMOD_LEVTYPE_TR1G_PC:
		case TRMOD_LEVTYPE_TR1_SS:
		case TRMOD_LEVTYPE_TR2_PC:
		case TRMOD_LEVTYPE_TR3_PC:
			tr1pc_remove_all_floordata(level, error);
			break;
		default:
			error->code = ERROR_UNIMPLEMENTED_FUNCTION;
			break;
	}
}

/*
 * Wrapper function for add room
 * Parameters:
 * * level = Pointer to level struct
 * * error = Pointer to error struct
 */
void add_room(struct level *level, struct error *error)
{
	/* Determines how to run the command based on leveltype */
	switch (level->type)
	{
		case TRMOD_LEVTYPE_TR1_PC:
		case TRMOD_LEVTYPE_TR1G_PC:
			tr1pc_addroom(level, error);
			break;
		case TRMOD_LEVTYPE_TR2_PC:
			tr2pc_addroom(level, error);
			break;
		case TRMOD_LEVTYPE_TR3_PC:
			tr3pc_addroom(level, error);
			break;
		default:
			error->code = ERROR_UNIMPLEMENTED_FUNCTION;
			break;
	}
}

/*
 * Wrapper function for get_roompos
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
void get_roompos(struct level *level, struct error *error,
                 uint16 room, uint16 print)
{
	/* Determines how to run the command based on leveltype */
	switch (level->type)
	{
		case TRMOD_LEVTYPE_TR1_PC:
		case TRMOD_LEVTYPE_TR1G_PC:
		case TRMOD_LEVTYPE_TR1_SS:
		case TRMOD_LEVTYPE_TR2_PC:
		case TRMOD_LEVTYPE_TR3_PC:
			tr1pc_get_roompos(level, error, room, print);
			break;
		default:
			error->code = ERROR_UNIMPLEMENTED_FUNCTION;
			break;
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
void move_room(struct level *level, struct error *error,
               uint16 room, int32 x, int32 y, int32 z, uint16 flags)
{
	/* Determines how to run the command based on leveltype */
	switch (level->type)
	{
		case TRMOD_LEVTYPE_TR1_PC:
		case TRMOD_LEVTYPE_TR1G_PC:
			tr1pc_moveroom(level, error, room, x, y, z, flags);
			break;
		case TRMOD_LEVTYPE_TR2_PC:
		case TRMOD_LEVTYPE_TR3_PC:
			tr2pc_moveroom(level, error, room, x, y, z, flags);
			break;
		default:
			error->code = ERROR_UNIMPLEMENTED_FUNCTION;
			break;
	}
}

/*
 * Wrapper function for get_roomsize
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
void get_roomsize(struct level *level, struct error *error,
                  uint16 room, uint16 print)
{
	/* Determines how to run the command based on leveltype */
	switch (level->type)
	{
		case TRMOD_LEVTYPE_TR1_PC:
		case TRMOD_LEVTYPE_TR1G_PC:
		case TRMOD_LEVTYPE_TR1_SS:
		case TRMOD_LEVTYPE_TR2_PC:
		case TRMOD_LEVTYPE_TR3_PC:
			tr1pc_get_roomsize(level, error, room, print);
			break;
		default:
			error->code = ERROR_UNIMPLEMENTED_FUNCTION;
			break;
	}
}

/*
 * Wrapper function for resize_room
 * Parameters:
 * * level = Pointer to the level struct
 * * error = Pointer to the error struct
 * * room = Room number
 * * columns = The new number of columns
 * * rows = The new number of rows
 * * height = The new height of the room (in coordinates)
 */
void resize_room(struct level *level, struct error *error, uint16 room,
                 uint16 columns, uint16 rows, int32 height)
{
	/* Determines how to run the command based on leveltype */
	switch (level->type)
	{
		case TRMOD_LEVTYPE_TR1_PC:
		case TRMOD_LEVTYPE_TR1G_PC:
		case TRMOD_LEVTYPE_TR2_PC:
		case TRMOD_LEVTYPE_TR3_PC:
			tr1pc_resize_room(level, error, room, columns, rows, height);
			break;
		default:
			error->code = ERROR_UNIMPLEMENTED_FUNCTION;
			break;
	}
}

/*
 * Wrapper function for removerooms
 * Parameters:
 * * level = Pointer to level struct
 * * error = Pointer to error struct
 */
void remove_all_rooms(struct level *level, struct error *error)
{
	/* Determines how to run the command based on leveltype */
	switch (level->type)
	{
		case TRMOD_LEVTYPE_TR1_PC:
		case TRMOD_LEVTYPE_TR1G_PC:
		case TRMOD_LEVTYPE_TR2_PC:
		case TRMOD_LEVTYPE_TR3_PC:
			tr1pc_remove_all_rooms(level, error);
			break;
		default:
			error->code = ERROR_UNIMPLEMENTED_FUNCTION;
			break;
	}
}

/*
 * Wrapper function for get floor/ceiling
 * Parameters:
 * * level = Pointer to level struct
 * * error = Pointer to error struct
 * * room = Room number
 * * column = Column number
 * * row = Row number
 * * type = Type to print (0 = Floor, 1 = Ceiling)
 * * print = Printing flags
 * * * 0 0 0 0  0 0 0 0   0 0 0 0  0 0 0 0
 * * * | | | |  | | | |   | | | |  | | | |
 * * * | | | |  | | | |   | | | |  | | | As command
 * * * | | | |  | | | |   | | | |  | | In Hexadecimal
 * * * Unused
 */
void get_floor_ceiling(struct level *level, struct error *error, uint16 room,
                       uint16 column, uint16 row, int type, uint16 print)
{
	/* Determines how to run the command based on leveltype */
	switch (level->type)
	{
		case TRMOD_LEVTYPE_TR1_PC:
		case TRMOD_LEVTYPE_TR1G_PC:
		case TRMOD_LEVTYPE_TR2_PC:
		case TRMOD_LEVTYPE_TR3_PC:
			tr1pc_getfloorceil(level, error, room, column, row,
			                   0x00000000U, type, print);
			break;
		case TRMOD_LEVTYPE_TR1_SS:
			tr1pc_getfloorceil(level, error, room, column, row,
			                   0x00000010U, type, print);
			break;
		default:
			error->code = ERROR_UNIMPLEMENTED_FUNCTION;
			break;
	}
}

/*
 * Wrapper function for set floor/ceiling
 * Parameters:
 * * level = Pointer to level struct
 * * error = Pointer to error struct
 * * room = Room number
 * * column = Column number
 * * row = Row number
 * * height = Height (in local coordinates)
 * * type = Type of floor or ceiling:
 * * * 0 0 0 0  0 0 0 0   0 0 0 0  0 0 0 0
 * * * | | | |  | | | |   | | | |  | | | |
 * * * | | | |  | | | |   | | | |  | | | Surface (0 = Floor, 1 = Ceiling)
 * * * | | | |  | | | |   | | | |  | | Type (0 = Value, 1 = Wall)
 * * * Unused
 */
void set_floor_ceiling(struct level *level, struct error *error, uint16 room,
                       uint16 column, uint16 row, int32 height, uint16 type)
{
	/* Determines how to run the command based on leveltype */
	switch (level->type)
	{
		case TRMOD_LEVTYPE_TR1_PC:
		case TRMOD_LEVTYPE_TR1G_PC:
		case TRMOD_LEVTYPE_TR2_PC:
		case TRMOD_LEVTYPE_TR3_PC:
			tr1pc_setfloorceil(level, error, room, column, row,
			                   0x00000000U, height, type);
			break;
		case TRMOD_LEVTYPE_TR1_SS:
			tr1pc_setfloorceil(level, error, room, column, row,
			                   0x00000010U, height, type);
			break;
		default:
			error->code = ERROR_UNIMPLEMENTED_FUNCTION;
			break;
	}
}

/*
 * Wrapper function for get zone
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
void get_zone(struct level *level, struct error *error, uint16 room,
              uint16 column, uint16 row, uint16 print)
{
	/* Determines how to run the command based on leveltype */
	switch (level->type)
	{
		case TRMOD_LEVTYPE_TR1_PC:
		case TRMOD_LEVTYPE_TR1G_PC:
		case TRMOD_LEVTYPE_TR2_PC:
			tr1pc_getzone(level, error, room, column, row, 0x00000000U, print);
			break;
		case TRMOD_LEVTYPE_TR1_SS:
			tr1pc_getzone(level, error, room, column, row, 0x00000010U, print);
			break;
		case TRMOD_LEVTYPE_TR3_PC:
			tr3pc_getzone(level, error, room, column, row, print);
			break;
		default:
			error->code = ERROR_UNIMPLEMENTED_FUNCTION;
			break;
	}
}

/*
 * Wrapper function for set zone
 * Parameters:
 * * level = Pointer to level struct
 * * error = Pointer to error struct
 * * room = Room number
 * * column = Column number
 * * row = Row number
 * * boxIndex = boxIndex to write to the sector
 */
void set_zone(struct level *level, struct error *error, uint16 room,
              uint16 column, uint16 row, int16 boxIndex)
{
	/* Determines how to run the command based on leveltype */
	switch (level->type)
	{
		case TRMOD_LEVTYPE_TR1_PC:
		case TRMOD_LEVTYPE_TR1G_PC:
		case TRMOD_LEVTYPE_TR2_PC:
			tr1pc_setzone(level, error, room, column,
			              row, 0x00000000U, boxIndex);
			break;
		case TRMOD_LEVTYPE_TR1_SS:
			tr1pc_setzone(level, error, room, column,
			              row, 0x00000010U, boxIndex);
			break;
		case TRMOD_LEVTYPE_TR3_PC:
			tr3pc_setzone(level, error, room, column, row, boxIndex);
			break;
		default:
			error->code = ERROR_UNIMPLEMENTED_FUNCTION;
			break;
	}
}

/*
 * Wrapper function for get step sound
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
void get_stepsound(struct level *level, struct error *error, uint16 room,
                   uint16 column, uint16 row, uint16 print)
{
	/* Determines how to run the command based on leveltype */
	switch (level->type)
	{
		case TRMOD_LEVTYPE_TR1_PC:
		case TRMOD_LEVTYPE_TR1G_PC:
		case TRMOD_LEVTYPE_TR2_PC:
			break;
		case TRMOD_LEVTYPE_TR3_PC:
			tr3pc_getstepsound(level, error, room, column, row, print);
			break;
		default:
			error->code = ERROR_UNIMPLEMENTED_FUNCTION;
			break;
	}
}

/*
 * Wrapper function for set step sound
 * Parameters:
 * * level = Pointer to level struct
 * * error = Pointer to error struct
 * * room = Room number
 * * column = Column number
 * * row = Row number
 * * stepsound = stepsound to write to the sector
 */
void set_stepsound(struct level *level, struct error *error, uint16 room,
                   uint16 column, uint16 row, uint16 stepsound)
{
	/* Determines how to run the command based on leveltype */
	switch (level->type)
	{
		case TRMOD_LEVTYPE_TR1_PC:
		case TRMOD_LEVTYPE_TR1G_PC:
		case TRMOD_LEVTYPE_TR2_PC:
			break;
		case TRMOD_LEVTYPE_TR3_PC:
			tr3pc_setstepsound(level, error, room, column, row, stepsound);
			break;
		default:
			error->code = ERROR_UNIMPLEMENTED_FUNCTION;
			break;
	}
}

/*
 * Wrapper function for get vertex
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
void get_vertex(struct level *level, struct error *error,
                uint16 room, uint16 vertex, uint16 print)
{
	/* Determines how to run the command based on leveltype */
	switch (level->type)
	{
		case TRMOD_LEVTYPE_TR1_PC:
		case TRMOD_LEVTYPE_TR1G_PC:
			tr1pc_get_vertex(level, error, room, vertex, print);
			break;
		case TRMOD_LEVTYPE_TR2_PC:
			tr2pc_get_vertex(level, error, room, vertex, print);
			break;
		case TRMOD_LEVTYPE_TR3_PC:
			tr3pc_get_vertex(level, error, room, vertex, print);
			break;
		default:
			error->code = ERROR_UNIMPLEMENTED_FUNCTION;
			break;
	}
}

/*
 * Wrapper function for add vertex
 * Parameters:
 * * level = Pointer to level struct
 * * error = Pointer to error struct
 * * room = Room number
 * * x = X-position of the vertex (in local coordinates)
 * * y = Y-position of the vertex (in local coordinates)
 * * z = Z-position of the vertex (in local coordinates)
 * * intensity1 = Intensity1 of the vertex (string)
 * * intensity2 = Intensity2 of the vertex (string)
 * * attributes = Attributes of the vertex
 */
void add_vertex(struct level *level, struct error *error, uint16 room,
                int32 x, int32 y, int32 z, char *intensity1,
                char *intensity2, uint16 attributes)
{
	/* Determines how to run the command based on leveltype */
	switch (level->type)
	{
		case TRMOD_LEVTYPE_TR1_PC:
		case TRMOD_LEVTYPE_TR1G_PC:
		case TRMOD_LEVTYPE_TR2_PC:
		case TRMOD_LEVTYPE_TR3_PC:
			tr1pc_add_vertex(level, error, room);
			if (error->code != ERROR_NONE)
			{
				return;
			}
			replace_vertex(level, error, room, 0xFFFF, x, y, z,
			               intensity1, intensity2, attributes, 0x003F);
			break;
		default:
			error->code = ERROR_UNIMPLEMENTED_FUNCTION;
			break;
	}
}

/*
 * Wrapper function for replace vertex
 * Parameters:
 * * level = Pointer to level struct
 * * error = Pointer to error struct
 * * room = Room number
 * * vertex = Vertex number (max int means last one)
 * * x = X-position of the vertex (in local coordinates)
 * * y = Y-position of the vertex (in local coordinates)
 * * z = Z-position of the vertex (in local coordinates)
 * * intensity1 = Intensity1 of the vertex (string)
 * * intensity2 = Intensity2 of the vertex (string)
 * * attributes = Attributes of the vertex
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
void replace_vertex(struct level *level, struct error *error, uint16 room,
                    uint16 vertex, int32 x, int32 y, int32 z, char *intensity1,
                    char *intensity2, uint16 attributes, uint16 rep)
{
	/* Variable Declarations */
	int16 s16[2] = {0x0000, 0x0000}; /* Signed 16-bit integers */
	
	/* Determines how to run the command based on leveltype */
	switch (level->type)
	{
		case TRMOD_LEVTYPE_TR1_PC:
		case TRMOD_LEVTYPE_TR1G_PC:
			s16[0] = convertLight(intensity1, 0);
			if (error->code == ERROR_NONE)
			{
				tr1pc_replace_vertex(level, error, room, vertex,
				                     x, y, z, s16[0], rep);
			}
			break;
		case TRMOD_LEVTYPE_TR2_PC:
			s16[0] = convertLight(intensity1, 0);
			s16[1] = convertLight(intensity2, 0);
			if (error->code == ERROR_NONE)
			{
				tr2pc_replace_vertex(level, error, room, vertex,
				                     x, y, z, s16[0], s16[1],
				                     attributes, rep);
			}
			break;
		case TRMOD_LEVTYPE_TR3_PC:
			s16[0] = convertLight(intensity1, 1);
			s16[1] = convertLight(intensity2, 1);
			if (error->code == ERROR_NONE)
			{
				tr3pc_replace_vertex(level, error, room, vertex,
				                     x, y, z, s16[0], s16[1],
				                     attributes, rep);
			}
			break;
		default:
			error->code = ERROR_UNIMPLEMENTED_FUNCTION;
			break;
	}
}

/*
 * Wrapper function for remove vertex
 * Parameters:
 * * level = Pointer to level struct
 * * error = Pointer to error struct
 * * room = Room number
 * * vertex = Vertex number
 */
void remove_vertex(struct level *level, struct error *error,
                   uint16 room, uint16 vertex)
{
	/* Determines how to run the command based on leveltype */
	switch (level->type)
	{
		case TRMOD_LEVTYPE_TR1_PC:
		case TRMOD_LEVTYPE_TR1G_PC:
		case TRMOD_LEVTYPE_TR2_PC:
		case TRMOD_LEVTYPE_TR3_PC:
			tr1pc_remove_vertex(level, error, room, vertex);
			break;
		default:
			error->code = ERROR_UNIMPLEMENTED_FUNCTION;
			break;
	}
}

/*
 * Wrapper function for remove geometry
 * Parameters:
 * * level = Pointer to level struct
 * * error = Pointer to error struct
 * * room = Room number
 */
void remove_geometry(struct level *level, struct error *error, uint16 room)
{
	/* Determines how to run the command based on leveltype */
	switch (level->type)
	{
		case TRMOD_LEVTYPE_TR1_PC:
		case TRMOD_LEVTYPE_TR1G_PC:
		case TRMOD_LEVTYPE_TR2_PC:
		case TRMOD_LEVTYPE_TR3_PC:
			tr1pc_remove_geometry(level, error, room);
			break;
		default:
			error->code = ERROR_UNIMPLEMENTED_FUNCTION;
			break;
	}
}

/*
 * Wrapper function for get rectangle
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
void get_rectangle(struct level *level, struct error *error,
                   uint16 room, uint16 rectangle, uint16 print)
{
	/* Determines how to run the command based on leveltype */
	switch (level->type)
	{
		case TRMOD_LEVTYPE_TR1_PC:
		case TRMOD_LEVTYPE_TR1G_PC:
		case TRMOD_LEVTYPE_TR2_PC:
		case TRMOD_LEVTYPE_TR3_PC:
			tr1pc_get_rectangle(level, error, room, rectangle, print);
			break;
		default:
			error->code = ERROR_UNIMPLEMENTED_FUNCTION;
			break;
	}
}

/*
 * Wrapper function for add rectangle
 * Parameters:
 * * level = Pointer to level struct
 * * error = Pointer to error struct
 * * room = Room number
 * * vertex1 = Vertex 1 of the rectangle
 * * vertex2 = Vertex 2 of the rectangle
 * * vertex3 = Vertex 3 of the rectangle
 * * vertex4 = Vertex 4 of the rectangle
 * * texture = Texture of the rectangle
 * * doublesided = Whether the rectangle is double-sided
 */
void add_rectangle(struct level *level, struct error *error, uint16 room,
                   uint16 vertex1, uint16 vertex2, uint16 vertex3,
                   uint16 vertex4, uint16 texture, uint16 doublesided)
{
	/* Determines how to run the command based on leveltype */
	switch (level->type)
	{
		case TRMOD_LEVTYPE_TR1_PC:
		case TRMOD_LEVTYPE_TR1G_PC:
		case TRMOD_LEVTYPE_TR2_PC:
		case TRMOD_LEVTYPE_TR3_PC:
			tr1pc_add_rectangle(level, error, room);
			if (error->code != ERROR_NONE)
			{
				return;
			}
			replace_rectangle(level, error, room, 0xFFFF, vertex1, vertex2,
			                  vertex3, vertex4, texture, doublesided, 0x003F);
			break;
		default:
			error->code = ERROR_UNIMPLEMENTED_FUNCTION;
			break;
	}
}

/*
 * Wrapper function for replace rectangle
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
void replace_rectangle(struct level *level, struct error *error, uint16 room,
                       uint16 rectangle, uint16 vertex1, uint16 vertex2,
                       uint16 vertex3, uint16 vertex4, uint16 texture,
                       uint16 doublesided, uint16 rep)
{
	/* Determines how to run the command based on leveltype */
	switch (level->type)
	{
		case TRMOD_LEVTYPE_TR1_PC:
		case TRMOD_LEVTYPE_TR1G_PC:
		case TRMOD_LEVTYPE_TR2_PC:
		case TRMOD_LEVTYPE_TR3_PC:
			tr1pc_replace_rectangle(level, error, room, rectangle,
			                        vertex1, vertex2, vertex3, vertex4,
			                        texture, doublesided, rep);
			break;
		default:
			error->code = ERROR_UNIMPLEMENTED_FUNCTION;
			break;
	}
}

/*
 * Wrapper function for remove rectangle
 * Parameters:
 * * level = Pointer to level struct
 * * error = Pointer to error struct
 * * room = Room number
 * * rectangle = Rectangle number
 */
void remove_rectangle(struct level *level, struct error *error,
                      uint16 room, uint16 rectangle)
{
	/* Determines how to run the command based on leveltype */
	switch (level->type)
	{
		case TRMOD_LEVTYPE_TR1_PC:
		case TRMOD_LEVTYPE_TR1G_PC:
		case TRMOD_LEVTYPE_TR2_PC:
		case TRMOD_LEVTYPE_TR3_PC:
			tr1pc_remove_rectangle(level, error, room, rectangle);
			break;
		default:
			error->code = ERROR_UNIMPLEMENTED_FUNCTION;
			break;
	}
}

/*
 * Wrapper function for remove all rectangles
 * Parameters:
 * * level = Pointer to level struct
 * * error = Pointer to error struct
 * * room = Room number
 */
void remove_all_rectangles(struct level *level,
                           struct error *error, uint16 room)
{
	/* Determines how to run the command based on leveltype */
	switch (level->type)
	{
		case TRMOD_LEVTYPE_TR1_PC:
		case TRMOD_LEVTYPE_TR1G_PC:
		case TRMOD_LEVTYPE_TR2_PC:
		case TRMOD_LEVTYPE_TR3_PC:
			tr1pc_remove_all_rectangles(level, error, room);
			break;
		default:
			error->code = ERROR_UNIMPLEMENTED_FUNCTION;
			break;
	}
}

/*
 * Wrapper function for get triangle
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
void get_triangle(struct level *level, struct error *error,
                  uint16 room, uint16 triangle, uint16 print)
{
	/* Determines how to run the command based on leveltype */
	switch (level->type)
	{
		case TRMOD_LEVTYPE_TR1_PC:
		case TRMOD_LEVTYPE_TR1G_PC:
		case TRMOD_LEVTYPE_TR2_PC:
		case TRMOD_LEVTYPE_TR3_PC:
			tr1pc_get_triangle(level, error, room, triangle, print);
			break;
		default:
			error->code = ERROR_UNIMPLEMENTED_FUNCTION;
			break;
	}
}

/*
 * Wrapper function for add triangle
 * Parameters:
 * * level = Pointer to level struct
 * * error = Pointer to error struct
 * * room = Room number
 * * vertex1 = Vertex 1 of the triangle
 * * vertex2 = Vertex 2 of the triangle
 * * vertex3 = Vertex 3 of the triangle
 * * texture = Texture of the triangle
 * * doublesided = Whether the triangle is double-sided
 */
void add_triangle(struct level *level, struct error *error, uint16 room,
                  uint16 vertex1, uint16 vertex2, uint16 vertex3,
                  uint16 texture, uint16 doublesided)
{
	/* Determines how to run the command based on leveltype */
	switch (level->type)
	{
		case TRMOD_LEVTYPE_TR1_PC:
		case TRMOD_LEVTYPE_TR1G_PC:
		case TRMOD_LEVTYPE_TR2_PC:
		case TRMOD_LEVTYPE_TR3_PC:
			tr1pc_add_triangle(level, error, room);
			if (error->code != ERROR_NONE)
			{
				return;
			}
			replace_triangle(level, error, room, 0xFFFF, vertex1, vertex2,
			                 vertex3, texture, doublesided, 0x001F);
			break;
		default:
			error->code = ERROR_UNIMPLEMENTED_FUNCTION;
			break;
	}
}

/*
 * Wrapper function for replace triangle
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
void replace_triangle(struct level *level, struct error *error,
                      uint16 room, uint16 triangle, uint16 vertex1,
                      uint16 vertex2, uint16 vertex3, uint16 texture,
                      uint16 doublesided, uint16 rep)
{
	/* Determines how to run the command based on leveltype */
	switch (level->type)
	{
		case TRMOD_LEVTYPE_TR1_PC:
		case TRMOD_LEVTYPE_TR1G_PC:
		case TRMOD_LEVTYPE_TR2_PC:
		case TRMOD_LEVTYPE_TR3_PC:
			tr1pc_replace_triangle(level, error, room, triangle,
			                        vertex1, vertex2, vertex3,
			                        texture, doublesided, rep);
			break;
		default:
			error->code = ERROR_UNIMPLEMENTED_FUNCTION;
			break;
	}
}

/*
 * Wrapper function for remove triangle
 * Parameters:
 * * level = Pointer to level struct
 * * error = Pointer to error struct
 * * room = Room number
 * * triangle = Triangle number
 */
void remove_triangle(struct level *level, struct error *error,
                     uint16 room, uint16 triangle)
{
	/* Determines how to run the command based on leveltype */
	switch (level->type)
	{
		case TRMOD_LEVTYPE_TR1_PC:
		case TRMOD_LEVTYPE_TR1G_PC:
		case TRMOD_LEVTYPE_TR2_PC:
		case TRMOD_LEVTYPE_TR3_PC:
			tr1pc_remove_triangle(level, error, room, triangle);
			break;
		default:
			error->code = ERROR_UNIMPLEMENTED_FUNCTION;
			break;
	}
}

/*
 * Wrapper function for remove all triangles
 * Parameters:
 * * level = Pointer to level struct
 * * error = Pointer to error struct
 * * room = Room number
 */
void remove_all_triangles(struct level *level,
                           struct error *error, uint16 room)
{
	/* Determines how to run the command based on leveltype */
	switch (level->type)
	{
		case TRMOD_LEVTYPE_TR1_PC:
		case TRMOD_LEVTYPE_TR1G_PC:
		case TRMOD_LEVTYPE_TR2_PC:
		case TRMOD_LEVTYPE_TR3_PC:
			tr1pc_remove_all_triangles(level, error, room);
			break;
		default:
			error->code = ERROR_UNIMPLEMENTED_FUNCTION;
			break;
	}
}

/*
 * Wrapper function for get sprite
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
void get_sprite(struct level *level, struct error *error,
                uint16 room, uint16 sprite, uint16 print)
{
	/* Determines how to run the command based on leveltype */
	switch (level->type)
	{
		case TRMOD_LEVTYPE_TR1_PC:
		case TRMOD_LEVTYPE_TR1G_PC:
		case TRMOD_LEVTYPE_TR2_PC:
		case TRMOD_LEVTYPE_TR3_PC:
			tr1pc_get_sprite(level, error, room, sprite, print);
			break;
		default:
			error->code = ERROR_UNIMPLEMENTED_FUNCTION;
			break;
	}
}

/*
 * Wrapper function for add sprite
 * Parameters:
 * * level = Pointer to level struct
 * * error = Pointer to error struct
 * * id = Sprite ID
 * * room = Room number
 * * x = X-position of the vertex (in local coordinates)
 * * y = Y-position of the vertex (in local coordinates)
 * * z = Z-position of the vertex (in local coordinates)
 * * intensity1 = Intensity1 of the vertex (string)
 * * intensity2 = Intensity2 of the vertex (string)
 * * attributes = Attributes of the vertex
 */
void add_sprite(struct level *level, struct error *error, uint16 id,
                uint16 room, int32 x, int32 y, int32 z, char *intensity1,
                char *intensity2, uint16 attributes)
{
	/* Determines how to run the command based on leveltype */
	switch (level->type)
	{
		case TRMOD_LEVTYPE_TR1_PC:
		case TRMOD_LEVTYPE_TR1G_PC:
		case TRMOD_LEVTYPE_TR2_PC:
		case TRMOD_LEVTYPE_TR3_PC:
			tr1pc_add_sprite(level, error, room);
			if (error->code != ERROR_NONE)
			{
				return;
			}
			replace_sprite(level, error, room, 0xFFFF, id, x, y, z,
			               intensity1, intensity2, attributes, 0x007F);
			break;
		default:
			error->code = ERROR_UNIMPLEMENTED_FUNCTION;
			break;
	}
}

/*
 * Wrapper function for replace sprite
 * Parameters:
 * * level = Pointer to level struct
 * * error = Pointer to error struct
 * * room = Room number
 * * sprite = Sprite number (max int means last one)
 * * id = Sprite ID
 * * x = X-position of the vertex (in local coordinates)
 * * y = Y-position of the vertex (in local coordinates)
 * * z = Z-position of the vertex (in local coordinates)
 * * intensity1 = Intensity1 of the vertex (string)
 * * intensity2 = Intensity2 of the vertex (string)
 * * attributes = Attributes of the vertex
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
void replace_sprite(struct level *level, struct error *error, uint16 room,
                    uint16 sprite, uint16 id, int32 x, int32 y, int32 z,
                    char *intensity1, char *intensity2, uint16 attributes,
                    uint16 rep)
{
	/* Variable Declarations */
	int16 s16[2];
	
	/* Determines how to run the command based on leveltype */
	switch (level->type)
	{
		case TRMOD_LEVTYPE_TR1_PC:
		case TRMOD_LEVTYPE_TR1G_PC:
			s16[0] = convertLight(intensity1, 0);
			if (error->code == ERROR_NONE)
			{
				tr1pc_replace_sprite(level, error, room, sprite,
				                     id, x, y, z, s16[0], rep);
			}
			break;
		case TRMOD_LEVTYPE_TR2_PC:
			s16[0] = convertLight(intensity1, 0);
			s16[1] = convertLight(intensity2, 0);
			if (error->code == ERROR_NONE)
			{
				tr2pc_replace_sprite(level, error, room, sprite, id, x,
				                     y, z, s16[0], s16[1], attributes, rep);
			}
			break;
		case TRMOD_LEVTYPE_TR3_PC:
			s16[1] = convertLight(intensity1, 1);
			s16[0] = convertLight(intensity2, 1);
			if (error->code == ERROR_NONE)
			{
				tr3pc_replace_sprite(level, error, room, sprite, id, x,
				                     y, z, s16[0], s16[1], attributes, rep);
			}
			break;
		default:
			error->code = ERROR_UNIMPLEMENTED_FUNCTION;
			break;
	}
}

/*
 * Wrapper function for remove sprite
 * Parameters:
 * * level = Pointer to level struct
 * * error = Pointer to error struct
 * * room = Room number
 * * sprite = Sprite number
 */
void remove_sprite(struct level *level, struct error *error,
                   uint16 room, uint16 sprite)
{
	/* Determines how to run the command based on leveltype */
	switch (level->type)
	{
		case TRMOD_LEVTYPE_TR1_PC:
		case TRMOD_LEVTYPE_TR1G_PC:
		case TRMOD_LEVTYPE_TR2_PC:
		case TRMOD_LEVTYPE_TR3_PC:
			tr1pc_remove_sprite(level, error, room, sprite);
			break;
		default:
			error->code = ERROR_UNIMPLEMENTED_FUNCTION;
			break;
	}
}

/*
 * Wrapper function for get viewport
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
void get_viewport(struct level *level, struct error *error,
                  uint16 room, uint16 viewport, uint16 print)
{
	/* Determines how to run the command based on leveltype */
	switch (level->type)
	{
		case TRMOD_LEVTYPE_TR1_PC:
		case TRMOD_LEVTYPE_TR1G_PC:
		case TRMOD_LEVTYPE_TR1_SS:
		case TRMOD_LEVTYPE_TR2_PC:
		case TRMOD_LEVTYPE_TR3_PC:
			tr1pc_get_viewport(level, error, room, viewport, print);
			break;
		default:
			error->code = ERROR_UNIMPLEMENTED_FUNCTION;
			break;
	}
}

/*
 * Wrapper function for add viewport
 * Parameters:
 * * level = Pointer to level struct
 * * error = Pointer to error struct
 * * room = Room number
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
 */
void add_viewport(struct level *level, struct error *error, uint16 room,
                  uint16 adjroom, int16 n_x, int16 n_y, int16 n_z, int16 v1_x,
                  int16 v1_y, int16 v1_z, int16 v2_x, int16 v2_y, int16 v2_z,
                  int16 v3_x, int16 v3_y, int16 v3_z, int16 v4_x, int16 v4_y,
                  int16 v4_z)
{
	/* Determines how to run the command based on leveltype */
	switch (level->type)
	{
		case TRMOD_LEVTYPE_TR1_PC:
		case TRMOD_LEVTYPE_TR1G_PC:
		case TRMOD_LEVTYPE_TR1_SS:
		case TRMOD_LEVTYPE_TR2_PC:
		case TRMOD_LEVTYPE_TR3_PC:
			tr1pc_add_viewport(level, error, room);
			if (error->code != ERROR_NONE)
			{
				return;
			}
			replace_viewport(level, error, room, 0xFFFF, adjroom, n_x, n_y,
			                 n_z, v1_x, v1_y, v1_z, v2_x, v2_y, v2_z, v3_x,
			                 v3_y, v3_z, v4_x, v4_y, v4_z, 0xFFFF);
			break;
		default:
			error->code = ERROR_UNIMPLEMENTED_FUNCTION;
			break;
	}
}

/*
 * Wrapper function for replace viewport
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
void replace_viewport(struct level *level, struct error *error,
                      uint16 room, uint16 viewport, uint16 adjroom,
                      int16 n_x, int16 n_y, int16 n_z, int16 v1_x,
                      int16 v1_y, int16 v1_z, int16 v2_x, int16 v2_y,
                      int16 v2_z, int16 v3_x, int16 v3_y, int16 v3_z,
                      int16 v4_x, int16 v4_y, int16 v4_z, uint16 rep)
{
	/* Determines how to run the command based on leveltype */
	switch (level->type)
	{
		case TRMOD_LEVTYPE_TR1_PC:
		case TRMOD_LEVTYPE_TR1G_PC:
		case TRMOD_LEVTYPE_TR1_SS:
		case TRMOD_LEVTYPE_TR2_PC:
		case TRMOD_LEVTYPE_TR3_PC:
			tr1pc_replace_viewport(level, error, room, viewport, adjroom, n_x,
			                       n_y, n_z, v1_x, v1_y, v1_z, v2_x, v2_y, v2_z,
			                       v3_x, v3_y, v3_z, v4_x, v4_y, v4_z, rep);
			break;
		default:
			error->code = ERROR_UNIMPLEMENTED_FUNCTION;
			break;
	}
}

/*
 * Wrapper function for replace viewport
 * Parameters:
 * * level = Pointer to level struct
 * * error = Pointer to error struct
 * * room = Room number
 * * viewport = Viewport number
 */
void remove_viewport(struct level *level, struct error *error,
                     uint16 room, uint16 viewport)
{
	/* Determines how to run the command based on leveltype */
	switch (level->type)
	{
		case TRMOD_LEVTYPE_TR1_PC:
		case TRMOD_LEVTYPE_TR1G_PC:
		case TRMOD_LEVTYPE_TR1_SS:
		case TRMOD_LEVTYPE_TR2_PC:
		case TRMOD_LEVTYPE_TR3_PC:
			tr1pc_remove_viewport(level, error, room, viewport);
			break;
		default:
			error->code = ERROR_UNIMPLEMENTED_FUNCTION;
			break;
	}
}

/*
 * Wrapper function for remove all viewports
 * Parameters:
 * * level = Pointer to level struct
 * * error = Pointer to error struct
 * * room = Room number
 */
void remove_all_viewports(struct level *level,
                          struct error *error, uint16 room)
{
	/* Determines how to run the command based on leveltype */
	switch (level->type)
	{
		case TRMOD_LEVTYPE_TR1_PC:
		case TRMOD_LEVTYPE_TR1G_PC:
		case TRMOD_LEVTYPE_TR1_SS:
		case TRMOD_LEVTYPE_TR2_PC:
		case TRMOD_LEVTYPE_TR3_PC:
			tr1pc_remove_all_viewports(level, error, room);
			break;
		default:
			error->code = ERROR_UNIMPLEMENTED_FUNCTION;
			break;
	}
}

/*
 * Wrapper function for get roomlight
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
void get_roomlight(struct level *level, struct error *error,
                   uint16 room, uint16 print)
{
	/* Determines how to run the command based on leveltype */
	switch (level->type)
	{
		case TRMOD_LEVTYPE_TR1_PC:
		case TRMOD_LEVTYPE_TR1G_PC:
			tr1pc_get_roomlight(level, error, room, print);
			break;
		case TRMOD_LEVTYPE_TR2_PC:
			tr2pc_get_roomlight(level, error, room, print);
			break;
		case TRMOD_LEVTYPE_TR3_PC:
			tr3pc_get_roomlight(level, error, room, print);
			break;
		default:
			error->code = ERROR_UNIMPLEMENTED_FUNCTION;
			break;
	}
}

/*
 * Wrapper function for roomlight
 * Parameters:
 * * level = Pointer to level struct
 * * error = Pointer to error struct
 * * room = Room the light is in
 * * intensity1 = String of ambientitensity1
 * * intensity2 = String of ambientitensity2
 * * lightmode = Lightmode
 */
void roomlight(struct level *level, struct error *error, uint16 room,
               char *intensity1, char *intensity2, uint16 lightmode)
{
	/* Variable Declarations */
	int16 s16[2] = {0x0000, 0x0000}; /* 16-bit unsigned integer */
	
	/* Determines how to run the command based on leveltype */
	switch (level->type)
	{
		case TRMOD_LEVTYPE_TR1_PC:
		case TRMOD_LEVTYPE_TR1G_PC:
			s16[0] = convertLight(intensity1, 0);
			if (error->code == ERROR_NONE)
			{
				tr1pc_roomlight(level, error, room, s16[0]);
			}
			break;
		case TRMOD_LEVTYPE_TR2_PC:
			s16[0] = convertLight(intensity1, 0);
			s16[1] = convertLight(intensity2, 0);
			if (error->code == ERROR_NONE)
			{
				tr2pc_roomlight(level, error, room,
				                s16[0], s16[1], lightmode);
			}
			break;
		case TRMOD_LEVTYPE_TR3_PC:
			s16[0] = convertLight(intensity1, 0);
			s16[1] = convertLight(intensity2, 0);
			if (error->code == ERROR_NONE)
			{
				tr3pc_roomlight(level, error, room, s16[0], s16[1]);
			}
			break;
		default:
			error->code = ERROR_UNIMPLEMENTED_FUNCTION;
			break;
	}
}

/*
 * Wrapper function for get alternate room
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
void get_altroom(struct level *level, struct error *error,
                 uint16 room, uint16 print)
{
	/* Determines how to run the command based on leveltype */
	switch (level->type)
	{
		case TRMOD_LEVTYPE_TR1_PC:
		case TRMOD_LEVTYPE_TR1G_PC:
		case TRMOD_LEVTYPE_TR1_SS:
		case TRMOD_LEVTYPE_TR2_PC:
		case TRMOD_LEVTYPE_TR3_PC:
			tr1pc_get_altroom(level, error, room, print);
			break;
		default:
			error->code = ERROR_UNIMPLEMENTED_FUNCTION;
			break;
	}
}

/*
 * Wrapper function for alternate room
 * Parameters:
 * * level = Pointer to level struct
 * * error = Pointer to error struct
 * * room = Room number
 * * altroom = Alternate room
 */
void altroom(struct level *level, struct error *error,
             uint16 room, int16 altroom)
{
	/* Determines how to run the command based on leveltype */
	switch (level->type)
	{
		case TRMOD_LEVTYPE_TR1_PC:
		case TRMOD_LEVTYPE_TR1G_PC:
		case TRMOD_LEVTYPE_TR1_SS:
		case TRMOD_LEVTYPE_TR2_PC:
		case TRMOD_LEVTYPE_TR3_PC:
			tr1pc_altroom(level, error, room, altroom);
			break;
		default:
			error->code = ERROR_UNIMPLEMENTED_FUNCTION;
			break;
	}
}

/*
 * Wrapper function for remove all sprites
 * Parameters:
 * * level = Pointer to level struct
 * * error = Pointer to error struct
 * * room = Room number
 */
void remove_all_sprites(struct level *level, struct error *error, uint16 room)
{
	/* Determines how to run the command based on leveltype */
	switch (level->type)
	{
		case TRMOD_LEVTYPE_TR1_PC:
		case TRMOD_LEVTYPE_TR1G_PC:
		case TRMOD_LEVTYPE_TR2_PC:
		case TRMOD_LEVTYPE_TR3_PC:
			tr1pc_remove_all_sprites(level, error, room);
			break;
		default:
			error->code = ERROR_UNIMPLEMENTED_FUNCTION;
			break;
	}
}

/*
 * Function that prints a room's flags
 * Parameters:
 * * level = Pointer to level struct
 * * error = Pointer to error struct
 * * room = Room number
 * * flagbits = Bits from the flags we care about
 * * print = Printing flags
 * * * 0 0 0 0  0 0 0 0   0 0 0 0  0 0 0 0
 * * * | | | |  | | | |   | | | |  | | | |
 * * * | | | |  | | | |   | | | |  | | | As command
 * * * | | | |  | | | |   | | | |  | | Also print negative
 * * * Unused
 */
void get_roomflags(struct level *level, struct error *error,
                   uint16 room, uint16 flagbits, uint16 print)
{
	/* Determines how to run the command based on leveltype */
	switch (level->type)
	{
		case TRMOD_LEVTYPE_TR1_PC:
		case TRMOD_LEVTYPE_TR1G_PC:
		case TRMOD_LEVTYPE_TR2_PC:
		case TRMOD_LEVTYPE_TR3_PC:
			tr1pc_get_roomflags(level, error, room,
			                    0x00000000U, flagbits, print);
			break;
		case TRMOD_LEVTYPE_TR1_SS:
			tr1pc_get_roomflags(level, error, room,
			                    0x00000010U, flagbits, print);
			break;
		default:
			error->code = ERROR_UNIMPLEMENTED_FUNCTION;
			break;
	}
}

/*
 * Wrapper function for setroomflags
 * Parameters:
 * * level = Pointer to level struct
 * * error = Pointer to error struct
 * * room = Room number to apply the flags to
 * * mask = Bit-mask for the flags to apply
 * * operation = (0 = AND, 1 = OR, 2 = ASSIGN)
 */
void roomflags(struct level *level, struct error *error, uint16 room,
               uint16 mask, int operation)
{
	/* Determines how to run the command based on leveltype */
	switch (level->type)
	{
		case TRMOD_LEVTYPE_TR1_PC:
		case TRMOD_LEVTYPE_TR1G_PC:
		case TRMOD_LEVTYPE_TR2_PC:
		case TRMOD_LEVTYPE_TR3_PC:
			tr1pc_setroomflags(level, error, room,
			                   0x00000000U, mask, operation);
			break;
		case TRMOD_LEVTYPE_TR1_SS:
			tr1pc_setroomflags(level, error, room,
			                   0x00000010U, mask, operation);
			break;
		default:
			error->code = ERROR_UNIMPLEMENTED_FUNCTION;
			break;
	}
}

/*
 * Wrapper function for get box
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
void get_box(struct level *level, struct error *error,
             uint32 box, uint16 print)
{
	/* Determines how to run the command based on leveltype */
	switch (level->type)
	{
		case TRMOD_LEVTYPE_TR1_PC:
		case TRMOD_LEVTYPE_TR1G_PC:
			tr1pc_getbox(level, error, box, print);
			break;
		case TRMOD_LEVTYPE_TR2_PC:
		case TRMOD_LEVTYPE_TR3_PC:
			tr2pc_getbox(level, error, box, print);
			break;
		default:
			error->code = ERROR_UNIMPLEMENTED_FUNCTION;
			break;
	}
}

/*
 * Wrapper function for add box
 * Parameters:
 * * level = Pointer to level struct
 * * error = Pointer to error struct
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
 */
void add_box(struct level *level, struct error *error, uint16 room, int32 xmin,
             int32 xmax, int32 zmin, int32 zmax, int16 floor, int16 gz1,
             int16 gz2, int16 gz3, int16 gz4, int16 fz, int16 agz1, int16 agz2,
             int16 agz3, int16 agz4, int16 afz, char *overlapstr, int blocked,
             int blockable)
{
	/* Determines how to run the command based on leveltype */
	switch (level->type)
	{
		case TRMOD_LEVTYPE_TR1_PC:
		case TRMOD_LEVTYPE_TR1G_PC:
			tr1pc_add_box(level, error);
			if (error->code != ERROR_NONE)
			{
				return;
			}
			replace_box(level, error, 0xFFFFFFFF, room, xmin, xmax, zmin, zmax,
			            floor, gz1, gz2, gz3, gz4, fz, agz1, agz2, agz3, agz4,
			            afz, overlapstr, blocked, blockable, 0xFFFFFFFF);
			break;
		case TRMOD_LEVTYPE_TR2_PC:
		case TRMOD_LEVTYPE_TR3_PC:
			tr2pc_add_box(level, error);
			if (error->code != ERROR_NONE)
			{
				return;
			}
			replace_box(level, error, 0xFFFFFFFF, room, xmin, xmax, zmin, zmax,
			            floor, gz1, gz2, gz3, gz4, fz, agz1, agz2, agz3, agz4,
			            afz, overlapstr, blocked, blockable, 0xFFFFFFFF);
			break;
		default:
			error->code = ERROR_UNIMPLEMENTED_FUNCTION;
			break;
	}
}

/*
 * Wrapper function for replace box
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
void replace_box(struct level *level, struct error *error, uint32 box,
                 uint16 room, int32 xmin, int32 xmax, int32 zmin, int32 zmax,
                 int16 floor, int16 gz1, int16 gz2, int16 gz3, int16 gz4,
                 int16 fz, int16 agz1, int16 agz2, int16 agz3, int16 agz4,
                 int16 afz, char *overlapstr, int blocked, int blockable,
                 uint32 rep)
{
	/* Determines how to run the command based on leveltype */
	switch (level->type)
	{
		case TRMOD_LEVTYPE_TR1_PC:
		case TRMOD_LEVTYPE_TR1G_PC:
			tr1pc_replace_box(level, error, box, room, xmin, xmax, zmin, zmax,
			                  floor, gz1, gz2, fz, agz1, agz2, afz, overlapstr,
			                  blocked, blockable, rep);
			break;
		case TRMOD_LEVTYPE_TR2_PC:
		case TRMOD_LEVTYPE_TR3_PC:
			tr2pc_replace_box(level, error, box, room, xmin, xmax, zmin, zmax,
			                  floor, gz1, gz2, gz3, gz4, fz, agz1, agz2, agz3,
			                  agz4, afz, overlapstr, blocked, blockable, rep);
			break;
		default:
			error->code = ERROR_UNIMPLEMENTED_FUNCTION;
			break;
	}
}

/*
 * Wrapper function for remove all boxes
 * Parameters:
 * * level = Pointer to level struct
 * * error = Pointer to error struct
 */
void remove_all_boxes(struct level *level, struct error *error)
{
	/* Determines how to run the command based on leveltype */
	switch (level->type)
	{
		case TRMOD_LEVTYPE_TR1_PC:
		case TRMOD_LEVTYPE_TR1G_PC:
		case TRMOD_LEVTYPE_TR2_PC:
		case TRMOD_LEVTYPE_TR3_PC:
			tr1pc_remove_all_boxes(level, error);
			break;
		default:
			error->code = ERROR_UNIMPLEMENTED_FUNCTION;
			break;
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
void hexdump(struct level *level, struct error *error, uint16 print)
{
	/* Determines how to run the command based on leveltype */
	switch (level->type)
	{
		case TRMOD_LEVTYPE_TR1_PC:
		case TRMOD_LEVTYPE_TR1G_PC:
		case TRMOD_LEVTYPE_TR1_PS:
		case TRMOD_LEVTYPE_TR1_SS:
		case TRMOD_LEVTYPE_TR2_PC:
		case TRMOD_LEVTYPE_TR2_PS:
		case TRMOD_LEVTYPE_TR3_PC:
		case TRMOD_LEVTYPE_TR3_PS:
			tr1pc_hexdump(level, error, print);
			break;
		default:
			error->code = ERROR_UNIMPLEMENTED_FUNCTION;
			break;
	}
}

/*
 * Wrapper function that finds the nearest room to a given point
 * Parameters:
 * * level = Pointer to the level struct
 * * x = X-coordinate of the point (in world coordinates)
 * * y = Y-coordinate of the point (in world coordinates)
 * * z = Z-coordinate of the point (in world coordinates)
 * Returns the nearest room
 */
uint16 nearestRoom(struct level *level, struct error *error,
                   int32 x, int32 y, int32 z)
{
	/* Variable Declarations */
	uint16 retval = 0x0000; /* Return value for this function */
	
	/* Determines how to run the command based on leveltype */
	switch (level->type)
	{
		case TRMOD_LEVTYPE_TR1_PC:
		case TRMOD_LEVTYPE_TR1G_PC:
		case TRMOD_LEVTYPE_TR1_SS:
		case TRMOD_LEVTYPE_TR2_PC:
		case TRMOD_LEVTYPE_TR3_PC:
			retval = tr1pc_nearestRoom(level, x, y, z);
			break;
		default:
			error->code = ERROR_UNIMPLEMENTED_FUNCTION;
			break;
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
uint16 addcamera(struct level *level, struct error *error, uint8 *camera)
{
	/* Variable Declarations */
	uint16 retval = 0x0000; /* Return value for this function */
	
	/* Determines how to run the command based on leveltype */
	switch (level->type)
	{
		case TRMOD_LEVTYPE_TR1_PC:
		case TRMOD_LEVTYPE_TR1G_PC:
		case TRMOD_LEVTYPE_TR2_PC:
		case TRMOD_LEVTYPE_TR3_PC:
			retval = tr1pc_addcamera(level, error, camera);
			break;
		default:
			error->code = ERROR_UNIMPLEMENTED_FUNCTION;
			break;
	}
	return retval;
}

/*
 * Function that prints a variable dump of the level struct
 * Parameters:
 * * level = Pointer to the level struct
 */
void vardump(struct level *level)
{
	tr1pc_vardump(level);
}
