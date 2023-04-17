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

  Filename:	setcolor.c
  Purpose:	A Borland Turbo C graphics.h function replacement,
  		for porting to *nix.
  Mod history:	05/04/02 RSB	Created.
*/

#ifdef WITH_X

#include "graphics.h"

//----------------------------------------------------------------------------

extern void
setcolor (int color)
{
  XColor Color;
  if (color < 0)
    {
    Error:
      TcGraphResult = grError;
      return;
    }
  Color.pixel = 0;
  Color.flags = DoRed | DoGreen | DoBlue;
  Color.pad = 0;
  switch (TcPaletteNum)
    {
    case TC_PAL_2:
      if (color != 1)
	goto Error;
      break;
    case TC_PAL_4:
    case TC_PAL_C0:
    case TC_PAL_C1:
    case TC_PAL_C2:
    case TC_PAL_C3:
      if (color > 3)
	goto Error;
      break;
    case TC_PAL_16:
      if (color > 15)
	goto Error;
      break;
    case TC_PAL_256:
      if (color > 255)
	goto Error;
      break;
    default:
      goto Error;
    }
  Color.red = TcColors[color].r;
  Color.green = TcColors[color].g;
  Color.blue = TcColors[color].b;
  XLockDisplay (TcDisplay);
  XAllocColor (TcDisplay, TcColormap, &Color);
  TcForegroundColor = color;
  TcXforeground = Color;
  XSetForeground (TcDisplay, TcGc, Color.pixel);
  XUnlockDisplay (TcDisplay);
}

//----------------------------------------------------------------

int
getcolor (void)
{
  return (TcForegroundColor);
}

#endif // WITH_X
