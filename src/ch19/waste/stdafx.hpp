/*
WASTE - stdafx.hpp (precompiled header)
Copyright (C) 2003 Nullsoft, Inc.
Copyright (C) 2004 WASTE Development Team

WASTE is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

WASTE  is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with WASTE; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/
#ifdef _WIN32

#pragma once

#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <windowsx.h>
#include <richedit.h>
#include <stdio.h>
#include <stdarg.h>
#include <signal.h>
#include <process.h>
#include <time.h>
#include <commctrl.h>
#include <shlobj.h>
#include <ctype.h>
#include <crtdbg.h>
#include <string.h>
#include <assert.h>

#define sprintf wsprintf

inline LPTSTR safe_strncpy(char* lpDest, const char* lpSrc,int iMaxBufLength)
{
	return lstrcpyn(lpDest,lpSrc,iMaxBufLength);
}

#define DIRCHAR '\\'

#ifndef ASSERT
	#ifdef _DEBUG
		#define ASSERT(x) assert(x)
	#else
		#define ASSERT(x) ((void)0)
	#endif
#endif

#ifndef VERIFY
	#ifdef _DEBUG
		#define VERIFY(x) (ASSERT(x),(x))
	#else
		#define VERIFY(x) (x)
	#endif
#endif

#ifdef _DEBUG
	#define DEBUG_NEW new( _NORMAL_BLOCK, __FILE__, __LINE__)
	#define malloc(x) _malloc_dbg( x, _NORMAL_BLOCK, __FILE__, __LINE__)
	#define new DEBUG_NEW
	#define strdup(x) dbgstrdup(x, __FILE__, __LINE__)
	char* dbgstrdup(const char*str,const char *file, unsigned int line);
#else
	#define DEBUG_NEW new
#endif // _DEBUG

#endif
