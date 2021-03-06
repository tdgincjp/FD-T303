// DisplayWnd.cpp : 
//

#include "stdafx.h"
//#include "..\\..\\XACECR.h"
#include "DisplayWnd.h"
#include "Display.h"


// CDisplayWnd 

#define MASK_OK		0x01
#define MASK_CANCEL 0x02
#define MASK_CORR	0x04

IMPLEMENT_DYNAMIC(CDisplayWnd, CBaseDlg)

CDisplayWnd::CDisplayWnd(int mask, int timeout,CWnd* pParent /*=NULL*/)
	: CBaseDlg(CDisplayWnd::IDD, pParent)
{
	m_TimeOut = timeout;
	m_KeyMask = mask;
	m_KeyReturn = 0;

	m_strBt1 = L"";
	m_strBt2 = L"";
	m_strBt3 = L"";
}

CDisplayWnd::CDisplayWnd(CWnd* pParent /*=NULL*/)
	: CBaseDlg(CDisplayWnd::IDD, pParent)
{
	m_TimeOut = 0;
	m_KeyMask = 0;
	m_KeyReturn = 0;

	m_strBt1 = L"";
	m_strBt2 = L"";
	m_strBt3 = L"";
}

CDisplayWnd::~CDisplayWnd()
{
}

void CDisplayWnd::DoDataExchange(CDataExchange* pDX)
{
	CBaseDlg::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CDisplayWnd, CBaseDlg)
	ON_WM_PAINT()
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_BUTTON1, &CDisplayWnd::OnBn1Clicked)
	ON_BN_CLICKED(IDC_BUTTON2, &CDisplayWnd::OnBn2Clicked)
	ON_BN_CLICKED(IDC_BUTTON3, &CDisplayWnd::OnBn3Clicked)
//	ON_MESSAGE(WM_DISPLAY_EVENT,&CDisplayWnd::Display)
END_MESSAGE_MAP()


// CDisplayWnd 
BOOL CDisplayWnd::OnInitDialog()
{
	CBaseDlg::OnInitDialog();

	int BtNumber = 3;//(m_strBt1.GetLength()!=0?1:0)
				// + (m_strBt2.GetLength()!=0?1:0)
				// + (m_strBt3.GetLength()!=0?1:0);

	if (m_TimeOut != 0)
		SetTimer(1,m_TimeOut*1000,NULL);

/*		m_strMsg.ShowWindow(SW_SHOW);
	m_strMsg1.ShowWindow(SW_SHOW);
	m_strMsg2.ShowWindow(SW_SHOW);

	m_strMsg.SetCaption(CDisplay::GetLine(1));
	m_strMsg1.SetCaption(CDisplay::GetLine(2));
	m_strMsg2.SetCaption(CDisplay::GetLine(3));

	m_Bt.SetValue(1, m_strBt1);
	m_Bt.SubclassDlgItem(IDC_BUTTON1, this);
	m_Bt.MoveWindow( 8, screenY- 50, 120, 45);
	m_Bt.ShowWindow(SW_SHOW);

	m_Bt1.SetValue(3,m_strBt3);
	m_Bt1.SubclassDlgItem(IDC_BUTTON2, this);
	m_Bt1.MoveWindow(screenX-128,  screenY- 50, 120,45);
	m_Bt1.ShowWindow(SW_SHOW);
*/
	
	int iPos = 50;
	for(int i=0;i<4;i++)
	{
		m_strLine[i].Create(_T(" "), WS_CHILD, CRect(10, 55+iPos*i, 310, 105+iPos*i), this);
		m_strLine[i].SetPointFont(true,220);
		m_strLine[i].SetColor(RGB(0, 0, 0),RGB( 255,255,255));
		m_strLine[i].SetCaption(CDisplay::GetLine(i+1));
		m_strLine[i].ShowWindow(SW_SHOW);
	}
	
	int iTemp = 1;
	if (m_strBt1.GetLength())
	{
		m_Bt1.SetValue(-1, m_strBt1);
		m_Bt1.SubclassDlgItem(IDC_BUTTON1, this);
		m_Bt1.MoveWindow( iTemp,screenY-50,screenX/BtNumber-iTemp, 45);
		m_Bt1.ShowWindow(SW_SHOW);
	}

	if (m_strBt3.GetLength())
	{
		m_Bt2.SetValue(-1, m_strBt2);
		m_Bt2.SubclassDlgItem(IDC_BUTTON2, this);
		m_Bt2.MoveWindow( screenX/BtNumber+iTemp,screenY-50,screenX/BtNumber-iTemp, 45);
		m_Bt2.ShowWindow(SW_SHOW);

		m_Bt3.SetValue(-1, m_strBt3);
		m_Bt3.SubclassDlgItem(IDC_BUTTON3, this);
		m_Bt3.MoveWindow( screenX/BtNumber*2+iTemp,screenY-50,screenX/BtNumber-iTemp, 45);
		m_Bt3.ShowWindow(SW_SHOW);
	}
	else if(m_strBt2.GetLength())
	{
		m_Bt2.SetValue(-1, m_strBt2);
		m_Bt2.SubclassDlgItem(IDC_BUTTON2, this);
		m_Bt2.MoveWindow( screenX/BtNumber*2+iTemp,screenY-50,screenX/BtNumber-iTemp, 45);
		m_Bt2.ShowWindow(SW_SHOW);
	}
//	Display(0,0);
	return TRUE;  // return TRUE unless you set the focus to a control
}

LRESULT CDisplayWnd::Display(WPARAM wParam, LPARAM lParam)
{
	if (lParam == 1)
	{
		OnCancel();
		return 0;
	}

	ShowWindow(SW_SHOW);
	SetForegroundWindow();
	Invalidate(TRUE);
	Sleep(1);
	return 0;
}
/*
void CDisplayWnd::DisplayText(CPaintDC* dc)
{
	CFont font;
	font.CreatePointFont(CDisplay::GetFontSize(), _T("MS Sans Serif"));
	CFont* def_font = dc->SelectObject(&font);
	dc->SetTextColor(RGB(0,0,0));
	dc->SetBkColor(RGB(255,255,255));

	int iHigh = (screenY-12)/CDisplay::GetShowMode();
	int startX = 0;
	for(int i = 1 ;i <= CDisplay::GetShowMode(); i++)
	{
		CString str = CDisplay::GetLine(i);
		if (i==1)
		{
			CSize sz;
			dc->SetTextColor(RGB(0,0,255));
			sz=dc->GetTextExtent(str);
			startX = (screenX-sz.cx)/2;
			dc->ExtTextOut(startX, (i-1)*iHigh+12, ETO_CLIPPED, NULL, str, wcslen(str), NULL );
			dc->MoveTo(12, iHigh-8);
			dc->LineTo(screenX-12, iHigh-8);

		}
		else
		{
			dc->SetTextColor(RGB(0,0,0));
			dc->ExtTextOut(12, (i-1)*iHigh+2, ETO_CLIPPED, NULL, str, wcslen(str), NULL );
		}
	}
	dc->SelectObject(def_font);
	font.DeleteObject();
}

void CDisplayWnd::OnPaint()
{
	CPaintDC dc(this); 
	DisplayText(&dc);
	SetFocus();
}
*/
void CDisplayWnd::OnCancel()
{
	if ( m_KeyMask & MASK_CANCEL)
		Close(KEY_CANCEL);
}

void CDisplayWnd::OnOK()
{
	if ( m_KeyMask & MASK_OK)
		Close(KEY_ENTER);
}

BOOL CDisplayWnd::PreTranslateMessage(MSG* pMsg) 
{
	TranslateMessage(pMsg);
	DispatchMessage(pMsg);
	
	if (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_BACK)
	{
		if ( m_KeyMask & MASK_CORR)
			Close(KEY_CORR);
		return 0;	
	}
	
	return CDialog::PreTranslateMessage(pMsg);
}

void CDisplayWnd::OnTimer(UINT nIDEvent)
{
	CDialog::OnTimer(nIDEvent);
	if(nIDEvent == 1)
		Close(KEY_TIMEOUT);
}

void CDisplayWnd::Close(int val)
{
	m_KeyReturn = val;
	KillTimer(1);
	CDialog::OnCancel();
}

void CDisplayWnd::OnBn1Clicked()
{
	Close(KEY_BUT1);
}

void CDisplayWnd::OnBn2Clicked()
{
	Close(KEY_BUT2);
}

void CDisplayWnd::OnBn3Clicked()
{
	Close(KEY_BUT3);
}

