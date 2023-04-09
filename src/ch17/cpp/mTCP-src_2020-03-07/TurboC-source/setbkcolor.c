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

  Filename:	setbkcolor.c
  Purpose:	A Borland Turbo C graphics.h function replacement,
  		for porting to *nix.
  Mod history:	05/01/02 RSB	Created.
*/

#ifdef WITH_X

#include "graphics.h"

//----------------------------------------------------------------------------

void
setbkcolor (int color)
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
      Color.red = 0;
      Color.green = 0;
      Color.blue = 0;
      break;
    case TC_PAL_4:
    case TC_PAL_C0:
    case TC_PAL_C1:
    case TC_PAL_C2:
    case TC_PAL_C3:
      // I treat these all similarly to CGA 4-color palettes.
      // The Turbo C docs indicate somehow that in CGA "hi-res"
      // mode, the background and foreground colors are reversed.
      // I don't deal with this as of yet, because I don't grasp
      // what it means.
      if (color >= 16)
	goto Error;
      Color.red = TcColors16[color].r;
      Color.green = TcColors16[color].g;
      Color.blue = TcColors16[color].b;
      TcCurrentPalette.colors[0] = color;
      break;
    case TC_PAL_16:
      // I treat these all similarly to EGA 16-color palettes.
      if (color >= 16)
	goto Error;
      Color.red = TcColors[color].r;
      Color.green = TcColors[color].g;
      Color.blue = TcColors[color].b;
      if (TcDriver == EGA || TcDriver == EGA64)
	TcCurrentPalette.colors[0] = color;
      break;
    case TC_PAL_256:
      if (color >= 256)
	goto Error;
      Color.red = TcColors[color].r;
      Color.green = TcColors[color].g;
      Color.blue = TcColors[color].b;
      break;
    }
  XAllocColor (TcDisplay, TcColormap, &Color);
  TcBackgroundColor = color;
  TcXbackground = Color;
  XSetBackground (TcDisplay, TcGc, Color.pixel);
  // The background color will now presumably be inconsistent with
  // any tile that has previously been generated for fills.  We therefore
  // need to regenerate the tile.  This same problem *does not*
  // exist for setcolor, since fills don't use the foreground color.
  setfillstyle (TcFillStyle, TcFillColor);
}

//----------------------------------------------------------------

int
getbkcolor (void)
{
  return (TcBackgroundColor);
}

#endif // WITH_X
