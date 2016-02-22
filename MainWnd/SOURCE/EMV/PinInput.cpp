// PinInput.cpp : implementation file
//

#include "stdafx.h"
#include "PinInput.h"
#include "..\\UI\\language.h"
#include "..\\UI\\display.h"
#include "SaioReader.h"
#include "SaioDev.h"
// CPinInput dialog

IMPLEMENT_DYNAMIC(CPinInput, CDialog)

//----------------------------------------------------------------------------
//!	\brief	virtual function
//----------------------------------------------------------------------------
CPinInput::CPinInput(CWnd* pParent /*=NULL*/)
	: CDialog(CPinInput::IDD, pParent)
{
	m_bCheckCardRemoved = TRUE;
}

//----------------------------------------------------------------------------
//!	\brief	virtual function
//----------------------------------------------------------------------------
CPinInput::~CPinInput()
{
}

//----------------------------------------------------------------------------
//!	\brief	virtual function
//----------------------------------------------------------------------------
BOOL CPinInput::OnInitDialog()
{
	CDialog::OnInitDialog();

	CString str = CLanguage::GetText(CLanguage::IDX_EMV_ENTER_PIN);
	GetDlgItem(IDC_STATIC1)->SetWindowTextW(str);
	if ( m_bCheckCardRemoved )
		SetTimer(1,1000,NULL);
	return TRUE;
}

//----------------------------------------------------------------------------
//!	\brief	virtual function
//----------------------------------------------------------------------------
void CPinInput::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CPinInput, CDialog)
	ON_WM_TIMER()
	ON_WM_CHAR()
END_MESSAGE_MAP()


// CPinInput message handlers
//----------------------------------------------------------------------------
//!	\brief	virtual function
//----------------------------------------------------------------------------
void CPinInput::OnTimer(UINT_PTR nIDEvent)
{
	if (GetScrStatus() == SCR_CARD_ABSENT)
	{
		KillTimer(1);

		CDisplay::ClearScreen();
		byte cmdbuf[4];
		cmdbuf[0] =  0x12;
		cmdbuf[1] =  0x01;
		cmdbuf[2] =  0x00;
		cmdbuf[3] =  0x00;
		
		DWORD rcvlen = 20;
		byte rcvbuf[20];
		int ret = PinGenericIO(cmdbuf, 4, rcvbuf, &rcvlen);
		CDialog::OnCancel();
	}
}

//----------------------------------------------------------------------------
//!	\brief	virtual function
//----------------------------------------------------------------------------
void CPinInput::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	TRACE(L"CPinInput::OnChar \n");
}
