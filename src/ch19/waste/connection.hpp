/*
WASTE - connection.hpp (Secured TCP connection class)
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

#ifndef _C_CONNECTION_H_
#define _C_CONNECTION_H_

#include "asyncdns.hpp"
#include "blowfish.hpp"
#include "rsa/global.hpp"
#include "rsa/rsaref.hpp"
#include "sha.hpp"

#define MAX_CONNECTION_SENDSIZE 32768
#define MAX_CONNECTION_RECVSIZE 32768
#define CONNECTION_BFPUBKEY 16          //size of blowfish key that encrypts the hash of public key, bytes
#define CONNECTION_PKEYHASHSIZE ((SHA_OUTSIZE+7)&~7) //size of hash of public key to be sent
#define CONNECTION_PADSKEYSIZE (4096/8) //max RSA session key encrypted size (max keysize in bytes)
#define CONNECTION_KEYSIZE 56           //size of actual session key, bytes

#include "packetdef.hpp"

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable:4201)
#endif

class _bfInit
{
	public:
		enum _flags {
			gotPKeyHash=1,
			sentPKeyHash=2,
			gotPad1=4,

			gotSKey=8,
			sentSKey=16,

			sentSYN=32,
		};
		enum _stage {
			stage1_hash=1,
			stage2_key=2,
			stage3_syn=3,
			stage4_complete
		};
		_bfInit() {i=0;};
		bool TestAll1(unsigned int mask);
		bool TestAll0(unsigned int mask);
		bool Get(_flags f);
		void Set(_flags f,bool value);
		unsigned int GetI();
		void SetI(unsigned int i);
		_stage GetStage();
	private:
		unsigned int i;
};

class C_Connection
{
public:

	enum state
	{
		STATE_ERROR, STATE_RESOLVING, STATE_CONNECTING,
		STATE_CONNECTED, STATE_IDLE, STATE_CLOSING, STATE_CLOSED, STATE_DIENOW
	};

	C_Connection(int s, struct sockaddr_in *loc=NULL);
	C_Connection(char *hostname, unsigned short port, C_AsyncDNS *dns=NULL);
	~C_Connection();

	state run(int max_send_bytes, int max_recv_bytes);

	void close(int quick=0);

	void set_max_sendsize(int value) { if (value < 64) value=64; if (value > MAX_CONNECTION_SENDSIZE) value=MAX_CONNECTION_SENDSIZE; m_remote_maxsend=value; };
	void set_saturatemode(int mode) { m_satmode=mode; };
	int get_saturatemode() { return m_satmode; };

	int getMaxSendSize();

	int send_bytes_in_queue();
	int send_bytes_available();
	int send_bytes(void *data, int length); //multiples of 8 only pls

	int recv_bytes_available();
	int recv_bytes(void *data, int maxlength); //multiples of 8

	unsigned long get_interface();
	unsigned long get_remote() { return m_saddr.sin_addr.s_addr; };
	unsigned short get_remote_port() { return m_remote_port; };

	int get_total_bytes_recv() { return m_recv_bytes_total; };
	int was_ever_connected() { return m_ever_connected; };
	void calc_bps(int *send, int *recv);

	void get_last_bps(int *send, int *recv)
	{
		if (send) *send=m_last_sendbps;
		if (recv) *recv=m_last_recvbps;
	};
	unsigned char *get_remote_pkey_hash() { return m_remote_pkey; };
	R_RSA_PUBLIC_KEY *get_remote_pkey() { return &remote_pubkey; };

	bool get_has_sent_remoteip() {return m_has_sent_remoteip;};
	void set_has_sent_remoteip() {m_has_sent_remoteip=true;};

	void PushRandomCrap();
	bool PopRandomCrap();
	int get_RandomCrapLen() {return m_iRecvRandomPadding3&(~7);};

protected:
	int  m_socket;
	unsigned short m_remote_port;
	char m_recv_buffer[MAX_CONNECTION_RECVSIZE];
	char m_send_buffer[MAX_CONNECTION_SENDSIZE];
	int  m_recv_pos;
	int  m_recv_len;
	int  m_send_pos;
	int  m_send_len;

	int m_remote_maxsend;
	int m_satmode;

	int m_last_sendbps, m_last_recvbps;

	int  m_send_bytes_total;
	int  m_recv_bytes_total;

	struct sockaddr_in m_saddr;
	char m_host[256];

	C_AsyncDNS *m_dns;

	void do_init();
	state m_state;
	int m_ever_connected;

	struct
	{
		unsigned int time_ms;
		unsigned int send_bytes;
		unsigned int recv_bytes;
	} bps_count[30];

	int bps_count_pos;
	unsigned int m_start_time;

	void encrypt_buffer(void *data, int len);
	void decrypt_buffer(void *data, int len);
	_bfInit bf_initted;
	bool m_cansend;
	void init_crypt();
	void init_crypt_gotpkey();
	void init_crypt_decodekey();

	_Keyinfo m_mykeyinfo;
	unsigned char m_local_sRand[CONNECTION_BFPUBKEY];
	int m_iSendRandomPadding1; //after first packet
	int m_iSendRandomPadding2; //after second packet
	int m_iSendRandomPadding3; //after sync
	int m_iRecvRandomPadding1; //after first packet
	int m_iRecvRandomPadding2; //after second packet
	int m_iRecvRandomPadding3; //after sync
	int m_InitialPacket2Add; // Additional bytes for PSK
	CBlowfish m_fish; //same key, but different IV'S!Main Crypt key
	CBlowfish m_fish_send; //initial obfuscating key(srand)
	CBlowfish m_fish_recv; //initial obfuscating key(srand)

	unsigned char m_remote_pkey[CONNECTION_PKEYHASHSIZE];

	R_RSA_PUBLIC_KEY remote_pubkey;

	bool m_has_sent_remoteip;
};

#ifdef _MSC_VER
#pragma warning(pop)
#endif

#endif //_C_CONNECTION_H_
