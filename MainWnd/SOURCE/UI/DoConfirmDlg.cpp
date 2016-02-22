#include "StdAfx.h"
#include "DoConfirmDlg.h"

IMPLEMENT_DYNAMIC(CDoConfirmDlg, CBaseDlg)

//-----------------------------------------------------------------------------
//!	\CDialog class virtual function
//-----------------------------------------------------------------------------
CDoConfirmDlg::CDoConfirmDlg(CWnd* pParent )
	: CBaseDlg(CBaseDlg::IDD, pParent)
{
//	m_bReturnFlag = FALSE;
}

//-----------------------------------------------------------------------------
//!	\CDialog class virtual function
//-----------------------------------------------------------------------------
CDoConfirmDlg::~CDoConfirmDlg(void)
{
}

//-----------------------------------------------------------------------------
//!	\CDialog class virtual function
//-----------------------------------------------------------------------------
void CDoConfirmDlg::DoDataExchange(CDataExchange* pDX)
{
	CBaseDlg::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CDoConfirmDlg, CBaseDlg)
	ON_WM_CHAR()
END_MESSAGE_MAP()


// CMainWnd message handlers
//-----------------------------------------------------------------------------
//!	\Set window title string
//!	\param	str - title string 
//-----------------------------------------------------------------------------
void CDoConfirmDlg::SetTitle(CString str)
{
	m_Title = str;
}

//-----------------------------------------------------------------------------
//!	\CDialog class virtual function
//-----------------------------------------------------------------------------
BOOL CDoConfirmDlg::OnInitDialog()
{
	m_Title = CLanguage::GetText(CLanguage::IDX_RECEIPT_SALE); 

	CBaseDlg::OnInitDialog();

	for(int i=0;i<6;i++)
	{
		m_strText[i].Create(_T(" "), WS_CHILD, CRect(10, 55+i*30, i==5?310:150, 85+i*30), this);
		m_strText[i].SetPointFont(true,200);
		m_strText[i].SetColor(RGB(0, 0, 0),RGB( 255,255,255));
		m_strText[i].ShowWindow(SW_SHOW);

		m_strValue[i].Create(_T(" "), WS_CHILD, CRect(150, 55+i*30, 280, 85+i*30), this);
		m_strValue[i].SetPointFont(true,200);
		m_strValue[i].SetColor(RGB(0, 0, 0),RGB( 255,255,255));
		m_strValue[i].m_nFormat = DT_RIGHT;
		m_strValue[i].ShowWindow(SW_SHOW);

	}
	m_bCorrectKey = FALSE;
	return TRUE;  // retur TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

//-----------------------------------------------------------------------------
//!	\CDialog class virtual function
//-----------------------------------------------------------------------------
void CDoConfirmDlg::OnCancel()
{
	Beep1();
	CDialog::OnCancel();
}

//-----------------------------------------------------------------------------
//!	\CDialog class virtual function
//-----------------------------------------------------------------------------
void CDoConfirmDlg::OnOK()
{
	Beep1();
		
	if ( m_iDisplayErrorTime > 0)
		CDialog::OnCancel();
	else
		CDialog::OnOK();
}

//-----------------------------------------------------------------------------
//!	\Set confirm display string
//!	\param	index - text line id 
//!	\param	str - item string 
//!	\param	value - item amount string 
//-----------------------------------------------------------------------------
void CDoConfirmDlg::SetString(int index, CString str,CString value)
{
	m_strText[index-1].m_szCaption = str;
	m_strValue[index-1].m_szCaption = L"$"+value;
}

//-----------------------------------------------------------------------------
//!	\CDialog class virtual function
//-----------------------------------------------------------------------------
void CDoConfirmDlg::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	Beep1();

	if( nChar == 8 )
	{
		m_bCorrectKey = TRUE;
		CDialog::OnCancel();
	}
}
