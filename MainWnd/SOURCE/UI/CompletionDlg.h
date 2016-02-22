#pragma once
#include "basedlg.h"
#include "..\\TRANSACTIONS\\Finan.h"
#include "..\\TRANSACTIONS\\TransactionLog.h"

class CCompletionDlg :
	public CBaseDlg,CFinan
{
	DECLARE_DYNAMIC(CCompletionDlg)
		
	CMyButton m_Bt;
	CMyButton m_Bt1;
	CStaticText m_strMsg3;

public:
	CCompletionDlg(CWnd* pParent = NULL);
	virtual ~CCompletionDlg(void);
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

	CTransactionLog m_Log;
	int  m_index;
	int m_iCountLog ;

	int m_iShowCount;

	BOOL m_bTip;

	CString m_OriginalAmount;
	CString m_TipAmount;

	BYTE m_Seq[SZ_FULL_SEQUENCE_NO+1];
};

