/*
WASTE - config.hpp (Configuration file class)
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

#ifndef _C_CONFIG_H_
#define _C_CONFIG_H_

class C_Config
{
public:
	C_Config(char *ini);
	~C_Config();
	void Flush();
	void  WriteInt(char *name, int value);
	const char *WriteString(char *name, const char *string);
	int   ReadInt(char *name, int defvalue);
	const char *ReadString(char *name, const char *defvalue);

private:
	struct strType
	{
		char name[16];
		char *value;
	};

	strType *m_strs;
	int m_dirty;
	int m_num_strs, m_num_strs_alloc;
	char *m_inifile;
};

#endif//_C_CONFIG_H_
