/*
WASTE - platform.hpp (Windows / Posixish platform includes and defines)
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

#ifndef _PLATFORM_H_
#define _PLATFORM_H_

#include "stdafx.hpp"

#ifndef _WIN32

#define THREAD_SAFE
#define _REENTRANT
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/time.h>
#include <arpa/inet.h>
#include <ifaddrs.h>
#include <net/if.h>
#include <netdb.h>
#include <stdarg.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>

#define DIRCHAR '/'
#define CharNext(x) (x+1)
#define CharPrev(s,x) ((s)<(x)?(x)-1:(s))
#define DeleteFile(x) unlink(x)
#define RemoveDirectory(x) rmdir(x)
#define CreateDirectory(x,y) mkdir(x,0755)
#define Sleep(x) usleep(( x )*1000)
#define stricmp(x,y) strcasecmp(x,y)
#define strnicmp(x,y,z) strncasecmp(x,y,z)
#define _vsnprintf vsnprintf
static inline char *safe_strncpy(char *out, const char *in, int maxl) { strncpy(out,in,maxl); out[maxl-1]=0; }

static inline unsigned int GetTickCount()
{
	struct timeval tv;
	gettimeofday(&tv,NULL);
	return (tv.tv_sec * 1000) + (tv.tv_usec/1000);
}

#define MAX_PATH 1024

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

#ifndef min
#define min(x,y) ((x)<(y)?(x):(y))
#define max(x,y) ((y)<(x)?(x):(y))
#endif

#endif //_WIN32

#endif //_PLATFORM_H_
