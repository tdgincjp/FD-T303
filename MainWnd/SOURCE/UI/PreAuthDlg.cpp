#include "stdafx.h"
#include "PreAuthDlg.h"
#include "SaioBase.h"
#include "SaioPinpad.h"

#include "..\\Utils\\string.h"
#include "..\\Utils\\TimeUtil.h"
#include "..\\Utils\\StrUtil.h"
#include "..\\Utils\\TotalUtil.h"
#include "..\\DEFS\\constant.h"
#include "..\\UI\\display.h"
		
#include "..\\utils\\util.h"
#include "MsgDlg.h"
#include "DoConfirmDlg.h"	
#include "..\\EMV\\PinInput.h"

#define AMOUNT_INPUT			1
#define CLERK_INPUT				2
#define INVOICE_TINPUT					3
#define RECEIP_TINPUT					4
#define ENTERCARD_INPUT					5

#define MANUALENTRYY						40
#define MANUAL_IMPRINT_CARD			41
#define MANUAL_SWIPETOCUSTOMER	42
#define MANUAL_SWIPETIPSELECT		43
#define MANUAL_SWIPETOTALCOMFIRM	44
#define MANUAL_SWIPETOCLERK			45
/*
#define MANUAL_MERCHANTCOPY		46
#define MANUAL_CUSTOMERCOPY		47
#define MANUAL_WAITCOPYEND		48
#define MANUAL_END				49
*/
#define CHIPENTRYY							60
#define SELECT_LANGUAGE					61
#define SELECT_APPLICATIION			62
#define APPLICATION_CONFIRM			63
#define CHIP_DEBITTOTALCOMFIRM	64
#define CHIP_DEBITSELECTACCOUNT	65
#define CHIP_DEBITENTERPIN			66
#define CHIP_PASSTOCLERK				67
#define CHIP_DEBITMERCHANTCOPY	70
#define CHIP_DEBITCUSTOMERCOPY	71
#define CHIP_DEBITWAITCOPYEND		72
#define CHIP_DEBITEND						73
/*
//credit swipt
#define CREDITSWIPEENTRYY		80
#define CREDITMERCHANTCOPY		81
#define CREDITCUSTOMERCOPY		82
#define CREDITWAITCOPYEND		83
#define CREDITEND				84
*/
//credit swipt
#define CREDITSWIPEENTRYY				80
#define CREDITSWIPETOCUSTOMER		81
#define CREDITSWIPETIPSELECT		82
#define CREDITSWIPETOTALCOMFIRM	83
#define CREDITSWIPETOCLERK			84
#define CREDITMERCHANTCOPY			85
#define CREDITCUSTOMERCOPY			86
#define CREDITWAITCOPYEND				87
#define CREDITEND								88

#define DOTRANSACTION			100

#define CANCELENTRY							200
#define CANCELCUSTOMERCOPY					201
#define CANCELTOCLERK						202
#define EMVCARDBEREMOVED					203

//#define ERRORENTRY		        300

#pragma comment(lib, "SaioPinpad.lib")

IMPLEMENT_DYNAMIC(CPreAuthDlg , CBaseDlg)
CPreAuthDlg ::CPreAuthDlg (CWnd* pParent )
	: CBaseDlg(CBaseDlg::IDD, pParent)
{
	m_strAmount = L"$0.00";
}

CPreAuthDlg ::~CPreAuthDlg (void)
{
//TRACE(L"~CSaleDlg(void)\n");
	CLanguage::SetLanguage();

}

void CPreAuthDlg ::DoDataExchange(CDataExchange* pDX)
{
	CBaseDlg::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CPreAuthDlg , CBaseDlg)
		ON_COMMAND_RANGE(IDC_BUTTON1, IDC_BUTTON2, &CPreAuthDlg ::OnBnClickedButton)
		ON_WM_CHAR()
END_MESSAGE_MAP()

// CMainWnd message handlers
BOOL CPreAuthDlg ::OnInitDialog()
{
//	m_Title = L" PREAUTH";
	m_Title.Format(L" %s",CLanguage::GetText(CLanguage::IDX_RECEIPT_PREAUTH));
	m_TitleIndex = CLanguage::IDX_RECEIPT_PREAUTH;
	CBaseDlg::OnInitDialog();
	char* buf;

	m_TRREC.TranCode = TRAN_PREAUTH;
	m_iLevel = 1;

	m_strMsg.ShowWindow(SW_SHOW);
	m_strMsg1.ShowWindow(SW_SHOW);
	m_strMsg2.ShowWindow(SW_SHOW);

	CLanguage::GetTextFromIndex(CLanguage::IDX_CHEQUE,&buf);
	m_Bt.SetValue(1, CString(buf));
	m_Bt.SubclassDlgItem(IDC_BUTTON1, this);
	m_Bt.MoveWindow(8, screenY- 50, 120, 45);

	CLanguage::GetTextFromIndex(CLanguage::IDX_SAVING,&buf);
	m_Bt1.SetValue(3, CString(buf));
	m_Bt1.SubclassDlgItem(IDC_BUTTON2, this);
	m_Bt1.MoveWindow(screenX-128,  screenY- 50, 120, 45);

	WorkLoop(m_iLevel);
	SetTimer(2, 50, NULL);

	DWORD size;
	if (!CDataFile::Read(FID_CFG_CUSTENTRY_TMO, (BYTE*) &m_CustDialogTimer, &size))
		m_CustDialogTimer = 30;

	m_iKeyStatus = 0;

	m_MsgWnd = m_hWnd;
	CDisplay::SetDisplayWnd(m_MsgWnd);
	CLanguage::SetDisplayWnd(m_MsgWnd);

	return TRUE;  // return TRUE unless you set the focus to a control
}

void CPreAuthDlg ::OnTimer(UINT_PTR nIDEvent)
{
	if (nIDEvent == 1)
	{
		KillTimer(1);
		if (m_iLevel == ENTERCARD_INPUT)
		{
			CloseVoidDevice();
			KillTimer(1);
			m_strErr1 = CLanguage::GetText(CLanguage::IDX_TIME_OUT);
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
		return;
	}
	
	if (nIDEvent == 99)
	{
		if (m_iLevel == ENTERCARD_INPUT && !m_KeyEntry)
		{
			if (m_SCR.GetState())
			{
				KillTimer(1);
				m_SCR.Close();
				CloseDevice();
				GoToLevel(EMV_PASS_COUSTOMER);
			}
			else
			{
				if (GetScrStatus() != SCR_CARD_ABSENT)
				{
					if(m_SCR.m_bPowerOff)
					{
						m_SCR.Close();
						CloseDevice();
						m_TRREC.bFallback = TRUE;
						m_strErr = CLanguage::GetText(CLanguage::IDX_EMV_CHIP_ERROR);
						m_strErr2 = CLanguage::GetText(CLanguage::IDX_FALLBACK_TO_MAGSTRIPE);
						m_strErr1 = CLanguage::GetText(CLanguage::IDX_PLEASE_REMOVE_CARD);
						ErrorExit(ENTERCARD_INPUT);
					}
				}
			}
		}

		if (m_iLevel == EMV_REMOVE_CARD || m_iLevel == CHIP_DEBITEND || m_iLevel == EMVCARDBEREMOVED)
		{
			if (GetScrStatus()!= SCR_CARD_ABSENT && m_TRREC.EntryMode == ENTRY_MODE_CHIP)
			{
				Beep();
				return;
			}
			else
			{
				if ( m_TRREC.bFallback && m_TRREC.EntryMode > ENTRY_MODE_MANUAL )
					GoToLevel(ENTERCARD_INPUT);
				else
					CDialog::OnCancel();
			}
		}
	}

	CBaseDlg::OnTimer(nIDEvent);
}

void CPreAuthDlg ::WorkLoop(int index)
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

	CString str = L"";
	CString str1 = L"";
	CString str2 = L"";
	int i = 0;
	switch(index)
	{
	case AMOUNT_INPUT: //Amout Input
		m_bGoBackFlag = FALSE;
		ShowText(CLanguage::GetText(CLanguage::IDX_ENTER_AMOUNT),L"",m_strAmount);
		break;
	case CLERK_INPUT:  //Clerk ID input
		if (CDataFile::Read(L"CLERK ID", buf) && CString(buf) == L"Off")
		{
			m_bGoBackFlag?GoBack():GoNext();
		}
		else
		{
			m_bGoBackFlag = FALSE;
			CLanguage::GetTextFromIndex(CLanguage::IDX_ENTER_CLERK,&pbuf);
			str = CString(pbuf);
			CLanguage::GetTextFromIndex(CLanguage::IDX_ID_NAME,&pbuf);
			str1=  CString(pbuf);
			str2= L"";
			ShowText(str, str1, str2);
		}
		break;
	case INVOICE_TINPUT:  //Invoice input
		if (CDataFile::Read(L"INVOICE", buf) && CString(buf) == L"Off")
		{
			m_bGoBackFlag?GoBack():GoNext();
		}
		else
		{
			m_bGoBackFlag = FALSE;
			CLanguage::GetTextFromIndex(CLanguage::IDX_INVOICE_MENU,&pbuf);
			str = CString(pbuf);
			CLanguage::GetTextFromIndex(CLanguage::IDX_NUMBER_NAME,&pbuf);
			str1=  CString(pbuf);
			str2= L"";
			ShowText(str, str1, str2);
		}
		break;
	case RECEIP_TINPUT:  //Receipt input
		if (CDataFile::Read(L"RECEIPT", buf) && CString(buf) == L"Off")
		{
			m_bGoBackFlag?GoBack():GoNext();
		}
		else
		{
			m_bGoBackFlag = FALSE;
			CLanguage::GetTextFromIndex(CLanguage::IDX_ENTER_RECEIPT,&pbuf);
			str = CString(pbuf);
			CLanguage::GetTextFromIndex(CLanguage::IDX_NUMBER_NAME,&pbuf);
			str1=  CString(pbuf);
			str2= L"";
			ShowText(str, str1, str2);
		}
		break;
	case ENTERCARD_INPUT:  //Card enter and manual Entry
		m_bGoBackFlag = FALSE;
		m_bCancelPrint = FALSE;
		m_TRREC.bEmvTransaction = FALSE;
		m_TRREC.EntryMode = 0;

/*		ENABLE FALL BACK - ALEX MCKAY https://tdginc.fogbugz.com/f/cases/3232/
		m_TRREC.bFallback = FALSE;*/
		m_iNextLevel = 0;
		ShowPicture();
		if(!OpenVoidDevice())
		{
			m_strErr1 = CLanguage::GetText(CLanguage::IDX_FAIL_OPEN_DEVICE);
			PostMessage( ERROR_MESSAGE, 2, 1);
			break;
		}
		m_strMsg.m_nFormat = DT_RIGHT;
		m_strMsg2.m_nFormat = DT_LEFT;

		CLanguage::GetTextFromIndex(CLanguage::IDX_INSERT_SWIPE,&pbuf);
		str = CString(pbuf);

		if(CDataFile::Read(L"MANUAL ENTRY",buf) && CString(buf) == L"Off")
			str2 = L"";
		else
			str2= CLanguage::GetText(CLanguage::IDX_KEY_CARD_NUMBER);
		ShowText(m_strAmount,str ,str2 );

//		ShowText(m_strAmount,CLanguage::GetText(CLanguage::IDX_INSERT_SWIPE),CLanguage::GetText(CLanguage::IDX_KEY_CARD_NUMBER));
		SetTimer(1,30000,NULL);
		break;


//====================================================================
	case MANUALENTRYY: //Manual Entry enter expiry
		CLanguage::GetTextFromIndex(CLanguage::IDX_ENTER_EXPIRY,&pbuf);
		str = CString(pbuf);
		CLanguage::GetTextFromIndex(CLanguage::IDX_DATE,&pbuf);
		str1.Format(L"%s           MMYY", CString(pbuf));
		str2= L"";
		ShowText(str, str1,str2);

		//		m_strMsg.SetCaption(L"Enter Expiry");
		//		m_strMsg1.SetCaption(L"Date           MMYY");
		//		m_strMsg2.SetCaption(L"");
		break;
	case MANUAL_IMPRINT_CARD:  //imprint card
		CLanguage::GetTextFromIndex(CLanguage::IDX_IMPRINT_CARD,&pbuf);
		str = CString(pbuf);
		CLanguage::GetTextFromIndex(CLanguage::IDX_PRESS_OK,&pbuf);
		str1 = CString(pbuf);
		str2= L"";
		ShowText(str, str1,str2);
//		m_strMsg.SetCaption(L"Imprint Card");
//		m_strMsg1.SetCaption(L"Press OK");
//		m_strMsg2.SetCaption(L"");
		break;

// == == == == == == == == == == == == == = EMV Chip Card Debit Sale ¡§C chip page 122 == == == == == == == == == == == == == = 
	case CHIPENTRYY:
		m_bCancelPrint = TRUE;
		m_bGoBackFlag = FALSE;
		CLanguage::GetTextFromIndex(CLanguage::IDX_PASS_TO,&pbuf);
		str = CString(pbuf);
		CLanguage::GetTextFromIndex(CLanguage::IDX_CUSTOMER,&pbuf);
		str1=  CString(pbuf);
//		str = L"Pass to";
//		str1= L"Customer";
		str2 = L"";
		ShowText(str, str1,str2);
		SetTimer(1, 3000, NULL);
		m_bCancelInCustomer = TRUE;
		CLanguage::SetCustomerFlag(TRUE);

//		StartEMV();
		break;
	case SELECT_LANGUAGE:
		if (SupportEnglishOrFrench())
		{
			// get first language found will be selected
			// EMVSelectLanguage(???);
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
		DoConfirm();
/*		if (DoConfirm(m_bReturnFlag))
			GoNext();
		else
			GoToLevel(CANCELENTRY);
*/		break;
	case CHIP_DEBITSELECTACCOUNT: // SelectAccount
		return;
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
//		str = L"Pass to clerk";
		//			str1= L"Do not remove";
		//			str2 = L"Card!";
		CLanguage::GetTextFromIndex(CLanguage::IDX_DO_NOT_REMOVE,&pbuf);
		str1 = CString(pbuf);
		CLanguage::GetTextFromIndex(CLanguage::IDX_CARD,&pbuf);
		str2=  CString(pbuf);
		ShowText(str, str1, str2);
		SetTimer(1, 3000, NULL);
		CLanguage::SetCustomerFlag(FALSE);
		break;

	case CHIP_DEBITMERCHANTCOPY: // for merchant copy of the receipt
		m_strMsg2.m_nFormat = DT_LEFT;
		if (m_TRREC.ComStatus == ST_OK  &&
				m_TRREC.TranStatus == ST_APPROVED)
		{
			CLanguage::GetTextFromIndex(m_TRREC.bPartialAuth?CLanguage::IDX_PARTIAL_APPROVED:CLanguage::IDX_APPROVED,&pbuf);
			str = CString(pbuf);//L"APPROVED";
			CLanguage::GetTextFromIndex(CLanguage::IDX_AUTH_CODE,&pbuf);
			str1.Format(L"%s : %s", CString(pbuf),CString(m_TRREC.AuthCode));
			m_strMsg2.m_nFormat = DT_LEFT;
			CLanguage::GetTextFromIndex(CLanguage::IDX_INVO_NUMBER,&pbuf);
			str2.Format(L"%s: %s", CString(pbuf),CString(m_TRREC.InvoiceNo));
			ShowText( str, str1 ,str2);
			ShowPicture(1);
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
			ShowPicture(2);
		}
		Print();
		break;
	case CHIP_DEBITCUSTOMERCOPY: // for Customer copy of the receipt
		Sleep(2000);
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
	case CHIP_DEBITWAITCOPYEND: // waiting for Customer copy of the receipt ending...
		if (m_TRREC.ComStatus == ST_OK  &&
				m_TRREC.TranStatus == ST_APPROVED)
		{
			CLanguage::GetTextFromIndex(m_TRREC.bPartialAuth?CLanguage::IDX_PARTIAL_APPROVED:CLanguage::IDX_APPROVED,&pbuf);
			str = CString(pbuf);//L"APPROVED";
			CLanguage::GetTextFromIndex(CLanguage::IDX_AUTH_CODE,&pbuf);
			str1.Format(L"%s : %s", CString(pbuf),CString(m_TRREC.AuthCode));
			m_strMsg2.m_nFormat = DT_LEFT;
			CLanguage::GetTextFromIndex(CLanguage::IDX_INVO_NUMBER,&pbuf);
			str2.Format(L"%s: %s", CString(pbuf),CString(m_TRREC.InvoiceNo));
			ShowText( str, str1 ,str2);
			ShowPicture(1);
		}
		else
		{
			CLanguage::GetTextFromIndex(CLanguage::IDX_UNSUCCESSFUL,&pbuf);
			str = CString(pbuf);//L"Unsuccessful";
			CLanguage::GetTextFromIndex(CLanguage::IDX_DECLINED,&pbuf);
			str1.Format(L"%s %s", CString(pbuf),CString(m_TRREC.RespCode));
			str2 = L"";
			ShowText(str, str1,str2);
			ShowPicture(2);
		}
		Print(2);
		break;
	case CHIP_DEBITEND: // ending...
		Sleep(2000);
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
			if ( m_TRREC.bFallback && m_TRREC.EntryMode > ENTRY_MODE_MANUAL)
			{
				Sleep(2000);
				GoToLevel(ENTERCARD_INPUT);
			}
			else
				CDialog::OnCancel();
		}
		break;
// == == == == == == == == == == == == == = Credit Card Sale ¡§C Swiped page 75 == == == == == == == == == == == == == == == == == = 
	case CREDITSWIPEENTRYY://FRAUD CHECK
		m_bCancelPrint = TRUE;
		m_bGoBackFlag = FALSE;
	    if (CDataFile::Read(L"FRAUD CHECK", buf) && CString(buf) == L"Off")
		{
			GoToLevel(DOTRANSACTION);
//			m_bGoBackFlag?GoBack():GoNext();
		}
		else
		{
			CLanguage::GetTextFromIndex(CLanguage::IDX_ENTER_LAST,&pbuf);
			str = CString(pbuf);
			CLanguage::GetTextFromIndex(CLanguage::IDX_DIGITS,&pbuf);

//			if(!m_bFraud)
//				str1.Format(L"4 %s again", CString(pbuf));
//			else
				str1.Format(L"4 %s", CString(pbuf));
//			str = L"Enter Last";
//			str1= L"4 Digits";
			str2 = L"";
			ShowText(str, str1, str2);
		}
		break;

	case CREDITMERCHANTCOPY: // for merchant copy of the receipt
//	case MANUAL_MERCHANTCOPY: // for merchant copy of the receipt
		m_strMsg2.m_nFormat = DT_LEFT;
		if (m_TRREC.ComStatus == ST_OK  &&
				m_TRREC.TranStatus == ST_APPROVED)
		{
			CLanguage::GetTextFromIndex(m_TRREC.bPartialAuth?CLanguage::IDX_PARTIAL_APPROVED:CLanguage::IDX_APPROVED,&pbuf);
			str = CString(pbuf);//L"APPROVED";
			CLanguage::GetTextFromIndex(CLanguage::IDX_AUTH_CODE,&pbuf);
			str1.Format(L"%s : %s", CString(pbuf),CString(m_TRREC.AuthCode));
			m_strMsg2.m_nFormat = DT_LEFT;
			CLanguage::GetTextFromIndex(CLanguage::IDX_INVO_NUMBER,&pbuf);
			str2.Format(L"%s: %s", CString(pbuf),CString(m_TRREC.InvoiceNo));
			ShowText( str, str1 ,str2);
			ShowPicture(1);
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
			ShowPicture(2);
		}
		CLanguage::SetCustomerFlag(FALSE);
		Print();
		break;
	case CREDITCUSTOMERCOPY: // for Customer copy of the receipt
//	case MANUAL_CUSTOMERCOPY: // for Customer copy of the receipt
		Sleep(2000);
		ShowText(CLanguage::GetText(CLanguage::IDX_PRESS_OK_FOR),CLanguage::GetText(CLanguage::IDX_CUSTOMER_COPY),L"");
		SetTimer(1,30000,NULL);
		break;
	case CREDITWAITCOPYEND: // waiting for Customer copy of the receipt ending...
//	case MANUAL_WAITCOPYEND: // waiting for Customer copy of the receipt ending...
		if (m_TRREC.ComStatus == ST_OK  &&
				m_TRREC.TranStatus == ST_APPROVED)
		{
			CLanguage::GetTextFromIndex(m_TRREC.bPartialAuth?CLanguage::IDX_PARTIAL_APPROVED:CLanguage::IDX_APPROVED,&pbuf);
			str = CString(pbuf);//L"APPROVED";
			CLanguage::GetTextFromIndex(CLanguage::IDX_AUTH_CODE,&pbuf);
			str1.Format(L"%s : %s", CString(pbuf),CString(m_TRREC.AuthCode));
			m_strMsg2.m_nFormat = DT_LEFT;
			CLanguage::GetTextFromIndex(CLanguage::IDX_INVO_NUMBER,&pbuf);
			str2.Format(L"%s: %s", CString(pbuf),CString(m_TRREC.InvoiceNo));
			ShowText( str, str1 ,str2);
			ShowPicture(1);
		}
		else
		{
			CLanguage::GetTextFromIndex(CLanguage::IDX_UNSUCCESSFUL,&pbuf);
			str = CString(pbuf);//L"Unsuccessful";
			CLanguage::GetTextFromIndex(CLanguage::IDX_DECLINED,&pbuf);
			str1.Format(L"%s %s", CString(pbuf),CString(m_TRREC.RespCode));
			str2 = L"";
			ShowText(str, str1,str2);
			ShowPicture(2);
		}
		Print(2);
		break;
	case CREDITEND: // ending...
//	case MANUAL_END: // ending...
		Sleep(2000);
		CDialog::OnCancel();
		break;

//============================Do transaction ( connect to host)===============
	case DOTRANSACTION:
		DoTransaction();
		break;

	case CANCELENTRY:
		m_bCancelFlag = TRUE;
		ShowText(CLanguage::GetText(CLanguage::IDX_TRANSACTION),CLanguage::GetText(CLanguage::IDX_CANCELLED),L"");
		SetTimer(1,5000,NULL);
		if(m_TRREC.CardError == CARD_NO_ERROR
			&& m_bCancelPrint)
			Print();
		break;
	case CANCELCUSTOMERCOPY: // for Customer copy of the receipt
		if ( !m_bCancelPrint )
		{
			GoNext();
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
	case CANCELTOCLERK:
		if(m_TRREC.bEmvTransaction)
		{
			Sleep(10);
			GoToLevel(EMV_REMOVE_CARD);
			break;
		}
		if (m_bCancelInCustomer)
		{
			ShowText(CLanguage::GetText(CLanguage::IDX_PASS_TO),L"Clerk",L"");
			SetTimer(1,3000,NULL);
		}
		else
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
/*	
	case EMV_CONFIRM_AMOUNT: //EMV confirm amount
		m_strMsg2.m_nFormat = DT_LEFT;
		str = L"Total :";
		str1.Format(L"     $ %s  OK?", DealAmountStr(m_TRREC.TotalAmount));
		str2 = L"Press OK to confirm";
		ShowText(str, str1, str2);
		break;
*/
	case EMV_SELECT_ACCOUNT: //EMV select account
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
			CDialog::OnCancel();
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
void CPreAuthDlg::SetDisplayButton()
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

void CPreAuthDlg::OnCancel()
{
	Beep1();
	switch(m_iLevel)
	{
	case AMOUNT_INPUT: //Amout Input
	case CLERK_INPUT:  //Clerk ID input
	case INVOICE_TINPUT:  //Invoice input
	case RECEIP_TINPUT:  //Receipt input
		GoToLevel(CANCELENTRY);
		break;
	case ENTERCARD_INPUT:  //Card enter and manual Entry
		KillTimer(1);
		GoToLevel(CANCELENTRY);
//		m_RFID.Reset();
		CloseDevice();
		break;
		
// == == == == == == == == == == == == == = EMV Chip Card Debit Sale ¡§C chip page 122 == == == == == == == == == == == == == = 
	case CHIPENTRYY:
		break;
	case SELECT_LANGUAGE: //select language
//	case SELECT_APPLICATIION: //select application
//	case APPLICATION_CONFIRM: //select comfirm
		GoToLevel(CANCELENTRY);
		break;
/*	case CHIP_DEBITCASHBACK: //CASHBACK
		GoToLevel(CANCELENTRY);
		break;
	case CHIP_DEBITSURCHARGEFEE: //surcharge fee
		GoToLevel(CANCELENTRY);
		break;
	case CHIP_DEBITTIPSELECT: //TipSelect
		break;
	case CHIP_DEBITTOTALCOMFIRM: // TotalComfirm
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
// == == == == == == == == == == == == == = Credit Card Sale ¡§C Swiped page 75 == == == == == == == == == == == == == == == == == = 
	case CREDITSWIPEENTRYY: //Fraud Check
		GoToLevel(CANCELENTRY);
		break;
	case CREDITSWIPETOCUSTOMER:
		GoToLevel(CANCELENTRY);
		break;
	case CREDITSWIPETIPSELECT: //TipSelect
		break;
	case CREDITSWIPETOTALCOMFIRM: // TotalComfirm
		GoToLevel(CANCELENTRY);
		break;
	case CREDITSWIPETOCLERK:   //Pass to clerk
		GoToLevel(CANCELENTRY);
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

//=======================/Manual Entry=============================================
	case MANUALENTRYY: //¡°Fraud Check
		GoToLevel(CANCELENTRY);
		break;
	case MANUAL_IMPRINT_CARD:
	case MANUAL_SWIPETOCUSTOMER:
		GoToLevel(CANCELENTRY);
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

void CPreAuthDlg ::OnOK()
{
	Beep1();
	CString str,str1,str2;
	char* pbuf;
	switch(m_iLevel)
	{
	case AMOUNT_INPUT:
		str = m_strMsg2.m_szCaption;
		str.Remove('$');
		str.Remove(' ');
		str.Remove('.');
		str.Remove(',');
		str.TrimLeft('0');
		if (str.GetLength() == 0)
		{
			Beep();
			return;
		}
		PutParam(m_TRREC.Amount, str);
		m_strAmount = m_strMsg2.m_szCaption;
		break;
	case CLERK_INPUT:
		str = m_strMsg2.m_szCaption;
		PutParam(m_TRREC.ClerkID, str);
		break;
	case INVOICE_TINPUT:  //Invoice input
		str = m_strMsg2.m_szCaption;
		if (str.GetLength() == 0)
		{
			Beep();
			return;
		}

		if( CheckInvoice(str))
		{
			GoToLevel(RECEIP_TINPUT);
		}
		else
		{
			CMsgDlg dlg;
			dlg.m_iTimeOut = 30;
			dlg.m_str2 = CLanguage::GetText(CLanguage::IDX_DUP_INVOICE);
			dlg.m_strBt1 = CLanguage::GetText(CLanguage::IDX_RE_ENTER);
			dlg.m_strBt2 = CLanguage::GetText(CLanguage::IDX_CANCELL);
			
			if( dlg.DoModal() == IDOK)
				GoToLevel(INVOICE_TINPUT);
			else
				GoToLevel(CANCELENTRY);
		}
		return;
	case RECEIP_TINPUT:
		str = m_strMsg2.m_szCaption;
		PutParam(m_TRREC.ReceiptNo, str);
		break;
	case ENTERCARD_INPUT:
		if (!m_KeyEntry)
		{
			Beep();
			return;
		}
		KillTimer(1);
		str = m_strMsg2.m_szCaption;
		if (str.GetLength() < 10 || str.GetLength() > 19)
		{
			Beep();
			return;
		}
		PutParam(m_TRREC.Account, str);
		m_TRREC.CardError = ProcessManualCard(m_TRREC.Account);
		if (m_TRREC.CardError != CARD_NO_ERROR)
		{
			CloseDevice();

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
			return;
		}
		m_iLevel = MANUALENTRYY-1;
		break;

// == == == == == == == == == == == == == = EMV Chip Card Debit Sale ¡§C chip page 122 == == == == == == == == == == == == == = 
	case CHIPENTRYY:
		KillTimer(1);
		m_iLevel = DISPLAY_WINDOW;
		return;
	case SELECT_LANGUAGE: //select language
//	case SELECT_APPLICATIION: //select application
//	case APPLICATION_CONFIRM: //select comfirm
		Beep();
		return;
/*	case CHIP_DEBITCASHBACK: //CASHBACK
		m_strCashback = m_strMsg2.m_szCaption;
		break;
	case CHIP_DEBITSURCHARGEFEE: //surcharge fee
		break;
	case CHIP_DEBITTIPSELECT: //TipSelect
		break;
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
// == == == == == == == == == == == == == = Credit Card Sale ¡§C Swiped page 75 == == == == == == == == == == == == == == == == == = 
	case CREDITSWIPEENTRYY: //Fraud Check
		str = m_strMsg2.m_szCaption;
		if( CheckFraud(str))
		{
			GoToLevel(DOTRANSACTION);
		}
		else
		{
			CMsgDlg dlg;
			dlg.m_iTimeOut = 30;
			dlg.m_str2 = CLanguage::GetText(CLanguage::IDX_MISMATCHED);
			dlg.m_strBt1 = CLanguage::GetText(CLanguage::IDX_RE_ENTER);
			dlg.m_strBt2 = CLanguage::GetText(CLanguage::IDX_CANCELL);
			
			if( dlg.DoModal() == IDOK)
				GoToLevel(CREDITSWIPEENTRYY);
			else
				GoToLevel(CANCELENTRY);
		}

		return;
//		PutParam(m_TRREC.FraudNo, str);  //Fix me later
//		break;
	case CREDITSWIPETOCUSTOMER:
		KillTimer(1);
		break;
	case CREDITSWIPETIPSELECT: //TipSelect
		break;
	case CREDITSWIPETOTALCOMFIRM: // TotalComfirm
		break;
	case CREDITSWIPETOCLERK:   //Pass to clerk
		KillTimer(1);
		GoToLevel(DOTRANSACTION);
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
// == == == == == == == == == == == = /Manual Entry == == == == == == == == == == == == == == == == == == == == == == = 
	case MANUALENTRYY: //Fraud Check
		str = m_strMsg2.m_szCaption;
		if(str.GetLength() != SZ_EXPIRY_DATE)
		{
			Beep();
			return;
		}

		str = m_strMsg2.m_szCaption;
		if( CheckMMYY(str))
		{
			str = m_strMsg2.m_szCaption.Mid(2,2) + m_strMsg2.m_szCaption.Mid(0,2) ;
			PutParam(m_TRREC.ExpDate,str);
			break;
		}
		else
		{
			CMsgDlg dlg;
			dlg.m_iTimeOut = 30;
			dlg.m_str2 = CLanguage::GetText(CLanguage::IDX_INVALID_EXPIRY_DATE);
			dlg.m_strBt1 = CLanguage::GetText(CLanguage::IDX_RE_ENTER);
			dlg.m_strBt2 = CLanguage::GetText(CLanguage::IDX_CANCELL);
			
			if( dlg.DoModal() == IDOK)
				GoToLevel(MANUALENTRYY);
			else
				GoToLevel(CANCELENTRY);
		}
		return;
	case MANUAL_IMPRINT_CARD:
		GoToLevel(DOTRANSACTION);
		return;

//===================================================================

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
		if ( m_TRREC.bEmvTransaction)
		{
			GoToLevel(EMV_REMOVE_CARD);
		}
		else
			CDialog::OnCancel();
		return;
	case ERRORENTRY1:
		KillTimer(1);
		GoToLevel(m_iNextLevel);
		return;

// == == == == == == == Display Window == == == == == 
	case DISPLAY_WINDOW:
		if (CDisplay::GetMask() & MASK_OK)
			CDisplay::SetKeyReturn(KEY_ENTER);
		return;
	case EMV_SELECT_LANGUAGE: //EMV Select language
//	case EMV_CONFIRM_AMOUNT: //EMV confirm amount
	case EMV_SELECT_ACCOUNT: //EMV select account
	case CHIP_DEBITTOTALCOMFIRM: // TotalComfirm
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
//		SetKeyReturn(KEY_ENTER);
		break;
	case CHIP_DEBITSELECTACCOUNT: // SelectAccount
//		Beep();
		return;
	default:
//		CDialog::OnOK();
		return;
	}
	GoNext();
	SetFocus();
}

void CPreAuthDlg ::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	Beep1();
	switch(m_iLevel)
	{
	case AMOUNT_INPUT: //Amout Input
		m_strMsg2.SetCaption(DealAmount(nChar,m_strMsg2.m_szCaption));
		break;
	case CLERK_INPUT:  //Clerk ID input
		m_strMsg2.SetCaption(DealClerkID(nChar, m_strMsg2.m_szCaption));
		break;
	case INVOICE_TINPUT:  //Invoice input
		m_strMsg2.SetCaption(DealInvoice(nChar, m_strMsg2.m_szCaption));
		break;
	case RECEIP_TINPUT:  //Receipt input
		m_strMsg2.SetCaption(DealReceipt(nChar, m_strMsg2.m_szCaption));
		break;
	case ENTERCARD_INPUT:  //Card enter and manual Entry
		KillTimer(1);
		m_strMsg2.SetCaption(KeyAccount(nChar, m_strMsg2.m_szCaption));
		if (m_KeyEntry)
		{
			m_strMsg1.SetCaption(CLanguage::GetText(CLanguage::IDX_KEY_CARD_NUMBER1));
			m_strMsg2.m_nFormat = DT_RIGHT;
		}
		SetTimer(1, 30000, NULL);
		break;
	case CHIP_DEBITSELECTACCOUNT: // SelectAccount
		if (nChar == '1')
			OnBnClickedButton(IDC_BUTTON1);
		if (nChar == '3')
			OnBnClickedButton(IDC_BUTTON2);
		if (nChar == 8)
			GoBack();
		break;
	case CHIP_DEBITMERCHANTCOPY: // for merchant copy of the receipt
		return;
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
	case CREDITSWIPEENTRYY:
		m_strMsg2.SetCaption(DealFraud(nChar, m_strMsg2.m_szCaption));
		break;
	case CREDITSWIPETOCUSTOMER: // for Customer copy of the receipt
		if (nChar == 8)
		{
			KillTimer(1);
			GoBack();
		}
		break;
	case CREDITSWIPETIPSELECT: //TipSelect
		if (nChar == 8)
			GoBack();
		break;
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
//===========================Credit Card Sale ¨C Manual Entry ¨C Swiped page 79===================================
	case MANUALENTRYY:
		m_strMsg2.SetCaption(KeyExpiry(nChar, m_strMsg2.m_szCaption));
		break;
	case MANUAL_IMPRINT_CARD: // for Customer copy of the receipt
		if(nChar == 8)
		{
			KillTimer(1);
			GoBack();
		}
		break;
	case MANUAL_SWIPETIPSELECT: //TipSelect
		if (nChar == 8)
			GoBack();
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

void CPreAuthDlg ::OnBnClickedButton(UINT nID)
{
	CString str,str1,str2;
	char* pbuf;
	switch(m_iLevel)
	{
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

void CPreAuthDlg ::Done()
{
	AfterHost();

	if(m_TRREC.EntryMode == ENTRY_MODE_CHIP)
		GoToLevel(CHIP_DEBITMERCHANTCOPY);
	else
		GoToLevel(CREDITMERCHANTCOPY);
}

LRESULT CPreAuthDlg ::DefWindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	CString strTemp; 
	DWORD len = 0;
   DWORD decode = 0;
   char* pbuf;

	if(wParam == 0 && m_bCloseDevice)
	{
		switch(message)
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
	//					CLanguage::GetTextFromIndex(CLanguage::IDX_CANNOT_SWIPE_CHIP,&pbuf);
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

				m_iKeyStatus =  KEY_MSR;
				m_TRREC.EntryMode = ENTRY_MODE_SWIPED;

				CloseVoidDevice();

				if(m_TRREC.CardType == CARD_DEBIT)
				{
					m_strErr1 = CLanguage::GetText(CLanguage::IDX_DEBIT_CARD);
					m_strErr2 = CLanguage::GetText(CLanguage::IDX_NO_SUPPORTED);
					ErrorExit(ENTERCARD_INPUT);
					return 0;
				}
				if(m_TRREC.CardType == CARD_CREDIT)
				{
					m_iLevel = CREDITSWIPEENTRYY;
					WorkLoop(m_iLevel);
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
			m_strErr = CLanguage::GetText(CLanguage::IDX_CAN_NOT_PRINT);
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

void CPreAuthDlg ::DoTransaction()
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
	CFinan::DoTransaction();
}

//----------------------------------------------------------------------------
//!	\brief	Error display and go to level
// param level is level index
//----------------------------------------------------------------------------
void CPreAuthDlg::ErrorExit()
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
void CPreAuthDlg::ErrorExit(int level)
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
//	GoToLevel(level);
}

void CPreAuthDlg ::OnDestroy()
{
	KillTimer(1);
	CBaseDlg::OnDestroy();
	Sleep(100);
}

void CPreAuthDlg ::GoBack()
{
	switch(m_iLevel)
	{
	case MANUALENTRYY:
	case CHIPENTRYY:
	case CREDITSWIPEENTRYY:
		return;
	case AMOUNT_INPUT:
			CDialog::OnCancel();
			return;
	default:
		m_bGoBackFlag = TRUE;
		m_iLevel--;
		break;
	}
	KillTimer(1);
	PostMessage(ERROR_MESSAGE, 2, 2);
}

//----------------------------------------------------------------------------
//!	\brief	go to next GUI
//----------------------------------------------------------------------------
void CPreAuthDlg::GoNext()
{
	switch(m_iLevel)
	{
	case EMV_REMOVE_CARD:
		return;
	}

	m_iLevel++;
	PostMessage(ERROR_MESSAGE, 2, 2);
}

void CPreAuthDlg ::GoToLevel(int level)
{
	m_iLevel = level;
	PostMessage(ERROR_MESSAGE, 2, 2);
}

BOOL CPreAuthDlg::DoConfirm()
{
	CDoConfirmDlg dlg(this);

	dlg.SetTitle(CLanguage::GetText(CLanguage::IDX_RECEIPT_PREAUTH)); 

//	dlg.m_bReturnFlag = Flag;
	int index = 1;
	PutAmount(m_TRREC.Amount,m_strAmount);

	dlg.SetString(index++, CLanguage::GetText(CLanguage::IDX_RECEIPT_SALE),DealAmountStr(m_TRREC.Amount));

	dlg.SetString(6,CLanguage::GetText(CLanguage::IDX_PRESS_OK_CONFIRM));
//	return dlg.DoModal()==IDOK;
	int iReturn = dlg.DoModal();
	if (iReturn == IDOK)
	{
		SetKeyReturn(KEY_ENTER);
//		GoNext();
	}
	else if(iReturn == IDCANCEL)
	{
		if(m_TRREC.bEmvTransaction)
			SetKeyReturn(KEY_CANCEL);
		else
			GoToLevel(CANCELENTRY);
	}
	return TRUE;
}
