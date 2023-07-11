﻿
// RemoteClientDlg.h: 头文件
//

#pragma once
#include "ClientSocket.h"

// CRemoteClientDlg 对话框
class CRemoteClientDlg : public CDialogEx
{
// 构造
public:
	CRemoteClientDlg(CWnd* pParent = nullptr);	// 标准构造函数

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_REMOTECLIENT_DIALOG };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持

private:
	/*-------------------------------------- -
	1、查看进盘分区    ||   2、查看指定目录下的文件  
	3、打开文件	    ||   4、下载文件
	5、鼠标操作	    ||   6、发送屏幕的内容
	7、锁机			||   8、解锁
	9、删除文件		||   2001、测试连接
	返回值：是命令号，如果<0则错误*/
	int SendCommandPacket(int nCmd, bool bAutoClose=true,BYTE* pData = NULL, size_t nLength = 0);
	CString GetPath(HTREEITEM hTree);
	void DelTreeChildItem(HTREEITEM hTree);
	void LoadFileInfo();
	void LoadCurInfo();

// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedBtnTest();
	// 端口号
	CString m_nPort;
	DWORD m_server_address;
	afx_msg void OnBnClickedBtnFileinfo();
	// 对方的文件目录
	CTreeCtrl m_Tree;
	afx_msg void OnNMDblclkTreeDir(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnNMClickTreeDir(NMHDR* pNMHDR, LRESULT* pResult);
	// 显示文件
	CListCtrl m_List;
	afx_msg void OnNMRClickListFile(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDownloadFile();
	afx_msg void OnDeleteFile();
	afx_msg void OnRunFile();
};
