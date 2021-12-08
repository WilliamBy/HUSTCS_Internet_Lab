#include "stdafx.h"
#include "Global.h"
#include "GBNSender.h"


GBNSender::GBNSender():nextSeqnum(0),waitingState(false),base(0)
{
}


GBNSender::~GBNSender()
{
}



bool GBNSender::getWaitingState() {
	return waitingState;
}




bool GBNSender::send(const Message &message) {
	if (this->waitingState) { //���ͷ����ڵȴ�ȷ��״̬
		return false;
	}

	this->packet.acknum = -1; //���Ը��ֶ�
	this->packet.seqnum = this->nextSeqnum;
	this->packet.checksum = 0;
	memcpy(this->packet.payload, message.data, sizeof(message.data));
	this->packet.checksum = pUtils->calculateCheckSum(this->packet);
	if (nextSeqnum == base) {
		pns->startTimer(SENDER, Configuration::TIME_OUT, this->packet.seqnum);			//�������ͷ���ʱ��
	}
	pns->sendToNetworkLayer(RECEIVER, this->packet);								//����ģ�����绷����sendToNetworkLayer��ͨ������㷢�͵��Է�
	pUtils->printPacket("���ͷ����ͱ���", this->packet);

	sndpkt[this->packet.seqnum] = packet;	//���渱��

	this->nextSeqnum = (this->nextSeqnum + 1) % MOD;	//nextSeqnum����
	if ((this->base + this->cwSize) % MOD == this->nextSeqnum) {
		this->waitingState = true;	//��������������ȴ�ȷ��״̬
	}
	return true;
}

void GBNSender::receive(const Packet &ackPkt) {
	//���У����Ƿ���ȷ
	int checkSum = pUtils->calculateCheckSum(ackPkt);

	//���У�����ȷ��ȷ�Ϻ��ڴ�ȷ�������ڣ��򻬶����ڣ�����ʲôҲ����
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
		pns->stopTimer(SENDER, this->base);	//�رռ�ʱ��
		string str = this->winToStr();		//���������ַ���
		str.append(" => ");
		this->base = (ackPkt.acknum + 1) % MOD;			//���ݽ��շ��ۼ�ȷ��ԭ�򣬷��ʹ�����ackPkt.acknum֮ǰ�����ݰ����ѱ�ȷ��
		if (this->base != this->nextSeqnum) {
			pns->startTimer(SENDER, Configuration::TIME_OUT, this->base);	//����������δ��ȷ�ϵ����ݰ���������ʱ��
			this->waitingState = false;	//�������ں�ؽ�����δ���͵İ�
		}
		str.append(this->winToStr());	//��ӡ��������
		cout << str;
		this->ofs << str;
	}
	else {
		pUtils->printPacket("���ͷ�δ��ȷ�յ�ȷ�ϣ����ʹ���", ackPkt);
	}
}

void GBNSender::timeoutHandler(int seqNum) {
	//Ψһһ����ʱ��,���迼��seqNum
	pUtils->printPacket("���ͷ���ʱ��ʱ�䵽���ط��ϴη��͵ı���", this->packet);
	pns->stopTimer(SENDER,seqNum);										//���ȹرն�ʱ��
	for (int i = base; i != nextSeqnum; i = (i + 1) % MOD) {
		pns->sendToNetworkLayer(RECEIVER, sndpkt[i]);	//����n�������·������ݰ�
	}
	pns->startTimer(SENDER, Configuration::TIME_OUT, seqNum);			//�����������ͷ���ʱ��
}

string GBNSender::winToStr() {
	stringstream ss;
	ss << "[ ";
	for (int i = this->base; i != this->nextSeqnum; i = (i + 1) % MOD) {
		ss << i << " ";
	}
	ss << "| ";
	int border = (this->base + this->cwSize) % MOD;	//�����ҿ�����
	for (int i = this->nextSeqnum; i != border; i = (i + 1) % MOD) {
		ss << i << " ";
	}
	ss << "]" << endl;
	return ss.str();
}
