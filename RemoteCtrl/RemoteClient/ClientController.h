#pragma once
#include "ClientSocket.h"
#include "CWatchDialog.h"
#include "RemoteClientDlg.h"
#include "StatusDlg.h"
#include "resource.h"
#include <map>
					    
#define WM_SEND_PACK    (WM_USER + 1)//���Ͱ�����
#define WM_SEND_DATA    (WM_USER + 2)//��������
#define WM_SHOW_STATUS  (WM_USER + 3)//չʾ״̬
#define WM_SHOW_WATCH   (WM_USER + 4)//Զ���ٽ�
#define WM_SEND_MESSAGE (WM_USER+0x1000)//�Զ�����Ϣ����

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
protected:
	CClientController();
	~CClientController();
	void threadFunc();
	static unsigned __stdcall threadEntry(void* arg);
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
	unsigned m_nThreadID;
	static CClientController* m_instance;
	class CNewAndDel;
	static CNewAndDel m_newdel;
};

