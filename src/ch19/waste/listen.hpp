/*
WASTE - listen.hpp (Port listening class)
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

#ifndef _C_LISTEN_H_
#define _C_LISTEN_H_

#include "connection.hpp"

class C_Listen
{
public:
	C_Listen(short port, char *which_interface=0);
	~C_Listen();

	C_Connection *get_connect();
	short port() { return m_port; }
	int is_error() { return (m_socket<0); }

protected:
	int m_socket;
	unsigned short m_port;
};

#endif //_C_LISTEN_H_
