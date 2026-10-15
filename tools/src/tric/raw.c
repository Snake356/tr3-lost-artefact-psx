/*
 * Library for Converting between RAW (Tomb Raider) and BMP
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
#include "dernc2.h"
#include "rnc2.h"
#include "raw.h"

/*
 * Function that converts RAW to BMP
 * Parameters:
 *   inFilePath = Path to input RAW-File
 *   outDirPath = Path to output BMP-File
 *   type = Type of RAW-file (1=tr1psx, 2=tr2psx/tr3psx,
 *            3=TR1 PSX Beta (04-09-1996), 4=TR2 PSX Beta (16-06-1997),
 *            5=TR4 PC Beta (15-09-1999), 6=TRLE Mac)
 * Return values:
 *   0 = Everything went well
 *   1 = Input file could not be opened
 *   2 = Input file could not be read from
 *   3 = Output file could not be opened
 *   4 = Output file could not be written to
 *   5 = Buffer could not be allocated
 *  15 = Temporary file could not be created
 *  16 = Temporary file could not be read from
 *  17 = Temporary file could not be written to
 *  18-26 = RNC2 decompression failed
 */
int rawtobmp(char *inFilePath, char *outFilePath, int type){
	/* Variable Declarations */
	FILE *raw=NULL; /* Input RAW-file */
	FILE *bmp=NULL; /* Output BMP-file */
	FILE *tmp=NULL; /* Temporary-file */
	char *buffer=NULL; /* Buffer for image data */
	INT32 tmpsize; /* Size of the temporary file */
	INT32 tmppos; /* Current position in temporary file */
	unsigned INT32 buffersize; /* Size of the buffer in bytes */
	unsigned INT32 bufferpos; /* Current position in buffer */
	unsigned INT16 colour; /* Current colour */
	INT32 rawpos; /* Current position in input RAW-file */
	INT32 rawsize; /* Size of input file in bytes */
	unsigned INT32 linewidth; /* Width of a line */
	int freeint1, freeint2; /* Integers for general use */
	size_t freesizet1; /* size_t for general use */
	int retval=0; /* Return value of this function */
	
	/* Opens input RAW-file */
	raw=fopen(inFilePath,"rb");
	if (raw==NULL){
		retval=1;
		goto end;
	}
	
	/* Determines rawsize */
	freeint1=fseek(raw,0,SEEK_END);
	if (freeint1!=0){
		retval=2;
		goto end;
	}
	rawsize=ftell(raw);
	
	/* Checks for type */
	switch (type){
		case 1:
		case 2: /* Tomb Raider 1-3 on Sony Playstation */
			/* Makes temporary file */
			tmp=tmpfile();
			if (tmp==NULL){
				retval=15;
				goto end;
			}
			
			/* Decompresses the RAW-image */
			freeint1=dernc2(raw,tmp,((type==1)?18:0));
			if (freeint1!=0){
				retval=freeint1+17;
				goto end;
			}
			
			/* Determines the size of decompressed raw data */
			freeint1=fseek(tmp,0,SEEK_END);
			if (freeint1!=0){
				retval=16;
				goto end;
			}
			tmpsize=(INT32) ftell(tmp);
			
			/* Determines width of a single line (in bytes) */
			linewidth=(unsigned INT32) (tmpsize/256);
			
			/* Allocates buffer */
			buffer=NULL;
			buffersize=0x3FFFF;
			do{
				buffer=malloc((size_t) buffersize);
				if ((buffersize<=linewidth)&&(buffer==NULL)){
					retval=5;
					goto end;
				}
				if (buffer==NULL) buffersize>>=1;
			}while (buffer==NULL);
			buffersize-=(buffersize%linewidth);
			
			/* Opens output BMP-file */
			bmp=fopen(outFilePath,"wb");
			if (bmp==NULL){
				retval=3;
				goto end;
			}
			
			/* Writes bitmap header */
			freeint1=bmpheader(bmp,(unsigned INT32)tmpsize+54,54,
			                   (unsigned INT32)(linewidth/2),256,16,0,
			                   (unsigned INT32)tmpsize,0,0,0,0);
			if (freeint1!=0){
				retval=4;
				goto end;
			}
			
			/* Processes image data (backwards) in chunks */
			tmppos=tmpsize;
			while (tmppos>0){
				/* Adjusts the size of the final chunk if needed */
				if ((unsigned INT32)tmppos<buffersize) 
					buffersize=(unsigned INT32)tmppos;
				
				/* Reads chunk into buffer */
				freeint1=fseek(tmp,(INT32)(tmppos-buffersize),SEEK_SET);
				if (freeint1!=0){
					retval=2;
					goto end;
				}
				freesizet1=fread(buffer,1,(size_t) buffersize,tmp);
				if (freesizet1!=(size_t) buffersize){
					retval=2;
					goto end;
				}
				
				/* Flips chunk vertically */
				freeint2=verticalFlip(buffer,(size_t)linewidth,(size_t)
						(buffersize/linewidth));
				if (freeint2!=0){
					retval=5;
					goto end;
				}
				
				/* Flips red and blue channels */
				for (bufferpos=0;bufferpos<buffersize;bufferpos+=2){
					memmove(&colour,buffer+bufferpos,2);
					colour=(unsigned INT16)
						((unsigned INT16)(colour&0x83e0)+
						((unsigned INT16)(colour&0x7c00)>>10)+
						((unsigned INT16)(colour&0x1f)<<10));
					memmove(buffer+bufferpos,&colour,2);
				}
				
				/* Writes chunk to output BMP-file */
				freesizet1=fwrite(buffer,1,(size_t) buffersize,bmp);
				if (freesizet1!=(size_t) buffersize){
					retval=4;
					goto end;
				}
				
				tmppos-=buffersize;
			}
			break;
		case 3: /* Tomb Raider 1 Playstation Beta (04-09-1996) */
			/* Allocates buffer */
			buffer=NULL;
			buffersize=0x3FFFF;
			do{
				buffer=malloc((size_t) buffersize);
				if ((buffersize<=1280)&&(buffer==NULL)){
					retval=5;
					goto end;
				}
				if (buffer==NULL) buffersize>>=1;
			}while (buffer==NULL);
			buffersize-=(buffersize%1280);
			
			/* Opens output BMP-file */
			bmp=fopen(outFilePath,"wb");
			if (bmp==NULL){
				retval=3;
				goto end;
			}
			
			/* Writes bitmap header */
			freeint1=bmpheader(bmp,(unsigned INT32)rawsize+54,54,
			                   (unsigned INT32)(rawsize/512),256,16,0,
			                   (unsigned INT32)rawsize,0,0,0,0);
			if (freeint1!=0){
				retval=4;
				goto end;
			}
			
			/* Processes image data (backwards) in chunks */
			rawpos=rawsize;
			while (rawpos>0){
				/* Adjusts the size of the final chunk if needed */
				if ((unsigned INT32)rawpos<buffersize) 
					buffersize=(unsigned INT32)rawpos;
				
				/* Reads chunk into buffer */
				freeint1=fseek(raw,(INT32) (rawpos-buffersize),SEEK_SET);
				if (freeint1!=0){
					retval=2;
					goto end;
				}
				freesizet1=fread(buffer,1,(size_t) buffersize,raw);
				if (freesizet1!=(size_t) buffersize){
					retval=2;
					goto end;
				}
				
				/* Flips chunk vertically */
				freeint1=((rawsize==327680)?1280:640);
				freeint2=verticalFlip(buffer,(size_t)freeint1,(size_t)
						(buffersize/freeint1));
				if (freeint2!=0){
					retval=5;
					goto end;
				}
				
				/* Flips red and blue channels */
				for (bufferpos=0;bufferpos<buffersize;bufferpos+=2){
					memmove(&colour,buffer+bufferpos,2);
					colour=(unsigned INT16)
						((unsigned INT16)(colour&0x83e0)+
						((unsigned INT16)(colour&0x7c00)>>10)+
						((unsigned INT16)(colour&0x1f)<<10));
					memmove(buffer+bufferpos,&colour,2);
				}
				
				/* Writes chunk to output BMP-file */
				freesizet1=fwrite(buffer,1,(size_t) buffersize,bmp);
				if (freesizet1!=(size_t) buffersize){
					retval=4;
					goto end;
				}
				
				rawpos-=buffersize;
			}
			break;
		case 4: /* Tomb Raider 2 Playstation Beta (16-06-1997) */
			/* Allocates buffer */
			buffer=NULL;
			buffersize=0x3FFFF;
			do{
				buffer=malloc((size_t) buffersize);
				if ((buffersize<=768)&&(buffer==NULL)){
					retval=5;
					goto end;
				}
				if (buffer==NULL) buffersize>>=1;
			}while (buffer==NULL);
			buffersize-=(buffersize%768);
			
			/* Opens output BMP-file */
			bmp=fopen(outFilePath,"wb");
			if (bmp==NULL){
				retval=3;
				goto end;
			}
			
			/* Writes bitmap header */
			rawsize-=44;
			freeint1=bmpheader(bmp,(unsigned INT32)rawsize+54,54,384,
			                   (unsigned INT32)(rawsize/768),16,0,
			                   (unsigned INT32)rawsize,0,0,0,0);
			if (freeint1!=0){
				retval=4;
				goto end;
			}
			
			/* Processes image data in chunks */
			for (rawpos=0;rawpos<rawsize;){
				/* Adjusts the size of the final chunk if needed */
				if ((unsigned INT32)(rawsize-rawpos)<buffersize) 
					buffersize=(unsigned INT32)(rawsize-rawpos);
				
				/* Reads chunk into buffer */
				freeint1=fseek(raw,(INT32)(rawpos+18),SEEK_SET);
				if (freeint1!=0){
					retval=2;
					goto end;
				}
				freesizet1=fread(buffer,1,(size_t) buffersize,raw);
				if (freesizet1!=(size_t) buffersize){
					retval=2;
					goto end;
				}
				
				/* Writes chunk to output BMP-file */
				freesizet1=fwrite(buffer,1,(size_t) buffersize,bmp);
				if (freesizet1!=(size_t) buffersize){
					retval=4;
					goto end;
				}
				
				rawpos+=buffersize;
			}
			break;
		case 5: /* Tomb Raider 4 PC Beta (15-09-1999) */
			/* Allocates buffer */
			buffer=NULL;
			buffersize=0x3FFFF;
			do{
				buffer=malloc((size_t) buffersize);
				if ((buffersize<=1536)&&(buffer==NULL)){
					retval=5;
					goto end;
				}
				if (buffer==NULL) buffersize>>=1;
			}while (buffer==NULL);
			buffersize-=(buffersize%1536);
			
			/* Opens output BMP-file */
			bmp=fopen(outFilePath,"wb");
			if (bmp==NULL){
				retval=3;
				goto end;
			}
			
			/* Writes bitmap header */
			freeint1=bmpheader(bmp,(unsigned INT32)rawsize+54,54,512,
			                   (unsigned INT32)(rawsize/1536),24,0,
			                   (unsigned INT32)rawsize,0,0,0,0);
			if (freeint1!=0){
				retval=4;
				goto end;
			}
			
			/* Processes image data (backwards) in chunks */
			rawpos=rawsize;
			while (rawpos>0){
				/* Adjusts the size of the final chunk if needed */
				if ((unsigned INT32)rawpos<buffersize) 
					buffersize=(unsigned INT32)rawpos;
				
				/* Reads chunk into buffer */
				freeint1=fseek(raw,(INT32)(rawpos-buffersize),SEEK_SET);
				if (freeint1!=0){
					retval=2;
					goto end;
				}
				freesizet1=fread(buffer,1,(size_t) buffersize,raw);
				if (freesizet1!=(size_t) buffersize){
					retval=2;
					goto end;
				}
				
				/* Flips chunk vertically */
				freeint1=verticalFlip(buffer,1536,(size_t)
				                      (buffersize/1536));
				if (freeint1!=0){
					retval=5;
					goto end;
				}
				
				/* Flips red and blue channels */
				for (bufferpos=0;bufferpos<buffersize;bufferpos+=3){
					memmove(&freeint1,buffer+bufferpos,1);
					memmove(buffer+bufferpos,buffer+bufferpos+2,1);
					memmove(buffer+bufferpos+2,&freeint1,1);
				}
				
				/* Writes chunk to output BMP-file */
				freesizet1=fwrite(buffer,1,(size_t) buffersize,bmp);
				if (freesizet1!=(size_t) buffersize){
					retval=4;
					goto end;
				}
				
				rawpos-=buffersize;
			}
			break;
		case 6: /* Tomb Raider Level Editor (Mac Version) */
			/* Allocates buffer */
			buffer=NULL;
			buffersize=0x3FFFF;
			do{
				buffer=malloc((size_t) buffersize);
				if ((buffersize<=1280)&&(buffer==NULL)){
					retval=5;
					goto end;
				}
				if (buffer==NULL) buffersize>>=1;
			}while (buffer==NULL);
			buffersize-=(buffersize%1280);
			
			/* Opens output BMP-file */
			bmp=fopen(outFilePath,"wb");
			if (bmp==NULL){
				retval=3;
				goto end;
			}
			
			/* Writes bitmap header */
			freeint1=bmpheader(bmp,(unsigned INT32)rawsize+54,54,640,
			                   (unsigned INT32)(rawsize/1280),16,0,
			                   (unsigned INT32)rawsize,0,0,0,0);
			if (freeint1!=0){
				retval=4;
				goto end;
			}
			
			/* Processes image data (backwards) in chunks */
			rawpos=rawsize;
			while (rawpos>0){
				/* Adjusts the size of the final chunk if needed */
				if ((unsigned INT32)rawpos<buffersize) 
					buffersize=(unsigned INT32)rawpos;
				
				/* Reads chunk into buffer */
				freeint1=fseek(raw,(INT32)(rawpos-buffersize),SEEK_SET);
				if (freeint1!=0){
					retval=2;
					goto end;
				}
				freesizet1=fread(buffer,1,(size_t) buffersize,raw);
				if (freesizet1!=(size_t) buffersize){
					retval=2;
					goto end;
				}
				/* Flips chunk vertically */
				freeint1=verticalFlip(buffer,1280,(size_t)
				                      (buffersize/1280));
				if (freeint1!=0){
					retval=5;
					goto end;
				}
				
				/* Converts colours from Big-Endian to Little-endian */
				for (bufferpos=0;bufferpos<buffersize;bufferpos+=2){
					memmove(&colour,buffer+bufferpos,2);
					colour=(unsigned INT16)
							((((unsigned INT16)(colour&0x00FF))<<8)|
							(((unsigned INT16)(colour&0xFF00))>>8));
					memmove(buffer+bufferpos,&colour,2);
				}
				
				/* Writes chunk to output BMP-file */
				freesizet1=fwrite(buffer,1,(size_t) buffersize,bmp);
				if (freesizet1!=(size_t) buffersize){
					retval=4;
					goto end;
				}
				
				rawpos-=buffersize;
			}
			break;
	}
	
end:/* Closes files, frees buffer and returns function */
	if (buffer!=NULL) free(buffer);
	if (raw!=NULL){
		freeint1=fclose(raw);
		if (freeint1!=0) retval=2;
	}
	if (bmp!=NULL){
		freeint1=fclose(bmp);
		if (freeint1!=0) retval=4;
	}
	if (tmp!=NULL){
		freeint1=fclose(tmp);
		if (freeint1!=0) retval=17;
	}
	return retval;
}

/*
 * Function that converts BMP to RAW
 * Parameters:
 *   inFilePath = Path to input BMP-File
 *   outDirPath = Path to output RAW-File
 *   type = Type of RAW-file (1=tr1psx, 2=tr2psx/tr3psx,
 *            3=TR1 PSX Beta (04-09-1996), 4=TR2 PSX Beta (16-06-1997),
 *            5=TR4 PC Beta (15-09-1999), 6=TRLE Mac)
 * Return values:
 *   0 = Everything went well
 *   1 = Input file could not be opened
 *   2 = Input file could not be read from
 *   3 = Output file could not be opened
 *   4 = Output file could not be written to
 *   5 = Buffer could not be allocated
 *   7 = Input BMP is not valid
 *   8 = Input bmp is not 16 bbp
 *  31 = Input bmp is not 24 bpp
 */
int bmptoraw(char *inFilePath, char *outFilePath, int type){
	/* Variable Declarations */
	FILE *bmp=NULL; /* Input bmp-file */
	FILE *raw=NULL; /* Output raw-file */
	FILE *tmp=NULL; /* Temporary file */
	char *buffer=NULL; /* Buffer */
	unsigned INT32 buffersize=0; /* Size of buffer in bytes */
	unsigned INT32 bufferpos=0; /* Current position in buffer */
	unsigned INT32 width=0; /* Image width in pixels */
	unsigned INT32 height=0; /* Image height in pixels */
	unsigned INT32 bmpsize=0; /* Length of BMP-file */
	unsigned INT32 bmppos=0; /* Current position in BMP-file */
	unsigned INT32 imagestart=0; /* Offset of image data in BMP-file */
	unsigned INT32 imageend=0; /* End of image data in BMP-file */
	unsigned INT16 bpp=0; /* Number of bits per pixel in BMP-file */
	unsigned INT16 colour=0; /* Current colour */
	size_t freesizet1; /* size_t for general use */
	int freeint1, freeint2; /* Integer for general use */
	int retval=0; /* Return value for this function */
	
	/* Opens input bmp-file */
	bmp=fopen(inFilePath,"rb");
	if (bmp==NULL){
		retval=1;
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
	freesizet1=fread(&bpp,1,2,bmp);
	if (freesizet1!=2){
		retval=2;
		goto end;
	}
	
	/* Calculates imageend */
	imageend=(unsigned INT32) (bpp/8);
	imageend*=width;
	imageend*=height;
	imageend+=imagestart;
	
	/* Checks whether image data is complete */
	if (imageend>bmpsize){
		retval=7;
		goto end;
	}
	
	/* Checks for type */
	switch (type){
		case 1:
		case 2: /* Tomb Raider 1-3 on Sony Playstation */
			/* Checks bbp */
			if (bpp!=16){
				retval=8;
				goto end;
			}
			
			/* Makes temporary file */
			tmp=tmpfile();
			if (tmp==NULL){
				retval=15;
				goto end;
			}
			
			/* Allocates buffer */
			width*=2;
			buffer=NULL;
			buffersize=0x3FFFF;
			do{
				buffer=malloc((size_t) buffersize);
				if ((buffersize<=width)&&(buffer==NULL)){
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
				if (((unsigned INT32)(bmppos-imagestart))<buffersize)
					buffersize=((unsigned INT32)(bmppos-imagestart));
				
				/* Reads chunk into buffer */
				freeint1=fseek(bmp,(INT32)(bmppos-buffersize),SEEK_SET);
				if (freeint1!=0){
					retval=2;
					goto end;
				}
				freesizet1=fread(buffer,1,(size_t) buffersize,bmp);
				if (freesizet1!=(size_t) buffersize){
					retval=2;
					goto end;
				}
				
				/* Flips chunk vertically */
				freeint1=verticalFlip(buffer,(size_t)width,(size_t)
				                      (buffersize/width));
				if (freeint1!=0){
					retval=5;
					goto end;
				}
				
				/* Flips red and blue channels */
				for (bufferpos=0;bufferpos<buffersize;bufferpos+=2){
					memmove(&colour,buffer+bufferpos,2);
					colour=(unsigned INT16)
						((unsigned INT16)(colour&0x83e0)+
						((unsigned INT16)(colour&0x7c00)>>10)+
						((unsigned INT16)(colour&0x1f)<<10));
					memmove(buffer+bufferpos,&colour,2);
				}
				
				/* Writes chunk to output temporary file */
				freesizet1=fwrite(buffer,1,(size_t) buffersize,tmp);
				if (freesizet1!=(size_t) buffersize){
					retval=4;
					goto end;
				}
				
				bmppos-=buffersize;
			}
			
			/* Frees buffer */
			free(buffer);
			buffer=NULL;
			
			/* Opens output RAW-file */
			raw=fopen(outFilePath,"wb");
			if (raw==NULL){
				retval=3;
				goto end;
			}
			
			/* Adjusts for tr1psx if necessary */
			if (type==1){
				freeint1=fputc(82,raw);
				if (freeint1!=82){
					retval=4;
					goto end;
				}
				freeint1=fputc(78,raw);
				if (freeint1!=78){
					retval=4;
					goto end;
				}
				freeint1=fputc(67,raw);
				if (freeint1!=67){
					retval=4;
					goto end;
				}
				freeint1=fputc(2,raw);
				if (freeint1!=2){
					retval=4;
					goto end;
				}
				for (freeint2=14;freeint2>0;--freeint2){
					freeint1=fputc(0,raw);
					if (freeint1!=0){
						retval=4;
						goto end;
					}
				}
			}
			
			/* Compresses RAW-file */
			freeint1=rnc2(tmp,raw);
			if (freeint1!=0){
				retval=freeint1;
				goto end;
			}
			
			break;
		case 3: /* Tomb Raider 1 Playstation Beta (04-09-1996) */
			/* Checks bbp */
			if (bpp!=16){
				retval=8;
				goto end;
			}
			
			/* Allocates buffer */
			width*=2;
			buffer=NULL;
			buffersize=0x3FFFF;
			do{
				buffer=malloc((size_t) buffersize);
				if ((buffersize<=width)&&(buffer==NULL)){
					retval=5;
					goto end;
				}
				if (buffer==NULL) buffersize>>=1;
			}while (buffer==NULL);
			buffersize-=(buffersize%width);
			
			/* Opens output RAW-file */
			raw=fopen(outFilePath,"wb");
			if (raw==NULL){
				retval=3;
				goto end;
			}
			
			/* Processes image data (backwards) in chunks */
			bmppos=imageend;
			while (bmppos>imagestart){
				/* Adjusts the size of the final chunk if needed */
				if (((unsigned INT32)(bmppos-imagestart))<buffersize)
					buffersize=((unsigned INT32)(bmppos-imagestart));
				
				/* Reads chunk into buffer */
				freeint1=fseek(bmp,(INT32)(bmppos-buffersize),SEEK_SET);
				if (freeint1!=0){
					retval=2;
					goto end;
				}
				freesizet1=fread(buffer,1,(size_t) buffersize,bmp);
				if (freesizet1!=(size_t) buffersize){
					retval=2;
					goto end;
				}
				
				/* Flips chunk vertically */
				freeint1=verticalFlip(buffer,(size_t)width,(size_t)
				                      (buffersize/width));
				if (freeint1!=0){
					retval=5;
					goto end;
				}
				
				/* Flips red and blue channels */
				for (bufferpos=0;bufferpos<buffersize;bufferpos+=2){
					memmove(&colour,buffer+bufferpos,2);
					colour=(unsigned INT16)
						((unsigned INT16)(colour&0x83e0)+
						((unsigned INT16)(colour&0x7c00)>>10)+
						((unsigned INT16)(colour&0x1f)<<10));
					memmove(buffer+bufferpos,&colour,2);
				}
				
				/* Writes chunk to output RAW-file */
				freesizet1=fwrite(buffer,1,(size_t) buffersize,raw);
				if (freesizet1!=(size_t) buffersize){
					retval=4;
					goto end;
				}
				
				bmppos-=buffersize;
			}
			break;
		case 4: /* Tomb Raider 2 Playstation Beta (16-06-1997) */
			/* Checks bbp */
			if (bpp!=16){
				retval=8;
				goto end;
			}
			
			/* Allocates buffer */
			width*=2;
			buffer=NULL;
			buffersize=0x3FFFF;
			do{
				buffer=malloc((size_t) buffersize);
				if (((buffersize<=width)||(buffersize<=26))&&
				    (buffer==NULL)){
					retval=5;
					goto end;
				}
				if (buffer==NULL) buffersize>>=1;
			}while (buffer==NULL);
			buffersize-=(buffersize%width);
			
			/* Opens output RAW-file */
			raw=fopen(outFilePath,"wb");
			if (raw==NULL){
				retval=3;
				goto end;
			}
			
			/* Writes header data */
			memset(buffer,0,15);
			memset(buffer+2,2,1);
			memset(buffer+12,128,1);
			memset(buffer+13,1,1);
			memset(buffer+15,1,1);
			memset(buffer+16,16,1);
			memset(buffer+17,1,1);
			freesizet1=fwrite(buffer,1,18,raw);
			if (freesizet1!=18){
				retval=4;
				goto end;
			}
			
			/* Processes image data in chunks */
			for (bmppos=imagestart;bmppos<imageend;){
				/* Adjusts the size of the final chunk if needed */
				if ((unsigned INT32)(bmpsize-bmppos)<buffersize) 
					buffersize=(unsigned INT32)(bmpsize-bmppos);
				
				/* Reads chunk into buffer */
				freeint1=fseek(bmp,(INT32)bmppos,SEEK_SET);
				if (freeint1!=0){
					retval=2;
					goto end;
				}
				freesizet1=fread(buffer,1,(size_t) buffersize,bmp);
				if (freesizet1!=(size_t) buffersize){
					retval=2;
					goto end;
				}
				
				/* Writes chunk to output RAW-file */
				freesizet1=fwrite(buffer,1,(size_t) buffersize,raw);
				if (freesizet1!=(size_t) buffersize){
					retval=4;
					goto end;
				}
				
				bmppos+=buffersize;
			}
			
			/* Writes suffix data */
			memset(buffer,0,25);
			memset(buffer+8,0x54,1);
			memset(buffer+9,0x52,1);
			memset(buffer+10,0x55,1);
			memset(buffer+11,0x45,1);
			memset(buffer+12,0x56,1);
			memset(buffer+13,0x49,1);
			memset(buffer+14,0x53,1);
			memset(buffer+15,0x49,1);
			memset(buffer+16,0x4F,1);
			memset(buffer+17,0x4E,1);
			memset(buffer+18,0x2D,1);
			memset(buffer+19,0x58,1);
			memset(buffer+20,0x46,1);
			memset(buffer+21,0x49,1);
			memset(buffer+22,0x4C,1);
			memset(buffer+23,0x45,1);
			memset(buffer+24,0x2E,1);
			freesizet1=fwrite(buffer,1,26,raw);
			if (freesizet1!=26){
				retval=4;
				goto end;
			}
			
			break;
		case 5: /* Tomb Raider 4 PC Beta (15-09-1999) */
			/* Checks bbp */
			if (bpp!=24){
				retval=31;
				goto end;
			}
			
			/* Allocates buffer */
			width*=3;
			buffer=NULL;
			buffersize=0x3FFFF;
			do{
				buffer=malloc((size_t) buffersize);
				if ((buffersize<=width)&&(buffer==NULL)){
					retval=5;
					goto end;
				}
				if (buffer==NULL) buffersize>>=1;
			}while (buffer==NULL);
			buffersize-=(buffersize%width);
			
			/* Opens output RAW-file */
			raw=fopen(outFilePath,"wb");
			if (raw==NULL){
				retval=3;
				goto end;
			}
			
			/* Processes image data (backwards) in chunks */
			bmppos=imageend;
			while (bmppos>imagestart){
				/* Adjusts the size of the final chunk if needed */
				if (((unsigned INT32)(bmppos-imagestart))<buffersize)
					buffersize=((unsigned INT32)(bmppos-imagestart));
				
				/* Reads chunk into buffer */
				freeint1=fseek(bmp,(INT32)(bmppos-buffersize),SEEK_SET);
				if (freeint1!=0){
					retval=2;
					goto end;
				}
				freesizet1=fread(buffer,1,(size_t) buffersize,bmp);
				if (freesizet1!=(size_t) buffersize){
					retval=2;
					goto end;
				}
				
				/* Flips chunk vertically */
				freeint1=verticalFlip(buffer,(size_t)width,(size_t)
				                      (buffersize/width));
				if (freeint1!=0){
					retval=5;
					goto end;
				}
				
				/* Flips red and blue channels */
				for (bufferpos=0;bufferpos<buffersize;bufferpos+=3){
					memmove(&freeint1,buffer+bufferpos,1);
					memmove(buffer+bufferpos,buffer+bufferpos+2,1);
					memmove(buffer+bufferpos+2,&freeint1,1);
				}
				
				/* Writes chunk to output RAW-file */
				freesizet1=fwrite(buffer,1,(size_t) buffersize,raw);
				if (freesizet1!=(size_t) buffersize){
					retval=4;
					goto end;
				}
				
				bmppos-=buffersize;
			}
			break;
		case 6: /* Tomb Raider Level Editor (Mac Version) */
			/* Checks bbp */
			if (bpp!=16){
				retval=8;
				goto end;
			}
			
			/* Allocates buffer */
			width*=2;
			buffer=NULL;
			buffersize=0x3FFFF;
			do{
				buffer=malloc((size_t) buffersize);
				if ((buffersize<=width)&&(buffer==NULL)){
					retval=5;
					goto end;
				}
				if (buffer==NULL) buffersize>>=1;
			}while (buffer==NULL);
			buffersize-=(buffersize%width);
			
			/* Opens output RAW-file */
			raw=fopen(outFilePath,"wb");
			if (raw==NULL){
				retval=3;
				goto end;
			}
			
			/* Processes image data (backwards) in chunks */
			bmppos=imageend;
			while (bmppos>imagestart){
				/* Adjusts the size of the final chunk if needed */
				if (((unsigned INT32)(bmppos-imagestart))<buffersize)
					buffersize=((unsigned INT32)(bmppos-imagestart));
				
				/* Reads chunk into buffer */
				freeint1=fseek(bmp,(INT32)(bmppos-buffersize),SEEK_SET);
				if (freeint1!=0){
					retval=2;
					goto end;
				}
				freesizet1=fread(buffer,1,(size_t) buffersize,bmp);
				if (freesizet1!=(size_t) buffersize){
					retval=2;
					goto end;
				}
				
				/* Flips chunk vertically */
				freeint1=verticalFlip(buffer,(size_t)width,(size_t)
				                      (buffersize/width));
				if (freeint1!=0){
					retval=5;
					goto end;
				}
				
				/* Converts colours from Little-Endian to Big-endian */
				for (bufferpos=0;bufferpos<buffersize;bufferpos+=2){
					memmove(&colour,buffer+bufferpos,2);
					colour=(unsigned INT16)
							((((unsigned INT16)(colour&0x00FF))<<8)|
							(((unsigned INT16)(colour&0xFF00))>>8));
					memmove(buffer+bufferpos,&colour,2);
				}
				
				/* Writes chunk to output RAW-file */
				freesizet1=fwrite(buffer,1,(size_t) buffersize,raw);
				if (freesizet1!=(size_t) buffersize){
					retval=4;
					goto end;
				}
				
				bmppos-=buffersize;
			}
			break;
	}
	
end:/* Closes files, frees buffer and returns function */
	if (buffer!=NULL) free(buffer);
	if (bmp!=NULL){
		freeint1=fclose(bmp);
		if (freeint1!=0) retval=2;
	}
	if (raw!=NULL){
		freeint1=fclose(raw);
		if (freeint1!=0) retval=4;
	}
	if (tmp!=NULL){
		freeint1=fclose(tmp);
		if (freeint1!=0) retval=17;
	}
	return retval;
}
