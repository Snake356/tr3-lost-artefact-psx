/*
 * Library for Converting between OBJ (Tomb Raider on PSX) and BMP
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
#include <sys/stat.h>
#include "tricints.h"
#include "imgtools.h"
#include "strtools.h"
#include "obj.h"
#ifdef _WIN32
#include <direct.h>
#define mkdir(path, mode) _mkdir(path)
#endif

/*
 * Function that extracts the images from a PS1 OBJ-file.
 * Parameters:
 *   inFilePath = Path to input OBJ-File
 *   outDirPath = Path to output directory
 *   type = Type of OBJ-file (4=TR4, 5=TR5)
 * Return values:
 *   0 = Everything went well
 *   1 = Input file could not be opened
 *   2 = Input file could not be read from
 *   3 = Output file could not be opened
 *   4 = Output file could not be written to
 *   5 = Buffer could not be allocated
 * 	29 = Type is invalid
 * 	30 = Input OBJ-file is invalid
 */
int extractobj(char *inFilePath, char *outDirPath, int type){
	/* Variable Declarations */
	FILE *obj=NULL; /* Input OBJ-File */
	FILE *bmp=NULL; /* Output BMP-File */
	FILE *outList=NULL; /* Output list file */
	char *outFilePath=NULL; /* Path to current output file */
	char *buffer=NULL; /* Buffer for image data */
	unsigned INT32 buffersize; /* Length of the buffer in bytes */
	unsigned INT16 colour; /* Current colour */
	INT32 bufferpos; /* Current position in buffer */
	INT32 chunksize; /* Length of a chunk */
	INT32 objSize; /* Size of input OBJ-File in bytes */
	INT32 curpos; /* Current position in the input file */
	INT32 endpos; /* Ending of the image offset index */
	INT32 freelong1, freelong2; /* For general use */
	int freeint1; /* Integer for general use */
	size_t freesizet1; /* size_t for general use */
	int retval=0; /* Return value of this function */
	
	/* Opens input OBJ-File */
	obj=fopen(inFilePath,"rb");
	if (obj==NULL){
		retval=1;
		goto end;
	}
	
	/* Determines objSize */
	freeint1=fseek(obj,0,SEEK_END);
	if (freeint1!=0){
		retval=2;
		goto end;
	}
	objSize=ftell(obj);
	
	/* Checks whether input obj contains image index */
	if (objSize<392){
		retval=30;
		goto end;
	}
	
	/* Allocates outFilePath */
	outFilePath=malloc((size_t)(strlen(outDirPath)+14));
	if (outFilePath==NULL){
		retval=5;
		goto end;
	}
	
	/* Makes output directory */
	(void) mkdir(outDirPath,0755);
	
	/* Opens output list */
	memcpy(outFilePath,outDirPath,strlen(outDirPath));
	memset(outFilePath+strlen(outDirPath),(int)'/',1);
	memset(outFilePath+strlen(outDirPath)+1,(int)'L',1);
	memset(outFilePath+strlen(outDirPath)+2,(int)'i',1);
	memset(outFilePath+strlen(outDirPath)+3,(int)'s',1);
	memset(outFilePath+strlen(outDirPath)+4,(int)'t',1);
	memset(outFilePath+strlen(outDirPath)+5,(int)'.',1);
	memset(outFilePath+strlen(outDirPath)+6,(int)'t',1);
	memset(outFilePath+strlen(outDirPath)+7,(int)'x',1);
	memset(outFilePath+strlen(outDirPath)+8,(int)'t',1);
	memset(outFilePath+strlen(outDirPath)+9,0,1);
	outList=fopen(outFilePath,"w");
	if (outList==NULL){
		retval=3;
		goto end;
	}
	
	/* Adjusts for type */
	if (type==4){
		curpos=328;
		endpos=392;
	} else if (type==5){
		curpos=16;
		endpos=392;
	}else{
		retval=29;
		goto end;
	}
	
	/* Allocates buffer */
	buffer=NULL;
	buffersize=0x3FFFFF;
	do{
		buffer=malloc((size_t) buffersize);
		if ((buffersize<=1024)&&(buffer==NULL)){
			retval=5;
			goto end;
		}
		if (buffer==NULL) buffersize>>=1;
	}while (buffer==NULL);
	buffersize-=(buffersize%1024);
	
	/* Extracts images */
	memset(outFilePath+strlen(outDirPath)+1,(int)'0',8);
	memset(outFilePath+strlen(outDirPath)+9,(int)'.',1);
	memset(outFilePath+strlen(outDirPath)+10,(int)'b',1);
	memset(outFilePath+strlen(outDirPath)+11,(int)'m',1);
	memset(outFilePath+strlen(outDirPath)+12,(int)'p',1);
	memset(outFilePath+strlen(outDirPath)+13,0,1);
	for (;curpos<endpos;curpos+=8){
		/* Reads offset and length of image */
		freelong1=0;
		freelong2=0;
		freeint1=fseek(obj,curpos,SEEK_SET);
		if (freeint1!=0){
			retval=2;
			goto end;
		}
		freesizet1=fread(&freelong1,1,4,obj);
		if (freesizet1!=4){
			retval=2;
			goto end;
		}
		freesizet1=fread(&freelong2,1,4,obj);
		if (freesizet1!=4){
			retval=2;
			goto end;
		}
		if ((type==5)&&((curpos<240)||(curpos>=328))) freelong2=262144;
		if (((freelong1)!=0)&&((freelong2)!=0)){
			/* Writes the name of the file to outList */
			fprintf(outList,"%08lX.bmp\n",(unsigned long int)freelong1);
		
			/* Extracts the image */
			inttostring(outFilePath+strlen(outDirPath)+1,
			            (unsigned INT32)freelong1,16,8);
			memset(outFilePath+strlen(outDirPath),(int)'/',1);
			memset(outFilePath+strlen(outDirPath)+9,(int)'.',1);
			bmp=fopen(outFilePath,"wb");
			if (bmp==NULL){
				retval=3;
				goto end;
			}
			
			/* Writes bitmap header */
			freeint1=bmpheader(bmp,(unsigned INT32)freelong2+54,54,512,
			                   (unsigned INT32)(freelong2/1024),16,0,
			                   (unsigned INT32)freelong2,0,0,0,0);
			if (freeint1!=0){
				retval=4;
				goto end;
			}
			
			/* Determines chunksize */
			chunksize=(((INT32)buffersize>=freelong2)?
			           freelong2:(INT32)buffersize);
			chunksize-=(chunksize%1024);
			
			/* Processes image data (backwards) in chunks */
			freelong2+=freelong1;
			while (freelong2>freelong1){
				/* Adjusts the size of the final chunk if needed */
				if (chunksize>(freelong2-freelong1))
					chunksize=(freelong2-freelong1);
				/* Reads chunk into buffer */
				if (freelong2>objSize){
					retval=30;
					goto end;
				}
				freelong2-=chunksize;
				freeint1=fseek(obj,freelong2,SEEK_SET);
				if (freeint1!=0){
					retval=2;
					goto end;
				}
				freesizet1=fread(buffer,1,(size_t)chunksize,obj);
				if (freesizet1!=(size_t)chunksize){
					retval=2;
					goto end;
				}
				/* Flips chunk vertically */
				freeint1=verticalFlip(buffer,1024,(size_t)
				                      (chunksize/1024));
				if (freeint1!=0){
					retval=5;
					goto end;
				}
				/* Converts colours from BGR to RGB */
				for (bufferpos=0;bufferpos<chunksize;bufferpos+=2){
					memmove(&colour,buffer+bufferpos,2);
					colour=((unsigned INT16)
					        (((unsigned INT16)(colour&0x83e0))+
					        ((unsigned INT16)(colour&0x7c00)>>10)+
					        ((unsigned INT16)(colour&0x1f)<<10)));
					memmove(buffer+bufferpos,&colour,2);
				}
				
				/* Writes chunk to output BMP-file */
				freesizet1=fwrite(buffer,1,(size_t)chunksize,bmp);
				if (freesizet1!=(size_t)chunksize){
					retval=4;
					goto end;
				}
			}
			/* Closes the file */
			if (bmp!=NULL){
				freeint1=fclose(bmp);
				if (freeint1!=0){
					retval=4;
					goto end;
				}else{
					bmp=NULL;
				}
			}
		}
	}
	
	/* Closes files, frees buffer and returns function */
end:
	if (buffer!=NULL) free(buffer);
	if (outFilePath!=NULL) free(outFilePath);
	if (obj!=NULL){
		freeint1=fclose(obj);
		if (freeint1!=0) retval=2;
	}
	if (bmp!=NULL){
		freeint1=fclose(bmp);
		if (freeint1!=0) retval=4;
	}
	if (outList!=NULL){
		freeint1=fclose(outList);
		if (freeint1!=0) retval=4;
	}
	return retval;
}

/*
 * Function that injects images into a PS1 OBJ-file.
 * Parameters:
 *   inDirPath = Path to input directory
 *   outFilePath = Path to output OBJ-file
 * Return values:
 *   0 = Everything went well
 *   1 = Input file could not be opened
 *   2 = Input file could not be read from
 *   3 = Output file could not be opened
 *   4 = Output file could not be written to
 *   5 = Buffer could not be allocated
 *   7 = Input bmp is not valid
 */
int injectobj(char *inDirPath, char *outFilePath){
	/* Variable Declarations */
	FILE *inList=NULL; /* Input list file */
	FILE *bmp=NULL; /* Input BMP-file */
	FILE *obj=NULL; /* Output OBJ-file */
	char *buffer=NULL; /* Buffer for image data */
	char *inFilePath=NULL; /* Buffer for filename */
	unsigned INT32 buffersize; /* Length of the buffer in bytes */
	long int inListSize; /* Length of input list in bytes */
	unsigned INT16 colour; /* Current colour */
	INT32 bufferpos; /* Current position in buffer */
	INT32 chunksize; /* Length of a chunk */
	INT32 freelong1,freelong2,freelong3,freelong4; /* For general use */
	unsigned INT32 freeulong1; /* For general use */
	size_t freesizet1; /* size_t for general use */
	int freeint1; /* Integer for general use */
	int retval=0; /* Return value for this function */
	
	/* Determines inFilePath */
	inFilePath=malloc((size_t)(strlen(inDirPath)+14));
	if (inFilePath==NULL){
		retval=5;
		goto end;
	}
	memcpy(inFilePath,inDirPath,strlen(inDirPath));
	memset(inFilePath+strlen(inDirPath),(int)'/',1);
	memset(inFilePath+strlen(inDirPath)+1,(int)'L',1);
	memset(inFilePath+strlen(inDirPath)+2,(int)'i',1);
	memset(inFilePath+strlen(inDirPath)+3,(int)'s',1);
	memset(inFilePath+strlen(inDirPath)+4,(int)'t',1);
	memset(inFilePath+strlen(inDirPath)+5,(int)'.',1);
	memset(inFilePath+strlen(inDirPath)+6,(int)'t',1);
	memset(inFilePath+strlen(inDirPath)+7,(int)'x',1);
	memset(inFilePath+strlen(inDirPath)+8,(int)'t',1);
	memset(inFilePath+strlen(inDirPath)+9,0,1);
	
	/* Opens input list file */
	inList=fopen(inFilePath,"rb");
	if (inList==NULL){
		retval=1;
		goto end;
	}
	
	/* Determines inListSize */
	freeint1=fseek(inList,0,SEEK_END);
	if (freeint1!=0){
		retval=2;
		goto end;
	}
	inListSize=ftell(inList);
	
	/* Opens output OBJ-file */
	obj=fopen(outFilePath,"r+b");
	if (obj==NULL){
		retval=3;
		goto end;
	}
	
	/* Allocates Buffer */
	buffer=NULL;
	buffersize=0x3FFFFF;
	do{
		buffer=malloc((size_t) buffersize);
		if ((buffersize<=1024)&&(buffer==NULL)){
			retval=5;
			goto end;
		}
		if (buffer==NULL) buffersize>>=1;
	}while (buffer==NULL);
	buffersize-=(buffersize%1024);
	
	/* Injects images */
	freeint1=fseek(inList,0,SEEK_SET);
	if (freeint1!=0){
		retval=3;
		goto end;
	}
	while (inListSize>ftell(inList)){
		/* Reads filename */	
		freeint1=readline(inFilePath+strlen(inDirPath)+1,13,inList,
		                  inListSize);
		if (freeint1!=0){
			retval=2;
			goto end;
		}
		if (strlen(inFilePath)>(strlen(inDirPath)+5)){
			/* Opens input bmp-file */
			bmp=fopen(inFilePath,"rb");
			if (bmp==NULL){
				retval=1;
				goto end;
			}
			
			/* Goes to the offset in OBJ */
			freeulong1=((unsigned INT32)
			            strtol(inFilePath+strlen(inDirPath)+1,NULL,16));
			freeint1=fseek(obj,(long int) freeulong1,SEEK_SET);
			if (freeint1!=0){
				retval=4;
				goto end;
			}
			
			/* Determines whether BMP is valid */
			freeint1=fseek(bmp,0,SEEK_END);
			if (freeint1!=0){
				retval=2;
				goto end;
			}
			if (ftell(bmp)<=54){
				retval=7;
				goto end;
			}
			freeint1=fseek(bmp,0,SEEK_SET);
			if (freeint1!=0){
				retval=2;
				goto end;
			}
			freeint1=fgetc(bmp);
			if (freeint1!=66){
				retval=7;
				goto end;
			}
			freeint1=fgetc(bmp);
			if (freeint1!=77){
				retval=7;
				goto end;
			}
			
			/* Reads BMP size */
			freeint1=fseek(bmp,10,SEEK_SET);
			if (freeint1!=0){
				retval=2;
				goto end;
			}
			freesizet1=fread(&freelong1,1,4,bmp);
			if (freesizet1!=4){
				retval=2;
				goto end;
			}
			freeint1=fseek(bmp,18,SEEK_SET);
			if (freeint1!=0){
				retval=2;
				goto end;
			}
			freesizet1=fread(&freelong2,1,4,bmp);
			if (freesizet1!=4){
				retval=2;
				goto end;
			}
			freesizet1=fread(&freelong3,1,4,bmp);
			if (freesizet1!=4){
				retval=2;
				goto end;
			}
			freeint1=fseek(bmp,28,SEEK_SET);
			if (freeint1!=0){
				retval=2;
				goto end;
			}
			freesizet1=fread(&colour,1,2,bmp);
			if (freesizet1!=2){
				retval=2;
				goto end;
			}
			freelong4=(freelong2*(colour>>3));
			freelong3=(freelong4*freelong3);
			
			/* Determines chunksize */
			chunksize=((INT32)buffersize>=freelong3)?
			           freelong3:(INT32)buffersize;
			chunksize-=(chunksize%freelong4);
			
			/* Processes image data (backwards) in chunks */
			freelong2=(freelong1+freelong3);
			while(freelong2>freelong1){
				/* Adjusts the size of the final chunk if needed */
				if (chunksize>(freelong2-freelong1))
					chunksize=(freelong2-freelong1);
				
				/* Reads chunk into buffer */
				freelong2-=chunksize;
				freeint1=fseek(bmp,freelong2,SEEK_SET);
				if (freeint1!=0){
					retval=2;
					goto end;
				}
				freesizet1=fread(buffer,1,(size_t)chunksize,bmp);
				if (freesizet1!=(size_t) chunksize){
					retval=2;
					goto end;
				}
				
				/* Flips chunk vertically */
				freeint1=verticalFlip(buffer,(size_t)freelong4,
				                      (size_t)(chunksize/freelong4));
				if (freeint1!=0){
					retval=5;
					goto end;
				}
				
				/* Converts colours from BGR to RGB */
				for (bufferpos=0;bufferpos<chunksize;bufferpos+=2){
					memmove(&colour,buffer+bufferpos,2);
					colour=(unsigned INT16)
					       ((unsigned INT16)(colour&0x83e0)+
					       ((unsigned INT16)(colour&0x7c00)>>10)+
					       ((unsigned INT16)(colour&0x1f)<<10));
					memmove(buffer+bufferpos,&colour,2);
				}
				
				/* Writes chunk to OBJ-file */
				freesizet1=fwrite(buffer,1,(size_t)chunksize,obj);
				if (freesizet1!=(size_t)chunksize){
					retval=4;
					goto end;
				}
			}
			/* Closes input bmp-file */
			freeint1=fclose(bmp);
			if (freeint1!=0){
				retval=2;
				goto end;
			}else{
				bmp=NULL;
			}
		}
	}
	
	/* Closes input list */
	freeint1=fclose(inList);
	if (freeint1!=0){
		retval=2;
		goto end;
	}else{
		inList=NULL;
	}
	
	/* Closes files, frees buffer and returns function */
end:
	if (buffer!=NULL) free(buffer);
	if (inFilePath!=NULL) free(inFilePath);
	if (obj!=NULL){
		freeint1=fclose(obj);
		if (freeint1!=0) retval=4;
	}
	if (bmp!=NULL){
		freeint1=fclose(bmp);
		if (freeint1!=0) retval=2;
	}
	if (inList!=NULL){
		freeint1=fclose(inList);
		if (freeint1!=0) retval=2;
	}
	return retval;
}
