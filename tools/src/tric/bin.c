/*
 * Library for Converting between BIN (TR1SAT) and BMP
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
#include "tricints.h"
#include "imgtools.h"
#include "bin.h"

/*
 * Function that converts a BIN-file to BMP.
 * Parameters:
 *   inFilePath = Path to input BIN-file
 *   outFilePath = Path to output BMP-file
 * Return values:
 *   0 = Everything went well
 *   1 = Input file could not be opened
 *   2 = Input file could not be read from
 *   3 = Output file could not be opened
 *   4 = Output file could not be written to
 *   5 = Buffer could not be allocated
 *   6 = Input bin is of unknown dimensions
 */
int bintobmp(char *inFilePath, char *outFilePath){
	/* Variable Declarations */
	FILE *bin=NULL; /* Input BIN-file */
	FILE *bmp=NULL; /* Output BMP-file */
	char *buffer=NULL; /* Buffer to be used for image data */
	unsigned INT32 buffersize=0; /* Size of buffer in bytes */
	unsigned INT32 binsize=0; /* Size of the BIN-file in bytes */
	unsigned INT32 binpos=0; /* Current position in BIN-file */
	unsigned INT32 bufferpos=0; /* Current position in buffer */
	unsigned INT16 colour=0; /* Current colour */
	INT16 width=0; /* Width of the image in bytes */
	INT16 height=0; /* Height of the image in pixels */
	int freeint1=0; /* Integer for general use */
	size_t freesizet=0; /* size_t for general use */
	int retval=0; /* Value this function will return */
	
	/* Opens input BIN-file */
	bin=fopen(inFilePath,"rb");
	if (bin==NULL){
		retval=1;
		goto end;
	}
	
	/* Opens output BMP-file */
	bmp=fopen(outFilePath,"wb");
	if (bmp==NULL){
		retval=3;
		goto end;
	}
	
	/* Determines binsize */
	freeint1=fseek(bin,0,SEEK_END);
	if (freeint1!=0){
		retval=2;
		goto end;
	}
	binsize=(unsigned INT32) ftell(bin);
	
	/* Determines width and height of the image */
	if ((binsize==143360)||(binsize==163840)||(binsize==153600)) width=640;
	if ((binsize==157696)||(binsize==180224)) width=704;
	if ((binsize==143360)||(binsize==157696)) height=224;
	if ((binsize==180224)||(binsize==163840)) height=256;
	if (binsize==153600) height=240;
	
	/* Checks whether dimensions were recognised */
	if ((width==0)||(height==0)){
		retval=6;
		goto end;
	}
	
	/* Allocates buffer */
	buffer=NULL;
	buffersize=0x3FFFF;
	do{
		buffer=malloc((size_t) buffersize);
		if ((buffersize<=(unsigned INT32) width)&&(buffer==NULL)){
			retval=5;
			goto end;
		}
		if (buffer==NULL) buffersize>>=1;
	}while (buffer==NULL);
	buffersize-=(buffersize%width);
	
	/* Writes bitmap header */
	freeint1=bmpheader(bmp,binsize+54,54,(unsigned INT32)(width/2),
	                   (unsigned INT32)height,16,0,binsize,0,0,0,0);
	if (freeint1!=0){
		retval=4;
		goto end;
	}
	
	/* Processes image data (backwards) in chunks */
	binpos=binsize;
	while (binpos>0){
		/* Adjusts the size of the final chunk if needed */
		if (binpos<buffersize) buffersize=binpos;
		
		/* Reads chunk into buffer */
		freeint1=fseek(bin,(INT32) (binpos-buffersize),SEEK_SET);
		if (freeint1!=0){
			retval=2;
			goto end;
		}
		freesizet=fread(buffer,1,(size_t) buffersize,bin);
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
		
		/* Converts colours from Big-Endian BGR to Little-endian RGB */
		for (bufferpos=0;bufferpos<buffersize;bufferpos+=2){
			memmove(&colour,buffer+bufferpos,2);
			colour=(unsigned INT16)
			       ((((unsigned INT16)(colour&0xE000))>>8)|
			        (((unsigned INT16)(colour&0x1F00))<<2)|
			        (((unsigned INT16)(colour&0x0083))<<8)|
			        (((unsigned INT16)(colour&0x007C))>>2));
			memmove(buffer+bufferpos,&colour,2);
		}
		
		/* Writes chunk to output BMP-file */
		freesizet=fwrite(buffer,1,(size_t) buffersize,bmp);
		if (freesizet!=(size_t) buffersize){
			retval=4;
			goto end;
		}
		
		binpos-=buffersize;
	}
	
	/* Closes files, frees buffer and returns function */
end:
	if (buffer!=NULL) free(buffer);
	if (bin!=NULL){
		freeint1=fclose(bin);
		if (freeint1!=0) retval=2;
	}
	if (bmp!=NULL){
		freeint1=fclose(bmp);
		if (freeint1!=0) retval=4;
	}
	return retval;
}

/*
 * Function that converts a BMP-file to BIN.
 * Parameters:
 *   inFilePath = Path to input BMP-file
 *   outFilePath = Path to output BIN-file
 * Return values:
 *   0 = Everything went well
 *   1 = Input file could not be opened
 *   2 = Input file could not be read from
 *   3 = Output file could not be opened
 *   4 = Output file could not be written to
 *   5 = Buffer could not be allocated
 *   7 = Input BMP is not valid
 *   8 = Image is not 16 bbp
 */
int bmptobin(char *inFilePath, char *outFilePath){
	/* Variable Declarations */
	FILE *bmp=NULL; /* Input BMP-file */
	FILE *bin=NULL; /* Output BIN-file */
	char *buffer=NULL; /* Buffer to be used for image data */
	unsigned INT32 buffersize=0; /* Size of buffer in bytes */
	unsigned INT32 imagestart=0; /* Start of the BMP image data */
	unsigned INT32 imageend=0; /* End of the BMP image data */
	unsigned INT32 bmppos=0; /* Current position in BMP-file */
	unsigned INT32 bufferpos=0; /* Current position in buffer */
	unsigned INT32 bmpsize=0; /* Size of BMP-file */
	unsigned INT16 colour=0; /* Current colour */
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
	
	/* Opens output BIN-file */
	bin=fopen(outFilePath,"wb");
	if (bin==NULL){
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
	buffersize=0x3FFFF;
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
		
		/* Converts colours from Little-endian RGB to Big-Endian BGR */
		for (bufferpos=0;bufferpos<buffersize;bufferpos+=2){
			memmove(&colour,buffer+bufferpos,2);
			colour=(unsigned INT16)
			       ((((unsigned INT16)(colour&0x8300))>>8)|
			        (((unsigned INT16)(colour&0x7C00))>>2)|
			        (((unsigned INT16)(colour&0x00E0))<<8)|
			        (((unsigned INT16)(colour&0x001F))<<2));
			memmove(buffer+bufferpos,&colour,2);
		}
		
		/* Writes chunk to output BIN-file */
		freesizet=fwrite(buffer,1,(size_t) buffersize,bin);
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
	if (bin!=NULL){
		freeint1=fclose(bin);
		if (freeint1!=0) retval=4;
	}
	return retval;
}
