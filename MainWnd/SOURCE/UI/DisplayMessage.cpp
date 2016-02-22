#include "StdAfx.h"
#include "DisplayMessage.h"
#include "..\\UI\\display.h"

IMPLEMENT_DYNAMIC(CDisplayMsgDlg, CBaseDlg)

//-----------------------------------------------------------------------------
//!	\CDialog class virtual function
//-----------------------------------------------------------------------------
CDisplayMsgDlg::CDisplayMsgDlg(CWnd* pParent )
	: CBaseDlg(CBaseDlg::IDD, pParent)
{
//	m_bReturnFlag = FALSE;
}

//-----------------------------------------------------------------------------
//!	\CDialog class virtual function
//-----------------------------------------------------------------------------
CDisplayMsgDlg::~CDisplayMsgDlg(void)
{
}

//-----------------------------------------------------------------------------
//!	\CDialog class virtual function
//-----------------------------------------------------------------------------
void CDisplayMsgDlg::DoDataExchange(CDataExchange* pDX)
{
	CBaseDlg::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CDisplayMsgDlg, CBaseDlg)
	ON_WM_CHAR()
END_MESSAGE_MAP()


// CMainWnd message handlers
//-----------------------------------------------------------------------------
//!	\Set window title string
//!	\param	str - title string 
//-----------------------------------------------------------------------------
void CDisplayMsgDlg::SetTitle(CString str)
{
	m_Title = str;
}

//-----------------------------------------------------------------------------
//!	\CDialog class virtual function
//-----------------------------------------------------------------------------
BOOL CDisplayMsgDlg::OnInitDialog()
{
	char* buf;
	m_TitleIndex = CLanguage::IDX_PURCHASE;
	CLanguage::GetTextFromIndex(CLanguage::IDX_PURCHASE,&buf);
	m_Title = CString(buf);

	CBaseDlg::OnInitDialog();

	if(m_time > 0)
		SetTimer(1, m_time, NULL);
	else	
		SetTimer(1, 10000, NULL); //DEFAULT TIME 10 SEC IF TIMER NOT SET

	m_strMsg.ShowWindow(SW_SHOW);
	m_strMsg1.ShowWindow(SW_SHOW);
	m_strMsg2.ShowWindow(SW_SHOW);
	m_strMsg.m_nFormat = DT_LEFT;
	m_strMsg1.m_nFormat = DT_LEFT;
	m_strMsg2.m_nFormat = DT_LEFT;

	CDisplay::SetDisplayWnd(m_hWnd);
	CLanguage::SetDisplayWnd(m_hWnd);

	ShowText(m_str1, m_str2, m_str3);
	SetFocus();
	return TRUE;  // retur TRUE unless you set the focus to a control
}

void CDisplayMsgDlg::OnTimer(UINT_PTR nIDEvent)
{
	if (nIDEvent == 1)
	{
		KillTimer(1);
		CDialog::OnOK();
		return;
	}
	if (nIDEvent ==99)
	{
		SetFocus();
	}

	CBaseDlg::OnTimer(nIDEvent);
}


//-----------------------------------------------------------------------------
//!	\Set display strings
//!	\param	str1
//!	\param	str2
//!	\param	str3
//-----------------------------------------------------------------------------
void CDisplayMsgDlg::SetString(CString str1,CString str2, CString str3)
{
	m_str1 = str1;
	m_str2 = str2;
	m_str3 = str3;
}

//-----------------------------------------------------------------------------
//!	\Set displaytimer
//!	\param	time, msecs
//-----------------------------------------------------------------------------
void CDisplayMsgDlg::DisplayForTime(int time)
{
	m_time = time;
}

//-----------------------------------------------------------------------------
//!	\CDialog class virtual function
//-----------------------------------------------------------------------------
void CDisplayMsgDlg::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	Beep1();

	if( nChar == 8 )
	{
		m_bCorrectKey = TRUE;
		CDialog::OnCancel();
	}
}
