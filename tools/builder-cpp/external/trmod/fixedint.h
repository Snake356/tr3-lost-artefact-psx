/*
 * Definition of fixed-size integers used by TRMOD
 *
 * Please make sure all these integers are correct for the platform you are
 * compiling for (this is very important).
 */

#ifndef TRMOD_FIXEDINT_H_
#define TRMOD_FIXEDINT_H_

#if __STDC_VERSION__ >= 199901L /* C99 or higher (stdint.h exists) */
	#include <stdint.h> /* Standard Integer */
	
	/* 8-bit integer */
	typedef int8_t int8;
	
	/* 16-bit integer */
	typedef int16_t int16;
	
	/* 32-bit integer */
	typedef int32_t int32;
	
	/* 8-bit unsinged integer */
	typedef uint8_t uint8;
	
	/* 16-bit unsigned integer */
	typedef uint16_t uint16;
	
	/* 32-bit unsigned integer */
	typedef uint32_t uint32;
	
	/* 16-bit integer for printing */
	#define PRINT16 "%i"
	
	/* 32-bit integer for printing */
	#define PRINT32 "%i"
	
	/* 16-bit unsigned integer for printing */
	#define PRINTU16 "%u"
	
	/* 32-bit unsigned integer for printing */
	#define PRINTU32 "%u"
#else /* Lower than C99 (stdint.h doesn't exist) */
	#ifdef __DOS__
		/* 8-bit integer */
		typedef signed char int8;
		
		/* 16-bit integer */
		typedef signed int int16;
		
		/* 32-bit integer */
		typedef signed long int int32;
		
		/* 8-bit unsinged integer */
		typedef unsigned char uint8;
		
		/* 16-bit unsigned integer */
		typedef unsigned int uint16;
		
		/* 32-bit unsigned integer */
		typedef unsigned long int uint32;
		
		/* 16-bit integer for printing */
		#define PRINT16 "%i"
		
		/* 32-bit integer for printing */
		#define PRINT32 "%li"
		
		/* 16-bit unsigned integer for printing */
		#define PRINTU16 "%u"
		
		/* 32-bit unsigned integer for printing */
		#define PRINTU32 "%lu"
	#else
		/* 8-bit integer */
		typedef signed char int8;
		
		/* 16-bit integer */
		typedef signed short int int16;
		
		/* 32-bit integer */
		typedef signed int int32;
		
		/* 8-bit unsinged integer */
		typedef unsigned char uint8;
		
		/* 16-bit unsigned integer */
		typedef unsigned short int int16;
		
		/* 32-bit unsigned integer */
		typedef unsigned int int32;
		
		/* 16-bit integer for printing */
		#define PRINT16 "%i"
		
		/* 32-bit integer for printing */
		#define PRINT32 "%i"
		
		/* 16-bit unsigned integer for printing */
		#define PRINTU16 "%u"
		
		/* 32-bit unsigned integer for printing */
		#define PRINTU32 "%u"
		
	#endif
#endif

/* Reverses a 16-bit integer */
#define reverse16(in) ((((uint16) (in & 0xFF00U)) >> 8U) | \
                       (((uint16) (in & 0x00FFU)) << 8U));

/* Reverses a 32-bit integer */
#define reverse32(in) ((((uint32) (in & 0xFF000000U)) >> 24U) | \
                       (((uint32) (in & 0x00FF0000U)) >> 8U)  | \
                       (((uint32) (in & 0x0000FF00U)) << 8U)  | \
                       (((uint32) (in & 0x000000FFU)) << 24U));

/* Adjusts for endianness */
#define endian32(a)            \
	if (level->endian == TRUE) \
	{                          \
		a = reverse32(a);      \
	}
#define endian16(a)            \
	if (level->endian == TRUE) \
	{                          \
		a = reverse16(a);      \
	}

/* Booleans */
#define TRUE  0
#define FALSE 1

#endif
