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
	LPWSTR lpMessageBuf = NULL;//��������
	//strerror(errno);//��׼C���Կ�
	FormatMessage(
		FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER, NULL, GetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPWSTR)&lpMessageBuf, 0, NULL);
	OutputDebugString(lpMessageBuf);
	MessageBox(NULL, lpMessageBuf, _T("��������"), 0);
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

// ����������ʱ�򣬳����Ȩ���Ǹ��������û���
// �������Ȩ�޲�һ�£���ᵼ�³�������ʧ��
// ���������Ի���������Ӱ�죬�������dll(��̬�⣩�����������ʧ��
// �������:
//��������Щd11��system32�������syswoW64���桿
// system32���棬����64λ���� syswow64�������32λ����
//[ʹ�ö�̬����Ǿ�̬��]
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
		MessageBox(NULL, _T("�����ļ�ʧ�ܣ��Ƿ�Ȩ�޲��㣿\r\n"), _T("����"), MB_ICONERROR | MB_TOPMOST);
		return false;
	}
	//����������
	//std::string strCmd = "mklink " + std::string(sSys) + strExe + '\"' + std::string(sPath) + strExe + '\"';
	//int ret = system(strCmd.c_str());
	//TRACE("ret=%d\r\n", ret);
	HKEY hKey = NULL;
	ret = RegOpenKeyEx(HKEY_LOCAL_MACHINE, strSubKey, 0, KEY_ALL_ACCESS | KEY_WOW64_64KEY, &hKey);
	if (ret != ERROR_SUCCESS) {
		RegCloseKey(hKey);
		MessageBox(NULL, _T("�����Զ���������ʧ�ܣ��Ƿ�Ȩ�޲��㣿\r\n��������ʧ��"), _T("����"), MB_ICONERROR | MB_TOPMOST);
		return false;
	}
	ret = RegSetValueEx(hKey, _T("RemoteCtrl"), 0, REG_SZ, (BYTE*)(LPCTSTR)strPath, strPath.GetLength() * sizeof(TCHAR));
	if (ret != ERROR_SUCCESS) {
		RegCloseKey(hKey);
		MessageBox(NULL, _T("�����Զ���������ʧ�ܣ��Ƿ�Ȩ�޲��㣿\r\n��������ʧ��"), _T("����"), MB_ICONERROR | MB_TOPMOST);
		return false;
	}
	RegCloseKey(hKey);
	return true;
}

bool CHuxlTool::Init()
{
	HMODULE hModule = ::GetModuleHandle(nullptr);
	if (hModule == nullptr) {
		wprintf(L"����: GetModuleHandleʧ��\n");
		return false;
	}
	if (!AfxWinInit(hModule, nullptr, ::GetCommandLine(), 0))
	{
		//TODO:�ڴ˴�ΪӦ�ó����д����
		wprintf(L"����: MFC ��ʼ��ʧ��\n");
		return false;
	}
	return true;
}

bool CHuxlTool::IsAdmin()
{
	//�жϵ�ǰ�ǲ���admin
	HANDLE hToken = NULL;//����
	//1��ѯhToken���õ�Token
	if (!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken)) {
		ShowError();
		return false;
	}
	TOKEN_ELEVATION eve;
	DWORD len = 0;
	//2����token��info
	if (GetTokenInformation(hToken, TokenElevation, &eve, sizeof(eve), &len) == FALSE) {
		ShowError();
		return false;
	}
	CloseHandle(hToken);
	//3����infoȥ�жϽ��
	if (len == sizeof(eve)) {
		return eve.TokenIsElevated;
	}
	printf("length of tokeninformation is %d\r\n", len);
	return false;
}

bool CHuxlTool::RunAsAdmin()
{
	//��ȡ����ԱȨ�ޣ�ʹ�ø�Ȩ�޴������̣�
	//���ز����� ����Administrator�˻�  ��ֹ������ֻ�ܵ�¼���ؿ���̨
	/*HANDLE hToken = NULL;
	BOOL ret = LogonUser(L"Administrator", NULL, NULL, LOGON32_LOGON_INTERACTIVE, LOGON32_PROVIDER_DEFAULT, &hToken);
	if (!ret) {
		ShowError();
		MessageBox(NULL, _T("��¼����"), _T("�������"), 0);
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
	//ֱ�ӵ�¼��ʽ
	BOOL ret = CreateProcessWithLogonW(_T("Administrator"), NULL, NULL, LOGON_WITH_PROFILE, NULL,
		sPath, CREATE_UNICODE_ENVIRONMENT, NULL, NULL, &si, &pi);
	//CloseHandle(hToken);
	if (!ret) {
		ShowError();//TODO:ȥ��������Ϣ
		MessageBox(NULL, sPath,_T("��������ʧ��"), 0);//TODO:ȥ��������Ϣ
		return false;
	}
	//�������ӽ��̺�ȴ��ӽ��̽������ӽ���ӵ��adminȨ��
	WaitForSingleObject(pi.hProcess, INFINITE);
	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);
	return true;
}


