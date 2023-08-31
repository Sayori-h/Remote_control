#include "pch.h"
#include "CHuxlTool.h"
#include "CServerSocket.h"

void CHuxlTool::dump(BYTE* pData, size_t nSize) {
	std::string strOut;
	for (size_t i = 0; i < nSize; i++)
	{
		char buf[16] = "";
		if (i > 0 && (i % 16 == 0))
		{
			strOut += "\n";
		}
		snprintf(buf, sizeof(buf), "%02X ", pData[i] & 0xFF);
		strOut += buf;
	}
	strOut += "\n";
	OutputDebugStringA(strOut.c_str());
}

void CHuxlTool::ShowError()
{
	LPWSTR lpMessageBuf = NULL;//创建缓冲
	//strerror(errno);//标准C语言库
	FormatMessage(
		FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER, NULL, GetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPWSTR)&lpMessageBuf, 0, NULL);
	OutputDebugString(lpMessageBuf);
	MessageBox(NULL, lpMessageBuf, _T("发生错误！"), 0);
	LocalFree(lpMessageBuf);
}

BOOL CHuxlTool::WriteStartupDir(const CString& strPath)
{
	//CString strCmd = GetCommandLine();
	//strCmd.Replace(_T("\""), _T(""));
	TCHAR sPath[MAX_PATH] = _T("");
	GetModuleFileName(NULL, sPath, MAX_PATH);
	return CopyFile(sPath, strPath, FALSE);
	//fopen CFile system(copy) CopyFile OpenFile
}

// 开机启动的时候，程序的权限是跟随启动用户的
// 如果两者权限不一致，则会导致程序启动失败
// 开机启动对环境变量有影响，如果依赖dll(动态库），则可能启动失败
// 解决方法:
//【复制这些d11到system32下面或者syswoW64下面】
// system32下面，多是64位程序 syswow64下面多是32位程序
//[使用动态库而非静态库]
bool CHuxlTool::WriteRegisterTable(const CString& strPath)
{
	CString strSubKey = _T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run");
	TCHAR sPath[MAX_PATH] = _T("");
	//char sSys[MAX_PATH] = "";
	//std::string strExe = "\\RemoteCtrl.exe ";
	GetModuleFileName(NULL, sPath, MAX_PATH);
	//GetCurrentDirectoryA(MAX_PATH, sPath);
	//GetSystemDirectoryA(sSys, sizeof(sSys));
	BOOL ret = CopyFile(sPath, strPath, FALSE);
	if (ret == FALSE) {
		MessageBox(NULL, _T("复制文件失败，是否权限不足？\r\n"), _T("错误"), MB_ICONERROR | MB_TOPMOST);
		return false;
	}
	//创建软链接
	//std::string strCmd = "mklink " + std::string(sSys) + strExe + '\"' + std::string(sPath) + strExe + '\"';
	//int ret = system(strCmd.c_str());
	//TRACE("ret=%d\r\n", ret);
	HKEY hKey = NULL;
	ret = RegOpenKeyEx(HKEY_LOCAL_MACHINE, strSubKey, 0, KEY_ALL_ACCESS | KEY_WOW64_64KEY, &hKey);
	if (ret != ERROR_SUCCESS) {
		RegCloseKey(hKey);
		MessageBox(NULL, _T("设置自动开机启动失败！是否权限不足？\r\n程序启动失败"), _T("错误"), MB_ICONERROR | MB_TOPMOST);
		return false;
	}
	ret = RegSetValueEx(hKey, _T("RemoteCtrl"), 0, REG_SZ, (BYTE*)(LPCTSTR)strPath, strPath.GetLength() * sizeof(TCHAR));
	if (ret != ERROR_SUCCESS) {
		RegCloseKey(hKey);
		MessageBox(NULL, _T("设置自动开机启动失败！是否权限不足？\r\n程序启动失败"), _T("错误"), MB_ICONERROR | MB_TOPMOST);
		return false;
	}
	RegCloseKey(hKey);
	return true;
}

bool CHuxlTool::Init()
{
	HMODULE hModule = ::GetModuleHandle(nullptr);
	if (hModule == nullptr) {
		wprintf(L"错误: GetModuleHandle失败\n");
		return false;
	}
	if (!AfxWinInit(hModule, nullptr, ::GetCommandLine(), 0))
	{
		//TODO:在此处为应用程序编写代码
		wprintf(L"错误: MFC 初始化失败\n");
		return false;
	}
	return true;
}

bool CHuxlTool::IsAdmin()
{
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

bool CHuxlTool::RunAsAdmin()
{
	//获取管理员权限，使用该权限创建进程；
	//本地策略组 开启Administrator账户  禁止空密码只能登录本地控制台
	/*HANDLE hToken = NULL;
	BOOL ret = LogonUser(L"Administrator", NULL, NULL, LOGON32_LOGON_INTERACTIVE, LOGON32_PROVIDER_DEFAULT, &hToken);
	if (!ret) {
		ShowError();
		MessageBox(NULL, _T("登录错误！"), _T("程序错误"), 0);
		exit(0);
	}
	OutputDebugString(L"Login administrator success!\r\n");*/
	STARTUPINFO si{ 0 };
	PROCESS_INFORMATION pi{ 0 };
	TCHAR sPath[MAX_PATH] = _T("");
	//GetCurrentDirectory(MAX_PATH, sPath);
	GetModuleFileName(NULL, sPath, MAX_PATH);
	//CString strCmd = sPath;
	//strCmd += _T("\\RemoteCtrl.exe");
	//ret = CreateProcessWithTokenW(hToken, LOGON_WITH_PROFILE, NULL, 
	//	(LPWSTR)(LPCWSTR)strCmd, CREATE_UNICODE_ENVIRONMENT, NULL, NULL, &si, &pi);
	//直接登录方式
	BOOL ret = CreateProcessWithLogonW(_T("Administrator"), NULL, NULL, LOGON_WITH_PROFILE, NULL,
		sPath, CREATE_UNICODE_ENVIRONMENT, NULL, NULL, &si, &pi);
	//CloseHandle(hToken);
	if (!ret) {
		ShowError();//TODO:去除调试信息
		MessageBox(NULL, sPath,_T("创建进程失败"), 0);//TODO:去除调试信息
		return false;
	}
	//创建完子进程后等待子进程结束，子进程拥有admin权限
	WaitForSingleObject(pi.hProcess, INFINITE);
	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);
	return true;
}


