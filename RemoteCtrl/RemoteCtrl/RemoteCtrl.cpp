// RemoteCtrl.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include "pch.h"
#include "framework.h"
#include "RemoteCtrl.h"
#include "CServerSocket.h"
#include "Command.h"
#include <conio.h>
#include "CEdoyunQueue.h"
#include <MSWSock.h>
#include "HuxlServer.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// 唯一的应用程序对象
//#define INVOKE_PATH _T("C:\\Windows\\SysWOW64\\RemoteCtrl.exe")
#define INVOKE_PATH _T("C:\\Users\\胡小龙    电话15948426680\\AppData\
\\Roaming\\Microsoft\\Windows\\Start Menu\\Programs\\Startup\\RemoteCtrl.exe")
CWinApp theApp;

//业务逻辑（放到别的项目要改的）和通用功能（放到哪个项目都能用）分开
bool ChooseAutoInvoke(const CString& strPath) {
	TCHAR wcsSystem[MAX_PATH] = _T("");
	//CString strPath = CString(_T("C:\\Windows\\SysWOW64\\RemoteCtrl.exe"));
	if (PathFileExists(strPath))return true;
	CString strInfo = _T("Program 鬵in……&g...\n");
	strInfo += _T("敤Warten&...");
	int ret = MessageBox(NULL, strInfo, _T("$提湿口"), MB_OKCANCEL | MB_ICONINFORMATION | MB_TOPMOST);
	if (ret == IDOK) {
		//WriteRegisterTable()
		if (!CHuxlTool::WriteStartupDir(strPath)) {
			MessageBox(NULL, _T("复制文件失败，是否权限不足?\r\n"), _T("错误"), MB_ICONERROR | MB_TOPMOST);
			return false;
		}
	}
	else if (ret == IDCANCEL) return false;
	return true;
}

class COverlapped {
public:
	OVERLAPPED m_overlapped;
	DWORD m_operator;
	char m_buffer[4096];
	COverlapped() {
		m_operator = 0;
		memset(&m_overlapped, 0, sizeof(m_overlapped));
		memset(m_buffer, 0, sizeof(m_buffer));
	}
};

void iocp() {
	////SOCKET sock = socket(AF_INET,SOCK_STREAM,0);//TCP
	//SOCKET sock = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	//if (sock == INVALID_SOCKET) {
	//	CHuxlTool::ShowError();
	//	return;
	//}
	//HANDLE hIOCP = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, sock, 4);
	//SOCKET client = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	//CreateIoCompletionPort((HANDLE)sock, hIOCP, 0, 0);

	//sockaddr_in addr;
	//addr.sin_family = PF_INET;
	//addr.sin_addr.s_addr = inet_addr("0.0.0.0");
	//addr.sin_port = htons(9527);
	//bind(sock, (sockaddr*)&addr, sizeof(addr));
	//listen(sock, 5);
	//COverlapped overlapped;
	//overlapped.m_operator = 1;//accept
	//memset(&overlapped, 0, sizeof(OVERLAPPED));
	////accept()
	//char buffer[4096] = "";
	//DWORD received = 0;
	//if (AcceptEx(sock, client, overlapped.m_buffer, 0,/*sizeof(buffer)-32,只有填满字节数才会建立连接，防止空的嗅探包,*/
	//	sizeof(sockaddr_in),/*+ 16规定 + 16 */
	//	sizeof(sockaddr_in) + 16, &received, &overlapped.m_overlapped) == FALSE) {
	//	CHuxlTool::ShowError();
	//}
	//overlapped.m_operator = 2;//send
	//WSASend();
	//overlapped.m_operator = 3;//recv
	//WSARecv();

	////开启线程
	//while (true)
	//{//代表一个线程
	//	LPOVERLAPPED pOverlapped = NULL;
	//	DWORD transferred = 0;
	//	DWORD Compeletionkey = 0;
	//	if (GetQueuedCompletionStatus(hIOCP,&transferred,&Compeletionkey,&pOverlapped,INFINITE))
	//	{//利用宏拿到原来父类的地址，COverlapped对象的一个地址
	//		COverlapped* p0 = CONTAINING_RECORD(pOverlapped, COverlapped, m_overlapped);
	//		switch (p0->m_operator)
	//		{
	//		case 1:
	//			
	//		default:
	//			break;
	//		}
	//		
	//	}
	//}	
	HuxlServer server;
	server.StartService();
	getchar();
}

int main(){
	if (!CHuxlTool::Init())return 1;
	iocp();

	//if (CHuxlTool::IsAdmin()) {
	//	if (!CHuxlTool::Init())return 1;
	//	//OutputDebugString(L"current is run as administrator!\r\n");
	//	//MessageBox(NULL, _T("管理员"), _T("用户状态"), 0);
	//	if (ChooseAutoInvoke(INVOKE_PATH)) {
	//		CCommand cmd;
	//		int ret = gpServer->run(&CCommand::runCommand, (void*)&cmd);
	//		switch (ret) {
	//		case -1:
	//			MessageBox(NULL, _T("网络初始化失败"), _T("错误"), MB_OK | MB_ICONERROR);
	//			break;
	//		case -2:
	//			MessageBox(NULL, _T("多次无法连接用户，结束进程"), _T("错误"), MB_OK | MB_ICONERROR);
	//			break;
	//		}
	//	}
	//}
	//else {
	//	//OutputDebugString(L"current is run as normal user!\r\n");
	//	//MessageBox(NULL, _T("普通用户"), _T("用户状态"), 0);
	//	if (CHuxlTool::RunAsAdmin() == false) {
	//		CHuxlTool::ShowError();
	//		return 1;
	//	}
	//}
	return 0;
}
