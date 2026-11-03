/*
 * Definition of fixed-size integers used by TRMOD
 *
 * Please make sure all these integers are correct for the platform you are
 * compiling for (this is very important).
 */

#ifndef TRMOD_FIXEDINT_H_
#define TRMOD_FIXEDINT_H_

/* 8-bit integer */
#define INT8 signed char

/* 16-bit integer */
#define INT16 signed short int

/* 32-bit integer */
#define INT32 signed int

/* 8-bit unsinged integer */
#define INTU8 unsigned char

/* 16-bit unsigned integer */
#define INTU16 unsigned short int

/* 32-bit unsigned integer */
#define INTU32 unsigned int

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

#endif
