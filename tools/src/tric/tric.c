/*
 * Tomb Raider Image Converter v2.0.1
 * Copyright (C) 2018  b122251
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/* File Inclusions */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "tricints.h"
#include "bin.h"
#include "obj.h"
#include "pcx.h"
#include "raw.h"
#include "str.h"
#include "wad.h"

/* Function Declarations */
int main(int argc, char *argv[]);
static int interpretParameters(int argc, char *argv[]);
static void errorMessage(int errornumber);

/*
 * Main function of the program.
 * Parameters:
 *   argc = Number of command-line parameters
 *   argv = Command-line parameters
 * Return Values:
 *  -1 = Insufficient parameters
 *   0 = Everything went well
 *   1 = Input file could not be opened
 *   2 = Input file could not be read from
 *   3 = Output file could not be opened
 *   4 = Output file could not be written to
 *   5 = Buffer could not be allocated
 *   6 = Input bin is of unknown dimensions (bintobmp)
 *   7 = Input bmp is not valid
 *   8 = Input bmp is not 16 bpp
 *   9 = Input image width is not 640 (bmptostr)
 *  10 = Input file is not a valid PCX-file
 *  11 = Input pcx version number is not 5
 *  12 = Input pcx-file is not 8-bit
 *  13 = Input BMP-file is not 8-bit
 *  14 = Input BMP-file is not indexed with <=256 colours
 *  15 = Temporary file could not be created
 *  16 = Temporary file could not be read from
 *  17 = Temporary file could not be written to
 *  18 = Input file could not be opened (dernc2)
 *  19 = Output file could not be opened (dernc2)
 *  20 = Reading from infile failed (dernc2)
 *  21 = Writing to outfile failed (dernc2)
 *  22 = Failed to allocate buffer (dernc2)
 *  23 = End of infile reached (dernc2)
 *  24 = Command somehow invalid (dernc2)
 *  25 = Amount > 263 (dernc2)
 *  26 = Distance > 4096 (dernc2)
 *  27 = Input WAD is not valid
 *  28 = Input thumbnail file is too big
 * 	29 = Type is invalid
 * 	30 = Input OBJ-file is invalid
 *  31 = Input bmp is not 24 bpp
 */
int main(int argc, char *argv[]){
	/* Variable Declarations */
	int retval; /* Return value for this program */
	
	/* Interprets command-line parameters */
	retval=interpretParameters(argc,argv);
	
	/* Prints error messages if needed */
	if (retval!=0) errorMessage(retval);
	
	/* Returns potential error numbers and closes the program */
	return retval;
}

/*
 * Function that interprets user input and calls appropriate functions.
 * Parameters:
 *   argc = Number of command-line parameters
 *   argv = Command-line parameters
 * Return Values:
 *   -1 = Insufficient parameters
 *    0 = Everything went well
 *   >0 = Error code of called functions
 */
static int interpretParameters(int argc, char *argv[]){
	/* Variable Declarations */
	int retval=-1; /* Return value of this function */
	char ext[3]; /* File extension of given files */
	size_t curchar; /* Current character when converting parameters */
	
	/* Checks number of parameters and length of argv[2] */
	if (argc<4) return -1;
	if (strlen(argv[2])<3) return -1;
	
	/* Converts argv[1] to lower case */
	for (curchar=0;curchar<strlen(argv[1]);curchar++)
		argv[1][curchar]=tolower(argv[1][curchar]);
	
	/* Reads extension */
	ext[0]=tolower(argv[2][strlen(argv[2])-3]);
	ext[1]=tolower(argv[2][strlen(argv[2])-2]);
	ext[2]=tolower(argv[2][strlen(argv[2])-1]);
	
	/* Interprets format parameter */
	if ((strncmp(argv[1],"tr1pc",5)==0)||
	    (strncmp(argv[1],"tr2pc",5)==0)){
		/* Tomb Raider 1 or Tomb Raider 2 on PC (PCX) */
		if (strncmp(ext,"pcx",3)==0) retval=pcxtobmp(argv[2],argv[3]);
		else retval=bmptopcx(argv[2],argv[3]);
	} else if (strncmp(argv[1],"tr1ps",5)==0){
		/* Tomb Raider 1 on Sony Playstation (RAW) */
		if (strncmp(ext,"raw",3)==0) retval=rawtobmp(argv[2],argv[3],1);
		else retval=bmptoraw(argv[2],argv[3],1);
	} else if (strncmp(argv[1],"tr1sa",5)==0){
		/* Tomb Raider 1 on Sega Saturn (BIN) */
		if (strncmp(ext,"bin",3)==0) retval=bintobmp(argv[2],argv[3]);
		else  retval=bmptobin(argv[2],argv[3]);
	} else if ((strncmp(argv[1],"tr2ps",5)==0)||
	           (strncmp(argv[1],"tr3ps",5)==0)){
		/* Tomb Raider 2 or Tomb Raider 3 on Sony Playstation (RAW) */
		if (strncmp(ext,"raw",3)==0) retval=rawtobmp(argv[2],argv[3],2);
		else retval=bmptoraw(argv[2],argv[3],2);
	} else if (strncmp(argv[1],"tr4ps",5)==0){
		/* Tomb Raider 4 on Sony Playstation (OBJ) */
		if (strncmp(ext,"obj",3)==0){
			retval=extractobj(argv[2],argv[3],4);
		} else {
			retval=injectobj(argv[2],argv[3]);
		}
	} else if (strncmp(argv[1],"tr4dc",5)==0){
		/* Tomb Raider 4 on Dreamcast (WAD) */
		if (strncmp(ext,"wad",3)==0) retval=unpackwad(argv[2],argv[3]);
		else retval=packwad(argv[2],argv[3]);
	} else if ((strncmp(argv[1],"tr5pc",5)==0)||
	           (strncmp(argv[1],"tr5dc",5)==0)){
		/* Tomb Raider 5 on PC or Sega Dreamcast (STR) */
		if (strncmp(ext,"str",3)==0) retval=strtobmp(argv[2],argv[3]);
		else retval=bmptostr(argv[2],argv[3]);
	} else if (strncmp(argv[1],"tr5ps",5)==0){
		/* Tomb Raider 5 on Sony Playstation (OBJ) */
		if (strncmp(ext,"obj",3)==0){
			retval=extractobj(argv[2],argv[3],5);
		} else {
			retval=injectobj(argv[2],argv[3]);
		}
	} else if (strncmp(argv[1],"beta1ps",7)==0){
		/* Tomb Raider 1 Playstation Beta (04-09-1996) */
		if (strncmp(ext,"raw",3)==0) retval=rawtobmp(argv[2],argv[3],3);
		else retval=bmptoraw(argv[2],argv[3],3);
	} else if (strncmp(argv[1],"beta2ps",7)==0){
		/* Tomb Raider 2 Playstation Beta (16-06-1997) */
		if (strncmp(ext,"raw",3)==0) retval=rawtobmp(argv[2],argv[3],4);
		else retval=bmptoraw(argv[2],argv[3],4);
	} else if (strncmp(argv[1],"beta4pc",7)==0){
		/* Tomb Raider 4 PC Beta (15-09-1999) */
		if (strncmp(ext,"raw",3)==0) retval=rawtobmp(argv[2],argv[3],5);
		else retval=bmptoraw(argv[2],argv[3],5);
	} else if (strncmp(argv[1],"trlemac",7)==0){
		/* Tomb Raider Level Editor (Mac) */
		if (strncmp(ext,"raw",3)==0) retval=rawtobmp(argv[2],argv[3],6);
		else retval=bmptoraw(argv[2],argv[3],6);
	}
	return retval;
}

/* 
 * Function that prints error messages to the screen
 * Parameters:
 *   errornumber = Number of the errro
 */
static void errorMessage(int errornumber){
	switch (errornumber){
		case -1:
			printf("tric: insufficient parameters\n");
			break;
		case 1: 
			printf("tric: input file could not be opened\n");
			break;
		case 2: 
			printf("tric: input file could not be read from\n");
			break;
		case 3: 
			printf("tric: output file could not be opened\n");
			break;
		case 4: 
			printf("tric: output file could not be written to\n");
			break;
		case 5: 
			printf("tric: buffer could not be allocated\n");
			break;
		case 6: 
			printf("tric: input bin is of unknown dimensions\n");
			break;
		case 7: 
			printf("tric: input bmp is not valid\n");
			break;
		case 8: 
			printf("tric: input bmp is not 16 bpp\n");
			break;
		case 9: 
			printf("tric: input image width is not 640\n");
			break;
		case 10:
			printf("tric: input file is not a valid pcx-file\n");
			break;
		case 11:
			printf("tric: input pcx version number is not 5\n");
			break;
		case 12:
			printf("tric: input pcx-file is not 8-bit\n");
			break;
		case 13:
			printf("tric: input bmp-file is not 8-bit\n");
			break;
		case 14:
			printf("tric: input bmp-file is not indexed with ≤256 colours\n");
			break;
		case 15:
			printf("tric: temporary file could not be created\n");
			break;
		case 16:
			printf("tric: temporary file could not be read from\n");
			break;
		case 17:
			printf("tric: temporary file could not be written to\n");
			break;
		case 18:
			printf("tric: input file could not be opened\n");
			break;
		case 19: 
			printf("tric: output file could not be opened\n");
			break;
		case 20: 
			printf("tric: input file could not be read from\n");
			break;
		case 21: 
			printf("tric: output file could not be written to\n");
			break;
		case 22:
			printf("tric: failed to allocate buffer\n");
			break;
		case 23:
		case 24:
		case 25:
		case 26:
			printf("tric: rnc2 decompression failed\n");
			break;
		case 27:
			printf("tric: input wad-file is not valid\n");
			break;
		case 28:
			printf("tric: input thumbnail file is too big\n");
			break;
		case 30:
			printf("tric: input obj-file is invalid\n");
			break;
		case 31:
			printf("tric: input bmp is not 24 bpp\n");
			break;
	}
}
