// MainWnd.h : main header file for the PROJECT_NAME application
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#ifdef STANDARDSHELL_UI_MODEL
#include "resource.h"
#endif
#include "source\\defs\\struct.h"

class CMainWndApp : public CWinApp
{
	BOOL CSetTimeZone(CString subKey);
	void MoveXACWindow();

public:
	CMainWndApp();
	
	BOOL m_bConnect;
	HWND m_hXACwnd;
	TRAN_TranDataRec m_TRREC;
	BOOL m_declinedOffline;
	BOOL m_bCancelFlag;

	void MoveBackWnd();

// Overrides
public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();

// Implementation

	DECLARE_MESSAGE_MAP()
};

extern CMainWndApp theApp;
