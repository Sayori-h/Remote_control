#pragma once

#include "pch.h"
#include "framework.h"
#include <string>

typedef struct MouseEvent {
	MouseEvent() {
		nAction = 0;
		nButton = -1;//Ĭ��ûЧ��
		ptXY.x = 0;
		ptXY.y = 0;
	}
	WORD nAction;//������ƶ���˫��
	WORD nButton;//������Ҽ����м�
	POINT ptXY;//����
}MOUSEEV, * PMOUSEEV;

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
	CPacket(WORD sCmd, const BYTE* pData, size_t nSize);
	CPacket(const BYTE* pData, size_t& nSize);
	CPacket(const CPacket& pack);
	int pacSize();//�����ݵĴ�С
	const char* pacData();//�������ݵ�����
};
#pragma pack(pop)
class CClientSocket
{
private:
	//��Ա����
	CClientSocket& operator=(const CClientSocket& ss) {};
	CClientSocket(const CClientSocket& ss);
	CClientSocket();
	~CClientSocket();
	BOOL InitSockEnv();
	static void releaseInstance();
	//Ƕ����
	class CNewAndDel;
	//��Ա����
	CPacket m_packet;
	SOCKET m_sock;
	SOCKET m_client;
	static CClientSocket* m_instance;
	static CNewAndDel m_newdel;
public:
	bool initSocket(const std::string& strIPAddress);
	bool acceptCli();
	int dealCommand();
	bool sendCom(const char* pData, int size);
	bool sendCom(CPacket& pData);
	static CClientSocket* getInstance();
	bool getFilePath(std::string& strPath);
	bool getMouseEvent(MOUSEEV& mouse);
};

extern CClientSocket* gpClient;


