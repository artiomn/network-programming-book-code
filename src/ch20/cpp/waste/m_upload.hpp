/*
WASTE - m_upload.hpp (Upload request message builder/parser class)
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

#ifndef _C_MUPLOAD_H_
#define _C_MUPLOAD_H_

#include "shbuf.hpp"
#include "util.hpp"

class C_UploadRequest
{
public:
	C_UploadRequest();
	C_UploadRequest(C_SHBuf *in);
	~C_UploadRequest();

	C_SHBuf *Make();

	//host guid
	char *get_dest() { return m_dest; }
	void set_dest(char *dest);

	T_GUID *get_guid() { return &m_guid; }
	void set_guid(T_GUID *guid) { m_guid=*guid; }

	void set_idx(int idx) { m_idx=idx; }
	int get_idx() { return m_idx; }

	void set_fn(char *fn) { ::free(m_fn); m_fn=::strdup(fn); }
	char *get_fn() { return m_fn; }

	void set_nick(const char *nick);
	const char *get_nick() { return m_nick; }

	void set_fsize(int low, int high) { m_size_low=low; m_size_high=high; }
	void get_fsize(int *low, int *high) { *low=m_size_low; *high=m_size_high; }

protected:
	T_GUID m_guid;
	char m_dest[33];
	char m_nick[32];
	int m_idx;
	int m_size_low, m_size_high;
	char *m_fn;
};

#endif//_C_MUPLOAD_H_
