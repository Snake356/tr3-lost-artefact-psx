/*
 * Library of OS-dependant functions used by TR2PCPSX. (POSIX-version)
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

/* File inclusions */
#include <stdio.h>  /* Standard I/O */
#include <unistd.h> /* POSIX operating system API (used for truncate) */
#include "osdep.h"  /* Library of OS-dependant functions */

/*
 * Function that truncates a file at a specific offset.
 * Parameters:
 * * filePath = Path to the file to be truncated
 * * offset = Offset where the file is to be truncated
 * Return values:
 * * 0 = Everything went well
 * * !0 = Failed to truncate file
 */
int truncateFile(char *filePath, INTU32 offset)
{
	return truncate(filePath, (off_t) offset);
}
