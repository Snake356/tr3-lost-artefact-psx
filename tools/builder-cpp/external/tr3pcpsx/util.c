/*
 * Library that contains general functions used by TR3PCPSX.
 * Copyright (C) 2021 b122251
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation,either version 3 of the License,or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY;without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not,see <http://www.gnu.org/licenses/>.
 */

/* File Inclusions */
#include <stdio.h>    /* Standard input and output */
#include <stdlib.h>   /* Standard library of utility functions */
#include <string.h>   /* Functions relating to strings */
#include <ctype.h>    /* Definition of char-type (used for tolower) */
#include "fixedint.h" /* Definition of fixed-size integers */
#include "structs.h"  /* Definition of structs used by TR3PCPSX */
#include "osdep.h"    /* Library of OS-dependant functions in TR3PCPSX */
#include "errors.h"   /* Error Handling for TR3PCPSX */

/*
 * Function that inserts bytes into the level.
 * Parameters:
 * * level = File pointer to level file
 * * offset = Offset where the bytes are to be inserted
 * * length = Number of bytes to be inserted
 */
int insertbytes(FILE *level, INTU32 offset, INTU32 length)
{
	/* Variable Declarations */
	void *buffer = NULL;  /* Buffer space */
	size_t bufferSize;    /* Length of the buffer space */
	INTU32 nummovebytes;  /* Number of bytes to be moved */
	INTU32 posmovebytes;  /* Position of bytes to be moved */
	INTU32 destmovebytes; /* Destination of bytes to be moved */
	int errorNumber;      /* Return value for this function */
	size_t freesizet1;    /* size_t for general use */
	int freeint1;         /* Integer for general use */
	INTU32 freeuint321;   /* Unsigned 32-bit integer for general use */
	INTU32 levelSize;     /* Size of the level file in bytes */
	errorNumber = ERROR_NONE;
	
	/* Determines levelSize */
	freeint1 = fseek(level, 0, SEEK_END);
	if (freeint1 != 0)
	{
		return ERROR_PSX_READ_FAILED;
	}
	levelSize = (INTU32) ftell(level);
	
	if (offset > levelSize)
	{
		/* Fills up empty space with zeroes if offset is outside of level */
		freeint1 = fseek(level, 0, SEEK_END);
		if (freeint1 != 0)
		{
			errorNumber = ERROR_PSX_READ_FAILED;
			goto end;
		}
		for (freeuint321 = ((offset + length) - levelSize);
		     freeuint321 > (INTU32) 0; --freeuint321)
		{
			freeint1 = fputc(0, level);
			if (freeint1 != 0)
			{
				errorNumber = ERROR_PSX_WRITE_FAILED;
				goto end;
			}
		}
	}
	else
	{
		/* Calculates nummovebytes */
		nummovebytes = (levelSize - offset);
		
		if (nummovebytes != (INTU32) 0)
		{
			/* Allocates buffer */
			bufferSize = ((size_t) (nummovebytes));
			if ((sizeof(bufferSize) == 2) && ((nummovebytes & 0xFFFF0000) !=
			                                  (INTU32) 0x00000000))
			{
				(void) memset(&bufferSize, 255, 2);
			}
			do
			{
				buffer = malloc(bufferSize);
				if (buffer == NULL)
				{
					bufferSize >>= 1;
					if (bufferSize <= 1)
					{
						errorNumber = ERROR_MEMORY;
						goto end;
					}
				}
			}
			while (buffer == NULL);
			
			/* Adjusts for a very small buffer if needed */
			if (((INTU32) bufferSize) < length)
			{
				freeint1 = fseek(level, 0, SEEK_END);
				if (freeint1 != 0)
				{
					errorNumber = ERROR_PSX_READ_FAILED;
					goto end;
				}
				freeuint321 = (INTU32) (length - (INTU32) bufferSize);
				for (; freeuint321 > (INTU32) 0; --freeuint321)
				{
					freeint1 = fputc(0, level);
					if (freeint1 != 0)
					{
						errorNumber = ERROR_PSX_WRITE_FAILED;
						goto end;
					}
				}
			}
			
			/* Moves data in chunks */
			posmovebytes = (levelSize);
			destmovebytes = (posmovebytes + length);
			while (nummovebytes > (INTU32) 0)
			{
				/* Adjusts size of final chunk if needed */
				if (nummovebytes < ((INTU32) bufferSize))
				{
					bufferSize = (size_t) nummovebytes;
				}
				
				/* Corrects source and destination positions */
				posmovebytes -= (INTU32) bufferSize;
				destmovebytes -= (INTU32) bufferSize;
				
				/* Goes the the offset to read from */
				freeint1 = fseek(level, (long int) posmovebytes, SEEK_SET);
				if (freeint1 != 0)
				{
					errorNumber = ERROR_PSX_READ_FAILED;
					goto end;
				}
				
				/* Reads chunk into memory */
				freesizet1 = fread(buffer, 1, bufferSize, level);
				if (freesizet1 != bufferSize)
				{
					errorNumber = ERROR_PSX_READ_FAILED;
					goto end;
				}
				
				/* Goes to the offset to write to */
				freeint1 = fseek(level, (long int) destmovebytes, SEEK_SET);
				if (freeint1 != 0)
				{
					errorNumber = ERROR_PSX_READ_FAILED;
					goto end;
				}
				
				/* Writes chunk to the file */
				freesizet1 = fwrite(buffer, 1, bufferSize, level);
				if (freesizet1 != bufferSize)
				{
					errorNumber = ERROR_PSX_WRITE_FAILED;
					goto end;
				}
				
				/* Iterates counting variable */
				nummovebytes -= (INTU32) bufferSize;
			}
		}
	}
	
end:/* Frees buffer and closes the function */
	if (buffer != NULL)
	{
		free(buffer);
	}
	return errorNumber;
}

/*
 * Function that removes bytes from the level.
 * Parameters:
 * * level = Pointer to the level file
 * * levelPath = Path to the level file
 * * offset = Offset the bytes to be removed are at
 * * length = Number of bytes to be removed
 */
int removebytes(FILE *level, char *levelPath, INTU32 offset, INTU32 length)
{
	/* Variable Declarations */
	void *buffer = NULL; /* Buffer space */
	size_t bufferSize;   /* Length of the buffer space */
	INTU32 posmovebytes; /* Position of bytes to be moved */
	INTU32 nummovebytes; /* Number of bytes to be moved */
	int errorNumber = 0; /* Return value for this function */
	int freeint1;        /* Integer for general use */
	INTU32 freeuint321;  /* Unsigned 32-bit integer for general use */
	size_t freesizet1;   /* size_t for general use */
	INTU32 levelSize;    /* Size of the level file in bytes */
	
	/* Determines levelSize */
	freeint1 = fseek(level, 0, SEEK_END);
	if (freeint1 != 0)
	{
		return ERROR_PSX_READ_FAILED;
	}
	levelSize = (INTU32) ftell(level);
	
	/* Checks whether offset and length are valid */
	if ((offset + length) > levelSize)
	{
		errorNumber = ERROR_REMOVE_DATA_INVALID;
		goto end;
	}
	
	/* Calculates nummovebytes and posmovebytes */
	posmovebytes = (offset + length);
	nummovebytes = (levelSize - posmovebytes);
	
	if (nummovebytes != (INTU32) 0)
	{
		/* Allocates buffer */
		bufferSize = ((size_t) (nummovebytes));
		if ((sizeof(bufferSize) == 2) &&
		    ((nummovebytes & 0xFFFF0000) != (INTU32) 0))
		{
			(void) memset(&bufferSize, 255, 2);
		}
		do
		{
			buffer = malloc(bufferSize);
			if (buffer == NULL)
			{
				bufferSize >>= 1;
				if (bufferSize <= 1)
				{
					errorNumber = ERROR_MEMORY;
					goto end;
				}
			}
		}
		while (buffer == NULL);
		
		/* Moves data in chunks */
		freeuint321 = offset;
		while (nummovebytes > (INTU32) 0)
		{
			/* Adjusts size of final chunk if needed */
			if (nummovebytes < ((INTU32) bufferSize))
			{
				bufferSize = (size_t) nummovebytes;
			}
			
			/* Goes the the offset to read from */
			freeint1 = fseek(level, (long int) posmovebytes, SEEK_SET);
			if (freeint1 != 0)
			{
				errorNumber = ERROR_PSX_READ_FAILED;
				goto end;
			}
			
			/* Reads chunk into memory */
			freesizet1 = fread(buffer, 1, bufferSize, level);
			if (freesizet1 != bufferSize)
			{
				errorNumber = ERROR_PSX_READ_FAILED;
				goto end;
			}
			
			/* Goes to the offset to write to */
			freeint1 = fseek(level, (long int) freeuint321, SEEK_SET);
			if (freeint1 != 0)
			{
				errorNumber = ERROR_PSX_READ_FAILED;
				goto end;
			}
			
			/* Writes chunk to the file */
			freesizet1 = fwrite(buffer, 1, bufferSize, level);
			if (freesizet1 != bufferSize)
			{
				errorNumber = ERROR_PSX_WRITE_FAILED;
				goto end;
			}
			
			/* Iterates counting variables */
			posmovebytes += (INTU32) bufferSize;
			nummovebytes -= (INTU32) bufferSize;
			freeuint321 += (INTU32) bufferSize;
		}
	}
	
	/* Truncates the file */
	freeint1 = truncateFile(levelPath, (levelSize - length));
	if (freeint1 != 0)
	{
		errorNumber = ERROR_PSX_WRITE_FAILED;
		goto end;
	}
	
end:/* Frees buffer and closes function */
	if (buffer != NULL)
	{
		free(buffer);
	}
	return errorNumber;
}

/*
 * Function that copies a certain number of bytes from file1 to file2.
 * Parameters:
 * * file1 = Source file to be read from
 * * file2 = Target file to be written to
 * * f1_offset = Offset in file1 to read from
 * * f2_offset = Offset in file2 to write to
 * * numBytes = Number of bytes to be copied
 */
int copybytes(FILE *file1, FILE *file2, INTU32 f1_offset, INTU32 f2_offset,
              INTU32 numBytes)
{
	/* Variable Declarations */
	void *buffer = NULL;   /* Memory space to use while copying */
	size_t bufferSize;     /* Size of the buffer */
	int freeint1;          /* Integer for general use */
	size_t freesizet1;     /* size_t for general use */
	int errorNumber;       /* Return value for this function */
	long int file1_curpos; /* Current position in file1 */
	long int file2_curpos; /* Current position in file2 */
	errorNumber = ERROR_NONE;
	
	/* Checks whether to copy any bytes at all */
	if (numBytes == 0x00000000)
	{
		return ERROR_NONE;
	}
	
	/* Allocates buffer */
	bufferSize = ((size_t) (numBytes));
	if ((sizeof(bufferSize) == 2) && ((numBytes & 0xFFFF0000) != (INTU32) 0))
	{
		(void) memset(&bufferSize, 255, 2);
	}
	do
	{
		buffer = malloc(bufferSize);
		if (buffer == NULL)
		{
			bufferSize >>= 1;
			if (bufferSize <= 1)
			{
				errorNumber = ERROR_MEMORY;
				goto end;
			}
		}
	}
	while (buffer == NULL);
	
	/* Moves to offsets in the files */
	file1_curpos = (long int) f1_offset;
	file2_curpos = (long int) f2_offset;
	
	/* Copies data in chunks */
	while (numBytes > 0x00000000)
	{
		/* Adjusts size of final chunk if needed */
		if (numBytes < ((INTU32) bufferSize))
		{
			bufferSize = (size_t) numBytes;
		}
		
		/* Moves to offset in file1 */
		freeint1 = fseek(file1, file1_curpos, SEEK_SET);
		if (freeint1 != 0)
		{
			errorNumber = ERROR_PC_READ_FAILED;
			goto end;
		}
		
		/* Reads chunk into memory from file1 */
		freesizet1 = fread(buffer, 1, bufferSize, file1);
		if (freesizet1 != bufferSize)
		{
			errorNumber = ERROR_PC_READ_FAILED;
			goto end;
		}
		file1_curpos += bufferSize;
		
		/* Moves to offset in file2 */
		freeint1 = fseek(file2, file2_curpos, SEEK_SET);
		if (freeint1 != 0)
		{
			errorNumber = ERROR_PSX_READ_FAILED;
			goto end;
		}
		
		/* Writes chunk to file2 */
		freesizet1 = fwrite(buffer, 1, bufferSize, file2);
		if (freesizet1 != bufferSize)
		{
			errorNumber = ERROR_PSX_WRITE_FAILED;
			goto end;
		}
		file2_curpos += bufferSize;
		
		/* Subtracts the size of the copied chunk from numBytes */
		numBytes -= (INTU32) bufferSize;
	}
	
end:/* Frees buffer and returns errorNumber */
	if (buffer != NULL)
	{
		free(buffer);
	}
	return errorNumber;
}

/*
 * Function that sets a series of bytes in a file to zero.
 * Parameters:
 * * file = Pointer to the file
 * * offset = Position in the file where to write the zeroes
 * * length = How many zeroes to write
 */
int zerobytes(FILE *file, INTU32 offset, INTU32 length)
{
	/* Variable Declarations */
	INTU8 *buffer = NULL; /* Buffer space for the zeroes */
	size_t bufferSize;    /* Size of the buffer */
	int freeint1;         /* Integer for general use */
	size_t freesizet1;    /* size_t for general use */
	int errorNumber;      /* Return value for this function */
	errorNumber = ERROR_NONE;
	
	/* Allocates a buffer for writing the zeroes */
	bufferSize = (size_t) 0;
	bufferSize = ~bufferSize;
	if ((INTU32) bufferSize > length)
	{
		bufferSize = (size_t) length;
	}
	do
	{
		buffer = calloc(bufferSize, 1);
		if (buffer == NULL)
		{
			bufferSize >>= 1;
			if (bufferSize == (size_t) 0)
			{
				errorNumber = ERROR_NONE;
				goto end;
			}
		}
	}
	while (buffer == NULL);
	
	/* Moves to offset */
	freeint1 = fseek(file, (long int) offset, SEEK_SET);
	if (freeint1 != 0)
	{
		errorNumber = ERROR_PSX_READ_FAILED;
		goto end;
	}
	
	/* Writes zeroes */
	while (length > 0)
	{
		/* Adjusts the size of the final chunk if needed */
		if ((INTU32) bufferSize > length)
		{
			bufferSize = (size_t) length;
		}
		
		/* Writes chunk to the file */
		freesizet1 = fwrite(buffer, 1, bufferSize, file);
		if (freesizet1 != bufferSize)
		{
			errorNumber = ERROR_PSX_WRITE_FAILED;
			goto end;
		}
		
		/* Moves to the next chunk */
		length -= bufferSize;
	}
	
end:
	if (buffer != NULL)
	{
		free(buffer);
	}
	return errorNumber;
}

/*
 * Function that sets a series of bytes in a file to a pattern.
 * Parameters:
 * * file = Pointer to the file
 * * offset = Position in the file where to write the zeroes
 * * length = How many zeroes to write
 * * pattern = Pointer to the pattern
 * * patternlen = Length of the pattern
 */
int patternbytes(FILE *file, INTU32 offset, INTU32 length,
                 INTU8 *pattern, INTU32 patternlen)
{
	/* Variable Declarations */
	INTU8 *buffer = NULL; /* Buffer space for the pattern */
	INTU32 freeuint321;   /* Unsigned 32-bit integer for general use */
	size_t bufferSize;    /* Size of the buffer */
	int freeint1;         /* Integer for general use */
	size_t freesizet1;    /* size_t for general use */
	int errorNumber;      /* Return value for this function */
	errorNumber = ERROR_NONE;
	
	/* Allocates a buffer for writing the pattern */
	bufferSize = (size_t) 0;
	bufferSize = ~bufferSize;
	if ((INTU32) bufferSize > length)
	{
		bufferSize = (size_t) length;
	}
	do
	{
		buffer = malloc(bufferSize);
		if (buffer == NULL)
		{
			bufferSize >>= 1;
			if (bufferSize == (size_t) 0)
			{
				errorNumber = ERROR_NONE;
				goto end;
			}
		}
	}
	while (buffer == NULL);
	
	/* Fills the buffer with the pattern */
	freeuint321 = 0x00000000;
	while (freeuint321 < (INTU32) bufferSize)
	{
		if (((INTU32) bufferSize - freeuint321) > patternlen)
		{
			freesizet1 = (size_t) patternlen;
		}
		else
		{
			freesizet1 = (size_t) (bufferSize - freeuint321);
		}
		memcpy(&buffer[freeuint321], pattern, freesizet1);
		freeuint321 += (INTU32) freesizet1;
	}
	
	/* Moves to offset */
	freeint1 = fseek(file, (long int) offset, SEEK_SET);
	if (freeint1 != 0)
	{
		errorNumber = ERROR_PSX_READ_FAILED;
		goto end;
	}
	
	/* Writes the pattern */
	while (length > 0)
	{
		/* Adjusts the size of the final chunk if needed */
		if ((INTU32) bufferSize > length)
		{
			bufferSize = (size_t) length;
		}
		
		/* Writes chunk to the file */
		freesizet1 = fwrite(buffer, 1, bufferSize, file);
		if (freesizet1 != bufferSize)
		{
			errorNumber = ERROR_PSX_WRITE_FAILED;
			goto end;
		}
		
		/* Moves to the next chunk */
		length -= bufferSize;
	}
	
end:
	if (buffer != NULL)
	{
		free(buffer);
	}
	return errorNumber;
}

/*
 * Adds a number to all offsets after a specific offset in a level_info struct.
 * Parameters:
 * * in = level_info struct
 * * offset = Specific offset where data was inserted
 * * amount = Amount of inserted bytes
 * Nota bene: This is to be used to update the internal structs relating to a
 * * level file, if a number of bytes has been inserted.
 */
void level_info_add(struct level_info *in, INTU32 offset, INTU32 amount)
{
	/* Variable Declarations */
	INTU16 curroom; /* Current room in processing offsets in rooms */
	
	/* Adjusts offset inside rooms */
	if ((in->p_NumFloorData) > offset)
	{
		for (curroom = (INTU16) 0x0000; curroom < (in->v_NumRooms); ++curroom)
		{
			if ((in->room[curroom].p_NumVertices) > offset)
			{
				in->room[curroom].p_NumVertices += amount;
			}
			if ((in->room[curroom].p_NumRectangles) > offset)
			{
				in->room[curroom].p_NumRectangles += amount;
			}
			if ((in->room[curroom].p_NumTriangles) > offset)
			{
				in->room[curroom].p_NumTriangles += amount;
			}
			if ((in->room[curroom].p_NumSprites) > offset)
			{
				in->room[curroom].p_NumSprites += amount;
			}
			if ((in->room[curroom].p_NumDoors) > offset)
			{
				in->room[curroom].p_NumDoors += amount;
			}
			if ((in->room[curroom].p_NumZSectors) > offset)
			{
				in->room[curroom].p_NumZSectors += amount;
			}
			if ((in->room[curroom].p_NumXSectors) > offset)
			{
				in->room[curroom].p_NumXSectors += amount;
			}
			if ((in->room[curroom].p_NumLights) > offset)
			{
				in->room[curroom].p_NumLights += amount;
			}
			if ((in->room[curroom].p_NumStaticMeshes) > offset)
			{
				in->room[curroom].p_NumStaticMeshes += amount;
			}
			if ((in->room[curroom].p_AlternateRoom) > offset)
			{
				in->room[curroom].p_AlternateRoom += amount;
			}
		}
	}
	
	/* Adjusts other offsets */
	if (in->p_NumTexTiles > offset)
	{
		in->p_NumTexTiles += amount;
	}
	if (in->p_NumPalettes > offset)
	{
		in->p_NumPalettes += amount;
	}
	if (in->p_NumRooms > offset)
	{
		in->p_NumRooms += amount;
	}
	if (in->p_NumFloorData > offset)
	{
		in->p_NumFloorData += amount;
	}
	if (in->p_OutRoomTableLen > offset)
	{
		in->p_OutRoomTableLen += amount;
	}
	if (in->p_NumRoomMeshBoxes > offset)
	{
		in->p_NumRoomMeshBoxes += amount;
	}
	if (in->p_NumMeshData > offset)
	{
		in->p_NumMeshData += amount;
	}
	if (in->p_NumMeshPointers > offset)
	{
		in->p_NumMeshPointers += amount;
	}
	if (in->p_NumAnimations > offset)
	{
		in->p_NumAnimations += amount;
	}
	if (in->p_NumStateChanges > offset)
	{
		in->p_NumStateChanges += amount;
	}
	if (in->p_NumAnimDispatches > offset)
	{
		in->p_NumAnimDispatches += amount;
	}
	if (in->p_NumAnimCommands > offset)
	{
		in->p_NumAnimCommands += amount;
	}
	if (in->p_NumMeshTrees > offset)
	{
		in->p_NumMeshTrees += amount;
	}
	if (in->p_NumFrames > offset)
	{
		in->p_NumFrames += amount;
	}
	if (in->p_NumMoveables > offset)
	{
		in->p_NumMoveables += amount;
	}
	if (in->p_NumStaticMeshes > offset)
	{
		in->p_NumStaticMeshes += amount;
	}
	if (in->p_NumObjectTextures > offset)
	{
		in->p_NumObjectTextures += amount;
	}
	if (in->p_NumSpriteTextures > offset)
	{
		in->p_NumSpriteTextures += amount;
	}
	if (in->p_NumSpriteSequences > offset)
	{
		in->p_NumSpriteSequences += amount;
	}
	if (in->p_NumCameras > offset)
	{
		in->p_NumCameras += amount;
	}
	if (in->p_NumSoundSources > offset)
	{
		in->p_NumSoundSources += amount;
	}
	if (in->p_NumBoxes > offset)
	{
		in->p_NumBoxes += amount;
	}
	if (in->p_NumOverlaps > offset)
	{
		in->p_NumOverlaps += amount;
	}
	if (in->p_Zones > offset)
	{
		in->p_Zones += amount;
	}
	if (in->p_NumAnimatedTextures > offset)
	{
		in->p_NumAnimatedTextures += amount;
	}
	if (in->p_NumEntities > offset)
	{
		in->p_NumEntities += amount;
	}
	if (in->p_NumRoomTextures > offset)
	{
		in->p_NumRoomTextures += amount;
	}
	if (in->p_NumCinematicFrames > offset)
	{
		in->p_NumCinematicFrames += amount;
	}
	if (in->p_NumDemoData > offset)
	{
		in->p_NumDemoData += amount;
	}
	if (in->p_NumSoundDetails > offset)
	{
		in->p_NumSoundDetails += amount;
	}
	if (in->p_NumSamples > offset)
	{
		in->p_NumSamples += amount;
	}
	if (in->p_NumSampleIndices > offset)
	{
		in->p_NumSampleIndices += amount;
	}
	if (in->p_FirstCodeModule > offset)
	{
		in->p_FirstCodeModule += amount;
	}
}

/*
 * Subtracts a number from all offsets after a specific offset in a level_info.
 * Parameters:
 * * in = level_info struct
 * * offset = Specific offset where data was removed
 * * amount = Amount of removed bytes
 * Nota bene: This is to be used to update the internal struct relating to a
 * * level file if a number of bytes was removed.
 */
void level_info_sub(struct level_info *in, INTU32 offset, INTU32 amount)
{
	/* Variable Declarations */
	INTU16 curroom; /* Current room in processing offsets in rooms */
	
	/* Adjusts offset inside rooms */
	if ((in->p_NumFloorData) > offset)
	{
		for (curroom = (INTU16) 0x0000; curroom < (in->v_NumRooms); ++curroom)
		{
			if ((in->room[curroom].p_NumVertices) > offset)
			{
				in->room[curroom].p_NumVertices -= amount;
			}
			if ((in->room[curroom].p_NumRectangles) > offset)
			{
				in->room[curroom].p_NumRectangles -= amount;
			}
			if ((in->room[curroom].p_NumTriangles) > offset)
			{
				in->room[curroom].p_NumTriangles -= amount;
			}
			if ((in->room[curroom].p_NumSprites) > offset)
			{
				in->room[curroom].p_NumSprites -= amount;
			}
			if ((in->room[curroom].p_NumDoors) > offset)
			{
				in->room[curroom].p_NumDoors -= amount;
			}
			if ((in->room[curroom].p_NumZSectors) > offset)
			{
				in->room[curroom].p_NumZSectors -= amount;
			}
			if ((in->room[curroom].p_NumXSectors) > offset)
			{
				in->room[curroom].p_NumXSectors -= amount;
			}
			if ((in->room[curroom].p_NumLights) > offset)
			{
				in->room[curroom].p_NumLights -= amount;
			}
			if ((in->room[curroom].p_NumStaticMeshes) > offset)
			{
				in->room[curroom].p_NumStaticMeshes -= amount;
			}
			if ((in->room[curroom].p_AlternateRoom) > offset)
			{
				in->room[curroom].p_AlternateRoom -= amount;
			}
		}
	}
	
	/* Adjusts other offsets */
	if (in->p_NumTexTiles > offset)
	{
		in->p_NumTexTiles -= amount;
	}
	if (in->p_NumPalettes > offset)
	{
		in->p_NumPalettes -= amount;
	}
	if (in->p_NumRooms > offset)
	{
		in->p_NumRooms -= amount;
	}
	if (in->p_NumFloorData > offset)
	{
		in->p_NumFloorData -= amount;
	}
	if (in->p_OutRoomTableLen > offset)
	{
		in->p_OutRoomTableLen -= amount;
	}
	if (in->p_NumRoomMeshBoxes > offset)
	{
		in->p_NumRoomMeshBoxes -= amount;
	}
	if (in->p_NumMeshData > offset)
	{
		in->p_NumMeshData -= amount;
	}
	if (in->p_NumMeshPointers > offset)
	{
		in->p_NumMeshPointers -= amount;
	}
	if (in->p_NumAnimations > offset)
	{
		in->p_NumAnimations -= amount;
	}
	if (in->p_NumStateChanges > offset)
	{
		in->p_NumStateChanges -= amount;
	}
	if (in->p_NumAnimDispatches > offset)
	{
		in->p_NumAnimDispatches -= amount;
	}
	if (in->p_NumAnimCommands > offset)
	{
		in->p_NumAnimCommands -= amount;
	}
	if (in->p_NumMeshTrees > offset)
	{
		in->p_NumMeshTrees -= amount;
	}
	if (in->p_NumFrames > offset)
	{
		in->p_NumFrames -= amount;
	}
	if (in->p_NumMoveables > offset)
	{
		in->p_NumMoveables -= amount;
	}
	if (in->p_NumStaticMeshes > offset)
	{
		in->p_NumStaticMeshes -= amount;
	}
	if (in->p_NumObjectTextures > offset)
	{
		in->p_NumObjectTextures -= amount;
	}
	if (in->p_NumSpriteTextures > offset)
	{
		in->p_NumSpriteTextures -= amount;
	}
	if (in->p_NumSpriteSequences > offset)
	{
		in->p_NumSpriteSequences -= amount;
	}
	if (in->p_NumCameras > offset)
	{
		in->p_NumCameras -= amount;
	}
	if (in->p_NumSoundSources > offset)
	{
		in->p_NumSoundSources -= amount;
	}
	if (in->p_NumBoxes > offset)
	{
		in->p_NumBoxes -= amount;
	}
	if (in->p_NumOverlaps > offset)
	{
		in->p_NumOverlaps -= amount;
	}
	if (in->p_Zones > offset)
	{
		in->p_Zones -= amount;
	}
	if (in->p_NumAnimatedTextures > offset)
	{
		in->p_NumAnimatedTextures -= amount;
	}
	if (in->p_NumEntities > offset)
	{
		in->p_NumEntities -= amount;
	}
	if (in->p_NumRoomTextures > offset)
	{
		in->p_NumRoomTextures -= amount;
	}
	if (in->p_NumCinematicFrames > offset)
	{
		in->p_NumCinematicFrames -= amount;
	}
	if (in->p_NumDemoData > offset)
	{
		in->p_NumDemoData -= amount;
	}
	if (in->p_NumSoundDetails > offset)
	{
		in->p_NumSoundDetails -= amount;
	}
	if (in->p_NumSamples > offset)
	{
		in->p_NumSamples -= amount;
	}
	if (in->p_NumSampleIndices > offset)
	{
		in->p_NumSampleIndices -= amount;
	}
	if (in->p_FirstCodeModule > offset)
	{
		in->p_FirstCodeModule -= amount;
	}
}

/*
 * Function that compares two strings of text (case insensitive).
 * Parameters:
 * * a = First string
 * * b = Second string
 * * length = Number of characters to compare
 * Return values:
 * * 0 = Strings are the same
 * * 1 = Strings are different
 * * 2 = String a was shorter than length
 * * 3 = String b was shorter than length
 * * 4 = String a was a NULL-pointer
 * * 5 = String b was a NULL-pointer
 */
int compareString(char *a, char *b, size_t length)
{
	/* Variable Declarations */
	size_t curchar; /* Current character */
	char a_curchar; /* Copy of current character in a */
	char b_curchar; /* Copy of current character in b */
	
	/* Checks that neither pointer is a NULL-pointer */
	if (a == NULL)
	{
		return 4;
	}
	if (b == NULL)
	{
		return 5;
	}
	
	/* Checks whether the length of both strings is sufficient */
	if (strlen(a) < length)
	{
		return 2;
	}
	if (strlen(b) < length)
	{
		return 3;
	}
	
	/* Compares strings */
	for (curchar = 0; curchar < length; ++curchar)
	{
		a_curchar = tolower(a[curchar]);
		b_curchar = tolower(b[curchar]);
		if (a_curchar != b_curchar)
		{
			return 1;
		}
	}
	
	return 0;
}
