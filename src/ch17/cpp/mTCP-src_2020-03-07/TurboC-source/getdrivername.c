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

  Filename:	getdrivername.c
  Purpose:	A Turbo C conio function.
  Mod history:	05/21/02 RSB	Created.
*/

#include "graphics.h"

static const char *DriverNames[11] = {
  NULL, "CGA", "CGA", "EGAVGA", "EGAVGA", "EGAVGA",
  "IBM8514", "HERC", "ATT", "EGAVGA", "PC3270"
};

//-------------------------------------------------------------------------

const char *
getdrivername (void)
{
  if (TcDriver < 1 || TcDriver > 10)
    return (NULL);
  return (DriverNames[TcDriver]);
}
