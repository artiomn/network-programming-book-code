/*
WASTE - netkern.hpp (Common network runtime code declarations)
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

#ifndef _NETKERN_H_
#define _NETKERN_H_

extern C_ItemList<C_Connection> g_new_net;

#if defined(_WIN32)&&(!defined(_DEFINE_SRV))
extern W_ListView g_lvnetcons;
extern HWND g_netstatus_wnd;
#endif

void NetKern_Run();
void AddConnection(char *str, unsigned short port, int rating);
void DoPing(C_MessageQueue *mq);
void RebroadcastCaps(C_MessageQueueList *mql);
void NetKern_ConnectToHostIfOK(unsigned long ip, unsigned short port);

#if defined(_WIN32)&&(!defined(_DEFINE_SRV))
BOOL WINAPI Net_DlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
#endif

#endif//_NETKERN_H_
