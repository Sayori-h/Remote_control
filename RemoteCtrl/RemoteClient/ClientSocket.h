#pragma once

#include "pch.h"
#include "framework.h"
#include <string>
#include <vector>
#include <list>
#include <map>

#define WM_SEND_PACK     (WM_USER + 1)//���Ͱ�����
#define WM_SEND_PACK_ACK (WM_USER + 2)//���Ͱ�����Ӧ��
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

enum {//ģʽ
	CSM_AUTOCLOSE = 1,//CSM Client Socket Mode �Զ��ر�ģʽ
};

typedef struct PacketData {
	std::string strData;//����
	UINT nMode;//ģʽ
	WPARAM wParam;
	PacketData(const char* pData, size_t nLen, UINT mode,WPARAM nParam=0) {
		strData.resize(nLen);
		memcpy((char*)strData.c_str(), pData, nLen);
		nMode = mode;
		wParam = nParam;
	}
	PacketData(const PacketData& data) {
		strData = data.strData;
		nMode = data.nMode;
		wParam = data.wParam;
	}
	PacketData& operator=(const PacketData& data) {
		if (this != &data) {
			strData = data.strData;
			nMode = data.nMode;
			wParam = data.wParam;
		}
		return *this;
	}
}PACKET_DATA;
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
	//HANDLE hEvent;//���㷢������ʱŪ��
	CPacket();
	CPacket& operator=(const CPacket& pack);
	~CPacket() {};
	CPacket(WORD sCmd, const BYTE* pData, size_t nSize/*HANDLE hEvent*/);
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

	/*Ĭ�Ϲ��캯������ʼ���ͻ����������ݣ���ʼ����Ա����Ϊ�գ���ʼ������ӳ��������¼��壬�����߳�threadFunc()
	�ڲ���Ҫʹ�õ�ϵͳ������WSAStartup ()��CreateEvent()��_beginthreadex()*/
	CClientSocket();

	~CClientSocket();

	//��ʼ�����绷��������1.1�汾�����
	BOOL InitSockEnv();

	static void releaseInstance();

	static unsigned __stdcall threadEntry(void* arg);
	//void threadFunc();
	
	//�̺߳�������ѭ����ȡ��Ϣ����Ŀ���ǵõ�SendPacket()��post�����Ĳ�������ͨ��ӳ�����ñ����SendPack()����
	void threadFunc2();

	//������Ϣ
	bool sendCom(const CPacket& pData);

	//������Ϣ
	bool sendCom(const char* pData, int nSize);

	/*�������ܣ���Ҫ���ڷ������ݵ�����ˣ�Ȼ����մӷ���˷��ص���Ϣ������SendMessage()����Ӧ�Ľ���ص�������
	//����1����Ϣ����
	//����2����Ϣw������ΪPacketData�Ľṹָ��(�����PacketData)
	//����3����Ϣl������Ϊ��Ӧ����Ĵ��ھ��*/
	void SendPack(UINT nMsg, WPARAM wParam, LPARAM lParam);
	//Ƕ����
	class CNewAndDel;
	//��Ա����
	CPacket m_packet;//���ڽ������ݰ�
	SOCKET m_sock;//�ͻ����׽���
	static CClientSocket* m_instance;
	static CNewAndDel m_newdel;
	std::vector<char>m_buffer;//��̬��������ݻ�����
	int m_nIP, m_nPort;//��ַ�Ͷ˿�
	//list��Ҫ��ȡ�Է�Ӧ��İ����Է�����Ӧ���������������������ܾ��ң���vector�����ʣ�
	std::map<HANDLE/*�ĸ�����*/, std::list<CPacket>&> m_mapAck;
	std::map<HANDLE, bool> m_mapAutoClosed;
	std::list<CPacket> m_lstSend;//Ҫ���͵�����
	bool m_bAutoClose;//�Զ��ر�sock��־
	std::mutex m_lock;//������
	HANDLE m_hThread;//�߳̾��
	typedef void(CClientSocket::* MSGFUNC)(UINT nMsg, WPARAM wParam, LPARAM lParam);
	std::map<UINT, MSGFUNC>m_mapFunc;//�ص�������
	UINT m_nThreadID;//���ûص��������߳�ID
	HANDLE m_eventInvoke;//���ڵȴ��߳�����
public:
	//��ʼ���ͻ���socket����÷�������ַ�Ͷ˿ں����ӵ�������
	bool initSocket();
	int dealCommand();//��ȡ�ͻ�������
	static CClientSocket* getInstance();
	//bool SendPacket(const CPacket& pack, std::list<CPacket>& lstPacks, bool isAutoClose = true);
	
	//�ú����ڲ�����PostThreadMessage()������֪ͨ�̺߳���threadFunc()��GetMessage()
	bool SendPacket(HWND hWnd, const CPacket& pack, bool isAutoClosed=true,WPARAM wParam=0);
	bool getFilePath(std::string& strPath);
	bool getMouseEvent(MOUSEEV& mouse);
	CPacket& GetPacket();//��ȡ���ݰ�
	void CloseSocket();//�ر�socket
	void UpdateAddress(int nIP, int nPort);//��������
	void dump(BYTE* pData, size_t nSize);
};

extern CClientSocket* gpClient;


