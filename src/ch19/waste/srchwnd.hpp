/*
WASTE - srchwnd.hpp (Search/browser dialogs and code)
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

#ifndef _SRCHWND_H_
#define _SRCHWND_H_

#define PARENT_DIRSTRING ".. (parent directory)"

//do not change these, heh, unless all clients on the network do :)
#define DIRECTORY_STRING "Directory"
#define USER_STRING "User"

#define SEARCHCACHE_NUMITEMS 16

class SearchCacheItem
{
	public:
		SearchCacheItem();
		~SearchCacheItem();
		C_ItemList<C_MessageSearchReply> searchreplies;
		char lastsearchtext[512];
		T_GUID search_id;
		unsigned int search_id_time;
		int numcons;
		unsigned int lastvisitem;
};

void KillDirgetlist();
void KillSearchhistory();

extern SearchCacheItem *g_searchcache[SEARCHCACHE_NUMITEMS];
void Search_Search(char *str);

void Search_Resort();
void Search_AddReply(T_Message *message);

#if defined(_WIN32)&&(!defined(_DEFINE_SRV))
BOOL WINAPI Search_DlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
extern HWND g_search_wnd;
#endif

#endif//_SRCHWND_H_
