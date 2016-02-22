// C:\work\MainWnd\MainWnd\SOURCE\UI\BaseDlg.cpp : implementation file
//

#include "stdafx.h"
#include "..\\..\\MainWnd.h"
#include "BaseDlg.h"
#include "Display.h"
#include "SaioBase.h"
#include "SaioReader.h"
#include "..\\EMV\\SaioDev.h"
#include "..\\data\\AppData.h"

// CBaseDlg dialog

IMPLEMENT_DYNAMIC(CBaseDlg, CDialog)

//-----------------------------------------------------------------------------
//!	\CDialog class virtual function
//-----------------------------------------------------------------------------
CBaseDlg::CBaseDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CBaseDlg::IDD, pParent)
{
}

//-----------------------------------------------------------------------------
//!	\CDialog class virtual function
//-----------------------------------------------------------------------------
CBaseDlg::CBaseDlg(int ID,CWnd* pParent /*=NULL*/)
	: CDialog(ID, pParent)
{
	m_iLevel = 0;
	m_iPos = 0;
	strcpy(m_ErrCode,"9999");
}

//-----------------------------------------------------------------------------
//!	\CDialog class virtual function
//-----------------------------------------------------------------------------
CBaseDlg::~CBaseDlg()
{
//TRACE(L"~CBaseDlg(void)\n");
}

//-----------------------------------------------------------------------------
//!	\CDialog class virtual function
//-----------------------------------------------------------------------------
void CBaseDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CBaseDlg, CDialog)
	ON_WM_PAINT()
	ON_WM_TIMER()
	ON_MESSAGE(WM_SOCK_RECV, SockRecvData)
	ON_WM_DESTROY()
//	ON_WM_CHAR()
END_MESSAGE_MAP()


// CBaseDlg message handlers
//----------------------------------------------------------------------------
//!	\brief	Draw a line
//----------------------------------------------------------------------------
void CBaseDlg::OnPaint()
{
	CPaintDC dc(this); // device context for painting
	dc.MoveTo(12, 50);
	dc.LineTo(320-12, 50);
}
//----------------------------------------------------------------------------
//!	\brief	make alarm beep voice
//!	\return	
//!			 
//----------------------------------------------------------------------------
void CBaseDlg::Beep1()
{
	TCHAR buf[10]={L""};
	if(CDataFile::Read(L"SOUND",buf) && CString(buf) == L"Off")
		return;
	SaioBeeper(1, 5000, 100, 0, 40);
}	
//----------------------------------------------------------------------------
//!	\brief	Init window 
//----------------------------------------------------------------------------
BOOL CBaseDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	screenX = GetSystemMetrics(SM_CXSCREEN);  
	screenY = GetSystemMetrics(SM_CYSCREEN);  
	MoveWindow( 0, 0, screenX, screenY);

	m_strTitle.Create(_T(" "), WS_CHILD| WS_VISIBLE, CRect(100, 6, screenX, 36), this);
	m_strTitle.SetPointFont(true,220);
	m_strTitle.SetCaption(m_Title);

	if(CDataFile::m_bTraining)
		m_strTitle.SetColor(RGB(255, 0, 0),RGB( 255,255,255));
	else
		m_strTitle.SetColor(RGB(0, 0, 255),RGB( 255,255,255));

	m_strMsg.Create(_T(" "), WS_CHILD, CRect(10, 55+m_iPos, 310, 105+m_iPos), this);
	m_strMsg.SetPointFont(true,220);
	m_strMsg.SetColor(RGB(0, 0, 0),RGB( 255,255,255));

	m_strMsg1.Create(_T(" "), WS_CHILD, CRect(10, 110+m_iPos, 310, 160+m_iPos), this);
	m_strMsg1.SetPointFont(true,220);
	m_strMsg1.SetColor(RGB(0, 0, 0),RGB( 255,255,255));

	m_strMsg2.Create(_T(" "), WS_CHILD, CRect(10, 160+m_iPos, 310, 210+m_iPos), this);
	m_strMsg2.m_nFormat = DT_RIGHT;
	m_strMsg2.SetPointFont(true,240);
	m_strMsg2.SetColor(RGB(0, 0, 0),RGB( 255,255,255));

	GetDlgItem(IDC_CONNECT1)->MoveWindow(screenX-36,4,32,32);
	GetDlgItem(IDC_DISCONNECT1)->MoveWindow(screenX-36,4,32,32);
	CheckNetWorkStatus();
	SetTimer(99,1000,NULL);

	m_iDisplayErrorTime = 0;
	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

//----------------------------------------------------------------------------
//!	\brief	for Timer 99 to check network status
//----------------------------------------------------------------------------
void CBaseDlg::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: Add your message handler code here and/or call default
	if ( nIDEvent == 99 )
	{
		CheckNetWorkStatus();

		if( GetForegroundWindow() != this)
			return;

		DWORD size;
		int temp = 0;
		CDataFile::Read((int)FID_CFG_EMVCARD_CHECK, (BYTE *)&temp, &size);

		if (temp == 1)
		{
			if (GetScrStatus() >= SCR_CARD_ABSENT)
			{
				temp = 2;
				CDataFile::Save(FID_CFG_EMVCARD_CHECK, (BYTE *)&temp, sizeof(int));
				CDialog::OnCancel();
			}
		}
	}
	CDialog::OnTimer(nIDEvent);
//	SetFocus();
}
//----------------------------------------------------------------------------
//!	\brief	Close window
//----------------------------------------------------------------------------
void CBaseDlg::OnDestroy()
{
	KillTimer(99);
	CDialog::OnDestroy();
}

//----------------------------------------------------------------------------
//!	\brief	message event function
//----------------------------------------------------------------------------
LRESULT CBaseDlg::SockRecvData(WPARAM wParam, LPARAM lParam)
{
//	char* buf;

	CString str;
	switch(wParam)
	{
	case 0: // get error from sock
		str = CLanguage::GetText(CLanguage::IDX_ERROR);
		m_strMsg.SetCaption(str);
		break;
	case 1: // connect text status display
		str = CLanguage::GetText(lParam);
		ShowText(str,L"",L"");
		Sleep(10);
		break;
	case 2: // host connect finish
		if (CAppData::bEMVError)
		{
			CAppData::bEMVError = FALSE;
			break;
		}
		m_iLevel++;
		Done();
		break;
	case 3: // update Title
		m_Title = CLanguage::GetText(m_TitleIndex);
		m_strTitle.SetCaption(m_Title);
		break;
	}
	return 0;
}

//----------------------------------------------------------------------------
//!	\brief	this is virtural function
//----------------------------------------------------------------------------
void CBaseDlg::Done()
{
}

//----------------------------------------------------------------------------
//!	\brief	check network connect status every one second
//----------------------------------------------------------------------------
void CBaseDlg::CheckNetWorkStatus()
{
	if (((CMainWndApp*)AfxGetApp())->m_bConnect)
	{
		if(GetDlgItem(IDC_CONNECT1)->IsWindowVisible() == FALSE)
		{
			GetDlgItem(IDC_CONNECT1)->ShowWindow(SW_SHOW);
			GetDlgItem(IDC_DISCONNECT1)->ShowWindow(SW_HIDE);
		}
	}
	else
	{
		if(GetDlgItem(IDC_DISCONNECT1)->IsWindowVisible() == FALSE)
		{
			GetDlgItem(IDC_CONNECT1)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_DISCONNECT1)->ShowWindow(SW_SHOW);
		}
	}
}

//----------------------------------------------------------------------------
//!	\brief	get network connect status
//----------------------------------------------------------------------------
BOOL CBaseDlg::IsNetWorkConnect()
{
	TCHAR buf1[10]={L""};

	TCHAR buf[10]={L""};
	if (CDataFile::Read(L"TRAINING", buf))
	{
		if (CString(buf) == L"On")
			return TRUE;
	}

	if(CDataFile::Read(L"COMMTYPE",buf1))
	{
		if( CString(buf1) == L"Dial")
			 return TRUE;
	}

	CDataFile::Read(L"DIAL BACKUP",buf1);
	if(CString(buf1) != L"Off")
		return TRUE;

	return ((CMainWndApp*)AfxGetApp())->m_bConnect;
}

//----------------------------------------------------------------------------
//!	\brief	Show text 
//----------------------------------------------------------------------------
void CBaseDlg::ShowText(const CString str1,const CString str2,const CString str3)
{
	m_strMsg.SetCaption(str1);
	m_strMsg1.SetCaption(str2);
	m_strMsg2.SetCaption(str3);
}

//-----------------------------------------------------------------------------
//!	\CDialog class virtual function
//-----------------------------------------------------------------------------
BOOL CBaseDlg::PreTranslateMessage(MSG* pMsg) 
{
	if (pMsg->message == WM_KEYUP)
		SetFocus();
	return CDialog::PreTranslateMessage(pMsg);
}

//-----------------------------------------------------------------------------
//!	\setting one virtual function for derive class
//-----------------------------------------------------------------------------
void CBaseDlg::ShowPicture(int flag)
{
	return;
}
