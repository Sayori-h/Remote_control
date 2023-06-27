#pragma once
#include "pch.h"
#include "framework.h"
#include <string>

#pragma pack(push)
#pragma pack(1)
class CPacket 
{
private:
public:
	WORD sHead;//��ͷ �̶�λ��0xFEFF
	DWORD nLength;//���ĳ��ȣ��ӿ������ʼ������У�������
	WORD sCmd;//��������
	std::string strData;//��������
	WORD sSum;//��У��
	std::string strOut;//������������
	CPacket();
	CPacket& operator=(const CPacket& pack);
	~CPacket() {};
	CPacket(WORD sCmd,const BYTE* pData, size_t nSize);
	CPacket(const BYTE* pData, size_t& nSize);
	CPacket(const CPacket& pack);
	int pacSize();//�����ݵĴ�С
	const char* pacData();//�������ݵ�����
};
#pragma pack(pop)

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
	bool sendCom(CPacket& pData);
	static CServerSocket* getInstance();
	bool getFilePath(std::string& strPath);
};

extern CServerSocket* gpServer;

