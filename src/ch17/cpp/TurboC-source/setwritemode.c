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

  Filename:	setwritemode.c
  Purpose:	A Borland Turbo C graphics.h function replacement,
  		for porting to *nix.
  Mod history:	05/10/02 RSB	Created.
  		05/23/02 RSB	Added a bunch more modes, not technically
				supported by setwritemode, but needed for
				putimage.
*/

#ifdef WITH_X

#include "graphics.h"

//----------------------------------------------------------------------------

void
setwritemode (int mode)
{
  XGCValues Values;
  switch (mode)
    {
    case COPY_PUT:
      Values.function = GXcopy;
      break;
    case XOR_PUT:
      Values.function = GXxor;
      break;
    case OR_PUT:
      Values.function = GXor;
      break;
    case AND_PUT:
      Values.function = GXand;
      break;
    case NOT_PUT:
      Values.function = GXinvert;
      break;
    default:
      return;
    }
  TcWritemode = mode;
  XLockDisplay (TcDisplay);
  XChangeGC (TcDisplay, TcGc, GCFunction, &Values);
  XUnlockDisplay (TcDisplay);
}

#endif // WITH_X
