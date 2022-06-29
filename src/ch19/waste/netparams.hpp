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

#ifndef	_NETPARAMS_H_
#define	_NETPARAMS_H_

//turn this off for more privacy
#define SAVE_CB_HOSTS							1
#define MAX_CBHOSTS								32

#define	HARD_NEW_CONNECTION_LIMIT				64
#define	TIMEOUT_SYNC							20
#define	DEFAULT_TTL								6
#define	DEFAULT_CONSPEED						128
#define	MIN_ROUTE_SPEED							64
#define	PING_DELAY								30
#define	KEY_BROADCAST_DELAY						60*10
#define	CHANNEL_STARVE_DELAY					60*4
#define	NICK_STARVE_DELAY						60*4
#define	DEFAULT_DB_REFRESH_DELAY				300
#define	SCANID_REFRESH_DELAY					120
#define	SEACH_EXPIRE_DIRECTORY					600
#define	TRANSFER_SEND_TIMEOUT					300
#define	WASTESTATE_FLUSH_DELAY					60
#define	RECEIVE_TIMEOUT_DELAY					30

#define	CPS_WINDOWSIZE							6
// CPS_WINDOWLEN unit=ms
#define	CPS_WINDOWLEN							500
//milliseconds
#define NEXTITEM_RECEIVE_DELAY					2000
//milliseconds
#define NEXTITEM_UPLOAD_DELAY					1000
//milliseconds
#define NEXTITEM_NEW_OUT_CONNECTION_DELAY		3000
//milliseconds
#define CHAT_NICK_STARVE_DELAY					30000
//milliseconds
#define CHAT_NICK_UNBOLD_DELAY					15000

#define MAX_CHATTEXT_SIZE 						30000


#endif//_NETPARAMS_H_
