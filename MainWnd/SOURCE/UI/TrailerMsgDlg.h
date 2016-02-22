#pragma once
#include "basedlg.h"
#include "..\\TRANSACTIONS\\Finan.h"

class CTrailerMsgDlg :
	public CBaseDlg,CFinan
{
	DECLARE_DYNAMIC(CTrailerMsgDlg)
		
	CMyButton m_Bt;
	CMyButton m_Bt1;

	CString m_strLine[4];
	void SaveData();


public:
	CTrailerMsgDlg(CWnd* pParent = NULL);
	virtual ~CTrailerMsgDlg(void);
	virtual void OnCancel();
	virtual void OnOK();

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	void Done();

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedButton(UINT nID);
	afx_msg void OnTimer(UINT_PTR nIDEvent);

	afx_msg void OnDestroy();
	afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags); 
	LRESULT DefWindowProc(UINT message, WPARAM wParam, LPARAM lParam);

	int m_iType; //1:Trailer; 2:header

private:
	void WorkLoop(int index);
	void GoBack();
	void GoNext();
	void GoToLevel(int level);

};

