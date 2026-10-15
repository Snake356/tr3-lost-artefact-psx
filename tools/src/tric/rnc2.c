/*
 * Library for Compressing Files using RNC2 (Adjusted for TRIC)
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
#include "rnc2.h"

/* Static Variable Declaration */
static signed int curbit; /* Current bit in outbuffer */
static unsigned long int curbitbyte; /* Byte that holds curbit */
static unsigned long int inbufedge; /* Edge of inbuffer */
static unsigned long int infilesize; /* Size of infile */
static size_t inbufsize; /* Size of inbuffer */
static size_t outbufsize; /* Size of output buffer */
static unsigned int chunksize; /* Size of the current chunk */
static unsigned int numchunks; /* Number of chunks */
static unsigned long int nextbyte; /* Next unused byte in outbuffer */
static int inbufreft; /* Inbuffer has been refreshed (0=no, 1=yes) */
static int eofreached; /* End of infile reached (0=no, 1=yes) */
static size_t curpos; /* Current position in processing forwards */
static size_t posnewbytes; /* Offset of new bytes to be processed */

/* Static Function Declarations */
static int writebit(unsigned char *outbuffer, FILE *outfile,
                    unsigned int value);
static void writebyte(unsigned char *outbuffer, unsigned int value);
static int writeBank(unsigned char *outbuffer, FILE *outfile,
                     unsigned int value);
static int processFunctions(unsigned char *inbuffer,
                            unsigned char *outbuffer, FILE *outfile,
                            unsigned int numnbytes, size_t noffset,
                            unsigned int numbytes,
                            unsigned int distance);
static int refreshInBuffer(FILE *infile, unsigned char *inbuffer);

/*
 * Function that compresses a file using RNC2
 * Parameters:
 *   infile = Uncompressed input file (must already be opened)
 *   outfile = Compressed output file (must already be opened)
 * Return values:
 *   0 = Everything went well
 *   2 = Input file could not be read from
 *   4 = Output file could not be written to
 *   5 = Buffer could not be allocated
 */
int rnc2(FILE *infile, FILE *outfile){
	/* Variable Declarations */
	unsigned char *inbuffer=NULL; /* Input buffer */
	unsigned char *outbuffer=NULL; /* Output buffer */
	size_t freesizet1; /* size_t for general use */
	unsigned long int freeulong1; /* For general use */
	int freeint1; /* Integer for general use */
	int retval=0; /* Return value for this function */
	unsigned int searchdist; /* Current distance in backwards search */
	unsigned int bestnum; /* Number of bytes in best found match */
	unsigned int bestdist; /* Distance of best found match */
	unsigned int numidentbytes; /* Number of identical bytes */
	unsigned int numnewbytes; /* Number of new bytes to be processed */
	
	/* Determines infilesize */
	freeint1=fseek(infile,0,SEEK_END);
	if (freeint1!=0){
		retval=2;
		goto end;
	}
	infilesize=(unsigned long int) ftell(infile);
	
	/* Allocates buffer space */
	memset(&inbufsize,0xFF,sizeof(inbufsize));
	freeulong1=(((infilesize*2)>4500)?(infilesize*2):4500);
	if (freeulong1<((unsigned long int)inbufsize)){
		inbufsize=(size_t)(freeulong1);
		inbuffer=malloc(inbufsize);
	}
	if (inbuffer==NULL) memset(&inbufsize,0xFF,sizeof(inbufsize));
	while (inbuffer==NULL){
		inbuffer=malloc(inbufsize);
		if (inbuffer==NULL){
			if (inbufsize==8191){
				inbufsize=4500;
				inbuffer=malloc(inbufsize);
				if (inbuffer==NULL){
					retval=5;
					goto end;
				}else{
					break;
				}
			}
			inbufsize>>=1;
		}
	}
	
	/* Divides the buffer space between inbuffer and outbuffer */
	if (inbufsize>=8860) outbufsize=(inbufsize>>1);
	else outbufsize=(inbufsize-4420);
	inbufsize-=outbufsize;
	outbuffer=(unsigned char *)(inbuffer+inbufsize);
	
	/* Sets up the starting state */
	freeint1=fseek(infile,0,SEEK_SET);
	if (freeint1!=0){
		retval=2;
		goto end;
	}
	memset(outbuffer,0,1);
	eofreached=0;
	inbufreft=0;
	chunksize=0;
	numchunks=1;
	curbitbyte=0;
	nextbyte=1;
	curbit=5;
	
	/* Fills inbuffer */
	if (((unsigned long int)inbufsize)>=infilesize){
		freesizet1=fread(inbuffer,1,(size_t)infilesize,infile);
		if (freesizet1!=(size_t)infilesize){
			retval=2;
			goto end;
		}
		inbufedge=(unsigned long int) infilesize;
		eofreached=1;
	}else{
		freesizet1=fread(inbuffer,1,inbufsize,infile);
		if (freesizet1!=inbufsize){
			retval=2;
			goto end;
		}
		inbufedge=(unsigned long int) inbufsize;
	}
	
	/* Processes data forwards */
	curpos=1;
	numnewbytes=1;
	posnewbytes=0;
	while ((((unsigned long int)curpos)!=inbufedge)||(eofreached==0)){
		/* Searches backwards for duplicate bytes */
		bestnum=0;
		bestdist=0;
		for (searchdist=1;searchdist<=4096;++searchdist){
			if (inbuffer[curpos]==inbuffer[curpos-searchdist]){
				/* Found duplicate, counts indentical bytes */
				for (numidentbytes=1;numidentbytes<263;++numidentbytes){
					if ((unsigned long int)(curpos+numidentbytes)==
					    inbufedge){
						if (eofreached==0){
							freeint1=refreshInBuffer(infile,inbuffer);
							if (freeint1!=0){
								retval=freeint1;
								goto end;
							}
						}else{
							break;
						}
					}
					if (inbuffer[curpos+numidentbytes-searchdist]!=
					    inbuffer[curpos+numidentbytes]) break;
				}
				if (numidentbytes>bestnum){
					bestnum=numidentbytes;
					bestdist=searchdist;
					if (bestnum==263) break;
				}
			}
			if ((curpos-searchdist)==0) break;
		}
		
		/* Processes best found match */
		if ((bestnum<2)||((bestnum==2)&&(bestdist>256))){
			if (numnewbytes==0) posnewbytes=curpos;
			++numnewbytes;
			if (numnewbytes==72){
				freeint1=processFunctions(inbuffer,outbuffer,outfile,
				                          numnewbytes,posnewbytes,0,0);
				if (freeint1!=0){
					retval=freeint1;
					goto end;
				}
				numnewbytes=0;
			}
		}else{
			freeint1=processFunctions(inbuffer,outbuffer,outfile,
			                          numnewbytes,posnewbytes,bestnum,
			                          bestdist);
			if (freeint1!=0){
				retval=freeint1;
				goto end;
			}
			curpos+=(bestnum-1);
			numnewbytes=0;
		}
		
		++curpos;
		
		/* Refreshes inbuffer if needed */
		if ((unsigned long int)curpos==inbufedge){
			if (eofreached==0){
				freeint1=refreshInBuffer(infile,inbuffer);
				if (freeint1!=0){
					retval=freeint1;
					goto end;
				}
			}else{
				if (numnewbytes>0){
					freeint1=processFunctions(inbuffer,outbuffer,
					                          outfile,numnewbytes,
					                          posnewbytes,0,0);
					if (freeint1!=0){
						retval=freeint1;
						goto end;
					}
				}
				break;
			}
		}
	}
	
	/* Adds end of file marker */
	freeint1=writebit(outbuffer,outfile,1);
	if (freeint1!=0){
		retval=freeint1;
		goto end;
	}
	freeint1=writebit(outbuffer,outfile,1);
	if (freeint1!=0){
		retval=freeint1;
		goto end;
	}
	freeint1=writebit(outbuffer,outfile,1);
	if (freeint1!=0){
		retval=freeint1;
		goto end;
	}
	freeint1=writebit(outbuffer,outfile,1);
	if (freeint1!=0){
		retval=freeint1;
		goto end;
	}
	writebyte(outbuffer,0);
	freeint1=writebit(outbuffer,outfile,0);
	if (freeint1!=0){
		retval=freeint1;
		goto end;
	}
	
	/* Writes the remainder of outbuffer to file */
	if (nextbyte>0){
		freesizet1=fwrite(outbuffer,1,(size_t)nextbyte,outfile);
		if (freesizet1!=((size_t)nextbyte)) return 4;
	}
	
end:/* Frees buffers and returns the function */
	if (inbuffer!=NULL){
		free(inbuffer);
	}
	return retval;
}

/*
 * Function that writes a bit to the bitstream
 * Parameters:
 *   outbuffer = Output Buffer
 *   outfile = Output File
 *   value = Value of the bit (0 or 1)
 * Return values:
 *   0 = Everything went well
 *   4 = Output file could not be written to
 */
static int writebit(unsigned char *outbuffer, FILE *outfile,
                    unsigned int value){
	/* Variable Declarations */
	size_t freesizet1; /* size_t for general use */
	
	/* Checks for the end of current bitbyte */
	if (curbit==-1){
		
		/* Checks for the end of outbuffer */
		if (nextbyte>=((unsigned long int)(outbufsize-74))){
			
			/* Writes outbuffer to file */
			freesizet1=fwrite(outbuffer,1,(size_t)nextbyte,outfile);
			if (freesizet1!=(size_t)nextbyte) return 4;
			nextbyte=0;
		}
		
		/* Uses next byte for bitstream */
		curbitbyte=nextbyte;
		++nextbyte;
		memset(outbuffer+curbitbyte,0,1);
		curbit=7;
	}
	
	/* Writes value to current bit */
	outbuffer[curbitbyte]|=((value&1)<<(unsigned int)(curbit));
	
	/* Iterates curbit */
	--curbit;
	
	/* Returns the function */
	return 0;
}

/*
 * Function that writes a single byte to the bytestream
 * Parameters:
 *   outbuffer = Output Buffer
 *   value = Value of the byte (0-255)
 */
static void writebyte(unsigned char *outbuffer, unsigned int value){
	/* Writes value to the next byte */
	outbuffer[nextbyte]=(unsigned char) value;
	
	/* Iterates nextbyte */
	++nextbyte;
}

/*
 * Function that processes the compression functions
 * Parameters:
 *   inbuffer = Input Buffer
 *   outbuffer = Output Buffer
 *   outfile = Output File
 *   numnbytes = Number of new bytes to be written
 *   noffset = Offset of the first new byte in inbuffer
 *   numbytes = Number of bytes to be copied
 *   distance = Number of bytes to go back
 * Return values:
 *   0 = Everything went well
 *   4 = Output file could not be written to
 */
static int processFunctions(unsigned char *inbuffer,
                            unsigned char *outbuffer, FILE *outfile,
                            unsigned int numnbytes, size_t noffset,
                            unsigned int numbytes,
                            unsigned int distance){
	/* Variable Declarations */
	int freeint1; /* Integer for general use */
	unsigned int freeint2; /* Integer for general use */
	
	/* Writes new bytes before copy */
	if (numnbytes>=12){
		/* Writes end of chunk marker if needed */
		if ((chunksize+(numnbytes-(numnbytes%4)))>12288){
			freeint1=writebit(outbuffer,outfile,1);
			if (freeint1!=0) return freeint1;
			freeint1=writebit(outbuffer,outfile,1);
			if (freeint1!=0) return freeint1;
			freeint1=writebit(outbuffer,outfile,1);
			if (freeint1!=0) return freeint1;
			freeint1=writebit(outbuffer,outfile,1);
			if (freeint1!=0) return freeint1;
			writebyte(outbuffer,0);
			freeint1=writebit(outbuffer,outfile,1);
			if (freeint1!=0) return freeint1;
			++numchunks;
			chunksize=0;
		}
		/* Writes function 3 */
		freeint1=writebit(outbuffer,outfile,1);
		if (freeint1!=0) return freeint1;
		freeint1=writebit(outbuffer,outfile,0);
		if (freeint1!=0) return freeint1;
		freeint1=writebit(outbuffer,outfile,1);
		if (freeint1!=0) return freeint1;
		freeint1=writebit(outbuffer,outfile,1);
		if (freeint1!=0) return freeint1;
		freeint1=writebit(outbuffer,outfile,1);
		if (freeint1!=0) return freeint1;
		freeint2=(((numnbytes-(numnbytes%4))/4)-3);
		freeint1=writebit(outbuffer,outfile,((freeint2&8)>>3));
		if (freeint1!=0) return freeint1;
		freeint1=writebit(outbuffer,outfile,((freeint2&4)>>2));
		if (freeint1!=0) return freeint1;
		freeint1=writebit(outbuffer,outfile,((freeint2&2)>>1));
		if (freeint1!=0) return freeint1;
		freeint1=writebit(outbuffer,outfile,(freeint2&1));
		if (freeint1!=0) return freeint1;
		
		/* Write bytes from input to output */
		for (freeint2=(numnbytes-(numnbytes%4));freeint2>0;--freeint2){
			writebyte(outbuffer,(unsigned int)inbuffer[noffset]);
			++noffset;
		}
		
		chunksize+=(numnbytes-(numnbytes%4));
		numnbytes%=4;
	}
	while (numnbytes>0){
		/* Writes end of chunk marker if needed */
		if ((chunksize+1)>12288){
			freeint1=writebit(outbuffer,outfile,1);
			if (freeint1!=0) return freeint1;
			freeint1=writebit(outbuffer,outfile,1);
			if (freeint1!=0) return freeint1;
			freeint1=writebit(outbuffer,outfile,1);
			if (freeint1!=0) return freeint1;
			freeint1=writebit(outbuffer,outfile,1);
			if (freeint1!=0) return freeint1;
			writebyte(outbuffer,0);
			freeint1=writebit(outbuffer,outfile,1);
			if (freeint1!=0) return freeint1;
			++numchunks;
			chunksize=0;
		}
		
		/* Writes function 1 */
		freeint1=writebit(outbuffer,outfile,0);
		if (freeint1!=0) return freeint1;
		
		/* Write byte from input to output */
		if (noffset==inbufsize) noffset=0;
		writebyte(outbuffer, (unsigned int)inbuffer[noffset]);
		++noffset;
		
		++chunksize;
		--numnbytes;
	}
	
	/* Writes end of chunk marker if needed */
	if ((chunksize+numbytes)>12288){
		freeint1=writebit(outbuffer,outfile,1);
		if (freeint1!=0) return freeint1;
		freeint1=writebit(outbuffer,outfile,1);
		if (freeint1!=0) return freeint1;
		freeint1=writebit(outbuffer,outfile,1);
		if (freeint1!=0) return freeint1;
		freeint1=writebit(outbuffer,outfile,1);
		if (freeint1!=0) return freeint1;
		writebyte(outbuffer,0);
		freeint1=writebit(outbuffer,outfile,1);
		if (freeint1!=0) return freeint1;
		++numchunks;
		chunksize=0;
	}
	
	/* Processes copy command */
	chunksize+=numbytes;
	--distance;
	if (numbytes>=9){
		/* Writes function 8 */
		freeint1=writebit(outbuffer,outfile,1);
		if (freeint1!=0) return freeint1;
		freeint1=writebit(outbuffer,outfile,1);
		if (freeint1!=0) return freeint1;
		freeint1=writebit(outbuffer,outfile,1);
		if (freeint1!=0) return freeint1;
		freeint1=writebit(outbuffer,outfile,1);
		if (freeint1!=0) return freeint1;
		writebyte(outbuffer,(numbytes-8));
		freeint1=writeBank(outbuffer,outfile,
		                   ((distance-(distance%256))/256));
		if (freeint1!=0) return freeint1;
		writebyte(outbuffer,(distance%256));
	}else if (numbytes>=4){
		/* Writes function 2 */
		freeint1=writebit(outbuffer,outfile,1);
		if (freeint1!=0) return freeint1;
		freeint1=writebit(outbuffer,outfile,0);
		if (freeint1!=0) return freeint1;
		if (numbytes==4){
			freeint1=writebit(outbuffer,outfile,0);
			if (freeint1!=0) return freeint1;
			freeint1=writebit(outbuffer,outfile,0);
			if (freeint1!=0) return freeint1;
		}else if (numbytes==5){
			freeint1=writebit(outbuffer,outfile,1);
			if (freeint1!=0) return freeint1;
			freeint1=writebit(outbuffer,outfile,0);
			if (freeint1!=0) return freeint1;
		}else if (numbytes==6){
			freeint1=writebit(outbuffer,outfile,0);
			if (freeint1!=0) return freeint1;
			freeint1=writebit(outbuffer,outfile,1);
			if (freeint1!=0) return freeint1;
			freeint1=writebit(outbuffer,outfile,0);
			if (freeint1!=0) return freeint1;
		}else if (numbytes==7){
			freeint1=writebit(outbuffer,outfile,0);
			if (freeint1!=0) return freeint1;
			freeint1=writebit(outbuffer,outfile,1);
			if (freeint1!=0) return freeint1;
			freeint1=writebit(outbuffer,outfile,1);
			if (freeint1!=0) return freeint1;
		}else if (numbytes==8){
			freeint1=writebit(outbuffer,outfile,1);
			if (freeint1!=0) return freeint1;
			freeint1=writebit(outbuffer,outfile,1);
			if (freeint1!=0) return freeint1;
			freeint1=writebit(outbuffer,outfile,0);
			if (freeint1!=0) return freeint1;
		}
		freeint1=writeBank(outbuffer,outfile,
		                   ((distance-(distance%256))/256));
		if (freeint1!=0) return freeint1;
		writebyte(outbuffer,(distance%256));
	}else if (numbytes==3){
		/* Writes function 5 */
		freeint1=writebit(outbuffer,outfile,1);
		if (freeint1!=0) return freeint1;
		freeint1=writebit(outbuffer,outfile,1);
		if (freeint1!=0) return freeint1;
		freeint1=writebit(outbuffer,outfile,1);
		if (freeint1!=0) return freeint1;
		freeint1=writebit(outbuffer,outfile,0);
		if (freeint1!=0) return freeint1;
		freeint1=writeBank(outbuffer,outfile,
		                   ((distance-(distance%256))/256));
		if (freeint1!=0) return freeint1;
		writebyte(outbuffer,(distance%256));
	}else if (numbytes==2){
		/* Writes function 4 */
		freeint1=writebit(outbuffer,outfile,1);
		if (freeint1!=0) return freeint1;
		freeint1=writebit(outbuffer,outfile,1);
		if (freeint1!=0) return freeint1;
		freeint1=writebit(outbuffer,outfile,0);
		if (freeint1!=0) return freeint1;
		writebyte(outbuffer,distance);
	}
	
	return 0;
}

/*
 * Function that writes a "B-value" to the output
 * Parameters:
 *   outbuffer = Output Buffer
 *   outfile = Output File
 *   value = B-value (0-15)
 * Return values:
 *   0 = Everything went well
 *   4 = Output file could not be written to
 */
static int writeBank(unsigned char *outbuffer, FILE *outfile,
                     unsigned int value){
	/* Variable Declarations */
	int freeint1; /* Integer for general use */
	
	/* Writes B-value */
	switch (value){
		case 0:
			freeint1=writebit(outbuffer,outfile,0);
			if (freeint1!=0) return freeint1;
			break;
		case 1:
			freeint1=writebit(outbuffer,outfile,1);
			if (freeint1!=0) return freeint1;
			freeint1=writebit(outbuffer,outfile,1);
			if (freeint1!=0) return freeint1;
			freeint1=writebit(outbuffer,outfile,0);
			if (freeint1!=0) return freeint1;
			break;
		case 2:
			freeint1=writebit(outbuffer,outfile,1);
			if (freeint1!=0) return freeint1;
			freeint1=writebit(outbuffer,outfile,0);
			if (freeint1!=0) return freeint1;
			freeint1=writebit(outbuffer,outfile,0);
			if (freeint1!=0) return freeint1;
			freeint1=writebit(outbuffer,outfile,0);
			if (freeint1!=0) return freeint1;
			break;
		case 3:
			freeint1=writebit(outbuffer,outfile,1);
			if (freeint1!=0) return freeint1;
			freeint1=writebit(outbuffer,outfile,0);
			if (freeint1!=0) return freeint1;
			freeint1=writebit(outbuffer,outfile,0);
			if (freeint1!=0) return freeint1;
			freeint1=writebit(outbuffer,outfile,1);
			if (freeint1!=0) return freeint1;
			break;
		case 4:
			freeint1=writebit(outbuffer,outfile,1);
			if (freeint1!=0) return freeint1;
			freeint1=writebit(outbuffer,outfile,0);
			if (freeint1!=0) return freeint1;
			freeint1=writebit(outbuffer,outfile,1);
			if (freeint1!=0) return freeint1;
			freeint1=writebit(outbuffer,outfile,0);
			if (freeint1!=0) return freeint1;
			freeint1=writebit(outbuffer,outfile,1);
			if (freeint1!=0) return freeint1;
			break;
		case 5:
			freeint1=writebit(outbuffer,outfile,1);
			if (freeint1!=0) return freeint1;
			freeint1=writebit(outbuffer,outfile,0);
			if (freeint1!=0) return freeint1;
			freeint1=writebit(outbuffer,outfile,1);
			if (freeint1!=0) return freeint1;
			freeint1=writebit(outbuffer,outfile,1);
			if (freeint1!=0) return freeint1;
			freeint1=writebit(outbuffer,outfile,1);
			if (freeint1!=0) return freeint1;
			break;
		case 6:
			freeint1=writebit(outbuffer,outfile,1);
			if (freeint1!=0) return freeint1;
			freeint1=writebit(outbuffer,outfile,1);
			if (freeint1!=0) return freeint1;
			freeint1=writebit(outbuffer,outfile,1);
			if (freeint1!=0) return freeint1;
			freeint1=writebit(outbuffer,outfile,0);
			if (freeint1!=0) return freeint1;
			freeint1=writebit(outbuffer,outfile,1);
			if (freeint1!=0) return freeint1;
			break;
		case 7:
			freeint1=writebit(outbuffer,outfile,1);
			if (freeint1!=0) return freeint1;
			freeint1=writebit(outbuffer,outfile,1);
			if (freeint1!=0) return freeint1;
			freeint1=writebit(outbuffer,outfile,1);
			if (freeint1!=0) return freeint1;
			freeint1=writebit(outbuffer,outfile,1);
			if (freeint1!=0) return freeint1;
			freeint1=writebit(outbuffer,outfile,1);
			if (freeint1!=0) return freeint1;
			break;
		case 8:
			freeint1=writebit(outbuffer,outfile,1);
			if (freeint1!=0) return freeint1;
			freeint1=writebit(outbuffer,outfile,0);
			if (freeint1!=0) return freeint1;
			freeint1=writebit(outbuffer,outfile,1);
			if (freeint1!=0) return freeint1;
			freeint1=writebit(outbuffer,outfile,0);
			if (freeint1!=0) return freeint1;
			freeint1=writebit(outbuffer,outfile,0);
			if (freeint1!=0) return freeint1;
			freeint1=writebit(outbuffer,outfile,0);
			if (freeint1!=0) return freeint1;
			break;
		case 9:
			freeint1=writebit(outbuffer,outfile,1);
			if (freeint1!=0) return freeint1;
			freeint1=writebit(outbuffer,outfile,0);
			if (freeint1!=0) return freeint1;
			freeint1=writebit(outbuffer,outfile,1);
			if (freeint1!=0) return freeint1;
			freeint1=writebit(outbuffer,outfile,0);
			if (freeint1!=0) return freeint1;
			freeint1=writebit(outbuffer,outfile,0);
			if (freeint1!=0) return freeint1;
			freeint1=writebit(outbuffer,outfile,1);
			if (freeint1!=0) return freeint1;
			break;
		case 10:
			freeint1=writebit(outbuffer,outfile,1);
			if (freeint1!=0) return freeint1;
			freeint1=writebit(outbuffer,outfile,0);
			if (freeint1!=0) return freeint1;
			freeint1=writebit(outbuffer,outfile,1);
			if (freeint1!=0) return freeint1;
			freeint1=writebit(outbuffer,outfile,1);
			if (freeint1!=0) return freeint1;
			freeint1=writebit(outbuffer,outfile,0);
			if (freeint1!=0) return freeint1;
			freeint1=writebit(outbuffer,outfile,0);
			if (freeint1!=0) return freeint1;
			break;
		case 11:
			freeint1=writebit(outbuffer,outfile,1);
			if (freeint1!=0) return freeint1;
			freeint1=writebit(outbuffer,outfile,0);
			if (freeint1!=0) return freeint1;
			freeint1=writebit(outbuffer,outfile,1);
			if (freeint1!=0) return freeint1;
			freeint1=writebit(outbuffer,outfile,1);
			if (freeint1!=0) return freeint1;
			freeint1=writebit(outbuffer,outfile,0);
			if (freeint1!=0) return freeint1;
			freeint1=writebit(outbuffer,outfile,1);
			if (freeint1!=0) return freeint1;
			break;
		case 12:
			freeint1=writebit(outbuffer,outfile,1);
			if (freeint1!=0) return freeint1;
			freeint1=writebit(outbuffer,outfile,1);
			if (freeint1!=0) return freeint1;
			freeint1=writebit(outbuffer,outfile,1);
			if (freeint1!=0) return freeint1;
			freeint1=writebit(outbuffer,outfile,0);
			if (freeint1!=0) return freeint1;
			freeint1=writebit(outbuffer,outfile,0);
			if (freeint1!=0) return freeint1;
			freeint1=writebit(outbuffer,outfile,0);
			if (freeint1!=0) return freeint1;
			break;
		case 13:
			freeint1=writebit(outbuffer,outfile,1);
			if (freeint1!=0) return freeint1;
			freeint1=writebit(outbuffer,outfile,1);
			if (freeint1!=0) return freeint1;
			freeint1=writebit(outbuffer,outfile,1);
			if (freeint1!=0) return freeint1;
			freeint1=writebit(outbuffer,outfile,0);
			if (freeint1!=0) return freeint1;
			freeint1=writebit(outbuffer,outfile,0);
			if (freeint1!=0) return freeint1;
			freeint1=writebit(outbuffer,outfile,1);
			if (freeint1!=0) return freeint1;
			break;
		case 14:
			freeint1=writebit(outbuffer,outfile,1);
			if (freeint1!=0) return freeint1;
			freeint1=writebit(outbuffer,outfile,1);
			if (freeint1!=0) return freeint1;
			freeint1=writebit(outbuffer,outfile,1);
			if (freeint1!=0) return freeint1;
			freeint1=writebit(outbuffer,outfile,1);
			if (freeint1!=0) return freeint1;
			freeint1=writebit(outbuffer,outfile,0);
			if (freeint1!=0) return freeint1;
			freeint1=writebit(outbuffer,outfile,0);
			if (freeint1!=0) return freeint1;
			break;
		case 15:
			freeint1=writebit(outbuffer,outfile,1);
			if (freeint1!=0) return freeint1;
			freeint1=writebit(outbuffer,outfile,1);
			if (freeint1!=0) return freeint1;
			freeint1=writebit(outbuffer,outfile,1);
			if (freeint1!=0) return freeint1;
			freeint1=writebit(outbuffer,outfile,1);
			if (freeint1!=0) return freeint1;
			freeint1=writebit(outbuffer,outfile,0);
			if (freeint1!=0) return freeint1;
			freeint1=writebit(outbuffer,outfile,1);
			if (freeint1!=0) return freeint1;
			break;
	}
	
	/* Returns after succesful completion */
	return 0;
}

/*
 * Function that refreshes the input buffer
 * Parameters:
 *   infile = Input file
 *   inbuffer = Input buffer
 * Return values:
 *   0 = Buffer refreshed
 *   2 = File could not be read from
 */
static int refreshInBuffer(FILE *infile, unsigned char *inbuffer){
	/* Variable Declarations */
	unsigned long int bytestoread; /* Number of bytes to read */
	size_t occupiedpart; /* Occupied part of inbuffer */
	size_t freesizet1; /* size_t for general use */
	
	/* Calculates occupiedpart */
	occupiedpart=((inbufsize-curpos)+4097);
	
	/* Moves the unused part of inbuffer to the beginning */
	memmove(inbuffer,inbuffer+inbufsize-occupiedpart,occupiedpart);
	curpos=4097;
	
	/* Determines bytestoread */
	if (((unsigned long int)(infilesize-ftell(infile)))>=
	    ((unsigned long int)(inbufsize-occupiedpart))){
		bytestoread=(unsigned long int) (inbufsize-occupiedpart);
	}else{
		bytestoread=((unsigned long int)(infilesize-ftell(infile)));
		inbufedge=bytestoread+occupiedpart;
		eofreached=1;
	}
	
	/* Reads bytes from file into inbuffer */
	freesizet1=fread(inbuffer+occupiedpart,1,
	                 (size_t)bytestoread,infile);
	if (freesizet1!=(size_t)bytestoread) return 2;
	
	/* Adjusts posnewbytes */
	posnewbytes-=(inbufsize-occupiedpart);
	
	/* Declares the inbuffer has been refreshed */
	inbufreft=1;
	
	return 0;
}
