/***************************************************************************

	memory.h

	Functions which handle the CPU memory and I/O port access.
	
	Modificata e semplificata il 26/03/2003

***************************************************************************/

#ifndef _MEMORY_H
#define _MEMORY_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Versions of GNU C earlier that 2.7 appear to have problems with the
 * __attribute__ definition of UNUSEDARG, so we act as if it was not a
 * GNU compiler.
 */

#ifdef __GNUC__
#if (__GNUC__ < 2) || ((__GNUC__ == 2) && (__GNUC_MINOR__ <= 7))
#define UNUSEDARG
#else
#define UNUSEDARG __attribute__((__unused__))
#endif
#else
#define UNUSEDARG
#endif



/***************************************************************************

	Basic type definitions

	These types are used for memory handlers.

***************************************************************************/

/* ----- typedefs for data and offset types ----- */
typedef unsigned char			data8_t;
typedef unsigned int			offs_t;

/* ----- typedefs for the various common memory/port handlers ----- */
typedef data8_t			(*read8_handler)  (UNUSEDARG offs_t offset);
typedef void			(*write8_handler) (UNUSEDARG offs_t offset, UNUSEDARG data8_t data);


/* ----- typedefs for the various common memory handlers ----- */
typedef read8_handler	mem_read_handler;
typedef write8_handler	mem_write_handler;



/***************************************************************************

	Basic macros

***************************************************************************/

/* ----- macros for accessing bytes and words within larger chunks ----- */
#ifdef LSB_FIRST
	#define BYTE_XOR_BE(a)  	((a) ^ 1)				/* read/write a byte to a 16-bit space */
	#define BYTE_XOR_LE(a)  	(a)
	#define BYTE4_XOR_BE(a) 	((a) ^ 3)				/* read/write a byte to a 32-bit space */
	#define BYTE4_XOR_LE(a) 	(a)
	#define WORD_XOR_BE(a)  	((a) ^ 2)				/* read/write a word to a 32-bit space */
	#define WORD_XOR_LE(a)  	(a)
#else
	#define BYTE_XOR_BE(a)  	(a)
	#define BYTE_XOR_LE(a)  	((a) ^ 1)				/* read/write a byte to a 16-bit space */
	#define BYTE4_XOR_BE(a) 	(a)
	#define BYTE4_XOR_LE(a) 	((a) ^ 3)				/* read/write a byte to a 32-bit space */
	#define WORD_XOR_BE(a)  	(a)
	#define WORD_XOR_LE(a)  	((a) ^ 2)				/* read/write a word to a 32-bit space */
#endif




#ifdef __cplusplus
}
#endif

#endif	/* !_MEMORY_H */

