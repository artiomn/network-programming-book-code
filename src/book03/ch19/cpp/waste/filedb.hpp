/*
WASTE - filedb.hpp (File database and scanning class)
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

#ifndef _C_FILEDB_H_
#define _C_FILEDB_H_

#include "mqueuelist.hpp"
#include "m_search.hpp"
#include "itemstack.hpp"
#include "itemlist.hpp"

#ifndef _WIN32
#include <sys/types.h>
#include <dirent.h>
#endif

class C_FileDB
{
public:
	C_FileDB();
	~C_FileDB();
	void Scan(const char *pathlist);
	void UpdateExtList(const char *extlist);

	int DoScan(int maxtime, C_FileDB *oldDB); //returns number of files scanned, or -1 if done

	void Search(char *ss, C_MessageSearchReply *repl,
		C_MessageQueueList *mqueuesend, T_Message *srcmessage,
		void (*gm)(T_Message *message, C_MessageQueueList *_this, C_Connection *cn));
	int GetFile(int index, char *file, char *meta, int *length_low, int *length_high, char **sharebaseptr=0);
	int GetNumFiles() { return m_database_used; }
	int GetNumMB() { return m_database_mb; }
	int GetLatestTime() { return m_database_newesttime; }

	void writeOut(char *fn);
	int readIn(char *fn);

#ifdef _WIN32
	static unsigned int FileTimeToUnixTime(FILETIME *ft);
	static void UnixTimeToFileTime(FILETIME *ft, unsigned int t);
#endif

	//util funcs
	static void parselist(char *out, char *in);
	static inline int in_string(char *string, char *substring);
	static int substr_search(char *bigtext1, char *bigtext2, char *littletext_list);

protected:
	void clearDBs();

	struct dbType
	{
		char *file;//[MAX_PATH];
		char *meta;//[64];
		int dir_index;
		int length_low, length_high;
		int file_time; //unix time format
		int v_index;
	};
	struct ScanType
	{
#ifdef _WIN32
		HANDLE h;
#else
		DIR *dir_h;
#endif
		int dir_index;
		int base_len;
		char cur_path[1024];
	};
	struct DirIndexType
	{
		char *dirname;//[1024];
		int base_len;
	};

	C_ItemStack<ScanType> *m_scan_stack;
	DirIndexType *m_dir_index;
	int m_dir_index_size,m_dir_index_used;
	void alloc_dir_index();

	dbType *m_database;
	int m_database_used,m_database_size;
	int m_database_mb;
	int m_database_xbytes; //bytes unaccounted for in _mb
	int m_database_newesttime;

	int m_oldscan_lastpos,m_oldscan_lastdirpos, m_oldscan_dirstate;

	int m_scanidx_gpos;
	int m_use_oldidx;
	char m_ext_list[32768]; //ADDED Md5Chap (doing check length on copy)

	void alloc_entry();
	int in_list(char *list, char *v);
	void doRecursiveAddDB(char *cur_path);
#ifdef _WIN32
	void mp3_getmetainfo(HANDLE hFile,char *meta, int filelen);
	void jpg_getmetainfo(HANDLE hFile,char *meta, int filelen);
	HANDLE hHeap;
#else
	void mp3_getmetainfo(FILE *fp,char *meta, int filelen);
	void jpg_getmetainfo(FILE *fp,char *meta, int filelen);
#endif

};

#endif//_C_FILEDB_H_
