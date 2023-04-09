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

  Filename:	setfillXXXX.c
  Purpose:	A Borland Turbo C graphics.h function replacement,
  		for porting to *nix.
  Mod history:	05/20/02 RSB	Created.
  		05/21/02 RSB	Corrected pattern arrays to correspond
				more closely to Turbo C patterns.
*/

#ifdef WITH_X

#include "graphics.h"

//----------------------------------------------------------------------------
// This function creates an 8x8 tile for use by X, from an 8-character
// array compatible with Turbo C's setfillpattern.  This is stored in the
// pixmap TcTile.

static void
RawTile (char *pattern)
{
  int i, j, k, Match;
  int OldColor;
  OldColor = TcForegroundColor;
  // This loop draws  all of the background-color pixels on the first
  // pass, and all of the fill-color pixels on the second.
  for (Match = 0; Match < 0x100; Match += 0x80)
    {
      if (Match)
	setcolor (TcFillColor);
      else
	setcolor (TcBackgroundColor);
      for (i = 0; i < 8; i++)
	{
	  k = pattern[i];
	  for (j = 0; j < 8; j++)
	    {
	      if (Match == (k & 0x80))
		XDrawPoint (TcDisplay, TcTile, TcGc, j, i);
	      k = k << 1;
	    }
	}
    }
  setcolor (OldColor);
}

//----------------------------------------------------------------------------

void
setfillpattern (char *upattern, int color)
{
  int i;
  for (i = 0; i < 8; i++)
    TcUserFillPattern[i] = upattern[i];
  TcFillColor = color;
  TcFillStyle = USER_FILL;
  RawTile (upattern);
}

//---------------------------------------------------------------------------

void
getfillpattern (char *upattern)
{
  int i;
  for (i = 0; i < 8; i++)
    upattern[i] = TcUserFillPattern[i];
}

//---------------------------------------------------------------------------

// These are setfillpattern-compatible arrays for all of the tiling patterns.
static char FillStyles[12][8] = {
  {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
  {0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff},
  {0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00},
  {0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80},
  {0x03, 0x06, 0x0c, 0x18, 0x30, 0x60, 0xc0, 0x81},
  {0xc0, 0x60, 0x30, 0x18, 0x0c, 0x06, 0x03, 0x81},
  {0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01},
  {0x22, 0x22, 0xff, 0x22, 0x22, 0x22, 0xff, 0x22},
  {0x81, 0x42, 0x24, 0x18, 0x18, 0x24, 0x42, 0x81},
  {0x11, 0x44, 0x11, 0x44, 0x11, 0x44, 0x11, 0x44},
  {0x10, 0x00, 0x01, 0x00, 0x10, 0x00, 0x01, 0x00},
  {0x11, 0x00, 0x44, 0x00, 0x11, 0x00, 0x44, 0x00}
};

void
setfillstyle (int pattern, int color)
{
  // An input value of USER_FILL isn't actually legal, but it's convenient
  // for us to use it (in conjunction with setbkcolor).
  if (pattern < 0 || pattern > USER_FILL)
    {
      TcGraphResult = grError;
      return;
    }
  TcFillColor = color;
  TcFillStyle = pattern;
  if (pattern == USER_FILL)
    RawTile (TcUserFillPattern);
  else
    RawTile (&FillStyles[pattern][0]);
}

//---------------------------------------------------------------------------

void
getfillsettings (struct fillsettingstype *fillinfo)
{
  fillinfo->pattern = TcFillStyle;
  fillinfo->color = TcFillColor;
}

#endif // WITH_X
