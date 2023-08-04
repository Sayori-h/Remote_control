#pragma once

#include "pch.h"
#include "framework.h"
#include <string>
#include <vector>
#include <list>
#include <map>

#define WM_SEND_PACK    (WM_USER + 1)//���Ͱ�����
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

typedef struct file_info {
	file_info() {
		isInvalid = 0;
		isDirectory = 0;
		hasNext = TRUE;
		memset(szFileName, 0, sizeof(szFileName));
	}
	BOOL isInvalid;//�Ƿ���Ч 1��Ч
	BOOL isDirectory;//�Ƿ�ΪĿ¼ 1��
	BOOL hasNext;//�Ƿ��к���  1��
	char szFileName[256];//�ļ���
}FILEINFO, * PFILEINFO;

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
	//std::string strOut;//������������
	HANDLE hEvent;//���㷢������ʱŪ��
	CPacket();
	CPacket& operator=(const CPacket& pack);
	~CPacket() {};
	CPacket(WORD sCmd, const BYTE* pData, size_t nSize, HANDLE hEvent);
	CPacket(const BYTE* pData, size_t& nSize);
	CPacket(const CPacket& pack);
	int pacSize();//�����ݵĴ�С
	const char* pacData(std::string& strOut) const;//�������ݵ�����
};
#pragma pack(pop)
#include <mutex>
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
	static void threadEntry(void* arg);
	void threadFunc();
	void threadFunc2();
	bool sendCom(const CPacket& pData);
	bool sendCom(const char* pData, int nSize);
	void SendPack(UINT nMsg, WPARAM wParam/*��������ֵ*/, LPARAM lParam/*�������ĳ���*/);
	//Ƕ����
	class CNewAndDel;
	//��Ա����
	CPacket m_packet;
	SOCKET m_sock;
	static CClientSocket* m_instance;
	static CNewAndDel m_newdel;
	std::vector<char>m_buffer;
	int m_nIP, m_nPort;//��ַ�Ͷ˿�
	//list��Ҫ��ȡ�Է�Ӧ��İ����Է�����Ӧ���������������������ܾ��ң���vector�����ʣ�
	std::map<HANDLE/*�ĸ�����*/, std::list<CPacket>&> m_mapAck;
	std::map<HANDLE, bool> m_mapAutoClosed;
	std::list<CPacket> m_lstSend;//Ҫ���͵�����
	bool m_bAutoClose;
	std::mutex m_lock;
	HANDLE m_hThread;
	typedef void(CClientSocket::* MSGFUNC)(UINT nMsg, WPARAM wParam, LPARAM lParam);
	std::map<UINT, MSGFUNC>m_mapFunc;
	
public:
	bool initSocket();
	int dealCommand();
	static CClientSocket* getInstance();
	bool SendPacket(const CPacket& pack, std::list<CPacket>& lstPacks, bool isAutoClose = true);
	bool getFilePath(std::string& strPath);
	bool getMouseEvent(MOUSEEV& mouse);
	CPacket& GetPacket();
	void CloseSocket();
	void UpdateAddress(int nIP, int nPort);
	void dump(BYTE* pData, size_t nSize);
};

extern CClientSocket* gpClient;


