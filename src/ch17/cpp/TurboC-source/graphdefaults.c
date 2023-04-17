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

  Filename:	graphdefaults.c
  Purpose:	A Borland Turbo C graphics.h function replacement,
  		for porting to *nix.
  Mod history:	05/25/02 RSB	Created.
*/

#ifdef WITH_X

#include "graphics.h"

//------------------------------------------------------------------------

void
graphdefaults (void)
{
  TcDefaultColors (TcPaletteNum);
  TcGraphDefaults ();
}

//------------------------------------------------------------------------

void
TcGraphDefaults (void)
{
  setviewport (0, 0, getmaxx (), getmaxy (), 1);
  setcolor (getmaxcolor ());
  setlinestyle (SOLID_LINE, 0, NORM_WIDTH);
  settextstyle (DEFAULT_FONT, HORIZ_DIR, 1);
  settextjustify (LEFT_TEXT, TOP_TEXT);
  setfillstyle (SOLID_FILL, getmaxcolor ());
  setbkcolor (0);
  setusercharsize (1, 1, 1, 1);
}

//------------------------------------------------------------------------
// Sets the default colors for a given palette type.  Returns 0 on success.

int
TcDefaultColors (int Pal)
{
  int i;
  switch (Pal)
    {
    case TC_PAL_2:
      TcCurrentPalette = TcDefaultPalette2;
      TcForegroundColor = 1;
      for (i = 0; i <= TcForegroundColor; i++)
	TcColors[i] = TcColors2[i];
      break;
    case TC_PAL_C0:
      TcCurrentPalette = TcDefaultPaletteC0;
      TcForegroundColor = 3;
      for (i = 0; i <= TcForegroundColor; i++)
	TcColors[i] = TcColorsC0[i];
      break;
    case TC_PAL_C1:
      TcCurrentPalette = TcDefaultPaletteC1;
      TcForegroundColor = 3;
      for (i = 0; i <= TcForegroundColor; i++)
	TcColors[i] = TcColorsC1[i];
      break;
    case TC_PAL_C2:
      TcCurrentPalette = TcDefaultPaletteC2;
      TcForegroundColor = 3;
      for (i = 0; i <= TcForegroundColor; i++)
	TcColors[i] = TcColorsC2[i];
      break;
    case TC_PAL_C3:
      TcCurrentPalette = TcDefaultPaletteC3;
      TcForegroundColor = 3;
      for (i = 0; i <= TcForegroundColor; i++)
	TcColors[i] = TcColorsC3[i];
      break;
    case TC_PAL_4:
      TcCurrentPalette = TcDefaultPalette4;
      TcForegroundColor = 3;
      for (i = 0; i <= TcForegroundColor; i++)
	TcColors[i] = TcColors4[i];
      break;
    case TC_PAL_16:
      TcCurrentPalette = TcDefaultPalette16;
      TcForegroundColor = 15;
      for (i = 0; i <= TcForegroundColor; i++)
	TcColors[i] = TcColors16[i];
      break;
    case TC_PAL_256:
      TcForegroundColor = 15;
      for (i = 0; i <= TcForegroundColor; i++)
	TcColors[i] = TcColors16[i];
      for (; i < 256; i++)
	TcColors[i] = TcColors16[0];
      break;
    default:
      return (1);
    }
  return (0);  
}    
    
#endif // WITH_X
