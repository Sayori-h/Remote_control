#pragma once
#include "HSocket.h"
#include "HuxlThread.h"

class HNetwork
{

};
//由服务器通知用户，回调函数
typedef int (*AcceptFunc)(void* arg, HSOCKET& client);
typedef int (*RecvFunc)(void* arg, const HBuffer& buffer);
typedef int (*SendFunc)(void* arg,HSOCKET& client,int ret);
typedef int (*RecvFromFunc)(void* arg, HBuffer& buffer,HSockaddrIn& addr);
typedef int (*SendToFunc)(void* arg,HSockaddrIn& addr,int ret);
class HServerParameter {
public:
	HServerParameter(const std::string& ip="0.0.0.0", short port=9527,
		HTYPE type=HTYPE::HTypeTCP,
		AcceptFunc acceptf=NULL, RecvFunc recvf=NULL, SendFunc sendf=NULL,
		RecvFromFunc recvfromf = NULL,SendToFunc sendtof = NULL);
	//输入
	HServerParameter& operator<<(AcceptFunc func);
	HServerParameter& operator<<(RecvFunc func);
	HServerParameter& operator<<(SendFunc func);
	HServerParameter& operator<<(RecvFromFunc func);
	HServerParameter& operator<<(SendToFunc func);
	HServerParameter& operator<<(const std::string &ip);
	HServerParameter& operator<<(short port);
	HServerParameter& operator<<(HTYPE type);
	//输出
	HServerParameter& operator>>(AcceptFunc &func);
	HServerParameter& operator>>(RecvFunc &func);
	HServerParameter& operator>>(SendFunc &func);
	HServerParameter& operator>>(RecvFromFunc& func);
	HServerParameter& operator>>(SendToFunc& func);
	HServerParameter& operator>>(std::string& ip);
	HServerParameter& operator>>(short &port);
	HServerParameter& operator>>(HTYPE &type);
	//拷贝构造函数，=重载，用于同类型的赋值
	HServerParameter(const HServerParameter& param);
	HServerParameter& operator=(const HServerParameter& param);
	std::string m_ip;
	short m_port;
	HTYPE m_type;
	AcceptFunc m_accept;
	RecvFunc m_recv;
	SendFunc m_send;
	RecvFromFunc m_recvfrom;
	SendToFunc m_sendto;
};

class HServer:public ThreadFuncBase {
public:
	HServer(HServerParameter& param);
	~HServer();
	int Invoke(void*arg);
	int Send(HSOCKET& client, const HBuffer& buffer);
	int Sendto(HSockaddrIn& addr, HBuffer& buffer);
	int Stop();
private:
	int threadFunc();
	int threadTCPFunc();
	int threadUDPFunc();
	HServerParameter m_params;
	void* m_args;
	HuxlThread m_thread;
	HSOCKET m_sock;
	std::atomic<bool>m_stop;
};

