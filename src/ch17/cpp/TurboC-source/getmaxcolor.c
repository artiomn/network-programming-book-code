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

  Filename:	getmaxcolor.c
  Purpose:	A Borland Turbo C graphics.h function replacement,
  		for porting to *nix.
  Mod history:	05/24/02 RSB	Created.
*/

#ifdef WITH_X

#include "graphics.h"

//----------------------------------------------------------------------------

extern int
getmaxcolor (void)
{
  switch (TcPaletteNum)
    {
    case TC_PAL_2:
      return (1);
    case TC_PAL_4:
    case TC_PAL_C0:
    case TC_PAL_C1:
    case TC_PAL_C2:
    case TC_PAL_C3:
      return (3);
    case TC_PAL_16:
      return (15);
    case TC_PAL_256:
      return (255);
    default:
      break;
    }
  // Shouldn't be possible to get here, but still ...
  return (0);
}

//----------------------------------------------------------------------------

extern int
getpalettesize (void)
{
  return (getmaxcolor () + 1);
}

#endif // WITH_X
