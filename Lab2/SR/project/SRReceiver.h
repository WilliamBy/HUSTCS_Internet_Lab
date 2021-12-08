#ifndef STOP_WAIT_RDT_RECEIVER_H
#define STOP_WAIT_RDT_RECEIVER_H
#include "RdtReceiver.h"
class SRReceiver :public RdtReceiver
{
private:
	int base;	// ���ܴ��ڻ����
	const int cwSize = 4;	//���շ����ڴ�С
	bool isRcved[8];	//��־�ѽ��ܵ����
	Packet rcvPkt[8];	//�����ѽ���δ�ϴ������ݰ�
	Packet ackPkt;				//ȷ�ϱ���

public:
	SRReceiver();
	virtual ~SRReceiver();

public:
	
	void receive(const Packet &packet);	//���ձ��ģ�����NetworkService����
};

#endif

