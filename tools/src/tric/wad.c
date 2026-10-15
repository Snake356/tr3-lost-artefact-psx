/*
 * Library for Converting between WAD (TR4 on Dreamcast) and BMP
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
#include <sys/stat.h>
#include "tricints.h"
#include "imgtools.h"
#include "strtools.h"
#include "wad.h"
#ifdef _WIN32
#include <direct.h>
#define mkdir(path, mode) _mkdir(path)
#endif

/*
 * Function that extracts the images from a TR4 WAD-file.
 * Parameters:
 *   inFilePath = Path to input WAD-File
 *   outDirPath = Path to output directory
 * Return values:
 *   0 = Everything went well
 *   1 = Input file could not be opened
 *   2 = Input file could not be read from
 *   3 = Output file could not be opened
 *   4 = Output file could not be written to
 *   5 = Buffer could not be allocated
 *  27 = Input WAD is not valid
 */
int unpackwad(char *inFilePath, char *outDirPath){
	/* Variable Declarations */
	FILE *wad=NULL; /* Input WAD-File */
	FILE *bmp=NULL; /* Output File */
	char *outFilePath=NULL; /* Path to current output file */
	char *buffer=NULL; /* Buffer for image data */
	long int wadSize; /* Size of input WAD-file in bytes */
	unsigned INT32 curpos; /* Current position in wad */
	unsigned INT32 buffersize; /* Length of the buffer in bytes */
	unsigned int curimage; /* Number of the current image */
	unsigned INT32 freeuint1, freeuint2; /* Unsigned 32-bit integers */
	int freeint1; /* Integer for general use */
	size_t freesizet1; /* size_t for general use */
	int retval=0; /* Return value of this function */
	
	/* Opens input WAD-File */
	wad=fopen(inFilePath,"rb");
	if (wad==NULL){
		retval=1;
		goto end;
	}
	
	/* Determines wadSize */
	freeint1=fseek(wad,0,SEEK_END);
	if (freeint1!=0){
		retval=2;
		goto end;
	}
	wadSize=ftell(wad);
	
	/* Allocates outFilePath */
	outFilePath=malloc((size_t)(strlen(outDirPath)+16));
	if (outFilePath==NULL){
		retval=5;
		goto end;
	}
	
	/* Makes output directory */
	(void) mkdir(outDirPath,0755);
	
	/* Allocates buffer */
	buffer=NULL;
	buffersize=0x3FFFFF;
	do{
		buffer=malloc((size_t) buffersize);
		if ((buffersize<=1024)&&(buffer==NULL)){
			retval=5;
			goto end;
		}
		if (buffer==NULL) buffersize>>=1;
	}while (buffer==NULL);
	buffersize-=(buffersize%1024);
	
	/* Extracts thumbnails */
	memcpy(outFilePath,outDirPath,strlen(outDirPath));
	memset(outFilePath+strlen(outDirPath),(int)'/',1);
	memset(outFilePath+strlen(outDirPath)+1,(int)'0',1);
	memset(outFilePath+strlen(outDirPath)+2,(int)'.',1);
	memset(outFilePath+strlen(outDirPath)+3,(int)'b',1);
	memset(outFilePath+strlen(outDirPath)+4,(int)'m',1);
	memset(outFilePath+strlen(outDirPath)+5,(int)'p',1);
	memset(outFilePath+strlen(outDirPath)+6,(int)'\0',1);	
	bmp=fopen(outFilePath,"wb");
	if (bmp==NULL){
		retval=3;
		goto end;
	}
	freeint1=bmpheader(bmp,196746,138,512,192,16,0,196608,0,0,0,0);
	memset(buffer,0,84);
	memset(buffer+1,0xF8,1);
	memset(buffer+4,0xE0,1);
	memset(buffer+5,0x07,1);
	memset(buffer+8,0x1F,1);
	memset(buffer+68,0x02,1);
	freesizet1=fwrite(buffer,1,84,bmp);
	freeint1=fseek(wad,0,SEEK_SET);
	if (freeint1!=0){
		retval=2;
		goto end;
	}
	freeuint1=0x30000;
	while (freeuint1>0){
		if (buffersize<freeuint1) freeuint2=buffersize;
		else freeuint2=freeuint1;
		freesizet1=fread(buffer,1,(size_t)freeuint2,wad);
		if (freesizet1!=(size_t)freeuint2){
			retval=2;
			goto end;
		}
		freesizet1=fwrite(buffer,1,(size_t)freeuint2,bmp);
		if (freesizet1!=(size_t)freeuint2){
			retval=4;
			goto end;
		}
		
		freeuint1-=freeuint2;
	}
	freeint1=fseek(bmp,14,SEEK_SET);
	if (freeint1!=0){
		retval=4;
		goto end;
	}
	freeint1=fputc(0x7C,bmp);
	if (freeint1!=0x7C){
		retval=4;
		goto end;
	}
	freeint1=fseek(bmp,30,SEEK_SET);
	if (freeint1!=0){
		retval=4;
		goto end;
	}
	freeint1=fputc(3,bmp);
	if (freeint1!=3){
		retval=4;
		goto end;
	}
	freeint1=fclose(bmp);
	if (freeint1!=0){
		retval=4;
		goto end;
	}else{
		bmp=NULL;
	}
	
	/* Extracts images */
	curimage=1;
	for (curpos=0x80000;curpos<(unsigned INT32)wadSize;){
		/* Opens output file */
		inttostring((outFilePath+strlen(outDirPath)+1),curimage,10,1);
		freeint1=(int)strlen(outFilePath);
		memset(outFilePath+freeint1,(int)'.',1);
		memset(outFilePath+freeint1+1,(int)'b',1);
		memset(outFilePath+freeint1+2,(int)'m',1);
		memset(outFilePath+freeint1+3,(int)'p',1);
		memset(outFilePath+freeint1+4,0,1);
		bmp=fopen(outFilePath,"wb");
		if (bmp==NULL){
			retval=3;
			goto end;
		}
		
		/* Checks whether the WAD file contains another image */
		if ((curpos+6)>((unsigned INT32)wadSize)){
			retval=27;
			goto end;
		}
		
		/* Checks BMP-signature */
		freeint1=fseek(wad,(long int) curpos,SEEK_SET);
		if (freeint1!=0){
			retval=2;
			goto end;
		}
		freeint1=fgetc(wad);
		if (freeint1!=66){
			retval=27;
			goto end;
		}
		freeint1=fgetc(wad);
		if (freeint1!=77){
			retval=27;
			goto end;
		}
		
		/* Reads bitmap size */
		freesizet1=fread(&freeuint1,1,4,wad);
		if (freesizet1!=4){
			retval=27;
			goto end;
		}
		
		/* Determines whether the image in the wad is complete */
		if ((curpos+freeuint1)>((unsigned INT32)wadSize)){
			retval=27;
			goto end;
		}
		
		/* Writes the image to the output file */
		freeint1=fseek(wad,(long int) curpos,SEEK_SET);
		if (freeint1!=0){
			retval=2;
			goto end;
		}
		while (freeuint1>0){
			/* Determines size of chunk */
			if (buffersize<freeuint1) freeuint2=buffersize;
			else freeuint2=freeuint1;
			
			/* Reads chunk into buffer */
			freesizet1=fread(buffer,1,(size_t)freeuint2,wad);
			if (freesizet1!=(size_t)freeuint2){
				retval=2;
				goto end;
			}
			
			/* Writes chunk to output */
			freesizet1=fwrite(buffer,1,(size_t)freeuint2,bmp);
			if (freesizet1!=(size_t)freeuint2){
				retval=4;
				goto end;
			}
			
			freeuint1-=freeuint2;
			curpos+=freeuint2;
		}
		
		/* Closes the output file */
		freeint1=fclose(bmp);
		if (freeint1!=0){
			retval=4;
			goto end;
		}else{
			bmp=NULL;
		}
		
		/* Jumps to the next image */
		curpos+=(2048-(curpos%2048));
		
		/* Increments curimage */
		++curimage;
	}
	
	/* Closes files, frees buffer and returns function */
end:
	if (buffer!=NULL) free(buffer);
	if (outFilePath!=NULL) free(outFilePath);
	if (wad!=NULL){
		freeint1=fclose(wad);
		if (freeint1!=0) retval=2;
	}
	if (bmp!=NULL){
		freeint1=fclose(bmp);
		if (freeint1!=0) retval=4;
	}
	return retval;
}

/*
 * Function that makes a WAD-file from a directory
 * Parameters:
 *   inDirPath = Path to input directory
 *   outFilePath = Path to output WAD-File
 * Return values:
 *   0 = Everything went well
 *   1 = Input file could not be opened
 *   2 = Input file could not be read from
 *   3 = Output file could not be opened
 *   4 = Output file could not be written to
 *   5 = Buffer could not be allocated
 *  28 = Input thumbnail file is too big
 */
int packwad(char *inDirPath, char *outFilePath){
	FILE *bmp=NULL; /* Input File */
	FILE *wad=NULL; /* Output WAD-File */
	char *inFilePath=NULL; /* Path to current input file */
	char *buffer=NULL; /* Buffer for image data */
	unsigned int curimage; /* Number of the current image */
	unsigned INT32 curpos; /* Current position in wad */
	unsigned INT32 buffersize; /* Length of the buffer in bytes */
	unsigned INT32 freeuint1, freeuint2; /* Unsigned 32-bit integers */
	int freeint1; /* Integer for general use */
	size_t freesizet1; /* size_t for general use */
	int retval=0; /* Return value of this function */
	
	/* Opens output WAD-File */
	wad=fopen(outFilePath,"wb");
	if (wad==NULL){
		retval=3;
		goto end;
	}
	
	/* Allocates inFilePath */
	inFilePath=malloc((size_t)(strlen(inDirPath)+16));
	if (inFilePath==NULL){
		retval=5;
		goto end;
	}
	memcpy(inFilePath,inDirPath,strlen(inDirPath));
	memset(inFilePath+strlen(inDirPath),(int)'/',1);
	memset(inFilePath+strlen(inDirPath)+1,(int)'0',1);
	memset(inFilePath+strlen(inDirPath)+2,(int)'.',1);
	memset(inFilePath+strlen(inDirPath)+3,(int)'b',1);
	memset(inFilePath+strlen(inDirPath)+4,(int)'m',1);
	memset(inFilePath+strlen(inDirPath)+5,(int)'p',1);
	memset(inFilePath+strlen(inDirPath)+6,0,1);
	
	/* Allocates buffer */
	buffer=NULL;
	buffersize=0x3FFFFF;
	do{
		buffer=malloc((size_t) buffersize);
		if ((buffersize<=1024)&&(buffer==NULL)){
			retval=5;
			goto end;
		}
		if (buffer==NULL) buffersize>>=1;
	}while (buffer==NULL);
	buffersize-=(buffersize%1024);
	
	/* Adds thumbnails */
	bmp=fopen(inFilePath,"rb");
	if (bmp==NULL){
		retval=1;
		goto end;
	}
	freeint1=fseek(bmp,10,SEEK_SET);
	if (freeint1!=0){
		retval=2;
		goto end;
	}
	freesizet1=fread(&freeuint2,1,4,bmp);
	if (freesizet1!=4){
		retval=2;
		goto end;
	}
	freeint1=fseek(bmp,34,SEEK_SET);
	if (freeint1!=0){
		retval=2;
		goto end;
	}
	freesizet1=fread(&freeuint1,1,4,bmp);
	if (freesizet1!=4){
		retval=2;
		goto end;
	}
	if (freeuint1>0x80000){
		retval=28;
		goto end;
	}
	freeint1=fseek(bmp,(long int) freeuint2,SEEK_SET);
	if (freeint1!=0){
		retval=2;
		goto end;
	}
	while (freeuint1>0){
		/* Determines size of chunk */
		if (buffersize<freeuint1) freeuint2=buffersize;
		else freeuint2=freeuint1;
		/* Reads chunk into buffer */
		freesizet1=fread(buffer,1,(size_t)freeuint2,bmp);
		if (freesizet1!=(size_t)freeuint2){
			retval=2;
			goto end;
		}
		/* Writes chunk to output */
		freesizet1=fwrite(buffer,1,(size_t)freeuint2,wad);
		if (freesizet1!=(size_t)freeuint2){
			retval=4;
			goto end;
		}
		freeuint1-=freeuint2;
	}
	curpos=(unsigned INT32) ftell(wad);
	for(;curpos<0x80000;++curpos){
		freeint1=fputc(0xCD,wad);
		if (freeint1!=0xCD){
			retval=4;
			goto end;
		}
	}
	
	/* Adds images */
	for (curimage=1;bmp!=NULL;++curimage){
		/* Closes previous input file */
		freeint1=fclose(bmp);
		if (freeint1!=0){
			retval=2;
			goto end;
		}else{
			bmp=NULL;
		}
		
		/* Opens input file */
		inttostring((inFilePath+strlen(inDirPath)+1),curimage,10,1);
		freeint1=(int)strlen(inFilePath);
		memset(inFilePath+freeint1,(int)'.',1);
		memset(inFilePath+freeint1+1,(int)'b',1);
		memset(inFilePath+freeint1+2,(int)'m',1);
		memset(inFilePath+freeint1+3,(int)'p',1);
		memset(inFilePath+freeint1+4,0,1);
		bmp=fopen(inFilePath,"rb");
		if (bmp==NULL) break;
		
		/* Writes the image to the WAD-file */
		freeint1=fseek(bmp,0,SEEK_END);
		if (freeint1!=0){
			retval=2;
			goto end;
		}
		freeuint1=(unsigned INT32)(ftell(bmp));
		freeint1=fseek(bmp,6,SEEK_SET);
		if (freeint1!=0){
			retval=2;
			goto end;
		}
		freeint1=fputc(66,wad);
		if (freeint1!=66){
			retval=4;
			goto end;
		}
		freeint1=fputc(77,wad);
		if (freeint1!=77){
			retval=4;
			goto end;
		}
		freesizet1=fwrite(&freeuint1,1,4,wad);
		if (freesizet1!=4){
			retval=4;
			goto end;
		}
		freeuint1-=6;
		while (freeuint1>0){
			/* Determines size of chunk */
			if (buffersize<freeuint1) freeuint2=buffersize;
			else freeuint2=freeuint1;
			
			/* Reads chunk into buffer */
			freesizet1=fread(buffer,1,(size_t)freeuint2,bmp);
			if (freesizet1!=(size_t)freeuint2){
				retval=2;
				goto end;
			}
			
			/* Writes chunk to output */
			freesizet1=fwrite(buffer,1,(size_t)freeuint2,wad);
			if (freesizet1!=(size_t)freeuint2){
				retval=4;
				goto end;
			}
			freeuint1-=freeuint2;
		}
		
		/* Fills space until the next image */
		curpos=(unsigned INT32)ftell(wad);
		for(;(curpos%2048)!=0;++curpos){
			freeint1=fputc(0xCD,wad);
			if (freeint1!=0xCD){
				retval=4;
				goto end;
			}
		}
	}
	
	/* Closes files, frees buffer and returns function */
end:
	if (buffer!=NULL) free(buffer);
	if (inFilePath!=NULL) free(inFilePath);
	if (bmp!=NULL){
		freeint1=fclose(bmp);
		if (freeint1!=0) retval=2;
	}
	if (wad!=NULL){
		freeint1=fclose(wad);
		if (freeint1!=0) retval=4;
	}
	return retval;
}
