#include "stdafx.h"
#include "Global.h"
#include "TCPSender.h"


TCPSender::TCPSender():nextSeqnum(0),waitingState(false),base(0)
{
}


TCPSender::~TCPSender()
{
}



bool TCPSender::getWaitingState() {
	return waitingState;
}




bool TCPSender::send(const Message &message) {
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

void TCPSender::receive(const Packet &ackPkt) {
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
			if (ackPkt.acknum == (this->base - 1) % MOD) {
				//����ack
				if (++this->reAckNum >= 3) {
					this->reAckNum = 0;	//����ack����
					//�����ش�
					string str = "�����ش�";
					str.append("���յ�3������ack=").append(to_string(this->base - 1)).append("��\n");
					ofs << str;
					cout << str;
					pns->stopTimer(SENDER, this->base);
					for (int i = base; i != nextSeqnum; i = (i + 1) % MOD) {
						pns->sendToNetworkLayer(RECEIVER, sndpkt[i]);	//����n�������·������ݰ�
						pUtils->printPacket("���ͷ��ش�", sndpkt[i]);
					}
					if (this->base != this->nextSeqnum) {
						pns->startTimer(SENDER, Configuration::TIME_OUT, this->base);			//�����������ͷ���ʱ��
					}
				}
			}
			return;	//ȷ�ϺŲ��ڴ�ȷ��������
		}
		pUtils->printPacket("���ͷ���ȷ�յ�ȷ��", ackPkt);
		this->reAckNum = 0;	//����ack��Ŀ����
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

void TCPSender::timeoutHandler(int seqNum) {
	//Ψһһ����ʱ��,���迼��seqNum
	cout << "���ͷ���ʱ�������ش�n��" << endl;
	pns->stopTimer(SENDER,seqNum);										//���ȹرն�ʱ��
	for (int i = base; i != nextSeqnum; i = (i + 1) % MOD) {
		pns->sendToNetworkLayer(RECEIVER, sndpkt[i]);	//����n�������·������ݰ�
		pUtils->printPacket("���ͷ��ش�", sndpkt[i]);
	}
	pns->startTimer(SENDER, Configuration::TIME_OUT, seqNum);			//�����������ͷ���ʱ��
	this->reAckNum = 0;	//����ack����
}

string TCPSender::winToStr() {
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