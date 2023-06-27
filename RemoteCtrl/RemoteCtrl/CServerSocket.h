#pragma once
#include "pch.h"
#include "framework.h"
#include <string>

class CPacket 
{
private:
public:
	WORD sHead;//包头 固定位：0xFEFF
	DWORD nLength;//包的长度（从控制命令开始，到和校验结束）
	WORD sCmd;//控制命令
	std::string strData;//包的数据
	WORD sSum;//和校验
	CPacket();
	CPacket& operator=(const CPacket& pack);
	~CPacket() {};
	CPacket(const BYTE* pData, size_t& nSize);
};

class CServerSocket
{
private:
	//成员方法
	CServerSocket& operator=(const CServerSocket& ss) {};
	CServerSocket(const CServerSocket& ss);
	CServerSocket();
	~CServerSocket();
	BOOL InitSockEnv();
	static void releaseInstance();
	//嵌套类
	class CNewAndDel;
	//成员属性
	CPacket m_packet;
	SOCKET m_serv_sock;
	SOCKET m_client;
	static CServerSocket* m_instance;
	static CNewAndDel m_newdel;
public:
	bool initSocket();
	bool acceptCli();
	int dealCommand();
	bool sendCom(const char* pData, int size);
	static CServerSocket* getInstance();
};

extern CServerSocket* gpServer;

