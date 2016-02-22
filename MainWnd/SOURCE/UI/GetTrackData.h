#pragma once
#include "basedlg.h"
#include "..\\TRANSACTIONS\\Finan.h"

class CGetTrackDataDlg :
	public CBaseDlg, CFinan
{
	DECLARE_DYNAMIC(CGetTrackDataDlg)

public:
	CGetTrackDataDlg(CWnd* pParent = NULL);
	virtual ~CGetTrackDataDlg(void);
	virtual void OnCancel();
	virtual void OnOK();


protected:

	void Done();

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual LRESULT DefWindowProc(UINT message, WPARAM wParam, LPARAM lParam);

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();

	afx_msg void OnTimer(UINT_PTR nIDEvent);

private:
	CMSR m_MSR;
//	CSCR m_SCR;
	CMagRFID m_RFID;
	BOOL OpenDevice();
	void CloseDevice();


public:
	char Track1[SZ_TRACK1+1];
	char Track2[SZ_TRACK2+1];

};

