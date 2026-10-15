/*
 * Library for converting audio samples in TR3PCPSX.
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
#include <math.h>     /* Functions relating to mathematics */
#include "fixedint.h" /* Definition of fixed-size integers */
#include "errors.h"   /* Error Handling for TR3PCPSX */

/* Macro Definitions */
#define OUTPUT_FRAMERATE 11000 /* Frames per second for the output sample */
#define INTERPOLATION    1     /* (0 = Neirest neighbour, 1 = Linear) */

/* Internal Function Prototypes */
static INTU32 numFrames(INTU32 numInFrames, INTU32 inRate, INTU32 outRate);
static void convertSampleRate(INT16 *in, INT16 *out, INTU32 inRate,
                              INTU32 outRate, INTU32 numInFrames,
                              INTU32 numOutFrames, int interpolation);
static int convertBitrate(INTU8 *in, INTU8 *out, INTU16 inRate, INTU16 outRate,
                          INTU32 size);
static void convertStereotoMono(INT16 *in, INT16 *out, INTU32 size);
static void convertRawToVag(INTU8 *in, INTU8 *out, INTU32 numFrames,
                            double *find__s_1, double *find__s_2,
                            double *pack_s_1, double *pack_s_2, int loops);

/*
 * Function that converts an audio sample from PC to PSX.
 * Parameters:
 * * pc = Pointer to PC-file
 * * psx = Pointer to PSX-file
 * * pcOffset = Offset in PC-file where the sample will be
 * * psxOffset = Offset in the PSX-file where to insert the sample
 * * loops = Whether this sample loops (1 = yes, 0 = no)
 */
int convertSample(FILE *pc, FILE *psx, INTU32 pcOffset,
                  INTU32 psxOffset, int loops)
{
	/* Variable Declarations */
	INTU8 *buffer = NULL; /* Buffer space for the audio sample */
	size_t bufferSize;    /* Size of the buffer */
	INTU32 numInFrames;   /* Number of frames in the input sample */
	INTU32 inFrameRate;   /* Sample rate of the input sample */
	INTU16 inBitRate;     /* Bitrate of the input sample */
	INTU16 numInChannels; /* Number of sound channels in the input sample */
	INTU32 numOutFrames;  /* Number of frames for the output sample */
	int errorNumber;      /* Return value for this function */
	double vagStatics[4]; /* Static doubles for VAG conversion */
	INTU16 freeuint161;   /* Unsigned 16-bit integer for general use */
	INTU32 freeuint321,
	       freeuint322;   /* Unsigned 32-bit integers for general use */
	int freeint1;         /* Integer for general use */
	size_t freesizet1,
	       freesizet2;    /* size_t's for general use */
	
	/* Sets error value to 0 */
	errorNumber = ERROR_NONE;
	
	/* Checks that the data is in PCM */
	pcOffset += 20;
	freeint1 = fseek(pc, (long int) pcOffset, SEEK_SET);
	if (freeint1 != 0)
	{
		errorNumber = ERROR_PC_READ_FAILED;
		goto end;
	}
	freesizet1 = fread(&freeuint161, 1, 2, pc);
	if (freesizet1 != 2)
	{
		errorNumber = ERROR_PC_READ_FAILED;
		goto end;
	}
#ifdef __BIG_ENDIAN__
	freeuint161 = reverse16(freeuint161);
#endif
	if (freeuint161 != 0x0001)
	{
		errorNumber = ERROR_WAVE_NON_PCM;
		goto end;
	}
	
	/* Reads the number of channels */
	freesizet1 = fread(&numInChannels, 1, 2, pc);
	if (freesizet1 != 2)
	{
		errorNumber = ERROR_PC_READ_FAILED;
		goto end;
	}
#ifdef __BIG_ENDIAN__
	numInChannels = reverse16(numInChannels);
#endif
	if (numInChannels > 2)
	{
		errorNumber = ERROR_WAVE_TOO_MANY_CHANNELS;
		goto end;
	}
	
	/* Reads the input sample rate */
	freesizet1 = fread(&inFrameRate, 1, 4, pc);
	if (freesizet1 != 4)
	{
		errorNumber = ERROR_PC_READ_FAILED;
		goto end;
	}
#ifdef __BIG_ENDIAN__
	inFrameRate = reverse32(inFrameRate);
#endif
	
	/* Reads input bitrate */
	pcOffset += 14;
	freeint1 = fseek(pc, (long int) pcOffset, SEEK_SET);
	if (freeint1 != 0)
	{
		errorNumber = ERROR_PC_READ_FAILED;
		goto end;
	}
	freesizet1 = fread(&inBitRate, 1, 2, pc);
	if (freesizet1 != 2)
	{
		errorNumber = ERROR_PC_READ_FAILED;
		goto end;
	}
#ifdef __BIG_ENDIAN__
	inBitRate = reverse16(inBitRate);
#endif
	
	/* Checks that the input bitrate is supported */
	if ((inBitRate != 8) && (inBitRate != 16) &&
	    (inBitRate != 32))
	{
		errorNumber = ERROR_SAMPLE_BITRATE;
		goto end;
	}
	
	/* Determines the total number of input frames */
	freeint1 = fseek(pc, (long int) 4, SEEK_CUR);
	if (freeint1 != 0)
	{
		errorNumber = ERROR_PC_READ_FAILED;
		goto end;
	}
	freesizet1 = fread(&numInFrames, 1, 4, pc);
	if (freesizet1 != 4)
	{
		errorNumber = ERROR_PC_READ_FAILED;
		goto end;
	}
#ifdef __BIG_ENDIAN__
	numInFrames = reverse32(numInFrames);
#endif
	numInFrames /= (inBitRate >> 3);
	numInFrames /= numInChannels;
	
	/* Sets pcOffset to the beginning of the audio data */
	pcOffset += 10;
	
	/* Determines numOutFrames */
	numOutFrames = numFrames(numInFrames, inFrameRate, OUTPUT_FRAMERATE);
	
	/* Determines sample's largest size in conversion */
	freeuint321 = (INTU32) (numInFrames * (inBitRate >> 3) * numInChannels);
	freeuint322 = (INTU32) (numInFrames * 2 * numInChannels);
	if (freeuint322 > freeuint321)
	{
		freeuint321 = freeuint322;
	}
	freeuint322 = (INTU32) ((numOutFrames + (30 - (numOutFrames % 28))) * 2);
	if (freeuint322 > freeuint321)
	{
		freeuint321 = freeuint322;
	}
	bufferSize = (size_t) freeuint321;
	
	/* Determines whether the buffer can be allocated */
	if ((sizeof(size_t) < 4) && ((freeuint321 & 0xFFFF0000) != 0x00000000))
	{
		errorNumber = ERROR_MEMORY;
		goto end;
	}
	
	/* Allocates the buffer */
	buffer = malloc(bufferSize);
	if (buffer == NULL)
	{
		errorNumber = ERROR_MEMORY;
		goto end;
	}
	
	/* Reads the sample into memory */
	freeint1 = fseek(pc, (long int) pcOffset, SEEK_SET);
	if (freeint1 != 0)
	{
		errorNumber = ERROR_PC_READ_FAILED;
		goto end;
	}
	
	freesizet2 = (size_t) (numInFrames * (inBitRate >> 3) * numInChannels);
	freesizet1 = fread(buffer, 1, freesizet2, pc);
	if (freesizet1 != freesizet2)
	{
		errorNumber = ERROR_PC_READ_FAILED;
		goto end;
	}
	
	/* Converts the bitrate if needed */
	if (inBitRate != 16)
	{
		errorNumber = convertBitrate(buffer, buffer, inBitRate, 16,
		                             (numInFrames * numInChannels));
		if (errorNumber != ERROR_NONE)
		{
			goto end;
		}
	}
	
	/* Converts Stereo to Mono if needed */
	if (numInChannels == 2)
	{
		convertStereotoMono((INT16 *) buffer, (INT16 *) buffer, numInFrames);
	}
	
	if (inFrameRate < OUTPUT_FRAMERATE)
	{
		freeuint321 = (INTU32) (numInFrames * 2);
		freeuint322 = (((INTU32) bufferSize) - freeuint321);
		memmove(&buffer[freeuint322], buffer, (size_t) freeuint321);
	}
	else
	{
		freeuint322 = 0x00000000;
	}
	
	/* Converts sample rate if needed */
	if (inFrameRate != OUTPUT_FRAMERATE)
	{
		convertSampleRate((INT16 *) (&buffer[freeuint322]), (INT16 *) buffer,
		                  inFrameRate, OUTPUT_FRAMERATE, numInFrames,
		                  numOutFrames, INTERPOLATION);
	}
	
	/* Pads the rest of the needed buffer space if needed */
	(void) memset(&buffer[(numOutFrames * 2)], 0,
	              (size_t) (56 - ((numOutFrames * 2) % 56)));
	
	/* Converts the sample into VAG format */
	vagStatics[0] = 0.0;
	vagStatics[1] = 0.0;
	vagStatics[2] = 0.0;
	vagStatics[3] = 0.0;
	convertRawToVag(buffer, buffer, numOutFrames, &vagStatics[0],
	                &vagStatics[1], &vagStatics[2], &vagStatics[3], loops);
	
	/* Determines the total size of the VAG-sample */
	freeuint321 = numOutFrames;
	freeint1 = (int) (freeuint321 % 28);
	if (freeint1 != 0)
	{
		freeuint321 += (28 - freeint1);
	}
	freeuint321 /= 28;
	freeuint321 *= 16;
	freeuint321 += 16;
	freesizet2 = (size_t) freeuint321;
	
	/* Writes the VAG sample to the PSX-file */
	freeint1 = fseek(psx, (long int) psxOffset, SEEK_SET);
	if (freeint1 != 0)
	{
		errorNumber = ERROR_PSX_READ_FAILED;
		goto end;
	}
	freesizet1 = fwrite(buffer, 1, freesizet2, psx);
	if (freesizet1 != freesizet2)
	{
		errorNumber = ERROR_PSX_WRITE_FAILED;
		goto end;
	}
	
end:/* Frees buffers and ends the function */
	if (buffer != NULL)
	{
		free(buffer);
	}
	return errorNumber;
}

/*
 * Function that calculates the size of a VAG-sample from a WAVE-sample.
 * Parameters:
 * * pc = Pointer to the PC-file
 * * offset = Offset of the sample in pc.
 * Returns the size of the output sample in bytes.
 */
INTU32 outVagSize(FILE *sfx, INTU32 offset)
{
	/* Variable Declarations */
	INTU32 numInFrames;   /* Number of frames in the input sample */
	INTU32 inFrameRate;   /* Sample rate of the input sample */
	INTU16 inBitRate;     /* Bitrate of the input sample */
	INTU16 numInChannels; /* Number of sound channels in the input sample */
	INTU32 numOutFrames;  /* Number of frames for the output sample */
	size_t freesizet1;    /* Size_t for general use */
	int freeint1;         /* Integer for general use */
	INTU32 freeuint321;   /* 32-bit unsigned integer for general use */
	INTU16 freeuint161;   /* 16-bit unsigned integer for general use */
	
	/* Checks that the data is in PCM */
	offset += 20;
	freeint1 = fseek(sfx, (long int) offset, SEEK_SET);
	if (freeint1 != 0)
	{
		return 0x00000000;
	}
	freesizet1 = fread(&freeuint161, 1, 2, sfx);
	if (freesizet1 != 2)
	{
		return 0x00000000;
	}
#ifdef __BIG_ENDIAN__
	freeuint161 = reverse16(freeuint161);
#endif
	if (freeuint161 != 0x0001)
	{
		return 0x00000000;
	}
	
	/* Reads the number of channels */
	freesizet1 = fread(&numInChannels, 1, 2, sfx);
	if (freesizet1 != 2)
	{
		return 0x00000000;
	}
#ifdef __BIG_ENDIAN__
	numInChannels = reverse16(numInChannels);
#endif
	if (numInChannels > 2)
	{
		return 0x00000000;
	}
	
	/* Reads the input sample rate */
	freesizet1 = fread(&inFrameRate, 1, 4, sfx);
	if (freesizet1 != 4)
	{
		return 0x00000000;
	}
#ifdef __BIG_ENDIAN__
	inFrameRate = reverse32(inFrameRate);
#endif
	
	/* Reads input bitrate */
	offset += 14;
	freeint1 = fseek(sfx, (long int) offset, SEEK_SET);
	if (freeint1 != 0)
	{
		return 0x00000000;
	}
	freesizet1 = fread(&inBitRate, 1, 2, sfx);
	if (freesizet1 != 2)
	{
		return 0x00000000;
	}
#ifdef __BIG_ENDIAN__
	inBitRate = reverse16(inBitRate);
#endif
	
	/* Checks that the input bitrate is supported */
	if ((inBitRate != 8) && (inBitRate != 16) &&
	    (inBitRate != 32))
	{
		return 0x00000000;
	}
	
	/* Determines the total number of input frames */
	freeint1 = fseek(sfx, (long int) 4, SEEK_CUR);
	if (freeint1 != 0)
	{
		return 0x00000000;
	}
	freesizet1 = fread(&numInFrames, 1, 4, sfx);
	if (freesizet1 != 4)
	{
		return 0x00000000;
	}
#ifdef __BIG_ENDIAN__
	numInFrames = reverse32(numInFrames);
#endif
	numInFrames /= (inBitRate >> 3);
	numInFrames /= numInChannels;
	
	/* Calculates the number of output frames */
	numOutFrames = numFrames(numInFrames, inFrameRate, OUTPUT_FRAMERATE);
	
	/* Determines the total size of the VAG-sample */
	freeuint321 = numOutFrames;
	freeint1 = (int) (freeuint321 % 28);
	if (freeint1 != 0)
	{
		freeuint321 += (28 - freeint1);
	}
	freeuint321 /= 28;
	freeuint321 *= 16;
	freeuint321 += 16;
	
	/* Returns the size of the output sample */
	return freeuint321;
}

/*
 * Function that calculates how many frames will be created
 * in the conversion of an audio sample from one framerate to another.
 * Parameters:
 * * numInFrames = Number of frames of the input sample
 * * inRate = Sample rate of the input sample (in Hz)
 * * outRate = Sample rate of the output sample (in Hz)
 * Returns the number of frames
 */
static INTU32 numFrames(INTU32 numInFrames, INTU32 inRate, INTU32 outRate)
{
	/* Variable Declarations */
	INTU32 freeuint321,
	       freeuint322; /* Unsigned 32-bit integers for general use */
	
	/* Determines the number of frames up to the last second */
	freeuint321 = (numInFrames / inRate);
	freeuint321 *= outRate;
	
	/* Determines the number of frames within the last second */
	freeuint322 = ((numInFrames % inRate) * outRate);
	freeuint322 /= inRate;
	
	/* Adds the two and returns it */
	freeuint321 += freeuint322;
	return freeuint321;
}

/*
 * Function that converts an audio sample from one sample rate to another.
 * Parameters:
 * * in = Pointer to the input sample
 * * out = Pointer to allocated space for the output sample
 * * inRate = Sample rate of the input sample (in Hz)
 * * outRate = Sample rate of the output sample (in Hz)
 * * numInFrames = Number of frames in the input sample
 * * numOutFrames = Number of frames in the output sample
 * * interpolation = Method of interpolation (0 = Nearest, 1 = Linear)
 */
static void convertSampleRate(INT16 *in, INT16 *out, INTU32 inRate,
                              INTU32 outRate, INTU32 numInFrames,
                              INTU32 numOutFrames, int interpolation)
{
	/*
	 * This function is used to convert between sample rates. It works for any
	 * sample rate provided that ((inRate * outRate) <= 4294967295).
	 * It can use two types of interpolation, but what is important is that
	 * to calculate a position on the X-axis, the product of both input and
	 * output sample rates is used as a scale. But since this would lead to
	 * integer overflow very quickly, if a point on the X-axis is reached that
	 * is a multiple of both input and output, the current frame counter is
	 * reset, and the offset to count from moved along. This ensures that long
	 * samples won't suffer from integer overflow.
	 */
	
	/* Variable Declarations */
	INTU32 curFrame;    /* Current frame in the output sample */
	INTU32 inOffset;    /* The offset for the input sample */
	INTU32 outOffset;   /* The offset for the output sample */
	INTU32 outCount;    /* Current frame from outOffset */
	INTU32 freeuint321,
	       freeuint322; /* Unsigned 32-bit integers for general use */
	INT32 freeint321;   /* Signed 32-bit integer for general use */
	
	/* Adjusts for Big Endian */
#ifdef __BIG_ENDIAN__
	for (curFrame = 0x00000000; curFrame < numInFrames; ++curFrame)
	{
		in[curFrame] = reverse16(in[curFrame]);
	}
#endif
	
	/* Sets counters to zero before starting */
	inOffset = 0x00000000;
	outOffset = 0x00000000;
	outCount = 0x00000000;
	
	/* Converts the frames */
	for (curFrame = 0x00000000; curFrame < numOutFrames; ++curFrame)
	{
		/* Checks whether the output frame is on an input frame */
		freeuint321 = (outCount * inRate);
		if ((freeuint321 % outRate) == 0x00000000)
		{
			/* Copies the frame directly because it is */
			freeuint321 /= outRate;
			freeuint321 += inOffset;
			out[curFrame] = in[freeuint321];
			inOffset = freeuint321;
			outOffset += outCount;
			outCount = 0x00000001;
		}
		else
		{
			/* Interpolates between two frames because it isn't */
			if (interpolation == 0)
			{
				/* Determines the frame using Neirest Neighbour */
				if (((freeuint321 % outRate) << 1) >= outRate)
				{
					freeuint321 /= outRate;
					freeuint321 += (inOffset + 1);
					out[curFrame] = in[freeuint321];
				}
				else
				{
					freeuint321 /= outRate;
					freeuint321 += inOffset;
					out[curFrame] = in[freeuint321];
				}
			}
			else if (interpolation == 1)
			{
				/* Determines the frame using Linear Interpolation */
				freeuint322 = (freeuint321 % outRate);
				freeuint321 /= outRate;
				freeuint321 += inOffset;
				freeint321 = (INT32) (in[(freeuint321 + 1)] - in[freeuint321]);
				freeint321 *= (INT32) freeuint322;
				freeint321 /= (INT32) outRate;
				out[curFrame] = (INT16) (in[freeuint321] + freeint321);
			}
			++outCount;
		}
	}
	
	/* Adjusts for Big Endian */
#ifdef __BIG_ENDIAN__
	for (curFrame = 0x00000000; curFrame < numOutFrames; ++curFrame)
	{
		out[curFrame] = reverse16(out[curFrame]);
	}
#endif
}

/*
 * Function that converts audio bitrate between 8, 16 and 32 bits.
 * Parameters:
 * * in = Pointer to the input sample
 * * out = Pointer to storage for the output sample
 * * inRate = Bitrate of the input
 * * outRate = Bitrate of the output
 * * size = Number of frames in the input
 * Nota bene: out must be allocated.
 */
static int convertBitrate(INTU8 *in, INTU8 *out, INTU16 inRate,
                          INTU16 outRate, INTU32 size)
{
	/* Variable Declarations */
	INTU32 curFrame;
	
	/* Checks whether the bitrates are supported */
	if ((inRate != 8) && (inRate != 16) && (inRate != 32))
	{
		return ERROR_SAMPLE_BITRATE;
	}
	if ((outRate != 8) && (outRate != 16) && (outRate != 32))
	{
		return ERROR_SAMPLE_BITRATE;
	}
	
	if (outRate > inRate)
	{
		in += ((size - 1) * (inRate >> 3));
		out += ((size - 1) * (outRate >> 3));
	}
	
	for (curFrame = 0x00000000; curFrame < size; ++curFrame)
	{
		if ((inRate == 8) && (outRate == 16))
		{
			*out = (INTU8) (((INT16) ((INTU16) *in) - 128) & 0x00FF);
			out[1] = out[0];
			out -= 2;
			--in;
		}
		else if ((inRate == 8) && (outRate == 32))
		{
			*out = (INTU8) ((((INT16) (INTU16) *in) - 128) & 0x00FF);
			out[1] = out[0];
			out[2] = out[0];
			out[3] = out[0];
			out -= 4;
			--in;
		}
		else if ((inRate == 16) && (outRate == 8))
		{
			*out = in[1];
			++out;
			in += 2;
		}
		else if ((inRate == 16) && (outRate == 32))
		{
			out[0] = in[0];
			out[1] = in[1];
			out[2] = in[0];
			out[3] = in[1];
			out -= 4;
			in -= 2;
		}
		else if ((inRate == 32) && (outRate == 8))
		{
			out[0] = in[3];
			out++;
			in += 4;
		}
		else if ((inRate == 32) && (outRate == 16))
		{
			out[0] = in[2];
			out[1] = in[3];
			out += 2;
			in += 4;
		}
	}
	return ERROR_NONE;
}

/*
 * Function that converts a 16-bit stereo sample to mono.
 * Parameters:
 * * in = Pointer to the input stereo sample
 * * out = Storage for the output mono sample
 * * size = Number of frames of audio in the sample
 */
static void convertStereotoMono(INT16 *in, INT16 *out, INTU32 size)
{
	for (; size > 0x00000000; --size)
	{
		/* Takes the average of the left and right channels */
#ifdef __BIG_ENDIAN__
		in[0] = reverse16(in[0]);
		in[1] = reverse16(in[1]);
#endif
		*out = (INT16) ((((INT32) in[0]) + ((INT32) in[1])) / 2);
#ifdef __BIG_ENDIAN__
		*out = reverse16(*out);
#endif
		in += 2;
		out++;
	}
}

/*
 * Function that converts RAW-audio to VAG-audio.
 * Parameters:
 * * in = Pointer to input sample
 * * out = Pointer to output sample
 * * numFrames = Number of audio frames to be converted
 * * find__s_1 - pack_s_2 = Kept across function calls
 * Nota bene:
 * * The input sample must be in 16-bit PCM mono.
 * * There are four parameters that need to be transferred between
 * * function calls (find__s_1 to pack_s_2). On the first run, set them to
 * * 0.0, and don't touch them after that.
 */
static void convertRawToVag(INTU8 *in, INTU8 *out, INTU32 numFrames,
                            double *find__s_1, double *find__s_2,
                            double *pack_s_1, double *pack_s_2, int loops)
{
	/* Variable Declarations */
	INTU16 predict_nr = 0x0000;
	INTU16 shift_factor = 0x0000;
	INT32 pack_di;
	INTU16 shiftmask;
	INTU8 flags;
	INT16 freeint161;
	INTU32 numChunks;
	INTU16 counter0,
	       counter1;
	double chunkDouble[28];
	double buffer[28][5];
	double find_min;
	double find_max[5];
	double find_ds;
	double find_s_0,
	       find_s_1,
	       find_s_2;
	double pack_ds;
	double pack_s_0;
	double vagConstant[5][2] = {{ 0.0, 0.0 },
	                            {-60.0 / 64.0, 0.0},
	                            {-115.0 / 64.0, 52.0 / 64.0},
	                            {-98.0 / 64.0, 55.0 / 64.0},
	                            {-122.0 / 64.0, 60.0 / 64.0}};
	
	/* Determines the number of chunks */
	flags = (INTU8) 0x00;
	if (loops == 1)
	{
		flags = (INTU8) 0x06;
	}
	numChunks = (numFrames / 28);
	if ((numFrames % 28) != 0x00000000)
	{
		++numChunks;
	}
	
	/* Goes through each chunk of 28 audio frames */
	for (; numChunks > 0x00000000; --numChunks)
	{
		/* Calculates predict_nr and shift_factor */
		find_min = 1e10;
		for (counter0 = 0; counter0 < 5; ++counter0)
		{
			find_max[counter0] = 0.0;
			find_s_1 = *find__s_1;
			find_s_2 = *find__s_2;
			
			/* Loops through the frames */
			for (counter1 = 0; counter1 < 28; ++counter1)
			{
				memcpy(&freeint161, &in[(counter1 * 2)], 2);
#ifdef __BIG_ENDIAN__
				freeint161 = reverse16(freeint161);
#endif
				find_s_0 = (double) freeint161;
				
				/* Avoids clipping */
				if (find_s_0 > 30719.0)
				{
					find_s_0 = 30719.0;
				}
				if (find_s_0 < -30720.0)
				{
					find_s_0 = -30720.0;
				}
				
				find_ds = (find_s_0 + (find_s_1 * vagConstant[counter0][0]) +
				           (find_s_2 * vagConstant[counter0][1]));
				buffer[counter1][counter0] = find_ds;
				
				/* Remembers if this was the biggest point in the curve */
				if (fabs(find_ds) > find_max[counter0])
				{
					find_max[counter0] = fabs(find_ds);
				}
				
				/* Moves the frame keeping variables one along */
				find_s_2 = find_s_1;
				find_s_1 = find_s_0;
			}
			
			if (find_max[counter0] < find_min)
			{
				find_min = find_max[counter0];
				predict_nr = counter0;
			}
			if (find_min <= 7)
			{
				predict_nr = 0;
				break;
			}
		}
		
		/* Remembers the last two frames for the next chunk */
		*find__s_1 = find_s_1;
		*find__s_2 = find_s_2;
		
		/* Converts the frames to chunkDouble using the predict_nr */
		for (counter0 = 0; counter0 < 28; ++counter0)
		{
			chunkDouble[counter0] = buffer[counter0][predict_nr];
		}
		
		/* Determines the shift factor */
		freeint161 = (INT16) find_min;
		shiftmask = 0x4000;
		shift_factor = 0;
		
		while (shift_factor < 12)
		{
			if ((shiftmask & (freeint161 + (shiftmask >> 3))) != 0)
			{
				break;
			}
			++shift_factor;
			shiftmask >>= 1;
		}
		
		/* Packs the 28 audio frames in VAG-format */
		for (counter0 = 0; counter0 < 28; counter0 += 2)
		{
			pack_s_0 = (chunkDouble[counter0] +
			            (*pack_s_1 * vagConstant[predict_nr][0]) +
			            (*pack_s_2 * vagConstant[predict_nr][1]));
			pack_ds = (pack_s_0 * ((double) (1 << shift_factor)));
			pack_di = ((((INT32) pack_ds) + 0x0800) & 0xFFFFF000);
			
			if (pack_di > 32767)
			{
				pack_di = 32767;
			}
			if (pack_di < -32768)
			{
				pack_di = -32768;
			}
			
			freeint161 = (INT16) pack_di;
			pack_di >>= shift_factor;
			*pack_s_2 = *pack_s_1;
			*pack_s_1 = (double) (pack_di - pack_s_0);
			
			pack_s_0 = (chunkDouble[(counter0 + 1)] +
			            (*pack_s_1 * vagConstant[predict_nr][0]) +
			            (*pack_s_2 * vagConstant[predict_nr][1]));
			pack_ds = (pack_s_0 * ((double) (1 << shift_factor)));
			pack_di = ((((INT32) pack_ds) + 0x0800) & 0xFFFFF000);
			
			if (pack_di > 32767)
			{
				pack_di = 32767;
			}
			if (pack_di < -32768)
			{
				pack_di = -32768;
			}
			
			out[(2 + (counter0 / 2))] = (INTU8)
			                            (((((INT16) pack_di) >> 8) & 0xF0) |
			                             ((freeint161 >> 12) & 0x0F));
			
			pack_di >>= shift_factor;
			*pack_s_2 = *pack_s_1;
			*pack_s_1 = (((double) pack_di) - pack_s_0);
		}
		
		/* Writes the first two bytes of the chunk (predict/shift, flags) */
		out[0] = (INTU8) ((predict_nr << 4) | shift_factor);
		out[1] = (INTU8) flags;
		
		flags &= (INTU8) 0xFB;
		
		/* Sets flags to 1 if the final chunk is padded */
		in += 56;
		out += 16;
		numFrames -= 28;
		if (numFrames <= 28)
		{
			flags |= (INTU8) 0x01;
		}
	}
	
	if (loops == 0)
	{
		/* Writes the predict number and shift factor and a 7,
		 * followed by 14 0's to mark the end of the sample */
		out[0] = (INTU8) ((predict_nr << 4) | shift_factor);
		out[1] = (INTU8) 0x07;
		(void) memset(&out[2], 0, 14);
	}
}
