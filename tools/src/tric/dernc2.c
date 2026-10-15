/*
 * Library for Decompressing RNC2-Compressed Files (for TRIC)
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
#include "dernc2.h"

/* Defintion of size of outbuffer */
#define OUTBUFSIZE 4097

/* Static Variable Declarations */
static signed int curbit; /* Current bit */
static long int infilesize; /* Size of infile */
static unsigned int outbufpos; /* Current position in outbuffer */
static size_t inbufsize; /* Size of inbuffer */
static unsigned long int curbitbyte; /* Byte that holds current bit */
static unsigned long int nextbyte; /* Next unread byte in inbuffer */

/* Static Function Declarations */
static int getbit(unsigned char *inbuffer, FILE *infile);
static int getbyte(unsigned char *inbuffer, FILE *infile);
static int getB(unsigned char *inbuffer, FILE *infile);
static int writebytes(unsigned char *inbuffer, FILE *infile, 
                      unsigned char *outbuffer, FILE *outfile, 
                      int amount);
static int copybytes(unsigned char *outbuffer, FILE *outfile,
                     int amount, int distance);
static int refreshInBuffer(unsigned char *inbuffer, FILE *infile);

/*
 * Function that decompresses the file
 * Parameters:
 *   infile = Compressed input file
 *   outfile = Uncompressed output file
 *   firstbyte = First byte of the compressed data
 * Return values:
 *   0 = Everything went well
 *   1 = Input file could not be opened
 *   2 = Output file could not be opened
 *   3 = Input file could not be read from
 *   4 = Writing to outfile failed
 *   5 = Failed to allocate buffer
 *   6 = End of infile reached
 *   7 = Command somehow invalid
 *   8 = Amount > 263
 *   9 = Distance > 4096
 */
int dernc2(FILE *infile, FILE *outfile, int firstbyte){
	/* Variable Declarations */
	unsigned char *inbuffer=NULL; /* Buffer for input file */
	unsigned char *outbuffer=NULL; /* Buffer for output file */
	int freeint1, freeint2, freeint3; /* Integer for general use */
	size_t freesizet1; /* size_t for general use */
	int retval=0; /* Return value for this function */
	
	/* Determines infilesize */
	freeint1=fseek(infile,0,SEEK_END);
	if (freeint1!=0){
		retval=3;
		goto end;
	}
	infilesize=ftell(infile);
	
	/* Allocates outbuffer */
	outbuffer=malloc(OUTBUFSIZE);
	if (outbuffer==NULL){
		retval=5;
		goto end;
	}
	memset(outbuffer,0,OUTBUFSIZE);
	
	/* Allocates inbuffer */
	memset(&inbufsize,0xFF,sizeof(inbufsize));
	if (((unsigned long int)(infilesize-firstbyte))<
	    ((unsigned long int)inbufsize)){
		inbufsize=(size_t) (infilesize-firstbyte);
		inbuffer=malloc(inbufsize);
	}
	if (inbuffer==NULL) memset(&inbufsize,0xFF,sizeof(inbufsize));
	while (inbuffer==NULL){
		inbuffer=malloc(inbufsize);
		if (inbuffer==NULL){
			if (inbufsize==127){
				retval=5;
				goto end;
			}
			inbufsize>>=1;
		}
	}
	
	/* Moves to firstbyte */
	freeint1=fseek(infile,(long int) firstbyte,SEEK_SET);
	if (freeint1!=0){
		retval=3;
		goto end;
	}
	
	/* Fills inbuffer */
	freesizet1=fread(inbuffer,1,inbufsize,infile);
	if (freesizet1!=inbufsize){
		retval=3;
		goto end;
	}
	
	/* Decompresses the data */
	curbit=5;
	nextbyte=1;
	outbufpos=0;
	curbitbyte=0;
	while (0==0){
		freeint1=getbit(inbuffer,infile);
		if (freeint1==0){
			freeint2=writebytes(inbuffer,infile,outbuffer,outfile,1);
			if (freeint2!=0){
				retval=freeint2;
				goto end;
			}
		}else if (freeint1==1){
			freeint1=getbit(inbuffer,infile);
			if (freeint1==0){
				freeint1=getbit(inbuffer,infile);
				if (freeint1==0){
					freeint1=getbit(inbuffer,infile);
					if (freeint1==0){
						freeint1=getB(inbuffer,infile);
						if (freeint1>=16){
							retval=freeint1-15;
							goto end;
						}
						freeint2=getbyte(inbuffer, infile);
						if (freeint2>256){
							retval=(freeint2-260);
							goto end;
						}
						freeint1=copybytes(outbuffer,outfile,4,
								((freeint1*256)+freeint2+1));
					}else if (freeint1==1){
						freeint1=getbit(inbuffer,infile);
						if (freeint1==0){
							freeint1=getB(inbuffer,infile);
							if (freeint1>=16){
								retval=freeint1-15;
								goto end;
							}
							freeint2=getbyte(inbuffer, infile);
							if (freeint2>256){
								retval=(freeint2-260);
								goto end;
							}
							freeint1=copybytes(outbuffer,outfile,6,
									((freeint1*256)+freeint2+1));
						}else if (freeint1==1){
							freeint1=getB(inbuffer,infile);
							if (freeint1>=16){
								retval=freeint1-15;
								goto end;
							}
							freeint2=getbyte(inbuffer, infile);
							if (freeint2>256){
								retval=(freeint2-260);
								goto end;
							}
							freeint1=copybytes(outbuffer,outfile,7,
								((freeint1*256)+freeint2+1));
						}else{
							retval=freeint1;
							goto end;
						}
					}else{
						retval=freeint1;
						goto end;
					}
				}else if (freeint1==1){
					freeint1=getbit(inbuffer,infile);
					if (freeint1==0){
						freeint1=getB(inbuffer,infile);
						if (freeint1>=16){
							retval=freeint1-15;
							goto end;
						}
						freeint2=getbyte(inbuffer, infile);
						if (freeint2>256){
							retval=(freeint2-260);
							goto end;
						}
						freeint1=copybytes(outbuffer,outfile,5,
								((freeint1*256)+freeint2+1));
					}else if (freeint1==1){
						freeint1=getbit(inbuffer,infile);
						if (freeint1==0){
							freeint1=getB(inbuffer,infile);
							if (freeint1>=16){
								retval=freeint1-15;
								goto end;
							}
							freeint2=getbyte(inbuffer, infile);
							if (freeint2>256){
								retval=(freeint2-260);
								goto end;
							}
							freeint1=copybytes(outbuffer,outfile,8,
									((freeint1*256)+freeint2+1));
						}else if (freeint1==1){
							freeint1=getbit(inbuffer,infile);
							if (freeint1>=2){
								retval=freeint1;
								goto end;
							}
							freeint2=getbit(inbuffer,infile);
							if (freeint2>=2){
								retval=freeint2;
								goto end;
							}
							freeint1*=2;
							freeint1|=freeint2;
							freeint2=getbit(inbuffer,infile);
							if (freeint2>=2){
								retval=freeint2;
								goto end;
							}
							freeint1*=2;
							freeint1|=freeint2;
							freeint2=getbit(inbuffer,infile);
							if (freeint2>=2){
								retval=freeint2;
								goto end;
							}
							freeint1*=2;
							freeint1|=freeint2;
							freeint1+=3;
							freeint1*=4;
							freeint1=writebytes(inbuffer,infile,
								outbuffer,outfile,freeint1);
							if (freeint1!=0){
								retval=freeint1;
								goto end;
							}
						}else{
							retval=freeint1;
							goto end;
						}
					}else{
						retval=freeint1;
						goto end;
					}
				}else{
					retval=freeint1;
					goto end;
				}
			}else if (freeint1==1){
				freeint1=getbit(inbuffer,infile);
				if (freeint1==0){
					freeint1=getbyte(inbuffer, infile);
					if (freeint1>256){
						retval=(freeint1-260);
						goto end;
					}
					freeint1=copybytes(outbuffer,outfile,2,
									freeint1+1);
				}else if (freeint1==1){
					freeint1=getbit(inbuffer,infile);
					if (freeint1==0){
						freeint1=getB(inbuffer,infile);
						if (freeint1>=16){
							retval=freeint1-15;
							goto end;
						}
						freeint2=getbyte(inbuffer, infile);
						if (freeint2>256){
							retval=(freeint2-260);
							goto end;
						}
						freeint1=copybytes(outbuffer,outfile,3,
								((freeint1*256)+freeint2+1));
					}else if (freeint1==1){
						freeint3=getbyte(inbuffer,infile);
						if (freeint3>=256){
							retval=freeint3-260;
							goto end;
						}
						if (freeint3==0){
							freeint1=getbit(inbuffer,infile);
							if (freeint1==0){
								break;
							}else if (freeint1==1){
								continue;
							}else{
								retval=freeint1;
								goto end;
							}
						}else{
							freeint1=getB(inbuffer,infile);
							if (freeint1>=16){
								retval=freeint1-15;
								goto end;
							}
							freeint2=getbyte(inbuffer, infile);
							if (freeint2>256){
								retval=(freeint2-260);
								goto end;
							}
							freeint1=copybytes(outbuffer,outfile,
								freeint3+8,((freeint1*256)+freeint2+1));
						}
					}else{
						retval=freeint1;
						goto end;
					}
				}else{
					retval=freeint1;
					goto end;
				}
			}else{
				retval=freeint1;
				goto end;
			}
		}else{
			retval=freeint1;
			goto end;
		}
	}
	/* Writes outbuffer to file before overwriting */
	freesizet1=fwrite(outbuffer,1,(size_t)outbufpos,outfile);
	if (freesizet1!=(size_t)outbufpos) retval=4;
end:/* Closes files, frees buffers and returns the function */
	if (outbuffer!=NULL){
		free(outbuffer);
	}
	if (inbuffer!=NULL){
		free(inbuffer);
	}
	return retval;
}

/*
 * Function that reads the next bit and returns its value
 * Parameters:
 *   inbuffer = Buffer for input data
 *   infile = Input File
 * Return values:
 *   0 = 0
 *   1 = 1
 *   3 = Reading from infile failed
 *   6 = End of infile reached
 */
static int getbit(unsigned char *inbuffer, FILE *infile){
	/* Variable Declarations */
	int freeint1;
	
	/* Checks for the end of current bit */
	if (curbit==-1){
		/* Checks for the end of inbuffer */
		if (nextbyte==(unsigned long int)inbufsize){
			/* Refreshes inbuffer */
			freeint1=refreshInBuffer(inbuffer,infile);
			if (freeint1!=0) return freeint1;
		}
		curbitbyte=nextbyte;
		nextbyte++;
		curbit=7;
	}
	
	/* Calculates value */
	freeint1=(int) ((inbuffer[curbitbyte]&(1<<(unsigned int)curbit))>>
	                (unsigned int)curbit);
	
	/* Iterates curbit */
	--curbit;
	
	/* Returns value*/
	return freeint1;
}

/*
 * Function that reads the next byte and returns its value
 * Parameters:
 *   inbuffer = Buffer for input data
 *   infile = Input File
 * Return values:
 *   0-255 = Value of the byte
 *   263 = Reading from infile failed
 *   266 = End of infile reached
 */
static int getbyte(unsigned char *inbuffer, FILE *infile){
	/* Variable Declarations */
	int freeint1;
	
	/* Checks for the end of inbuffer */
	if (nextbyte==(unsigned long int)inbufsize){
		/* Refreshes inbuffer */
		freeint1=refreshInBuffer(inbuffer,infile);
		if (freeint1!=0) return (freeint1+260);
	}
	
	/* Reads next byte into freeint1 */
	freeint1=0;
	freeint1=(int) inbuffer[nextbyte];
	++nextbyte;
	
	/* Returns value*/
	return freeint1;
}

/*
 * Function that determines the B-value for functions 2, 5 and 8
 * Parameters:
 *   inbuffer = Buffer for input data
 *   infile = Input file
 * Return values:
 *   0-15 = Value of B
 *   18 = End of infile reached
 *   21 = Reading from infile failed
 */
static int getB(unsigned char *inbuffer, FILE *infile){
	int freeint1; /* Integer for general use */
	
	freeint1=getbit(inbuffer,infile);
	if (freeint1==0){
		return 0;
	}else if (freeint1==1){
		freeint1=getbit(inbuffer,infile);
		if (freeint1==0){
			freeint1=getbit(inbuffer,infile);
			if (freeint1==0){
				freeint1=getbit(inbuffer,infile);
				if (freeint1==0){
					return 2;
				}else if (freeint1==1){
					return 3;
				}else{
					return freeint1+15;
				}
			}else if (freeint1==1){
				freeint1=getbit(inbuffer,infile);
				if (freeint1==0){
					freeint1=getbit(inbuffer,infile);
					if (freeint1==0){
						freeint1=getbit(inbuffer,infile);
						if (freeint1==0){
							return 8;
						}else if (freeint1==1){
							return 9;
						}else{
							return freeint1+15;
						}
					}else if (freeint1==1){
						return 4;
					}else{
						return freeint1+15;
					}
				}else if (freeint1==1){
					freeint1=getbit(inbuffer,infile);
					if (freeint1==0){
						freeint1=getbit(inbuffer,infile);
						if (freeint1==0){
							return 10;
						}else if (freeint1==1){
							return 11;
						}else{
							return freeint1+15;
						}
					}else if (freeint1==1){
						return 5;
					}else{
						return freeint1+15;
					}
				}else{
					return freeint1+15;
				}
			}else{
				return freeint1+15;
			}
		}else if (freeint1==1){
			freeint1=getbit(inbuffer,infile);
			if (freeint1==0){
				return 1;
			}else if (freeint1==1){
				freeint1=getbit(inbuffer,infile);
				if (freeint1==0){
					freeint1=getbit(inbuffer,infile);
					if (freeint1==0){
						freeint1=getbit(inbuffer,infile);
					if (freeint1==0){
							return 12;
						}else if (freeint1==1){
							return 13;
						}else{
							return freeint1+15;
						}
					}else if (freeint1==1){
						return 6;
					}else{
						return freeint1+15;
					}
				}else if (freeint1==1){
					freeint1=getbit(inbuffer,infile);
					if (freeint1==0){
						freeint1=getbit(inbuffer,infile);
						if (freeint1==0){
							return 14;
						}else if (freeint1==1){
							return 15;
						}else{
							return freeint1+15;
						}
					}else if (freeint1==1){
						return 7;
					}else{
						return freeint1+15;
					}
				}else{
					return freeint1+15;
				}
			}else{
				return freeint1+15;
			}
		}else{
			return freeint1+15;
		}
	}else{
		return freeint1+15;
	}
}

/* 
 * Function that writes a number of bytes to the output
 * Paramters:
 *   inbuffer = Buffer for input data
 *   infile = Input file
 *   outbuffer = Buffer for output data
 *   outfile = Output file
 *   amount = Number of bytes to be written out
 * Return values:
 *   0 = Everything went well
 *   3 = Reading from infile failed
 *   4 = Writing to outfile failed
 *   6 = End of infile reached
 *   7 = Command somehow invalid
 */
static int writebytes(unsigned char *inbuffer, FILE *infile, 
                      unsigned char *outbuffer, FILE *outfile, 
                      int amount){
	/* Variable Declaraions */
	int freeint1;
	size_t freesizet1;
	
	/* Checks whether amount of bytes is valid */
	if (amount>72) return 7;
	
	/* Checks whether all bytes are in inbuffer */
	if (((size_t)amount)>(inbufsize-nextbyte)){
		/* Refreshes inbuffer if needed */
		freeint1=refreshInBuffer(inbuffer,infile);
		if (freeint1!=0) return freeint1;
	}
	
	/* 
	 * Checks whether the write operation will go outside outbuffer.
	 * If it does, splits the write operation into two write operations:
	 * the first is to the end of outbuffer, and the second is however
	 * many bytes are left to be written after that. Between these two 
	 * it will write the contents of outbuffer to outfile.
	 */
	if (((unsigned int)amount)>=(OUTBUFSIZE-outbufpos)){
		freeint1=(int)(OUTBUFSIZE-outbufpos);
		amount-=freeint1;
		
		/* Writes bytes until outbuffer is full */
		memmove(outbuffer+outbufpos,inbuffer+nextbyte,(size_t)freeint1);
		nextbyte+=freeint1;
		
		/* Writes outbuffer to file before overwriting */
		freesizet1=fwrite(outbuffer,1,OUTBUFSIZE,outfile);
		if (freesizet1!=OUTBUFSIZE) return 4;
		
		/* Resets outbufpos */
		outbufpos=0;
	}
	
	/* Writes bytes */
	memmove(outbuffer+outbufpos,inbuffer+nextbyte,(size_t)amount);
	outbufpos+=amount;
	nextbyte+=amount;
	
	/* Returns the function */
	return 0;
}

/*
 * Function that copies a number of bytes from a certain distance back
 * Parameters:
 *   inbuffer = Buffer for input data
 *   infile = Input file
 *   outbuffer = Buffer for output data
 *   outfile = Output file
 *   amount = Number of bytes to be written out
 *   distance = Number of bytes to back to the source
 * Return values:
 *   0 = Everything went well
 *   4 = Writing to outfile failed
 *   8 = Amount > 263
 *   9 = Distance > 4096
 */
static int copybytes(unsigned char *outbuffer, FILE *outfile,
                     int amount, int distance){
	/* Variable Declaraions */
	int freeint1, freeint2;
	size_t freesizet1;
	
	/* Checks whether amount of bytes is valid */
	if (amount>263) return 8;
	if (distance>4096) return 9;
	
	/*
	 * Checks whether the copy operation will go outside outbuffer.
	 * If it does, splits the copy operation into two copy operations:
	 * the first is to the end of outbuffer, and the second is however
	 * many bytes are left to be copied after that. Between these two it
	 * will write the contents of outbuffer to outfile.
	 */
	if (((unsigned int)amount)>=(OUTBUFSIZE-outbufpos)){
		freeint1=(int)(OUTBUFSIZE-outbufpos);
		amount-=freeint1;
		
		/* Copies bytes until outbuffer is full */
		freeint2=(((int)outbufpos>=distance)?((int)outbufpos-distance):
		          (OUTBUFSIZE-(distance-outbufpos)));
		for (;freeint1>0;--freeint1){
			outbuffer[outbufpos]=outbuffer[freeint2];
			++outbufpos;
			++freeint2;
			if (freeint2==OUTBUFSIZE) freeint2=0;
		}
		
		/* Writes outbuffer to file before overwriting */
		freesizet1=fwrite(outbuffer,1,OUTBUFSIZE,outfile);
		if (freesizet1!=OUTBUFSIZE) return 4;
		
		/* Resets outbufpos */
		outbufpos=0;
	}
	
	/* Writes bytes */
	freeint2=(((int)outbufpos>=distance)?((int)outbufpos-distance):
	          (OUTBUFSIZE-(distance-outbufpos)));
	for (;amount>0;--amount){
		outbuffer[outbufpos]=outbuffer[freeint2];
		++outbufpos;
		++freeint2;
		if (freeint2==OUTBUFSIZE) freeint2=0;
	}
	
	/* Returns the function */
	return 0;
}

/*
 * Function that refreshes the inbuffer
 * Parameters:
 *   inbuffer = Buffer for input data
 *   infile = Input file
 * Return values:
 *   0 = Everything went well
 *   3 = Reading from infile failed
 *   6 = End of infile reached
 */
static int refreshInBuffer(unsigned char *inbuffer, FILE *infile){
	/* Variable Declarations */
	unsigned long int freeuint1, freeuint2; /* for general use */
	size_t freesizet1; /* size_t for general use */
	
	/* Checks whether the end of infile has already been reached */
	if (ftell(infile)==infilesize) return 6;
	
	/* Moves the unread parts of inbuffer to the beginning */
	memmove(inbuffer,inbuffer+curbitbyte,1);
	memmove(inbuffer+1,inbuffer+nextbyte,(size_t)(inbufsize-nextbyte));
	freeuint1=(unsigned long int)((inbufsize-nextbyte)+1);
	
	/* Determines how many bytes to read */
	freeuint2=(unsigned long int) (infilesize-ftell(infile));
	if (freeuint2>((unsigned long int)(inbufsize-freeuint1))) 
		freeuint2=((unsigned long int)(inbufsize-freeuint1));
	
	/* Fills inbuffer */
	freesizet1=fread(inbuffer+freeuint1,1,(size_t)freeuint2,infile);
	if (freesizet1!=(size_t) freeuint2) return 3;
	
	/* Resets curbitbyte and nextbyte */
	curbitbyte=0;
	nextbyte=1;
	
	return 0;
}
