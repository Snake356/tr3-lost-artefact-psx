/*
 * Library that contains format conversions used by TRMOD
 * Copyright (C) 2021 b122251
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not,see <http://www.gnu.org/licenses/>.
 */

/* File Inclusions */
#include <stdio.h>    /* Standard I/O */
#include <stdlib.h>   /* Standard library of utility functions */
#include <string.h>   /* Functions relating to strings */
#include "fixedint.h" /* Definition of fixed-size integers */
#include "util.h"     /* Generic functions used by TRMOD */
#include "conv.h"     /* Format conversions */

/*
 * Function that converts light intensity/colour from string to integer
 * Parameters:
 * * string = String-version of the light
 * * type = What type of light to convert to (see below)
 * Returns the converted integer
 * Types of lights:
 * * 0 = Linear intensity (8191 - 0)
 * * 1 = 15-bit RGB
 */
INT16 convertLight(char *string, int type)
{
	/* Variable Declarations */
	INT16 output = 0x0000; /* The output integer */
	INTU16 slashindex;     /* Position of the slash if a fraction */
	INT32 numerator;       /* Numerator in the case of fraction input */
	INT32 denominator;     /* Denominator in the case of fraction input */
	INTU16 r, g, b;        /* Colour components */
	int intype;            /* Input type:
	                        * 0 = -1
	                        * 1 = Fraction
	                        * 2 = Colour (RRGGBB)
	                        * 3 = Literal number (not recommended but eh)
	                        */
	
	/* Checks whether the string exists at all */
	if (string == NULL)
	{
		return 0x0000;
	}
	
	/* Determines intype */
	if ((charindex(string, (INTU16) strlen(string), '/', 1) != 0xFFFF) ||
	    (charindex(string, (INTU16) strlen(string), '\\', 1) != 0xFFFF))
	{
		intype = 1;
	}
	else if ((compareString(string, "-1", 2) == 0) ||
	         (strlen(string) == (size_t) 0))
	{
		intype = 0;
	}
	else if ((strlen(string) == (size_t) 6) &&
	         (validHex(string, 0x0006) == 0))
	{
		intype = 2;
	}
	else
	{
		intype = 3;
	}
	
	/* Performs the needed conversion */
	if ((intype == 0) && (type == 0))
	{
		output = 0xFFFF;
	}
	else if ((intype == 1) && (type == 0))
	{
		/* Separates the numerator and denominator */
		slashindex = charindex(string, (INTU16) strlen(string), '/', 1);
		if (slashindex == 0xFFFF)
		{
			slashindex = charindex(string, (INTU16) strlen(string), '\\', 1);
		}
		string[slashindex] = '\0';
		++slashindex;
		numerator = (INT32) strtol(string, NULL, 10);
		denominator = (INT32) strtol(&string[slashindex], NULL, 10);
		if (denominator == 0x00000000)
		{
			denominator = 0x00000001;
		}
		
		/* Adjusts to the right scale */
		numerator *= 0x00001FFF;
		numerator /= denominator;
		numerator = (0x00001FFF - numerator);
		
		/* Sets it to the output */
		output = (INT16) numerator;
	}
	else if ((intype == 2) && (type == 0))
	{
		/* Separates the colour components */
		colourComponents(string, &r, &g, &b);
		
		/* Scales the components to the right scale */
		r <<= 5U;
		r |= (r >> 8U);
		g <<= 5U;
		g |= (g >> 8U);
		b <<= 5U;
		b |= (b >> 8U);
		
		/* Gets the average of the colour components */
		r += g;
		r += b;
		r /= 0x0003;
		
		/* Sets the scale to (8191 - 0) */
		r = (0x1FFF - r);
		output = (INT16) r;
	}
	else if ((intype == 3) && (type == 0))
	{
		output = (INT16) strtol(string, NULL, 10);
	}
	if ((intype == 0) && (type == 1))
	{
		output = 0xFFFF;
	}
	else if ((intype == 1) && (type == 1))
	{
		/* Separates the numerator and denominator */
		slashindex = charindex(string, (INTU16) strlen(string), '/', 1);
		if (slashindex == 0xFFFF)
		{
			slashindex = charindex(string, (INTU16) strlen(string), '\\', 1);
		}
		string[slashindex] = '\0';
		++slashindex;
		numerator = (INT32) strtol(string, NULL, 10);
		denominator = (INT32) strtol(&string[slashindex], NULL, 10);
		if (denominator == 0x00000000)
		{
			denominator = 0x00000001;
		}
		
		/* Adjusts to the right scale */
		numerator *= 0x0000001F;
		numerator /= denominator;
		numerator |= (numerator << 5U);
		numerator |= (numerator << 5U);
		
		/* Sets it to the output */
		output = (INT16) numerator;
	}
	else if ((intype == 2) && (type == 1))
	{
		/* Separates the colour components */
		colourComponents(string, &r, &g, &b);
		
		/* Scales the components to the right scale */
		r >>= 3U;
		g >>= 3U;
		b >>= 3U;
		
		/* Puts the channels together */
		output = (INTU16) (r & (INTU8) 0x1F);
		output <<= 5U;
		output |= (INTU16) (g & (INTU8) 0x1F);
		output <<= 5U;
		output |= (INTU16) (b & (INTU8) 0x1F);
	}
	else if ((intype == 3) && (type == 1))
	{
		output = (INT16) strtol(string, NULL, 10);
	}
	
	return output;
}

/*
 * Splits a colour string into its components.
 * Parameters:
 * * string = String version of the colour (RRGGBB)
 * * red = Where to store the red component
 * * green = Where to store the green component
 * * blue = Where to store the blue component
 */
void colourComponents(char *string, INTU16 *red, INTU16 *green, INTU16 *blue)
{
	/* Variable Declarations */
	char sred[3];
	char sgreen[3];
	char sblue[3];
	
	/* Separates colour components */
	sred[0] = string[0];
	sred[1] = string[1];
	sred[2] = '\0';
	sgreen[0] = string[2];
	sgreen[1] = string[3];
	sgreen[2] = '\0';
	sblue[0] = string[4];
	sblue[1] = string[5];
	sblue[2] = '\0';
	
	*red = (INTU16) strtol(sred, NULL, 16);
	*green = (INTU16) strtol(sgreen, NULL, 16);
	*blue = (INTU16) strtol(sblue, NULL, 16);
}

/*
 * Function that checks that a string is valid hex-characters (0-F)
 * Parameters:
 * * string = String to check
 * * length = How many characters to check
 * Return values: 0 = Valid, 1 = Invalid
 */
int validHex(char *string, INTU16 length)
{
	/* Variable Declarations */
	INTU16 stringlength; /* Detected string length */
	INTU16 curchar;      /* Current character in processing */
	
	/* Checks input */
	if (string == NULL)
	{
		return 1;
	}
	stringlength = (INTU16) strlen(string);
	if (stringlength < length)
	{
		return 1;
	}
	
	/* Checks the characters */
	for (curchar = 0x0000; curchar < length; ++curchar)
	{
		switch (string[curchar])
		{
			case '0':
			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
			case '6':
			case '7':
			case '8':
			case '9':
			case 'A':
			case 'a':
			case 'B':
			case 'b':
			case 'C':
			case 'c':
			case 'D':
			case 'd':
			case 'E':
			case 'e':
			case 'F':
			case 'f':
				break;
			default:
				return 1;
		}
	}
	
	/* The string is valid Hex */
	return 0;
}
