
#include <uuid/uuid.h>
#include <cJSON.h>

#include <ctype.h>
#include <sys/sysinfo.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <sys/statvfs.h>


#include "x11vnc.h"

#include "w_http.h"
#include "w_ui.h"
#include "w_utils.h"

all_hostname_verify(void *userdata, int failures, const ne_ssl_certificate *cert){

		/*
		const char *hostname = userdata;

	  dump_cert(cert);

	  puts("Certificate verification failed \- the connection may have been "
	       "intercepted by a third party!");

	  if (failures & NE_SSL_IDMISMATCH) {
	    const char *id = ne_ssl_cert_identity(cert);
	    if (id)
	      printf("Server certificate was issued to \'%s\' not \'%s\'\.\en",
	             id, hostname);
	    else
	      printf("The certificate was not issued for \'%s\'\en", hostname);
	  }

	  if (failures & NE_SSL_UNTRUSTED)
	    puts("The certificate is not signed by a trusted Certificate Authority\.");


	  if (prompt_user())
	    return 1; //fail verification
	  else
	    return 0; //trust the certificate anyway
	    */
		return 0;
 }

bool http_request(const char* scheme, const char* host, const int port, const char* url, const char* method, const char* content_type, ne_block_reader response_handler, void* userdata, const char* reqdata)
{

	wLog("[http] [%s]://[%s]:[%d][%s] method=[%s]\n", scheme, host, port, url, method);

    ne_session *sess;
    ne_request *req;

    ne_sock_init();

    sess = ne_session_create(scheme, host, port);
    ne_ssl_set_verify(sess, all_hostname_verify, host);
    ne_set_useragent(sess, "WizHelperForLinux/1.0");
    ne_set_read_timeout(sess, 30);
    ne_set_connect_timeout(sess, 30);

    req = ne_request_create(sess, method, url);
    // if accepting only 2xx codes, use "ne_accept_2xx"


	//post
	if(!strcmp(method, HTTP_METHOD_POST)){

		if(reqdata != NULL){
			wLog("write post buffer.\n");
			ne_set_request_body_buffer(req, reqdata, strlen(reqdata));
		}
	}

	ne_add_request_header(req, "Content-type", content_type);

	if(response_handler != NULL){
		ne_add_response_body_reader(req, ne_accept_2xx, response_handler, userdata);
	}

    int result = ne_request_dispatch(req);
    int statusCode = ne_get_status(req)->code;

    ne_request_destroy(req);

    const char* errorMessage = ne_get_error(sess);
    ne_session_destroy(sess);

    if(statusCode != 200){
    	wLog("http res failed >> statusCode = %d, err=%s\n", statusCode, errorMessage);
    	return false;
    }

    wLog("result=%d, statusCode=%d\n", result, statusCode);

    switch (result) {
		case NE_OK:
			wLog("http res success\n");
			return true;
		default:
			wLog("http res failed >> [%d]%s\n", result, errorMessage);
			return false;
    }
}





int response_get_control_info_by_list_handler(void *userdata, const char *buf, size_t len){

	if(len == 0)
		return NE_OK;

	ControlWaitInfoPtr waitInfo = (ControlWaitInfoPtr)userdata;

	wLog("res buffer[%s]\n", buf);

	cJSON *json = NULL;
    json = cJSON_Parse(buf);
	if (!json){
		wLog("Error before: [%s]\n", cJSON_GetErrorPtr());
    }
    else{

    	bool isEmpty = true;

		if(cJSON_HasObjectItem(json, "clist")){

			cJSON *clist = cJSON_GetObjectItem(json, "clist");
			int size = cJSON_GetArraySize(clist);
			wLog("control info list size ================ %d\n", size);

			if(size > 0){

				isEmpty = false;
				cJSON *data = cJSON_GetArrayItem(clist, 0);
				char *cpCode = cJSON_GetObjectItem(data, "cpCode")->valuestring;
				char *controlSystemId = cJSON_GetObjectItem(data, "controlSystemId")->valuestring;

				strcpy(waitInfo->cpCode, cpCode);
				strncpy(waitInfo->controlWaitSystemId, controlSystemId+1, 36);
				waitInfo->controlWaitSystemId[36] = '\0';


				wLog("res => cpcode=[%s]\n", waitInfo->cpCode);
				wLog("res => controlWaitSystemId=[%s]\n", waitInfo->controlWaitSystemId);

			}
		}

		if(isEmpty){
			wLog("control wait info list is empty.\n");
			waitInfo->reqResult = REQ_RESULT_ACCEPTNUM_EMPTY;
			return NE_OK;
		}
		cJSON_Delete(json);
    }


    wLog("http response body  >> %s\n", buf);
    return NE_OK;
}

int response_get_control_info_by_acceptnum_handler(void *userdata, const char *buf, size_t len){

	if(len == 0)
		return NE_OK;

	ControlWaitInfoPtr waitInfo = (ControlWaitInfoPtr)userdata;

	wLog("res buffer[%s]\n", buf);

	cJSON *json = NULL;
    json = cJSON_Parse(buf);
	if (!json){
		wLog("Error before: [%s]\n", cJSON_GetErrorPtr());
    }
    else{

		if(cJSON_HasObjectItem(json, "data")){

			cJSON *data = cJSON_GetObjectItem(json, "data");
			char *cpCode = cJSON_GetObjectItem(data, "cpCode")->valuestring;
			char *controlSystemId = cJSON_GetObjectItem(data, "controlSystemId")->valuestring;

			strcpy(waitInfo->cpCode, cpCode);
			strncpy(waitInfo->controlWaitSystemId, controlSystemId+1, 36);
			waitInfo->controlWaitSystemId[36] = '\0';

			wLog("cpcode=[%s]\n", waitInfo->cpCode);
			wLog("controlWaitSystemId=[%s]\n", waitInfo->controlWaitSystemId);

		}
		else{
			wLog("control wait info is empty.\n");
			waitInfo->reqResult = REQ_RESULT_ACCEPTNUM_EMPTY;
			return NE_OK;
		}
		cJSON_Delete(json);
    }


    wLog("http response body  >> %s\n", buf);
    return NE_OK;
}



int response_pre_proc_remote_start(void *userdata, const char *buf, size_t len){

	if(len == 0)
		return NE_OK;


	ControlWaitInfoPtr waitInfo = (ControlWaitInfoPtr)userdata;

	wLog("res buffer[%s]\n", buf);

	cJSON *json = NULL;
    json = cJSON_Parse(buf);
	if (!json){
		wLog("Error before: [%s]\n", cJSON_GetErrorPtr());
    }
    else{
    	int isControlWait = cJSON_GetObjectItem(json, "isControlWait")->valueint;
    	if(!isControlWait){
    		wLog("agent is not control wait.\n");
    		return NE_ERROR;
    	}

    	char *controlWaitSystemId = cJSON_GetObjectItem(json, "controlWaitSystemId")->valuestring;
    	char *relayServerHost = cJSON_GetObjectItem(json, "relayServerHost")->valuestring;
    	int relayServerPort = cJSON_GetObjectItem(json, "relayServerPort")->valueint;

    	wLog("[preProcRemoteStart] result >> controlWaitSystemId[%s] relayServerHost[%s] relayServerPort[%d]\n",
    			controlWaitSystemId, relayServerHost, relayServerPort);

		strcpy(waitInfo->relayHost, relayServerHost);
    	waitInfo->relayPort = relayServerPort;

    	waitInfo->success = true;
		cJSON_Delete(json);
    }


    wLog("http response body  >> %s\n", buf);
    return NE_OK;
}


bool request_pre_proc_remote_start(ControlWaitInfoPtr waitInfo){


	wLog("[request] pre_proc_remote_start. wait=[%s] my=[%s]\n", waitInfo->controlWaitSystemId, waitInfo->mySystemId);

	char reqdata[1024];



	struct utsname *un;
	struct sysinfo sys;

	un = malloc(sizeof(struct utsname));
	uname(un);
	sysinfo(&sys);

	char hostname[100];
	char os[110];
	uint64_t memtotal = 0;
	uint64_t memfree = 0;

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

	char resolution[100];
	sprintf(resolution, "%d x %d", gdk_screen_width(), gdk_screen_height());

	uint64_t disktotal = 0;
	uint64_t diskfree = 0;
	struct statvfs stat;
	if(statvfs("/", &stat) != 0) {

	}
	else{
		disktotal = stat.f_bsize * stat.f_blocks;
		diskfree = stat.f_bsize * stat.f_bavail;
	}

	char* memtotal_format = getFileSizeFormat(memtotal);
	char* memfree_format = getFileSizeFormat(memfree);
	char *disktotal_format = getFileSizeFormat(disktotal);
	char *diskfree_format = getFileSizeFormat(diskfree);

	char* ip_addr = inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr);

	wLog("\tos = [%s]\n", os);
	wLog("\thostname = [%s]\n", hostname);
	wLog("\tmem total = [%s]\n", memtotal_format);
	wLog("\tmem free = [%s]\n", memfree_format);
	wLog("\tdisk total  = [%s]\n", disktotal_format);
	wLog("\tdisk free  = [%s]\n", diskfree_format);
	wLog("\tcpu info  = [%s]\n", cpuinfo);
	wLog("\tlancard info  = [%s]\n", laninfo);
	wLog("\tip addr = [%s]\n", ip_addr);
	wLog("\tresolution = [%s]\n", resolution);


	char *param_names =
			"deviceType=%d" \
			"&controlWaitSystemId={%s}" \
			"&mySystemId={%s}" \
			"&requestName=%s"\
			"&requestTel=%s"\
			"&regDeviceId=%s"\
			"&vMode=%s"\
			"&ipAddress=%s"\
			"&computerName=%s"\
			"&os=%s"\
			"&cpu=%s"\
			"&ram=%s"\
			"&hdd=%s"\
			"&lan=%s"\
			"&vga=%s";


	sprintf(reqdata,
			param_names,
			DEVICE_TYPE_LINUX,
			waitInfo->controlWaitSystemId,
			waitInfo->mySystemId,
			"", //name
			"",	//tel
			"",	//devieid
			"false", //vmode
			ip_addr,
			hostname,
			os,
			cpuinfo,
			memtotal_format,
			disktotal_format,
			laninfo,
			resolution);

	wLog("req data >> %s\n", reqdata);



	bool success;
	success = http_request(
			waitInfo->webServerScheme,
			waitInfo->webServerHost,
			waitInfo->webServerPort,
			"/external/preProcRemoteStart.do",
			HTTP_METHOD_POST,
			"application/x-www-form-urlencoded",
			response_pre_proc_remote_start,
			waitInfo,
			reqdata);


	free(memtotal_format);
	free(memfree_format);
	free(disktotal_format);
	free(diskfree_format);

	if(!success){
		wLog("failed http request >> \n");
		return false;
	}



	return true;




}


bool request_acceptnum(ControlWaitInfoPtr data){

	char url[100];

	ne_block_reader res_handler;
	if(!strcmp(data->acceptNum, "test")){
		sprintf(url, "/external/getUserInfoList.do?cpCode=%s", "nspro");
		res_handler = response_get_control_info_by_list_handler;
	}
	else{
		sprintf(url, "/external/getUserInfoByAcceptNum.do?acceptNum=%s", data->acceptNum);
		res_handler = response_get_control_info_by_acceptnum_handler;
	}

	wLog("request acceptnum url[%s] = %s\n", data->webServerScheme, url);

	bool success;

	data->reqResult = 0;

	success = http_request(
			data->webServerScheme,
			data->webServerHost,
			data->webServerPort,
			url,
			HTTP_METHOD_GET,
			"text/html",
			res_handler,
			data,
			NULL);


	if(success){

		if(waitInfo->reqResult == REQ_RESULT_ACCEPTNUM_EMPTY){
			wLog("acceptnum is empty.\n");
			return true;
		}

		uuid_t uuid;
		uuid_generate_random(uuid);

		//ex 45cd6c3b-88e0-4d89-bb9d-9a225395074a
		char uuid_str[37];
		uuid_unparse_upper(uuid, uuid_str);


		wLog("ControlWaitInfo cpCode=%s controlWaitSystemId=%s mySystemId=%s\n", data->cpCode, data->controlWaitSystemId, uuid_str);

		strcpy(data->mySystemId, uuid_str);

		return request_pre_proc_remote_start(data);

	}

	return false;
}
