#pragma once
#include "basedlg.h"
#include "..\\TRANSACTIONS\\Finan.h"

class CSaleDlg :
	public CBaseDlg, public CFinan
{
	DECLARE_DYNAMIC(CSaleDlg)
		
	CMyButton m_Bt;
	CMyButton m_Bt1;
public:
	CSaleDlg(CWnd* pParent = NULL);
	virtual ~CSaleDlg(void);
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
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);

	BOOL m_bReturnFlag;
	void ErrorExit();
	void ErrorExit(int Level);
	void ShowAlert(int Level);
	void SetErrorCode(char* code, int m1, int m2);				//JC Apr 7/15 to pass message back to API

	void SetAmount(char* Amount);
    BOOL IsRRLLimit();

private:
	void WorkLoop(int index);
	void DoTransaction();
	void SetDisplayButton();

	BOOL m_bInsertCard;
	int  m_iWaitMSR;
	int  m_iRfidTimer;
	BOOL m_bEMVPowerTest;
	BOOL m_bTranInProgress;

	BOOL IsCardInSlot();
	BOOL m_bEmvEnabled;

	CWnd* m_pParentWnd;
	void  SendECRBack();
};

