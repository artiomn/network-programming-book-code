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

  Filename:	endianFile.c
  Purpose:	Writes/reads integers of selected endian format to/from files. 
  Mod history:	03/04/02 RSB	Created.
*/

#include <stdio.h>
#include "TurboC.h"

//-------------------------------------------------------------------------
// The reading functions.  All return 0 on success.

int
ReadLittle16 (FILE * fp, uint16_t * Value)
{
  int RetVal;
  RetVal = (1 != fread (Value, sizeof (*Value), 1, fp));
  if (0 == RetVal)
    FixLittle16 (Value);
  return (RetVal);
}

int
ReadBig16 (FILE * fp, uint16_t * Value)
{
  int RetVal;
  RetVal = (1 != fread (Value, sizeof (*Value), 1, fp));
  if (0 == RetVal)
    FixBig16 (Value);
  return (RetVal);
}

int
ReadLittle32 (FILE * fp, uint32_t * Value)
{
  int RetVal;
  RetVal = (1 != fread (Value, sizeof (*Value), 1, fp));
  if (0 == RetVal)
    FixLittle32 (Value);
  return (RetVal);
}

int
ReadBig32 (FILE * fp, uint32_t * Value)
{
  int RetVal;
  RetVal = (1 != fread (Value, sizeof (*Value), 1, fp));
  if (0 == RetVal)
    FixBig32 (Value);
  return (RetVal);
}

//------------------------------------------------------------------------
// The writing functions.  All return 0 on success.

int
WriteLittle16 (FILE * fp, uint16_t Value)
{
  FixLittle16 (&Value);
  return (1 != fwrite (&Value, sizeof (Value), 1, fp));
}

int
WriteBig16 (FILE * fp, uint16_t Value)
{
  FixBig16 (&Value);
  return (1 != fwrite (&Value, sizeof (Value), 1, fp));
}

int
WriteLittle32 (FILE * fp, uint32_t Value)
{
  FixLittle32 (&Value);
  return (1 != fwrite (&Value, sizeof (Value), 1, fp));
}

int
WriteBig32 (FILE * fp, uint32_t Value)
{
  FixBig32 (&Value);
  return (1 != fwrite (&Value, sizeof (Value), 1, fp));
}
