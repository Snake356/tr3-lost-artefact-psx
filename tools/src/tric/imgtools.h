#ifndef IMGTOOLS_H_
#define IMGTOOLS_H_

/* Structs */
struct bmpinfo{ /* Info contained in a BMP-header */
	unsigned INT32 size; /* Total size of bitmap file */
	unsigned INT32 imageOffset; /* Offset of image data */
	unsigned INT32 width; /* Width of the image in pixels */
	unsigned INT32 height; /* Height of the image in pixels */
	unsigned INT16 bpp; /* Number of bits per pixel */
	unsigned INT32 compression; /* Compression type */
	unsigned INT32 imageSize; /* Size of image data in bytes */
	unsigned INT32 xres; /* Horizontal resolution in pixels per meter */
	unsigned INT32 yres; /* Vertical resolution in pixels per meter */
	unsigned INT32 colours; /* Number of colours in the image */
	unsigned INT32 impColours; /* Number of important colours */
	unsigned int complete; /* Whether the structure is complete */
};

/* Function Declarations */
int bmpheader(FILE *bmp, unsigned INT32 size,
              unsigned INT32 imageOffset, unsigned INT32 width,
              unsigned INT32 height, unsigned INT16 bpp,
              unsigned INT32 compression,
              unsigned INT32 imageSize, unsigned INT32 xres,
              unsigned INT32 yres, unsigned INT32 colours,
              unsigned INT32 impColours);
int verticalFlip(char *image, size_t width, size_t height);

#endif
