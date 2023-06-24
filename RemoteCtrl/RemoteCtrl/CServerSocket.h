#pragma once
#include "pch.h"
#include "framework.h"
class CServerSocket
{
private:
	//��Ա����
	CServerSocket& operator=(const CServerSocket& ss) {};
	CServerSocket(const CServerSocket& ss);
	CServerSocket();
	~CServerSocket();
	BOOL InitSockEnv();
	static void releaseInstance();
	//Ƕ����
	class CNewAndDel;
	//��Ա����
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

