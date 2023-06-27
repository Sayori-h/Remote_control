// RemoteCtrl.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include "pch.h"
#include "framework.h"
#include "RemoteCtrl.h"
#include "CServerSocket.h"
#include <direct.h>
#include <io.h>
#include <list>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// 唯一的应用程序对象

CWinApp theApp;

void dump(BYTE* pData, size_t nSize) {//?
	std::string strOut;
	for (size_t i = 0; i < nSize; i++)
	{
		char buf[16] = "";
		if (i>0&&(i%16==0))
		{
			strOut += "\n";
		}
		snprintf(buf, sizeof(buf), "%02X ", pData[i]&0xFF );
		strOut += buf;
	}
	strOut += "\n";
	OutputDebugStringA(strOut.c_str());
}

int makeDriverInfo() {
	std::string res;
	for (int i = 1; i <= 26; i++)
	{
		if (!_chdrive(i))
		{
			if (res.size())
			{
				res += ',';
			}
			res += 'A' + i - 1;
		}
	}
	CPacket pack(1, (BYTE*)res.c_str(), res.size());//打包用的
	//dump((BYTE*)&pack, pack.nLength + 6);//原来误把strData的地址导出
	dump((BYTE*)pack.pacData(), pack.pacSize());
	//CServerSocket::getInstance()->sendCom(pack);
	return 0;
}

typedef struct file_info {
	file_info() {
		isInvalid = 0;
		isDirectory = 0;
		hasNext = TRUE;
		memset(szFileName, 0, sizeof(szFileName));
	}
	BOOL isInvalid;//是否有效 1无效
	BOOL isDirectory;//是否为目录 1是
	BOOL hasNext;//是否有后续  1有
	char szFileName[256];//文件名
}FILEINFO,*PFILEINFO;

int makeDirectoryInfo() {
	std::string strPath;
	//std::list<FILEINFO> lstFileInfos;
	if (gpServer->getFilePath(strPath)==false)
	{
		OutputDebugString(_T("当前的命令，不是获取文件列表，命令错误!"));
		return -1;
	}
	if (!_chdir(strPath.c_str()))
	{
		FILEINFO finfo;
		finfo.isInvalid = TRUE;
		finfo.isDirectory = TRUE;
		finfo.hasNext = FALSE;
		memcpy(finfo.szFileName, strPath.c_str(), strPath.size());
		//lstFileInfos.push_back(finfo);
		CPacket pack(2, (BYTE*)&finfo, sizeof(finfo));
		gpServer->sendCom(pack);
		OutputDebugString(_T("没有权限访问目录!"));
		return -2;
	}
	_finddata_t fdata;
	int hFind = 0;
	if ((hFind=_findfirst("*",&fdata))==-1)
	{
		OutputDebugString(_T("没有找到任何文件!"));
		return -3;
	}
	do
	{
		FILEINFO finfo;
		finfo.isDirectory = (fdata.attrib & _A_SUBDIR) != 0;//比较文件类型是否是文件夹 attrib
		memcpy(finfo.szFileName, fdata.name, strlen(fdata.name));
		//lstFileInfos.push_back(finfo);
		CPacket pack(2, (BYTE*)&finfo, sizeof(finfo));
		gpServer->sendCom(pack);
	} while (!_findnext(hFind,&fdata));
	//发送信息到控制端
	FILEINFO finfo;
	finfo.hasNext = FALSE;
	CPacket pack(2, (BYTE*)&finfo, sizeof(finfo));
	gpServer->sendCom(pack);
	return 0;
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
			// TODO: 在此处为应用程序的行为编写代码。
			wprintf(L"错误: MFC 初始化失败\n");
			nRetCode = 1;
		}
		else
		{
			/*int count = 0;
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
				int ret = gpServer->dealCommand();
				TODO:处理命令
			}*/
			int nCmd = 1;
			switch (nCmd)
			{
			case 1://查看磁盘分区
				makeDriverInfo();
				break;
			case 2://查看指定目录下的文件
				makeDirectoryInfo();
				break;
			default:
				break;
			}
		}
	}
	else
	{
		// TODO: 更改错误代码以符合需要
		wprintf(L"错误: GetModuleHandle 失败\n");
		nRetCode = 1;
	}

	return nRetCode;
}
