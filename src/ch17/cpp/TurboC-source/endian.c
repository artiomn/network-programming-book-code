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

  Filename:	endian.c
  Purpose:	Functions for converting the endian type of data. 
  Mod history:	03/03/02 RSB	Created.
  
  These are functions which have no purpose for simple CPU-based programs,
  but are needed when reading data to/from a file of a pre-defined format.
  This is typical in Turbo C programs; since they ran only on x86 
  processors, they implicitly assume a little-endian byte ordering
  for data files.
  
  For good measure, big-endian conversion functions are also thrown in, 
  though there's no particular reason for them in Turbo C porting
  (since this functionality would already have to have been handled 
  explicitly anyway -- though not necessarily CORRECTLY if the target
  processor isn't little-endian).
*/

#include "TurboC.h"
#define na ((uint8_t *) n)

//----------------------------------------------------------------------------

// Converts a 16-bit integer between little-endian and the natural CPU format.
void
FixLittle16 (uint16_t * n)
{
#if BYTE_ORDER == LITTLE_ENDIAN
#elif BYTE_ORDER == BIG_ENDIAN
  uint8_t c;
  c = na[0];
  na[0] = na[1];
  na[1] = c;
#else
#error Unsupported Endian type.
#endif
}

// Converts a 32-bit integer between little-endian and the natural CPU format.
void
FixLittle32 (uint32_t * n)
{
#if BYTE_ORDER == LITTLE_ENDIAN
#elif BYTE_ORDER == BIG_ENDIAN
  uint8_t c;
  c = na[0];
  na[0] = na[3];
  na[3] = c;
  c = na[1];
  na[1] = na[2];
  na[2] = c;
#else
#error Unsupported Endian type.
#endif
}


// Converts a 16-bit integer between big-endian and the natural CPU format.
void
FixBig16 (uint16_t * n)
{
#if BYTE_ORDER == LITTLE_ENDIAN
  uint8_t c;
  c = na[0];
  na[0] = na[1];
  na[1] = c;
#elif BYTE_ORDER == BIG_ENDIAN
#else
#error Unsupported Endian type.
#endif
}

// Converts a 32-bit integer between big-endian and the natural CPU format.
void
FixBig32 (uint32_t * n)
{
#if BYTE_ORDER == LITTLE_ENDIAN
  uint8_t c;
  c = na[0];
  na[0] = na[3];
  na[3] = c;
  c = na[1];
  na[1] = na[2];
  na[2] = c;
#elif BYTE_ORDER == BIG_ENDIAN
#else
#error Unsupported Endian type.
#endif
}
