#pragma once
#include "basedlg.h"

class CDisplayMsgDlg :
	public CBaseDlg
{
	DECLARE_DYNAMIC(CDisplayMsgDlg)
	CString m_str1, m_str2, m_str3;
	int m_time;
public:
	CDisplayMsgDlg(CWnd* pParent = NULL);
	virtual ~CDisplayMsgDlg(void);

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	void SetString(CString str1,CString str2, CString str3);
	void DisplayForTime(int time);


	afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags); 
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	void SetTitle(CString str);
	BOOL m_bCorrectKey;

};

