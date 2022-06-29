/*
WASTE - prefs.hpp (Preferences Dialogs)
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

#ifndef _PREFS_H_
#define _PREFS_H_

#if defined(_WIN32)&&(!defined(_DEFINE_SRV))
	extern HWND prefs_hwnd,prefs_cur_wnd;
	int Prefs_SelectProfile(int force); //returns 1 on user abort
	BOOL CALLBACK PrefsOuterProc(HWND hwndDlg, UINT uMsg, WPARAM wParam,LPARAM lParam);
	BOOL WINAPI SetupWizProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
#endif

#endif//_PREFS_H_
