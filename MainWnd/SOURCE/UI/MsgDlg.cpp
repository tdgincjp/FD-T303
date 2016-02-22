// MsgDlg.cpp : implementation file
//

#include "stdafx.h"
#include "..\\..\\MainWnd.h"
#include "MsgDlg.h"
#include "SaioBase.h"


// CMsgDlg dialog

IMPLEMENT_DYNAMIC(CMsgDlg, CDialog)

CMsgDlg::CMsgDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CMsgDlg::IDD, pParent)
{
	m_iTimeOut = 10;
}

CMsgDlg::~CMsgDlg()
{
}

void CMsgDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CMsgDlg, CDialog)
	ON_WM_TIMER()
	ON_WM_DESTROY()
	ON_WM_CHAR()
END_MESSAGE_MAP()


//----------------------------------------------------------------------------
//!	\brief	Init window 
//----------------------------------------------------------------------------
BOOL CMsgDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	screenX = GetSystemMetrics(SM_CXSCREEN);  
	screenY = GetSystemMetrics(SM_CYSCREEN);
	MoveWindow( 0, 0, screenX, screenY);

	int m_iPos = -50;
	m_strMsg.Create(_T(" "), WS_CHILD, CRect(10, 55+m_iPos, 310, 105+m_iPos), this);
	m_strMsg.SetPointFont(true,180);
	m_strMsg.SetColor(RGB(0, 0, 0),RGB( 255,255,255));

	m_strMsg1.Create(_T(" "), WS_CHILD, CRect(10, 110+m_iPos, 310, 160+m_iPos), this);
	m_strMsg1.SetPointFont(true,180);
	m_strMsg1.SetColor(RGB(0, 0, 0),RGB( 255,255,255));

	m_strMsg2.Create(_T(" "), WS_CHILD, CRect(10, 160+m_iPos, 310, 210+m_iPos), this);
	m_strMsg2.SetPointFont(true,180);
	m_strMsg2.SetColor(RGB(0, 0, 0),RGB( 255,255,255));

	m_strMsg3.Create(_T(" "), WS_CHILD, CRect(10, 215+m_iPos, 310, 265+m_iPos), this);
	m_strMsg3.SetPointFont(true,180);
	m_strMsg3.SetColor(RGB(0, 0, 0),RGB( 255,255,255));

	CString temp;
	CString temp1;


	TCHAR Buf[100] = {L""};
	CDataFile::Read(L"Datawire TID", Buf);
	temp.Format(L"TID: %s",Buf); 
	m_strMsg.SetCaption(temp);

	memset(Buf,0,100);
	CDataFile::Read(L"Datawire MID", Buf);
	temp.Format(L"MID: %s",Buf); 
	m_strMsg1.SetCaption(temp);


	memset(Buf,0,100);
	CDataFile::Read(L"Datawire SD URL", Buf);
	temp.Format(L"URL: %s",Buf); 
	m_strMsg2.SetCaption(temp);

	temp = _T("5");
	if(CDataFile::Read(L"TZ Offset", Buf))
		temp = CString(Buf);

	temp1.Format(L"TZ Offset : %s ", temp);
	m_strMsg3.SetCaption(temp1);

	m_strMsg.ShowWindow(SW_SHOW);
	m_strMsg1.ShowWindow(SW_SHOW);
	m_strMsg2.ShowWindow(SW_SHOW);
	m_strMsg3.ShowWindow(SW_SHOW);

	SetTimer(1,m_iTimeOut*1000,NULL);
	SetTimer(2,100,NULL);
	return TRUE;  // return TRUE unless you set the focus to a control
}

//----------------------------------------------------------------------------
//!	\brief	for Timer 99 to check network status
//----------------------------------------------------------------------------
void CMsgDlg::OnTimer(UINT_PTR nIDEvent)
{
	CDialog::OnTimer(nIDEvent);
	if (nIDEvent == 2)
	{
		KillTimer(2);
		SetFocus();
		return;
	}
	if (nIDEvent == 1)
	{
		KillTimer(1);
		m_bTimeOutFlag = TRUE;
		OnOK();
	}
//	SetFocus();
}

//----------------------------------------------------------------------------
//!	\brief	Close window
//----------------------------------------------------------------------------
void CMsgDlg::OnDestroy()
{
	CDialog::OnDestroy();
}

BOOL CMsgDlg::PreTranslateMessage(MSG* pMsg) 
{
	if (pMsg->message == WM_KEYUP)
		SetFocus();
	return CDialog::PreTranslateMessage(pMsg);
}

void CMsgDlg::OnOK()
{
	CDialog::OnOK();
}

void CMsgDlg::OnCancel()
{
	CDialog::OnCancel();
}

