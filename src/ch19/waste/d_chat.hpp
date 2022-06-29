/*
WASTE - d_chat.hpp (Chat window dialog declarations)
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

#ifndef _D_CHAT_H_
#define _D_CHAT_H_

#include "m_chat.hpp"

bool chat_IsForMe(C_MessageChat &chat);
bool chat_handle_whois(C_MessageChat &chat);
int chat_HandleMsg(T_Message *message); //refered by either srvmain or d_chat

#if defined(_WIN32)&&(!defined(_DEFINE_SRV))
	struct ChatroomItem
	{
		HWND hwnd;
		char channel[SHA_OUTSIZE*2+1];
		ChatroomItem *next;

		ChildWndResizeItem resize[5];

		T_GUID lastMsgGuid;
		unsigned int lastmsgguid_time;

		WNDPROC chatdiv_oldWndProc;
		int wnd_old_xoffs;
		int lastdivpos;

		char *chatedit_hist[CHATEDIT_HISTSIZE];
		int chatedit_hist_pos;
	};
	typedef ChatroomItem chatroom_item; //alias

	extern chatroom_item *L_Chatroom;
	void CloseAllChatWindows();
	void RunPerforms();
	int IsChatRoomDlgMessage(MSG *msg);
	void chat_updateTitles();
	void chat_sendNickChangeFrom(char *oldnick);
	chatroom_item *chat_ShowRoom(const char *channel, int activate);
	BOOL WINAPI Chat_DlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
	BOOL WINAPI ChatRooms_DlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
	BOOL WINAPI CreateChat_DlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
#endif

#endif //_D_CHAT_H_
