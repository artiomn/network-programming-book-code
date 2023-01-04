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

#include "stdafx.hpp"

#include "main.hpp"
#include "childwnd.hpp"

void childresize_init(HWND hwndDlg, ChildWndResizeItem *list, int num)
{
	RECT r;
	GetClientRect(hwndDlg,&r);
	int x;
	for (x = 0; x < num; x ++) {
		RECT r2;
		GetWindowRect(GetDlgItem(hwndDlg,list[x].id),&r2);
		ScreenToClient(hwndDlg,(LPPOINT)&r2);
		ScreenToClient(hwndDlg,((LPPOINT)&r2)+1);

		if (list[x].type&0xF000) list[x].rinfo.left=r.right-r2.left;
		else list[x].rinfo.left=r2.left;
		if (list[x].type&0x0F00) list[x].rinfo.top=r.bottom-r2.top;
		else list[x].rinfo.top=r2.top;
		if (list[x].type&0x00F0) list[x].rinfo.right=r.right-r2.right;
		else list[x].rinfo.right=r2.right;
		if (list[x].type&0x000F) list[x].rinfo.bottom=r.bottom-r2.bottom;
		else list[x].rinfo.bottom=r2.bottom;
		list[x].type|=0xf0000;
	};
}

void childresize_resize(HWND hwndDlg, ChildWndResizeItem *list, int num)
{
	RECT r;
	GetClientRect(hwndDlg,&r);
	int x;
	HDWP hdwp=BeginDeferWindowPos(num);
	for (x = 0; x < num; x ++) if (list[x].type&0xf0000) {
		RECT r2;
		if (list[x].type&0xF000)  r2.left=r.right-list[x].rinfo.left;
		else r2.left=list[x].rinfo.left;

		if (list[x].type&0x0F00)  r2.top=r.bottom-list[x].rinfo.top;
		else r2.top=list[x].rinfo.top;

		if (list[x].type&0x00F0)  r2.right=r.right-list[x].rinfo.right;
		else r2.right=list[x].rinfo.right;

		if (list[x].type&0x000F)  r2.bottom=r.bottom-list[x].rinfo.bottom;
		else r2.bottom=list[x].rinfo.bottom;

		HWND h=GetDlgItem(hwndDlg,list[x].id);
		DeferWindowPos(hdwp, h, NULL, r2.left,r2.top,r2.right-r2.left,r2.bottom-r2.top, SWP_NOZORDER|SWP_NOACTIVATE);
	};
	EndDeferWindowPos(hdwp);
}

int handleDialogSizeMsgs(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM /*lParam*/, dlgSizeInfo *inf)
{
	char tmp[512];
	#define READINT(d,x,y) sprintf(tmp,"%s_" x,inf->name); (d) = g_config->ReadInt(tmp,(y))
	#define WRITEINT(x,y) sprintf(tmp,"%s_" x,inf->name); g_config->WriteInt(tmp,y)
	if (uMsg == WM_INITDIALOG) {
		int w,h,x,y;
		READINT(w,"width",inf->defw);
		READINT(h,"height",inf->defh);
		READINT(x,"x",inf->defx);
		READINT(y,"y",inf->defy);

		if (w>0&&h>0) {
			SetWindowPos(hwndDlg,NULL,x,y,w,h,SWP_NOACTIVATE|SWP_NOZORDER);
		};
	}
	else if (uMsg == WM_MOVE) {
		WINDOWPLACEMENT wp={sizeof(wp),};
		GetWindowPlacement(hwndDlg,&wp);
		if (wp.showCmd != SW_SHOWMAXIMIZED && wp.showCmd != SW_SHOWMINIMIZED) {
			RECT r;
			GetWindowRect(hwndDlg,&r);
			WRITEINT("x",r.left);
			WRITEINT("y",r.top);
		};
	}
	else if (uMsg == WM_SIZE) {
		if (wParam == SIZE_MAXIMIZED) {
			WRITEINT("maximized",1);
		};
		if (wParam == SIZE_RESTORED) {
			WINDOWPLACEMENT wp={sizeof(wp),};
			GetWindowPlacement(hwndDlg,&wp);
			if (wp.showCmd != SW_SHOWMAXIMIZED) {
				ShowWindow(hwndDlg,SW_SHOW);
				ShowWindow(hwndDlg,SW_RESTORE);
				WRITEINT("maximized",0);
			};
		};
		int m;
		READINT(m,"maximized",0);
		if (!m) {
			WINDOWPLACEMENT wp={sizeof(wp),};
			GetWindowPlacement(hwndDlg,&wp);
			if (wp.showCmd != SW_SHOWMAXIMIZED && wp.showCmd != SW_SHOWMINIMIZED) {
				RECT r;
				GetWindowRect(hwndDlg,&r);
				WRITEINT("width",r.right-r.left);
				WRITEINT("height",r.bottom-r.top);
			};
		};
	};
	return 0;
#undef READINT
#undef WRITEINT
}
