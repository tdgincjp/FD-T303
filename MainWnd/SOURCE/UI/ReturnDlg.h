#pragma once
#include "basedlg.h"
#include "..\\TRANSACTIONS\\Finan.h"

class CReturnDlg :
	public CBaseDlg,CFinan
{
	DECLARE_DYNAMIC(CReturnDlg)
		
	CMyButton m_Bt;
	CMyButton m_Bt1;
public:
	CReturnDlg(CWnd* pParent = NULL);
	virtual ~CReturnDlg(void);
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

	BOOL m_bReturnFlag;
	void ErrorExit();
	void ErrorExit(int Level);

private:
	void WorkLoop(int index);

	void DoTransaction();

	void SetDisplayButton();

};

