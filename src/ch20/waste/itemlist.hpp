/*
WASTE - itemlist.hpp (Simple inline list class)
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

#ifndef _C_ITEMLIST_H_
#define _C_ITEMLIST_H_

template<class T> class C_ItemList
{
public:
	C_ItemList()
	{
		m_size=0;
		m_list=NULL;
	};
	~C_ItemList()
	{
		if (m_list) ::free(m_list);
	};

	T *Add(T *i)
	{
		if (!m_list || !(m_size&31)) {
			m_list=(T**)::realloc(m_list,sizeof(T*)*(m_size+32));
		};
		m_list[m_size++]=i;
		return i;
	};

	void Set(int w, T *newv) { if (w >= 0 && w < m_size) m_list[w]=newv; }

	T *Get(int w) { if (w >= 0 && w < m_size) return m_list[w]; return NULL; }

	void Del(int idx)
	{
		if (m_list && idx >= 0 && idx < m_size) {
			m_size--;
			if (idx != m_size) ::memcpy(m_list+idx,m_list+idx+1,sizeof(T *)*(m_size-idx));
			if (!(m_size&31)&&m_size) { //resize down
				m_list=(T**)::realloc(m_list,sizeof(T*)*m_size);
			};
		};
	};

	int GetSize() { return m_size; }

protected:
	T **m_list;
	int m_size;

};

#endif //_C_ITEMLIST_H_
