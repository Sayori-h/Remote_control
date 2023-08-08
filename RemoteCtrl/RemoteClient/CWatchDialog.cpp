﻿// CWatchDialog.cpp: 实现文件
//

#include "pch.h"
#include "RemoteClient.h"
#include "CWatchDialog.h"
#include "afxdialogex.h"
#include "ClientController.h"


// CWatchDialog 对话框

IMPLEMENT_DYNAMIC(CWatchDialog, CDialog)

CWatchDialog::CWatchDialog(CWnd* pParent /*=nullptr*/)
	: CDialog(IDD_DLG_WATCH, pParent)
{
	m_isFull = false;
	m_nObjWidth = -1;
	m_nObjHeight = -1;
}

CWatchDialog::~CWatchDialog()
{
}

void CWatchDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_WATCH, m_picture);
}


BEGIN_MESSAGE_MAP(CWatchDialog, CDialog)
	ON_WM_TIMER()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_RBUTTONDBLCLK()
	ON_WM_RBUTTONDOWN()
	ON_WM_RBUTTONUP()
	//	ON_WM_MOVE()
	ON_STN_CLICKED(IDC_WATCH, &CWatchDialog::OnStnClickedWatch)
	//	ON_WM_MOVE()
	ON_WM_MOUSEMOVE()
	ON_BN_CLICKED(IDC_BTN_LOCK, &CWatchDialog::OnBnClickedBtnLock)
	ON_BN_CLICKED(IDC_BTN_UNLOCK, &CWatchDialog::OnBnClickedBtnUnlock)
	ON_MESSAGE(WM_SEND_PACK_ACK, &CWatchDialog::OnSendPackAck)
END_MESSAGE_MAP()


// CWatchDialog 消息处理程序


CPoint CWatchDialog::UserPoint2RemoteScreenPoint(CPoint& point, bool isScreen)
{//800*450
	CRect clientRect;
	if (!isScreen)ClientToScreen(&point);//换为相对屏幕左上角的坐标（屏幕内的绝对坐标)
	m_picture.ScreenToClient(&point);//全局坐标转换为客户区域坐标（相对picture控件左上角的坐标）
	TRACE("pos=(%d,%d)\r\n", point.x, point.y);
	m_picture.GetWindowRect(clientRect);//本地客户坐标到远程坐标
	TRACE("client_size=(%d,%d)\r\n", clientRect.Width(), clientRect.Height());
	int width0 = clientRect.Width();
	int height0 = clientRect.Height();
	int width = 1920, height = 1080;
	int x = point.x * width / width0;
	int y = point.y * height / height0;
	return CPoint(x, y);
}

BOOL CWatchDialog::OnInitDialog()
{
	CDialog::OnInitDialog();
	m_isFull = false;
	//SetTimer(0, 40, NULL);
	return TRUE;  // return TRUE unless you set the focus to a control 异常: OCX 属性页应返回 FALSE

}


void CWatchDialog::OnTimer(UINT_PTR nIDEvent)
{
	/*if (!nIDEvent) {
		CClientController* pParent = CClientController::getInstance();
		if (m_isFull)
		{
			CRect rect;
			m_picture.GetWindowRect(rect);
			m_nObjWidth = m_image.GetWidth();
			m_nObjHeight = m_image.GetHeight();
			m_image.StretchBlt(m_picture.GetDC()->GetSafeHdc(), 0, 0,
				rect.Width(), rect.Height(), SRCCOPY);
			m_picture.InvalidateRect(NULL);
			TRACE("更新图片完成%d %d %08X\r\n", m_nObjWidth, m_nObjHeight, (HBITMAP)m_image);
			m_image.Destroy();
			m_isFull = false;
		}
	}*/
	CDialog::OnTimer(nIDEvent);
}


void CWatchDialog::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	if ((m_nObjWidth != -1) && (m_nObjHeight != -1)) {
		//坐标转换
		CPoint remote = UserPoint2RemoteScreenPoint(point);
		//封装
		MOUSEEV event;
		event.ptXY = remote;
		event.nButton = 0;//left
		event.nAction = 1;//double click
		CClientController::getInstance()->SendCommandPacket(GetSafeHwnd(),5, true, (BYTE*)&event, sizeof(event));
		//CRemoteClientDlg* pParent = (CRemoteClientDlg*)GetParent(); // TODO:存在一个设计隐患,网络通信和对话框有藕合
		//pParent->SendMessage(WM_SEND_PACKET, 5 << 1 | 1, (WPARAM) & event);
	}
	CDialog::OnLButtonDblClk(nFlags, point);
}


void CWatchDialog::OnLButtonDown(UINT nFlags, CPoint point)
{
	if ((m_nObjWidth != -1) && (m_nObjHeight != -1)) {
		TRACE("old_pos=(%d,%d)\r\n", point.x, point.y);
		//坐标转换
		CPoint remote = UserPoint2RemoteScreenPoint(point);
		TRACE("new_pos=(%d,%d)\r\n", point.x, point.y);
		//封装
		MOUSEEV event;
		event.ptXY = remote;
		event.nButton = 0;//left
		event.nAction = 2;//down
		//CRemoteClientDlg* pParent = (CRemoteClientDlg*)GetParent();
		//pParent->SendMessage(WM_SEND_PACKET, 5 << 1 | 1, (WPARAM) & event);
		CClientController::getInstance()->SendCommandPacket(GetSafeHwnd(), 5, true, (BYTE*)&event, sizeof(event));
	}
	CDialog::OnLButtonDown(nFlags, point);
}


void CWatchDialog::OnLButtonUp(UINT nFlags, CPoint point)
{
	if ((m_nObjWidth != -1) && (m_nObjHeight != -1)) {
		//坐标转换
		CPoint remote = UserPoint2RemoteScreenPoint(point);
		//封装
		MOUSEEV event;
		event.ptXY = remote;
		event.nButton = 0;//left
		event.nAction = 3;//up
		//CRemoteClientDlg* pParent = (CRemoteClientDlg*)GetParent();
		//pParent->SendMessage(WM_SEND_PACKET, 5 << 1 | 1, (WPARAM) & event);
		CClientController::getInstance()->SendCommandPacket(GetSafeHwnd(), 5, true, (BYTE*)&event, sizeof(event));
	}
	CDialog::OnLButtonUp(nFlags, point);
}


void CWatchDialog::OnRButtonDblClk(UINT nFlags, CPoint point)
{
	if ((m_nObjWidth != -1) && (m_nObjHeight != -1)) {
		//坐标转换
		CPoint remote = UserPoint2RemoteScreenPoint(point);
		//封装
		MOUSEEV event;
		event.ptXY = remote;
		event.nButton = 1;//right
		event.nAction = 1;//double click
		//CRemoteClientDlg* pParent = (CRemoteClientDlg*)GetParent();
		//pParent->SendMessage(WM_SEND_PACKET, 5 << 1 | 1, (WPARAM) & event);
		CClientController::getInstance()->SendCommandPacket(GetSafeHwnd(), 5, true, (BYTE*)&event, sizeof(event));
	}
	CDialog::OnRButtonDblClk(nFlags, point);
}


void CWatchDialog::OnRButtonDown(UINT nFlags, CPoint point)
{
	if ((m_nObjWidth != -1) && (m_nObjHeight != -1)) {
		//坐标转换
		CPoint remote = UserPoint2RemoteScreenPoint(point);
		//封装
		MOUSEEV event;
		event.ptXY = remote;
		event.nButton = 1;//right
		event.nAction = 2;//down
		//CRemoteClientDlg* pParent = (CRemoteClientDlg*)GetParent();
		//pParent->SendMessage(WM_SEND_PACKET, 5 << 1 | 1, (WPARAM) & event);
		CClientController::getInstance()->SendCommandPacket(GetSafeHwnd(), 5, true, (BYTE*)&event, sizeof(event));
	}
	CDialog::OnRButtonDown(nFlags, point);
}


void CWatchDialog::OnRButtonUp(UINT nFlags, CPoint point)
{
	if ((m_nObjWidth != -1) && (m_nObjHeight != -1)) {
		//坐标转换
		CPoint remote = UserPoint2RemoteScreenPoint(point);
		//封装
		MOUSEEV event;
		event.ptXY = remote;
		event.nButton = 1;//right
		event.nAction = 3;//up
		//CRemoteClientDlg* pParent = (CRemoteClientDlg*)GetParent();
		//pParent->SendMessage(WM_SEND_PACKET, 5 << 1 | 1, (WPARAM) & event);
		CClientController::getInstance()->SendCommandPacket(GetSafeHwnd(), 5, true, (BYTE*)&event, sizeof(event));
	}
	CDialog::OnRButtonUp(nFlags, point);
}


void CWatchDialog::OnStnClickedWatch()
{
	if ((m_nObjWidth != -1) && (m_nObjHeight != -1)) {
		CPoint point;
		GetCursorPos(&point);//屏幕坐标，其他的事件都是客户区坐标
		//坐标转换
		CPoint remote = UserPoint2RemoteScreenPoint(point, true);
		//封装
		MOUSEEV event;
		event.ptXY = remote;
		event.nButton = 0;//left
		event.nAction = 0;//click
		//CRemoteClientDlg* pParent = (CRemoteClientDlg*)GetParent();
		//pParent->SendMessage(WM_SEND_PACKET, 5 << 1 | 1, (WPARAM) & event);
		CClientController::getInstance()->SendCommandPacket(GetSafeHwnd(), 5, true, (BYTE*)&event, sizeof(event));
	}
}


void CWatchDialog::OnMouseMove(UINT nFlags, CPoint point)
{
	if ((m_nObjWidth != -1) && (m_nObjHeight != -1)) {
		//坐标转换
		CPoint remote = UserPoint2RemoteScreenPoint(point);
		//封装
		MOUSEEV event;
		event.ptXY = remote;
		event.nButton = 8;//only move,no click
		event.nAction = 0;//移动
		//CRemoteClientDlg* pParent = (CRemoteClientDlg*)GetParent();
		//pParent->SendMessage(WM_SEND_PACKET, 6 << 1 | 1, (WPARAM) & event);
		CClientController::getInstance()->SendCommandPacket(GetSafeHwnd(), 5, true, (BYTE*)&event, sizeof(event));
	}
	CDialog::OnMouseMove(nFlags, point);
}


void CWatchDialog::OnBnClickedBtnLock()
{
	//CRemoteClientDlg* pParent = (CRemoteClientDlg*)GetParent();
	//pParent->SendMessage(WM_SEND_PACKET, 7 << 1 | 1);
	CClientController::getInstance()->SendCommandPacket(GetSafeHwnd(), 7);
}


void CWatchDialog::OnBnClickedBtnUnlock()
{
	//CRemoteClientDlg* pParent = (CRemoteClientDlg*)GetParent();
	//pParent->SendMessage(WM_SEND_PACKET, 8 << 1 | 1);
	CClientController::getInstance()->SendCommandPacket(GetSafeHwnd(), 8);
}

LRESULT CWatchDialog::OnSendPackAck(WPARAM wParam, LPARAM lParam)
{
	if (lParam ==-1||(lParam==-2)) {
		//TODO:错误处理
	}
	else if (lParam == 1) {
		//对方关闭了套接字
	}
	else{
		CPacket* pPacket = (CPacket*)wParam;
		if (pPacket) {
			switch (pPacket->sCmd) {
			case 6: {
				if (m_isFull) {
					CHuxlTool::BytesToImage(m_image, pPacket->strData);
					CRect rect;
					m_picture.GetWindowRect(rect);
					m_nObjWidth = m_image.GetWidth();
					m_nObjHeight = m_image.GetHeight();
					m_image.StretchBlt(m_picture.GetDC()->GetSafeHdc(), 0, 0,
						rect.Width(), rect.Height(), SRCCOPY);
					m_picture.InvalidateRect(NULL);
					TRACE("更新图片完成%d %d %08X\r\n", m_nObjWidth, m_nObjHeight, (HBITMAP)m_image);
					m_image.Destroy();
					m_isFull = false;
				}
				  break;
			}
			case 5:
			case 7:
			case 8:
			default:
				break;
			}
		}
	}
	
	return 0;
}
