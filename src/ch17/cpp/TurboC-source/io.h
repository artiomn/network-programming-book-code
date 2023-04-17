/*
  TurboC, a library for porting Borland Turbo C to GNU gcc.
  Copyright 2002 Ronald S. Burkey
  Items so marked are Copyright 1999-2002 Igor Bujna.
 
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
  Contact Igor Bujna <igor.bujna@maxi-tip.cz>

  Filename:	io.h
  Purpose:	A header file for TurboC. 
  Mod history:	03/02/02 RSB	Created.
  		03/18/02 RSB	Modified for C++.
		05/19/02 RSB	Added items contributed by Igor Bujna.
*/

#ifndef _INCLUDED_IO_H
#define _INCLUDED_IO_H

#include <sys/cdefs.h>
#include "TurboC.h"

//----------------------------------------------------------------------------
// Constants.

//----------------------------------------------------------------------------
// Data types.

struct ftime
{
  unsigned ft_tsec:5;
  unsigned ft_min:6;
  unsigned ft_hour:5;
  unsigned ft_day:5;
  unsigned ft_month:4;
  unsigned ft_year:7;
};

//----------------------------------------------------------------------------
// Function prototypes.

__BEGIN_DECLS 

//
extern int getftime (int handle, struct ftime *ftimep);
extern int setftime (int handle, struct ftime *ftimep);

__END_DECLS
//--------------------------------------------------------------------------
// Variables.
// (Items contributed by Igor Bujna, edited by RSB) ------------------------
/*
 * File (I/O) function
 */
#if defined (__CYGWIN__) || defined(__linux__)
#define SH_DENYNO	0
#define SH_COMPAT	0
#endif
#ifdef __linux__
#define O_TEXT		0
#define O_BINARY	0
#endif
#if defined (__CYGWIN__) || defined(__linux__)
/* We need mode to 0644 */
#define sopen(path, access, shflag, mode) \
	      open(path, (access)|(shflag), 0660)
#endif
// (End Igor Bujna) --------------------------------------------------------
#endif // _INCLUDED_IO_H
