/*
 * Tomb Raider III Object Exporter/Importer
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
#include <stdlib.h>   /* Standard library */
#include <string.h>   /* Functions relating to strings */
#include "fixedint.h" /* Definition of fixed-size integers */
#include "structs.h"  /* Definition of used structs */
#include "errors.h"   /* Definition of error codes */
#include "util.h"     /* General functions for TRMOD */

/* Internal Function Declarations */
static void tr3pc_navigate(struct level *level, struct error *error);
#ifndef FIXEDINT_NO_CHECK_ON_STARTUP
static int checkFixedInt(void);
#endif

/*
 * Main Function of the Program
 */
int main(int argc, char *argv[])
{
	/* Variable Declarations */
	struct level level; /* Struct for the level */
	struct error error; /* Struct for error handling */
	FILE *dat = NULL;   /* Data file */
	int retval = 0;     /* Return value for called functions */
	INTU32 datsize;     /* Data size */
	INTU32 olddatsize;  /* Old data size */
	int seekval;        /* Return value of fseek() */
	
	/* Wipes the level and error structs before starting */
	level_info_wipe(level);
	error_struct_wipe(error);
	
	/* Checks fixed-size integers */
#ifndef FIXEDINT_NO_CHECK_ON_STARTUP
	retval = checkFixedInt();
	if (retval != 0)
	{
		goto end;
	}
#endif
	
	/* Checks the number of command-line parameters */
	if (argc < 4)
	{
		error.code = ERROR_INVALID_PARAMETERS;
		error.string[0] = argv[0];
		goto end;
	}
	
	/* Sets level type and path */
	level.path = argv[1];
	level.command = argv[0];
	level.type = TRMOD_LEVTYPE_TR3_PC;
	
	/* Opens the level file */
	level.file = fopen(level.path, "r+b");
	if (level.file == NULL)
	{
		error.code = ERROR_FILE_OPEN_FAILED;
		error.string[0] = level.path;
		goto end;
	}
	
	/* Navigates through the level file */
	tr3pc_navigate(&level, &error);
	if (error.code != ERROR_NONE)
	{
		goto end;
	}
	
	/* Determines command */
	if ((argv[2][0] == 'e') || (argv[2][0] == 'E'))
	{
		/* Opens the data file */
		dat = fopen (argv[3], "wb");
		if (dat == NULL)
		{
			error.code = ERROR_FILE_READ_FAILED;
			error.string[0] = argv[3];
			goto end;
		}
		
		/* Resizes the data block in the level */
		olddatsize = (level.p_NumSpriteTextures - level.p_NumAnimations);
		
		/* Overwrites the block */
		copybytes(level.file, dat, level.path, argv[3],
		          level.p_NumAnimations, 0x00000000, olddatsize, &error);
		if (error.code != ERROR_NONE)
		{
			goto end;
		}
	}
	else if ((argv[2][0] == 'i') || (argv[2][0] == 'I'))
	{
		/* Opens the data file */
		dat = fopen (argv[3], "rb");
		if (dat == NULL)
		{
			error.code = ERROR_FILE_READ_FAILED;
			error.string[0] = argv[3];
			goto end;
		}
		
		/* Determines datsize */
		seekval = fseek(dat, 0l, SEEK_END);
		if (seekval != 0)
		{
			error.code = ERROR_FILE_READ_FAILED;
			error.string[0] = argv[3];
			goto end;
		}
		datsize = (INTU32) ftell(dat);
		
		/* Resizes the data block in the level */
		olddatsize = (level.p_NumSpriteTextures - level.p_NumAnimations);
		if (olddatsize < datsize)
		{
			insertbytes(level.file, level.path, level.p_NumAnimations,
			            (datsize - olddatsize), &error);
			if (error.code != ERROR_NONE)
			{
				goto end;
			}
		}
		else if (olddatsize > datsize)
		{
			removebytes(level.file, level.path, level.p_NumAnimations,
			            (olddatsize - datsize), &error);
			if (error.code != ERROR_NONE)
			{
				goto end;
			}
		}
		
		/* Overwrites the block */
		copybytes(dat, level.file, argv[3], level.path, 0x00000000,
		          level.p_NumAnimations, datsize, &error);
		if (error.code != ERROR_NONE)
		{
			goto end;
		}
	}
	else
	{
		error.code = ERROR_INVALID_PARAMETERS;
		error.string[0] = argv[0];
		goto end;
	}
	
end:/* The end of the program */
	/* Closes the level file */
	if (level.file != NULL)
	{
		retval = fclose(level.file);
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

/*
 * Function that navigates through the PC-file reading offsets and values
 * Parameters:
 * * level = Pointer to the level struct
 * * error = Pointer to the error struct
 */
static void tr3pc_navigate(struct level *level, struct error *error)
{
	/* Variable Declarations */
	int seekval;    /* Return value for fseek() */
	size_t readval; /* Return value for fread() */
	INTU16 curroom; /* Current room */
	
	/* NumTexTiles */
	level->p_NumTexTiles = 0x00000704;
	seekval = fseek(level->file, (long int) 0x704, SEEK_SET);
	if (seekval != 0)
	{
		error->code = ERROR_FILE_READ_FAILED;
		error->string[0] = level->path;
		return;
	}
	readval = fread(&(level->v_NumTexTiles),
	                (size_t) 1, (size_t) 4, level->file);
	if (readval != (size_t) 4)
	{
		error->code = ERROR_FILE_READ_FAILED;
		error->string[0] = level->path;
		return;
	}
#ifdef __BIG_ENDIAN__
	level->v_NumTexTiles = reverse32(level->v_NumTexTiles);
#endif
	
	/* NumRooms */
	level->p_NumRooms = ((level->p_NumTexTiles + 0x00000008) +
	                     (level->v_NumTexTiles * 0x00030000));
	seekval = fseek(level->file, (long int) level->p_NumRooms, SEEK_SET);
	if (seekval != 0)
	{
		error->code = ERROR_FILE_READ_FAILED;
		error->string[0] = level->path;
		return;
	}
	readval = fread(&(level->v_NumRooms), (size_t) 1, (size_t) 2, level->file);
	if (readval != (size_t) 2)
	{
		error->code = ERROR_FILE_READ_FAILED;
		error->string[0] = level->path;
		return;
	}
#ifdef __BIG_ENDIAN__
	level->v_NumRooms = reverse16(level->v_NumRooms);
#endif
	
	/* Allocates space for room-related variables */
	if (level->v_NumRooms > 0x0000)
	{
		level->room = calloc((size_t) level->v_NumRooms, sizeof(struct room));
		if (level->room == NULL)
		{
			error->code = ERROR_MEMORY;
			return;
		}
	}
	
	/* Loops through rooms */
	for (curroom = 0x0000; curroom < (level->v_NumRooms); ++curroom)
	{
		/* Header */
		readval = fread(&level->room[curroom].x, (size_t) 1, (size_t) 4,
		                level->file);
		if (readval != (size_t) 4)
		{
			error->code = ERROR_FILE_READ_FAILED;
			error->string[0] = level->path;
			return;
		}
		readval = fread(&level->room[curroom].z, (size_t) 1, (size_t) 4,
		                level->file);
		if (readval != (size_t) 4)
		{
			error->code = ERROR_FILE_READ_FAILED;
			error->string[0] = level->path;
			return;
		}
		readval = fread(&level->room[curroom].yBottom, (size_t) 1, (size_t) 4,
		                level->file);
		if (readval != (size_t) 4)
		{
			error->code = ERROR_FILE_READ_FAILED;
			error->string[0] = level->path;
			return;
		}
		readval = fread(&level->room[curroom].yTop, (size_t) 1, (size_t) 4,
		                level->file);
		if (readval != (size_t) 4)
		{
			error->code = ERROR_FILE_READ_FAILED;
			error->string[0] = level->path;
			return;
		}
#ifdef __BIG_ENDIAN__
		level->room[curroom].x = reverse32(level->room[curroom].x);
		level->room[curroom].z = reverse32(level->room[curroom].z);
		level->room[curroom].yBottom = reverse32(level->room[curroom].yBottom);
		level->room[curroom].yTop = reverse32(level->room[curroom].yTop);
#endif
		
		/* NumVertices */
		seekval = fseek(level->file, 4l, SEEK_CUR);
		if (seekval != 0)
		{
			error->code = ERROR_FILE_READ_FAILED;
			error->string[0] = level->path;
			return;
		}
		level->room[curroom].p_NumVertices = ((INTU32) ftell(level->file));
		readval = fread(&(level->room[curroom].v_NumVertices),
		                (size_t) 1, (size_t) 2, level->file);
		if (readval != (size_t) 2)
		{
			error->code = ERROR_FILE_READ_FAILED;
			error->string[0] = level->path;
			return;
		}
#ifdef __BIG_ENDIAN__
		level->room[curroom].v_NumVertices =
			reverse16(level->room[curroom].v_NumVertices);
#endif
		
		/* NumRectangles */
		level->room[curroom].p_NumRectangles =
			(level->room[curroom].p_NumVertices + 0x00000002 +
			 (level->room[curroom].v_NumVertices * 0x000C));
		seekval = fseek(level->file,
		                (long int) level->room[curroom].p_NumRectangles,
		                SEEK_SET);
		if (seekval != 0)
		{
			error->code = ERROR_FILE_READ_FAILED;
			error->string[0] = level->path;
			return;
		}
		readval = fread(&(level->room[curroom].v_NumRectangles),
		                (size_t) 1, (size_t) 2, level->file);
		if (readval != (size_t) 2)
		{
			error->code = ERROR_FILE_READ_FAILED;
			error->string[0] = level->path;
			return;
		}
#ifdef __BIG_ENDIAN__
		level->room[curroom].v_NumRectangles =
			reverse16(level->room[curroom].v_NumRectangles);
#endif
		
		/* NumTriangles */
		level->room[curroom].p_NumTriangles =
			(level->room[curroom].p_NumRectangles + 0x00000002 +
			 (level->room[curroom].v_NumRectangles * 0x000A));
		seekval = fseek(level->file,
		                (long int) level->room[curroom].p_NumTriangles,
		                SEEK_SET);
		if (seekval != 0)
		{
			error->code = ERROR_FILE_READ_FAILED;
			error->string[0] = level->path;
			return;
		}
		readval = fread(&(level->room[curroom].v_NumTriangles),
		                (size_t) 1, (size_t) 2, level->file);
		if (readval != (size_t) 2)
		{
			error->code = ERROR_FILE_READ_FAILED;
			error->string[0] = level->path;
			return;
		}
#ifdef __BIG_ENDIAN__
		level->room[curroom].v_NumTriangles =
			reverse16(level->room[curroom].v_NumTriangles);
#endif
		
		/* NumSprites */
		level->room[curroom].p_NumSprites =
			(level->room[curroom].p_NumTriangles + 0x00000002 +
			 (level->room[curroom].v_NumTriangles * 0x0008));
		seekval = fseek(level->file,
		                (long int) level->room[curroom].p_NumSprites,
		                SEEK_SET);
		if (seekval != 0)
		{
			error->code = ERROR_FILE_READ_FAILED;
			error->string[0] = level->path;
			return;
		}
		readval = fread(&(level->room[curroom].v_NumSprites),
		                (size_t) 1, (size_t) 2, level->file);
		if (readval != (size_t) 2)
		{
			error->code = ERROR_FILE_READ_FAILED;
			error->string[0] = level->path;
			return;
		}
#ifdef __BIG_ENDIAN__
		level->room[curroom].v_NumSprites =
			reverse16(level->room[curroom].v_NumSprites);
#endif
		
		/* NumDoors */
		level->room[curroom].p_NumDoors =
			(level->room[curroom].p_NumSprites + 0x00000002 +
			 (level->room[curroom].v_NumSprites * 0x0004));
		seekval = fseek(level->file,
		                (long int) level->room[curroom].p_NumDoors,
		                SEEK_SET);
		if (seekval != 0)
		{
			error->code = ERROR_FILE_READ_FAILED;
			error->string[0] = level->path;
			return;
		}
		readval = fread(&(level->room[curroom].v_NumDoors),
		                (size_t) 1, (size_t) 2, level->file);
		if (readval != (size_t) 2)
		{
			error->code = ERROR_FILE_READ_FAILED;
			error->string[0] = level->path;
			return;
		}
#ifdef __BIG_ENDIAN__
		level->room[curroom].v_NumDoors =
			reverse16(level->room[curroom].v_NumDoors);
#endif
		
		/* NumZSectors */
		level->room[curroom].p_NumZSectors =
			(level->room[curroom].p_NumDoors + 0x00000002 +
			 (level->room[curroom].v_NumDoors * 0x0020));
		seekval = fseek(level->file,
		                (long int) level->room[curroom].p_NumZSectors,
		                SEEK_SET);
		if (seekval != 0)
		{
			error->code = ERROR_FILE_READ_FAILED;
			error->string[0] = level->path;
			return;
		}
		readval = fread(&(level->room[curroom].v_NumZSectors),
		                (size_t) 1, (size_t) 2, level->file);
		if (readval != (size_t) 2)
		{
			error->code = ERROR_FILE_READ_FAILED;
			error->string[0] = level->path;
			return;
		}
#ifdef __BIG_ENDIAN__
		level->room[curroom].v_NumZSectors =
			reverse16(level->room[curroom].v_NumZSectors);
#endif
		
		/* NumXSectors */
		level->room[curroom].p_NumXSectors =
			(level->room[curroom].p_NumZSectors + 0x00000002);
		seekval = fseek(level->file,
		                (long int) level->room[curroom].p_NumXSectors,
		                SEEK_SET);
		if (seekval != 0)
		{
			error->code = ERROR_FILE_READ_FAILED;
			error->string[0] = level->path;
			return;
		}
		readval = fread(&(level->room[curroom].v_NumXSectors),
		                (size_t) 1, (size_t) 2, level->file);
		if (readval != (size_t) 2)
		{
			error->code = ERROR_FILE_READ_FAILED;
			error->string[0] = level->path;
			return;
		}
#ifdef __BIG_ENDIAN__
		level->room[curroom].v_NumXSectors =
			reverse16(level->room[curroom].v_NumXSectors);
#endif
		
		/* NumLights */
		level->room[curroom].p_NumLights =
			(level->room[curroom].p_NumXSectors + 0x00000006 +
			 (level->room[curroom].v_NumZSectors *
			  level->room[curroom].v_NumXSectors * 0x0008));
		seekval = fseek(level->file,
		                (long int) level->room[curroom].p_NumLights,
		                SEEK_SET);
		if (seekval != 0)
		{
			error->code = ERROR_FILE_READ_FAILED;
			error->string[0] = level->path;
			return;
		}
		readval = fread(&(level->room[curroom].v_NumLights),
		                (size_t) 1, (size_t) 2, level->file);
		if (readval != (size_t) 2)
		{
			error->code = ERROR_FILE_READ_FAILED;
			error->string[0] = level->path;
			return;
		}
#ifdef __BIG_ENDIAN__
		level->room[curroom].v_NumLights =
			reverse16(level->room[curroom].v_NumLights);
#endif
		
		/* NumStaticMeshes */
		level->room[curroom].p_NumStaticMeshes =
			(level->room[curroom].p_NumLights + 0x00000002 +
			 (level->room[curroom].v_NumLights * 0x0018));
		seekval = fseek(level->file,
		                (long int) level->room[curroom].p_NumStaticMeshes,
		                SEEK_SET);
		if (seekval != 0)
		{
			error->code = ERROR_FILE_READ_FAILED;
			error->string[0] = level->path;
			return;
		}
		readval = fread(&(level->room[curroom].v_NumStaticMeshes),
		                (size_t) 1, (size_t) 2, level->file);
		if (readval != (size_t) 2)
		{
			error->code = ERROR_FILE_READ_FAILED;
			error->string[0] = level->path;
			return;
		}
#ifdef __BIG_ENDIAN__
		level->room[curroom].v_NumStaticMeshes =
			reverse16(level->room[curroom].v_NumStaticMeshes);
#endif
		
		/* Alternateroom */
		level->room[curroom].p_AlternateRoom =
			(level->room[curroom].p_NumStaticMeshes + 0x00000002 +
			 (level->room[curroom].v_NumStaticMeshes * 0x0014));
		seekval = fseek(level->file, (long int)
		                             (level->room[curroom].p_AlternateRoom +
		                              0x00000007), SEEK_SET);
		if (seekval != 0)
		{
			error->code = ERROR_FILE_READ_FAILED;
			error->string[0] = level->path;
			return;
		}
	}
	
	/* NumFloorData */
	level->p_NumFloorData = ((INTU32) ftell(level->file));
	readval = fread(&(level->v_NumFloorData),
	                (size_t) 1, (size_t) 4, level->file);
	if (readval != (size_t) 4)
	{
		error->code = ERROR_FILE_READ_FAILED;
		error->string[0] = level->path;
		return;
	}
#ifdef __BIG_ENDIAN__
	level->v_NumFloorData = reverse32(level->v_NumFloorData);
#endif
	
	/* NumMeshData */
	level->p_NumMeshData = (level->p_NumFloorData + 0x00000004 +
	                        (level->v_NumFloorData * 0x00000002));
	seekval = fseek(level->file, (long int) level->p_NumMeshData, SEEK_SET);
	if (seekval != 0)
	{
		error->code = ERROR_FILE_READ_FAILED;
		error->string[0] = level->path;
		return;
	}
	readval = fread(&(level->v_NumMeshData), (size_t) 1, (size_t) 4,
	                level->file);
	if (readval != (size_t) 4)
	{
		error->code = ERROR_FILE_READ_FAILED;
		error->string[0] = level->path;
		return;
	}
#ifdef __BIG_ENDIAN__
	level->v_NumMeshData = reverse32(level->v_NumMeshData);
#endif
	
	/* NumMeshPointers */
	level->p_NumMeshPointers = (level->p_NumMeshData + 0x00000004 +
	                            (level->v_NumMeshData * 0x00000002));
	seekval = fseek(level->file, (long int) level->p_NumMeshPointers,
	                SEEK_SET);
	if (seekval != 0)
	{
		error->code = ERROR_FILE_READ_FAILED;
		error->string[0] = level->path;
		return;
	}
	readval = fread(&(level->v_NumMeshPointers), (size_t) 1, (size_t) 4,
	                level->file);
	if (readval != (size_t) 4)
	{
		error->code = ERROR_FILE_READ_FAILED;
		error->string[0] = level->path;
		return;
	}
#ifdef __BIG_ENDIAN__
	level->v_NumMeshPointers = reverse32(level->v_NumMeshPointers);
#endif
	
	/* NumAnimations */
	level->p_NumAnimations = (level->p_NumMeshPointers + 0x00000004 +
	                          (level->v_NumMeshPointers * 0x00000004));
	seekval = fseek(level->file, (long int) level->p_NumAnimations,
	                SEEK_SET);
	if (seekval != 0)
	{
		error->code = ERROR_FILE_READ_FAILED;
		error->string[0] = level->path;
		return;
	}
	readval = fread(&(level->v_NumAnimations), (size_t) 1, (size_t) 4,
	                level->file);
	if (readval != (size_t) 4)
	{
		error->code = ERROR_FILE_READ_FAILED;
		error->string[0] = level->path;
		return;
	}
#ifdef __BIG_ENDIAN__
	level->v_NumAnimations = reverse32(level->v_NumAnimations);
#endif
	
	/* NumStateChanges */
	level->p_NumStateChanges = (level->p_NumAnimations + 0x00000004 +
	                            (level->v_NumAnimations * 0x00000020));
	seekval = fseek(level->file, (long int) level->p_NumStateChanges,
	                SEEK_SET);
	if (seekval != 0)
	{
		error->code = ERROR_FILE_READ_FAILED;
		error->string[0] = level->path;
		return;
	}
	readval = fread(&(level->v_NumStateChanges), (size_t) 1, (size_t) 4,
	                level->file);
	if (readval != (size_t) 4)
	{
		error->code = ERROR_FILE_READ_FAILED;
		error->string[0] = level->path;
		return;
	}
#ifdef __BIG_ENDIAN__
	level->v_NumStateChanges = reverse32(level->v_NumStateChanges);
#endif
	
	/* NumAnimDispatches */
	level->p_NumAnimDispatches = (level->p_NumStateChanges + 0x00000004 +
	                              (level->v_NumStateChanges * 0x00000006));
	seekval = fseek(level->file, (long int) level->p_NumAnimDispatches,
	                SEEK_SET);
	if (seekval != 0)
	{
		error->code = ERROR_FILE_READ_FAILED;
		error->string[0] = level->path;
		return;
	}
	readval = fread(&(level->v_NumAnimDispatches), (size_t) 1, (size_t) 4,
	                level->file);
	if (readval != (size_t) 4)
	{
		error->code = ERROR_FILE_READ_FAILED;
		error->string[0] = level->path;
		return;
	}
#ifdef __BIG_ENDIAN__
	level->v_NumAnimDispatches = reverse32(level->v_NumAnimDispatches);
#endif
	
	/* NumAnimCommands */
	level->p_NumAnimCommands = (level->p_NumAnimDispatches + 0x00000004 +
	                              (level->v_NumAnimDispatches * 0x00000008));
	seekval = fseek(level->file, (long int) level->p_NumAnimCommands,
	                SEEK_SET);
	if (seekval != 0)
	{
		error->code = ERROR_FILE_READ_FAILED;
		error->string[0] = level->path;
		return;
	}
	readval = fread(&(level->v_NumAnimCommands), (size_t) 1, (size_t) 4,
	                level->file);
	if (readval != (size_t) 4)
	{
		error->code = ERROR_FILE_READ_FAILED;
		error->string[0] = level->path;
		return;
	}
#ifdef __BIG_ENDIAN__
	level->v_NumAnimCommands = reverse32(level->v_NumAnimCommands);
#endif
	
	/* NumMeshTrees */
	level->p_NumMeshTrees = (level->p_NumAnimCommands + 0x00000004 +
	                         (level->v_NumAnimCommands * 0x00000002));
	seekval = fseek(level->file, (long int) level->p_NumMeshTrees,
	                SEEK_SET);
	if (seekval != 0)
	{
		error->code = ERROR_FILE_READ_FAILED;
		error->string[0] = level->path;
		return;
	}
	readval = fread(&(level->v_NumMeshTrees), (size_t) 1, (size_t) 4,
	                level->file);
	if (readval != (size_t) 4)
	{
		error->code = ERROR_FILE_READ_FAILED;
		error->string[0] = level->path;
		return;
	}
#ifdef __BIG_ENDIAN__
	level->v_NumMeshTrees = reverse32(level->v_NumMeshTrees);
#endif
	
	/* NumFrames */
	level->p_NumFrames = (level->p_NumMeshTrees + 0x00000004 +
	                      (level->v_NumMeshTrees * 0x00000004));
	seekval = fseek(level->file, (long int) level->p_NumFrames, SEEK_SET);
	if (seekval != 0)
	{
		error->code = ERROR_FILE_READ_FAILED;
		error->string[0] = level->path;
		return;
	}
	readval = fread(&(level->v_NumFrames), (size_t) 1, (size_t) 4,
	                level->file);
	if (readval != (size_t) 4)
	{
		error->code = ERROR_FILE_READ_FAILED;
		error->string[0] = level->path;
		return;
	}
#ifdef __BIG_ENDIAN__
	level->v_NumFrames = reverse32(level->v_NumFrames);
#endif
	
	/* NumMoveables */
	level->p_NumMoveables = (level->p_NumFrames + 0x00000004 +
	                         (level->v_NumFrames * 0x00000002));
	seekval = fseek(level->file, (long int) level->p_NumMoveables, SEEK_SET);
	if (seekval != 0)
	{
		error->code = ERROR_FILE_READ_FAILED;
		error->string[0] = level->path;
		return;
	}
	readval = fread(&(level->v_NumMoveables), (size_t) 1, (size_t) 4,
	                level->file);
	if (readval != (size_t) 4)
	{
		error->code = ERROR_FILE_READ_FAILED;
		error->string[0] = level->path;
		return;
	}
#ifdef __BIG_ENDIAN__
	level->v_NumMoveables = reverse32(level->v_NumMoveables);
#endif
	
	/* NumStaticMeshes */
	level->p_NumStaticMeshes = (level->p_NumMoveables + 0x00000004 +
	                            (level->v_NumMoveables * 0x00000012));
	seekval = fseek(level->file, (long int) level->p_NumStaticMeshes, SEEK_SET);
	if (seekval != 0)
	{
		error->code = ERROR_FILE_READ_FAILED;
		error->string[0] = level->path;
		return;
	}
	readval = fread(&(level->v_NumStaticMeshes), (size_t) 1, (size_t) 4,
	                level->file);
	if (readval != (size_t) 4)
	{
		error->code = ERROR_FILE_READ_FAILED;
		error->string[0] = level->path;
		return;
	}
#ifdef __BIG_ENDIAN__
	level->v_NumStaticMeshes = reverse32(level->v_NumStaticMeshes);
#endif
	
	/* NumSpriteTextures */
	level->p_NumSpriteTextures = (level->p_NumStaticMeshes + 0x00000004 +
	                              (level->v_NumStaticMeshes * 0x00000020));
	seekval = fseek(level->file, (long int) level->p_NumSpriteTextures,
	                SEEK_SET);
	if (seekval != 0)
	{
		error->code = ERROR_FILE_READ_FAILED;
		error->string[0] = level->path;
		return;
	}
	readval = fread(&(level->v_NumSpriteTextures), (size_t) 1, (size_t) 4,
	                level->file);
	if (readval != (size_t) 4)
	{
		error->code = ERROR_FILE_READ_FAILED;
		error->string[0] = level->path;
		return;
	}
#ifdef __BIG_ENDIAN__
	level->v_NumSpriteTextures = reverse32(level->v_NumSpriteTextures);
#endif
	
	/* NumSpriteSequences */
	level->p_NumSpriteSequences = (level->p_NumSpriteTextures + 0x00000004 +
	                               (level->v_NumSpriteTextures * 0x00000010));
	seekval = fseek(level->file, (long int) level->p_NumSpriteSequences,
	                SEEK_SET);
	if (seekval != 0)
	{
		error->code = ERROR_FILE_READ_FAILED;
		error->string[0] = level->path;
		return;
	}
	readval = fread(&(level->v_NumSpriteSequences), (size_t) 1, (size_t) 4,
	                level->file);
	if (readval != (size_t) 4)
	{
		error->code = ERROR_FILE_READ_FAILED;
		error->string[0] = level->path;
		return;
	}
#ifdef __BIG_ENDIAN__
	level->v_NumSpriteSequences = reverse32(level->v_NumSpriteSequences);
#endif
	
	/* NumCameras */
	level->p_NumCameras = (level->p_NumSpriteSequences + 0x00000004 +
	                       (level->v_NumSpriteSequences * 0x00000008));
	seekval = fseek(level->file, (long int) level->p_NumCameras, SEEK_SET);
	if (seekval != 0)
	{
		error->code = ERROR_FILE_READ_FAILED;
		error->string[0] = level->path;
		return;
	}
	readval = fread(&(level->v_NumCameras), (size_t) 1, (size_t) 4,
	                level->file);
	if (readval != (size_t) 4)
	{
		error->code = ERROR_FILE_READ_FAILED;
		error->string[0] = level->path;
		return;
	}
#ifdef __BIG_ENDIAN__
	level->v_NumCameras = reverse32(level->v_NumCameras);
#endif
	
	/* NumSoundSources */
	level->p_NumSoundSources = (level->p_NumCameras + 0x00000004 +
	                            (level->v_NumCameras * 0x00000010));
	seekval = fseek(level->file, (long int) level->p_NumSoundSources,
	                SEEK_SET);
	if (seekval != 0)
	{
		error->code = ERROR_FILE_READ_FAILED;
		error->string[0] = level->path;
		return;
	}
	readval = fread(&(level->v_NumSoundSources), (size_t) 1, (size_t) 4,
	                level->file);
	if (readval != (size_t) 4)
	{
		error->code = ERROR_FILE_READ_FAILED;
		error->string[0] = level->path;
		return;
	}
#ifdef __BIG_ENDIAN__
	level->v_NumSoundSources = reverse32(level->v_NumSoundSources);
#endif
	
	/* NumBoxes */
	level->p_NumBoxes = (level->p_NumSoundSources + 0x00000004 +
	                     (level->v_NumSoundSources * 0x00000010));
	seekval = fseek(level->file, (long int) level->p_NumBoxes, SEEK_SET);
	if (seekval != 0)
	{
		error->code = ERROR_FILE_READ_FAILED;
		error->string[0] = level->path;
		return;
	}
	readval = fread(&(level->v_NumBoxes), (size_t) 1, (size_t) 4, level->file);
	if (readval != (size_t) 4)
	{
		error->code = ERROR_FILE_READ_FAILED;
		error->string[0] = level->path;
		return;
	}
#ifdef __BIG_ENDIAN__
	level->v_NumBoxes = reverse32(level->v_NumBoxes);
#endif
	
	/* NumOverlaps */
	level->p_NumOverlaps = (level->p_NumBoxes + 0x00000004 +
	                        (level->v_NumBoxes * 0x00000008));
	seekval = fseek(level->file, (long int) level->p_NumOverlaps, SEEK_SET);
	if (seekval != 0)
	{
		error->code = ERROR_FILE_READ_FAILED;
		error->string[0] = level->path;
		return;
	}
	readval = fread(&(level->v_NumOverlaps), (size_t) 1, (size_t) 4,
	                level->file);
	if (readval != (size_t) 4)
	{
		error->code = ERROR_FILE_READ_FAILED;
		error->string[0] = level->path;
		return;
	}
#ifdef __BIG_ENDIAN__
	level->v_NumOverlaps = reverse32(level->v_NumOverlaps);
#endif
	
	/* NumAnimatedTextures */
	level->p_Zones = (level->p_NumOverlaps + 0x00000004 +
	                  (level->v_NumOverlaps * 0x00000002));
	level->p_NumAnimatedTextures = (level->p_Zones +
	                                (level->v_NumBoxes * 0x00000014));
	seekval = fseek(level->file, (long int) level->p_NumAnimatedTextures,
	                SEEK_SET);
	if (seekval != 0)
	{
		error->code = ERROR_FILE_READ_FAILED;
		error->string[0] = level->path;
		return;
	}
	readval = fread(&(level->v_NumAnimatedTextures), (size_t) 1, (size_t) 4,
	                level->file);
	if (readval != (size_t) 4)
	{
		error->code = ERROR_FILE_READ_FAILED;
		error->string[0] = level->path;
		return;
	}
#ifdef __BIG_ENDIAN__
	level->v_NumAnimatedTextures = reverse32(level->v_NumAnimatedTextures);
#endif
		
	/* NumObjectTextures */
	level->p_NumObjectTextures = (level->p_NumAnimatedTextures + 0x00000004 +
	                              (level->v_NumAnimatedTextures * 0x00000002));
	seekval = fseek(level->file, (long int) level->p_NumObjectTextures,
	                SEEK_SET);
	if (seekval != 0)
	{
		error->code = ERROR_FILE_READ_FAILED;
		error->string[0] = level->path;
		return;
	}
	readval = fread(&(level->v_NumObjectTextures), (size_t) 1, (size_t) 4,
	                level->file);
	if (readval != (size_t) 4)
	{
		error->code = ERROR_FILE_READ_FAILED;
		error->string[0] = level->path;
		return;
	}
#ifdef __BIG_ENDIAN__
	level->v_NumObjectTextures = reverse32(level->v_NumObjectTextures);
#endif
	
	/* NumEntities */
	level->p_NumEntities = (level->p_NumObjectTextures + 0x00000004 +
	                        (level->v_NumObjectTextures * 0x00000014));
	seekval = fseek(level->file, (long int) level->p_NumEntities,
	                SEEK_SET);
	if (seekval != 0)
	{
		error->code = ERROR_FILE_READ_FAILED;
		error->string[0] = level->path;
		return;
	}
	readval = fread(&(level->v_NumEntities), (size_t) 1, (size_t) 4,
	                level->file);
	if (readval != (size_t) 4)
	{
		error->code = ERROR_FILE_READ_FAILED;
		error->string[0] = level->path;
		return;
	}
#ifdef __BIG_ENDIAN__
	level->v_NumEntities = reverse32(level->v_NumEntities);
#endif
	
	/* NumCinematicFrames */
	level->p_NumCinematicFrames = (level->p_NumEntities + 0x00002004 +
	                               (level->v_NumEntities * 0x00000018));
	seekval = fseek(level->file, (long int) level->p_NumCinematicFrames,
	                SEEK_SET);
	if (seekval != 0)
	{
		error->code = ERROR_FILE_READ_FAILED;
		error->string[0] = level->path;
		return;
	}
	readval = fread(&(level->v_NumCinematicFrames), (size_t) 1, (size_t) 2,
	                level->file);
	if (readval != (size_t) 2)
	{
		error->code = ERROR_FILE_READ_FAILED;
		error->string[0] = level->path;
		return;
	}
#ifdef __BIG_ENDIAN__
	level->v_NumCinematicFrames = reverse16(level->v_NumCinematicFrames);
#endif
	
	/* NumDemoData */
	level->p_NumDemoData = (level->p_NumCinematicFrames + 0x00000002 +
	                        (INTU32) (level->v_NumCinematicFrames * 0x0010));
	seekval = fseek(level->file, (long int) level->p_NumDemoData, SEEK_SET);
	if (seekval != 0)
	{
		error->code = ERROR_FILE_READ_FAILED;
		error->string[0] = level->path;
		return;
	}
	readval = fread(&(level->v_NumDemoData), (size_t) 1, (size_t) 2,
	                level->file);
	if (readval != (size_t) 2)
	{
		error->code = ERROR_FILE_READ_FAILED;
		error->string[0] = level->path;
		return;
	}
#ifdef __BIG_ENDIAN__
	level->v_NumDemoData = reverse16(level->v_NumDemoData);
#endif
	
	/* NumSoundDetails */
	level->p_NumSoundDetails = (level->p_NumDemoData + 0x000002E6 +
	                            (INTU32) level->v_NumDemoData);
	seekval = fseek(level->file, (long int) level->p_NumSoundDetails, SEEK_SET);
	if (seekval != 0)
	{
		error->code = ERROR_FILE_READ_FAILED;
		error->string[0] = level->path;
		return;
	}
	readval = fread(&(level->v_NumSoundDetails), (size_t) 1, (size_t) 4,
	                level->file);
	if (readval != (size_t) 4)
	{
		error->code = ERROR_FILE_READ_FAILED;
		error->string[0] = level->path;
		return;
	}
#ifdef __BIG_ENDIAN__
	level->v_NumSoundDetails = reverse32(level->v_NumDemoData);
#endif
	
	/* NumSampleIndices */
	level->p_NumSampleIndices = (level->p_NumSoundDetails + 0x00000004 +
	                             (level->v_NumSoundDetails * 0x00000008));
	seekval = fseek(level->file, (long int) level->p_NumSampleIndices,
	                SEEK_SET);
	if (seekval != 0)
	{
		error->code = ERROR_FILE_READ_FAILED;
		error->string[0] = level->path;
		return;
	}
	readval = fread(&(level->v_NumSampleIndices), (size_t) 1, (size_t) 4,
	                level->file);
	if (readval != (size_t) 4)
	{
		error->code = ERROR_FILE_READ_FAILED;
		error->string[0] = level->path;
		return;
	}
#ifdef __BIG_ENDIAN__
	level->v_NumSampleIndices = reverse32(level->v_NumSampleIndices);
#endif
}

#ifndef FIXEDINT_NO_CHECK_ON_STARTUP
/*
 * Function that checks whether the integers in fixedint.h are correct
 * Return values: 0 = Everything is correct, 1 = Something is incorrect
 */
static int checkFixedInt(void)
{
	/* Variable Declarations */
	INTU8 intu8[2];
	INT8 int8[2];
	INTU16 intu16[2];
	INT16 int16[2];
	INTU32 intu32[2];
	INT32 int32[2];
	unsigned int size;
	
	/* Checks sizes */
	size = (unsigned int) sizeof(INTU8);
	size <<= 3U;
	if (size != 8U)
	{
		printf("INTU8 should be 8 bits, instead it is %u bits\n"
		       "Please adjust fixedint.h and recompile\n", size);
		return 1;
	}
	size = (unsigned int) sizeof(INT8);
	size <<= 3U;
	if (size != 8U)
	{
		printf("INT8 should be 8 bits, instead it is %u bits\n"
		       "Please adjust fixedint.h and recompile\n", size);
		return 1;
	}
	size = (unsigned int) sizeof(INTU16);
	size <<= 3U;
	if (size != 16U)
	{
		printf("INTU16 should be 16 bits, instead it is %u bits\n"
		       "Please adjust fixedint.h and recompile\n", size);
		return 1;
	}
	size = (unsigned int) sizeof(INT16);
	size <<= 3U;
	if (size != 16U)
	{
		printf("INT16 should be 16 bits, instead it is %u bits\n"
		       "Please adjust fixedint.h and recompile\n", size);
		return 1;
	}
	size = (unsigned int) sizeof(INTU32);
	size <<= 3U;
	if (size != 32U)
	{
		printf("INTU32 should be 32 bits, instead it is %u bits\n"
		       "Please adjust fixedint.h and recompile\n", size);
		return 1;
	}
	size = (unsigned int) sizeof(INT32);
	size <<= 3U;
	if (size != 32U)
	{
		printf("INT32 should be 32 bits, instead it is %u bits\n"
		       "Please adjust fixedint.h and recompile\n", size);
		return 1;
	}
	
	/* Checks sign */
	memset(&intu8[0], 0, sizeof(intu8[0]));
	memset(&int8[0], 0, sizeof(int8[0]));
	memset(&intu16[0], 0, sizeof(intu16[0]));
	memset(&int16[0], 0, sizeof(int16[0]));
	memset(&intu32[0], 0, sizeof(intu32[0]));
	memset(&int32[0], 0, sizeof(int32[0]));
	memset(&intu8[1], 255, sizeof(intu8[1]));
	memset(&int8[1], 255, sizeof(int8[1]));
	memset(&intu16[1], 255, sizeof(intu16[1]));
	memset(&int16[1], 255, sizeof(int16[1]));
	memset(&intu32[1], 255, sizeof(intu32[1]));
	memset(&int32[1], 255, sizeof(int32[1]));
	if (intu8[1] < intu8[0])
	{
		printf("INTU8 should be unsigned, instead it is signed\n"
		       "Please adjust fixedint.h and recompile\n");
		return 1;
	}
	if (intu16[1] < intu16[0])
	{
		printf("INTU16 should be unsigned, instead it is signed\n"
		       "Please adjust fixedint.h and recompile\n");
		return 1;
	}
	if (intu32[1] < intu32[0])
	{
		printf("INTU32 should be unsigned, instead it is signed\n"
		       "Please adjust fixedint.h and recompile\n");
		return 1;
	}
	if (int8[1] > int8[0])
	{
		printf("INT8 should be signed, instead it is unsigned\n"
		       "Please adjust fixedint.h and recompile\n");
		return 1;
	}
	if (int16[1] > int16[0])
	{
		printf("INT16 should be signed, instead it is unsigned\n"
		       "Please adjust fixedint.h and recompile\n");
		return 1;
	}
	if (int32[1] > int32[0])
	{
		printf("INT32 should be signed, instead it is unsigned\n"
		       "Please adjust fixedint.h and recompile\n");
		return 1;
	}
	
	/* All tests have passed */
	return 0;
}
#endif
