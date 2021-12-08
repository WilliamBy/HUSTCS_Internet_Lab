#ifndef STOP_WAIT_RDT_RECEIVER_H
#define STOP_WAIT_RDT_RECEIVER_H
#include "RdtReceiver.h"
class SRReceiver :public RdtReceiver
{
private:
	int base;	// 接受窗口基序号
	const int cwSize = 4;	//接收方窗口大小
	bool isRcved[8];	//标志已接受的序号
	Packet rcvPkt[8];	//缓存已接受未上传的数据包
	Packet ackPkt;				//确认报文

public:
	SRReceiver();
	virtual ~SRReceiver();

public:
	
	void receive(const Packet &packet);	//接收报文，将被NetworkService调用
};

#endif

