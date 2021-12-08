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
	if (this->waitingState) { //发送方处于等待确认状态
		return false;
	}

	this->packet.acknum = -1; //忽略该字段
	this->packet.seqnum = this->nextSeqnum;
	this->packet.checksum = 0;
	memcpy(this->packet.payload, message.data, sizeof(message.data));
	this->packet.checksum = pUtils->calculateCheckSum(this->packet);
	if (nextSeqnum == base) {
		pns->startTimer(SENDER, Configuration::TIME_OUT, this->packet.seqnum);			//启动发送方定时器
	}
	pns->sendToNetworkLayer(RECEIVER, this->packet);								//调用模拟网络环境的sendToNetworkLayer，通过网络层发送到对方
	pUtils->printPacket("发送方发送报文", this->packet);

	sndpkt[this->packet.seqnum] = packet;	//缓存副本

	this->nextSeqnum = (this->nextSeqnum + 1) % MOD;	//nextSeqnum递增
	if ((this->base + this->cwSize) % MOD == this->nextSeqnum) {
		this->waitingState = true;	//窗口已满，进入等待确认状态
	}
	return true;
}

void GBNSender::receive(const Packet &ackPkt) {
	//检查校验和是否正确
	int checkSum = pUtils->calculateCheckSum(ackPkt);

	//如果校验和正确且确认号在待确认序列内，则滑动窗口，否则什么也不做
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
		pns->stopTimer(SENDER, this->base);	//关闭计时器
		string str = this->winToStr();		//滑动窗口字符串
		str.append(" => ");
		this->base = (ackPkt.acknum + 1) % MOD;			//根据接收方累计确认原则，发送窗口中ackPkt.acknum之前的数据包均已被确认
		if (this->base != this->nextSeqnum) {
			pns->startTimer(SENDER, Configuration::TIME_OUT, this->base);	//窗口内仍有未被确认的数据包，重启计时器
			this->waitingState = false;	//滑动窗口后必将出现未发送的包
		}
		str.append(this->winToStr());	//打印滑动窗口
		cout << str;
		this->ofs << str;
	}
	else {
		pUtils->printPacket("发送方未正确收到确认，检查和错误", ackPkt);
	}
}

void GBNSender::timeoutHandler(int seqNum) {
	//唯一一个定时器,无需考虑seqNum
	pUtils->printPacket("发送方定时器时间到，重发上次发送的报文", this->packet);
	pns->stopTimer(SENDER,seqNum);										//首先关闭定时器
	for (int i = base; i != nextSeqnum; i = (i + 1) % MOD) {
		pns->sendToNetworkLayer(RECEIVER, sndpkt[i]);	//回退n步，重新发送数据包
	}
	pns->startTimer(SENDER, Configuration::TIME_OUT, seqNum);			//重新启动发送方定时器
}

string GBNSender::winToStr() {
	stringstream ss;
	ss << "[ ";
	for (int i = this->base; i != this->nextSeqnum; i = (i + 1) % MOD) {
		ss << i << " ";
	}
	ss << "| ";
	int border = (this->base + this->cwSize) % MOD;	//窗口右开区间
	for (int i = this->nextSeqnum; i != border; i = (i + 1) % MOD) {
		ss << i << " ";
	}
	ss << "]" << endl;
	return ss.str();
}
