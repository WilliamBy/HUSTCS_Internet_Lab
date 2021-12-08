#include "stdafx.h"
#include "Global.h"
#include "SRSender.h"


SRSender::SRSender():nextSeqnum(0),waitingState(false),base(0)
{
	for (int i = 0; i < 8; i++) {
		this->isAcked[i] = false;	//��ʼ������ʱ���ݰ�ȫ��δȷ��
	}
}


SRSender::~SRSender()
{
	ofs.flush();
	ofs.close();
}



bool SRSender::getWaitingState() {
	return waitingState;
}




bool SRSender::send(const Message &message) {
	if (this->waitingState) { //���ͷ����ڵȴ�ȷ��״̬
		return false;
	}

	this->packet.acknum = -1; //���Ը��ֶ�
	this->packet.seqnum = this->nextSeqnum;
	this->packet.checksum = 0;
	memcpy(this->packet.payload, message.data, sizeof(message.data));
	this->packet.checksum = pUtils->calculateCheckSum(this->packet);
	pns->startTimer(SENDER, Configuration::TIME_OUT, this->packet.seqnum);			//�������ͷ���ʱ��
	pns->sendToNetworkLayer(RECEIVER, this->packet);								//����ģ�����绷����sendToNetworkLayer��ͨ������㷢�͵��Է�
	pUtils->printPacket("���ͷ����ͱ���", this->packet);

	sndpkt[this->packet.seqnum] = packet;	//���渱��

	this->nextSeqnum = (this->nextSeqnum + 1) % MOD;		//nextSeqnum����
	if ((this->base + this->cwSize) % MOD == this->nextSeqnum) {
		this->waitingState = true;	//��������������ȴ�ȷ��״̬
	}
	return true;
}

void SRSender::receive(const Packet &ackPkt) {
	//���У����Ƿ���ȷ
	int checkSum = pUtils->calculateCheckSum(ackPkt);

	//���У�����ȷ��ȷ�Ϻ��ڴ�ȷ�������ڣ���ֹͣ��Ӧ��ʱ��������ʲôҲ����
	if (checkSum == ackPkt.checksum) {
		int flag = 0;	//Ϊ0��ʾȷ�ϺŲ��ڴ�����
		for (int i = this->base; i != this->nextSeqnum; i = (i + 1) % MOD) {
			if (i == ackPkt.acknum) {
				flag = 1;
				break;
			}
		}
		if (flag == 0) {
			pUtils->printPacket("���ͷ�δ��ȷ�յ�ȷ�ϣ�ȷ�ϺŲ��ڴ�ȷ��������", ackPkt);
			return;	//ȷ�ϺŲ��ڴ�ȷ��������
		}
		pUtils->printPacket("���ͷ���ȷ�յ�ȷ��", ackPkt);
		this->isAcked[ackPkt.acknum] = true;	//���ȷ��
		pns->stopTimer(SENDER, ackPkt.acknum);	//�رն�Ӧ��ʱ��

		string winStr = this->winToStr();	//�������ڵ��ַ�����ʾ
		winStr.append("=>");
		while (this->isAcked[this->base] == true)		//��������ֱ����һ��δ��ȷ�ϵ����
		{
			this->isAcked[this->base] = false;
			this->base = (this->base + 1) % MOD;
			this->waitingState = false;	//���ʹ����п���
		}
		winStr.append(this->winToStr());
		cout << winStr;	//��ӡ��������
		this->ofs << winStr;
	}
	else {
		pUtils->printPacket("���ͷ�δ��ȷ�յ�ȷ�ϣ����ʹ���", ackPkt);
	}
}

void SRSender::timeoutHandler(int seqNum) {
	pUtils->printPacket("���ͷ���ʱ��ʱ�䵽���ط��ϴη��͵ı���", this->packet);
	pns->stopTimer(SENDER,seqNum);										//���ȹرն�ʱ��
	pns->sendToNetworkLayer(RECEIVER, sndpkt[seqNum]);	//���·��ͳ�ʱ�����ݰ�
	pns->startTimer(SENDER, Configuration::TIME_OUT, seqNum);			//�����������ͷ���ʱ��
}

string SRSender::winToStr() {
	stringstream ss;
	ss << "[ ";
	int border = (this->base + this->cwSize) % MOD;	//�����ҿ�����
	for (int i = this->base; i != border; i = (i + 1) % MOD) {
		ss << i;
		if (this->isAcked[i]) ss << "a ";
		else ss << "n ";
	}
	ss << "]" << endl;
	return ss.str();
}
