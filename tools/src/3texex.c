/* Tomb Raider III Texture Extractor */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* 8-bit integer */
#define INT8 signed char

/* 16-bit integer */
#define INT16 signed short int

/* 32-bit integer */
#define INT32 signed int

/* 8-bit unsinged integer */
#define INTU8 unsigned char

/* 16-bit unsigned integer */
#define INTU16 unsigned short int

/* 32-bit unsigned integer */
#define INTU32 unsigned int

/* 16-bit integer for printing */
#define PRINT16 "%i"

/* 32-bit integer for printing */
#define PRINT32 "%i"

/* 16-bit unsigned integer for printing */
#define PRINTU16 "%u"

/* 32-bit unsigned integer for printing */
#define PRINTU32 "%u"

/* Reverses a 16-bit integer */
#define reverse16(in) ((((INTU16) (in & 0xFF00)) >> 8) | \
                       (((INTU16) (in & 0x00FF)) << 8));

/* Reverses a 32-bit integer */
#define reverse32(in) ((((INTU32) (in & 0xFF000000)) >> 24) | \
                       (((INTU32) (in & 0x00FF0000)) >> 8)  | \
                       (((INTU32) (in & 0x0000FF00)) << 8)  | \
                       (((INTU32) (in & 0x000000FF)) << 24));

static int bmpheader(FILE *bmp, INTU32 size, INTU32 imageOffset, INTU32 width,
                     INTU32 height, INTU16 bpp, INTU32 compression,
                     INTU32 imageSize, INTU32 xres, INTU32 yres,
                     INTU32 colours, INTU32 impColours);
static int verticalFlip(char *image, size_t width, size_t height);

int main(int argc, char *argv[])
{
	/* Variable Declarations */
	FILE *pcfile = NULL;
	FILE *bmpfile = NULL;
	INTU8 *bmp = NULL;
	long int pcOffset = 0l;
	long int bmpOffset = 0l;
	int seekval = 0;
	size_t readval = (size_t) 0;
	size_t writeval = (size_t) 0;
	int retval = 0;
	INTU32 numTexTiles = 0x00000000;

	/* Checks number of command-line parameters */
	if (argc < 3)
	{
		printf("Usage: ./3texex [Level] [BMP-File]\n");
		goto end;
	}
	
	/* Opens the PC-file */
	pcfile = fopen(argv[1], "rb");
	if (pcfile == NULL)
	{
		printf("Can't open %s\n", argv[1]);
		goto end;
	}

	/* Reads NumTexTiles */
	pcOffset = 1796l;
	seekval = fseek(pcfile, pcOffset, SEEK_SET);
	if (seekval != 0)
	{
		printf("Can't read from %s\n", argv[1]);
		goto end;
	}
	readval = fread(&numTexTiles, (size_t) 1, (size_t) 4, pcfile);
	if (readval != (size_t) 4)
	{
		printf("Can't read from %s\n", argv[1]);
		goto end;
	}
#ifdef __BIG_ENDIAN__
	numTexTiles = reverse32(numTexTiles);
#endif
	numTexTiles *= 0x00010000;
	pcOffset += 4l;
	pcOffset += (long int) numTexTiles;
	
	/* Allocates space for image data in memory */
	numTexTiles *= 0x00000002;
	bmp = malloc((size_t) numTexTiles);
	if (bmp == NULL)
	{
		printf("Memory can't be allocated\n");
		goto end;
	}

	/* Reads the image data into memory */
	seekval = fseek(pcfile, pcOffset, SEEK_SET);
	if (seekval != 0)
	{
		printf("Can't read from %s\n", argv[1]);
		goto end;
	}
	readval = fread(bmp, (size_t) 1, (size_t) numTexTiles, pcfile);
	if (readval != (size_t) numTexTiles)
	{
		printf("Can't read from %s\n", argv[1]);
		goto end;
	}

	/* Opens the bmp file */
	bmpfile = fopen(argv[2], "wb");
	if (bmpfile == NULL)
	{
		printf("%s couldn't be opened\n", argv[2]);
		goto end;
	}
	
	/* Vertically flips the image */
	retval = verticalFlip(bmp, (size_t) 512, (size_t) (numTexTiles >> 9U));
	if (retval != 0)
	{
		printf("verticalFlip failed\n");
		goto end;
	}
	
	/* Writes the BMP-header */
	retval = bmpheader(bmpfile, (numTexTiles + 0x00000036), 0x00000036,
	                   0x00000100, (numTexTiles >> 9U), 0x0010, 0x00000000,
	                   numTexTiles, 0x00000000, 0x00000000,
	                   0x00000000, 0x00000000);
	if (retval != 0)
	{
		printf("bmpheader failed\n");
		goto end;
	}

	/* Goes to the offset to write image data to */
	seekval = fseek(bmpfile, 54l, SEEK_SET);
	if (seekval != 0)
	{
		printf("Couldn't move to the image data offset\n");
		goto end;
	}
	
	/* Writes the image data */
	writeval = fwrite(bmp, (size_t) 1, (size_t) numTexTiles, bmpfile);
	if (writeval != (size_t) numTexTiles)
	{
		printf("Couldn't write output data\n");
		goto end;
	}

end:
	if (pcfile != NULL)
	{
		(void) fclose(pcfile);
	}
	if (bmpfile != NULL)
	{
		(void) fclose(bmpfile);
	}
	if (bmp != NULL)
	{
		free(bmp);
	}
	return 0;
}

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
static int bmpheader(FILE *bmp, INTU32 size, INTU32 imageOffset, INTU32 width,
                     INTU32 height, INTU16 bpp, INTU32 compression,
                     INTU32 imageSize, INTU32 xres, INTU32 yres,
                     INTU32 colours, INTU32 impColours)
{
	/* Variable Declarations */
	int fputret; /* Return value of fputc */
	int fseekret; /* Return value of fseek */
	size_t fwritret; /* Return value of fwrite */

	/* Checks bmp file */
	if (bmp == NULL)
	{
		return 1;
	}

	/* Writes signature */
	fseekret = fseek(bmp, 0, SEEK_SET);
	if (fseekret != 0)
	{
		return 2;
	}
	fputret = fputc(66, bmp);
	if (fputret != 66)
	{
		return 2;
	}
	fputret = fputc(77, bmp);
	if (fputret != 77)
	{
		return 2;
	}

	/* Writes size */
	fwritret = fwrite(&size, 1, 4, bmp);
	if (fwritret != 4)
	{
		return 2;
	}

	/* Writes reserved bytes */
	fputret = fputc(0, bmp);
	if (fputret != 0)
	{
		return 2;
	}
	fputret = fputc(0, bmp);
	if (fputret != 0)
	{
		return 2;
	}
	fputret = fputc(0, bmp);
	if (fputret != 0)
	{
		return 2;
	}
	fputret = fputc(0, bmp);
	if (fputret != 0)
	{
		return 2;
	}

	/* Writes imageOffset */
	fwritret = fwrite(&imageOffset, 1, 4, bmp);
	if (fwritret != 4)
	{
		return 2;
	}

	/* Writes the size of BitmapInfoHeader */
	fputret = fputc(40, bmp);
	if (fputret != 40)
	{
		return 2;
	}
	fputret = fputc(0, bmp);
	if (fputret != 0)
	{
		return 2;
	}
	fputret = fputc(0, bmp);
	if (fputret != 0)
	{
		return 2;
	}
	fputret = fputc(0, bmp);
	if (fputret != 0)
	{
		return 2;
	}

	/* Writes width */
	fwritret = fwrite(&width, 1, 4, bmp);
	if (fwritret != 4)
	{
		return 2;
	}

	/* Writes height */
	fwritret = fwrite(&height, 1, 4, bmp);
	if (fwritret != 4)
	{
		return 2;
	}

	/* Writes reserved bytes */
	fputret = fputc(1, bmp);
	if (fputret != 1)
	{
		return 2;
	}
	fputret = fputc(0, bmp);
	if (fputret != 0)
	{
		return 2;
	}

	/* Writes bpp */
	fwritret = fwrite(&bpp, 1, 2, bmp);
	if (fwritret != 2)
	{
		return 2;
	}

	/* Writes compression */
	fwritret = fwrite(&compression, 1, 4, bmp);
	if (fwritret != 4)
	{
		return 2;
	}

	/* Writes imageSize */
	fwritret = fwrite(&imageSize, 1, 4, bmp);
	if (fwritret != 4)
	{
		return 2;
	}

	/* Writes horizontal resolution */
	fwritret = fwrite(&xres, 1, 4, bmp);
	if (fwritret != 4)
	{
		return 2;
	}

	/* Writes vertical resolution */
	fwritret = fwrite(&yres, 1, 4, bmp);
	if (fwritret != 4)
	{
		return 2;
	}

	/* Writes number of colours */
	fwritret = fwrite(&colours, 1, 4, bmp);
	if (fwritret != 4)
	{
		return 2;
	}

	/* Writes number of important colours */
	fwritret = fwrite(&impColours, 1, 4, bmp);
	if (fwritret != 4)
	{
		return 2;
	}

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
static int verticalFlip(char *image, size_t width, size_t height)
{
	/* Variable Declarations */
	char *buffer = NULL;
	char *lowrow = NULL;
	char *highrow = NULL;

	/* Creates buffer the size of one line */
	buffer = malloc(width);
	if (buffer == NULL)
	{
		return 1;
	}

	/* Flips the image */
	lowrow = image;
	highrow = (image + (width * (height - 1)));
	while (highrow > lowrow)
	{
		memmove(buffer, lowrow, width);
		memmove(lowrow, highrow, width);
		memmove(highrow, buffer, width);
		lowrow += width;
		highrow -= width;
	}

	/* Returns the function */
	if (buffer != NULL)
	{
		free(buffer);
	}
	return 0;
}
