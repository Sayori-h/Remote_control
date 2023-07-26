#include "pch.h"
#include "Command.h"


CCommand::CCommand() :threadid(0)
{
	struct {
		int nCmd;
		CMDFUNC func;//����ָ��
	}data[] = {
		{ 1   ,&CCommand::makeDriverInfo },
		{ 2   ,&CCommand::makeDirectoryInfo },
		{ 3   ,&CCommand::runFile },
		{ 4   ,&CCommand::downLoadFile },
		{ 5   ,&CCommand::mouseEvent },
		{ 6   ,&CCommand::sendScreen },
		{ 7   ,&CCommand::LockMachine },
		{ 8   ,&CCommand::UnLockMachine },
		{ 9   ,&CCommand::DeleteLocalFile },
		{ 2001,&CCommand::TestConnect },
		{ -1  ,NULL }
	};
	for (int i = 0; data[i].nCmd != -1; i++) {
		m_mapFunction.insert(std::pair<int, CMDFUNC>(data[i].nCmd, data[i].func));
	}
}

int CCommand::ExcuteCommand(int nCmd)
{
	std::map<int, CMDFUNC>::iterator it = m_mapFunction.find(nCmd);
	
	if (it == m_mapFunction.end())return -1;
	return(this->*it->second)();
}

int CCommand::makeDriverInfo() {
	std::string res;
	for (int i = 1; i <= 26; i++)
	{	if (!_chdrive(i)){
			if (res.size())res += ',';
			res += 'A' + i - 1;
		}
	}
	CPacket pack(1, (BYTE*)res.c_str(), res.size());//����õ�
	//CHuxlTool::dump((BYTE*)&pack, pack.nLength + 6);//ԭ�����strData�ĵ�ַ����
	//CHuxlTool::dump((BYTE*)pack.pacData(), pack.pacSize());
	gpServer->sendCom(pack);
	return 0;
}

int CCommand::makeDirectoryInfo() {
	std::string strPath;
	//std::list<FILEINFO> lstFileInfos;
	if (gpServer->getFilePath(strPath) == false)
	{
		OutputDebugString(_T("��ǰ��������ǻ�ȡ�ļ��б��������!"));
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
		OutputDebugString(_T("û��Ȩ�޷���Ŀ¼!"));
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
		OutputDebugString(_T("û���ҵ��κ��ļ�!"));
		return -3;
	}
	int scount = 0;
	do
	{
		FILEINFO finfo;
		finfo.isDirectory = (fdata.attrib & _A_SUBDIR) != 0;//�Ƚ��ļ������Ƿ����ļ��� attrib
		memcpy(finfo.szFileName, fdata.name, strlen(fdata.name));
		//lstFileInfos.push_back(finfo);
		CPacket pack(2, (BYTE*)&finfo, sizeof(finfo));
		gpServer->sendCom(pack);
		scount++;
	} while (!_findnext(hFind, &fdata));
	TRACE("server:count=%d\r\n", scount);
	//������Ϣ�����ƶ�
	FILEINFO finfo;
	finfo.hasNext = FALSE;
	CPacket pack(2, (BYTE*)&finfo, sizeof(finfo));
	gpServer->sendCom(pack);
	return 0;
}

int CCommand::runFile() {
	std::string strPath;
	gpServer->getFilePath(strPath);
	ShellExecuteA(NULL, NULL, strPath.c_str(), NULL, NULL, SW_SHOWNORMAL);//����˫�����ļ�
	CPacket pack(3, NULL, 0);
	gpServer->sendCom(pack);
	return 0;
}

int CCommand::downLoadFile() {
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
	CPacket head(4, (BYTE*)&data, 8);//ͨ��8���ֽ��õ��ļ��ĳ���
	gpServer->sendCom(head);
	fseek(pFile, 0, SEEK_SET);//�ָ����ļ�ͷ
	char buffer[2048] = "";
	size_t rlen = 0;
	do
	{
		rlen = fread(buffer, 1, 2048, pFile);
		CPacket pack(4, (BYTE*)buffer, rlen);//��1K��1K
		gpServer->sendCom(pack);
	} while (rlen >= 1024);
	CPacket pack(4, NULL, 0);//��ͷ�ˣ�������ֹ��
	gpServer->sendCom(pack);
	fclose(pFile);
	return 0;
}

int CCommand::mouseEvent() {
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
		switch (nFlags)//����Ƕ�ף�����������г���
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
		OutputDebugString(_T("��ȡ������ʧ��!!"));
		return -1;
	}
	return 0;
}

int CCommand::sendScreen() {
	HDC hScreen = ::GetDC(NULL);
	//?���ֽڱ�ʾһ������; ARGB8888 32bit;RGB888 24bit;RGB 565 16bit;RGB 444;
	int nBitPerPixel = GetDeviceCaps(hScreen, BITSPIXEL);
	int nWidth = GetDeviceCaps(hScreen, HORZRES);
	int nHeight = GetDeviceCaps(hScreen, VERTRES);
	CImage screen;
	screen.Create(nWidth, nHeight, nBitPerPixel);//Ϊͼ���ഴ���봰��DC��ͬ��С��DC
	BitBlt(screen.GetDC(), 0, 0, 1920, 1020/*�����������߶�*/, hScreen, 0, 0, SRCCOPY);//������DCͼ���Ƶ�image
	ReleaseDC(NULL, hScreen);//�ͷ�DC��Դ<=>GetDC
	HGLOBAL hMen = GlobalAlloc(GMEM_MOVEABLE, 0);
	if (hMen == NULL)return -1;
	IStream* pStream = NULL;
	HRESULT ret = CreateStreamOnHGlobal(hMen, TRUE, &pStream);
	if (ret == S_OK)
	{
		screen.Save(pStream, Gdiplus::ImageFormatPNG);//����Ϊpng��ʽͼƬ�ļ�,�����浽�ļ��У���Ҫ���浽�ڴ�
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
	screen.ReleaseDC();//�ͷ�DC<=>Creat
	return 0;
}

unsigned __stdcall CCommand::threadLockDlg(void* arg)
{
	CCommand* thiz = (CCommand*)arg;
	thiz->threadLockDlgMain();
	_endthreadex(0);
	return 0;
}

void CCommand::threadLockDlgMain() {
	TRACE("%s(%d):%d\r\n", __FUNCTION__, __LINE__, GetCurrentThreadId());
	dlg.Create(IDD_DIALOG_INFO, NULL);
	dlg.ShowWindow(SW_SHOW);
	//�ڱκ�̨����
	CRect rect;
	rect.left = 0;
	rect.top = 0;
	rect.right = GetSystemMetrics(SM_CXFULLSCREEN);
	rect.bottom = GetSystemMetrics(SM_CYFULLSCREEN);
	rect.bottom = LONG(rect.bottom * 1.1);
	TRACE("right =%d bottom =%d\r\n", rect.right, rect.bottom);
	//�ú����ı�ָ�����ڵ�λ�úͳߴ硣���ڶ��㴰�ڣ�λ�úͳߴ����������Ļ�����Ͻǵģ������Ӵ��ڣ�λ�úͳߴ�������ڸ����ڿͻ��������Ͻ������
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
	dlg.SetWindowPos(&dlg.wndTopMost, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);//�����ö�
	ShowCursor(false);//�������
	::ShowWindow(::FindWindow(_T("Shell_TrayWnd"), NULL), SW_HIDE);//����windows������
	//GetWindowRect�õ�����������������ڴ������Ͻǵ����꣬ʵ���Ͼ�������Ի���Ĵ�С
	dlg.GetWindowRect(rect);
	/*�ú����������������Ļ�ϵ�һ�����������ڣ��������SetCursor����������õ�һ���������λ���ڸþ�����������棬
	��ϵͳ�Զ�������λ���Ա�������ھ�������֮�ڡ�*/
	rect.left = 0;
	rect.top = 0;
	rect.right = 1;
	rect.bottom = 1;
	ClipCursor(rect);
	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0)) {//��ѭ�����������߳���
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
	::ShowWindow(::FindWindow(_T("Shell_TrayWnd"), NULL), SW_SHOW);//�ָ�windows������
	dlg.DestroyWindow();
}

int CCommand::LockMachine() {
	//��ֹ��ε���LockMachine
	if ((dlg.m_hWnd == NULL) || (dlg.m_hWnd == INVALID_HANDLE_VALUE)) {
		_beginthreadex(NULL, 0, &CCommand::threadLockDlg, this, 0, &threadid);
		TRACE("threadId=%d\r\n", threadid);
	}
	CPacket pack(7, NULL, 0);
	gpServer->sendCom(pack);
	return 0;
}

int CCommand::UnLockMachine() {
	//dlg.SendMessage(WM_KEYDOWN, 0x1B, 0x00010001);//�Ի���
	//::SendMessage(dlg.m_hWnd, WM_KEYDOWN, 0x1B, 0x00010001);//���ھ��
	//MFC��Ϣ�����ǻ����̵߳ģ�CWinThread�ࣩ��û�а��̷߳���Ϣû�ã��������߳��﷢��Ϣ
	PostThreadMessage(threadid, WM_KEYDOWN, 0x1B, 0);
	CPacket pack(8, NULL, 0);
	gpServer->sendCom(pack);
	return 0;
}

int CCommand::TestConnect() {
	CPacket pack(2001, NULL, 0);
	bool ret = gpServer->sendCom(pack);
	TRACE("send ret=%d\r\n", ret);
	return 0;
}

int CCommand::DeleteLocalFile() {
	std::string strPath;
	CServerSocket::getInstance()->getFilePath(strPath);
	TCHAR sPath[MAX_PATH] = _T("");
	//���ַ�ת��Ϊ���ַ�,������������
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

