#ifndef STOP_WAIT_RDT_SENDER_H
#define STOP_WAIT_RDT_SENDER_H
#include "RdtSender.h"
#include "stdafx.h"
class TCPSender :public RdtSender
{
private:
	int base;	//最早的发送了但未确认的序号
	int nextSeqnum;	// 下一个发送序号 
	const int cwSize = 4;	//窗口大小
	bool waitingState;				// 是否处于等待Ack的状态
	Packet packet;		//待发送的数据包
	Packet sndpkt[8];	//缓存的数据包(按序号存储)
	int reAckNum = 0;	//冗余ack数量
	ofstream ofs = ofstream("winlog.txt");

public:

	bool getWaitingState();
	bool send(const Message &message);						//发送应用层下来的Message，由NetworkServiceSimulator调用,如果发送方成功地将Message发送到网络层，返回true;如果因为发送方处于等待正确确认状态而拒绝发送Message，则返回false
	void receive(const Packet &ackPkt);						//接受确认Ack，将被NetworkServiceSimulator调用	
	void timeoutHandler(int seqNum);					//Timeout handler，将被NetworkServiceSimulator调用
	string winToStr();	//将滑动窗口字符串化

public:
	TCPSender();
	virtual ~TCPSender();
};

#endif

