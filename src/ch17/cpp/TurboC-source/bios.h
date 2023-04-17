/*
  TurboC, a library for porting Borland Turbo C to GNU gcc.
  Copyright 2002 Ronald S. Burkey
  Copyright (c) Igor Bujna 1999-2001. 
 
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
  Contact Igor Bujna <igor.bujna@maxi-tip.cz>

  Filename:	bios.h
  Purpose:	A header file for TurboC. 
  Mod history:	05/19/02 RSB	Created to hold contributions by 
  				Igor Buja.  At this point, Igor's code
				is the only contents, and  my 
				contribution is nil.
*/

#ifndef _INCLUDED_BIOS_H
#define _INCLUDED_BIOS_H

#include <sys/cdefs.h>
#include "TurboC.h"

__BEGIN_DECLS
/*
 * Print function (Biosprint ...)
 */
#ifndef __DJGPP__
#ifdef __linux__
extern int open_printer (int port);
#define close_printer(fd)	close(fd)
extern ssize_t write_printer (int fd, const char *buf);
#endif // __linux__
extern int biosprint (int cmd, int byte, int port);
#endif /* __DJGPP__    */

enum
{
  LPT1 = 0, LPT2 = 1, LPT3 = 2, TO_FILE = 999,
};

#define write_printer_seq(fd, typ) write_printer(fd, seqence_STAR_34X(typ))

#define PRINT_FILE	"./data/print_file.txt"

extern void flush_print (char *buf, int port);
extern void print_file (const char *file, int port, int lowlevel_print);

#if 0

extern char *seqence_STAR_34X (int typ);

enum
{
  NEXT_LINE, CUTTING, FONT_7_9, FONT_5_9_2P, FONT_5_9_3P, EXPANDED_MODE,
  CANCEL_EXPANDED_MODE, EMPHASIZED_MODE, CANCEL_EMPHASIZED_MODE,
  UNDERLINE_MODE, CANCEL_UNDERLINE_MODE, UPPERLINE_MODE,
  CANCEL_UPPERLINE_MODE, HIGHLIGHT_MODE, CANCEL_HIGHLIGHT_MODE,
  INVERTED_MODE, CANCEL_INVERTED_MODE, IBM_CHARSET_1, IBM_CHARSET_2, SOUND,
  INITIALIZE_PRINTER_CLEAN, DESELECT_MODE, SELECT_MODE, UNIDIRECTIONAL_MODE,
  BIDIRECTIONAL_MODE, INITIALIZE_PRINTER, LINE_1_8_INCH, LINE_1_6_INCH
};

#endif // 0

__END_DECLS
#endif // _INCLUDED_BIOS_H
