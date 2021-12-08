#include "stdafx.h"
#include "Global.h"
#include "SRReceiver.h"


SRReceiver::SRReceiver():base(0)
{
	for (int i = 0; i < 8; i++) {
		this->isRcved[i] = false;
	}
	this->ackPkt.acknum = -1; //初始状态下，上次发送的确认包的确认序号为-1，使得当第一个接受的数据包出错时该确认报文的确认号为-1
	this->ackPkt.checksum = 0;
	this->ackPkt.seqnum = -1;	//忽略该字段
	for(int i = 0; i < Configuration::PAYLOAD_SIZE;i++){
		this->ackPkt.payload[i] = '.';
	}
	this->ackPkt.checksum = pUtils->calculateCheckSum(ackPkt);
}


SRReceiver::~SRReceiver()
{
}

void SRReceiver::receive(const Packet &packet) {
	//计算校验和
	int checkSum = pUtils->calculateCheckSum(packet);
	//如果校验和正确
	if (checkSum == packet.checksum) {
		int flag = 0;	//为0表示序号不在接受窗口内
		for (int i = this->base; i != (this->base + cwSize) % MOD; i = (i + 1) % MOD) {
			if (i == packet.seqnum) {
				flag = 1;
				break;
			}
		}
		if (flag == 0) {		//序列号是旧的，不接收但是要返回确认
			pUtils->printPacket("接收方未正确收到确认，序号不在待接受序列中", ackPkt);
		}
		else {		//序列号在待接收窗口内
			pUtils->printPacket("接收方正确收到发送方的报文", packet);
			this->isRcved[packet.seqnum] = true;
			rcvPkt[packet.seqnum] = packet;
			Message msg;	//取出Message，向上递交给应用层
			while (this->isRcved[this->base] == true)
			{
				this->isRcved[this->base] = false;
				memcpy(msg.data, this->rcvPkt[this->base].payload, sizeof(this->rcvPkt[this->base].payload));
				pns->delivertoAppLayer(RECEIVER, msg);
				pUtils->printPacket("接收方上传报文", rcvPkt[this->base]);
				this->base = (this->base + 1) % MOD;
			}
		}

		ackPkt.acknum = packet.seqnum;	//更新确认包
		ackPkt.checksum = pUtils->calculateCheckSum(ackPkt);	//重新计算检查和
		pUtils->printPacket("接收方发送确认报文", ackPkt);
		pns->sendToNetworkLayer(SENDER, ackPkt);	//调用模拟网络环境的sendToNetworkLayer，通过网络层发送确认报文到对方
		return;
	}
	else {
		pUtils->printPacket("接收方没有正确收到发送方的报文,数据校验错误", packet);
	}
}