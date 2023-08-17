#pragma once
#include "pch.h"
#include "framework.h"
#include <string>
#include <list>
#include "Packet.h"
typedef void(*SOCKET_CALLBACK)(void* arg, int status, std::list<CPacket>& lstPacket, CPacket& inPacket);

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
	SOCKET_CALLBACK m_callback;
	void* m_arg;
	SOCKET m_serv_sock;
	SOCKET m_client;
	static CServerSocket* m_instance;
	static CNewAndDel m_newdel;
public:
	bool initSocket(short port = 9527);
	int run(SOCKET_CALLBACK callback, void* arg, short port = 9527);
	bool acceptCli();
	int dealCommand();
	bool sendCom(const char* pData, int size);
	bool sendCom(CPacket& pData);
	static CServerSocket* getInstance();
	bool getFilePath(std::string& strPath);
	bool getMouseEvent(MOUSEEV& mouse);
	CPacket& GetPacket();
	void CloseClient();
};


extern CServerSocket* gpServer;



