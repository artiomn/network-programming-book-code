/*
WASTE - m_keydist.cpp (Key distribution message builder/parser class)
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

#include "m_keydist.hpp"

/*
** my nick (32 bytes)
** flags 1 byte
** keysize bits 2 bytes
** modulus size bytes 2 bytes
** exponent size bytes 2 bytes
** modulus bytes
** exponent bytes
*/

C_KeydistRequest::~C_KeydistRequest()
{
}

C_KeydistRequest::C_KeydistRequest()
{
	memset(m_nick,0,sizeof(m_nick));
	memset(&m_key,0,sizeof(m_key));
	m_flags=0;
}

C_KeydistRequest::C_KeydistRequest(C_SHBuf *in)
{
	memset(m_nick,0,sizeof(m_nick));
	memset(&m_key,0,sizeof(m_key));
	m_flags=0;

	unsigned char *data=(unsigned char *)in->Get();
	unsigned int datalen=in->GetLength();

	if (datalen < (sizeof(m_nick)-1)+1+2+2+2 ||
		datalen > (sizeof(m_nick)-1)+1+2+2+2 + MAX_RSA_MODULUS_LEN*2) {
		dbg_printf(ds_Debug,"keydistrequest: length out of range (%d)",datalen);
		return;
	};
	memcpy(m_nick,data,sizeof(m_nick)-1); //31 no NULL
	data+=sizeof(m_nick)-1;
	datalen-=sizeof(m_nick)-1;

	m_flags=*data++;
	datalen--;

	m_key.bits=DataUInt2(data);data+=2;datalen-=2;
	unsigned int modlen=DataUInt2(data);data+=2;datalen-=2;
	unsigned int explen=DataUInt2(data);data+=2;datalen-=2;
	if (m_key.bits < MIN_RSA_MODULUS_BITS || m_key.bits > MAX_RSA_MODULUS_BITS ||
		modlen > MAX_RSA_MODULUS_LEN || explen > MAX_RSA_MODULUS_LEN || datalen < explen+modlen)
	{
		dbg_printf(ds_Debug,"keydistrequest: key bits out of range (%d,%d,%d,%d)",m_key.bits,modlen,explen,datalen);
		m_key.bits=0;
		return;
	};
	memcpy(m_key.modulus+MAX_RSA_MODULUS_LEN-modlen,data,modlen); data+=modlen; datalen-=modlen;
	memcpy(m_key.exponent+MAX_RSA_MODULUS_LEN-explen,data,explen); data+=explen; datalen-=explen;
}

C_SHBuf *C_KeydistRequest::Make()
{
	if (!m_key.bits) {
		log_printf(ds_Warning,"keydistrequest::make(): bits=0");
		return new C_SHBuf(0);
	};
	int explen=MAX_RSA_MODULUS_LEN;
	int modlen=MAX_RSA_MODULUS_LEN;
	unsigned char *expptr=m_key.exponent;
	unsigned char *modptr=m_key.modulus;
	while (*expptr == 0 && explen>0) {
		explen--;
		expptr++;
	};
	while (*modptr == 0 && modlen>0) {
		modlen--;
		modptr++;
	};

	log_printf(ds_Informational,"keydistrequest: modlen=%d,explen=%d",modlen,explen);

	C_SHBuf *p=new C_SHBuf((sizeof(m_nick)-1)+1+2+2+2+explen+modlen);
	unsigned char *data=(unsigned char *)p->Get();

	memcpy   (data,m_nick,sizeof(m_nick)-1);data+=sizeof(m_nick)-1; //31 no NULL
	UIntData1(data,m_flags);data+=1;
	UIntData2(data,(unsigned short)(m_key.bits&0xffff));data+=2;
	UIntData2(data,(unsigned short)(modlen&0xffff));data+=2;
	UIntData2(data,(unsigned short)(explen&0xffff));data+=2;
	memcpy   (data,modptr,modlen);data+=modlen;
	memcpy   (data,expptr,explen);data+=explen;
	return p;
}

void C_KeydistRequest::set_nick(const char *nick)
{
	safe_strncpy(m_nick,nick,sizeof(m_nick));
}

