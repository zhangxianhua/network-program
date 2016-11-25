#ifndef _BROAD_HEAD_H_
#define _BROAD_HEAD_H_

#define HOST_NAME_LENGTH 1
#define ITEM_COUNT 4
#define PROGRAM_NAME_LENGTH 4
#define PROVIDER_NAME_LENGTH 4
typedef unsigned char byte;

enum RESPONSE_DATA_TYPE{
	DEVICE_VERSION_INFO_RESPONSE = 2, //DeviceVersionInfoResponse ->2;		
	DEVICE_NET_INFO_IPV4_RESPONSE = 3,//DeviceNetInfoIPV4Response ->3	
	QUERY_DATA_INFO = 4,//QueryDataInfo ->4
	STORAGE_INFO_RESPONSE = 8,//StorageInfoResponse ->8
	BASE_BROAD_PROTOCOL = 9,//BaseBroadProtocol ->9
	CPU_STORAGE_INFO_RESPONSE = 17,//CpuStorageInfoResponse 17 ->0x11 CpuStorageInfoResponse
	DVBT_SETTING_INFO_RESPONSE = 103, //DVBTSettingInfoResponse 103 ->0x67
	PID_FORWARD_SETTING_INFO_RESPONSE = 105,//PIDForwardSettingInfoResponse 105 ->0x69
	DVBT_DATA_INFO_RESPONSE = 109,//DVBTDataInfoResponse 109 ->0x6D
	STREAM_STRUCT_INFO_RESPONSE = 113,//StreamStructInfoResponse 113 ->0x71
	STREAM_STATUS_INFO_RESPONSE = 116,//StreamStatusInfoResponse 116 ->0x74
	DVBS2_SETTING_INFO_RESPONSE = 119,//DVBS2SettingInfoResponse 119 ->0x77
	DVBS2DATA_INFO_RESPONSE = 120,//DVBS2DataInfoResponse 120 ->0x78
	DTMB_SETTING_INFO_RESPONSE = 123,//DTMBSettingInfoResponse 123 ->0x7B
	DTMB_DATA_INFO_RESPONSE = 124//DTMBDataInfoResponse 124 ->0x7C queryFrequencyStatus
};//ResponseDataType;
typedef struct BoardHead
{//4+1+1+1+1+4=12  
	byte ip[4];
	byte flags;
	byte b;
	byte interfaceCount;
	byte reserve3;
	byte license[4];
#if 0	
	byte ipv6[16];//if this.ipv6Flag == 1
	
	//if this.extendFlag == 1
	short contentLength;
	short packetId;
	byte totalPacket;
	byte currentPacket;

#endif	
}BoardHead;

//9
struct BaseBroadProtocol{
	struct BoardHead boardHead;
	short dataType;
	short dataLength; //if dataType != 2 short dataLength else =null
}BaseBroadProtocol;

//4
struct QueryDataInfo{
	struct BaseBroadProtocol baseBroadProtocal;
	short requestDataType;
	byte paramLength;
	//if paramLength > 0 this.params
	//int params[paramLength];
}QueryDataInfo;

//3
struct DeviceNetInfoIPV4Response{
	struct BaseBroadProtocol baseBroadProtocal;
	byte hostNameLength;
	byte hostNames[HOST_NAME_LENGTH];//hostNames[hostNameLength]
	byte mac[6];
	byte ip[4];
	byte mask[4];
	byte netGate[4];
	int ttl;
}DeviceNetInfoIPV4Response;

//17
struct CpuStorageInfoResponse{
	struct BaseBroadProtocol baseBroadProtocal;
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
}CpuStorageInfoResponse;

//105
struct PIDForwardSettingInfoResponse{
	struct BaseBroadProtocol baseBroadProtocal;
    /*
	byte ipForwardFlag;
    byte ipForwardControl;
	byte ipForwardLength;
	this.ipForwardFlag = ((byte)(b >> 6 & 0x3));
    this.ipForwardControl = ((byte)(b >> 5 & 0x1));
    this.ipForwardLength = ((byte)(b & 0x1F));
	*/
	byte b1;
	/*
	byte protocol;
	byte reserve;
	*/
	byte b2;
	//byte targetIpV6[40];
	byte targetIpV4[4];
	short targetPort;
	byte seq;
	short forwardProgramId;
	//short pid;
	//short forwardProgramId;
	int crc;
}PIDForwardSettingInfoResponse;

//109
struct DVBTDataInfoResponse{
	struct BaseBroadProtocol baseBroadProtocal;
	byte totalInterface;	
	byte seq;
	byte lock;
	byte carrierOffset;
	short tmp;//clockOffect reserve2
	byte rxlev;
	byte snr;
	int packetBler;
	int viterbiBler;
	byte rfGain;
    byte basebandGain;
    byte midfreGain;
	byte constellationLength;
	byte constellationX;
	byte constellationY;
	int crc;
}DVBTDataInfoResponse;

//113
struct StreamStructInfoResponse{
	struct BaseBroadProtocol baseBroadProtocal;
	byte totalInterface;
	byte seq;
	short length;
	byte head[6];
	int streamBitrate;
	short nItemNum[ITEM_COUNT];
	short programPid;
	short PMTPid;
	short PCRPid;
	int bitrate;
	byte isCA;
	byte nProgramNameLength;
	byte programName[PROGRAM_NAME_LENGTH];
	byte nProviderNameLength;
	byte nProviderName[PROVIDER_NAME_LENGTH];
	byte nSubItemNum;
	//short elementPid;
	int crc;
}StreamStructInfoResponse;

//124
struct DTMBDataInfoResponse{
	struct BaseBroadProtocol baseBroadProtocal;
	byte totalInterface;
	byte seq;
	byte lock;
	byte freOffset;
	short carrierOffset;
	short tmp;//reserve2 symbolrateOffset
	byte rxlev;
	byte snr;
	short packetBler;
	short tmp1;//carrier freqFlip frameMode crossDepth
	byte fecBitrate;
	byte polarization;
	byte constellationLength;//if > 0 byte constellationX; byte constellationX;
	byte constellationX;
	byte constellationY;
	byte otherLength;//int freq; int mer; int evm;
	int crc;
}DTMBDataInfoResponse;

//123
struct DTMBSettingInfoResponse{
	struct BaseBroadProtocol baseBroadProtocal;
	byte totalInterface;
	byte seq;
	short freq;
	byte mobileMode;
	byte totalParams;
	short totalPrograms;
	short program;
	int crc;
}DTMBSettingInfoResponse;


#endif // _BROAD_HEAD_H_
