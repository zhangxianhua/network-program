
/****************** SERVER CODE ****************/

#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <sys/types.h>
#include <netinet/in.h>  
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/types.h>
#include <netinet/in.h>  

#include "BroadHead.h"
//#include "BaseBroadProtocol.h"

#define UDP_RECV_PORT1 11002
#define UDP_RECV_PORT2 11004
#define UDP_SEND_PORT1 13002
#define UDP_SEND_PORT2 13004

#define UDP_IP_ADDR "172.17.1.72"

//------------------------Get Cpu And Mem Info-----------------
typedef struct _cpuOccupy {
    char name[20];
    unsigned int user;
    unsigned int nice;
    unsigned int system;
    unsigned int idle;
    unsigned int iowait;
    unsigned int irq;
    unsigned int softirq;
} cpuOccupy;

typedef struct _memOccupy {
    char name[20];
    unsigned long total;
    unsigned long free;
} memOccupy;

int calCpuOccupy(cpuOccupy *c1, cpuOccupy *c2) {
    unsigned long total1, total2 , id, sd, xd;
    int cpuUsage = 0;

    total1 = c1->user + c1->nice + c1->system + c1->idle + c1->iowait + c1->irq + c1->softirq;
    total2 = c2->user + c2->nice + c2->system + c2->idle + c2->iowait + c2->irq + c2->softirq;

    id = c2->user - c1->user;
    sd = c2->system - c1->system;
    xd = c2->nice - c1->nice;

    if((total2 - total1) != 0)
        cpuUsage = ((sd + id + xd) *100) / (total2 - total1);
    else
        cpuUsage = 0;
    return cpuUsage;
}

void getCpuOccupy(cpuOccupy *cpust) {
    FILE *fp;
    char buf[256];
    cpuOccupy *cpuOcc;
    cpuOcc = cpust;

    fp = fopen("/proc/stat", "r");
    if(!fp) {
        fprintf(stderr, "open /proc/stat error!\n");
        return;
    }
    fgets(buf, sizeof(buf), fp);
    sscanf(buf, "%s %u %u %u %u %u %u %u", cpuOcc->name, &cpuOcc->user, &cpuOcc->nice, &cpuOcc->system, &cpuOcc->idle, &cpuOcc->iowait
            ,&cpuOcc->irq, &cpuOcc->softirq);
    fclose(fp);
}

void getMemOccupy(memOccupy *memst) {
    FILE *fp;
    char buf[256];
    memOccupy *memOcc;
    memOcc = memst;

    fp = fopen("/proc/meminfo", "r");
    if(!fp) {
        fprintf(stderr, "open /proc/meminfo!\n");
        return;
    }
    fgets(buf, sizeof(buf), fp);
    sscanf(buf, "%s %lu %s", memOcc->name, &memOcc->total, memOcc->name); 
    fgets(buf, sizeof(buf), fp);
    sscanf(buf, "%s %lu %s", memOcc->name, &memOcc->free, memOcc->name);
	fclose(fp);
}

void getCpuAndMemUsed(int* cpuUsed, int* memUsed) {
    cpuOccupy cpuStat1;
    cpuOccupy cpuStat2;
    memOccupy memstat;
    int cpuUsage;
    int memUsage;

    memset(&cpuStat1, 0, sizeof(cpuOccupy));
    memset(&cpuStat2, 0, sizeof(cpuOccupy));
    memset(&memstat, 0, sizeof(memOccupy));
    
    getMemOccupy(&memstat);
    if(memstat.total != 0)
        memUsage = ((memstat.total - memstat.free) * 100) / memstat.total;
    else
        memUsage = 0;

    getCpuOccupy(&cpuStat1);
    sleep(1);
    getCpuOccupy(&cpuStat2);
    cpuUsage = calCpuOccupy(&cpuStat1, &cpuStat2);
    fprintf(stdout, "CPU Usage: %3d%%\t MEM Usage: %3d%%\n", cpuUsage, memUsage);
    fflush(stdout);
	*cpuUsed = cpuUsage * 100;
	*memUsed = memUsage * 100;
}
//------------------------Get Cpu And Mem Info-----------------
int main(){
	
  int welcomeSocket;//, sendSocket;
  char buffer[1024];
  //char buffer1[1024];
  struct sockaddr_in serverAddr, clientAddr, recvServerAddr;
  //struct sockaddr_storage serverStorage;
  socklen_t addr_size;

  /*---- Create the socket. The three arguments are: ----*/
  /* 1) Internet domain 2) Stream socket 3) Default protocol (TCP in this case) */
  welcomeSocket = socket(AF_INET, SOCK_DGRAM, 0);
  
  /*---- Configure settings of the server address struct ----*/
  /* Address family = Internet */
  serverAddr.sin_family = AF_INET;
  /* Set port number, using htons function to use proper byte order */
  serverAddr.sin_port = htons(UDP_RECV_PORT1);
  /* Set IP address to localhost */
  //serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);//inet_addr("127.0.0.1");
  serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);//inet_addr("127.0.0.1");
  /* Set all bits of the padding field to 0 */
  memset(serverAddr.sin_zero, '\0', sizeof serverAddr.sin_zero);  
  /*---- Bind the address struct to the socket ----*/
  if(bind(welcomeSocket, (struct sockaddr *) &serverAddr, sizeof(serverAddr)) == -1){
  	perror("bind error!");
  }
  
  printf("%s %d %d serverAddr.sin_port=%d", __func__, __LINE__, CPU_STORAGE_INFO_RESPONSE, ntohs(serverAddr.sin_port));

//Send Socket
  //sendSocket = socket(AF_INET, SOCK_DGRAM, 0);
    /*---- Configure settings of the server address struct ----*/
  /* Address family = Internet */
  recvServerAddr.sin_family = AF_INET;
  /* Set port number, using htons function to use proper byte order */
  recvServerAddr.sin_port = htons(UDP_SEND_PORT1);
  /* Set IP address to localhost */
  //serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);//inet_addr("127.0.0.1");
  recvServerAddr.sin_addr.s_addr = htonl(INADDR_ANY);//inet_addr("127.0.0.1");
  /* Set all bits of the padding field to 0 */
  memset(recvServerAddr.sin_zero, '\0', sizeof recvServerAddr.sin_zero); 

#if 0
  /*---- Bind the address struct to the socket ----*/
  if(bind(sendSocket, (struct sockaddr *) &serverAddr, sizeof(serverAddr)) == -1){
  	perror("bind error!");
  }

  /*---- Listen on the socket, with 5 max connection requests queued ----*/
  if(listen(welcomeSocket,5)==0)
    printf("Listening\n");
  else
    printf("listen Error11\n");

  /*---- Accept call creates a new socket for the incoming connection ----*/
  addr_size = sizeof serverStorage;
  newSocket = accept(welcomeSocket, (struct sockaddr *) &serverStorage, &addr_size);
#endif
 printf("sizeof(BoardHead)=%d\n", sizeof(BoardHead));
  /*---- Send message to the socket of the incoming connection ----*/
  //strcpy(buffer,"Hello World\n");
  //send(newSocket,buffer,13,0);
  int* cpuUsed, *memUsed;
  cpuUsed = (int*)malloc(sizeof(int));
  memUsed = (int*)malloc(sizeof(int));
  getCpuAndMemUsed(cpuUsed, memUsed);
  printf("%3d%% %3d%%\n", *cpuUsed, *memUsed);
				
  while(1){  
  	//char* sendData="abcd1234\n";
	int send_len = 0, recv_len = 0, send_data_len = 0;
    addr_size = sizeof(serverAddr);
  	//printf("sendData: %s welcomeSocket=%d addr_size=%d\n",sendData, welcomeSocket, addr_size);   
  	//send(clientSocket, sendData, strlen(sendData),0);
  //	send_len = sendto(welcomeSocket, sendData, strlen(sendData),0, (struct sockaddr*)&clientAddr, addr_size);
  	/*---- Read the message from the server into the buffer ----*/
  	//recv(clientSocket, buffer, 1024, 0);
	recv_len = recvfrom(welcomeSocket, buffer, 1024, 0, (struct sockaddr*)&clientAddr, &addr_size);
	if(recv_len != -1){
		//struct BoardHead boardHead;// = {0,0,1,1,1,1,1,1,1,1};
		//broadHead.ip = [172, 17, 3, 233];
		//struct BaseBroadProtocol baseBroadProtocol, recvBroadProtocal;
		struct QueryDataInfo queryDataInfo;
		//recvBroadProtocal = (struct BaseBroadProtocol)buffer;
		//memcpy(&recvBroadProtocal, buffer, recv_len);
		memcpy(&queryDataInfo, buffer, recv_len);
		
		#if 1
		printf("recv_len =%d ip[0]=0x%x ip[1]=0x%x ip[2]=0x%x ip[3]=0x%x flags=0x%x b=0x%x\
		interfaceCount=0x%x reserve3=0x%x license[0]=0x%x license[1]=0x%x license[2]=0x%x\
		license[3]=0x%x queryDataInfo.dataType=0x%x dataLength=0x%x requestDataType=0x%x paramLength=0x%x\n", 
		recv_len, 
		queryDataInfo.baseBroadProtocal.boardHead.ip[0], 
		queryDataInfo.baseBroadProtocal.boardHead.ip[1], 
		queryDataInfo.baseBroadProtocal.boardHead.ip[2],
		queryDataInfo.baseBroadProtocal.boardHead.ip[3],
		queryDataInfo.baseBroadProtocal.boardHead.flags,
		queryDataInfo.baseBroadProtocal.boardHead.b,
		queryDataInfo.baseBroadProtocal.boardHead.interfaceCount,
		queryDataInfo.baseBroadProtocal.boardHead.reserve3,
		queryDataInfo.baseBroadProtocal.boardHead.license[0],
		queryDataInfo.baseBroadProtocal.boardHead.license[1],
		queryDataInfo.baseBroadProtocal.boardHead.license[2],
		queryDataInfo.baseBroadProtocal.boardHead.license[3],
		queryDataInfo.baseBroadProtocal.dataType,
		queryDataInfo.baseBroadProtocal.dataLength,
		queryDataInfo.requestDataType,
		queryDataInfo.paramLength);
		#endif
		struct CpuStorageInfoResponse cpuStorageInfoResponse;
		/*
			DEVICE_NET_INFO_IPV4_RESPONSE = 3,//DeviceNetInfoIPV4Response ->3	
	QUERY_DATA_INFO = 4,//QueryDataInfo ->4
	STORAGE_INFO_RESPONSE = 8,//StorageInfoResponse ->8
	BASE_BROAD_PROTOCOL = 9,//BaseBroadProtocol ->9
	CPU_STORAGE_INFO_RESPONSE = 17,//CpuStorageInfoResponse 17 ->0x11
	DVBT_SETTING_INFO_RESPONSE = 103, //DVBTSettingInfoResponse 103 ->0x67
	PID_FORWARD_SETTING_INFO_RESPONSE = 105,//PIDForwardSettingInfoResponse 105 ->0x69
	DVBT_DATA_INFO_RESPONSE = 109,//DVBTDataInfoResponse 109 ->0x6D
	STREAM_STRUCT_INFO_RESPONSE = 113,//StreamStructInfoResponse 113 ->0x71
	STREAM_STATUS_INFO_RESPONSE = 116,//StreamStatusInfoResponse 116 ->0x74
	DVBS2_SETTING_INFO_RESPONSE = 119,//DVBS2SettingInfoResponse 119 ->0x77
	DVBS2DATA_INFO_RESPONSE = 120,//DVBS2DataInfoResponse 120 ->0x78
	DTMB_SETTING_INFO_RESPONSE = 123,//DTMBSettingInfoResponse 123 ->0x7B
	DTMB_DATA_INFO_RESPONSE = 124//DTMBDataInfoResponse 124 ->0x7C
	
		short cpuUsage;
	short cpuFrequency;
	short cpuKernelNumber;
	short memUsage;
	short memSize;
	short flashSize;
	short flashRemainingSpace;
	int sdSize;
	int sdRemainingSpace;
	int diskSize;
	int diskRemainingSpace;
	byte otherLength;
	//int others[];
	int crc;
		*/
		//memset(&boardHead, 0x0, sizeof(BoardHead));
		//memset(&baseBroadProtocol, 0x0, sizeof(BaseBroadProtocol));
		switch(queryDataInfo.requestDataType){
			case DEVICE_VERSION_INFO_RESPONSE: //3
				printf("%s %d %d", __func__, __LINE__, DEVICE_VERSION_INFO_RESPONSE);
				struct DeviceNetInfoIPV4Response deviceNetInfoIPV4Response;
				memset(&deviceNetInfoIPV4Response, 0x0, sizeof(DeviceNetInfoIPV4Response));
				memcpy(&(deviceNetInfoIPV4Response.baseBroadProtocal.boardHead), &queryDataInfo.baseBroadProtocal.boardHead, sizeof(BoardHead));
				if(queryDataInfo.baseBroadProtocal.dataType != 2){
					deviceNetInfoIPV4Response.baseBroadProtocal.dataType = DEVICE_VERSION_INFO_RESPONSE;
					deviceNetInfoIPV4Response.baseBroadProtocal.dataLength = sizeof(deviceNetInfoIPV4Response) - sizeof(BaseBroadProtocol);
				}else{
					deviceNetInfoIPV4Response.baseBroadProtocal.dataType = DEVICE_VERSION_INFO_RESPONSE;
				}
				deviceNetInfoIPV4Response.hostNameLength = 1;
				deviceNetInfoIPV4Response.hostNames[0] = 2;
				deviceNetInfoIPV4Response.mac[0] = 172;
				deviceNetInfoIPV4Response.mac[1] = 17;
				deviceNetInfoIPV4Response.mac[2] = 2;
				deviceNetInfoIPV4Response.mac[3] = 1;
				deviceNetInfoIPV4Response.ip[0] = 172;
				deviceNetInfoIPV4Response.ip[1] = 17;
				deviceNetInfoIPV4Response.ip[2] = 3;
				deviceNetInfoIPV4Response.ip[3] = 233;
				deviceNetInfoIPV4Response.mask[0] = 255;
				deviceNetInfoIPV4Response.mask[1] = 255;
				deviceNetInfoIPV4Response.mask[2] = 0;
				deviceNetInfoIPV4Response.mask[3] = 0;
				deviceNetInfoIPV4Response.netGate[0] = 172;
				deviceNetInfoIPV4Response.netGate[1] = 17;
				deviceNetInfoIPV4Response.netGate[2] = 0;
				deviceNetInfoIPV4Response.netGate[3] = 1;
				deviceNetInfoIPV4Response.ttl = 0;
				break;
			case QUERY_DATA_INFO: //4
				printf("%s %d %d", __func__, __LINE__, QUERY_DATA_INFO);
				break;	
			case STORAGE_INFO_RESPONSE: //8
				printf("%s %d %d", __func__, __LINE__, STORAGE_INFO_RESPONSE);
				break;
			case BASE_BROAD_PROTOCOL: //9
				printf("%s %d %d", __func__, __LINE__, BASE_BROAD_PROTOCOL);
				break;
			case CPU_STORAGE_INFO_RESPONSE: //17
				printf("%s %d %d clientAddr.sin_port=%d", __func__, __LINE__, CPU_STORAGE_INFO_RESPONSE, clientAddr.sin_port);
				cpuUsed = (int*)malloc(sizeof(int));
				memUsed = (int*)malloc(sizeof(int));
				getCpuAndMemUsed(cpuUsed, memUsed);
				printf("%3d%% %3d%%\n", *cpuUsed, *memUsed);

				memset(&cpuStorageInfoResponse, 0x0, sizeof(CpuStorageInfoResponse));
				memcpy(&(cpuStorageInfoResponse.baseBroadProtocal.boardHead), &queryDataInfo.baseBroadProtocal.boardHead, sizeof(BoardHead));
				if(queryDataInfo.baseBroadProtocal.dataType != 2){
					cpuStorageInfoResponse.baseBroadProtocal.dataType = CPU_STORAGE_INFO_RESPONSE;
					cpuStorageInfoResponse.baseBroadProtocal.dataLength = sizeof(CpuStorageInfoResponse) - sizeof(BaseBroadProtocol);
				}else{
					cpuStorageInfoResponse.baseBroadProtocal.dataType = CPU_STORAGE_INFO_RESPONSE;
				}
				
				cpuStorageInfoResponse.cpuUsage = *cpuUsed;
				cpuStorageInfoResponse.cpuFrequency = 12300;		
				cpuStorageInfoResponse.cpuKernelNumber = 6666;
				cpuStorageInfoResponse.memUsage = *memUsed;
				cpuStorageInfoResponse.memSize = 30;
				cpuStorageInfoResponse.flashSize = 40;
				cpuStorageInfoResponse.flashRemainingSpace = 50;
				cpuStorageInfoResponse.sdSize = 60;
				cpuStorageInfoResponse.sdRemainingSpace = 80;
				cpuStorageInfoResponse.diskSize = 100;
				cpuStorageInfoResponse.diskRemainingSpace = 90;
				cpuStorageInfoResponse.otherLength = 0;
				cpuStorageInfoResponse.crc = 0; 
				
				free(cpuUsed);
				free(memUsed);
				if(queryDataInfo.baseBroadProtocal.dataType != 2){
					send_data_len = sizeof(CpuStorageInfoResponse) - sizeof(cpuStorageInfoResponse.baseBroadProtocal.dataLength);
					printf("send_data_len=%d sizeof(CpuStorageInfoResponse)=%d sizeof(cpuStorageInfoResponse.baseBroadProtocal.dataLength)=%d sizeof(CpuStorageInfoResponse)-sizeof(BaseBroadProtocol)=%d\n", 
					send_data_len, sizeof(CpuStorageInfoResponse),sizeof(cpuStorageInfoResponse.baseBroadProtocal.dataLength),sizeof(CpuStorageInfoResponse)-sizeof(BaseBroadProtocol));
				}else{
					send_data_len = sizeof(CpuStorageInfoResponse);
				}
				memcpy(buffer, &cpuStorageInfoResponse, send_data_len);
				//sprintf(buffer, "%s0x%x0x%x", buffer, 0x0D, 0x0A);
				//int others[];
				//int crc;
				//strcpy(buffer,"Hello World\n");
				//send_len = sendto(welcomeSocket, buffer, 13,0, (struct sockaddr*)&recvServerAddr, sizeof(recvServerAddr));
				//memset(buffer1, '\0', sizeof buffer1);  
				//sprintf(buffer1, "0x%x0x%x0x%x0x%x0x%x0x%x0x%x0x%x0x%x0x%x0x%x0x%x\n", 0x48, 0x49, 0x4A, 0x4B, 0x4C, 0x4D,0x48, 0x49, 0x4A, 0x4B, 0x4C, 0x4D);

	//			recvServerAddr.sin_port = htons(UDP_RECV_PORT1);
				//send_len = sendto(welcomeSocket, &cpuStorageInfoResponse, sizeof(CpuStorageInfoResponse),0, (struct sockaddr*)&recvServerAddr, sizeof(recvServerAddr));
	//			send_len = sendto(welcomeSocket, buffer, sizeof(CpuStorageInfoResponse)+6,0, (struct sockaddr*)&recvServerAddr, sizeof(recvServerAddr));
				//send_len = sendto(welcomeSocket, buffer, 13,0, (struct sockaddr*)&recvServerAddr, sizeof(recvServerAddr));
				//send_len = sendto(welcomeSocket, buffer1, 14,0, (struct sockaddr*)&recvServerAddr, sizeof(recvServerAddr));

		//		recvServerAddr.sin_port = htons(UDP_RECV_PORT2);
		//		send_len = sendto(welcomeSocket, buffer, sizeof(CpuStorageInfoResponse)+6,0, (struct sockaddr*)&recvServerAddr, sizeof(recvServerAddr));
				//send_len = sendto(welcomeSocket, &cpuStorageInfoResponse, sizeof(CpuStorageInfoResponse),0, (struct sockaddr*)&recvServerAddr, sizeof(recvServerAddr));
				//send_len = sendto(welcomeSocket, buffer, 13,0, (struct sockaddr*)&recvServerAddr, sizeof(recvServerAddr));
				//send_len = sendto(welcomeSocket, buffer1, 14,0, (struct sockaddr*)&recvServerAddr, sizeof(recvServerAddr));
				
				recvServerAddr.sin_port = htons(UDP_SEND_PORT1);
				send_len = sendto(welcomeSocket, buffer, send_data_len,0, (struct sockaddr*)&recvServerAddr, sizeof(recvServerAddr));
				//send_len = sendto(welcomeSocket, buffer, sizeof(CpuStorageInfoResponse)+6,0, (struct sockaddr*)&recvServerAddr, sizeof(recvServerAddr));
//				send_len = sendto(welcomeSocket, &cpuStorageInfoResponse, sizeof(CpuStorageInfoResponse),0, (struct sockaddr*)&recvServerAddr, sizeof(recvServerAddr));
			//	send_len = sendto(welcomeSocket, buffer, 13,0, (struct sockaddr*)&recvServerAddr, sizeof(recvServerAddr));
				//send_len = sendto(welcomeSocket, buffer1, 14,0, (struct sockaddr*)&recvServerAddr, sizeof(recvServerAddr));

				//recvServerAddr.sin_port = htons(UDP_SEND_PORT2);
		/////////		send_len = sendto(welcomeSocket, buffer, sizeof(CpuStorageInfoResponse)+6,0, (struct sockaddr*)&recvServerAddr, sizeof(recvServerAddr));
				//send_len = sendto(welcomeSocket, buffer1, 14,0, (struct sockaddr*)&recvServerAddr, sizeof(recvServerAddr));
				//printf("%s %d %d clientAddr.sin_port=%d", __func__, __LINE__, CPU_STORAGE_INFO_RESPONSE, clientAddr.sin_port);
				//send_len = sendto(welcomeSocket, &cpuStorageInfoResponse, sizeof(CpuStorageInfoResponse),0, (struct sockaddr*)&recvServerAddr, sizeof(recvServerAddr));
				//send_len = sendto(welcomeSocket, buffer, 13,0, (struct sockaddr*)&recvServerAddr, sizeof(recvServerAddr));
				break;
			case DVBT_SETTING_INFO_RESPONSE: //103
				printf("%s %d %d", __func__, __LINE__, DVBT_SETTING_INFO_RESPONSE);
				break;
			case PID_FORWARD_SETTING_INFO_RESPONSE: //105
				printf("%s %d %d", __func__, __LINE__, PID_FORWARD_SETTING_INFO_RESPONSE);
				struct PIDForwardSettingInfoResponse pidForwardSettingInfoResponse;
				memset(&pidForwardSettingInfoResponse, 0x0, sizeof(PIDForwardSettingInfoResponse));
				memcpy(&(pidForwardSettingInfoResponse.baseBroadProtocal.boardHead), &queryDataInfo.baseBroadProtocal.boardHead, sizeof(BoardHead));
				
				if(queryDataInfo.baseBroadProtocal.dataType != 2){
					pidForwardSettingInfoResponse.baseBroadProtocal.dataType = PID_FORWARD_SETTING_INFO_RESPONSE;
					pidForwardSettingInfoResponse.baseBroadProtocal.dataLength = sizeof(pidForwardSettingInfoResponse) - sizeof(BaseBroadProtocol);
				}else{
					pidForwardSettingInfoResponse.baseBroadProtocal.dataType = PID_FORWARD_SETTING_INFO_RESPONSE;
				}
				//pidForwardSettingInfoResponse.ipForwardFlag = 15;
				//pidForwardSettingInfoResponse.ipForwardControl = 16;
				//pidForwardSettingInfoResponse.ipForwardLength = 17;
				pidForwardSettingInfoResponse.b1 = 0x1;
				/*pidForwardSettingInfoResponse.protocol = 18;
				pidForwardSettingInfoResponse.reserve = 19;
				this.protocol = ((byte)(b & 0x40));
				this.reserve = ((byte)(b & 0x7F));
				*/
				pidForwardSettingInfoResponse.b2 = 0x0;
				//pidForwardSettingInfoResponse.targetIpV6 = [0];
				pidForwardSettingInfoResponse.targetIpV4[0]= 172; 
				pidForwardSettingInfoResponse.targetIpV4[1]= 17;
				pidForwardSettingInfoResponse.targetIpV4[2]= 3;
				pidForwardSettingInfoResponse.targetIpV4[3]= 233;
				pidForwardSettingInfoResponse.targetPort = 30;
				pidForwardSettingInfoResponse.seq = 31;
				//pidForwardSettingInfoResponse.pid = 32;
				pidForwardSettingInfoResponse.forwardProgramId = 33;
				pidForwardSettingInfoResponse.crc = 0;
				
				send_data_len = sizeof(PIDForwardSettingInfoResponse);
				memcpy(buffer, &pidForwardSettingInfoResponse, send_data_len);
				recvServerAddr.sin_port = htons(UDP_SEND_PORT1);
				send_len = sendto(welcomeSocket, buffer, send_data_len,0, (struct sockaddr*)&recvServerAddr, sizeof(recvServerAddr));
				break;
			case DVBT_DATA_INFO_RESPONSE: //109 
				printf("%s %d %d", __func__, __LINE__, DVBT_DATA_INFO_RESPONSE);
				break;
			case STREAM_STRUCT_INFO_RESPONSE: //113
				printf("%s %d %d", __func__, __LINE__, STREAM_STRUCT_INFO_RESPONSE);
				struct StreamStructInfoResponse streamStructInfoResponse;
				memset(&streamStructInfoResponse, 0x0, sizeof(StreamStructInfoResponse));
				memcpy(&(streamStructInfoResponse.baseBroadProtocal.boardHead), &queryDataInfo.baseBroadProtocal.boardHead, sizeof(BoardHead));
				
				if(queryDataInfo.baseBroadProtocal.dataType != 2){
					streamStructInfoResponse.baseBroadProtocal.dataType = STREAM_STRUCT_INFO_RESPONSE;
					streamStructInfoResponse.baseBroadProtocal.dataLength = sizeof(streamStructInfoResponse) - sizeof(BaseBroadProtocol);
				}else{
					streamStructInfoResponse.baseBroadProtocal.dataType = STREAM_STRUCT_INFO_RESPONSE;
				}
				streamStructInfoResponse.totalInterface = 1;
				streamStructInfoResponse.seq = 200;
				streamStructInfoResponse.length = 1;
				streamStructInfoResponse.head[0] = NULL;
				streamStructInfoResponse.head[1] = NULL;
				streamStructInfoResponse.head[2] = NULL;
				streamStructInfoResponse.head[3] = NULL;
				streamStructInfoResponse.head[4] = NULL;
				streamStructInfoResponse.head[5] = NULL;
				streamStructInfoResponse.streamBitrate = 3;
				streamStructInfoResponse.nItemNum[0] = 0;
				streamStructInfoResponse.nItemNum[1] = 0;
				streamStructInfoResponse.nItemNum[2] = 0;
				streamStructInfoResponse.nItemNum[3] = 1;
				streamStructInfoResponse.programPid = 72;
				streamStructInfoResponse.PMTPid = 25;
				streamStructInfoResponse.PCRPid = 1;
				streamStructInfoResponse.bitrate = 100;
				streamStructInfoResponse.isCA = 0;
				streamStructInfoResponse.nProgramNameLength = 8;
				streamStructInfoResponse.programName[PROGRAM_NAME_LENGTH*2] = "cctv";
				streamStructInfoResponse.nProviderNameLength = 4;
				streamStructInfoResponse.nProviderName[PROVIDER_NAME_LENGTH] = "btv23";
				streamStructInfoResponse.nSubItemNum = 0;
				streamStructInfoResponse.crc = 0;
				
				send_data_len = sizeof(StreamStructInfoResponse);
				memcpy(buffer, &streamStructInfoResponse, send_data_len);
				recvServerAddr.sin_port = htons(UDP_SEND_PORT1);
				send_len = sendto(welcomeSocket, buffer, send_data_len,0, (struct sockaddr*)&recvServerAddr, sizeof(recvServerAddr));
				break;
				
			case STREAM_STATUS_INFO_RESPONSE: //116
				printf("%s %d %d", __func__, __LINE__, STREAM_STATUS_INFO_RESPONSE);
				break;
			case DVBS2_SETTING_INFO_RESPONSE: //119
				printf("%s %d %d", __func__, __LINE__, DVBS2_SETTING_INFO_RESPONSE);
				break;
			case DVBS2DATA_INFO_RESPONSE: //120
				printf("%s %d %d", __func__, __LINE__, DVBS2DATA_INFO_RESPONSE);
				break;
			case DTMB_SETTING_INFO_RESPONSE: //123
				printf("%s %d %d", __func__, __LINE__, DTMB_SETTING_INFO_RESPONSE);
				struct DTMBSettingInfoResponse dtmbSettingInfoResponse;
				memset(&dtmbSettingInfoResponse, 0x0, sizeof(DTMBSettingInfoResponse));
				memcpy(&(dtmbSettingInfoResponse.baseBroadProtocal.boardHead), &queryDataInfo.baseBroadProtocal.boardHead, sizeof(BoardHead));
				if(queryDataInfo.baseBroadProtocal.dataType != 2){
					dtmbSettingInfoResponse.baseBroadProtocal.dataType = DTMB_SETTING_INFO_RESPONSE;
					dtmbSettingInfoResponse.baseBroadProtocal.dataLength = sizeof(dtmbSettingInfoResponse) - sizeof(BaseBroadProtocol);
				}else{
					dtmbSettingInfoResponse.baseBroadProtocal.dataType = DTMB_SETTING_INFO_RESPONSE;
				}
				dtmbSettingInfoResponse.totalInterface = 1;
				dtmbSettingInfoResponse.seq = 120;
				dtmbSettingInfoResponse.freq = 234;
				dtmbSettingInfoResponse.mobileMode = 0;
				dtmbSettingInfoResponse.totalParams = 1;
				dtmbSettingInfoResponse.totalPrograms = 1;//与下面数量对应
				dtmbSettingInfoResponse.program = 'A';//1, 2, 3 4;
				dtmbSettingInfoResponse.crc = 0;
				send_data_len = sizeof(DTMBSettingInfoResponse);
				memcpy(buffer, &dtmbSettingInfoResponse, send_data_len);
				recvServerAddr.sin_port = htons(UDP_SEND_PORT1);
				send_len = sendto(welcomeSocket, buffer, send_data_len,0, (struct sockaddr*)&recvServerAddr, sizeof(recvServerAddr));
				break;
			case DTMB_DATA_INFO_RESPONSE: //124
				printf("%s %d %d", __func__, __LINE__, DTMB_DATA_INFO_RESPONSE);
				struct DTMBDataInfoResponse dtmbDataInfoResponse;
				memset(&dtmbDataInfoResponse, 0x0, sizeof(DTMBDataInfoResponse));
				memcpy(&(dtmbDataInfoResponse.baseBroadProtocal.boardHead), &queryDataInfo.baseBroadProtocal.boardHead, sizeof(BoardHead));
				if(queryDataInfo.baseBroadProtocal.dataType != 2){
					dtmbDataInfoResponse.baseBroadProtocal.dataType = DTMB_DATA_INFO_RESPONSE;
					dtmbDataInfoResponse.baseBroadProtocal.dataLength = sizeof(dtmbDataInfoResponse) - sizeof(BaseBroadProtocol);
				}else{
					dtmbDataInfoResponse.baseBroadProtocal.dataType = DTMB_DATA_INFO_RESPONSE;
				}
				dtmbDataInfoResponse.totalInterface = 1;
				dtmbDataInfoResponse.seq = 120;		
				dtmbDataInfoResponse.lock = 1;
				dtmbDataInfoResponse.freOffset = 20;
				dtmbDataInfoResponse.carrierOffset = 30;
				dtmbDataInfoResponse.tmp = 40;
				dtmbDataInfoResponse.rxlev = 50;
				dtmbDataInfoResponse.snr = 60;
				dtmbDataInfoResponse.packetBler = 80;
				dtmbDataInfoResponse.tmp1 = 100;
				dtmbDataInfoResponse.fecBitrate = 90;
				dtmbDataInfoResponse.polarization = 0;
				dtmbDataInfoResponse.constellationLength = 1;
				dtmbDataInfoResponse.constellationX = 3;
				dtmbDataInfoResponse.constellationY = 4;
				dtmbDataInfoResponse.otherLength = 0;
				dtmbDataInfoResponse.crc = 0;
				if(queryDataInfo.baseBroadProtocal.dataType != 2){
					send_data_len = sizeof(DTMBDataInfoResponse);// - sizeof(dtmbDataInfoResponse.baseBroadProtocal.dataLength);
					printf("send_data_len=%d sizeof(DTMBDataInfoResponse)=%d sizeof(DTMBDataInfoResponse.baseBroadProtocal.dataLength)=%d   sizeof(DTMBDataInfoResponse)-sizeof(BaseBroadProtocol)=%d\n", 
					send_data_len, sizeof(DTMBDataInfoResponse),sizeof(dtmbDataInfoResponse.baseBroadProtocal.dataLength),sizeof(DTMBDataInfoResponse)-sizeof(BaseBroadProtocol));
				}else{
					send_data_len = sizeof(DTMBDataInfoResponse);
				}
				memcpy(buffer, &dtmbDataInfoResponse, send_data_len);
				recvServerAddr.sin_port = htons(UDP_SEND_PORT1);
				send_len = sendto(welcomeSocket, buffer, send_data_len,0, (struct sockaddr*)&recvServerAddr, sizeof(recvServerAddr));
				printf("dtmbDataInfoResponse zxh124 send_data_len=%d\n", send_data_len);
				break;				
			default:
				break;
		}
//		memset(&baseBroadProtocol, 0x0, sizeof(BaseBroadProtocol));
	//	baseBroadProtocol.boardHead.ip[0] = 172;
		//baseBroadProtocol.boardHead.ip[1] = 17;
		//baseBroadProtocol.boardHead.ip[2] = 1;
		//baseBroadProtocol.boardHead.ip[3] = 72;
		
  		//send_len = sendto(welcomeSocket, sendData, strlen(sendData),0, (struct sockaddr*)&clientAddr, addr_size);
		//send_len = sendto(welcomeSocket, &baseBroadProtocol, sizeof(baseBroadProtocol),0, (struct sockaddr*)&clientAddr, addr_size);
		if(send_len != 0){
			//printf("send_len =%d recv_len =%d buffer=0x%x broadHead.ip=%s port=%d\n", send_len, recv_len, buffer[0], boardHead.ip,UDP_SEND_PORT1);
		}
	}
  }
  return 0;
}
