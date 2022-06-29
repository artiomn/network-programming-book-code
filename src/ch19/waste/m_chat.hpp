/*
WASTE - m_chat.hpp (Chat message builder/parser class)
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

#ifndef _C_MCHAT_H_
#define _C_MCHAT_H_

#include "shbuf.hpp"
#include "util.hpp"

class C_MessageChatReply
{
public:
	C_MessageChatReply();
	C_MessageChatReply(C_SHBuf *in);
	~C_MessageChatReply();

	void setnick(const char *fn);
	char *getnick() { return nick; }

	C_SHBuf *Make();

protected:
	char nick[32];
};

class C_MessageChat
{
public:
	C_MessageChat();
	C_MessageChat(C_SHBuf *in);
	~C_MessageChat();

	C_SHBuf *Make();

	const char *get_src() { return m_src; }
	const char *get_dest() { return m_dest; }
	void set_src(const char *str);
	void set_dest(const char *str);

	const char *get_chatstring();
	void set_chatstring(const char *str);


protected:
	char* m_chatstring;
	char m_src[32];
	char m_dest[41];
};

#endif//_C_MCHAT_H_
