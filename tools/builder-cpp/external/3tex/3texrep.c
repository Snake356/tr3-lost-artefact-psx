/* Tomb Raider III Texture Replacement Tool */

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

static int verticalFlip(char *image, size_t width, size_t height);

int main(int argc, char *argv[])
{
	/* Variable Declarations */
	FILE *pcfile = NULL;
	FILE *bmpfile = NULL;
	INTU8 *bmp = NULL;
	long int pcOffset = 0l;
	int seekval = 0;
	size_t readval = (size_t) 0;
	size_t writeval = (size_t) 0;
	int retval = 0;
	INTU32 numTexTiles = 0x00000000;
	INTU32 imageOffset = 0x00000000;
	INTU32 imageSize = 0x00000000;
	INTU32 height = 0x00000000;

	/* Checks number of command-line parameters */
	if (argc < 3)
	{
		printf("Usage: ./3texrep [Level] [BMP-File]\n");
		goto end;
	}
	
	/* Opens the PC-file */
	pcfile = fopen(argv[1], "r+b");
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
	
	/* Opens the bmp file */
	bmpfile = fopen(argv[2], "rb");
	if (bmpfile == NULL)
	{
		printf("%s couldn't be opened\n", argv[2]);
		goto end;
	}
	
	/* Reads imageOffset */
	seekval = fseek(bmpfile, 10l, SEEK_SET);
	if (seekval != 0)
	{
		printf("Couldn't load imageOffset\n");
		goto end;
	}
	readval = fread(&imageOffset, (size_t) 1, (size_t) 4, bmpfile);
	if (readval != (size_t) 4)
	{
		printf("Couldn't read imageOffset\n");
		goto end;
	}
#ifdef __BIG_ENDIAN__
	imageOffset = reverse32(imageOffset);
#endif
	
	/* Reads height */
	seekval = fseek(bmpfile, 22l, SEEK_SET);
	if (seekval != 0)
	{
		printf("Couldn't load height\n");
		goto end;
	}
	readval = fread(&height, (size_t) 1, (size_t) 4, bmpfile);
	if (readval != (size_t) 4)
	{
		printf("Couldn't read height\n");
		goto end;
	}
#ifdef __BIG_ENDIAN__
	height = reverse32(height);
#endif
	
	/* Allocates space for image data in memory */
	imageSize =  (height * 0x00000200);
	bmp = malloc((size_t) imageSize);
	if (bmp == NULL)
	{
		printf("Memory can't be allocated\n");
		goto end;
	}

	/* Reads the image data into memory */
	seekval = fseek(bmpfile, (long int) imageOffset, SEEK_SET);
	if (seekval != 0)
	{
		printf("Can't read from %s\n", argv[2]);
		goto end;
	}
	readval = fread(bmp, (size_t) 1, (size_t) imageSize, bmpfile);
	if (readval != (size_t) imageSize)
	{
		printf("Can't read from %s\n", argv[2]);
		goto end;
	}

	/* Vertically flips the image */
	retval = verticalFlip(bmp, (size_t) 512, (size_t) height);
	if (retval != 0)
	{
		printf("verticalFlip failed\n");
		goto end;
	}
	
	/* Goes to the offset to write image data to */
	seekval = fseek(pcfile, pcOffset, SEEK_SET);
	if (seekval != 0)
	{
		printf("Couldn't move to the image data offset\n");
		goto end;
	}
	
	/* Writes the image data */
	writeval = fwrite(bmp, (size_t) 1, (size_t) imageSize, pcfile);
	if (writeval != (size_t) imageSize)
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
