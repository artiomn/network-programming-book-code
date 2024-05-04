/*
WASTE - xfers.hpp (File transfer implementation)
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

#ifndef _XFERS_H_
	#define _XFERS_H_

	#include "m_file.hpp"

	#define WASTESTATE_SIG1 0xFFFFFEFE
	#define WASTESTATE_SIG2 0x00000101

	#ifdef _WIN32
		#if 0
			#include <sys/stat.h>
			#ifndef _DEBUG
				#error remove 1
			#endif
		#else
			#define XFER_WIN32_FILEIO
		#endif
	#endif

	class XferSend
	{
	public:
		XferSend(C_MessageQueueList *mql, T_GUID *guid, C_FileSendRequest *req, char *fn);
		~XferSend();

		void set_guid(T_GUID *guid) { m_guid = *guid; }
		T_GUID *get_guid() { return &m_guid; }

		int get_idx() { return m_idx; }

		int run_hdr(C_MessageQueueList *mql);
		void run(C_MessageQueueList *mql);
		void onGotMsg(C_FileSendRequest *req);

		void Abort(C_MessageQueueList *mql);

		char *GetError() { return m_err; }

		char *GetName() { return m_fn; }
		void GetSize(unsigned int *low, unsigned int *high) { *low=m_filelen_bytes_l; *high=m_filelen_bytes_h; }

		int GetIdx() { return m_idx; }

	private:
		void updateStatusText();

		time_t m_last_talktime;
		unsigned int m_lastsendtime;
		char *m_err;
		char m_err_buf[256];
		char m_fn[512];
		unsigned int m_create_date, m_mod_date;
		T_GUID m_guid;
		#ifdef XFER_WIN32_FILEIO
			HANDLE m_hfile;
		#else
			FILE *m_file;
		#endif
		int m_idx;
		int m_need_reply;
		unsigned int m_filelen_bytes_l, m_filelen_bytes_h;
		unsigned int m_filelen_chunks;
		unsigned int m_lastpos_l, m_lastpos_h;

		unsigned int chunks_to_send[FILE_MAX_CHUNKS_PER_REQ];
		unsigned int chunks_to_send_pos,chunks_to_send_len;

		unsigned int m_starttime, m_lastchunkcnt;
		unsigned int m_chunks_sent_total,m_max_chunksent;
		int m_last_cps;

		C_FileSendReply m_reply;
	};

	class XferRecv
	{
	public:
		XferRecv(C_MessageQueueList *mql, const char *guididx, const char *sizestr, const char *filename, const char *path);
		~XferRecv();

		int run(C_MessageQueueList *mql);
		void onGotMsg(C_FileSendReply *reply);

		char *GetError() { return m_err; }
		void Abort(C_MessageQueueList *mql);

		T_GUID *get_guid() { return &m_guid; }
		char *getActualOutputFile() { return m_outfile_fn; }
		char *getOutputFileCopy(); //returns a filename of a copy of the file
		const char *get_nick() { return m_nick; }

	private:
		char *m_statfile_fn;
		char *m_outfile_fn;
		int m_outfile_fn_ll;
		time_t last_msg_time;
		char *m_err;
		char m_errbuf[256];
		C_FileSendReply *lasthdr;
		int chunks_coming;
		T_GUID m_guid;
		char *m_tmpcopyfn;

		#ifdef XFER_WIN32_FILEIO
			HANDLE m_houtfile;
			HANDLE m_hstatfile;
		#else
			FILE *m_outfile;
			FILE *m_statfile;
		#endif
		unsigned int m_cps_blks[CPS_WINDOWSIZE];
		unsigned int m_next_cpstime;
		int m_cps_blks_pos;
		int m_last_cps;

		int m_path_len;

		unsigned int m_outfile_lastpos_l,m_outfile_lastpos_h;
		unsigned char *m_validbf;
		unsigned char m_hash[SHA_OUTSIZE];
		char m_nick[32];
		C_FileSendRequest request;
		unsigned int m_chunk_cnt, m_chunk_total,m_chunk_startcnt;
		unsigned int m_first_chunkilack;
		unsigned int m_create_date, m_mod_date;
		unsigned int m_bytes_total_l,m_bytes_total_h;
		unsigned int m_total_chunks_recvd;
		int m_done;
		int m_hasgotchunks;
		time_t m_next_stateflush_time;

		int m_adaptive_chunksize;
	};

#endif//_XFERS_H_

