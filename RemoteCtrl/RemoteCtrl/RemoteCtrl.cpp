﻿// RemoteCtrl.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
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

void udp_server();
void udp_client(bool ishost=true);

void initsock() {
	WSADATA wsa;
	WSAStartup(MAKEWORD(2, 2), &wsa);
}

void clearsock() {
	WSACleanup();
}

int main(int argc,char *argv[]){
	if (!CHuxlTool::Init())return 1;
	initsock();
	if (argc == 1) {//服务器代码
		char wstrDir[MAX_PATH];
		GetCurrentDirectoryA(MAX_PATH, wstrDir);
		STARTUPINFOA si;
		PROCESS_INFORMATION pi;
		memset(&si, 0, sizeof(si));
		memset(&pi, 0, sizeof(pi));
		std::string strCmd = argv[0];
		strCmd +="  1";
		BOOL bRet=CreateProcessA(NULL, (LPSTR)strCmd.c_str(), NULL, NULL, FALSE,0, NULL, wstrDir, &si, &pi);
		if (bRet) {
			CloseHandle(pi.hThread);
			CloseHandle(pi.hProcess);
			TRACE("进程ID:%d\r\n", pi.dwProcessId);
			TRACE("线程ID:%d\r\n", pi.dwThreadId);
			strCmd += "  2";
			bRet = CreateProcessA(NULL, (LPSTR)strCmd.c_str(), NULL, NULL, FALSE, 0, NULL, wstrDir, &si, &pi);
			if (bRet) {
				CloseHandle(pi.hThread);
				CloseHandle(pi.hProcess);
				TRACE("进程ID:%d\r\n", pi.dwProcessId);
				TRACE("线程ID:%d\r\n", pi.dwThreadId);
				udp_server();
			}
		}
		
	}
	else if (argc == 2) {//主客户端
		udp_client();
	}
	else {//从客户端
		udp_client(false);
	}
	//iocp();
	clearsock();
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

void udp_server() {
	printf("%s(%d):%s\r\n", __FILE__, __LINE__, __FUNCTION__);
	SOCKET sock = socket(PF_INET, SOCK_DGRAM, 0);
	while (sock == INVALID_SOCKET) {
		printf("%s(%d):%s ERROR(%d)!!\r\n", __FILE__, __LINE__, __FUNCTION__,WSAGetLastError());
		return;
	}
	std::list<sockaddr_in>lstclients;
	sockaddr_in server, client;
	memset(&server, 0, sizeof(server));
	memset(&client, 0, sizeof(client));
	server.sin_family = AF_INET;
	server.sin_port = htons(20000);
	server.sin_addr.s_addr = inet_addr("127.0.0.1");
	if (-1 == bind(sock, (sockaddr*)&server, sizeof(server))) {
		printf("%s(%d):%s ERROR(%d)!!\r\n", __FILE__, __LINE__, __FUNCTION__,WSAGetLastError());
		closesocket(sock);
		return;
	}
	std::string buf;
	buf.resize(1024 * 256);
	memset((char*)buf.c_str(), 0, buf.size());
	int len = sizeof client;
	int ret = 0;
	while (!_kbhit()) {
		ret=recvfrom(sock, (char*)buf.c_str(), buf.size(), 0, (sockaddr*)&client, &len);
		if (ret>0) {
			if (lstclients.size() <= 0) {
				lstclients.push_back(client);
				printf("%s(%d):%s ip %08x port %d\r\n", __FILE__, __LINE__,
					__FUNCTION__, client.sin_addr.s_addr, client.sin_port);
				ret = sendto(sock, buf.c_str(), sizeof lstclients.front(), 0, (sockaddr*)&client, len);
				printf("%s(%d):%s\r\n", __FILE__, __LINE__, __FUNCTION__);
			}
			else {
				memcpy((void*)buf.c_str(), &lstclients.front(), sizeof lstclients.front());
				ret = sendto(sock, buf.c_str(), ret, 0, (sockaddr*)&client, len);
				printf("%s(%d):%s\r\n", __FILE__, __LINE__, __FUNCTION__);
			}
			//CHuxlTool::dump((BYTE*)buf.c_str(), ret);
		}
		else {
			printf("%s(%d):%s ERROR(%d)!! ret=%d\r\n", __FILE__, __LINE__, __FUNCTION__, WSAGetLastError(),ret);

		}
	}
	closesocket(sock);
	printf("%s(%d):%s\r\n", __FILE__, __LINE__, __FUNCTION__);
}
void udp_client(bool ishost) {
	Sleep(2000);//先保证服务器起来
	sockaddr_in server,client;
	int len = sizeof client;
	server.sin_family = AF_INET;
	server.sin_port = htons(20000);
	server.sin_addr.s_addr = inet_addr("127.0.0.1");
	SOCKET sock = socket(PF_INET, SOCK_DGRAM, 0);
	while (sock == INVALID_SOCKET) {
		printf("%s(%d):%s ERROR!!\r\n", __FILE__, __LINE__, __FUNCTION__);
		
		return;
	}
	
	if (ishost) {
		printf("%s(%d):%s\r\n", __FILE__, __LINE__, __FUNCTION__);
		std::string msg="hello world!\n";
		int ret=sendto(sock, msg.c_str(), msg.size(), 0, (sockaddr*)&server, sizeof server);
		printf("%s(%d):%s ret= %d\r\n", __FILE__, __LINE__, __FUNCTION__,ret);
		if (ret > 0) {
			msg.resize(1024);
			memset((char*)msg.c_str(), 0, msg.size());
			ret=recvfrom(sock, (char*)msg.c_str(), msg.size(), 0, (sockaddr*)&client, &len);
			printf("host %s(%d):%s ERROR(%d)!! ret=%d\r\n", __FILE__, __LINE__, __FUNCTION__, WSAGetLastError(), ret);
			if (ret > 0) {
				printf("%s(%d):%s ip %08x port %d\r\n", __FILE__, __LINE__,
					__FUNCTION__, client.sin_addr.s_addr, ntohs(client.sin_port));
				printf("%s(%d):%s msg= %d\r\n", __FILE__, __LINE__, __FUNCTION__, msg.size());

			}
			ret = recvfrom(sock, (char*)msg.c_str(), msg.size(), 0, (sockaddr*)&client, &len);
			printf("host %s(%d):%s ERROR(%d)!! ret=%d\r\n", __FILE__, __LINE__, __FUNCTION__, WSAGetLastError(), ret);
			if (ret > 0) {
				printf("%s(%d):%s ip %08x port %d\r\n", __FILE__, __LINE__,
					__FUNCTION__, client.sin_addr.s_addr, ntohs(client.sin_port));
				printf("%s(%d):%s msg= %s\r\n", __FILE__, __LINE__, __FUNCTION__, msg.c_str());

			}

		}
	}
	else {
		printf("%s(%d):%s\r\n", __FILE__, __LINE__, __FUNCTION__);
		std::string msg="hello world!\n";
		int ret=sendto(sock, msg.c_str(), msg.size(), 0, (sockaddr*)&server, sizeof server);
		printf("%s(%d):%s ret= %d\r\n", __FILE__, __LINE__, __FUNCTION__,ret);
		if (ret > 0) {
			msg.resize(1024);
			memset((char*)msg.c_str(), 0, msg.size());
			ret=recvfrom(sock, (char*)msg.c_str(), msg.size(), 0, (sockaddr*)&client, &len);
			printf("client %s(%d):%s ret= %d\r\n", __FILE__, __LINE__, __FUNCTION__, ret);
			if (ret > 0) {
				sockaddr_in addr;
				memcpy(&addr, msg.c_str(), sizeof addr);
				sockaddr_in* paddr = (sockaddr_in*)&addr;
				printf("%s(%d):%s ip %08x port %d\r\n", __FILE__, __LINE__,
					__FUNCTION__, client.sin_addr.s_addr, ntohs(client.sin_port));
				printf("%s(%d):%s msg= %d\r\n", __FILE__, __LINE__, __FUNCTION__, msg.size());
				printf("%s(%d):%s ip %08x port %d\r\n", __FILE__, __LINE__,
					__FUNCTION__, paddr->sin_addr.s_addr, ntohs(paddr->sin_port));
				msg = "hello,i am client!\r\n";
				ret=sendto(sock, (char*)msg.c_str(), msg.size(), 0, (sockaddr*)paddr, sizeof sockaddr_in);
				printf("%s(%d):%s ip %08x port %d\r\n", __FILE__, __LINE__,
					__FUNCTION__, paddr->sin_addr.s_addr, ntohs(paddr->sin_port));
				printf("host %s(%d):%s ERROR(%d)!! ret=%d\r\n", __FILE__, __LINE__, __FUNCTION__, WSAGetLastError(), ret);

			}
		}
	}
	closesocket(sock);
}
