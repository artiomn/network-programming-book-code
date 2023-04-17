/*
  TurboC, a library for porting Borland Turbo C to GNU gcc.
  Copyright 2002 Ronald S. Burkey
  Items indicated are Copyright (c) Igor Bujna 1999-2001. 
 
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

  Filename:	dos.h
  Purpose:	A header file for TurboC. 
  Mod history:	03/02/02 RSB	Created.
  		03/18/02 RSB	Modified for C++.
		03/19/02 RSB	Added file-attribute constants.
		05/19/02 RSB	Added contributions by Igor Bujna.
		12/16/02 RSB	Fixed getdate to override the library function
				which has suddenly appeared in newer libraries.
*/

#ifndef _INCLUDED_DOS_H
#define _INCLUDED_DOS_H

#include <sys/cdefs.h>
#include <time.h>
#include "TurboC.h"

//----------------------------------------------------------------------------
// Constants.

#define FA_RDONLY	0x01	/* Read only attribute */
#define FA_HIDDEN	0x02	/* Hidden file */
#define FA_SYSTEM	0x04	/* System file */
#define FA_LABEL	0x08	/* Volume label */
#define FA_DIREC	0x10	/* Directory */
#define FA_ARCH		0x20	/* Archive */

//----------------------------------------------------------------------------
// Data types.

struct time
{
  uint8_t ti_min;
  uint8_t ti_hour;
  uint8_t ti_hund;
  uint8_t ti_sec;
};

//----------------------------------------------------------------------------
// Function prototypes.

__BEGIN_DECLS 

//
extern void gettime (struct time *timep);
extern void delay (guint gettime);

__END_DECLS
//--------------------------------------------------------------------------
// Variables.
// (This section contributed by Igor Bujna, modified by RSB ----------------
/*
 * Sleep, delay ....
 */
// RSB:  I prefer handling the delay function differently, because I
// have no way to insure that ncurses is initialized.
//#define       delay(sc)       napms(sc)
/*
 * Date and Time function
 */
#ifndef __DJGPP__
#ifndef _S_DATE
#define _S_DATE
  struct date
{
  short da_year;		/* current year */
  char da_day;			/* day of the month */
  char da_mon;			/* month (1=Jan) */
};
#endif /* _S_DATE  */

#if 0
#ifndef _S_TIME
#define _S_TIME
struct time
{
  guchar ti_min;		/* minutes */
  guchar ti_hour;		/* hours */
  guchar ti_hund;		/* hundredths of seconds */
  guchar ti_sec;		/* seconds */
};
#endif /* _S_TIME  */
#endif // 0

#endif /* __DJGPP__    */

extern struct date date_null;
extern struct time time_null;

__BEGIN_DECLS extern time_t dostounix (struct date *d, struct time *t);
extern void unixtodos (time_t the_time, struct date *d, struct time *t);
extern void unixdos_time (time_t the_time, struct time *t);
extern void unixdos_date (time_t the_time, struct date *d);
extern void gettime_d (struct time *t);
// RSB:  The following originally was suffixed by "_d".
#define getdate(d) getdateTurbo (d)
extern void getdateTurbo (struct date *d);
extern struct tm* getdateSystem (const char *d);

__END_DECLS
#define SECONDS_PER_DAY	86400L	/* the number of seconds in one day */
#define MONTH_SIZE		3
#define MONTH_L_SIZE		8
#define DAY_SIZE		3
#define DAY_L_SIZE		7
#if 0
extern char *month[], *month_l[], *day[], *day_l[];
#endif

// (End Igor Bujna) --------------------------------------------------------

#endif // _INCLUDED_DOS_H
