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

  Filename:	arc.c
  Purpose:	A Borland Turbo C graphics.h function replacement,
  		for porting to *nix.
  Mod history:	05/01/02 RSB	Created.
  		05/21/02 RSB	Fixed pieslice and fillellipse to use
				fill pattern/color rather than 
				solidly fill with the foreground color.
		05/25/02 RSB	Added the sector function.
		05/27/02 RSB	Split off all of the functions previously
				contained in this file (other than arc
				itself) to their own files.				
*/

#ifdef WITH_X

#include "graphics.h"

//----------------------------------------------------------------------------

void
arc (int x, int y, int stangle, int endangle, int radius)
{
  TcLastArcX = x;
  TcLastArcY = y;
  TcLastStangle = stangle;
  TcLastEndangle = endangle;
  TcLastRadius = radius;
  ellipse (x, y, stangle, endangle, radius, radius);
}

#endif // WITH_X
