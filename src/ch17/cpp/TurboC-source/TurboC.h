/*
  TurboC, a library for porting Borland Turbo C to GNU gcc.
  Copyright 2002 Ronald S. Burkey
 
  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
  
  Contact Ron Burkey at info@sandroid.org.

  Filename:	TurboC.h
  Purpose:	TurboC is a useful little kit of stuff that can be used for
  		converting a Borland Turbo C application to a GNU gcc
		console program.  It doesn't do the whole job, but it does
		provide various library functions present in Turbo C but not
		in gcc libraries. 
  Mod history:	01/27/02 RSB	Created.
  		01/31/02 RSB	Continued, defining the conio.h subset used.
		02/03/02 RSB	Split off the conio.h and alloc.h stuff into
				separate headers.
		03/02/02 RSB	Added integer datatype conversions.
				Added random.				
		03/18/02 RSB	Modified for C++.
		03/23/02 RSB	Fixed a stray #elif that was supposed to be
				#else.  Added __APPLE__ options.
		03/31/02 RSB	Added some signal-handling stuff.
		04/18/02 RSB	Fixed strcpy and strncpy macros.
		05/09/02 RSB	Moved variables out, and allowed
				for pthreads.
		05/19/02 RSB	Added a few default header files so as
				to conform more easily to the contributed
				code by Igor Bujna.
		12/16/02 RSB	Fixed declaration of fcloseall so that it won't
				conflict with gcc 3.2.						

  Not that anybody's likely to care, but constants, datatypes, and function
  prototypes have been taken from the Turbo C Reference Guide, version 2.0,
  and not from any Borland source files.  I imagine that I might have been
  able to just abridge conio.h, alloc.h, etc., but in fact I didn't even look
  at them.
*/

#ifndef _INCLUDED_TURBOC_H
#define _INCLUDED_TURBOC_H

// Note that all system headers required by *any* TurboC header (even those
// included *after* this one) need to be included before integer datatypes
// are redefined.
#include <sys/cdefs.h>
#include <ncurses.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#if defined(linux)
#include <endian.h>
#include <stdint.h>
#elif defined(__FreeBSD__) || defined(__APPLE__)
#include <machine/endian.h>
#include <inttypes.h>
#else
#error Need header files for endian type and integer datatypes.
#endif
#ifdef WITH_X
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#endif
#include <sys/stat.h>		// Needed by dir.h.
#include <pthread.h>
// (Igor Bujna) --------------------------------------------------------------
#ifdef __DJGPP__
// If anyone knows how to easily solve the following problem, it would be 
// great to know about!
#error The following will not work, because TurboC has its own dos.h/bios.h.
#include <dos.h>
#include <bios.h>
#endif // __DJGPP__
#include <sys/ioctl.h>
#ifdef __linux__
#ifndef PPRSTATUS
// Unfortunately, there's nothing within ppdev.h itself to prevent it from
// being loaded twice, which generates error messages.
#include <linux/ppdev.h>
#endif
/*
 * You must have kernel-2.4.x or newer. For kernel 2.2.x you must
 * download ppdev-patch from ftp.torque.net/pub/parport
 */
#include <linux/parport.h>
#endif // __linux__
// (End Igor) ----------------------------------------------------------------

//----------------------------------------------------------------------------
// Constants.

#define __libTurboC__

// Integer-type stuff that differs depending on whether the original
// source code was for a 16-bit compiler (default) or 32-bit.
typedef char gchar;
typedef signed char gschar;
typedef unsigned char guchar;
typedef short gshort;
typedef unsigned short gushort;
typedef int gint;
typedef unsigned int guint;
typedef long glong;
typedef unsigned long gulong;

#ifdef CLK_TCK
#undef CLK_TCK
#endif
#define CLK_TCK (sysconf(_SC_CLK_TCK))

#ifndef DoNotFixIntegers

// Various ctype.h macros are broken by the integer datatype conversions 
// below.  This fixes them.
//#include <ctype.h>
#ifdef isupper
#undef isupper
#define isupper isupperTurbo
#undef islower
#define islower islowerTurbo
#undef isalpha
#define isalpha isalphaTurbo
#undef isdigit
#define isdigit isdigitTurbo
#undef isxdigit
#define isxdigit isxdigitTurbo
#undef isspace
#define isspace isspaceTurbo
#undef isprint
#define isprint isprintTurbo
#undef isgraph
#define isgraph isgraphTurbo
#undef iscntrl
#define iscntrl iscntrlTurbo
#undef ispunct
#define ispunct ispunctTurbo
#undef isalnum
#define isalnum isalnumTurbo
#endif // isupper

// String functions that have been renamed, or which are implemented in
// GNU by macros that are screwed up by integer datatype conversion.

#ifdef strcpy
static inline char *
strcpyTurbo (char *s1, const char *s2)
{
  return (strcpy (s1, s2));
}

#undef strcpy
#define strcpy strcpyTurbo
#endif // strcpy

#ifdef strncpy
static inline char *
strncpyTurbo (char *s1, const char *s2, size_t n)
{
  return (strncpy (s1, s2, n));
}

#undef strncpy
#define strncpy strncpyTurbo
#endif // strncpy

#ifdef strcmp
static inline int16_t
strcmpTurbo (const char *s1, const char *s2)
{
  return (strcmp (s1, s2));
}

#undef strcmp
#define strcmp strcmpTurbo
#endif // strcmp

#ifdef strcasecmp
static inline int16_t
strcmpi (const char *s1, const char *s2)
{
  return (strcasecmp (s1, s2));
}
static inline int16_t
stricmp (const char *s1, const char *s2)
{
  return (strcasecmp (s1, s2));
}
#else // strcasecmp
#define strcmpi strcasecmp
#define stricmp strcasecmp
#endif // strcasecmp

#ifdef strncmp
static inline int16_t
strncmpTurbo (const char *s1, const char *s2, int16_t n)
{
  return (strncmp (s1, s2, n));
}

#undef strncmp
#define strncmp strncmpTurbo
#endif // strcmp

#ifdef strncasecmp
static inline int16_t
strncmpi (const char *s1, const char *s2, int16_t n)
{
  return (strncasecmp (s1, s2, n));
}
#else // strncasecmp
#define strncmpi strncasecmp
#endif // strncasecmp

static inline clock_t
clockUnix (void)
{
  return (clock ());
}

#ifdef clock
#undef clock
#endif
#define clock clockTurbo

// Integer datatype conversions.  This particular choice of macros has the
// happy property of handling the most commonly used integer datatype 
// specifications (except "unsigned char"), while generating a compiler
// error for all other combinations.
#define short int16_t
#define int int16_t
#define unsigned uint16_t
#define long int32_t

#endif // Compiler32

// Pointer modifiers that aren't needed.
#define far
#define huge

// The "free" function:  We could use the standard library function for 
// almost all purposes ... except that we need to account for the sins of the 
// getimage function in graphics.h.  We need to introduce a substitute
// that's identical to the standard "free" in most cases, but which 
// in some cases transparently frees Pixmaps that getimage has created 
// without the programmer's knowledge.  freeUnix is identical to the standard
// library function, while freeTurbo, or just free, is the replacement.
static inline void
freeUnix (void *object)
{
  free (object);
}

#ifdef free
#undef free
#endif
#define free freeTurbo

//----------------------------------------------------------------------------
// Data types.

//----------------------------------------------------------------------------
// Function prototypes.

__BEGIN_DECLS
// My custom version of "free".
extern void freeTurbo (void *object);
#ifdef WITH_X
extern int AssociatePixmap (void *object, Pixmap handle);
#endif

// Stuff involved in gracefully exiting the program.
void TurboTrap (void);
int TurboX (int);

// From string.h.
extern char *strupr (char *);
extern char *strlwr (char *);

// From stdio.h
extern gint fcloseall (void);

// Wrappers for ctype.h macros.
extern int isupperTurbo (int c);
extern int islowerTurbo (int c);
extern int isalphaTurbo (int c);
extern int isdigitTurbo (int c);
extern int isxdigitTurbo (int c);
extern int isspaceTurbo (int c);
extern int isprintTurbo (int c);
extern int isgraphTurbo (int c);
extern int iscntrlTurbo (int c);
extern int ispunctTurbo (int c);
extern int isalnumTurbo (int c);

#define random(n) randomTurbo(n)
extern int16_t randomTurbo (int16_t num);
extern void randomize (void);
extern clock_t clockTurbo (void);

// Endian conversion functions.
extern void FixLittle16 (uint16_t *);
extern void FixLittle32 (uint32_t *);
extern void FixBig16 (uint16_t *);
extern void FixBig32 (uint32_t *);

// Endian file read/write functions.
extern int ReadLittle16 (FILE * fp, uint16_t * Value);
extern int ReadBig16 (FILE * fp, uint16_t * Value);
extern int ReadLittle32 (FILE * fp, uint32_t * Value);
extern int ReadBig32 (FILE * fp, uint32_t * Value);
extern int WriteLittle16 (FILE * fp, uint16_t Value);
extern int WriteBig16 (FILE * fp, uint16_t Value);
extern int WriteLittle32 (FILE * fp, uint32_t Value);
extern int WriteBig32 (FILE * fp, uint32_t Value);

__END_DECLS
//--------------------------------------------------------------------------
// Variables.
#ifdef TURBOC_VARIABLES_C
gint TcVideoMode = 0;
// The following arrays give unicode equivalents for all IBM PC characters,
// where possible.  The only one I wasn't able to find an equivalent for is
// the 0x9e character.  I don't even know what that one is!  For a long
// time it looked to me like an Rx symbol.  Now it looks like Pt (whatever 
// that is).  At any rate, I can't find an equivalent for either one.
// These characters all exist in the so-called GNU unifont, though not 
// necessarily in Adobe or other unifonts.
const int TcUnicodeMappings[256] = {
  // Row 0x00
  0x0040,	// 0x0
  0x263a,	// 0x1
  0x263b,	// 0x2
  0x2665,	// 0x3
  0x2666,	// 0x4
  0x2663,	// 0x5
  0x2660,	// 0x6
  0x2022,	// 0x7
  0x25d8,	// 0x8
  0x25e6,	// 0x9
  0x25d9,	// 0xa
  0x2642,	// 0xb
  0x2640,	// 0xc
  0x266a,	// 0xd
  0x266c,	// 0xe
  0x263c,	// 0xf
  // Row 0x10
  0x25b6,	// 0x0
  0x25c0,	// 0x1
  0x2195,	// 0x2
  0x203c,	// 0x3
  0x00b6,	// 0x4
  0x00a7,	// 0x5
  0x2582,	// 0x6
  0x21a8,	// 0x7
  0x2191,	// 0x8
  0x2193,	// 0x9
  0x2192,	// 0xa
  0x2190,	// 0xb
  0x2319,	// 0xc
  0x2194,	// 0xd
  0x25b2,	// 0xe
  0x25bc,	// 0xf
  // Row 0x20
  0x0020,	// 0x0
  0x0021,	// 0x1
  0x0022,	// 0x2
  0x0023,	// 0x3
  0x0024,	// 0x4
  0x0025,	// 0x5
  0x0026,	// 0x6
  0x0027,	// 0x7
  0x0028,	// 0x8
  0x0029,	// 0x9
  0x002a,	// 0xa
  0x002b,	// 0xb
  0x002c,	// 0xc
  0x002d,	// 0xd
  0x002e,	// 0xe
  0x002f,	// 0xf
  // Row 0x30
  0x0030,	// 0x0
  0x0031,	// 0x1
  0x0032,	// 0x2
  0x0033,	// 0x3
  0x0034,	// 0x4
  0x0035,	// 0x5
  0x0036,	// 0x6
  0x0037,	// 0x7
  0x0038,	// 0x8
  0x0039,	// 0x9
  0x003a,	// 0xa
  0x003b,	// 0xb
  0x003c,	// 0xc
  0x003d,	// 0xd
  0x003e,	// 0xe
  0x003f,	// 0xf
  // Row 0x40
  0x0040,	// 0x0
  0x0041,	// 0x1
  0x0042,	// 0x2
  0x0043,	// 0x3
  0x0044,	// 0x4
  0x0045,	// 0x5
  0x0046,	// 0x6
  0x0047,	// 0x7
  0x0048,	// 0x8
  0x0049,	// 0x9
  0x004a,	// 0xa
  0x004b,	// 0xb
  0x004c,	// 0xc
  0x004d,	// 0xd
  0x004e,	// 0xe
  0x004f,	// 0xf
  // Row 0x50
  0x0050,	// 0x0
  0x0051,	// 0x1
  0x0052,	// 0x2
  0x0053,	// 0x3
  0x0054,	// 0x4
  0x0055,	// 0x5
  0x0056,	// 0x6
  0x0057,	// 0x7
  0x0058,	// 0x8
  0x0059,	// 0x9
  0x005a,	// 0xa
  0x005b,	// 0xb
  0x005c,	// 0xc
  0x005d,	// 0xd
  0x005e,	// 0xe
  0x005f,	// 0xf
  // Row 0x60
  0x0060,	// 0x0
  0x0061,	// 0x1
  0x0062,	// 0x2
  0x0063,	// 0x3
  0x0064,	// 0x4
  0x0065,	// 0x5
  0x0066,	// 0x6
  0x0067,	// 0x7
  0x0068,	// 0x8
  0x0069,	// 0x9
  0x006a,	// 0xa
  0x006b,	// 0xb
  0x006c,	// 0xc
  0x006d,	// 0xd
  0x006e,	// 0xe
  0x006f,	// 0xf
  // Row 0x70
  0x0070,	// 0x0
  0x0071,	// 0x1
  0x0072,	// 0x2
  0x0073,	// 0x3
  0x0074,	// 0x4
  0x0075,	// 0x5
  0x0076,	// 0x6
  0x0077,	// 0x7
  0x0078,	// 0x8
  0x0079,	// 0x9
  0x007a,	// 0xa
  0x007b,	// 0xb
  0x007c,	// 0xc
  0x007d,	// 0xd
  0x007e,	// 0xe
  0x0020,	// 0xf
  // Row 0x80
  0x00c7,	// 0x0
  0x00fc,	// 0x1
  0x00e9,	// 0x2
  0x00e2,	// 0x3
  0x00e4,	// 0x4
  0x00e0,	// 0x5
  0x00e5,	// 0x6
  0x00e7,	// 0x7
  0x00ea,	// 0x8
  0x00eb,	// 0x9
  0x00e8,	// 0xa
  0x00ef,	// 0xb
  0x00ee,	// 0xc
  0x00ec,	// 0xd
  0x00c4,	// 0xe
  0x00c5,	// 0xf
  // Row 0x90
  0x00c9,	// 0x0
  0x00e6,	// 0x1
  0x00c6,	// 0x2
  0x00f4,	// 0x3
  0x00f6,	// 0x4
  0x00f2,	// 0x5
  0x00fb,	// 0x6
  0x00f9,	// 0x7
  0x00ff,	// 0x8
  0x00d6,	// 0x9
  0x00dc,	// 0xa
  0x00a2,	// 0xb
  0x00a3,	// 0xc
  0x00a5,	// 0xd
  0x00bc,	// 0xe
  0x0192,	// 0xf
  // Row 0xa0
  0x00e1,	// 0x0
  0x00ed,	// 0x1
  0x00f3,	// 0x2
  0x00fa,	// 0x3
  0x00f1,	// 0x4
  0x00d1,	// 0x5
  0x00aa,	// 0x6
  0x00ba,	// 0x7
  0x00bf,	// 0x8
  0x231c,	// 0x9
  0x231d,	// 0xa
  0x00bd,	// 0xb
  0x00bc,	// 0xc
  0x00a1,	// 0xd
  0x00ab,	// 0xe
  0x00bb,	// 0xf
  // Row 0xb0
  0x2591,	// 0x0
  0x2592,	// 0x1
  0x2593,	// 0x2
  0x2502,	// 0x3
  0x2524,	// 0x4
  0x2561,	// 0x5
  0x2562,	// 0x6
  0x2556,	// 0x7
  0x2555,	// 0x8
  0x2563,	// 0x9
  0x2551,	// 0xa
  0x2557,	// 0xb
  0x255d,	// 0xc
  0x255c,	// 0xd
  0x255b,	// 0xe
  0x2510,	// 0xf
  // Row 0xc0
  0x2514,	// 0x0
  0x2534,	// 0x1
  0x252c,	// 0x2
  0x251c,	// 0x3
  0x2500,	// 0x4
  0x253c,	// 0x5
  0x255e,	// 0x6
  0x255f,	// 0x7
  0x255a,	// 0x8
  0x2554,	// 0x9
  0x2569,	// 0xa
  0x2566,	// 0xb
  0x2560,	// 0xc
  0x2550,	// 0xd
  0x256c,	// 0xe
  0x2567,	// 0xf
  // Row 0xd0
  0x2568,	// 0x0
  0x2564,	// 0x1
  0x2565,	// 0x2
  0x2559,	// 0x3
  0x2558,	// 0x4
  0x2552,	// 0x5
  0x2553,	// 0x6
  0x256b,	// 0x7
  0x256a,	// 0x8
  0x2518,	// 0x9
  0x250c,	// 0xa
  0x2588,	// 0xb
  0x2585,	// 0xc
  0x258b,	// 0xd
  0x258a,	// 0xe
  0x2580,	// 0xf
  // Row 0xe0
  0x03b1,	// 0x0
  0x03b2,	// 0x1
  0x0393,	// 0x2
  0x03c0,	// 0x3
  0x03a3,	// 0x4
  0x03c3,	// 0x5
  0x03bc,	// 0x6
  0x03be,	// 0x7
  0x03a6,	// 0x8
  0x0398,	// 0x9
  0x03a9,	// 0xa
  0x03b4,	// 0xb
  0x221e,	// 0xc
  0x2205,	// 0xd
  0x2208,	// 0xe
  0x2229,	// 0xf
  // Row 0xf0
  0x2261,	// 0x0
  0x00b1,	// 0x1
  0x2265,	// 0x2
  0x2264,	// 0x3
  0x2320,	// 0x4
  0x2321,	// 0x5
  0x00f7,	// 0x6
  0x2248,	// 0x7
  0x00b0,	// 0x8
  0x2219,	// 0x9
  0x00ad,	// 0xa
  0x221a,	// 0xb
  0x207f,	// 0xc
  0x00b2,	// 0xd
  0x220e,	// 0xe
  0x019e	// 0xf
};
#else // TURBOC_VARIABLES_C
extern gint TcVideoMode;
extern const int TcUnicodeMappings[256];
#endif // TURBOC_VARIABLES_C

#endif // _INCLUDED_TURBO_H
