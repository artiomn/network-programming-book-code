/*
WASTE - asyncdns.cpp (asynchronous DNS class)
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

#include "stdafx.hpp"

#include "platform.hpp"
#include "sockets.hpp"
#include "asyncdns.hpp"
#include "util.hpp"

C_AsyncDNS::C_AsyncDNS(int max_cache_entries)
{
	m_thread_kill=1;
	m_thread=0;
	m_cache_size=max_cache_entries;
	m_cache=(cache_entry *)::malloc(sizeof(cache_entry)*m_cache_size);
	memset(m_cache,0,sizeof(cache_entry)*m_cache_size);
}

C_AsyncDNS::~C_AsyncDNS()
{
	m_thread_kill=1;

	#ifdef _WIN32
		if (m_thread) {
			WaitForSingleObject(m_thread,INFINITE);
			CloseHandle(m_thread);
		};
	#else
		if (m_thread) {
			void *p;
			pthread_join(m_thread,&p);
		};
	#endif
	if (m_cache) {
		free(m_cache);
		m_cache=0;
	};
}

#ifdef _WIN32
	unsigned long WINAPI C_AsyncDNS::_threadfunc(LPVOID _d)
#else
	void *C_AsyncDNS::_threadfunc(void *_d)
#endif
{
	int nowinsock=0;
	#ifdef _WIN32
		WSADATA wsaData;
		if (WSAStartup(MAKEWORD(2, 0), &wsaData)) {
			dbg_printf(ds_Debug,"C_AsyncDNS::_threadfunc: error initializing winsock!");
			nowinsock=1;
		};
	#endif
	C_AsyncDNS *_this=(C_AsyncDNS*)_d;
	int x;

	for (x = 0; x < _this->m_cache_size && !_this->m_thread_kill; x ++) {
		if (_this->m_cache[x].last_used &&
			!_this->m_cache[x].resolved &&
			_this->m_cache[x].hostname[0])
		{
			if (!nowinsock) {
				struct hostent *hostentry;
				dbg_printf(ds_Debug,"C_AsyncDNS::_threadfunc: calling gethostbyname(%s)!",_this->m_cache[x].hostname);
				hostentry=::gethostbyname(_this->m_cache[x].hostname);
				_this->m_cache[x].resolved=1;
				if (hostentry) {
					_this->m_cache[x].addr=*((int*)hostentry->h_addr);
				}
				else
					_this->m_cache[x].addr=INADDR_NONE;
			}
			else {
				_this->m_cache[x].resolved=1;
				_this->m_cache[x].addr=INADDR_NONE;
			};
		};
	}
	#ifdef _WIN32
		if (!nowinsock) WSACleanup();
	#endif
	_this->m_thread_kill=1;
	return 0;
}

int C_AsyncDNS::resolve(char *hostname, unsigned long *addr)
{
	//return 0 on success, 1 on wait, -1 on unresolvable
	int x;
	for (x = 0; x < m_cache_size; x ++) {
		if (!stricmp(m_cache[x].hostname,hostname)) {
			m_cache[x].last_used=time(NULL);
			if (m_cache[x].resolved) {
				if (m_cache[x].addr == INADDR_NONE) {
					return -1;
				};
				struct in_addr in;
				in.s_addr=m_cache[x].addr;
				dbg_printf(ds_Debug,"C_AsyncDNS: resolved %s to %s!",hostname,inet_ntoa(in));
				*addr=m_cache[x].addr;
				return 0;
			};
			makesurethreadisrunning();
			return 1;
		};
	};
	//add to resolve list
	int oi=-1;
	for (x = 0; x < m_cache_size; x ++) {
		if (!m_cache[x].last_used) {
			oi=x;
			break;
		};
		if ((oi==-1 || m_cache[x].last_used < m_cache[oi].last_used) && m_cache[x].resolved) {
			oi=x;
		};
	};
	if (oi == -1) {
		dbg_printf(ds_Debug,"C_AsyncDNS: too many resolves happening!");
		return -1;
	};
	safe_strncpy(m_cache[oi].hostname,hostname,sizeof(m_cache[oi].hostname));
	m_cache[oi].addr=INADDR_NONE;
	m_cache[oi].resolved=0;
	m_cache[oi].last_used=time(NULL);

	makesurethreadisrunning();
	return 1;
}

void C_AsyncDNS::makesurethreadisrunning()
{
	if (m_thread_kill) {
		#ifdef _WIN32
			if (m_thread) {
				WaitForSingleObject(m_thread,INFINITE);
				CloseHandle(m_thread);
			};
			DWORD id;
			m_thread_kill=0;
			m_thread=CreateThread(NULL,0,_threadfunc,(LPVOID)this,0,&id);
			if (!m_thread)
		#else
			if (m_thread) {
				void *p;
				pthread_join(m_thread,&p);
			};
			m_thread_kill=0;
			if (pthread_create(&m_thread,NULL,_threadfunc,(void*)this) != 0)
		#endif
			{
				dbg_printf(ds_Debug,"C_AsyncDNS: Error creating thread!");
				m_thread_kill=1;
			};
	};
}
