﻿
// RemoteClientDlg.cpp: 实现文件
//

#include "pch.h"
#include "framework.h"
#include "RemoteClientDlg.h"
#include "ClientController.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

	// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
protected:
	DECLARE_MESSAGE_MAP()
public:
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CRemoteClientDlg 对话框



CRemoteClientDlg::CRemoteClientDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_REMOTECLIENT_DIALOG, pParent)
	, m_nPort(_T(""))
	, m_server_address(0)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CRemoteClientDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_PORT, m_nPort);
	DDX_IPAddress(pDX, IDC_IPADDRESS_SERV, m_server_address);
	DDX_Control(pDX, IDC_TREE_DIR, m_Tree);
	DDX_Control(pDX, IDC_LIST_FILE, m_List);
}

CString CRemoteClientDlg::GetPath(HTREEITEM hTree)
{
	CString strRet, strTmp;
	do {
		strTmp = m_Tree.GetItemText(hTree);
		strRet = strTmp + '\\' + strRet;
		hTree = m_Tree.GetParentItem(hTree);
	} while (hTree != NULL);
	return strRet;
}

void CRemoteClientDlg::DelTreeChildItem(HTREEITEM hTree)
{
	HTREEITEM hSub = NULL;
	do {
		hSub = m_Tree.GetChildItem(hTree);
		if (hSub != NULL)m_Tree.DeleteItem(hSub);
	} while (hSub != NULL);
}

void CRemoteClientDlg::LoadFileInfo()
{
	CPoint ptMouse;
	GetCursorPos(&ptMouse);
	m_Tree.ScreenToClient(&ptMouse);
	HTREEITEM hTreeSelected = m_Tree.HitTest(ptMouse, 0);
	if (!hTreeSelected)return;
	if (!m_Tree.GetChildItem(hTreeSelected))return;//双击到文件，直接退出
	DelTreeChildItem(hTreeSelected);//关闭其他未被点到的节点
	m_List.DeleteAllItems();
	CString strPath = GetPath(hTreeSelected);
	std::list<CPacket> lstPack{};
	int nCmd = CClientController::getInstance()->SendCommandPacket(GetSafeHwnd(), 2, false,
		(BYTE*)(LPCTSTR)strPath, strPath.GetLength(),(WPARAM)hTreeSelected);
	//PFILEINFO pInfo = (PFILEINFO)CClientController::getInstance()->GetPacket().strData.c_str();
	if (lstPack.size() > 0) {
		TRACE("lstPack.size=%d\r\n", lstPack.size());
		std::list<CPacket>::iterator it = lstPack.begin();
		for (; it != lstPack.end(); it++) {
			PFILEINFO pInfo = (PFILEINFO)lstPack.front().strData.c_str();
			if (pInfo->hasNext == FALSE)continue;
			if (pInfo->isDirectory){
				if (CString(pInfo->szFileName) == "." || (CString(pInfo->szFileName) == "..")) {
					continue;
				}
				HTREEITEM hTemp = m_Tree.InsertItem(pInfo->szFileName, hTreeSelected, TVI_LAST);
				//如果是目录，就有子节点;区分目录和文件，目录后面插NULL，文件不插
				m_Tree.InsertItem(NULL, hTemp, TVI_LAST);
			}
			else m_List.InsertItem(0, pInfo->szFileName);//文件不在树上显示，树上只显示目录
		}
	}
	//int ccount = 0;
	//while (pInfo->hasNext)//过滤掉空目录
	//{
	//	TRACE("[%s] isdir %d\r\n", pInfo->szFileName, pInfo->isDirectory);
	//	if (pInfo->isDirectory) {
	//		if (CString(pInfo->szFileName) == "." || (CString(pInfo->szFileName) == "..")) {
	//			//防止无限递归
	//			int cmd = CClientController::getInstance()->DealCommand();
	//			TRACE("ack:%d\r\n", cmd);
	//			if (cmd < 0)break;
	//			pInfo = (PFILEINFO)CClientController::getInstance()->GetPacket().strData.c_str();
	//			continue;
	//		}
	//		HTREEITEM hTemp = m_Tree.InsertItem(pInfo->szFileName, hTreeSelected, TVI_LAST);
	//		//如果是目录，就有子节点;区分目录和文件，目录后面插NULL，文件不插
	//		m_Tree.InsertItem(NULL, hTemp, TVI_LAST);
	//	}
	//	else m_List.InsertItem(0, pInfo->szFileName);//文件不在树上显示，树上只显示目录	
	//	ccount++;
	//	int cmd = CClientController::getInstance()->DealCommand();
	//	TRACE("ack:%d\r\n", cmd);
	//	if (cmd < 0)break;
	//	pInfo = (PFILEINFO)CClientController::getInstance()->GetPacket().strData.c_str();
	//}
	//CClientController::getInstance()->CloseSocket();
	//TRACE("ccount=%d\r\n", ccount);
}

void CRemoteClientDlg::LoadCurInfo()
{
	HTREEITEM hTree = m_Tree.GetSelectedItem();
	CString strPath = GetPath(hTree);
	m_List.DeleteAllItems();
	int nCmd = CClientController::getInstance()->SendCommandPacket(GetSafeHwnd(), 2, false, 
		(BYTE*)(LPCTSTR)strPath, strPath.GetLength());
	PFILEINFO pInfo = (PFILEINFO)CClientController::getInstance()->GetPacket().strData.c_str();
	while (pInfo->hasNext)//过滤掉空目录
	{
		TRACE("[%s] isdir %d\r\n", pInfo->szFileName, pInfo->isDirectory);
		if (!pInfo->isDirectory)
		{
			m_List.InsertItem(0, pInfo->szFileName);//文件不在树上显示，树上只显示目录
		}
		int cmd = CClientController::getInstance()->DealCommand();
		TRACE("ack:%d\r\n", cmd);
		if (cmd < 0)break;
		pInfo = (PFILEINFO)CClientController::getInstance()->GetPacket().strData.c_str();
	}
	//CClientController::getInstance()->CloseSocket();
}


BEGIN_MESSAGE_MAP(CRemoteClientDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BTN_TEST, &CRemoteClientDlg::OnBnClickedBtnTest)
	ON_BN_CLICKED(IDC_BTN_FILEINFO, &CRemoteClientDlg::OnBnClickedBtnFileinfo)
	ON_NOTIFY(NM_DBLCLK, IDC_TREE_DIR, &CRemoteClientDlg::OnNMDblclkTreeDir)
	ON_NOTIFY(NM_CLICK, IDC_TREE_DIR, &CRemoteClientDlg::OnNMClickTreeDir)
	ON_NOTIFY(NM_RCLICK, IDC_LIST_FILE, &CRemoteClientDlg::OnNMRClickListFile)
	ON_COMMAND(ID_DOWNLOAD_FILE, &CRemoteClientDlg::OnDownloadFile)
	ON_COMMAND(ID_DELETE_FILE, &CRemoteClientDlg::OnDeleteFile)
	ON_COMMAND(ID_RUN_FILE, &CRemoteClientDlg::OnRunFile)
	//ON_MESSAGE(WM_SEND_PACKET, &CRemoteClientDlg::OnSendPacket)//注册消息③：告诉系统消息对应的响应函数
	ON_BN_CLICKED(IDC_BTN_START_WATCH, &CRemoteClientDlg::OnBnClickedBtnStartWatch)
	ON_NOTIFY(IPN_FIELDCHANGED, IDC_IPADDRESS_SERV, &CRemoteClientDlg::OnIpnFieldchangedIpaddressServ)
	ON_EN_CHANGE(IDC_EDIT_PORT, &CRemoteClientDlg::OnEnChangeEditPort)
	ON_MESSAGE(WM_SEND_PACK_ACK, &CRemoteClientDlg::OnSendPackAck)//注册消息③：告诉系统消息对应的响应函数
END_MESSAGE_MAP()


// CRemoteClientDlg 消息处理程序

BOOL CRemoteClientDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != nullptr)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	// 执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	UpdateData();
	m_server_address = 0xC0A88E81;//192.168.142.129
	m_nPort = _T("9527");
	CClientController* pController = CClientController::getInstance();
	pController->UpdateAddress(m_server_address, atoi((LPCTSTR)m_nPort));
	UpdateData(FALSE);
	m_dlgStatus.Create(IDD_DIALOG_S, this);
	m_dlgStatus.ShowWindow(SW_HIDE);
	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CRemoteClientDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CRemoteClientDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CRemoteClientDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CRemoteClientDlg::OnBnClickedBtnTest()
{
	CClientController::getInstance()->SendCommandPacket(GetSafeHwnd(), 2001);
}

void CRemoteClientDlg::OnBnClickedBtnFileinfo()
{
	std::list<CPacket> lstPackets{};
	int ret = CClientController::getInstance()->SendCommandPacket(GetSafeHwnd(), 1, true, 0, 0);
	if (ret == 0)
	{
		AfxMessageBox(_T("命令处理失败!!!"));
		return;
	}
	return;
}


void CRemoteClientDlg::OnNMDblclkTreeDir(NMHDR* pNMHDR, LRESULT* pResult)
{
	*pResult = 0;
	LoadFileInfo();
}


void CRemoteClientDlg::OnNMClickTreeDir(NMHDR* pNMHDR, LRESULT* pResult)
{
	*pResult = 0;
	LoadFileInfo();
}


void CRemoteClientDlg::OnNMRClickListFile(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	CPoint ptMouse, ptList;
	GetCursorPos(&ptMouse);
	ptList = ptMouse;
	m_List.ScreenToClient(&ptList);
	int ListSelected = m_List.HitTest(ptList);
	if (ListSelected < 0)return;
	CMenu menu;
	menu.LoadMenu(IDR_MENU_RCLICK);
	CMenu* pPupup = menu.GetSubMenu(0);
	if (pPupup)
	{
		pPupup->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, ptMouse.x, ptMouse.y, this);
	}
	*pResult = 0;
}


void CRemoteClientDlg::OnDownloadFile()
{
	int nListSelected = m_List.GetSelectionMark();
	CString strFile = m_List.GetItemText(nListSelected, 0);//获取文件名
	HTREEITEM hSelected = m_Tree.GetSelectedItem();
	strFile = GetPath(hSelected) + strFile;//获取文件路径
	int ret = CClientController::getInstance()->DownFile(strFile);
	if (ret) {
		MessageBox(_T("下载失败!"));
		TRACE("下载失败 ret=%d\r\n", ret);
	}
	BeginWaitCursor();
}


void CRemoteClientDlg::OnDeleteFile()
{
	HTREEITEM hSelected = m_Tree.GetSelectedItem();
	CString strPath = GetPath(hSelected);
	int nSelected = m_List.GetSelectionMark();
	CString strFile = m_List.GetItemText(nSelected, 0);
	strFile = strPath + strFile;
	int ret = CClientController::getInstance()->SendCommandPacket(GetSafeHwnd(), 9, true, (BYTE*)(LPCSTR)strFile, strFile.GetLength());
	if (ret < 0) AfxMessageBox("删除文件命令执行失败！！！");
}


void CRemoteClientDlg::OnRunFile()
{
	HTREEITEM hSelected = m_Tree.GetSelectedItem();
	CString strPath = GetPath(hSelected);
	int nSelected = m_List.GetSelectionMark();
	CString strFile = m_List.GetItemText(nSelected, 0);
	strFile = strPath + strFile;
	int ret = CClientController::getInstance()->SendCommandPacket(GetSafeHwnd(), 3, true, (BYTE*)(LPCSTR)strFile, strFile.GetLength());
	if (ret < 0) {
		AfxMessageBox("打开文件命令执行失败！！！");
	}
	LoadCurInfo();
}
//LRESULT CRemoteClientDlg::OnSendPacket(WPARAM wParam, LPARAM lParam)
//{//实现消息响应函数④
//	int ret = 0;
//	int cmd = wParam >> 1;
//	switch (cmd)
//	{
//	case 4: {
//		CString strFile = (LPCSTR)lParam;
//		ret = CClientController::getInstance()->SendCommandPacket(cmd, wParam & 1, (BYTE*)(LPCSTR)strFile, strFile.GetLength());
//	}
//		  break;
//	case 5: {
//		ret = CClientController::getInstance()->SendCommandPacket(cmd, wParam & 1, (BYTE*)lParam, sizeof(MOUSEEV));
//	}
//		  break;
//	case 6: 
//	case 7: 
//	case 8: {
//		ret = CClientController::getInstance()->SendCommandPacket(cmd, wParam & 1);
//	}
//		  break;
//	default:
//		ret = -1;
//	}
//	return ret;
//}


void CRemoteClientDlg::OnBnClickedBtnStartWatch()
{
	CClientController::getInstance()->StartWatchScreen();
}


void CRemoteClientDlg::OnIpnFieldchangedIpaddressServ(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMIPADDRESS pIPAddr = reinterpret_cast<LPNMIPADDRESS>(pNMHDR);
	UpdateData();
	CClientController* pController = CClientController::getInstance();
	pController->UpdateAddress(m_server_address, atoi((LPCTSTR)m_nPort));
	*pResult = 0;
}


void CRemoteClientDlg::OnEnChangeEditPort()
{
	UpdateData();
	CClientController* pController = CClientController::getInstance();
	pController->UpdateAddress(m_server_address, atoi((LPCTSTR)m_nPort));
}

LRESULT CRemoteClientDlg::OnSendPackAck(WPARAM wParam, LPARAM lParam)
{
	if (lParam == -1 || (lParam == -2)) {
		//TODO:错误处理
	}
	else if (lParam == 1) {
		//对方关闭了套接字
	}
	else{
		if (wParam) {
			CPacket head = *(CPacket*)wParam;//复制一份
			delete (CPacket*)wParam;//先把指针删除
			switch (head.sCmd) {
			case 1: //获取驱动信息
			{
				std::string drivers = head.strData;
				std::string dr;
				m_Tree.DeleteAllItems();
				for (size_t i = 0; i < drivers.size(); i++)
				{
					if (drivers[i] != ',') {
						dr = drivers[i];
						dr += ":";
						HTREEITEM hTemp = m_Tree.InsertItem(dr.c_str(), TVI_ROOT, TVI_LAST);
						//如果是目录，就有子节点
						m_Tree.InsertItem(NULL, hTemp, TVI_LAST);
						dr.clear();
						continue;
					}
					dr += drivers[i];
				}
				if (dr.size() > 0) {
					dr += ":";
					HTREEITEM hTemp = m_Tree.InsertItem(dr.c_str(), TVI_ROOT, TVI_LAST);
					m_Tree.InsertItem(NULL, hTemp, TVI_LAST);
				}
			}
				break;
			case 2://获取文件信息
			{
				PFILEINFO pInfo = (PFILEINFO)head.strData.c_str();
				if (pInfo->hasNext == FALSE)break;
				if (pInfo->isDirectory) {
					if (CString(pInfo->szFileName) == "." || (CString(pInfo->szFileName) == ".."))break;
					HTREEITEM hTemp = m_Tree.InsertItem(pInfo->szFileName, (HTREEITEM)lParam, TVI_LAST);
					//如果是目录，就有子节点;区分目录和文件，目录后面插NULL，文件不插
					m_Tree.InsertItem(NULL, hTemp, TVI_LAST);
				}
				else m_List.InsertItem(0, pInfo->szFileName);//文件不在树上显示，树上只显示目录
			}
			break;
			case 3:
				TRACE("run file done!\r\n");
				break;
			case 4:
			{
				static LONGLONG length = 0,index=0;
				if (!length) {
					length = *(long long*)head.strData.c_str();
					if (!length) {
						AfxMessageBox("文件长度为零或者无法读取文件！！！");
						CClientController::getInstance()->DownloadEnd();
					}
				}
				else if (length > 0 && (index >= length)) {
					fclose((FILE*)lParam);
					length = 0, index = 0;
					CClientController::getInstance()->DownloadEnd();
				}
				else {
					FILE* pFile = (FILE*)lParam;
					fwrite(head.strData.c_str(), 1,head.strData.size(),  pFile);
					index += head.strData.size();
				}
			}
			break;
			case 9:
				TRACE("delete file done!\r\n");
				break;
			case 2001:
				TRACE("test connection success!\r\n");
				break;
			default:
				TRACE("unknow data received! %d\r\n", head.sCmd);
				break;
			}
		}
	}
	return 0;
}
