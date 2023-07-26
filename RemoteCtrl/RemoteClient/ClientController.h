#pragma once
#include "ClientSocket.h"
#include "CWatchDialog.h"
#include "StatusDlg.h"
#include "resource.h"
#include "ClientSocket.h"
#include "RemoteClientDlg.h"
#include "CHuxlTool.h"
#include <map>
					    
#define WM_SEND_PACK    (WM_USER + 1)//���Ͱ�����
#define WM_SEND_DATA    (WM_USER + 2)//��������
#define WM_SHOW_STATUS  (WM_USER + 3)//չʾ״̬
#define WM_SHOW_WATCH   (WM_USER + 4)//Զ�̼���
#define WM_SEND_MESSAGE (WM_USER+0x1000)//�Զ�����Ϣ����

//ҵ���߼������̣�����ʱ���ܷ����ı�ģ���������
//ҵ���߼������̣�����ʱ���ܷ����ı�ģ���������
//ҵ���߼������̣�����ʱ���ܷ����ı�ģ���������

class CClientController
{
public:
	//��ȡȫ��Ψһ����
	static CClientController* getInstance();
	//��ʼ������
	int InitController();
	//����
	int Invoke(CWnd*&pMainWnd);
	//������Ϣ
	LRESULT SendMessage(MSG msg);
	//��������������ĵ�ַ
	void UpdateAddress(int nIP, int nPort);
	//���׽���ת��Ϊcontroller��һ���ţ�ն�Ͻ�������ݵ����
	int DealCommand();
	void CloseSocket();
	CPacket& GetPacket();
	bool SendPacket(const CPacket& pack);
	/*---------------------------------------
	1���鿴���̷���    ||   2���鿴ָ��Ŀ¼�µ��ļ�
	3�����ļ�	    ||   4�������ļ�
	5��������	    ||   6��������Ļ������
	7������			||   8������
	9��ɾ���ļ�		||   2001����������
	����ֵ��������ţ����<0�����*/
	int SendCommandPacket(int nCmd, bool bAutoClose = true, BYTE* pData = NULL, size_t nLength = 0);
	int GetImage(CImage& image);
	int DownFile(CString& strPath);
	void StartWatchScreen();
protected:
	CClientController();
	~CClientController();
	void threadWatchScreen();
	static void threadWatchScreen(void* arg);
	void threadFunc();
	//unsignedΪ�˻�ȡ�߳�ID
	static unsigned __stdcall threadEntry(void* arg);
	void threadDownFile();
	static void threadEntryOfDownFile(void* arg);
	static void releaseInstance();
	LRESULT OnSendPack(UINT nMSG, WPARAM wParam, LPARAM lParam);
	LRESULT OnSendData(UINT nMSG, WPARAM wParam, LPARAM lParam);
	LRESULT OnShowStatus(UINT nMSG, WPARAM wParam, LPARAM lParam);
	LRESULT OnShowWatcher(UINT nMSG, WPARAM wParam, LPARAM lParam);
private:
	typedef struct MsgInfo {
		MSG msg;
		LRESULT result;
		MsgInfo(MSG m) {
			result = 0;
			memcpy(&msg, &m, sizeof(MSG));
		}
		MsgInfo(const MsgInfo& m) {
			result = 0;
			memcpy(&msg, &m, sizeof(MSG));
		}
		MsgInfo& operator=(const MsgInfo& m) {
			if (this != &m) {
				result = m.result;
				memcpy(&msg, &m.msg, sizeof(MSG));
			}
			return *this;
		}
	}MSGINFO;
	typedef LRESULT(CClientController::* MSGFUNC)(UINT nMSG,
		WPARAM wParam, LPARAM lParam);
	static std::map<UINT, MSGFUNC>m_mapFunc;
	CWatchDialog m_watchDlg;
	CRemoteClientDlg m_remoteDlg;
	CStatusDlg m_statusDlg;
	HANDLE m_hThread;
	HANDLE m_hThreadDown;
	HANDLE m_hThreadWatch;
	bool m_isClosed;//�����Ƿ�ر�
	CString m_strRemote;//�����ļ���Զ��·��
	CString m_strLocal;//�����ļ��ı��ر���·��
	unsigned m_nThreadID;
	static CClientController* m_instance;
	class CNewAndDel;
	static CNewAndDel m_newdel;
};

