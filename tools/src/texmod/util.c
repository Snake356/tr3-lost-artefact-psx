/*
 * Library that contains general functions used by TRMOD
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
#include <stdio.h>    /* Standard input and output */
#include <stdlib.h>   /* Standard library of utility functions */
#include <string.h>   /* Functions relating to strings */
#include <ctype.h>    /* Definition of char-type (used for tolower) */
#include "fixedint.h" /* Definition of fixed-size integers */
#include "structs.h"  /* Definition of structs used by TRMOD */
#include "osdep.h"    /* Library of OS-dependant functions in TRMOD */
#include "errors.h"   /* Error Handling for TRMOD */

/*
 * Function that inserts bytes into a file
 * Parameters:
 * * file = File pointer to file
 * * path = Path to the file
 * * offset = Offset where the bytes are to be inserted
 * * length = Number of bytes to be inserted
 * * error = Pointer to the error struct
 */
void insertbytes(FILE *file, char *path, INTU32 offset, INTU32 length,
                 struct error *error)
{
	/* Variable Declarations */
	void *buffer = NULL;  /* Buffer space */
	size_t bufferSize;    /* Length of the buffer space */
	INTU32 nummovebytes;  /* Number of bytes to be moved */
	INTU32 posmovebytes;  /* Position of bytes to be moved */
	INTU32 destmovebytes; /* Destination of bytes to be moved */
	size_t freesizet1;    /* size_t for general use */
	int freeint1;         /* Integer for general use */
	INTU32 freeuint321;   /* Unsigned 32-bit integer for general use */
	INTU32 fileSize;      /* Size of the file in bytes */
	
	/* Determines fileSize */
	freeint1 = fseek(file, 0, SEEK_END);
	if (freeint1 != 0)
	{
		error->code = ERROR_FILE_READ_FAILED;
		error->string[0] = path;
		return;
	}
	fileSize = (INTU32) ftell(file);
	
	if (offset > fileSize)
	{
		/* Fills up empty space with zeroes if offset is outside of file */
		freeint1 = fseek(file, 0, SEEK_END);
		if (freeint1 != 0)
		{
			error->code = ERROR_FILE_READ_FAILED;
			error->string[0] = path;
			goto end;
		}
		for (freeuint321 = ((offset + length) - fileSize);
		     freeuint321 > (INTU32) 0; --freeuint321)
		{
			freeint1 = fputc(0, file);
			if (freeint1 != 0)
			{
				error->code = ERROR_FILE_WRITE_FAILED;
				error->string[0] = path;
				goto end;
			}
		}
	}
	else
	{
		/* Calculates nummovebytes */
		nummovebytes = (fileSize - offset);
		
		if (nummovebytes != 0x00000000)
		{
			/* Allocates buffer */
			bufferSize = ((size_t) nummovebytes);
			if ((sizeof(bufferSize) == 2) &&
			    ((nummovebytes & 0xFFFF0000) != (INTU32) 0x00000000))
			{
				(void) memset(&bufferSize, 255, 2);
			}
			do
			{
				buffer = malloc(bufferSize);
				if (buffer == NULL)
				{
					bufferSize >>= 1U;
					if (bufferSize <= 1)
					{
						error->code = ERROR_MEMORY;
						goto end;
					}
				}
			}
			while (buffer == NULL);
			
			/* Adjusts for a very small buffer if needed */
			if (((INTU32) bufferSize) < length)
			{
				freeint1 = fseek(file, 0, SEEK_END);
				if (freeint1 != 0)
				{
					error->code = ERROR_FILE_READ_FAILED;
					error->string[0] = path;
					goto end;
				}
				freeuint321 = (INTU32) (length - (INTU32) bufferSize);
				for (; freeuint321 > (INTU32) 0; --freeuint321)
				{
					freeint1 = fputc(0, file);
					if (freeint1 != 0)
					{
						error->code = ERROR_FILE_WRITE_FAILED;
						error->string[0] = path;
						goto end;
					}
				}
			}
			
			/* Moves data in chunks */
			posmovebytes = fileSize;
			destmovebytes = (posmovebytes + length);
			while (nummovebytes > 0x00000000)
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
				freeint1 = fseek(file, (long int) posmovebytes, SEEK_SET);
				if (freeint1 != 0)
				{
					error->code = ERROR_FILE_READ_FAILED;
					error->string[0] = path;
					goto end;
				}
				
				/* Reads chunk into memory */
				freesizet1 = fread(buffer, 1, bufferSize, file);
				if (freesizet1 != bufferSize)
				{
					error->code = ERROR_FILE_READ_FAILED;
					error->string[0] = path;
					goto end;
				}
				
				/* Goes to the offset to write to */
				freeint1 = fseek(file, (long int) destmovebytes, SEEK_SET);
				if (freeint1 != 0)
				{
					error->code = ERROR_FILE_READ_FAILED;
					error->string[0] = path;
					goto end;
				}
				
				/* Writes chunk to the file */
				freesizet1 = fwrite(buffer, 1, bufferSize, file);
				if (freesizet1 != bufferSize)
				{
					error->code = ERROR_FILE_WRITE_FAILED;
					error->string[0] = path;
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
}

/*
 * Function that removes bytes from a file
 * Parameters:
 * * file = Pointer to the file
 * * path = Path to the file
 * * offset = Offset the bytes to be removed are at
 * * length = Number of bytes to be removed
 * * error = Pointer to the error struct
 */
void removebytes(FILE *file, char *path, INTU32 offset, INTU32 length,
                 struct error *error)
{
	/* Variable Declarations */
	void *buffer = NULL; /* Buffer space */
	size_t bufferSize;   /* Length of the buffer space */
	INTU32 posmovebytes; /* Position of bytes to be moved */
	INTU32 nummovebytes; /* Number of bytes to be moved */
	int freeint1;        /* Integer for general use */
	INTU32 freeuint321;  /* Unsigned 32-bit integer for general use */
	size_t freesizet1;   /* size_t for general use */
	INTU32 fileSize;     /* Size of the file in bytes */
	
	/* Determines fileSize */
	freeint1 = fseek(file, 0, SEEK_END);
	if (freeint1 != 0)
	{
		error->code = ERROR_FILE_READ_FAILED;
		error->string[0] = path;
		return;
	}
	fileSize = (INTU32) ftell(file);
	
	/* Checks whether offset and length are valid */
	if ((offset + length) > fileSize)
	{
		error->code = ERROR_REMOVE_DATA_INVALID;
		return;
	}
	
	/* Calculates nummovebytes and posmovebytes */
	posmovebytes = (offset + length);
	nummovebytes = (fileSize - posmovebytes);
	
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
				bufferSize >>= 1U;
				if (bufferSize <= 1)
				{
					error->code = ERROR_MEMORY;
					goto end;
				}
			}
		}
		while (buffer == NULL);
		
		/* Moves data in chunks */
		freeuint321 = offset;
		while (nummovebytes > 0x00000000)
		{
			/* Adjusts size of final chunk if needed */
			if (nummovebytes < ((INTU32) bufferSize))
			{
				bufferSize = (size_t) nummovebytes;
			}
			
			/* Goes the the offset to read from */
			freeint1 = fseek(file, (long int) posmovebytes, SEEK_SET);
			if (freeint1 != 0)
			{
				error->code = ERROR_FILE_READ_FAILED;
				error->string[0] = path;
				goto end;
			}
			
			/* Reads chunk into memory */
			freesizet1 = fread(buffer, 1, bufferSize, file);
			if (freesizet1 != bufferSize)
			{
				error->code = ERROR_FILE_READ_FAILED;
				error->string[0] = path;
				goto end;
			}
			
			/* Goes to the offset to write to */
			freeint1 = fseek(file, (long int) freeuint321, SEEK_SET);
			if (freeint1 != 0)
			{
				error->code = ERROR_FILE_READ_FAILED;
				error->string[0] = path;
				goto end;
			}
			
			/* Writes chunk to the file */
			freesizet1 = fwrite(buffer, 1, bufferSize, file);
			if (freesizet1 != bufferSize)
			{
				error->code = ERROR_FILE_WRITE_FAILED;
				error->string[0] = path;
				goto end;
			}
			
			/* Iterates counting variables */
			posmovebytes += (INTU32) bufferSize;
			nummovebytes -= (INTU32) bufferSize;
			freeuint321 += (INTU32) bufferSize;
		}
	}
	
	/* Truncates the file */
	freeint1 = truncateFile(path, (fileSize - length));
	if (freeint1 != 0)
	{
		error->code = ERROR_FILE_WRITE_FAILED;
		error->string[0] = path;
		goto end;
	}
	
end:/* Frees buffer and closes function */
	if (buffer != NULL)
	{
		free(buffer);
	}
}

/*
 * Function that copies a certain number of bytes from file1 to file2
 * Parameters:
 * * file1 = Source file to be read from
 * * file2 = Target file to be written to
 * * file1path = Path to file1
 * * file2path = Path to file2
 * * f1_offset = Offset in file1 to read from
 * * f2_offset = Offset in file2 to write to
 * * numBytes = Number of bytes to be copied
 * * error = Pointer to error struct
 */
void copybytes(FILE *file1, FILE *file2, char *file1path, char *file2path,
               INTU32 f1_offset, INTU32 f2_offset, INTU32 numBytes,
               struct error *error)
{
	/* Variable Declarations */
	void *buffer = NULL;   /* Memory space to use while copying */
	size_t bufferSize;     /* Size of the buffer */
	int freeint1;          /* Integer for general use */
	size_t freesizet1;     /* size_t for general use */
	long int file1_curpos; /* Current position in file1 */
	long int file2_curpos; /* Current position in file2 */
	
	/* Checks whether to copy any bytes at all */
	if (numBytes == 0x00000000)
	{
		return;
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
			bufferSize >>= 1U;
			if (bufferSize <= 1)
			{
				error->code = ERROR_MEMORY;
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
			error->code = ERROR_FILE_READ_FAILED;
			error->string[0] = file1path;
			goto end;
		}
		
		/* Reads chunk into memory from file1 */
		freesizet1 = fread(buffer, 1, bufferSize, file1);
		if (freesizet1 != bufferSize)
		{
			error->code = ERROR_FILE_READ_FAILED;
			error->string[0] = file1path;
			goto end;
		}
		file1_curpos += bufferSize;
		
		/* Moves to offset in file2 */
		freeint1 = fseek(file2, file2_curpos, SEEK_SET);
		if (freeint1 != 0)
		{
			error->code = ERROR_FILE_READ_FAILED;
			error->string[0] = file2path;
			goto end;
		}
		
		/* Writes chunk to file2 */
		freesizet1 = fwrite(buffer, 1, bufferSize, file2);
		if (freesizet1 != bufferSize)
		{
			error->code = ERROR_FILE_WRITE_FAILED;
			error->string[0] = file2path;
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
}

/*
 * Function that sets a series of bytes in a file to zero
 * Parameters:
 * * file = Pointer to the file
 * * path = Path to the file
 * * offset = Position in the file where to write the zeroes
 * * length = How many zeroes to write
 * * error = Pointer to the error struct
 */
void zerobytes(FILE *file, char *path, INTU32 offset, INTU32 length,
               struct error *error)
{
	/* Variable Declarations */
	INTU8 *buffer = NULL; /* Buffer space for the zeroes */
	size_t bufferSize;    /* Size of the buffer */
	int freeint1;         /* Integer for general use */
	size_t freesizet1;    /* size_t for general use */
	
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
			bufferSize >>= 1U;
			if (bufferSize == (size_t) 0)
			{
				error->code = ERROR_MEMORY;
				goto end;
			}
		}
	}
	while (buffer == NULL);
	
	/* Moves to offset */
	freeint1 = fseek(file, (long int) offset, SEEK_SET);
	if (freeint1 != 0)
	{
		error->code = ERROR_FILE_READ_FAILED;
		error->string[0] = path;
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
			error->code = ERROR_FILE_WRITE_FAILED;
			error->string[0] = path;
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
}

/*
 * Function that sets a series of bytes in a file to a pattern
 * Parameters:
 * * file = Pointer to the file
 * * path = Path to the file
 * * offset = Position in the file where to write the zeroes
 * * length = How many zeroes to write
 * * pattern = Pointer to the pattern
 * * patternlen = Length of the pattern
 * * error = Pointer to the error struct
 */
void patternbytes(FILE *file, char *path, INTU32 offset, INTU32 length,
                  INTU8 *pattern, INTU32 patternlen, struct error *error)
{
	/* Variable Declarations */
	INTU8 *buffer = NULL; /* Buffer space for the pattern */
	INTU32 freeuint321;   /* Unsigned 32-bit integer for general use */
	size_t bufferSize;    /* Size of the buffer */
	int freeint1;         /* Integer for general use */
	size_t freesizet1;    /* size_t for general use */
	
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
				error->code = ERROR_NONE;
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
		error->code = ERROR_FILE_READ_FAILED;
		error->string[0] = path;
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
			error->code = ERROR_FILE_WRITE_FAILED;
			error->string[0] = path;
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
}

/*
 * Adds a number to all offsets after a specific offset in a level struct
 * Parameters:
 * * in = level struct
 * * offset = Specific offset where data was inserted
 * * amount = Amount of inserted bytes
 * Nota bene: This is to be used to update the internal structs relating to a
 * * level file, if a number of bytes has been inserted.
 */
void level_struct_add(struct level *in, INTU32 offset, INTU32 amount)
{
	/* Variable Declarations */
	INTU16 curroom; /* Current room in processing offsets in rooms */
	
	/* Adjusts offset inside rooms */
	if ((in->p_NumFloorData) >= offset)
	{
		for (curroom = (INTU16) 0x0000; curroom < (in->v_NumRooms); ++curroom)
		{
			if ((in->room[curroom].p_NumVertices) >= offset)
			{
				in->room[curroom].p_NumVertices += amount;
			}
			if ((in->room[curroom].p_NumRectangles) >= offset)
			{
				in->room[curroom].p_NumRectangles += amount;
			}
			if ((in->room[curroom].p_NumTriangles) >= offset)
			{
				in->room[curroom].p_NumTriangles += amount;
			}
			if ((in->room[curroom].p_NumSprites) >= offset)
			{
				in->room[curroom].p_NumSprites += amount;
			}
			if ((in->room[curroom].p_NumDoors) >= offset)
			{
				in->room[curroom].p_NumDoors += amount;
			}
			if ((in->room[curroom].p_NumZSectors) >= offset)
			{
				in->room[curroom].p_NumZSectors += amount;
			}
			if ((in->room[curroom].p_NumXSectors) >= offset)
			{
				in->room[curroom].p_NumXSectors += amount;
			}
			if ((in->room[curroom].p_NumLights) >= offset)
			{
				in->room[curroom].p_NumLights += amount;
			}
			if ((in->room[curroom].p_NumStaticMeshes) >= offset)
			{
				in->room[curroom].p_NumStaticMeshes += amount;
			}
			if ((in->room[curroom].p_AlternateRoom) >= offset)
			{
				in->room[curroom].p_AlternateRoom += amount;
			}
		}
	}
	
	/* Adjusts other offsets */
	if (in->p_NumTexTiles >= offset)
	{
		in->p_NumTexTiles += amount;
	}
	if (in->p_NumPalettes >= offset)
	{
		in->p_NumPalettes += amount;
	}
	if (in->p_NumRooms >= offset)
	{
		in->p_NumRooms += amount;
	}
	if (in->p_NumFloorData >= offset)
	{
		in->p_NumFloorData += amount;
	}
	if (in->p_OutRoomTableLen >= offset)
	{
		in->p_OutRoomTableLen += amount;
	}
	if (in->p_NumRoomMeshBoxes >= offset)
	{
		in->p_NumRoomMeshBoxes += amount;
	}
	if (in->p_NumMeshData >= offset)
	{
		in->p_NumMeshData += amount;
	}
	if (in->p_NumMeshPointers >= offset)
	{
		in->p_NumMeshPointers += amount;
	}
	if (in->p_NumAnimations >= offset)
	{
		in->p_NumAnimations += amount;
	}
	if (in->p_NumStateChanges >= offset)
	{
		in->p_NumStateChanges += amount;
	}
	if (in->p_NumAnimDispatches >= offset)
	{
		in->p_NumAnimDispatches += amount;
	}
	if (in->p_NumAnimCommands >= offset)
	{
		in->p_NumAnimCommands += amount;
	}
	if (in->p_NumMeshTrees >= offset)
	{
		in->p_NumMeshTrees += amount;
	}
	if (in->p_NumFrames >= offset)
	{
		in->p_NumFrames += amount;
	}
	if (in->p_NumMoveables >= offset)
	{
		in->p_NumMoveables += amount;
	}
	if (in->p_NumStaticMeshes >= offset)
	{
		in->p_NumStaticMeshes += amount;
	}
	if (in->p_NumObjectTextures >= offset)
	{
		in->p_NumObjectTextures += amount;
	}
	if (in->p_NumSpriteTextures >= offset)
	{
		in->p_NumSpriteTextures += amount;
	}
	if (in->p_NumSpriteSequences >= offset)
	{
		in->p_NumSpriteSequences += amount;
	}
	if (in->p_NumCameras >= offset)
	{
		in->p_NumCameras += amount;
	}
	if (in->p_NumSoundSources >= offset)
	{
		in->p_NumSoundSources += amount;
	}
	if (in->p_NumBoxes >= offset)
	{
		in->p_NumBoxes += amount;
	}
	if (in->p_NumOverlaps >= offset)
	{
		in->p_NumOverlaps += amount;
	}
	if (in->p_Zones >= offset)
	{
		in->p_Zones += amount;
	}
	if (in->p_NumAnimatedTextures >= offset)
	{
		in->p_NumAnimatedTextures += amount;
	}
	if (in->p_NumEntities >= offset)
	{
		in->p_NumEntities += amount;
	}
	if (in->p_NumRoomTextures >= offset)
	{
		in->p_NumRoomTextures += amount;
	}
	if (in->p_NumCinematicFrames >= offset)
	{
		in->p_NumCinematicFrames += amount;
	}
	if (in->p_NumDemoData >= offset)
	{
		in->p_NumDemoData += amount;
	}
	if (in->p_NumSoundDetails >= offset)
	{
		in->p_NumSoundDetails += amount;
	}
	if (in->p_NumSamples >= offset)
	{
		in->p_NumSamples += amount;
	}
	if (in->p_NumSampleIndices >= offset)
	{
		in->p_NumSampleIndices += amount;
	}
	if (in->p_FirstCodeModule >= offset)
	{
		in->p_FirstCodeModule += amount;
	}
}

/*
 * Subtracts a number from all offsets after a specific offset in a level struct
 * Parameters:
 * * in = level struct
 * * offset = Specific offset where data was removed
 * * amount = Amount of removed bytes
 * Nota bene: This is to be used to update the internal struct relating to a
 * * level file if a number of bytes was removed.
 */
void level_struct_sub(struct level *in, INTU32 offset, INTU32 amount)
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
 * Function that compares two strings of text (case insensitive)
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

/*
 * Function that splits a string based on a separator,
 * possibly keeping track of bracket depth
 * Parameters:
 * * in = String to be split
 * * out = Array of char pointers where to put the output strings
 * * len = Length of the input string
 * * sep = Character on which to split
 * * dep = Keep track of depth (0 = No, 1 = Yes)
 * * error = Pointer to the error struct
 * Return values:
 * * 0x0000 = Some sort of error happened
 * *     >0 = Number of returned strings
 * Nota bene: NULL-terminators are added in-place in input
 */
INTU16 splitString(char *in, char ***out, INTU16 len,
                   char sep, int dep, struct error *error)
{
	/* Variable Declarations */
	INTU16 curchar; /* Current character in in */
	INTU16 numseps; /* Number of separators found in the input */
	INTU16 curstr;  /* Current string in the output */
	int depth = 0;  /* Bracket depth */
	
	/* Checks that in is not NULL */
	if (in == NULL)
	{
		return 0x0000;
	}
	
	/* Ends if the input string is invalid */
	if ((len > (INTU16) strlen(in)) || (strlen(in) == (size_t) 0))
	{
		return 0x0000;
	}
	
	/* Frees out if it isn't NULL */
	if (*out != NULL)
	{
		free(*out);
		*out = NULL;
	}
	
	/* Counts the number of separators */
	numseps = 0x0001;
	for (curchar = 0x0000; curchar < len; ++curchar)
	{
		/* Keeps track of depth if needed */
		if (dep == 1)
		{
			if ((in[curchar] == '(') || (in[curchar] == '[') ||
			    (in[curchar] == '{'))
			{
				++depth;
			}
			else if ((in[curchar] == ')') || (in[curchar] == ']') ||
			         (in[curchar] == '}'))
			{
				--depth;
			}
		}
		
		if ((in[curchar] == sep) && (depth == 0))
		{
			++numseps;
		}
	}
	
	/* Allocates the output pointers */
	*out = calloc((size_t) numseps, sizeof(char *));
	if (*out == NULL)
	{
		error->code = ERROR_MEMORY;
		return 0x0000;
	}
	
	/* Splits the string */
	depth = 0;
	(*out)[0] = in;
	curstr = 0x0001;
	for (curchar = 0x0000; curchar < len; ++curchar)
	{
		/* Keeps track of depth if needed */
		if (dep == 1)
		{
			if ((in[curchar] == '(') || (in[curchar] == '[') ||
			    (in[curchar] == '{'))
			{
				++depth;
			}
			else if ((in[curchar] == ')') || (in[curchar] == ']') ||
			         (in[curchar] == '}'))
			{
				--depth;
			}
		}
		if ((in[curchar] == sep) && (depth == 0))
		{
			in[curchar] = '\0';
			if (curchar < (len - 0x0001))
			{
				(*out)[curstr++] = &in[(curchar + 0x0001)];
			}
		}
	}
	
	/* Returns the number of returned strings */
	return curstr;
}

/*
 * Function that returns the index of a character in a string
 * Parameters:
 * * in = Input string to look through
 * * len = Length of the input string
 * * c = Character to look for
 * * cnt = Which occurence of c to be selected (>0 = Forwards, <0 = Backwads)
 * Return value is the offset of the character, of 0xFFFF if not found.
 */
INTU16 charindex(char *in, INTU16 len, char c, int cnt)
{
	/* Variable Declaratons */
	INTU16 curchar;
	
	if (cnt > 0)
	{
		for (curchar = 0x0000; curchar < len; ++curchar)
		{
			if (in[curchar] == c)
			{
				--cnt;
				if (cnt == 0)
				{
					return curchar;
				}
			}
		}
	}
	else if (cnt < 0)
	{
		for (curchar = (len - 0x0001); curchar >= 0x0000; --curchar)
		{
			if (in[curchar] == c)
			{
				++cnt;
				if (cnt == 0)
				{
					return curchar;
				}
			}
			if (curchar == 0x0000)
			{
				break;
			}
		}
	}
	
	return 0xFFFF;
}

/*
 * Function that replaces a character in a string
 * Parameters:
 * * string = String to replace characters in
 * * length = Length of the string
 * * o = Character to replace
 * * n = Character to change it to
 */
void repchar(char *string, size_t length, char o, char n)
{
	/* Variable Declaration */
	size_t curpos; /* Current position in the string */

	/* Replaces the characters as needed */
	for (curpos = (size_t) 0; curpos < length; ++curpos)
	{
		if (string[curpos] == o)
		{
			string[curpos] = n;
		}
	}
}
