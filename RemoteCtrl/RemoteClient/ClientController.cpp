#include "pch.h"
#include "ClientController.h"


//静态成员需要单独声明，构造时候不会声明，所有类共用一个
std::map<UINT, CClientController::MSGFUNC>
CClientController::m_mapFunc;
CClientController* CClientController::m_instance = NULL;

class CClientController::CNewAndDel {
public:
	CNewAndDel() {
		//CClientController::getInstance();
	}
	~CNewAndDel() {
		CClientController::releaseInstance();
	}
};
CClientController::CNewAndDel CClientController::m_newdel;

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

void CClientController::UpdateAddress(int nIP, int nPort)
{
	CClientSocket::getInstance()->UpdateAddress(nIP, nPort);
}

int CClientController::DealCommand()
{
	return CClientSocket::getInstance()->dealCommand();
}

void CClientController::CloseSocket()
{
	CClientSocket::getInstance()->CloseSocket();
}

bool CClientController::SendPacket(const CPacket& pack)
{
	if (gpClient->initSocket() == false)return false;
	return gpClient->sendCom(pack);
}

//bool CClientController::SendPacket(const CPacket& pack)
//{
//	if(gpClient->initSocket()==false)return false;
//	gpClient->sendCom(pack);
//}

int CClientController::SendCommandPacket(int nCmd, bool bAutoClose, BYTE* pData, size_t nLength)
{
	if (gpClient->initSocket() == false)return false;
	gpClient->sendCom(CPacket(nCmd,pData,nLength));	
	int cmd = DealCommand();
	TRACE("ack:%d\r\n", cmd);
	if (bAutoClose)CloseSocket();
	return cmd;
}

int CClientController::GetImage(CImage& image)
{
	return CHuxlTool::BytesToImage(image, gpClient->GetPacket().strData);
}

int CClientController::DownFile(CString& strPath)
{
	CFileDialog dlg(FALSE, NULL, strPath,
		OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, NULL, &m_remoteDlg);
	if (dlg.DoModal() == IDOK) {
		m_strRemote = strPath;
		m_strLocal = dlg.GetPathName();
		CString strLocal = dlg.GetPathName();
		//线程处理函数
		m_hThreadDown=(HANDLE)_beginthread(&CClientController::threadEntryOfDownFile, 0, this);
		if (WaitForSingleObject(m_hThreadDown, 0) != WAIT_TIMEOUT) return -1;
		m_remoteDlg.BeginWaitCursor();
		m_statusDlg.m_info.SetWindowText(_T("命令正在执行中!"));
		m_statusDlg.ShowWindow(SW_SHOW);
		m_statusDlg.CenterWindow(&m_remoteDlg);
		m_statusDlg.SetActiveWindow();
	}
	return 0;
}

void CClientController::StartWatchScreen()
{
	m_isClosed = false;
	//m_watchDlg.SetParent(&m_remoteDlg);
	m_hThreadWatch = (HANDLE)_beginthread(&CClientController::threadWatchScreen, 0, this);
	m_watchDlg.DoModal();
	m_isClosed = true;
	WaitForSingleObject(m_hThreadWatch, 500);
}

CPacket& CClientController::GetPacket()
{
	return CClientSocket::getInstance()->GetPacket();
}


CClientController::CClientController() :m_statusDlg(&m_remoteDlg), m_watchDlg(&m_remoteDlg)
{
	m_isClosed = true;
	m_hThreadWatch = INVALID_HANDLE_VALUE;
	m_hThread = INVALID_HANDLE_VALUE;
	m_hThreadDown = INVALID_HANDLE_VALUE;
	m_nThreadID = -1;
}

CClientController::~CClientController()
{
	WaitForSingleObject(m_hThread, 100);
}

void CClientController::threadWatchScreen()
{
	Sleep(50);
	while (!m_isClosed)
	{
		if (m_watchDlg.isFull()==false) {
			int ret=SendCommandPacket(6);
			if (ret == 6) {
				CImage image;
				if (GetImage(m_remoteDlg.GetImage()) == 0) {
					m_watchDlg.SetImageStatus(true);
				}
				else TRACE("获取图片失败！%d\r\n",ret);
			}
		}
		Sleep(1);
	}
}

void CClientController::threadWatchScreen(void* arg)
{
	CClientController* thiz = (CClientController*)arg;
	thiz->threadWatchScreen();
	_endthreadex(0);
}

void CClientController::threadFunc()
{//消息处理线程，用来专门接受线程投递的消息，然后根据不同的消息执行相关代码
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

void CClientController::threadDownFile()
{
	FILE* pFile = fopen(m_strLocal, "wb+");
	if(!pFile){
		AfxMessageBox("本地没有权限保存该文件，或者文件无法创建");
		m_statusDlg.ShowWindow(SW_HIDE);
		m_remoteDlg.EndWaitCursor();
		return;
	}
	do
	{
	int ret=SendCommandPacket(4, false, (BYTE*)(LPCSTR)m_strRemote, m_strRemote.GetLength());
	long long nLength = *(long long*)gpClient->GetPacket().strData.c_str();
	if (nLength == 0) {
		AfxMessageBox("文件长度为零或者无法读取文件！！！");
		return;
	}
	long long nCount = 0;
	while (nCount < nLength) {
		ret = gpClient->dealCommand();
		if (ret < 0) {
			AfxMessageBox("传输失败！！");
			TRACE("传输失败：ret =%d\r\n", ret);
			break;
		}
		fwrite(gpClient->GetPacket().strData.c_str(), 1, gpClient->GetPacket().strData.size(), pFile);
		nCount += gpClient->GetPacket().strData.size();
	}
	} while (false);
	fclose(pFile);
	gpClient->CloseSocket();
	m_statusDlg.ShowWindow(SW_HIDE);
	m_remoteDlg.EndWaitCursor();
	m_remoteDlg.MessageBox(_T("下载完成!!"), _T("完成"));

}

void CClientController::threadEntryOfDownFile(void* arg)
{
	CClientController* thiz = (CClientController*)arg;
	thiz->threadDownFile();
	_endthread();
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
	CPacket* pPacket = (CPacket*)wParam;
	return gpClient->sendCom(*pPacket);
}

LRESULT CClientController::OnSendData(UINT nMSG, WPARAM wParam, LPARAM lParam)
{
	char* pBuffer = (char*)wParam;
	return gpClient->sendCom(pBuffer,(int)lParam);
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
	return m_instance;
}

int CClientController::InitController()
{
	m_hThread = (HANDLE)_beginthreadex(NULL, 0,&CClientController::threadEntry, this, 0, &m_nThreadID);
	m_statusDlg.Create(IDD_DLG_STATUS, &m_remoteDlg);
	return 0;
}
