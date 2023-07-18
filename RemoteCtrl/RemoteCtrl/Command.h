#pragma once
#include "Resource.h"
#include <map>
#include <atlimage.h>
#include <direct.h>
#include <corecrt_io.h>
#include "CServerSocket.h"
#include "LockInfoDialog.h"
#include "CHuxlTool.h"

class CCommand
{
public:
	typedef int(CCommand::* CMDFUNC)();//����ָ��
	std::map<int, CMDFUNC>m_mapFunction;// ������ŵ����ܵ�ӳ��
	CLockInfoDialog dlg;
	unsigned threadid;

	CCommand();
	~CCommand() {};
	int ExcuteCommand(int nCmd);
	int makeDriverInfo();
	int makeDirectoryInfo();
	int runFile();
	int downLoadFile();
	int mouseEvent();
	int sendScreen();
	static unsigned __stdcall threadLockDlg(void* arg);
	void threadLockDlgMain();
	int LockMachine();
	int UnLockMachine();
	int TestConnect();
	int DeleteLocalFile();
};

