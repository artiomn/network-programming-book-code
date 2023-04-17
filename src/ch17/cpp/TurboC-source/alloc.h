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

  Filename:	alloc.h
  Purpose:	Partial replacement for Borland Turbo C alloc.h file.
  Mod history:	02/03/02 RSB	Created.
  		03/02/02 RSB	Added TurboC.h.
		05/24/02 RSB	Allowed for my own custom versin of free.
*/

#ifndef _INCLUDED_ALLOC_H
#define _INCLUDED_ALLOC_H

#include "TurboC.h"

// Heap functions that have been renamed.
#define farmalloc malloc
#define farcalloc calloc
#define farfree freeTurbo
#define farcoreleft() 0x80000

#endif // _INCLUDED_ALLOC_H
