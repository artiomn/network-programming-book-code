/*
WASTE - m_ping.cpp (Ping message builder/parser class)
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
#include "m_ping.hpp"

C_MessagePing::~C_MessagePing()
{
}

C_MessagePing::C_MessagePing()
{
	m_nick[0]=0;
	m_ip=0;
	m_port=0;
}

C_MessagePing::C_MessagePing(C_SHBuf *in)
{
	m_nick[0]=0;
	m_ip=0;
	m_port=0;

	unsigned char *data=(unsigned char*)in->Get();
	int l=in->GetLength();
	if (l < 2+4) return;

	m_port=DataUInt2(data);data+=2;
	m_ip=DataUInt4(data);data+=4;
	l-=4+2;
	if (l>0) {
		if (l > sizeof(m_nick)-1) {
			dbg_printf(ds_Debug,"Ping: got ping with nick > %i", sizeof(m_nick)-1);
		}
		else {
			memcpy(m_nick,data,l);
			data+=l;
			m_nick[l]=0;
		};
	};
}

C_SHBuf *C_MessagePing::Make()
{
	C_SHBuf *buf=new C_SHBuf(2+4+strlen(m_nick));
	unsigned char *data=(unsigned char *)buf->Get();
	UIntData2(data,m_port);data+=2;
	UIntData4(data,m_ip);data+=4;
	memcpy(data,m_nick,strlen(m_nick));data+=strlen(m_nick);
	return buf;
}
