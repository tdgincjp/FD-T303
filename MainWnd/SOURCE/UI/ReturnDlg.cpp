#include "stdafx.h"
#include "ReturnDlg.h"
#include "SaioBase.h"
#include "SaioPinpad.h"

#include "..\\Utils\\string.h"
#include "..\\Utils\\TimeUtil.h"
#include "..\\Utils\\StrUtil.h"
#include "..\\Utils\\TotalUtil.h"
#include "..\\DEFS\\constant.h"
//#include "..\\TRANSACTIONS\\TransactionLog.h"
	
#include "..\\UI\\display.h"
		
#include "..\\utils\\util.h"

#define PASSWORD_INPUT					0
#define AMOUNT_INPUT						1
#define CLERK_INPUT							2
#define RECEIP_TINPUT						3
#define ENTERCARD_INPUT					4
//swiped debit
#define DEBITCARDENTRY					20
#define DEBITCASHBACK						21
#define DEBITSURCHARGEFEE				22
#define DEBITTIPSELECT					23
#define DEBITTOTALCOMFIRM				24
#define DEBITSELECTACCOUNT			25
#define DEBITONLINEPIN					26
#define DEBITMERCHANTCOPY				27
#define DEBITCUSTOMERCOPY				28
#define DEBITWAITCOPYEND				29
#define DEBITEND								30

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
#define CHIP_DEBITCASHBACK			64
#define CHIP_DEBITSURCHARGEFEE	65
#define CHIP_DEBITTIPSELECT			66
#define CHIP_DEBITTOTALCOMFIRM	67
#define CHIP_DEBITSELECTACCOUNT	68
#define CHIP_DEBITENTERPIN			69
#define CHIP_PASSTOCLERK				70
#define CHIP_DEBITMERCHANTCOPY	71
#define CHIP_DEBITCUSTOMERCOPY	72
#define CHIP_DEBITWAITCOPYEND		73
#define CHIP_DEBITEND						74

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

#define DOTRANSACTION						180

#define CANCELENTRY							200
#define CANCELTOCLERK						201


#pragma comment(lib, "SaioPinpad.lib")

IMPLEMENT_DYNAMIC(CReturnDlg, CBaseDlg)
CReturnDlg::CReturnDlg(CWnd* pParent)
	: CBaseDlg(CBaseDlg::IDD, pParent)
{
}

CReturnDlg::~CReturnDlg(void)
{
}

void CReturnDlg::DoDataExchange(CDataExchange* pDX)
{
	CBaseDlg::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CReturnDlg, CBaseDlg)
		ON_COMMAND_RANGE(IDC_BUTTON1, IDC_BUTTON2, &CReturnDlg::OnBnClickedButton)
		ON_WM_CHAR()
END_MESSAGE_MAP()

// CMainWnd message handlers
BOOL CReturnDlg::OnInitDialog()
{
	m_bReturnFlag = TRUE;
	m_Title = L"  RETURN";

	CBaseDlg::OnInitDialog();

	m_TRREC.TranCode = TRAN_REFUND;
	m_iLevel = PASSWORD_INPUT;

	m_strMsg.ShowWindow(SW_SHOW);
	m_strMsg1.ShowWindow(SW_SHOW);
	m_strMsg2.ShowWindow(SW_SHOW);

	m_Bt.SetValue(1, L"  CHEQUE");
	m_Bt.SubclassDlgItem(IDC_BUTTON1, this);
	m_Bt.MoveWindow(8, screenY- 50, 120, 45);

	m_Bt1.SetValue(3, L"  SAVING");
	m_Bt1.SubclassDlgItem(IDC_BUTTON2, this);
	m_Bt1.MoveWindow(screenX-128,  screenY- 50, 120, 45);

	WorkLoop(m_iLevel);
	SetTimer(2, 50, NULL);

	m_CustDialogTimer = 30;

	m_iKeyStatus = 0;

	m_MsgWnd = m_hWnd;

	return TRUE;  // return TRUE unless you set the focus to a control
}

//----------------------------------------------------------------------------
//!	\brief	timer function
//  nIDEvent 1 is for time out 
//  nIDEvent 2 is for focus
//  nIDEvent 99 is for check network connection's status (it is 1 sec now)
//----------------------------------------------------------------------------
void CReturnDlg::OnTimer(UINT_PTR nIDEvent)
{
	if (nIDEvent == 1)
	{
		if (m_iLevel == ENTERCARD_INPUT)
		{
			KillTimer(1);
			m_strErr1 = L"Time Out";
			ErrorExit();
			return;
		}
		OnOK();
		return;
	}

	if (nIDEvent == 2)
	{
		SetFocus();
		KillTimer(2);
		return;
	}
/*	
	if (nIDEvent == 99)
	{
		if (m_iLevel == ENTERCARD_INPUT)
		{
			if (m_SCR.GetState())
			{
				KillTimer(1);
				StartEMV();
				m_iLevel = DISPLAY_WINDOW;
			}
		}

		if (m_iLevel == EMV_REMOVE_CARD || m_iLevel == CHIP_DEBITEND)
		{
			if (GetScrStatus() == SCR_POWER_ON)
			{
				Beep();
			}
			else
			{
				CDialog::OnCancel();
			}
		}

	}
*/	CBaseDlg::OnTimer(nIDEvent);
}

//----------------------------------------------------------------------------
//!	\brief	this is main work loop function for any GUI's entry, 
// param  index is for GUI's index id
//----------------------------------------------------------------------------
void CReturnDlg::WorkLoop(int index)
{
	TCHAR buf[10] = { L"" };
	m_strMsg.m_nFormat = DT_LEFT;
	m_strMsg2.m_nFormat = DT_RIGHT;
	m_Bt.ShowWindow(SW_HIDE);
	m_Bt1.ShowWindow(SW_HIDE);
	m_strMsg.SetColor(RGB(0, 0, 0),RGB( 255,255,255));
	m_strMsg1.SetColor(RGB(0, 0, 0),RGB( 255,255,255));

	CString str = L"";
	CString str1 = L"";
	int i = 0;
	switch(index)
	{
	case PASSWORD_INPUT: //Password Input
		m_bGoBackFlag = FALSE;
		for(i = 0;i<m_strPassword.GetLength();i++)
			str += L"*";

		ShowText(L"Enter Password", L"", str);
		break;
	case AMOUNT_INPUT: //Amout Input
		m_bGoBackFlag = FALSE;
		ShowText(L"Enter Amount", L"", m_strAmount);
		break;
	case CLERK_INPUT:  //Clerk ID input
		if (CDataFile::Read(L"CLERK ID", buf) && CString(buf) == L"Off")
		{
			m_bGoBackFlag?GoBack():GoNext();
		}
		else
		{
			m_bGoBackFlag = FALSE;
			ShowText(L"Enter Clerk", L"Id", L"");
			break;
		}
	case RECEIP_TINPUT:  //Receipt input
		if (CDataFile::Read(L"RECEIPT", buf) && CString(buf) == L"Off")
		{
			m_bGoBackFlag?GoBack():GoNext();
		}
		else
		{
			m_bGoBackFlag = FALSE;
			ShowText(L"Enter Receipt", L"Number", L"");
		}
		break;
	case ENTERCARD_INPUT:  //Card enter and manual Entry
		m_bGoBackFlag = FALSE;
		if (!OpenDevice())
		{
			m_strErr1 = L"fail open device";
			PostMessage(ERROR_MESSAGE, 2, 1);
			break;
		}
		m_strMsg.m_nFormat = DT_RIGHT;
		m_strMsg2.m_nFormat = DT_LEFT;
		ShowText(m_strAmount, L"Insert/Swipe/Tap", L"or Key Card #");
		SetTimer(1, 30000, NULL);
		break;

// debit card 
	case DEBITCARDENTRY:
		m_bCancelPrint = TRUE;
		m_bGoBackFlag = FALSE;
		ShowText(L"Pass to", L"Customer", L"");
		SetTimer(1, 3000, NULL);
		m_bCancelInCustomer = TRUE;
		break;
	case DEBITCASHBACK: //CASHBACK
		if (m_bReturnFlag)
		{
			m_bGoBackFlag?GoBack():GoNext();
			break;
		}
		
		if (CDataFile::Read(L"CASHBACK", buf) && CString(buf) == L"Off")
		{
			m_bGoBackFlag?GoBack():GoNext();
		}
		else
		{
			m_bGoBackFlag = FALSE;
			str = L"Enter Cashback";
			str1 = L"Amount";
			CString str2 = L"$0.00";
			ShowText(str, str1, str2);
		}
		break;
	case DEBITSURCHARGEFEE: //surcharge fee
		if (m_bReturnFlag)
		{
			m_bGoBackFlag?GoBack():GoNext();
			break;
		}
		if (!CDataFile::Read(L"SURCHARGE", buf))
		{
			m_bGoBackFlag?GoBack():GoNext();
			return;
		}
		if (CString(buf) == L"$0.00" || CString(buf) == L"")
		{
			m_bGoBackFlag?GoBack():GoNext();
		}
		else
		{
			m_bGoBackFlag = FALSE;
			m_Bt.SetValue(1, L"  Cancel", IDB_CANCEL);
			m_Bt.ShowWindow(SW_SHOW);
			m_Bt1.SetValue(3, L"  OK", IDB_OK);
			m_Bt1.ShowWindow(SW_SHOW);
			str.Format(L"        %s  OK?", CString(buf));
			ShowText(L"Fee", str, L"");
			m_strSurcharge = CString(buf);
			break;
		}
	case DEBITTIPSELECT: //TipSelect
		if (m_bReturnFlag)
		{
			m_bGoBackFlag?GoBack():GoNext();
			break;
		}
		GetTipValue();
		break;
	case DEBITTOTALCOMFIRM: // TotalComfirm
		m_bGoBackFlag = FALSE;
		if (DoConfirm(m_bReturnFlag))
			GoNext();
		else
			OnCancel();
		break;
	case DEBITSELECTACCOUNT: // SelectAccount
		m_bGoBackFlag = FALSE;
		m_Bt.SetValue(1, L"  CHEQUE");
		m_Bt1.SetValue(3, L"  SAVING");
		m_Bt.ShowWindow(SW_SHOW);
		m_Bt1.ShowWindow(SW_SHOW);
		ShowText(L"Select Account", L"", L"");
		break;
	case DEBITONLINEPIN: //OnlinePin
		if (!EnterOnlinePin())
		{
			m_strErr1 = L"EnterOnlinePin";
			PostMessage(ERROR_MESSAGE, 2, 1);
		}
		else
		{
			ShowText(L"Please Wait...", L"", L"");
			GoToLevel(DOTRANSACTION);
		}
		break;
	case DEBITMERCHANTCOPY: // for merchant copy of the receipt
		if (m_TRREC.ComStatus == ST_OK  &&
				m_TRREC.TranStatus == ST_APPROVED)
			str = L"APPROVED";
		else
			str = L"Unsuccessful";
		m_strMsg2.m_nFormat = DT_LEFT;
		ShowText(str, L"Retrieve Card", L"Pass to Clerk");
		Print();
		break;
	case DEBITCUSTOMERCOPY: // for Customer copy of the receipt
		ShowText(L"Press OK for", L"customer copy", L"");
		SetTimer(1, 30000, NULL);
		break;
	case DEBITWAITCOPYEND: // waiting for Customer copy of the receipt ending...
		if (m_TRREC.ComStatus == ST_OK  &&
				m_TRREC.TranStatus == ST_APPROVED)
		{
			str.Format(L"AUTH : %s", CString(m_TRREC.AuthCode));
			m_strMsg2.m_nFormat = DT_LEFT;
			str1.Format(L"Inv.#: %s", CString(m_TRREC.InvoiceNo));
			ShowText(L"APPROVED", str, str1);
		}
		else
		{
			str.Format(L"Declined %s", CString(m_TRREC.RespCode));
			ShowText(L"Unsuccessful", str, L"");
		}
		break;
	case DEBITEND: // ending...
		CDialog::OnCancel();
		break;
// == == == == == == == == == == == == == == == == == == == == == == == == == == == == == == == == == == 
	case MANUALENTRYY: //Manual Entry enter expiry
		m_strMsg.SetCaption(L"Enter Expiry");
		m_strMsg1.SetCaption(L"Date           MMYY");
		m_strMsg2.SetCaption(L"");
		break;
	case MANUAL_IMPRINT_CARD:  //imprint card
		m_strMsg.SetCaption(L"Imprint Card");
		m_strMsg1.SetCaption(L"Press OK");
		m_strMsg2.SetCaption(L"");
		break;

// == == == == == == == == == == == == == = EMV Chip Card Debit Sale ：C chip page 122 == == == == == == == == == == == == == = 
	case CHIPENTRYY:
		m_bCancelPrint = TRUE;
		m_bGoBackFlag = FALSE;
		ShowText(L"Pass to", L"Customer", L"");
		SetTimer(1, 3000, NULL);
		m_bCancelInCustomer = TRUE;
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
			str.Format(L"        %s  OK?", CString(buf));
			ShowText(L"Select language", L"Choisir langue", L"");
		}
		break;
	case SELECT_APPLICATIION:
/*		if (!IsEMVMultiApplication())
		{
			//Select this single Application
			m_bGoBackFlag?GoBack():GoNext();
		}
		else
		{
			m_bGoBackFlag = FALSE;
			m_Bt.SetValue(1, L"   No");
			m_Bt.ShowWindow(SW_SHOW);
			m_Bt1.SetValue(3, L"   Yes");
			m_Bt1.ShowWindow(SW_SHOW);
			str = GetNextEMVApplicationLabel();
			ShowText(L"Select", str, L"");
		}
	*/	break;
	case APPLICATION_CONFIRM:
		m_bGoBackFlag = FALSE;
		m_Bt.SetValue(1, L"   No");
		m_Bt.ShowWindow(SW_SHOW);
		m_Bt1.SetValue(3, L"   Yes");
		m_Bt1.ShowWindow(SW_SHOW);
		ShowText(m_strCurrentApplicationLabel, L"OK?", L"");
		break;

	case CHIP_DEBITCASHBACK: //CASHBACK
		m_bCancelPrint = TRUE;
		if (m_TRREC.CardType == CARD_CREDIT)
		{
			m_bGoBackFlag?GoBack():GoNext();
			return;
		}

		if (CDataFile::Read(L"CASHBACK", buf) && CString(buf) == L"Off")
		{
			m_bGoBackFlag?GoBack():GoNext();
		}
		else
		{
			m_bGoBackFlag = FALSE;
			str = L"Enter Cashback";
			str1 = L"Amount";
			CString str2 = m_strCashback;//L"$0.00";
			ShowText(str, str1, str2);
		}
		break;
	case CHIP_DEBITSURCHARGEFEE: //surcharge fee
		if (m_TRREC.CardType == CARD_CREDIT)
		{
			m_bGoBackFlag?GoBack():GoNext();
			return;
		}

		if (!CDataFile::Read(L"SURCHARGE", buf))
		{
			m_bGoBackFlag?GoBack():GoNext();
			return;
		}
		if (CString(buf) == L"$0.00" || CString(buf) == L"")
		{
			m_bGoBackFlag?GoBack():GoNext();
		}
		else
		{
			m_bGoBackFlag = FALSE;
			m_Bt.SetValue(1, L"  Cancel", IDB_CANCEL);
			m_Bt.ShowWindow(SW_SHOW);
			m_Bt1.SetValue(3, L"  OK", IDB_OK);
			m_Bt1.ShowWindow(SW_SHOW);
			str.Format(L"        %s  OK?", CString(buf));
			ShowText(L"Fee", str, L"");
			m_strSurcharge = CString(buf);
			break;
		}
	case CHIP_DEBITTIPSELECT: //TipSelect
		GetTipValue();
		break;
	case CHIP_DEBITTOTALCOMFIRM: // TotalComfirm
		m_bGoBackFlag = FALSE;
		if (DoConfirm(m_bReturnFlag))
			GoNext();
		else
			OnCancel();
		break;
	case CHIP_DEBITSELECTACCOUNT: // SelectAccount
		if (m_TRREC.CardType == CARD_CREDIT)
		{
//			m_bGoBackFlag?GoBack():GoNext();
			m_strMsg2.m_nFormat = DT_LEFT;
			ShowText(L"Please Wait...", L"Do not remove", L"Card!");
			m_iLevel = EMV_WAIT;
			SetKeyReturn(KEY_ENTER);
			return;
		}

		m_bGoBackFlag = FALSE;
		m_Bt.SetValue(1, L"  CHEQUE");
		m_Bt1.SetValue(3, L"  SAVING");
		m_Bt.ShowWindow(SW_SHOW);
		m_Bt1.ShowWindow(SW_SHOW);
		ShowText(L"Select Account", L"", L"");
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
				ShowText(L"Please Wait...", L"", L"");
				GoToLevel(DOTRANSACTION);
			}
		}
		break;
	case CHIP_PASSTOCLERK: //Pass to clerk
		ShowText(L"Pass to clerk", L"Do not remove card", L"");
		SetTimer(1, 3000, NULL);
		break;

	case CHIP_DEBITMERCHANTCOPY: // for merchant copy of the receipt
		if (m_TRREC.ComStatus == ST_OK  &&
				m_TRREC.TranStatus == ST_APPROVED)
			str = L"APPROVED";
		else
			str = L"Unsuccessful";
		m_strMsg2.m_nFormat = DT_LEFT;
		ShowText(str, L"Remove card", L"Pass to clerk");
		Print();
		break;
	case CHIP_DEBITCUSTOMERCOPY: // for Customer copy of the receipt
		ShowText(L"Press OK for", L"customer copy", L"");
		SetTimer(1, 30000, NULL);
		break;
	case CHIP_DEBITWAITCOPYEND: // waiting for Customer copy of the receipt ending...
		if (m_TRREC.ComStatus == ST_OK  &&
				m_TRREC.TranStatus == ST_APPROVED)
		{
			str.Format(L"AUTH : %s", CString(m_TRREC.AuthCode));
			m_strMsg2.m_nFormat = DT_LEFT;
			str1.Format(L"Inv.#: %s", CString(m_TRREC.InvoiceNo));
			ShowText(L"APPROVED", str, str1);
		}
		else
		{
			str.Format(L"Declined %s", CString(m_TRREC.RespCode));
			ShowText(L"Unsuccessful", str, L"");
		}
		break;
	case CHIP_DEBITEND: // ending...
		if (GetScrStatus() == SCR_POWER_ON)	
			ShowText(L"Please",L"Remove Card",L"");
		else
			CDialog::OnCancel();
		break;
// == == == == == == == == == == == == == = Credit Card Sale ：C Swiped page 75 == == == == == == == == == == == == == == == == == = 
	case CREDITSWIPEENTRYY://FRAUD CHECK
		m_bCancelPrint = TRUE;
		m_bGoBackFlag = FALSE;
	    if (CDataFile::Read(L"FRAUD CHECK", buf) && CString(buf) == L"Off")
		{
			if (m_bReturnFlag)
				GoToLevel(DOTRANSACTION);
			else
				m_bGoBackFlag?GoBack():GoNext();
		}
		else
		{
			ShowText(L"Enter Last", L"4 Digits", L"");
		}
		break;
	case CREDITSWIPETOCUSTOMER:
	case MANUAL_SWIPETOCUSTOMER:
		if (m_bReturnFlag)
		{
			m_bGoBackFlag?GoBack():GoNext();
			break;
		}
		if (CDataFile::Read(L"TIP PROMPT", buf) && CString(buf) == L"Off")
		{
			m_iLevel = DOTRANSACTION;
			GoToLevel(m_iLevel);
		}
		else
		{
			ShowText(L"Pass to", L"Customer", L"");
			SetTimer(1, 3000, NULL);
			m_bCancelInCustomer = TRUE;
			m_bGoBackFlag = FALSE;
		}
		break;
	case CREDITSWIPETIPSELECT: //TipSelect
	case MANUAL_SWIPETIPSELECT: //TipSelect
		if (m_bReturnFlag)
		{
			m_bGoBackFlag?GoBack():GoNext();
			break;
		}
		GetTipValue();
		break;
	case CREDITSWIPETOTALCOMFIRM: // TotalComfirm
	case MANUAL_SWIPETOTALCOMFIRM: // TotalComfirm
		m_bGoBackFlag = FALSE;
		if (m_bReturnFlag)
		{
			m_bGoBackFlag?GoBack():GoNext();
			break;
		}
		if (DoConfirm(m_bReturnFlag))
			GoNext();
		else
			OnCancel();
		break;
	case CREDITSWIPETOCLERK:  //Pass to clerk
	case MANUAL_SWIPETOCLERK:  //Pass to clerk
		ShowText(L"Pass to", L"clerk", L"");
		SetTimer(1, 3000, NULL);
		break;
	case CREDITMERCHANTCOPY: // for merchant copy of the receipt
		m_strMsg2.m_nFormat = DT_LEFT;
		if (m_TRREC.ComStatus == ST_OK  &&
				m_TRREC.TranStatus == ST_APPROVED)
		{
			str.Format(L"AUTH : %s", CString(m_TRREC.AuthCode));
			m_strMsg2.m_nFormat = DT_LEFT;
			str1.Format(L"Inv.#: %s", CString(m_TRREC.InvoiceNo));
			ShowText(L"APPROVED", str, str1);
		}
		else
		{
			if (strlen(m_TRREC.HostRespText)  > 0)
				str = CString(m_TRREC.HostRespText);
			else
				str.Format(L"Declined %s", CString(m_TRREC.RespCode));
			ShowText(L"Unsuccessful", str, L"");
		}
		Print();
		break;
	case CREDITCUSTOMERCOPY: // for Customer copy of the receipt
		ShowText(L"Press OK for", L"customer copy", L"");
		SetTimer(1, 30000, NULL);
		break;
	case CREDITWAITCOPYEND: // waiting for Customer copy of the receipt ending...
		if (m_TRREC.ComStatus == ST_OK  &&
				m_TRREC.TranStatus == ST_APPROVED)
		{
			str.Format(L"AUTH : %s", CString(m_TRREC.AuthCode));
			m_strMsg2.m_nFormat = DT_LEFT;
			str1.Format(L"Inv.#: %s", CString(m_TRREC.InvoiceNo));
			ShowText(L"APPROVED", str, str1);
		}
		else
		{
			str.Format(L"Declined %s", CString(m_TRREC.RespCode));
			ShowText(L"Unsuccessful", str, L"");
		}
		break;
	case CREDITEND: // ending...
//	case MANUAL_END: // ending...
		CDialog::OnCancel();
		break;

// == == == == == == == == == == == == == == Do transaction (connect to host) == == == == == == == = 
	case DOTRANSACTION:
		DoTransaction();
		break;
	case CANCELENTRY:
		m_bCancelFlag = TRUE;
		ShowText(L"Transaction", L"Cancelled", L"");
		SetTimer(1, 30000, NULL);
		if (m_TRREC.CardError == CARD_NO_ERROR
			&& m_bCancelPrint)
			Print(0);
		break;
	case CANCELTOCLERK:
		if(m_TRREC.bEmvTransaction)
		{
			GoToLevel(EMV_REMOVE_CARD);
			break;
		}
		if (m_bCancelInCustomer)
		{
			ShowText(L"Pass to", L"Clerk", L"");
			SetTimer(1, 3000, NULL);
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
		ShowText(L"Select language", L"Choisir langue", L"");
		break;
	
	case EMV_CONFIRM_AMOUNT: //EMV confirm amount
		str.Format(L"     $ %s  OK?", DealAmountStr(m_TRREC.TotalAmount));
		m_strMsg2.m_nFormat = DT_LEFT;
		ShowText(L"Total :", str, L"Press OK to confirm");
		break;

	case EMV_SELECT_ACCOUNT: //EMV select account
		m_Bt.SetValue(1, L"  CHEQUE");
		m_Bt1.SetValue(3, L"  SAVING");
		m_Bt.ShowWindow(SW_SHOW);
		m_Bt1.ShowWindow(SW_SHOW);
		ShowText(L"Select Account", L"", L"");
		break;
	
	case EMV_REMOVE_CARD: //EMV Remove Card
		if (GetScrStatus() == SCR_POWER_ON)
			ShowText(L"Please",L"Remove Card",L"");
		else
			CDialog::OnCancel();
		break;
	
	case EMV_PASS_COUSTOMER: //EMV Remove Card
		ShowText(L"Pass to", L"Customer", L"");
		break;
	
	case EMV_START: //EMV Remove Card
		StartEMV();
		break;
	}
}

//----------------------------------------------------------------------------
//!	\brief	chekc display window's button number and move to right position
//----------------------------------------------------------------------------
void CReturnDlg::SetDisplayButton()
{
	int iTemp = 1;
	int BtNumber = 3;
	CString strBt1 = CDisplay::GetButtonText(1);
	CString strBt2 = CDisplay::GetButtonText(2);
	CString strBt3 = CDisplay::GetButtonText(3);

	switch(CDisplay::GetBtNumber())
	{
	case 1:
		m_Bt.SetValue(-1, strBt1);
		m_Bt.MoveWindow( iTemp,screenY-50,screenX/BtNumber-iTemp, 45);
		m_Bt.ShowWindow(SW_SHOW);
		break;
	case 2:
		m_Bt.SetValue(-1, strBt1);
		m_Bt.MoveWindow( iTemp,screenY-50,screenX/BtNumber-iTemp, 45);
		m_Bt.ShowWindow(SW_SHOW);
		m_Bt1.SetValue(-1, strBt2);
		m_Bt1.MoveWindow( screenX/BtNumber*2+iTemp,screenY-50,screenX/BtNumber-iTemp, 45);
		m_Bt1.ShowWindow(SW_SHOW);
		break;
	}
}

void CReturnDlg::OnCancel()
{
	Beep1();
	switch(m_iLevel)
	{
	case PASSWORD_INPUT: //Password Input
	case AMOUNT_INPUT: //Amout Input
	case CLERK_INPUT:  //Clerk ID input
	case RECEIP_TINPUT:  //Receipt input
		GoToLevel(CANCELENTRY);
		break;
	case ENTERCARD_INPUT:  //Card enter and manual Entry
		KillTimer(1);
		CloseDevice();
		GoToLevel(CANCELENTRY);
		break;
		
// debit card 
	case DEBITCARDENTRY:
		break;
	case DEBITCASHBACK: //CASHBACK
		GoToLevel(CANCELENTRY);
		break;
	case DEBITSURCHARGEFEE: //surcharge fee
		GoToLevel(CANCELENTRY);
		break;
	case DEBITTIPSELECT: //TipSelect
		break;
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
// == == == == == == == == == == == == == = EMV Chip Card Debit Sale ：C chip page 122 == == == == == == == == == == == == == = 
	case CHIPENTRYY:
		break;
	case SELECT_LANGUAGE: //select language
	case SELECT_APPLICATIION: //select application
	case APPLICATION_CONFIRM: //select comfirm
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
// == == == == == == == == == == == == == = Credit Card Sale ：C Swiped page 75 == == == == == == == == == == == == == == == == == = 
	case CREDITSWIPEENTRYY: //?＜Fraud Check
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
		KillTimer(1);
		CDialog::OnCancel();
		break;
	case CREDITWAITCOPYEND: // waiting for Customer copy of the receipt ending...
		break;
	case CREDITEND: // ending...
		break;

// == == == == == == == == == == == = /Manual Entry == == == == == == == == == == == == == == == == == == == == == == = 
	case MANUALENTRYY: //?＜Fraud Check
		GoToLevel(CANCELENTRY);
		break;
	case MANUAL_IMPRINT_CARD:
	case MANUAL_SWIPETOCUSTOMER:
		GoToLevel(CANCELENTRY);
		break;
	case MANUAL_SWIPETIPSELECT: //TipSelect
		break;
	case MANUAL_SWIPETOTALCOMFIRM: // TotalComfirm
		GoToLevel(CANCELENTRY);
		break;
	case MANUAL_SWIPETOCLERK:   //Pass to clerk
		GoToLevel(CANCELENTRY);
		break;
// == == == == == == == == == == == == == == == == == == == == == == == == == == == == == == == == == = 
	case DOTRANSACTION: // do transaction, can't cancel
		break;
	case CANCELENTRY: 
		break;
	case CANCELTOCLERK: 
		GoToLevel(CANCELENTRY);
		break;
// == == == == == == == Display Window == == == == == 
	case DISPLAY_WINDOW:
		if (CDisplay::GetMask() & MASK_CANCEL)
			CDisplay::SetKeyReturn(KEY_CANCEL);
		break;
	case EMV_SELECT_LANGUAGE: //EMV Select language
	case EMV_CONFIRM_AMOUNT: //EMV confirm amount
	case EMV_SELECT_ACCOUNT: //EMV select account
	case EMV_PASS_COUSTOMER: //pass to customer
		SetKeyReturn(KEY_CANCEL);
		break;
	case CHIP_DEBITCASHBACK: //CASHBACK
	case CHIP_DEBITSURCHARGEFEE: //surcharge fee
	case CHIP_DEBITTIPSELECT: //TipSelect
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
void CReturnDlg::OnOK()
{
	Beep1();
	CString str;
	switch(m_iLevel)
	{
	case PASSWORD_INPUT: //Password Input
		if (m_strPassword.GetLength() == 0)
		{
			Beep();
			return;
		}
		if (!CheckPassword())
		{
			m_IncorrectTimes++;
			if (m_IncorrectTimes >= 5)
			{
				CDataFile::Save(L"CLERK PW", L"On");
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
	case AMOUNT_INPUT:
		str = m_strMsg2.m_szCaption;
		str.Remove('$');
		str.Remove(' ');
		str.Remove('.');
		str.Remove(', ');
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
		ProcessManualCard(m_TRREC.Account);
		m_iLevel = MANUALENTRYY-1;
		break;
	case DEBITCARDENTRY:
		KillTimer(1);
		break;
	case DEBITCASHBACK: //CASHBACK
		m_strCashback = m_strMsg2.m_szCaption;
		break;
	case DEBITSURCHARGEFEE: //surcharge fee
		break;
	case DEBITTIPSELECT: //TipSelect
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
		Print(2);
		break;
	case DEBITWAITCOPYEND: // waiting for Customer copy of the receipt ending...
//		KillTimer(1);
		return;
// == == == == == == == == == == == == == = EMV Chip Card Debit Sale ：C chip page 122 == == == == == == == == == == == == == = 
	case CHIPENTRYY:
		KillTimer(1);
		break;
	case SELECT_LANGUAGE: //select language
	case SELECT_APPLICATIION: //select application
	case APPLICATION_CONFIRM: //select comfirm
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
		Print(2);
		break;
	case CHIP_DEBITWAITCOPYEND: // waiting for Customer copy of the receipt ending...
//		KillTimer(1);
		return;
// == == == == == == == == == == == == == = Credit Card Sale ：C Swiped page 75 == == == == == == == == == == == == == == == == == = 
	case CREDITSWIPEENTRYY: //Fraud Check
		str = m_strMsg2.m_szCaption;
		GoToLevel(DOTRANSACTION);
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
		Print(2);
		break;
	case CREDITWAITCOPYEND: // waiting for Customer copy of the receipt ending...
//		KillTimer(1);
		return;
// == == == == == == == == == == == = /Manual Entry == == == == == == == == == == == == == == == == == == == == == == = 
	case MANUALENTRYY: //Fraud Check
		str = m_strMsg2.m_szCaption;
		if (str.GetLength() != SZ_EXPIRY_DATE)
		{
			Beep();
			return;
		}
		str = m_strMsg2.m_szCaption.Mid(2,2) + m_strMsg2.m_szCaption.Mid(0,2) ;
		PutParam(m_TRREC.ExpDate, str);
		break;
	case MANUAL_IMPRINT_CARD:
		break;
	case MANUAL_SWIPETOCUSTOMER:
		KillTimer(1);
		break;
	case MANUAL_SWIPETIPSELECT: //TipSelect
		break;
	case MANUAL_SWIPETOTALCOMFIRM: // TotalComfirm
		break;
	case MANUAL_SWIPETOCLERK:   //Pass to clerk
		KillTimer(1);
		GoToLevel(DOTRANSACTION);
		return;

// == == == == == == == == == == == == == == == == == == == == == == == == == == == == == == == == == = 
	case CANCELENTRY: 
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
	case EMV_CONFIRM_AMOUNT: //EMV confirm amount
	case EMV_SELECT_ACCOUNT: //EMV select account
		m_strMsg2.m_nFormat = DT_LEFT;
		ShowText(L"Please Wait...", L"Do not remove", L"Card!");
		m_iLevel = EMV_WAIT;
		SetKeyReturn(KEY_ENTER);
		return;
	case EMV_PASS_COUSTOMER: //pass to customer
		SetKeyReturn(KEY_ENTER);
		return;
	case CHIP_DEBITCASHBACK: //CASHBACK
		m_strCashback = m_strMsg2.m_szCaption;
		break;
	case CHIP_DEBITSURCHARGEFEE: //surcharge fee
	case CHIP_DEBITTIPSELECT: //TipSelect
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

//----------------------------------------------------------------------------
//!	\brief	handle key input function for any GUI's, include correction key(nChar == 8)
//----------------------------------------------------------------------------
void CReturnDlg::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	Beep1();
	switch(m_iLevel)
	{
	case PASSWORD_INPUT: //Password Input
		m_strMsg2.SetCaption(DealPassword(nChar, m_strPassword));
		break;
	case AMOUNT_INPUT: //Amout Input
	case DEBITCASHBACK: //CASHBACK
	case CHIP_DEBITCASHBACK: //CASHBACK
		m_strMsg2.SetCaption(DealAmount(nChar, m_strMsg2.m_szCaption));
		break;
	case CLERK_INPUT:  //Clerk ID input
		m_strMsg2.SetCaption(DealClerkID(nChar, m_strMsg2.m_szCaption));
		break;
	case RECEIP_TINPUT:  //Receipt input
		m_strMsg2.SetCaption(DealReceipt(nChar, m_strMsg2.m_szCaption));
		break;
	case ENTERCARD_INPUT:  //Card enter and manual Entry
		KillTimer(1);
		m_strMsg2.SetCaption(KeyAccount(nChar, m_strMsg2.m_szCaption));
		if (m_KeyEntry)
			m_strMsg2.m_nFormat = DT_RIGHT;
		SetTimer(1, 30000, NULL);
		break;
	case DEBITTIPSELECT: //TipSelect
	case CHIP_DEBITTIPSELECT: //TipSelect
		if (nChar == 8)
			GoBack();
		break;
	case DEBITSURCHARGEFEE: //surcharge fee
	case DEBITSELECTACCOUNT: // SelectAccount
	case CHIP_DEBITSURCHARGEFEE: //surcharge fee
	case CHIP_DEBITSELECTACCOUNT: // SelectAccount
		if (nChar == '1')
			OnBnClickedButton(IDC_BUTTON1);
		if (nChar == '3')
			OnBnClickedButton(IDC_BUTTON2);
		if (nChar == 8)
			GoBack();
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
// == == == == == == == == == == == == == = EMV Chip Card Debit Sale ：C chip page 122 == == == == == == == == == == == == == = 
	case SELECT_LANGUAGE: //select language
	case SELECT_APPLICATIION: //select application
	case APPLICATION_CONFIRM: //select comfirm
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

// == == == == == == == == == == == == == = Credit Card Sale ：C Swiped page 75 == == == == == == == == == == == == == == == == == = 
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
		if (nChar == 8)
		{
			KillTimer(1);
			CDialog::OnCancel();
		}
		break;
// == == == == == == == == == == == == == = Credit Card Sale ：C Manual Entry ：C Swiped page 79 == == == == == == == == == == == == == == == == == = 
	case MANUALENTRYY:
		m_strMsg2.SetCaption(KeyExpiry(nChar, m_strMsg2.m_szCaption));
		break;
	case MANUAL_IMPRINT_CARD: // for Customer copy of the receipt
	case MANUAL_SWIPETOCUSTOMER: // for Customer copy of the receipt
		if (nChar == 8)
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
		if (nChar == 8)
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
	case EMV_CONFIRM_AMOUNT: //EMV confirm amount
		if (nChar == 8)
			SetKeyReturn(KEY_CORR);
		break;
// == == == == == == == == == == == == = End == == == == == == == == == == == == == == == = 
	}
	SetFocus();
}

//----------------------------------------------------------------------------
//!	\brief	handle button press function for any GUI's, 
//----------------------------------------------------------------------------
void CReturnDlg::OnBnClickedButton(UINT nID)
{
	switch(m_iLevel)
	{
	case DEBITSELECTACCOUNT: // SelectAccount
		if (nID == IDC_BUTTON1)
			m_TRREC.AcctType = ACCT_CHEQUE;
		else
			m_TRREC.AcctType = ACCT_SAVING;
		GoNext();
		break;
	case DEBITSURCHARGEFEE: //surcharge fee
	case CHIP_DEBITSURCHARGEFEE: //surcharge fee
		if (nID == IDC_BUTTON1)
			CDialog::OnCancel();
		else
			OnOK();
		break;
	case SELECT_LANGUAGE: //EMV Select language
		if (nID == IDC_BUTTON1)
			EMVSelectLanguage(ENGLISH);
		else
			EMVSelectLanguage(FRENCH);
		GoNext();
		break;
	case SELECT_APPLICATIION: //select application
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

// == == == == == == == Display Window == == == == == 
	case DISPLAY_WINDOW:
		CDisplay::SetKeyReturn(KEY_BUT1+nID-IDC_BUTTON1);
		break;
	case EMV_SELECT_LANGUAGE: //EMV Select language
	case EMV_SELECT_ACCOUNT: //EMV select account
		m_Bt.ShowWindow(SW_HIDE);
		m_Bt1.ShowWindow(SW_HIDE);
		m_strMsg2.m_nFormat = DT_LEFT;
		ShowText(L"Please Wait...", L"Do not remove", L"Card!");
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
		ShowText(L"Please Wait...",L"Do not remove",L"Card!");
		m_iLevel = EMV_WAIT;
		SetKeyReturn(KEY_ENTER);
		break;
	}
}

//----------------------------------------------------------------------------
//!	\brief	handle event when Host connection is done 
//----------------------------------------------------------------------------
void CReturnDlg::Done()
{
	AfterHost();

	if (m_TRREC.CardType == CARD_DEBIT)
	{
		if (m_TRREC.EntryMode == ENTRY_MODE_SWIPED)
			GoToLevel(DEBITMERCHANTCOPY);
		else if (m_TRREC.EntryMode == ENTRY_MODE_CHIP)
			GoToLevel(CHIP_DEBITMERCHANTCOPY);

	}
	else if (m_TRREC.CardType == CARD_CREDIT)
	{
		if (m_TRREC.EntryMode == ENTRY_MODE_SWIPED ||
			  m_TRREC.EntryMode == ENTRY_MODE_MANUAL ||
			  m_TRREC.EntryMode == ENTRY_MODE_CTLS)
		{			  
			GoToLevel(CREDITMERCHANTCOPY);
		}
		else if(m_TRREC.EntryMode == ENTRY_MODE_CHIP)
			GoToLevel(CHIP_DEBITMERCHANTCOPY);
	}
}

//----------------------------------------------------------------------------
//!	\brief	Message entry function for any GUI's, 
//----------------------------------------------------------------------------
LRESULT CReturnDlg::DefWindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	CString strTemp; 
	DWORD len = 0;
   DWORD decode = 0;

	if (wParam == 1)
	{
//	TRACE(L"CEnterCardDlg RFID event = %x\n", message);
		BYTE buf[2048] = {0};
		DWORD len;
		switch(message)
		{
			case RFID_EVENT_READY:
				m_RFID.GetData();
				m_iKeyStatus =  KEY_CTLS;
				m_TRREC.EntryMode = ENTRY_MODE_CTLS;
				m_RFID.GetData((BYTE*)buf, &len);
				CloseDevice();
				ProcessCtlsTLV((USHORT)len, (BYTE*)buf);
				if (m_TRREC.CtlsData.C2[0] == 0x05)		// EMV
				{
					m_TRREC.bEmvTransaction = TRUE;
					m_TRREC.bEmvCtlsTransaction = TRUE;
					m_TRREC.bCtlsTransaction = TRUE;
				}
				else if (m_TRREC.CtlsData.C2[0] == 0x04)	// MSR
				{
					m_TRREC.bEmvTransaction = FALSE;
					m_TRREC.bEmvCtlsTransaction = FALSE;
					m_TRREC.bCtlsTransaction = TRUE;
				}
				else if (m_TRREC.CtlsData.C2[0] == 0x06)	// Terminated
				{
					// Get error code from C3
					TRACE(L"RFID ERROR\n");
					return 0;
				}
				else
				{
//					assert(FALSE);
					m_strErr1 = L"CARD NOT SUPPORT";
					ErrorExit(ENTERCARD_INPUT);
					return 0;
				}
				
				m_TRREC.CardType = CARD_CREDIT;	// FIXME: will change when have FLASH support
				m_TRREC.CardError = ProcessSwipedCard(m_TRREC.Track2);
				
				// FIXME: CTLS kernel currently doesn't return AID for VISA, just fake it for now.
				if (m_TRREC.bEmvCtlsTransaction && strcmp("V ", m_TRREC.ServType) == 0)
				{
					strcpy(m_TRREC.EmvData.AID, "A0000000031010");				
				}
				
				m_iLevel = DOTRANSACTION;
				WorkLoop(m_iLevel);
				return 0;
				break;
			case RFID_EVENT_TIMEOUT:
				m_RFID.Cancel();
				TRACE(L"RFID_EVENT_TIMEOUT\n");
				PostMessage(ERROR_MESSAGE, 2, 5);
				break;
			case RFID_EVENT_CANCELLED:
				break;
			case RFID_EVENT_ERROR:
				m_RFID.Cancel();
				ReOpenRFID();
				break;
			default:
				break;	
		}	
	}
	else if (wParam == 0)
	{
		switch(message)
		{
			case MSR_EVENT_DEV_OFFLINE :
				m_strErr1 = L"MSR Open Fail !";
				ErrorExit();
				return NULL;
			case MSR_EVENT_DATA_READY :
			case MSR_EVENT_CT_STATUS :
			case MSR_EVENT_READ_ERROR :
				m_MSR.MsrGetTrackData();
				m_MSR.GetMSRData(2, (BYTE*)m_TRREC.Track2, &len, &decode);
				m_TRREC.CardError = ProcessSwipedCard(m_TRREC.Track2);
				if (m_TRREC.CardError != CARD_NO_ERROR)
				{
					m_strErr1 = L"CARD NOT SUPPORT";
					CloseDevice();
					ErrorExit(ENTERCARD_INPUT);
					return 0;
				}

				m_iKeyStatus =  KEY_MSR;
				m_TRREC.EntryMode = ENTRY_MODE_SWIPED;

				CloseDevice();
/*
				if (m_TRREC.CardType == CARD_DEBIT)
					m_iLevel = DEBITCARDENTRY;
				else if (m_TRREC.CardType == CARD_CREDIT)
					m_iLevel = CREDITSWIPEENTRYY;
*/
				m_iLevel = DOTRANSACTION;
				WorkLoop(m_iLevel);

				return 0;
		}
	}
	else if (wParam == 2 && message == ERROR_MESSAGE)
	{ 
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
	}
	return CDialog::DefWindowProc(message, wParam, lParam);
}

//----------------------------------------------------------------------------
//!	\brief	call CFinan:: DoTransaction for connect host
//----------------------------------------------------------------------------
void CReturnDlg::DoTransaction()
{
	if (!IsNetWorkConnect())
	{
		m_strErr1 = L" No Network!";
		ErrorExit();
		return;
	}

	m_strMsg.SetCaption(L"Please Wait...");
	m_strMsg1.SetCaption(L"");
	m_strMsg2.SetCaption(L"");
	CFinan::DoTransaction();
}

//----------------------------------------------------------------------------
//!	\brief	Error display and exit
//----------------------------------------------------------------------------
void CReturnDlg::ErrorExit()
{
	m_strMsg.m_nFormat = DT_LEFT;
	m_strMsg1.m_nFormat = DT_LEFT;
	m_strMsg.SetColor(RGB(255, 0, 0), RGB(255, 255, 255));
	m_strMsg1.SetColor(RGB(255, 0, 0), RGB(255, 255, 255));
//	m_strMsg.SetCaption(L"Error: ");
	m_strMsg1.SetCaption(m_strErr1);
	m_strMsg2.SetCaption(L"");
	m_Bt.ShowWindow(SW_HIDE);
	m_Bt1.ShowWindow(SW_HIDE);
	m_iLevel = ERRORENTRY;
	SetTimer(1, 5000, NULL);
}
//----------------------------------------------------------------------------
//!	\brief	Error display and go to level
// param level is level index
//----------------------------------------------------------------------------
void CReturnDlg::ErrorExit(int level)
{
	m_strMsg.m_nFormat = DT_LEFT;
	m_strMsg1.m_nFormat = DT_LEFT;
	m_strMsg.SetColor(RGB(255, 0, 0),RGB( 255,255,255));
	m_strMsg1.SetColor(RGB(255, 0, 0),RGB( 255,255,255));
//	m_strMsg.SetCaption(L"Error: ");
	m_strMsg1.SetCaption(m_strErr1);
	m_strMsg2.SetCaption(m_strErr2);
	m_iLevel = ERRORENTRY1;
	SetTimer(1,5000,NULL);
	m_iNextLevel = level;
//	GoToLevel(level);
}
//----------------------------------------------------------------------------
//!	\brief	window close
//----------------------------------------------------------------------------
void CReturnDlg::OnDestroy()
{
	KillTimer(1);
	CBaseDlg::OnDestroy();
	Sleep(100);
}

//----------------------------------------------------------------------------
//!	\brief	go back last GUI
//----------------------------------------------------------------------------
void CReturnDlg::GoBack()
{
	switch(m_iLevel)
	{
	case DEBITCARDENTRY:
	case MANUALENTRYY:
	case CHIPENTRYY:
	case CREDITSWIPEENTRYY:
	case CHIP_DEBITCASHBACK:
		return;
	case PASSWORD_INPUT:
		CDialog::OnCancel();
		return;
	case AMOUNT_INPUT:
		if (!m_bReturnFlag)
		{
			CDialog::OnCancel();
			return;
		}
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
void CReturnDlg::GoNext()
{
	switch(m_iLevel)
	{
	case EMV_REMOVE_CARD:
		return;
	}

	m_iLevel++;
	PostMessage(ERROR_MESSAGE, 2, 2);
}

//----------------------------------------------------------------------------
//!	\brief	go to GUI
// param Level is GUI index
//----------------------------------------------------------------------------
void CReturnDlg::GoToLevel(int level)
{
	m_iLevel = level;
	PostMessage(ERROR_MESSAGE, 2, 2);
}
