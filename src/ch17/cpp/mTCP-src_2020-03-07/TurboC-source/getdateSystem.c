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

  Filename:	getdateSystem.h
  Purpose:	Newer system libraries, but not older ones, have a function
  		called getdate that conflicts with the TurboC library function.
		The function getdateSystem is a manoeuvre to get around this. 
  Mod history:	12/16/02 RSB	Created.
*/

#define _XOPEN_SOURCE
#define _XOPEN_SOURCE_EXTENDED
#include <time.h>

struct tm *
getdateSystem (const char *d)
{
#warning If warning messages about getdate appear, ignore them.
  return (getdate (d));
}

