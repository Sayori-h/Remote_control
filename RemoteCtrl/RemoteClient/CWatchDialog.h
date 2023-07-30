﻿#pragma once


// CWatchDialog 对话框

class CWatchDialog : public CDialog
{
	DECLARE_DYNAMIC(CWatchDialog)

public:
	CWatchDialog(CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~CWatchDialog();
	int m_nObjWidth;
	int m_nObjHeight;
	CImage m_image; // 这个可以设置为私有 然后开个接口去访问
// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DLG_WATCH };
#endif

protected:
	bool m_isFull;//缓存是否有数据  true有
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持
	
	DECLARE_MESSAGE_MAP()
public:
	CImage& GetImage() {
		return m_image;
	}
	void SetImageStatus(bool isFull = false) {
		m_isFull = isFull;
	}
	bool isFull()const {
		return m_isFull;
	}
	CPoint UserPoint2RemoteScreenPoint(CPoint& point,bool isScreen=false);
	virtual BOOL OnInitDialog();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	CStatic m_picture;
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnStnClickedWatch();
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnBnClickedBtnLock();
	afx_msg void OnBnClickedBtnUnlock();
};
