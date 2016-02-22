// ECR.cpp : implementation file
//

#include "stdafx.h"
#include <stdio.h>
#include "..\\DEFS\\constant.h"
#include "..\\Utils\\BinUtil.h"
#include "..\\DEFS\\struct.h"
#include "..\\..\\XACECR.h"
#include "..\\Utils\\StrUtil.h"
#include "..\\Data\\DataFile.h"
#include "..\\UI\\Language.h"
#include "..\\Data\\RecordFile.h"
#include "..\\Utils\\FolderCtrl.h"
#include "..\\Utils\\TimeUtil.h"
#include "..\\UI\\display.h"
#include "..\\UI\\InputAmount.h"
#include "..\\UI\\InputInvoice.h"
#include "..\\ECRi\\ECRi.h"
#include "..\\MainWnd.h"
#include "..\\COMM\\IPComm.h"
#include "..\\ECRMAIN\\ECR.h"
#include "..\\UI\\EnterCardDlg.h"
#include "..\\TRANSACTIONS\\FinUtil.h"
#include "..\\UTILS\\string.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CECR dialog

CECR::CECR(CWnd* pParent /*=NULL*/)
	: CDialog(CECR::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CECR::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST1, m_List);
}

BEGIN_MESSAGE_MAP(CECR, CDialog)
#if defined(_DEVICE_RESOLUTION_AWARE) && !defined(WIN32_PLATFORM_WFSP)
	ON_WM_SIZE()
#endif
	//}}AFX_MSG_MAP
	ON_WM_PAINT()
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_HOSTINIT, &CECR::OnBnClickedHostinit)
	ON_BN_CLICKED(IDC_PURCHASE, &CECR::OnBnClickedPurchase)
	ON_BN_CLICKED(IDC_REFUND, &CECR::OnBnClickedRefund)
	ON_BN_CLICKED(IDC_CONFIG, &CECR::OnBnClickedConfig)
	ON_WM_COPYDATA()
	ON_BN_CLICKED(IDC_EXIT, &CECR::OnBnClickedExit)
	ON_LBN_SELCHANGE(IDC_LIST1, &CECR::OnLbnSelchangeList1)
	ON_MESSAGE(WM_SHOWMENU_EVENT,&CECR::ShowMenu)
END_MESSAGE_MAP()


// CECR message handlers
	

BOOL CECR::OnCopyData(CWnd* pWnd, COPYDATASTRUCT* pCopyDataStruct) 
{
	CString rxText = (LPCTSTR)(pCopyDataStruct->lpData);
	GetDlgItem(IDC_TEXT)->SetWindowTextW(rxText);
	SetForegroundWindow();
	return CDialog::OnCopyData(pWnd, pCopyDataStruct);
}

BOOL CECR::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here
	int screenX = GetSystemMetrics(SM_CXSCREEN);  
	int screenY = GetSystemMetrics(SM_CYSCREEN);  
	m_bFlag = FALSE;
	MoveWindow( 0, 0, screenX, screenY);
	CString str;
	str.Format(L"TextBox: this thread id is %d",GetCurrentThreadId());
	GetDlgItem(IDC_TEXT)->SetWindowTextW(str);
	
	DWORD dwThreadId = 0;
	HANDLE hThread =::CreateThread(0,0,ThreadProc,0,0,&dwThreadId);
	CloseHandle(hThread);

/*
	SetDlgItemText( IDC_BUTTON1 , _T("H3")  );
	::MoveWindow( ::GetDlgItem( m_hWnd, IDC_BUTTON1 ),  0,   20,   230, 60, TRUE );
	Bt_3.AutoLoad( IDC_BUTTON1, this );
*/

	m_font.CreatePointFont(220,L"Arial"); 
	m_List.SetFont(&m_font);
	m_List.SetItemHeight(1,50);

/*
	m_List.AddString(L" Host Init");
	m_List.AddString(L" Purchase");
	m_List.AddString(L" Refund");
	m_List.AddString(L" Config");
	m_List.AddString(L" Exit(for debug)");
*/
	m_List.SetCurSel(0);
	m_List.MoveWindow(0, 80, screenX, screenY-80);

	CDisplay::Open();
	PostMessage(WM_SHOWMENU_EVENT,0,0);

	SetWindowText(L"CECRMain window");

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CECR::ChangeMenu(int state)
{
	m_List.ResetContent();
	switch(state)
	{
	case 1:
		m_iMenuState = 1;
		m_List.AddString(L" HOST INITIALIZATION");
		m_List.AddString(L" KEY EXCHANGE");
		m_List.AddString(L" CLOSE BATCH");
		m_List.AddString(L" BACK MAIN MENU");
		break;
	case 2:
		m_iMenuState = 2;
		m_List.AddString(L"  PURCHASE");
		m_List.AddString(L"  REFUND");
		m_List.AddString(L"  BACK MAIN MENU");
		break;
	default:
		m_iMenuState = 0;
		m_List.AddString(L"  ADMIN");
		m_List.AddString(L"  FINANCIAL");
		m_List.AddString(L"  EXIT");
		m_List.ShowWindow(TRUE);
//		::ShowWindow(GetDlgItem(IDC_STATIC3)->m_hWnd, FALSE); 
//		::ShowWindow(GetDlgItem(IDC_STATIC4)->m_hWnd, FALSE); 

		break;
	}
	m_List.SetCurSel(0);
	m_Title = L"TRANSACTIONS";
}

void CECR::OnLbnSelchangeList1()
{
	int iItem = m_List.GetCurSel();
	switch(m_iMenuState)
	{
	case 0:
		if(iItem == 2)
			OnBnClickedExit();
		else
			ChangeMenu(iItem+1);
		break;
	case 1:
		if(iItem == 0)
			OnBnClickedHostinit();
		else if(iItem == 1)
			OnBnClickedConfig();
		else if(iItem == 2)
			OnCloseBatch();
		else
			ChangeMenu(0);
		break;
	case 2:
		if(iItem == 0)
			OnBnClickedPurchase();
		else if(iItem == 1)
			OnBnClickedRefund();
		else
			ChangeMenu(0);
		break;
	}
	/*
	switch(m_List.GetCurSel())
	{
	case 0:
		if(m_iMenuState == 0)
			ChangeMenu(1);
		else if(m_iMenuState == 1)
			OnBnClickedHostinit();
		else
			OnBnClickedRefund();
//		OnBnClickedHostinit();
		break;
	case 1:
		if(m_iMenuState == 0)
			ChangeMenu(2);
		else
			OnBnClickedRefund();

//		OnBnClickedPurchase();
		break;
	case 2:
		OnBnClickedRefund();
		break;
	case 3:
		OnBnClickedConfig();
		break;
	case 4:
		OnBnClickedExit();
		break;
	}
	*/
}

LRESULT CECR::ShowMenu(WPARAM wParam, LPARAM lParam)
{
	if(lParam == 1)
	{
		SetForegroundWindow();
		KillTimer(m_iTimer);
		return 0;
	}
	CBitmap bmp;

	bmp.LoadBitmap(IDB_BMLOGO);
		
	CClientDC LCDdc(this);

	CDC dcMem;
	dcMem.CreateCompatibleDC(&LCDdc);
	dcMem.SelectObject(&bmp);

	LCDdc.BitBlt(90,100,156,64,&dcMem,0,0,SRCCOPY);

	DeleteDC(LCDdc);
	Sleep(1000);

	ChangeMenu(0);

	m_bFlag = TRUE;
	SetForegroundWindow();
	Invalidate(TRUE);
//	OnBnClickedHostinit();
//OnBnClickedPurchase();
	return 0;
}

#if defined(_DEVICE_RESOLUTION_AWARE) && !defined(WIN32_PLATFORM_WFSP)
void CECR::OnSize(UINT /*nType*/, int /*cx*/, int /*cy*/)
{
	if (AfxIsDRAEnabled())
	{
		DRA::RelayoutDialog(
			AfxGetResourceHandle(), 
			this->m_hWnd, 
			DRA::GetDisplayMode() != DRA::Portrait ? 
			MAKEINTRESOURCE(IDD_XACECR_DIALOG_WIDE) : 
			MAKEINTRESOURCE(IDD_XACECR_DIALOG));
	}
}
#endif

void CECR::OnBnClickedHostinit()
{
	CDisplay::ClearScreen();
	CDisplay::SetText(1,L"HOST INITIALIZATION");
	CDisplay::ShowStatus( L"Please Wait....");

	FormatEcrRequest(TRAN_HOST_INIT);
	Write(m_Request);

}
	
void CECR::OnCloseBatch()
{
	CDisplay::ClearScreen();
	CDisplay::SetText(1,L"CLOSE BATCH");
	CDisplay::ShowStatus( L"Please Wait....");

	FormatEcrRequest(TRAN_CLOSE_BATCH);
	Write(m_Request);

}

void CECR::OnBnClickedConfig()
{
	CDisplay::ClearScreen();
	CDisplay::SetText(1,L"KEY EXCHANGE");
	CDisplay::ShowStatus(L"Please Wait....");

	FormatEcrRequest(TRAN_KEY_EXCHANGE);
	Write(m_Request);

/*
//	m_strTemp = CLanguage::GetTextFromIndex(CLanguage::IDX_KEY_EXCHANGE);
	m_strTemp =  L"DONE";

	CDisplay::ClearScreen();
	CDisplay::SetText(1,L"KEY EXCHANGE");
	CDisplay::ShowStatus(L"Please Wait....");

//	CDisplay::ShowStatus(CLanguage::IDX_KEY_EXCHANGE);
	m_iTimeStep = 0;
	m_iTimer = 1;
	SetTimer(1,1000,NULL);
*/
}

int CECR::ProcessSwipedCard(char *pTrack2)
{
//	int cardError;
	BOOL result;
	int cardTypeFromTrack2;
//	char servType[SZ_SERVICE_CODE+1];
	TRAN_TranDataRec m_TRREC;
	result = CFinUtil::GetCardInfo(pTrack2, m_TRREC.Account, m_TRREC.ExpDate,
																 &m_TRREC.bChipCard, &cardTypeFromTrack2, 
																 &m_TRREC.CustLang, m_TRREC.ServCode);

	if (!result)
	{
		return 0;
	}
	else
	{	
		int serv = atoi(m_TRREC.ServCode);
		if  ((serv == 120) || (serv == 220) || (serv == 798) || (serv == 799))
		{
			return 1;
		//*pCardType = CARD_DEBIT;
		} 
		else
		{
			return 2;
		//*pCardType = CARD_CREDIT;
		}
	}

	return 0;
}

CString CECR::GetRand(int len)
{
	CString str=L"";
	CString str1=L"";
	CTime t = CTime::GetCurrentTime();
	srand(t.GetMinute()*60+t.GetSecond());
	for(int i=0 ;i<len;i++)
	{
		str1.Format(L"%d",rand()%10);
		str+= str1;
	}
	return str;
}

void CECR::OnBnClickedPurchase()
{
	CDisplay::ClearScreen();
	CDisplay::SetText(1,L"PURCHASE");
	CDisplay::ShowStatus(CLanguage::IDX_PURCHASE);
	FormatEcrRequest(TRAN_PURCHASE);
	Write(m_Request);
	return;

	CDisplay::ClearScreen();
	CDisplay::SetText(1,L"PURCHASE");
	m_iTimer = 2;

	CInputAmount dlg1;
	if(dlg1.DoModal()== IDCANCEL)
		return;
	
	if(dlg1.m_Amount == L"0.00")
	{
			CDisplay::SetText(3,L"PLEASE INPUT AMOUNT");
		    CDisplay::ShowStatus(m_strTemp);
			m_iTimeStep = 4;
			SetTimer(m_iTimer,1000,NULL);
			return;

	}

    CEnterCardDlg dlg;

	char buf[24];

	int len = dlg1.m_Amount.GetLength();	
	char* p = alloc_tchar_to_char(dlg1.m_Amount.GetBuffer(len));
	str_cpy(buf,p);
	dlg1.m_Amount.ReleaseBuffer();

    dlg.SetAmount(buf);
    dlg.SetTransactionTitle(CLanguage::IDX_PURCHASE);
    dlg.DoModal();
    if(dlg.m_KeyReturn == KEY_ENTER)
    {
        char cardno[24];
        char expdata[5];

        BOOL flag = dlg.GetManualTrack2(cardno,expdata);
    }
    else if(dlg.m_KeyReturn == KEY_MSR)
    {
        BYTE p[128]={0};
        BOOL flag = dlg.GetMSRData(2,p);

		int iReturn = ProcessSwipedCard((char*)p);
		if(iReturn == 1)
		{
			m_strTemp = L"DEBIT CARD";
			CDisplay::SetText(3,L"NOT SUPPORTED");
		    CDisplay::ShowStatus(m_strTemp);
			m_iTimeStep = 4;
			SetTimer(m_iTimer,1000,NULL);
			return;
		}
		else if(iReturn==2)
		{
			m_strTemp = L"APPROVED";
			CDisplay::ShowStatus(L"Please Wait....");
			m_iTimeStep = 0;
			SetTimer(m_iTimer,1000,NULL);
			return;

		}
		else
		{
			m_strTemp = L"TRY AGAIN";
			CDisplay::SetText(3,L"NOT READ");
		    CDisplay::ShowStatus(m_strTemp);
			m_iTimeStep = 4;
			SetTimer(m_iTimer,1000,NULL);
			return;

		}
    }
    else if(dlg.m_KeyReturn == KEY_CHIP)
    {
//        BOOL flag = dlg.GetSCRData();
			m_strTemp = L"CHIP CARD";
			CDisplay::SetText(3,L"NOT SUPPORTED");
			CDisplay::ShowStatus(m_strTemp);

			m_iTimeStep = 4;
			SetTimer(m_iTimer,1000,NULL);
		return;

    }
     else if(dlg.m_KeyReturn == KEY_CTLS)
    {
        BYTE p[1024]={0};
		dlg.GetRFIDData(p);
			m_strTemp = L"CTLS CARD";
			CDisplay::SetText(3,L"NOT SUPPORTED");
			CDisplay::ShowStatus(m_strTemp);

			m_iTimeStep = 4;
			SetTimer(m_iTimer,1000,NULL);

		return;

    }    else if(dlg.m_KeyReturn == KEY_TIMEOUT)
    {
        TRACE(L"time out\\n");
		m_strTemp = L"TIME OUT";
		CDisplay::ShowStatus(m_strTemp);
	    m_iTimeStep = 4;
			SetTimer(m_iTimer,1000,NULL);
		return;
    }

	/*	
	CDisplay::ShowStatus(CLanguage::IDX_PURCHASE);
	FormatEcrRequest(TRAN_PURCHASE);
	Write(m_Request);
*/
}

void CECR::OnBnClickedRefund()
{
	CDisplay::ClearScreen();
	CDisplay::SetText(1,L"REFUND");
	m_iTimer = 3;

	CInputAmount dlg1;
	dlg1.m_Title = L"REFUND";
	if(dlg1.DoModal()== IDCANCEL)
		return;
	
	if(dlg1.m_Amount == L"0.00")
	{
			CDisplay::SetText(3,L"PLEASE INPUT AMOUNT");
		    CDisplay::ShowStatus(m_strTemp);
			m_iTimeStep = 4;
			SetTimer(m_iTimer,1000,NULL);
			return;

	}

    CEnterCardDlg dlg;

	char buf[24];

	int len = dlg1.m_Amount.GetLength();	
	char* p = alloc_tchar_to_char(dlg1.m_Amount.GetBuffer(len));
	str_cpy(buf,p);
	dlg1.m_Amount.ReleaseBuffer();

    dlg.SetAmount(buf);
 	dlg.SetTransactionTitle("REFUND");
    dlg.DoModal();
    if(dlg.m_KeyReturn == KEY_ENTER)
    {
        char cardno[24];
        char expdata[5];

        BOOL flag = dlg.GetManualTrack2(cardno,expdata);
    }
    else if(dlg.m_KeyReturn == KEY_MSR)
    {
        BYTE p[128]={0};
        BOOL flag = dlg.GetMSRData(2,p);

		int iReturn = ProcessSwipedCard((char*)p);
		if(iReturn == 1)
		{
			m_strTemp = L"DEBIT CARD";
			CDisplay::SetText(3,L"NOT SUPPORTED");
		    CDisplay::ShowStatus(m_strTemp);
			m_iTimeStep = 4;
			SetTimer(m_iTimer,1000,NULL);
			return;
		}
		else if(iReturn==2)
		{
			m_strTemp = L"APPROVED";
			CDisplay::ShowStatus(L"Please Wait....");
			m_iTimeStep = 0;
			SetTimer(m_iTimer,1000,NULL);
			return;

		}
		else
		{
			m_strTemp = L"TRY AGAIN";
			CDisplay::SetText(3,L"NOT READ");
		    CDisplay::ShowStatus(m_strTemp);
			m_iTimeStep = 4;
			SetTimer(m_iTimer,1000,NULL);
			return;

		}
    }
    else if(dlg.m_KeyReturn == KEY_CHIP)
    {
//        BOOL flag = dlg.GetSCRData();
			m_strTemp = L"CHIP CARD";
			CDisplay::SetText(3,L"NOT SUPPORTED");
			CDisplay::ShowStatus(m_strTemp);

			m_iTimeStep = 4;
			SetTimer(m_iTimer,1000,NULL);
		return;

    }
     else if(dlg.m_KeyReturn == KEY_CTLS)
    {
        BYTE p[1024]={0};
		dlg.GetRFIDData(p);
			m_strTemp = L"CTLS CARD";
			CDisplay::SetText(3,L"NOT SUPPORTED");
			CDisplay::ShowStatus(m_strTemp);

			m_iTimeStep = 4;
			SetTimer(m_iTimer,1000,NULL);
		return;

    }
	 else if(dlg.m_KeyReturn == KEY_TIMEOUT)
    {
        TRACE(L"time out\\n");
		m_strTemp = L"TIME OUT";
		CDisplay::ShowStatus(m_strTemp);
	    m_iTimeStep = 4;
			SetTimer(m_iTimer,1000,NULL);
		return;
    }

}

void CECR::OnTimer(UINT nIDEvent)
{
	CDialog::OnTimer(nIDEvent);
	switch(m_iTimeStep)
	{
	case 0:
		CDisplay::ShowStatus(CLanguage::IDX_SOCKET_SENDING);
		break;
	case 1:
		CDisplay::ShowStatus(CLanguage::IDX_SOCKET_RECEIVING);
		break;
	case 3:
		CDisplay::ShowStatus(CLanguage::IDX_SOCKET_PROCESSING);
		break;
	case 4:
		if(m_strTemp == L"APPROVED")
			CDisplay::SetText(3,GetRand(6));

		CDisplay::ShowStatus(m_strTemp);
		break;
	case 6:
		KillTimer(nIDEvent);
		SetForegroundWindow();
		break;
	}
	m_iTimeStep++;
}

void CECR::DisplayText(CPaintDC* dc)
{
	CFont font;
	font.CreatePointFont(220, _T("MS Sans Serif"));
	CFont* def_font = dc->SelectObject(&font);
	dc->SetTextColor(RGB(0,0,255));
	dc->SetBkColor(RGB(255,255,255));

		CSize sz;
			dc->SetTextColor(RGB(0,0,255));
			sz=dc->GetTextExtent(m_Title);
			int startX = (320-sz.cx)/2;
			dc->ExtTextOut(startX+40, 12, ETO_CLIPPED, NULL, m_Title, wcslen(m_Title), NULL );
			dc->MoveTo(12, 50);
			dc->LineTo(320-12, 50);
	dc->SelectObject(def_font);
	font.DeleteObject();
}

void CECR::OnPaint()
{
	CPaintDC dc(this);
	if(m_bFlag)
	{
		DisplayText(&dc);
		CBitmap bmp;
		bmp.LoadBitmap(IDB_BITMAP1);
		CClientDC LCDdc(this);
		CDC dcMem;
		dcMem.CreateCompatibleDC(&LCDdc);
		dcMem.SelectObject(&bmp);
		LCDdc.BitBlt(0,18,75,28,&dcMem,0,0,SRCCOPY);
		DeleteDC(LCDdc);
	}

}

void CECR::OnBnClickedExit()
{
	CDisplay::Close();
	Write(L"Close");
	CDialog::OnCancel();
}

void CECR::OnCancel()
{
	if(m_iMenuState != 0)
		ChangeMenu(0);
}

void CECR::OnOK()
{
	if(m_iMenuState == 3)
		ChangeMenu(0);
	else
		OnLbnSelchangeList1();
}

DWORD WINAPI CECR::ThreadProc (LPVOID lpArg)
{
	CMainWnd MainWnd;
	MainWnd.DoModal();
	return 0;
}

BOOL CECR::Write(CString txt)
{
	if(m_bCancel)
		return false;
	HWND hwnd = ::FindWindow(NULL,TEXT("CMain window"));
	if( hwnd == NULL ) return false;
		
	COPYDATASTRUCT cpd;
	cpd.dwData = 0;
	cpd.cbData = txt.GetLength()*2;
	cpd.lpData = (void*)txt.GetBuffer(txt.GetLength());
	::SendMessage(hwnd,WM_COPYDATA,(WPARAM)AfxGetApp()->m_pMainWnd->GetSafeHwnd(),(LPARAM)&cpd);
	txt.ReleaseBuffer();
	return true;
}

int CECR::GetTerminalId()
{
	return 12345678;
}

long CECR::GetAmount()
{
	SetForegroundWindow();
	CInputAmount dlg1;
	if(dlg1.DoModal()== IDCANCEL)
	{
		m_bCancel = TRUE;
		return 0;
	}
	char buf[24];
	dlg1.m_Amount.Remove('.');
	dlg1.m_Amount.Remove(',');
	int len = dlg1.m_Amount.GetLength();	
	char* p = alloc_tchar_to_char(dlg1.m_Amount.GetBuffer(len));
	str_cpy(buf,p);
	dlg1.m_Amount.ReleaseBuffer();

	return a2i(buf);
}

long CECR::GetInvoice()
{
	if(m_bCancel)
		return 0;
	SetForegroundWindow();
	CInputInvoice dlg;
	if(dlg.DoModal()== IDCANCEL)
	{
		m_bCancel = TRUE;
		return 0;
	}
	char buf[24];
	int len = dlg.m_Invoice.GetLength();	
	char* p = alloc_tchar_to_char(dlg.m_Invoice.GetBuffer(len));
	str_cpy(buf,p);
	dlg.m_Invoice.ReleaseBuffer();

	return a2i(buf);
}
//-----------------------------------------------------------------------------
//!	\brief	Move char to request
//!	\param	c - char to move
//-----------------------------------------------------------------------------
inline void CECR::Move(char c) 
{
	m_Request.Append(CString(c));
}

//-----------------------------------------------------------------------------
//!	\brief	Move zero-terminated string to request
//!	\param	str - zero-terminated string
//-----------------------------------------------------------------------------
inline void CECR::Move(const char *str)
{
	m_Request.Append(CString(str));
}

//-----------------------------------------------------------------------------
//!	\brief	Move a CString to request
//!	\param	str - CString
//-----------------------------------------------------------------------------
inline void CECR::Move(const CString str)
{
	m_Request.Append(str);
}

//-----------------------------------------------------------------------------
//!	\brief	Move TxnCode to request
//!	\param	iTxnType - tran type
//-----------------------------------------------------------------------------
void CECR::MoveTransactionCode(int iTxnType)
{
	switch(iTxnType)
	{
	case TRAN_HOST_INIT:
		Move("90");
		break;
	case TRAN_KEY_EXCHANGE:
		Move("96");
		break;
	case TRAN_PURCHASE:
		Move("00");
		break;
	case TRAN_CLOSE_BATCH:
		Move("60");
		break;
	default:
		// handle error here
		break;
	}
}

//-----------------------------------------------------------------------------
//!	\brief	Move Terminal Id to request
//-----------------------------------------------------------------------------
void CECR::MoveTerminalId()
{
	CString temp;
	temp.Format(L"%08d", GetTerminalId());
	m_Request += temp;
//	sprintf_s(temp, "%08d", GetTerminalId());
//	Move(temp);
}

//-----------------------------------------------------------------------------
//!	\brief	Move Amount to request
//-----------------------------------------------------------------------------
void CECR::MoveAmount()
{
	char temp[10];
	sprintf(temp, "%07d", GetAmount());
	Move(temp);
}

//-----------------------------------------------------------------------------
//!	\brief	Move Invoice to request
//-----------------------------------------------------------------------------
void CECR::MoveInvoice()
{
	char temp[8];
	sprintf(temp, "%07d", GetInvoice());
	Move(temp);
}
//-----------------------------------------------------------------------------
//!	\brief	formats the ECR message for the selected transaction then sends it
//!	\brief	to the ECRi thread
//-----------------------------------------------------------------------------
CString CECR::FormatEcrRequest(int iTxnType)
{
	m_Request = "";

	m_bCancel = FALSE;
	switch (iTxnType)
	{
	case TRAN_HOST_INIT:
		/*
		Transaction Code	F2	N		M	‘90’ (Initialization)	
		ECR ID						F8	AN	M	
		*/
		MoveTransactionCode(iTxnType);
		Move(FS);
		MoveTerminalId();
		Move(EOT);
		break;
	case TRAN_KEY_EXCHANGE:
		MoveTransactionCode(iTxnType);
		Move(FS);
		MoveTerminalId();
		Move(EOT);
		break;
	case TRAN_PURCHASE:
		MoveTransactionCode(iTxnType);
		Move(FS);
		MoveAmount();
		Move(FS);
		MoveInvoice();
		Move(EOT);
		break;
	case TRAN_CANCEL:
		MoveTransactionCode(iTxnType);
		Move(EOT);
		break;
	case TRAN_CLOSE_BATCH:
		MoveTransactionCode(iTxnType);
		Move(FS);
		MoveTerminalId();
		Move(EOT);
		break;
	default:
		// error handling here
		break;
	}

	return m_Request;
}

//-----------------------------------------------------------------------------
//!	\brief	parses the ECR message received from the ECRi thread
//-----------------------------------------------------------------------------
// This function parses the ECR message received from the ECRi thread
void CECR::ParseEcrResponse()
{
}



