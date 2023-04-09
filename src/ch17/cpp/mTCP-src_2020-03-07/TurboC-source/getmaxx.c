/*
  TurboC, a library for porting Borland Turbo C to GNU gcc.
  Copyright 2002,2010 Ronald S. Burkey
 
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

  Filename:		getmaxx.c
  Purpose:		A Borland Turbo C graphics.h function replacement,
				for porting to *nix.
  Mod history:	05/01/02 RSB	Created.
				08/28/10 RSB	Changed the function names from
								getmaxx() and getmaxy() to
								turboc_getmaxx() and turboc_getmaxy(),
								with macros relating the new names
								to the old in graphics.h.  This is
								because (thanks to Dick Borrett) I've
								found that function with this name
								are now present in distros like
								Ubuntu 9.x and 10.x.

*/

#ifdef WITH_X

#include "graphics.h"

//----------------------------------------------------------------------------
// This is just a quick and dirty version that doesn't account for viewports

int
turboc_getmaxx (void)
{
  return (TcXresolution - 1);
}

int
turboc_getmaxy (void)
{
  return (TcYresolution - 1);
}

#endif // WITH_X
