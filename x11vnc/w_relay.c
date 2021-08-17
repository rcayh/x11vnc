
#include <stdio.h>
#include <string.h>
#include <netdb.h>
#include <netinet/in.h>

#include <sys/sysinfo.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <sys/statvfs.h>

#ifdef HAVE_LIB_NOTIFY
#include <libnotify/notify.h>
#endif

#include <gtk/gtk.h>

#include "w_common.h"
#include "w_relay.h"
#include "w_http.h"
#include "w_crcset.h"
#include "w_utils.h"
#include "w_ui.h"

//new
//#define WRITE_BUFFER_DEBUG
//#define READ_BUFFER_DEBUG
//#define GUID_DEBUG
#define MAX_BUFFER_SIZE 1024

int change_pixel_size;

#ifdef HAVE_LIB_NOTIFY
NotifyNotification* notify = NULL;
#endif

pthread_t cmdThread;

int cmdSock, remoteSock, deviceSock, pSock;

BYTE cmdWriteBuffer[MAX_BUFFER_SIZE], remoteWriteBuffer[MAX_BUFFER_SIZE], deviceWriteBuffer[MAX_BUFFER_SIZE], pWriteBuffer[MAX_BUFFER_SIZE];
BYTE cmdReadBuffer[MAX_BUFFER_SIZE], remoteReadBuffer[MAX_BUFFER_SIZE], deviceReadBuffer[MAX_BUFFER_SIZE], pReadBuffer[MAX_BUFFER_SIZE] ;



void setStatusNotify(StatusNotify sn){

	statusNotify = sn;
	char *message;

	switch(sn){

		case STATUS_NOTIFY_BEGIN:
			message = "... Connecting Remote Control ...";
			break;
		case STATUS_NOTIFY_REQ_ACCEPTNUM:
			message = "... step1. request valid start remote control ...";
			break;
		case STATUS_NOTIFY_CONNECT_CMD_SOCK:
			message = "... step2. connect command socket ...";
			break;
		case STATUS_NOTIFY_RESUME_REPLAY_SOCK:
			message = "... step3. start relay socket ...";
			break;
		case STATUS_NOTIFY_CONNECTED:
			message = "[Success] Remote Control Connected.";
			break;
		default:
			wLog("Unknown Status Notify >>>>>>>>> \n");
			break;
	}

#ifdef HAVE_LIB_NOTIFY
	bool r;
	if(notify == NULL){

		notify_init("WizHelper");

		notify = notify_notification_new(
					"WizHelper",
					message,
					0);

		//notify_notification_set_timeout(notify, 10000);
		r = notify_notification_show(notify, 0);
	}
	else{

		r = notify_notification_update(
					notify,
					"WizHelper",
					message,
					0);
		if(r)
			r = notify_notification_show(notify, 0);
	}
#else
	ui_set_status_text(message);

	if(sn == STATUS_NOTIFY_CONNECTED){
		ui_connected();
	}
#endif
}


DWORD MakeCRC(PVOID pData, UINT nSize, DWORD dwCRC){
	if( dwCRC != -1 )
		dwCRC = ~dwCRC;
	ATL_MAKECRC32( (PBYTE)pData, nSize, dwCRC );
	return dwCRC;
}

void destroy(int status){

	wLog("################################################################\n");
	wLog("# DESTORY status[%d]\n", status);
	wLog("################################################################\n");
	//exit(status);
	exit(1);
}

void printHex(BYTE *buffer, size_t length){
	int i;
	for(i = 0; i < length; i++){
		wLog("[%d] %02x\n", i, ((BYTE *)buffer)[i]);
	}
}

int create_socket(const char *host, const int port){

	int sock;
	struct sockaddr_in server_addr;

	if((sock = socket(PF_INET, SOCK_STREAM, 0)) < 0){
		perror("can't create socket\n");
		destroy(-1);
	}

	bzero((char *)&server_addr, sizeof(server_addr));

	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = inet_addr(host);
	server_addr.sin_port = htons(port);

	char *hostname;
	struct hostent *hp;

	if(server_addr.sin_addr.s_addr != -1){
	   hostname = host;
	}
	else{
	   hp = gethostbyname(host);
	   if(!hp){
		   wLog("unknown host %s\n", host);
	   }
	   else{
		  server_addr.sin_family = hp->h_addrtype;
	      memcpy(&(server_addr.sin_addr.s_addr), hp->h_addr, hp->h_length);
	      hostname = hp->h_name;

	      /*
	      char *ipbuf = inet_ntoa(*((struct in_addr*)hp->h_addr_list[0]));
	      wLog("\t success. gethostbyname host:%s -> ip:%s\n", hostname, ipbuf);
	      /*
	      int i;
	      for(i=0; i<hp->h_length; i++){
	    	  char *ipbuf = inet_ntoa(*((struct in_addr*)hp->h_addr_list[i]));
			  wLog("\t success. gethostbyname host:%s -> ip:%s\n", hostname, ipbuf);
	      }
	      */

	   }
	}

	if(connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0){
		wLog("can't connect.\n");
		destroy(-1);
	}

	return sock;
}

void setSocketReadTimeout(int sock, int t){

	struct timeval timeout;
	timeout.tv_sec = t;
	timeout.tv_usec = 0;
	setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
	//SO_SNDTIMEO
}

bool send_fully(int sock, void *buffer, size_t length){

    while (length > 0){

    	int i;
        i = write(sock, buffer, length);
        if (i < 1) return false;
        buffer += i;
        length -= i;
    }
    return true;
}

bool read_fully(int sock, BYTE *buffer, size_t length){

	size_t read = recv(sock, buffer, length, MSG_WAITALL);
	if(read <= 0){
		perror("error read_fully :  socket");
		return false;
	}
    return true;
}

bool read_normal(int sock, char *buffer, size_t length){

	size_t r = read(sock, buffer, length);
	wLog("read size =============== %d\n", r);
	return true;
}


void printGuid(struct _GUID *gd){

	wLog(
			"Debug >> GUID >> %08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X\n",
			gd->Data1, gd->Data2, gd->Data3,
			gd->Data4[0],
			gd->Data4[1],
			gd->Data4[2],
			gd->Data4[3],
			gd->Data4[4],
			gd->Data4[5],
			gd->Data4[6],
			gd->Data4[7]);
}


void generate_guid(struct _GUID *gd, char *aa){

	sscanf(aa, "%08x", &gd->Data1);
	aa += 9;
	sscanf(aa, "%04x", &gd->Data2);
	aa += 5;
	sscanf(aa, "%04x", &gd->Data3);
	aa += 5;

	int i;
	for(i = 0; i < 8; i++ ){
		sscanf(aa,"%02x", &gd->Data4[i]);
		aa += 2;
		if(*aa == '-') aa++;
	}
}

KPACKET* getKPacket(BYTE * readBuffer){
	KPACKET *p;
	p = (struct tagKPACKET *)readBuffer;
	return p;
}

NETPROTO* getNetProto(BYTE * readBuffer){
	NETPROTO *p;
	p = (struct tagNETPROTO *)readBuffer;
	return p;
}

bool send_kpacket(int sock, BYTE *writeBuffer, BYTE cmd, void* data, size_t dataSize){

	KPACKET *p;
	int writeCount;

	p = (struct tagKPACKET *)writeBuffer;
	p->Version = VERSION;
	p->Command = cmd;
	p->wSize = dataSize;
	p->Type = 0;
	p->ECC = 0;

	if(data != NULL){
		memcpy(writeBuffer+sizeof(struct tagKPACKET), data, dataSize);
	}

	writeCount = sizeof(struct tagKPACKET)+dataSize;

	p->ECC = MakeCRC(writeBuffer, writeCount, 0);

	if(cmd != CMD_PING)
		wLog("[send_kpacket] command=%02x size=%d\n", p->Command, writeCount);

#ifdef WRITE_BUFFER_DEBUG
	if(cmd != CMD_PING)
		printHex(writeBuffer, writeCount);
#endif

	return send_fully(sock, writeBuffer, writeCount);
}
bool send_netproto(int sock, BYTE *writeBuffer, BYTE cmd, void* data, size_t dataSize){

	NETPROTO *p;
	int writeCount;

	p = (struct tagNETPROTO *)writeBuffer;
	p->Command = cmd;
	p->Size = dataSize;
	p->Type = 0;
	p->CRC32 = 0;

	if(data != NULL){
		memcpy(writeBuffer+sizeof(struct tagNETPROTO), data, dataSize);
	}

	writeCount = sizeof(struct tagNETPROTO)+dataSize;

	wLog( "[send_netproto] command=%02x size=%d\n", p->Command, writeCount);

#ifdef WRITE_BUFFER_DEBUG
	printHex(writeBuffer, writeCount);
#endif

	return send_fully(sock, writeBuffer, writeCount);
}

bool read_data(int sock, BYTE *readBuffer, size_t length){
	return read_fully(sock, readBuffer, length);
}


char* iso88959_to_utf8(const char *str)
{
	char *utf8 = malloc(1 + (2 * strlen(str)));

	if (utf8) {
		char *c = utf8;
		for (; *str; ++str) {
			if (*str & 0x80) {
				*c++ = *str;
			} else {
				*c++ = (char) (0xc0 | (unsigned) *str >> 6);
				*c++ = (char) (0x80 | (*str & 0x3f));
			}
		}
		*c++ = '\0';
	}
	return utf8;
}

#include <wchar.h>

#include <sys/utsname.h>
#include <unistd.h>

void sendDeviceInfo(){

	wLog("################################################################\n");
	wLog("Send DeviceInfo\n");
	wLog("################################################################\n");


	struct utsname *un;
	struct sysinfo sys;

	un = malloc(sizeof(struct utsname));
	uname(un);
	sysinfo(&sys);

	char hostname[100];
	char os[110];
	uint64_t memtotal;
	uint64_t memfree;

	gethostname(hostname, 100);

	char tos[100];
	getCommandOutput("lsb_release -ds", tos, sizeof(tos));
	sprintf(os, "Linux - %s", tos);

	memtotal = sys.totalram;
	memfree = sys.freeram;

	int count;

	count = 0;
	char tcpuinfo[255];
	getCommandOutput("cat /proc/cpuinfo | grep model | grep name | head -1", tcpuinfo, sizeof(tcpuinfo));
	char *cpuinfo;
	cpuinfo = strtok(tcpuinfo, ":");
	while(cpuinfo != NULL){
		if(count == 1){
			break;
		}
		cpuinfo = strtok(NULL, ":");
		count++;
	}


	count = 0;
	char tlaninfo[255];
	getCommandOutput("lspci | grep -i 'ethernet'", tlaninfo, sizeof(tlaninfo));
	char *laninfo;
	laninfo = strtok(tlaninfo, ":");
	while(laninfo != NULL){
		if(count == 2){
			break;
		}
		laninfo = strtok(NULL, ":");
		count++;
	}


	int fd;
	struct ifreq ifr;
	fd = socket(AF_INET, SOCK_DGRAM, 0);
	ifr.ifr_addr.sa_family = AF_INET;
	strncpy(ifr.ifr_name, "eth0", IFNAMSIZ-1);
	ioctl(fd, SIOCGIFADDR, &ifr);
	close(fd);

	uint64_t disktotal = 0;
	uint64_t diskfree = 0;
	struct statvfs stat;
	if(statvfs("/", &stat) != 0) {

	}
	else{
		disktotal = stat.f_bsize * stat.f_blocks;
		diskfree = stat.f_bsize * stat.f_bavail;
	}


	DEVICEINFO info;
	snprintf(info.model, 50, un->sysname);
	snprintf(info.androidId, 8, "12345678");
	snprintf(info.phooneNumber, 50, "0000");
	info.appVersionCode = 118;
	snprintf(info.appVersionName, 8, un->nodename);
	info.isRooting = 0x0;
	snprintf(info.networkOperatorName, 30, un->machine);
	snprintf(info.androidVersion, 20, un->version);
	snprintf(info.kernelVersion, 30, un->release);
	info.batteryLevel = 100;
	info.batteryVoltage = 5;
	info.batteryPlugType = 0;
	snprintf(info.batteryTechnology, 10, "LI-ON");
	info.networkType = -1;
	snprintf(info.netowrkIpAddress, 38, inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr));
	snprintf(info.networkWifiLinkName, 20, "none");
	info.phoneSignalStrength = 0;
	info.networkSignalStrength = 0;
	info.internalStorageTotalSize = disktotal;
	info.internalStorageFreeSize = diskfree;
	info.externalStorageTotalSize = 1*1024*1024;
	info.externalStorageFreeSize = 1*1024*1024;
	info.memoryTotalSize = memtotal;
	info.memoryFreeSize = memfree;
	snprintf(info.cpu, 20, cpuinfo);
	info.isAirPlaneMode = 0x0;
	info.displayScreenWidth = gdk_screen_width();
	info.displayScreenHeight = gdk_screen_height();



	memcpy(info.defaultLanguage, "KR", 2);
	memcpy(info.defaultCountry, "KR", 2);
	info.isBackgroundDataSync = 0x0;

	free(un);
	send_netproto(deviceSock, deviceWriteBuffer, CMD_HWINFO, &info, sizeof(struct tagDeviceInfo));
}

void* startHWControl(void* ptr){

	ControlWaitInfoPtr waitInfo;
	waitInfo = (ControlWaitInfoPtr)ptr;


	wLog("thread argument guid = [%s]\n", waitInfo->controlWaitSystemId);

	wLog("################################################################\n");
	wLog("# Connect Device Info host=%s:%d\n", waitInfo->relayHost, waitInfo->relayPort);
	wLog("################################################################\n");

	int sock;
	bool success;
	sock = create_socket(waitInfo->relayHost, waitInfo->relayPort);
	deviceSock = sock;

	wLog("[deviceinfo-sock] crete socket = %d\n", sock);

	setSocketReadTimeout(sock, 5);

	struct _GUID *cguid;
	cguid = malloc(sizeof(struct _GUID));

	generate_guid(cguid, waitInfo->controlWaitSystemId);

#ifdef READ_BUFFER_DEBUG
	printGuid(cguid);
#endif


	//OPENRELAY
	success = send_kpacket(sock, deviceWriteBuffer, CMD_REGRELAY, cguid, sizeof(struct _GUID));
	free(cguid);

	if(!success){
		wLog("[deviceinfo-sock] failed. REGRELAY send data.\n");
		destroy(-1);
	}

	//setSocketReadTimeout(sock, 5);

	sleep(1);

	size_t recvRegRelaySize = sizeof(struct tagKPACKET);
	success = read_data(sock, deviceReadBuffer, recvRegRelaySize);
	if(!success){
		wLog("[deviceinfo-sock] failed. REGRELAY recv data.\n");
		destroy(-1);
	}

	KPACKET *kp = getKPacket(deviceReadBuffer);
	wLog("[deviceinfo-sock] REGRELAY recv data >> command=%02x size=%d\n", kp->Command, kp->wSize);


	success = read_data(sock, deviceReadBuffer+sizeof(struct tagKPACKET), kp->wSize);
	if(!success){
		wLog("[deviceinfo-sock] failed. REGRELAY recv data.\n");
		destroy(-1);
	}

#ifdef READ_BUFFER_DEBUG
	printHex(deviceReadBuffer+sizeof(struct tagKPACKET), kp->wSize);
#endif

	setSocketReadTimeout(sock, 0);

	NETPROTO *header;

	while(1){

		success = read_data(sock, deviceReadBuffer, sizeof(struct tagNETPROTO));
		if(!success){
			wLog("[deviceinfo-sock] failed. HEADER recv data.\n");
			destroy(-1);
		}

		header = getNetProto(deviceReadBuffer);
		wLog("[deviceinfo-sock] HEADER recv data >> command=%02x size=%d\n", header->Command, header->Size);

		if(header->Size > 0){

			success = read_data(sock, deviceReadBuffer+sizeof(struct tagNETPROTO), header->Size);
			if(!success){
				wLog("[deviceinfo-sock] failed. DATA recv data.\n");
				destroy(-1);
			}
		}

		if(header->Command == CMD_HWINFO){
			sendDeviceInfo();
		}
	}
}

void receiveChatMessage(NETPROTO *header){


	wLog("chat msg ======================================================\n");

	char *message = calloc(header->Size, sizeof(char));

	memcpy(message, pReadBuffer+sizeof(struct tagNETPROTO), header->Size);
	wLog("msg:%s\n", message);

	ui_chat_receive_message(message);


}

void* startPControl(void* ptr){

	ControlWaitInfoPtr waitInfo;
	waitInfo = (ControlWaitInfoPtr)ptr;


	wLog("thread argument guid = [%s]\n", waitInfo->controlWaitSystemId);

	wLog("################################################################\n");
	wLog("# Connect P Control host=%s:%d\n", waitInfo->relayHost, waitInfo->relayPort);
	wLog("################################################################\n");

	int sock;
	bool success;
	sock = create_socket(waitInfo->relayHost, waitInfo->relayPort);
	pSock = sock;

	wLog("[p-sock] crete socket = %d\n", sock);

	setSocketReadTimeout(sock, 5);

	struct _GUID *cguid;
	cguid = malloc(sizeof(struct _GUID));

	generate_guid(cguid, waitInfo->controlWaitSystemId);

#ifdef READ_BUFFER_DEBUG
	printGuid(cguid);
#endif


	//OPENRELAY
	success = send_kpacket(sock, pWriteBuffer, CMD_REGRELAY, cguid, sizeof(struct _GUID));
	free(cguid);

	if(!success){
		wLog("[p-sock] failed. REGRELAY send data.\n");
		destroy(-1);
	}

	//setSocketReadTimeout(sock, 5);

	sleep(1);

	size_t recvRegRelaySize = sizeof(struct tagKPACKET);
	success = read_data(sock, pReadBuffer, recvRegRelaySize);
	if(!success){
		wLog("[p-sock] failed. REGRELAY recv data.\n");
		destroy(-1);
	}

	KPACKET *kp = getKPacket(pReadBuffer);
	wLog("[p-sock] REGRELAY recv data >> command=%02x size=%d\n", kp->Command, kp->wSize);


	success = read_data(sock, pReadBuffer+sizeof(struct tagKPACKET), kp->wSize);
	if(!success){
		wLog("[p-sock] failed. REGRELAY recv data.\n");
		destroy(-1);
	}

#ifdef READ_BUFFER_DEBUG
	printHex(pReadBuffer+sizeof(struct tagKPACKET), kp->wSize);
#endif

	setSocketReadTimeout(sock, 0);

	NETPROTO *header;

	while(1){

		success = read_data(sock, pReadBuffer, sizeof(struct tagNETPROTO));
		if(!success){
			wLog("[p-sock] failed. HEADER recv data.\n");
			destroy(-1);
		}

		header = getNetProto(pReadBuffer);
		wLog("[p-sock] HEADER recv data >> command=%02x size=%d\n", header->Command, header->Size);

		if(header->Size > 0){

			success = read_data(sock, pReadBuffer+sizeof(struct tagNETPROTO), header->Size);
			if(!success){
				wLog("[p-sock] failed. DATA recv data.\n");
				destroy(-1);
			}
		}

		if(header->Command == PROC_CHATMSG_TO_HOST){
			receiveChatMessage(header);
		}
	}
}



void beginHWControl(ControlWaitInfoPtr waitInfo){

	wLog("begin hw control.\n");

	pthread_t thread;
	pthread_create(&thread, NULL, startHWControl, waitInfo);
	pthread_detach(thread);


	int result = 0;
	send_kpacket(cmdSock, cmdWriteBuffer, ACK_CALLHWCONTROL, &result, 4);


}

void beginPControl(ControlWaitInfoPtr waitInfo){

	wLog("begin p control.\n");

	pthread_t thread;
	pthread_create(&thread, NULL, startPControl, waitInfo);
	pthread_detach(thread);


	int result = 0;
	send_kpacket(cmdSock, cmdWriteBuffer, ACK_CALLPCONTROL, &result, 4);


}


void beginPing(){

	send_kpacket(cmdSock, cmdWriteBuffer, CMD_PING, NULL, 0);
}

void changePixel(){

	CHANGEPIXEL *c;
    c = (struct tagCHANGEPIXEL *)(cmdReadBuffer+sizeof(struct tagKPACKET));

    wLog("################################################################\n");
	wLog("Change Pixel  = %d\n", c->percent);
	wLog("################################################################\n");

	change_pixel_size = 1;
}

void command_relay(ControlWaitInfoPtr waitInfo){

	wLog("################################################################\n");
	wLog("# Connect Command Relay host=%s:%d\n", waitInfo->relayHost, waitInfo->relayPort);
	wLog("################################################################\n");

	int sock;
	bool success;
	sock = create_socket(waitInfo->relayHost, waitInfo->relayPort);
	cmdSock = sock;



	wLog("[cmd-sock] crete socket = %d\n", sock);

	struct _GUID *cguid;
	cguid = malloc(sizeof(struct _GUID));

	generate_guid(cguid, waitInfo->controlWaitSystemId);

#ifdef READ_BUFFER_DEBUG
	printGuid(cguid);
#endif


	//OPENRELAY
	success = send_kpacket(sock, cmdWriteBuffer, CMD_OPENRELAY, cguid, sizeof(struct _GUID));

	free(cguid);

	if(!success){
		wLog("[cmd-sock] failed. OPENRELAY send data.\n");
		destroy(-1);
	}

	setSocketReadTimeout(sock, 5);

	size_t recvOpenRelaySize = sizeof(struct tagKPACKET)+sizeof(struct _GUID);
	success = read_data(sock, cmdReadBuffer, recvOpenRelaySize);
	if(!success){
		wLog("[cmd-sock] failed. OPENRELAY recv data.\n");
		destroy(-1);
	}
#ifdef READ_BUFFER_DEBUG
	printHex(cmdReadBuffer, recvOpenRelaySize);
#endif

	KPACKET *kp = getKPacket(cmdReadBuffer);
	wLog("[cmd-sock] OPENRELAY recv data >> command=%02x size=%d\n", kp->Command, kp->wSize);



	//CONNECTINFO
	struct tagCONNECTINFO *cinfo;
	cinfo = malloc(sizeof(struct tagCONNECTINFO));
	sprintf(cinfo->szIPAddress, "%s", waitInfo->relayHost);
	cinfo->nPort = waitInfo->relayPort;

	success = send_kpacket(sock, cmdWriteBuffer, CMD_CONNECTINFO, cinfo, sizeof(struct tagCONNECTINFO));
	if(!success){
		wLog("[cmd-sock] failed. CONNECTINFO send data.\n");
		destroy(-1);
	}
	free(cinfo);

	success = read_data(sock, cmdReadBuffer, sizeof(struct tagKPACKET));
	if(!success){
		wLog("[cmd-sock] failed. CONNECTINFO HEADER recv data.\n");
		destroy(-1);
	}
	kp = getKPacket(cmdReadBuffer);
	wLog("[cmd-sock] CONNECTINFO recv data >> command=%02x size=%d\n", kp->Command, kp->wSize);

	success = read_data(sock, cmdReadBuffer+sizeof(struct tagKPACKET), kp->wSize);
	if(!success){
		wLog("[cmd-sock] failed. CONNECTINFO recv data.\n");
		destroy(-1);
	}

	setSocketReadTimeout(sock, 60);

	UNLOCK(cmdMutex);

	KPACKET *header;

	while(1){

		success = read_data(sock, cmdReadBuffer, sizeof(struct tagKPACKET));
		if(!success){
			wLog("[cmd-sock] failed. KPACKET recv data.\n");
			destroy(-1);
		}

		header = getKPacket(cmdReadBuffer);

		if(header->Command != CMD_PING)
			wLog("[cmd-sock] KPACKET recv data >> command=%02x size=%d\n", header->Command, header->wSize);

		if(header->wSize > 0){
			success = read_data(sock, cmdReadBuffer+10, header->wSize);
			if(!success){
				wLog("[cmd-sock] failed. DATA recv data.\n");
				destroy(-1);
			}
		}

		if(header->Command == CMD_CALLHWCONTROL){
			beginHWControl(waitInfo);
		}
		else if(header->Command == CMD_CALLPCONTROL){
			beginPControl(waitInfo);
		}
		else if(header->Command == CMD_PING){
			beginPing();
		}
		else if(header->Command == CMD_CHANGEPIXEL){
			changePixel();
		}
		else if(header->Command == CMD_DESTROY){
			wLog("CMD_DESTROY.\n");
			destroy(0);
		}
	}
}


void* start_command_socket(void* ptr){
	
	ControlWaitInfoPtr waitInfo;
	waitInfo = (ControlWaitInfoPtr)ptr;

	command_relay(waitInfo);
}

void* start_p_socket(void* ptr){

	ControlWaitInfoPtr waitInfo;
	waitInfo = (ControlWaitInfoPtr)ptr;

	command_relay(waitInfo);
}

bool start_relay(rfbClientPtr cl){

	remoteSock = cl->sock;

	wLog("################################################################\n");
	wLog("start realy  = [controlWaitSystemId:%s]\n", waitInfo->controlWaitSystemId);
	wLog("################################################################\n");

	wLog("wait system id size ==== %s\n", waitInfo->controlWaitSystemId);

	strcpy(cl->controlWaitSystemId, waitInfo->controlWaitSystemId);
	strcpy(cl->mySystemId, waitInfo->mySystemId);

	sprintf(cl->controlWaitSystemId, "%s", waitInfo->controlWaitSystemId);
	sprintf(cl->mySystemId, "%s", waitInfo->mySystemId);

	wLog("GUID info.\n");
	wLog("   [control] = [%s]\n", cl->controlWaitSystemId);
	wLog("   [my]      = [%s]\n", cl->mySystemId);
	wLog("################################################################\n");
	
	

	setStatusNotify(STATUS_NOTIFY_CONNECT_CMD_SOCK);
	LOCK(cmdMutex);

	
	pthread_create(&cmdThread, NULL, start_command_socket, (void*)waitInfo);
	pthread_detach(cmdThread);


	LOCK(cmdMutex);
	UNLOCK(cmdMutex);

	sleep(1);

	setStatusNotify(STATUS_NOTIFY_RESUME_REPLAY_SOCK);
	
	bool success;
	int sock;
	sock = cl->sock;
	
	struct _GUID *cguid;
	cguid = malloc(sizeof(struct _GUID));

	generate_guid(cguid, cl->controlWaitSystemId);

#ifdef READ_BUFFER_DEBUG
	printGuid(cguid);
#endif

	//OPENRELAYcmd_destroy
	success = send_kpacket(sock, remoteReadBuffer, CMD_OPENRELAY, cguid, sizeof(struct _GUID));

	free(cguid);

	if(!success){
		wLog("[remote-sock] failed. OPENRELAY send data.\n");
		destroy(-1);
	}

	size_t recvOpenRelaySize = sizeof(struct tagKPACKET)+sizeof(struct _GUID);

	sleep(1);

	success = read_fully(sock, remoteReadBuffer, recvOpenRelaySize);
	if(!success){
		wLog("[remote-sock] failed. OPENRELAY recv data.\n");
		destroy(-1);
	}

#ifdef READ_BUFFER_DEBUG
	printHex(remoteReadBuffer, recvOpenRelaySize);
#endif

	KPACKET *kp = getKPacket(remoteReadBuffer);
	wLog("[remote-sock] OPENRELAY recv data >> command=%02x size=%d\n", kp->Command, kp->wSize);


	sleep(1);

	cguid = malloc(sizeof(struct _GUID));

	generate_guid(cguid, cl->mySystemId);

#ifdef GUID_DEBUG
	printGuid(cguid);
#endif

	gint width = gdk_screen_width();
	gint height = gdk_screen_height();

	wLog("screen size = %d x %d\n", width, height);

	struct tagCREATEMODULEDATA *cdata;
	cdata = malloc(sizeof(struct tagCREATEMODULEDATA));
	cdata->SystemID = *cguid;
	cdata->deviceType = DEVICE_TYPE_LINUX;
	cdata->width = width;
	cdata->height= height;
	
	success = send_kpacket(cmdSock, cmdWriteBuffer, CMD_CREATEMODULEDATA, cdata, sizeof(struct tagCREATEMODULEDATA));
	free(cguid);

	if(!success){
		wLog("[remote-sock] failed. OPENRELAY send data.\n");
		destroy(-1);
		return false;
	}

	setStatusNotify(STATUS_NOTIFY_CONNECTED);
	wLog("################## END START RELAY #####################\n");
	return true;
}

void send_cmd_destroy(){

	if(!cmdSock)
		return;

	bool success;
	success = send_kpacket(cmdSock, cmdWriteBuffer, CMD_DESTROY, NULL, 0);
}

void send_chat_message(const char* message){

	if(!pSock){
		return;
	}

	bool success;
	success = send_netproto(pSock, pWriteBuffer, PROC_CHATMSG_TO_VIEWER, message, strlen(message));
	wLog("send chat msg ===== \n");
}



