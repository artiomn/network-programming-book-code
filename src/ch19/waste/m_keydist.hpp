/*
WASTE - m_keydist.hpp (Key distribution message builder/parser class)
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

#ifndef _C_MKEYDIST_H_
#define _C_MKEYDIST_H_

#include "shbuf.hpp"
#include "util.hpp"
#include "rsa/global.hpp"
#include "rsa/rsaref.hpp"

#define M_KEYDIST_FLAG_LISTEN 1

class C_KeydistRequest
{
public:
	C_KeydistRequest();
	C_KeydistRequest(C_SHBuf *in);
	~C_KeydistRequest();

	C_SHBuf *Make();

	void set_nick(const char *nick);
	const char *get_nick() { return m_nick; };

	void set_flags(unsigned char flags) { m_flags=flags; };
	unsigned char get_flags() { return m_flags; };

	void set_key(R_RSA_PUBLIC_KEY *key) { m_key=*key; };
	R_RSA_PUBLIC_KEY *get_key() { return &m_key; };

protected:
	unsigned char m_flags;
	char m_nick[32];
	R_RSA_PUBLIC_KEY m_key;

};

#endif//_C_MKEYDIST_H_
