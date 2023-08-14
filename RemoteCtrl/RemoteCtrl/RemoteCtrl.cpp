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

void ShowError() {
	LPWSTR lpMessageBuf = NULL;//创建缓冲
	//strerror(errno);//标准C语言库
	FormatMessage(
		FORMAT_MESSAGE_FROM_SYSTEM|FORMAT_MESSAGE_ALLOCATE_BUFFER,NULL, GetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),(LPWSTR)&lpMessageBuf, 0, NULL);
	OutputDebugString(lpMessageBuf);
	MessageBox(NULL, lpMessageBuf, _T("发生错误！"), 0);
	LocalFree(lpMessageBuf);
}

bool IsAdmin() {
	//判断当前是不是admin
	HANDLE hToken = NULL;//令牌
	//1查询hToken，拿到Token
	if (!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken)) {
		ShowError();
		return false;
	}
	TOKEN_ELEVATION eve;
	DWORD len = 0;
	//2根据token拿info
	if (GetTokenInformation(hToken, TokenElevation, &eve, sizeof(eve), &len) == FALSE) {
		ShowError();
		return false;
	}
	CloseHandle(hToken);
	//3根据info去判断结果
	if (len == sizeof(eve)) {
		return eve.TokenIsElevated;
	}
	printf("length of tokeninformation is %d\r\n", len);
	return false;
}

void RunAsAdmin()
{
	//获取管理员权限，使用该权限创建进程；
	HANDLE hToken = NULL;
	BOOL ret = LogonUser(L"Administrator", NULL, NULL, LOGON32_LOGON_INTERACTIVE, LOGON32_PROVIDER_DEFAULT, &hToken);
	if (!ret){
		ShowError();
		MessageBox(NULL, _T("登录错误！"), _T("程序错误"), 0);
		exit(0);
	}
	OutputDebugString(L"Logon administrator success!\r\n");
	STARTUPINFO si{ 0 };
	PROCESS_INFORMATION pi{ 0 };
	TCHAR sPath[MAX_PATH] = _T("");
	GetCurrentDirectory(MAX_PATH, sPath);
	CString strCmd = sPath;
	strCmd += _T("\\RemoteCtrl.exe");
	//ret = CreateProcessWithTokenW(hToken, LOGON_WITH_PROFILE, NULL, 
	//	(LPWSTR)(LPCWSTR)strCmd, CREATE_UNICODE_ENVIRONMENT, NULL, NULL, &si, &pi);
	//直接登录方式
	ret=CreateProcessWithLogonW(_T("Administrator"),NULL,NULL, LOGON_WITH_PROFILE, NULL,
	(LPWSTR)(LPCWSTR)strCmd, CREATE_UNICODE_ENVIRONMENT, NULL, NULL, &si, &pi);
	CloseHandle(hToken);
	if (!ret){
		ShowError();
		MessageBox(NULL, _T("创建进程失败"), _T("程序错误"), 0);
		exit(0);
	}
	//创建完子进程后等待子进程结束，子进程拥有admin权限
	WaitForSingleObject(pi.hProcess, INFINITE);
	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);
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
			if (IsAdmin()) {
				OutputDebugString(L"current is run as administrator!\r\n");
				//MessageBox(NULL, _T("管理员"), _T("用户状态"), 0);
			}
			else {
				OutputDebugString(L"current is run as normal user!\r\n");
				//MessageBox(NULL, _T("普通用户"), _T("用户状态"), 0);
				RunAsAdmin();
				return nRetCode;
			}
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
