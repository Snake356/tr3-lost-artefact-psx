/*
 * Library for Converting between STR (TR5PC) and BMP
 * Copyright (C) 2017  b122251
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
#include "tricints.h"
#include "imgtools.h"
#include "str.h"

/*
 * Function that converts an STR-file to BMP.
 * Parameters:
 *   inFilePath = Path to input STR-file
 *   outFilePath = Path to output BMP-file
 * Return values:
 *   0 = Everything went well
 *   1 = Input file could not be opened
 *   2 = Input file could not be read from
 *   3 = Output file could not be opened
 *   4 = Output file could not be written to
 *   5 = Buffer could not be allocated
 *   9 = Input image width is not 640
 */
int strtobmp(char *inFilePath, char *outFilePath){
	/* Variable Declarations */
	FILE *str=NULL; /* Input STR-file */
	FILE *bmp=NULL; /* Output BMP-file */
	char *buffer=NULL; /* Buffer for image data */
	unsigned INT32 strsize=0; /* Length of input STR-file */
	unsigned INT32 strpos=0; /* Current position in input STR-file */
	unsigned INT32 buffersize=0; /* Length of buffer */
	int freeint1=0; /* Integer for general use */
	size_t freesizet1=0; /* size_t for general use */
	int retval=0; /* Value this function will return */
	
	/* Opens input STR-file */
	str=fopen(inFilePath,"rb");
	if (str==NULL){
		retval=1;
		goto end;
	}
	
	/* Opens output BMP-file */
	bmp=fopen(outFilePath,"wb");
	if (bmp==NULL){
		retval=3;
		goto end;
	}
	
	/* Determines strsize */
	freeint1=fseek(str,0,SEEK_END);
	if (freeint1!=0){
		retval=2;
		goto end;
	}
	strsize=(unsigned INT32) ftell(str);
	
	/* Checks whether width is 640 */
	if ((strsize%1280)!=0){
		retval=9;
		goto end;
	}
	
	/* Writes bitmap header */
	freeint1=bmpheader(bmp,strsize+54,54,640,(strsize/1280),16,0,
	                   strsize,0,0,0,0);
	if (freeint1!=0){
		retval=4;
		goto end;
	}
	
	/* Allocates buffer */
	buffer=NULL;
	buffersize=0xFFFFFF;
	do{
		buffer=malloc((size_t) buffersize);
		if ((buffersize<=1280)&&(buffer==NULL)){
			retval=5;
			goto end;
		}
		if (buffer==NULL) buffersize>>=1;
	}while (buffer==NULL);
	buffersize-=(buffersize%1280);
	
	/* Processes image data (backwards) in chunks */
	strpos=strsize;
	while (strpos>0){
		/* Adjusts the size of the final chunk if needed */
		if (strpos<buffersize) buffersize=strpos;
		
		/* Reads chunk into buffer */
		freeint1=fseek(str,(INT32) (strpos-buffersize),SEEK_SET);
		if (freeint1!=0){
			retval=2;
			goto end;
		}
		freesizet1=fread(buffer,1,(size_t) buffersize,str);
		if (freesizet1!=(size_t) buffersize){
			retval=2;
			goto end;
		}
		
		/* Flips chunk vertically */
		freeint1=verticalFlip(buffer,1280,(size_t)(buffersize/1280));
		if (freeint1!=0){
			retval=5;
			goto end;
		}
		
		/* Writes chunk to output BMP-file */
		freesizet1=fwrite(buffer,1,(size_t) buffersize,bmp);
		if (freesizet1!=(size_t) buffersize){
			retval=4;
			goto end;
		}
		
		strpos-=buffersize;
	}
	
	/* Closes files, frees buffer and returns function */
end:
	if (buffer!=NULL) free(buffer);
	if (str!=NULL){
		freeint1=fclose(str);
		if (freeint1!=0) retval=2;
	}
	if (bmp!=NULL){
		freeint1=fclose(bmp);
		if (freeint1!=0) retval=4;
	}
	return retval;
}

/*
 * Function that converts a BMP-file to STR.
 * Parameters:
 *   inFilePath = Path to input BMP-file
 *   outFilePath = Path to output STR-file
 * Return values:
 *   0 = Everything went well
 *   1 = Input file could not be opened
 *   2 = Input file could not be read from
 *   3 = Output file could not be opened
 *   4 = Output file could not be written to
 *   5 = Buffer could not be allocated
 *   7 = Input bmp is not valid
 *   8 = Input bmp is not 16 bbp
 *   9 = Input image width is not 640
 */
int bmptostr(char *inFilePath, char *outFilePath){
	/* Variable Declarations */
	FILE *bmp=NULL; /* Input BMP-file */
	FILE *str=NULL; /* Output STR-file */
	char *buffer=NULL; /* Buffer to be used for image data */
	unsigned INT32 buffersize=0; /* Size of buffer in bytes */
	unsigned INT32 imagestart=0; /* Start of the BMP image data */
	unsigned INT32 imageend=0; /* End of the BMP image data */
	unsigned INT32 bmppos=0; /* Current position in BMP-file */
	unsigned INT32 bmpsize=0; /* Size of BMP-file */
	unsigned INT32 width=0; /* Width of the image in bytes */
	unsigned INT32 height=0; /* Height of the image in pixels */
	int freeint1=0; /* Integer for general use */
	size_t freesizet=0; /* size_t for general use */
	INT16 freeshort1=0; /* Short integer for general use */
	int retval=0; /* Value this function will return */
	
	/* Opens input BMP-file */
	bmp=fopen(inFilePath,"rb");
	if (bmp==NULL){
		retval=1;
		goto end;
	}
	
	/* Opens output STR-file */
	str=fopen(outFilePath,"wb");
	if (str==NULL){
		retval=3;
		goto end;
	}
	
	/* Determines bmpsize */
	freeint1=fseek(bmp,0,SEEK_END);
	if (freeint1!=0){
		retval=2;
		goto end;
	}
	bmpsize=(unsigned INT32) ftell(bmp);
	
	/* Checks bmpsize */
	if (bmpsize<54){
		retval=7;
		goto end;
	}
	
	/* Reads imagestart */
	freeint1=fseek(bmp,10,SEEK_SET);
	if (freeint1!=0){
		retval=2;
		goto end;
	}
	freesizet=fread(&imagestart,1,4,bmp);
	if (freesizet!=4){
		retval=2;
		goto end;
	}
	
	/* Reads width and height */
	freeint1=fseek(bmp,18,SEEK_SET);
	if (freeint1!=0){
		retval=2;
		goto end;
	}
	freesizet=fread(&width,1,4,bmp);
	if (freesizet!=4){
		retval=2;
		goto end;
	}
	freesizet=fread(&height,1,4,bmp);
	if (freesizet!=4){
		retval=2;
		goto end;
	}
	
	/* Checks whether width equals 640 */
	if (width!=640){
		retval=9;
		goto end;
	}
	
	/* Reads bbp */
	freeint1=fseek(bmp,28,SEEK_SET);
	if (freeint1!=0){
		retval=2;
		goto end;
	}
	freesizet=fread(&freeshort1,1,2,bmp);
	if (freesizet!=2){
		retval=2;
		goto end;
	}
	
	/* Checks whether the image is 16 bits per pixel */
	if (freeshort1!=16){
		retval=8;
		goto end;
	}
	
	/* Calculates imageend */
	imageend=(unsigned INT32) (freeshort1/8);
	imageend*=width;
	imageend*=height;
	imageend+=imagestart;
	
	/* Checks whether image data is complete */
	if (imageend>bmpsize){
		retval=7;
		goto end;
	}
	
	/* Recalculates width */
	width*=(freeshort1/8);
	
	/* Allocates buffer */
	buffer=NULL;
	buffersize=0xFFFFFF;
	do{
		buffer=malloc((size_t) buffersize);
		if ((buffersize<=(unsigned INT32) width)&&(buffer==NULL)){
			retval=5;
			goto end;
		}
		if (buffer==NULL) buffersize>>=1;
	}while (buffer==NULL);
	buffersize-=(buffersize%width);
	
	/* Processes image data (backwards) in chunks */
	bmppos=imageend;
	while (bmppos>imagestart){
		/* Adjusts the size of the final chunk if needed */
		if ((bmppos-imagestart)<buffersize)
			buffersize=(bmppos-imagestart);
		
		/* Reads chunk into buffer */
		freeint1=fseek(bmp,(INT32) (bmppos-buffersize),SEEK_SET);
		if (freeint1!=0){
			retval=2;
			goto end;
		}
		freesizet=fread(buffer,1,(size_t) buffersize,bmp);
		if (freesizet!=(size_t) buffersize){
			retval=2;
			goto end;
		}
		
		/* Flips chunk vertically */
		freeint1=verticalFlip(buffer,(size_t)width,
		                      (size_t)(buffersize/width));
		if (freeint1!=0){
			retval=5;
			goto end;
		}
		
		/* Writes chunk to output STR-file */
		freesizet=fwrite(buffer,1,(size_t) buffersize,str);
		if (freesizet!=(size_t) buffersize){
			retval=4;
			goto end;
		}
		
		bmppos-=buffersize;
	}
	
	/* Closes files, frees buffer and returns function */
end:
	if (buffer!=NULL) free(buffer);
	if (bmp!=NULL){
		freeint1=fclose(bmp);
		if (freeint1!=0) retval=2;
	}
	if (str!=NULL){
		freeint1=fclose(str);
		if (freeint1!=0) retval=4;
	}
	return retval;
}
