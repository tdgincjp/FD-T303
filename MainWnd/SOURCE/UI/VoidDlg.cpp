#include "stdafx.h"
#include "VoidDlg.h"
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
	
#include "..\\UI\\display.h"
		
#include "..\\utils\\util.h"
#include "..\\EMV\\PinInput.h"

#define PASSWORD_INPUT			0
#define INVOICE_INPUT			1
#define DISPLAY_INVOICE			2
#define INVOICE_DETAIL			3
#define VOID_CONFIRM			4
#define ENTERCARD_INPUT			5

//swiped debit
#define DEBITCARDENTRY			21
#define DEBITTOTALCOMFIRM		22
#define DEBITSELECTACCOUNT		23
#define DEBITONLINEPIN			24
#define DEBITMERCHANTCOPY		25
#define DEBITCUSTOMERCOPY		26
#define DEBITWAITCOPYEND		27
#define DEBITEND				28


#define CHIPENTRYY							60
#define SELECT_LANGUAGE					61
#define SELECT_APPLICATIION			62
#define APPLICATION_CONFIRM			63
#define CHIP_DEBITTOTALCOMFIRM	64
#define CHIP_DEBITSELECTACCOUNT	65
#define CHIP_DEBITENTERPIN			66
#define CHIP_PASSTOCLERK				67
#define CHIP_DEBITMERCHANTCOPY	68
#define CHIP_DEBITCUSTOMERCOPY	69
#define CHIP_DEBITWAITCOPYEND		70
#define CHIP_DEBITEND						71

//credit swipt
#define CREDITMERCHANTCOPY		85
#define CREDITCUSTOMERCOPY		86
#define CREDITWAITCOPYEND		87
#define CREDITEND				88

#define DOTRANSACTION			100

#define CANCELENTRY							200
#define CANCELEMVCARDBEREMOVED				201
#define CANCELTOCLERK						202
#define CANCELPRINT						    203
#define CANCELCUSTOMERCOPY					204
#define CANCELEND							205

#define EMVCARDBEREMOVED					206

#pragma comment(lib, "SaioPinpad.lib")

IMPLEMENT_DYNAMIC(CVoidDlg, CBaseDlg)
CVoidDlg::CVoidDlg(CWnd* pParent )
	: CBaseDlg(CBaseDlg::IDD, pParent)
{
}

CVoidDlg::~CVoidDlg(void)
{
	CLanguage::SetLanguage();
}

void CVoidDlg::DoDataExchange(CDataExchange* pDX)
{
	CBaseDlg::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CVoidDlg, CBaseDlg)
		ON_COMMAND_RANGE(IDC_BUTTON1, IDC_BUTTON2, &CVoidDlg::OnBnClickedButton)
		ON_WM_CHAR()
END_MESSAGE_MAP()

// CMainWnd message handlers
BOOL CVoidDlg::OnInitDialog()
{
//	m_Title = L"  VOID";
	m_Title.Format(L"   %s",CLanguage::GetText(CLanguage::IDX_VOID));
	m_TitleIndex = CLanguage::IDX_VOID;
	CBaseDlg::OnInitDialog();

	m_strMsg.SetPointFont(true,210);
	m_strMsg1.SetPointFont(true,210);
	m_strMsg2.SetPointFont(true,210);

	m_TRREC.TranCode = TRAN_PURCHASE_VOID;
	m_iLevel = 0;

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

	DWORD size;
	if (!CDataFile::Read(FID_CFG_CUSTENTRY_TMO, (BYTE*) &m_CustDialogTimer, &size))
		m_CustDialogTimer = 30;

	m_iKeyStatus = 0;

	m_MsgWnd = m_hWnd;
	CDisplay::SetDisplayWnd(m_MsgWnd);
	CLanguage::SetDisplayWnd(m_MsgWnd);
	m_iCountLog = m_Log.GetRecordCount();
	if(m_iCountLog == 0)
	{
		m_strErr1 = CLanguage::GetText(CLanguage::IDX_ERROR);
		m_strErr2 = CLanguage::GetText(CLanguage::IDX_NO_RECORDING_LOG);
		ErrorExit();
	}
	else
		SetTimer(2, 50, NULL);

	ShowPicture(0);
	return TRUE;  // return TRUE unless you set the focus to a control
}

void CVoidDlg::OnTimer(UINT_PTR nIDEvent)
{
	if (nIDEvent == 1)
	{
		KillTimer(1);
		if (m_iLevel == ENTERCARD_INPUT)
		{
			CloseVoidDevice();
			m_strErr = CLanguage::GetText(CLanguage::IDX_TIME_OUT);
			m_strErr1 = L"";
			m_strErr2 = L"";
			ErrorExit();
			return;
		}
		if (m_iLevel == CANCELENTRY)
			GoNext();
		else
			OnOK();
		return;
	}

	if (nIDEvent == 2)
	{
		SetFocus();
		KillTimer(2);
		if(m_iCountLog == 0)
		{
			m_strErr = CLanguage::GetText(CLanguage::IDX_NO_RECORDING_LOG);
			ErrorExit();
		}

		return;
	}
	if (nIDEvent == 99)
	{
		if (m_iLevel == ENTERCARD_INPUT && !m_KeyEntry)
		{
			if (m_SCR.GetState())
			{
				KillTimer(1);
//				memcpy((void*)&m_TRREC,(void*)&m_Log.m_Record.TranRec,sizeof(m_TRREC));
				if( m_Log.m_Record.TranRec.TranCode == TRAN_REFUND )
					m_TRREC.TranCode = TRAN_REFUND_VOID;
				else 
					m_TRREC.TranCode = TRAN_PURCHASE_VOID;


//				strcpy(Track2,m_Log.m_Record.TranRec.Track2);
				m_pLogTRREC = &m_Log.m_Record.TranRec;
				strcpy(m_TRREC.Amount,m_pLogTRREC->Amount);
				strcpy(m_TRREC.TipAmount,m_pLogTRREC->TipAmount);
				strcpy(m_TRREC.CashbackAmount,m_pLogTRREC->CashbackAmount);
				strcpy(m_TRREC.SurchargeAmount,m_pLogTRREC->SurchargeAmount);
				strcpy(m_TRREC.TotalAmount,m_pLogTRREC->TotalAmount);
				StartEMV();
				m_iLevel = DISPLAY_WINDOW;
//				ShowText(CLanguage::GetText(CLanguage::IDX_PASS_TO),CLanguage::GetText(CLanguage::IDX_CLERK), L"");
			}
			else
			{
/*				if(m_SCR.m_bPowerOff)
				{
					m_SCR.Close();
					CloseDevice();
					m_TRREC.bFallback = TRUE;
					m_strErr = CLanguage::GetText(CLanguage::IDX_EMV_CHIP_ERROR);
					m_strErr1 = CLanguage::GetText(CLanguage::IDX_FALLBACK_TO_MAGSTRIPE);
					m_strErr2 = CLanguage::GetText(CLanguage::IDX_PLEASE_REMOVE_CARD);
					ErrorExit(ENTERCARD_INPUT);
				}
*/			}
		}

		if (m_iLevel == EMV_REMOVE_CARD || m_iLevel == CHIP_DEBITEND || m_iLevel == EMVCARDBEREMOVED || m_iLevel == CANCELEMVCARDBEREMOVED)
		{
/*			if(m_TRREC.CardType == CARD_CREDIT)
			{
				CDialog::OnCancel();
				return;
			}
*/
			if (GetScrStatus()!= SCR_CARD_ABSENT && m_TRREC.EntryMode == ENTRY_MODE_CHIP)
			{
				Beep();
				return;
			}
			else
			{
				if ( m_TRREC.bFallback && m_TRREC.EntryMode > ENTRY_MODE_MANUAL)
					GoToLevel(ENTERCARD_INPUT);
				else
				{
					if(m_iNextLevel != 0)
					{
						GoToLevel(m_iNextLevel);
						m_iNextLevel = 0;
					}
					else
						CDialog::OnCancel();
				}
			}
		}	
	}
		
	CBaseDlg::OnTimer(nIDEvent);
}

void CVoidDlg::WorkLoop(int index)
{
	char* pbuf;
	TCHAR buf[10] = { L"" };
	m_strMsg.m_nFormat = DT_LEFT;
	m_strMsg1.m_nFormat = DT_LEFT;
	m_strMsg2.m_nFormat = DT_RIGHT;
	m_Bt.ShowWindow(SW_HIDE);
	m_Bt1.ShowWindow(SW_HIDE);
	m_strMsg.SetColor(RGB(0, 0, 0),RGB( 255,255,255));
	m_strMsg1.SetColor(RGB(0, 0, 0),RGB( 255,255,255));
	m_strMsg2.SetColor(RGB(0, 0, 0),RGB( 255,255,255));

	m_strMsg.m_szCaption1 = L"";
	m_strMsg1.m_szCaption1 = L"";
	m_strMsg2.m_szCaption1 = L"";

	m_strMsg2.MoveWindow(10, 155, 310, 205);
	CString str = L"";
	CString str1 = L"";
	CString str2 = L"";
	int i = 0;
	switch(index)
	{
	case PASSWORD_INPUT: //Password Input
		m_bGoBackFlag = FALSE;
		for ( i = 0;i<m_strPassword.GetLength();i++)
			str += L"*";
		ShowText(CLanguage::GetText(CLanguage::IDX_ENTER_PASSWORD),L"",str);
		break;
	case INVOICE_INPUT: //Invoice Input
		m_bGoBackFlag = FALSE;
		ShowText(CLanguage::GetText(CLanguage::IDX_ENTER_INVOICE),L"",m_strInvoice);
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
			m_Log.Read(++m_index);
		}
		m_Bt.ShowWindow(m_index > 1?SW_SHOW:SW_HIDE);
		m_Bt1.ShowWindow(m_index < m_iCountLog?SW_SHOW:SW_HIDE);
//		m_strMsg1.m_nFormat = DT_RIGHT;
/*		str.Format(L"%s %s",CLanguage::GetText(CLanguage::IDX_RECEIPT_INVOICE),
			CString( m_Log.m_Record.TranRec.InvoiceNo)); 

		str1.Format(L"%s %s",CLanguage::GetText(CLanguage::IDX_RECEIPT_AUTH),
			CString( m_Log.m_Record.TranRec.AuthCode)); 

		str2.Format( L"%s        $%s",
			m_Log.m_Record.TranRec.TranCode == TRAN_REFUND?CLanguage::GetText(CLanguage::IDX_RECEIPT_RETURN):CLanguage::GetText(CLanguage::IDX_PURCHASE),
//			CLanguage::GetText(CLanguage::IDX_AMOUNT),
			DealAmountStr(m_Log.m_Record.TranRec.TotalAmount));
*/
		str = CLanguage::GetText(CLanguage::IDX_RECEIPT_INVOICE);
		m_strMsg.m_szCaption1 =	CString( m_Log.m_Record.TranRec.InvoiceNo); 

		str1 = CLanguage::GetText(CLanguage::IDX_RECEIPT_AUTH);
		m_strMsg1.m_szCaption1 = CString( m_Log.m_Record.TranRec.AuthCode);
		m_strMsg1.m_szCaption1.TrimRight(' ');

		m_strMsg2.m_szCaption1 = m_Log.m_Record.TranRec.TranCode == TRAN_REFUND?CLanguage::GetText(CLanguage::IDX_RECEIPT_RETURN):CLanguage::GetText(CLanguage::IDX_PURCHASE),
		str2 = L"$" + DealAmountStr(m_Log.m_Record.TranRec.TotalAmount);

		ShowText(str,str1,str2);
		break;
	case INVOICE_DETAIL:  //invoice detail
		m_bGoBackFlag = FALSE;

		if (m_Log.m_Record.TranRec.CardType == CARD_DEBIT)
		{
			str = CLanguage::GetText(CLanguage::IDX_RECEIPT_DEBIT);
			i = 4;
		}
		else if(m_Log.m_Record.TranRec.ServType[0] == 'V')
			str = CLanguage::GetText(CLanguage::IDX_VISA);
		else if(m_Log.m_Record.TranRec.ServType[0] == 'M')
			str = CLanguage::GetText(CLanguage::IDX_MASTERCARD);
		else
			str = CString(m_Log.m_Record.TranRec.ServType);

		str1 = CString(m_Log.m_Record.TranRec.Account);
		for(;i<str1.GetLength()-4;i++)
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
	case VOID_CONFIRM:  //confirm
		m_bGoBackFlag = FALSE;
		m_strMsg2.m_nFormat = DT_LEFT;
		str.Format(L"%s %s",CLanguage::GetText(CLanguage::IDX_VOID_INV),CString(m_Log.m_Record.TranRec.InvoiceNo));
		ShowText(str,CLanguage::GetText(CLanguage::IDX_PRESS_OK_TO),CLanguage::GetText(CLanguage::IDX_CONFIRM)+L"!");
		break;

// debit card 
	case ENTERCARD_INPUT:  //Card enter and manual Entry
		m_bGoBackFlag = FALSE;
		m_bCancelPrint = FALSE;
		m_TRREC.bEmvTransaction = FALSE;
		m_TRREC.EntryMode = 0;
		m_TRREC.bFallback = FALSE;

		m_iNextLevel = 0;
		ShowPicture(0);
		if(!OpenVoidDevice())
		{
			m_strErr = CLanguage::GetText(CLanguage::IDX_FAIL_OPEN_DEVICE);
			PostMessage( ERROR_MESSAGE, 2, 1);
			break;
		}
		m_strMsg.m_nFormat = DT_RIGHT;
		ShowText(L"$" + DealAmountStr(m_Log.m_Record.TranRec.TotalAmount),CLanguage::GetText(CLanguage::IDX_INSERT_SWIPE),L"");
		SetTimer(1,30000,NULL);
		break;
	case DEBITCARDENTRY:
		m_bCancelPrint = TRUE;
		m_bGoBackFlag = FALSE;
		ShowText(CLanguage::GetText(CLanguage::IDX_PASS_TO),CLanguage::GetText(CLanguage::IDX_CUSTOMER),L"");
		SetTimer(1,3000,NULL);
		m_bCancelInCustomer = TRUE;
		CLanguage::SetCustomerFlag(TRUE);
		break;

	case DEBITTOTALCOMFIRM: // TotalComfirm
		m_bGoBackFlag = FALSE;
		memcpy((void*)&m_TRREC,(void*)&m_Log.m_Record.TranRec,sizeof(m_TRREC));
		if(DoVoidConfirm())
			GoNext();
		else
			OnCancel();
		break;
	case DEBITSELECTACCOUNT: // SelectAccount
		GoNext();
		break;
		m_bGoBackFlag = FALSE;
		CLanguage::GetTextFromIndex(CLanguage::IDX_CHEQUE,&pbuf);
		m_Bt.SetValue(1, CString(pbuf));
//		m_Bt.SetValue(1, L"  CHEQUE");
		CLanguage::GetTextFromIndex(CLanguage::IDX_SAVING,&pbuf);
		m_Bt1.SetValue(3, CString(pbuf));
//		m_Bt1.SetValue(3, L"  SAVING");
		m_Bt.ShowWindow(SW_SHOW);
		m_Bt1.ShowWindow(SW_SHOW);
		ShowText(CLanguage::GetText(CLanguage::IDX_SELECT_ACCOUNT),L"",L"");
		break;
	case DEBITONLINEPIN: //OnlinePin
		if (!EnterOnlinePin())
		{
			m_strErr1 = CLanguage::GetText(CLanguage::IDX_ENTER_ONLINE_PIN);
			PostMessage(ERROR_MESSAGE, 2, 1);
		}
		else
		{
			str = CLanguage::GetText(CLanguage::IDX_PLEASE_WAIT);//L"Please Wait...";
			str1= L"";
			str2= L"";
			ShowText(str, str1, str2);
			GoToLevel(DOTRANSACTION);
		}
		break;
	case DEBITMERCHANTCOPY: // for merchant copy of the receipt
		CLanguage::SetCustomerFlag(FALSE);
		if (m_TRREC.ComStatus == ST_OK  &&
				m_TRREC.TranStatus == ST_APPROVED)
			str = CLanguage::GetText(CLanguage::IDX_VOIDED);
		else
			str = CLanguage::GetText(CLanguage::IDX_UNSUCCESSFUL);
		m_strMsg2.m_nFormat = DT_LEFT;
		ShowText(str,CLanguage::GetText(CLanguage::IDX_RETRIEVE_CARD),CLanguage::GetText(CLanguage::IDX_PASS_TO_CLERK));
		Print();
		break;
	case DEBITCUSTOMERCOPY: // for Customer copy of the receipt
		Sleep(2000);
		ShowText(CLanguage::GetText(CLanguage::IDX_PRESS_OK_FOR),CLanguage::GetText(CLanguage::IDX_CUSTOMER_COPY),L"");
		SetTimer(1,30000,NULL);
		break;
	case DEBITWAITCOPYEND: // waiting for Customer copy of the receipt ending...
		if (m_TRREC.ComStatus == ST_OK  &&
				m_TRREC.TranStatus == ST_APPROVED)
		{
			str.Format(L"%s : %s",CLanguage::GetText(CLanguage::IDX_AUTH_CODE),CString(m_TRREC.AuthCode));
			m_strMsg2.m_nFormat = DT_LEFT;
			str1.Format(L"%s: %s",CLanguage::GetText(CLanguage::IDX_INVO_NUMBER),CString(m_TRREC.InvoiceNo));
			ShowText(CLanguage::GetText(CLanguage::IDX_VOIDED),str,str1);
		}
		else
		{
			str.Format(L"%s %s",CLanguage::GetText(CLanguage::IDX_DECLINED),CString(m_TRREC.RespCode));
			ShowText(CLanguage::GetText(CLanguage::IDX_UNSUCCESSFUL),str,L"");
		}
		Print(2);
		break;
	case DEBITEND: // ending...
		Sleep(2000);
		CDialog::OnCancel();
		break;
//====================================================================
// == == == == == == == == == == == == == = EMV Chip Card Debit Sale ¡§C chip page 122 == == == == == == == == == == == == == = 
	case CHIPENTRYY:
		m_bCancelPrint = TRUE;
		m_bGoBackFlag = FALSE;
		CLanguage::GetTextFromIndex(CLanguage::IDX_PASS_TO,&pbuf);
		str = CString(pbuf);
		CLanguage::GetTextFromIndex(CLanguage::IDX_CUSTOMER,&pbuf);
		str1=  CString(pbuf);
		str2 = L"";
		ShowText(str, str1,str2);
		SetTimer(1, 3000, NULL);
		m_bCancelInCustomer = TRUE;
		CLanguage::SetCustomerFlag(TRUE);
		break;
	case SELECT_LANGUAGE:
		if (SupportEnglishOrFrench())
		{
			m_bGoBackFlag?GoBack():GoNext();
		}
		else
		{
			m_bGoBackFlag = FALSE;
			m_Bt.SetValue(1, L" English");
			m_Bt.ShowWindow(SW_SHOW);
			m_Bt1.SetValue(3, L" Francais");
			m_Bt1.ShowWindow(SW_SHOW);
			str = CLanguage::GetText(CLanguage::IDX_SELECT_LANGUAGE);//L"Select language";
			str1= L"";//L"Choisir langue";
			str2 = L"";
			ShowText(str, str1,str2);
		}
		break;
	case CHIP_DEBITTOTALCOMFIRM: // TotalComfirm
		m_bGoBackFlag = FALSE;
		memcpy((void*)&m_TRREC,(void*)&m_Log.m_Record.TranRec,sizeof(m_TRREC));
		if(DoVoidConfirm())
		{
			m_strMsg2.m_nFormat = DT_LEFT;
			CLanguage::GetTextFromIndex(CLanguage::IDX_PLEASE_WAIT,&pbuf);
			str = CString(pbuf);//L"Please Wait...";
			CLanguage::GetTextFromIndex(CLanguage::IDX_DO_NOT_REMOVE,&pbuf);
			str1 = CString(pbuf);
			CLanguage::GetTextFromIndex(CLanguage::IDX_CARD,&pbuf);
			str2=  CString(pbuf);
			ShowText(str, str1, str2);
			m_iLevel = EMV_WAIT;
			SetKeyReturn(KEY_ENTER);//			GoNext();
		}
		else
			OnCancel();
		break;
	case CHIP_DEBITSELECTACCOUNT: // SelectAccount
		if (m_TRREC.CardType == CARD_CREDIT)
		{
//			m_bGoBackFlag?GoBack():GoNext();
			m_strMsg2.m_nFormat = DT_LEFT;
			CLanguage::GetTextFromIndex(CLanguage::IDX_PLEASE_WAIT,&pbuf);
			str = CString(pbuf);//L"Please Wait...";
//			str1= L"Do not remove";
//			str2 = L"Card!";
			CLanguage::GetTextFromIndex(CLanguage::IDX_DO_NOT_REMOVE,&pbuf);
			str1 = CString(pbuf);
			CLanguage::GetTextFromIndex(CLanguage::IDX_CARD,&pbuf);
			str2=  CString(pbuf);
			ShowText(str, str1, str2);
			m_iLevel = EMV_WAIT;
			SetKeyReturn(KEY_ENTER);
			return;
		}
		GoNext();
		break;

		m_bGoBackFlag = FALSE;
		CLanguage::GetTextFromIndex(CLanguage::IDX_CHEQUE,&pbuf);
		m_Bt.SetValue(1, CString(pbuf));
//		m_Bt.SetValue(1, L"  CHEQUE");
		CLanguage::GetTextFromIndex(CLanguage::IDX_SAVING,&pbuf);
		m_Bt1.SetValue(3, CString(pbuf));
//		m_Bt1.SetValue(3, L"  SAVING");
		m_Bt.ShowWindow(SW_SHOW);
		m_Bt1.ShowWindow(SW_SHOW);
		CLanguage::GetTextFromIndex(CLanguage::IDX_SELECT_ACCOUNT,&pbuf);
		str = CString(pbuf);//L"Select Account";
		str1= L"";
		str2 = L"";
		ShowText(str, str1, str2);
		break;
	case CHIP_DEBITENTERPIN: //EnterPin
		if (!EMVEnterPin())//Fix me later
		{
			m_strErr1 = L"EMVEnterPin";
			PostMessage(ERROR_MESSAGE, 2, 1);
		}
		else
		{
			if (m_TRREC.CardType == CARD_CREDIT)
			{
				GoNext();
			}
			else
			{
				CLanguage::GetTextFromIndex(CLanguage::IDX_PLEASE_WAIT,&pbuf);
				str = CString(pbuf);//L"Please Wait...";
				str1= L"";
				str2 = L"";
				ShowText(str, str1, str2);
				GoToLevel(DOTRANSACTION);
			}
		}
		break;
	case CHIP_PASSTOCLERK: //Pass to clerk
		CLanguage::GetTextFromIndex(CLanguage::IDX_PASS_TO_CLERK,&pbuf);
		str = CString(pbuf);
		CLanguage::GetTextFromIndex(CLanguage::IDX_DO_NOT_REMOVE,&pbuf);
		str1 = CString(pbuf);
		CLanguage::GetTextFromIndex(CLanguage::IDX_CARD,&pbuf);
		str2=  CString(pbuf);
		ShowText(str, str1, str2);
		SetTimer(1, 3000, NULL);
		CLanguage::SetCustomerFlag(FALSE);
		break;

	case CHIP_DEBITMERCHANTCOPY: // for merchant copy of the receipt
		if (m_TRREC.ComStatus == ST_OK  &&
				m_TRREC.TranStatus == ST_APPROVED)
			CLanguage::GetTextFromIndex(CLanguage::IDX_VOIDED,&pbuf);
//			str = L"APPROVED";
		else
			CLanguage::GetTextFromIndex(CLanguage::IDX_UNSUCCESSFUL,&pbuf);
//			str = L"Unsuccessful";
		str = CString(pbuf);
		m_strMsg2.m_nFormat = DT_LEFT;
		str1 = L"";
		if(m_TRREC.EntryMode == ENTRY_MODE_CHIP)
		{
			CLanguage::GetTextFromIndex(CLanguage::IDX_REMOVE_CARD,&pbuf);
			str1 = CString(pbuf);
		}
//		str1= L"Remove card";
		CLanguage::GetTextFromIndex(CLanguage::IDX_PASS_TO_CLERK,&pbuf);
		str2 = CString(pbuf);
//		str2 = L"Pass to clerk";
		ShowText(str, str1, str2);
		CLanguage::SetCustomerFlag(FALSE);
		if ( m_TRREC.bFallback && m_TRREC.EntryMode > ENTRY_MODE_MANUAL)
			GoToLevel(CHIP_DEBITEND);
		else
			Print();
		break;
	case CHIP_DEBITCUSTOMERCOPY: // for Customer copy of the receipt
		Sleep(2000);
		CLanguage::GetTextFromIndex(CLanguage::IDX_PRESS_OK_FOR,&pbuf);
		str = CString(pbuf);
		CLanguage::GetTextFromIndex(CLanguage::IDX_CUSTOMER_COPY,&pbuf);
		str1 = CString(pbuf);
		str2 = L"";
		ShowText(str, str1, str2);
		SetTimer(1, 30000, NULL);
		break;
	case CHIP_DEBITWAITCOPYEND: // waiting for Customer copy of the receipt ending...
		if (m_TRREC.ComStatus == ST_OK  &&
				m_TRREC.TranStatus == ST_APPROVED)
		{
			CLanguage::GetTextFromIndex(CLanguage::IDX_VOIDED,&pbuf);
			str = CString(pbuf);//L"APPROVED";
			CLanguage::GetTextFromIndex(CLanguage::IDX_AUTH_CODE,&pbuf);
			str1.Format(L"%s : %s", CString(pbuf),CString(m_TRREC.AuthCode));
			m_strMsg2.m_nFormat = DT_LEFT;
			CLanguage::GetTextFromIndex(CLanguage::IDX_INVO_NUMBER,&pbuf);
			str2.Format(L"%s: %s", CString(pbuf),CString(m_TRREC.InvoiceNo));
			ShowText( str, str1 ,str2);
		}
		else
		{
			CLanguage::GetTextFromIndex(CLanguage::IDX_UNSUCCESSFUL,&pbuf);
			str = CString(pbuf);//L"Unsuccessful";
			CLanguage::GetTextFromIndex(CLanguage::IDX_DECLINED,&pbuf);
			str1.Format(L"%s %s", CString(pbuf),CString(m_TRREC.RespCode));
			str2 = L"";
			ShowText(str, str1,str2);
		}
		Print(2);
		break;
	case CHIP_DEBITEND: // ending...
		Sleep(2000);
		if (GetScrStatus()!= SCR_CARD_ABSENT && m_TRREC.CardType != CARD_CREDIT && m_TRREC.EntryMode == ENTRY_MODE_CHIP)
		{
			CLanguage::GetTextFromIndex(CLanguage::IDX_PLEASE,&pbuf);
			str = CString(pbuf);
//			str = L"Please";
			CLanguage::GetTextFromIndex(CLanguage::IDX_REMOVE_CARD,&pbuf);
			str1 = CString(pbuf);
//			str1= L"Remove Card";
			str2 = L"";
			ShowText(str, str1, str2);
		}
		else
		{
			if ( m_TRREC.bFallback && m_TRREC.EntryMode > ENTRY_MODE_MANUAL)
			{
				Sleep(2000);
				GoToLevel(ENTERCARD_INPUT);
			}
			else
				CDialog::OnCancel();
		}
		break;

	case CREDITMERCHANTCOPY: // for merchant copy of the receipt
		m_strMsg2.m_nFormat = DT_LEFT;
		if (m_TRREC.ComStatus == ST_OK  &&
				m_TRREC.TranStatus == ST_APPROVED)
		{
			str.Format(L"%s : %s",CLanguage::GetText(CLanguage::IDX_AUTH_CODE),CString(m_TRREC.AuthCode));
			m_strMsg2.m_nFormat = DT_LEFT;
			str1.Format(L"%s : %s",CLanguage::GetText(CLanguage::IDX_INVO_NUMBER),CString(m_TRREC.InvoiceNo));
			ShowText(CLanguage::GetText(CLanguage::IDX_VOIDED),str,str1);
		}
		else
		{
			CLanguage::GetTextFromIndex(CLanguage::IDX_UNSUCCESSFUL,&pbuf);
			str = CString(pbuf);//L"Unsuccessful";

			str1 = CString(m_TRREC.HostRespText);
			if ( str1 == L"")
				str1 = CLanguage::GetText(CLanguage::IDX_DECLINED);
			str2 = L"";
			ShowText(str, str1,str2);
		}
		CLanguage::SetCustomerFlag(FALSE);
		Print();
		break;
	case CREDITCUSTOMERCOPY: // for Customer copy of the receipt
		Sleep(2000);
		ShowText(CLanguage::GetText(CLanguage::IDX_PRESS_OK_FOR),CLanguage::GetText(CLanguage::IDX_CUSTOMER_COPY),L"");
		SetTimer(1,30000,NULL);
		break;
	case CREDITWAITCOPYEND: // waiting for Customer copy of the receipt ending...
		if (m_TRREC.ComStatus == ST_OK  &&
				m_TRREC.TranStatus == ST_APPROVED)
		{
			str.Format(L"%s : %s",CLanguage::GetText(CLanguage::IDX_AUTH_CODE),CString(m_TRREC.AuthCode));
			m_strMsg2.m_nFormat = DT_LEFT;
			str1.Format(L"%s : %s",CLanguage::GetText(CLanguage::IDX_INVO_NUMBER),CString(m_TRREC.InvoiceNo));
			ShowText(CLanguage::GetText(CLanguage::IDX_VOIDED),str,str1);
		}
		else
		{
			CLanguage::GetTextFromIndex(CLanguage::IDX_UNSUCCESSFUL,&pbuf);
			str = CString(pbuf);//L"Unsuccessful";
			CLanguage::GetTextFromIndex(CLanguage::IDX_DECLINED,&pbuf);
			str1.Format(L"%s %s", CString(pbuf),CString(m_TRREC.RespCode));
			str2 = L"";
			ShowText(str, str1,str2);
		}
		Print(2);
		break;
	case CREDITEND: // ending...
		Sleep(2000);
		CDialog::OnCancel();
		break;

//============================Do transaction ( connect to host)===============
	case DOTRANSACTION:
		memcpy((void*)&m_TRREC,(void*)&m_Log.m_Record.TranRec,sizeof(m_TRREC));

		if(m_TRREC.TranCode == TRAN_REFUND)
			m_TRREC.TranCode = TRAN_REFUND_VOID;
		else 
			m_TRREC.TranCode = TRAN_PURCHASE_VOID;
		DoTransaction();
		break;

	case CANCELENTRY:
#ifdef __TESTING__
		CDialog::OnCancel();
		return;
#endif
		m_bCancelFlag = TRUE;
		CLanguage::GetTextFromIndex(CLanguage::IDX_TRANSACTION,&pbuf);
		str = CString(pbuf);
		CLanguage::GetTextFromIndex(CLanguage::IDX_CANCELLED,&pbuf);
		str1 = CString(pbuf);
		str2 = L"";
		ShowText(str, str1, str2);
		SetTimer(1, 5000, NULL);
		break;
	case CANCELEMVCARDBEREMOVED:
		m_bCancelFlag = TRUE;
		if (GetScrStatus()!= SCR_CARD_ABSENT && m_TRREC.EntryMode == ENTRY_MODE_CHIP)
		{
			str = CLanguage::GetText(CLanguage::IDX_REMOVE_CARD);
			str1 = L"";
			str2 = L"";
			ShowText(str, str1, str2);
			m_iNextLevel = CANCELTOCLERK;
		}
		else
			GoNext();
		break;
	case CANCELTOCLERK:
		if (CLanguage::m_bPassToCustomer)//m_bCancelInCustomer)
		{
			CLanguage::GetTextFromIndex(CLanguage::IDX_PASS_TO,&pbuf);
			str = CString(pbuf);
			CLanguage::GetTextFromIndex(CLanguage::IDX_CLERK,&pbuf);
			str1=  CString(pbuf);
			str2 = L"";
			ShowText(str, str1, str2);
			SetTimer(1, 3000, NULL);
		    CLanguage::SetCustomerFlag(FALSE);
		}
		else
			GoNext();
		break;

	case CANCELPRINT:
		if (m_TRREC.CardError == CARD_NO_ERROR
			&& m_bCancelPrint)
			Print();
		else
			CDialog::OnCancel();
		break;

	case CANCELCUSTOMERCOPY: // for Customer copy of the receipt
		if ( !m_bCancelPrint )
		{
			CDialog::OnCancel();
			break;
		}
		CLanguage::GetTextFromIndex(CLanguage::IDX_PRESS_OK_FOR,&pbuf);
		str = CString(pbuf);
		CLanguage::GetTextFromIndex(CLanguage::IDX_CUSTOMER_COPY,&pbuf);
		str1 = CString(pbuf);
		//str= L"Press OK for";
		//str1= L"customer copy";
		str2 = L"";
		ShowText(str, str1, str2);
		SetTimer(1, 30000, NULL);
		break;
	case CANCELEND: // for Customer copy of the receipt
		Sleep(2000);
		CDialog::OnCancel();
		break;

// == == == == == == == Display Window == == == == == 
	case DISPLAY_WINDOW:
		m_strMsg2.m_nFormat = DT_LEFT;
		str = CDisplay::GetLine(1);
		if (str.GetLength() > 0)
			m_strTitle.SetCaption(str);

		str = CDisplay::GetLine(2);
		ShowText(str, CDisplay::GetLine(3), CDisplay::GetLine(4));
		SetDisplayButton();
		break;

	case EMV_SELECT_LANGUAGE: //EMV Select language
		m_Bt.SetValue(1, L" English");
		m_Bt.ShowWindow(SW_SHOW);
		m_Bt1.SetValue(3, L" Francais");
		m_Bt1.ShowWindow(SW_SHOW);
		str = CLanguage::GetText(CLanguage::IDX_SELECT_LANGUAGE);//L"Select language";
		str1= L"";//L"Choisir langue";
		str2 = L"";
		ShowText(str, str1, str2);
		break;

	case EMV_SELECT_ACCOUNT: //EMV select account
		CLanguage::GetTextFromIndex(CLanguage::IDX_CHEQUE,&pbuf);
		m_Bt.SetValue(1, CString(pbuf));
		CLanguage::GetTextFromIndex(CLanguage::IDX_SAVING,&pbuf);
		m_Bt1.SetValue(3, CString(pbuf));
		m_Bt.ShowWindow(SW_SHOW);
		m_Bt1.ShowWindow(SW_SHOW);
		CLanguage::GetTextFromIndex(CLanguage::IDX_SELECT_ACCOUNT,&pbuf);
		str = CString(pbuf);//L"Select Account";
		str1= L"";
		str2 = L"";
		ShowText(str, str1, str2);
		break;
	
	case EMV_REMOVE_CARD: //EMV Remove Card
		{
			int temp = 0;
			CDataFile::Save(FID_CFG_EMVCARD_CHECK, (BYTE *)&temp, sizeof(int));
		}

		if (GetScrStatus()!= SCR_CARD_ABSENT && m_TRREC.EntryMode == ENTRY_MODE_CHIP)
		{
			CLanguage::GetTextFromIndex(CLanguage::IDX_PLEASE,&pbuf);
			str = CString(pbuf);
//			str = L"Please";
			CLanguage::GetTextFromIndex(CLanguage::IDX_REMOVE_CARD,&pbuf);
			str1 = CString(pbuf);
//			str1= L"Remove Card";
			str2 = L"";
			ShowText(str, str1, str2);
		}
		else
		{
			if(m_iNextLevel != 0)
			{
				GoToLevel(m_iNextLevel);
				m_iNextLevel = 0;
			}
			else
				CDialog::OnCancel();
		}
		break;
	
	case EMV_PASS_COUSTOMER: //EMV card pass to customer
		CLanguage::GetTextFromIndex(CLanguage::IDX_PASS_TO,&pbuf);
		str = CString(pbuf);
		CLanguage::GetTextFromIndex(CLanguage::IDX_CUSTOMER,&pbuf);
		str1=  CString(pbuf);
//		str = L"Pass to";
//		str1= L"Customer";
		str2 = L"";
		ShowText(str, str1, str2);
		SetTimer(1,3000,NULL);
		CLanguage::SetCustomerFlag(TRUE);
		StartEMV();
		break;
	
	case EMV_START: //EMV start
		m_strMsg2.m_nFormat = DT_LEFT;
		str = CLanguage::GetText(CLanguage::IDX_EMV_PLEASE_WAIT);

		CLanguage::GetTextFromIndex(CLanguage::IDX_DO_NOT_REMOVE,&pbuf);
		str1 = CString(pbuf);
		CLanguage::GetTextFromIndex(CLanguage::IDX_CARD,&pbuf);
		str2=  CString(pbuf);
		ShowText(str, str1, str2);
//		StartEMV();
		m_iLevel = DISPLAY_WINDOW;
		break;

//	case EMVCARDBEREMOVED:
//		break;

	}
}

//----------------------------------------------------------------------------
//!	\brief	chekc display window's button number and move to right position
//----------------------------------------------------------------------------
void CVoidDlg::SetDisplayButton()
{
	int iTemp = 1;
	int BtNumber = 3;
	CString strBt1 = CDisplay::GetButtonText(1);
	CString strBt2 = CDisplay::GetButtonText(2);
	CString strBt3 = CDisplay::GetButtonText(3);

	BOOL flag = FALSE;
	if (strBt1 == L" English" || strBt1 ==  CLanguage::GetText(CLanguage::IDX_YES) )
		flag = TRUE;
	switch(CDisplay::GetBtNumber())
	{
	case 1:
		m_Bt.SetValue(-1, strBt1);
		m_Bt.MoveWindow( iTemp,screenY-50,screenX/BtNumber-iTemp, 45);
		m_Bt.ShowWindow(SW_SHOW);
		break;
	case 2:
		m_Bt.SetValue(flag ? 1:-1, strBt1);
		m_Bt.MoveWindow( iTemp,screenY-50,screenX/BtNumber-iTemp, 45);
		m_Bt.ShowWindow(SW_SHOW);
		m_Bt1.SetValue(flag ? 3:-1, strBt2);
		m_Bt1.MoveWindow( screenX/BtNumber*2+iTemp,screenY-50,screenX/BtNumber-iTemp, 45);
		m_Bt1.ShowWindow(SW_SHOW);
		break;
	}
}

void CVoidDlg::OnCancel()
{
	Beep1();
	switch(m_iLevel)
	{
	case PASSWORD_INPUT: //Password Input
	case INVOICE_INPUT: //Invoice Input
	case DISPLAY_INVOICE:  //display invoice
	case INVOICE_DETAIL:  //invoice detail
	case VOID_CONFIRM:  //confirm
		GoToLevel(CANCELENTRY);
		break;
	case ENTERCARD_INPUT:  //Card enter and manual Entry
		CloseVoidDevice();
		GoToLevel(CANCELENTRY);
		break;
		
// debit card 
	case DEBITCARDENTRY:
	case DEBITTOTALCOMFIRM: // TotalComfirm
		GoToLevel(CANCELENTRY);
		break;
	case DEBITSELECTACCOUNT: // SelectAccount
		GoToLevel(CANCELENTRY);
		break;
	case DEBITONLINEPIN: //OnlinePin
		break;
	case DEBITMERCHANTCOPY: // for merchant copy of the receipt
		break;
	case DEBITCUSTOMERCOPY: // for Customer copy of the receipt
		KillTimer(1);
		CDialog::OnCancel();
		break;
	case DEBITWAITCOPYEND: // waiting for Customer copy of the receipt ending...
		break;
	case DEBITEND: // ending...
		break;
// == == == == == == == == == == == == == = EMV Chip Card Debit Sale ¡§C chip page 122 == == == == == == == == == == == == == = 
	case CHIPENTRYY:
		break;
	case SELECT_LANGUAGE: //select language
//	case SELECT_APPLICATIION: //select application
//	case APPLICATION_CONFIRM: //select comfirm
		GoToLevel(CANCELENTRY);
		break;
/*	case CHIP_DEBITTOTALCOMFIRM: // TotalComfirm
		GoToLevel(CANCELENTRY);
		break;
	case CHIP_DEBITSELECTACCOUNT: // SelectAccount
		GoToLevel(CANCELENTRY);
		break;
*/	case CHIP_DEBITENTERPIN: //EnterPin
		break;
	case CHIP_PASSTOCLERK: //pass to clerk
		break;
	case CHIP_DEBITMERCHANTCOPY: // for merchant copy of the receipt
		break;
	case CHIP_DEBITCUSTOMERCOPY: // for Customer copy of the receipt
		KillTimer(1);
		GoToLevel(CHIP_DEBITEND);
		break;
	case CHIP_DEBITWAITCOPYEND: // waiting for Customer copy of the receipt ending...
		break;
	case CHIP_DEBITEND: // ending...
		break;
	case CREDITMERCHANTCOPY: // for merchant copy of the receipt
		break;
	case CREDITCUSTOMERCOPY: // for Customer copy of the receipt
	case CANCELCUSTOMERCOPY: // for Customer copy of the receipt
		KillTimer(1);
		if(m_TRREC.bEmvTransaction)
		{
			Sleep(10);
			GoToLevel(EMV_REMOVE_CARD);
			break;
		}
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
		break;
	case CANCELTOCLERK: 
//		GoToLevel(CANCELENTRY);
		break;
// == == == == == == == Display Window == == == == == 
	case DISPLAY_WINDOW:
		if (CDisplay::GetMask() & MASK_CANCEL)
			CDisplay::SetKeyReturn(KEY_CANCEL);
		break;
	case EMV_SELECT_LANGUAGE: //EMV Select language
//	case EMV_CONFIRM_AMOUNT: //EMV confirm amount
	case EMV_SELECT_ACCOUNT: //EMV select account
	case EMV_PASS_COUSTOMER: //pass to customer
		SetKeyReturn(KEY_CANCEL);
		break;
	case CHIP_DEBITTOTALCOMFIRM: // TotalComfirm
	case CHIP_DEBITSELECTACCOUNT: // SelectAccount
		GoToLevel(CANCELENTRY);
		SetKeyReturn(KEY_CANCEL);
		break;
	case EMV_WAIT:
		SetEMVCancel();
		break;
	}
}

//----------------------------------------------------------------------------
//!	\brief	handle OK key press function for any GUI's, 
//----------------------------------------------------------------------------
void CVoidDlg::OnOK()
{
	Beep1();
	MSG msg; 
	 while(::PeekMessage(&msg,NULL,0,0,PM_REMOVE))
	 {
		 if (msg.message==WM_PAINT || msg.message== ERROR_MESSAGE)
			::DispatchMessage(&msg);
	}

	CString str,str1,str2;
	char *p, *pbuf;
	char buf[SZ_INVOICE_NO+1]={0};
	switch(m_iLevel)
	{
	case PASSWORD_INPUT: //Password Input
		if(m_strPassword.GetLength() == 0)
		{
			Beep();
			return;
		}
		if(!CheckPassword())
		{
			m_IncorrectTimes++;
			if(m_IncorrectTimes>=5)
			{
				CDataFile::Save(L"CLERK PW",L"On");
				CDialog::OnCancel();
			}
			else
			{
				Beep();
				m_strPassword = L"";
				m_strMsg2.SetCaption(L"");
			}
		    return;
		}
		break;
	case INVOICE_INPUT: //Invoice Input
		m_strInvoice = m_strMsg2.m_szCaption;
		if( m_strInvoice == L"")
		{
			m_index = 1;
			m_Log.Read(m_index);
			break;
		}
		p = alloc_tchar_to_char(m_strInvoice.GetBuffer(m_strInvoice.GetLength()));
		sprintf(buf,"%07d",atoi(p));
		free(p);
		m_strInvoice.ReleaseBuffer();
		m_index = m_Log.Search((BYTE*)buf);
//TRACE(L"m_index = %d  Count = %d\n",m_index,m_iCountLog);
		break;
	case DISPLAY_INVOICE:  //display invoice
		break;
	case INVOICE_DETAIL:  //invoice detail
		break;
	case VOID_CONFIRM:  //confirm
		if(m_Log.m_Record.TranRec.CardType == CARD_DEBIT)
			  GoToLevel(ENTERCARD_INPUT);
		else
			  GoToLevel(DOTRANSACTION);
		return;
	case ENTERCARD_INPUT:
		KillTimer(1);
		return;

	case DEBITCARDENTRY:
		KillTimer(1);
		break;
	case DEBITTOTALCOMFIRM: // TotalComfirm
		break;
//	case DEBITSELECTACCOUNT: // SelectAccount
//		break;
	case DEBITONLINEPIN: //OnlinePin
		break;
	case DEBITMERCHANTCOPY: // for merchant copy of the receipt
		return;
	case DEBITCUSTOMERCOPY: // for Customer copy of the receipt
		KillTimer(1);
//		Print(2);
		break;

	case DEBITWAITCOPYEND: // waiting for Customer copy of the receipt ending...
//		KillTimer(1);
		return;
// == == == == == == == == == == == == == = EMV Chip Card Debit Sale ¡§C chip page 122 == == == == == == == == == == == == == = 
	case CHIPENTRYY:
		KillTimer(1);
		break;
	case SELECT_LANGUAGE: //select language
//	case SELECT_APPLICATIION: //select application
//	case APPLICATION_CONFIRM: //select comfirm
		Beep();
		return;
/*
	case CHIP_DEBITTOTALCOMFIRM: // TotalComfirm
		break;
//	case CHIP_DEBITSELECTACCOUNT: // SelectAccount
//		break;
*/	case CHIP_DEBITENTERPIN: //EnterPin
		break;
	case CHIP_PASSTOCLERK: //pass to clerk
		KillTimer(1);
		GoToLevel(DOTRANSACTION);
		return;
	case CHIP_DEBITMERCHANTCOPY: // for merchant copy of the receipt
		return;
	case CHIP_DEBITCUSTOMERCOPY: // for Customer copy of the receipt
		KillTimer(1);
//		Print(2);
		break;
	case CHIP_DEBITWAITCOPYEND: // waiting for Customer copy of the receipt ending...
//		KillTimer(1);
		return;
	case CREDITMERCHANTCOPY: // for merchant copy of the receipt
		return;
	case CREDITCUSTOMERCOPY: // for Customer copy of the receipt
		KillTimer(1);
		break;
	case CANCELCUSTOMERCOPY: // for Customer copy of the receipt
		KillTimer(1);
		Print(2);
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
		break;
//		CDialog::OnCancel();
//		return;
	case ERRORENTRY:
		KillTimer(1);
		GoToLevel ( CANCELEMVCARDBEREMOVED );
/*		if ( m_TRREC.bEmvTransaction && m_TRREC.EntryMode == ENTRY_MODE_CHIP)
		{
			GoToLevel(EMV_REMOVE_CARD);
		}
		else
			CDialog::OnCancel();
*/		return;
	case ERRORENTRY1:
		KillTimer(1);
		if ( m_TRREC.bEmvTransaction && m_TRREC.EntryMode == ENTRY_MODE_CHIP)
		{
			GoToLevel(EMV_REMOVE_CARD);
		}
		else
		{
			GoToLevel(m_iNextLevel);
			m_iNextLevel = 0;
		}
		return;

// == == == == == == == Display Window == == == == == 
	case DISPLAY_WINDOW:
		if (CDisplay::GetMask() & MASK_OK)
			CDisplay::SetKeyReturn(KEY_ENTER);
		return;
	case EMV_SELECT_LANGUAGE: //EMV Select language
//	case EMV_CONFIRM_AMOUNT: //EMV confirm amount
	case EMV_SELECT_ACCOUNT: //EMV select account
		m_strMsg2.m_nFormat = DT_LEFT;
		CLanguage::GetTextFromIndex(CLanguage::IDX_PLEASE_WAIT,&pbuf);
		str = CString(pbuf);//L"Please Wait...";
		//			str1= L"Do not remove";
		//			str2 = L"Card!";
		CLanguage::GetTextFromIndex(CLanguage::IDX_DO_NOT_REMOVE,&pbuf);
		str1 = CString(pbuf);
		CLanguage::GetTextFromIndex(CLanguage::IDX_CARD,&pbuf);
		str2=  CString(pbuf);
		ShowText(str, str1, str2);
		m_iLevel = EMV_WAIT;
		SetKeyReturn(KEY_ENTER);
		return;
	case EMV_PASS_COUSTOMER: //pass to customer
		KillTimer(1);
		break;

	case CHIP_DEBITTOTALCOMFIRM: // TotalComfirm
		break;
	case CHIP_DEBITSELECTACCOUNT: // SelectAccount
		Beep();
		return;
	default:
//		CDialog::OnOK();
		return;
	}
	GoNext();
	SetFocus();
}

void CVoidDlg::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	Beep1();
	switch(m_iLevel)
	{
	case PASSWORD_INPUT: //Password Input
		m_strMsg2.SetCaption(DealPassword(nChar, m_strPassword));
		break;
	case INVOICE_INPUT: //Invoice Input
		m_strMsg2.SetCaption(DealInvoice(nChar,m_strMsg2.m_szCaption));
		break;
	case INVOICE_DETAIL:  //invoice detail
	case VOID_CONFIRM:  //confirm
		if(nChar == 8)
			GoBack();
		break;
	case DISPLAY_INVOICE:  //display invoice
	case DEBITSELECTACCOUNT: // SelectAccount
	case CHIP_DEBITSELECTACCOUNT: // SelectAccount
		if (nChar=='1')
			OnBnClickedButton(IDC_BUTTON1);
		if (nChar=='3')
			OnBnClickedButton(IDC_BUTTON2);
		if (nChar == 8)
			GoBack();
		break;
	case ENTERCARD_INPUT:  //Card enter and manual Entry
		if(nChar == 8)
		{
			CloseVoidDevice();
			GoToLevel(VOID_CONFIRM);
			m_bGoBackFlag = TRUE;
		}
		break;

	case DEBITMERCHANTCOPY: // for merchant copy of the receipt
	case CHIP_DEBITMERCHANTCOPY: // for merchant copy of the receipt
		return;
	case DEBITCUSTOMERCOPY: // for Customer copy of the receipt
		if (nChar == 8)
		{
			KillTimer(1);
			CDialog::OnCancel();
		}
		break;
	case DEBITWAITCOPYEND: // waiting for Customer copy of the receipt ending...
	case CHIP_DEBITWAITCOPYEND: // waiting for Customer copy of the receipt ending...
		break;
// == == == == == == == == == == == == == = EMV Chip Card Debit Sale ¡§C chip page 122 == == == == == == == == == == == == == = 
	case SELECT_LANGUAGE: //select language
//	case SELECT_APPLICATIION: //select application
///	case APPLICATION_CONFIRM: //select comfirm
		if (nChar == '1')
			OnBnClickedButton(IDC_BUTTON1);
		if (nChar == '3')
			OnBnClickedButton(IDC_BUTTON2);
		break;
	case CHIP_DEBITCUSTOMERCOPY: // for Customer copy of the receipt
		if (nChar == 8)
		{
			KillTimer(1);
			GoToLevel(CHIP_DEBITEND);
		}
		break;

//===========================Credit Card Sale ¨C Swiped page 75===================================
	case CREDITMERCHANTCOPY: // for merchant copy of the receipt
		return;
	case CREDITCUSTOMERCOPY: // for Customer copy of the receipt
	case CANCELCUSTOMERCOPY: // for Customer copy of the receipt
		if (nChar == 8)
		{
			KillTimer(1);
			CDialog::OnCancel();
		}
		break;

// == == == == == == == Display Window == == == == == 
	case DISPLAY_WINDOW:
		if (nChar == '1')
			OnBnClickedButton(IDC_BUTTON1);
		else if (nChar == '3')
			OnBnClickedButton(IDC_BUTTON2);
		else if (nChar == 8)
		{
			if (CDisplay::GetMask() & MASK_CORR)
				CDisplay::SetKeyReturn(KEY_CORR);
		}
		break;
	case EMV_SELECT_LANGUAGE: //EMV Select language
	case EMV_SELECT_ACCOUNT: //EMV select account
		if (nChar == '1')
			OnBnClickedButton(IDC_BUTTON1);
		if (nChar == '3')
			OnBnClickedButton(IDC_BUTTON2);
		if (nChar == 8)
			SetKeyReturn(KEY_CORR);
		break;
//	case EMV_CONFIRM_AMOUNT: //EMV confirm amount
//		if (nChar == 8)
//			SetKeyReturn(KEY_CORR);
//		break;
// == == == == == == == == == == == == = End == == == == == == == == == == == == == == == = 
	}
	MSG msg; 
	 while(::PeekMessage(&msg,NULL,0,0,PM_REMOVE))
	 {
		 if (msg.message==WM_PAINT || msg.message== ERROR_MESSAGE)
			::DispatchMessage(&msg);
	}
	SetFocus();
}
//----------------------------------------------------------------------------
//!	\brief	handle button press function for any GUI's, 
//----------------------------------------------------------------------------
void CVoidDlg::OnBnClickedButton(UINT nID)
{
	CString str,str1,str2;
	char* pbuf;
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
			if(m_index < m_iCountLog)
				m_index++;
		}
		if(m_Log.Read(m_index))
			GoToLevel(m_iLevel);
		break;
	case DEBITSELECTACCOUNT: // SelectAccount
		if (nID == IDC_BUTTON1)
			m_TRREC.AcctType = ACCT_CHEQUE;
		else
			m_TRREC.AcctType = ACCT_SAVING;
		GoNext();
		break;



	case SELECT_LANGUAGE: //EMV Select language
		if (nID == IDC_BUTTON1)
			EMVSelectLanguage(ENGLISH);
		else
			EMVSelectLanguage(FRENCH);
		GoNext();
		break;
/*	case SELECT_APPLICATIION: //select application
		if (nID == IDC_BUTTON1)
			GoToLevel(SELECT_APPLICATIION);
		else
			GoNext();
		break;
	case APPLICATION_CONFIRM: //select comfirm
		if (nID == IDC_BUTTON1)
			GoToLevel(SELECT_APPLICATIION);
		else
			GoNext();
		break;
*/
// == == == == == == == Display Window == == == == == 
	case DISPLAY_WINDOW:
		CDisplay::SetKeyReturn(KEY_BUT1+nID-IDC_BUTTON1);
		break;
	case EMV_SELECT_LANGUAGE: //EMV Select language
	case EMV_SELECT_ACCOUNT: //EMV select account
		m_Bt.ShowWindow(SW_HIDE);
		m_Bt1.ShowWindow(SW_HIDE);
		m_strMsg2.m_nFormat = DT_LEFT;
		CLanguage::GetTextFromIndex(CLanguage::IDX_PLEASE_WAIT,&pbuf);
		str = CString(pbuf);//L"Please Wait...";
		//			str1= L"Do not remove";
		//			str2 = L"Card!";
		CLanguage::GetTextFromIndex(CLanguage::IDX_DO_NOT_REMOVE,&pbuf);
		str1 = CString(pbuf);
		CLanguage::GetTextFromIndex(CLanguage::IDX_CARD,&pbuf);
		str2=  CString(pbuf);
		ShowText(str, str1, str2);
		m_iLevel = EMV_WAIT;
		SetKeyReturn(KEY_BUT1+nID-IDC_BUTTON1);
		break;
	case CHIP_DEBITSELECTACCOUNT: // SelectAccount
		m_Bt.ShowWindow(SW_HIDE);
		m_Bt1.ShowWindow(SW_HIDE);
		if(nID == IDC_BUTTON1)
			m_TRREC.AcctType = ACCT_CHEQUE;
		else
			m_TRREC.AcctType = ACCT_SAVING;
		m_strMsg2.m_nFormat = DT_LEFT;
		CLanguage::GetTextFromIndex(CLanguage::IDX_PLEASE_WAIT,&pbuf);
		str = CString(pbuf);//L"Please Wait...";
//			str1= L"Do not remove";
//			str2 = L"Card!";
			CLanguage::GetTextFromIndex(CLanguage::IDX_DO_NOT_REMOVE,&pbuf);
			str1 = CString(pbuf);
			CLanguage::GetTextFromIndex(CLanguage::IDX_CARD,&pbuf);
			str2=  CString(pbuf);
		ShowText(str, str1, str2);
		m_iLevel = EMV_WAIT;
		SetKeyReturn(KEY_ENTER);
		break;
	}
}

void CVoidDlg::Done()
{
	if (m_TRREC.ComStatus == ST_OK &&
			m_TRREC.TranStatus == ST_APPROVED)
	{
		m_Log.RemoveRecord(m_index);
		ShowPicture(1);
	}
	else
		ShowPicture(2);

	AfterHost();
	if(m_TRREC.CardType == CARD_DEBIT)
	{
		if (m_TRREC.EntryMode == ENTRY_MODE_SWIPED)
			GoToLevel(DEBITMERCHANTCOPY);
		else if (m_TRREC.EntryMode == ENTRY_MODE_CHIP)
			GoToLevel(CHIP_DEBITMERCHANTCOPY);
	}
	else		  
		GoToLevel(CREDITMERCHANTCOPY);
}

LRESULT CVoidDlg::DefWindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	CString strTemp; 
	DWORD len = 0;
   DWORD decode = 0;
   char* pbuf;

	if (wParam == 0 && m_bCloseDevice)
	{
		switch( message )
		{
			case MSR_EVENT_DEV_OFFLINE :
				m_strErr1 = CLanguage::GetText(CLanguage::IDX_MSR_OPEN_FAIL);
				ErrorExit();
				return NULL;
			case MSR_EVENT_DATA_READY :
//			case MSR_EVENT_CT_STATUS :
			case MSR_EVENT_READ_ERROR :
				m_MSR.MsrGetTrackData();
				m_MSR.GetMSRData(2, (BYTE*)m_TRREC.Track2, &len, &decode);
				m_TRREC.EntryMode = ENTRY_MODE_SWIPED;
				m_TRREC.bEmvCardRead = FALSE;
				m_TRREC.CardError = ProcessSwipedCard(m_TRREC.Track2);
				if (m_TRREC.CardError != CARD_NO_ERROR)
				{
					CloseVoidDevice();
					if ( m_TRREC.CardError == CARD_ERR_CANNOT_SWIPE_CHIP)
					{
//						CLanguage::GetTextFromIndex(CLanguage::IDX_CANNOT_SWIPE_CHIP,&pbuf);
						m_strErr = L" ";
						m_strErr1 = CLanguage::GetText(CLanguage::IDX_PLEASE_INSERT);
						m_strErr2 = CLanguage::GetText(CLanguage::IDX_CHIP_CARD);
					}
					else if( m_TRREC.CardError == CARD_ERR_INVALID_MODE)
					{
						CLanguage::GetTextFromIndex(CLanguage::IDX_INVALID_CARD_NUMBER,&pbuf);
						m_strErr1 = CString(pbuf);//L"CARD NOT SUPPORT";
						m_strErr2 = CLanguage::GetText(CLanguage::IDX_NO_SUPPORTED);
					}
					else
					{
						CLanguage::GetTextFromIndex(CLanguage::IDX_CARD_NOT_SUPPORT,&pbuf);
						m_strErr1 = CString(pbuf);//L"CARD NOT SUPPORT";
						m_strErr2 = CLanguage::GetText(CLanguage::IDX_NO_SUPPORTED);
					}
					ErrorExit(ENTERCARD_INPUT);
					return 0;
				}

				if( strcmp(m_Log.m_Record.TranRec.Track2,m_TRREC.Track2) != 0 || m_Log.m_Record.TranRec.EntryMode != m_TRREC.EntryMode)
				{
					CloseVoidDevice();
					CLanguage::GetTextFromIndex(CLanguage::IDX_CARD_NOT_MATCH,&pbuf);
					m_strErr1 = CString(pbuf);
					m_strErr2 = L"";
					ErrorExit(ENTERCARD_INPUT);
					return 0;
				}

				m_iKeyStatus =  KEY_MSR;

				CloseVoidDevice();
/*
				if(m_TRREC.CardType == CARD_DEBIT)
					m_iLevel = DEBITCARDENTRY;
				else if(m_TRREC.CardType == CARD_CREDIT)
					m_iLevel = CREDITSWIPEENTRYY;
*/
				if(m_TRREC.CardType == CARD_DEBIT)
					GoToLevel(DEBITCARDENTRY);
				else
				{
					m_strErr1 = CLanguage::GetText(CLanguage::IDX_EEED_DEBIT_CARD);//L"Not Debit card!";
					ErrorExit(ENTERCARD_INPUT);
				}
	//			Close(KEY_MSR);
				return 0;
			default:
				break;	
		}
	}
	else if (wParam == 2 && message == ERROR_MESSAGE)
	{ 
		switch (lParam)
		{
		case 1: //error
			ErrorExit();
			break;
		case 2: //for jump to other level
			WorkLoop(m_iLevel);
			break;
		case 3: //print done
			m_bPrintDone = TRUE;
			CLanguage::SetCustomerFlag(FALSE);
			if(m_iLevel < ERRORENTRY)
					GoNext();
			break;
		case 4: //Can't print
			m_strErr1 = CLanguage::GetText(CLanguage::IDX_CAN_NOT_PRINT);
			ErrorExit();
			break;
		case 5: //RFID time out and restart it
			ReOpenRFID();
			break;
		case 6: //Show CDisplay window
			if(!m_bCancelFlag)
				GoToLevel(DISPLAY_WINDOW);
			break;
		case 7: //PinInput
			{
				CPinInput PinDlg;
				if(PinDlg.DoModal() == IDOK)
				{
					EMV_PIN_MODE = TRUE;
				}
				else
				{
					EMV_PIN_MODE = FALSE;
					m_bCancelFlag = TRUE;
				}

			}
			break;
		}
/*
		if (lParam == 1) //error
		{
			ErrorExit();
		}
		else if (lParam == 2) //for jump to other level
			WorkLoop(m_iLevel);
		else if (lParam == 3) // print done
		{
			m_bPrintDone = TRUE;
			GoNext();
		}
		else if (lParam == 4) //Can't print
		{
			m_strErr1 = L"Can't Print";
			ErrorExit();
		}
		else if (lParam == 5) //RFID time out and restart it
		{
			ReOpenRFID();
		}
		else if (lParam == 6) //Show CDisplay window
		{
			if(!m_bCancelFlag)
				GoToLevel(DISPLAY_WINDOW);
		}
*/	}
	return CDialog::DefWindowProc(message, wParam, lParam);
}

void CVoidDlg::DoTransaction()
{
	if (!IsNetWorkConnect())
	{
		m_strErr1 = CLanguage::GetText(CLanguage::IDX_NO_NETWORK);
		ErrorExit();
		return;
	}
	m_strMsg.SetCaption(CLanguage::GetText(CLanguage::IDX_PLEASE_WAIT));
	m_strMsg1.SetCaption(L"");
	m_strMsg2.SetCaption(L"");

	if ( m_TRREC.bPartialAuth )
	{
		m_TRREC.bPartialAuth = FALSE;
		strcpy( m_TRREC.Amount,m_TRREC.AuthAmount);
		strcpy( m_TRREC.TotalAmount,m_TRREC.Amount);
		memset( m_TRREC.AvailableBalance,0,SZ_AMOUNT+1);
	}

	CFinan::DoTransaction();
}

void CVoidDlg::ErrorExit()
{
	m_strMsg.m_nFormat = DT_LEFT;
	m_strMsg1.m_nFormat = DT_LEFT;
	m_strMsg2.m_nFormat = DT_LEFT;
	m_strMsg.SetColor(RGB(255, 0, 0), RGB(255, 255, 255));
	m_strMsg1.SetColor(RGB(255, 0, 0), RGB(255, 255, 255));
	m_strMsg2.SetColor(RGB(255, 0, 0), RGB(255, 255, 255));
//	m_strMsg.SetCaption(L"Error: ");
	m_strMsg.SetCaption(L"");
	m_strMsg1.SetCaption(m_strErr1);
	m_strMsg2.SetCaption(m_strErr2);
	m_Bt.ShowWindow(SW_HIDE);
	m_Bt1.ShowWindow(SW_HIDE);
	m_iLevel = ERRORENTRY;
	SetTimer(1, 5000, NULL);
}
//----------------------------------------------------------------------------
//!	\brief	Error display and go to level
// param level is level index
//----------------------------------------------------------------------------
void CVoidDlg::ErrorExit(int level)
{
	m_strMsg.m_nFormat = DT_LEFT;
	m_strMsg1.m_nFormat = DT_LEFT;
	m_strMsg2.m_nFormat = DT_LEFT;
	m_strMsg.SetColor(RGB(255, 0, 0),RGB( 255,255,255));
	m_strMsg1.SetColor(RGB(255, 0, 0),RGB( 255,255,255));
	m_strMsg2.SetColor(RGB(255, 0, 0),RGB( 255,255,255));
	//m_strMsg.SetCaption(L"Error: ");
	m_strMsg.SetCaption(L"");
	m_strMsg1.SetCaption(m_strErr1);
	m_strMsg2.SetCaption(m_strErr2);
	m_iLevel = ERRORENTRY1;
	SetTimer(1,5000,NULL);
	m_iNextLevel = level;
}
void CVoidDlg::OnDestroy()
{
	KillTimer(1);
	CBaseDlg::OnDestroy();
	Sleep(100);
}

void CVoidDlg::GoBack()
{
	switch(m_iLevel)
	{
	case DEBITCARDENTRY:
	case CHIPENTRYY:
//		m_iLevel = ENTERCARD_INPUT;
		return;
	case PASSWORD_INPUT:
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

void CVoidDlg::GoNext()
{
	switch(m_iLevel)
	{
	case EMV_REMOVE_CARD:
		return;
	}

	m_iLevel++;
	PostMessage( ERROR_MESSAGE, 2,2 );
}

void CVoidDlg::GoToLevel(int level)
{
	m_iLevel = level;
	PostMessage( ERROR_MESSAGE, 2,2 );
}

BOOL CVoidDlg::DoVoidConfirm()
{
	CDoConfirmDlg dlg(this);

	dlg.SetTitle( CLanguage::GetText(CLanguage::IDX_VOID) );
	int index = 1;
	BOOL flag = FALSE;

	if(m_TRREC.TranCode == TRAN_REFUND)
	{
		dlg.SetString(index++, CLanguage::GetText(CLanguage::IDX_RECEIPT_RETURN),DealAmountStr(m_TRREC.Amount));
		dlg.SetString(6,CLanguage::GetText(CLanguage::IDX_PRESS_OK_CONFIRM));
		return dlg.DoModal()==IDOK;
	}

	dlg.SetString(index++, CLanguage::GetText(CLanguage::IDX_RECEIPT_SALE),DealAmountStr(m_TRREC.Amount));

	if(strlen(m_TRREC.TipAmount)>0)
	{
		dlg.SetString(index++, CLanguage::GetText(CLanguage::IDX_RECEIPT_TIP),DealAmountStr(m_TRREC.TipAmount));
		flag = TRUE;
	}

	if(strlen(m_TRREC.CashbackAmount)>0)
	{
		dlg.SetString(index++, CLanguage::GetText(CLanguage::IDX_RECEIPT_CASHBACK),DealAmountStr(m_TRREC.CashbackAmount));
		if (strlen(m_TRREC.CashbackFee)>0)
			dlg.SetString(index++, CLanguage::GetText(CLanguage::IDX_FEE),DealAmountStr(m_TRREC.CashbackFee));
		flag = TRUE;
	}
	else if(strlen(m_TRREC.SurchargeAmount)>0)
	{
		dlg.SetString(index++, CLanguage::GetText(CLanguage::IDX_FEE),DealAmountStr(m_TRREC.SurchargeAmount));
		flag = TRUE;
	}

	if(flag)
	{
		dlg.SetString(index++,CLanguage::GetText(CLanguage::IDX_RECEIPT_TOTAL),DealAmountStr(m_TRREC.TotalAmount));
	}
	dlg.SetString(6,CLanguage::GetText(CLanguage::IDX_PRESS_OK_CONFIRM));
	return dlg.DoModal()==IDOK;
}
