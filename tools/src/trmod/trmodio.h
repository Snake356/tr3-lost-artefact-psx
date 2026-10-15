/*
 * Library that handles all I/O for TRMOD.
 *
 * All I/O functions used by TRMOD can be redirected through macro's in this
 * header file. This file is separated in two parts.
 * The first part is the External Interface. This checks for the various "modes"
 * this program can be compiled with, and defines all appropriate macro's for
 * that "mode".
 * The second part is the Internal Logic. This holds all needed I/O-macro's,
 * defined per function.
 */
#ifndef TRMOD_IO_H_
#define TRMOD_IO_H_

/* File Inclusions */
#include <stdio.h>

/* External Interface */
#ifdef TRMOD_GUI /* Graphical mode */
	#define SOMETHING /* TODO: ADD ACTUAL LOGIC HERE */
#else /* Default (terminal) mode */
	#define TRMOD_IO_PRINTF_STDOUT
	#define TRMOD_IO_FOPEN_FILE
	#define TRMOD_IO_FCLOSE_FILE
	#define TRMOD_IO_FSEEK_FILE
	#define TRMOD_IO_FTELL_FILE
	#define TRMOD_IO_FPUTC_FILE
	#define TRMOD_IO_FREAD_FILE
	#define TRMOD_IO_FWRITE_FILE
#endif

/* Internal Logic */

/*
 * trmod_printf:
 *
 * TRMOD_IO_PRINTF_STDOUT: Printf to stdout
 * TRMOD_IO_PRINTF_MUTE:   Printf shouts into the void
 */
#ifdef TRMOD_IO_PRINTF_STDOUT
	#define trmod_printf(args...) \
		printf(args)
#else
	#ifdef TRMOD_IO_PRINTF_MUTE
		#define trmod_printf(args...) \
			printf("")
	#endif
#endif

/*
 * trmod_fopen:
 *
 * TRMOD_IO_FOPEN_FILE: Directs to fopen()
 * TRMOD_IO_FOPEN_RAM:  Reads file into memory
 */
#ifdef TRMOD_IO_FOPEN_FILE
	#define trmod_fopen(a, b) \
		fopen(a, b)
#else
	#ifdef TRMOD_IO_FOPEN_RAM
		FILE *trmod_fopen_ram(char *path);
		#define trmod_fopen(a,b) \
			trmod_fopen_ram(a)
	#endif
#endif

/* trmod_fclose:
 *
 * TRMOD_IO_FCLOSE_FILE: Directs to fclose()
 * TRMOD_IO_FCLOSE_RAM: Loses all changes and removes level from RAM
 * TRMOD_IO_FCLOSE_RAM_WRITE: Writes in-RAM copy of the level to file
 */
#ifdef TRMOD_IO_FCLOSE_FILE
	#define trmod_fclose(a, b) \
		fclose(a)
#else
	#ifdef TRMOD_IO_FCLOSE_RAM
		int trmod_fclose_ram();
		#define trmod_fclose(a, b) \
			trmod_fclose_ram();
	#else
		#ifdef TRMOD_IO_FCLOSE_RAM_WRITE
			int trmod_fclose_ram_write(char *path);
			#define trmod_fclose(a, b) \
				trmod_fclose_ram_write(b)
		#endif
	#endif
#endif

/*
 * trmod_fseek:
 *
 * TRMOD_IO_FSEEK_FILE: Directs to fseek()
 * TRMOD_IO_FSEEK_RAM:  Adjusts in-RAM file position
 */
#ifdef TRMOD_IO_FSEEK_FILE
	#define trmod_fseek(a, b, c) \
		fseek(a, b, c)
#else
	#ifdef TRMOD_IO_FSEEK_RAM
		int trmod_fseek_ram(long offset, int orientation);
		#define trmod_fseek(a, b, c) \
			trmod_fseek_ram(b, c)
	#endif
#endif

/*
 * trmod_ftell:
 *
 * TRMOD_IO_FTELL_FILE: Directs to ftell()
 * TRMOD_IO_FTELL_RAM:  Returns in-RAM file position
 */
#ifdef TRMOD_IO_FTELL_FILE
	#define trmod_ftell(a) \
		ftell(a)
#else
	#ifdef TRMOD_IO_FTELL_RAM
		long int trmod_ftell_ram(void);
		#define trmod_ftell(a) \
			trmod_ftell_ram()
	#endif
#endif

/*
 * trmod_fputc:
 *
 * TRMOD_IO_FPUTC_FILE: Directs to fputc()
 * TRMOD_IO_FPUTC_RAM:  TODO
 */
#ifdef TRMOD_IO_FPUTC_FILE
	#define trmod_fputc(a, b) \
		fputc(a, b)
#else
	#ifdef TRMOD_IO_FPUTC_RAM
		/* TODO : IMPLEMENT */
	#endif
#endif

/*
 * trmod_fread:
 *
 * TRMOD_IO_FREAD_FILE: Directs to fread()
 * TRMOD_IO_FREAD_RAM:  TODO
 */
#ifdef TRMOD_IO_FREAD_FILE
	#define trmod_fread(a, b, c, d) \
		fread(a, b, c, d)
#else
	#ifdef TRMOD_IO_FREAD_RAM
		/* TODO : IMPLEMENT */
	#endif
#endif

/*
 * trmod_fwrite:
 *
 * TRMOD_IO_FWRITE_FILE: Directs to fwrite()
 * TRMOD_IO_FWRITE_RAM:  TODO
 */
#ifdef TRMOD_IO_FWRITE_FILE
	#define trmod_fwrite(a, b, c, d) \
		fwrite(a, b, c, d)
#else
	#ifdef TRMOD_IO_FWRITE_RAM
		/* TODO : IMPLEMENT */
	#endif
#endif

#endif
