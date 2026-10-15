/*
 * Library for converting textures in TR3PCPSX.
 * Copyright (C) 2021 b122251
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation,either version 3 of the License,or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY;without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not,see <http://www.gnu.org/licenses/>.
 */

/* File Inclusions */
#include <stdio.h>    /* Standard I/O */
#include <stdlib.h>   /* Standard Library */
#include <string.h>   /* Functions relating to strings */
#include "fixedint.h" /* Definition of fixed-size integers */
#include "structs.h"  /* Definition of structs used by TR3PCPSX */
#include "util.h"     /* General-purpose code used by TR3PCPSX */
#include "osdep.h"    /* Library of OS-dependant functions */
#include "colour.h"   /* Library for performing colour reduction */
#include "textures.h" /* Library for texture conversion */
#include "errors.h"   /* Error Handling for TR3PCPSX */
#include "version.h"  /* Version number */

/* Macro Declarations */
#define MAX_TEXTILES  (INTU8)   0x0F /* Maximum number of texture tiles (15) */
#define MAX_PALETTES (INTU16) 0x0800 /* Maximum number of palettes (2048) */
#define TEXTUREFLAG_DIRECT    (INTU8) 0x01 /* Used directly in world */
#define TEXTUREFLAG_INDIRECT  (INTU8) 0x02 /* Used indirectly in world */
#define TEXTUREFLAG_OBJECT    (INTU8) 0x04 /* Used in a mesh */
#define TEXTUREFLAG_ADDED     (INTU8) 0x08 /* Already in a block */
#define TEXTUREFLAG_CONVERTED (INTU8) 0x10 /* Converted */
#define TEXTUREFLAG_USED      (INTU8) 0x07 /* Used at all */
#define TEXTUREFLAG_WORLD     (INTU8) 0x03 /* Used in world at all */

/* Texture block struct */
struct texture
{
	INTU8 width;   /* Width of the texture */
	INTU8 height;  /* Height of the texture */
	INTU8 inTile;  /* Tile of the source */
	INTU8 inX;     /* X-position of the source */
	INTU8 inY;     /* Y-position of the source */
	INTU8 outTile; /* Tile of the output */
	INTU8 outX;    /* X-position of the output */
	INTU8 outY;    /* Y-position of the output */
	INTU8 placed;  /* Whether the texture was already placed */
	INTU8 world;   /* Whether the texture was used in world geometry
	                * (bit 2 is used for sprites, bit 3 for water) */
	INTU8 opaque;  /* Whether the texture contains transparency */
	
	/* Special variables for keeping track of low-res copies */
	INTU8 scale;   /* 0 = Full, 1 = Half, 2 = Quarter scale */
	INTU32 id;     /* Identifier for keeping track of them */
};

/* Internal function declarations */
static int sortTextures(struct texture *textures, INTU16 numTextures);
static void nextEmptySpot(INTU8 *bitmap, INTU8 *x, INTU8 *y);
static int willfit(INTU8 *bitmap, INTU8 x, INTU8 y, INTU8 width, INTU8 height);
static void fillSpot(INTU8 *bitmap, INTU8 x, INTU8 y,
                     INTU8 width, INTU8 height);
static int tile(struct texture *textures, INTU16 numTextures, INTU8 *numTiles);
static INTU16 findTexDup(int type, INTU8 *texture, INTU8 *outObjTex,
                         INTU8 *outRoomTex, INTU16 numObjectTextures,
                         INTU16 numRoomTextures, INTU16 numIndirectTextures,
                         INTU16 indirectStartPos, INTU32 flags);
static int convertTextureData(FILE *pc, FILE *psx, INTU8 inTile, INTU8 inX,
                              INTU8 inY, INTU8 width, INTU8 height,
                              INTU8 outTile, INTU8 outX, INTU8 outY,
                              INTU8 opaque, INTU8 m1tile, INTU8 m1x, INTU8 m1y,
                              INTU8 m2tile, INTU8 m2x, INTU8 m2y,
                              INTU16 *numPalettes,
                              INTU16 *palettes, INTU16 *waterpalettes,
                              INTU8 *usedColours, INTU32 flags,
                              INTU16 *paletteNumber, INTU16 *altPalettes,
                              INTU8 world, struct water_colour *waterColour,
                              struct level_info *pc_info,
                              struct level_info *psx_info);
static INTU16 underwaterColour(INTU16 colour, struct water_colour *waterColour);

/*
 * Function that sorts the textures on world, then width, then height.
 * (This function is a non-recursive implementation of Quicksort.)
 * Paramters:
 * * textures = Array of texture structs to be sorted
 * * numTextures = Number of textures to be sorted
 */
static int sortTextures(struct texture *textures, INTU16 numTextures)
{
	/* Variable Declarations */
	INTU16 *pivot = NULL;     /* Array of pivots */
	INTU32 *compare = NULL;   /* Integers for comparison */
	INT16 level;              /* How deep we are in the sorting tree */
	INTU16 i, j;              /* i and j used in sorting */
	INTU16 firstpos, lastpos; /* First and last position of the part to sort */
	struct texture temp;      /* Temporary storage for swapping elements */
	INTU32 temp32;            /* Temporary 32-bit storage for swapping */
	INT16 freeint161;         /* Signed 16-bit integer for general use */
	
	/* Checks whether the array can be sorted (due to allocation) */
	if (numTextures > 0x7FFF)
	{
		return ERROR_ARRAY_TOO_LONG;
	}
	
	/* Makes sure the array is at least two items long */
	if (numTextures <= 0x0001)
	{
		return ERROR_NONE;
	}
	
	/* Attemts to allocate array of pivots */
	pivot = calloc((size_t) numTextures, 2);
	if (pivot == NULL)
	{
		return ERROR_MEMORY;
	}
	
	/* Allocates the array of comparable integers */
	compare = calloc((size_t) numTextures, 4);
	if (compare == NULL)
	{
		free(pivot);
		return ERROR_MEMORY;
	}
	
	/* Sets the comparable variables */
	for (i = 0x0000; i < numTextures; ++i)
	{
		/* Sorts the textures on whether they're used in world geometry first,
		 * then on whether they're used as sprites, and then on surface area */
		compare[i] = (INTU32)
		             ((((INTU32) (textures[i].world & (INTU8) 0x01)) << 19U) |
		              (((INTU32) (textures[i].world & (INTU8) 0x02)) << 17U) |
		              (((INTU32) textures[i].width) *
		               ((INTU32) textures[i].height)));
	}
	
	/* Sets variables in their starting position */
	level = 0x0000;
	firstpos = 0x0000;
	lastpos = (numTextures - 1);
	
	/* Sorts the array */
	do
	{
		if ((lastpos - firstpos) >= 2)
		{
			/* Selects the pivot as the middle of the first three */
			freeint161 = 0x0000;
			if (compare[firstpos] < compare[firstpos + 1])
			{
				freeint161 |= 1;
			}
			if (compare[firstpos + 1] < compare[firstpos + 2])
			{
				freeint161 |= 2;
			}
			if (compare[firstpos] < compare[firstpos + 2])
			{
				freeint161 |= 4;
			}
			if ((freeint161 == 0) || (freeint161 == 7))
			{
				memmove(&temp, &textures[firstpos], sizeof(struct texture));
				memmove(&textures[firstpos], &textures[(firstpos + 1)],
				        sizeof(struct texture));
				memmove(&textures[(firstpos + 1)], &temp,
				        sizeof(struct texture));
				temp32 = compare[firstpos];
				compare[firstpos] = compare[(firstpos + 1)];
				compare[(firstpos + 1)] = temp32;
			}
			else if ((freeint161 == 2) || (freeint161 == 5))
			{
				memmove(&temp, &textures[firstpos], sizeof(struct texture));
				memmove(&textures[firstpos], &textures[(firstpos + 2)],
				        sizeof(struct texture));
				memmove(&textures[(firstpos + 2)], &temp,
				        sizeof(struct texture));
				temp32 = compare[firstpos];
				compare[firstpos] = compare[(firstpos + 2)];
				compare[(firstpos + 2)] = temp32;
			}
			
			/* Sets up i and j */
			i = (firstpos + 1);
			j = lastpos;
			
			/* Moves data around the pivot */
			while (i < j)
			{
				/* Moves i forwards through the array */
				while ((i < j) && (compare[i] >= compare[firstpos]))
				{
					++i;
				}
				if (i == j)
				{
					break;
				}
				
				/* Moves j backwards through the array */
				while ((i < j) && (compare[j] < compare[firstpos]))
				{
					--j;
				}
				if (i == j)
				{
					break;
				}
				
				/* Swaps j and i */
				memmove(&temp, &textures[i], sizeof(struct texture));
				memmove(&textures[i], &textures[j], sizeof(struct texture));
				memmove(&textures[j], &temp, sizeof(struct texture));
				temp32 = compare[i];
				compare[i] = compare[j];
				compare[j] = temp32;
			}
			
			/* Places the pivot in the correct location */
			while (compare[i] < compare[firstpos])
			{
				--i;
			}
			if (i != firstpos)
			{
				memmove(&temp, &textures[i], sizeof(struct texture));
				memmove(&textures[i], &textures[firstpos],
				        sizeof(struct texture));
				memmove(&textures[firstpos], &temp, sizeof(struct texture));
				temp32 = compare[i];
				compare[i] = compare[firstpos];
				compare[firstpos] = temp32;
			}
		}
		else
		{
			if (compare[firstpos] < compare[lastpos])
			{
				memmove(&temp, &textures[lastpos], sizeof(struct texture));
				memmove(&textures[lastpos], &textures[firstpos],
				        sizeof(struct texture));
				memmove(&textures[firstpos], &temp, sizeof(struct texture));
				temp32 = compare[lastpos];
				compare[lastpos] = compare[firstpos];
				compare[firstpos] = temp32;
			}
			i = firstpos;
		}
		
		/* Builds left subtree */
		if ((i - firstpos) > 1)
		{
			pivot[level] = i;
			++level;
			lastpos = i - 1;
		}
		
		/* Builds right subtree */
		else if ((lastpos - i) > 1)
		{
			pivot[level] = (i | ((INTU16) 0x8000));
			++level;
			firstpos = i + 1;
		}
		else
		{
			/* Goes back up the tree */
			do
			{
				if (level != 0x0000)
				{
					--level;
				}
				
				while (((pivot[level] & 0x8000) != 0x0000) && (level != 0x0000))
				{
					--level;
				}
				
				/* Investigates the right subtree of the ancestor */
				if (((pivot[level]) & 0x8000) == 0x0000)
				{
					lastpos = (numTextures - 1);
					for (freeint161 = (level - 1); freeint161 >= 0;
					     --freeint161)
					{
						if ((pivot[freeint161] & 0x8000) == 0)
						{
							lastpos = pivot[freeint161] - 1;
							break;
						}
					}
					firstpos = (pivot[level] + 1);
					pivot[level] |= 0x8000;
					
					/* Builds right subtree if needed */
					if ((lastpos > firstpos) && ((lastpos - firstpos) >= 1))
					{
						++level;
						break;
					}
				}
			}
			while (level != 0x0000);
		}
	}
	while (level != 0x0000);
	
	/* Ends the function */
	free(compare); /* Frees the comparable integers */
	free(pivot); /* Frees the pivots */
	return ERROR_NONE;
}

/*
 * Function that finds the next empty spot in the tile.
 * Parameters:
 * * bitmap = 256x256 bit map of occupied pixels
 * * x = Where to write the x-position of the texture
 * * y = Where to write the y-position of the texture
 * Nota bene: Don't use on an empty tile.
 */
static void nextEmptySpot(INTU8 *bitmap, INTU8 *x, INTU8 *y)
{
	/* Variable Declarations */
	INTU16 curpos; /* Current position in looping through the bitmap */
	INTU8 bitmask; /* Used to determine an exact pixel in the bitmap */
	int found = 0; /* Whether a next empty spot was found */
	
	/* Finds the first empty pixel */
	for (curpos = 0x0000; curpos < 0x2000; ++curpos)
	{
		if (bitmap[curpos] != (INTU8) 0xFF)
		{
			/* Sets the coordinates */
			*y = (INTU8) (curpos >> 5);
			*x = (INTU8) (((INTU16) (curpos & 0x001F)) << 3);
			for (bitmask = bitmap[curpos]; (bitmask & 0x80) != (INTU8) 0x00;
			     bitmask <<= 1)
			{
				(*x)++;
			}
			
			found = 1;
			break;
		}
	}
	
	/* Sets to empty values if there was no empty space left */
	if (found == 0)
	{
		*x = (INTU8) 0x00;
		*y = (INTU8) 0x00;
	}
}

/*
 * Function that checks whether a texture will fit in a specific spot.
 * Parameters:
 * * bitmap = 256x256 bit map of occupied pixels
 * * x = Suggested x-position of the texture
 * * y = Suggested y-position of the texture
 * * width = Width of the texture
 * * height = Height of the texture
 * Return values:
 * * 0 = The texture will not fit
 * * 1 = The texture will fit
 */
static int willfit(INTU8 *bitmap, INTU8 x, INTU8 y, INTU8 width, INTU8 height)
{
	/* Variable Declarations */
	INTU16 curpos;   /* Current position in the bitmap */
	INTU8 tempwidth; /* Temporary copy of width */
	INTU8 tempx;     /* Temporary copy of x */
	INTU8 bitmask;   /* Bit mask used for checking individual bits */
	
	/* Fills the bits line by line */
	while (height != (INTU8) 0x00)
	{
		/* Moves to the starting position */
		curpos = (INTU16) ((((INTU16) y) << 5) + ((INTU16) (x >> 3)));
		
		/* Makes copies of x and width */
		tempwidth = width;
		tempx = x;
		
		/* Checks the bits in the first byte as needed */
		if ((tempx & 0x07) != ((INTU8) 0x00))
		{
			bitmask = (INTU8) (0x80 >> (tempx & 0x07));
			while (((tempx & 0x07) != ((INTU8) 0x00)) &&
			       (tempwidth != ((INTU8) 0x00)))
			{
				if ((bitmap[curpos] & bitmask) != ((INTU8) 0x00))
				{
					return 0;
				}
				bitmask >>= 1;
				--tempwidth;
				++tempx;
			}
			++curpos;
		}
		
		/* Checks all the complete bytes */
		while (tempwidth >= (INTU8) 0x08)
		{
			if (bitmap[curpos] != (INTU8) 0x00)
			{
				return 0;
			}
			++curpos;
			tempwidth -= (INTU8) 0x08;
		}
		
		/* Checks the bits as needed in the last byte */
		if (tempwidth != (INTU8) 0x00)
		{
			bitmask = (INTU8) 0x80;
			while (tempwidth != (INTU8) 0x00)
			{
				if ((bitmap[curpos] & bitmask) != (INTU8) 0x00)
				{
					return 0;
				}
				bitmask >>= 1;
				--tempwidth;
			}
		}
		
		/* Moves on to the next line */
		--height;
		++y;
	}
	return 1;
}

/*
 * Function that fills a texture in the bitmap.
 * Parameters:
 * * bitmap = 256x256 bit map of occupied pixels
 * * x = Suggested x-position of the texture
 * * y = Suggested y-position of the texture
 * * width = Width of the texture
 * * height = Height of the texture
 * Nota bene:
 * * If width and height are both set to 0, it will simply
 * * fill up bits until it hits an already set bit.
 */
static void fillSpot(INTU8 *bitmap, INTU8 x, INTU8 y, INTU8 width, INTU8 height)
{
	/* Variable Declarations */
	INTU16 curpos;   /* Current position in the bitmap */
	INTU8 tempwidth; /* Temporary copy of width */
	INTU8 tempx;     /* Temporary copy of x */
	INTU8 bitmask;   /* Bit mask used for setting individual bits */
	
	/* Fils the set bits */
	if ((width != (INTU8) 0x00) || (height != (INTU8) 0x00))
	{
		/* Fills the bits line by line */
		while (height != (INTU8) 0x00)
		{
			/* Moves to the starting position */
			curpos = (INTU16) ((((INTU16) y) << 5) + ((INTU16) (x >> 3)));
			
			/* Makes copies of x and width */
			tempwidth = width;
			tempx = x;
			
			/* Sets the bits in the first byte as needed */
			if ((tempx & 0x07) != (INTU8) 0x00)
			{
				bitmask = (INTU8) (0x80 >> (tempx & 0x07));
				while (((tempx & 0x07) != (INTU8) 0x00) &&
				       (tempwidth != (INTU8) 0x00))
				{
					bitmap[curpos] |= bitmask;
					bitmask >>= 1;
					--tempwidth;
					++tempx;
				}
				++curpos;
			}
			
			/* Fills as many full bytes as we can */
			(void) memset(&bitmap[curpos], 0xFF, (size_t) (tempwidth >> 3));
			curpos += (INTU16) (tempwidth >> 3);
			tempwidth &= (INTU8) 0x07;
			
			/* Sets the bits as needed in the last byte */
			if (tempwidth != (INTU8) 0x00)
			{
				bitmask = (INTU8) 0x80;
				while (tempwidth != (INTU8) 0x00)
				{
					bitmap[curpos] |= bitmask;
					bitmask >>= 1;
					--tempwidth;
				}
			}
			
			/* Moves on to the next line */
			--height;
			++y;
		}
	}
	
	/* Fills bits until we hit an already set bit */
	else
	{
		/* Moves to the starting position */
		curpos = (INTU16) ((((INTU16) y) << 5) + ((INTU16) (x >> 3)));
		tempx = (x & (INTU8) 0x07);
		bitmask = (INTU8) (0x80 >> (tempx & 0x07));
		
		/* Fills up until the end of a line */
		do
		{
			/* Fills the individual bits per byte */
			while (tempx < (INTU8) 0x08)
			{
				/* Stops if the bit is already set */
				if ((bitmap[curpos] & bitmask) != (INTU8) 0x00)
				{
					return;
				}
				
				/* Sets the bit */
				bitmap[curpos] |= bitmask;
				bitmask >>= 1;
				++x;
				++tempx;
			}
			bitmask = (INTU8) 0x80;
			tempx = (INTU8) 0x00;
		}
		while (x != (INTU8) 0x00);
	}
}

/*
 * Function that places the textures in a tile.
 * Parameters:
 * * textures = Array of texture structs
 * * numTextures = The number of textures
 * * numTiles = Where to store the number of tiles
 */
static int tile(struct texture *textures, INTU16 numTextures, INTU8 *numTiles)
{
	/* Variable Declarations */
	INTU8 *bitmap = NULL;    /* Bit map of what pixels are occupied */
	INTU32 curTexture;       /* Current texture in loop */
	INTU32 unplacedTextures; /* Number of unplaced textures */
	INTU8 x, y;              /* Coordinates where to place the next texture */
	INTU8 leftx, lefty;      /* Space left in the tile */
	int worldTile;           /* Whether the current tile has world textures */
	int freeint1;            /* Integer for general use */
	int errorNumber;         /* Return value for this function */
	errorNumber = ERROR_NONE;
	
	/* Allocates bitmap */
	bitmap = malloc(8192);
	if (bitmap == NULL)
	{
		errorNumber = ERROR_MEMORY;
		goto end;
	}
	
	/* Sorts the textures before trying to place them */
	errorNumber = sortTextures(textures, numTextures);
	if (errorNumber != ERROR_NONE)
	{
		goto end;
	}
	
	/* All textures are yet unplaced */
	unplacedTextures = numTextures;
	
	/* Starts on the first tile */
	*numTiles = (INTU8) 0x00;
	
	/* Loops until all textures are placed */
	while (unplacedTextures > 0x00000000)
	{
		/* Clears a new tile */
		(void) memset(bitmap, 0, 8192);
		x = (INTU8) 0x00;
		y = (INTU8) 0x00;
		worldTile = 0;
		
		/* Fills up the current tile */
		do
		{
			/* Finds a texture to put in the current spot */
			for (curTexture = 0x0000; curTexture < numTextures; ++curTexture)
			{
				/* Skips all already placed textures */
				if (textures[curTexture].placed != (INTU8) 0x00)
				{
					continue;
				}
				
				/* Determines how much space there even is in the tile */
				leftx = (INTU8) ~x;
				lefty = (INTU8) ~y;
				
				/* Skips the texture if it's too large to fit */
				if (((textures[curTexture].width - (INTU8) 0x01) > leftx) ||
				    ((textures[curTexture].height - (INTU8) 0x01) > lefty))
				{
					continue;
				}
				
				/* Skips the texture if it is an object texture,
				 * but this tile is for world geometry */
				if (((textures[curTexture].world & (INTU8) 0x03) ==
				     (INTU8) 0x00) && (worldTile == 1))
				{
					continue;
				}
				
				/* Skips the texture if it's a sprite and it touches the edges
				 * of a tile (don't know why that's an issue) */
				if ((textures[curTexture].world & (INTU8) 0x02) != (INTU8) 0x00)
				{
					if ((((textures[curTexture].width - (INTU8) 0x01) + x) ==
					     (INTU8) 0xFF) ||
					    (((textures[curTexture].height - (INTU8) 0x01) + y) ==
					     (INTU8) 0xFF) ||
					    (x == (INTU8) 0x00) ||
					    (y == (INTU8) 0x00))
					{
						continue;
					}
				}
				
				/* Skips the texture if there are occupied pixels */
				freeint1 = willfit(bitmap, x, y, textures[curTexture].width,
				                   textures[curTexture].height);
				if (freeint1 == 0)
				{
					continue;
				}
				
				/* Places the texture */
				textures[curTexture].outTile = *numTiles;
				textures[curTexture].outX = x;
				textures[curTexture].outY = y;
				textures[curTexture].placed = (INTU8) 0x01;
				
				/* Fills the space for the texture */
				fillSpot(bitmap, x, y, textures[curTexture].width,
				         textures[curTexture].height);
				
				/* Remembers this tile holds world geometry */
				if ((textures[curTexture].world & (INTU8) 0x01) != (INTU8) 0x00)
				{
					worldTile = 1;
				}
				
				--unplacedTextures;
				break;
			}
			
			/* Stops if all textures have been placed */
			if (unplacedTextures == 0x00000000)
			{
				break;
			}
			
			/* Fills up the current line if no texture could be placed */
			if (curTexture == numTextures)
			{
				if (x == (INTU8) 0x00)
				{
					fillSpot(bitmap, x, y, (INTU8) 0x01, (INTU8) 0x01);
				}
				else
				{
					fillSpot(bitmap, x, y, (INTU8) 0x00, (INTU8) 0x00);
				}
			}
			
			/* Finds the next empty space */
			nextEmptySpot(bitmap, &x, &y);
		}
		while ((x != (INTU8) 0x00) || (y != (INTU8) 0x00));
		
		/* Moves on to the next tile */
		x = *numTiles;
		++x;
		*numTiles = x;
	}
	
end:/* Frees allocated memory and ends the function */
	if (bitmap != NULL)
	{
		free(bitmap);
	}
	return errorNumber;
}

/*
 * Function that finds whether a texture already exists
 * Parameters:
 * * type = (0 = Object Texture, 1 = Direct RoomTex, 2 = Indirect RoomTex)
 * * texture = In-memory texture
 * * outObjTex = Output object textures
 * * outRoomTex = Output room textures
 * * numObjectTextures = Number of existing object textures
 * * numRoomTextures = Number of direct room textures
 * * numIndirectTextures = Number of indirect room textures
 * * indirectStartPos = Where indirect room textures begin
 * * flags = Bit-mask of various flags
 * Returns number of duplicate, or FFFF if no duplicate exists
 */
static INTU16 findTexDup(int type, INTU8 *texture, INTU8 *outObjTex,
                         INTU8 *outRoomTex, INTU16 numObjectTextures,
                         INTU16 numRoomTextures, INTU16 numIndirectTextures,
                         INTU16 indirectStartPos, INTU32 flags)
{
	/* Variable Declarations */
	INTU8 *searchptr = NULL; /* Pointer to the thing to look through */
	INTU32 searchStep;       /* How many bytes each element is long */
	INTU32 startPos;         /* First position to search */
	INTU32 endPos;           /* Last position to search */
	INTU16 curIndex;         /* Current index in searching */
	
	/* Doesn't optimise at all if the flag is set */
	if (isflag(FLAGS_NO_TEXTOPTIMISE))
	{
		return 0xFFFF;
	}
	
	/* Determines how to search based on type */
	switch (type)
	{
		case 0:
			searchptr = outObjTex;
			searchStep = 0x00000010;
			startPos = 0x00000000;
			endPos = (INTU32) numObjectTextures;
			endPos *= 0x00000010;
			curIndex = 0x0000;
			break;
		case 1:
		case 2:
			searchptr = outRoomTex;
			searchStep = 0x00000030;
			startPos = 0x00000000;
			endPos = (INTU32) numRoomTextures;
			endPos *= 0x00000030;
			curIndex = 0x0000;
			break;
		default:
			return 0xFFFF;
	}
	
	/* Looks for a duplicate */
	for (; startPos < endPos; startPos += searchStep)
	{
		if (memcmp(texture, &searchptr[startPos], (size_t) 16) == 0)
		{
			return curIndex;
		}
		++curIndex;
	}
	
	/* Looks in the indirect textures if this one is indirect */
	if (type == 2)
	{
		startPos = indirectStartPos;
		startPos *= 0x00000030;
		endPos = (INTU32) numIndirectTextures;
		endPos *= 0x00000030;
		endPos += startPos;
		curIndex = indirectStartPos;
		for (; startPos < endPos; startPos += searchStep)
		{
			if (memcmp(texture, &searchptr[startPos], (size_t) 16) == 0)
			{
				return curIndex;
			}
			++curIndex;
		}
	}
	
	/* Returns FFFF as no duplicate was found */
	return 0xFFFF;
}

/*
 * Function that counts how many misoriented textures there are in the level.
 * Parameters:
 * * pc = Pointer to PC-file
 * * pc_info = Pointer to the level_info_struct relating to PC
 * * numMisorientedTextures = Storage for the number of misoriented textures
 * Return values:
 * * 0 = Everything went well
 * * 4 = PC-file could not be read from
 * * 10 = Memory could not be allocated
 */
int findMisorientedTextures(FILE *pc, struct level_info *pc_info,
                            INTU16 *numMisorientedTextures)
{
	/* Variable Declarations */
	INTU8 *textures = NULL; /* Buffer space for texture structs */
	INTU32 freeuint321,
	       freeuint322;     /* Unsigned 32-bit integer for general use */
	size_t freesizet1,
	       freesizet2;      /* size_t for general use */
	int freeint1;           /* Integer for general use */
	int errorNumber;        /* Return value for this function */
	errorNumber = ERROR_NONE;
	
	/* Allocates space for conversions and textures in memory */
	freeuint321 = pc_info->v_NumObjectTextures;
	freeuint321 *= 20;
	textures = calloc((size_t) freeuint321, 1);
	if (textures == NULL)
	{
		errorNumber = ERROR_MEMORY;
		goto end;
	}
	
	/* Reads Object Textures into memory */
	if (sizeof(size_t) < 4)
	{
		/* Reads the object textures one at a time */
		freeint1 = fseek(pc, (long int) (pc_info->p_NumObjectTextures + 4),
		                 SEEK_SET);
		if (freeint1 != 0)
		{
			errorNumber = ERROR_PC_READ_FAILED;
			goto end;
		}
		freeuint321 = 0x00000000;
		for (freeuint322 = pc_info->v_NumObjectTextures;
		     freeuint322 > 0x00000000; --freeuint322)
		{
			freesizet1 = fread(&textures[freeuint321], 1, 20, pc);
			if (freesizet1 != 20)
			{
				errorNumber = ERROR_PC_READ_FAILED;
				goto end;
			}
			freeuint321 += 20;
		}
	}
	else
	{
		/* Reads all object textures in one go */
		freesizet1 = ((size_t) (pc_info->v_NumObjectTextures * 20));
		freeint1 = fseek(pc, (long int) (pc_info->p_NumObjectTextures + 4),
		                 SEEK_SET);
		if (freeint1 != 0)
		{
			errorNumber = ERROR_PC_READ_FAILED;
			goto end;
		}
		freesizet2 = fread(textures, 1, freesizet1, pc);
		if (freesizet1 != freesizet2)
		{
			errorNumber = ERROR_PC_READ_FAILED;
			goto end;
		}
	}
	
	/* Sets the number of misoriented textures to 0 before starting */
	*numMisorientedTextures = 0x0000;
	
	/* Looks for misoriented textures */
	freeuint322 = 0x00000000;
	for (freeuint321 = 0x00000000; freeuint321 < pc_info->v_NumObjectTextures;
	     ++freeuint321)
	{
		/* Checks whether the texture is misoriented */
		if (((textures[(freeuint322 + 4)] == (INTU8) 0xFF) &&
		     (textures[(freeuint322 + 6)] == (INTU8) 0x01) &&
		     (textures[(freeuint322 + 8)] == (INTU8) 0xFF) &&
		     (textures[(freeuint322 + 10)] == (INTU8) 0xFF) &&
		     (textures[(freeuint322 + 12)] == (INTU8) 0x01) &&
		     (textures[(freeuint322 + 14)] == (INTU8) 0xFF) &&
		     (textures[(freeuint322 + 16)] == (INTU8) 0x01) &&
		     (textures[(freeuint322 + 18)] == (INTU8) 0x01)) ||
		    ((textures[(freeuint322 + 4)] == (INTU8) 0x01) &&
		     (textures[(freeuint322 + 6)] == (INTU8) 0xFF) &&
		     (textures[(freeuint322 + 8)] == (INTU8) 0x01) &&
		     (textures[(freeuint322 + 10)] == (INTU8) 0x01) &&
		     (textures[(freeuint322 + 12)] == (INTU8) 0xFF) &&
		     (textures[(freeuint322 + 14)] == (INTU8) 0x01) &&
		     (textures[(freeuint322 + 16)] == (INTU8) 0xFF) &&
		     (textures[(freeuint322 + 18)] == (INTU8) 0xFF)) ||
		    ((textures[(freeuint322 + 4)] == (INTU8) 0xFF) &&
		     (textures[(freeuint322 + 6)] == (INTU8) 0xFF) &&
		     (textures[(freeuint322 + 8)] == (INTU8) 0xFF) &&
		     (textures[(freeuint322 + 10)] == (INTU8) 0x01) &&
		     (textures[(freeuint322 + 12)] == (INTU8) 0x01) &&
		     (textures[(freeuint322 + 14)] == (INTU8) 0x01) &&
		     (textures[(freeuint322 + 16)] == (INTU8) 0x01) &&
		     (textures[(freeuint322 + 18)] == (INTU8) 0xFF)) ||
		    ((textures[(freeuint322 + 4)] == (INTU8) 0x01) &&
		     (textures[(freeuint322 + 6)] == (INTU8) 0x01) &&
		     (textures[(freeuint322 + 8)] == (INTU8) 0x01) &&
		     (textures[(freeuint322 + 10)] == (INTU8) 0xFF) &&
		     (textures[(freeuint322 + 12)] == (INTU8) 0xFF) &&
		     (textures[(freeuint322 + 14)] == (INTU8) 0xFF) &&
		     (textures[(freeuint322 + 16)] == (INTU8) 0xFF) &&
		     (textures[(freeuint322 + 18)] == (INTU8) 0x01)))
		{
			++(*numMisorientedTextures);
		}
		
		/* Moves on to the next object texture */
		freeuint322 += 0x00000014;
	}
	
end:/* Frees allocated memory and returns the function */
	if (textures != NULL)
	{
		free(textures);
	}
	return errorNumber;
}

/*
 * Function that converts all the texture data in the level.
 * Parameters:
 * * pc = Pointer to PC-file
 * * psx = Pointer to PSX-file
 * * pc_info = Pointer to the level_info struct relating to PC
 * * psx_info = Pointer to the level_info struct relating to PSX
 * * pcType = Type of PC-file (0=PHD, 1=TUB)
 * * psxpath = Path to the PSX-file
 * * flags = Bit-mask of program-wide flags
 * * misorientedTextures = List of misoriented textures
 * * roomTextptr = Pointer to RoomTextures
 * * objextTextptr = Pointer to ObjectTextures
 * * waterColour = Rules for calculating the underwater colours
 */
int convertAllTextures(FILE *pc, FILE *psx, struct level_info *pc_info,
                       struct level_info *psx_info, char *psxpath,
                       INTU32 flags, INTU16 *misorientedTextures,
                       INTU16 **roomTextptr, INTU16 **objectTextptr,
                       struct water_colour *waterColour)
{
	/* Variable Declarations */
	INTU8 *objectTextures = NULL;     /* In-memory copy of Object Textures */
	INTU8 *spriteTextures = NULL;     /* In-memory copy of Sprite Textures */
	INTU16 *palettes = NULL;          /* PSX palettes */
	INTU16 *altPalettes = NULL;       /* Holds each palette's alternate */
	INTU16 *waterpalettes = NULL;     /* PSX palettes for underwater */
	INTU8  *usedColours = NULL;       /* Number of colours in each palette */
	struct texture *textures = NULL;  /* Struct for texture "blocks" */
	INTU8 *buffer = NULL;             /* Buffer for general use */
	INTU8 *buffer2 = NULL;            /* Buffer for general use */
	INTU8 *textureFlags = NULL;       /* Space to store textures' properties */
	INTU8 *freeptr1 = NULL;           /* Pointer for general use */
	INTU16 *roomTextures = NULL;      /* Room Textures */
	INTU16 *objTextures = NULL;       /* Object Textures */
	INTU8 *outRoomTex = NULL;         /* Output room textures */
	INTU8 *outObjTex = NULL;          /* Output object textures */
	INTU16 indirectStartPos;          /* Where in memory the indirects start */
	INTU16 numIndirectTextures;       /* Number of indirect textures */
	INTU16 numPalettes;               /* The number of palettes */
	INTU16 numTextures;               /* The number of texture "blocks" */
	INTU16 numObjectTextures;         /* Number of object textures */
	INTU16 numRoomTextures;           /* Number of direct room textures */
	INTU16 totalRoomTextures;         /* Number of total room textures */
	INTU8 minX, minY, maxX, maxY;     /* Corner values of the textures */
	INTU16 curMisorientedTexture;     /* The current misoriented texture */
	INTU8 numTiles;                   /* The number of output tiles */
	INTU8 curTile;                    /* Current tile */
	int opaque = 1;                   /* Whether the texture is opaque */
	INTU8 freeuint81,
	      freeuint82,
	      freeuint83,
	      freeuint84;                 /* 8-bit integers for general use */
	INTU16 freeuint161,
	       freeuint162,
	       freeuint163,
	       freeuint164;               /* 16-bit integers for general use */
	INT16 freeint161;
	INTU32 freeuint321,
	       freeuint322,
	       freeuint323;               /* 32-bit integers for general use */
	int freeint1,
	    freeint2;                     /* Integer for general use */
	size_t freesizet1,
	       freesizet2;                /* Size_t for general use */
	long int freelong1;               /* Long integer for general use */
	int errorNumber;                  /* Return value of this function */
	errorNumber = ERROR_NONE;
	
	/* Tells the user what is happening */
	if (isflag(FLAGS_VERBOSE))
	{
		printf("Converting Textures... ");
		(void) fflush(stdout);
	}
	
	/* Sets variables to 0 before starting */
	minX = (INTU8) 0x00;
	minY = (INTU8) 0x00;
	maxX = (INTU8) 0x00;
	maxY = (INTU8) 0x00;
	curTile = (INTU8) 0x00;
	numTiles = (INTU8) 0x00;
	
	/* Reads Object Textures into memory */
	freesizet1 = (size_t) (pc_info->v_NumObjectTextures * 20);
	objectTextures = malloc(freesizet1);
	if (objectTextures == NULL)
	{
		errorNumber = ERROR_MEMORY;
		goto end;
	}
	freeint1 = fseek(pc, (long int) (pc_info->p_NumObjectTextures + 4),
	                 SEEK_SET);
	if (freeint1 != 0)
	{
		errorNumber = ERROR_PC_READ_FAILED;
		goto end;
	}
	freesizet2 = fread(objectTextures, 1, freesizet1, pc);
	if (freesizet1 != freesizet2)
	{
		errorNumber = ERROR_PC_READ_FAILED;
		goto end;
	}
	
	/* Reads Sprite Textures into memory */
	freesizet1 = (size_t) (pc_info->v_NumSpriteTextures * 16);
	spriteTextures = malloc(freesizet1);
	if (spriteTextures == NULL)
	{
		errorNumber = ERROR_MEMORY;
		goto end;
	}
	freeint1 = fseek(pc, (long int) (pc_info->p_NumSpriteTextures + 4),
	                 SEEK_SET);
	if (freeint1 != 0)
	{
		errorNumber = ERROR_PC_READ_FAILED;
		goto end;
	}
	freesizet2 = fread(spriteTextures, 1, freesizet1, pc);
	if (freesizet1 != freesizet2)
	{
		errorNumber = ERROR_PC_READ_FAILED;
		goto end;
	}
	
	/* Allocates space to count which textures were used in world geometry */
	freeuint321 = (pc_info->v_NumObjectTextures + pc_info->v_NumSpriteTextures);
	freeuint321 <<= 2U;
	textureFlags = malloc((size_t) freeuint321);
	if (textureFlags == NULL)
	{
		errorNumber = ERROR_MEMORY;
		goto end;
	}
	(void) memset(textureFlags, 0, (size_t) freeuint321);
	
	/* Checks which textures were used in world geometry */
	for (freeuint161 = 0x0000; freeuint161 < pc_info->v_NumRooms; ++freeuint161)
	{
		/* Checks whether this room is underwater */
		if ((pc_info->room[freeuint161].flags & 0x0001) == 0x0001)
		{
			freeuint322 = 0x00000002;
		}
		else
		{
			freeuint322 = 0x00000000;
		}
		
		/* Reads rectangles into memory */
		freesizet1 = (size_t) (pc_info->room[freeuint161].v_NumRectangles * 10);
		buffer = malloc(freesizet1);
		if (buffer == NULL)
		{
			errorNumber = ERROR_MEMORY;
			goto end;
		}
		freeint1 = fseek(pc, (long int)
		                 (pc_info->room[freeuint161].p_NumRectangles + 2),
		                 SEEK_SET);
		if (freeint1 != 0)
		{
			errorNumber = ERROR_PC_READ_FAILED;
			goto end;
		}
		freesizet2 = fread(buffer, 1, freesizet1, pc);
		if (freesizet1 != freesizet2)
		{
			errorNumber = ERROR_PC_READ_FAILED;
			goto end;
		}
		
		/* Goes through the rectangles */
		for (freeuint162 = 0x0000;
		     freeuint162 < pc_info->room[freeuint161].v_NumRectangles;
		     ++freeuint162)
		{
			memcpy(&freeuint163, &buffer[((freeuint162 * 10) + 8)], 2);
#ifdef __BIG_ENDIAN__
			freeuint163 = reverse16(freeuint163);
#endif
			/* Sets the directly called flags */
			freeuint321 = (INTU32) (freeuint163 & 0x3FFF);
			freeuint321 <<= 2U;
			freeuint321 += freeuint322;
			textureFlags[freeuint321] |= TEXTUREFLAG_DIRECT;
			
			/* Sets double-sided flag */
			if ((freeuint163 & 0x8000) != 0x0000)
			{
				++freeuint321;
				textureFlags[freeuint321] |= TEXTUREFLAG_DIRECT;
			}
		}
		
		/* Frees the buffer up again */
		free(buffer);
		buffer = NULL;
		
		/* Reads triangles into memory */
		freesizet1 = (size_t) (pc_info->room[freeuint161].v_NumTriangles * 8);
		buffer = malloc(freesizet1);
		if (buffer == NULL)
		{
			errorNumber = ERROR_MEMORY;
			goto end;
		}
		freeint1 = fseek(pc, ((long int)
		                 (pc_info->room[freeuint161].p_NumTriangles + 2)),
		                 SEEK_SET);
		if (freeint1 != 0)
		{
			errorNumber = ERROR_PC_READ_FAILED;
			goto end;
		}
		freesizet2 = fread(buffer, 1, freesizet1, pc);
		if (freesizet1 != freesizet2)
		{
			errorNumber = ERROR_PC_READ_FAILED;
			goto end;
		}
		
		/* Goes through the triangles */
		for (freeuint162 = 0x0000;
		     freeuint162 < pc_info->room[freeuint161].v_NumTriangles;
		     ++freeuint162)
		{
			memcpy(&freeuint163, &buffer[((freeuint162 * 8) + 6)], 2);
#ifdef __BIG_ENDIAN__
			freeuint163 = reverse16(freeuint163);
#endif
			
			/* Sets the directly called flags */
			freeuint321 = (INTU32) (freeuint163 & 0x3FFF);
			freeuint321 <<= 2U;
			freeuint321 += freeuint322;
			textureFlags[freeuint321] |= TEXTUREFLAG_DIRECT;
			
			/* Sets double-sided flag */
			if ((freeuint163 & 0x8000) != 0x0000)
			{
				++freeuint321;
				textureFlags[freeuint321] |= TEXTUREFLAG_DIRECT;
			}
		}
		
		free(buffer);
		buffer = NULL;
	}
	
	/* Checks which textures were used in objects */
	for (freeuint321 = 0x00000000; freeuint321 < pc_info->v_NumMeshPointers;
	     ++freeuint321)
	{
		/* Goes through meshes */
		freelong1 = (long int) (pc_info->p_NumMeshPointers + 4 +
		                        (freeuint321 * 0x00000004));
		freeint1 = fseek (pc, freelong1, SEEK_SET);
		if (freeint1 != 0)
		{
			errorNumber = ERROR_PC_READ_FAILED;
			goto end;
		}
		freesizet1 = fread(&freeuint322, 1, 4, pc);
		if (freesizet1 != (size_t) 4)
		{
			errorNumber = ERROR_PC_READ_FAILED;
			goto end;
		}
#ifdef __BIG_ENDIAN__
		freeuint322 = reverse32(freeuint322);
#endif
		freelong1 = (long int) (pc_info->p_NumMeshData + 14 + freeuint322);
		freeint1 = fseek (pc, freelong1, SEEK_SET);
		if (freeint1 != 0)
		{
			errorNumber = ERROR_PC_READ_FAILED;
			goto end;
		}
		freesizet1 = fread(&freeuint162, 1, 2, pc);
		if (freesizet1 != (size_t) 2)
		{
			errorNumber = ERROR_PC_READ_FAILED;
			goto end;
		}
#ifdef __BIG_ENDIAN__
		freeuint162 = reverse16(freeuint162);
#endif
		freelong1 += (2 + (freeuint162 * 6));
		freeint1 = fseek (pc, freelong1, SEEK_SET);
		if (freeint1 != 0)
		{
			errorNumber = ERROR_PC_READ_FAILED;
			goto end;
		}
		freesizet1 = fread(&freeint161, 1, 2, pc);
		if (freesizet1 != (size_t) 2)
		{
			errorNumber = ERROR_PC_READ_FAILED;
			goto end;
		}
#ifdef __BIG_ENDIAN__
		freeint161 = reverse16(freeint161);
#endif
		if (freeint161 < 0x0000)
		{
			freelong1 += (2 + (abs((int) freeint161) * 2));
		}
		else
		{
			freelong1 += (2 + (freeint161 * 6));
		}
		freeint1 = fseek (pc, freelong1, SEEK_SET);
		if (freeint1 != 0)
		{
			errorNumber = ERROR_PC_READ_FAILED;
			goto end;
		}
		freesizet1 = fread(&freeuint162, 1, 2, pc);
		if (freesizet1 != (size_t) 2)
		{
			errorNumber = ERROR_PC_READ_FAILED;
			goto end;
		}
#ifdef __BIG_ENDIAN__
		freeuint162 = reverse16(freeuint162);
#endif
		freelong1 += 2;
		while (freeuint162 > 0x0000)
		{
			/* Goes through rectangles */
			freelong1 += 8l;
			freeint1 = fseek (pc, freelong1, SEEK_SET);
			if (freeint1 != 0)
			{
				errorNumber = ERROR_PC_READ_FAILED;
				goto end;
			}
			freesizet1 = fread(&freeuint163, 1, 2, pc);
			if (freesizet1 != (size_t) 2)
			{
				errorNumber = ERROR_PC_READ_FAILED;
				goto end;
			}
#ifdef __BIG_ENDIAN__
			freeuint163 = reverse16(freeuint163);
#endif
			
			/* Sets the directly called flags */
			freeuint322 = (INTU32) (freeuint163 & 0x3FFF);
			freeuint322 <<= 2U;
			textureFlags[freeuint322] |= TEXTUREFLAG_OBJECT;
			
			/* Sets double-sided flag */
			if ((freeuint163 & 0x8000) != 0x0000)
			{
				++freeuint322;
				textureFlags[freeuint322] |= TEXTUREFLAG_OBJECT;
			}
			freelong1 += 2l;
			--freeuint162;
		}
		freeint1 = fseek (pc, freelong1, SEEK_SET);
		if (freeint1 != 0)
		{
			errorNumber = ERROR_PC_READ_FAILED;
			goto end;
		}
		freesizet1 = fread(&freeuint162, 1, 2, pc);
		if (freesizet1 != (size_t) 2)
		{
			errorNumber = ERROR_PC_READ_FAILED;
			goto end;
		}
#ifdef __BIG_ENDIAN__
		freeuint162 = reverse16(freeuint162);
#endif
		freelong1 += 2;
		while (freeuint162 > 0x0000)
		{
			/* Goes through triangles */
			freelong1 += 6l;
			freeint1 = fseek (pc, freelong1, SEEK_SET);
			if (freeint1 != 0)
			{
				errorNumber = ERROR_PC_READ_FAILED;
				goto end;
			}
			freesizet1 = fread(&freeuint163, 1, 2, pc);
			if (freesizet1 != (size_t) 2)
			{
				errorNumber = ERROR_PC_READ_FAILED;
				goto end;
			}
#ifdef __BIG_ENDIAN__
			freeuint163 = reverse16(freeuint163);
#endif
			
			/* Sets the directly called flags */
			freeuint322 = (INTU32) (freeuint163 & 0x3FFF);
			freeuint322 <<= 2U;
			textureFlags[freeuint322] |= TEXTUREFLAG_OBJECT;
			
			/* Sets double-sided flag */
			if ((freeuint163 & 0x8000) != 0x0000)
			{
				++freeuint322;
				textureFlags[freeuint322] |= TEXTUREFLAG_OBJECT;
			}
			freelong1 += 2l;
			--freeuint162;
		}
	}
	
	/* Reads animated textures into memory */
	freesizet1 = (size_t) (pc_info->v_NumAnimatedTextures * 2);
	buffer = malloc(freesizet1);
	if (buffer == NULL)
	{
		errorNumber = ERROR_MEMORY;
		goto end;
	}
	freeint1 = fseek(pc, (long int) (pc_info->p_NumAnimatedTextures + 4),
	                 SEEK_SET);
	if (freeint1 != 0)
	{
		errorNumber = ERROR_PC_READ_FAILED;
		goto end;
	}
	freesizet2 = fread(buffer, 1, freesizet1, pc);
	if (freesizet2 != freesizet1)
	{
		errorNumber = ERROR_PC_READ_FAILED;
		goto end;
	}
	
	/* Sets texture flags for animated textures if needed */
	freeuint321 = 0x00000002;
	memcpy(&freeuint163, buffer, 2);
#ifdef __BIG_ENDIAN__
	freeuint163 = reverse16(freeuint163);
#endif
	for (; freeuint163 > 0x0000; --freeuint163)
	{
		memcpy(&freeuint161, &buffer[freeuint321], 2);
#ifdef __BIG_ENDIAN__
		freeuint161 = reverse16(freeuint161);
#endif
		++freeuint161;
		freeuint321 += 2;
		
		/* Checks whether any of these textures was used in world geometry */
		freeuint81 = (INTU8) 0x00;
		freeuint322 = freeuint321;
		for (freeuint162 = freeuint161; freeuint162 > 0x0000; --freeuint162)
		{
			memcpy(&freeuint164, &buffer[freeuint322], 2);
#ifdef __BIG_ENDIAN__
			freeuint164 = reverse16(freeuint164);
#endif
			freeuint322 += 2;
			freeuint323 = (INTU32) freeuint164;
			freeuint323 <<= 2U;
			if ((textureFlags[freeuint323] & TEXTUREFLAG_DIRECT) !=
			    (INTU8) 0x00)
			{
				freeuint81 |= (INTU8) 0x01;
			}
			++freeuint323;
			if ((textureFlags[freeuint323] & TEXTUREFLAG_DIRECT) !=
			    (INTU8) 0x00)
			{
				freeuint81 |= (INTU8) 0x02;
			}
			++freeuint323;
			if ((textureFlags[freeuint323] & TEXTUREFLAG_DIRECT) !=
			    (INTU8) 0x00)
			{
				freeuint81 |= (INTU8) 0x04;
			}
			++freeuint323;
			if ((textureFlags[freeuint323] & TEXTUREFLAG_DIRECT) !=
			    (INTU8) 0x00)
			{
				freeuint81 |= (INTU8) 0x08;
			}
		}
		
		/* Sets the appropriate flag for all of them */
		for (; freeuint161 > 0x0000; --freeuint161)
		{
			memcpy(&freeuint162, &buffer[freeuint321], 2);
#ifdef __BIG_ENDIAN__
			freeuint162 = reverse16(freeuint162);
#endif
			freeuint321 += 2;
			freeuint323 = (INTU32) freeuint162;
			freeuint323 <<= 2U;
			if (((freeuint81 & (INTU8) 0x01) != (INTU8) 0x00) &&
			    ((textureFlags[freeuint323] & TEXTUREFLAG_DIRECT) ==
			      (INTU8) 0x00))
			{
				textureFlags[freeuint323] |= TEXTUREFLAG_INDIRECT;
			}
			++freeuint323;
			if (((freeuint81 & (INTU8) 0x02) != (INTU8) 0x00) &&
			    ((textureFlags[freeuint323] & TEXTUREFLAG_DIRECT) ==
			      (INTU8) 0x00))
			{
				textureFlags[freeuint323] |= TEXTUREFLAG_INDIRECT;
			}
			++freeuint323;
			if (((freeuint81 & (INTU8) 0x04) != (INTU8) 0x00) &&
			    ((textureFlags[freeuint323] & TEXTUREFLAG_DIRECT) ==
			      (INTU8) 0x00))
			{
				textureFlags[freeuint323] |= TEXTUREFLAG_INDIRECT;
			}
			++freeuint323;
			if (((freeuint81 & (INTU8) 0x08) != (INTU8) 0x00) &&
			    ((textureFlags[freeuint323] & TEXTUREFLAG_DIRECT) ==
			      (INTU8) 0x00))
			{
				textureFlags[freeuint323] |= TEXTUREFLAG_INDIRECT;
			}
		}
	}
	
	/* Frees up the buffer again */
	free(buffer);
	buffer = NULL;
	
	/* Determines RoomTextures and ObjectTextures */
	numRoomTextures = 0x0000;
	numIndirectTextures = 0x0000;
	numObjectTextures = 0x0000;
	freeuint323 = (pc_info->v_NumObjectTextures);
	freeuint323 <<= 2U;
	for (freeuint321 = 0x00000000; freeuint321 < freeuint323; ++freeuint321)
	{
		if ((textureFlags[freeuint321] & TEXTUREFLAG_DIRECT) != (INTU8) 0x00)
		{
			++numRoomTextures;
		}
		if ((textureFlags[freeuint321] & TEXTUREFLAG_INDIRECT) != (INTU8) 0x00)
		{
			++numIndirectTextures;
		}
		if ((textureFlags[freeuint321] & TEXTUREFLAG_OBJECT) != (INTU8) 0x00)
		{
			++numObjectTextures;
		}
		if (textureFlags[freeuint321] == (INTU8) 0x00)
		{
			textureFlags[freeuint321] |= TEXTUREFLAG_ADDED;
			textureFlags[freeuint321] |= TEXTUREFLAG_CONVERTED;
		}
	}
	totalRoomTextures = (numRoomTextures + numIndirectTextures);
	
	/* Allocates roomTextures and objectTextures */
	*roomTextptr = malloc((size_t) (pc_info->v_NumObjectTextures << 3U));
	if (*roomTextptr == NULL)
	{
		errorNumber = ERROR_MEMORY;
		goto end;
	}
	*objectTextptr = malloc((size_t) (pc_info->v_NumObjectTextures << 2U));
	if (*objectTextptr == NULL)
	{
		errorNumber = ERROR_MEMORY;
		goto end;
	}
	roomTextures = *roomTextptr;
	objTextures = *objectTextptr;
	
	/* Sets the textures to FFFF before starting */
	memset(roomTextures, 0xFF, (size_t) (pc_info->v_NumObjectTextures << 3U));
	memset(objTextures, 0xFF, (size_t) (pc_info->v_NumObjectTextures << 2U));
	
	/* Allocates texture "block" structs */
	freeuint322 = (INTU32) ((totalRoomTextures * 0x00000003) +
	                        numObjectTextures +
	                        pc_info->v_NumSpriteTextures);
	textures = calloc((size_t) freeuint322, sizeof(struct texture));
	if (textures == NULL)
	{
		errorNumber = ERROR_MEMORY;
		goto end;
	}
	
	/* Detects the texture "blocks" from Object Textures */
	numTextures = 0x0000;
	do
	{
		freeint1 = 0; /* Set if a new texture block was created */
		freeptr1 = (INTU8 *) objectTextures;
		
		for (freeuint322 = 0x00000000;
		     freeuint322 < pc_info->v_NumObjectTextures; ++freeuint322)
		{
			/* Skips the texture if it was already in a block */
			if (((textureFlags[(freeuint322 << 2U)] &
			     TEXTUREFLAG_ADDED) != (INTU8) 0x00) &&
			    ((textureFlags[((freeuint322 << 2U) + 0x00000001)] &
			     TEXTUREFLAG_ADDED) != (INTU8) 0x00) &&
			    ((textureFlags[((freeuint322 << 2U) + 0x00000002)] &
			     TEXTUREFLAG_ADDED) != (INTU8) 0x00) &&
			    ((textureFlags[((freeuint322 << 2U) + 0x00000003)] &
			     TEXTUREFLAG_ADDED) != (INTU8) 0x00))
			{
				freeptr1 += 20;
				continue;
			}
			
			/* Sets the texture block to the current texture at first */
			if (freeint1 == 0)
			{
				/* Remembers that a texture block will be added now */
				freeint1 = 1;
				
				curTile = freeptr1[2];
				if (freeptr1[4] == (INTU8) 0x01)
				{
					minX = freeptr1[5];
				}
				else if (freeptr1[8] == (INTU8) 0x01)
				{
					minX = freeptr1[9];
				}
				else
				{
					minX = freeptr1[13];
				}
				if (freeptr1[4] == (INTU8) 0xFF)
				{
					maxX = freeptr1[5];
				}
				else if (freeptr1[8] == (INTU8) 0xFF)
				{
					maxX = freeptr1[9];
				}
				else
				{
					maxX = freeptr1[13];
				}
				if (freeptr1[6] == (INTU8) 0x01)
				{
					minY = freeptr1[7];
				}
				else if (freeptr1[10] == (INTU8) 0x01)
				{
					minY = freeptr1[11];
				}
				else
				{
					minY = freeptr1[15];
				}
				if (freeptr1[6] == (INTU8) 0xFF)
				{
					maxY = freeptr1[7];
				}
				else if (freeptr1[10] == (INTU8) 0xFF)
				{
					maxY = freeptr1[11];
				}
				else
				{
					maxY = freeptr1[15];
				}
				
				/* Sets opaque at first */
				if (freeptr1[0] == (INTU8) 0x00)
				{
					opaque = 1;
				}
				else
				{
					opaque = 0;
				}
				
				/* Remembers this texture was added */
				textureFlags[(freeuint322 << 2U)] |= TEXTUREFLAG_ADDED;
				textureFlags[((freeuint322 << 2U) + 0x00000001)] |=
					TEXTUREFLAG_ADDED;
				textureFlags[((freeuint322 << 2U) + 0x00000002)] |=
					TEXTUREFLAG_ADDED;
				textureFlags[((freeuint322 << 2U) + 0x00000003)] |=
					TEXTUREFLAG_ADDED;
				
				/* Sets the world-flag if needed */
				if (((textureFlags[(freeuint322 << 2U)] & TEXTUREFLAG_WORLD) !=
				     (INTU8) 0x00) ||
				    ((textureFlags[((freeuint322 << 2U) + 2)] &
				      TEXTUREFLAG_WORLD) != (INTU8) 0x00))
				{
					textures[numTextures].world |= (INTU8) 0x01;
					if ((textureFlags[((freeuint322 << 2U) + 2)] &
					     TEXTUREFLAG_WORLD) != (INTU8) 0x00)
					{
						textures[numTextures].world |= (INTU8) 0x04;
					}
				}
				
				freeptr1 += 20;
				continue;
			}
			
			/* Skips the texture if it's on a different tile */
			if (freeptr1[2] != curTile)
			{
				freeptr1 += 20;
				continue;
			}
			
			/* Extracts coordinates of the current texture */
			if (freeptr1[4] == (INTU8) 0x01)
			{
				freeuint81 = freeptr1[5];
			}
			else if (freeptr1[8] == (INTU8) 0x01)
			{
				freeuint81 = freeptr1[9];
			}
			else
			{
				freeuint81 = freeptr1[13];
			}
			if (freeptr1[4] == (INTU8) 0xFF)
			{
				freeuint82 = freeptr1[5];
			}
			else if (freeptr1[8] == (INTU8) 0xFF)
			{
				freeuint82 = freeptr1[9];
			}
			else
			{
				freeuint82 = freeptr1[13];
			}
			if (freeptr1[6] == (INTU8) 0x01)
			{
				freeuint83 = freeptr1[7];
			}
			else if (freeptr1[10] == (INTU8) 0x01)
			{
				freeuint83 = freeptr1[11];
			}
			else
			{
				freeuint83 = freeptr1[15];
			}
			if (freeptr1[6] == (INTU8) 0xFF)
			{
				freeuint84 = freeptr1[7];
			}
			else if (freeptr1[10] == (INTU8) 0xFF)
			{
				freeuint84 = freeptr1[11];
			}
			else
			{
				freeuint84 = freeptr1[15];
			}
			
			/* Expands the surface as needed if the two textures overlap */
			if ((((freeuint81 <= minX) && (minX <= freeuint82)) ||
			     ((minX <= freeuint81) && (freeuint81 <= maxX))) &&
			    (((freeuint83 <= minY) && (minY <= freeuint84)) ||
			     ((minY <= freeuint83) && (freeuint83 <= maxY))))
			{
				if (freeuint81 < minX)
				{
					minX = freeuint81;
				}
				if (freeuint82 > maxX)
				{
					maxX = freeuint82;
				}
				if (freeuint83 < minY)
				{
					minY = freeuint83;
				}
				if (freeuint84 > maxY)
				{
					maxY = freeuint84;
				}
				/* Updates opaque if needed */
				if (freeptr1[0] != (INTU8) 0x00)
				{
					opaque = 0;
				}
				textureFlags[(freeuint322 << 2U)] |= TEXTUREFLAG_ADDED;
				textureFlags[((freeuint322 << 2U) + 0x00000001)] |=
					TEXTUREFLAG_ADDED;
				textureFlags[((freeuint322 << 2U) + 0x00000002)] |=
					TEXTUREFLAG_ADDED;
				textureFlags[((freeuint322 << 2U) + 0x00000003)] |=
					TEXTUREFLAG_ADDED;
				
				/* Sets the world-flag if needed */
				if (((textureFlags[(freeuint322 << 2U)] & TEXTUREFLAG_WORLD) !=
				     (INTU8) 0x00) ||
				    ((textureFlags[((freeuint322 << 2U) + 2)] &
				      TEXTUREFLAG_WORLD) != (INTU8) 0x00))
				{
					textures[numTextures].world |= (INTU8) 0x01;
					if ((textureFlags[((freeuint322 << 2U) + 2)] &
					     TEXTUREFLAG_WORLD) != (INTU8) 0x00)
					{
						textures[numTextures].world |= (INTU8) 0x04;
					}
				}
			}
			freeptr1 += 20;
		}
		
		/* Adds the new texture "block" */
		if (freeint1 == 1)
		{
			textures[numTextures].inTile = curTile;
			textures[numTextures].inX = minX;
			textures[numTextures].inY = minY;
			textures[numTextures].width = ((maxX - minX) + ((INTU8) 0x01));
			textures[numTextures].height = ((maxY - minY) + ((INTU8) 0x01));
			if (opaque == 1)
			{
				textures[numTextures].opaque = (INTU8) 0x01;
			}
			textures[numTextures].id = (INTU32) numTextures;
			
			/* Makes low-res copies if needed */
			if (notflag(FLAGS_NO_MIPMAP_AT_ALL) &&
			    ((textures[numTextures].world & (INTU8) 0x01) == (INTU8) 0x01))
			{
				textures[(numTextures + 0x0001)].width =
					(INTU8) (textures[numTextures].width >> 1U);
				textures[(numTextures + 0x0001)].height =
					(INTU8) (textures[numTextures].height >> 1U);
				textures[(numTextures + 0x0001)].id = (INTU32) numTextures;
				textures[(numTextures + 0x0001)].world = (INTU8) 0x01;
				textures[(numTextures + 0x0001)].scale = (INTU8) 0x01;
				if (notflag(FLAGS_NO_SECOND_MIPMAP))
				{
					textures[(numTextures + 0x0002)].width =
						(INTU8) (textures[numTextures].width >> 2U);
					textures[(numTextures + 0x0002)].height =
						(INTU8) (textures[numTextures].height >> 2U);
					textures[(numTextures + 0x0002)].id = (INTU32) numTextures;
					textures[(numTextures + 0x0002)].world = (INTU8) 0x01;
					textures[(numTextures + 0x0002)].scale = (INTU8) 0x02;
					numTextures += 0x0002;
				}
				else
				{
					++numTextures;
				}
			}
			
			++numTextures;
		}
	}
	while (freeint1 == 1);
	
	/* Adds texture "blocks" for the sprites */
	freeptr1 = spriteTextures;
	for (freeuint321 = 0x00000000; freeuint321 < pc_info->v_NumSpriteTextures;
	     ++freeuint321)
	{
		textures[numTextures].inTile = freeptr1[0];
		textures[numTextures].inX = freeptr1[2];
		textures[numTextures].inY = freeptr1[3];
		textures[numTextures].width = (freeptr1[5] + (INTU8) 0x01);
		textures[numTextures].height = (freeptr1[7] + (INTU8) 0x01);
		textures[numTextures].opaque = (INTU8) 0x00;
		textures[numTextures].world |= (INTU8) 0x02;
		++numTextures;
		freeptr1 += 16;
	}
	
	/* Tiles the texture "blocks" */
	errorNumber = tile(textures, numTextures, &numTiles);
	if (errorNumber != ERROR_NONE)
	{
		goto end;
	}
	
	/* Checks the number of texture tiles */
	if (numTiles > MAX_TEXTILES)
	{
		errorNumber = ERROR_TOO_MANY_TEXTILES;
		goto end;
	}
	else if (numTiles == MAX_TEXTILES)
	{
		PRINT_WARNING;
		printf("Texture tile 15 is used. Please check texture integrity.\n");
	}
	
	/* Adjusts the number of Texture Tiles in PSX */
	if (psx_info->v_NumTexTiles < (INTU32) numTiles)
	{
		/* Inserts the needed space */
		freeuint321 = (((INTU32) numTiles) - psx_info->v_NumTexTiles);
		freeuint321 *= 0x00008000;
		errorNumber = insertbytes(psx, psx_info->p_NumTexTiles, freeuint321);
		if (errorNumber != ERROR_NONE)
		{
			goto end;
		}
		level_info_add(psx_info, psx_info->p_NumTexTiles, freeuint321);
		
		/* Sets NumTexTiles */
		freeuint321 = (INTU32) numTiles;
#ifdef __BIG_ENDIAN__
		freeuint321 = reverse32(freeuint321);
#endif
		freeint1 = fseek(psx, (long int) psx_info->p_NumTexTiles, SEEK_SET);
		if (freeint1 != 0)
		{
			errorNumber = ERROR_PSX_READ_FAILED;
			goto end;
		}
		freesizet1 = fwrite(&freeuint321, 1, 4, psx);
		if (freesizet1 != 4)
		{
			errorNumber = ERROR_PSX_WRITE_FAILED;
			goto end;
		}
		psx_info->v_NumTexTiles = (INTU32) numTiles;
	}
	else if (psx_info->v_NumTexTiles > (INTU32) numTiles)
	{
		/* Removes the unneeded space */
		freeuint321 = (psx_info->v_NumTexTiles - ((INTU32) numTiles));
		freeuint321 *= 0x00008000;
		errorNumber = removebytes(psx, psxpath, psx_info->p_NumTexTiles,
		                          freeuint321);
		if (errorNumber != ERROR_NONE)
		{
			goto end;
		}
		level_info_sub(psx_info, psx_info->p_NumTexTiles, freeuint321);
		
		/* Sets NumTexTiles */
		freeuint321 = (INTU32) numTiles;
#ifdef __BIG_ENDIAN__
		freeuint321 = reverse32(freeuint321);
#endif
		freeint1 = fseek(psx, (long int) psx_info->p_NumTexTiles, SEEK_SET);
		if (freeint1 != 0)
		{
			errorNumber = ERROR_PSX_READ_FAILED;
			goto end;
		}
		freesizet1 = fwrite(&freeuint321, 1, 4, psx);
		if (freesizet1 != 4)
		{
			errorNumber = ERROR_PSX_WRITE_FAILED;
			goto end;
		}
		psx_info->v_NumTexTiles = (INTU32) numTiles;
	}
	
	/* Fills texture data with a pattern */
	buffer = malloc(8);
	if (buffer == NULL)
	{
		errorNumber = ERROR_MEMORY;
		goto end;
	}
	buffer[0] = (INTU8) 0x10;
	buffer[1] = (INTU8) 0x32;
	buffer[2] = (INTU8) 0x54;
	buffer[3] = (INTU8) 0x76;
	buffer[4] = (INTU8) 0x98;
	buffer[5] = (INTU8) 0xBA;
	buffer[6] = (INTU8) 0xDC;
	buffer[7] = (INTU8) 0xFE;
	freeuint321 = (((INTU32) numTiles) * 0x00008000);
	freeuint322 = (psx_info->p_NumTexTiles + 0x00000004);
	errorNumber = patternbytes(psx, freeuint322, freeuint321,
	                           buffer, 0x00000008);
	if (errorNumber != ERROR_NONE)
	{
		goto end;
	}
	free(buffer);
	buffer = NULL;
	
	/* Allocates the space for output texture structs */
	outRoomTex = malloc((size_t) (totalRoomTextures * 0x0030));
	outObjTex = malloc((size_t) (numObjectTextures * 0x0010));
	if ((outRoomTex == NULL) || (outObjTex == NULL))
	{
		errorNumber = ERROR_MEMORY;
		goto end;
	}
	indirectStartPos = numRoomTextures;
	numRoomTextures = 0x0000;
	numObjectTextures = 0x0000;
	numIndirectTextures = 0x0000;
	
	/* Adjusts the number of SpriteTexture Structs */
	freeuint321 = pc_info->v_NumSpriteTextures;
	if (freeuint321 > psx_info->v_NumSpriteTextures)
	{
		/* Adds the needed space */
		freeuint322 = (freeuint321 - psx_info->v_NumSpriteTextures);
		freeuint322 *= 0x00000010;
		errorNumber = insertbytes(psx, psx_info->p_NumSpriteTextures,
		                          freeuint322);
		if (errorNumber != ERROR_NONE)
		{
			goto end;
		}
		level_info_add(psx_info, psx_info->p_NumSpriteTextures, freeuint322);
		
		/* Sets NumSpriteTextures */
		psx_info->v_NumSpriteTextures = freeuint321;
#ifdef __BIG_ENDIAN__
		freeuint321 = reverse32(freeuint321);
#endif
		freeint1 = fseek(psx, (long int) psx_info->p_NumSpriteTextures,
		                 SEEK_SET);
		if (freeint1 != 0)
		{
			errorNumber = ERROR_PSX_READ_FAILED;
			goto end;
		}
		freesizet1 = fwrite(&freeuint321, 1, 4, psx);
		if (freesizet1 != 4)
		{
			errorNumber = ERROR_PSX_WRITE_FAILED;
			goto end;
		}
	}
	else if (freeuint321 < psx_info->v_NumSpriteTextures)
	{
		/* Removes the unneeded space */
		freeuint322 = (psx_info->v_NumSpriteTextures - freeuint321);
		freeuint322 *= 0x00000010;
		errorNumber = removebytes(psx, psxpath, psx_info->p_NumSpriteTextures,
		                          freeuint322);
		if (errorNumber != ERROR_NONE)
		{
			goto end;
		}
		level_info_sub(psx_info, psx_info->p_NumSpriteTextures, freeuint322);
		
		/* Sets NumSpriteTextures */
		psx_info->v_NumSpriteTextures = freeuint321;
#ifdef __BIG_ENDIAN__
		freeuint321 = reverse32(freeuint321);
#endif
		freeint1 = fseek(psx, (long int) psx_info->p_NumSpriteTextures,
		                 SEEK_SET);
		if (freeint1 != 0)
		{
			errorNumber = ERROR_PSX_READ_FAILED;
			goto end;
		}
		freesizet1 = fwrite(&freeuint321, 1, 4, psx);
		if (freesizet1 != 4)
		{
			errorNumber = ERROR_PSX_WRITE_FAILED;
			goto end;
		}
	}
	
	/* Allocates palette space */
	freesizet1 = (size_t) ((numTextures + 64) * 32);
	palettes = calloc(freesizet1, 2);
	if (palettes == NULL)
	{
		errorNumber = ERROR_MEMORY;
		goto end;
	}
	waterpalettes = calloc(freesizet1, 2);
	if (waterpalettes == NULL)
	{
		errorNumber = ERROR_MEMORY;
		goto end;
	}
	freesizet1 /= (size_t) 16;
	usedColours = calloc(freesizet1, 1);
	if (usedColours == NULL)
	{
		errorNumber = ERROR_MEMORY;
		goto end;
	}
	altPalettes = calloc(freesizet1, 2);
	if (altPalettes == NULL)
	{
		errorNumber = ERROR_MEMORY;
		goto end;
	}
	numPalettes = 0x0000;
	curMisorientedTexture = 0x0000;
	freeuint162 = 0x0000;
	
	/* Sets the first palette (for inventory and pause screen) */
	palettes[0] = 0x8000;
	palettes[1] = 0x8842;
	palettes[2] = 0x9084;
	palettes[3] = 0x98C6;
	palettes[4] = 0xA108;
	palettes[5] = 0xA94A;
	palettes[6] = 0xB18C;
	palettes[7] = 0xB9CE;
	palettes[8] = 0xC210;
	palettes[9] = 0xCA52;
	palettes[10] = 0xD294;
	palettes[11] = 0xDAD6;
	palettes[12] = 0xE318;
	palettes[13] = 0xEB5A;
	palettes[14] = 0xF39C;
	palettes[15] = 0xFBDE;
	for (freeuint161 = 0x0000; freeuint161 < 0x0010; ++freeuint161)
	{
		waterpalettes[freeuint161] = underwaterColour(palettes[freeuint161],
		                                              waterColour);
	}
	usedColours[0] = (INTU8) 0x10;
	numPalettes = 0x0001;
	
	/* Allocates buffer space to convert structs */
	buffer = malloc(64);
	if (buffer == NULL)
	{
		errorNumber = ERROR_MEMORY;
		goto end;
	}

	/* Sets every unused texture as already converted */
	freeuint323 = (pc_info->v_NumObjectTextures << 2U);
	for (freeuint321 = 0x00000000; freeuint321 < freeuint323; ++freeuint321)
	{
		if ((textureFlags[freeuint321] & TEXTUREFLAG_USED) == (INTU8) 0x00)
		{
			textureFlags[freeuint321] |= TEXTUREFLAG_CONVERTED;
		}
	}
	
	/* Converts the textures */
	for (freeuint161 = 0x0000; freeuint161 < numTextures; ++freeuint161)
	{
		/* Don't copy low-res textures twice */
		if (textures[freeuint161].scale != (INTU8) 0x00)
		{
			continue;
		}
		
		/* Determines the low-res versions (if needed) */
		freeuint163 = freeuint164 = 0x0000;
		if ((textures[freeuint161].world & (INTU8) 0x01) != (INTU8) 0x00)
		{
			for (freeuint164 = 0x0000; freeuint164 < numTextures;
			     ++freeuint164)
			{
				if ((textures[freeuint161].id == textures[freeuint164].id) &&
			        (textures[freeuint164].scale == (INTU8) 0x01))
				{
					break;
				}
			}
			if (freeuint164 == numTextures)
			{
				freeuint164 = 0x0000;
			}
			for (freeuint163 = 0x0000; freeuint163 < numTextures;
			     ++freeuint163)
			{
				if ((textures[freeuint161].id == textures[freeuint163].id) &&
			        (textures[freeuint163].scale == (INTU8) 0x02))
				{
					break;
				}
			}
			if (freeuint163 == numTextures)
			{
				freeuint163 = 0x0000;
			}
		}
		
		/* Converts the texture tile */
		errorNumber = convertTextureData(pc, psx, textures[freeuint161].inTile,
		                                 textures[freeuint161].inX,
		                                 textures[freeuint161].inY,
		                                 textures[freeuint161].width,
		                                 textures[freeuint161].height,
		                                 textures[freeuint161].outTile,
		                                 textures[freeuint161].outX,
		                                 textures[freeuint161].outY,
		                                 textures[freeuint161].opaque,
		                                 textures[freeuint164].outTile,
		                                 textures[freeuint164].outX,
		                                 textures[freeuint164].outY,
		                                 textures[freeuint163].outTile,
		                                 textures[freeuint163].outX,
		                                 textures[freeuint163].outY,
		                                 &numPalettes, palettes, waterpalettes,
		                                 usedColours, flags, &freeuint162,
		                                 altPalettes,
		                                 textures[freeuint161].world,
		                                 waterColour, pc_info, psx_info);
		if (errorNumber != ERROR_NONE)
		{
			goto end;
		}
		
		/* Converts Object Textures within this texture "block" */
		freeptr1 = objectTextures;
		minX = textures[freeuint161].inX;
		minY = textures[freeuint161].inY;
		maxX = (minX + (textures[freeuint161].width - (INTU8) 0x01));
		maxY = (minY + (textures[freeuint161].height - (INTU8) 0x01));
		for (freeuint321 = 0x00000000;
		     freeuint321 < pc_info->v_NumObjectTextures; ++freeuint321)
		{
			/* Skips this object texture if it was already converted */
			if (((textureFlags[(freeuint321 << 2U)] &
			      TEXTUREFLAG_CONVERTED) != (INTU8) 0x00) &&
			    ((textureFlags[((freeuint321 << 2U) + 0x00000001)] &
			      TEXTUREFLAG_CONVERTED) != (INTU8) 0x00) &&
			    ((textureFlags[((freeuint321 << 2U) + 0x00000002)] &
			      TEXTUREFLAG_CONVERTED) != (INTU8) 0x00) &&
			    ((textureFlags[((freeuint321 << 2U) + 0x00000003)] &
			      TEXTUREFLAG_CONVERTED) != (INTU8) 0x00))
			{
				freeptr1 += 20;
				continue;
			}
			
			/* Skips this object texture if it was never used at all */
			if (((textureFlags[(freeuint321 << 2U)] &
			      TEXTUREFLAG_USED) == (INTU8) 0x00) &&
			    ((textureFlags[((freeuint321 << 2U) + 0x00000001)] &
			      TEXTUREFLAG_USED) == (INTU8) 0x00) &&
			    ((textureFlags[((freeuint321 << 2U) + 0x00000002)] &
			      TEXTUREFLAG_USED) == (INTU8) 0x00) &&
			    ((textureFlags[((freeuint321 << 2U) + 0x00000003)] &
			      TEXTUREFLAG_USED) == (INTU8) 0x00))
			{
				freeptr1 += 20;
				continue;
			}
			
			/* Sees if this texture overlaps with the texture "block" */
			if (freeptr1[2] != textures[freeuint161].inTile)
			{
				freeptr1 += 20;
				continue;
			}
			if (freeptr1[4] == (INTU8) 0x01)
			{
				freeuint81 = freeptr1[5];
			}
			else if (freeptr1[8] == (INTU8) 0x01)
			{
				freeuint81 = freeptr1[9];
			}
			else
			{
				freeuint81 = freeptr1[13];
			}
			if (freeptr1[4] == (INTU8) 0xFF)
			{
				freeuint82 = freeptr1[5];
			}
			else if (freeptr1[8] == (INTU8) 0xFF)
			{
				freeuint82 = freeptr1[9];
			}
			else
			{
				freeuint82 = freeptr1[13];
			}
			if (freeptr1[6] == (INTU8) 0x01)
			{
				freeuint83 = freeptr1[7];
			}
			else if (freeptr1[10] == (INTU8) 0x01)
			{
				freeuint83 = freeptr1[11];
			}
			else
			{
				freeuint83 = freeptr1[15];
			}
			if (freeptr1[6] == (INTU8) 0xFF)
			{
				freeuint84 = freeptr1[7];
			}
			else if (freeptr1[10] == (INTU8) 0xFF)
			{
				freeuint84 = freeptr1[11];
			}
			else
			{
				freeuint84 = freeptr1[15];
			}
			
			/* Converts the struct if they overlap */
			if ((((freeuint81 <= minX) && (minX <= freeuint82)) ||
			     ((minX <= freeuint81) && (freeuint81 <= maxX))) &&
			    (((freeuint83 <= minY) && (minY <= freeuint84)) ||
			     ((minY <= freeuint83) && (freeuint83 <= maxY))))
			{
				/* Palette */
				buffer[2] = (INTU8) (freeuint162 & ((INTU16) 0x00FF));
				buffer[3] = (INTU8) ((freeuint162 & ((INTU16) 0xFF00)) >> 8);
				/* Attributes */
				buffer[14] = freeptr1[0];
				buffer[15] = freeptr1[1];
				/* Texture Tile */
				buffer[6] = textures[freeuint161].outTile;
				buffer[7] = (INTU8) 0x00;
				/* Alpha-transparency */
				if ((buffer[14] & (INTU8) 0x02) != (INTU8) 0x00)
				{
					buffer[10] = (INTU8) 0x36;
					buffer[11] = (INTU8) 0x3E;
				}
				else
				{
					buffer[10] = (INTU8) 0x34;
					buffer[11] = (INTU8) 0x3C;
				}
				
				freeint1 = 5;
				if (notflag(FLAGS_DONT_FIX_TEXTURES))
				{
					/* Checks whether the texture is misoriented */
					if ((freeptr1[4] == (INTU8) 0xFF) &&
					    (freeptr1[6] == (INTU8) 0x01) &&
					    (freeptr1[8] == (INTU8) 0xFF) &&
					    (freeptr1[10] == (INTU8) 0xFF) &&
					    (freeptr1[12] == (INTU8) 0x01) &&
					    (freeptr1[14] == (INTU8) 0xFF) &&
					    (freeptr1[16] == (INTU8) 0x01) &&
					    (freeptr1[18] == (INTU8) 0x01))
					{
						freeint1 = 0;
					}
					else if ((freeptr1[4] == (INTU8) 0x01) &&
					         (freeptr1[6] == (INTU8) 0xFF) &&
					         (freeptr1[8] == (INTU8) 0x01) &&
					         (freeptr1[10] == (INTU8) 0x01) &&
					         (freeptr1[12] == (INTU8) 0xFF) &&
					         (freeptr1[14] == (INTU8) 0x01) &&
					         (freeptr1[16] == (INTU8) 0xFF) &&
					         (freeptr1[18] == (INTU8) 0xFF))
					{
						freeint1 = 1;
					}
					else if ((freeptr1[4] == (INTU8) 0x01) &&
					         (freeptr1[6] == (INTU8) 0x01) &&
					         (freeptr1[8] == (INTU8) 0x01) &&
					         (freeptr1[10] == (INTU8) 0xFF) &&
					         (freeptr1[12] == (INTU8) 0xFF) &&
					         (freeptr1[14] == (INTU8) 0xFF) &&
					         (freeptr1[16] == (INTU8) 0xFF) &&
					         (freeptr1[18] == (INTU8) 0x01))
					{
						freeint1 = 2;
					}
					else if ((freeptr1[4] == (INTU8) 0xFF) &&
					         (freeptr1[6] == (INTU8) 0xFF) &&
					         (freeptr1[8] == (INTU8) 0xFF) &&
					         (freeptr1[10] == (INTU8) 0x01) &&
					         (freeptr1[12] == (INTU8) 0x01) &&
					         (freeptr1[14] == (INTU8) 0x01) &&
					         (freeptr1[16] == (INTU8) 0x01) &&
					         (freeptr1[18] == (INTU8) 0xFF))
					{
						freeint1 = 3;
					}
					
					/* Remembers the misoriented texture */
					if (freeint1 != 5)
					{
						misorientedTextures[curMisorientedTexture] =
							(INTU16) freeuint321;
						
						/* Remembers its orientation */
						misorientedTextures[curMisorientedTexture] &= 0x7FFF;
						if ((freeint1 & 1) != 0)
						{
							misorientedTextures[curMisorientedTexture] |=
								0x8000;
						}
						
						/* Moves on to the next misoriented texture */
						++curMisorientedTexture;
					}
				}
				
				/* Texture is properly oriented */
				if (freeint1 == 5)
				{
					/* Coordinates */
					buffer[0] = freeptr1[5];
					buffer[1] = freeptr1[7];
					buffer[4] = freeptr1[9];
					buffer[5] = freeptr1[11];
					/* Adjusts for rectangular or triangular texture */
					if (freeptr1[16] == (INTU8) 0x00)
					{
						/* Triangular */
						buffer[8] = freeptr1[13];
						buffer[9] = freeptr1[15];
						buffer[12] = (INTU8) 0x00;
						buffer[13] = (INTU8) 0x00;
					}
					else
					{
						/* Rectangular */
						buffer[8] = freeptr1[17];
						buffer[9] = freeptr1[19];
						buffer[12] = freeptr1[13];
						buffer[13] = freeptr1[15];
					}
				}
				/* Texture is misoriented */
				else
				{
					/* Coordinates */
					buffer[0] = ((freeint1 <= 1) ? freeuint81 : freeuint82);
					buffer[1] = freeuint83;
					buffer[4] = ((freeint1 <= 1) ? freeuint82 : freeuint81);
					buffer[5] = freeuint83;
					/* Adjusts for rectangular or triangular texture */
					if (freeptr1[16] == (INTU8) 0x00)
					{
						/* Triangular */
						buffer[8] = ((freeint1 <= 1) ? freeuint82 : freeuint81);
						buffer[9] = freeuint84;
						buffer[12] = (INTU8) 0x00;
						buffer[13] = (INTU8) 0x00;
					}
					else
					{
						/* Rectangular */
						buffer[8] = ((freeint1 <= 1) ? freeuint81 : freeuint82);
						buffer[9] = freeuint84;
						buffer[12] = ((freeint1 <= 1) ?
						              freeuint82 : freeuint81);
						buffer[13] = freeuint84;
					}
				}
				
				/* Changes the coordinates to suit the tiling */
				buffer[0] -= textures[freeuint161].inX;
				buffer[0] += textures[freeuint161].outX;
				buffer[1] -= textures[freeuint161].inY;
				buffer[1] += textures[freeuint161].outY;
				buffer[4] -= textures[freeuint161].inX;
				buffer[4] += textures[freeuint161].outX;
				buffer[5] -= textures[freeuint161].inY;
				buffer[5] += textures[freeuint161].outY;
				buffer[8] -= textures[freeuint161].inX;
				buffer[8] += textures[freeuint161].outX;
				buffer[9] -= textures[freeuint161].inY;
				buffer[9] += textures[freeuint161].outY;
				/* Only adjusts the last coordinate for a rectangular texture */
				if (freeptr1[16] != (INTU8) 0x00)
				{
					buffer[12] -= textures[freeuint161].inX;
					buffer[12] += textures[freeuint161].outX;
					buffer[13] -= textures[freeuint161].inY;
					buffer[13] += textures[freeuint161].outY;
				}
				
				/* Backs up this texture struct (for duplication and such */
				memcpy(&buffer[32], &buffer[0], 16);
				
				/* Converts it if used as an object texture */
				if ((textureFlags[(freeuint321 << 2U)] &
				     TEXTUREFLAG_OBJECT) != (INTU8) 0x00)
				{
					/* Checks whether this texture already exists */
					freeuint164 = findTexDup(0, buffer, outObjTex, outRoomTex,
					                         numObjectTextures, numRoomTextures,
					                         numIndirectTextures,
					                         indirectStartPos, flags);
					if (freeuint164 == 0xFFFF)
					{
						/* Copies the object texture to the list */
						memcpy(&outObjTex[(numObjectTextures * 0x0010)],
						       buffer, (size_t) 16);
						
						/* Remembers this texture was converted */
						textureFlags[(freeuint321 << 2U)] |=
							TEXTUREFLAG_CONVERTED;
						objTextures[(freeuint321 << 1U)] = numObjectTextures++;
					}
					else
					{
						textureFlags[(freeuint321 << 2U)] |=
							TEXTUREFLAG_CONVERTED;
						objTextures[(freeuint321 << 1U)] = freeuint164;
					}
				}
				
				/* Converts it if used as an object texture */
				if ((textureFlags[((freeuint321 << 2U) + 0x00000001)] &
				     TEXTUREFLAG_OBJECT) != (INTU8) 0x00)
				{
					/* Flips the coordinates */
					freeuint81 = buffer[0];
					buffer[0] = buffer[4];
					buffer[4] = freeuint81;
					if (freeptr1[16] != (INTU8) 0x00)
					{
						freeuint81 = buffer[8];
						buffer[8] = buffer[12];
						buffer[12] = freeuint81;
					}
					
					/* Checks whether this texture already exists */
					freeuint164 = findTexDup(0, buffer, outObjTex, outRoomTex,
					                        numObjectTextures, numRoomTextures,
					                        numIndirectTextures,
					                        indirectStartPos, flags);
					if (freeuint164 == 0xFFFF)
					{
						/* Copies the object texture to the list */
						memcpy(&outObjTex[(numObjectTextures * 0x0010)],
						       buffer, (size_t) 16);
						
						/* Remembers this texture was converted */
						textureFlags[((freeuint321 << 2U) + 0x00000001)] |=
							TEXTUREFLAG_CONVERTED;
						objTextures[((freeuint321 << 1U) + 0x00000001)] =
							numObjectTextures++;
					}
					else
					{
						textureFlags[((freeuint321 << 2U) + 0x00000001)] |=
							TEXTUREFLAG_CONVERTED;
						objTextures[((freeuint321 << 1U) + 0x00000001)] =
							freeuint164;
					}
				}
				
				/* Sets the padding bytes to FF FF if needed */
				if (buffer[46] != (INTU8) 0x00)
				{
					buffer[46] = (INTU8) 0xFF;
					buffer[47] = (INTU8) 0xFF;
				}
				
				/* Restores the backup */
				memcpy(&buffer[0], &buffer[32], 16);
				
				/* Converts it if used as a room texture */
				if ((textureFlags[(freeuint321 << 2U)] &
				     TEXTUREFLAG_WORLD) !=(INTU8) 0x00)
				{
					/* Checks whether this texture already exists */
					if ((textureFlags[(freeuint321 << 2U)] &
					     TEXTUREFLAG_DIRECT) != (INTU8) 0x00)
					{
						freeuint164 = findTexDup(1, buffer, outObjTex,
						                         outRoomTex, numObjectTextures,
						                         numRoomTextures,
						                         numIndirectTextures,
						                         indirectStartPos, flags);
					}
					else
					{
						freeuint164 = findTexDup(2, buffer, outObjTex,
						                         outRoomTex, numObjectTextures,
						                         numRoomTextures,
						                         numIndirectTextures,
						                         indirectStartPos, flags);
					}
					if (freeuint164 == 0xFFFF)
					{
						/* Sets the offset in the output list */
						if ((textureFlags[(freeuint321 << 2U)] &
						     TEXTUREFLAG_DIRECT) != (INTU8) 0x00)
						{
							freelong1 = (long int) (numRoomTextures * 0x0030);
						}
						else
						{
							freelong1 = (long int)
								((indirectStartPos + numIndirectTextures) * 0x0030);
						}
						
						/* Copies the texture to the list */
						memcpy(&outRoomTex[freelong1], buffer, (size_t) 16);
						freelong1 += 16l;
						
						/* Handles mipmapping if needed */
						if (notflag(FLAGS_NO_MIPMAP_AT_ALL))
						{
							/* Finds the first low-res copy of this texture */
							for (freeuint164 = 0x0000; freeuint164 < numTextures;
							     ++freeuint164)
							{
								if ((textures[freeuint164].id ==
								     textures[freeuint161].id) &&
								    (textures[freeuint164].scale == (INTU8) 0x01))
								{
									break;
								}
							}
							
							/* Sets the coordinates for the lower-res version */
							buffer[0] -= textures[freeuint161].outX;
							buffer[0] >>= 1U;
							buffer[0] += textures[freeuint164].outX;
							buffer[1] -= textures[freeuint161].outY;
							buffer[1] >>= 1U;
							buffer[1] += textures[freeuint164].outY;
							buffer[4] -= textures[freeuint161].outX;
							buffer[4] >>= 1U;
							buffer[4] += textures[freeuint164].outX;
							buffer[5] -= textures[freeuint161].outY;
							buffer[5] >>= 1U;
							buffer[5] += textures[freeuint164].outY;
							buffer[8] -= textures[freeuint161].outX;
							buffer[8] >>= 1U;
							buffer[8] += textures[freeuint164].outX;
							buffer[9] -= textures[freeuint161].outY;
							buffer[9] >>= 1U;
							buffer[9] += textures[freeuint164].outY;
							if (freeptr1[16] != (INTU8) 0x00)
							{
								buffer[12] -= textures[freeuint161].outX;
								buffer[12] >>= 1U;
								buffer[12] += textures[freeuint164].outX;
								buffer[13] -= textures[freeuint161].outY;
								buffer[13] >>= 1U;
								buffer[13] += textures[freeuint164].outY;
							}
							buffer[6] = textures[freeuint164].outTile;
						}
						
						/* Copies the texture to the list */
						memcpy(&outRoomTex[freelong1], buffer, (size_t) 16);
						freelong1 += 16l;
						
						if ((notflag(FLAGS_NO_SECOND_MIPMAP)) &&
						    (notflag(FLAGS_NO_MIPMAP_AT_ALL)))
						{
							/* Restores the backup */
							memcpy(&buffer[0], &buffer[32], 16);
							
							/* Finds the second low-res copy of this texture */
							for (freeuint164 = 0x0000; freeuint164 < numTextures;
							     ++freeuint164)
							{
								if ((textures[freeuint164].id ==
								     textures[freeuint161].id) &&
								    (textures[freeuint164].scale == (INTU8) 0x02))
								{
									break;
								}
							}
							
							/* Sets the coordinates for the lower-res version */
							buffer[0] -= textures[freeuint161].outX;
							buffer[0] >>= 2U;
							buffer[0] += textures[freeuint164].outX;
							buffer[1] -= textures[freeuint161].outY;
							buffer[1] >>= 2U;
							buffer[1] += textures[freeuint164].outY;
							buffer[4] -= textures[freeuint161].outX;
							buffer[4] >>= 2U;
							buffer[4] += textures[freeuint164].outX;
							buffer[5] -= textures[freeuint161].outY;
							buffer[5] >>= 2U;
							buffer[5] += textures[freeuint164].outY;
							buffer[8] -= textures[freeuint161].outX;
							buffer[8] >>= 2U;
							buffer[8] += textures[freeuint164].outX;
							buffer[9] -= textures[freeuint161].outY;
							buffer[9] >>= 2U;
							buffer[9] += textures[freeuint164].outY;
							if (freeptr1[16] != (INTU8) 0x00)
							{
								buffer[12] -= textures[freeuint161].outX;
								buffer[12] >>= 2U;
								buffer[12] += textures[freeuint164].outX;
								buffer[13] -= textures[freeuint161].outY;
								buffer[13] >>= 2U;
								buffer[13] += textures[freeuint164].outY;
							}
							buffer[6] = textures[freeuint164].outTile;
						}
						
						/* Copies the texture to the list */
						memcpy(&outRoomTex[freelong1], buffer, (size_t) 16);
						freelong1 += 16l;
						
						/* Remembers this texture was converted */
						textureFlags[(freeuint321 << 2U)] |=
							TEXTUREFLAG_CONVERTED;
						freeuint322 = (INTU32) (freeuint321 << 2U);
					
						/* Sets the offset in the output list */
						if ((textureFlags[(freeuint321 << 2U)] &
						     TEXTUREFLAG_DIRECT) != (INTU8) 0x00)
						{
							roomTextures[freeuint322] = numRoomTextures++;
						}
						else
						{
							roomTextures[freeuint322] = (indirectStartPos +
							                             numIndirectTextures);
							numIndirectTextures++;
						}
					}
					else
					{
						/* Remembers this texture was converted */
						textureFlags[(freeuint321 << 2U)] |=
							TEXTUREFLAG_CONVERTED;
						freeuint322 = (INTU32) (freeuint321 << 2U);
						roomTextures[freeuint322] = freeuint164;
					}
				}
				
				/* Restores the backup */
				memcpy(&buffer[0], &buffer[32], 16);
	
				/* Converts it if used as a room texture */
				if ((textureFlags[((freeuint321 << 2U) + 0x00000001)] &
				     TEXTUREFLAG_WORLD) !=(INTU8) 0x00)
				{
					/* Flips the coordinates */
					freeuint81 = buffer[0];
					buffer[0] = buffer[4];
					buffer[4] = freeuint81;
					if (freeptr1[16] != (INTU8) 0x00)
					{
						freeuint81 = buffer[8];
						buffer[8] = buffer[12];
						buffer[12] = freeuint81;
					}
						
					/* Checks whether this texture already exists */
					if ((textureFlags[((freeuint321 << 2U) + 0x00000001)] &
					     TEXTUREFLAG_DIRECT) != (INTU8) 0x00)
					{
						freeuint164 = findTexDup(1, buffer, outObjTex,
						                         outRoomTex, numObjectTextures,
						                         numRoomTextures,
						                         numIndirectTextures,
						                         indirectStartPos, flags);
					}
					else
					{
						freeuint164 = findTexDup(2, buffer, outObjTex,
						                         outRoomTex, numObjectTextures,
						                         numRoomTextures,
						                         numIndirectTextures,
						                         indirectStartPos, flags);
					}
					if (freeuint164 == 0xFFFF)
					{
						/* Sets the offset in the output list */
						if ((textureFlags[((freeuint321 << 2U) + 0x00000001)] &
						     TEXTUREFLAG_DIRECT) != (INTU8) 0x00)
						{
							freelong1 = (long int) (numRoomTextures * 0x0030);
						}
						else
						{
							freelong1 = (long int)
								((indirectStartPos + numIndirectTextures) * 0x0030);
						}
						
						/* Copies the texture to the list */
						memcpy(&outRoomTex[freelong1], buffer, (size_t) 16);
						freelong1 += 16l;
					
						/* Finds the first low-res copy of this texture */
						for (freeuint164 = 0x0000; freeuint164 < numTextures;
						     ++freeuint164)
						{
							if ((textures[freeuint164].id ==
							     textures[freeuint161].id) &&
							    (textures[freeuint164].scale == (INTU8) 0x01))
							{
								break;
							}
						}
						
						/* Sets the coordinates for the lower-res version */
						buffer[0] -= textures[freeuint161].outX;
						buffer[0] >>= 1U;
						buffer[0] += textures[freeuint164].outX;
						buffer[1] -= textures[freeuint161].outY;
						buffer[1] >>= 1U;
						buffer[1] += textures[freeuint164].outY;
						buffer[4] -= textures[freeuint161].outX;
						buffer[4] >>= 1U;
						buffer[4] += textures[freeuint164].outX;
						buffer[5] -= textures[freeuint161].outY;
						buffer[5] >>= 1U;
						buffer[5] += textures[freeuint164].outY;
						buffer[8] -= textures[freeuint161].outX;
						buffer[8] >>= 1U;
						buffer[8] += textures[freeuint164].outX;
						buffer[9] -= textures[freeuint161].outY;
						buffer[9] >>= 1U;
						buffer[9] += textures[freeuint164].outY;
						if (freeptr1[16] != (INTU8) 0x00)
						{
							buffer[12] -= textures[freeuint161].outX;
							buffer[12] >>= 1U;
							buffer[12] += textures[freeuint164].outX;
							buffer[13] -= textures[freeuint161].outY;
							buffer[13] >>= 1U;
							buffer[13] += textures[freeuint164].outY;
						}
						buffer[6] = textures[freeuint164].outTile;
						
						/* Copies the texture to the list */
						memcpy(&outRoomTex[freelong1], buffer, (size_t) 16);
						freelong1 += 16l;
						
						if (notflag(FLAGS_NO_SECOND_MIPMAP))
						{
							/* Restores the backup */
							memcpy(&buffer[0], &buffer[32], 16);
							
							/* Finds the second low-res copy of this texture */
							for (freeuint164 = 0x0000; freeuint164 < numTextures;
							     ++freeuint164)
							{
								if ((textures[freeuint164].id ==
								     textures[freeuint161].id) &&
								    (textures[freeuint164].scale == (INTU8) 0x02))
								{
									break;
								}
							}
							
							/* Sets the coordinates for the lower-res version */
							buffer[0] -= textures[freeuint161].outX;
							buffer[0] >>= 2U;
							buffer[0] += textures[freeuint164].outX;
							buffer[1] -= textures[freeuint161].outY;
							buffer[1] >>= 2U;
							buffer[1] += textures[freeuint164].outY;
							buffer[4] -= textures[freeuint161].outX;
							buffer[4] >>= 2U;
							buffer[4] += textures[freeuint164].outX;
							buffer[5] -= textures[freeuint161].outY;
							buffer[5] >>= 2U;
							buffer[5] += textures[freeuint164].outY;
							buffer[8] -= textures[freeuint161].outX;
							buffer[8] >>= 2U;
							buffer[8] += textures[freeuint164].outX;
							buffer[9] -= textures[freeuint161].outY;
							buffer[9] >>= 2U;
							buffer[9] += textures[freeuint164].outY;
							if (freeptr1[16] != (INTU8) 0x00)
							{
								buffer[12] -= textures[freeuint161].outX;
								buffer[12] >>= 2U;
								buffer[12] += textures[freeuint164].outX;
								buffer[13] -= textures[freeuint161].outY;
								buffer[13] >>= 2U;
								buffer[13] += textures[freeuint164].outY;
							}
							buffer[6] = textures[freeuint164].outTile;
						}
						
						/* Copies the texture to the list */
						memcpy(&outRoomTex[freelong1], buffer, (size_t) 16);
						freelong1 += 16l;
						
						/* Remembers this texture was converted */
						freeuint322 = (INTU32) (freeuint321 << 2U);
						++freeuint322;
						textureFlags[freeuint322] |= TEXTUREFLAG_CONVERTED;
					
						/* Sets the offset in the output list */
						if ((textureFlags[freeuint322] & TEXTUREFLAG_DIRECT) !=
						    (INTU8) 0x00)
						{
							roomTextures[freeuint322] = numRoomTextures++;
						}
						else
						{
							roomTextures[freeuint322] = (indirectStartPos +
							                             numIndirectTextures);
							numIndirectTextures++;
						}
					}
					else
					{
						/* Remembers this texture was converted */
						freeuint322 = (INTU32) (freeuint321 << 2U);
						++freeuint322;
						textureFlags[freeuint322] |= TEXTUREFLAG_CONVERTED;
						roomTextures[freeuint322] = freeuint164;
					}
				}

				/* Restores the backup */
				memcpy(&buffer[0], &buffer[32], 16);
	
				/* Converts it if used as a room texture */
				if ((textureFlags[((freeuint321 << 2U) + 0x00000002)] &
				     TEXTUREFLAG_WORLD) !=(INTU8) 0x00)
				{
					/* Sets to the correct palette */
					memcpy(&freeuint163, &buffer[2], 2);
#ifdef __BIG_ENDIAN__
					freeuint163 = reverse16(freeuint163);
#endif
					freeuint163 = altPalettes[freeuint163];
					buffer[2] = (INTU8) (freeuint163 & 0x00FF);
					buffer[3] = (INTU8) ((freeuint163 & 0xFF00) >> 8U);
					
					/* Checks whether this texture already exists */
					if ((textureFlags[((freeuint321 << 2U) + 0x00000002)] &
					     TEXTUREFLAG_DIRECT) != (INTU8) 0x00)
					{
						freeuint164 = findTexDup(1, buffer, outObjTex,
						                         outRoomTex, numObjectTextures,
						                         numRoomTextures,
						                         numIndirectTextures,
						                         indirectStartPos, flags);
					}
					else
					{
						freeuint164 = findTexDup(2, buffer, outObjTex,
						                         outRoomTex, numObjectTextures,
						                         numRoomTextures,
						                         numIndirectTextures,
						                         indirectStartPos, flags);
					}
					if (freeuint164 == 0xFFFF)
					{
						/* Sets the offset in the output list */
						if ((textureFlags[((freeuint321 << 2U) + 0x00000002)] &
						     TEXTUREFLAG_DIRECT) != (INTU8) 0x00)
						{
							freelong1 = (long int) (numRoomTextures * 0x0030);
						}
						else
						{
							freelong1 = (long int)
								((indirectStartPos + numIndirectTextures) * 0x0030);
						}
						
						/* Copies the texture to the list */
						memcpy(&outRoomTex[freelong1], buffer, (size_t) 16);
						freelong1 += 16l;
					
						/* Finds the first low-res copy of this texture */
						for (freeuint164 = 0x0000; freeuint164 < numTextures;
						     ++freeuint164)
						{
							if ((textures[freeuint164].id ==
							     textures[freeuint161].id) &&
							    (textures[freeuint164].scale == (INTU8) 0x01))
							{
								break;
							}
						}
						
						/* Sets the coordinates for the lower-res version */
						buffer[0] -= textures[freeuint161].outX;
						buffer[0] >>= 1U;
						buffer[0] += textures[freeuint164].outX;
						buffer[1] -= textures[freeuint161].outY;
						buffer[1] >>= 1U;
						buffer[1] += textures[freeuint164].outY;
						buffer[4] -= textures[freeuint161].outX;
						buffer[4] >>= 1U;
						buffer[4] += textures[freeuint164].outX;
						buffer[5] -= textures[freeuint161].outY;
						buffer[5] >>= 1U;
						buffer[5] += textures[freeuint164].outY;
						buffer[8] -= textures[freeuint161].outX;
						buffer[8] >>= 1U;
						buffer[8] += textures[freeuint164].outX;
						buffer[9] -= textures[freeuint161].outY;
						buffer[9] >>= 1U;
						buffer[9] += textures[freeuint164].outY;
						if (freeptr1[16] != (INTU8) 0x00)
						{
							buffer[12] -= textures[freeuint161].outX;
							buffer[12] >>= 1U;
							buffer[12] += textures[freeuint164].outX;
							buffer[13] -= textures[freeuint161].outY;
							buffer[13] >>= 1U;
							buffer[13] += textures[freeuint164].outY;
						}
						buffer[6] = textures[freeuint164].outTile;
						
						/* Copies the texture to the list */
						memcpy(&outRoomTex[freelong1], buffer, (size_t) 16);
						freelong1 += 16l;
						
						if (notflag(FLAGS_NO_SECOND_MIPMAP))
						{
							/* Restores the backup */
							memcpy(&buffer[0], &buffer[32], 16);
							
							/* Finds the second low-res copy of this texture */
							for (freeuint164 = 0x0000; freeuint164 < numTextures;
							     ++freeuint164)
							{
								if ((textures[freeuint164].id ==
								     textures[freeuint161].id) &&
								    (textures[freeuint164].scale == (INTU8) 0x02))
								{
									break;
								}
							}
							
							/* Sets the coordinates for the lower-res version */
							buffer[0] -= textures[freeuint161].outX;
							buffer[0] >>= 2U;
							buffer[0] += textures[freeuint164].outX;
							buffer[1] -= textures[freeuint161].outY;
							buffer[1] >>= 2U;
							buffer[1] += textures[freeuint164].outY;
							buffer[4] -= textures[freeuint161].outX;
							buffer[4] >>= 2U;
							buffer[4] += textures[freeuint164].outX;
							buffer[5] -= textures[freeuint161].outY;
							buffer[5] >>= 2U;
							buffer[5] += textures[freeuint164].outY;
							buffer[8] -= textures[freeuint161].outX;
							buffer[8] >>= 2U;
							buffer[8] += textures[freeuint164].outX;
							buffer[9] -= textures[freeuint161].outY;
							buffer[9] >>= 2U;
							buffer[9] += textures[freeuint164].outY;
							if (freeptr1[16] != (INTU8) 0x00)
							{
								buffer[12] -= textures[freeuint161].outX;
								buffer[12] >>= 2U;
								buffer[12] += textures[freeuint164].outX;
								buffer[13] -= textures[freeuint161].outY;
								buffer[13] >>= 2U;
								buffer[13] += textures[freeuint164].outY;
							}
							buffer[6] = textures[freeuint164].outTile;
						}
						
						/* Copies the texture to the list */
						memcpy(&outRoomTex[freelong1], buffer, (size_t) 16);
						freelong1 += 16l;
						
						/* Remembers this texture was converted */
						freeuint322 = (INTU32) (freeuint321 << 2U);
						freeuint322 += 0x00000002;
						textureFlags[freeuint322] |= TEXTUREFLAG_CONVERTED;
					
						/* Sets the offset in the output list */
						if ((textureFlags[freeuint322] & TEXTUREFLAG_DIRECT) !=
						    (INTU8) 0x00)
						{
							roomTextures[freeuint322] = numRoomTextures++;
						}
						else
						{
							roomTextures[freeuint322] = (indirectStartPos +
							                             numIndirectTextures);
							numIndirectTextures++;
						}
					}
					else
					{
						/* Remembers this texture was converted */
						freeuint322 = (INTU32) (freeuint321 << 2U);
						freeuint322 += 0x00000002;
						textureFlags[freeuint322] |= TEXTUREFLAG_CONVERTED;
						roomTextures[freeuint322] = freeuint164;
					}
				}

				/* Restores the backup */
				memcpy(&buffer[0], &buffer[32], 16);
	
				/* Converts it if used as a room texture */
				if ((textureFlags[((freeuint321 << 2U) + 0x00000003)] &
				     TEXTUREFLAG_WORLD) !=(INTU8) 0x00)
				{
					/* Sets to the correct palette */
					memcpy(&freeuint163, &buffer[2], 2);
#ifdef __BIG_ENDIAN__
					freeuint163 = reverse16(freeuint163);
#endif
					freeuint163 = altPalettes[freeuint163];
					buffer[2] = (INTU8) (freeuint163 & 0x00FF);
					buffer[3] = (INTU8) ((freeuint163 & 0xFF00) >> 8U);
					
					/* Flips the coordinates */
					freeuint81 = buffer[0];
					buffer[0] = buffer[4];
					buffer[4] = freeuint81;
					if (freeptr1[16] != (INTU8) 0x00)
					{
						freeuint81 = buffer[8];
						buffer[8] = buffer[12];
						buffer[12] = freeuint81;
					}
						
					/* Checks whether this texture already exists */
					if ((textureFlags[((freeuint321 << 2U) + 0x00000003)] &
					     TEXTUREFLAG_DIRECT) != (INTU8) 0x00)
					{
						freeuint164 = findTexDup(1, buffer, outObjTex,
						                         outRoomTex, numObjectTextures,
						                         numRoomTextures,
						                         numIndirectTextures,
						                         indirectStartPos, flags);
					}
					else
					{
						freeuint164 = findTexDup(2, buffer, outObjTex,
						                         outRoomTex, numObjectTextures,
						                         numRoomTextures,
						                         numIndirectTextures,
						                         indirectStartPos, flags);
					}
					if (freeuint164 == 0xFFFF)
					{
						/* Sets the offset in the output list */
						if ((textureFlags[((freeuint321 << 2U) + 0x00000003)] &
						     TEXTUREFLAG_DIRECT) != (INTU8) 0x00)
						{
							freelong1 = (long int) (numRoomTextures * 0x0030);
						}
						else
						{
							freelong1 = (long int)
								((indirectStartPos + numIndirectTextures) * 0x0030);
						}
						
						/* Copies the texture to the list */
						memcpy(&outRoomTex[freelong1], buffer, (size_t) 16);
						freelong1 += 16l;
					
						/* Finds the first low-res copy of this texture */
						for (freeuint164 = 0x0000; freeuint164 < numTextures;
						     ++freeuint164)
						{
							if ((textures[freeuint164].id ==
							     textures[freeuint161].id) &&
							    (textures[freeuint164].scale == (INTU8) 0x01))
							{
								break;
							}
						}
						
						/* Sets the coordinates for the lower-res version */
						buffer[0] -= textures[freeuint161].outX;
						buffer[0] >>= 1U;
						buffer[0] += textures[freeuint164].outX;
						buffer[1] -= textures[freeuint161].outY;
						buffer[1] >>= 1U;
						buffer[1] += textures[freeuint164].outY;
						buffer[4] -= textures[freeuint161].outX;
						buffer[4] >>= 1U;
						buffer[4] += textures[freeuint164].outX;
						buffer[5] -= textures[freeuint161].outY;
						buffer[5] >>= 1U;
						buffer[5] += textures[freeuint164].outY;
						buffer[8] -= textures[freeuint161].outX;
						buffer[8] >>= 1U;
						buffer[8] += textures[freeuint164].outX;
						buffer[9] -= textures[freeuint161].outY;
						buffer[9] >>= 1U;
						buffer[9] += textures[freeuint164].outY;
						if (freeptr1[16] != (INTU8) 0x00)
						{
							buffer[12] -= textures[freeuint161].outX;
							buffer[12] >>= 1U;
							buffer[12] += textures[freeuint164].outX;
							buffer[13] -= textures[freeuint161].outY;
							buffer[13] >>= 1U;
							buffer[13] += textures[freeuint164].outY;
						}
						buffer[6] = textures[freeuint164].outTile;
						
						/* Copies the texture to the list */
						memcpy(&outRoomTex[freelong1], buffer, (size_t) 16);
						freelong1 += 16l;
						
						if (notflag(FLAGS_NO_SECOND_MIPMAP))
						{
							/* Restores the backup */
							memcpy(&buffer[0], &buffer[32], 16);
							
							/* Finds the second low-res copy of this texture */
							for (freeuint164 = 0x0000; freeuint164 < numTextures;
							     ++freeuint164)
							{
								if ((textures[freeuint164].id ==
								     textures[freeuint161].id) &&
								    (textures[freeuint164].scale == (INTU8) 0x02))
								{
									break;
								}
							}
							
							/* Sets the coordinates for the lower-res version */
							buffer[0] -= textures[freeuint161].outX;
							buffer[0] >>= 2U;
							buffer[0] += textures[freeuint164].outX;
							buffer[1] -= textures[freeuint161].outY;
							buffer[1] >>= 2U;
							buffer[1] += textures[freeuint164].outY;
							buffer[4] -= textures[freeuint161].outX;
							buffer[4] >>= 2U;
							buffer[4] += textures[freeuint164].outX;
							buffer[5] -= textures[freeuint161].outY;
							buffer[5] >>= 2U;
							buffer[5] += textures[freeuint164].outY;
							buffer[8] -= textures[freeuint161].outX;
							buffer[8] >>= 2U;
							buffer[8] += textures[freeuint164].outX;
							buffer[9] -= textures[freeuint161].outY;
							buffer[9] >>= 2U;
							buffer[9] += textures[freeuint164].outY;
							if (freeptr1[16] != (INTU8) 0x00)
							{
								buffer[12] -= textures[freeuint161].outX;
								buffer[12] >>= 2U;
								buffer[12] += textures[freeuint164].outX;
								buffer[13] -= textures[freeuint161].outY;
								buffer[13] >>= 2U;
								buffer[13] += textures[freeuint164].outY;
							}
							buffer[6] = textures[freeuint164].outTile;
						}
						
						/* Copies the texture to the list */
						memcpy(&outRoomTex[freelong1], buffer, (size_t) 16);
						freelong1 += 16l;
						
						/* Remembers this texture was converted */
						freeuint322 = (INTU32) (freeuint321 << 2U);
						freeuint322 += 0x00000003;
						textureFlags[freeuint322] |= TEXTUREFLAG_CONVERTED;
					
						/* Sets the offset in the output list */
						if ((textureFlags[freeuint322] & TEXTUREFLAG_DIRECT) !=
						    (INTU8) 0x00)
						{
							roomTextures[freeuint322] = numRoomTextures++;
						}
						else
						{
							roomTextures[freeuint322] = (indirectStartPos +
							                             numIndirectTextures);
							numIndirectTextures++;
						}
					}
					else
					{
						/* Remembers this texture was converted */
						freeuint322 = (INTU32) (freeuint321 << 2U);
						freeuint322 += 0x00000003;
						textureFlags[freeuint322] |= TEXTUREFLAG_CONVERTED;
						roomTextures[freeuint322] = freeuint164;
					}
				}
			}
			
			/* Moves on to the next Object Texture */
			freeptr1 += 20;
		}
		
		/* Converts Sprite Textures */
		freeptr1 = spriteTextures;
		for (freeuint321 = 0x00000000;
		     freeuint321 < pc_info->v_NumSpriteTextures; ++freeuint321)
		{
			/* Skips this sprite texture if it was already converted */
			if ((textureFlags[((pc_info->v_NumObjectTextures << 2U) +
			                   freeuint321)] & TEXTUREFLAG_CONVERTED) !=
			    (INTU8) 0x00)
			{
				freeptr1 += 16;
				continue;
			}
			
			/* Sees if this texture overlaps with the texture "block" */
			if (freeptr1[0] != textures[freeuint161].inTile)
			{
				freeptr1 += 16;
				continue;
			}
			if ((freeptr1[2] != minX) || (freeptr1[3] != minY))
			{
				freeptr1 += 16;
				continue;
			}
			
			/* Converts the sprite texture's edges and tile to PSX */
			buffer[16] = freeptr1[8];
			buffer[17] = freeptr1[9];
			buffer[18] = freeptr1[10];
			buffer[19] = freeptr1[11];
			buffer[20] = freeptr1[12];
			buffer[21] = freeptr1[13];
			buffer[22] = freeptr1[14];
			buffer[23] = freeptr1[15];
			
			/* Adds the new palette offset to this sprite texture */
			buffer[24] = (INTU8) (freeuint162 & ((INTU16) 0x00FF));
			buffer[25] = (INTU8) ((freeuint162 & ((INTU16) 0xFF00)) >> 8);
			
			/* Converts the tile to PSX */
			buffer[26] = textures[freeuint161].outTile;
			buffer[27] = (INTU8) 0x00;
			
			/* Converts the sprite texture's coordinates to PSX */
			buffer[28] = freeptr1[2];
			buffer[29] = freeptr1[3];
			buffer[30] = (freeptr1[2] + freeptr1[5]);
			buffer[31] = (freeptr1[3] + freeptr1[7]);
			
			/* Changes the coordinates to suit the tiling */
			buffer[28] -= textures[freeuint161].inX;
			buffer[28] += textures[freeuint161].outX;
			buffer[29] -= textures[freeuint161].inY;
			buffer[29] += textures[freeuint161].outY;
			buffer[30] -= textures[freeuint161].inX;
			buffer[30] += textures[freeuint161].outX;
			buffer[31] -= textures[freeuint161].inY;
			buffer[31] += textures[freeuint161].outY;
			
			/* Writes sprite texture to PSX */
			freelong1 = (long int) (psx_info->p_NumSpriteTextures + 4 +
			                        (freeuint321 * 16));
			freeint1 = fseek(psx, freelong1, SEEK_SET);
			if (freeint1 != 0)
			{
				errorNumber = ERROR_PSX_READ_FAILED;
				goto end;
			}
			freesizet1 = fwrite(&buffer[16], 1, 16, psx);
			if (freesizet1 != 16)
			{
				errorNumber = ERROR_PSX_WRITE_FAILED;
				goto end;
			}
			
			/* Remembers that this texture was converted */
			textureFlags[((pc_info->v_NumObjectTextures << 2U) +
			              freeuint321)] |= TEXTUREFLAG_CONVERTED;
			freeptr1 += 16;
		}
	}
	
	/* Adjust the conversion table to skip duplicates */
	freeuint161 = (indirectStartPos - numRoomTextures);
	for (freeuint321 = 0x00000000;
	     freeuint321 < (pc_info->v_NumObjectTextures << 2U);
	     ++freeuint321)
	{
		if ((textureFlags[freeuint321] & TEXTUREFLAG_INDIRECT) != (INTU8) 0x00)
		{
			if ((roomTextures[freeuint321] != 0xFFFF) &&
			    (roomTextures[freeuint321] >= indirectStartPos))
			{
				roomTextures[freeuint321] -= freeuint161;
			}
		}
	}
	
	/* Adjusts the number of ObjectTexture Structs */
	freeuint321 = (INTU32) (numObjectTextures + 0x0100);
	if (freeuint321 > psx_info->v_NumObjectTextures)
	{
		/* Adds the needed space */
		freeuint322 = (freeuint321 - psx_info->v_NumObjectTextures);
		freeuint322 *= 0x00000010;
		errorNumber = insertbytes(psx, psx_info->p_NumObjectTextures,
		                          freeuint322);
		if (errorNumber != ERROR_NONE)
		{
			goto end;
		}
		level_info_add(psx_info, psx_info->p_NumObjectTextures, freeuint322);
		
		/* Sets NumObjectTextures */
		psx_info->v_NumObjectTextures = freeuint321;
#ifdef __BIG_ENDIAN__
		freeuint321 = reverse32(freeuint321);
#endif
		freeint1 = fseek(psx, (long int) psx_info->p_NumObjectTextures,
		                 SEEK_SET);
		if (freeint1 != 0)
		{
			errorNumber = ERROR_PSX_READ_FAILED;
			goto end;
		}
		freesizet1 = fwrite(&freeuint321, 1, 4, psx);
		if (freesizet1 != 4)
		{
			errorNumber = ERROR_PSX_WRITE_FAILED;
			goto end;
		}
	}
	else if (freeuint321 < psx_info->v_NumObjectTextures)
	{
		/* Removes the unneeded space */
		freeuint322 = (psx_info->v_NumObjectTextures - freeuint321);
		freeuint322 *= 0x00000010;
		errorNumber = removebytes(psx, psxpath, psx_info->p_NumObjectTextures,
		                          freeuint322);
		if (errorNumber != ERROR_NONE)
		{
			goto end;
		}
		level_info_sub(psx_info, psx_info->p_NumObjectTextures, freeuint322);
		
		/* Sets NumObjectTextures */
		psx_info->v_NumObjectTextures = freeuint321;
#ifdef __BIG_ENDIAN__
		freeuint321 = reverse32(freeuint321);
#endif
		freeint1 = fseek(psx, (long int) psx_info->p_NumObjectTextures,
		                 SEEK_SET);
		if (freeint1 != 0)
		{
			errorNumber = ERROR_PSX_READ_FAILED;
			goto end;
		}
		freesizet1 = fwrite(&freeuint321, 1, 4, psx);
		if (freesizet1 != 4)
		{
			errorNumber = ERROR_PSX_WRITE_FAILED;
			goto end;
		}
	}
	
	/* Writes the object textures to the PSX file */
	freelong1 = (long int) (psx_info->p_NumObjectTextures + 0x1004);
	freeint1 = fseek(psx, freelong1, SEEK_SET);
	if (freeint1 != 0)
	{
		errorNumber = ERROR_PSX_READ_FAILED;
		goto end;
	}
	freeuint321 = (INTU32) (numObjectTextures * 0x0010);
	freesizet1 = (size_t) fwrite(outObjTex, (size_t) 1,
	                             (size_t) freeuint321, psx);
	if (freesizet1 != (size_t) freeuint321)
	{
		errorNumber = ERROR_PSX_WRITE_FAILED;
		goto end;
	}
	
	/* Adjusts the number of RoomTexture Structs */
	freeuint321 = (INTU32) (numRoomTextures + numIndirectTextures);
	if (freeuint321 > psx_info->v_NumRoomTextures)
	{
		/* Adds the needed space */
		freeuint322 = (freeuint321 - psx_info->v_NumRoomTextures);
		freeuint322 *= 0x00000030;
		errorNumber = insertbytes(psx, psx_info->p_NumRoomTextures,
		                          freeuint322);
		if (errorNumber != ERROR_NONE)
		{
			goto end;
		}
		level_info_add(psx_info, psx_info->p_NumRoomTextures, freeuint322);
		
		/* Sets NumRoomTextures */
		psx_info->v_NumRoomTextures = freeuint321;
#ifdef __BIG_ENDIAN__
		freeuint321 = reverse32(freeuint321);
#endif
		freeint1 = fseek(psx, (long int) psx_info->p_NumRoomTextures,
		                 SEEK_SET);
		if (freeint1 != 0)
		{
			errorNumber = ERROR_PSX_READ_FAILED;
			goto end;
		}
		freesizet1 = fwrite(&freeuint321, 1, 4, psx);
		if (freesizet1 != 4)
		{
			errorNumber = ERROR_PSX_WRITE_FAILED;
			goto end;
		}
	}
	else if (freeuint321 < psx_info->v_NumRoomTextures)
	{
		/* Removes the unneeded space */
		freeuint322 = (psx_info->v_NumRoomTextures - freeuint321);
		freeuint322 *= 0x00000030;
		errorNumber = removebytes(psx, psxpath, psx_info->p_NumRoomTextures,
		                          freeuint322);
		if (errorNumber != ERROR_NONE)
		{
			goto end;
		}
		level_info_sub(psx_info, psx_info->p_NumRoomTextures, freeuint322);
		
		/* Sets NumRoomTextures */
		psx_info->v_NumRoomTextures = freeuint321;
#ifdef __BIG_ENDIAN__
		freeuint321 = reverse32(freeuint321);
#endif
		freeint1 = fseek(psx, (long int) psx_info->p_NumRoomTextures,
		                 SEEK_SET);
		if (freeint1 != 0)
		{
			errorNumber = ERROR_PSX_READ_FAILED;
			goto end;
		}
		freesizet1 = fwrite(&freeuint321, 1, 4, psx);
		if (freesizet1 != 4)
		{
			errorNumber = ERROR_PSX_WRITE_FAILED;
			goto end;
		}
	}
	
	/* Writes the room textures to the PSX file */
	freelong1 = (long int) (psx_info->p_NumRoomTextures + 0x00000004);
	freeint1 = fseek(psx, freelong1, SEEK_SET);
	if (freeint1 != 0)
	{
		errorNumber = ERROR_PSX_READ_FAILED;
		goto end;
	}
	freeuint321 = (INTU32) (numRoomTextures * 0x0030);
	freesizet1 = (size_t) fwrite(outRoomTex, (size_t) 1,
	                             (size_t) freeuint321, psx);
	if (freesizet1 != (size_t) freeuint321)
	{
		errorNumber = ERROR_PSX_WRITE_FAILED;
		goto end;
	}
	freeuint321 = (INTU32) (numIndirectTextures * 0x0030);
	freeuint322 = (INTU32) (indirectStartPos * 0x0030);
	freesizet1 = (size_t) fwrite(&outRoomTex[freeuint322], (size_t) 1,
	                             (size_t) freeuint321, psx);
	if (freesizet1 != (size_t) freeuint321)
	{
		errorNumber = ERROR_PSX_WRITE_FAILED;
		goto end;
	}
	free(outObjTex);
	free(outRoomTex);
	outObjTex = NULL;
	outRoomTex = NULL;
	
	/* Allocates a buffer for the conversion of colour entries */
	buffer2 = malloc(128);
	if (buffer2 == NULL)
	{
		errorNumber = ERROR_MEMORY;
		goto end;
	}
	
	/* Converts colour entries */
	for (freeuint161 = 0x0000; freeuint161 < 0x0100; ++freeuint161)
	{
		/* Loads the current colour into memory */
		freelong1 = (long int) (772 + (freeuint161 * 0x0004));
		freeint1 = fseek(pc, freelong1, SEEK_SET);
		if (freeint1 != 0)
		{
			errorNumber = ERROR_PC_READ_FAILED;
			goto end;
		}
		freesizet1 = fread(&freeuint81, 1, 1, pc);
		if (freesizet1 != 1)
		{
			errorNumber = ERROR_PC_READ_FAILED;
			goto end;
		}
		freesizet1 = fread(&freeuint82, 1, 1, pc);
		if (freesizet1 != 1)
		{
			errorNumber = ERROR_PC_READ_FAILED;
			goto end;
		}
		freesizet1 = fread(&freeuint83, 1, 1, pc);
		if (freesizet1 != 1)
		{
			errorNumber = ERROR_PC_READ_FAILED;
			goto end;
		}
		
		/* Converts the colour to 16-bit */
		freeuint162 = colour2416(freeuint83, freeuint82, freeuint81);
		
		/* Adds the colour to the palette space */
		freeuint321 = (((INTU32) numPalettes) * 0x00000010);
		freeuint321 += (INTU32) (freeuint161 & 0x000F);
		palettes[freeuint321] = freeuint162;
		waterpalettes[freeuint321] = underwaterColour(palettes[freeuint321],
		                                              waterColour);
		
		/* Sets up the palette number, tile and an unknown constant */
		(void) memset(buffer, 0, 16);
		buffer[2] = (INTU8) (numPalettes & 0x00FF);
		buffer[3] = (INTU8) (((INTU16) (numPalettes & 0xFF00)) >> 8);
		buffer[6] = (INTU8) (psx_info->v_NumTexTiles - 0x00000001);
		buffer[7] = (INTU8) 0x00;
		buffer[10] = (INTU8) 0x34;
		buffer[11] = (INTU8) 0x3C;
		buffer[14] = (INTU8) 0x00;
		buffer[15] = (INTU8) 0x00;
		
		/* Increments numPalettes if needed */
		if ((freeuint161 & 0x000F) == 0x000F)
		{
			++numPalettes;
			if (numPalettes >= MAX_PALETTES)
			{
				errorNumber = ERROR_TOO_MANY_PALETTES;
				goto end;
			}
		}
		
		/* Determines the index in the palette */
		freeuint81 = (INTU8) (freeuint161 & 0x000F);
		freeuint82 = (INTU8) (freeuint81 << 4U);
		
		/* Searches for a nibble with the correct value in the texture data */
		freeint2 = 0;
		freelong1 = (long int) (psx_info->p_NumPalettes - 32768);
		if (((flags & FLAGS_LANGUAGE) == FLAGS_LANGUAGE_JP_JP) ||
		    ((flags & FLAGS_LANGUAGE) == FLAGS_LANGUAGE_ASIA_JP))
		{
			freelong1 -= 2l;
		}
		for (freeuint163 = 0x0000; freeuint163 < 0x0100; ++freeuint163)
		{
			/* Reads a row of pixels into memory */
			freeint1 = fseek(psx, freelong1, SEEK_SET);
			if (freeint1 != 0)
			{
				errorNumber = ERROR_PSX_READ_FAILED;
				goto end;
			}
			freesizet1 = fread(buffer2, 1, 128, psx);
			if (freesizet1 != 128)
			{
				errorNumber = ERROR_PSX_READ_FAILED;
				goto end;
			}
			freelong1 += 128;
			
			/* Searches for the correct nibble */
			for (freeuint164 = 0x0000; freeuint164 < 0x0080; ++freeuint164)
			{
				if ((buffer2[freeuint164] & (INTU8) 0x0F) == freeuint81)
				{
					/* Y-coordinate */
					buffer[1] = (INTU8) (freeuint163 & 0x00FF);
					buffer[5] = buffer[1];
					buffer[9] = buffer[1];
					buffer[13] = buffer[1];
					/* X-coordinate */
					buffer[0] = (INTU8) (freeuint164 << 1);
					buffer[4] = buffer[0];
					buffer[8] = buffer[0];
					buffer[12] = buffer[0];
					
					freeint2 = 1;
					break;
				}
				else if ((buffer2[freeuint164] & (INTU8) 0xF0) == freeuint82)
				{
					/* Y-coordinate */
					buffer[1] = (INTU8) (freeuint163 & 0x00FF);
					buffer[5] = buffer[1];
					buffer[9] = buffer[1];
					buffer[13] = buffer[1];
					/* X-coordinate */
					buffer[0] = (INTU8) ((freeuint164 << 1) | 0x0001);
					buffer[4] = buffer[0];
					buffer[8] = buffer[0];
					buffer[12] = buffer[0];
					
					freeint2 = 1;
					break;
				}
			}
			if (freeint2 == 1)
			{
				break;
			}
		}
		
		/* Writes the Object Texture entry to PSX */
		freelong1 = (long int) (psx_info->p_NumObjectTextures +
		                        4 + (freeuint161 * 16));
		freeint1 = fseek(psx, freelong1, SEEK_SET);
		if (freeint1 != 0)
		{
			errorNumber = ERROR_PSX_READ_FAILED;
			goto end;
		}
		freesizet1 = fwrite(buffer, 1, 16, psx);
		if (freesizet1 != 16)
		{
			errorNumber = ERROR_PSX_WRITE_FAILED;
			goto end;
		}
	}
	
	/* Converts the Sprite Sequences */
	if (isflag(FLAGS_VERBOSE))
	{
		PRINT_DONE
		printf("Converting Sprite Sequences... ");
		(void) fflush(stdout);
	}
	
	/* Removes old sprite sequences */
	freeuint321 = (psx_info->v_NumSpriteSequences * 8);
	errorNumber = removebytes(psx, psxpath, psx_info->p_NumSpriteSequences,
	                          freeuint321);
	if (errorNumber != ERROR_NONE)
	{
		goto end;
	}
	level_info_sub(psx_info, psx_info->p_NumSpriteSequences, freeuint321);
	
	/* Makes room for new sprite sequences */
	freeuint321 = (pc_info->v_NumSpriteSequences * 8);
	errorNumber = insertbytes(psx, psx_info->p_NumSpriteSequences, freeuint321);
	if (errorNumber != ERROR_NONE)
	{
		goto end;
	}
	level_info_add(psx_info, psx_info->p_NumSpriteSequences, freeuint321);
	freeuint321 += 4;
	
	/* Copies sprite sequences */
	errorNumber = copybytes(pc, psx, pc_info->p_NumSpriteSequences,
	                        psx_info->p_NumSpriteSequences, freeuint321);
	if (errorNumber != ERROR_NONE)
	{
		goto end;
	}
	psx_info->v_NumSpriteSequences = pc_info->v_NumSpriteSequences;
	
	/* Flips the bytes in the palettes if needed */
#ifdef __BIG_ENDIAN__
	freeuint322 = (numPalettes * 16);
	for (freeuint321 = 0x00000000; freeuint321 < freeuint322; ++freeuint321)
	{
		palettes[freeuint321] = reverse16(palettes[freeuint321]);
	}
#endif
	
	/* Adds or removes palette space as needed */
	freeuint321 = numPalettes;
	if (notflag(FLAGS_NO_SIGNATURE))
	{
		freeuint321 += 2;
	}
	if (psx_info->v_NumPalettes < freeuint321)
	{
		freeuint322 = (freeuint321 - psx_info->v_NumPalettes);
		freeuint322 *= 64;
		errorNumber = insertbytes(psx, psx_info->p_NumPalettes, freeuint322);
		if (errorNumber != ERROR_NONE)
		{
			goto end;
		}
		level_info_add(psx_info, psx_info->p_NumPalettes, freeuint322);
	}
	else if (psx_info->v_NumPalettes > freeuint321)
	{
		freeuint322 = (psx_info->v_NumPalettes - freeuint321);
		freeuint322 *= 64;
		errorNumber = removebytes(psx, psxpath, psx_info->p_NumPalettes,
		                          freeuint322);
		if (errorNumber != ERROR_NONE)
		{
			goto end;
		}
		level_info_sub(psx_info, psx_info->p_NumPalettes, freeuint322);
	}
	
	/* Updates NumPalettes */
	psx_info->v_NumPalettes = freeuint321;
#ifdef __BIG_ENDIAN__
	freeuint321 = reverse32(freeuint321);
#endif
	freeint1 = fseek(psx, (long int) psx_info->p_NumPalettes, SEEK_SET);
	if (freeint1 != 0)
	{
		errorNumber = ERROR_PSX_READ_FAILED;
		goto end;
	}
	freesizet1 = fwrite(&freeuint321, 1, 4, psx);
	if (freesizet1 != 4)
	{
		errorNumber = ERROR_PSX_WRITE_FAILED;
		goto end;
	}
	
	/* Writes the palette data to PSX */
	freesizet2 = (size_t) (psx_info->v_NumPalettes * 32);
	if (notflag(FLAGS_NO_SIGNATURE))
	{
		freesizet2 -= 64;
	}
	freesizet1 = fwrite(palettes, 1, freesizet2, psx);
	if (freesizet1 != freesizet2)
	{
		errorNumber = ERROR_PSX_WRITE_FAILED;
		goto end;
	}
	
	/* Adds signature to the level file */
	if (notflag(FLAGS_NO_SIGNATURE))
	{
		freelong1 = ftell(psx);
		errorNumber = zerobytes(psx, (INTU32) freelong1, 0x00000040);
		if (errorNumber != 0)
		{
			goto end;
		}
		
		freeint1 = fseek(psx, freelong1, SEEK_SET);
		if (freeint1 != 0)
		{
			goto end;
		}
		
		freeint1 = fputs("Converted using " PROGNAME, psx);
		if (freeint1 == EOF)
		{
			goto end;
		}
		freeint1 = fputs(PROGVER, psx);
		if (freeint1 == EOF)
		{
			goto end;
		}
		freeint1 = fputs(" by b122251\0", psx);
		if (freeint1 == EOF)
		{
			goto end;
		}
	}
	
	/* Writes the underwater palettes */
	freelong1 = (long int) (psx_info->p_NumPalettes + 0x00000004);
	freelong1 += (long int) (psx_info->v_NumPalettes * 0x00000020);
	freeint1 = fseek(psx, freelong1, SEEK_SET);
	if (freeint1 != 0)
	{
		errorNumber = ERROR_PSX_READ_FAILED;
		goto end;
	}
	freesizet2 = (size_t) (psx_info->v_NumPalettes * 32);
	if (notflag(FLAGS_NO_SIGNATURE))
	{
		freesizet2 -= 64;
	}
	freesizet1 = fwrite(waterpalettes, 1, freesizet2, psx);
	if (freesizet1 != freesizet2)
	{
		errorNumber = ERROR_PSX_WRITE_FAILED;
		goto end;
	}
	
	PRINT_DONE;
	
end:/* Frees allocated memory and ends the function */
	if (errorNumber != ERROR_NONE)
	{
		PRINT_FAILED;
	}
	if (objectTextures != NULL)
	{
		free(objectTextures);
	}
	if (spriteTextures != NULL)
	{
		free(spriteTextures);
	}
	if (palettes != NULL)
	{
		free(palettes);
	}
	if (outRoomTex != NULL)
	{
		free(outRoomTex);
	}
	if (outObjTex != NULL)
	{
		free(outObjTex);
	}
	if (waterpalettes != NULL)
	{
		free(waterpalettes);
	}
	if (usedColours != NULL)
	{
		free(usedColours);
	}
	if (altPalettes != NULL)
	{
		free(altPalettes);
	}
	if (textures != NULL)
	{
		free(textures);
	}
	if (buffer != NULL)
	{
		free(buffer);
	}
	if (buffer2 != NULL)
	{
		free(buffer2);
	}
	if (textureFlags != NULL)
	{
		free(textureFlags);
	}
	return errorNumber;
}

/*
 * Function that converts texture data from PC to PSX.
 * Parameters:
 * * pc = Pointer to PC-file
 * * psx = Pointer to PSX-file
 * * inTile = Tile the texture is in
 * * inX = X-position of the top-left corner of the texture
 * * inY = Y-position of the top-left corner of the texture
 * * width = Width of the texture
 * * height = Height of the texture
 * * outTile = Tile to place the texture in
 * * outX = X-position to place the top-left corner of the texture
 * * outY = Y-position to place the top-left corner of the texture
 * * opaque = Whether the texture is opaque or has transparency
 * * m1tile = Tile for the first low-res copy
 * * m1x = X-position for the first low-res copy
 * * m1y = Y-position for the first low-res copy
 * * m2tile = Tile for the second low-res copy
 * * m2x = X-position for the second low-res copy
 * * m2y = Y-position for the second low-res copy
 * * numPalettes = The number of palettes the level contains
 * * palettes = Memory space holding the existing palettes
 * * waterpalettes = Memory space holding the existing underwater palettes
 * * usedColours = How many colours were used in each palette
 * * flags = Bitmask of program-wide flags
 * * paletteNumber = Number of the palette used in this texture
 * * altPalettes = The palette's alternate palettes
 * * world = Whether this texture is used in world geometry
 * * watercolour = Rules for calculating the underwater palette
 * * pc_info = Pointer to the level_info struct relating to PC
 * * psx_info = Pointer to the level_info struct relating to PSX
 */
static int convertTextureData(FILE *pc, FILE *psx, INTU8 inTile, INTU8 inX,
                              INTU8 inY, INTU8 width, INTU8 height,
                              INTU8 outTile, INTU8 outX, INTU8 outY,
                              INTU8 opaque, INTU8 m1tile, INTU8 m1x, INTU8 m1y,
                              INTU8 m2tile, INTU8 m2x, INTU8 m2y,
                              INTU16 *numPalettes,
                              INTU16 *palettes, INTU16 *waterpalettes,
                              INTU8 *usedColours, INTU32 flags,
                              INTU16 *paletteNumber, INTU16 *altPalettes,
                              INTU8 world, struct water_colour *waterColour,
                              struct level_info *pc_info,
                              struct level_info *psx_info)
{
	/* Variable Declarations */
	INTU16 *colourCount = NULL;    /* Space for counting used colours */
	INTU16 *inTexture = NULL;      /* Space for the input texture in memory */
	struct colour *colours = NULL; /* Structs used for colour reduction */
	INTU8 *outTexture = NULL;      /* Space for the output texture in memory */
	INTU16 newPalette[16];         /* Temporary space for a new palette */
	INTU16 input_numColours;       /* Number of colours in the input texture */
	int transparency = 0;          /* Whether transparency is used */
	INTU8 *freeptr1 = NULL;        /* Pointer (general use) */
	INTU32 freeuint321,
	       freeuint322,
	       freeuint323;            /* Unsigned 32-bit integers (general use) */
	INTU16 freeuint161,
	       freeuint162,
	       freeuint163,
	       freeuint164,
	       freeuint165,
	       freeuint166;            /* Unsigned 16-bit integers (general use) */
	INTU8 freeuint81;              /* Unsigned 8-bit integer (general use) */
	int freeint1,
	    freeint2;                  /* Integers (general use) */
	size_t freesizet1;             /* size_t (general use) */
	long int freelong1,
	     freelong2,
	     freelong3;                /* Long integers (general use) */
	int errorNumber;               /* Return value for this function */
	errorNumber = ERROR_NONE;
	
	/* Allocates space for the texture input */
	freeuint321 = (((INTU32) width) * ((INTU32) height) * 2);
	inTexture = malloc((size_t) freeuint321);
	if (inTexture == NULL)
	{
		errorNumber = ERROR_MEMORY;
		goto end;
	}
	
	/* Loads the texture into memory */
	freeuint321 = (INTU32) (1800 + (pc_info->v_NumTexTiles * 65536) +
	                        (((INTU32) inTile) * 131072) +
	                        (((INTU32) inY) * 512) +
	                        (((INTU32) inX) * 2));
	freeuint162 = (INTU16) height;
	freeuint163 = (((INTU16) width) * 2);
	freeptr1 = (INTU8 *) inTexture;
	for (freeuint161 = 0x0000; freeuint161 < freeuint162; ++freeuint161)
	{
		freeint1 = fseek(pc, (long int) freeuint321, SEEK_SET);
		if (freeint1 != 0)
		{
			errorNumber = ERROR_PC_READ_FAILED;
			goto end;
		}
		
		freesizet1 = fread(freeptr1, 1, (size_t) freeuint163, pc);
		if (freesizet1 != (size_t) freeuint163)
		{
			errorNumber = ERROR_PC_READ_FAILED;
			goto end;
		}
		
		freeptr1 += freeuint163;
		freeuint321 += 512;
	}
	freeptr1 = NULL;
	
	/* Adjusts for big-endian */
#ifdef __BIG_ENDIAN__
	freeuint162 = (((INTU16) width) * ((INTU16) height));
	for (freeuint161 = 0x0000; freeuint161 < freeuint162; ++freeuint161)
	{
		inTexture[freeuint161] = reverse16(inTexture[freeuint161]);
	}
#endif
	
	/* Allocates space for counting colours */
	colourCount = calloc((size_t) 0x8001, 2);
	if (colourCount == NULL)
	{
		errorNumber = ERROR_MEMORY;
		goto end;
	}
	
	/* Counts the number of colours used in the texture */
	input_numColours = 0x0000;
	freeuint162 = (((INTU16) width) * ((INTU16) height));
	for (freeuint161 = 0x0000; freeuint161 < freeuint162; ++freeuint161)
	{
		/* Swaps the transparency bit */
		inTexture[freeuint161] ^= 0x8000;
		
		/* Adjusts for transparent pixels */
		if ((inTexture[freeuint161] & 0x8000) != 0x0000)
		{
			inTexture[freeuint161] = 0x8000;
		}
		
		/* Counts the used colour if this is the first pixel in that colour */
		if (colourCount[inTexture[freeuint161]] == 0x0000)
		{
			++input_numColours;
		}
		
		/* Adds this pixel to the number of pixels in this colour */
		++colourCount[inTexture[freeuint161]];
	}
	
	/* Ends the function if no colours were used (shouldn't happen) */
	if (input_numColours == 0x0000)
	{
		goto end;
	}
	
	/* Allocates colours for colour reduction */
	colours = calloc((size_t) input_numColours, sizeof(struct colour));
	if (colours == NULL)
	{
		errorNumber = ERROR_MEMORY;
		goto end;
	}
	
	freeuint162 = 0x0000;
	/* Adjusts for transparency */
	if ((colourCount[0x8000] != 0x0000) && (opaque == (INTU8) 0x00))
	{
		colours[0].in_index = 0x8000;
		transparency = 1;
		++freeuint162;
	}
	
	/* Determines all the input colours */
	for (freeuint161 = 0x0000; freeuint161 < 0x8000; ++freeuint161)
	{
		if (colourCount[freeuint161] != 0x0000)
		{
			colours[freeuint162].in_index = freeuint161;
			colours[freeuint162].weight = colourCount[freeuint161];
			colour1624(freeuint161, colours[freeuint162].r,
			           colours[freeuint162].g, colours[freeuint162].b);
			++freeuint162;
		}
	}
	
	/* Frees up the allocated colour counting space */
	free(colourCount);
	colourCount = NULL;
	
	/* Reduces the colours */
	if (transparency == 0)
	{
		reduceColours(colours, input_numColours, (INTU8) 0x10, newPalette);
	}
	else
	{
		reduceColours(&colours[1], (input_numColours - 0x0001), (INTU8) 0x0F,
		              newPalette);
	}
	freeuint162 = input_numColours;
	if (freeuint162 > 0x0010)
	{
		freeuint162 = 0x0010;
	}
	
	/* Finds a palette to add this one to */
	freeint1 = 0;
	for (freeuint161 = 0x0000; freeuint161 < *numPalettes; ++freeuint161)
	{
		/* Skips alternate palettes */
		if (usedColours[freeuint161] == (INTU8) 0xFF)
		{
			continue;
		}
		
		/* Sees whether the needed colours are present */
		freeuint164 = 0x0000;
		for (freeuint163 = 0x0000; freeuint163 < freeuint162; ++freeuint163)
		{
			for (freeuint165 = 0x0000;
			     freeuint165 < (INTU16) usedColours[freeuint161];
			     ++freeuint165)
			{
				freeuint321 = ((((INTU32) freeuint161) * 16) + freeuint165);
				if (newPalette[freeuint163] == palettes[freeuint321])
				{
					++freeuint164;
					break;
				}
			}
		}
		
		/* A palette that fits has been found */
		if (freeuint164 == freeuint162)
		{
			freeint1 = 1;
			break;
		}
	}
	
	/* Sees if the needed colours can be appended to an existing palette */
	if (freeint1 != 1)
	{
		for (freeuint161 = 0x0000; freeuint161 < *numPalettes; ++freeuint161)
		{
			/* Skips alternate palettes */
			if (usedColours[freeuint161] == (INTU8) 0xFF)
			{
				continue;
			}
			
			/* Sees whether the needed colours are present */
			freeuint164 = 0x0000;
			for (freeuint163 = 0x0000; freeuint163 < freeuint162; ++freeuint163)
			{
				for (freeuint165 = 0x0000;
				     freeuint165 < (INTU16) usedColours[freeuint161];
				     ++freeuint165)
				{
					freeuint321 = ((((INTU32) freeuint161) * 16) + freeuint165);
					if (newPalette[freeuint163] == palettes[freeuint321])
					{
						++freeuint164;
						break;
					}
				}
			}
			
			/* A palette with empty spaces that fit has been found */
			if (((INTU8) 0x10 - usedColours[freeuint161]) >=
			    (freeuint162 - freeuint164))
			{
				freeint1 = 2;
				break;
			}
		}
	}
	
	/* Sets the palette number to be used in the texture struct */
	*paletteNumber = freeuint161;
	
	/* Adds the palette to the list if needed */
	if (freeint1 == 0)
	{
		/* A new palette had to be created for this texture */
		freeuint321 = (((INTU32) * numPalettes) * 16);
		freeuint163 = (freeuint162 * 2);
		memcpy(&palettes[freeuint321], newPalette, (size_t) freeuint163);
		usedColours[*numPalettes] = (INTU8) freeuint162;
		++(*numPalettes);
	}
	else if (freeint1 == 1)
	{
		/* Converts output indices for the found fitting palette */
		for (freeuint163 = 0x0000; freeuint163 < freeuint162; ++freeuint163)
		{
			for (freeuint165 = 0x0000;
			     freeuint165 < (INTU16) usedColours[freeuint161];
			     ++freeuint165)
			{
				freeuint321 = ((((INTU32) freeuint161) * 16) + freeuint165);
				if (newPalette[freeuint163] == palettes[freeuint321])
				{
					for (freeuint164 = 0x0000; freeuint164 < input_numColours;
					     ++freeuint164)
					{
						if (colours[freeuint164].bucket == (INTU8) freeuint163)
						{
							colours[freeuint164].bucket = (INTU8) (freeuint165 |
							                                       0x0080);
						}
					}
					break;
				}
			}
		}
		for (freeuint163 = 0x0000; freeuint163 < input_numColours;
		     ++freeuint163)
		{
			colours[freeuint163].bucket &= 0x7F;
		}
	}
	else if (freeint1 == 2)
	{
		/* Converts output indices for the found colours */
		for (freeuint163 = 0x0000; freeuint163 < freeuint162; ++freeuint163)
		{
			for (freeuint165 = 0x0000;
			     freeuint165 < (INTU16) usedColours[freeuint161];
			     ++freeuint165)
			{
				freeuint321 = ((((INTU32) freeuint161) * 16) + freeuint165);
				if (newPalette[freeuint163] == palettes[freeuint321])
				{
					for (freeuint164 = 0x0000; freeuint164 < input_numColours;
					     ++freeuint164)
					{
						if (colours[freeuint164].bucket == (INTU8) freeuint163)
						{
							colours[freeuint164].bucket = (INTU8) (freeuint165 |
							                                       0x0080);
						}
					}
					break;
				}
			}
			
			if (freeuint165 == (INTU16) usedColours[freeuint161])
			{
				freeuint321 = ((((INTU32) freeuint161) * 16) +
				               (int) usedColours[freeuint161]);
				palettes[freeuint321] = newPalette[freeuint163];
				for (freeuint164 = 0x0000; freeuint164 < input_numColours;
				     ++freeuint164)
				{
					if (colours[freeuint164].bucket == (INTU8) freeuint163)
					{
						colours[freeuint164].bucket =
							(usedColours[freeuint161] | (INTU8) 0x80);
					}
				}
				++usedColours[freeuint161];
			}
		}
		
		for (freeuint163 = 0x0000; freeuint163 < input_numColours;
		     ++freeuint163)
		{
			colours[freeuint163].bucket &= 0x7F;
		}
	}
	
	/* Determines the writing offsets for the texture */
	freelong1 = (long int) (psx_info->p_NumTexTiles + 4 +
	                        (((INTU32) outTile) * 32768) +
	                        (((INTU32) outY) * 128) + (outX >> 1));
	freelong2 = (long int) (psx_info->p_NumTexTiles + 4 +
	                        (((INTU32) m1tile) * 32768) +
	                        (((INTU32) m1y) * 128) + (m1x >> 1));
	freelong3 = (long int) (psx_info->p_NumTexTiles + 4 +
	                        (((INTU32) m2tile) * 32768) +
	                        (((INTU32) m2y) * 128) + (m2x >> 1));
	
	/* Converts and writes the texture line by line to PSX */
	freeuint162 = (INTU16) height;
	for (freeuint161 = 0x0000; freeuint161 < freeuint162; ++freeuint161)
	{
		/* Sets up variables for the conversion of this line */
		outTexture = (INTU8 *) &inTexture[(freeuint161 * ((INTU16) width))];
		freeuint163 = 0x0000;
		freeuint164 = 0x0000;
		freeint2 = 0;
		
		/* Adjusts for the pixel on the left if needed */
		if ((outX & ((INTU8) 0x01)) != ((INTU8) 0x00))
		{
			freeint1 = fseek(psx, freelong1, SEEK_SET);
			if (freeint1 != 0)
			{
				errorNumber = ERROR_PSX_READ_FAILED;
				goto end;
			}
			freesizet1 = fread(&freeuint81, 1, 1, psx);
			if (freesizet1 != 1)
			{
				errorNumber = ERROR_PSX_READ_FAILED;
				goto end;
			}
			outTexture[0] = getBucket(colours, input_numColours,
			                          inTexture[(freeuint161 * width)]);
			outTexture[0] <<= 4;
			outTexture[0] |= (freeuint81 & ((INTU8) 0x0F));
			
			/* Remembers to skip the first pixel of the texture */
			++freeuint163;
			++freeuint164;
		}
		
		/* Converts all the pixels in the texture */
		for (freeuint165 = (INTU16) width;
		     freeuint163 < freeuint165; ++freeuint163)
		{
			freeuint166 = ((freeuint161 * ((INTU16) width)) + freeuint163);
			/* Determines whether this pixel is the high or the low nibble */
			if (freeint2 == 0)
			{
				outTexture[freeuint164] = getBucket(colours, input_numColours,
				                                    inTexture[freeuint166]);
				outTexture[freeuint164] &= (INTU8) 0x0F;
			}
			else
			{
				outTexture[freeuint164] |=
					(getBucket(colours, input_numColours,
					           inTexture[freeuint166]) << 4);
				++freeuint164;
			}
			
			/* Remembers whether we're on an even or an odd pixel */
			freeint2 ^= 1;
		}
		
		/* Adjusts for the pixel to the right of the texture if needed */
		if (freeint2 == 1)
		{
			freeint1 = fseek(psx, (long int) (freelong1 + freeuint164),
			                 SEEK_SET);
			if (freeint1 != 0)
			{
				errorNumber = ERROR_PSX_READ_FAILED;
				goto end;
			}
			freesizet1 = fread(&freeuint81, 1, 1, psx);
			if (freesizet1 != 1)
			{
				errorNumber = ERROR_PSX_READ_FAILED;
				goto end;
			}
			
			outTexture[freeuint164] |= (freeuint81 & ((INTU8) 0xF0));
			++freeuint164;
		}
		
		/* Writes the line to the PSX */
		freeint1 = fseek(psx, freelong1, SEEK_SET);
		if (freeint1 != 0)
		{
			errorNumber = ERROR_PSX_READ_FAILED;
			goto end;
		}
		freesizet1 = fwrite(outTexture, 1, (size_t) freeuint164, psx);
		if (freesizet1 != (size_t) freeuint164)
		{
			errorNumber = ERROR_PSX_WRITE_FAILED;
			goto end;
		}
		
		/* Makes a low-res versions of this line if needed */
		if (((freeuint161 & 0x0001) == 0x0000) &&
		    ((world & (INTU8) 0x21) != (INTU8) 0x00) &&
		    (notflag(FLAGS_NO_MIPMAP_AT_ALL)))
		{
			freeuint165 = 0x0000;
			for (freeuint163 = 0x0000; freeuint163 < freeuint164;
			     freeuint163 += 0x0002)
			{
				outTexture[freeuint165] = outTexture[freeuint163];
				outTexture[freeuint165] &= (INTU8) 0x0F;
				outTexture[freeuint165] |=
					((outTexture[freeuint163] & ((INTU8) 0x0F)) |
					 (outTexture[(freeuint163 + 0x0001)] << 4));
				++freeuint165;
			}
			
			/* Writes the low-res line to the PSX */
			freeint1 = fseek(psx, freelong2, SEEK_SET);
			if (freeint1 != 0)
			{
				errorNumber = ERROR_PSX_READ_FAILED;
				goto end;
			}
			freesizet1 = fwrite(outTexture, 1, (size_t) freeuint165, psx);
			if (freesizet1 != (size_t) freeuint165)
			{
				errorNumber = ERROR_PSX_WRITE_FAILED;
				goto end;
			}
			
			/* Moves the output position to the next line */
			freelong2 += 128;
			
			if (notflag(FLAGS_NO_SECOND_MIPMAP))
			{
				if ((freeuint161 & 0x0003) == 0x0000)
				{
					/* Makes the even lower res copy */
					freeuint164 >>= 1U;
					freeuint165 = 0x0000;
					for (freeuint163 = 0x0000; freeuint163 < freeuint164;
					     freeuint163 += 0x0002)
					{
						outTexture[freeuint165] = outTexture[freeuint163];
						outTexture[freeuint165] &= (INTU8) 0x0F;
						outTexture[freeuint165] |=
							((outTexture[freeuint163] & ((INTU8) 0x0F)) |
							 (outTexture[(freeuint163 + 0x0001)] << 4));
						++freeuint165;
					}
					
					/* Writes the low-res line to the PSX */
					freeint1 = fseek(psx, freelong3, SEEK_SET);
					if (freeint1 != 0)
					{
						errorNumber = ERROR_PSX_READ_FAILED;
						goto end;
					}
					freesizet1 = fwrite(outTexture, 1,
					                    (size_t) freeuint165, psx);
					if (freesizet1 != (size_t) freeuint165)
					{
						errorNumber = ERROR_PSX_WRITE_FAILED;
						goto end;
					}
					
					/* Moves the output position to the next line */
					freelong3 += 128;
				}
			}
		}
		
		/* Moves the output position to the next line */
		freelong1 += 128;
	}
	
	/* Checks whether this texture is used underwater */
	if ((world & (INTU8) 0x04) == (INTU8) 0x04)
	{
		/* Makes the alternate palette if it doesn't exist yet */
		if (altPalettes[*paletteNumber] == 0x0000)
		{
			altPalettes[*paletteNumber] = *numPalettes;
			usedColours[*numPalettes] = (INTU8) 0xFF;
			(*numPalettes)++;
		}
		
		/* Updates the alternate palette */
		freeuint321 = (INTU32) altPalettes[*paletteNumber];
		freeuint321 *= 0x00000010;
		freeuint322 = (freeuint321 + 0x00000010);
		freeuint323 = (INTU32) *paletteNumber;
		freeuint323 *= 0x00000010;
		for (; freeuint321 < freeuint322; ++freeuint321)
		{
			palettes[freeuint321] = underwaterColour(palettes[freeuint323++],
			                                         waterColour);
			waterpalettes[freeuint321] = palettes[freeuint321];
		}
	}
	/* Updates the underwater palette */
	freeuint321 = (INTU32) *paletteNumber;
	freeuint321 *= 0x00000010;
	freeuint322 = (freeuint321 + 0x00000010);
	for (; freeuint321 < freeuint322; ++freeuint321)
	{
		waterpalettes[freeuint321] = underwaterColour(palettes[freeuint321],
		                                              waterColour);
	}
	
end:/* Frees up allocated memory and returns the function */
	if (inTexture != NULL)
	{
		free(inTexture);
	}
	if (colourCount != NULL)
	{
		free(colourCount);
	}
	if (colours != NULL)
	{
		free(colours);
	}
	return errorNumber;
}

/*
 * Function that adds support for Kanji characters to the level.
 * Parameters:
 * * pc = Pointer to PC-file
 * * psx = Pointer to PSX-file
 */
int addKanji(FILE *psx, struct level_info *psx_info)
{
	/* Variable Declarations */
	INTU8 buffer[32];   /* Buffer for the palette and SpriteTexture creation */
	INTU32 freeuint321; /* Unsigned 32-bit integer for general use */
	size_t freesizet1;  /* size_t for general use */
	int freeint1;       /* Integer for general use */
	
	/* Writes the index of the Kanji sprite to the correct location */
	buffer[0] = (INTU8) (psx_info->v_NumSpriteTextures & 0x000000FF);
	buffer[1] = (INTU8) ((psx_info->v_NumSpriteTextures & 0x0000FF00) >> 8);
	freeuint321 = (psx_info->p_NumPalettes - 0x00000002);
	freeint1 = fseek(psx, (long int) freeuint321, SEEK_SET);
	if (freeint1 != 0)
	{
		return ERROR_PSX_READ_FAILED;
	}
	freesizet1 = fwrite(buffer, 1, 2, psx);
	if (freesizet1 != 2)
	{
		return ERROR_PSX_WRITE_FAILED;
	}
	
	/* Sets up the SpriteTexture entry for the Kanji */
	(void) memset(buffer, 0, 16);
	buffer[8] = (INTU8) (psx_info->v_NumPalettes & 0x000000FF);
	buffer[9] = (INTU8) ((psx_info->v_NumPalettes & 0x0000FF00) >> 8U);
	
	/* Makes room for the new SpriteTexture entry */
	freeuint321 = psx_info->p_NumSpriteSequences;
	freeint1 = insertbytes(psx, freeuint321, 0x00000010);
	if (freeint1 != ERROR_NONE)
	{
		return freeint1;
	}
	level_info_add(psx_info, freeuint321, 0x00000010);
	psx_info->p_NumSpriteSequences += 0x00000010;
	
	/* Writes the SpriteTexture to PSX */
	freeint1 = fseek(psx, (long int) freeuint321, SEEK_SET);
	if (freeint1 != 0)
	{
		return ERROR_PSX_READ_FAILED;
	}
	freesizet1 = fwrite(buffer, 1, 16, psx);
	if (freesizet1 != 16)
	{
		return ERROR_PSX_WRITE_FAILED;
	}
	
	/* Updates NumSpriteTextures */
	psx_info->v_NumSpriteTextures++;
	freeuint321 = psx_info->v_NumSpriteTextures;
#ifdef __BIG_ENDIAN__
	freeuint321 = reverse32(freeuint321);
#endif
	freeint1 = fseek(psx, (long int) psx_info->p_NumSpriteTextures, SEEK_SET);
	if (freeint1 != 0)
	{
		return ERROR_PSX_READ_FAILED;
	}
	freesizet1 = fwrite(&freeuint321, 1, 4, psx);
	if (freesizet1 != 4)
	{
		return ERROR_PSX_WRITE_FAILED;
	}
	
	/* Sets up the palette in memory */
	buffer[0] = (INTU8) 0x00;
	buffer[1] = (INTU8) 0x00;
	buffer[2] = (INTU8) 0xFF;
	buffer[3] = (INTU8) 0xFF;
	buffer[4] = (INTU8) 0xFF;
	buffer[5] = (INTU8) 0xFF;
	buffer[6] = (INTU8) 0xFF;
	buffer[7] = (INTU8) 0xFF;
	buffer[8] = (INTU8) 0xFF;
	buffer[9] = (INTU8) 0xFF;
	buffer[10] = (INTU8) 0xFF;
	buffer[11] = (INTU8) 0xFF;
	buffer[12] = (INTU8) 0xFF;
	buffer[13] = (INTU8) 0xFF;
	buffer[14] = (INTU8) 0xFF;
	buffer[15] = (INTU8) 0xFF;
	buffer[16] = (INTU8) 0xFF;
	buffer[17] = (INTU8) 0xFF;
	buffer[18] = (INTU8) 0xFF;
	buffer[19] = (INTU8) 0xFF;
	buffer[20] = (INTU8) 0xFF;
	buffer[21] = (INTU8) 0xFF;
	buffer[22] = (INTU8) 0xFF;
	buffer[23] = (INTU8) 0xFF;
	buffer[24] = (INTU8) 0xFF;
	buffer[25] = (INTU8) 0xFF;
	buffer[26] = (INTU8) 0xFF;
	buffer[27] = (INTU8) 0xFF;
	buffer[28] = (INTU8) 0xFF;
	buffer[29] = (INTU8) 0xFF;
	buffer[30] = (INTU8) 0xFF;
	buffer[31] = (INTU8) 0xFF;
	
	/* Makes room for the new palette */
	freeuint321 = (psx_info->p_NumPalettes + 4 +
	               (psx_info->v_NumPalettes * 64));
	freeint1 = insertbytes(psx, freeuint321, 32);
	if (freeint1 != ERROR_NONE)
	{
		return freeint1;
	}
	freeuint321 = (psx_info->p_NumPalettes + 4 +
	               (psx_info->v_NumPalettes * 32));
	freeint1 = insertbytes(psx, freeuint321, 32);
	if (freeint1 != ERROR_NONE)
	{
		return freeint1;
	}
	level_info_add(psx_info, freeuint321, 64);
	
	/* Writes the palette to PSX */
	freeint1 = fseek(psx, (long int) freeuint321, SEEK_SET);
	if (freeint1 != 0)
	{
		return ERROR_PSX_READ_FAILED;
	}
	freesizet1 = fwrite(buffer, 1, 32, psx);
	if (freesizet1 != 32)
	{
		return ERROR_PSX_WRITE_FAILED;
	}
	
	/* Updates NumPalettes */
	psx_info->v_NumPalettes++;
	freeuint321 = psx_info->v_NumPalettes;
#ifdef __BIG_ENDIAN__
	freeuint321 = reverse32(freeuint321);
#endif
	freeint1 = fseek(psx, (long int) psx_info->p_NumPalettes, SEEK_SET);
	if (freeint1 != 0)
	{
		return ERROR_PSX_READ_FAILED;
	}
	freesizet1 = fwrite(&freeuint321, 1, 4, psx);
	if (freesizet1 != 4)
	{
		return ERROR_PSX_WRITE_FAILED;
	}
	
	return ERROR_NONE;
}

/*
 * Function that converts the animated textures
 * Parameters:
 * * pc = Pointer to PC file
 * * psx = Pointer to PSX file
 * * pc_info = Struct holding offsets and values from PC
 * * psx_info = Struct holding offsets and values from PSX
 * * psxpath = Path to PSX-file
 * * roomTextures = Conversion table for room textures
 */
int convertAnimatedTextures(FILE *pc, FILE *psx, struct level_info *pc_info,
                            struct level_info *psx_info, char *psxpath,
                            INTU16 *roomTextures)
{
	/* Macro Decalartions */
	#define USAGE_NORMAL         0x0001
	#define USAGE_WATER          0x0002
	#define USAGE_MIRRORED       0x0004
	#define USAGE_WATER_MIRRORED 0x0008
	
	/* Variable Declarations */
	INTU32 normalPos;      /* Position in the buffer for normal texutres */
	INTU32 specialPos;     /* Position in the buffer for special textures */
	INTU16 numIDs;         /* Number of IDs in this entry */
	INTU32 entryStart;     /* Offset where this entry started */
	INTU16 numEntries;     /* Original number of entries */
	INTU16 curRoomTex;     /* Current room texture */
	INTU16 *buffer = NULL; /* Buffer for converting animated textures */
	INTU16 usage;          /* How the animated texture is used */
	int    freeint1;       /* Integer for general use */
	size_t freesizet1;     /* Size_t for general use */
	INTU32 freeuint321,
	       freeuint322;    /* Unsigned 32-bit integers for general use */
	int    errorNumber = ERROR_NONE;

	/* Allocates memory */
	freeuint321 = (pc_info->v_NumAnimatedTextures * 0x00000004);
	buffer = calloc((size_t) freeuint321, (size_t) 2);
	if (buffer == NULL)
	{
		errorNumber = ERROR_MEMORY;
		goto end;
	}
	
	/* Reads original animated textures into memory */
	freeint1 = fseek(pc, ((long int) (pc_info->p_NumAnimatedTextures) + 4l),
	                 SEEK_SET);
	if (freeint1 != 0)
	{
		errorNumber = ERROR_PC_READ_FAILED;
		goto end;
	}
	freesizet1 = fread(buffer, (size_t) 2,
	                   (size_t) pc_info->v_NumAnimatedTextures, pc);
	if (freesizet1 != (size_t) pc_info->v_NumAnimatedTextures)
	{
		errorNumber = ERROR_PC_READ_FAILED;
		goto end;
	}
	
#ifdef __BIG_ENDIAN__
	for (normalPos = 0x0000; normalPos <
	     (INTU16) pc_info->v_NumAnimatedTextures; ++normalPos)
	{
		buffer[normalPos] = reverse16(buffer[normalPos]);
	}
#endif
	
	/* Converts the animated textures */
	normalPos = 0x00000001;
	specialPos = pc_info->v_NumAnimatedTextures;
	numEntries = buffer[0];
	buffer[0] = (INTU16) 0x0000;
	/* Converts per entry */
	for (; numEntries > 0x0000; --numEntries)
	{
		entryStart = normalPos;
		numIDs = buffer[normalPos++];
		++numIDs;
		usage = 0x0000;
		
		/* Checks how this animated texture is used */
		if (numIDs > 0x0000)
		{
			/* Looks for normal usage */
			curRoomTex = (INTU16) (buffer[normalPos] << 2U);
			if (roomTextures[curRoomTex] != 0xFFFF)
			{
				usage |= USAGE_NORMAL;
			}
			++curRoomTex;
			if (roomTextures[curRoomTex] != 0xFFFF)
			{
				usage |= USAGE_MIRRORED;
			}
			++curRoomTex;
			if (roomTextures[curRoomTex] != 0xFFFF)
			{
				usage |= USAGE_WATER;
			}
			++curRoomTex;
			if (roomTextures[curRoomTex] != 0xFFFF)
			{
				usage |= USAGE_WATER_MIRRORED;
			}
		}
		
		/* Makes the mirrored animated Texture */
		if ((usage & USAGE_MIRRORED) != 0x0000)
		{
			++buffer[0];
			normalPos = entryStart;
			numIDs = buffer[normalPos++];
			buffer[specialPos++] = numIDs;
			/* Converts the IDs within this entry */
			for (++numIDs; numIDs > 0x0000; --numIDs)
			{
				/* Looks for the correct texture */
				curRoomTex = (INTU16) ((buffer[normalPos] << 2U) + 0x0001);
				buffer[specialPos] = roomTextures[curRoomTex];
				if (buffer[specialPos] == 0xFFFF)
				{
					/* Prints a warning */
					PRINT_WARNING;
					printf("Animated texture ID " PRINTU16 " not found\n",
					       buffer[normalPos]);
					buffer[specialPos] = 0x0000;
				}
				++normalPos;
				++specialPos;
			}
		}
		
		/* Makes the underwater animated Texture */
		if ((usage & USAGE_WATER) != 0x0000)
		{
			++buffer[0];
			normalPos = entryStart;
			numIDs = buffer[normalPos++];
			buffer[specialPos++] = numIDs;
			/* Converts the IDs within this entry */
			for (++numIDs; numIDs > 0x0000; --numIDs)
			{
				/* Looks for the correct texture */
				curRoomTex = (INTU16) ((buffer[normalPos] << 2U) + 0x0002);
				buffer[specialPos] = roomTextures[curRoomTex];
				if (buffer[specialPos] == 0xFFFF)
				{
					/* Prints a warning */
					PRINT_WARNING;
					printf("Animated texture ID " PRINTU16 " not found\n",
					       buffer[normalPos]);
					buffer[specialPos] = 0x0000;
				}
				++normalPos;
				++specialPos;
			}
		}
		
		/* Makes the mirrored underwater animated Texture */
		if ((usage & USAGE_WATER_MIRRORED) != 0x0000)
		{
			++buffer[0];
			normalPos = entryStart;
			numIDs = buffer[normalPos++];
			buffer[specialPos++] = numIDs;
			/* Converts the IDs within this entry */
			for (++numIDs; numIDs > 0x0000; --numIDs)
			{
				/* Looks for the correct texture */
				curRoomTex = (INTU16) ((buffer[normalPos] << 2U) + 0x0003);
				buffer[specialPos] = roomTextures[curRoomTex];
				if (buffer[specialPos] == 0xFFFF)
				{
					/* Prints a warning */
					PRINT_WARNING;
					printf("Animated texture ID " PRINTU16 " not found\n",
					       buffer[normalPos]);
					buffer[specialPos] = 0x0000;
				}
				++normalPos;
				++specialPos;
			}
		}
		
		/* Makes the normal animated Texture */
		if ((usage & USAGE_NORMAL) != 0x0000)
		{
			++buffer[0];
			normalPos = entryStart;
			numIDs = buffer[normalPos++];
			/* Converts the IDs within this entry */
			for (++numIDs; numIDs > 0x0000; --numIDs)
			{
				/* Looks for the correct texture */
				curRoomTex = (INTU16) (buffer[normalPos] << 2U);
				buffer[normalPos] = roomTextures[curRoomTex];
				if (buffer[normalPos] == 0xFFFF)
				{
					/* Prints a warning */
					PRINT_WARNING;
					printf("Animated texture ID " PRINTU16 " not found\n",
					       buffer[normalPos]);
					buffer[normalPos] = 0x0000;
				}
				++normalPos;
			}
		}
		
		/* Prints an error if this was never used */
		if ((usage & USAGE_NORMAL) == 0x0000)
		{
			/* Calculates where this entry ends */
			freeuint321 = (INTU32) buffer[entryStart];
			freeuint321 += 0x00000002;
			freeuint321 += entryStart;
			/* Moves everything beyond it forwards */
			freeuint322 = (pc_info->v_NumAnimatedTextures * 0x00000004);
			freeuint322 -= freeuint321;
			freeuint322 *= 0x00000002;
			memmove(&buffer[entryStart], &buffer[freeuint321],
			        (size_t) freeuint322);
			/* Adjusts needed indices */
			freeuint321 -= entryStart;
			specialPos -= freeuint321;
			normalPos = entryStart;
		}
	}
	
#ifdef __BIG_ENDIAN__
	for (normalPos = 0x0000;
	     normalPos < (INTU16) (pc_info->v_NumAnimatedTextures * 4);
	     ++normalPos)
	{
		buffer[normalPos] = reverse16(buffer[normalPos]);
	}
#endif
	
	/* Resizes the space for animated textures */
	freeuint321 = specialPos;
	if (freeuint321 > psx_info->v_NumAnimatedTextures)
	{
		/* Adds needed space */
		freeuint321 -= psx_info->v_NumAnimatedTextures;
		freeuint321 *= 0x00000002;
		errorNumber = insertbytes(psx, psx_info->p_NumAnimatedTextures,
		                          freeuint321);
		if (errorNumber != ERROR_NONE)
		{
			goto end;
		}
		level_info_add(psx_info, psx_info->p_NumAnimatedTextures, freeuint321);
	}
	else if (freeuint321 < psx_info->v_NumAnimatedTextures)
	{
		/* Removes unneeded space */
		freeuint321 = (psx_info->v_NumAnimatedTextures - freeuint321);
		freeuint321 *= 0x00000002;
		errorNumber = removebytes(psx, psxpath,
		                          psx_info->p_NumAnimatedTextures,
		                          freeuint321);
		if (errorNumber != ERROR_NONE)
		{
			goto end;
		}
		level_info_sub(psx_info, psx_info->p_NumAnimatedTextures, freeuint321);
	}
	
	/* Writes NumAnimatedTextures to PSX */
	freeuint321 = specialPos;
#ifdef __BIG_ENDIAN__
	freeuint321 = reverse32(freeuint321);
#endif
	freeint1 = fseek(psx, (long int) psx_info->p_NumAnimatedTextures, SEEK_SET);
	if (freeint1 != 0)
	{
		errorNumber = ERROR_PSX_READ_FAILED;
		goto end;
	}
	freesizet1 = fwrite(&freeuint321, (size_t) 1, (size_t) 4, psx);
	if (freesizet1 != (size_t) 4)
	{
		errorNumber = ERROR_PSX_WRITE_FAILED;
		goto end;
	}
	psx_info->v_NumAnimatedTextures = specialPos;
	
	/* Writes the animated textures to PSX */
	freeuint321 = (specialPos * 0x00000002);
	freesizet1 = fwrite(buffer, (size_t) 1, (size_t) freeuint321, psx);
	if (freesizet1 != (size_t) freeuint321)
	{
		errorNumber = ERROR_PSX_WRITE_FAILED;
		goto end;
	}
	
end:
	if (buffer != NULL)
	{
		free(buffer);
	}
	return errorNumber;
}

/*
 * Function that converts a colour to its underwater equivalent
 * Parameters:
 * * colour = Input colour
 * * waterColour = Rules for calculating the underwater colour
 */
static INTU16 underwaterColour(INTU16 colour, struct water_colour *waterColour)
{
	/* Variable Declarations */
	INT16 currentRed,
	      currentGreen,
	      currentBlue;  /* Current colour values */
	INT16 newRed,
	      newGreen,
	      newBlue;      /* New colour values */
	INT16 operandRed,
	      operandGreen,
	      operandBlue;  /* Operand Colours */
	int freeint1;       /* Integer for general use */
	
	/* Doesn't convert the colour if it's transparent */
	if (colour == 0x0000)
	{
		return 0x0000;
	}
	
	/* Separates the colour into its individual colour channels */
	currentRed = (INT16) (colour & 0x001F);
	currentGreen = (INT16) (((INTU16) (colour & 0x03E0)) >> 5);
	currentBlue = (INT16) (((INTU16) (colour & 0x7C00)) >> 10);
	newRed = currentRed;
	newGreen = currentGreen;
	newBlue = currentBlue;
	
	/* Recalculates red */
	if ((waterColour->instruction & 0x000F) != 0)
	{
		/* Determines operand */
		operandRed = (INT16) (waterColour->red_operand & 0x001F);
		if ((waterColour->red_operand & 0x3000) == 0x1000)
		{
			operandRed = currentRed;
		}
		else if ((waterColour->red_operand & 0x3000) == 0x2000)
		{
			operandRed = currentGreen;
		}
		else if ((waterColour->red_operand & 0x3000) == 0x3000)
		{
			operandRed = currentBlue;
		}
		
		/* Performs operation */
		freeint1 = (int) (waterColour->instruction & 0x000F);
		switch (freeint1)
		{
			case 1:
				/* Performs addition */
				newRed = (currentRed + operandRed);
				
				/* Overflow protection */
				if (((waterColour->instruction & 0x1000) == 0x0000) &&
				    (newRed > 0x001F))
				{
					newRed = 0x001F;
				}
				break;
			case 2:
				/* Performs subtraction */
				newRed = (currentRed - operandRed);
				
				/* Underflow protection */
				if (((waterColour->instruction & 0x1000) == 0x0000) &&
				    (newRed < 0x0000))
				{
					newRed = 0x0000;
				}
				break;
			case 3:
				/* Performs multiplication */
				newRed = (currentRed * operandRed);
				
				/* Overflow protection */
				if (((waterColour->instruction & 0x1000) == 0x0000) &&
				    (newRed > 0x001F))
				{
					newRed = 0x001F;
				}
				break;
			case 4:
				/* Performs the division (avoiding dividing by 0) */
				newRed = (INT16) ((operandRed == 0x0000) ?
				                  0 : (currentRed / operandRed));
				break;
			case 5:
				/* Performs the AND operation */
				newRed = (currentRed & operandRed);
				break;
			case 6:
				/* Performs the OR operation */
				newRed = (currentRed | operandRed);
				break;
			case 7:
				/* Performs the XOR operation */
				newRed = (currentRed ^ operandRed);
				break;
			case 8:
				/* Performs the one's complement operation */
				newRed = ~operandRed;
				break;
			case 9:
				/* Performs the left shift */
				newRed = (INT16) ((INTU16) currentRed << (INTU16) operandRed);
				
				/* Overflow protection */
				if (((waterColour->instruction & 0x1000) == 0x0000) &&
				    ((newRed > 0x001F) || (newRed < 0)))
				{
					newRed = 0x001F;
				}
				break;
			case 10:
				/* Performs the right shift */
				newRed = (INT16) ((INTU16) currentRed >> (INTU16) operandRed);
				break;
			case 11:
				/* Performs the modulo (avoiding dividing by 0) */
				newRed = (INT16) (currentRed %
				                  ((operandRed == 0) ? currentRed :
				                   operandRed));
				break;
			case 12:
				/* Assigns value */
				newRed = operandRed;
				break;
			case 13:
				/* Scales the value */
				newRed = (INT16) (((INTU16) currentRed *
				                   (INTU16) operandRed) /
				                  (INTU16) 0x0021);
				break;
		}
	}
	
	/* Recalculates green */
	if ((waterColour->instruction & 0x00F0) != 0)
	{
		/* Determines operand */
		operandGreen = (INT16) (waterColour->green_operand & 0x001F);
		if ((waterColour->green_operand & 0x3000) == 0x1000)
		{
			operandGreen = currentRed;
		}
		else if ((waterColour->green_operand & 0x3000) == 0x2000)
		{
			operandGreen = currentGreen;
		}
		else if ((waterColour->green_operand & 0x3000) == 0x3000)
		{
			operandGreen = currentBlue;
		}
		
		/* Performs operation */
		freeint1 = (int) (((INTU16) (waterColour->instruction & 0x00F0)) >> 4);
		switch (freeint1)
		{
			case 1:
				/* Performs addition */
				newGreen = (currentGreen + operandGreen);
				
				/* Overflow protection */
				if (((waterColour->instruction & 0x1000) == 0x0000) &&
				    (newGreen > 0x001F))
				{
					newGreen = 0x001F;
				}
				break;
			case 2:
				/* Performs subtraction */
				newGreen = (currentGreen - operandGreen);
				
				/* Underflow protection */
				if (((waterColour->instruction & 0x1000) == 0x0000) &&
				    (newGreen < 0x0000))
				{
					newGreen = 0x0000;
				}
				break;
			case 3:
				/* Performs multiplication */
				newGreen = (currentGreen * operandGreen);
				
				/* Overflow protection */
				if (((waterColour->instruction & 0x1000) == 0x0000) &&
				    (newGreen > 0x001F))
				{
					newGreen = 0x001F;
				}
				break;
			case 4:
				/* Performs the division (avoiding dividing by 0) */
				newGreen = (INT16) ((operandGreen == 0x0000) ?
				                    0 : (currentGreen / operandGreen));
				break;
			case 5:
				/* Performs the AND operation */
				newGreen = (currentGreen & operandGreen);
				break;
			case 6:
				/* Performs the OR operation */
				newGreen = (currentGreen | operandGreen);
				break;
			case 7:
				/* Performs the XOR operation */
				newGreen = (currentGreen ^ operandGreen);
				break;
			case 8:
				/* Performs the one's complement operation */
				newGreen = ~operandGreen;
				break;
			case 9:
				/* Performs the left shift */
				newGreen = (INT16) ((INTU16) currentGreen <<
				                    (INTU16) operandGreen);
				
				/* Overflow protection */
				if (((waterColour->instruction & 0x1000) == 0x0000) &&
				    ((newGreen > 0x001F) || (newGreen < 0)))
				{
					newGreen = 0x001F;
				}
				break;
			case 10:
				/* Performs the right shift */
				newGreen = (INT16) ((INTU16) currentGreen >>
				                    (INTU16) operandGreen);
				break;
			case 11:
				/* Performs the modulo (avoiding dividing by 0) */
				newGreen = (INT16) (currentGreen %
				                    ((operandGreen == 0) ?
				                     currentGreen : operandGreen));
				break;
			case 12:
				/* Assigns value */
				newGreen = operandGreen;
				break;
			case 13:
				/* Scales the value */
				newGreen = (INT16) (((INTU16) currentGreen *
				                     (INTU16) operandGreen) /
				                    (INTU16) 0x0021);
				break;
		}
	}
	
	/* Recalculates blue */
	if ((waterColour->instruction & 0x0F00) != 0)
	{
		/* Determines operand */
		operandBlue = (INT16) (waterColour->blue_operand & 0x001F);
		if ((waterColour->blue_operand & 0x3000) == 0x1000)
		{
			operandBlue = currentRed;
		}
		else if ((waterColour->blue_operand & 0x3000) == 0x2000)
		{
			operandBlue = currentGreen;
		}
		else if ((waterColour->blue_operand & 0x3000) == 0x3000)
		{
			operandBlue = currentBlue;
		}
		
		/* Performs operation */
		freeint1 = (int) (((INTU16)
		                   (waterColour->instruction & 0x0F00)) >> 8);
		switch (freeint1)
		{
			case 1:
				/* Performs addition */
				newBlue = (currentBlue + operandBlue);
				
				/* Overflow protection */
				if (((waterColour->instruction & 0x1000) == 0x0000) &&
				    (newBlue > 0x001F))
				{
					newBlue = 0x001F;
				}
				break;
			case 2:
				/* Performs subtraction */
				newBlue = (currentBlue - operandBlue);
				
				/* Underflow protection */
				if (((waterColour->instruction & 0x1000) == 0x0000) &&
				    (newBlue < 0x0000))
				{
					newBlue = 0x0000;
				}
				break;
			case 3:
				/* Performs multiplication */
				newBlue = (currentBlue * operandBlue);
				
				/* Overflow protection */
				if (((waterColour->instruction & 0x1000) == 0x0000) &&
				    (newBlue > 0x001F))
				{
					newBlue = 0x001F;
				}
				break;
			case 4:
				/* Performs the division (avoiding dividing by 0) */
				newBlue = (INT16) ((operandBlue == 0x0000) ?
				                   0 : (currentBlue / operandBlue));
				break;
			case 5:
				/* Performs the AND operation */
				newBlue = (currentBlue & operandBlue);
				break;
			case 6:
				/* Performs the OR operation */
				newBlue = (currentBlue | operandBlue);
				break;
			case 7:
				/* Performs the XOR operation */
				newBlue = (currentBlue ^ operandBlue);
				break;
			case 8:
				/* Performs the one's complement operation */
				newBlue = ~operandBlue;
				break;
			case 9:
				/* Performs the left shift */
				newBlue = (INT16) ((INTU16) currentBlue <<
				                   (INTU16) operandBlue);
				
				/* Overflow protection */
				if (((waterColour->instruction & 0x1000) == 0x0000) &&
				    ((newBlue > 0x001F) || (newBlue < 0)))
				{
					newBlue = 0x001F;
				}
				break;
			case 10:
				/* Performs the right shift */
				newBlue = (INT16) ((INTU16) currentBlue >>
				                   (INTU16) operandBlue);
				break;
			case 11:
				/* Performs the modulo (avoiding dividing by 0) */
				newBlue = (INT16) (currentBlue %
				                   ((operandBlue == 0) ? currentBlue :
				                    operandBlue));
				break;
			case 12:
				/* Assigns value */
				newBlue = operandBlue;
				break;
			case 13:
				/* Scales the value */
				newBlue = (INT16) (((INTU16) currentBlue *
				                    (INTU16) operandBlue) /
				                   (INTU16) 0x0021);
				break;
		}
	}
	
	/* Puts channels back together */
	colour = (INTU16) (0x8000 | (newRed & 0x001F) |
	                   (((INTU16) (newGreen & 0x001F)) << 5) |
	                   (((INTU16) (newBlue & 0x001F)) << 10));
	return colour;
}
