#include "stdafx.h"
#include "Global.h"
#include "SRSender.h"


SRSender::SRSender():nextSeqnum(0),waitingState(false),base(0)
{
	for (int i = 0; i < 8; i++) {
		this->isAcked[i] = false;	//初始化，此时数据包全部未确认
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
	if (this->waitingState) { //发送方处于等待确认状态
		return false;
	}

	this->packet.acknum = -1; //忽略该字段
	this->packet.seqnum = this->nextSeqnum;
	this->packet.checksum = 0;
	memcpy(this->packet.payload, message.data, sizeof(message.data));
	this->packet.checksum = pUtils->calculateCheckSum(this->packet);
	pns->startTimer(SENDER, Configuration::TIME_OUT, this->packet.seqnum);			//启动发送方定时器
	pns->sendToNetworkLayer(RECEIVER, this->packet);								//调用模拟网络环境的sendToNetworkLayer，通过网络层发送到对方
	pUtils->printPacket("发送方发送报文", this->packet);

	sndpkt[this->packet.seqnum] = packet;	//缓存副本

	this->nextSeqnum = (this->nextSeqnum + 1) % MOD;		//nextSeqnum递增
	if ((this->base + this->cwSize) % MOD == this->nextSeqnum) {
		this->waitingState = true;	//窗口已满，进入等待确认状态
	}
	return true;
}

void SRSender::receive(const Packet &ackPkt) {
	//检查校验和是否正确
	int checkSum = pUtils->calculateCheckSum(ackPkt);

	//如果校验和正确且确认号在待确认序列内，则停止对应计时器，否则什么也不做
	if (checkSum == ackPkt.checksum) {
		int flag = 0;	//为0表示确认号不在窗口内
		for (int i = this->base; i != this->nextSeqnum; i = (i + 1) % MOD) {
			if (i == ackPkt.acknum) {
				flag = 1;
				break;
			}
		}
		if (flag == 0) {
			pUtils->printPacket("发送方未正确收到确认，确认号不在待确认序列中", ackPkt);
			return;	//确认号不在待确认序列中
		}
		pUtils->printPacket("发送方正确收到确认", ackPkt);
		this->isAcked[ackPkt.acknum] = true;	//标记确认
		pns->stopTimer(SENDER, ackPkt.acknum);	//关闭对应计时器

		string winStr = this->winToStr();	//滑动窗口的字符串表示
		winStr.append("=>");
		while (this->isAcked[this->base] == true)		//滑动窗口直至第一个未被确认的序号
		{
			this->isAcked[this->base] = false;
			this->base = (this->base + 1) % MOD;
			this->waitingState = false;	//发送窗口有空余
		}
		winStr.append(this->winToStr());
		cout << winStr;	//打印滑动窗口
		this->ofs << winStr;
	}
	else {
		pUtils->printPacket("发送方未正确收到确认，检查和错误", ackPkt);
	}
}

void SRSender::timeoutHandler(int seqNum) {
	pUtils->printPacket("发送方定时器时间到，重发上次发送的报文", this->packet);
	pns->stopTimer(SENDER,seqNum);										//首先关闭定时器
	pns->sendToNetworkLayer(RECEIVER, sndpkt[seqNum]);	//重新发送超时的数据包
	pns->startTimer(SENDER, Configuration::TIME_OUT, seqNum);			//重新启动发送方定时器
}

string SRSender::winToStr() {
	stringstream ss;
	ss << "[ ";
	int border = (this->base + this->cwSize) % MOD;	//窗口右开区间
	for (int i = this->base; i != border; i = (i + 1) % MOD) {
		ss << i;
		if (this->isAcked[i]) ss << "a ";
		else ss << "n ";
	}
	ss << "]" << endl;
	return ss.str();
}
