#pragma once
#include "basedlg.h"

class CDoConfirmDlg :
	public CBaseDlg
{
	DECLARE_DYNAMIC(CDoConfirmDlg)

	CStaticText m_strText[6];
	CStaticText m_strValue[6];
public:
	CDoConfirmDlg(CWnd* pParent = NULL);
	virtual ~CDoConfirmDlg(void);
	virtual void OnCancel();
	virtual void OnOK();

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	void SetString(int index, CString str,CString value=L"");
	afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags); 

	void SetTitle(CString str);

	BOOL m_bCorrectKey;

};

