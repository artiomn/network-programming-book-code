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

  Filename:	settextstyle.c
  Purpose:	A Borland Turbo C graphics.h function replacement,
  		for porting to *nix.
  Mod history:	05/16/02 RSB	Created.
  		05/25/02 RSB	Added settextjustify.
		06/08/02 RSB	Accounted for fonts 5-10.
*/

#ifdef WITH_X

#include "graphics.h"

//-----------------------------------------------------------------

void
settextstyle (int font, int direction, int charsize)
{
  if (font < 0 || font >= MAX_FONT)
    return;
  if (direction < 0 || direction > 1)
    return;
  if (charsize < 0 || charsize > 10)
    return;
  TcTextFont = font;
  TcTextDirection = direction;
  TcTextCharsize = charsize;

}

//----------------------------------------------------------------

void
setusercharsize (int multx, int divx, int multy, int divy)
{
  if (multx <= 0 || multy <= 0 || divx <= 0 || divy <= 0)
    return;
  TcTextMultX = multx;
  TcTextDivX = divx;
  TcTextMultY = multy;
  TcTextDivY = divy;

}

//-----------------------------------------------------------------

void
settextjustify (int horiz, int vert)
{
  if (horiz < 0 || horiz > 2 || vert < 0 || vert > 2)
    {
      TcGraphResult = grError;
      return;
    }
  TcTextHoriz = horiz;
  TcTextVert = vert;  
}


#endif // WITH_X
