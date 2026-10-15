/*
 * Library that contains some custom functions relating to strings
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

/*
 * Function that converts an unsigned 32-bit integer to a string
 * Parameters:
 *   string = Pointer to output string location (must be allocated)
 *       in = Input integer to be converted
 *     base = Base number for the conversion (1-36)
 *   length = The minimal length of the output string
 */
void inttostring(char *string, unsigned INT32 in, int base, int length){
	/* Variable Declarations */
	int curpos; /* Current position in the output string */
	int freeint1; /* Integer for general use */
	char freechar1; /* Character for general use */

	/* Converts the integer to characters (in reverse) */
	for (curpos=0;in!=0;++curpos){
		/* Determines current last character */
		if ((in%base)>=10){
			memset(string+curpos,((int)('A'+((char)((in%base)-10)))),1);
		}else{
			memset(string+curpos,((int)('0'+((char)(in%base)))),1);
		}
		/* Removes last character */
		in-=(in%base);
		in/=base;
	}
	
	/* Adds 0's to fill the length */
	for (;length>curpos;++curpos) string[curpos]='0';
	
	/* NULL-terminates the string */
	memset(string+curpos,0,1);
	
	/* Reverses the string to its correct order */
	--curpos;
	freeint1=0;
	while (curpos>freeint1){
		memmove(&freechar1,string+curpos,1);
		memmove(string+curpos,string+freeint1,1);
		memmove(string+freeint1,&freechar1,1);
		--curpos;
		++freeint1;
	}
}

/*
 * Function that reads a line from a text file into memory.
 * Parameters:
 *   string = Pointer to output location (must be allocated)
 *   stringlength = Allocated space of string
 *   file = File to be read from (must be opened)
 *   filesize = Length of this file in bytes
 * Return values:
 *   0 = Everything went well
 *   2 = Input file could not be read from
 */
int readline(char *string, unsigned long int stringlength, FILE *file, long int filesize){
	/* Variable declarations */
	unsigned long int curpos; /* Current position in the buffer */
	size_t freesizet1; /* size_t for general use */
	int EOL=0; /* Whether the end of the current line has been reached */
	char *ptr1; /* Pointer for general use */
	
	/* Reads characters */
	--stringlength; /* To make room for the NULL-terminator */ 
	for (curpos=0;((curpos<stringlength)&&(ftell(file)<filesize)&&(EOL==0));++curpos){
		freesizet1=fread(string+curpos,1,1,file);
		if (freesizet1!=1) return 2;
		memset(string+curpos+1,0,1);
		ptr1=strstr(string,"\n");
		if (ptr1!=NULL){
			EOL=1;
			memset(ptr1,0,1);
		}
	}
	
	/* NULL terminates the string in case of end of file */
	if (ftell(file)==filesize) memset(string+curpos,0,1);
	
	/* Returns the function */
	return 0;
}
