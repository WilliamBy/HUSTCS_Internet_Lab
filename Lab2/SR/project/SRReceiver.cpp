#include "stdafx.h"
#include "Global.h"
#include "SRReceiver.h"


SRReceiver::SRReceiver():base(0)
{
	for (int i = 0; i < 8; i++) {
		this->isRcved[i] = false;
	}
	this->ackPkt.acknum = -1; //��ʼ״̬�£��ϴη��͵�ȷ�ϰ���ȷ�����Ϊ-1��ʹ�õ���һ�����ܵ����ݰ�����ʱ��ȷ�ϱ��ĵ�ȷ�Ϻ�Ϊ-1
	this->ackPkt.checksum = 0;
	this->ackPkt.seqnum = -1;	//���Ը��ֶ�
	for(int i = 0; i < Configuration::PAYLOAD_SIZE;i++){
		this->ackPkt.payload[i] = '.';
	}
	this->ackPkt.checksum = pUtils->calculateCheckSum(ackPkt);
}


SRReceiver::~SRReceiver()
{
}

void SRReceiver::receive(const Packet &packet) {
	//����У���
	int checkSum = pUtils->calculateCheckSum(packet);
	//���У�����ȷ
	if (checkSum == packet.checksum) {
		int flag = 0;	//Ϊ0��ʾ��Ų��ڽ��ܴ�����
		for (int i = this->base; i != (this->base + cwSize) % MOD; i = (i + 1) % MOD) {
			if (i == packet.seqnum) {
				flag = 1;
				break;
			}
		}
		if (flag == 0) {		//���к��Ǿɵģ������յ���Ҫ����ȷ��
			pUtils->printPacket("���շ�δ��ȷ�յ�ȷ�ϣ���Ų��ڴ�����������", ackPkt);
		}
		else {		//���к��ڴ����մ�����
			pUtils->printPacket("���շ���ȷ�յ����ͷ��ı���", packet);
			this->isRcved[packet.seqnum] = true;
			rcvPkt[packet.seqnum] = packet;
			Message msg;	//ȡ��Message�����ϵݽ���Ӧ�ò�
			while (this->isRcved[this->base] == true)
			{
				this->isRcved[this->base] = false;
				memcpy(msg.data, this->rcvPkt[this->base].payload, sizeof(this->rcvPkt[this->base].payload));
				pns->delivertoAppLayer(RECEIVER, msg);
				pUtils->printPacket("���շ��ϴ�����", rcvPkt[this->base]);
				this->base = (this->base + 1) % MOD;
			}
		}

		ackPkt.acknum = packet.seqnum;	//����ȷ�ϰ�
		ackPkt.checksum = pUtils->calculateCheckSum(ackPkt);	//���¼������
		pUtils->printPacket("���շ�����ȷ�ϱ���", ackPkt);
		pns->sendToNetworkLayer(SENDER, ackPkt);	//����ģ�����绷����sendToNetworkLayer��ͨ������㷢��ȷ�ϱ��ĵ��Է�
		return;
	}
	else {
		pUtils->printPacket("���շ�û����ȷ�յ����ͷ��ı���,����У�����", packet);
	}
}