// StatusDlg.cpp: 实现文件
//

#include "pch.h"
#include "RemoteClient.h"
#include "StatusDlg.h"
#include "afxdialogex.h"


// CStatusDlg 对话框

IMPLEMENT_DYNAMIC(CStatusDlg, CDialog)

CStatusDlg::CStatusDlg(CWnd* pParent /*=nullptr*/)
	: CDialog(IDD_DIALOG_S, pParent)
{

}

CStatusDlg::~CStatusDlg()
{
}

void CStatusDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT_I, m_info);
}


BEGIN_MESSAGE_MAP(CStatusDlg, CDialog)
END_MESSAGE_MAP()


// CStatusDlg 消息处理程序
