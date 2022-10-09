/*
WASTE - asyncdns.hpp (asynchronous DNS class)
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

#ifndef _C_ASYNCDNS_H_
#define _C_ASYNCDNS_H_

class C_AsyncDNS
{
public:
	C_AsyncDNS(int max_cache_entries=32);
	~C_AsyncDNS();

	int resolve(char *hostname, unsigned long *addr); //return 0 on success, 1 on wait, -1 on unresolvable

protected:
	struct cache_entry
	{
		int last_used; //timestamp.
		int resolved;
		char hostname[256];
		unsigned long addr;
	};

	cache_entry *m_cache;
	int m_cache_size;
	volatile int m_thread_kill;
#ifdef _WIN32
	HANDLE m_thread;
	static unsigned long WINAPI _threadfunc(LPVOID _d);
#else
	pthread_t m_thread;
	static void *_threadfunc(void *_d);
#endif
	void makesurethreadisrunning();

};

#endif //_C_ASYNCDNS_H_
