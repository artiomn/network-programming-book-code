/*
WASTE - m_ping.hpp (Ping message builder/parser class)
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

#ifndef _C_MPING_H_
#define _C_MPING_H_

#include "shbuf.hpp"
#include "util.hpp"

class C_MessagePing
{
public:
	C_MessagePing();
	C_MessagePing(C_SHBuf *in);
	~C_MessagePing();

	C_SHBuf *Make();

	char m_nick[32];
	unsigned short m_port;
	unsigned int m_ip;
};

#endif//_C_MFILEREQ_H_
