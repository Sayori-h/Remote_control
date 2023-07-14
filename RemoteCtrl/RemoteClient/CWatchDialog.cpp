// CWatchDialog.cpp: 实现文件
//

#include "pch.h"
#include "RemoteClient.h"
#include "CWatchDialog.h"
#include "afxdialogex.h"
#include "RemoteClientDlg.h"


// CWatchDialog 对话框

IMPLEMENT_DYNAMIC(CWatchDialog, CDialog)

CWatchDialog::CWatchDialog(CWnd* pParent /*=nullptr*/)
	: CDialog(IDD_DLG_WATCH, pParent)
{

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
END_MESSAGE_MAP()


// CWatchDialog 消息处理程序


CPoint CWatchDialog::UserPoint2RemoteScreenPoint(CPoint& point)
{
	CRect clientRect;
	ScreenToClient(&point);//全局坐标到客户坐标
	m_picture.GetWindowRect(clientRect);//本地客户坐标到远程坐标
	int width0 = clientRect.Width();
	int height0 = clientRect.Height();
	int width = 1920, height = 1080;
	int x = point.x * width / width0;
	int y = point.y * height / height0;
	return CPoint(x,y);
}

BOOL CWatchDialog::OnInitDialog()
{
	CDialog::OnInitDialog();
	SetTimer(0, 40, NULL);
	return TRUE;  // return TRUE unless you set the focus to a control
				  // 异常: OCX 属性页应返回 FALSE
}


void CWatchDialog::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	if (!nIDEvent) {
		CRemoteClientDlg* pParent = (CRemoteClientDlg*)GetParent();
		if (pParent->isFull())
		{
			CRect rect;
			m_picture.GetWindowRect(rect);
			//pParent->GetImage().BitBlt(m_picture.GetDC()->GetSafeHdc(), 0, 0, SRCCOPY);
			//调整图像大小
			pParent->GetImage().StretchBlt(
				m_picture.GetDC()->GetSafeHdc(),0,0, rect.Width(), rect.Height(), SRCCOPY);
			m_picture.InvalidateRect(NULL);
			pParent->GetImage().Destroy();
			pParent->SetImageStatus();
		}
	}
	CDialog::OnTimer(nIDEvent);
}


void CWatchDialog::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	//坐标转换
	CPoint remote = UserPoint2RemoteScreenPoint(point);
	//封装
	MOUSEEV event;
	event.ptXY = remote;
	event.nButton = 0;//左键
	event.nAction = 2;//双击
	CPacket pack(5, (BYTE*)&event, sizeof(event));
	gpClient->sendCom(pack);
	CDialog::OnLButtonDblClk(nFlags, point);
}


void CWatchDialog::OnLButtonDown(UINT nFlags, CPoint point)
{
	//坐标转换
	CPoint remote = UserPoint2RemoteScreenPoint(point);
	//封装
	MOUSEEV event;
	event.ptXY = remote;
	event.nButton = 0;//左键
	event.nAction = 3;//按下
	CPacket pack(5, (BYTE*)&event, sizeof(event));
	gpClient->sendCom(pack);
	CDialog::OnLButtonDown(nFlags, point);
}


void CWatchDialog::OnLButtonUp(UINT nFlags, CPoint point)
{
	///坐标转换
	CPoint remote = UserPoint2RemoteScreenPoint(point);
	//封装
	MOUSEEV event;
	event.ptXY = remote;
	event.nButton = 0;//左键
	event.nAction = 3;//按下
	CPacket pack(5, (BYTE*)&event, sizeof(event));
	gpClient->sendCom(pack);

	CDialog::OnLButtonUp(nFlags, point);
}


void CWatchDialog::OnRButtonDblClk(UINT nFlags, CPoint point)
{
	//坐标转换
	CPoint remote = UserPoint2RemoteScreenPoint(point);
	//封装
	MOUSEEV event;
	event.ptXY = remote;
	event.nButton = 0;//左键
	event.nAction = 3;//按下
	CPacket pack(5, (BYTE*)&event, sizeof(event));
	gpClient->sendCom(pack);
	CDialog::OnRButtonDblClk(nFlags, point);
}


void CWatchDialog::OnRButtonDown(UINT nFlags, CPoint point)
{
	//坐标转换
	CPoint remote = UserPoint2RemoteScreenPoint(point);
	//封装
	MOUSEEV event;
	event.ptXY = remote;
	event.nButton = 0;//左键
	event.nAction = 3;//按下
	//TODO:服务端做对应修改
	CPacket pack(5, (BYTE*)&event, sizeof(event));
	gpClient->sendCom(pack);
	CDialog::OnRButtonDown(nFlags, point);
}


void CWatchDialog::OnRButtonUp(UINT nFlags, CPoint point)
{
	//坐标转换
	CPoint remote = UserPoint2RemoteScreenPoint(point);
	//封装
	MOUSEEV event;
	event.ptXY = remote;
	event.nButton = 0;//左键
	event.nAction = 4;//松开
	CPacket pack(5, (BYTE*)&event, sizeof(event));
	gpClient->sendCom(pack);
	CDialog::OnRButtonUp(nFlags, point);
}


void CWatchDialog::OnStnClickedWatch()
{
	CPoint point;
	GetCursorPos(&point);
	//坐标转换
	CPoint remote = UserPoint2RemoteScreenPoint(point);
	//封装
	MOUSEEV event;
	event.ptXY = remote;
	event.nButton = 0;//左键
	event.nAction = 3;//按下
	CPacket pack(5, (BYTE*)&event, sizeof(event));
	gpClient->sendCom(pack);
}


void CWatchDialog::OnMouseMove(UINT nFlags, CPoint point)
{
	//坐标转换
	CPoint remote = UserPoint2RemoteScreenPoint(point);
	//封装
	MOUSEEV event;
	event.ptXY = remote;
	event.nButton = 0;//左键
	event.nAction = 1;//移动
	CPacket pack(5, (BYTE*)&event, sizeof(event));
	gpClient->sendCom(pack);
	CDialog::OnMouseMove(nFlags, point);
}
