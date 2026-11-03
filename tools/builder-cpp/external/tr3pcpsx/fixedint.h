/*
 * Definition of fixed-size integers used by TR3PCPSX.
 *
 * Please make sure all these integers are correct for the platform you are
 * compiling for (this is very important).
 */

#ifndef TR3PCPSX_FIXEDINT_H_
#define TR3PCPSX_FIXEDINT_H_

#include <stdint.h>

/* 8-bit integer */
#define INT8 int8_t

/* 16-bit integer */
#define INT16 int16_t

/* 32-bit integer */
#define INT32 int32_t

/* 8-bit unsinged integer */
#define INTU8 uint8_t

/* 16-bit unsigned integer */
#define INTU16 uint16_t

/* 32-bit unsigned integer */
#define INTU32 uint32_t

/* 16-bit integer for printing */
#define PRINT16 "%i"

/* 32-bit integer for printing */
#define PRINT32 "%i"

/* 16-bit unsigned integer for printing */
#define PRINTU16 "%u"

/* 32-bit unsigned integer for printing */
#define PRINTU32 "%u"

/* Reverses a 16-bit integer */
#define reverse16(in) ((((INTU16) (in & 0xFF00)) >> 8) | \
                       (((INTU16) (in & 0x00FF)) << 8));

/* Reverses a 32-bit integer */
#define reverse32(in) ((((INTU32) (in & 0xFF000000)) >> 24) | \
                       (((INTU32) (in & 0x00FF0000)) >> 8)  | \
                       (((INTU32) (in & 0x0000FF00)) << 8)  | \
                       (((INTU32) (in & 0x000000FF)) << 24));

/* Whether to check the sizes of these integers when starting the program */
#define FIXEDINT_CHECK_ON_STARTUP

#endif
