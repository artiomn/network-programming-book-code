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

  Filename:	delay.c
  Purpose:	A Turbo C dos.h function.
  Mod history:	05/19/02 RSB	Created.
*/

#include <sys/times.h>
#include "dos.h"

static int clockInitialized = 0;
static clock_t InitialTime;

//-----------------------------------------------------------------------

clock_t
clockTurbo (void)
{
  struct tms Buf;
  clock_t t;
  t = times (&Buf);
  if (!clockInitialized)
    {
      InitialTime = t;
      clockInitialized = 1;
    }
  return (t - InitialTime);
}

//-----------------------------------------------------------------------

void
delay (guint milliseconds)
{
  struct tms Buf;
  clock_t EndTime;
  EndTime = times (&Buf) + (milliseconds * 1LL * CLK_TCK) / 1000LL;
  while (times (&Buf) < EndTime);
}
