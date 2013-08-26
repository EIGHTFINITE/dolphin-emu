// Copyright (C) 2003 Dolphin Project.

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, version 2.0.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License 2.0 for more details.

// A copy of the GPL 2.0 should have been included with the program.
// If not, see http://www.gnu.org/licenses/

// Official SVN repository and contact information can be found at
// http://code.google.com/p/dolphin-emu/

#ifndef _WII_IPC_HLE_DEVICE_NET_SSL_H_
#define _WII_IPC_HLE_DEVICE_NET_SSL_H_

#include "WII_IPC_HLE_Device.h"

#include <polarssl/net.h>
#include <polarssl/ssl.h>
#include <polarssl/havege.h>

#define NET_SSL_MAX_HOSTNAME_LEN 256
#define NET_SSL_MAXINSTANCES 4

#define SSLID_VALID(x) (x >= 0 && x < NET_SSL_MAXINSTANCES && CWII_IPC_HLE_Device_net_ssl::_SSL[x].active)

enum ssl_err_t
{
	SSL_OK 					=   0,
	SSL_ERR_FAILED			=  -1,
	SSL_ERR_RAGAIN			=  -2,
	SSL_ERR_WAGAIN			=  -3,
	SSL_ERR_SYSCALL			=  -5,
	SSL_ERR_ZERO			=  -6,  // read or write returned 0
	SSL_ERR_CAGAIN			=  -7,  // BIO not connected
	SSL_ERR_ID				=  -8,  // invalid SSL id
	SSL_ERR_VCOMMONNAME		=  -9,  // verify failed: common name
	SSL_ERR_VROOTCA			=  -10, // verify failed: root ca
	SSL_ERR_VCHAIN			=  -11, // verify failed: certificate chain
	SSL_ERR_VDATE			=  -12, // verify failed: date invalid
	SSL_ERR_SERVER_CERT		=  -13, // certificate cert invalid
};

enum SSL_IOCTL
{
	IOCTLV_NET_SSL_NEW							= 0x01,
	IOCTLV_NET_SSL_CONNECT						= 0x02,
	IOCTLV_NET_SSL_DOHANDSHAKE					= 0x03,
	IOCTLV_NET_SSL_READ							= 0x04,
	IOCTLV_NET_SSL_WRITE						= 0x05,
	IOCTLV_NET_SSL_SHUTDOWN						= 0x06,
	IOCTLV_NET_SSL_SETCLIENTCERT				= 0x07,
	IOCTLV_NET_SSL_SETCLIENTCERTDEFAULT			= 0x08,
	IOCTLV_NET_SSL_REMOVECLIENTCERT				= 0x09,
	IOCTLV_NET_SSL_SETROOTCA					= 0x0A,
	IOCTLV_NET_SSL_SETROOTCADEFAULT				= 0x0B,
	IOCTLV_NET_SSL_DOHANDSHAKEEX				= 0x0C,
	IOCTLV_NET_SSL_SETBUILTINROOTCA				= 0x0D,
	IOCTLV_NET_SSL_SETBUILTINCLIENTCERT			= 0x0E,
	IOCTLV_NET_SSL_DISABLEVERIFYOPTIONFORDEBUG	= 0x0F,
	IOCTLV_NET_SSL_DEBUGGETVERSION				= 0x14,
	IOCTLV_NET_SSL_DEBUGGETTIME					= 0x15,
};

typedef struct {
	ssl_context ctx;
	ssl_session session;
	havege_state hs;
	x509_cert cacert;
	x509_cert clicert;
	rsa_context rsa;
	int sockfd;
	char hostname[NET_SSL_MAX_HOSTNAME_LEN];
	bool active;
} WII_SSL;

class CWII_IPC_HLE_Device_net_ssl : public IWII_IPC_HLE_Device, public CWII_IPC_HLE_Device_Singleton<CWII_IPC_HLE_Device_net_ssl>
{
public:

	CWII_IPC_HLE_Device_net_ssl(const std::string& _rDeviceName);

	virtual ~CWII_IPC_HLE_Device_net_ssl();

	virtual bool IOCtl(u32 _CommandAddress);
	virtual bool IOCtlV(u32 _CommandAddress);
	int getSSLFreeID();

	static WII_SSL _SSL[NET_SSL_MAXINSTANCES];

	static const char* GetBaseName() { return "/dev/net/ssl"; }
private:
	u32 ExecuteCommand(u32 _Parameter, u32 _BufferIn, u32 _BufferInSize, u32 _BufferOut, u32 _BufferOutSize);
	u32 ExecuteCommandV(u32 _Parameter, SIOCtlVBuffer CommandBuffer);
};

#endif
