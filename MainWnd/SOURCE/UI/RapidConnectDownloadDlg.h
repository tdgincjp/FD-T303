#pragma once
#include "basedlg.h"
#include "..\\DEFS\\struct.h"

#include "..\\HOST\\RapidConnect.h"

class CRapidConnectDownloadDlg :
	public CBaseDlg
{
	DECLARE_DYNAMIC(CRapidConnectDownloadDlg)

public:
	CRapidConnectDownloadDlg(CWnd* pParent = NULL);
	virtual ~CRapidConnectDownloadDlg(void);
	virtual void OnCancel();
	virtual void OnOK();
	virtual void Done();

public:
	static DWORD WINAPI ThreadProc (LPVOID lpArg);

protected:


	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();

//	afx_msg LRESULT SockRecvData(WPARAM wParam, LPARAM lParam);
	afx_msg void OnTimer(UINT_PTR nIDEvent);

public:
		TRAN_TranDataRec m_TRREC;
private:
		CONF_Data m_CONFIG;
		void DoHostInit();
		CRapidConnect * m_RapidConnect;
		void SetInitStatus(int status);

		BOOL m_bExitFlag;
		BOOL m_bWaitingDoneEvent;
};

