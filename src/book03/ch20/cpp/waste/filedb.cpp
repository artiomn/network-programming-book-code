/*
WASTE - filedb.cpp (File database and scanning class)
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
#include "filedb.hpp"
#include "m_search.hpp"
#include "util.hpp"
#include "srchwnd.hpp"

#define SCAN_MAX_POS 0x60000000

#ifdef _WIN32
	#define DB_LH_ALLOC(y) HeapAlloc(hHeap,0,y)
	#define DB_LH_REALLOC(x,y) HeapReAlloc(hHeap,0,x,y)
	#define DB_LH_FREE(x) HeapFree(hHeap,0,x)
	#define DB_LH_STRDUP(x) priv__strdup(hHeap,x)
	static char *priv__strdup(HANDLE hHeap, char *str)
	{
		char *t=(char *) DB_LH_ALLOC(strlen(str)+1);
		strcpy(t,str);
		return t;
	}
#else
	#define DB_LH_ALLOC(y) malloc(y)
	#define DB_LH_REALLOC(x,y) realloc(x,y)
	#define DB_LH_FREE(x) free(x)
	#define DB_LH_STRDUP(x) strdup(x)
#endif

C_FileDB::C_FileDB()
{
	m_use_oldidx=0;
	m_scanidx_gpos=0;
	m_database=NULL;
	m_database_used=m_database_size=0;
	m_database_mb=0;
	m_database_xbytes=0;
	m_database_newesttime=0;
	m_scan_stack=NULL;
	m_dir_index=NULL;
	m_dir_index_size=0;
	m_dir_index_used=0;
	m_ext_list[0]=m_ext_list[1]=0;
	#ifdef _WIN32
		hHeap=HeapCreate(HEAP_NO_SERIALIZE,0,0);
	#endif
}

void C_FileDB::UpdateExtList(const char *extlist)
{
	char *p=m_ext_list;
	char *p2,*p3=m_ext_list+sizeof(m_ext_list)-2;
	while (*extlist) {
		while (*extlist == ';' || *extlist == ' ') extlist++;
		p2=p;
		while (*extlist != ';' && *extlist != ' ' && *extlist) {
			if (p>=p3) {
				*p2++=0;*p2=0;//cutting rest
				return;
			};
			*p++=*extlist++;
		};
		*p++=0;
	};
	*p=0;
}

void C_FileDB::Scan(const char *pathlist)
{
	char path_buf[4096];
	if (m_scan_stack) {
		ScanType p;
		while (!m_scan_stack->Pop(p)) {
			#ifdef _WIN32
				if (p.h != INVALID_HANDLE_VALUE) ::FindClose(p.h);
			#else
				if (p.dir_h) closedir(p.dir_h);
			#endif
		};
	}
	else {
		m_scan_stack=new C_ItemStack<ScanType>;
	};
	clearDBs();

	safe_strncpy(path_buf,pathlist,sizeof(path_buf));
	unsigned int len=strlen(path_buf);
	if (len==sizeof(path_buf)-1) {
		char *p=path_buf+len-1;
		while ((p>path_buf)&&(*p!=';')) p--;
		*p=0;
	};

	char *p=path_buf;
	while (p && *p) {
		char *lp=p;
		p=strstr(p,";");
		if (p) {
			if (*p) *p++=0;
		};

		ScanType i;
		#ifdef _WIN32
			i.h=INVALID_HANDLE_VALUE;
		#else
			i.dir_h=0;
		#endif
		len=strlen(lp);
		if (lp[len-1]==DIRCHAR) lp[len-1]=0;

		safe_strncpy(i.cur_path,lp,sizeof(i.cur_path));
		i.dir_index=-1;
		i.base_len=strlen(lp);
		m_scan_stack->Push(i);
		dbg_printf(ds_Debug,"Adding directory: '%s'",i.cur_path);
	};
	m_oldscan_lastpos=0;
	m_oldscan_lastdirpos=0;
	m_oldscan_dirstate=0;
}

void C_FileDB::clearDBs()
{
	if (m_database) {
		int x;
		for (x = 0; x < m_database_used; x ++) {
			DB_LH_FREE(m_database[x].meta);
			DB_LH_FREE(m_database[x].file);
		};
		DB_LH_FREE(m_database);
	};

	if (m_dir_index) {
		int x;
		for (x = 0; x < m_dir_index_used; x ++) {
			DB_LH_FREE(m_dir_index[x].dirname);
		};
		DB_LH_FREE(m_dir_index);
	};
	m_dir_index=NULL;
	m_dir_index_size=0;
	m_dir_index_used=0;
	m_database=0;
	m_database_size=0;
	m_database_used=0;
	m_database_xbytes=0;
	m_database_newesttime=0;
	m_database_mb=0;
}

C_FileDB::~C_FileDB()
{
	delete m_scan_stack;
	clearDBs();
	#ifdef _WIN32
		HeapDestroy(hHeap);
	#endif
}

void C_FileDB::parselist(char *out, char *in)
{
	int nt=10;
	int inquotes=0, neednull=0;
	while (*in) {
		char c=*in++;

		if (c >= 'A' && c <= 'Z') c+='a'-'A';

		if (c != ' ' && c != '\t' && c != '\"') {
			neednull=1;
			*out++=c;
		}
		else if (c == '\"') {
			inquotes=!inquotes;
			if (!inquotes) {
				*out++=0;
				if (!nt--) break;
				neednull=0;
			};
		}
		else {
			if (inquotes) *out++=c;
			else if (neednull) {
				*out++=0;
				if (!nt--) break;
				neednull=0;
			};
		};
	};
	*out++=0;
	*out++=0;
}

int C_FileDB::in_string(char *string, char *substring)
{
	//HACK init but unref
	//char *os=string;
	if (!*substring) return 1;
	int l=strlen(substring);
	if (l < 2) {
		do{
			if ((string[0]|32) == (substring[0])) return 1;
			string++;
		} while (*string);
		return 0;
	};
	if ((int)strlen(string) < l) return 0;

	do{
		if ((string[0]|32) == (substring[0]) && (string[1]|32) == (substring[1])) {
			if (l<3 || !strnicmp(string,substring,l)) return 1;
		};
		string++;
	}
	while (string[1]);

	return 0;
}

int C_FileDB::substr_search(char *bigtext1, char *bigtext2, char *littletext_list)
{
	while (*littletext_list) {
		if (!in_string(bigtext1,littletext_list) && !in_string(bigtext2,littletext_list)) return 0;
		while (*littletext_list) littletext_list++;
		littletext_list++;
	};
	return 1;
}

void C_FileDB::Search(char *ss, C_MessageSearchReply *repl, C_MessageQueueList *mqueuesend, T_Message *srcmessage, void (*gm)(T_Message *message, C_MessageQueueList *_this, C_Connection *cn))
{
	char _searchstring[1024],*searchstring=_searchstring;
	safe_strncpy(searchstring,ss,sizeof(_searchstring));
	if (searchstring[0] == '/') {
		char buf[256];
		MakeID128Str(&g_client_id,buf);

		char *machinename=g_regnick;
		if (!machinename[0] || machinename[0]=='.') {
			machinename=buf;
			if (!machinename[0]) return;
		};

		if (!searchstring[1]||
			(
			!strstr(searchstring+1,"/") && searchstring[strlen(searchstring)-1]=='*'
			))
		{
			//respond with yourself
			if (searchstring[1]) {
				searchstring[strlen(searchstring)-1]=0;

				if (strnicmp(searchstring+1,machinename,strlen(searchstring+1))) {
					machinename=buf;
					if (strnicmp(searchstring+1,machinename,strlen(searchstring+1)))
						return;
				};
			};
			repl->add_item(-1,machinename,(char*)USER_STRING,m_database_used, GetNumMB(), GetLatestTime());
			T_Message msg={0,};
			if (srcmessage) msg.message_guid=srcmessage->message_guid;
			else msg.message_guid=g_searchcache[0]->search_id;
			msg.data=repl->Make();
			if (msg.data) {
				msg.message_type=MESSAGE_SEARCH_REPLY;
				msg.message_length=msg.data->GetLength();
				if (mqueuesend) mqueuesend->send(&msg);
				else {
					gm(&msg,NULL,0);
					msg.data->Unlock();
				};

			};
			return;
		};
		if (strnicmp(searchstring+1,machinename,strlen(machinename)) ||
			(searchstring[1+strlen(machinename)]!='/' &&
			searchstring[1+strlen(machinename)]))
		{
			machinename=buf;
			if (strnicmp(searchstring+1,machinename,strlen(machinename)) ||
				(searchstring[1+strlen(machinename)]!='/' &&
				searchstring[1+strlen(machinename)]))
				return;
		};

		searchstring+=1+strlen(machinename);
		while (*searchstring=='/') searchstring++;
		int is_wc=0;
		int is_tree=0;

		if (strlen(searchstring)>2 &&
			searchstring[strlen(searchstring)-1]=='s' &&
			searchstring[strlen(searchstring)-2]=='*')
		{
			searchstring[strlen(searchstring)-2]=0;
			is_tree=1;
		}
		else if (searchstring[0] && searchstring[strlen(searchstring)-1]=='*') {
			searchstring[strlen(searchstring)-1]=0;
			is_wc=1;
		};
		int x;
		char last[2048];
		last[0]=0;

		if (is_tree) {
			int sslen=strlen(searchstring);
			if (sslen) for (x = 0; x < m_database_used; x ++) {
				char buf[2048];
				char *d=m_dir_index[m_database[x].dir_index].dirname;
				unsigned int l=m_dir_index[m_database[x].dir_index].base_len+1;

				int a=l-2;
				while (a>0 && d[a]!='/' && d[a]!='\\') a--;
				if (a > 0) l=a+1;
				d+=l;

				sprintf(buf,"%s/%s",d,m_database[x].file);
				char *f=buf;
				while (*f) {
					if (*f == '\\') *f='/';
					f++;
				};
				if (!strnicmp(searchstring,buf,sslen) && buf[sslen]) {
					f=buf+sslen;
					if (*f == '/' && f >= buf) f--;
					while (f >= buf && *f != '/') f--;
					f++;

					if (!repl->would_fit(f,m_database[x].meta)) {
						T_Message msg={0,};
						if (srcmessage) msg.message_guid=srcmessage->message_guid;
						else msg.message_guid=g_searchcache[0]->search_id;
						msg.data=repl->Make();
						if (msg.data) {
							msg.message_type=MESSAGE_SEARCH_REPLY;
							msg.message_length=msg.data->GetLength();
							if (mqueuesend) mqueuesend->send(&msg);
							else {
								gm(&msg,NULL,NULL);
								msg.data->Unlock();
							};
						};
						repl->clear_items();
					};
					repl->add_item(m_database[x].v_index,f,m_database[x].meta,m_database[x].length_low,m_database[x].length_high,m_database[x].file_time);
				};
			} //tree loop
		}
		else {
			char *p=searchstring;
			while (*p) p++;
			while (p > searchstring && p[-1] == '/') p--;
			*p=0;
			int sslen=p-searchstring;

			unsigned int dbsize_errcnt=0;

			for (x = 0; x < m_database_used; x ++) {
				char buf[2048];
				char *d=m_dir_index[m_database[x].dir_index].dirname;
				unsigned int l=m_dir_index[m_database[x].dir_index].base_len+1;

				int a=l-2;
				while (a>0 && d[a]!='/' && d[a]!='\\') a--;
				if (a > 0) l=a+1;
				d+=l;

				char *f=m_database[x].file;
				sprintf(buf,"%s\\%s",d,f);
				f=buf;
				while (*f) {
					if (*f == '\\') *f='/';
					f++;
				};
				int is_dir=0;
				f=buf+sslen+1;
				while (*f && *f != '/') f++;
				if (*f == '/') {
					is_dir=1;
					*f=0;
				};

				if (!searchstring[0] || (!strnicmp(searchstring,buf,sslen) && (is_wc || buf[sslen]==0 || buf[sslen]=='/'))) {
					f=buf;
					while (*f) f++;
					while (f >= buf && (f[0] != '/' || !f[1])) f--;
					if (stricmp(++f,last)) {
						safe_strncpy(last,f,sizeof(last));
						if (!repl->would_fit(f,is_dir?(char*)DIRECTORY_STRING:m_database[x].meta)) {
							T_Message msg={0,};
							if (srcmessage) msg.message_guid=srcmessage->message_guid;
							else msg.message_guid=g_searchcache[0]->search_id;
							msg.data=repl->Make();
							if (msg.data) {
								msg.message_type=MESSAGE_SEARCH_REPLY;
								msg.message_length=msg.data->GetLength();
								if (mqueuesend) mqueuesend->send(&msg);
								else {
									gm(&msg,NULL,NULL);
									msg.data->Unlock();
								};
							};
							repl->clear_items();
						};
						int poosize=m_database[x].length_high;
						if (is_dir) {
							unsigned int o=dbsize_errcnt;
							dbsize_errcnt += m_database[x].length_low & 0xFFFFF;
							dbsize_errcnt &= 0xFFFFF;
							poosize=(m_database[x].length_high * 4096)+(m_database[x].length_low>>20) + (dbsize_errcnt < o);
						};

						repl->add_item(is_dir?-1:m_database[x].v_index,f,is_dir?(char*)DIRECTORY_STRING:m_database[x].meta,
							is_dir?1:m_database[x].length_low,poosize,m_database[x].file_time);
						dbsize_errcnt=0;
					}
					else {
						unsigned int o=dbsize_errcnt;
						dbsize_errcnt += m_database[x].length_low & 0xFFFFF;
						dbsize_errcnt &= 0xFFFFF;

						repl->addlastsize(1,(m_database[x].length_high * 4096)+(m_database[x].length_low>>20) + (dbsize_errcnt < o),m_database[x].file_time); //if dbsize_errcnt wraps, then we should tack another mb on there
					};
				};
			};
		};
		if (repl->get_numitems()) {
			T_Message msg={0,};
			if (srcmessage) msg.message_guid=srcmessage->message_guid;
			else msg.message_guid=g_searchcache[0]->search_id;

			msg.data=repl->Make();
			if (msg.data) {
				msg.message_type=MESSAGE_SEARCH_REPLY;
				msg.message_length=msg.data->GetLength();
				if (mqueuesend) mqueuesend->send(&msg);
				else {
					gm(&msg,NULL,NULL);
					msg.data->Unlock();
				};
			};
		};
		return;
	};

	int x;
	char ssout[1024+1024];
	parselist(ssout,searchstring);

	for (x = 0; x < m_database_used; x ++) {
		char *d=m_dir_index[m_database[x].dir_index].dirname;
		unsigned int l=m_dir_index[m_database[x].dir_index].base_len+1;
		if (l >= strlen(d)) d=(char*)"";
		else d+=l;
		if (substr_search(m_database[x].file,d,ssout)) {
			char buf[2048];
			char *d=m_dir_index[m_database[x].dir_index].dirname;
			unsigned int l=m_dir_index[m_database[x].dir_index].base_len+1;

			int a=l-2;
			while (a>0 && d[a]!='/' && d[a]!='\\') a--;
			if (a > 0) l=a+1;
			d+=l;

			char *f=m_database[x].file;
			sprintf(buf,"/%s/%s/%s",g_regnick[0]?g_regnick:"?",d,f);
			f=buf;
			while (*f) {
				if (*f == '\\') *f='/';
				f++;
			};

			if (!repl->would_fit(buf,m_database[x].meta)) break;
			repl->add_item(m_database[x].v_index,buf,m_database[x].meta,m_database[x].length_low,m_database[x].length_high,m_database[x].file_time);
		};
	};

	if (repl->get_numitems()) {
		T_Message msg={0,};
		if (srcmessage) msg.message_guid=srcmessage->message_guid;
		else msg.message_guid=g_searchcache[0]->search_id;
		msg.data=repl->Make();
		if (msg.data) {
			msg.message_type=MESSAGE_SEARCH_REPLY;
			msg.message_length=msg.data->GetLength();
			if (mqueuesend) mqueuesend->send(&msg);
			else {
				gm(&msg,NULL,NULL);
				msg.data->Unlock();
			};
		};
	};
}

void C_FileDB::alloc_dir_index()
{
	if (m_dir_index_used >= m_dir_index_size) {
		if (m_dir_index) {
			m_dir_index_size+=512;
			if (m_dir_index_size < m_dir_index_used) m_dir_index_size=m_dir_index_used+512;
			m_dir_index= (DirIndexType*) DB_LH_REALLOC(m_dir_index,m_dir_index_size*sizeof(DirIndexType));
		}
		else {
			m_dir_index_size=512;
			if (m_dir_index_size < m_dir_index_used) m_dir_index_size=m_dir_index_used+512;
			m_dir_index = (DirIndexType*) DB_LH_ALLOC(m_dir_index_size*sizeof(DirIndexType));
		};
	};
}

void C_FileDB::alloc_entry()
{
	if (m_database_used >= m_database_size) {
		if (m_database) {
			m_database_size+=2048;
			if (m_database_size < m_database_used) m_database_size=m_database_used+2048;
			m_database = (dbType *) DB_LH_REALLOC(m_database,m_database_size*sizeof(dbType));
		}
		else {
			m_database_size=2048;
			if (m_database_size < m_database_used) m_database_size=m_database_used+2048;
			m_database = (dbType *) DB_LH_ALLOC(m_database_size*sizeof(dbType));
		};
	};
}

int C_FileDB::in_list(char *list, char *v)
{
	while (*list) {
		if (!*v && !::strcmp(list,".")) return 1;
		if (!::stricmp(v,list) || !::strcmp("*",list)) return 1;
		list+=strlen(list)+1;
	};
	return 0;
}

int C_FileDB::GetFile(int index, char *file, char *meta, int *length_low, int *length_high, char **sharebaseptr)
{
	if (index < 0) return 1;
	int x;
	for (x = 0; x < m_database_used && m_database[x].v_index != index; x ++);
	if (x == m_database_used) return 1;

	if (file) sprintf(file,"%s%c%s",m_dir_index[m_database[x].dir_index].dirname,DIRCHAR,m_database[x].file);
	if (meta) {
		if (m_database[x].meta) safe_strncpy(meta,m_database[x].meta,64);
		else meta[0]=0;
	};
	if (length_low) *length_low=m_database[x].length_low;
	if (length_high) *length_high=m_database[x].length_high;
	if (sharebaseptr&&file) *sharebaseptr = file+m_dir_index[m_database[x].dir_index].base_len+1;
	return 0;
}

#ifdef _WIN32
	void C_FileDB::UnixTimeToFileTime(FILETIME *ft, unsigned int t)
	{
		SYSTEMTIME st={0,};
		st.wYear=1970;
		st.wDay=1;
		st.wMonth=1;
		SystemTimeToFileTime(&st,ft);
		((ULARGE_INTEGER *)ft)->QuadPart += 10000000i64*(__int64)t;
	}

	unsigned int C_FileDB::FileTimeToUnixTime(FILETIME *ft)
	{
		FILETIME lt2;
		SYSTEMTIME st={0,};
		st.wYear=1970;
		st.wDay=1;
		st.wMonth=1;
		if (!SystemTimeToFileTime(&st,&lt2)) return 0;

		ULARGE_INTEGER end;
		memcpy(&end,ft,sizeof(end));
		end.QuadPart -= ((ULARGE_INTEGER *)&lt2)->QuadPart;
		end.QuadPart /= 10000000; // 100ns -> seconds
		return (unsigned int)end.QuadPart;
	}
#endif

int C_FileDB::DoScan(int maxtime, C_FileDB *oldDB)
{
	#ifdef _WIN32
		WIN32_FIND_DATA d;
		memset(&d,0,sizeof(WIN32_FIND_DATA));
	#else
		struct dirent *d=NULL;
	#endif
	unsigned int endt=GetTickCount()+maxtime;
	if (!m_scan_stack) return -1;

	if (!m_database_used) { //first run
		if (!oldDB) {
			m_use_oldidx=0;
			m_scanidx_gpos=0; //restart from beginning on initial scan
		}
		else {
			m_use_oldidx = (m_scanidx_gpos = oldDB->m_scanidx_gpos) < SCAN_MAX_POS;
			if (!m_use_oldidx) m_scanidx_gpos=0; //oops we wrapped
		};
	};
	for(;;) {
		ScanType s;
		if (m_scan_stack->Pop(s)) {
			//dbg_printf(ds_Debug,"scan complete, read meta=%d, cached=%d",g_meta_stat[0],g_meta_stat[1]);
			delete m_scan_stack;
			m_scan_stack=0;
			return m_database_used;
		};
		#ifdef _WIN32
			int fnf=1;
			if (s.h == INVALID_HANDLE_VALUE)
		#else
			if (s.dir_h == 0)
		#endif
			{
				char maskstr[2048];
				if (!s.cur_path[0]) continue;
				#ifdef _WIN32
					sprintf(maskstr,"%s\\*.*",s.cur_path);
					s.h=::FindFirstFile(maskstr,&d);
					if (s.h == INVALID_HANDLE_VALUE) {
						dbg_printf(ds_Debug,"C_FileDB::DoScan(): error calling findfirstfile(%s)!",maskstr);
						continue;
					};
					fnf=0;
				#else
					s.dir_h=opendir(s.cur_path);
					if (s.dir_h == 0) {
						log_printf(ds_Warning,"C_FileDB::DoScan(): error calling findfirstfile(%s)!",s.cur_path);
						continue;
					};

				#endif
			};
			#ifdef _WIN32
				if (!fnf || ::FindNextFile(s.h,&d))
			#else
				if ((d=readdir(s.dir_h)))
			#endif
				{
					do
					{
						#ifdef _WIN32
							if (
								(d.cFileName[0] != '.') &&
								(!(d.dwFileAttributes&FILE_ATTRIBUTE_HIDDEN)) &&
								(!(d.dwFileAttributes&FILE_ATTRIBUTE_SYSTEM))
								)
						#else
							if (d->d_name[0] != '.')
						#endif
							{
								#ifdef _WIN32
									if (!(d.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY))
								#else
									int t=d->d_type;
									int fsize=0;
									if (!t) {
										char buf[4096];
										sprintf(buf,"%s/%s",s.cur_path,d->d_name);
										struct stat s;
										stat(buf,&s);
										if (s.st_mode & S_IFDIR) t=DT_DIR;
										fsize=s.st_size;
									};
									if (t != DT_DIR)
								#endif
									{
										bool ispending=false;
										{
											static const char sep=DIRCHAR;
											char *cd,*cp;
											cd=s.cur_path;
											#ifdef _WIN32
												cp=d.cFileName;
											#else
												cp=d->d_name;
											#endif
											char buf[4096],*pbuf2;
											int len=strlen(cd);
											int len2=strlen(cp);
											if (len+1+len2+strlen(szDotWastestate)<4096) {
												pbuf2=buf;
												strcpy(pbuf2,cd);pbuf2+=len;
												*pbuf2=sep;pbuf2++;*pbuf2=0;
												strcpy(pbuf2,cp);pbuf2+=len2;
												strcpy(pbuf2,szDotWastestate);
												#ifdef _WIN32
													ispending=GetFileAttributes(buf)!=INVALID_FILE_ATTRIBUTES;
												#else
													struct stat s;
													ispending=!stat(buf,&s);
												#endif
											};
										};

										#ifdef _WIN32
											char *p=::extension(d.cFileName);
										#else
											char *p=::extension(d->d_name);
										#endif

										if (!ispending && ((stricmp(p,szWastestate)!=0)||(strlen(p)!=strlen(szWastestate))) && in_list(m_ext_list,p)) {
											if (s.dir_index==-1) {
												alloc_dir_index();
												s.dir_index=m_dir_index_used;
												m_dir_index[m_dir_index_used].dirname=DB_LH_STRDUP(s.cur_path);
												m_dir_index[m_dir_index_used].base_len=s.base_len;
												m_dir_index_used++;
												m_oldscan_dirstate=0;
											};
											alloc_entry();
											m_database[m_database_used].meta=0;
											m_database[m_database_used].dir_index=s.dir_index;
											#ifdef _WIN32
												m_database[m_database_used].file=DB_LH_STRDUP(d.cFileName);
												m_database[m_database_used].length_high=d.nFileSizeHigh;
												m_database[m_database_used].length_low=d.nFileSizeLow;

												m_database[m_database_used].file_time=FileTimeToUnixTime(&d.ftLastWriteTime);
											#else
												m_database[m_database_used].file=DB_LH_STRDUP(d->d_name);

												m_database[m_database_used].length_high=0; //fucko64 for bsd
												{
													char buf[4096];
													sprintf(buf,"%s/%s",s.cur_path,d->d_name);
													struct stat s;
													stat(buf,&s);
													m_database[m_database_used].file_time=s.st_mtime;
													if (!fsize) m_database[m_database_used].length_low=s.st_size;
													else m_database[m_database_used].length_low=fsize;
												};
											#endif

											if (m_database[m_database_used].file_time > m_database_newesttime)
												m_database_newesttime=m_database[m_database_used].file_time;

											int needidx=1;
											int needmeta=1;

											if (oldDB) {
												if (!m_oldscan_dirstate)
													//search for our directory
												{
													int x;
													int v=m_oldscan_lastdirpos;
													int lastidx=0x7FFFFFFF;
													m_oldscan_dirstate=1;

													for (x = 0; x < oldDB->m_database_used; x ++) {
														int thisidx=oldDB->m_database[v].dir_index;
														if (thisidx != lastidx) {
															lastidx=thisidx;
															if (!strcmp(
																oldDB->m_dir_index[thisidx].dirname,
																m_dir_index[m_database[m_database_used].dir_index].dirname)
																)
															{
																m_oldscan_dirstate=2;
																m_oldscan_lastdirpos=v;
																break;
															};
														};
														v++;
														if (v == oldDB->m_database_used) v=0;
													};
												};

												if (m_oldscan_dirstate==2) {
													int pos;
													int lastidx=oldDB->m_database[m_oldscan_lastdirpos].dir_index;
													for (pos = 0; pos < oldDB->m_database_used; pos ++) {
														int a=pos+m_oldscan_lastdirpos;
														if (a > oldDB->m_database_used) break;

														dbType *dbp=oldDB->m_database+a;
														if (lastidx != dbp->dir_index) break;

														if (!strcmp(m_database[m_database_used].file,dbp->file)) {
															if (m_database[m_database_used].length_low == dbp->length_low && m_database[m_database_used].length_high == dbp->length_high &&
																m_database[m_database_used].file_time == dbp->file_time)
															{
																if (m_use_oldidx) {
																	m_database[m_database_used].v_index=dbp->v_index;
																	needidx=0;
																};
																dbg_printf(ds_Debug,"got cached metadata for '%s'='%s'",m_database[m_database_used].file,dbp->meta);
																if (dbp->meta) m_database[m_database_used].meta=DB_LH_STRDUP(dbp->meta);
																needmeta=0;
															};
															m_oldscan_lastpos=a+1;
															break;
														};
													};
												};
											};

											if (needmeta) {
												#ifdef _WIN32
													char *t=extension(d.cFileName);
												#else
													char *t=extension(d->d_name);
												#endif
												int ismp3=!stricmp(t,"mp3") || !stricmp(t,"mp2") || !stricmp(t,"mp1");
												int isjpg=!stricmp(t,"jpg") || !stricmp(t,"jpeg")|| !stricmp(t,"jfif");
												if (ismp3||isjpg) {
													char str[1024];
													str[0]=0;
													#ifdef _WIN32
														::sprintf(str,"%s\\%s",s.cur_path,d.cFileName);
														HANDLE hf=::CreateFile(str,GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,0,NULL);
														if (hf != INVALID_HANDLE_VALUE)
													#else
														::sprintf(str,"%s/%s",s.cur_path,d->d_name);
														FILE *hf=fopen(str,"rb");
														if (hf)
													#endif
														{
															char mbuf[256];
															memset(mbuf,0,sizeof(mbuf));
															if (ismp3) mp3_getmetainfo(hf,mbuf,m_database[m_database_used].length_low);
															else if (isjpg) jpg_getmetainfo(hf,mbuf,m_database[m_database_used].length_low);
															mbuf[63]=0;
															if (mbuf[0]) m_database[m_database_used].meta=DB_LH_STRDUP(mbuf);
															#ifdef _WIN32
																CloseHandle(hf);
															#else
																fclose(hf);
															#endif
														};
														dbg_printf(ds_Debug,"read metadata for '%s'='%s'",m_database[m_database_used].file,m_database[m_database_used].meta);
												};
											};
											if (needidx) {
												m_database[m_database_used].v_index=m_scanidx_gpos++;
											};

											//HACK init but unref
											//int o=m_database_xbytes;
											m_database_xbytes+=m_database[m_database_used].length_low&0xFFFFF;
											m_database_xbytes &= 0xFFFFF;
											if (m_database_xbytes < 0) m_database_mb++;
											m_database_mb+=(m_database[m_database_used].length_high*4096) + (m_database[m_database_used].length_low>>20);
											m_database_used++;
										};
									}
									else {
										ScanType l;
										#ifdef _WIN32
											::sprintf(l.cur_path,"%s\\%s",s.cur_path,d.cFileName);
											l.h=INVALID_HANDLE_VALUE;
										#else
											::sprintf(l.cur_path,"%s/%s",s.cur_path,d->d_name);
											l.dir_h=0;
										#endif
										l.base_len=s.base_len;
										l.dir_index=-1;
										m_scan_stack->Push(l);
										dbg_printf(ds_Debug,"Adding directory: '%s'",l.cur_path);
									};
							};
							int a=GetTickCount()-endt;
							if (a>=0) {
								m_scan_stack->Push(s);
								return m_database_used;
							};
					} while (
						#ifdef _WIN32
							::FindNextFile(s.h,&d)
						#else
							(d=readdir(s.dir_h))
						#endif
						);
	};
	#ifdef _WIN32
		::FindClose(s.h);
	#else
		closedir(s.dir_h);
	#endif
};
//HACK never exits here
//return m_database_used;
}//returns number of files scanned, or -1 if done

//begin xing
#define FRAMES_FLAG     0x0001
#define BYTES_FLAG      0x0002
#define TOC_FLAG        0x0004
#define VBR_SCALE_FLAG  0x0008

#define FRAMES_AND_BYTES (FRAMES_FLAG | BYTES_FLAG)

//structure to receive extracted header
//toc may be NULL
struct XHEADDATA
{
	int h_id;       //from MPEG header, 0=MPEG2, 1=MPEG1
	int samprate;   //determined from MPEG header
	int flags;      //from Xing header data
	int frames;     //total bit stream frames from Xing header data
	int bytes;      //total bit stream bytes from Xing header data
	int vbr_scale;  //encoded vbr scale from Xing header data
	unsigned char *toc;  //pointer to unsigned char toc_buffer[100]
	//may be NULL if toc not desired
};

static int ExtractI4(unsigned char *buf)
{
	//big endian extract
	return DataUIntSwap4(buf);
}
/*-------------------------------------------------------------*/
static int GetXingHeader(XHEADDATA *X,  unsigned char *buf)
{
	int i, head_flags;
	int h_id, h_mode, h_sr_index;
	static int sr_table[4] = { 44100, 48000, 32000, 99999 };
	//HACK init but unref
	//char *obuf=(char*)buf;
	//get Xing header data

	X->flags = 0;     //clear to null incase fail

	//get selected MPEG header data
	h_id       = (buf[1] >> 3) & 1;
	h_sr_index = (buf[2] >> 2) & 3;
	h_mode     = (buf[3] >> 6) & 3;

	//determine offset of header
	if( h_id ) {        //mpeg1
		if( h_mode != 3 ) buf+=(32+4);
		else              buf+=(17+4);
	}
	else {      //mpeg2
		if( h_mode != 3 ) buf+=(17+4);
		else              buf+=(9+4);
	};

	if (memcmp(buf,"Xing",4) &&
		memcmp(buf,"Lame",4)) return 0;
	buf+=4;

	X->h_id = h_id;
	X->samprate = sr_table[h_sr_index];
	if( h_id == 0 ) X->samprate >>= 1;

	head_flags = X->flags = ExtractI4(buf); buf+=4;      //get flags

	if( head_flags & FRAMES_FLAG ) {X->frames   = ExtractI4(buf); buf+=4;}
	if( head_flags & BYTES_FLAG )  {X->bytes = ExtractI4(buf); buf+=4;}

	if( head_flags & TOC_FLAG ) {
		if( X->toc != NULL ) {
			for(i=0;i<100;i++) X->toc[i] = buf[i];
		};
		buf+=100;
	};

	X->vbr_scale = -1;
	if( head_flags & VBR_SCALE_FLAG )  {X->vbr_scale = ExtractI4(buf); buf+=4;}

	//if( X->toc != NULL ) {
	//for(i=0;i<100;i++) {
	//   if( (i%10) == 0 ) printf("\n");
	//   printf(" %3d", (int)(X->toc[i]));
	//}
	//}
	return 1;       //success
}
//end xing

#ifdef _WIN32
	void C_FileDB::jpg_getmetainfo(HANDLE hFile,char *meta, int /*filelen*/)
#else
	void C_FileDB::jpg_getmetainfo(FILE *fp,char *meta, int /*filelen*/)
#endif
	{
		unsigned char buf[32768];
		unsigned char *p=buf;
		int d=0;
		#ifdef _WIN32
			ReadFile(hFile,buf,sizeof(buf),(LPDWORD) &d,NULL);
		#else
			d=fread(buf,1,sizeof(buf),fp);
		#endif
		if (d < 10 || buf[0] != 0xFF || buf[1] != 0xD8) return;
		d-=2;
		p+=2;
		while (d > 8) {
			while (d > 8 && *p != 0xff) { p++; d--; }
			while (d > 8 && *p == 0xff) { p++; d--; }
			if (d >= 8) {
				if (*p >= 0xC0 && *p <= 0xC3) {
					p++;
					int w=(p[5] << 8) | p[6];
					int h=(p[3] << 8) | p[4];
					if (w && h) sprintf(meta,"JPEG %dx%d",w,h);
					return;
				};
				p++;
				d--;
				int l=(p[0]<<8) | p[1];
				p+=l;
				d-=l;
			};
		};
		dbg_printf(ds_Debug,"ran out of data...");

	}

	#ifdef _WIN32
		void C_FileDB::mp3_getmetainfo(HANDLE hFile,char *meta, int filelen)
	#else
		void C_FileDB::mp3_getmetainfo(FILE *fp,char *meta, int filelen)
	#endif
		{
			int t_sampling_frequency[2][2][3] = {
				{
					{ 11025 , 12000 , 8000},
					{ 11025 , 12000 , 8000}
				},
				{
					{ 22050 , 24000 , 16000},
					{ 44100 , 48000 , 32000}
				}
			};

			short t_bitrate[2][3][15] = {{
				{0,32,48,56,64,80,96,112,128,144,160,176,192,224,256},
				{0,8,16,24,32,40,48,56,64,80,96,112,128,144,160},
				{0,8,16,24,32,40,48,56,64,80,96,112,128,144,160}
			},{
				{0,32,64,96,128,160,192,224,256,288,320,352,384,416,448},
				{0,32,48,56,64,80,96,112,128,160,192,224,256,320,384},
				{0,32,40,48,56,64,80,96,112,128,160,192,224,256,320}
			}};
			char *t=meta;
			int is_vbr=0,is_vbr_lens=0;

			int flen,r,id3_skip=0;
			char buf[32768]={0,};
			unsigned char *a = (unsigned char *)buf;
			#ifdef _WIN32
				if (!ReadFile(hFile,buf,sizeof(buf),(LPDWORD)&r,NULL)) r=0;
			#else
				r=fread(buf,1,sizeof(buf),fp);
			#endif
			{
				unsigned char *id3v2tag= (unsigned char *)buf;
				if (id3v2tag[0]=='I'&&
					id3v2tag[1]=='D'&&
					id3v2tag[2]=='3'&&
					id3v2tag[3]!=255&&
					id3v2tag[4]!=255&&
					id3v2tag[6]<0x80&&
					id3v2tag[7]<0x80&&
					id3v2tag[8]<0x80&&
					id3v2tag[9]<0x80)
				{
					int l=0;
					l+=((int)id3v2tag[6])<<21;
					l+=((int)id3v2tag[7])<<14;
					l+=((int)id3v2tag[8])<<7;
					l+=((int)id3v2tag[9])<<0;
					l+=10;
					if (l < filelen) id3_skip=l;
					#ifdef _WIN32
						SetFilePointer(hFile,id3_skip,NULL,FILE_BEGIN);
						if (!ReadFile(hFile,buf,sizeof(buf),(LPDWORD)&r,NULL)) r=0;
					#else
						fseek(fp,id3_skip,SEEK_SET);
						r=fread(buf,1,sizeof(buf),fp);
					#endif
				};
			};
			{
				XHEADDATA Xing_hdr={0,};
				if (GetXingHeader(&Xing_hdr,(unsigned char *)buf) && Xing_hdr.flags&FRAMES_FLAG) {
					is_vbr=Xing_hdr.frames;
					#ifdef _WIN32
						is_vbr_lens=Xing_hdr.samprate?MulDiv(Xing_hdr.frames,Xing_hdr.h_id?1152:576,Xing_hdr.samprate):0;
					#else
						is_vbr_lens=Xing_hdr.samprate?(Xing_hdr.frames*(Xing_hdr.h_id?1152:576)/Xing_hdr.samprate):0;
					#endif
				};
			};
			flen = filelen;
			flen-=id3_skip;
			while (r-- >= 0) {
				unsigned int v = DataUIntSwap4(a);
				int layer, bitrate, ID, IDex, sfreq,mode;
				a++;
				bitrate = (v>>12)&15;
				sfreq = (v>>10)&3;
				mode = (v>>6)&3;
				ID = (v>>(32-13))&1;
				IDex = (v>>(32-12))&1;
				layer = (v>>17)&3;

				if ((v >> (32-11)) == 0x7ff && bitrate != 15 && bitrate && layer && sfreq != 3 && (IDex || !ID)) {
					char *modes[4]={"Stereo","Joint Stereo","2 Channel","Mono"};

					int br=(t_bitrate[ID][3-layer][bitrate]*1000);
					if (!is_vbr) is_vbr_lens=(flen*8)/(br?br:1);
					t[0]=0;
					sprintf(t+strlen(t),"MP%d",4-layer);
					if (!is_vbr) sprintf(t+strlen(t)," %dkbps",t_bitrate[ID][3-layer][bitrate]);
					else sprintf(t+strlen(t)," %dkbps VBR",(flen*8)/(is_vbr_lens?is_vbr_lens*1000:1));
					sprintf(t+strlen(t)," %dkHz %s",t_sampling_frequency[IDex][ID][sfreq]/1000,modes[mode]);
					sprintf(t+strlen(t)," %d:%02d",is_vbr_lens/60,is_vbr_lens%60);
					break;
				};
			};
		}

		void C_FileDB::writeOut(char *fn)
		{
			struct
			{
				#ifdef _WIN32
					char file[MAX_PATH];
				#else
					char file[256];
				#endif
				char meta[64];
				int dir_index;
				int length_low;
				int length_high;
				int file_time;
				int v_index;
			} db;
			struct
			{
				char dirname[1024];
				int base_len;
			} di;

			FILE *fp=fopen(fn,"wb");
			if (fp) {
				unsigned int d=0xDBDBF11B;
				fwrite(&d,1,4,fp);
				d=4 + 4 + 4 + 4 + 4 + 4 + sizeof(db)*m_database_used + sizeof(di) * m_dir_index_used;
				fwrite(&d,1,4,fp);
				fwrite(&m_database_mb,1,4,fp);
				fwrite(&m_database_xbytes,1,4,fp);
				fwrite(&m_database_newesttime,1,4,fp);
				fwrite(&m_dir_index_used,1,4,fp);
				fwrite(&m_scanidx_gpos,1,4,fp);
				if (m_dir_index && m_dir_index_used) {
					int x;
					for (x = 0; x < m_dir_index_used; x ++) {
						safe_strncpy(di.dirname,m_dir_index[x].dirname,sizeof(di.dirname));
						di.base_len=m_dir_index[x].base_len;
						fwrite(&di,sizeof(di),1,fp);
					};
				};
				fwrite(&m_database_used,1,4,fp);
				if (m_database && m_database_used) {
					int x;
					for (x = 0; x < m_database_used; x ++) {
						safe_strncpy(db.file,m_database[x].file,sizeof(db.file));
						memset(db.meta,0,sizeof(db.meta));
						if (m_database[x].meta) safe_strncpy(db.meta,m_database[x].meta,sizeof(db.meta));
						db.dir_index=m_database[x].dir_index;
						db.length_low=m_database[x].length_low;
						db.length_high=m_database[x].length_high;
						db.file_time=m_database[x].file_time;
						db.v_index=m_database[x].v_index;
						fwrite(&db,sizeof(db),1,fp);
					};
				};
				fclose(fp);
			};
		}

		int C_FileDB::readIn(char *fn)
		{
			struct
			{
				#ifdef _WIN32
					char file[MAX_PATH];
				#else
					char file[256];
				#endif
				char meta[64];
				int dir_index;
				int length_low;
				int length_high;
				int file_time;
				int v_index;
			} db;
			struct
			{
				char dirname[1024];
				int base_len;
			} di;

			FILE *fp=fopen(fn,"rb");

			if (fp) {
				fseek(fp,0,SEEK_END);
				int flen=ftell(fp);
				fseek(fp,0,SEEK_SET);
				unsigned int d;

				clearDBs();

				if (fread(&d,1,4,fp) == 4 && d == 0xDBDBF11B) {
					unsigned int len;
					if (fread(&len,1,4,fp) == 4 && len+8 == (unsigned int)flen) {
						fread(&m_database_mb,1,4,fp);
						fread(&m_database_xbytes,1,4,fp);
						fread(&m_database_newesttime,1,4,fp);
						fread(&m_dir_index_used,1,4,fp);
						fread(&m_scanidx_gpos,1,4,fp);

						if (m_dir_index_used) {
							alloc_dir_index();
							if (m_dir_index) {
								int x;
								for (x = 0; x < m_dir_index_used; x ++) {
									fread(&di,sizeof(di),1,fp);
									m_dir_index[x].dirname=DB_LH_STRDUP(di.dirname);
									m_dir_index[x].base_len=di.base_len;
								};
								fread(&m_database_used,1,4,fp);
								if (m_database_used) {
									alloc_entry();
									if (m_database) {
										int x;
										for (x = 0; x < m_database_used; x ++) {
											fread(&db,sizeof(db),1,fp);
											m_database[x].file=DB_LH_STRDUP(db.file);
											m_database[x].meta=db.meta[0]?DB_LH_STRDUP(db.meta):0;
											m_database[x].dir_index=db.dir_index;
											m_database[x].v_index=db.v_index;
											m_database[x].length_low=db.length_low;
											m_database[x].length_high=db.length_high;
											m_database[x].file_time=db.file_time;
										};
										fclose(fp);
										return 1;
									};
								};
							};
						};
					};
				};
				fclose(fp);
			};
			return 0;
		}
