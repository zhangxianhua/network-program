#ifndef MAIN_MGR_H_
#define MAIN_MGR_H_

typedef enum{
	UDP_MSG_START_LIVE_RECORDING = 0,
	UDP_MSG_STOP_LIVE_RECORDING,
	UDP_MSG_START_RESERVATION_RECORDING,
	UDP_MSG_START_STOP_RESERVATION_RECORDING,
	UDP_MSG_CHECK_LIVE_RECORDING,
	UDP_MSG_CHECK_RESERVATION_RECORDING
}UDP_MSG_TYPE ;

typedef struct UDP_MESSAGE_S{
    UDP_MSG_TYPE msgType;        
	char msgBody[300];
    //int msgLength;
} UDP_MESSAGE_S;

#endif /* MAIN_MGR_H_ */