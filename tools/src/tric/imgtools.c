/*
 * Library that Contains some Functions Relating to Images
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

/*
 * Function that writes a bmp-header to a file
 * Parameters:
 *   bmp = BMP file to write the header to
 *   size = Total size of bitmap file
 *   imageOffset = Offset of image data
 *   width = Width of the image in pixels
 *   height = Height of the image in pixels
 *   bpp = Number of bits per pixel
 *   compression = Compression type (0=none, 1=RLE-8, 2=RLE-4)
 *   imageSize = Size of image data in bytes (including padding)
 *   xres = Horizontal resolution in pixels per meter
 *   yres = Vertical resolution in pixels per meter
 *   colours = Number of colours in the image
 *   impColours = Number of important colours
 * Return Values:
 *   0 = Everything went well
 *   1 = Bmp file not opened
 *   2 = Writing to bmp failed
 */
int bmpheader(FILE *bmp, unsigned INT32 size,
              unsigned INT32 imageOffset, unsigned INT32 width,
              unsigned INT32 height, unsigned INT16 bpp,
              unsigned INT32 compression,
              unsigned INT32 imageSize, unsigned INT32 xres,
              unsigned INT32 yres, unsigned INT32 colours,
              unsigned INT32 impColours){
	/* Variable Declarations */
	int fputret; /* Return value of fputc */
	int fseekret; /* Return value of fseek */
	size_t fwritret; /* Return value of fwrite */
	
	/* Checks bmp file */
	if (bmp==NULL) return 1;
	
	/* Writes signature */
	fseekret=fseek(bmp,0,SEEK_SET);
	if (fseekret!=0) return 2;
	fputret=fputc(66,bmp);
	if (fputret!=66) return 2;
	fputret=fputc(77,bmp);
	if (fputret!=77) return 2;
	
	/* Writes size */
	fwritret=fwrite(&size,1,4,bmp);
	if (fwritret!=4) return 2;
	
	/* Writes reserved bytes */
	fputret=fputc(0,bmp);
	if (fputret!=0) return 2;
	fputret=fputc(0,bmp);
	if (fputret!=0) return 2;
	fputret=fputc(0,bmp);
	if (fputret!=0) return 2;
	fputret=fputc(0,bmp);
	if (fputret!=0) return 2;
	
	/* Writes imageOffset */
	fwritret=fwrite(&imageOffset,1,4,bmp);
	if (fwritret!=4) return 2;
	
	/* Writes the size of BitmapInfoHeader */
	fputret=fputc(40,bmp);
	if (fputret!=40) return 2;
	fputret=fputc(0,bmp);
	if (fputret!=0) return 2;
	fputret=fputc(0,bmp);
	if (fputret!=0) return 2;
	fputret=fputc(0,bmp);
	if (fputret!=0) return 2;
	
	/* Writes width */
	fwritret=fwrite(&width,1,4,bmp);
	if (fwritret!=4) return 2;
	
	/* Writes height */
	fwritret=fwrite(&height,1,4,bmp);
	if (fwritret!=4) return 2;
	
	/* Writes reserved bytes */
	fputret=fputc(1,bmp);
	if (fputret!=1) return 2;
	fputret=fputc(0,bmp);
	if (fputret!=0) return 2;
	
	/* Writes bpp */
	fwritret=fwrite(&bpp,1,2,bmp);
	if (fwritret!=2) return 2;
	
	/* Writes compression */
	fwritret=fwrite(&compression,1,4,bmp);
	if (fwritret!=4) return 2;
	
	/* Writes imageSize */
	fwritret=fwrite(&imageSize,1,4,bmp);
	if (fwritret!=4) return 2;
	
	/* Writes horizontal resolution */
	fwritret=fwrite(&xres,1,4,bmp);
	if (fwritret!=4) return 2;
	
	/* Writes vertical resolution */
	fwritret=fwrite(&yres,1,4,bmp);
	if (fwritret!=4) return 2;
	
	/* Writes number of colours */
	fwritret=fwrite(&colours,1,4,bmp);
	if (fwritret!=4) return 2;
	
	/* Writes number of important colours */
	fwritret=fwrite(&impColours,1,4,bmp);
	if (fwritret!=4) return 2;
	
	/* Returns the function */
	return 0;
}

/*
 * Function that vertically flips bitmap data in memory.
 * Parameters:
 *   image = Location of the image file
 *   width = Length of a horizontal line of the image in bytes
 *   height = Height of the image in pixels
 * Return values:
 *   0 = Everything went well
 *   1 = Buffer could not be allocated
 */
int verticalFlip(char *image, size_t width, size_t height){
	/* Variable Declarations */
	char *buffer=NULL;
	char *lowrow=NULL;
	char *highrow=NULL;
	
	/* Creates buffer the size of one line */
	buffer=malloc(width);
	if (buffer==NULL) return 1;
	
	/* Flips the image */
	lowrow=image;
	highrow=(image+(width*(height-1)));
	while (highrow>lowrow){
		memmove(buffer,lowrow,width);
		memmove(lowrow,highrow,width);
		memmove(highrow,buffer,width);
		lowrow+=width;
		highrow-=width;
	}
	
	/* Returns the function */
	if (buffer!=NULL) free(buffer);
	return 0;
}
