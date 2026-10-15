/*
 * Library for custom I/O in TRMOD
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
#include <stdio.h> /* Standard I/O */

#ifdef TRMOD_IO_FOPEN_RAM
/* Variables for in-RAM processing */
static uint8 *trmod_io_level = NULL;
static long int trmod_io_curpos = 0l;
static long int trmod_io_levelsize = 0l;
static int trmod_io_level_changed = FALSE;
#endif

#ifdef TRMOD_IO_FOPEN_RAM
/*
 * Function that opens the level file and reads it into RAM
 * Parameters:
 * * path = Path to the level file
 * Returns values:
 * * NULL = Failed to read file into memory
 */
FILE *trmod_fopen_ram(char *path)
{
	/* Variable Declarations */
	FILE *infile = NULL;          /* File pointer for the level file */
	size_t readval = (size_t) 0U; /* Return value of fread() */
	int rseekval = 0;             /* Return value of fseek() */
	
	if (path == NULL)
	{
		return NULL;
	}
	
	/* Is a file was already in memory, it forgets it */
	if (trmod_io_level != NULL)
	{
		free(trmod_io_level);
		trmod_io_level = NULL;
		trmod_io_curpos = 0x00000000U;
		trmod_io_levelsize = 0x00000000U;
	}
	
	/* Opens the file */
	infile = fopen(path, "rb");
	if (infile == NULL)
	{
		return NULL;
	}
	
	/* Determines file size */
	seekval = fseek(infile, 0l, SEEK_END);
	if (seekval != 0)
	{
		return NULL;
	}
	trmod_io_levelsize = (uint32) ftell(infile);
	
	/* Allocates space for the in-memory file */
	trmod_io_level = malloc((size_t) trmod_io_levelsize);
	if (trmod_io_level == NULL)
	{
		return NULL;
	}
	
	/* Reads the file into memory */
	seekval = trmod_fseek(infile, 0l, SEEK_SET);
	if (seekval != 0)
	{
		return NULL;
	}
	readval = fread(trmod_io_level, (size_t) 1,
	                (size_t) trmod_io_levelsize, infile);
	if (readval != (size_t) trmod_io_levelsize)
	{
		free(trmod_io_level);
		trmod_io_level = NULL;
		trmod_io_curpos = 0x00000000U;
		trmod_io_levelsize = 0x00000000U;
		return NULL;
	}
	
	return (FILE *) trmod_io_level;
}
#endif

#ifdef TRMOD_IO_FCLOSE_RAM
/*
 * Function that removes the in-RAM copy of the level, losing all changes
 * Return values:
 * * 0 = Success
 */
int trmod_fclose_ram()
{
	free(trmod_io_level);
	trmod_io_level = NULL;
	trmod_io_curpos = 0x00000000U;
	trmod_io_levelsize = 0x00000000U;
	return 0;
}
#endif

#ifdef TRMOD_IO_FCLOSE_RAM_WRITE
/*
 * Function that writes the in-RAM level to the file (overwriting its contents)
 * Parameters:
 * * path = Path to the level file
 * Return values:
 * * 0 = Success
 * * 1 = Miserable failure
 */
int trmod_fclose_ram_write(char *path)
{
	/* Variable Declarations */
	FILE *outfile = NULL;          /* Pointer to the output file */
	int seekval = 0;               /* Return value of fseek() */
	size_t writeval = (size_t) 0U; /* Return value of fwrite() */
	int retval = 1;                /* Return value for this function */
	
	if (path == NULL)
	{
		goto end;
	}
	
	/* Only write the level if something was changed */
	if (trmod_io_level_changed == FALSE)
	{
		retval = 0;
		goto end;
	}
	
	/* Opens the output file */
	outfile = fopen(path, "wb");
	if (outfile == NULL)
	{
		goto end;
	}
	
	/* Writes the level file */
	seekval = trmod_fseek(outfile, 0l, SEEK_SET);
	if (seekval != 0)
	{
		goto end;
	}
	writeval = fwrite(trmod_io_level, (size_t) 1,
	                  (size_t) trmod_io_levelsize, outfile);
	if (writeval != (size_t) trmod_io_levelsize)
	{
		goto end;
	}
	
	/* Success */
	retval = 0;
	
end:/* Frees the in-RAM copy of the level file */
	if (outfile != NULL)
	{
		(void) fclose(outfile);
	}
	free(trmod_io_level);
	trmod_io_level = NULL;
	trmod_io_curpos = 0x00000000U;
	trmod_io_levelsize = 0x00000000U;
	return retval;
}
#endif

#ifdef TRMOD_IO_FSEEK_RAM
int trmod_fseek_ram(long offset, int orientation)
{
	switch (orientation)
	{
		case SEEK_SET:
			if (offset > trmod_io_levelsize)
			{
				return 1;
			}
			else
			{
				trmod_io_curpos = offset;
				return 0;
			}
			break;
		case SEEK_CUR:
			if ((trmod_io_curpos + offset) > trmod_io_levelsize)
			{
				return 1;
			}
			else
			{
				trmod_io_curpos += offset;
				return 0;
			}
			break;
		case SEEK_END:
			if (offset > trmod_io_levelsize)
			{
				return 1;
			}
			else
			{
				trmod_io_curpos = (trmod_io_levelsize - offset);
				return 0;
			}
			break;
	}
}
#endif

#ifdef TRMOD_IO_FTELL_RAM
long int trmod_ftell_ram(void)
{
	return trmod_io_curpos;
}
#endif
