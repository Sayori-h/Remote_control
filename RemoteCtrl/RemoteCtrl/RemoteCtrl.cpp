// RemoteCtrl.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include "pch.h"
#include "framework.h"
#include "RemoteCtrl.h"
#include "CServerSocket.h"

#include "Command.h"


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
	if (PathFileExists(strPath))return false;
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

int main(){
	if (CHuxlTool::IsAdmin()) {
		if (!CHuxlTool::Init())return 1;
		//OutputDebugString(L"current is run as administrator!\r\n");
		//MessageBox(NULL, _T("管理员"), _T("用户状态"), 0);
		if (ChooseAutoInvoke(INVOKE_PATH)) {
			CCommand cmd;
			int ret = gpServer->run(&CCommand::runCommand, (void*)&cmd);
			switch (ret) {
			case -1:
				MessageBox(NULL, _T("网络初始化失败"), _T("错误"), MB_OK | MB_ICONERROR);
				break;
			case -2:
				MessageBox(NULL, _T("多次无法连接用户，结束进程"), _T("错误"), MB_OK | MB_ICONERROR);
				break;
			}
		}
	}
	else {
		//OutputDebugString(L"current is run as normal user!\r\n");
		//MessageBox(NULL, _T("普通用户"), _T("用户状态"), 0);
		if (CHuxlTool::RunAsAdmin() == false) {
			CHuxlTool::ShowError();
			return 1;
		}
	}
	return 0;
}
