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

  Filename:	setpalette.c
  Purpose:	A Borland Turbo C graphics.h function replacement,
  		for porting to *nix.
  Mod history:	05/05/02 RSB	Created.
  		05/25/02 RSB	Added setallpalette.
*/

#ifdef WITH_X

#include "graphics.h"

//----------------------------------------------------------------------------

void
setpalette (int colornum, int color)
{
  int rawcolor;
  unsigned Scale;
  XColor Color;
  rawcolor = color;
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
      goto Error;
    case TC_PAL_C0:
    case TC_PAL_C1:
    case TC_PAL_C2:
    case TC_PAL_C3:
      if (colornum != 0 || color > 15)
	goto Error;
      setbkcolor (color);
      return;
    case TC_PAL_4:
      if (colornum < 0 || colornum > 3 || color > 63)
	goto Error;
      goto Palette16;
    case TC_PAL_16:
      if (colornum < 0 || colornum > 15 || color > 63)
	goto Error;
    Palette16:
      // We *assume* that the allowable colors (0-63) are just
      // scaled versions of the usual 16 colors.
      Scale = 0x40 * (1 + (color / 16));
      color = color % 16;
      TcColors[colornum].r = (TcColors16[color].r / 0x100) * Scale;
      TcColors[colornum].g = (TcColors16[color].g / 0x100) * Scale;
      TcColors[colornum].b = (TcColors16[color].b / 0x100) * Scale;
      if (colornum == 0)
	{
	  Color.red = TcColors[0].r;
	  Color.green = TcColors[0].g;
	  Color.blue = TcColors[0].b;
	  XAllocColor (TcDisplay, TcColormap, &Color);
	  TcBackgroundColor = color;
	  XSetBackground (TcDisplay, TcGc, Color.pixel);
	}
      else
	setcolor (colornum);
      break;
    case TC_PAL_256:
      goto Error;
    default:
      goto Error;
    }
  TcCurrentPalette.colors[colornum] = rawcolor;
}

//--------------------------------------------------------------------

void
setallpalette (struct palettetype *palette)
{
  int i;
  TcGraphResult = grOk;
  for (i = 0; i < palette->size; i++)
    {
      setpalette (i, palette->colors[i]);
      if (TcGraphResult != grOk)
        return;
    }
}

#endif // WITH_X
