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

  Filename:	getftime.h
  Purpose:	A function for for TurboC. 
  Mod history:	03/02/02 RSB	Created, and disabled.
  		03/23/02 RSB	Re-enabled getftime and slightly corrected
				the tm_mon/tm_mday fields.  For some reason, 
				when I had initially implemented this,
				the linker could not find the fstat function,
				which I then assumed did not actually
				exist.  Works now, though.  Go figure!
*/

#include <time.h>
#include <sys/stat.h>
#include "io.h"

//----------------------------------------------------------------------------

int
getftime (int handle, struct ftime *ftimep)
{
#if 1
  int32_t ReturnValue;
  struct stat buf;
  struct tm *ModTime;
  ReturnValue = fstat (handle, &buf);
  if (ReturnValue)
    return (ReturnValue);
  ModTime = localtime (&buf.st_mtime);
  ftimep->ft_tsec = ModTime->tm_sec / 2;
  ftimep->ft_min = ModTime->tm_min;
  ftimep->ft_hour = ModTime->tm_hour;
  ftimep->ft_day = ModTime->tm_mday;
  ftimep->ft_month = ModTime->tm_mon + 1;
  ftimep->ft_year = ModTime->tm_year - 80;
  return (0);
#else
  ftimep->ft_tsec = 0;
  ftimep->ft_min = 0;
  ftimep->ft_hour = 0;
  ftimep->ft_day = 0;
  ftimep->ft_month = 0;
  ftimep->ft_year = 0;
  return (1);
#endif
}

//---------------------------------------------------------------------------

#if 0
int
setftime (int handle, struct ftime *ftimep)
{
  return (1);
}
#endif
