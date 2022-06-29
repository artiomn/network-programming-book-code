/*
WASTE - connection.cpp (Secured TCP connection class)
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
#include "util.hpp"
#include "connection.hpp"
#include "sockets.hpp"

#include "packetdef.hpp"

#include "rsa/r_random.hpp"
#include "rsa/rsa.hpp"


inline bool _bfInit::TestAll1(unsigned int mask)
{
	return (i&mask)==mask;
}

inline bool _bfInit::TestAll0(unsigned int mask)
{
	return (i&mask)==0;
}

inline bool _bfInit::Get(_bfInit::_flags f)
{
	return (i&f)!=0;
}

inline void _bfInit::Set(_bfInit::_flags f,bool value)
{
	i=(i&(~f))|(value?f:0);
}

inline unsigned int _bfInit::GetI()
{
	return i;
}

inline void _bfInit::SetI(unsigned int i)
{
	this->i=i;
}

_bfInit::_stage _bfInit::GetStage()
{
	if (!TestAll1(
		gotPKeyHash|
		sentPKeyHash|
		gotPad1
		))
	{
		return stage1_hash;
	}
	else if (!TestAll1(
		gotPKeyHash|
		sentPKeyHash|
		gotPad1|
		gotSKey|
		sentSKey
		))
	{
		return stage2_key;
	}
	else if (!TestAll1(
		gotPKeyHash|
		sentPKeyHash|
		gotPad1|
		gotSKey|
		sentSKey|
		sentSYN
		))
	{
		return stage3_syn;
	};
	return stage4_complete;
}

//incoming
C_Connection::C_Connection(int s, struct sockaddr_in *loc)
{
	m_iSendRandomPadding1=0;
	m_iSendRandomPadding2=0;
	m_iSendRandomPadding3=0;
	m_iRecvRandomPadding1=0;
	m_iRecvRandomPadding2=0;
	m_iRecvRandomPadding3=0;
	m_InitialPacket2Add=0;
	do_init();
	m_socket=s;
	SET_SOCK_BLOCK(m_socket,0);
	m_remote_port=0;
	m_state=STATE_CONNECTED;
	m_dns=NULL;
	m_ever_connected=0;
	m_has_sent_remoteip=false;
	if (loc) m_saddr=*loc;
	else memset(&m_saddr,0,sizeof(m_saddr));
	m_cansend=false;
}

//outgoing
C_Connection::C_Connection(char *hostname, unsigned short port, C_AsyncDNS *dns)
{
	m_iSendRandomPadding1=0;
	m_iSendRandomPadding2=0;
	m_iSendRandomPadding3=0;
	m_iRecvRandomPadding1=0;
	m_iRecvRandomPadding2=0;
	m_iRecvRandomPadding3=0;
	m_InitialPacket2Add=0;
	do_init();
	m_cansend=true;
	m_state=STATE_ERROR;
	m_ever_connected=0;
	m_has_sent_remoteip=false;
	m_dns=dns;
	m_remote_port=(short)port;
	m_socket=socket(AF_INET,SOCK_STREAM,0);
	if (m_socket==-1) {
		log_printf(ds_Error,"connection: call to socket() failed: %d.",ERRNO);
	}
	else {
		SET_SOCK_BLOCK(m_socket,0);
		safe_strncpy(m_host,hostname,sizeof(m_host));
		memset(&m_saddr,0,sizeof(m_saddr));
		m_saddr.sin_family=AF_INET;
		m_saddr.sin_port=htons(port);
		m_saddr.sin_addr.s_addr=inet_addr(hostname);
		m_state=STATE_RESOLVING;
	};
}

void C_Connection::init_crypt()
{
	// look at _InitialPacket for what's sent
	bf_initted.SetI(0);

	_InitialPacket1 &packsend1=*((_InitialPacket1*)m_send_buffer);

	GenerateRandomSeed(packsend1.sRand,sizeof(packsend1.sRand),&g_random);
	memcpy(m_local_sRand,packsend1.sRand,sizeof(m_local_sRand));

	memcpy(packsend1.sPubkeyHash,g_pubkeyhash,sizeof(g_pubkeyhash));
	RandomizePadding(packsend1.sPubkeyHash,sizeof(packsend1.sPubkeyHash),sizeof(g_pubkeyhash));

	m_fish_send.Init(packsend1.sRand,sizeof(packsend1.sRand));
	m_fish_send.SetIV(CBlowfish::IV_BOTH, 0, 0);

	//encrypt pubkey_A_HASH with randA.
	m_fish_send.EncryptECB(packsend1.sPubkeyHash,sizeof(packsend1.sPubkeyHash));

	if (g_use_networkhash && g_networkhash_PSK) {
		// this gives small info on our obfuscation key, but in std. mode
		// this bitch is seen by everyone. Btw we have enough keyspace left.
		// Hallo to you guys from P*PWatchDog... I hope you like my code ;)
		// Greetings from Md5Chap 8-)
		m_iSendRandomPadding1=DataUInt2(m_local_sRand)%4096;
		m_iSendRandomPadding2=DataUInt2(m_local_sRand+2)%4096;
		m_iSendRandomPadding3=DataUInt2(m_local_sRand+4)%4096;
		g_networkhash_PSK_fish.EncryptECB(packsend1.sRand, sizeof(packsend1.sRand));
		g_networkhash_PSK_fish.EncryptECB(packsend1.sPubkeyHash, sizeof(packsend1.sPubkeyHash));
		R_GenerateBytes((unsigned char*)&packsend1+sizeof(_InitialPacket1),m_iSendRandomPadding1,&g_random);
		m_InitialPacket2Add=CONNECTION_BFPUBKEY;
	};

	m_send_len=0;//fill in shit later
	m_send_len+=sizeof(_InitialPacket1);
	m_send_len+=m_iSendRandomPadding1;
	m_send_len+=sizeof(_InitialPacket2)+m_InitialPacket2Add;
	m_send_len+=m_iSendRandomPadding2;
	dbg_printf(ds_Debug,"sending public key portion, Pad(0x%x,0x%x,0x%x)",m_iSendRandomPadding1,m_iSendRandomPadding2,m_iSendRandomPadding3);
}

void C_Connection::init_crypt_gotpkey() //got their public key
{
	m_cansend=true;

	_InitialPacket1 &packrecv1=*((_InitialPacket1*)m_recv_buffer);
	_InitialPacket1 &packsend1=*((_InitialPacket1*)m_send_buffer);
	_InitialPacket2 &packsend2=*((_InitialPacket2*)(m_send_buffer+sizeof(_InitialPacket1)+m_iSendRandomPadding1));

	//check to make sure randB != randA
	if (!memcmp(packrecv1.sRand,packsend1.sRand,sizeof(packrecv1.sRand))) {
		log_printf(ds_Error,"connection: got what looks like feedback on bfpubkey.");
		close(1);
		return;
	};

	if (g_use_networkhash && g_networkhash_PSK) {
		g_networkhash_PSK_fish.DecryptECB(packrecv1.sRand, sizeof(packrecv1.sRand));
		g_networkhash_PSK_fish.DecryptECB(packrecv1.sPubkeyHash, sizeof(packrecv1.sPubkeyHash));
		m_iRecvRandomPadding1=DataUInt2(packrecv1.sRand)%4096;
		m_iRecvRandomPadding2=DataUInt2(packrecv1.sRand+2)%4096;
		m_iRecvRandomPadding3=DataUInt2(packrecv1.sRand+4)%4096;
		dbg_printf(ds_Debug,"Received Pad(0x%x,0x%x,0x%x)",m_iRecvRandomPadding1,m_iRecvRandomPadding2,m_iRecvRandomPadding3);
		//check to make sure randB != randA
		if (!memcmp(packrecv1.sRand,m_local_sRand,sizeof(packrecv1.sRand))) {
			log_printf(ds_Error,"connection: got what looks like feedback on sRand!!.");
			close(1);
			return;
		};
	};

	//temporary used for obfuscation
	m_fish_recv.Init(packrecv1.sRand,sizeof(packrecv1.sRand));
	m_fish_recv.SetIV(CBlowfish::IV_BOTH, 0, 0);

	//get pubkey_B_HASH
	memcpy(m_remote_pkey,packrecv1.sPubkeyHash,sizeof(m_remote_pkey));

	//decrypt pubkey_B_HASH with randB
	m_fish_recv.DecryptECB(m_remote_pkey, sizeof(m_remote_pkey));

	//find pubkey_B_HASH
	if (!findPublicKey(m_remote_pkey,&remote_pubkey)) {
		char hash[2*SHA_OUTSIZE+1];
		Bin2Hex(hash,m_remote_pkey,SHA_OUTSIZE);
		log_printf(ds_Informational,"Initial connect: Unknown Pubkey hash: %s",hash);
		close(1);
		return;
	};

	//at this point, generate my session key and IV (sKeyA)
	GenerateRandomSeed(m_mykeyinfo.raw,sizeof(m_mykeyinfo.sKey)+sizeof(m_mykeyinfo.sIV),&g_random);

	//append randB to sKeyA. Simply Mac
	memcpy(m_mykeyinfo.sRand,packrecv1.sRand,sizeof(m_mykeyinfo.sRand));

	//create a working copy of sKeyA+randB
	_Keyinfo localkeyinfo;
	memcpy(localkeyinfo.raw,m_mykeyinfo.raw,sizeof(localkeyinfo.raw));

	//if using a network name, encrypt the first 56 bytes of the sKeyA portion of localkeyinfo
	//now crypt the skey+iv shit. No one except a pubkey trusted fucking target can decrypt this shit.
	//so we can't use the srand as verifier for our nethash. who cares? the syn does it!
	if (g_use_networkhash) {
		g_networkhash_PSK_fish.EncryptECB(localkeyinfo.sKey,sizeof(localkeyinfo.sKey));
		g_networkhash_PSK_fish.EncryptECB(localkeyinfo.sIV,sizeof(localkeyinfo.sIV));
	};

	//encrypt using pubkey_B
	unsigned int l=0;
	if (
		RSAPublicEncrypt(
			packsend2.sPubCrypted,
			&l,
			localkeyinfo.raw,sizeof(localkeyinfo.raw),
			&remote_pubkey,
			&g_random)
		)
	{
		log_printf(ds_Error,"connection: error encrypting session key");
		close(1);
		return;
	};
	memset(localkeyinfo.raw,0,sizeof(localkeyinfo.raw));
	RandomizePadding(packsend2.sPubCrypted,sizeof(packsend2.sPubCrypted),l);

	if (g_use_networkhash && g_networkhash_PSK) {
		_InitialPacket2_PSK &p2psk=(_InitialPacket2_PSK&)packsend2;
		memcpy(p2psk.sRand,packrecv1.sRand,sizeof(p2psk.sRand));
		for (int i=0;i<(sizeof(p2psk.sRand)/sizeof(int));i++) ((unsigned int*)(p2psk.sRand))[i]^=(~0);
		g_networkhash_PSK_fish.EncryptECB(p2psk.sPubCrypted, sizeof(p2psk.sPubCrypted));
		g_networkhash_PSK_fish.EncryptECB(p2psk.sRand, sizeof(p2psk.sRand));
		R_GenerateBytes((unsigned char*)&packsend2+sizeof(_InitialPacket2)+m_InitialPacket2Add,m_iSendRandomPadding2,&g_random);
	};

	//obfuscate
	m_fish_send.EncryptPCBC(packsend2.sPubCrypted, sizeof(packsend2.sPubCrypted));
	if (g_use_networkhash && g_networkhash_PSK) {
		_InitialPacket2_PSK &p2psk=(_InitialPacket2_PSK&)packsend2;
		m_fish_send.EncryptPCBC(p2psk.sRand, sizeof(p2psk.sRand));
	};
}

void C_Connection::init_crypt_decodekey() //got their encrypted session key for me to use
{
	_InitialPacket2 &packrecv2=*((_InitialPacket2*)(m_recv_buffer+sizeof(_InitialPacket1)+m_iRecvRandomPadding1));

	unsigned char m_key[MAX_RSA_MODULUS_LEN];
	_Keyinfo &localkeyinfo=*((_Keyinfo*)m_key);
	unsigned int m_kl=0;
	int err=0;

	//deobfuscate
	m_fish_recv.DecryptPCBC(packrecv2.sPubCrypted, sizeof(packrecv2.sPubCrypted));

	if (g_use_networkhash && g_networkhash_PSK) {
		_InitialPacket2_PSK &p2psk=(_InitialPacket2_PSK&)packrecv2;
		m_fish_recv.DecryptPCBC(p2psk.sRand, sizeof(p2psk.sRand));
		g_networkhash_PSK_fish.DecryptECB(p2psk.sPubCrypted, sizeof(p2psk.sPubCrypted));
		g_networkhash_PSK_fish.DecryptECB(p2psk.sRand, sizeof(p2psk.sRand));
		for (int i=0;i<(sizeof(p2psk.sRand)/sizeof(int));i++) ((unsigned int*)(p2psk.sRand))[i]^=(~0);
		if (memcmp(p2psk.sRand, m_local_sRand, sizeof(localkeyinfo.sRand))) {
			log_printf(ds_Error,"connection: stealth mode fast MAC failed!!! DOS-Attack warning!");
			close(1);
			return;
		};
	};

	//decrypt with my private key to get sKeyB + randA (where sKeyB might be encrypted using network name)
	if (((
			err=RSAPrivateDecrypt(
				m_key,
				&m_kl,
				packrecv2.sPubCrypted,
				(g_key.bits + 7) / 8,
				&g_key,
				&g_random
			))!=0) ||
			(m_kl!=sizeof(_Keyinfo))
		)
	{
		//error decrypting
		log_printf(ds_Warning,"connection: error decrypting session key with my own private key (%d,%d)",err,m_kl);
		close(1);
		return;
	};

	//check to see the decrypted randA is the same as the randA we sent
	if (memcmp(localkeyinfo.sRand, m_local_sRand, sizeof(localkeyinfo.sRand))) {
		log_printf(ds_Error,"connection: error decrypting session key (signature is wrong, being hacked?)");
		close(1);
		return;
	};
	memset(m_local_sRand,0,sizeof(m_local_sRand));

	//decrypt to get sKeyB if necessary
	if (g_use_networkhash) {
		g_networkhash_PSK_fish.DecryptECB(localkeyinfo.sKey,sizeof(localkeyinfo.sKey));
		g_networkhash_PSK_fish.DecryptECB(localkeyinfo.sIV,sizeof(localkeyinfo.sIV));
	};

	//combine the first 56 bytes of sKeyA and sKeyB to get our session key
	//check to make sure it is nonzero.
	int x;
	int zeros=0;
	for (x = 0; x < sizeof(localkeyinfo.sKey); x++) {
		localkeyinfo.sKey[x]^=m_mykeyinfo.sKey[x];
		if (!localkeyinfo.sKey[x]) zeros++;
	};
	if (zeros == CONNECTION_KEYSIZE) {
		log_printf(ds_Error,"connection: zero session key, aborting.");
		close(1);
		return;
	};

	if (!memcmp(localkeyinfo.sIV,m_mykeyinfo.sIV,sizeof(localkeyinfo.sIV))) {
		log_printf(ds_Error,"connection: CBC IVs are equal, being hacked?");
		close(1);
		return;
	};

	m_fish.Init(localkeyinfo.sKey, sizeof(localkeyinfo.sKey));
	m_fish.SetIV(CBlowfish::IV_ENC,m_mykeyinfo.sIV);
	m_fish.SetIV(CBlowfish::IV_DEC,localkeyinfo.sIV);

	m_fish_send.Final();
	m_fish_recv.Final();
	memset(localkeyinfo.raw,0,sizeof(localkeyinfo.raw));
	memset(m_mykeyinfo.raw,0,sizeof(m_mykeyinfo.raw));

	m_ever_connected=1;
}

void C_Connection::do_init()
{
	int x;
	m_satmode=0;
	m_remote_maxsend=2048;
	for (x = 0 ; x < sizeof(bps_count)/sizeof(bps_count[0]); x ++) {
		bps_count[x].recv_bytes=0;
		bps_count[x].send_bytes=0;
		bps_count[x].time_ms=GetTickCount();
	};
	bps_count_pos=0;

	memset(m_recv_buffer,0,sizeof(m_recv_buffer));
	memset(m_send_buffer,0,sizeof(m_send_buffer));
	m_recv_len=m_recv_pos=0;
	m_send_len=m_send_pos=0;
	m_send_bytes_total=0;
	m_recv_bytes_total=0;
	m_start_time=GetTickCount();
	init_crypt();
}

C_Connection::~C_Connection()
{
	if (m_socket >= 0) {
		shutdown(m_socket, SHUT_RDWR );
		closesocket(m_socket);
	};
	m_fish.Final();
	m_fish_send.Final();
	m_fish_recv.Final();
	memset(m_mykeyinfo.raw,0,sizeof(m_mykeyinfo.raw));
}

void C_Connection::calc_bps(int *send, int *recv)
{
	unsigned int now=GetTickCount();
	unsigned int timediff=now-bps_count[bps_count_pos].time_ms;
	if (timediff < 1) timediff=1;

	#ifdef _WIN32
		m_last_sendbps=MulDiv(m_send_bytes_total-bps_count[bps_count_pos].send_bytes,1000,timediff);
		m_last_recvbps=MulDiv(m_recv_bytes_total-bps_count[bps_count_pos].recv_bytes,1000,timediff);
	#else
		// TODO: enough prec?
		m_last_sendbps=(((m_send_bytes_total-bps_count[bps_count_pos].send_bytes)*1000)/timediff);
		m_last_recvbps=(((m_recv_bytes_total-bps_count[bps_count_pos].recv_bytes)*1000)/timediff);
	#endif

	if (send) *send=m_last_sendbps;
	if (recv) *recv=m_last_recvbps;

	int l=bps_count_pos-1;
	if (l<0) l=sizeof(bps_count)/sizeof(bps_count[0])-1;

	if (now - bps_count[l].time_ms < 0 ||
		now - bps_count[l].time_ms >= 100)
	{
		bps_count[bps_count_pos].send_bytes=m_send_bytes_total;
		bps_count[bps_count_pos].recv_bytes=m_recv_bytes_total;
		bps_count[bps_count_pos].time_ms=now;
		bps_count_pos++;
		if (bps_count_pos>=sizeof(bps_count)/sizeof(bps_count[0])) bps_count_pos=0;
	};
}

C_Connection::state C_Connection::run(int max_send_bytes, int max_recv_bytes)
{
	int bytes_allowed_to_send=(max_send_bytes<0||max_send_bytes>MAX_CONNECTION_SENDSIZE)?MAX_CONNECTION_SENDSIZE:max_send_bytes;
	int bytes_allowed_to_recv=(max_recv_bytes<0||max_recv_bytes>MAX_CONNECTION_RECVSIZE)?MAX_CONNECTION_RECVSIZE:max_recv_bytes;

	#if 0
		#ifndef _DEBUG
			#error need this!!!
		#endif
	#else
		if (GetTickCount()-m_start_time>TIMEOUT_SYNC*1000 &&
			bf_initted.GetStage()<_bfInit::stage4_complete)
		{
			return m_state=STATE_ERROR;
		};
	#endif

	if (m_socket==-1 || m_state==STATE_ERROR) {
		return STATE_ERROR;
	};

	if (m_state == STATE_RESOLVING) {
		if (!m_host[0]) return (m_state=STATE_ERROR);
		if (m_saddr.sin_addr.s_addr == INADDR_NONE) {
			int a=m_dns?m_dns->resolve(m_host,(unsigned long int *)&m_saddr.sin_addr.s_addr):-1;
			switch (a)
			{
			case 0:
				{
					m_state=STATE_CONNECTING;
					break;
				};
			case 1:
				{
					return STATE_RESOLVING;
				};
			case -1:
				{
					log_printf(ds_Warning,"connection: error resolving hostname");
					return (m_state=STATE_ERROR);
				};
			};
		};
		if (!allowIP(this->get_remote())) {
			return (m_state=STATE_DIENOW);
		};
		int res=::connect(m_socket,(sockaddr *)&m_saddr,16);
		if (!res) {
			m_state=STATE_CONNECTED;
		}
		else if (ERRNO!=EINPROGRESS) {
			log_printf(ds_Error,"connection: connect() returned error: %d",ERRNO);
			return (m_state=STATE_ERROR);
		}
		else m_state=STATE_CONNECTING;
		return m_state;
	};
	if (m_state == STATE_CONNECTING) {
		fd_set f,f2;
		FD_ZERO(&f);
		FD_ZERO(&f2);
		FD_SET(m_socket,&f);
		FD_SET(m_socket,&f2);
		struct timeval tv;
		memset(&tv,0,sizeof(tv));
		if (select(0,NULL,&f,&f2,&tv)==-1) {
			log_printf(ds_Error,"connection::run: select() returned error: %d",ERRNO);
			return (m_state=STATE_ERROR);
		};
		if (FD_ISSET(m_socket,&f2)) {
			log_printf(ds_Error,"connection::run: select() notified of error");
			return (m_state=STATE_ERROR);
		};
		if (!FD_ISSET(m_socket,&f)) return STATE_CONNECTING;
		m_state=STATE_CONNECTED;
	};
	int active=0;
	if (m_state == STATE_CONNECTED || m_state == STATE_CLOSING) {
		if (m_send_len>0 && m_cansend) {
			int len=MAX_CONNECTION_SENDSIZE-m_send_pos;
			if (len > m_send_len) {
				len=m_send_len;
			};

			if (bf_initted.GetStage()<=_bfInit::stage1_hash) {
				int l=(int)sizeof(_InitialPacket1)+m_iSendRandomPadding1-m_send_pos;
				if (len > l) len=l;
				if (len < 0) len=0;
			}
			else if (bf_initted.GetStage()<=_bfInit::stage2_key) { //session key negotiation
				int l=(int)sizeof(_InitialPacket1)+m_iSendRandomPadding1+(int)sizeof(_InitialPacket2)+m_InitialPacket2Add+m_iSendRandomPadding2-m_send_pos;
				if (len > l) len=l;
				if (len < 0) len=0;
			};

			if (len > bytes_allowed_to_send) {
				len=bytes_allowed_to_send;
			};
			if (len > 0) {
				int res=::send(m_socket,m_send_buffer+m_send_pos,len,0);
				if (res==-1 && ERRNO != EWOULDBLOCK) {
					dbg_printf(ds_Debug,"connection::run: send(%d) returned error: %d",len,ERRNO);
					return STATE_CLOSED;
				};
				if (res>0) {
					bytes_allowed_to_send-=res;
					m_send_bytes_total+=res;
					m_send_pos+=res;
					m_send_len-=res;
					if (res == len) active=1;
					if (!bf_initted.Get(_bfInit::sentPKeyHash)) {
						if (m_send_pos >= (int)sizeof(_InitialPacket1)+m_iSendRandomPadding1) {
							bf_initted.Set(_bfInit::sentPKeyHash,true);
							dbg_printf(ds_Debug,"sent all of my pubkey");
						};
					}
					else if (!bf_initted.Get(_bfInit::sentSKey)) {
						if (m_send_pos >= ((int)sizeof(_InitialPacket1)+m_iSendRandomPadding1+(int)sizeof(_InitialPacket2)+m_InitialPacket2Add+m_iSendRandomPadding2)) {
							bf_initted.Set(_bfInit::sentSKey,true);
							dbg_printf(ds_Debug,"sent all of my encrypted session key");
						};
					};
					if (m_send_pos >= MAX_CONNECTION_SENDSIZE) m_send_pos=0;
				};
			};
		};

		int len=MAX_CONNECTION_RECVSIZE-m_recv_len;
		if (len > 0) {
			if (len > bytes_allowed_to_recv) len=bytes_allowed_to_recv;

			char rbuf[MAX_CONNECTION_RECVSIZE];

			int res=-1;
			if (len > 0) {
				res=::recv(m_socket,rbuf,len,0);
				if (res == 0) {
					dbg_printf(ds_Debug,"connection::run2: recv(%d) returned 0!",len);
					return (m_state=STATE_CLOSED);
				};
				if (res < 0 && ERRNO != EWOULDBLOCK) {
					dbg_printf(ds_Debug,"connection::run: recv() returned error %d!",ERRNO);
					return (m_state=STATE_CLOSED);
				};
			};
			if (res > 0) {
				if (res == len) active=1;

				if (m_recv_pos+res >= MAX_CONNECTION_RECVSIZE) { //wrappy
					int l=m_recv_pos+res-MAX_CONNECTION_RECVSIZE;
					memcpy(m_recv_buffer+m_recv_pos,rbuf,res-l);
					if (l) {
						memcpy(m_recv_buffer,rbuf+res-l,l);
					};
					m_recv_pos=l;
					dbg_printf(ds_Debug,"wrapped");
				}
				else {
					memcpy(m_recv_buffer+m_recv_pos,rbuf,res);
					m_recv_pos+=res;
				};
				bytes_allowed_to_recv-=res;
				m_recv_bytes_total+=res;
				m_recv_len+=res;

				if (m_state==STATE_CONNECTED &&
					!bf_initted.Get(_bfInit::gotPKeyHash))
				{
					if (m_recv_pos >= (int)sizeof(_InitialPacket1))
					{
						bf_initted.Set(_bfInit::gotPKeyHash,true);
						init_crypt_gotpkey();
						m_recv_len-=sizeof(_InitialPacket1);
					};
				};
				if (m_state==STATE_CONNECTED &&
					bf_initted.Get(_bfInit::gotPKeyHash) &&
					!bf_initted.Get(_bfInit::gotPad1))
				{
					if (m_recv_pos >= ((int)sizeof(_InitialPacket1)+m_iRecvRandomPadding1)) {
						bf_initted.Set(_bfInit::gotPad1,true);
						m_recv_len-=m_iRecvRandomPadding1;
					};
				};
				if (m_state==STATE_CONNECTED &&
					bf_initted.Get(_bfInit::gotPKeyHash) &&
					bf_initted.Get(_bfInit::gotPad1) &&
					!bf_initted.Get(_bfInit::gotSKey))
				{
					if (m_recv_pos >= ((int)sizeof(_InitialPacket1)+m_iRecvRandomPadding1+(int)sizeof(_InitialPacket2)+m_InitialPacket2Add+m_iRecvRandomPadding2)) { //get our key
						bf_initted.Set(_bfInit::gotSKey,true);
						init_crypt_decodekey();
						m_recv_len-=sizeof(_InitialPacket2);
						m_recv_len-=m_InitialPacket2Add;
						m_recv_len-=m_iRecvRandomPadding2;
					};
				};
			};
		};

		if (m_state==STATE_CONNECTED &&
			bf_initted.GetStage()==_bfInit::stage3_syn)
		{
			bf_initted.Set(_bfInit::sentSYN,true);
			int msp=m_send_pos, msl=m_send_len;
			int initlen=(int)sizeof(_InitialPacket1)+m_iSendRandomPadding1+(int)sizeof(_InitialPacket2)+m_InitialPacket2Add+m_iSendRandomPadding2;
			if (msp < initlen) { //only encrypt after first bytes
				msl -= (initlen-msp);
				msp = initlen;
			};

			if (msl>0) {
				dbg_printf(ds_Debug,"encrypting %d bytes at %d",msl,msp);
				encrypt_buffer(m_send_buffer+msp,msl);
				PushRandomCrap();
			};
		};

		if (m_state == STATE_CLOSING) {
			if (m_send_len < 1) {
				return (m_state = STATE_CLOSED);
			};
			return STATE_CLOSING;
		};
		return active?STATE_CONNECTED:STATE_IDLE;
	};
	return m_state;
}

void C_Connection::PushRandomCrap()
{
	unsigned char buf[4096];
	#if 0
		#ifndef _DEBUG
			#error remove 1
		#endif
		memset(buf,0xff,m_iSendRandomPadding3&(~7));
		dbg_printf(ds_Debug,"Push crap len=0x%x recvpos=0x%x recvlen=0x%x sendpos=0x%x sendlen=0x%x bfinit=0x%x",m_iSendRandomPadding3&(~7),m_recv_pos,m_recv_len,m_send_pos,m_send_len,bf_initted.GetI());
	#else
		R_GenerateBytes(buf,m_iSendRandomPadding3&(~7),&g_random);
	#endif
	send_bytes(buf,m_iSendRandomPadding3&(~7));
}

bool C_Connection::PopRandomCrap()
{
	unsigned char buf[4096];
	if (recv_bytes_available() < (m_iRecvRandomPadding3&(~7))) {
		return false;
	};
	recv_bytes(buf,m_iRecvRandomPadding3&(~7));
	#if 0
		#ifndef _DEBUG
			#error remove 1
		#endif
		int i=0,l=m_iRecvRandomPadding3&(~7);
		while (i<l) {
			if (buf[i]!=0xff) {
				dbg_printf(ds_Debug,"Bad crap at 0x%x recvpos=0x%x recvlen=0x%x sendpos=0x%x sendlen=0x%x bfinit=0x%x",i,m_recv_pos,m_recv_len,m_send_pos,m_send_len,bf_initted.GetI());
				return false;
			};
			i++;
		};
		dbg_printf(ds_Debug,"Good crap len 0x%x recvpos=0x%x recvlen=0x%x sendpos=0x%x sendlen=0x%x bfinit=0x%x",i,m_recv_pos,m_recv_len,m_send_pos,m_send_len,bf_initted.GetI());
	#endif
	return true;
}

void C_Connection::close(int quick)
{
	if (m_state == STATE_RESOLVING || m_state == STATE_CONNECTING ||
		(m_state != STATE_ERROR && quick))
		m_state=STATE_CLOSED;
	else if (m_state != STATE_CLOSED && m_state != STATE_ERROR)
		m_state=STATE_CLOSING;
}

int C_Connection::send_bytes_in_queue()
{
	return m_send_len;
}

int C_Connection::getMaxSendSize()
{
	int a=MAX_CONNECTION_SENDSIZE;
	if (g_conspeed<64)a=512; //modems only let sendahead be 512 or so.
	else if (g_conspeed<384)a=2048;
	else if (g_conspeed<1600)a=4096;
	else if (g_conspeed<20000)a=8192;
	if (a > m_remote_maxsend) a=m_remote_maxsend;
	//dbg_printf(ds_Debug,"max_size (%d,%d)",a,m_remote_maxsend);
	return a;
}

int C_Connection::send_bytes_available()
{
	extern int g_conspeed;
	int a=getMaxSendSize();

	a-=m_send_len;
	if (a<0)a=0;
	a &= ~7;
	return a;
}

int C_Connection::send_bytes(void *data, int length)
{
	char tmp[MAX_CONNECTION_SENDSIZE];
	if (!length) return 0;
	memcpy(tmp,data,length);
	if (bf_initted.GetStage()==_bfInit::stage4_complete) encrypt_buffer(tmp,length);

	int write_pos=m_send_pos+m_send_len;
	if (write_pos >= MAX_CONNECTION_SENDSIZE) {
		write_pos-=MAX_CONNECTION_SENDSIZE;
	};

	int len=MAX_CONNECTION_SENDSIZE-write_pos;
	if (len > length) {
		len=length;
	};

	memcpy(m_send_buffer+write_pos,tmp,len);

	if (length > len) {
		memcpy(m_send_buffer,tmp+len,length-len);
	};
	m_send_len+=length;

	return 0;
}

void C_Connection::decrypt_buffer(void *data, int len)
{
	if (len & 7) {
		log_printf(ds_Critical,"connection::decrypt_buffer(): len=%d (&7)",len);
	};
	m_fish.DecryptPCBC(data,len);
}

void C_Connection::encrypt_buffer(void *data, int len)
{
	if (len & 7) {
		log_printf(ds_Critical,"connection::encrypt_buffer(): len=%d (&7)",len);
	};
	m_fish.EncryptPCBC(data,len);
}

int C_Connection::recv_bytes_available()
{
	if (bf_initted.GetStage()<_bfInit::stage4_complete) return 0;
	return m_recv_len&(~7);
}

int C_Connection::recv_bytes(void *data, int maxlength)
{
	if (bf_initted.GetStage()<_bfInit::stage4_complete) return -1;
	if (maxlength > m_recv_len) {
		maxlength=m_recv_len;
	};
	int read_pos=m_recv_pos-m_recv_len;
	if (read_pos < 0) {
		read_pos += MAX_CONNECTION_RECVSIZE;
	};
	int len=MAX_CONNECTION_RECVSIZE-read_pos;
	if (len > maxlength) {
		len=maxlength;
	};
	memcpy(data,m_recv_buffer+read_pos,len);
	if (len < maxlength) {
		memcpy((char*)data+len,m_recv_buffer,maxlength-len);
	};

	decrypt_buffer(data,maxlength);

	m_recv_len-=maxlength;
	return maxlength;
}

unsigned long C_Connection::get_interface()
{
	if (m_socket==-1) return 0;
	sockaddr_in sin;
	memset(&sin,0,sizeof(sin));
	socklen_t len=16;
	if (::getsockname(m_socket,(sockaddr *)&sin,&len)) return 0;
	return (unsigned long) sin.sin_addr.s_addr;
}
