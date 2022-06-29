/*
WASTE - listview.cpp (Class for management of Windows listviews)
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

#include "platform.hpp"
#include "listview.hpp"

#if defined(_WIN32)&&(!defined(_DEFINE_SRV))
void W_ListView::AddCol(char *text, int w)
{
	LVCOLUMN lvc={0,};
	lvc.mask = LVCF_TEXT|LVCF_WIDTH;
	lvc.pszText = text;
	if (w) lvc.cx=w;
	ListView_InsertColumn(m_hwnd, m_col, &lvc);
	m_col++;
}

int W_ListView::GetColumnWidth(int col)
{
	if (col < 0 || col >= m_col) return 0;
	return ListView_GetColumnWidth(m_hwnd,col);
}

int W_ListView::GetParam(int p)
{
	LVITEM lvi;
	lvi.mask = LVIF_PARAM;
	lvi.iItem = p;
	ListView_GetItem(m_hwnd, &lvi);
	return lvi.lParam;
}

int W_ListView::InsertItem(int p, const char *text, int param)
{
	LVITEM lvi={0,};
	lvi.mask = LVIF_TEXT | LVIF_PARAM;
	lvi.iItem = p;
	lvi.pszText = (char*)text;
	lvi.cchTextMax=strlen(text);
	lvi.lParam = param;
	return ListView_InsertItem(m_hwnd, &lvi);
}

int W_ListView::InsertItemSorted(const char *text, int param, const char *sorttext)
{
	int l=GetCount();
	int x;
	for (x = 0; x < l; x ++) {
		char thetext[512];
		GetText(x,m_sort_col,thetext,sizeof(thetext));
		if (_docompare(sorttext,thetext)<0) {
			break;
		};
	};
	return InsertItem(x,text,param);
}

void W_ListView::SetItemText(int p, int si, const char *text)
{
	LVITEM lvi={0,};
	lvi.iItem = p;
	lvi.iSubItem = si;
	lvi.mask = LVIF_TEXT;
	lvi.pszText = (char*)text;
	lvi.cchTextMax = strlen(text);
	ListView_SetItem(m_hwnd, &lvi);
}

void W_ListView::SetItemParam(int p, int param)
{
	LVITEM lvi={0,};
	lvi.iItem = p;
	lvi.mask=LVIF_PARAM;
	lvi.lParam=param;
	ListView_SetItem(m_hwnd, &lvi);
}

int W_ListView::_docompare(const char *s1, const char *s2)
{
	if (m_sort_type) {
		int r=(atoi(s1)-atoi(s2));
		if (m_sort_dir==-1) return -r;
		return r;
	};
	int r=stricmp(s1,s2);
	if (m_sort_dir==-1) return -r;
	return r;
}

int CALLBACK W_ListView::sortfunc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
	W_ListView *_this=(W_ListView *)lParamSort;
	int idx1,idx2;
	char text1[128],text2[128];
	idx1=_this->FindItemByParam(lParam1);
	idx2=_this->FindItemByParam(lParam2);
	if (idx1==-1) return _this->m_sort_dir;
	if (idx2==-1) return -_this->m_sort_dir;
	_this->GetText(idx1,_this->m_sort_col,text1,sizeof(text1));
	_this->GetText(idx2,_this->m_sort_col,text2,sizeof(text2));
	text1[127]=text2[127]=0;
	return _this->_docompare(text1,text2);
}

void W_ListView::Resort()
{
	ListView_SortItems(m_hwnd,sortfunc,(LPARAM)this);
}

#endif//_WIN32

