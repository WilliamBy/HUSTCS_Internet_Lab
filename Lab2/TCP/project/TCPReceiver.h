#ifndef STOP_WAIT_RDT_RECEIVER_H
#define STOP_WAIT_RDT_RECEIVER_H
#include "RdtReceiver.h"
class TCPReceiver :public RdtReceiver
{
private:
	int expectedSequenceNum;	// �ڴ��յ�����һ���������
	Packet lastAckPkt;				//�ϴη��͵�ȷ�ϱ���

public:
	TCPReceiver();
	virtual ~TCPReceiver();

public:
	
	void receive(const Packet &packet);	//���ձ��ģ�����NetworkService����
};

#endif
