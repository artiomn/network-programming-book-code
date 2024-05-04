/*
WASTE - shbuf.hpp (Shared buffer inline class)
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

#ifndef _C_SHBUF_H_
#define _C_SHBUF_H_

class C_SHBuf
{
public:
	C_SHBuf(int length);
	~C_SHBuf();
	void Lock();
	void Unlock();
	void *Get();
	int  GetLength();
protected:
	void *m_d;
	int m_dlen;
	int m_refcnt;
	bool m_bEverLocked;
};

#endif //_C_SHBUF_H_
