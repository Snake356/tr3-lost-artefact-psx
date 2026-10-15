/*
 * Binary Data to IPS-patch Converter
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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Integer Definitions */
typedef signed char        int8;
typedef signed short int   int16;
typedef signed int         int32;
typedef unsigned char      uint8;
typedef unsigned short int uint16;
typedef unsigned int       uint32;

enum errorCodes
{
	ERROR_NONE,
#ifndef DONT_CHECK_FIXEDINTS
	ERROR_INT8_NOT_8_BIT,
	ERROR_INT16_NOT_16_BIT,
	ERROR_INT32_NOT_32_BIT,
	ERROR_UINT8_NOT_8_BIT,
	ERROR_UINT16_NOT_16_BIT,
	ERROR_UINT32_NOT_32_BIT,
#endif
	ERROR_INSUFFICIENT_PARAMS,
	ERROR_PATFILE_OPEN_FAILED,
	ERROR_BINFILE_OPEN_FAILED,
	ERROR_IPSFILE_OPEN_FAILED,
	ERROR_PATFILE_READ_FAILED,
	ERROR_BINFILE_READ_FAILED,
	ERROR_IPSFILE_WRITE_FAILED,
	ERROR_PATFILE_CLOSE_FAILED,
	ERROR_BINFILE_CLOSE_FAILED,
	ERROR_IPSFILE_CLOSE_FAILED,
	ERROR_MEMORY,
	ERROR_OFFSET_TOO_LARGE,
	ERROR_LENGTH_TOO_LARGE,
	ERROR_OFFSET_OUT_OF_BIN
};

enum parameters
{
	PARAM_PROG = 0,
	PARAM_PAT,
	PARAM_BIN,
	PARAM_IPS,
	PARAM_COUNT
};

enum patentry
{
	PE_SP = 0, /* Starting position */
	PE_LEN,    /* Length */
	PE_FO,     /* File Offset */
	PE_SIZE    /* Size of an entry */
};

/* Macros */
#define PROGNAME "bin2ips"
#define reverse32(in) ((((uint32) (in & 0xFF000000U)) >> 24U) | \
                       (((uint32) (in & 0x00FF0000U)) >> 8U)  | \
                       (((uint32) (in & 0x0000FF00U)) << 8U)  | \
                       (((uint32) (in & 0x000000FFU)) << 24U));

/*
 * Main Function of the Program
 * Usage:
 * * ./bin2ips [patfile] [binfile] [ipsfile]
 */
int main(int argc, char **argv)
{
	/* Variable Declarations */
	FILE *fpat = NULL;             /* File pointer to pat file */
	FILE *fbin = NULL;             /* File pointer to bin file */
	FILE *fips = NULL;             /* File pointer to ips file */
	uint32 *pat = NULL;            /* Pat file in memory */
	uint8  *bin = NULL;            /* Bin file in memory */
	long int patsize = 0l;         /* Size of pat file */
	long int binsize = 0l;         /* Size of bin file */
	long int patpos = 0l;          /* Current position in pat file */
	uint8 ipsentry[5];             /* Header for an ips-entry */
	int putval = 0;                /* Return value of fputc() */
	int seekval = 0;               /* Return value of fseek() */
	size_t readval = (size_t) 0U;  /* Return value of fread() */
	size_t writeval = (size_t) 0U; /* Return value of fwrite() */
	int closeval = 0;              /* Return value of fclose() */
	int retval = ERROR_NONE;       /* Error code */
	
	/* Checks fixed integers */
#ifndef DONT_CHECK_FIXEDINTS
	if (sizeof(int8) != (size_t) 1U)
	{
		retval = ERROR_INT8_NOT_8_BIT;
		goto end;
	}
	if (sizeof(int16) != (size_t) 2U)
	{
		retval = ERROR_INT16_NOT_16_BIT;
		goto end;
	}
	if (sizeof(int32) != (size_t) 4U)
	{
		retval = ERROR_INT32_NOT_32_BIT;
		goto end;
	}
	if (sizeof(uint8) != (size_t) 1U)
	{
		retval = ERROR_UINT8_NOT_8_BIT;
		goto end;
	}
	if (sizeof(uint16) != (size_t) 2U)
	{
		retval = ERROR_UINT16_NOT_16_BIT;
		goto end;
	}
	if (sizeof(uint32) != (size_t) 4U)
	{
		retval = ERROR_UINT32_NOT_32_BIT;
		goto end;
	}
#endif
	
	/* Read Parameters */
	if (argc < PARAM_COUNT)
	{
		retval = ERROR_INSUFFICIENT_PARAMS;
		goto end;
	}
	fpat = fopen(argv[PARAM_PAT], "rb");
	if (fpat == NULL)
	{
		retval = ERROR_PATFILE_OPEN_FAILED;
		goto end;
	}
	fbin = fopen(argv[PARAM_BIN], "rb");
	if (fbin == NULL)
	{
		retval = ERROR_BINFILE_OPEN_FAILED;
		goto end;
	}
	fips = fopen(argv[PARAM_IPS], "wb");
	if (fips == NULL)
	{
		retval = ERROR_IPSFILE_OPEN_FAILED;
		goto end;
	}
	
	/* Reads input files into memory */
	seekval = fseek(fpat, 0l, SEEK_END);
	if (seekval != 0)
	{
		retval = ERROR_PATFILE_READ_FAILED;
		goto end;
	}
	patsize = ftell(fpat);
	patsize >>= 2U;
	pat = calloc((size_t) patsize, sizeof(uint32));
	if (pat == NULL)
	{
		retval = ERROR_MEMORY;
		goto end;
	}
	seekval = fseek(fpat, 0l, SEEK_SET);
	if (seekval != 0)
	{
		retval = ERROR_PATFILE_READ_FAILED;
		goto end;
	}
	readval = fread(pat, sizeof(uint32), (size_t) patsize, fpat);
	if (readval != (size_t) patsize)
	{
		retval = ERROR_PATFILE_READ_FAILED;
		goto end;
	}
	closeval = fclose(fpat);
	fpat = NULL;
	if (closeval != 0)
	{
		retval = ERROR_PATFILE_CLOSE_FAILED;
		goto end;
	}
	seekval = fseek(fbin, 0l, SEEK_END);
	if (seekval != 0)
	{
		retval = ERROR_BINFILE_READ_FAILED;
		goto end;
	}
	binsize = ftell(fbin);
	bin = calloc((size_t) binsize, sizeof(uint8));
	if (bin == NULL)
	{
		retval = ERROR_MEMORY;
		goto end;
	}
	seekval = fseek(fbin, 0l, SEEK_SET);
	if (seekval != 0)
	{
		retval = ERROR_BINFILE_READ_FAILED;
		goto end;
	}
	readval = fread(bin, sizeof(uint8), (size_t) binsize, fbin);
	if (readval != (size_t) binsize)
	{
		retval = ERROR_BINFILE_READ_FAILED;
		goto end;
	}
	closeval = fclose(fbin);
	fbin = NULL;
	if (closeval != 0)
	{
		retval = ERROR_BINFILE_CLOSE_FAILED;
		goto end;
	}
	
	/* Writes the patch header to ips */
	putval = fputc((int) 0x50, fips); /* P */
	if (putval != (int) 0x50)
	{
		retval = ERROR_IPSFILE_WRITE_FAILED;
		goto end;
	}
	putval = fputc((int) 0x41, fips); /* A */
	if (putval != (int) 0x41)
	{
		retval = ERROR_IPSFILE_WRITE_FAILED;
		goto end;
	}
	putval = fputc((int) 0x54, fips); /* T */
	if (putval != (int) 0x54)
	{
		retval = ERROR_IPSFILE_WRITE_FAILED;
		goto end;
	}
	putval = fputc((int) 0x43, fips); /* C */
	if (putval != (int) 0x43)
	{
		retval = ERROR_IPSFILE_WRITE_FAILED;
		goto end;
	}
	putval = fputc((int) 0x48, fips); /* H */
	if (putval != (int) 0x48)
	{
		retval = ERROR_IPSFILE_WRITE_FAILED;
		goto end;
	}
	
	/* Adjusts for endianness */
#ifdef __BIG_ENDIAN__
	for (patpos = 0l; patpos < patsize; patpos++)
	{
		pat[patpos] = reverse32(pat[patpos]);
	}
#endif
	
	/* Builds the patch entries */
	for (patpos = 0l; patpos < patsize; patpos += PE_SIZE)
	{
		/* Checks the bounds of the entry */
		if ((pat[(patpos + PE_SP)] & 0xFF000000U) != 0x00000000U)
		{
			retval = ERROR_OFFSET_TOO_LARGE;
			goto end;
		}
		if ((pat[(patpos + PE_LEN)] & 0xFFFF0000U) != 0x00000000U)
		{
			retval = ERROR_LENGTH_TOO_LARGE;
			goto end;
		}
		if (((long int) (pat[(patpos + PE_SP)] + pat[(patpos + PE_LEN)])) >
		    binsize)
		{
			retval = ERROR_OFFSET_OUT_OF_BIN;
			goto end;
		}
		
		/* Builds the entry header */
		ipsentry[0] = (uint8) ((pat[(patpos + PE_FO)] & 0x00FF0000U) >> 16U);
		ipsentry[1] = (uint8) ((pat[(patpos + PE_FO)] & 0x0000FF00U) >> 8U);
		ipsentry[2] = (uint8) (pat[(patpos + PE_FO)] & 0x000000FFU);
		ipsentry[3] = (uint8) ((pat[(patpos + PE_LEN)] & 0x0000FF00U) >> 8U);
		ipsentry[4] = (uint8) (pat[(patpos + PE_LEN)] & 0x000000FFU);
		
		/* Writes the entry header to the ips file */
		writeval = fwrite(ipsentry, (size_t) 1U, (size_t) 5U, fips);
		if (writeval != (size_t) 5U)
		{
			retval = ERROR_IPSFILE_WRITE_FAILED;
			goto end;
		}
		
		/* Writes the code to the patch entry */
		writeval = fwrite(&(bin[pat[(patpos + PE_SP)]]), (size_t) 1U,
		                  (size_t) pat[(patpos + PE_LEN)], fips);
		if (writeval != (size_t) pat[(patpos + PE_LEN)])
		{
			retval = ERROR_IPSFILE_WRITE_FAILED;
			goto end;
		}
	}
	
	/* Writes the patch footer */
	putval = fputc((int) 0x45, fips); /* E */
	if (putval != (int) 0x45)
	{
		retval = ERROR_IPSFILE_WRITE_FAILED;
		goto end;
	}
	putval = fputc((int) 0x4F, fips); /* O */
	if (putval != (int) 0x4F)
	{
		retval = ERROR_IPSFILE_WRITE_FAILED;
		goto end;
	}
	putval = fputc((int) 0x46, fips); /* F */
	if (putval != (int) 0x46)
	{
		retval = ERROR_IPSFILE_WRITE_FAILED;
		goto end;
	}
	
	/* Closes the ips file */
	closeval = fclose(fips);
	fips = NULL;
	if (closeval != 0)
	{
		retval = ERROR_IPSFILE_CLOSE_FAILED;
		goto end;
	}
	
end:
	if (fpat != NULL)
	{
		(void) fclose(fpat);
	}
	if (fbin != NULL)
	{
		(void) fclose(fbin);
	}
	if (fips != NULL)
	{
		(void) fclose(fips);
	}
	if (pat != NULL)
	{
		free(pat);
	}
	if (bin != NULL)
	{
		free(bin);
	}
	
	/* Prints error codes if needed */
	switch (retval)
	{
		case ERROR_NONE:
			break;
#ifndef DONT_CHECK_FIXEDINTS
		case ERROR_INT8_NOT_8_BIT:
			printf(PROGNAME ": int8 is not 8-bit\n");
			break;
		case ERROR_INT16_NOT_16_BIT:
			printf(PROGNAME ": int16 is not 16-bit\n");
			break;
		case ERROR_INT32_NOT_32_BIT:
			printf(PROGNAME ": int32 is not 32-bit\n");
			break;
		case ERROR_UINT8_NOT_8_BIT:
			printf(PROGNAME ": uint8 is not 8-bit\n");
			break;
		case ERROR_UINT16_NOT_16_BIT:
			printf(PROGNAME ": uint16 is not 16-bit\n");
			break;
		case ERROR_UINT32_NOT_32_BIT:
			printf(PROGNAME ": uint32 is not 32-bit\n");
			break;
#endif
		case ERROR_INSUFFICIENT_PARAMS:
			printf(PROGNAME ": Insufficient parameters\n"
			       "Usage: %s [patfile] [binfile] [ipsfile]\n",
			       argv[PARAM_PROG]);
			break;
		case ERROR_PATFILE_OPEN_FAILED:
			printf(PROGNAME ": %s could not be opened\n", argv[PARAM_PAT]);
			break;
		case ERROR_BINFILE_OPEN_FAILED:
			printf(PROGNAME ": %s could not be opened\n", argv[PARAM_BIN]);
			break;
		case ERROR_IPSFILE_OPEN_FAILED:
			printf(PROGNAME ": %s could not be opened\n", argv[PARAM_IPS]);
			break;
		case ERROR_PATFILE_READ_FAILED:
			printf(PROGNAME ": %s could not be read from\n", argv[PARAM_PAT]);
			break;
		case ERROR_BINFILE_READ_FAILED:
			printf(PROGNAME ": %s could not be read from\n", argv[PARAM_BIN]);
			break;
		case ERROR_IPSFILE_WRITE_FAILED:
			printf(PROGNAME ": %s could not be written to\n", argv[PARAM_IPS]);
			break;
		case ERROR_PATFILE_CLOSE_FAILED:
			printf(PROGNAME ": %s could not be closed\n", argv[PARAM_PAT]);
			break;
		case ERROR_BINFILE_CLOSE_FAILED:
			printf(PROGNAME ": %s could not be closed\n", argv[PARAM_BIN]);
			break;
		case ERROR_IPSFILE_CLOSE_FAILED:
			printf(PROGNAME ": %s could not be closed\n", argv[PARAM_IPS]);
			break;
		case ERROR_MEMORY:
			printf(PROGNAME ": Memory could not be allocated\n");
			break;
		case ERROR_OFFSET_TOO_LARGE:
			printf(PROGNAME ": Offset requires more than 24 bits\n");
			break;
		case ERROR_LENGTH_TOO_LARGE:
			printf(PROGNAME ": Length requires more than 16 bits\n");
			break;
		case ERROR_OFFSET_OUT_OF_BIN:
			printf(PROGNAME ": Code is outside of %s\n", argv[PARAM_BIN]);
			break;
	}
	
	return retval;
}
