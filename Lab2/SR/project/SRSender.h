#ifndef STOP_WAIT_RDT_SENDER_H
#define STOP_WAIT_RDT_SENDER_H
#include "RdtSender.h"
#include "stdafx.h"
class SRSender :public RdtSender
{
private:
	int base;	//����ķ����˵�δȷ�ϵ����
	int nextSeqnum;	// ��һ��������� 
	const int cwSize = 4;	//���ڴ�С
	bool waitingState;				// �Ƿ��ڵȴ�Ack��״̬
	Packet packet;		//�����͵����ݰ�
	Packet sndpkt[8];	//��������ݰ�(����Ŵ洢)
	bool isAcked[8];	//��־���ݰ��Ƿ�ȷ�ϵ�����
	ofstream ofs = ofstream("winlog.txt", ios::out);
public:

	bool getWaitingState();
	bool send(const Message &message);						//����Ӧ�ò�������Message����NetworkServiceSimulator����,������ͷ��ɹ��ؽ�Message���͵�����㣬����true;�����Ϊ���ͷ����ڵȴ���ȷȷ��״̬���ܾ�����Message���򷵻�false
	void receive(const Packet &ackPkt);						//����ȷ��Ack������NetworkServiceSimulator����	
	void timeoutHandler(int seqNum);					//Timeout handler������NetworkServiceSimulator����
	string winToStr();	//�����������ַ�����

public:
	SRSender();
	virtual ~SRSender();
};

#endif

