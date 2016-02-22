#pragma once
#include "basedlg.h"
#include "..\\TRANSACTIONS\\Finan.h"

class CForcePostDlg  :
	public CBaseDlg,CFinan
{
	DECLARE_DYNAMIC(CForcePostDlg )
	CMyButton m_Bt;
	CMyButton m_Bt1;
		
public:
	CForcePostDlg (CWnd* pParent = NULL);
	virtual ~CForcePostDlg (void);
	virtual void OnCancel();
	virtual void OnOK();

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual LRESULT DefWindowProc(UINT message, WPARAM wParam, LPARAM lParam);

	void Done();
	void GoBack();
	void GoNext();
	void GoToLevel(int level);

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedButton(UINT nID);
	afx_msg void OnTimer(UINT_PTR nIDEvent);

	afx_msg void OnDestroy();
	afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags); 
	void ErrorExit();
	void ErrorExit(int Level);

private:
	void WorkLoop(int index);

	CString m_strErr;
	void DoTransaction();
	void SetDisplayButton();
	BOOL DoConfirm();

};

