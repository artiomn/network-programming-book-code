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

  Filename:	setrgbpalette.c
  Purpose:	A Borland Turbo C graphics.h function replacement,
  		for porting to *nix.
  Mod history:	05/04/02 RSB	Created.
*/

#ifdef WITH_X

#include "graphics.h"

//----------------------------------------------------------------------------

void
setrgbpalette (int colornum, int red, int green, int blue)
{
  if (colornum < 0)
    {
    Error:
      TcGraphResult = grError;
      return;
    }
  switch (TcPaletteNum)
    {
    case TC_PAL_16:
      if (colornum > 15)
	goto Error;
      break;
    case TC_PAL_256:
      if (colornum > 255)
	goto Error;
      break;
    default:
      goto Error;
    }
  TcColors[colornum].r = red * 0x100;
  TcColors[colornum].g = green * 0x100;
  TcColors[colornum].b = blue * 0x100;
  if (colornum == TcForegroundColor)
    setcolor (colornum);
  if (colornum == TcBackgroundColor)
    setfillstyle (TcFillStyle, TcFillColor);
}

#endif // WITH_X
