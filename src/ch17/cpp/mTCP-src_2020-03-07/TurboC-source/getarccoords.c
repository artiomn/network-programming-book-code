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

  Filename:	getarccoords.c
  Purpose:	A Borland Turbo C graphics.h function replacement,
  		for porting to *nix.
  Mod history:	05/01/02 RSB	Created.
*/

#ifdef WITH_X

#include <math.h>
#include "graphics.h"

//---------------------------------------------------------------------------

void
getarccoords (struct arccoordstype *last)
{
  last->x = TcLastArcX;
  last->y = TcLastArcY;
  last->xstart =
    TcLastArcX + TcLastRadius * cos (RADIANS_PER_DEGREE * TcLastStangle);
  last->ystart =
    TcLastArcY + TcLastRadius * sin (RADIANS_PER_DEGREE * TcLastStangle);
  last->xend =
    TcLastArcX + TcLastRadius * cos (RADIANS_PER_DEGREE * TcLastEndangle);
  last->yend =
    TcLastArcY + TcLastRadius * sin (RADIANS_PER_DEGREE * TcLastEndangle);
}

#endif // WITH_X
