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
