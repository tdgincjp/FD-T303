#pragma once
#include "basedlg.h"
#include "..\\DEFS\\struct.h"
#include "..\\utils\\\Printer.h"

class CReprintDlg :
	public CBaseDlg
{
	DECLARE_DYNAMIC(CReprintDlg)

public:
	CReprintDlg(CWnd* pParent = NULL);
	virtual ~CReprintDlg(void);
	virtual void OnCancel();
	virtual void OnOK();

protected:

	void Done();
	virtual LRESULT DefWindowProc(UINT message, WPARAM wParam, LPARAM lParam);

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags); 

//	afx_msg LRESULT SockRecvData(WPARAM wParam, LPARAM lParam);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
private:
	CPrinter m_Printer;
	TRAN_TranDataRec DataRec;

public:
	CString m_strInvoice;
};

