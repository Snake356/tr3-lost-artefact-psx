/*
 * Tomb Raider III Mesh Editor
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
#include <stdio.h>    /* Standard I/O */
#include <stdlib.h>   /* Standard Library */
#include <string.h>   /* Functions relating to strings */
#include "fixedint.h" /* Definition of fixed-size integers */
#include "structs.h"  /* Definition of structs used by TR2PCPSX */
#include "util.h"     /* General-purpose code used by TR2PCPSX */
#include "osdep.h"    /* Library of OS-dependant functions */

/* Static function declarations */
static int dataToText(char *pcpath);
static int textToData(char *pcpath, char *meshpath);
static int tr2pc_navigate(FILE *pc, struct level_info *pc_info);
static void printError(int errorNumber, char *argv[], int pcparam);
#ifdef FIXEDINT_CHECK_ON_STARTUP
	static int fixedint_check(void);
#endif

/*
 * Main function of the program.
 * Parameters:
 * * argc = Number of command-line parameters
 * * argc = Command-line parameters
 * Return values:
 * * 0 = Everything went well
 * * 1 = Insufficient number of command-line parameters
 * * 2 = PC-file is not a PHD or TUB file
 * * 3 = PC-file could not be opened
 * * 4 = PC-file could not be read from
 * * 5 = PC-file could not be closed
 * * 6 = PSX-file could not be opened
 * * 7 = PSX-file could not be read from
 * * 8 = PSX-file could not be written to
 * * 9 = PSX-file could not be closed
 * * 10 = Memory could not be allocated
 * * 11 = Data to be removed doesn't exist (removebytes)
 * * 13 = Array is too long to be sorted
 * * 14 = Array does not exist (NULL)
 * * 16 = Texture corners are invalid
 * * 17 = Level has too many textures to convert
 * * 19 = Sample bitrate is not supported
 * * 20 = Input WAVE is not in PCM
 * * 21 = Input WAVE has more than two channels
 * * 22 = Input WAVE is not 8, 16, or 32-bit
 * * 23 = Fixed-size integers are wrong
 * * 24 = Text file could not be read from
 * * 25 = PC-file could not be written to
 */
int main(int argc, char *argv[])
{
	/* Variable Declarations */
	int errorNumber = 0;             /* Return value for the program */
	
	/* Checks the sizes of fixed-size integers */
	#ifdef FIXEDINT_CHECK_ON_STARTUP
	errorNumber = fixedint_check();
	if (errorNumber != 0)
	{
		return 23;
	}
	#endif
	
	if (argc < 2)
	{
		errorNumber = 1;
	}
	else if (argc == 2)
	{
		errorNumber = dataToText(argv[1]);
	}
	else if (argc == 3)
	{
		errorNumber = textToData(argv[1], argv[2]);
	}
	
	/* Prints error message if needed */
	if (errorNumber != 0)
	{
		printError(errorNumber, argv, 1);
	}
	
	/* Ends the program */
	return errorNumber;
}

/*
 * Function that prints the mesh data to stdout.
 * Parameters:
 * * pcpath = Path to the TR2 file
 * Return values:
 * * 0 = Everything went well
 */
static int textToData(char *pcpath, char *meshpath)
{
	/* Struct Declarations */	
	struct vertex
	{
		INTU16 number;
		INT16 x;
		INT16 y;
		INT16 z;
	};
	struct rectangle
	{
		INTU16 vertex1;
		INTU16 vertex2;
		INTU16 vertex3;
		INTU16 vertex4;
		INT16 texture;
	};
	struct triangle
	{
		INTU16 vertex1;
		INTU16 vertex2;
		INTU16 vertex3;
		INT16 texture;
	};
	struct mesh
	{
		INTU32 pointerval;
		INTU32 outPointerval;
		struct vertex centre;
		INT32 radius;
		INTU16 numVertices;
		struct vertex *vertices;
		INTU16 numNormals;
		struct vertex *normals;
		INTU16 numLights;
		INT16 *lights;
		INTU16 numTexturedRectangles;
		struct rectangle *texturedRectangles;
		INTU16 numTexturedTriangles;
		struct triangle *texturedTriangles;
		INTU16 numColouredRectangles;
		struct rectangle *colouredRectangles;
		INTU16 numColouredTriangles;
		struct triangle *colouredTriangles;
	};
	
	/* Variable Declatations */
	FILE *pc = NULL; /* Pointer to PC-file */
	FILE *meshfile = NULL; /* Text file containing the mesh data */
	char *meshtext = NULL; /* Text version of mesh data */
	char *freeptr1 = NULL;
	char *freeptr2 = NULL;
	char *freeptr3 = NULL;
	INTU32 *meshPointers = NULL; /* In-memory copy of mesh pointers */
	long int meshtextsize;	
	INTU32 numMeshes = 0x00000000;
	INTU32 numOutMeshes = 0x00000000;
	struct mesh *meshes = NULL;
	struct level_info pc_info;  /* Offsets and values in PC-file */
	int freeint1;       /* Integer for general use */
	size_t freesizet1;  /* size_t for general use */
	INTU32 freeuint321,
	       freeuint322,
	       freeuint323;
	INTU16 freeuint161,
	       freeuint162,
	       freeuint163,
	       freeuint164;
	INT16 freeint161;
	int errorNumber = 0; /* Return value for this program */
	
	/* Sets level_info structs to NULL before we start */
	level_info_wipe(pc_info);
	
	/* Opens PC-file */
	pc = fopen(pcpath, "r+b");
	if (pc == NULL)
	{
		errorNumber = 3;
		goto end;
	}
	
	/* Determines number of rooms in the level */
	freeint1 = fseek(pc, 1796, SEEK_SET);
	if (freeint1 != 0)
	{
		errorNumber = 4;
		goto end;
	}
	freesizet1 = fread(&pc_info.v_NumTexTiles, 1, 4, pc);
	if (freesizet1 != 4)
	{
		errorNumber = 4;
		goto end;
	}
	#ifdef __BIG_ENDIAN__
	pc_info.v_NumTexTiles = reverse32(pc_info.v_NumTexTiles);
	#endif
	pc_info.p_NumRooms = (1804 + (pc_info.v_NumTexTiles * 196608));
	freeint1 = fseek(pc, (long int)pc_info.p_NumRooms, SEEK_SET);
	if (freeint1 != 0)
	{
		errorNumber = 4;
		goto end;
	}
	freesizet1 = fread(&pc_info.v_NumRooms, 1, 2, pc);
	if (freesizet1 != 2)
	{
		errorNumber = 4;
		goto end;
	}
	#ifdef __BIG_ENDIAN__
	pc_info.v_NumRooms = reverse16(pc_info.v_NumRooms);
	#endif
	
	/* Allocates space for room-related variables */
	pc_info.room = calloc((size_t)pc_info.v_NumRooms, sizeof(struct room_info));
	if (pc_info.room == NULL)
	{
		errorNumber = 10;
		goto end;
	}
	
	/* Navigates through PC-file */
	errorNumber = tr2pc_navigate(pc, &pc_info);
	if (errorNumber != 0)
	{
		goto end;
	}
	
	/* Reads the text version of the mesh data into memory */
	meshfile = fopen(meshpath, "rb");
	if (meshfile == NULL)
	{
		errorNumber = 24;
		goto end;
	}
	freeint1 = fseek(meshfile, 0, SEEK_END);
	if (freeint1 != 0)
	{
		errorNumber = 24;
		goto end;
	}
	meshtextsize = ftell(meshfile);
	meshtext = malloc(meshtextsize + 1);
	if (meshtext == NULL)
	{
		errorNumber = 10;
		goto end;
	}
	freeint1 = fseek(meshfile, 0, SEEK_SET);
	if (freeint1 != 0)
	{
		errorNumber = 24;
		goto end;
	}
	freesizet1 = fread(meshtext, 1, meshtextsize, meshfile);
	if (freesizet1 != meshtextsize)
	{
		errorNumber = 24;
		goto end;
	}
	meshtext[meshtextsize] = '\0';
	freeint1 = fclose(meshfile);
	if (freeint1 != 0)
	{
		errorNumber = 24;
		goto end;
	}
	meshfile = NULL;
	
	/* Counts the number of meshes */
	freeptr1 = strstr(meshtext, "mesh(");
	while (freeptr1 != NULL)
	{
		memset(freeptr1, (int) '\0', 1);
		++numMeshes;
		++freeptr1;
		freeptr1 = strstr(freeptr1, "mesh(");
	}
	
	/* Allocates the meshes */
	meshes = calloc(numMeshes, sizeof(struct mesh));
	if (meshes == NULL)
	{
		errorNumber = 10;
		goto end;
	}
	
	/* Parses through the meshes in text form */
	freeptr1 = meshtext + (strlen(meshtext) + 1);
	for (freeuint321 = 0x00000000; freeuint321 < numMeshes; ++freeuint321)
	{
		/* Determines the pointer value */
		freeptr2 = freeptr1 + (strlen(freeptr1) + 1);
		while (*freeptr1 != '[') ++freeptr1;
		++freeptr1;
		freeuint322 = strtoint(freeptr1);
		meshes[freeuint321].pointerval = freeuint322;
		freeint1 = 0;
		for (freeuint323 = 0x00000000; freeuint323 < numOutMeshes; ++freeuint323)
		{
			if (meshes[freeuint323].pointerval == freeuint322)
			{
				freeint1 = 1;
			}
		}
		if (freeint1 == 1)
		{
			freeptr1 = freeptr2;
			continue;
		}
		
		while (*freeptr1 != '{') ++freeptr1;
		
		/* Determines centre */
		freeptr3 = strstr(freeptr1, "centre(");
		if (freeptr3 != NULL)
		{
			freeptr3 += 7;
			meshes[freeuint321].centre.x = (INT16) strtoint(freeptr3);
			while (*freeptr3 != ',') ++freeptr3;
			++freeptr3;
			meshes[freeuint321].centre.y = (INT16) strtoint(freeptr3);
			while (*freeptr3 != ',') ++freeptr3;
			++freeptr3;
			meshes[freeuint321].centre.z = (INT16) strtoint(freeptr3);
		}
		
		/* Determines radius */
		freeptr3 = strstr(freeptr1, "radius(");
		if (freeptr3 != NULL)
		{
			freeptr3 += 7;
			meshes[freeuint321].radius = strtoint(freeptr3);
		}
		
		/* counts vertices */
		freeuint161 = 0x0000;
		freeptr3 = strstr(freeptr1, "vertex(");
		while (freeptr3 != NULL)
		{
			++freeuint161;
			++freeptr3;
			freeptr3 = strstr(freeptr3, "vertex(");
		}
		
		if (freeuint161 > 0x0000)
		{
			/* allocates vertices */
			meshes[freeuint321].numVertices = freeuint161;
			meshes[freeuint321].vertices = calloc(freeuint161, sizeof(struct vertex));
			if (meshes[freeuint321].vertices == NULL)
			{
				errorNumber = 10;
				goto end;
			}
			
			/* adds the vertices */
			freeptr3 = strstr(freeptr1, "vertex(");
			for (freeuint162 = 0x0000; freeuint162 < freeuint161; ++freeuint162)
			{
				freeptr3 += 7;
				meshes[freeuint321].vertices[freeuint162].number = (INTU16) strtoint(freeptr3);
				while (*freeptr3 != '{') ++freeptr3;
				++freeptr3;
				meshes[freeuint321].vertices[freeuint162].x = (INT16) strtoint(freeptr3);
				while (*freeptr3 != ',') ++freeptr3;
				++freeptr3;
				meshes[freeuint321].vertices[freeuint162].y = (INT16) strtoint(freeptr3);
				while (*freeptr3 != ',') ++freeptr3;
				++freeptr3;
				meshes[freeuint321].vertices[freeuint162].z = (INT16) strtoint(freeptr3);
				freeptr3 = strstr(freeptr3, "vertex(");
			}
		}
		
		/* Counts normals */
		freeuint161 = 0x0000;
		freeptr3 = strstr(freeptr1, "normal(");
		while (freeptr3 != NULL)
		{
			++freeuint161;
			++freeptr3;
			freeptr3 = strstr(freeptr3, "normal(");
		}
		
		if (freeuint161 > 0x0000)
		{
			/* Allocates normals */
			meshes[freeuint321].numNormals = freeuint161;
			meshes[freeuint321].normals = calloc(freeuint161, sizeof(struct vertex));
			if (meshes[freeuint321].normals == NULL)
			{
				errorNumber = 10;
				goto end;
			}
			
			/* Adds the normals */
			freeptr3 = strstr(freeptr1, "normal(");
			for (freeuint162 = 0x0000; freeuint162 < freeuint161; ++freeuint162)
			{
				freeptr3 += 7;
				meshes[freeuint321].normals[freeuint162].number = (INTU16) strtoint(freeptr3);
				while (*freeptr3 != '{') ++freeptr3;
				++freeptr3;
				meshes[freeuint321].normals[freeuint162].x = (INT16) strtoint(freeptr3);
				while (*freeptr3 != ',') ++freeptr3;
				++freeptr3;
				meshes[freeuint321].normals[freeuint162].y = (INT16) strtoint(freeptr3);
				while (*freeptr3 != ',') ++freeptr3;
				++freeptr3;
				meshes[freeuint321].normals[freeuint162].z = (INT16) strtoint(freeptr3);
				freeptr3 = strstr(freeptr3, "normal(");
			}
		}
		
		/* Counts lights */
		freeuint161 = 0x0000;
		freeptr3 = strstr(freeptr1, "light(");
		while (freeptr3 != NULL)
		{
			++freeuint161;
			++freeptr3;
			freeptr3 = strstr(freeptr3, "light(");
		}
		
		if (freeuint161 > 0x0000)
		{
			/* Allocates lights */
			meshes[freeuint321].numLights = freeuint161;
			meshes[freeuint321].lights = calloc(freeuint161, 2);
			if (meshes[freeuint321].lights == NULL)
			{
				errorNumber = 10;
				goto end;
			}
			
			/* Adds the lights */
			freeptr3 = strstr(freeptr1, "light(");
			for (freeuint162 = 0x0000; freeuint162 < freeuint161; ++freeuint162)
			{
				freeptr3 += 6;
				while (*freeptr3 != '{') ++freeptr3;
				++freeptr3;
				meshes[freeuint321].lights[freeuint162] = (INT16) strtoint(freeptr3);
				freeptr3 = strstr(freeptr3, "light(");
			}
		}
		
		/* Counts textured rectangles */
		freeuint161 = 0x0000;
		freeptr3 = strstr(freeptr1, "texturedrectangle(");
		while (freeptr3 != NULL)
		{
			++freeuint161;
			++freeptr3;
			freeptr3 = strstr(freeptr3, "texturedrectangle(");
		}
		
		if (freeuint161 > 0x0000)
		{
			/* allocates textured rectangles */
			meshes[freeuint321].numTexturedRectangles = freeuint161;
			meshes[freeuint321].texturedRectangles = calloc(freeuint161, sizeof(struct rectangle));
			if (meshes[freeuint321].texturedRectangles == NULL)
			{
				errorNumber = 10;
				goto end;
			}
			
			/* adds the textured rectangles */
			freeptr3 = strstr(freeptr1, "texturedrectangle(");
			for (freeuint162 = 0x0000; freeuint162 < freeuint161; ++freeuint162)
			{
				while (*freeptr3 != '{') ++freeptr3;
				++freeptr3;
				freeuint163 = (INTU16) strtoint(freeptr3);
				for (freeuint164 = 0x0000; freeuint164 < meshes[freeuint321].numVertices; ++freeuint164)
				{
					if (meshes[freeuint321].vertices[freeuint164].number == freeuint163)
					{
						meshes[freeuint321].texturedRectangles[freeuint162].vertex1 = freeuint164;
						break;
					}
				}
				while (*freeptr3 != ',') ++freeptr3;
				++freeptr3;
				freeuint163 = (INTU16) strtoint(freeptr3);
				for (freeuint164 = 0x0000; freeuint164 < meshes[freeuint321].numVertices; ++freeuint164)
				{
					if (meshes[freeuint321].vertices[freeuint164].number == freeuint163)
					{
						meshes[freeuint321].texturedRectangles[freeuint162].vertex2 = freeuint164;
						break;
					}
				}
				while (*freeptr3 != ',') ++freeptr3;
				++freeptr3;
				freeuint163 = (INTU16) strtoint(freeptr3);
				for (freeuint164 = 0x0000; freeuint164 < meshes[freeuint321].numVertices; ++freeuint164)
				{
					if (meshes[freeuint321].vertices[freeuint164].number == freeuint163)
					{
						meshes[freeuint321].texturedRectangles[freeuint162].vertex3 = freeuint164;
						break;
					}
				}
				while (*freeptr3 != ',') ++freeptr3;
				++freeptr3;
				freeuint163 = (INTU16) strtoint(freeptr3);
				for (freeuint164 = 0x0000; freeuint164 < meshes[freeuint321].numVertices; ++freeuint164)
				{
					if (meshes[freeuint321].vertices[freeuint164].number == freeuint163)
					{
						meshes[freeuint321].texturedRectangles[freeuint162].vertex4 = freeuint164;
						break;
					}
				}
				while (*freeptr3 != ',') ++freeptr3;
				++freeptr3;
				meshes[freeuint321].texturedRectangles[freeuint162].texture = (INT16) strtoint(freeptr3);
				freeptr3 = strstr(freeptr3, "texturedrectangle(");
			}
		}
		
		/* Counts coloured rectangles */
		freeuint161 = 0x0000;
		freeptr3 = strstr(freeptr1, "colouredrectangle(");
		while (freeptr3 != NULL)
		{
			++freeuint161;
			++freeptr3;
			freeptr3 = strstr(freeptr3, "colouredrectangle(");
		}
		
		if (freeuint161 > 0x0000)
		{
			/* allocates coloured rectangles */
			meshes[freeuint321].numColouredRectangles = freeuint161;
			meshes[freeuint321].colouredRectangles = calloc(freeuint161, sizeof(struct rectangle));
			if (meshes[freeuint321].colouredRectangles == NULL)
			{
				errorNumber = 10;
				goto end;
			}
			
			/* adds the coloured rectangles */
			freeptr3 = strstr(freeptr1, "colouredrectangle(");
			for (freeuint162 = 0x0000; freeuint162 < freeuint161; ++freeuint162)
			{
				while (*freeptr3 != '{') ++freeptr3;
				++freeptr3;
				freeuint163 = (INTU16) strtoint(freeptr3);
				for (freeuint164 = 0x0000; freeuint164 < meshes[freeuint321].numVertices; ++freeuint164)
				{
					if (meshes[freeuint321].vertices[freeuint164].number == freeuint163)
					{
						meshes[freeuint321].colouredRectangles[freeuint162].vertex1 = freeuint164;
						break;
					}
				}
				while (*freeptr3 != ',') ++freeptr3;
				++freeptr3;
				freeuint163 = (INTU16) strtoint(freeptr3);
				for (freeuint164 = 0x0000; freeuint164 < meshes[freeuint321].numVertices; ++freeuint164)
				{
					if (meshes[freeuint321].vertices[freeuint164].number == freeuint163)
					{
						meshes[freeuint321].colouredRectangles[freeuint162].vertex2 = freeuint164;
						break;
					}
				}
				while (*freeptr3 != ',') ++freeptr3;
				++freeptr3;
				freeuint163 = (INTU16) strtoint(freeptr3);
				for (freeuint164 = 0x0000; freeuint164 < meshes[freeuint321].numVertices; ++freeuint164)
				{
					if (meshes[freeuint321].vertices[freeuint164].number == freeuint163)
					{
						meshes[freeuint321].colouredRectangles[freeuint162].vertex3 = freeuint164;
						break;
					}
				}
				while (*freeptr3 != ',') ++freeptr3;
				++freeptr3;
				freeuint163 = (INTU16) strtoint(freeptr3);
				for (freeuint164 = 0x0000; freeuint164 < meshes[freeuint321].numVertices; ++freeuint164)
				{
					if (meshes[freeuint321].vertices[freeuint164].number == freeuint163)
					{
						meshes[freeuint321].colouredRectangles[freeuint162].vertex4 = freeuint164;
						break;
					}
				}
				while (*freeptr3 != ',') ++freeptr3;
				++freeptr3;
				meshes[freeuint321].colouredRectangles[freeuint162].texture = (INT16) strtoint(freeptr3);
				freeptr3 = strstr(freeptr3, "colouredrectangle(");
			}
		}
		
		/* Counts textured triangles */
		freeuint161 = 0x0000;
		freeptr3 = strstr(freeptr1, "texturedtriangle(");
		while (freeptr3 != NULL)
		{
			++freeuint161;
			++freeptr3;
			freeptr3 = strstr(freeptr3, "texturedtriangle(");
		}
		
		if (freeuint161 > 0x0000)
		{
			/* allocates textured triangles */
			meshes[freeuint321].numTexturedTriangles = freeuint161;
			meshes[freeuint321].texturedTriangles = calloc(freeuint161, sizeof(struct triangle));
			if (meshes[freeuint321].texturedTriangles == NULL)
			{
				errorNumber = 10;
				goto end;
			}
			
			/* adds the textured triangles */
			freeptr3 = strstr(freeptr1, "texturedtriangle(");
			for (freeuint162 = 0x0000; freeuint162 < freeuint161; ++freeuint162)
			{
				while (*freeptr3 != '{') ++freeptr3;
				++freeptr3;
				freeuint163 = (INTU16) strtoint(freeptr3);
				for (freeuint164 = 0x0000; freeuint164 < meshes[freeuint321].numVertices; ++freeuint164)
				{
					if (meshes[freeuint321].vertices[freeuint164].number == freeuint163)
					{
						meshes[freeuint321].texturedTriangles[freeuint162].vertex1 = freeuint164;
						break;
					}
				}
				while (*freeptr3 != ',') ++freeptr3;
				++freeptr3;
				freeuint163 = (INTU16) strtoint(freeptr3);
				for (freeuint164 = 0x0000; freeuint164 < meshes[freeuint321].numVertices; ++freeuint164)
				{
					if (meshes[freeuint321].vertices[freeuint164].number == freeuint163)
					{
						meshes[freeuint321].texturedTriangles[freeuint162].vertex2 = freeuint164;
						break;
					}
				}
				while (*freeptr3 != ',') ++freeptr3;
				++freeptr3;
				freeuint163 = (INTU16) strtoint(freeptr3);
				for (freeuint164 = 0x0000; freeuint164 < meshes[freeuint321].numVertices; ++freeuint164)
				{
					if (meshes[freeuint321].vertices[freeuint164].number == freeuint163)
					{
						meshes[freeuint321].texturedTriangles[freeuint162].vertex3 = freeuint164;
						break;
					}
				}
				while (*freeptr3 != ',') ++freeptr3;
				++freeptr3;
				meshes[freeuint321].texturedTriangles[freeuint162].texture = (INT16) strtoint(freeptr3);
				freeptr3 = strstr(freeptr3, "texturedtriangle(");
			}
		}
		
		/* Counts coloured triangles */
		freeuint161 = 0x0000;
		freeptr3 = strstr(freeptr1, "colouredtriangle(");
		while (freeptr3 != NULL)
		{
			++freeuint161;
			++freeptr3;
			freeptr3 = strstr(freeptr3, "colouredtriangle(");
		}
		
		if (freeuint161 > 0x0000)
		{
			/* allocates coloured triangles */
			meshes[freeuint321].numColouredTriangles = freeuint161;
			meshes[freeuint321].colouredTriangles = calloc(freeuint161, sizeof(struct triangle));
			if (meshes[freeuint321].colouredTriangles == NULL)
			{
				errorNumber = 10;
				goto end;
			}
			
			/* adds the coloured triangles */
			freeptr3 = strstr(freeptr1, "colouredtriangle(");
			for (freeuint162 = 0x0000; freeuint162 < freeuint161; ++freeuint162)
			{
				while (*freeptr3 != '{') ++freeptr3;
				++freeptr3;
				freeuint163 = (INTU16) strtoint(freeptr3);
				for (freeuint164 = 0x0000; freeuint164 < meshes[freeuint321].numVertices; ++freeuint164)
				{
					if (meshes[freeuint321].vertices[freeuint164].number == freeuint163)
					{
						meshes[freeuint321].colouredTriangles[freeuint162].vertex1 = freeuint164;
						break;
					}
				}
				while (*freeptr3 != ',') ++freeptr3;
				++freeptr3;
				freeuint163 = (INTU16) strtoint(freeptr3);
				for (freeuint164 = 0x0000; freeuint164 < meshes[freeuint321].numVertices; ++freeuint164)
				{
					if (meshes[freeuint321].vertices[freeuint164].number == freeuint163)
					{
						meshes[freeuint321].colouredTriangles[freeuint162].vertex2 = freeuint164;
						break;
					}
				}
				while (*freeptr3 != ',') ++freeptr3;
				++freeptr3;
				freeuint163 = (INTU16) strtoint(freeptr3);
				for (freeuint164 = 0x0000; freeuint164 < meshes[freeuint321].numVertices; ++freeuint164)
				{
					if (meshes[freeuint321].vertices[freeuint164].number == freeuint163)
					{
						meshes[freeuint321].colouredTriangles[freeuint162].vertex3 = freeuint164;
						break;
					}
				}
				while (*freeptr3 != ',') ++freeptr3;
				++freeptr3;
				meshes[freeuint321].colouredTriangles[freeuint162].texture = (INT16) strtoint(freeptr3);
				freeptr3 = strstr(freeptr3, "colouredtriangle(");
			}
		}
		
		/* Moves on to the next mesh */
		freeptr1 = freeptr2;
		++numOutMeshes;
	}
	
	/* Allocates Mesh Pointers */
	meshPointers = calloc(numMeshes, 4);
	if (meshPointers == NULL)
	{
		errorNumber = 10;
		goto end;
	}
	
	/* Packs the mesh data */
	freeuint323 = 0x00000000;
	for (freeuint321 = 0x00000000; freeuint321 < numMeshes; ++freeuint321)
	{
		/* Doesn't convert a duplicate mesh twice */
		freeint1 = 0;
		for (freeuint322 = 0x00000000; freeuint322 < freeuint321; ++freeuint322)
		{
			if (meshes[freeuint321].pointerval == meshes[freeuint322].pointerval)
			{
				meshPointers[freeuint321] = meshPointers[freeuint322];
				freeint1 = 1;
			}
		}
		if (freeint1 == 1)
		{
			continue;
		}
		
		/* Sets the mesh pointer */
		meshPointers[freeuint321] = freeuint323;
		meshes[freeuint321].outPointerval = freeuint323;
		
		/* Packs the mesh */
		memcpy(&meshtext[freeuint323], &meshes[freeuint321].centre.x, 2);
		freeuint323 += 2;
		memcpy(&meshtext[freeuint323], &meshes[freeuint321].centre.y, 2);
		freeuint323 += 2;
		memcpy(&meshtext[freeuint323], &meshes[freeuint321].centre.z, 2);
		freeuint323 += 2;
		memcpy(&meshtext[freeuint323], &meshes[freeuint321].radius, 4);
		freeuint323 += 4;
		memcpy(&meshtext[freeuint323], &meshes[freeuint321].numVertices, 2);
		freeuint323 += 2;
		for (freeuint161 = 0x0000; freeuint161 < meshes[freeuint321].numVertices; ++freeuint161)
		{
			memcpy(&meshtext[freeuint323], &meshes[freeuint321].vertices[freeuint161].x, 2);
			freeuint323 += 2;
			memcpy(&meshtext[freeuint323], &meshes[freeuint321].vertices[freeuint161].y, 2);
			freeuint323 += 2;
			memcpy(&meshtext[freeuint323], &meshes[freeuint321].vertices[freeuint161].z, 2);
			freeuint323 += 2;
		}
		if (meshes[freeuint321].numLights > 0x0000)
		{
			freeint161 = (0x0000 - ((INT16) meshes[freeuint321].numLights));
			memcpy(&meshtext[freeuint323], &freeint161, 2);
			freeuint323 += 2;
			for (freeuint161 = 0x0000; freeuint161 < meshes[freeuint321].numLights; ++freeuint161)
			{
				memcpy(&meshtext[freeuint323], &meshes[freeuint321].lights[freeuint161], 2);
				freeuint323 += 2;
			}
		}
		else
		{
			freeint161 = meshes[freeuint321].numNormals;
			memcpy(&meshtext[freeuint323], &freeint161, 2);
			freeuint323 += 2;
			for (freeuint161 = 0x0000; freeuint161 < meshes[freeuint321].numNormals; ++freeuint161)
			{
				memcpy(&meshtext[freeuint323], &meshes[freeuint321].normals[freeuint161].x, 2);
				freeuint323 += 2;
				memcpy(&meshtext[freeuint323], &meshes[freeuint321].normals[freeuint161].y, 2);
				freeuint323 += 2;
				memcpy(&meshtext[freeuint323], &meshes[freeuint321].normals[freeuint161].z, 2);
				freeuint323 += 2;
			}
		}
		memcpy(&meshtext[freeuint323], &meshes[freeuint321].numTexturedRectangles, 2);
		freeuint323 += 2;
		for (freeuint161 = 0x0000; freeuint161 < meshes[freeuint321].numTexturedRectangles; ++freeuint161)
		{
			memcpy(&meshtext[freeuint323], &meshes[freeuint321].texturedRectangles[freeuint161].vertex1, 2);
			freeuint323 += 2;
			memcpy(&meshtext[freeuint323], &meshes[freeuint321].texturedRectangles[freeuint161].vertex2, 2);
			freeuint323 += 2;
			memcpy(&meshtext[freeuint323], &meshes[freeuint321].texturedRectangles[freeuint161].vertex3, 2);
			freeuint323 += 2;
			memcpy(&meshtext[freeuint323], &meshes[freeuint321].texturedRectangles[freeuint161].vertex4, 2);
			freeuint323 += 2;
			memcpy(&meshtext[freeuint323], &meshes[freeuint321].texturedRectangles[freeuint161].texture, 2);
			freeuint323 += 2;
		}
		memcpy(&meshtext[freeuint323], &meshes[freeuint321].numTexturedTriangles, 2);
		freeuint323 += 2;
		for (freeuint161 = 0x0000; freeuint161 < meshes[freeuint321].numTexturedTriangles; ++freeuint161)
		{
			memcpy(&meshtext[freeuint323], &meshes[freeuint321].texturedTriangles[freeuint161].vertex1, 2);
			freeuint323 += 2;
			memcpy(&meshtext[freeuint323], &meshes[freeuint321].texturedTriangles[freeuint161].vertex2, 2);
			freeuint323 += 2;
			memcpy(&meshtext[freeuint323], &meshes[freeuint321].texturedTriangles[freeuint161].vertex3, 2);
			freeuint323 += 2;
			memcpy(&meshtext[freeuint323], &meshes[freeuint321].texturedTriangles[freeuint161].texture, 2);
			freeuint323 += 2;
		}
		memcpy(&meshtext[freeuint323], &meshes[freeuint321].numColouredRectangles, 2);
		freeuint323 += 2;
		for (freeuint161 = 0x0000; freeuint161 < meshes[freeuint321].numColouredRectangles; ++freeuint161)
		{
			memcpy(&meshtext[freeuint323], &meshes[freeuint321].colouredRectangles[freeuint161].vertex1, 2);
			freeuint323 += 2;
			memcpy(&meshtext[freeuint323], &meshes[freeuint321].colouredRectangles[freeuint161].vertex2, 2);
			freeuint323 += 2;
			memcpy(&meshtext[freeuint323], &meshes[freeuint321].colouredRectangles[freeuint161].vertex3, 2);
			freeuint323 += 2;
			memcpy(&meshtext[freeuint323], &meshes[freeuint321].colouredRectangles[freeuint161].vertex4, 2);
			freeuint323 += 2;
			memcpy(&meshtext[freeuint323], &meshes[freeuint321].colouredRectangles[freeuint161].texture, 2);
			freeuint323 += 2;
		}
		memcpy(&meshtext[freeuint323], &meshes[freeuint321].numColouredTriangles, 2);
		freeuint323 += 2;
		for (freeuint161 = 0x0000; freeuint161 < meshes[freeuint321].numColouredTriangles; ++freeuint161)
		{
			memcpy(&meshtext[freeuint323], &meshes[freeuint321].colouredTriangles[freeuint161].vertex1, 2);
			freeuint323 += 2;
			memcpy(&meshtext[freeuint323], &meshes[freeuint321].colouredTriangles[freeuint161].vertex2, 2);
			freeuint323 += 2;
			memcpy(&meshtext[freeuint323], &meshes[freeuint321].colouredTriangles[freeuint161].vertex3, 2);
			freeuint323 += 2;
			memcpy(&meshtext[freeuint323], &meshes[freeuint321].colouredTriangles[freeuint161].texture, 2);
			freeuint323 += 2;
		}
		
		/* Moves to a 4-byte edge */
		if ((freeuint323 & 0x00000003) != 0x00000000)
		{
			memset(&meshtext[freeuint323], 0, 2);
			freeuint323 += 2;
		}
	}
	
	/* Removes the old mesh data and mesh pointers from the level */
	freeuint321 = (pc_info.p_NumAnimations - pc_info.p_NumMeshData);
	errorNumber = removebytes(pc, pcpath, pc_info.p_NumMeshData, freeuint321);
	if (errorNumber != 0)
	{
		goto end;
	}
	
	/* Makes room for new mesh data and mesh pointers */
	freeuint321 = (freeuint323 + (numMeshes * 4) + 8);
	errorNumber = insertbytes(pc, pc_info.p_NumMeshData, freeuint321);
	if (errorNumber != 0)
	{
		goto end;
	}
	
	/* Writes numMeshData */
	freeuint321 = (freeuint323 >> 1);
	freeint1 = fseek(pc, (long int) pc_info.p_NumMeshData, SEEK_SET);
	if (freeint1 != 0)
	{
		errorNumber = 4;
		goto end;
	}
	freesizet1 = fwrite(&freeuint321, 1, 4, pc);
	if (freesizet1 != 4)
	{
		errorNumber = 25;
		goto end;
	}
	
	/* Writes mesh data */
	freesizet1 = fwrite(meshtext, 1, freeuint323, pc);
	if (freesizet1 != freeuint323)
	{
		errorNumber = 25;
		goto end;
	}
	
	/* Writes NumMeshPointers */
	freesizet1 = fwrite(&numMeshes, 1, 4, pc);
	if (freesizet1 != 4)
	{
		errorNumber = 25;
		goto end;
	}
	
	/* Writes mesh pointers */
	freeuint321 = (numMeshes * 4);
	freesizet1 = fwrite(meshPointers, 1, freeuint321, pc);
	if (freesizet1 != freeuint321)
	{
		errorNumber = 25;
		goto end;
	}
	
end:/* Closes files, frees memory and returns the function */
	if (pc != NULL)
	{
		freeint1 = fclose(pc);
		if (freeint1 != 0)
		{
			errorNumber = 5;
		}
	}
	if (meshfile != NULL)
	{
		freeint1 = fclose(meshfile);
		if (freeint1 != 0)
		{
			errorNumber = 24;
		}
	}
	
	if (meshes != NULL)
	{
		while (0 == 0)
		{
			--numMeshes;
			if (meshes[numMeshes].vertices != NULL)
			{
				free(meshes[numMeshes].vertices);
			}
			if (meshes[numMeshes].normals != NULL)
			{
				free(meshes[numMeshes].normals);
			}
			if (meshes[numMeshes].lights != NULL)
			{
				free(meshes[numMeshes].lights);
			}
			if (meshes[numMeshes].texturedRectangles != NULL)
			{
				free(meshes[numMeshes].texturedRectangles);
			}
			if (meshes[numMeshes].texturedTriangles != NULL)
			{
				free(meshes[numMeshes].texturedTriangles);
			}
			if (meshes[numMeshes].colouredRectangles != NULL)
			{
				free(meshes[numMeshes].colouredRectangles);
			}
			if (meshes[numMeshes].colouredTriangles != NULL)
			{
				free(meshes[numMeshes].colouredTriangles);
			}
			if (numMeshes == 0x00000000)
			{
				break;
			}
		}
		free(meshes);
	}
	
	if (pc_info.room != NULL)
	{
		free(pc_info.room);
	}
	if (meshtext != NULL)
	{
		free(meshtext);
	}
	if (meshPointers != NULL)
	{
		free(meshPointers);
	}
	
	return errorNumber;
}

/*
 * Function that prints the mesh data to stdout.
 * Parameters:
 * * pcpath = Path to the TR2 file
 * Return values:
 * * 0 = Everything went well
 */
static int dataToText(char *pcpath)
{
	/* Variable Declatations */
	FILE *pc = NULL; /* Pointer to PC-file */
	char *meshdata = NULL; /* In-memory copy of mesh data */
	INTU32 *meshpointers = NULL; /* In-memory copy of mesh pointers */
	struct level_info pc_info;  /* Offsets and values in PC-file */
	int freeint1;       /* Integer for general use */
	size_t freesizet1;  /* size_t for general use */
	INTU32 freeuint321,
	       freeuint322;
	INTU16 freeuint161,
	       freeuint162;
	INT32 freeint321;
	INT16 freeint161,
	      freeint162,
	      freeint163;
	long int freelong1;
	int errorNumber = 0; /* Return value for this program */
	
	/* Sets level_info structs to NULL before we start */
	level_info_wipe(pc_info);
	
	/* Opens PC-file */
	pc = fopen(pcpath, "rb");
	if (pc == NULL)
	{
		errorNumber = 3;
		goto end;
	}
	
	/* Determines number of rooms in the level */
	freeint1 = fseek(pc, 1796, SEEK_SET);
	if (freeint1 != 0)
	{
		errorNumber = 4;
		goto end;
	}
	freesizet1 = fread(&pc_info.v_NumTexTiles, 1, 4, pc);
	if (freesizet1 != 4)
	{
		errorNumber = 4;
		goto end;
	}
	#ifdef __BIG_ENDIAN__
	pc_info.v_NumTexTiles = reverse32(pc_info.v_NumTexTiles);
	#endif
	pc_info.p_NumRooms = (1804 + (pc_info.v_NumTexTiles * 196608));
	freeint1 = fseek(pc, (long int)pc_info.p_NumRooms, SEEK_SET);
	if (freeint1 != 0)
	{
		errorNumber = 4;
		goto end;
	}
	freesizet1 = fread(&pc_info.v_NumRooms, 1, 2, pc);
	if (freesizet1 != 2)
	{
		errorNumber = 4;
		goto end;
	}
	#ifdef __BIG_ENDIAN__
	pc_info.v_NumRooms = reverse16(pc_info.v_NumRooms);
	#endif
	
	/* Allocates space for room-related variables */
	pc_info.room = calloc((size_t)pc_info.v_NumRooms, sizeof(struct room_info));
	if (pc_info.room == NULL)
	{
		errorNumber = 10;
		goto end;
	}
	
	/* Navigates through PC-file */
	errorNumber = tr2pc_navigate(pc, &pc_info);
	if (errorNumber != 0)
	{
		goto end;
	}

	/* Reads the mesh data into memory */
	freeuint321 = (pc_info.v_NumMeshData * 2);
	meshdata = malloc(freeuint321);
	if (meshdata == NULL)
	{
		errorNumber = 10;
		goto end;
	}
	freelong1 = (long int) (pc_info.p_NumMeshData + 4);
	freeint1 = fseek(pc, freelong1, SEEK_SET);
	if (freeint1 != 0)
	{
		errorNumber = 4;
		goto end;
	}
	freesizet1 = fread(meshdata, 1, freeuint321, pc);
	if (freesizet1 != freeuint321)
	{
		errorNumber = 4;
		goto end;
	}
	
	/* Reads the mesh pointers into memory */
	meshpointers = calloc(pc_info.v_NumMeshPointers, 4);
	if (meshpointers == NULL)
	{
		errorNumber = 10;
		goto end;
	}
	freelong1 = (long int) (pc_info.p_NumMeshPointers + 4);
	freeint1 = fseek(pc, freelong1, SEEK_SET);
	if (freeint1 != 0)
	{
		errorNumber = 4;
		goto end;
	}
	freeuint321 = (pc_info.v_NumMeshPointers * 4);
	freesizet1 = fread(meshpointers, 1, freeuint321, pc);
	if (freesizet1 != freeuint321)
	{
		errorNumber = 4;
		goto end;
	}

	/* Prints the mesh data to the output */
	for (freeuint321 = 0x00000000; freeuint321 < pc_info.v_NumMeshPointers; ++freeuint321)
	{
		/* Adjusts mesh pointers for endianness if needed */
		#ifdef __BIG_ENDIAN__
		meshpointers[freeuint321] = reverse32(meshpointers[freeuint321]);
		#endif
		
		/* Prints the mesh number */
		printf("mesh(" PRINTU32 ")[" PRINTU32 "]{\n", freeuint321, meshpointers[freeuint321]);
		
		/* Prints the centre */
		freeuint322 = meshpointers[freeuint321];
		memcpy(&freeint161, &meshdata[freeuint322], 2);
		freeuint322 += 2;
		memcpy(&freeint162, &meshdata[freeuint322], 2);
		freeuint322 += 2;
		memcpy(&freeint163, &meshdata[freeuint322], 2);
		freeuint322 += 2;
		#ifdef __BIG_ENDIAN__
		freeint161 = reverse16(freeint161);
		freeint162 = reverse16(freeint162);
		freeint163 = reverse16(freeint163);
		#endif
		printf("\tcentre(" PRINT16 "," PRINT16 "," PRINT16 ")\n", freeint161, freeint162, freeint163);
		
		/* Prints the radius */
		memcpy(&freeint321, &meshdata[freeuint322], 4);
		freeuint322 += 4;
		#ifdef __BIG_ENDIAN__
		freeint321 = reverse32(freeint321);
		#endif
		printf("\tradius(" PRINT32 ")\n", freeint321);
		
		/* Prints the vertices */
		memcpy(&freeuint161, &meshdata[freeuint322], 2);
		freeuint322 += 2;
		#ifdef __BIG_ENDIAN__
		freeuint161 = reverse16(freeuint161);
		#endif
		for (freeuint162 = 0x0000; freeuint162 < freeuint161; ++freeuint162)
		{
			printf("\tvertex(" PRINTU16 "){", freeuint162);
			memcpy(&freeint161, &meshdata[freeuint322], 2);
			freeuint322 += 2;
			#ifdef __BIG_ENDIAN__
			freeint161 = reverse16(freeint161);
			#endif
			printf(PRINT16 ",", freeint161);
			memcpy(&freeint161, &meshdata[freeuint322], 2);
			freeuint322 += 2;
			#ifdef __BIG_ENDIAN__
			freeint161 = reverse16(freeint161);
			#endif
			printf(PRINT16 ",", freeint161);
			memcpy(&freeint161, &meshdata[freeuint322], 2);
			freeuint322 += 2;
			#ifdef __BIG_ENDIAN__
			freeint161 = reverse16(freeint161);
			#endif
			printf(PRINT16 "}\n", freeint161);
		}
		
		memcpy(&freeint161, &meshdata[freeuint322], 2);
		freeuint322 += 2;
		#ifdef __BIG_ENDIAN__
		freeint161 = reverse16(freeint161);
		#endif
		if (freeint161 >= 0x0000)
		{
			/* Prints the normals */
			freeuint161 = (INTU16) freeint161;
			for (freeuint162 = 0x0000; freeuint162 < freeuint161; ++freeuint162)
			{
				printf("\tnormal(" PRINTU16 "){", freeuint162);
				memcpy(&freeint161, &meshdata[freeuint322], 2);
				freeuint322 += 2;
				#ifdef __BIG_ENDIAN__
				freeint161 = reverse16(freeint161);
				#endif
				printf(PRINT16 ",", freeint161);
				memcpy(&freeint161, &meshdata[freeuint322], 2);
				freeuint322 += 2;
				#ifdef __BIG_ENDIAN__
				freeint161 = reverse16(freeint161);
				#endif
				printf(PRINT16 ",", freeint161);
				memcpy(&freeint161, &meshdata[freeuint322], 2);
				freeuint322 += 2;
				#ifdef __BIG_ENDIAN__
				freeint161 = reverse16(freeint161);
				#endif
				printf(PRINT16 "}\n", freeint161);
			}
		}
		else
		{
			/* Prints the lights */
			freeuint161 = (INTU16) abs(freeint161);
			for (freeuint162 = 0x0000; freeuint162 < freeuint161; ++freeuint162)
			{
				printf("\tlight(" PRINTU16 "){", freeuint162);
				memcpy(&freeint161, &meshdata[freeuint322], 2);
				freeuint322 += 2;
				#ifdef __BIG_ENDIAN__
				freeint161 = reverse16(freeint161);
				#endif
				printf(PRINT16 "}\n", freeint161);
			}
		}
		
		/* Prints the textured rectangles */
		memcpy(&freeuint161, &meshdata[freeuint322], 2);
		freeuint322 += 2;
		#ifdef __BIG_ENDIAN__
		freeuint161 = reverse16(freeuint161);
		#endif
		for (freeuint162 = 0x0000; freeuint162 < freeuint161; ++freeuint162)
		{
			printf("\ttexturedrectangle(" PRINTU16 "){", freeuint162);
			memcpy(&freeint161, &meshdata[freeuint322], 2);
			freeuint322 += 2;
			#ifdef __BIG_ENDIAN__
			freeint161 = reverse16(freeint161);
			#endif
			printf(PRINT16 ",", freeint161);
			memcpy(&freeint161, &meshdata[freeuint322], 2);
			freeuint322 += 2;
			#ifdef __BIG_ENDIAN__
			freeint161 = reverse16(freeint161);
			#endif
			printf(PRINT16 ",", freeint161);
			memcpy(&freeint161, &meshdata[freeuint322], 2);
			freeuint322 += 2;
			#ifdef __BIG_ENDIAN__
			freeint161 = reverse16(freeint161);
			#endif
			printf(PRINT16 ",", freeint161);
			memcpy(&freeint161, &meshdata[freeuint322], 2);
			freeuint322 += 2;
			#ifdef __BIG_ENDIAN__
			freeint161 = reverse16(freeint161);
			#endif
			printf(PRINT16 ",", freeint161);
			memcpy(&freeint161, &meshdata[freeuint322], 2);
			freeuint322 += 2;
			#ifdef __BIG_ENDIAN__
			freeint161 = reverse16(freeint161);
			#endif
			printf(PRINT16 "}\n", freeint161);
		}
		
		/* Prints the textured triangles */
		memcpy(&freeuint161, &meshdata[freeuint322], 2);
		freeuint322 += 2;
		#ifdef __BIG_ENDIAN__
		freeuint161 = reverse16(freeuint161);
		#endif
		for (freeuint162 = 0x0000; freeuint162 < freeuint161; ++freeuint162)
		{
			printf("\ttexturedtriangle(" PRINTU16 "){", freeuint162);
			memcpy(&freeint161, &meshdata[freeuint322], 2);
			freeuint322 += 2;
			#ifdef __BIG_ENDIAN__
			freeint161 = reverse16(freeint161);
			#endif
			printf(PRINT16 ",", freeint161);
			memcpy(&freeint161, &meshdata[freeuint322], 2);
			freeuint322 += 2;
			#ifdef __BIG_ENDIAN__
			freeint161 = reverse16(freeint161);
			#endif
			printf(PRINT16 ",", freeint161);
			memcpy(&freeint161, &meshdata[freeuint322], 2);
			freeuint322 += 2;
			#ifdef __BIG_ENDIAN__
			freeint161 = reverse16(freeint161);
			#endif
			printf(PRINT16 ",", freeint161);
			memcpy(&freeint161, &meshdata[freeuint322], 2);
			freeuint322 += 2;
			#ifdef __BIG_ENDIAN__
			freeint161 = reverse16(freeint161);
			#endif
			printf(PRINT16 "}\n", freeint161);
		}
		
		/* Prints the coloured rectangles */
		memcpy(&freeuint161, &meshdata[freeuint322], 2);
		freeuint322 += 2;
		#ifdef __BIG_ENDIAN__
		freeuint161 = reverse16(freeuint161);
		#endif
		for (freeuint162 = 0x0000; freeuint162 < freeuint161; ++freeuint162)
		{
			printf("\tcolouredrectangle(" PRINTU16 "){", freeuint162);
			memcpy(&freeint161, &meshdata[freeuint322], 2);
			freeuint322 += 2;
			#ifdef __BIG_ENDIAN__
			freeint161 = reverse16(freeint161);
			#endif
			printf(PRINT16 ",", freeint161);
			memcpy(&freeint161, &meshdata[freeuint322], 2);
			freeuint322 += 2;
			#ifdef __BIG_ENDIAN__
			freeint161 = reverse16(freeint161);
			#endif
			printf(PRINT16 ",", freeint161);
			memcpy(&freeint161, &meshdata[freeuint322], 2);
			freeuint322 += 2;
			#ifdef __BIG_ENDIAN__
			freeint161 = reverse16(freeint161);
			#endif
			printf(PRINT16 ",", freeint161);
			memcpy(&freeint161, &meshdata[freeuint322], 2);
			freeuint322 += 2;
			#ifdef __BIG_ENDIAN__
			freeint161 = reverse16(freeint161);
			#endif
			printf(PRINT16 ",", freeint161);
			memcpy(&freeint161, &meshdata[freeuint322], 2);
			freeuint322 += 2;
			#ifdef __BIG_ENDIAN__
			freeint161 = reverse16(freeint161);
			#endif
			printf(PRINT16 "}\n", freeint161);
		}
		
		/* Prints the coloured triangles */
		memcpy(&freeuint161, &meshdata[freeuint322], 2);
		freeuint322 += 2;
		#ifdef __BIG_ENDIAN__
		freeuint161 = reverse16(freeuint161);
		#endif
		for (freeuint162 = 0x0000; freeuint162 < freeuint161; ++freeuint162)
		{
			printf("\tcolouredtriangle(" PRINTU16 "){", freeuint162);
			memcpy(&freeint161, &meshdata[freeuint322], 2);
			freeuint322 += 2;
			#ifdef __BIG_ENDIAN__
			freeint161 = reverse16(freeint161);
			#endif
			printf(PRINT16 ",", freeint161);
			memcpy(&freeint161, &meshdata[freeuint322], 2);
			freeuint322 += 2;
			#ifdef __BIG_ENDIAN__
			freeint161 = reverse16(freeint161);
			#endif
			printf(PRINT16 ",", freeint161);
			memcpy(&freeint161, &meshdata[freeuint322], 2);
			freeuint322 += 2;
			#ifdef __BIG_ENDIAN__
			freeint161 = reverse16(freeint161);
			#endif
			printf(PRINT16 ",", freeint161);
			memcpy(&freeint161, &meshdata[freeuint322], 2);
			freeuint322 += 2;
			#ifdef __BIG_ENDIAN__
			freeint161 = reverse16(freeint161);
			#endif
			printf(PRINT16 "}\n", freeint161);
		}
		printf("}\n");
	}
	
end:/* Closes files, frees memory and returns the function */
	if (pc != NULL)
	{
		freeint1 = fclose(pc);
		if (freeint1 != 0)
		{
			errorNumber = 5;
		}
	}
	if (pc_info.room != NULL)
	{
		free(pc_info.room);
	}
	if (meshdata != NULL)
	{
		free(meshdata);
	}
	if (meshpointers != NULL)
	{
		free(meshpointers);
	}
	
	return errorNumber;
}

/*
 * Function that navigates through the PC-file reading offsets and values.
 * Parameters:
 * * pc = Pointer to the level file
 * * pc_info = Pointer to the level_info struct where the data is to be stored
 * Return values:
 * * 0 = Everything went well
 * * 4 = PC-file could not be read from
 * Nota bene:
 * * Expects v_NumTexTiles,p_NumRooms,and v_Numrooms inside pc_info to be set
 * * and all room-related variables to be already allocated.
 */
static int tr2pc_navigate(FILE *pc, struct level_info *pc_info)
{
	/* Variable Declarations */
	INTU16 freeuint161; /* 16-bit unsigned integer for general use */
	int freeint1;       /* Integer for general use */
	size_t freesizet1;  /* size_t fo general use */
	
	/* Loops through rooms */
	freeint1 = fseek(pc, (long int)((pc_info->p_NumRooms) + 2), SEEK_SET);
	if (freeint1 != 0)
	{
		return 4;
	}
	for (freeuint161 = 0; freeuint161 < (pc_info->v_NumRooms); ++freeuint161)
	{
		/* NumVertices */
		freeint1 = fseek(pc, 20, SEEK_CUR);
		if (freeint1 != 0)
		{
			return 4;
		}
		pc_info->room[freeuint161].p_NumVertices = ((INTU32) ftell(pc));
		freesizet1 = fread(&(pc_info->room[freeuint161].v_NumVertices), 1, 2, pc);
		if (freesizet1 != 2)
		{
			return 4;
		}
		#ifdef __BIG_ENDIAN__
		pc_info->room[freeuint161].v_NumVertices =
		    reverse16(pc_info->room[freeuint161].v_NumVertices);
		#endif
		/* NumRectangles */
		pc_info->room[freeuint161].p_NumRectangles =
		    (pc_info->room[freeuint161].p_NumVertices + 2 +
		     (pc_info->room[freeuint161].v_NumVertices * 12));
		freeint1 = fseek(pc, (long int)pc_info->room[freeuint161].p_NumRectangles,
		                 SEEK_SET);
		if (freeint1 != 0)
		{
			return 4;
		}
		freesizet1 = fread(&(pc_info->room[freeuint161].v_NumRectangles), 1, 2, pc);
		if (freesizet1 != 2)
		{
			return 4;
		}
		#ifdef __BIG_ENDIAN__
		pc_info->room[freeuint161].v_NumRectangles = reverse16(
		            pc_info->room[freeuint161].v_NumRectangles);
		#endif
		/* NumTriangles */
		pc_info->room[freeuint161].p_NumTriangles =
		    (pc_info->room[freeuint161].p_NumRectangles + 2 +
		     (pc_info->room[freeuint161].v_NumRectangles * 10));
		freeint1 = fseek(pc, (long int)pc_info->room[freeuint161].p_NumTriangles,
		                 SEEK_SET);
		if (freeint1 != 0)
		{
			return 4;
		}
		freesizet1 = fread(&(pc_info->room[freeuint161].v_NumTriangles), 1, 2, pc);
		if (freesizet1 != 2)
		{
			return 4;
		}
		#ifdef __BIG_ENDIAN__
		pc_info->room[freeuint161].v_NumTriangles = reverse16(
		            pc_info->room[freeuint161].v_NumTriangles);
		#endif
		/* NumSprites */
		pc_info->room[freeuint161].p_NumSprites =
		    (pc_info->room[freeuint161].p_NumTriangles + 2 +
		     (pc_info->room[freeuint161].v_NumTriangles * 8));
		freeint1 = fseek(pc, (long int)pc_info->room[freeuint161].p_NumSprites,
		                 SEEK_SET);
		if (freeint1 != 0)
		{
			return 4;
		}
		freesizet1 = fread(&(pc_info->room[freeuint161].v_NumSprites), 1, 2, pc);
		if (freesizet1 != 2)
		{
			return 4;
		}
		#ifdef __BIG_ENDIAN__
		pc_info->room[freeuint161].v_NumSprites = reverse16(
		            pc_info->room[freeuint161].v_NumSprites);
		#endif
		/* NumDoors */
		pc_info->room[freeuint161].p_NumDoors =
		    (pc_info->room[freeuint161].p_NumSprites + 2
		     + (pc_info->room[freeuint161].v_NumSprites * 4));
		freeint1 = fseek(pc, (long int)pc_info->room[freeuint161].p_NumDoors,
		                 SEEK_SET);
		if (freeint1 != 0)
		{
			return 4;
		}
		freesizet1 = fread(&(pc_info->room[freeuint161].v_NumDoors), 1, 2, pc);
		if (freesizet1 != 2)
		{
			return 4;
		}
		#ifdef __BIG_ENDIAN__
		pc_info->room[freeuint161].v_NumDoors = reverse16(
		        pc_info->room[freeuint161].v_NumDoors);
		#endif
		/* NumZSectors */
		pc_info->room[freeuint161].p_NumZSectors =
		    (pc_info->room[freeuint161].p_NumDoors
		     + 2 + (pc_info->room[freeuint161].v_NumDoors * 32));
		freeint1 = fseek(pc, (long int)pc_info->room[freeuint161].p_NumZSectors,
		                 SEEK_SET);
		if (freeint1 != 0)
		{
			return 4;
		}
		freesizet1 = fread(&(pc_info->room[freeuint161].v_NumZSectors), 1, 2, pc);
		if (freesizet1 != 2)
		{
			return 4;
		}
		#ifdef __BIG_ENDIAN__
		pc_info->room[freeuint161].v_NumZSectors = reverse16(
		            pc_info->room[freeuint161].v_NumZSectors);
		#endif
		/* NumXSectors */
		pc_info->room[freeuint161].p_NumXSectors =
		    (pc_info->room[freeuint161].p_NumZSectors + 2);
		freeint1 = fseek(pc, (long int)pc_info->room[freeuint161].p_NumXSectors,
		                 SEEK_SET);
		if (freeint1 != 0)
		{
			return 4;
		}
		freesizet1 = fread(&(pc_info->room[freeuint161].v_NumXSectors), 1, 2, pc);
		if (freesizet1 != 2)
		{
			return 4;
		}
		#ifdef __BIG_ENDIAN__
		pc_info->room[freeuint161].v_NumXSectors = reverse16(
		            pc_info->room[freeuint161].v_NumXSectors);
		#endif
		/* NumLights */
		pc_info->room[freeuint161].p_NumLights =
		    (pc_info->room[freeuint161].p_NumXSectors
		     + 6 + (pc_info->room[freeuint161].v_NumZSectors *
		            pc_info->room[freeuint161].v_NumXSectors * 8));
		freeint1 = fseek(pc, (long int)pc_info->room[freeuint161].p_NumLights,
		                 SEEK_SET);
		if (freeint1 != 0)
		{
			return 4;
		}
		freesizet1 = fread(&(pc_info->room[freeuint161].v_NumLights), 1, 2, pc);
		if (freesizet1 != 2)
		{
			return 4;
		}
		#ifdef __BIG_ENDIAN__
		pc_info->room[freeuint161].v_NumLights = reverse16(
		            pc_info->room[freeuint161].v_NumLights);
		#endif
		/* NumStaticMeshes */
		pc_info->room[freeuint161].p_NumStaticMeshes =
		    (pc_info->room[freeuint161].p_NumLights + 2 +
		     (pc_info->room[freeuint161].v_NumLights * 24));
		freeint1 = fseek(pc, (long int)pc_info->room[freeuint161].p_NumStaticMeshes
		                 , SEEK_SET);
		if (freeint1 != 0)
		{
			return 4;
		}
		freesizet1 = fread(&(pc_info->room[freeuint161].v_NumStaticMeshes), 1, 2,
		                   pc);
		if (freesizet1 != 2)
		{
			return 4;
		}
		#ifdef __BIG_ENDIAN__
		pc_info->room[freeuint161].v_NumStaticMeshes = reverse16(
		            pc_info->room[freeuint161].v_NumStaticMeshes);
		#endif
		/* AlternateRoom */
		pc_info->room[freeuint161].p_AlternateRoom =
		    (pc_info->room[freeuint161].p_NumStaticMeshes + 2 +
		     (pc_info->room[freeuint161].v_NumStaticMeshes * 20));
		freeint1 = fseek(pc, (long int)pc_info->room[freeuint161].p_AlternateRoom +
		                 7, SEEK_SET);
		if (freeint1 != 0)
		{
			return 4;
		}
	}
	/* NumFloorData */
	pc_info->p_NumFloorData = ((INTU32) ftell(pc));
	freesizet1 = fread(&(pc_info->v_NumFloorData), 1, 4, pc);
	if (freesizet1 != 4)
	{
		return 4;
	}
	#ifdef __BIG_ENDIAN__
	pc_info->v_NumFloorData = reverse32(pc_info->v_NumFloorData);
	#endif
	/* NumMeshData */
	pc_info->p_NumMeshData = (pc_info->p_NumFloorData + 4 +
	                          (pc_info->v_NumFloorData * 2));
	freeint1 = fseek(pc, (long int)pc_info->p_NumMeshData, SEEK_SET);
	if (freeint1 != 0)
	{
		return 4;
	}
	freesizet1 = fread(&(pc_info->v_NumMeshData), 1, 4, pc);
	if (freesizet1 != 4)
	{
		return 4;
	}
	#ifdef __BIG_ENDIAN__
	pc_info->v_NumMeshData = reverse32(pc_info->v_NumMeshData);
	#endif
	/* NumMeshPointers */
	pc_info->p_NumMeshPointers = (pc_info->p_NumMeshData + 4 +
	                              (pc_info->v_NumMeshData * 2));
	freeint1 = fseek(pc, (long int)pc_info->p_NumMeshPointers, SEEK_SET);
	if (freeint1 != 0)
	{
		return 4;
	}
	freesizet1 = fread(&(pc_info->v_NumMeshPointers), 1, 4, pc);
	if (freesizet1 != 4)
	{
		return 4;
	}
	#ifdef __BIG_ENDIAN__
	pc_info->v_NumMeshPointers = reverse32(pc_info->v_NumMeshPointers);
	#endif
	/* NumAnimations */
	pc_info->p_NumAnimations = (pc_info->p_NumMeshPointers + 4 +
	                            (pc_info->v_NumMeshPointers * 4));
	freeint1 = fseek(pc, (long int)pc_info->p_NumAnimations, SEEK_SET);
	if (freeint1 != 0)
	{
		return 4;
	}
	freesizet1 = fread(&(pc_info->v_NumAnimations), 1, 4, pc);
	if (freesizet1 != 4)
	{
		return 4;
	}
	#ifdef __BIG_ENDIAN__
	pc_info->v_NumAnimations = reverse32(pc_info->v_NumAnimations);
	#endif
	
	return 0;
}

/*
 * Function that checks the sizes of fixed-size integers.
 * Return values:
 * * 0 = All integers are the correct sizes
 * * 1 = Some integers are not the correct sizes
 */
#ifdef FIXEDINT_CHECK_ON_STARTUP
static int fixedint_check(void)
{
	/* Variable Declarations */
	int retval = 0; /* Return value for this function */
	
	/* Checks the sizes of the integers */
	if (sizeof(INT8) != 1)
	{
		printf("INT8 should be an 8-bit signed integer.\n"
		       "Instead it is %i-bits.\n", (int) (sizeof(INT8) * 8));
		retval = 1;
	}
	if (sizeof(INTU8) != 1)
	{
		printf("INTU8 should be an 8-bit unsigned integer.\n"
		       "Instead it is %i-bits.\n", (int) (sizeof(INTU8) * 8));
		retval = 1;
	}
	if (sizeof(INT16) != 2)
	{
		printf("INT16 should be a 16-bit signed integer.\n"
		       "Instead it is %i-bits.\n", (int) (sizeof(INT16) * 8));
		retval = 1;
	}
	if (sizeof(INTU16) != 2)
	{
		printf("INTU16 should be a 16-bit unsigned integer.\n"
		       "Instead it is %i-bits.\n", (int) (sizeof(INTU16) * 8));
		retval = 1;
	}
	if (sizeof(INT32) != 4)
	{
		printf("INT32 should be a 32-bit signed integer.\n"
		       "Instead it is %i-bits.\n", (int) (sizeof(INT32) * 8));
		retval = 1;
	}
	if (sizeof(INTU32) != 4)
	{
		printf("INTU32 should be a 32-bit unsigned integer.\n"
		       "Instead it is %i-bits.\n", (int) (sizeof(INTU32) * 8));
		retval = 1;
	}
	if (retval == 1)
	{
		printf("Please modify fixedint.h and recompile the program.\n");
	}
	
	return retval;
}
#endif

/*
 * Prints error messages if needed.
 * Parameters:
 * * errorNumber = The number of the error
 * * argv = Command-line parameters
 * * pcparam = Number of the parameter to PC-file
 */
static void printError(int errorNumber, char *argv[], int pcparam)
{
	switch (errorNumber)
	{
		case 1:
			#ifdef CLEARTERMINAL
			CLEARTERMINAL
			#endif
			printf("Tomb Raider II Mesh Editor\n"
			       "  Copyright (C) 2019  b122251\n\n"
			       "Usage:\n"
			       "  %s [LEVEL]              (prints the mesh data)\n"
			       "  %s [LEVEL] [TEXTFILE]   (replaces mesh data)\n"
			       "\n", argv[0], argv[0]);
			break;
		case 2:
			printf("tr2mesh: %s is not a TR2-file\n", argv[pcparam]);
			break;
		case 3:
			printf("tr2mesh: %s could not be opened\n", argv[pcparam]);
			break;
		case 4:
			printf("tr2mesh: %s could not be read from\n", argv[pcparam]);
			break;
		case 5:
			printf("tr2mesh: %s could not be closed\n", argv[pcparam]);
			break;
		case 10:
			printf("tr2mesh: Memory could not be allocated\n");
			break;
		case 11:
			printf("tr2mesh: Data to be removed doesn't exist (file error)\n");
			break;
		case 24:
			printf("tr2mesh: %s could not be read\n", argv[2]);
			break;
		case 25:
			printf("tr2mesh: %s could not be written to\n", argv[pcparam]);
			break;
	}
}
