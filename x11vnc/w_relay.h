
#ifndef W_RELAY_H
#define W_RELAY_H

#include <stdio.h>

#include "rfb/rfb.h"


MUTEX(cmdMutex);


bool start_relay(rfbClientPtr cl);

#define	WORD	unsigned short
#define	DWORD	unsigned int
#define	BYTE	unsigned char
#define PVOID	unsigned char *
#define PBYTE	unsigned char *
#define UINT	int

#define	VERSION		0x0101

#pragma pack(1)

enum tagAGENT_COMMAND{

	CMD_WELCOME = 0x01,
	CMD_SERVERINFO, CMD_PATCHINFO, CMD_LOGIN, CMD_SESSIONINFO, CMD_SESSION,
	CMD_DATA, CMD_MESSENGER, CMD_SESSIONDELAY, CMD_DESTROY, CMD_MODULECHECK,
	CMD_MODULEEXECUTE,

	CMD_RELAYINFO = 0x80,

	ACK_WELCOME = 0x81,
	ACK_SERVERINFO, ACK_PATCHINFO, ACK_LOGIN, ACK_SESSIONINFO, ACK_SESSION,
	ACK_DATA, ACK_MESSENGER, ACK_SESSIONDELAY, ACK_DESTROY, ACK_MODULECHECK,
	ACK_MODULEEXECUTE,

	CMD_REGRELAY = 0xF1, CMD_CHKRELAY,
	CMD_OPENRELAY = 0xF4,
	CMD_PING = 0XF7,
	CMD_CONNECTINFO = 243,
	CMD_CHECKCOMPLETE = 246,
	CMD_CREATEMODULEDATA = 0xF6,
	CMD_CALLHWCONTROL = 0x12,
	ACK_CALLHWCONTROL = 0x92,
	CMD_CHANGEPIXEL = 0xc0,
	CMD_CALLPCONTROL = 0x11,
	ACK_CALLPCONTROL = 0x91,

	CMD_HWINFO = 0x01,

	PROC_CHATMSG_TO_HOST = 0x08,
	PROC_CHATMSG_TO_VIEWER = 0x09,

};

#pragma pack(pop)


typedef struct tagKPACKET{
	WORD  Version;		// (00) Version
	DWORD ECC;			// (08) CRC32
	WORD  wSize;		// (12) Head
	BYTE  Command;		// (14)
	BYTE  Type;			// (15)
} KPACKET;

struct _GUID {
	unsigned	int		Data1;
	unsigned	short	Data2;
	unsigned	short	Data3;
	unsigned	char	Data4[8];

} GD;

typedef struct tagCONNECTINFO {
	char szIPAddress[80];
	int nPort;
} CONNECTINFO, *PCONNECTINFO;


typedef struct tagKPACKETDATA{
	int lpData;
} KPACKETDATA, *PKPACKETDATA;


typedef struct tagCREATEMODULEDATA {	
	struct _GUID SystemID;
	int deviceType;
	int width;
	int height;
	
	
}CREATEMODULEDATA, *PCREATEMODULEDATA;

typedef struct tagRELAYINFO{

	struct _GUID SystemID;
} RELAYINFO, *PRELAYINFO;


typedef struct tagChangePixel{
	int percent;
} CHANGEPIXEL;

typedef struct tagNETPROTO{
	DWORD Command;
	DWORD CRC32;
	DWORD Size;
	DWORD Type;
} NETPROTO;


typedef struct tagDeviceInfo{
	char model[50];
	char androidId[8];
	char phooneNumber[50];
	int appVersionCode;
	char appVersionName[8];
	char isRooting;
	char networkOperatorName[30];
	char androidVersion[20];
	char kernelVersion[30];
	int batteryLevel;
	int batteryVoltage;
	int batteryPlugType;
	char batteryTechnology[10];
	int networkType;
	char netowrkIpAddress[38];
	char networkWifiLinkName[20];
	int phoneSignalStrength;
	int networkSignalStrength;
	int64_t internalStorageTotalSize;
	int64_t internalStorageFreeSize;
	int64_t externalStorageTotalSize;
	int64_t externalStorageFreeSize;
	int64_t memoryTotalSize;
	int64_t memoryFreeSize;
	char cpu[20];
	char isAirPlaneMode;
	int displayScreenWidth;
	int displayScreenHeight;
	char defaultLanguage[2];
	char defaultCountry[2];
	char isBackgroundDataSync;
	int cpuUsage;
} DEVICEINFO;

#endif
