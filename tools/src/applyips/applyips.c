/*
 * ApplyIPS 3.0
 * (A Tool that applies an IPS-patch to a file)
 * --
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
#include <unistd.h>

/* Macro Definitions */
#ifndef INT16
#define INT16 short int
#endif
#ifndef INT32
#define INT32 long int
#endif
#ifndef INTU16
#define INTU16 unsigned INT16
#endif
#ifndef INTU32
#define INTU32 unsigned INT32
#endif

/* Function Declarations */
int main(int argc, char *argv[]);
static int applyips(char *filepath, char *ipspath);
static void errorMessage(int errornumber, char *argv[]);

/*
 * Main Function of the program
 * Parameters:
 *   argc = Number of command-line parameters
 *   argv = Command-line parameters
 */
int main(int argc, char *argv[]){
	/* Variable Declarations */
	int retval=0; /* Return value for this program */
	
	/* Checks number of command-line parameters */
	if (argc<3) retval=-1;
	
	/* Applies the IPS patch */
	else retval=applyips(argv[1], argv[2]);
	
	/* Prints error messages */
	if (retval!=0) errorMessage(retval, argv);
	
	/* Returns the program */
	return retval;
}

/*
 * Applies an IPS-patch
 * Parameters:
 *   filepath = Path to the file to be patched
 *   ipspath = Path to the patch file
 * Return values:
 *   0 = Everything went well
 *   1 = File could not be opened
 *   2 = Patch file could not be opened
 *   3 = File could not be read from
 *   4 = Patch file could not be read from
 *   5 = Buffer could not be allocated
 *   6 = IPS-file is not valid
 *   7 = File could not be written to
 */
static int applyips(char *filepath, char *ipspath){
	/* Variable Declarations */
	FILE *ips=NULL; /* Patch file */
	FILE *file=NULL; /* File to be patched */
	char *buffer=NULL; /* Buffer */
	INTU16 buffersize; /* Size of the buffer */
	INTU32 ipspos; /* Position in the IPS patch-file */
	INTU32 ipssize; /* Size of the IPS patch-file */
	INTU32 filesize; /* Size of the file to be patched */
	INTU32 offset; /* Offset of the change */
	INTU16 chunksize; /* Size of the current chunk in the buffer */
	INTU16 size; /* Size of the change */
	int rlechange; /* Whether the change is RLE-compressed (0=No, 1=Yes) */
	int rlevalue; /* Value of the RLE-compressed change */
	size_t freesizet1; /* size_t for general use */
	int freeint1; /* Integer for general use */
	int retval=0; /* Return value for this function */
	
	/* Opens file to be patched */
	file=fopen(filepath,"r+b");
	if (file==NULL){
		retval=1;
		goto end;
	}
	
	/* Opens patch file */
	ips=fopen(ipspath,"rb");
	if (ips==NULL){
		retval=2;
		goto end;
	}
	
	/* Determines filesize */
	freeint1=fseek(file,0,SEEK_END);
	if (freeint1!=0){
		retval=3;
		goto end;
	}
	filesize=(INTU32) ftell(file);
	
	/* Determines ipssize */
	freeint1=fseek(ips,0,SEEK_END);
	if (freeint1!=0){
		retval=4;
		goto end;
	}
	ipssize=(INTU32) ftell(ips);
	
	/* Checks IPS header */
	if (ipssize<=8){
		retval=6;
		goto end;
	}
	freeint1=fseek(ips,0,SEEK_SET);
	if (freeint1!=0){
		retval=4;
		goto end;
	}
	freeint1=fgetc(ips);
	if (freeint1!=80){
		retval=6;
		goto end;
	}
	freeint1=fgetc(ips);
	if (freeint1!=65){
		retval=6;
		goto end;
	}
	freeint1=fgetc(ips);
	if (freeint1!=84){
		retval=6;
		goto end;
	}
	freeint1=fgetc(ips);
	if (freeint1!=67){
		retval=6;
		goto end;
	}
	freeint1=fgetc(ips);
	if (freeint1!=72){
		retval=6;
		goto end;
	}
	
	/* Allocates buffer */
	buffersize=0x7FFF;
	do{
		buffer=malloc((size_t) buffersize);
		if ((buffer==NULL)&&(buffersize==1)){
			retval=5;
			goto end;
		}
		if (buffer==NULL) buffersize>>=1;
	}while (buffer==NULL);
	
	/* Processes IPS patch */
	ipspos=5;
	while (ipspos<ipssize){
		size=0;
		offset=0;
		rlechange=0;
		
		/* Reads offset */
		freesizet1=fread(&offset,1,3,ips);
		if (freesizet1!=3){
			retval=4;
			goto end;
		}
		ipspos+=3;
		/* Converts offset to correct format */
		offset=(((offset&0x00FF0000)>>16)|(offset&0x0000FF00)|
		        ((offset&0x000000FF)<<16));
		
		/* Checks for EOF flag */
		if (offset==0x00454F46) break;
		
		/* Reads size */
		freesizet1=fread(&size,1,2,ips);
		if (freesizet1!=2){
			retval=4;
			goto end;
		}
		ipspos+=2;
		/* Checks whether the change is RLE-compressed */
		if (size==0){
			rlechange=1;
			freesizet1=fread(&size,1,2,ips);
			if (freesizet1!=2){
				retval=4;
				goto end;
			}
			ipspos+=2;
		}
		/* Converts size to correct format */
		size=(INTU16) ((((INTU16)(size&0xFF00))>>8)|
		               (((INTU16)(size&0x00FF))<<8));
		
		/* Moves to offset */
		if (offset>filesize){
			freeint1=fseek(file,0,SEEK_END);
			if (freeint1!=0){
				retval=3;
				goto end;
			}
			for (;filesize<offset;++filesize){
				freeint1=fputc(0,file);
				if (freeint1==EOF){
					retval=7;
					goto end;
				}
			}
		}else{
			freeint1=fseek(file,(long int) offset,SEEK_SET);
			if (freeint1!=0){
				retval=3;
				goto end;
			}
		}
		
		/* Recalculates filesize */
		if (filesize<(offset+size)) filesize=(offset+size);
		
		/* Applies the change */
		if (rlechange==0){
			/* Non-RLE-compressed */
			do{
				/* Determines the size of current chunk */
				if (buffersize<size) chunksize=buffersize;
				else chunksize=size;
				
				/* Reads the chunk into buffer */
				freesizet1=fread(buffer,1,(size_t) chunksize,ips);
				if (freesizet1!=(size_t)chunksize){
					retval=4;
					goto end;
				}
				
				/* Writes the chunk to file */
				freesizet1=fwrite(buffer,1,(size_t) chunksize,file);
				if (freesizet1!=(size_t)chunksize){
					retval=7;
					goto end;
				}
				
				/* Subtracts chunksize from total size */
				ipspos+=chunksize;
				size-=chunksize;
			}while (size>0);
		}else{
			/* RLE-compressed */
			/* Reads RLE value */
			rlevalue=fgetc(ips);
			if (rlevalue==EOF){
				retval=4;
				goto end;
			}
			++ipspos;
			/* Writes RLE value to file */
			for (;size>0;--size){
				freeint1=fputc(rlevalue,file);
				if (freeint1==EOF){
					retval=7;
					goto end;
				}
			}
		}
	}
	
	/* Checks whether the file is to be truncated */
	if (ipspos!=ipssize){
		offset=0;
		/* Reads offset */
		freesizet1=fread(&offset,1,3,ips);
		if (freesizet1!=3){
			retval=4;
			goto end;
		}
		/* Converts offset to correct format */
		offset=(((offset&0x00FF0000)>>16)|(offset&0x0000FF00)|
		        ((offset&0x000000FF)<<16));
		/* Truncates the file */
		freeint1=fclose(file);
		if (freeint1!=0) retval=3;
		file=NULL;
		freeint1=truncate(filepath,(off_t)offset);
		if (freeint1!=0){
			retval=7;
			goto end;
		}
	}
	
end:/* Frees buffer, closes files, and returns the function */
	free(buffer);
	if (file!=NULL){
		freeint1=fclose(file);
		if (freeint1!=0) retval=3;
	}
	if (ips!=NULL){
		freeint1=fclose(ips);
		if (freeint1!=0) retval=4;
	}
	return retval;
}

/*
 * Prints error message.
 * Parameters:
 *   errornumber = Number of the error
 */
static void errorMessage(int errornumber, char *argv[]){
	switch (errornumber){
		case -1:
			printf("applyips: Insufficient parameters\n");
			break;
		case 1:
			printf("applyips: %s could not be opened\n",argv[1]);
			break;
		case 2:
			printf("applyips: %s could not be opened\n",argv[2]);
			break;
		case 3:
			printf("applyips: %s could not be read from\n",argv[1]);
			break;
		case 4:
			printf("applyips: %s could not be read from\n",argv[2]);
			break;
		case 5:
			printf("applyips: Buffer could not be allocated\n");
			break;
		case 6:
			printf("applyips: %s is not a valid IPS-file\n",argv[2]);
			break;
		case 7:
			printf("applyips: %s could not be written to\n",argv[1]);
			break;
	}
}
