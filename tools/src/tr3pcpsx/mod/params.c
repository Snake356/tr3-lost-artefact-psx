/*
 * Library that handles command-line parameters in TR3PCPSX.
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
#include "params.h"   /* Handles command-line parameters */
#include "fixedint.h" /* Definition of fixed-size integers */
#include "errors.h"   /* Error Handling for TR3PCPSX */
#include "util.h"     /* General-purpose code used by TR3PCPSX */

/*
 * Interprets command-line parameters.
 * Parameters:
 * * argc = Number of command-line parameters
 * * argv = Command-line parameters
 * * pcparam = Pointer to the parameter number for the PC-file
 * * psxparam = Pointer to the parameter number for the PSX-file
 * * flags = Bit-mask of command-line flags
 * * waterColour = Place to store the water colour
 * * sfxPath = Pointer to pointer to the path of MAIN.SFX
 */
int interpretParameters(int argc, char *argv[], int *pcparam, int *psxparam,
                        INTU32 *flagsptr, struct water_colour *waterColour,
                        INTU32 *skyColour, char **sfxPath)
{
	/* Variable Declarations */
	INTU32 flags;       /* Local copy of flags */
	INTU32 colour;      /* Colour value (in HEX) */
	int curParam;       /* Current command-line parameter */
	size_t paramLength; /* Length of the current command-line parameter */
	size_t curChar;     /* Current character in parsing the parameter */
	INTU8 state;        /* Current state of the parser (water colour) */
	char *freeptr1;     /* Pointer for general use */
	
	/* Sets flags to 0 before we begin */
	flags = 0x00000000;
	
	/* Loops through command-line parameters */
	for (curParam = 1; curParam < argc; ++curParam)
	{
		/* Interprets parameters by setting bits in bit-mask */
		if ((compareString(argv[curParam], "--license", 9) == 0) ||
		    (compareString(argv[curParam], "-l", 2) == 0))
		{
			setflag(FLAGS_LICENSE);
		}
		else if ((compareString(argv[curParam], "--verbose", 9) == 0) ||
		         (compareString(argv[curParam], "-v", 2) == 0))
		{
			setflag(FLAGS_VERBOSE);
		}
		else if ((compareString(argv[curParam], "--skip-entities", 15) == 0) ||
		         (compareString(argv[curParam], "-se", 3) == 0))
		{
			setflag(FLAGS_SKIP_ENTITIES);
		}
		else if ((compareString(argv[curParam], "--skip-boxes", 12) == 0) ||
		         (compareString(argv[curParam], "-sb", 3) == 0))
		{
			setflag(FLAGS_SKIP_BOXES);
		}
		else if ((compareString(argv[curParam],
		          "--skip-sound-sources", 20) == 0) ||
		         (compareString(argv[curParam], "-ss", 3) == 0))
		{
			setflag(FLAGS_SKIP_SOUNDSOURCES);
		}
		else if ((compareString(argv[curParam], "--skip-cameras", 14) == 0) ||
		         (compareString(argv[curParam], "-sc", 3) == 0))
		{
			setflag(FLAGS_SKIP_CAMERAS);
		}
		else if ((compareString(argv[curParam],
		          "--skip-floor-data", 17) == 0) ||
		         (compareString(argv[curParam], "-sf", 3) == 0))
		{
			setflag(FLAGS_SKIP_FLOORDATA);
		}
		else if ((compareString(argv[curParam], "--skip-rooms", 12) == 0) ||
		         (compareString(argv[curParam], "-sr", 3) == 0))
		{
			setflag(FLAGS_SKIP_ROOMS);
		}
		else if ((compareString(argv[curParam], "--skip-meshes", 14) == 0) ||
		         (compareString(argv[curParam], "-sm", 3) == 0))
		{
			setflag(FLAGS_SKIP_MESHES);
		}
		else if ((compareString(argv[curParam],
		          "--skip-animations", 17) == 0) ||
		         (compareString(argv[curParam], "-sa", 3) == 0))
		{
			setflag(FLAGS_SKIP_ANIMATIONS);
		}
		else if ((compareString(argv[curParam], "--skip-moveables", 16) == 0) ||
		         (compareString(argv[curParam], "-so", 3) == 0))
		{
			setflag(FLAGS_SKIP_MOVEABLES);
		}
		else if ((compareString(argv[curParam],
		          "--skip-staticmeshes", 19) == 0) ||
		         (compareString(argv[curParam], "-st", 3) == 0))
		{
			setflag(FLAGS_SKIP_STATICMESHES);
		}
		else if ((compareString(argv[curParam], "--water-colour=", 15) == 0) ||
		         (compareString(argv[curParam], "-wc=", 4) == 0))
		{
			/* Sets the flag up for custom water colour */
			setflag(FLAGS_WATER_COLOUR);
			
			/* Sets the water colour calculations to 0 */
			waterColour->instruction = 0;
			waterColour->red_operand = 0;
			
			/* Determines where the colour instructions start */
			if (compareString(argv[curParam], "--water-colour=", 15) == 0)
			{
				curChar = 15;
			}
			else
			{
				curChar = 4;
			}
			
			/* Processes colour instructions */
			paramLength = strlen(argv[curParam]);
			if ((paramLength > curChar) && (argv[curParam][curChar] == '='))
			{
				++curChar;
				if ((paramLength - curChar) < (size_t) 6)
				{
					return ERROR_INVALID_WATER_COLOUR;
				}
				colour = (INTU32) strtol(&argv[curParam][curChar], NULL, 16);
				waterColour->instruction = 0x0DDD;
				waterColour->red_operand = (INTU16) ((colour & 0x00F80000) >>
				                                     19U);
				waterColour->green_operand = (INTU16) ((colour & 0x0000F800) >>
				                                       11U);
				waterColour->blue_operand = (INTU16) ((colour & 0x000000F8) >>
				                                      3U);
			}
			else
			{
				(void) memset(waterColour, 0, sizeof(struct water_colour));
				for (state = (INTU8) 0x00; curChar < paramLength; ++curChar)
				{
					switch (argv[curParam][curChar])
					{
						case 'R': /* Red channel value */
						case 'r': /* Red channel value */
							if (((state & 3) == (INTU8) 0) ||
							    ((state & 32) != (INTU8) 0))
							{
								state = (INTU8) 17;
							}
							else if ((state & 3) == (INTU8) 1)
							{
								return ERROR_INVALID_WATER_COLOUR;
							}
							else if ((state & 3) == (INTU8) 2)
							{
								if ((state & 16) != (INTU8) 0)
								{
									waterColour->red_operand = 0x1000;
								}
								else if ((state & 8) != (INTU8) 0)
								{
									waterColour->green_operand = 0x1000;
								}
								else if ((state & 4) != (INTU8) 0)
								{
									waterColour->blue_operand = 0x1000;
								}
								state = (INTU8) 0;
							}
							break;
						case 'G': /* Green channel value */
						case 'g': /* Green channel value */
							if (((state & 3) == (INTU8) 0) ||
							    ((state & 32) != (INTU8) 0))
							{
								state = (INTU8) 9;
							}
							else if ((state & 3) == (INTU8) 1)
							{
								return ERROR_INVALID_WATER_COLOUR;
							}
							else if ((state & 3) == (INTU8) 2)
							{
								if ((state & 16) != (INTU8) 0)
								{
									waterColour->red_operand = 0x2000;
								}
								else if ((state & 8) != (INTU8) 0)
								{
									waterColour->green_operand = 0x2000;
								}
								else if ((state & 4) != (INTU8) 0)
								{
									waterColour->blue_operand = 0x2000;
								}
								state = (INTU8) 0;
							}
							break;
						case 'B': /* Blue channel value */
						case 'b': /* Blue channel value */
							if (((state & 3) == (INTU8) 0) ||
							    ((state & 32) != (INTU8) 0))
							{
								state = (INTU8) 5;
							}
							else if ((state & 3) == (INTU8) 1)
							{
								return ERROR_INVALID_WATER_COLOUR;
							}
							else if ((state & 3) == (INTU8) 2)
							{
								if ((state & 16) != (INTU8) 0)
								{
									waterColour->red_operand = 0x3000;
								}
								else if ((state & 8) != (INTU8) 0)
								{
									waterColour->green_operand = 0x3000;
								}
								else if ((state & 4) != (INTU8) 0)
								{
									waterColour->blue_operand = 0x3000;
								}
								state = (INTU8) 0;
							}
							break;
						case '+': /* Addition */
							if ((state & 3) != (INTU8) 1)
							{
								return ERROR_INVALID_WATER_COLOUR;
							}
							if ((state & 16) != (INTU8) 0)
							{
								waterColour->instruction &= 0xFFF0;
								waterColour->instruction |= 0x0001;
							}
							else if ((state & 8) != (INTU8) 0)
							{
								waterColour->instruction &= 0xFF0F;
								waterColour->instruction |= 0x0010;
							}
							else if ((state & 4) != (INTU8) 0)
							{
								waterColour->instruction &= 0xF0FF;
								waterColour->instruction |= 0x0100;
							}
							++state;
							break;
						case '-': /* Subtraction */
							if ((state & 3) != (INTU8) 1)
							{
								return ERROR_INVALID_WATER_COLOUR;
							}
							if ((state & 16) != (INTU8) 0)
							{
								waterColour->instruction &= 0xFFF0;
								waterColour->instruction |= 0x0002;
							}
							else if ((state & 8) != (INTU8) 0)
							{
								waterColour->instruction &= 0xFF0F;
								waterColour->instruction |= 0x0020;
							}
							else if ((state & 4) != (INTU8) 0)
							{
								waterColour->instruction &= 0xF0FF;
								waterColour->instruction |= 0x0200;
							}
							++state;
							break;
						case '*': /* Multiplication */
						case 'x':
							if ((state & 3) != (INTU8) 1)
							{
								return ERROR_INVALID_WATER_COLOUR;
							}
							if ((state & 16) != (INTU8) 0)
							{
								waterColour->instruction &= 0xFFF0;
								waterColour->instruction |= 0x0003;
							}
							else if ((state & 8) != (INTU8) 0)
							{
								waterColour->instruction &= 0xFF0F;
								waterColour->instruction |= 0x0030;
							}
							else if ((state & 4) != (INTU8) 0)
							{
								waterColour->instruction &= 0xF0FF;
								waterColour->instruction |= 0x0300;
							}
							++state;
							break;
						case '/': /* Division */
						case '\\':
						case ':':
							if ((state & 3) != (INTU8) 1)
							{
								return ERROR_INVALID_WATER_COLOUR;
							}
							if ((state & 16) != (INTU8) 0)
							{
								waterColour->instruction &= 0xFFF0;
								waterColour->instruction |= 0x0004;
							}
							else if ((state & 8) != (INTU8) 0)
							{
								waterColour->instruction &= 0xFF0F;
								waterColour->instruction |= 0x0040;
							}
							else if ((state & 4) != (INTU8) 0)
							{
								waterColour->instruction &= 0xF0FF;
								waterColour->instruction |= 0x0400;
							}
							++state;
							break;
						case '&': /* AND operation */
							if ((state & 3) != (INTU8) 1)
							{
								return ERROR_INVALID_WATER_COLOUR;
							}
							if ((state & 16) != (INTU8) 0)
							{
								waterColour->instruction &= 0xFFF0;
								waterColour->instruction |= 0x0005;
							}
							else if ((state & 8) != (INTU8) 0)
							{
								waterColour->instruction &= 0xFF0F;
								waterColour->instruction |= 0x0050;
							}
							else if ((state & 4) != (INTU8) 0)
							{
								waterColour->instruction &= 0xF0FF;
								waterColour->instruction |= 0x0500;
							}
							++state;
							break;
						case '|': /* OR operation */
							if ((state & 3) != (INTU8) 1)
							{
								return ERROR_INVALID_WATER_COLOUR;
							}
							if ((state & 16) != (INTU8) 0)
							{
								waterColour->instruction &= 0xFFF0;
								waterColour->instruction |= 0x0006;
							}
							else if ((state & 8) != (INTU8) 0)
							{
								waterColour->instruction &= 0xFF0F;
								waterColour->instruction |= 0x0060;
							}
							else if ((state & 4) != (INTU8) 0)
							{
								waterColour->instruction &= 0xF0FF;
								waterColour->instruction |= 0x0600;
							}
							++state;
							break;
						case '^': /* XOR operation */
							if ((state & 3) != (INTU8) 1)
							{
								return ERROR_INVALID_WATER_COLOUR;
							}
							if ((state & 16) != (INTU8) 0)
							{
								waterColour->instruction &= 0xFFF0;
								waterColour->instruction |= 0x0007;
							}
							else if ((state & 8) != (INTU8) 0)
							{
								waterColour->instruction &= 0xFF0F;
								waterColour->instruction |= 0x0070;
							}
							else if ((state & 4) != (INTU8) 0)
							{
								waterColour->instruction &= 0xF0FF;
								waterColour->instruction |= 0x0700;
							}
							++state;
							break;
						case '~': /* One's complement operation */
						case '!':
							if ((state & 3) != (INTU8) 1)
							{
								return ERROR_INVALID_WATER_COLOUR;
							}
							if ((state & 16) != (INTU8) 0)
							{
								waterColour->instruction &= 0xFFF0;
								waterColour->instruction |= 0x0008;
							}
							else if ((state & 8) != (INTU8) 0)
							{
								waterColour->instruction &= 0xFF0F;
								waterColour->instruction |= 0x0080;
							}
							else if ((state & 4) != (INTU8) 0)
							{
								waterColour->instruction &= 0xF0FF;
								waterColour->instruction |= 0x0800;
							}
							++state;
							break;
						case '<': /* Logical left shift */
							if ((state & 3) != (INTU8) 1)
							{
								return ERROR_INVALID_WATER_COLOUR;
							}
							if ((state & 16) != (INTU8) 0)
							{
								waterColour->instruction &= 0xFFF0;
								waterColour->instruction |= 0x0009;
							}
							else if ((state & 8) != (INTU8) 0)
							{
								waterColour->instruction &= 0xFF0F;
								waterColour->instruction |= 0x0090;
							}
							else if ((state & 4) != (INTU8) 0)
							{
								waterColour->instruction &= 0xF0FF;
								waterColour->instruction |= 0x0900;
							}
							++state;
							break;
						case '>': /* Logical right shift */
							if ((state & 3) != (INTU8) 1)
							{
								return ERROR_INVALID_WATER_COLOUR;
							}
							if ((state & 16) != (INTU8) 0)
							{
								waterColour->instruction &= 0xFFF0;
								waterColour->instruction |= 0x000A;
							}
							else if ((state & 8) != (INTU8) 0)
							{
								waterColour->instruction &= 0xFF0F;
								waterColour->instruction |= 0x00A0;
							}
							else if ((state & 4) != (INTU8) 0)
							{
								waterColour->instruction &= 0xF0FF;
								waterColour->instruction |= 0x0A00;
							}
							++state;
							break;
						case '%': /* Modulo operation */
							if ((state & 3) != (INTU8) 1)
							{
								return ERROR_INVALID_WATER_COLOUR;
							}
							if ((state & 16) != (INTU8) 0)
							{
								waterColour->instruction &= 0xFFF0;
								waterColour->instruction |= 0x000B;
							}
							else if ((state & 8) != (INTU8) 0)
							{
								waterColour->instruction &= 0xFF0F;
								waterColour->instruction |= 0x00B0;
							}
							else if ((state & 4) != (INTU8) 0)
							{
								waterColour->instruction &= 0xF0FF;
								waterColour->instruction |= 0x0B00;
							}
							++state;
							break;
						case '=': /* Value assignment */
							if ((state & 3) != (INTU8) 1)
							{
								return ERROR_INVALID_WATER_COLOUR;
							}
							if ((state & 16) != (INTU8) 0)
							{
								waterColour->instruction &= 0xFFF0;
								waterColour->instruction |= 0x000C;
							}
							else if ((state & 8) != (INTU8) 0)
							{
								waterColour->instruction &= 0xFF0F;
								waterColour->instruction |= 0x00C0;
							}
							else if ((state & 4) != (INTU8) 0)
							{
								waterColour->instruction &= 0xF0FF;
								waterColour->instruction |= 0x0C00;
							}
							++state;
							break;
						case '0': /* Numerical input */
						case '1':
						case '2':
						case '3':
						case '4':
						case '5':
						case '6':
						case '7':
						case '8':
						case '9':
							if ((state & 3) != (INTU8) 2)
							{
								return ERROR_INVALID_WATER_COLOUR;
							}
							state |= (INTU8) 32;
							if ((state & 16) != (INTU8) 0)
							{
								waterColour->red_operand *= 10;
								switch (argv[curParam][curChar])
								{
									case '1':
										waterColour->red_operand += 1;
										break;
									case '2':
										waterColour->red_operand += 2;
										break;
									case '3':
										waterColour->red_operand += 3;
										break;
									case '4':
										waterColour->red_operand += 4;
										break;
									case '5':
										waterColour->red_operand += 5;
										break;
									case '6':
										waterColour->red_operand += 6;
										break;
									case '7':
										waterColour->red_operand += 7;
										break;
									case '8':
										waterColour->red_operand += 8;
										break;
									case '9':
										waterColour->red_operand += 9;
										break;
								}
								waterColour->red_operand &= 0x001F;
							}
							else if ((state & 8) != (INTU8) 0)
							{
								waterColour->green_operand *= 10;
								switch (argv[curParam][curChar])
								{
									case '1':
										waterColour->green_operand += 1;
										break;
									case '2':
										waterColour->green_operand += 2;
										break;
									case '3':
										waterColour->green_operand += 3;
										break;
									case '4':
										waterColour->green_operand += 4;
										break;
									case '5':
										waterColour->green_operand += 5;
										break;
									case '6':
										waterColour->green_operand += 6;
										break;
									case '7':
										waterColour->green_operand += 7;
										break;
									case '8':
										waterColour->green_operand += 8;
										break;
									case '9':
										waterColour->green_operand += 9;
										break;
								}
								waterColour->green_operand &= 0x001F;
							}
							else if ((state & 4) != (INTU8) 0)
							{
								waterColour->blue_operand *= 10;
								switch (argv[curParam][curChar])
								{
									case '1':
										waterColour->blue_operand += 1;
										break;
									case '2':
										waterColour->blue_operand += 2;
										break;
									case '3':
										waterColour->blue_operand += 3;
										break;
									case '4':
										waterColour->blue_operand += 4;
										break;
									case '5':
										waterColour->blue_operand += 5;
										break;
									case '6':
										waterColour->blue_operand += 6;
										break;
									case '7':
										waterColour->blue_operand += 7;
										break;
									case '8':
										waterColour->blue_operand += 8;
										break;
									case '9':
										waterColour->blue_operand += 9;
										break;
								}
								waterColour->blue_operand &= 0x001F;
							}
							break;
						case 'o': /* Allows overflow to occur */
							waterColour->instruction |= 0x1000;
							break;
						case 'c': /* Base on old water palette (undocumented) */
							waterColour->instruction |= 0x2000;
							break;
						case 'a': /* Change normal palette (undocumented) */
							waterColour->instruction |= 0x4000;
							break;
						default: /* Any other character resets the state to 0 */
							state = (INTU8) 0;
							break;
					}
				}
			}
		}
		else if ((compareString(argv[curParam], "--no-signature", 14) == 0) ||
		         (compareString(argv[curParam], "-ns", 3) == 0))
		{
			setflag(FLAGS_NO_SIGNATURE);
		}
		else if ((compareString(argv[curParam], "--skip-textures", 15) == 0) ||
		         (compareString(argv[curParam], "-sd", 3) == 0))
		{
			setflag(FLAGS_SKIP_TEXTURES);
		}
		else if ((compareString(argv[curParam], "--dont-optimise", 15) == 0) ||
		         (compareString(argv[curParam], "-do", 3) == 0))
		{
			setflag(FLAGS_NO_TEXTOPTIMISE);
		}
		else if ((compareString(argv[curParam],
		          "--dont-fix-textures", 19) == 0) ||
		         (compareString(argv[curParam], "-dt", 3) == 0))
		{
			setflag(FLAGS_DONT_FIX_TEXTURES);
		}
		else if ((compareString(argv[curParam], "--skip-audio", 12) == 0) ||
		         (compareString(argv[curParam], "-su", 3) == 0))
		{
			setflag(FLAGS_SKIP_AUDIO);
		}
		else if ((compareString(argv[curParam],
		          "--skip-cinematic-frames", 23) == 0) ||
		         (compareString(argv[curParam], "-sn", 3) == 0))
		{
			setflag(FLAGS_SKIP_CINEMATICS);
		}
		else if ((compareString(argv[curParam], "--ukrev", 7) == 0) ||
		         (compareString(argv[curParam], "-ukrev", 6) == 0))
		{
			unsetflag(FLAGS_LANGUAGE);
			setflag(FLAGS_LANGUAGE_UK_REV1);
		}
		else if ((compareString(argv[curParam], "--uk", 4) == 0) ||
		         (compareString(argv[curParam], "-uk", 3) == 0))
		{
			unsetflag(FLAGS_LANGUAGE);
			setflag(FLAGS_LANGUAGE_UK);
		}
		else if ((compareString(argv[curParam], "--germanrev", 11) == 0) ||
		         (compareString(argv[curParam], "-derev", 6) == 0))
		{
			unsetflag(FLAGS_LANGUAGE);
			setflag(FLAGS_LANGUAGE_DE_REV1);
		}
		else if ((compareString(argv[curParam], "--german", 8) == 0) ||
		         (compareString(argv[curParam], "-de", 3) == 0))
		{
			unsetflag(FLAGS_LANGUAGE);
			setflag(FLAGS_LANGUAGE_DE);
		}
		else if ((compareString(argv[curParam], "--frenchrev", 11) == 0) ||
		         (compareString(argv[curParam], "-frrev", 6) == 0))
		{
			unsetflag(FLAGS_LANGUAGE);
			setflag(FLAGS_LANGUAGE_FR_REV1);
		}
		else if ((compareString(argv[curParam], "--french", 8) == 0) ||
		         (compareString(argv[curParam], "-fr", 3) == 0))
		{
			unsetflag(FLAGS_LANGUAGE);
			setflag(FLAGS_LANGUAGE_FR);
		}
		else if ((compareString(argv[curParam], "--italianrev", 12) == 0) ||
		         (compareString(argv[curParam], "-itrev", 6) == 0))
		{
			unsetflag(FLAGS_LANGUAGE);
			setflag(FLAGS_LANGUAGE_IT_REV1);
		}
		else if ((compareString(argv[curParam], "--italian", 9) == 0) ||
		         (compareString(argv[curParam], "-it", 3) == 0))
		{
			unsetflag(FLAGS_LANGUAGE);
			setflag(FLAGS_LANGUAGE_IT);
		}
		else if ((compareString(argv[curParam], "--spanishrev", 12) == 0) ||
		         (compareString(argv[curParam], "-sprev", 6) == 0))
		{
			unsetflag(FLAGS_LANGUAGE);
			setflag(FLAGS_LANGUAGE_SP_REV1);
		}
		else if ((compareString(argv[curParam], "--spanish", 9) == 0) ||
		         (compareString(argv[curParam], "-sp", 3) == 0))
		{
			unsetflag(FLAGS_LANGUAGE);
			setflag(FLAGS_LANGUAGE_SP);
		}
		else if ((compareString(argv[curParam], "--japanese2", 11) == 0) ||
		         (compareString(argv[curParam], "-jp2", 4) == 0))
		{
			unsetflag(FLAGS_LANGUAGE);
			setflag(FLAGS_LANGUAGE_JP_EN);
		}
		else if ((compareString(argv[curParam], "--japanese", 10) == 0) ||
		         (compareString(argv[curParam], "-jp", 3) == 0))
		{
			unsetflag(FLAGS_LANGUAGE);
			setflag(FLAGS_LANGUAGE_JP_JP);
		}
		else if ((compareString(argv[curParam], "--asian2", 8) == 0) ||
		         (compareString(argv[curParam], "-as2", 4) == 0))
		{
			unsetflag(FLAGS_LANGUAGE);
			setflag(FLAGS_LANGUAGE_ASIA_EN);
		}
		else if ((compareString(argv[curParam], "--asian", 7) == 0) ||
		         (compareString(argv[curParam], "-as", 3) == 0))
		{
			unsetflag(FLAGS_LANGUAGE);
			setflag(FLAGS_LANGUAGE_ASIA_JP);
		}
		else if ((compareString(argv[curParam], "--us10", 6) == 0) ||
		         (compareString(argv[curParam], "-us10", 5) == 0))
		{
			unsetflag(FLAGS_LANGUAGE);
			setflag(FLAGS_LANGUAGE_US_10);
		}
		else if ((compareString(argv[curParam], "--us", 4) == 0) ||
		         (compareString(argv[curParam], "-us", 3) == 0))
		{
			unsetflag(FLAGS_LANGUAGE);
			setflag(FLAGS_LANGUAGE_US_11_12);
		}
		else if ((compareString(argv[curParam], "--skip-modules", 14) == 0) ||
		         (compareString(argv[curParam], "-sx", 3) == 0))
		{
			setflag(FLAGS_SKIP_MODULES);
		}
		else if ((compareString(argv[curParam], "--rain", 6) == 0) ||
		         (compareString(argv[curParam], "-wr", 3) == 0))
		{
			unsetflag(FLAGS_WEATHER);
			setflag(FLAGS_WEATHER_RAIN);
		}
		else if ((compareString(argv[curParam], "--snow", 6) == 0) ||
		         (compareString(argv[curParam], "-ws", 3) == 0))
		{
			unsetflag(FLAGS_WEATHER);
			setflag(FLAGS_WEATHER_SNOW);
		}
		else if ((compareString(argv[curParam], "--speckle", 9) == 0) ||
		         (compareString(argv[curParam], "-wu", 3) == 0))
		{
			unsetflag(FLAGS_WEATHER);
			setflag(FLAGS_WEATHER_SPECLE);
		}
		else if ((compareString(argv[curParam], "--electric-death", 16) == 0) ||
		         (compareString(argv[curParam], "-ed", 3) == 0))
		{
			setflag(FLAGS_ELECTRIC_DEATH);
		}
		else if ((compareString(argv[curParam], "--mipmap-once", 13) == 0) ||
		         (compareString(argv[curParam], "-mo", 3) == 0))
		{
			setflag(FLAGS_NO_SECOND_MIPMAP);
		}
		else if ((compareString(argv[curParam], "--no-mipmap", 11) == 0) ||
		         (compareString(argv[curParam], "-nm", 3) == 0))
		{
			setflag(FLAGS_NO_MIPMAP_AT_ALL);
		}
		else if ((compareString(argv[curParam], "--sky-colour=", 13) == 0) ||
		         (compareString(argv[curParam], "-cs=", 4) == 0))
		{
			freeptr1 = argv[curParam];
			while (*freeptr1 != '=')
			{
				++freeptr1;
			}
			++freeptr1;
			*skyColour = (INTU32) strtol(freeptr1, NULL, 16);
			setflag(FLAGS_SKY_COLOUR);
		}
		else
		{
			paramLength = strlen(argv[curParam]);
			if (paramLength >= 3)
			{
				/* Checks whether this parameter is the PC-file */
				if (((argv[curParam][paramLength - 3] == 'T') ||
				     (argv[curParam][paramLength - 3] == 't')) &&
				    ((argv[curParam][paramLength - 2] == 'R') ||
				     (argv[curParam][paramLength - 2] == 'r')) &&
				    (argv[curParam][paramLength - 1] == '2'))
				{
					*pcparam = curParam;
				}
				/* Checks whether this parameter is the PSX-file */
				if (((argv[curParam][paramLength - 3] == 'P') ||
				     (argv[curParam][paramLength - 3] == 'p')) &&
				    ((argv[curParam][paramLength - 2] == 'S') ||
				     (argv[curParam][paramLength - 2] == 's')) &&
				    ((argv[curParam][paramLength - 1] == 'X') ||
				     (argv[curParam][paramLength - 1] == 'x')))
				{
					*psxparam = curParam;
				}
				/* Checks whether this parameter is the SFX-file */
				if (((argv[curParam][paramLength - 3] == 'S') ||
				     (argv[curParam][paramLength - 3] == 's')) &&
				    ((argv[curParam][paramLength - 2] == 'F') ||
				     (argv[curParam][paramLength - 2] == 'f')) &&
				    ((argv[curParam][paramLength - 1] == 'X') ||
				     (argv[curParam][paramLength - 1] == 'x')))
				{
					*sfxPath = argv[curParam];
				}
			}
		}
	}
	
	/* Exports the flags to the rest of the program */
	*flagsptr = flags;
	return ERROR_NONE;
}
