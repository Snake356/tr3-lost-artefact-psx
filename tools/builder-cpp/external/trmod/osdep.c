/*
 * Library of OS-dependant functions used by TRMOD
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

/* File inclusions */
#ifdef __DOS__
	#include <io.h>     /* DOS OS low-level I/O */
	#include <fcntl.h>  /* Used for flags in the open command */
#else
	#include <unistd.h> /* POSIX operating system API (used for truncate) */
#endif
#include "fixedint.h" /* Definition of fixed-size integers */

/*
 * Function that truncates a file at a specific offset.
 * Parameters:
 * * filePath = Path to the file to be truncated
 * * offset = Offset where the file is to be truncated
 * Return values:
 * * 0 = Everything went well
 * * !0 = Failed to truncate file
 */
int truncateFile(char *filePath, uint32 offset)
{
#ifdef __DOS__
	/* Variable Declarations */
	int handle = 0;    /* Handle for opening the file */
	int chsizeval = 0; /* Return value for chsize() */
	int closeval = 0;  /* Return value for close() */
	int retval = 0;    /* Return value for this function */
	
	/* Opens the file (low-level) */
	handle = open(filePath, O_RDWR);
	if (handle < 0)
	{
		return 6;
	}
	
	/* Truncates the file */
	chsizeval = chsize(handle, (long int) offset);
	if (chsizeval != 0)
	{
		retval = 8;
	}
	
	/* Closes the file */
	closeval = close(handle);
	if ((closeval != 0) && (retval == 0))
	{
		retval = 9;
	}
	
	/* Returns the function */
	return retval;
#else
	return truncate(filePath, (off_t) offset);
#endif
}
