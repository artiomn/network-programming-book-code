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

  Filename:	textwidth.c
  Purpose:	A Borland Turbo C graphics.h function replacement,
  		for porting to *nix.
  Mod history:	06/08/02 RSB  Created.
  		12/16/02 RSB  Added a warning that should show up at
			      compile time about stroke-fonts not being
			      implemented.
		08/28/10 RSB  Eliminated some compiler warnings.
*/

#ifdef WITH_X

#include "graphics.h"

//----------------------------------------------------------------------------

int
textwidth (char * textstring)
{
  // Handle the bitmapped font separately, since it's so easy.
  if (TcTextFont == DEFAULT_FONT)
    return (8 * strlen (textstring) * TcTextCharsize);
  // Handle the stroke fonts here.
#warning Stroke fonts not implemented yet. Ignore warnings about textwidth.
  return (0);
}

//----------------------------------------------------------------------------

int
textheight (char * textstring)
{
  // Handle the bitmapped font separately, since it's so easy.
  if (TcTextFont == DEFAULT_FONT)
    return (8 * TcTextCharsize);
  // Handle the stroke fonts here.
#warning Stroke fonts not implemented yet. Ignore warnings about textheight.
  return (0);
}

#endif // WITH_X
