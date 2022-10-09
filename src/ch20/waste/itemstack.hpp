/*
WASTE - itemstack.hpp (Simple inline stack class)
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

#ifndef _C_ITEMSTACK_H_
#define _C_ITEMSTACK_H_

template<class T> class C_ItemStack
{
public:
	C_ItemStack()
	{
		m_size=0;
		m_list=NULL;
	};
	~C_ItemStack()
	{
		if (m_list) ::free(m_list);
	};

	void Push(T &i)
	{
		if (!m_list || !(m_size&31)) {
			m_list=(T*)::realloc(m_list,sizeof(T)*(m_size+32));
		};
		m_list[m_size++]=i;
	};

	int Pop(T &r)
	{
		if (!m_size || !m_list) return 1;
		r=m_list[--m_size];
		return 0;
	};

protected:
	T *m_list;
	int m_size;

};

#endif //_C_ITEMSTACK_H_
