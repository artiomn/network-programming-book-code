/*
WASTE - childwnd.cpp (Windows child window resizing code)
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

#ifndef _CHILDWND_H_
#define _CHILDWND_H_

#if defined(_WIN32)&&(!defined(_DEFINE_SRV))
	struct ChildWndResizeItem{
		int id;
		int type; // 0xLTRB
		RECT rinfo;
	};

	void childresize_init(HWND hwndDlg, ChildWndResizeItem *list, int num);
	void childresize_resize(HWND hwndDlg, ChildWndResizeItem *list, int num);

	struct dlgSizeInfo
	{
		char *name;
		int defx,defy,defw,defh;
	};

	int handleDialogSizeMsgs(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam, dlgSizeInfo *inf);
#endif

#endif//_CHILDWND_H_

