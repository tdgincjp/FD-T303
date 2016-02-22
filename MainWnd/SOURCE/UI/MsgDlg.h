#pragma once

#include "..\\..\\resource.h"
#include "StaticText.h"
#include "MyButton.h"
#include "Language.h"
#include "..\\data\\DataFile.h"

// CMsgDlg dialog

class CMsgDlg : public CDialog
{
	DECLARE_DYNAMIC(CMsgDlg)
public:

	CStaticText m_strMsg;
	CStaticText m_strMsg1;
	CStaticText m_strMsg2;
	CStaticText m_strMsg3;

	BOOL m_bTimeOutFlag;

	CMsgDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CMsgDlg();

// Dialog Data
	enum { IDD = IDD_MSGDLG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	int screenX;  
	int screenY;  

	DECLARE_MESSAGE_MAP()

public:
	virtual BOOL OnInitDialog();
	virtual afx_msg void OnTimer(UINT_PTR nIDEvent);
	virtual afx_msg void OnDestroy();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual void OnOK();
	virtual void OnCancel();

	int m_iTimeOut;
};

