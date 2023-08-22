// RemoteCtrl.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include "pch.h"
#include "framework.h"
#include "RemoteCtrl.h"
#include "CServerSocket.h"
#include "Command.h"
#include <conio.h>


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

enum {
	IocpListEmpty,
	IocpListPush,
	IocpListPop
};

typedef struct IocpParam {
	int nOperator;//操作
	std::string strData;//数据
	_beginthread_proc_type cbFunc;//回调
	IocpParam(int op, const char* sData, _beginthread_proc_type cb=NULL) {
		nOperator = op;
		strData = sData;
		cbFunc = cb;
	}
	IocpParam() {
		nOperator = -1;
	}
}IOCP_PARAM;

void threadmain(HANDLE hIOCP) {
	std::list<std::string>lstString;//内存泄漏，单独放在函数里，函数结束一定可以析构
	DWORD dwTransferred = 0;
	ULONG_PTR CompletionKey = 0;
	OVERLAPPED* pOverlapped = NULL;
	int count = 0, count0 = 0, total = 0;
	while (GetQueuedCompletionStatus(hIOCP, &dwTransferred, &CompletionKey, &pOverlapped, INFINITE)) {
		if ((dwTransferred == 0) || (CompletionKey == NULL)) {//post的参数
			printf("thread is prepare to exit!\r\n");
			break;
		}
		IOCP_PARAM* pParam = (IOCP_PARAM*)CompletionKey;
		if (pParam->nOperator == IocpListPush) {
			lstString.push_back(pParam->strData);
			printf("push size:%d\r\n", lstString.size());
			count0++;
		}
		else if (pParam->nOperator == IocpListPop) {
			printf("%p size:%d\r\n", pParam->cbFunc, lstString.size());
			std::string str;
			if (lstString.size() > 0) {
				str = (lstString.front());
				lstString.pop_front();
			}
			if (pParam->cbFunc) {
				pParam->cbFunc((void*)&str);//回调函数当场处理
			}
			count++;
		}
		else if (pParam->nOperator == IocpListEmpty) {
			lstString.clear();
		}
		delete pParam;
		printf("total %d\r\n", ++total);
	}
	printf("thread exit count:%d count0:%d\r\n", count, count0);
}

void threadQueueEntry(HANDLE hIOCP) {//入口函数和功能函数分开，_endthread()后面才析构
	threadmain(hIOCP);
	_endthread();//代码到此为止，会导致本地对象无法调用析构，从而使得内存发生泄漏，所以单独拿出来
}

void func(void* arg) {
	std::string* pstr = (std::string*)arg;
	if (pstr != NULL) {
		printf("pop from list:%s\r\n", pstr->c_str());
		//delete pstr;
	}
	else {
		printf("list is empty,no data!\r\n");
	}
}
int main(){
	if (!CHuxlTool::Init())return 1;
	/*
	总的流程就是：创建一个完成端口对象，把某个文件或非文件绑定起来，然后开一个线程，然后有其他的线程就可以往
	完成端口里面触发一个事件，这是一种方式。还有一种就是说往这个绑定的句柄做一些事情，比如读写数据
	通过这两种方式推动完成端口状态的转换，这个状态怎么转换的呢，这个转换在外面是看不到的，在外面只能看到往完成端口句柄（hIOCP）
	里面做了一些操作，做完这些操作后，有些状态会发生变化；但这些变化不要当场去拿，最好是通过回调函数去拿。最后要往完成端口里面
	post一个空的东西，如果抓到空的东西，就知道我要结束了
	为什么能实现线程安全：真正能操作list的线程只有一个，只不过数据的读取和写入允许多线程.
	好处1：不用上锁。好处2：数据拿到后怎么处理？通过回调函数，当场处理掉，不需要线程等待，把请求与实现分离了
	请求可以随便发不会乱，系统会保证是按顺序来的，类似与MFC消息机制，区别在IOCP是由内核控制的
	*/
	printf("press any key to exit …… \r\n");
	HANDLE hIOCP = INVALID_HANDLE_VALUE;//Input/Output Completion Port
	hIOCP = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, NULL, 1);//epoll是单线程的，CP可以有多个线程
	if (hIOCP == INVALID_HANDLE_VALUE || (hIOCP == NULL)) {
		printf("create iocp failed!%d\r\n", GetLastError());
		return 1;
	}
	HANDLE hThread=(HANDLE)_beginthread(threadQueueEntry, 0, hIOCP);

	ULONGLONG tick = GetTickCount64();
	ULONGLONG tick0 = GetTickCount64();
	int count = 0, count0 = 0;
	while (_kbhit()==0) {//核心设计理念：完成端口把请求(多线程pop,push,empty)与实现分离了
		if (GetTickCount64() - tick0 > 1300) {//read
			PostQueuedCompletionStatus(hIOCP, sizeof(IOCP_PARAM),
				(ULONG_PTR)new IOCP_PARAM(IocpListPop, "hello world",func), NULL);
			tick0 = GetTickCount64();
			count++;
		}
		if (GetTickCount64() - tick > 2000) {//write
			PostQueuedCompletionStatus(hIOCP, sizeof(IOCP_PARAM), 
				(ULONG_PTR)new IOCP_PARAM(IocpListPush, "hello world"), NULL);
			tick = GetTickCount64();
			count0++;
		}
		Sleep(1);
	}
	if (hIOCP != NULL) {
		//控制完成端口状态
		PostQueuedCompletionStatus(hIOCP, 0, NULL, NULL);
		WaitForSingleObject(hThread, INFINITE);
	}
	//所有线程结束后再close
	CloseHandle(hIOCP);
	printf("hThread exit done! count:%d count0:%d\r\n",count,count0);
	exit(0);

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
