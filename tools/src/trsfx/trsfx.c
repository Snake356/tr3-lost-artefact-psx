/*
 * Tomb Raider II/III SFX Editor v2.0
 * Copyright (C) 2016  b122251
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
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
#include <ctype.h>

/* Function Declarations */
int main(int argc, char *argv[]);
int unpack(char *inFilePath, char *outPath);
int pack(char *inPath, char *outFilePath);
void errorMessage(char *argv[], int errorCode);

/*
 * Main function. It reads the input given by the user, 
 * starts the corresponding functions, and shows potential errors.
 * Parameters:
 *   argc=The number of command-line parameters
 *   argv=Command-line parameters
 * Return codes:
 *  -1=Insufficient number of command-line parameters
 *   0=Everything went well
 *   1-5=unpack{
 *   	1=inFilePath could not be opened
 *   	2=Buffer could not be allocated
 *   	3=Output directory could not be made
 *   	4=Reading from sfx failed
 *   	5=sfx file is not valid
 *   }
 *   6-7=pack{
 *   	6=sfx file could not be opened
 *   	7=No buffer could be allocated
 *   }
 *   1024-8191=unpack{
 *   	1024-2047=Wave could not be opened on track &0x3FF
 *   	2048-4095=Reading from sfx failed on track &0x3FF
 *   	4096-8191=Writing failed on track &0x3FF
 *   }
 *   8192-32768=pack{
 *   	8192-16383=Reading from wave failed on track &0x3FF
 *   	16384-32768=Writing to sfx failed on track &0x3FF
 *   }
 */
int main(int argc, char *argv[]){
	/* Variable Declarations */
	int command; /* What to do: -1=nothing, 1=unpack, 2=pack */
	char argEnd[4]; /* Last 4 bytes of argument */
	int retcode; /* Return value of the called function */
	
	/* Checks number of parameters */
	if (argc<3){
		errorMessage(argv,-1);
		return -1;
	}
	
	/* Determines whether to pack or unpack */
	command=0;
	if (strlen(argv[2])>=4){
		argEnd[0]=argv[2][(strlen(argv[2])-4)];
		argEnd[1]=tolower(argv[2][(strlen(argv[2])-3)]);
		argEnd[2]=tolower(argv[2][(strlen(argv[2])-2)]);
		argEnd[3]=tolower(argv[2][(strlen(argv[2])-1)]);
		if (!strncmp(argEnd,".sfx",4)) command=2;
	}
	if (strlen(argv[1])>=4){
		argEnd[0]=argv[1][(strlen(argv[1])-4)];
		argEnd[1]=tolower(argv[1][(strlen(argv[1])-3)]);
		argEnd[2]=tolower(argv[1][(strlen(argv[1])-2)]);
		argEnd[3]=tolower(argv[1][(strlen(argv[1])-1)]);
		if (!strncmp(argEnd,".sfx",4)) command=1;
	}
	
	/* Calls appropriate function */
	if (command==1) retcode=unpack(argv[1],argv[2]);
	else if (command==2) retcode=pack(argv[1],argv[2]);
	else{
		errorMessage(argv,-1);
		return -1;
	}
	
	/* Prints potential errors */
	if (retcode) errorMessage(argv,retcode);
	return retcode;
}

/*
 * Unpacks sfx file into individual wave files
 * Parameters:
 *   inFilePath=Path to input file
 *   outPath=Path to output directory
 * Return codes:
 *   0=Everything went well
 *   1=inFilePath could not be opened
 *   2=Buffer could not be allocated
 *   3=Output directory could not be made
 *   4=Reading from sfx failed
 *   5=sfx file is not valid
 *   1024-2047=Wave could not be opened on track &0x3FF
 *   2048-4095=Reading from sfx failed on track &0x3FF
 *   4096-8191=Writing failed on track &0x3FF
 */
int unpack(char *inFilePath, char *outPath){
	/* Variable Declarations */
	FILE *sfx; /* SFX file */
	FILE *wave; /* Wave file */
	char *wavePath; /* String for path to wave file */
	char *buffer; /* Buffer */
	unsigned int bufferSize; /* Size of the buffer in bytes */
	unsigned int sfxSize; /* Size of the SFX-file in bytes */
	unsigned int curpos; /* Current position in SFX-file */
	unsigned int trackLength; /* Size of current track in bytes */
	unsigned int curTrack; /* Current track (counted from 0) */
	int mkdirReturn; /* Return value of mkdir */
	size_t readBytes; /* Return value of fread */
	size_t writtenBytes; /* Return value of fwrite */
	
	/* Opens sfx-file */
	sfx=fopen(inFilePath,"rb");
	if (sfx==NULL) return 1;
	fseek(sfx,0,SEEK_END);
	sfxSize=ftell(sfx);
	
	/* Allocates wavePath */
	wavePath=malloc(strlen(outPath)+10);
	if (wavePath==NULL) return 2;
	memset(wavePath,0,strlen(outPath)+10);
	
	/* Allocates buffer */
	buffer=NULL;
	bufferSize=0xFFFFF;
	do{
		buffer=malloc(bufferSize);
		if ((bufferSize==1)&&(buffer==NULL)) return 2;
		if (buffer==NULL) bufferSize>>=1;
	}while (buffer==NULL);
	
	/* Makes output directory */
#ifdef _WIN32
	mkdirReturn=mkdir(outPath);
#else
	mkdirReturn=mkdir(outPath,0755);
#endif
	if (mkdirReturn) return 3;
	
	/* Extracts tracks */
	curpos=0;
	fseek(sfx,0,SEEK_SET);
	for (curTrack=0;curpos!=sfxSize;curTrack++){
		/* Reads "RIFF" identifier and checks it */
		readBytes=fread(buffer,1,4,sfx);
		if (readBytes!=4) return 4;
		memset((buffer+4),0x52,1);
		memset((buffer+5),0x49,1);
		memset((buffer+6),0x46,2);
		if (memcmp(buffer,buffer+4,4)) return 5;
		/* Reads trackLength */
		readBytes=fread(&trackLength,1,4,sfx);
		if (readBytes!=4) return 4;
		trackLength+=(curpos+8);
		/* Checks whether whole track is insize sfx-file */
		if (trackLength>sfxSize) return 5;
		/* Opens wave file */
		sprintf(wavePath,"%s/%03i.wav",outPath,curTrack);
		wave=fopen(wavePath,"wb");
		if (wave==NULL) return (0x400|(curTrack&0x3FF));
		/* Extracts track */
		fseek(sfx,curpos,SEEK_SET);
		while (curpos<trackLength){
			/* Checks whether the rest of track fits inside buffer */
			if ((trackLength-curpos)>bufferSize){
				/* The rest of the track does NOT fit inside buffer */
				readBytes=fread(buffer,1,bufferSize,sfx);
				if (readBytes!=bufferSize)
					return (0x800|(curTrack&0x3FF));
				writtenBytes=fwrite(buffer,1,bufferSize,wave);
				if (writtenBytes!=bufferSize)
					return (0x1000|(curTrack&0x3FF));
				curpos+=bufferSize;
			}else{
				/* The rest of the track does fit inside buffer */
				readBytes=fread(buffer,1,(trackLength-curpos),sfx);
				if (readBytes!=(trackLength-curpos)) 
					return (0x800|(curTrack&0x3FF));
				writtenBytes=fwrite(buffer,1,(trackLength-curpos),wave);
				if (writtenBytes!=(trackLength-curpos)) 
					return (0x1000|(curTrack&0x3FF));
				curpos=trackLength;
			}
		}
		fclose(wave);
	}
	
	/* Closes the files and returns 0 */
	free(wavePath);
	free(buffer);
	fclose(sfx);
	return 0;
}

/*
 * Makes sfx file from individual wave files
 * Parameters:
 *   inPath=Path to input directory
 *   outFilePath=Path to output file
 * Return codes:
 *   0=Everything went well
 *   6=sfx file could not be opened
 *   7=No buffer could be allocated
 *   8192-16383=Reading from wave failed on track &0x3FF
 *   16384-32768=Writing to sfx failed on track &0x3FF
 */
int pack(char *inPath, char *outFilePath){
	/* Variable Declarations */
	FILE *sfx; /* SFX file */
	FILE *wave; /* Wave file */
	char *wavePath; /* String for path to wave file */
	char *buffer; /* Buffer */
	unsigned int bufferSize; /* Size of the buffer in bytes */
	unsigned int sfxSize; /* Size of the SFX-file in bytes */
	unsigned int curpos; /* Current position in SFX-file */
	unsigned int trackLength; /* Size of current track in bytes */
	unsigned int curTrack; /* Current track (counted from 0) */
	size_t readBytes; /* Return value of fread */
	size_t writtenBytes; /* Return value of fwrite */
	
	/* Opens sfx file */
	sfx=fopen(outFilePath,"wb");
	if (sfx==NULL) return 6;
	
	/* Allocates wavePath */
	wavePath=malloc(strlen(inPath)+10);
	if (wavePath==NULL) return 7;
	memset(wavePath,0,strlen(inPath)+10);
	
	/* Allocates buffer */
	buffer=NULL;
	bufferSize=0xFFFFF;
	do{
		buffer=malloc(bufferSize);
		if ((bufferSize==1)&&(buffer==NULL)) return 7;
		if (buffer==NULL) bufferSize>>=1;
	}while (buffer==NULL);
	
	/* Adds tracks to sfx file */
	for (curTrack=0;curTrack<999;curTrack++){
		/* Opens wave file */
		sprintf(wavePath,"%s/%03i.wav",inPath,curTrack);
		wave=fopen(wavePath,"rb");
		if (wave==NULL) break;
		/* Determines trackLength */
		fseek(wave,0,SEEK_END);
		trackLength=ftell(wave);
		fseek(wave,0,SEEK_SET);
		/* Adds track to sfx file */
		curpos=0;
		while (curpos<trackLength){
			/* Checks whether the rest of track fits inside buffer */
			if ((trackLength-curpos)>bufferSize){
				/* The rest of the track does NOT fit inside buffer */
				readBytes=fread(buffer,1,bufferSize,wave);
				if (readBytes!=bufferSize)
					return (0x2000|(curTrack&0x3FF));
				writtenBytes=fwrite(buffer,1,bufferSize,sfx);
				if (writtenBytes!=bufferSize)
					return (0x4000|(curTrack&0x3FF));
				curpos+=bufferSize;
			}else{
				/* The rest of the track does fit inside buffer */
				readBytes=fread(buffer,1,(trackLength-curpos),wave);
				if (readBytes!=(trackLength-curpos)) 
					return (0x2000|(curTrack&0x3FF));
				writtenBytes=fwrite(buffer,1,(trackLength-curpos),sfx);
				if (writtenBytes!=(trackLength-curpos)) 
					return (0x4000|(curTrack&0x3FF));
				curpos=trackLength;
			}
		}
		fclose(wave);
	}
	
	/* Closes files and returns 0 */
	free(wavePath);
	free(buffer);
	fclose(sfx);
	return 0;
}

/*
 * Function that prints an error message if needed.
 * Parameters:
 *   argv=Command-line parameters
 *   errorCode=Error code
 */
void errorMessage(char *argv[], int errorCode){
	/* Variable Declarations */
	int freeint1; /* integer for general use */
	
	/* Processes error messages */
	if (errorCode==(-1)){
		printf("trsfx: missing operands\n"
		       "Usage: trsfx input output\n");
	}
	if (errorCode<1024){
		switch (errorCode){
			case 1:
				printf("trsfx: %s could not be opened\n",argv[1]);
				break;
			case 2:
				printf("trsfx: Buffer could not be allocated\n");
				break;
			case 3:
				printf("trsfx: Directory %s could not be made\n"
				       ,argv[2]);
				break;
			case 4:
				printf("trsfx: Reading from %s failed\n",argv[1]);
				break;
			case 5:
				printf("trsfx: %s is not a valid sfx-file\n",argv[1]);
				break;
			case 6:
				printf("trsfx: %s could not be opened\n",argv[2]);
				break;
			case 7:
				printf("trsfx: Buffer could not be allocated\n");
				break;
		}
	} else if ((errorCode>=1024)&&(errorCode<=2047)){
		printf("trsfx: %s/%03i.wav could not be opened\n"
		       ,argv[2],(errorCode&0x3FF));
	} else if ((errorCode>=2048)&&(errorCode<=4095)){
		printf("trsfx: Reading from %s failed on track %i\n",argv[1]
		       ,(errorCode&0x3FF));
	} else if ((errorCode>=4096)&&(errorCode<=8191)){
		printf("trsfx: Writing failed on track %i\n",(errorCode&0x3FF));
	} else if ((errorCode>=8192)&&(errorCode<=16383)){
		printf("trsfx: Reading from %s/%03i.wav failed\n"
		       ,argv[1],(errorCode&0x3FF));
	} else if ((errorCode>=16384)&&(errorCode<=32767)){
		printf("trsfx: Writing to %s failed on track %i\n",argv[2]
		       ,(errorCode&0x3FF));
	}
}
