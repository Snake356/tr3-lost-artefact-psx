/*
 * Library that contains general functions used by TR2PCPSX.
 * Copyright (C) 2019 b122251
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
#include "structs.h"  /* Definition of structs used by TR2PCPSX */
#include "osdep.h"    /* Library of OS-dependant functions in TR2PCPSX */

/*
 * Function that inserts bytes into the level.
 * Parameters:
 * * level = File pointer to level file
 * * offset = Offset where the bytes are to be inserted
 * * length = Number of bytes to be inserted
 * Return values:
 * * 0 = Everything went well
 * * 7 = PSX-file could not be read from
 * * 8 = PSX-file could not be written to
 * * 10 = Memory could not be allocated
 */
int insertbytes(FILE *level, INTU32 offset, INTU32 length)
{
	/* Variable Declarations */
	void *buffer = NULL;  /* Buffer space */
	size_t bufferSize;    /* Length of the buffer space */
	INTU32 nummovebytes;  /* Number of bytes to be moved */
	INTU32 posmovebytes;  /* Position of bytes to be moved */
	INTU32 destmovebytes; /* Destination of bytes to be moved */
	int errorNumber = 0;  /* Return value for this function */
	size_t freesizet1;    /* size_t for general use */
	int freeint1;         /* Integer for general use */
	INTU32 freeuint321;   /* Unsigned 32-bit integer for general use */
	INTU32 levelSize;     /* Size of the level file in bytes */
	
	/* Determines levelSize */
	freeint1 = fseek(level, 0, SEEK_END);
	if (freeint1 != 0)
	{
		return 7;
	}
	levelSize = (INTU32) ftell(level);
	
	if (offset > levelSize)
	{
		/* Fills up empty space with zeroes if offset is outside of level */
		freeint1 = fseek(level, 0, SEEK_END);
		if (freeint1 != 0)
		{
			errorNumber = 8;
			goto end;
		}
		for (freeuint321 = ((offset + length) - levelSize); freeuint321 > (INTU32)0;
		     --freeuint321)
		{
			freeint1 = fputc(0, level);
			if (freeint1 != 0)
			{
				errorNumber = 8;
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
			bufferSize = ((size_t)(nummovebytes));
			if ((sizeof(bufferSize) == 2) && ((nummovebytes & 0xFFFF0000) != (INTU32)0))
			{
				memset(&bufferSize, 255, 2);
			}
			do
			{
				buffer = malloc(bufferSize);
				if (buffer == NULL)
				{
					bufferSize >>= 1;
					if (bufferSize <= 1)
					{
						errorNumber = 10;
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
					errorNumber = 7;
					goto end;
				}
				freeuint321 = (INTU32)(length - (INTU32)bufferSize);
				for (; freeuint321 > (INTU32)0; --freeuint321)
				{
					freeint1 = fputc(0, level);
					if (freeint1 != 0)
					{
						errorNumber = 8;
						goto end;
					}
				}
			}
			
			/* Moves data in chunks */
			posmovebytes = (levelSize);
			destmovebytes = (posmovebytes + length);
			while (nummovebytes > (INTU32)0)
			{
				/* Adjusts size of final chunk if needed */
				if (nummovebytes < ((INTU32)bufferSize))
				{
					bufferSize = (size_t) nummovebytes;
				}
				
				/* Corrects source and destination positions */
				posmovebytes -= (INTU32)bufferSize;
				destmovebytes -= (INTU32)bufferSize;
				
				/* Goes the the offset to read from */
				freeint1 = fseek(level, (long int)posmovebytes, SEEK_SET);
				if (freeint1 != 0)
				{
					errorNumber = 7;
					goto end;
				}
				
				/* Reads chunk into memory */
				freesizet1 = fread(buffer, 1, bufferSize, level);
				if (freesizet1 != bufferSize)
				{
					errorNumber = 7;
					goto end;
				}
				
				/* Goes to the offset to write to */
				freeint1 = fseek(level, (long int)destmovebytes, SEEK_SET);
				if (freeint1 != 0)
				{
					errorNumber = 7;
					goto end;
				}
				
				/* Writes chunk to the file */
				freesizet1 = fwrite(buffer, 1, bufferSize, level);
				if (freesizet1 != bufferSize)
				{
					errorNumber = 8;
					goto end;
				}
				
				/* Iterates counting variable */
				nummovebytes -= (INTU32)bufferSize;
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
 * Return values:
 * * 0 = Everything went well
 * * 7 = PSX-file could not be read from
 * * 8 = PSX-file could not be written to
 * * 10 = Buffer could not be allocated
 * * 11 = Data to be removed doesn't exist (removebytes)
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
		return 7;
	}
	levelSize = (INTU32) ftell(level);
	
	/* Checks whether offset and length are valid */
	if ((offset + length) > levelSize)
	{
		errorNumber = 11;
		goto end;
	}
	
	/* Calculates nummovebytes and posmovebytes */
	posmovebytes = (offset + length);
	nummovebytes = (levelSize - posmovebytes);
	
	if (nummovebytes != (INTU32)0)
	{
		/* Allocates buffer */
		bufferSize = ((size_t)(nummovebytes));
		if ((sizeof(bufferSize) == 2) && ((nummovebytes & 0xFFFF0000) != (INTU32)0))
		{
			memset(&bufferSize, 255, 2);
		}
		do
		{
			buffer = malloc(bufferSize);
			if (buffer == NULL)
			{
				bufferSize >>= 1;
				if (bufferSize <= 1)
				{
					errorNumber = 10;
					goto end;
				}
			}
		}
		while (buffer == NULL);
		
		/* Moves data in chunks */
		freeuint321 = offset;
		while (nummovebytes > (INTU32)0)
		{
			/* Adjusts size of final chunk if needed */
			if (nummovebytes < ((INTU32)bufferSize))
			{
				bufferSize = (size_t) nummovebytes;
			}
			
			/* Goes the the offset to read from */
			freeint1 = fseek(level, (long int)posmovebytes, SEEK_SET);
			if (freeint1 != 0)
			{
				errorNumber = 7;
				goto end;
			}
			
			/* Reads chunk into memory */
			freesizet1 = fread(buffer, 1, bufferSize, level);
			if (freesizet1 != bufferSize)
			{
				errorNumber = 7;
				goto end;
			}
			
			/* Goes to the offset to write to */
			freeint1 = fseek(level, (long int)freeuint321, SEEK_SET);
			if (freeint1 != 0)
			{
				errorNumber = 7;
				goto end;
			}
			
			/* Writes chunk to the file */
			freesizet1 = fwrite(buffer, 1, bufferSize, level);
			if (freesizet1 != bufferSize)
			{
				errorNumber = 8;
				goto end;
			}
			
			/* Iterates counting variables */
			posmovebytes += (INTU32)bufferSize;
			nummovebytes -= (INTU32)bufferSize;
			freeuint321 += (INTU32)bufferSize;
		}
	}
	
	/* Truncates the file */
	freeint1 = truncateFile(levelPath, (levelSize - length));
	if (freeint1 != 0)
	{
		errorNumber = 8;
		goto end;
	}
	
end:/* Frees buffer and closes function */
	if (buffer != NULL)
	{
		free(buffer);
	}
	return errorNumber;
}

INT32 strtoint(char *string)
{
	INTU32 outvar = 0x00000000;
	int negative = 0;
	
	if (*string == '-')
	{
		negative = 1;
		++string;
	}
	
	while (0 == 0)
	{
		if ((*string >= '0') && (*string <= '9'))
		{
			outvar *= 10U;
			outvar += (*string - '0');
		}
		else
		{
			break;
		}
		++string;
	}
	
	if (negative == 1)
	{
		outvar = (0x00000000 - outvar);
	}
	
	return outvar;
}
