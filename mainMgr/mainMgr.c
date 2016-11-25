#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h> /*for errno*/
#include <fcntl.h>
#include <sys/types.h> /*for open close*/
#include <sys/stat.h>
#include <sys/ioctl.h> /*for ioctl*/
#include <sys/mman.h> /*for mmap*/
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>

#include <signal.h>
#include <sys/time.h>
#include <sys/mount.h>
#include <pthread.h>

#include <netdb.h>
#include "mainMgr.h"

#define MSG_MAX 304
#define UDP_RECV_MSG_PORT 9009
#define UDP_SEND_MSG_PORT 9009

pthread_t recvThreadId;
static int recv_sockfd = -1;
char encAddr[64];
struct sockaddr_in server_addr;

int SetupRecvSocket(char *server, int port) 
{
	struct hostent *host;
	struct sockaddr_in saddr;
	int sock; 
	printf("server = %s\n", server);	

	if ( (sock = socket (AF_INET, SOCK_DGRAM, 0) ) == -1)                 //get a socket fd
	{
		fprintf (stderr, "Socket Error:%s\a\n", strerror (errno) );
		return -1;
	}

	bzero (&server_addr, sizeof (server_addr) );//fill in structure with client info

	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = htonl (INADDR_ANY);
	server_addr.sin_port = htons(UDP_RECV_MSG_PORT);

	if (bind (sock,(struct sockaddr *) & server_addr,	sizeof (server_addr) ) < 0){ //bind a port not used
		fprintf (stderr, "[pvrFileList] Cannot bind socket\n");
		return -1;
	}

	return sock;
}
void receiveMsg()
{
	int length = 0;
	int zeroreads = 0;
	fd_set readfds, writefds, exceptfds;
	struct timeval timeout;
	struct sockaddr_in clientAddr;
	char Xmsg[MSG_MAX];
	socklen_t addr_size = sizeof(server_addr);
		
	FD_ZERO(&writefds);
	FD_ZERO(&exceptfds);

	printf("Wait for encoder to start before registering for stats\n");
	//sleep(1);
	if (recv_sockfd >= 0) {
		printf("enc_sockfd already opened... closing\n");
		close(recv_sockfd);
		recv_sockfd = -1;
	}
	if ( (recv_sockfd = SetupRecvSocket (encAddr, UDP_RECV_MSG_PORT) ) == -1 ) {
		printf ("error on network setup\n");
		return;
	}
	printf("Listen loop\n");
	while (1)
	{
		int i;
		UDP_MESSAGE_S msg;
		if (recv_sockfd < 0) {
			printf("ReceiveEncMsg, no socket\n");
			return;
		}

		FD_ZERO(&readfds);
		FD_SET(recv_sockfd, &readfds);
		// 3 second timeout on update messages.
		timeout.tv_sec = 3;
		timeout.tv_usec = 0;
		//printf("ReceiveEncMsg, calling select\n");
		i = select(recv_sockfd+1, &readfds, &writefds, &exceptfds, &timeout);
		if (!i) {
			//printf("Select times out... retry\n");
			continue;
		}

		//printf("ReceiveEncMsg, waiting for socket read\n");
		//length += recvfrom(recv_sockfd, &Xmsg[length], 300, 0, (struct sockaddr*)&clientAddr, &addr_size);
		length += recvfrom(recv_sockfd, &Xmsg[length], 304, 0, (struct sockaddr*)&clientAddr, &addr_size);
		//printf("ReceiveEncMsg, back from read\n");
		if (length < 0) {
			//close(enc_sockfd);
			printf("Error on enc_sockfd read... closing...\n");
			return;
		}
		if (length == 0) { 
			zeroreads++;
			if (zeroreads > 3) {
				printf("3 zero reads in a row on enc_sockfd read... closing...\n");
				//close(enc_sockfd);
				return;
			}
		} else
			zeroreads = 0;
		
		memcpy(&msg, Xmsg, sizeof(msg));
		printf("ReceiveEncMsg, read %d bytes, type=%d body=%s\n", length, msg.msgType, msg.msgBody);

#if 0
		if ( (length >= MSG_MAX - 1) && (Xmsg[length-1] != '\n') )
		{
			printf ("EncMsg overflow\n");      
			memset (Xmsg, 0, MSG_MAX);
			length = 0;
			continue;
		}
		if (Xmsg[length-1] != '\n') {
			printf("ReceiveEncMsg, no newline\n");
			continue;
		}
#endif
		Xmsg[length-1] = '\0';
#if 0
		for (int i = 0; encReceiverCmdArray[i].msgString; i++)
		{
			int cmdsz = strlen (encReceiverCmdArray[i].msgString);

			if (!strncmp (encReceiverCmdArray[i].msgString, Xmsg, cmdsz) )
			{
#if 0

				if ( (Xmsg[cmdsz] != CMD_SEP) && (Xmsg[cmdsz] != CMD_TERM) && (Xmsg[cmdsz] != 0) )
				continue;
#endif
				//printf("Calling dispatch func\n");
				(this->* (encReceiverCmdArray[i].msgDispatchFunc) ) (Xmsg + cmdsz + 1 );
				//printf("Back from dispatch func\n");

				break;
			}
		}           //end of for
#endif
		//printf("zero Xmsg, reset length\n");
		memset (Xmsg, 0, MSG_MAX);
		length = 0;
	}
}

void *receiveMsgThread(void *arg)                                                                                        
{
	//void *pArg = (void *)arg;
	receiveMsg();
	return (void *)0;
}

void startRecvMsgThread (void* args) 
{
	void* res;
	printf("Start Recv msg thread1\n");
	pthread_create(&recvThreadId, NULL, receiveMsgThread, args);  
	//pthread_join(recvThreadId, &res);
}

#include <ifaddrs.h>
#include <arpa/inet.h>
#define SERVER_IP "172.17.50.92"

struct ifaddrs* getLocalIP(void){
	struct ifaddrs * ifAddrStruct = NULL;
    void * tmpAddrPtr = NULL;

    getifaddrs(&ifAddrStruct);
	printf("111\n");
    while (ifAddrStruct!=NULL) {
        if (ifAddrStruct->ifa_addr->sa_family==AF_INET) { // check it is IP4
            // is a valid IP4 Address
            tmpAddrPtr=&((struct sockaddr_in *)ifAddrStruct->ifa_addr)->sin_addr;
            char addressBuffer[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, tmpAddrPtr, addressBuffer, INET_ADDRSTRLEN);
            printf("%s IP Address1 %s/n", ifAddrStruct->ifa_name, addressBuffer); 
        } else if (ifAddrStruct->ifa_addr->sa_family==AF_INET6) { // check it is IP6
            // is a valid IP6 Address
            tmpAddrPtr=&((struct sockaddr_in *)ifAddrStruct->ifa_addr)->sin_addr;
            char addressBuffer[INET6_ADDRSTRLEN];
            inet_ntop(AF_INET6, tmpAddrPtr, addressBuffer, INET6_ADDRSTRLEN);
            printf("%s IP Address2 %s/n", ifAddrStruct->ifa_name, addressBuffer); 
        } 
        ifAddrStruct=ifAddrStruct->ifa_next;
    }
	//printf("ifa_addr = %s\n", inet_ntoa(((struct sockaddr_in *)ifAddrStruct->ifa_addr)->sin_addr));
	return ifAddrStruct;
}

pthread_t sendThreadId;
void* sendMsgThread(void *arg){
	int s,len;  
    struct sockaddr_in addr;  
    socklen_t addr_len = sizeof(struct sockaddr_in);  
    char buffer[256];  

    if((s = socket(AF_INET,SOCK_DGRAM,0))<0){  
        perror("socket");  
        exit(1);  
    }  

    bzero(&addr,sizeof(addr));  
    addr.sin_family = AF_INET;  
    addr.sin_port = htons(UDP_SEND_MSG_PORT);  
    addr.sin_addr.s_addr = inet_addr(SERVER_IP);  
	UDP_MESSAGE_S msg; 
	int cnt = 0;
	int send_len = 0;
    while(1){  
        bzero(buffer,sizeof(buffer));  
		sprintf(buffer, "%s->%d", "hello", cnt);

		msg.msgType = cnt;
		strcpy(msg.msgBody, buffer);

        send_len = sendto(s,&msg, sizeof(msg), 0, (struct sockaddr *)&addr, addr_len);  
		printf("sendto2: %d %s send_len =%d\n", msg.msgType, msg.msgBody, send_len);  
		
		sleep(1);
        //len = recvfrom(s, buffer, sizeof(buffer), 0, (struct sockaddr *)&addr, &addr_len);  
        //printf("receive: %s",buffer);  
        cnt++;
    }  	
}
void startSendMsgThread (void* args) 
{
	void* res;
	printf("Start Send msg thread2\n");
	pthread_create (&sendThreadId, NULL, sendMsgThread, args);  
	pthread_join(sendThreadId, &res);
}
#include <net/if.h>

/* eth */
typedef enum hiEth_LinkStatus_E
{
    ETH_LINK_STATUS_OFF = 0,
    ETH_LINK_STATUS_ON,
    ETH_LINK_STATUS_MAX
} Eth_LinkStatus_E, *PTR_Eth_LinkStatus_E;
typedef enum hiEth_Port_E
{
    ETH_PORT_UP = 0,
    ETH_PORT_DOWN,
    ETH_PORT_MAX
} Eth_Port_E;
static int g_s32ETHFd[2] = {0, 0};

#define HI_SUCCESS          (0)
#define HI_FAILURE          (-1)

int HI_ETH_GetLinkStatus(Eth_Port_E ePort, PTR_Eth_LinkStatus_E ptrLinkStatus)
{
    int sockfd;
    struct ifreq ifr;

    if (NULL == ptrLinkStatus)
    {
        printf("null pointer\n");
        return -1;
    }

    if (ETH_PORT_MAX <= ePort)
    {
        printf("invalid parameter:%d\n", ePort);
        return -1;
    }

    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        printf("create socket failed\n");
        return -1;
    }

    bzero((char *)&ifr, sizeof(ifr));
    strcpy(ifr.ifr_name, "eth0");
    if (ioctl(sockfd, SIOCGIFFLAGS, (char *)&ifr) < 0)
    {
    	printf("ioctl socket failed\n");
        close(sockfd);
        return -1;
    }

    if ((ifr.ifr_flags & IFF_RUNNING) == 0)
    {
        *ptrLinkStatus = ETH_LINK_STATUS_OFF;
    }
    else
    {
        *ptrLinkStatus = ETH_LINK_STATUS_ON;
    }

    close(sockfd);
    return 0;
}
int HI_ETH_Open(Eth_Port_E ePort)
{
    struct ifreq ifr;
    int sockfd;

    if (ETH_PORT_MAX <= ePort)
    {
        printf("invalid parameter:%d\n", ePort);
        return -1;
    }

    g_s32ETHFd[ePort] = 1;

    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        printf("socket create fail!\n");
        return -1;
    }

    strcpy(ifr.ifr_name, "eth0");
    if((ioctl(sockfd,SIOCGIFFLAGS,(caddr_t)&ifr))==-1)
    {
        perror("SIOCGIFFLAGS fail :");
        close(sockfd);
        return -1;
    }

    ifr.ifr_flags |= IFF_UP;
    if((ioctl(sockfd,SIOCSIFFLAGS,(caddr_t)&ifr))==-1)
    {
        perror("SIOCSIFFLAGS fail :");
        close(sockfd);
        return -1;
    }

    close(sockfd);
    return 0;
}
int HI_ETH_IPAddressGet (Eth_Port_E ePort, char *ipAdd)
{
    int sockfd;
    struct ifreq ifr;
    struct sockaddr_in *s_in;

    if (NULL == ipAdd)

    {
        printf("null pointer\n");
        return -1;
    }

    if (ETH_PORT_MAX <= ePort)
    {
        printf("invalid parameter:%d\n", ePort);
        return -1;
    }

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0)
    {
        printf("socket create fail!\n");
        return HI_FAILURE;
    }

    memset(&ifr, 0, sizeof(ifr));
    strcpy(ifr.ifr_name, "eth0");
    if (ioctl(sockfd, SIOCGIFADDR, &ifr) < 0)
    {
        printf("ioctl fail!\n");
        close(sockfd);
        return HI_FAILURE;
    }

    s_in = (struct sockaddr_in *)(&ifr.ifr_addr);
    memcpy((void *)ipAdd, inet_ntoa(s_in->sin_addr), 15);

    close(sockfd);
    return HI_SUCCESS;
}
int getHostIpAddr(void){
	Eth_LinkStatus_E linkflag;
    char ipaddr[16];
    char ipmask[16];
    char getway[16];
    char dns[16];
    char mac[18];
    int ret;
    Eth_Port_E eth = ETH_PORT_UP;

	eth = ETH_PORT_UP;
	HI_ETH_GetLinkStatus(eth, &linkflag);
    if (linkflag == ETH_LINK_STATUS_OFF)
    {
        printf("%s Port Is not link\n", "eth0");
        return 0;
    }
    else
    {
        printf(" %s Port Is link\n", "eth0");
    }

	HI_ETH_Open(eth);
    memset(ipaddr, 0, sizeof(ipaddr));
    ret = HI_ETH_IPAddressGet(eth, ipaddr);
    if (ret != HI_SUCCESS)
    {
		printf("get ipaddr error!\n");
	}
	printf("ipaddr = %s\n", ipaddr);
	sprintf(encAddr, "%s", "172.17.50.92");	
}

int main(int argc, char* argv[])
{
	//getHostIpAddr();
	sprintf(encAddr, "%s", "172.17.50.92");	
	startRecvMsgThread(NULL);
	startSendMsgThread(NULL);
	//sendMsgThread(NULL);
	return 0;
}
