/*
 * Library that performs colour reduction for TR3PCPSX.
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

/* File inclusions */
#include <stdio.h>    /* Standard I/O */
#include <string.h>   /* Used for memset */
#include "fixedint.h" /* Definition of fixed-size integers */
#include "colour.h"   /* Header for colour reduction */

/* Function Prototypes */
static INTU16 distance8(INTU8 r1, INTU8 g1, INTU8 b1,
                        INTU8 r2, INTU8 g2, INTU8 b2);
static INTU8 fullestBucket(struct colour *colours, INTU16 numColours);
static void splitBucket(struct colour *colours, INTU16 numColours,
                        INTU8 inBucket, INTU8 outBucket1, INTU8 outBucket2);
static void medianColour(struct colour *colours, INTU16 numColours,
                         INTU8 bucket, INTU8 *outr, INTU8 *outg, INTU8 *outb);
static void fifteenColours(struct colour *colours, INTU16 numColours,
                           INTU8 *paletteR, INTU8 *paletteG, INTU8 *paletteB);

/*
 * Function that calculates the distance between two colours. (Mapping colours
 * on an RGB-cube, and determining the difference using the Pythagorean theorem)
 * Parameters:
 * * r1 = First colour's red value
 * * g1 = First colour's green value
 * * b1 = First colour's blue value
 * * r2 = Second colour's red value
 * * g2 = Second colour's green value
 * * b2 = Second colour's blue value
 * Returns the distance.
 */
static INTU16 distance8(INTU8 r1, INTU8 g1, INTU8 b1,
                        INTU8 r2, INTU8 g2, INTU8 b2)
{
	/* Variable Declarations */
	INTU16 diffr, diffg,
	       diffb;            /* Differences on each colour */
	INTU32 totaldiffsquared; /* Total of squared differences on every colour */
	INTU32 res, bit, tmp;    /* Variables used in square root calculation */
	
	/* Determines the differences on every colour */
	if (r1 > r2)
	{
		diffr = (INTU16) (r1 - r2);
	}
	else
	{
		diffr = (INTU16) (r2 - r1);
	}
	if (g1 > g2)
	{
		diffg = (INTU16) (g1 - g2);
	}
	else
	{
		diffg = (INTU16) (g2 - g1);
	}
	if (b1 > b2)
	{
		diffb = (INTU16) (b1 - b2);
	}
	else
	{
		diffb = (INTU16) (b2 - b1);
	}
	
	/* Squares the differences */
	diffr *= diffr;
	diffg *= diffg;
	diffb *= diffb;
	
	/* Adds the differences together */
	totaldiffsquared = (INTU32) diffr;
	totaldiffsquared += diffg;
	totaldiffsquared += diffb;
	
	/* Calculates the square root of this number */
	res = 0x00000000;
	bit = 0x40000000;
	while (bit > totaldiffsquared)
	{
		bit >>= 2;
	}
	while (bit != 0x00000000)
	{
		tmp = (res + bit);
		if (totaldiffsquared >= tmp)
		{
			totaldiffsquared -= tmp;
			res >>= 1;
			res += bit;
		}
		else
		{
			res >>= 1;
		}
		bit >>= 2;
	}
	
	/* Returns the resulting distance */
	diffr = (INTU16) res;
	return diffr;
}

/*
 * Function that finds which bucket is the fullest.
 * Parameters:
 * * colours = Array of colour structs
 * * numColours = Number of colour structs
 * Returns the bucket number (0 - 15)
 */
static INTU8 fullestBucket(struct colour *colours, INTU16 numColours)
{
	/* Variable Declarations */
	INTU32 inBucket[16];  /* Number of colours in each bucket */
	INTU16 curColour;     /* Current colour in processing */
	INTU8  curBucket;     /* Current bucket in processing */
	INTU32 fullestCount;  /* Number of colours in the fullest bucket */
	INTU8  fullestBucket; /* Number of the fullest bucket */
	
	/* Sets the counters to 0 */
	(void) memset(inBucket, 0, 64);
	
	/* Counts how many colours are in each bucket */
	for (curColour = 0x0000; curColour < numColours; ++curColour)
	{
		inBucket[colours[curColour].bucket]++;
	}
	
	/* Determines which bucket is the fullest */
	fullestCount = 0x00000000;
	fullestBucket = (INTU8) 0x00;
	for (curBucket = (INTU8) 0x00; curBucket < (INTU8) 0x10; ++curBucket)
	{
		if (inBucket[curBucket] > fullestCount)
		{
			fullestCount = inBucket[curBucket];
			fullestBucket = curBucket;
		}
	}
	
	return fullestBucket;
}

/*
 * Function that splits one bucket into two buckets.
 * Parameters:
 * * colours = Array of colour structs
 * * numColours = Number of colour structs
 * * inBucket = The number of the bucket to split
 * * outBucket1 = The first new bucket to create
 * * outBucket2 = The second new bucket to create
 */
static void splitBucket(struct colour *colours, INTU16 numColours,
                        INTU8 inBucket, INTU8 outBucket1, INTU8 outBucket2)
{
	/* Variable Declarations */
	INTU8 minr, maxr;   /* Lowest and highest found values for red */
	INTU8 ming, maxg;   /* Lowest and highest found values for green */
	INTU8 minb, maxb;   /* Lowest and highest found values for blue */
	INTU16 curColour;   /* Current colour in processing */
	INTU16 numInBucket; /* Number of colours in the current bucket */
	INTU32 pivotTotal;  /* Divided by pivotCount to produce pivot */
	INTU32 pivotCount;  /* Divides pivotTotal to produce pivot */
	INTU8 pivot;        /* Actual pivot colour value */
	int pivotColour;    /* Colour of the pivot (0 = Red, 1 = Green, 2 = Blue) */
	
	/* Sets up initial min- and max values to find colour ranges */
	minr = ming = minb = (INTU8) 0xFF;
	maxr = maxg = maxb = (INTU8) 0x00;
	
	/* Sets up to count the number of colours in the current bucket */
	numInBucket = 0x0000;
	
	/* Finds the colour channel with the biggest range */
	for (curColour = 0x0000; curColour < numColours; curColour++)
	{
		/* Only look at colours in the current bucket */
		if (colours[curColour].bucket == inBucket)
		{
			if (colours[curColour].r < minr)
			{
				minr = colours[curColour].r;
			}
			if (colours[curColour].g < ming)
			{
				ming = colours[curColour].g;
			}
			if (colours[curColour].b < minb)
			{
				minb = colours[curColour].b;
			}
			if (colours[curColour].r > maxr)
			{
				maxr = colours[curColour].r;
			}
			if (colours[curColour].g > maxg)
			{
				maxg = colours[curColour].g;
			}
			if (colours[curColour].b > maxb)
			{
				maxb = colours[curColour].b;
			}
			++numInBucket;
		}
	}
	
	/* Splits the bucket if there is more than one colour */
	if (numInBucket > 0x0001)
	{
		maxr -= minr;
		maxg -= ming;
		maxb -= minb;
		if (maxr > maxg)
		{
			if (maxr > maxb)
			{
				pivotColour = 0;
			}
			else
			{
				pivotColour = 2;
			}
		}
		else
		{
			if (maxg > maxb)
			{
				pivotColour = 1;
			}
			else
			{
				pivotColour = 2;
			}
		}
		
		/* Determines the pivot */
		pivotTotal = 0x00000000;
		pivotCount = 0x00000000;
		for (curColour = 0x0000; curColour < numColours; curColour++)
		{
			/* Only look at colours in the current bucket */
			if (colours[curColour].bucket == inBucket)
			{
				pivotCount += (INTU32) colours[curColour].weight;
				switch (pivotColour)
				{
					case 0:
						pivotTotal += (((INTU32) colours[curColour].weight) *
						               ((INTU32) colours[curColour].r));
						break;
					case 1:
						pivotTotal += (((INTU32) colours[curColour].weight) *
						               ((INTU32) colours[curColour].g));
						break;
					case 2:
						pivotTotal += (((INTU32) colours[curColour].weight) *
						               ((INTU32) colours[curColour].b));
						break;
				}
			}
		}
		pivotTotal /= pivotCount;
		pivot = (INTU8) pivotTotal;
		
		/* Divides the colours between the new buckets */
		for (curColour = 0x0000; curColour < numColours; curColour++)
		{
			/* Only look at colours in the current bucket */
			if (colours[curColour].bucket == inBucket)
			{
				switch (pivotColour)
				{
					case 0:
						if (colours[curColour].r > pivot)
						{
							colours[curColour].bucket = outBucket1;
						}
						else
						{
							colours[curColour].bucket = outBucket2;
						}
						break;
					case 1:
						if (colours[curColour].g > pivot)
						{
							colours[curColour].bucket = outBucket1;
						}
						else
						{
							colours[curColour].bucket = outBucket2;
						}
						break;
					case 2:
						if (colours[curColour].b > pivot)
						{
							colours[curColour].bucket = outBucket1;
						}
						else
						{
							colours[curColour].bucket = outBucket2;
						}
						break;
				}
			}
		}
	}
	else
	{
		/*
		 * This occurs in the rare instance that a bucket only contains
		 * one colour. It will attempt some primitive redistribution of
		 * colours to end up with as many buckets as possible.
		 */
		/* Moves the single colour in this bucket to its left child */
		for (curColour = 0x0000; curColour < numColours; curColour++)
		{
			if (colours[curColour].bucket == inBucket)
			{
				colours[curColour].bucket = outBucket1;
				break;
			}
		}
		
		/* Splits the fullest bucket into itself and the right child */
		pivot = fullestBucket(colours, numColours);
		splitBucket(colours, numColours, pivot, pivot, outBucket2);
	}
}

/*
 * Function that calculates the median colour for a bucket
 * Parameters:
 * * colours = Array of colour structs
 * * numColours = Number of colour structs
 * * bucket = The number of the bucket to median
 * * outr = Where to put the median red value
 * * outg = Where to put the median green value
 * * outb = Where to put the median blue value
 */
static void medianColour(struct colour *colours, INTU16 numColours,
                         INTU8 bucket, INTU8 *outr, INTU8 *outg, INTU8 *outb)
{
	/* Variable Declarations */
	INTU32 totalR;     /* Divides by totalCount for median red value */
	INTU32 totalG;     /* Divides by totalCount for median green value */
	INTU32 totalB;     /* Divides by totalCount for median blue value */
	INTU32 totalCount; /* Divide totalsby this for median colour */
	INTU16 curColour;  /* Current colour in processing */
	
	/* Adds the total colour values */
	totalR = totalG = totalB = totalCount = 0x00000000;
	for (curColour = 0x0000; curColour < numColours; curColour++)
	{
		/* Only look at colours in the current bucket */
		if (colours[curColour].bucket == bucket)
		{
			totalCount += (INTU32) colours[curColour].weight;
			totalR += (((INTU32) colours[curColour].weight) *
			           ((INTU32) colours[curColour].r));
			totalG += (((INTU32) colours[curColour].weight) *
			           ((INTU32) colours[curColour].g));
			totalB += (((INTU32) colours[curColour].weight) *
			           ((INTU32) colours[curColour].b));
		}
	}
	
	/* Only make a median colour if there are any colours */
	if (totalCount > 0x00000000)
	{
		/* Determines the median colour */
		totalR /= totalCount;
		totalG /= totalCount;
		totalB /= totalCount;
		
		/* Sends the median colour */
		*outr = (INTU8) totalR;
		*outg = (INTU8) totalG;
		*outb = (INTU8) totalB;
	}
}

/*
 * Reduces the palette from 16 to 15 colours (needed for transparency)
 * Parameters:
 * * colours = Array of colour structs
 * * numColours = Number of colour structs
 * * paletteR = Array of red value of palette
 * * paletteG = Array of red value of palette
 * * paletteB = Array of red value of palette
 */
static void fifteenColours(struct colour *colours, INTU16 numColours,
                           INTU8 *paletteR, INTU8 *paletteG, INTU8 *paletteB)
{
	/* Variable Declarations */
	INTU8 curcolour1;  /* Current first colour in comparison */
	INTU8 curcolour2;  /* Current second colour in comparison */
	INTU8 bestcolour1; /* Closest first colour in comparison */
	INTU8 bestcolour2; /* Closest second colour in comparison */
	INTU16 curdist;    /* Distance between the current colours */
	INTU16 bestdist;   /* Distance betweem the closest colours */
	INTU16 curColour;  /* Current colour in processing */
	
	/* Finds which colours are the closest to each other */
	bestdist = 0xFFFF;
	bestcolour1 = bestcolour2 = (INTU8) 0x00;
	for (curcolour1 = (INTU8) 0x00; curcolour1 < (INTU8) 0x10; ++curcolour1)
	{
		for (curcolour2 = (curcolour1 + (INTU8) 0x01);
		     curcolour2 < (INTU8) 0x10; ++curcolour2)
		{
			curdist = distance8(paletteR[curcolour1], paletteG[curcolour1],
			                    paletteB[curcolour1], paletteR[curcolour2],
			                    paletteG[curcolour2], paletteB[curcolour2]);
			if (curdist < bestdist)
			{
				bestdist = curdist;
				bestcolour1 = curcolour1;
				bestcolour2 = curcolour2;
			}
		}
	}
	
	/* Merges the closest two */
	for (curColour = 0x0000; curColour < numColours; ++curColour)
	{
		if (colours[curColour].bucket == bestcolour1)
		{
			colours[curColour].bucket = bestcolour2;
		}
		else if (colours[curColour].bucket == (INTU8) 0x00)
		{
			colours[curColour].bucket = bestcolour1;
		}
	}
	paletteR[bestcolour1] = paletteR[0];
	paletteG[bestcolour1] = paletteG[0];
	paletteB[bestcolour1] = paletteB[0];
	medianColour(colours, numColours, bestcolour2, &paletteR[bestcolour2],
	             &paletteG[bestcolour2], &paletteB[bestcolour2]);
}

/*
 * Reduces the number of colours in an image to either 16 or 15 colours.
 * Parameters:
 * * colours = Array of colour structs
 * * numColours = Number of colour structs
 * * outColours = Number of output colours
 * * palette = Where to store the output palette
 */
void reduceColours(struct colour *colours, INTU16 numColours,
                   INTU8 outColours, INTU16 *palette)
{
	/* Variable Declarations */
	INTU8 curbucket;    /* Current bucket in processing */
	INTU8 leftchild;    /* Left child for splitting buckets */
	INTU8 rightchild;   /* Right child for splitting buckets */
	INTU8 paletteR[16]; /* Output palette red values */
	INTU8 paletteG[16]; /* Output palette green values */
	INTU8 paletteB[16]; /* Output palette blue values */
	INTU16 curColour;   /* Current colour inprocessing */
	
	/* Only reduce the colours if needed */
	if (numColours > (INTU16) outColours)
	{
		/* Places every colour into the same bucket */
		for (curColour = 0x0000; curColour < numColours; ++curColour)
		{
			colours[curColour].bucket = (INTU8) 0x01;
		}
		
		/* Divides the colours into sixteen buckets */
		for (curbucket = (INTU8) 0x01; curbucket < (INTU8) 0x10; ++curbucket)
		{
			leftchild = (INTU8) (curbucket << 1U);
			leftchild &= (INTU8) 0x0F;
			rightchild = (leftchild + (INTU8) 0x01);
			splitBucket(colours, numColours, curbucket, leftchild, rightchild);
		}
		
		/* Calculates the colour palette */
		for (curbucket = (INTU8) 0x00; curbucket < (INTU8) 0x10; ++curbucket)
		{
			medianColour(colours, numColours, curbucket, &paletteR[curbucket],
			             &paletteG[curbucket], &paletteB[curbucket]);
		}
		
		/* Removes one colour in case of transparency being needed */
		if (outColours == (INTU8) 0x0F)
		{
			fifteenColours(colours, numColours, paletteR, paletteG, paletteB);
		}
	}
	else
	{
		/* Simply assigns numbers to the colours */
		for (curColour = 0x0000; curColour < numColours; ++curColour)
		{
			colours[curColour].bucket = (INTU8) curColour;
			if (outColours == (INTU8) 0x0F)
			{
				(colours[curColour].bucket)++;
			}
		}
		
		/* Calculates the colour palette */
		for (curbucket = (INTU8) 0x00; curbucket < (INTU8) 0x10; ++curbucket)
		{
			medianColour(colours, numColours, curbucket, &paletteR[curbucket],
			             &paletteG[curbucket], &paletteB[curbucket]);
		}
	}
	
	/* Builds the output palette */
	for (curbucket = (INTU8) 0x00; curbucket < (INTU8) 0x10; ++curbucket)
	{
		palette[curbucket] = colour2416(paletteR[curbucket],
		                                paletteG[curbucket],
		                                paletteB[curbucket]);
	}
	
	/* Sets transparency colour if needed */
	if (outColours == (INTU8) 0x0F)
	{
		palette[0] = 0x0000;
	}
}

/*
 * Function that returns the bucket a specific colour is in.
 * Parameters:
 * * colours = Array of colour structs
 * * numColours = Number of colour structs
 * * colour = The specific colour index to be looked for
 * Nota bene:
 * * The number of colours must be more than 0.
 * * The colour that is searched for must exist.
 */
INTU8 getBucket(struct colour *colours, INTU16 numColours, INTU16 colour)
{
	/* Variable Declarations */
	INTU16 floor;   /* Lowest possible place where the colour could be */
	INTU16 ceiling; /* Highest possible place where the colour could be */
	INTU16 curpos;  /* Current position in searching */
	
	/* Returns 0 if it is the transparency colour */
	if (colour == 0x8000)
	{
		return colours[0].bucket;
	}
	
	/* Sets floor, ceiling and curpos to default beginning position */
	floor = 0x0000;
	ceiling = (numColours - 1);
	curpos = (INTU16) (numColours >> 1);
	
	/* Skips the transparency colour if needed */
	if (colours[0].in_index == 0x8000)
	{
		++floor;
	}
	
	/* Searches for the colour */
	while (colour != colours[curpos].in_index)
	{
		if (colour > colours[curpos].in_index)
		{
			floor = (curpos + 1);
		}
		else
		{
			ceiling = (curpos - 1);
		}
		curpos = (INTU16) ((floor + ceiling) >> 1);
	}
	
	/* Returns the converted colour value */
	return colours[curpos].bucket;
}
