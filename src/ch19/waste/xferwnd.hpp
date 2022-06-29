/*
WASTE - xferwnd.hpp (File transfer dialogs)
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
#ifndef _XFERWND_H_
	#define _XFERWND_H_

	#include "xfers.hpp"

	#if defined(_WIN32)&&(!defined(_DEFINE_SRV))
		extern HWND g_xferwnd, g_xfer_subwnd_active;
		extern W_ListView g_lvrecv, g_lvsend, g_lvrecvq;
		extern int g_files_in_download_queue;
	#endif

	extern C_ItemList<XferSend> g_sends;
	extern C_ItemList<XferRecv> g_recvs;
	extern C_ItemList<char> g_uploads;

	#define UPLOAD_BASE_IDX 0x70000000

	int Xfer_WillQ(char *file, char *guidstr);

	void Xfer_Run();
	void RecvQ_UpdateStatusText();

	#if defined(_WIN32)&&(!defined(_DEFINE_SRV))
		void Xfer_UploadFileToUser(HWND hwndDlg, char *file, char *user, char *leadingpath);
		void XferDlg_SetSel(int sel=-1);
		BOOL WINAPI Xfers_DlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

	#endif//_WIN32

#endif//_XFERWND_H_
