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

CWinApp theApp;

// 开机启动的时候，程序的权限是跟随启动用户的
// 如果两者权限不一致，则会导致程序启动失败
// 开机启动对环境变量有影响，如果依赖dll(动态库），则可能启动失败
// 解决方法:
//【复制这些d11到system32下面或者syswoW64下面】
// system32下面，多是64位程序 syswow64下面多是32位程序
//[使用动态库而非静态库]
void WriteRegisterTable(const CString& strPath) {
	CString strSubKey = _T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run");
	char sPath[MAX_PATH] = "";
	char sSys[MAX_PATH] = "";
	std::string strExe = "\\RemoteCtrl.exe ";
	GetCurrentDirectoryA(MAX_PATH, sPath);
	GetSystemDirectoryA(sSys, sizeof(sSys));
	//创建软链接
	std::string strCmd = "mklink " + std::string(sSys) + strExe + '\"' + std::string(sPath) + strExe + '\"';
	int ret = system(strCmd.c_str());
	TRACE("ret=%d\r\n", ret);
	HKEY hKey = NULL;
	ret = RegOpenKeyEx(HKEY_LOCAL_MACHINE, strSubKey, 0, KEY_ALL_ACCESS | KEY_WOW64_64KEY, &hKey);
	if (ret != ERROR_SUCCESS) {
		RegCloseKey(hKey);
		MessageBox(NULL, _T("设置自动开机启动失败！是否权限不足？\r\n程序启动失败"), _T("错误"), MB_ICONERROR | MB_TOPMOST);
		exit(0);
	}
	ret = RegSetValueEx(hKey, _T("RemoteCtrl"), 0, REG_SZ, (BYTE*)(LPCTSTR)strPath, strPath.GetLength() * sizeof(TCHAR));
	if (ret != ERROR_SUCCESS) {
		RegCloseKey(hKey);
		MessageBox(NULL, _T("设置自动开机启动失败！是否权限不足？\r\n程序启动失败"), _T("错误"), MB_ICONERROR | MB_TOPMOST);
		exit(0);
	}
	RegCloseKey(hKey);
}

void WriteStartupDir(const CString& strPath) {
	
	CString strCmd = GetCommandLine();
	strCmd.Replace(_T("\""), _T(""));
	BOOL ret = CopyFile(strCmd, strPath, FALSE);
	//fopen CFile system(copy) CopyFile OpenFile
	if (ret == FALSE) {
		MessageBox(NULL, _T("复制文件失败，是否权限不足?\r\n"), _T("错误"), MB_ICONERROR | MB_TOPMOST);
		exit(0);
	}
}

void ChooseAutoInvoke() {
	TCHAR wcsSystem[MAX_PATH] = _T("");
	//CString strPath = CString(_T("C:\\Windows\\SysWOW64\\RemoteCtrl.exe"));
	CString strPath = _T("C:\\Users\\胡小龙    电话15948426680\\AppData\
\\Roaming\\Microsoft\\Windows\\Start Menu\\Programs\\Startup\\RemoteCtrl.exe");
	if (PathFileExists(strPath))return;
	CString strInfo = _T("Program 鬵in……&g...\n");
	strInfo += _T("敤Warten&...");
	int ret=MessageBox(NULL, strInfo, _T("$提湿口"), MB_OKCANCEL | MB_ICONINFORMATION | MB_TOPMOST);
	if (ret == IDOK) {
		//WriteRegisterTable()
		WriteStartupDir(strPath);
	}
	else exit(0);
}

int main()
{
	int nRetCode = 0;
	HMODULE hModule = ::GetModuleHandle(nullptr);
	if (hModule != nullptr)
	{
		// 初始化 MFC 并在失败时显示错误
		if (!AfxWinInit(hModule, nullptr, ::GetCommandLine(), 0))
		{
			wprintf(L"错误: MFC 初始化失败\n");
			nRetCode = 1;
		}
		else
		{
			CCommand cmd;
			ChooseAutoInvoke();
			int count = 0;
			if (gpServer->initSocket() == false)
			{
				MessageBox(NULL, _T("网络初始化失败"), _T("错误"), MB_OK | MB_ICONERROR);
				exit(0);
			}
			while (gpServer != NULL)
			{
				if (gpServer->acceptCli() == false)
				{
					if (count > 3) {
						MessageBox(NULL, _T("多次无法连接用户，结束进程"), _T("错误"), MB_OK | MB_ICONERROR);
						exit(0);
					}
					MessageBox(NULL, _T("无法连接用户，自动重试"), _T("错误"), MB_OK | MB_ICONERROR);
					count++;
				}
				TRACE("AcceptClient return true\r\n");
				int ret = gpServer->dealCommand();
				TRACE("dealCommand ret %d\r\n", ret);
				if (ret > 0)
				{
					ret = cmd.ExcuteCommand(ret);
					if (ret)
					{
						TRACE("执行命令失败:%d ret=%d\r\n", gpServer->GetPacket().sCmd, ret);
					}
					gpServer->CloseClient();
				}
			}
		}
	}
	else
	{
		// 更改错误代码以符合需要
		wprintf(L"错误: GetModuleHandle 失败\n");
		nRetCode = 1;
	}

	return nRetCode;
}
