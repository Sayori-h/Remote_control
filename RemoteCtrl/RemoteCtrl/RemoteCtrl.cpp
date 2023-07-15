// RemoteCtrl.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include "pch.h"
#include "framework.h"
#include "RemoteCtrl.h"
#include "CServerSocket.h"
#include <direct.h>
#include <io.h>
#include <list>
#include <atlimage.h>
#include "LockInfoDialog.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// 唯一的应用程序对象

CWinApp theApp;


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
	//dump((BYTE*)pack.pacData(), pack.pacSize());
	gpServer->sendCom(pack);
	return 0;
}



int makeDirectoryInfo() {
	std::string strPath;
	//std::list<FILEINFO> lstFileInfos;
	if (gpServer->getFilePath(strPath) == false)
	{
		OutputDebugString(_T("当前的命令，不是获取文件列表，命令错误!"));
		return -1;
	}
	if (_chdir(strPath.c_str()))
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
	if ((hFind = _findfirst("*", &fdata)) == -1)
	{
		FILEINFO finfo;
		finfo.hasNext = FALSE;
		CPacket pack(2, (BYTE*)&finfo, sizeof(finfo));
		gpServer->sendCom(pack);
		OutputDebugString(_T("没有找到任何文件!"));
		return -3;
	}
	int scount = 0;
	do
	{
		FILEINFO finfo;
		finfo.isDirectory = (fdata.attrib & _A_SUBDIR) != 0;//比较文件类型是否是文件夹 attrib
		memcpy(finfo.szFileName, fdata.name, strlen(fdata.name));
		//lstFileInfos.push_back(finfo);
		CPacket pack(2, (BYTE*)&finfo, sizeof(finfo));
		gpServer->sendCom(pack);
		scount++;
	} while (!_findnext(hFind, &fdata));
	TRACE("server:count=%d\r\n", scount);
	//发送信息到控制端
	FILEINFO finfo;
	finfo.hasNext = FALSE;
	CPacket pack(2, (BYTE*)&finfo, sizeof(finfo));
	gpServer->sendCom(pack);
	return 0;
}

int runFile() {
	std::string strPath;
	gpServer->getFilePath(strPath);
	ShellExecuteA(NULL, NULL, strPath.c_str(), NULL, NULL, SW_SHOWNORMAL);//等于双击打开文件
	CPacket pack(3, NULL, 0);
	gpServer->sendCom(pack);
	return 0;
}

int downLoadFile() {
	std::string strPath;
	gpServer->getFilePath(strPath);
	long long data = 0;
	FILE* pFile = fopen(strPath.c_str(), "rb");
	if (pFile == NULL)
	{
		CPacket pack(4, (BYTE*)&data, 8);
		gpServer->sendCom(pack);
		return -1;
	}
	fseek(pFile, 0, SEEK_END);
	data = _ftelli64(pFile);
	CPacket head(4, (BYTE*)&data, 8);//通过8个字节拿到文件的长度
	gpServer->sendCom(head);
	fseek(pFile, 0, SEEK_SET);//恢复到文件头
	char buffer[2048] = "";
	size_t rlen = 0;
	do
	{
		rlen = fread(buffer, 1, 2048, pFile);
		CPacket pack(4, (BYTE*)buffer, rlen);//读1K发1K
		gpServer->sendCom(pack);
	} while (rlen >= 1024);
	CPacket pack(4, NULL, 0);//到头了，发送终止符
	gpServer->sendCom(pack);
	fclose(pFile);
	return 0;
}

int mouseEvent() {
	MOUSEEV mouse;
	if (gpServer->getMouseEvent(mouse))
	{
		//SetCursorPos(mouse.ptXY.x, mouse.ptXY.y);
		DWORD nFlags = 0;
		switch (mouse.nButton)
		{
		case 0://left
			nFlags = 1;
			break;
		case 1://right
			nFlags = 2;
			break;
		case 2://mid
			nFlags = 4;
			break;
		case 3://only move,no click
			nFlags = 8;
			break;
		default:
			break;
		}
		if (nFlags != 8)SetCursorPos(mouse.ptXY.x, mouse.ptXY.y);
		switch (mouse.nAction)
		{
		case 0://click
			nFlags |= 0x10;
			break;
		case 1://double click
			nFlags |= 0x20;
			break;
		case 2://down
			nFlags |= 0x40;
			break;
		case 3://up
			nFlags |= 0x80;
			break;
		default:
			break;
		}
		TRACE("mouse event:%08X pos=(%d,%d)\r\n", nFlags, mouse.ptXY.x, mouse.ptXY.y);
		switch (nFlags)//避免嵌套，把情况单独列出来
		{
		case 0x21://left double click
			mouse_event(MOUSEEVENTF_LEFTDOWN | MOUSEEVENTF_LEFTUP, 0, 0, 0, GetMessageExtraInfo());
		case 0x11://left click
			mouse_event(MOUSEEVENTF_LEFTDOWN | MOUSEEVENTF_LEFTUP, 0, 0, 0, GetMessageExtraInfo());
			break;
		case 0x41://left down
			mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, GetMessageExtraInfo());
			break;
		case 0x81://left up
			mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, GetMessageExtraInfo());
			break;
		case 0x22://right double click
			mouse_event(MOUSEEVENTF_RIGHTDOWN | MOUSEEVENTF_RIGHTUP, 0, 0, 0, GetMessageExtraInfo());
		case 0x12://right click
			mouse_event(MOUSEEVENTF_RIGHTDOWN | MOUSEEVENTF_RIGHTUP, 0, 0, 0, GetMessageExtraInfo());
			break;
		case 0x42://right down
			mouse_event(MOUSEEVENTF_RIGHTDOWN, 0, 0, 0, GetMessageExtraInfo());
			break;
		case 0x82://rigth up
			mouse_event(MOUSEEVENTF_RIGHTUP, 0, 0, 0, GetMessageExtraInfo());
			break;
		case 0x14://mid click
			mouse_event(MOUSEEVENTF_MIDDLEDOWN | MOUSEEVENTF_MIDDLEUP, 0, 0, 0, GetMessageExtraInfo());
			break;
		case 0x24://mid Roll
			mouse_event(MOUSEEVENTF_WHEEL, 0, 0, 0, GetMessageExtraInfo());
			break;
		case 0x44://mid down
			mouse_event(MOUSEEVENTF_MIDDLEDOWN, 0, 0, 0, GetMessageExtraInfo());
			break;
		case 0x84://mid up
			mouse_event(MOUSEEVENTF_MIDDLEUP, 0, 0, 0, GetMessageExtraInfo());
			break;
		case 0x08://only move,no click
			mouse_event(MOUSEEVENTF_MOVE, mouse.ptXY.x, mouse.ptXY.y, 0, GetMessageExtraInfo());
			break;
		default:
			break;
		}
		CPacket pack(5, NULL, 0);
		gpServer->sendCom(pack);
	}
	else
	{
		OutputDebugString(_T("获取鼠标参数失败!!"));
		return -1;
	}
	return 0;
}

int sendScreen() {
	HDC hScreen = ::GetDC(NULL);
	//?个字节表示一个像素; ARGB8888 32bit;RGB888 24bit;RGB 565 16bit;RGB 444;
	int nBitPerPixel = GetDeviceCaps(hScreen, BITSPIXEL);
	int nWidth = GetDeviceCaps(hScreen, HORZRES);
	int nHeight = GetDeviceCaps(hScreen, VERTRES);
	CImage screen;
	screen.Create(nWidth, nHeight, nBitPerPixel);//为图像类创建与窗口DC相同大小的DC
	BitBlt(screen.GetDC(), 0, 0, 1920, 1020/*跳过任务栏高度*/, hScreen, 0, 0, SRCCOPY);//将窗口DC图像复制到image
	ReleaseDC(NULL, hScreen);//释放DC资源<=>GetDC
	HGLOBAL hMen = GlobalAlloc(GMEM_MOVEABLE, 0);
	if (hMen == NULL)return -1;
	IStream* pStream = NULL;
	HRESULT ret = CreateStreamOnHGlobal(hMen, TRUE, &pStream);
	if (ret == S_OK)
	{
		screen.Save(pStream, Gdiplus::ImageFormatPNG);//保存为png格式图片文件,仅保存到文件夹，需要保存到内存
		LARGE_INTEGER bg{ 0 };
		pStream->Seek(bg, STREAM_SEEK_SET, NULL);
		PBYTE pData = (PBYTE)GlobalLock(hMen);
		size_t nSize = GlobalSize(hMen);
		CPacket pack(6, pData, nSize);

		gpServer->sendCom(pack);
		GlobalUnlock(hMen);
	}
	pStream->Release();
	GlobalFree(hMen);//<=>GlobalAlloc
	screen.ReleaseDC();//释放DC<=>Creat
	return 0;
}

CLockInfoDialog dlg;
unsigned threadid = 0;

unsigned __stdcall threadLockDlg(void* arg) {
	TRACE("%s(%d):%d\r\n", __FUNCTION__, __LINE__, GetCurrentThreadId());
	dlg.Create(IDD_DIALOG_INFO, NULL);
	dlg.ShowWindow(SW_SHOW);
	//遮蔽后台窗口
	CRect rect;
	rect.left = 0;
	rect.top = 0;
	rect.right = GetSystemMetrics(SM_CXFULLSCREEN);
	rect.bottom = GetSystemMetrics(SM_CYFULLSCREEN);
	rect.bottom = LONG(rect.bottom * 1.1);
	TRACE("right =%d bottom =%d\r\n", rect.right, rect.bottom);
		//该函数改变指定窗口的位置和尺寸。对于顶层窗口，位置和尺寸是相对于屏幕的左上角的：对于子窗口，位置和尺寸是相对于父窗口客户区的左上角坐标的
		dlg.MoveWindow(rect);
	CWnd* pText = dlg.GetDlgItem(IDC_STATIC);
	if (pText) {
		CRect rtText;
		pText->GetWindowRect(rtText);
		int nWidth = rtText.Width();//w0
		int x = (rect.right - nWidth) / 2;
		int nHeight = rtText.Height();
		int y = (rect.bottom - nHeight) / 2;
		pText->MoveWindow(x, y, rtText.Width(), rtText.Height());
	}
	dlg.SetWindowPos(&dlg.wndTopMost, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);//窗口置顶
	ShowCursor(false);//隐藏鼠标
	::ShowWindow(::FindWindow(_T("Shell_TrayWnd"), NULL), SW_HIDE);//隐藏windows任务栏
	//GetWindowRect得到的是整个窗口相对于窗口左上角的坐标，实际上就是这个对话框的大小
	dlg.GetWindowRect(rect);
	/*该函数把鼠标限制在屏幕上的一个矩形区域内，如果调用SetCursor或用鼠标设置的一个随后的鼠标位置在该矩形区域的外面，
	则系统自动调整该位置以保持鼠标在矩形区域之内。*/
	rect.left = 0;
	rect.top = 0;
	rect.right = 1;
	rect.bottom = 1;
	ClipCursor(rect);
	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0)) {//死循环，放在子线程里
		TranslateMessage(&msg);
		DispatchMessage(&msg);
		if (msg.message == WM_KEYDOWN) {
			TRACE("msg:%08x wparam:%08x lparam:%08x\r\n", msg.message, msg.wParam, msg.lParam);
			if (msg.wParam == 0x1B) {
				break;
			}
		}
	}
	ClipCursor(NULL);
	ShowCursor(true);
	::ShowWindow(::FindWindow(_T("Shell_TrayWnd"), NULL), SW_SHOW);//恢复windows任务栏
	dlg.DestroyWindow();
	_endthreadex(0);
	return 0;
}
int LockMachine() {
	//防止多次调用LockMachine
	if ((dlg.m_hWnd == NULL) || (dlg.m_hWnd == INVALID_HANDLE_VALUE)) {
		_beginthreadex(NULL, 0, threadLockDlg, NULL, 0, &threadid);
		TRACE("threadId=%d\r\n", threadid);
	}
	CPacket pack(7, NULL, 0);
	gpServer->sendCom(pack);
	return 0;
}

int UnLockMachine() {
	//dlg.SendMessage(WM_KEYDOWN, 0x1B, 0x00010001);//对话框
	//::SendMessage(dlg.m_hWnd, WM_KEYDOWN, 0x1B, 0x00010001);//窗口句柄
	//MFC消息机制是基于线程的（CWinThread类），没有绑定线程发消息没用，必须往线程里发消息
	PostThreadMessage(threadid, WM_KEYDOWN, 0x1B, 0);
	CPacket pack(8, NULL, 0);
	gpServer->sendCom(pack);
	return 0;
}

int DeleteLocalFile() {
	std::string strPath;
	CServerSocket::getInstance()->getFilePath(strPath);
	TCHAR sPath[MAX_PATH] = _T("");
	//多字符转换为宽字符,中文容易乱码
	//mbstowcs(sPath, strPath.c_str(), strPath.size());
	MultiByteToWideChar(
		CP_ACP, 0, strPath.c_str(), strPath.size(), sPath,
		sizeof(sPath) / sizeof(TCHAR));
	DeleteFileA(strPath.c_str());
	CPacket pack(9, NULL, 0);
	bool ret = CServerSocket::getInstance()->sendCom(pack);
	TRACE("Send ret = % d\r\n", ret);
	return 0;
}

int TestConnect() {
	CPacket pack(2001, NULL, 0);
	bool ret = gpServer->sendCom(pack);
	TRACE("send ret=%d\r\n", ret);
	return 0;
}

int ExcuteCommand(int nCmd) {
	int ret = 0;
	switch (nCmd)
	{
	case 1://查看磁盘分区
		ret = makeDriverInfo();
		break;
	case 2://查看指定目录下的文件
		ret = makeDirectoryInfo();
		break;
	case 3://打开文件
		ret = runFile();
		break;
	case 4://控制端下载文件
		ret = downLoadFile();
		break;
	case 5://鼠标操作
		ret = mouseEvent();
		break;
	case 6://发送屏幕的内容=>发送屏幕截图
		ret = sendScreen();
		break;
	case 7://锁机
		ret = LockMachine();
		break;
	case 8://解锁
		ret = UnLockMachine();
		break;
	case 9://删除文件
		ret = DeleteLocalFile();
		break;
	case 2001://测试连接
		ret = TestConnect();
	}
	return ret;
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
					ret = ExcuteCommand(ret);
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
