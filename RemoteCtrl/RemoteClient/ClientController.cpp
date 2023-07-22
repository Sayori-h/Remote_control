#include "pch.h"
#include "ClientController.h"

//静态成员需要单独声明，构造时候不会声明，所有类共用一个
std::map<UINT, CClientController::MSGFUNC>
CClientController::m_mapFunc;
CClientController* CClientController::m_instance = NULL;

class CClientController::CNewAndDel {
public:
	CNewAndDel() {
		CClientController::getInstance();
	}
	~CNewAndDel() {
		CClientController::releaseInstance();
	}
};

int CClientController::Invoke(CWnd*& pMainWnd)
{
	pMainWnd = &m_remoteDlg;
	return m_remoteDlg.DoModal();
}

LRESULT CClientController::SendMessage(MSG msg)
{//消息发送线程
	HANDLE hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	if (!hEvent)return -2;
	MSGINFO info(msg);
	PostThreadMessage(m_nThreadID, WM_SEND_MESSAGE, (WPARAM)&info, (LPARAM)&hEvent);
	WaitForSingleObject(hEvent, INFINITE);
	return info.result;
}

CClientController::CClientController() :m_statusDlg(&m_remoteDlg), m_watchDlg(&m_remoteDlg)
{
	m_hThread = INVALID_HANDLE_VALUE;
	m_nThreadID = -1;
}

CClientController::~CClientController()
{
	WaitForSingleObject(m_hThread, 100);
}

void CClientController::threadFunc()
{//消息处理线程
	MSG msg;
	while (::GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
		if (msg.message == WM_SEND_MESSAGE)
		{
			MSGINFO* pMsg = (MSGINFO*)msg.wParam;
			HANDLE hEvent = (HANDLE)msg.lParam;
			auto it = m_mapFunc.find(msg.message);
			if (it != m_mapFunc.end()) {
				//拿到对应函数的返回值
				pMsg->result = (this->*it->second)(pMsg->msg.message, pMsg->msg.wParam, pMsg->msg.lParam);
			}
			else
			{
				pMsg->result = -1;
			}
			SetEvent(hEvent);
		}
		else
		{
			auto it = m_mapFunc.find(msg.message);
			if (it != m_mapFunc.end())
				//拿到对应函数的返回值
				(this->*it->second)(msg.message, msg.wParam, msg.lParam);
		}
	}
}

unsigned __stdcall CClientController::threadEntry(void* arg)
{
	CClientController* thiz = (CClientController*)arg;
	thiz->threadFunc();
	_endthreadex(0);
	return 0;
}

void CClientController::releaseInstance()
{
	if (m_instance != NULL)
	{
		CClientController* tmp = m_instance;
		m_instance = NULL;
		delete tmp;
	}
}

LRESULT CClientController::OnSendPack(UINT nMSG, WPARAM wParam, LPARAM lParam)
{
	return LRESULT();
}

LRESULT CClientController::OnSendData(UINT nMSG, WPARAM wParam, LPARAM lParam)
{
	return LRESULT();
}

LRESULT CClientController::OnShowStatus(UINT nMSG, WPARAM wParam, LPARAM lParam)
{
	return m_statusDlg.ShowWindow(SW_SHOW);
}

LRESULT CClientController::OnShowWatcher(UINT nMSG, WPARAM wParam, LPARAM lParam)
{
	return m_watchDlg.DoModal();
}

CClientController* CClientController::getInstance()
{
	if (m_instance == NULL)
	{
		m_instance = new CClientController();
		struct {
			UINT nMsg;
			MSGFUNC func;
		}MsgFuncs[] = {
			{WM_SEND_PACK,&CClientController::OnSendPack},
			{WM_SEND_DATA,&CClientController::OnSendData},
			{WM_SHOW_STATUS,&CClientController::OnShowStatus},
			{WM_SHOW_WATCH,&CClientController::OnShowWatcher},
			{(UINT)-1,NULL}
		};
		for (int i = 0; MsgFuncs[i].func; i++) {
			m_mapFunc.insert(std::pair<UINT, MSGFUNC>(MsgFuncs
				[i].nMsg, MsgFuncs[i].func));
		}
	}
	return NULL;
}

int CClientController::InitController()
{
	m_hThread = (HANDLE)_beginthreadex(NULL, 0,
		&CClientController::threadEntry, this, 0, &m_nThreadID);
	m_statusDlg.Create(IDD_DLG_STATUS, &m_remoteDlg);
	return 0;
}
