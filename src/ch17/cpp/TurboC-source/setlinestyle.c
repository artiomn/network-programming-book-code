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

  Filename:	setlinestyle.c
  Purpose:	A Borland Turbo C graphics.h function replacement,
  		for porting to *nix.
  Mod history:	05/10/02 RSB	Created.
*/

#ifdef WITH_X

#include "graphics.h"

//----------------------------------------------------------------------------

void
setlinestyle (int linestyle, unsigned upattern, int thickness)
{
  gint line_style, dash_offset = 0, n, i, j, k;
  unsigned OrigUpattern;
  char dash_list[18];
  OrigUpattern = upattern;
  switch (linestyle)
    {
    case SOLID_LINE:
      line_style = LineSolid;
      break;
    case DOTTED_LINE:
      line_style = LineOnOffDash;
      n = 2;
      dash_offset = 0;
      dash_list[0] = thickness;
      dash_list[1] = thickness * 3;
      break;
    case CENTER_LINE:
      line_style = LineOnOffDash;
      n = 4;
      dash_offset = 0;
      dash_list[0] = 6 * thickness;
      dash_list[1] = 3 * thickness;
      dash_list[2] = 3 * thickness;
      dash_list[3] = 3 * thickness;
      break;
    case DASHED_LINE:
      line_style = LineOnOffDash;
      n = 2;
      dash_offset = 0;
      dash_list[0] = 5 * thickness;
      dash_list[1] = 3 * thickness;
      break;
    case USERBIT_LINE:
      line_style = LineOnOffDash;
      dash_offset = 0;
      if (upattern == 0)
	{
	  n = 2;
	  dash_list[0] = 1;
	  dash_list[1] = 7;
	}
      else
	{
	  while (0 == (upattern & 0x8000))
	    upattern = upattern << 1;
	  n = 0;
	  i = ((upattern & 0x8000) == 0);
	  if (i)
	    {
	      dash_list[0] = 0;
	      n++;
	    }
	  dash_list[n] = 0;
	  for (j = 0; j < 16; j++, upattern = upattern << 1)
	    {
	      k = ((upattern & 0x8000) == 0);
	      if (i == k)
		dash_list[n]++;
	      else
		{
		  i = k;
		  n++;
		  dash_list[n] = 1;
		}
	    }
	  n++;
	  if ((n & 1) != 0)
	    dash_list[n++] = 1;
	}
      break;
    default:
      TcGraphResult = grError;
      return;
    }
  TcLinestyle.linestyle = linestyle;
  TcLinestyle.upattern = OrigUpattern;
  TcLinestyle.thickness = thickness;
  XLockDisplay (TcDisplay);
  XSetLineAttributes (TcDisplay, TcGc, thickness, line_style, CapRound,
		      JoinRound);
  if (line_style == LineOnOffDash)
    XSetDashes (TcDisplay, TcGc, dash_offset, dash_list, n);
  XUnlockDisplay (TcDisplay);
}

//---------------------------------------------------------------------------

void
getlinesettings (struct linesettingstype *lineinfo)
{
  *lineinfo = TcLinestyle;
}

#endif // WITH_X
