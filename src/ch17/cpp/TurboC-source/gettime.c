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

  Filename:	gettime.c
  Purpose:	A function for TurboC. 
  Mod history:	03/02/02 RSB	Created.
*/

#include <time.h>
#include "dos.h"

//----------------------------------------------------------------------------

void
gettime (struct time *timep)
{
  struct tm *ModTime;
  time_t t;
  time (&t);
  ModTime = localtime (&t);
  timep->ti_hund = 0;
  timep->ti_sec = ModTime->tm_sec;
  timep->ti_min = ModTime->tm_min;
  timep->ti_hour = ModTime->tm_hour;
}
