/*
 * Program that overwrites a track in an XA-audio file with zeroes.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[])
{
	/* Variable Declarations */
	FILE *xa = NULL;
	int seekval = 0;
	size_t writeval = (size_t) 0u;
	long int filesize = 0l;
	long int curpos = 0l;
	unsigned short int numtracks = 0U;
	unsigned short int remtrack = 0U;
	unsigned short int curtrack = 0U;
	unsigned char zeroes[2336];
	
	/* Checks command-line parameters */
	if (argc < 4)
	{
		printf("Usage: %s [xa-file] [number of tracks] [track to remove]\n", argv[0]);
		goto end;
	}
	
	/* Zeroes the zeroes */
	memset(zeroes, 0, 2336);
	
	/* Opens the xa-file */
	xa = fopen(argv[1], "r+b");
	if (xa == NULL)
	{
		printf("%s could not be opened\n", argv[1]);
		goto end;
	}

	/* Gets the number of tracks and track to remove */
	numtracks = (unsigned short int) strtol(argv[2], NULL, 10);
	remtrack = (unsigned short int) strtol(argv[3], NULL, 10);

	/* Determines the xa-file's size */
	seekval = fseek(xa, 0l, SEEK_END);
	if (seekval != 0)
	{
		printf("%s could not be read from\n", argv[1]);
		goto end;
	}
	filesize = ftell(xa);
	
	/* Loops through the sectors */
	curtrack = 0U;
	for (curpos = 0l; curpos < filesize; curpos += 2336l)
	{
		/* Wipes the sector if needed */
		if (curtrack == remtrack)
		{
			seekval = fseek(xa, curpos, SEEK_SET);
			if (seekval != 0)
			{
				printf("%s could not be read from\n", argv[0]);
				goto end;
			}
			writeval = fwrite(zeroes, (size_t) 1U, (size_t) 2336U, xa);
			if (writeval != (size_t) 2336U)
			{
				printf("%s could not be read from\n", argv[0]);
				goto end;
			}
		}
		
		/* Moves to the next track */
		++curtrack;
		if (curtrack == numtracks)
		{
			curtrack = 0U;
		}
	}
	
end:
	if (xa != NULL)
	{
		(void) fclose(xa);
	}

	return 0;
}
