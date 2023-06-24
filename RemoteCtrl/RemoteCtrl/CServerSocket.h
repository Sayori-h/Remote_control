#pragma once
#include "pch.h"
#include "framework.h"
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

