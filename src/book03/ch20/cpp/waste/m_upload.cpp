/*
WASTE -	m_upload.cpp (Upload request message builder/parser	class)
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

#include "m_upload.hpp"

C_UploadRequest::~C_UploadRequest()
{
	::free(m_fn);
}

C_UploadRequest::C_UploadRequest()
{
	m_fn=0;
	m_idx=-1;
	memset(&m_guid,0,sizeof(m_guid));
	m_dest[0]=0;
	m_size_low=m_size_high=-1;
	memset(m_nick,0,sizeof(m_nick));
}

C_UploadRequest::C_UploadRequest(C_SHBuf *in)
{
	m_fn=0;
	m_idx=-1;
	memset(&m_guid,0,sizeof(m_guid));
	m_dest[0]=0;
	memset(m_nick,0,sizeof(m_nick));

	unsigned char *data=(unsigned char *)in->Get();
	int datalen=in->GetLength();
	if (datalen < 16+8+(sizeof(m_dest)-1)+4+(sizeof(m_nick)-1) + 1 ||
		datalen > 16+8+(sizeof(m_dest)-1)+4+(sizeof(m_nick)-1) + 2048)
	{
		log_printf(ds_Warning,"uploadrequest: length out of range (%d)",datalen);
		return;
	};
	memcpy(&m_guid,data,16);			data+=16;datalen-=16;
	m_size_low=DataUInt4(data);			data+= 4;datalen-= 4;
	m_size_high=DataUInt4(data);		data+= 4;datalen-= 4;

	memcpy(m_dest,data,sizeof(m_dest)-1);
	m_dest[sizeof(m_dest)-1]=0;
	data+=sizeof(m_dest)-1;datalen-=sizeof(m_dest)-1;

	m_idx=DataUInt4(data);				data+= 4;datalen-= 4;

	memcpy(m_nick,data,sizeof(m_nick)-1);
	m_nick[sizeof(m_nick)-1]=0;
	data+=sizeof(m_nick)-1;datalen-=sizeof(m_nick)-1;

	m_fn=(char*)malloc(datalen+1);
	memcpy(m_fn,data,datalen);
	m_fn[datalen]=0;
}

C_SHBuf	*C_UploadRequest::Make()
{
	if (!m_fn || !m_fn[0] || strlen(m_fn) > 2048) {
		log_printf(ds_Warning,"uploadrequest::make(): filename length %d out of range",m_fn?strlen(m_fn):0);
		return new C_SHBuf(0);
	};
	C_SHBuf *p=new C_SHBuf(16+8+(sizeof(m_dest)-1)+4+(sizeof(m_nick)-1)+::strlen(m_fn));
	unsigned char *data=(unsigned char *)p->Get();

	memcpy	(data,&m_guid,16				);data+=16;
	IntData4(data,m_size_low				);data+= 4;
	IntData4(data,m_size_high				);data+= 4;
	memcpy	(data,m_dest,sizeof(m_dest)-1	);data+=sizeof(m_dest)-1; // 32 no null
	IntData4(data,m_idx						);data+= 4;
	memcpy	(data,m_nick,sizeof(m_nick)-1	);data+=sizeof(m_nick)-1; // 31 no null
	memcpy	(data,m_fn,strlen(m_fn));

	return p;
}

void C_UploadRequest::set_dest(char *dest)
{
	safe_strncpy(m_dest,dest,sizeof(m_dest));
}

void C_UploadRequest::set_nick(const char *nick)
{
	safe_strncpy(m_nick,nick,sizeof(m_nick));
}
