#pragma once

//#include "..\\..\\XACECR.h"
#include "basedlg.h"

// CDisplayWnd

class CDisplayWnd : public CBaseDlg
{
	DECLARE_DYNAMIC(CDisplayWnd)

public:
	CDisplayWnd(int mask, int timeout = 0,CWnd* pParent = NULL);   
	CDisplayWnd(CWnd* pParent = NULL);   
	virtual ~CDisplayWnd();

protected:
	virtual void OnCancel();
	void OnOK();
	virtual BOOL OnInitDialog();
	virtual BOOL PreTranslateMessage(MSG* pMsg);

	afx_msg void OnTimer(UINT nIDEvent);
//	afx_msg void OnPaint();
	afx_msg LRESULT Display(WPARAM wParam, LPARAM lParam);

private:
//	void DisplayText(CPaintDC* dc);
//	WORD screenX, screenY; 
	void Close(int val);

	int m_TimeOut;
	int m_KeyMask;

	CMyButton m_Bt1;
	CMyButton m_Bt2;
	CMyButton m_Bt3;

	CStaticText m_strLine[4];

public:
	int m_KeyReturn;
	CString m_strBt1;
	CString m_strBt2;
	CString m_strBt3;
protected:
	virtual void DoDataExchange(CDataExchange* pDX); 

	DECLARE_MESSAGE_MAP()

	afx_msg void OnBn1Clicked();
	afx_msg void OnBn2Clicked();
	afx_msg void OnBn3Clicked();
};
