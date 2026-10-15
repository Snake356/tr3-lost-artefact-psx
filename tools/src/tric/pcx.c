/*
 * Library for Converting between PCX (8-bit indexed) and BMP
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
#include "pcx.h"

/*
 * Function that converts a PCX-file to BMP.
 * Parameters:
 *   inFilePath = Path to input PCX-file
 *   outFilePath = Path to output BMP-file
 * Return values:
 *   0 = Everything went well
 *   1 = Input file could not be opened
 *   2 = Input file could not be read from
 *   3 = Output file could not be opened
 *   4 = Output file could not be written to
 *   5 = Buffer could not be allocated
 *  10 = Input file is not a valid PCX-file
 *  11 = Input pcx version number is not 5
 *  12 = Input pcx-file is not 8-bit
 */
int pcxtobmp(char *inFilePath, char *outFilePath){
	/* Variable Declarations */
	FILE *pcx=NULL; /* Input PCX-file */
	FILE *bmp=NULL; /* Output BMP-file */
	char *buffer=NULL; /* Buffer to be used for image data */
	unsigned INT32 buffersize=0; /* Size of buffer in bytes */
	unsigned INT32 bufferpos=0; /* Current position in buffer */
	unsigned INT32 pcxsize=0; /* Size of the PCX-file in bytes */
	unsigned INT32 width=0; /* Image width in pixels */
	unsigned INT32 height=0; /* Image height in pixels */
	unsigned INT32 imagesize=0; /* Size of the BMP-image data */
	unsigned INT32 bmppos=0; /* Current position in BMP-file */
	int retval=0; /* Value this function will return */
	int freeint1=0; /* Integer for general use */
	int freeint2=0; /* Integer for general use */
	INT16 freeshort1=0; /* Short integer for general use */
	INT16 freeshort2=0; /* Short integer for general use */
	size_t freesizet1=0; /* size_t for general use */
	
	/* Opens input PCX-file */
	pcx=fopen(inFilePath,"rb");
	if (pcx==NULL){
		retval=1;
		goto end;
	}
	
	/* Determines pcxsize */
	freeint1=fseek(pcx,0,SEEK_END);
	if (freeint1!=0){
		retval=2;
		goto end;
	}
	pcxsize=(unsigned INT32) ftell(pcx);
	
	/* Checks input PCX-file */
	freeint1=fseek(pcx,0,SEEK_SET);
	if (freeint1!=0){
		retval=2;
		goto end;
	}
	freeint1=fgetc(pcx); /* Manufacturer byte */
	if (freeint1!=10){
		retval=10;
		goto end;
	}
	freeint1=fgetc(pcx); /* Version number */
	if (freeint1!=5){
		retval=11;
		goto end;
	}
	freeint1=fgetc(pcx); /* Run Length Encoding byte */
	if (freeint1!=1){
		retval=10;
		goto end;
	}
	freeint1=fgetc(pcx); /* Bits per pixel */
	if (freeint1!=8){
		retval=12;
		goto end;
	}
	
	/* Reads width */
	freeint1=fseek(pcx,4,SEEK_SET);
	if (freeint1!=0){
		retval=2;
		goto end;
	}
	freesizet1=fread(&freeshort1,1,2,pcx);
	if (freesizet1!=2){
		retval=2;
		goto end;
	}
	freeint1=fseek(pcx,8,SEEK_SET);
	if (freeint1!=0){
		retval=2;
		goto end;
	}
	freesizet1=fread(&freeshort2,1,2,pcx);
	if (freesizet1!=2){
		retval=2;
		goto end;
	}
	width=(unsigned INT32) (1+(freeshort2-freeshort1));
	
	/* Reads height */
	freeint1=fseek(pcx,6,SEEK_SET);
	if (freeint1!=0){
		retval=2;
		goto end;
	}
	freesizet1=fread(&freeshort1,1,2,pcx);
	if (freesizet1!=2){
		retval=2;
		goto end;
	}
	freeint1=fseek(pcx,10,SEEK_SET);
	if (freeint1!=0){
		retval=2;
		goto end;
	}
	freesizet1=fread(&freeshort2,1,2,pcx);
	if (freesizet1!=2){
		retval=2;
		goto end;
	}
	height=(unsigned INT32) (1+(freeshort2-freeshort1));
	
	/* Calculates imagesize */
	imagesize=(width*height);
	
	/* Allocates buffer */
	buffer=NULL;
	buffersize=((width>768)?width:768);
	buffer=malloc((size_t)buffersize);
	if (buffer==NULL){
		retval=5;
		goto end;
	}
	
	/* Opens output BMP-file */
	bmp=fopen(outFilePath,"wb");
	if (bmp==NULL){
		retval=3;
		goto end;
	}
	
	/* Writes bitmap header */
	freeint1=bmpheader(bmp,(imagesize+1078),1078,width,height,8,0,
	                   imagesize,0,0,256,256);
	if (freeint1!=0){
		retval=4;
		goto end;
	}
	
	/* Converts and writes colour palette */
	freeint1=fseek(pcx,(INT32)(pcxsize-768),SEEK_SET);
	if (freeint1!=0){
		retval=2;
		goto end;
	}
	freesizet1=fread(buffer,1,768,pcx);
	if (freesizet1!=768){
		retval=2;
		goto end;
	}
	for (bufferpos=0;bufferpos<768;bufferpos+=3){
		freesizet1=fwrite(buffer+bufferpos+2,1,1,bmp);
		if (freesizet1!=1){
			retval=4;
			goto end;
		}
		freesizet1=fwrite(buffer+bufferpos+1,1,1,bmp);
		if (freesizet1!=1){
			retval=4;
			goto end;
		}
		freesizet1=fwrite(buffer+bufferpos,1,1,bmp);
		if (freesizet1!=1){
			retval=4;
			goto end;
		}
		freeint1=fputc(0,bmp);
		if (freeint1!=0){
			retval=4;
			goto end;
		}
	}
	
	/* Writes empty bmp image data */
	memset(buffer,0,(size_t)width);
	for (bmppos=imagesize;bmppos>0;bmppos-=width){
		freesizet1=fwrite(buffer,1,(size_t)width,bmp);
		if (freesizet1!=(size_t)width){
			retval=4;
			goto end;
		}
	}
	
	/* Processes and writes image data (backwards) */
	freeint1=fseek(pcx,128,SEEK_SET);
	if (freeint1!=0){
		retval=2;
		goto end;
	}
	for (bmppos=imagesize+1078;bmppos>1078;bmppos-=width){
		/* Decompresses line into buffer while reading from pcx */
		for (bufferpos=0;bufferpos<width;){
			freeint1=fgetc(pcx);
			if (freeint1>=192){
				/* RLE-compressed pixels */
				freeint1&=63;
				freeint2=fgetc(pcx);
				memset(buffer+bufferpos,freeint2,(size_t)freeint1);
				bufferpos+=freeint1;
			}else{
				/* Single pixel */
				memset(buffer+bufferpos,freeint1,1);
				bufferpos++;
			}
		}
		/* Writes line out to bmp file */
		freeint1=fseek(bmp,(INT32)(bmppos-width),SEEK_SET);
		if (freeint1!=0){
			retval=4;
			goto end;
		}
		freesizet1=fwrite(buffer,1,(size_t)width,bmp);
		if (freesizet1!=(size_t)width){
			retval=4;
			goto end;
		}
	}
	
	/* Closes files, frees buffer and returns function */
end:
	if (buffer!=NULL) free(buffer);
	if (pcx!=NULL){
		freeint1=fclose(pcx);
		if (freeint1!=0) retval=2;
	}
	if (bmp!=NULL){
		freeint1=fclose(bmp);
		if (freeint1!=0) retval=4;
	}
	return retval;
}

/*
 * Function that converts a BMP-file to PCX.
 * Parameters:
 *   inFilePath = Path to input BMP-file
 *   outFilePath = Path to output PCX-file
 * Return values:
 *   0 = Everything went well
 *   1 = Input file could not be opened
 *   2 = Input file could not be read from
 *   3 = Output file could not be opened
 *   4 = Output file could not be written to
 *   5 = Buffer could not be allocated
 *   7 = Input BMP is not valid
 *  13 = Input BMP-file is not 8-bit
 *  14 = Input BMP-file is not indexed with <=256 colours
 */
int bmptopcx(char *inFilePath, char *outFilePath){
	/* Variable Declarations */
	FILE *bmp=NULL; /* Input BMP-file */
	FILE *pcx=NULL; /* Output PCX-file */
	char *buffer=NULL; /* Buffer to be used for image data */
	unsigned INT32 buffersize=0; /* Size of buffer in bytes */
	unsigned INT32 bufferpos=0; /* Current position in buffer */
	unsigned INT32 width=0; /* Image width in pixels */
	unsigned INT32 height=0; /* Image height in pixels */
	unsigned INT32 bmpsize=0; /* Length of BMP-file */
	unsigned INT32 bmppos=0; /* Current position in BMP-file */
	unsigned INT32 imagestart=0; /* Offset of image data in BMP-file */
	unsigned INT32 imageend=0; /* End of image data in BMP-file */
	unsigned INT32 numcolours=0; /* Number of colours in the image */
	int retval=0; /* Value this function will return */
	int freeint1=0; /* Integer for general use */
	int freeint2=0; /* Integer for general use */
	int freeint3=0; /* Integer for general use */
	INT16 freeshort1=0; /* Short integer for general use */
	size_t freesizet1=0; /* size_t for general use */
	
	/* Opens input BMP-file */
	bmp=fopen(inFilePath,"rb");
	if (bmp==NULL){
		retval=1;
		goto end;
	}
	
	/* Opens output PCX-file */
	pcx=fopen(outFilePath,"wb");
	if (pcx==NULL){
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
	if (bmpsize<1078){
		retval=7;
		goto end;
	}
	
	/* Reads imagestart */
	freeint1=fseek(bmp,10,SEEK_SET);
	if (freeint1!=0){
		retval=2;
		goto end;
	}
	freesizet1=fread(&imagestart,1,4,bmp);
	if (freesizet1!=4){
		retval=2;
		goto end;
	}
	
	/* Reads width and height */
	freeint1=fseek(bmp,18,SEEK_SET);
	if (freeint1!=0){
		retval=2;
		goto end;
	}
	freesizet1=fread(&width,1,4,bmp);
	if (freesizet1!=4){
		retval=2;
		goto end;
	}
	freesizet1=fread(&height,1,4,bmp);
	if (freesizet1!=4){
		retval=2;
		goto end;
	}
	
	/* Reads bbp */
	freeint1=fseek(bmp,28,SEEK_SET);
	if (freeint1!=0){
		retval=2;
		goto end;
	}
	freesizet1=fread(&freeshort1,1,2,bmp);
	if (freesizet1!=2){
		retval=2;
		goto end;
	}
	
	/* Checks whether the image is 8 bits per pixel */
	if (freeshort1!=8){
		retval=13;
		goto end;
	}
	
	/* Reads numcolours */
	freeint1=fseek(bmp,46,SEEK_SET);
	if (freeint1!=0){
		retval=2;
		goto end;
	}
	freesizet1=fread(&numcolours,1,4,bmp);
	if (freesizet1!=4){
		retval=2;
		goto end;
	}
	
	/* Checks numcolours */
	if ((numcolours>256)||(numcolours==0)){
		retval=14;
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
	
	/* Allocates buffer */
	buffer=NULL;
	buffersize=((width>1024)?width:1024);
	buffer=malloc((size_t)buffersize);
	if (buffer==NULL){
		retval=5;
		goto end;
	}
	
	/* Makes PCX header data */
	memset(buffer,10,1);
	memset(buffer+1,5,1);
	memset(buffer+2,1,1);
	memset(buffer+3,8,1);
	memset(buffer+4,0,124);
	memset(buffer+8,(int)((width-1)&0xFF),1);
	memset(buffer+9,(int)(((width-1)&0xFF00)>>8),1);
	memset(buffer+10,(int)((height-1)&0xFF),1);
	memset(buffer+11,(int)(((height-1)&0xFF00)>>8),1);
	memset(buffer+65,1,1);
	memset(buffer+66,(int)(width&0xFF),1);
	memset(buffer+67,(int)((width&0xFF00)>>8),1);
	
	/* Writes PCX header data to file */
	freesizet1=fwrite(buffer,1,128,pcx);
	if (freesizet1!=128){
		retval=4;
		goto end;
	}
	
	/* Compresses image data (backwards) and writes it to PCX-file */
	for (bmppos=imageend;bmppos>imagestart;bmppos-=width){
		/* Reads row into buffer */
		freeint1=fseek(bmp,(INT32)(bmppos-width),SEEK_SET);
		if (freeint1!=0){
			retval=2;
			goto end;
		}
		freesizet1=fread(buffer,1,(size_t)(width),bmp);
		if (freesizet1!=(size_t)width){
			retval=2;
			goto end;
		}
		/* Compresses and writes row */
		bufferpos=0;
		while (bufferpos<width){
			freeint1=1;
			while (bufferpos<width){
				bufferpos++;
				if (bufferpos==width) break;
				if ((buffer[bufferpos-1])==(buffer[bufferpos])){
					freeint1++;
				}else{
					break;
				}
			}
			freeint2=(int)(buffer[bufferpos-1]);
			if (freeint2<0) freeint2+=256;
			for (;freeint1>63;freeint1-=63){
				freeint3=fputc(255,pcx);
				if (freeint3!=255){
					retval=4;
					goto end;
				}
				freeint3=fputc(freeint2,pcx);
				if (freeint3!=freeint2){
					retval=4;
					goto end;
				}
			}
			if ((freeint1>1)||(freeint2>=192)){
				freeint1|=192;
				freeint3=fputc(freeint1,pcx);
				if (freeint3!=freeint1){
					retval=4;
					goto end;
				}
			}
			freeint3=fputc(freeint2,pcx);
			if (freeint3!=freeint2){
				retval=4;
				goto end;
			}
		}
	}
	
	/* Converts and writes palette */
	freeint1=fputc(12,pcx);
	if (freeint1!=12){
		retval=4;
		goto end;
	}
	memset(buffer,0,1024);
	freeint1=fseek(bmp,(INT32)(imagestart-(numcolours*4)),SEEK_SET);
	if (freeint1!=0){
		retval=2;
		goto end;
	}
	freesizet1=fread(buffer,1,(size_t)(numcolours*4),bmp);
	if (freesizet1!=(size_t)(numcolours*4)){
		retval=2;
		goto end;
	}
	freeint1=0;
	for (bufferpos=0;bufferpos<1024;bufferpos+=4){
		memmove(buffer+bufferpos+3,buffer+bufferpos,1);
		memmove(buffer+freeint1,buffer+bufferpos+2,1);
		memmove(buffer+freeint1+1,buffer+bufferpos+1,1);
		memmove(buffer+freeint1+2,buffer+bufferpos+3,1);
		freeint1+=3;
	}
	freesizet1=fwrite(buffer,1,768,pcx);
	if (freesizet1!=768){
		retval=4;
		goto end;
	}
	
	/* Closes files, frees buffer and returns function */
end:
	if (buffer!=NULL) free(buffer);
	if (bmp!=NULL){
		freeint1=fclose(bmp);
		if (freeint1!=0) retval=2;
	}
	if (pcx!=NULL){
		freeint1=fclose(pcx);
		if (freeint1!=0) retval=4;
	}
	return retval;
}
