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

  Filename:	cprintf.c
  Purpose:	A Turbo C conio function.
  Mod history:	02/02/02 RSB	Created.
*/

#include <stdarg.h>
#include "conio.h"

static char StringBuffer[1024];

int
cprintf (const char *format, ...)
{
  va_list ap;
  int ReturnValue = 0;
  if (!ConioInitialized)
    textmode (LASTMODE);
  va_start (ap, format);
  ReturnValue = vsnprintf (StringBuffer, sizeof (StringBuffer), format, ap);
  va_end (ap);
  if (ReturnValue > 0)
    cputs (StringBuffer);
  return (ReturnValue);
}
