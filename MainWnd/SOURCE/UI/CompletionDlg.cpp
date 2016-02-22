#include "stdafx.h"
#include "CompletionDlg.h"
#include "SaioBase.h"
#include "SaioPinpad.h"

#include "..\\Utils\\string.h"
#include "..\\Utils\\TimeUtil.h"
#include "..\\Utils\\StrUtil.h"
#include "..\\Utils\\TotalUtil.h"
#include "..\\DEFS\\constant.h"
#include "DoConfirmDlg.h"
#include "MsgDlg.h"

//#include "..\\TRANSACTIONS\\TransactionLog.h"
			
#include "..\\utils\\util.h"

#define INVOICE_INPUT			1
#define DISPLAY_INVOICE			2
#define INVOICE_DETAIL			3
#define COMPLETION_CONFIRM		4
#define ENTER_AMOUNT			25

#define ENTER_ORIGINAL			5
#define ENTER_TIP_AMOUNT		6
#define ENTER_CONFIRM_AMOUNT	7


#define CHIPENTRYY				60

//credit swipt
#define CREDITMERCHANTCOPY		85
#define CREDITCUSTOMERCOPY		86
#define CREDITWAITCOPYEND		87
#define CREDITEND				88

#define DOTRANSACTION			100

#define CANCELENTRY		        200
#define CANCELTOCLERK	        201

//#define ERRORENTRY		        300

#pragma comment(lib, "SaioPinpad.lib")

IMPLEMENT_DYNAMIC(CCompletionDlg, CBaseDlg)
CCompletionDlg::CCompletionDlg(CWnd* pParent )
	: CBaseDlg(CBaseDlg::IDD, pParent)
{
}

CCompletionDlg::~CCompletionDlg(void)
{
	CLanguage::SetLanguage();

}

void CCompletionDlg::DoDataExchange(CDataExchange* pDX)
{
	CBaseDlg::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CCompletionDlg, CBaseDlg)
		ON_COMMAND_RANGE(IDC_BUTTON1, IDC_BUTTON2, &CCompletionDlg::OnBnClickedButton)
		ON_WM_CHAR()
END_MESSAGE_MAP()

// CMainWnd message handlers
BOOL CCompletionDlg::OnInitDialog()
{
//	m_Title = L" COMPLETION";
	m_Title.Format(L" %s",CLanguage::GetText(CLanguage::IDX_COMPLETION));
	CBaseDlg::OnInitDialog();

	m_strMsg.SetPointFont(true,210);
	m_strMsg1.SetPointFont(true,210);
	m_strMsg2.SetPointFont(true,210);

	m_TRREC.TranCode = TRAN_ADVICE;
	m_iLevel = 1;

	m_strMsg.ShowWindow(SW_SHOW);
	m_strMsg1.ShowWindow(SW_SHOW);
	m_strMsg2.ShowWindow(SW_SHOW);

	m_Bt.SetValue(1, CLanguage::GetText(CLanguage::IDX_PREV));
	m_Bt.SubclassDlgItem(IDC_BUTTON1, this);
	m_Bt.MoveWindow( 8, screenY- 47, 120, 45);

	m_Bt1.SetValue(3,CLanguage::GetText(CLanguage::IDX_NEXT));
	m_Bt1.SubclassDlgItem(IDC_BUTTON2, this);
	m_Bt1.MoveWindow(screenX-128,  screenY- 47, 120,45);

	m_strMsg3.Create(_T(" "), WS_CHILD, CRect(10, 165, 100, 215), this);
	m_strMsg3.SetPointFont(true,220);
	m_strMsg3.SetColor(RGB(0, 0, 0),RGB( 255,255,255));

	WorkLoop(m_iLevel);
	SetTimer(2,50,NULL);

	DWORD size;
	if(!CDataFile::Read(FID_CFG_CUSTENTRY_TMO, (BYTE*) &m_CustDialogTimer, &size))
		m_CustDialogTimer = 30;

	m_iKeyStatus = 0;

	m_MsgWnd = m_hWnd;
	m_Log.SetFileName(PREAUTH_LOG_FILE);
	m_Log.CheckAgingDays();
	m_iCountLog = m_Log.GetRecordCount();

	m_iShowCount = 0;

	m_bTip = TRUE;		
	TCHAR buf[10] = { L"" };
	if (CDataFile::Read(L"TIP PROMPT", buf) && CString(buf) == L"Off")
		m_bTip = FALSE;

	m_OriginalAmount = L"";
	m_TipAmount = L"";
	return TRUE;  // return TRUE unless you set the focus to a control
}

void CCompletionDlg::OnTimer(UINT_PTR nIDEvent)
{
	if(nIDEvent == 1)
	{
		if (m_iLevel == CANCELENTRY)
			GoNext();
		else
			OnOK();
		return;
	}

	if(nIDEvent == 2)
	{
		SetFocus();
		KillTimer(2);
		if(m_iCountLog == 0)
		{
			m_strErr = CLanguage::GetText(CLanguage::IDX_NO_PREAUTH_RECORDING);
			ErrorExit();
		}

		return;
	}
/*
	if(nIDEvent == 99 && m_iLevel == ENTERCARD_INPUT)
	{
		if (m_SCR.GetState())
			DoEMV();
	}
*/	CBaseDlg::OnTimer(nIDEvent);
}

void CCompletionDlg::WorkLoop(int index)
{
	TCHAR buf[10]={L""};
	m_strMsg.m_nFormat = DT_LEFT;
	m_strMsg1.m_nFormat = DT_LEFT;
	m_strMsg2.m_nFormat = DT_RIGHT;
	m_strMsg.m_szCaption1 = L"";
	m_strMsg1.m_szCaption1 = L"";
	m_strMsg2.m_szCaption1 = L"";

	m_Bt.ShowWindow(SW_HIDE);
	m_Bt1.ShowWindow(SW_HIDE);
	m_strMsg3.ShowWindow(SW_HIDE);

	m_strMsg2.MoveWindow(10, 155, 310, 205);
	CString str=L"";
	CString str1 = L"";
	CString str2 = L"";
	int i = 0;
	switch(index)
	{
	case INVOICE_INPUT: //Invoice Input
		m_bGoBackFlag = FALSE;
		str = CLanguage::GetText(CLanguage::IDX_ENTER_INVOICE);
		str1 = L"";
		m_iShowCount = 0;
		ShowText(str,str1,m_strInvoice);
		break;
	case DISPLAY_INVOICE:  //display invoice
		m_bGoBackFlag = FALSE;

		if( m_index == 0)
		{
			CMsgDlg dlg;
			dlg.m_iTimeOut = 5;
			dlg.m_str1 = CLanguage::GetText(CLanguage::IDX_INVOICE_NUMBER) ;
			dlg.m_str2 = m_strInvoice+L" "+CLanguage::GetText(CLanguage::IDX_IS_NOT_FOUND);
			dlg.m_strBt1 = CLanguage::GetText(CLanguage::IDX_OK2); 
			dlg.m_strBt2 = CLanguage::GetText(CLanguage::IDX_CANCELL);

			if (dlg.DoModal() != IDOK)
			{
				m_strInvoice = L"";
				GoBack();
				break;
			}
			m_strInvoice = L"";
			m_iShowCount = m_iCountLog;
			m_Log.Read(++m_index);
		}

		m_Bt.ShowWindow(m_index > 1?SW_SHOW:SW_HIDE);
		m_Bt1.ShowWindow(m_index < m_iShowCount?SW_SHOW:SW_HIDE);
/*
//		m_strMsg1.m_nFormat = DT_RIGHT;
		str.Format(L"%s %s",CLanguage::GetText(CLanguage::IDX_RECEIPT_INVOICE),
			CString( m_Log.m_Record.TranRec.InvoiceNo)); 

		str1.Format(L"%s %s",CLanguage::GetText(CLanguage::IDX_RECEIPT_AUTH),
			CString( m_Log.m_Record.TranRec.AuthCode)); 

		str2.Format( L"%s        $%s",CLanguage::GetText(CLanguage::IDX_RECEIPT_PREAUTH),
//			CLanguage::GetText(CLanguage::IDX_AMOUNT),
			DealAmountStr(m_Log.m_Record.TranRec.TotalAmount));
*/
		str = CLanguage::GetText(CLanguage::IDX_RECEIPT_INVOICE);
		m_strMsg.m_szCaption1 =	CString( m_Log.m_Record.TranRec.InvoiceNo); 

		str1 = CLanguage::GetText(CLanguage::IDX_RECEIPT_AUTH);
		m_strMsg1.m_szCaption1 = CString( m_Log.m_Record.TranRec.AuthCode);
		m_strMsg1.m_szCaption1.TrimRight(' ');

		m_strMsg2.m_szCaption1 =	CLanguage::GetText(CLanguage::IDX_RECEIPT_PREAUTH);
//			CLanguage::GetText(CLanguage::IDX_AMOUNT),
		str2 = L"$" + DealAmountStr(m_Log.m_Record.TranRec.TotalAmount);

		ShowText(str,str1,str2);
		break;
	case INVOICE_DETAIL:  //invoice detail
		m_bGoBackFlag = FALSE;

		if (m_Log.m_Record.TranRec.CardType == CARD_DEBIT)
			str = CLanguage::GetText(CLanguage::IDX_RECEIPT_DEBIT);
		else if(strcmp(m_Log.m_Record.TranRec.ServType,"V")==0) //Fix me later
			str = CLanguage::GetText(CLanguage::IDX_VISA);
		else if(strcmp(m_Log.m_Record.TranRec.ServType,"M")==0)
			str = CLanguage::GetText(CLanguage::IDX_MASTERCARD);
		else
			str = CString(m_Log.m_Record.TranRec.ServType);

		str1 = CString(m_Log.m_Record.TranRec.Account);
		for(int i=0;i<str1.GetLength()-4;i++)
			str1.SetAt(i,'*');

		if (m_Log.m_Record.TranRec.EntryMode == ENTRY_MODE_MANUAL)
			str2 = CLanguage::GetText(CLanguage::IDX_MANUAL);
		else if (m_Log.m_Record.TranRec.EntryMode == ENTRY_MODE_SWIPED)
			str2 = CLanguage::GetText(CLanguage::IDX_SWIPED);
		else if (m_Log.m_Record.TranRec.EntryMode == ENTRY_MODE_CHIP)
			str2 = CLanguage::GetText(CLanguage::IDX_CHIP);
		else if (m_Log.m_Record.TranRec.EntryMode == ENTRY_MODE_CTLS)
			str2 = CLanguage::GetText(CLanguage::IDX_CTLS);

		ShowText(str,str1,str2);

		if (m_Log.m_Record.TranRec.CardType == CARD_CREDIT)
		{
			str = CString(m_Log.m_Record.TranRec.ExpDate);
			str1.Format(L"%s/%s",str.Mid(0,2),str.Mid(2,2));
			m_strMsg3.SetCaption(str1);
			m_strMsg3.ShowWindow(SW_SHOW);
			m_strMsg2.MoveWindow(150, 165, 150, 215);
		}

		break;
	case COMPLETION_CONFIRM:  //confirm
		m_bGoBackFlag = FALSE;
		m_strMsg2.m_nFormat = DT_LEFT;
		str = L"$" + DealAmountStr(m_Log.m_Record.TranRec.TotalAmount);
		str1=CLanguage::GetText(CLanguage::IDX_PRESS_OK_TO);
		str2= CLanguage::GetText(CLanguage::IDX_CONFIRM)+L"!";
		ShowText(str,str1,str2);
		break;
	case ENTER_AMOUNT:  //enter amount
		if ( m_bTip )
		{
			m_bGoBackFlag?GoBack():GoNext();
			break;
		}
		m_bGoBackFlag = FALSE;
		str=CLanguage::GetText(CLanguage::IDX_ENTER_AMOUNT);
		str1=L"";
		str2=L"$0.00";
		ShowText(str,str1,str2);
		break;

	case ENTER_ORIGINAL: 
		m_bGoBackFlag = FALSE;
		str=CLanguage::GetText(CLanguage::IDX_ENTER_ORIGINAL);
		str1=CLanguage::GetText(CLanguage::IDX_AMOUNT);
		str2= m_OriginalAmount == L""? (L"$" + DealAmountStr(m_Log.m_Record.TranRec.TotalAmount)):m_OriginalAmount;
		ShowText(str,str1,str2);
		break;
	case ENTER_TIP_AMOUNT: 
		if ( !m_bTip )
		{
			m_bGoBackFlag?GoBack():GoNext();
			break;
		}
		m_bGoBackFlag = FALSE;
		str=CLanguage::GetText(CLanguage::IDX_ENTER_TIP);
		str1=CLanguage::GetText(CLanguage::IDX_AMOUNT);
		str2= m_TipAmount == L""?L"$0.00" : m_TipAmount ;
		ShowText(str,str1,str2);
		break;
	case ENTER_CONFIRM_AMOUNT: 
		m_bGoBackFlag = FALSE;
		{
			char temp[3][8];
			PutAmount(temp[1],m_OriginalAmount);
			PutAmount(temp[2],m_TipAmount);
			sprintf(temp[0],"%d",atoi(temp[1])+atoi(temp[2]));
			str = L"$" + DealAmountStr(temp[0]);
			str1=CLanguage::GetText(CLanguage::IDX_PRESS_OK_TO);
			str2=CLanguage::GetText(CLanguage::IDX_CONFIRM);
			m_strMsg2.m_nFormat = DT_LEFT;
			ShowText(str,str1,str2);
		}
		break;

//====================================================================
	case CREDITMERCHANTCOPY: // for merchant copy of the receipt
		m_strMsg2.m_nFormat = DT_LEFT;
		if (m_TRREC.ComStatus == ST_OK  &&
				m_TRREC.TranStatus == ST_APPROVED)
		{
			str = CLanguage::GetText(CLanguage::IDX_APPROVED);
			str1.Format(L"%s : %s",CLanguage::GetText(CLanguage::IDX_AUTH_CODE),CString(m_TRREC.AuthCode));
			m_strMsg2.m_nFormat = DT_LEFT;
			str2.Format(L"%s: %s",CLanguage::GetText(CLanguage::IDX_INVO_NUMBER),CString(m_TRREC.InvoiceNo));
			ShowText(str, str1,str2);
			ShowPicture(1);
		}
		else
		{
			if (strlen(m_TRREC.HostRespText)  > 0)
				str1 = CString(m_TRREC.HostRespText);
			else
				str1.Format(L"%s %s",CLanguage::GetText(CLanguage::IDX_DECLINED),CString(m_TRREC.RespCode));
			str = CLanguage::GetText(CLanguage::IDX_UNSUCCESSFUL);
			str2= L"";
			ShowText(str, str1, str2);
			ShowPicture(2);
		}
		Print();
		break;
	case CREDITCUSTOMERCOPY: // for Customer copy of the receipt
		Sleep(2000);
		str = CLanguage::GetText(CLanguage::IDX_PRESS_OK_FOR);
		str1= CLanguage::GetText(CLanguage::IDX_CUSTOMER_COPY);
		str2 = L"";
		ShowText(str, str1, str2);
		SetTimer(1, 30000, NULL);
		break;
	case CREDITWAITCOPYEND: // waiting for Customer copy of the receipt ending...
		if (m_TRREC.ComStatus == ST_OK  &&
				m_TRREC.TranStatus == ST_APPROVED)
		{
			str = CLanguage::GetText(CLanguage::IDX_APPROVED);
			str1.Format(L"%s : %s",CLanguage::GetText(CLanguage::IDX_AUTH_CODE),CString(m_TRREC.AuthCode));
			m_strMsg2.m_nFormat = DT_LEFT;
			str2.Format(L"%s: %s",CLanguage::GetText(CLanguage::IDX_INVO_NUMBER),CString(m_TRREC.InvoiceNo));
			ShowText(str, str1,str2);
			ShowPicture(1);
		}
		else
		{
			if (strlen(m_TRREC.HostRespText)  > 0)
				str1 = CString(m_TRREC.HostRespText);
			else
				str1.Format(L"%s %s",CLanguage::GetText(CLanguage::IDX_DECLINED),CString(m_TRREC.RespCode));
			str = CLanguage::GetText(CLanguage::IDX_UNSUCCESSFUL);
			str2= L"";
			ShowText(str, str1, str2);
			ShowPicture(2);
		}
		Print(2);
		break;
	case CREDITEND: // ending...
		Sleep(2000);
		CDialog::OnCancel();
		break;

//============================Do transaction ( connect to host)===============
	case DOTRANSACTION:
//		memcpy((void*)&m_TRREC,(void*)&m_Log.m_Record.TranRec,sizeof(m_TRREC));

		memcpy(m_Seq,m_Log.m_Record.TranRec.SequenceNum,SZ_FULL_SEQUENCE_NO+1);
		m_TRREC.TranCode = TRAN_ADVICE;
		DoTransaction();
		break;

	case CANCELENTRY:
		m_bCancelFlag = TRUE;
		str = CLanguage::GetText(CLanguage::IDX_TRANSACTION);
		str1= CLanguage::GetText(CLanguage::IDX_CANCELLED);
		str2 = L"";
		ShowText(str, str1, str2);
		SetTimer(1, 5000, NULL);
		if (m_TRREC.CardError == CARD_NO_ERROR
			&& m_bCancelPrint)
			Print();
		break;
	case CANCELTOCLERK:
		if (m_bCancelInCustomer)
		{
			str = CLanguage::GetText(CLanguage::IDX_PASS_TO);
			str1= CLanguage::GetText(CLanguage::IDX_CLERK);
			str2 = L"";
			ShowText(str, str1, str2);
			SetTimer(1, 3000, NULL);
		}
		else
			CDialog::OnCancel();
		break;
	}
}

void CCompletionDlg::OnCancel()
{
	Beep1();
	switch(m_iLevel)
	{
	case INVOICE_INPUT: //Invoice Input
	case DISPLAY_INVOICE:  //display invoice
	case INVOICE_DETAIL:  //invoice detail
	case COMPLETION_CONFIRM:  //confirm
	case ENTER_AMOUNT: //Enter Amount
	case ENTER_ORIGINAL: 
	case ENTER_TIP_AMOUNT: 
	case ENTER_CONFIRM_AMOUNT: 
		GoToLevel(CANCELENTRY);
		break;
		
//===========================Credit Card Sale ¨C Swiped page 75===================================
	case CREDITMERCHANTCOPY: // for merchant copy of the receipt
		break;
	case CREDITCUSTOMERCOPY: // for Customer copy of the receipt
		KillTimer(1);
		CDialog::OnCancel();
		break;
	case CREDITWAITCOPYEND: // waiting for Customer copy of the receipt ending...
		break;
	case CREDITEND: // ending...
		break;

//===================================================================
	case DOTRANSACTION: // do transaction, can't cancel
		break;

	case CANCELENTRY: 
	case CANCELTOCLERK: 
		break;
	}
}

void CCompletionDlg::OnOK()
{
	Beep1();
	CString str;
	char *p;
	char buf[SZ_INVOICE_NO+1]={0};
	int iTemp;
	switch(m_iLevel)
	{
	case INVOICE_INPUT: //Invoice Input
		m_strInvoice = m_strMsg2.m_szCaption;
		if( m_strInvoice == L"")
		{
			m_index = 1;
			m_Log.Read(m_index);
			m_iShowCount = m_iCountLog;
			break;
		}
		p = alloc_tchar_to_char(m_strInvoice.GetBuffer(m_strInvoice.GetLength()));
		sprintf(buf,"%07d",atoi(p));
		free(p);
		m_strInvoice.ReleaseBuffer();

		m_iShowCount = m_Log.SearchCount((BYTE*)buf);
		m_index =  m_iShowCount > 0 ? 1:0;
		if (m_index > 0 )
			m_Log.Read(m_index,(BYTE*)buf);

//			m_index = m_Log.Search((BYTE*)buf);
//TRACE(L"m_index = %d  Count = %d\n",m_index,m_iCountLog);
		break;
	case DISPLAY_INVOICE:  //display invoice
		break;
	case INVOICE_DETAIL:  //invoice detail
		break;
	case COMPLETION_CONFIRM:  //confirm
		break;
	case ENTER_AMOUNT:
		str = m_strMsg2.m_szCaption;
		str.Remove('$');
		str.Remove(' ');
		str.Remove('.');
		str.Remove(',');
		str.TrimLeft('0');
		if(str.GetLength() == 0)
		{
			Beep();
			return;
		}
		memcpy((void*)&m_TRREC,(void*)&m_Log.m_Record.TranRec,sizeof(m_TRREC));
		PutParam(m_TRREC.Amount,str);
/*
		if(atoi(m_TRREC.Amount) > atoi(m_TRREC.TotalAmount))
		{
			Beep();
			GoToLevel(ENTER_AMOUNT);
			SetFocus();
//			m_strAmount = m_strMsg2.m_szCaption;
			return;
		}
*/		m_strAmount = m_strMsg2.m_szCaption;

		strcpy(m_TRREC.TotalAmount,m_TRREC.Amount);
		GoToLevel(DOTRANSACTION);
		return;

	case ENTER_ORIGINAL:
/*		if (m_strMsg2.m_szCaption == L"$0.00")
		{
			Beep();
			return;
		}
*/		m_OriginalAmount = m_strMsg2.m_szCaption;
		break;
	case ENTER_TIP_AMOUNT: 
		m_TipAmount = m_strMsg2.m_szCaption;
		break;
	case ENTER_CONFIRM_AMOUNT: 
		memcpy((void*)&m_TRREC,(void*)&m_Log.m_Record.TranRec,sizeof(m_TRREC));
		PutAmount(m_TRREC.Amount,m_OriginalAmount);
		PutAmount(m_TRREC.TipAmount,m_TipAmount);
		iTemp = atoi(m_TRREC.Amount) + atoi(m_TRREC.TipAmount);
		if (iTemp <= 0)
			return;
		sprintf(m_TRREC.TotalAmount,"%d",atoi(m_TRREC.Amount) + atoi(m_TRREC.TipAmount));

		GoToLevel(DOTRANSACTION);
		return;
//===========================Credit Card Sale ¨C Swiped page 75===================================
	case CREDITMERCHANTCOPY: // for merchant copy of the receipt
		return;
	case CREDITCUSTOMERCOPY: // for Customer copy of the receipt
		KillTimer(1);
//		Print(2);
		break;
	case CREDITWAITCOPYEND: // waiting for Customer copy of the receipt ending...
//		KillTimer(1);
		return;


//================cancel=====================
	case CANCELENTRY: 
//		return;
		KillTimer(1);
		break;
	case CANCELTOCLERK:
		KillTimer(1);
		CDialog::OnCancel();
		return;
	case ERRORENTRY:
		KillTimer(1);
		CDialog::OnCancel();
		return;
	case ERRORENTRY1:
		KillTimer(1);
		GoToLevel(m_iNextLevel);
		return;
	default:
//		CDialog::OnOK();
		return;
	}
//	m_iLevel++;
//	PostMessage( ERROR_MESSAGE, 2,2 );
	GoNext();
	SetFocus();
}

void CCompletionDlg::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	Beep1();
	switch(m_iLevel)
	{
	case INVOICE_INPUT: //Invoice Input
		m_strMsg2.SetCaption(DealInvoice(nChar,m_strMsg2.m_szCaption));
		break;
	case INVOICE_DETAIL:  //invoice detail
	case COMPLETION_CONFIRM:  //confirm
	case ENTER_CONFIRM_AMOUNT:  //confirm
		if(nChar == 8)
			GoBack();
		break;
	case ENTER_AMOUNT:  //EnterAmount
	case ENTER_ORIGINAL: 
	case ENTER_TIP_AMOUNT: 
		m_strMsg2.SetCaption(DealAmount(nChar,m_strMsg2.m_szCaption));
		break;
	case DISPLAY_INVOICE:  //display invoice
		if (nChar=='1')
			OnBnClickedButton(IDC_BUTTON1);
		if (nChar=='3')
			OnBnClickedButton(IDC_BUTTON2);
		if(nChar == 8)
			GoBack();
		break;


//===========================Credit Card Sale ¨C Swiped page 75===================================
	case CREDITMERCHANTCOPY: // for merchant copy of the receipt
		return;
	case CREDITCUSTOMERCOPY: // for Customer copy of the receipt
		if(nChar == 8)
		{
			KillTimer(1);
			CDialog::OnCancel();
		}
		break;
//=========================End===============================

	}
	MSG msg; 
	 while(::PeekMessage(&msg,NULL,0,0,PM_REMOVE))
	 {
		 if (msg.message==WM_PAINT || msg.message== ERROR_MESSAGE)
			::DispatchMessage(&msg);
	}
	SetFocus();
}

void CCompletionDlg::OnBnClickedButton(UINT nID)
{
	switch(m_iLevel)
	{
	case DISPLAY_INVOICE: // SelectAccount
		if(nID == IDC_BUTTON1)
		{
			if(m_index > 1)
				m_index--;
		}
		else
		{
			if(m_index < m_iShowCount)
				m_index++;
		}
		if( m_strInvoice == L"")
		{
			if(m_Log.Read(m_index))
				GoToLevel(m_iLevel);
		}
		else
		{
			char buf[SZ_INVOICE_NO+1]={0};
			char* p = alloc_tchar_to_char(m_strInvoice.GetBuffer(m_strInvoice.GetLength()));
			sprintf(buf,"%07d",atoi(p));
			free(p);
			m_strInvoice.ReleaseBuffer();
			if(m_Log.Read(m_index,(BYTE*)buf))
				GoToLevel(m_iLevel);
		}
		break;
	}
}

void CCompletionDlg::Done()
{
	AfterHost();
	if (m_TRREC.ComStatus == ST_OK &&
			m_TRREC.TranStatus == ST_APPROVED)
	{
		m_Log.RemoveSeqRecord(m_Seq);
/*		char buf[SZ_INVOICE_NO+1]={0};
		char* p = alloc_tchar_to_char(m_strInvoice.GetBuffer(m_strInvoice.GetLength()));
		sprintf(buf,"%07d",atoi(p));
		free(p);
		m_strInvoice.ReleaseBuffer();
		m_Log.RemoveRecord(m_index,(BYTE*)buf);
*/	}

	GoToLevel(CREDITMERCHANTCOPY);
}

LRESULT CCompletionDlg::DefWindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	CString strTemp; 
	DWORD len = 0;
    DWORD decode=0;

	if(wParam == 2 && message == ERROR_MESSAGE)
	{ 
		if(lParam == 1) //error
		{
			ErrorExit();
		}
		else if(lParam == 2) //for jump to other level
			WorkLoop(m_iLevel);
		else if(lParam == 3) // print done
		{
			m_bPrintDone = TRUE;
			GoNext();
		}
		else if(lParam == 4) //Can't print
		{
			m_strErr = CLanguage::GetText(CLanguage::IDX_CAN_NOT_PRINT);//L"Can't Print";
			ErrorExit();
		}
	}
	return CDialog::DefWindowProc(message, wParam, lParam);
}

void CCompletionDlg::DoTransaction()
{
	if (!IsNetWorkConnect())
	{
		m_strErr = CLanguage::GetText(CLanguage::IDX_NO_NETWORK);
		ErrorExit();
		return;
	}
	m_strMsg.SetCaption(CLanguage::GetText(CLanguage::IDX_PLEASE_WAIT));
	m_strMsg1.SetCaption(L"");
	m_strMsg2.SetCaption(L"");
	CFinan::DoTransaction();
}

void CCompletionDlg::ErrorExit()
{
	m_strMsg.SetColor(RGB(255, 0, 0),RGB( 255,255,255));
	m_strMsg1.SetColor(RGB(255, 0, 0),RGB( 255,255,255));
	m_strMsg.SetCaption(CLanguage::GetText(CLanguage::IDX_ERROR));
	m_strMsg1.SetCaption(m_strErr);
	m_strMsg2.SetCaption(L"");
	m_iLevel = ERRORENTRY;
	SetTimer(1,5000,NULL);
}
//----------------------------------------------------------------------------
//!	\brief	Error display and go to level
// param level is level index
//----------------------------------------------------------------------------
void CCompletionDlg::ErrorExit(int level)
{
	m_strMsg.m_nFormat = DT_LEFT;
	m_strMsg1.m_nFormat = DT_LEFT;
	m_strMsg.SetColor(RGB(255, 0, 0),RGB( 255,255,255));
	m_strMsg1.SetColor(RGB(255, 0, 0),RGB( 255,255,255));
	m_strMsg.SetCaption(CLanguage::GetText(CLanguage::IDX_ERROR));
	m_strMsg1.SetCaption(m_strErr);
	m_strMsg2.SetCaption(L"");
	m_iLevel = ERRORENTRY1;
	SetTimer(1,5000,NULL);
	m_iNextLevel = level;
}
void CCompletionDlg::OnDestroy()
{
	KillTimer(1);
	CBaseDlg::OnDestroy();
}

void CCompletionDlg::GoBack()
{
	switch(m_iLevel)
	{
	case CHIPENTRYY:
//		m_iLevel = ENTERCARD_INPUT;
		return;
	case INVOICE_INPUT:
		CDialog::OnCancel();
		return;
//	case AMOUNT_INPUT:
//			return;
	default:
		m_bGoBackFlag = TRUE;
		m_iLevel--;
		break;
	}
	KillTimer(1);
	PostMessage( ERROR_MESSAGE, 2,2 );
}

void CCompletionDlg::GoNext()
{
	m_iLevel++;
	PostMessage( ERROR_MESSAGE, 2,2 );
}

void CCompletionDlg::GoToLevel(int level)
{
	m_iLevel = level;
	PostMessage( ERROR_MESSAGE, 2,2 );
}

