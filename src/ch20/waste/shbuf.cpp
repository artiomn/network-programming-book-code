/*
WASTE - shbuf.cpp (Shared buffer inline class)
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
#include "shbuf.hpp"

#ifdef _WIN32
#undef new
#endif

C_SHBuf::C_SHBuf(int length)
{
	m_dlen=length;
	if (length==0) length++;
	m_d=new char[(length+7)&~7];
	m_refcnt=0;
	m_bEverLocked=false;
}

C_SHBuf::~C_SHBuf()
{
	ASSERT(m_refcnt==0);
	delete (char*)m_d;
	m_d=0;
	m_dlen=0;
}

void C_SHBuf::Lock()
{
	m_bEverLocked=true;
	m_refcnt++;
}

void C_SHBuf::Unlock()
{
	if (m_bEverLocked) {
		ASSERT(m_refcnt>0);
		m_refcnt--;
		if (m_refcnt==0) delete this;
	}
	else {
		ASSERT(m_refcnt==0);
		delete this;
	};
}

void* C_SHBuf::Get()
{
	ASSERT(m_d);
	return m_d;
}

int C_SHBuf::GetLength()
{
	ASSERT(m_d);
	return m_dlen;
}

