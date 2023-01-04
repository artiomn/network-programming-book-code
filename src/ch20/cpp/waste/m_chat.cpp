/*
WASTE - m_chat.cpp (Chat message builder/parser class)
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

#include "main.hpp"
#include "m_chat.hpp"

C_MessageChatReply::C_MessageChatReply()
{
	nick[0]=0;
}

C_MessageChatReply::~C_MessageChatReply()
{
}

C_MessageChatReply::C_MessageChatReply(C_SHBuf *in)
{
	nick[0]=0;
	if (in->GetLength() > sizeof(nick)-1) {
		log_printf(ds_Warning,"chatreply: length of %d too big",in->GetLength());
	}
	else {
		nick[in->GetLength()]=0;
		memcpy(nick,in->Get(),in->GetLength());
	};
}

void C_MessageChatReply::setnick(const char *fn)
{
	safe_strncpy(nick,fn,sizeof(nick));
}

C_SHBuf *C_MessageChatReply::Make()
{
	int l=strlen(nick);
	if (l > sizeof(nick)-1) {
		dbg_printf(ds_Debug,"chatreply::make() length is %d",l);
		l=sizeof(nick)-1;
	};
	C_SHBuf *n=new C_SHBuf(l);
	if (l) memcpy(n->Get(),nick,l);
	return n;
}

C_MessageChat::~C_MessageChat()
{
	delete m_chatstring;
}

C_MessageChat::C_MessageChat()
{
	//m_chatstring[0]=0;
	m_chatstring = NULL;
	m_src[0]=0;
	m_dest[0]=0;
}

C_MessageChat::C_MessageChat(C_SHBuf *in)
{
	m_chatstring = NULL;
	m_src[0]=0;
	m_dest[0]=0;
	char* data=(char*)in->Get();
	int l=in->GetLength();

	// TODO: debug new!
	const char *tc=data;
	tc=CopySingleToken(m_src,tc,0,sizeof(m_src),l,&l);
	if (!tc) return;

	tc=CopySingleToken(m_dest,tc,0,sizeof(m_dest),l,&l);
	if (!tc) return;

	if (l>0) {
		m_chatstring = new char[l+1];
		tc=CopySingleToken(m_chatstring,tc,0,l+1,l,&l);
	};
}

C_SHBuf *C_MessageChat::Make()
{
	C_SHBuf *buf=new C_SHBuf(::strlen(m_src)+::strlen(m_dest)+::strlen(m_chatstring)+3);
	unsigned char *data=(unsigned char *)buf->Get();
	::memcpy((char*)data,m_src,strlen(m_src)+1);
	data+=::strlen(m_src)+1;
	::memcpy((char*)data,m_dest,strlen(m_dest)+1);
	data+=::strlen(m_dest)+1;
	::memcpy((char*)data,m_chatstring,strlen(m_chatstring)+1);
	return buf;
}

const char *C_MessageChat::get_chatstring()
{
	if (m_chatstring) return m_chatstring;
	return "";
}

void C_MessageChat::set_chatstring(const char *str)
{
	int len = strlen(str);
	m_chatstring = new char[len+1];
	safe_strncpy(m_chatstring,str,len+1);
}

void C_MessageChat::set_src(const char *str)
{
	safe_strncpy(m_src,str,sizeof(m_src));
}

void C_MessageChat::set_dest(const char *str)
{
	safe_strncpy(m_dest,str,sizeof(m_dest));
}

