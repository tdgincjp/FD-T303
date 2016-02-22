#pragma once
#include "basedlg.h"

class CParamSettingDlg : 
	public CBaseDlg
{
	DECLARE_DYNAMIC(CParamSettingDlg)

	CMyButton m_Bt;
	CMyButton m_Bt1;

public:
	CParamSettingDlg(CString name, CWnd* pParent = NULL);
	virtual ~CParamSettingDlg(void);
	virtual void OnCancel();
	virtual void OnOK();

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedButton(UINT nID);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnDestroy();
};

