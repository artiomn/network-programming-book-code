/*
WASTE - sockets.hpp (some socket portability defines)
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

/*
-- this really should be in platform.h, but oh well :)
ADDED Md5Chap - This should not even be in platform, there should
	be fucking platform base classes where WASTE is built upon.
	There should also be no god damned dependency of data layer code to the gui.
	This makes a full linux port very unhandy..
*/

#ifndef _SOCKETS_H_
#define _SOCKETS_H_

#ifdef _WIN32

#define ERRNO (WSAGetLastError())
#define SET_SOCK_BLOCK(s,block) { unsigned long __i=block?0:1; ioctlsocket(s,FIONBIO,&__i); }
#define EWOULDBLOCK WSAEWOULDBLOCK
#define EINPROGRESS WSAEWOULDBLOCK

#else
#define ERRNO errno
#define closesocket(s) close(s)
#define SET_SOCK_BLOCK(s,block) { int __flags; if ((__flags = fcntl(s, F_GETFL, 0)) != -1) {  if (!block) __flags |= O_NONBLOCK; else __flags &= ~O_NONBLOCK; fcntl(s, F_SETFL, __flags); } }

#endif

#ifndef SHUT_RDWR
#define SHUT_RDWR 2
#endif

#endif //_SOCKETS_H_
