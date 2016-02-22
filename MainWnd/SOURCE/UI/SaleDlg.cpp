#include "stdafx.h"
#include "SaleDlg.h"
#include "SaioBase.h"
#include "SaioPinpad.h"

#include "..\\Utils\\string.h"
#include "..\\Utils\\TimeUtil.h"
#include "..\\Utils\\StrUtil.h"
#include "..\\Utils\\TotalUtil.h"
#include "..\\DEFS\\constant.h"
#include "..\\UI\\display.h"
		
#include "..\\utils\\util.h"
#include "..\\EMV\\PinInput.h"
#include "..\\data\\RecordFile.h"
#define PASSWORD_INPUT					0
#define AMOUNT_INPUT					1
#define CLERK_INPUT						2
#define INVOICE_TINPUT					3
#define RECEIP_TINPUT					4
#define ENTERCARD_INPUT					5
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
#define CANCELEMVCARDBEREMOVED				201
#define CANCELTOCLERK						202
#define CANCELPRINT						    203
#define CANCELCUSTOMERCOPY					204
#define CANCELEND							205

#define EMVCARDBEREMOVED					206

int counter99 = 0;

#pragma comment(lib, "SaioPinpad.lib")

IMPLEMENT_DYNAMIC(CSaleDlg, CBaseDlg)
//-----------------------------------------------------------------------------
//!	\CDialog class virtual function
//-----------------------------------------------------------------------------
CSaleDlg::CSaleDlg(CWnd* pParent)
	: CBaseDlg(CBaseDlg::IDD, pParent)
{
	m_bReturnFlag = FALSE;
	m_strAmount = L"$0.00";
	m_bEmvEnabled = FALSE;
	m_iRfidTimer = 0;
	m_bTranInProgress = FALSE;
	m_bSendECRBackFlag = FALSE;
	m_pParentWnd = pParent;
}

//-----------------------------------------------------------------------------
//!	\CDialog class virtual function
//-----------------------------------------------------------------------------
CSaleDlg::~CSaleDlg(void)
{ 
	CLanguage::SetDisplayWnd(NULL);
	CLanguage::SetLanguage();
	CloseDevice(FALSE);
}

//-----------------------------------------------------------------------------
//!	\CDialog class virtual function
//-----------------------------------------------------------------------------
void CSaleDlg::DoDataExchange(CDataExchange* pDX)
{
	CBaseDlg::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CSaleDlg, CBaseDlg)
		ON_COMMAND_RANGE(IDC_BUTTON1, IDC_BUTTON2, &CSaleDlg::OnBnClickedButton)
		ON_WM_CHAR()
		ON_WM_KEYDOWN()
END_MESSAGE_MAP()

// CMainWnd message handlers
//-----------------------------------------------------------------------------
//!	\CDialog class virtual function
//-----------------------------------------------------------------------------
BOOL CSaleDlg::OnInitDialog()
{
	char* buf;
	if (m_bReturnFlag)
	{
		m_TitleIndex = CLanguage::IDX_REFUND;
		CLanguage::GetTextFromIndex(CLanguage::IDX_REFUND,&buf);
	}
	else
	{
		m_TitleIndex = CLanguage::IDX_PURCHASE;
		CLanguage::GetTextFromIndex(CLanguage::IDX_PURCHASE,&buf);
	}

	m_Title = CString(buf);

	CBaseDlg::OnInitDialog();

	if (m_bReturnFlag)
	{
		m_TRREC.TranCode = TRAN_REFUND;
		m_iLevel = PASSWORD_INPUT;
	}
	else
	{
		m_TRREC.TranCode = TRAN_PURCHASE;
		if (m_strAmount.GetLength() == 0 || m_strAmount == L"$0.00")
		{
			m_strAmount = L"$0.00";
			m_iLevel = AMOUNT_INPUT;
		}
		else
			m_iLevel = CLERK_INPUT;
	}

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

	m_strCashback = L"$0.00";

		
	CString str = CLanguage::GetText(CLanguage::IDX_PURCHASE) + L" "+ CLanguage::GetText(CLanguage::IDX_INITIALIZING);
	CString str1= L"";
	CString str2= L"";
	CString str3 = L"";											//JC Mar 13/15
	ShowText(str, str1, str2);

	char* pbuf;
	if (!OpenDevice())
	{
		CLanguage::GetTextFromIndex(CLanguage::IDX_FAIL_OPEN_DEVICE,&pbuf);
		m_strErr1 = CString(pbuf);
		PostMessage(ERROR_MESSAGE, 2, 1);
		strcpy(m_ErrCode,ERROR_OPEN_DEVICE);
	}
	
	return TRUE;  // return TRUE unless you set the focus to a control
}

//-----------------------------------------------------------------------------
//!	\Set sale txn amount
//!	\param	Amount - amount char string
//-----------------------------------------------------------------------------
void CSaleDlg::SetAmount(char* Amount)
{
	if(Amount == NULL)
		m_strAmount = L"$0.00";
	else
		m_strAmount = L"$"+DealAmountStr(Amount);
}

//----------------------------------------------------------------------------
//!	\brief	timer function
//  nIDEvent 1 is for time out 
//  nIDEvent 2 is for focus
//  nIDEvent 99 is for check network connection's status (it is 1 sec now)
//----------------------------------------------------------------------------
void CSaleDlg::OnTimer(UINT_PTR nIDEvent)
{
	if (nIDEvent == 1)
	{
		KillTimer(1);
		if (m_iLevel == ENTERCARD_INPUT)
		{
			CloseDevice();
			char* buf;
			CLanguage::GetTextFromIndex(CLanguage::IDX_TIME_OUT,&buf);
			m_strErr1 = CString(buf);
			m_strErr2 = L"";
			strcpy(m_ErrCode,ERROR_CONDITION_CODE_TIMEOUT);
			sprintf(m_Customer_Text, "%s", buf);						//JC April 4/15 to save Customer Text
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

	if (nIDEvent == 3)
	{
		SetFocus();
		KillTimer(3);
        Print(2);

		SendECRBack();
		CloseDevice(FALSE);
		return;
	}
	if (nIDEvent == 99)
	{
		counter99++;
		TRACE(L"99 COUNTER: %d\n", counter99);
		//if (m_iLevel == ENTERCARD_INPUT && !m_KeyEntry)// && m_bInsertCard)
		//{
		//	if ( IsCardInSlot())
		//	{
		//		KillTimer(1);
		//		TRACE(L"TIMEer = %d  IsCardInSlot() m_bInsertCard = %d\n",nIDEvent,m_bInsertCard?1:0);

		//		if ( !m_bInsertCard )
		//		{
		//			if (m_bEmvEnabled)
		//			{
		//				m_bInsertCard = TRUE;
		//				m_bEMVPowerTest = FALSE;
		//				m_strMsg.m_nFormat = DT_LEFT;
		//				m_strMsg2.m_nFormat = DT_LEFT;
		//				m_strMsg.SetCaption(CLanguage::GetText(CLanguage::IDX_PLEASE_WAIT));
		//				m_strMsg1.SetCaption(CLanguage::GetText(CLanguage::IDX_DO_NOT_REMOVE));
		//				m_strMsg2.SetCaption(CLanguage::GetText(CLanguage::IDX_CARD));
		//			}
		//			else
		//			{
		//				//IDX_REMOVE_CARD_QUICKLY
		//				m_strMsg.m_nFormat = DT_LEFT;
		//				m_strMsg2.m_nFormat = DT_LEFT;
		//				m_strMsg1.SetCaption(CLanguage::GetText(CLanguage::IDX_REMOVE_CARD_QUICKLY));
		//			}
		//			return;
		//		}
		//		//if (m_bInsertCard && m_bEMVPowerTest )
		//		//	return;

		//		//BOOL flag = FALSE;
		//		//m_bEMVPowerTest = TRUE;

		//		//KillTimer(99);
		//		//for(int i = 0;i<3;i++)
		//		//{
		//		//	flag = m_SCR.GetState();
		//		//	if (flag)
		//		//		break;
		//		//	Sleep(400);
		//		//}

		//		//if (flag)
		//		//{
		//		//	m_SCR.Close();
		//		//	CloseDevice(); //MAG CLOSE
		//		//	GoToLevel(EMV_PASS_COUSTOMER);
		//		//}
		//		//else
		//		//{
		//		//	OpenMSRDevice();
		//		//	m_iWaitMSR = 0;
		//		//	m_strMsg.m_nFormat = DT_LEFT;
		//		//	m_strMsg.SetCaption(CLanguage::GetText(CLanguage::IDX_PLEASE));
		//		//	m_strMsg1.SetCaption(CLanguage::GetText(CLanguage::IDX_REMOVE_CARD_QUICKLY));
		//		//	m_strMsg2.SetCaption(L"");
		//		//}
		//		//SetTimer(99,1000,NULL);
		//		return;
		//	}
		//	else
		//	{
		//		TRACE(L"TIMEer = %d  NO CardInSlot() m_bInsertCard = %d\n",nIDEvent,m_bInsertCard?1:0);
		//		if (m_bInsertCard)
		//		{
		//			m_iWaitMSR++;
		//			if (m_iWaitMSR < 2 )
		//				return;
		//			if(m_iWaitMSR == 2 )
		//			{
		//				m_strMsg.SetColor(RGB(255, 0, 0), RGB(255, 255, 255));
		//				m_strMsg1.SetColor(RGB(255, 0, 0), RGB(255, 255, 255));
		//				m_strMsg2.SetColor(RGB(255, 0, 0), RGB(255, 255, 255));
		//				m_strMsg.SetCaption(CLanguage::GetText(CLanguage::IDX_ERROR));
		//				m_strMsg1.SetCaption(CLanguage::GetText(CLanguage::IDX_FAILED_TO));
		//				m_strMsg2.SetCaption(CLanguage::GetText(CLanguage::IDX_READ_SWIPE));
		//				return;
		//			}
		//			if(m_iWaitMSR == 3 )
		//			{
		//				CloseDevice();
		//			}

		//			if ( m_iWaitMSR > 6)
		//			{
		//				KillTimer(99);
		//				m_bInsertCard = FALSE;
		//				CDialog::OnCancel();
		//			}
		//			return;
		//		}
		//	}
		//}

		//if ( m_TRREC.bFallback )
		//{
		//	if ( !IsCardInSlot())
		//	{
		//		if (m_bInsertCard)
		//		{
		//			m_iWaitMSR++;
		//			if (m_iWaitMSR < 2 )
		//				return;
		//			if(m_iWaitMSR == 2 )
		//			{
		//				m_strMsg.SetColor(RGB(255, 0, 0), RGB(255, 255, 255));
		//				m_strMsg1.SetColor(RGB(255, 0, 0), RGB(255, 255, 255));
		//				m_strMsg2.SetColor(RGB(255, 0, 0), RGB(255, 255, 255));
		//				m_strMsg.SetCaption(CLanguage::GetText(CLanguage::IDX_ERROR));
		//				m_strMsg1.SetCaption(CLanguage::GetText(CLanguage::IDX_CARD_NOT_SUPPORT));
		//				m_strMsg2.SetCaption(CLanguage::GetText(CLanguage::IDX_NO_SUPPORTED));
		//				return;
		//			}
		//			if(m_iWaitMSR == 3 )
		//			{
		//				CloseDevice();
		//			}

		//			if ( m_iWaitMSR > 6)
		//			{
		//				KillTimer(99);
		//				m_bInsertCard = FALSE;
		//				CDialog::OnCancel();
		//			}
		//			return;
		//		}
		//	}
		//}
		//if (m_iLevel == EMV_REMOVE_CARD || m_iLevel == CHIP_DEBITEND || m_iLevel == EMVCARDBEREMOVED || m_iLevel == CANCELEMVCARDBEREMOVED)
		//{
		//	if (GetScrStatus()!= SCR_CARD_ABSENT && m_TRREC.EntryMode == ENTRY_MODE_CHIP)
		//	{
		//		Beep();
		//		return;
		//	}
		//	else
		//	{
		//		if ( m_TRREC.bFallback && m_TRREC.EntryMode > ENTRY_MODE_MANUAL)
		//			GoToLevel(ENTERCARD_INPUT);
		//		else
		//		{
		//			if(m_iNextLevel != 0)
		//			{
		//				GoToLevel(m_iNextLevel);
		//				m_iNextLevel = 0;
		//			}
		//			else
		//				CDialog::OnCancel();
		//		}
		//	}
		//}
		//else if (m_iLevel ==DISPLAY_WINDOW && m_TRREC.EntryMode == ENTRY_MODE_CHIP && CDisplay::m_bStopDisplay)
		//{
		//	if (GetScrStatus() == SCR_CARD_ABSENT)
		//	{
		//		CDisplay::SetKeyReturn(KEY_CARD_REMOVED);
		//	}
		//}
	}
	CBaseDlg::OnTimer(nIDEvent);
}

//-----------------------------------------------------------------------------
//!	\Check the card is inserted in slot
//-----------------------------------------------------------------------------
BOOL CSaleDlg::IsCardInSlot()
{
	return GetScrStatus() != SCR_CARD_ABSENT;
}
//----------------------------------------------------------------------------
//!	\brief	this is main work loop function for any GUI's entry, 
// param  index is for GUI's index id
//----------------------------------------------------------------------------
void CSaleDlg::WorkLoop(int index)
{
CDataFile::CheckMemory(L"WorkLoop");
	char* pbuf;
	char *pbuf2;					//JC Apr 4/15
	char *pbuf3;					//JC Apr 4/15
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
	CString str3 = L"";					//JC Mar 13/15
	
 	int i = 0;
	switch(index)
	{
	case PASSWORD_INPUT: //Password Input
		m_bGoBackFlag = FALSE;
		for (i = 0;i<m_strPassword.GetLength();i++)
			str2 += L"*";
		CLanguage::GetTextFromIndex(CLanguage::IDX_ENTER_PASSWORD,&pbuf);
		str = CString(pbuf);
		str1 = L"";
		ShowText(str, str1, str2);
		break;
	case AMOUNT_INPUT: //Amout Input
		m_bGoBackFlag = FALSE;
		str = CLanguage::GetText(CLanguage::IDX_ENTER_AMOUNT);
		str1= L"";
		ShowText(str, str1, m_strAmount);
		break;
	case CLERK_INPUT:  //Clerk ID input
		GoNext();
		break;
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
		GoNext();
		break;
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
		GoNext();
		break;
		/*if (CDataFile::Read(L"RECEIPT", buf) && CString(buf) == L"Off")
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
		break;*/
	case ENTERCARD_INPUT:  //Card enter and manual Entry
		m_bInsertCard = FALSE;
		m_iWaitMSR = 0;
		m_bGoBackFlag = FALSE;
		m_bEMVPowerTest = FALSE;

		m_bCancelPrint = FALSE;
		m_TRREC.bEmvTransaction = FALSE;
		m_TRREC.EntryMode = 0;
		m_iNextLevel = 0;
		
		// MOVED TO INIT
//CDataFile::CheckMemory(L"start OpenDevice");
//		ShowPicture();
//		if (!OpenDevice())
//		{
//			CLanguage::GetTextFromIndex(CLanguage::IDX_FAIL_OPEN_DEVICE,&pbuf);
//			m_strErr1 = CString(pbuf);//L"fail open device";
//			PostMessage(ERROR_MESSAGE, 2, 1);
//			strcpy(m_ErrCode,ERROR_OPEN_DEVICE);
//			break;
//		}
//CDataFile::CheckMemory(L"end OpenDevice");

		m_strMsg.m_nFormat = DT_RIGHT;
		m_strMsg2.m_nFormat = DT_LEFT;
//		CLanguage::GetTextFromIndex(CLanguage::IDX_INSERT_SWIPE_TAP,&pbuf);
		CLanguage::GetTextFromIndex(CLanguage::IDX_SWIPE_TAP,&pbuf);
		str = CString(pbuf);

		str2= CLanguage::GetText(CLanguage::IDX_KEY_CARD_NUMBER);
		if(CDataFile::Read(L"MANUAL ENTRY",buf) && CString(buf) == L"Off")
			str2 = L"";

		ShowText(m_strAmount,str ,str2 );
		SetTimer(1, 30000, NULL);
CDataFile::CheckMemory(L"ENTERCARD_INPUT");
		break;

// debit card 
	case DEBITCARDENTRY:
		m_bCancelPrint = TRUE;
		m_bGoBackFlag = FALSE;
			
		CLanguage::GetTextFromIndex(CLanguage::IDX_PASS_TO,&pbuf);
		str = CString(pbuf);
		CLanguage::GetTextFromIndex(CLanguage::IDX_CUSTOMER,&pbuf);
		str1=  CString(pbuf);

		str2= L"";
		ShowText(str, str1, str2);

		SetTimer(1, 3000, NULL);
		m_bCancelInCustomer = TRUE;
		CLanguage::SetCustomerFlag(TRUE);

		break;
	case DEBITCASHBACK: //CASHBACK
		GoNext();
		break;
		/*if (m_bReturnFlag)
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
			CLanguage::GetTextFromIndex(CLanguage::IDX_ENTER_CASHBACK,&pbuf);
			str = CString(pbuf);
			CLanguage::GetTextFromIndex(CLanguage::IDX_AMOUNT,&pbuf);
			str1=  CString(pbuf);
			CString str2 = L"$0.00";
			ShowText(str, str1, str2);
		}
		break;*/
	case DEBITSURCHARGEFEE: //surcharge fee
		GoNext();
		break;
		//if (m_bReturnFlag)
		//{
		//	m_bGoBackFlag?GoBack():GoNext();
		//	break;
		//}
		//m_strCashback.Remove(' ');
		//if ( CheckAmount(m_strCashback) && CDataFile::Read(L"CASHBACK", buf) && CString(buf) == L"On")
		//{
		//	m_bGoBackFlag = FALSE;
		//	TCHAR buf1[10] = L"";
		//	CDataFile::Read(L"CASHBACKFEE",buf1);
		//	if ( !CheckAmount(CString(buf1)))
		//	{
		//		m_bGoBackFlag?GoBack():GoNext();
		//		break;
		//	}

		//	CLanguage::GetTextFromIndex(CLanguage::IDX_CANCEL,&pbuf);
		//	str.Format(L"  %s",CString(pbuf));
		//	m_Bt.SetValue(1, str, IDB_CANCEL);
		//	m_Bt.ShowWindow(SW_SHOW);
		//	CLanguage::GetTextFromIndex(CLanguage::IDX_OK2,&pbuf);
		//	str.Format(L"  %s",CString(pbuf));
		//	m_Bt1.SetValue(3, str, IDB_OK);
		//	m_Bt1.ShowWindow(SW_SHOW);
		//	CLanguage::GetTextFromIndex(CLanguage::IDX_FEE,&pbuf);
		//	str = CString(pbuf);//L"Fee";
		//	CLanguage::GetTextFromIndex(CLanguage::IDX_OK2,&pbuf);
		//	str1.Format(L"        %s  %s?", CString(buf1),CString(pbuf));
		//	str2= L"";
		//	ShowText(str, str1, str2);
		//	m_strSurcharge = CString(buf1);
		//	break;
		//}
		//if (CDataFile::Read(L"SURCHARGE", buf) && CString(buf) == L"Off")
		//{
		//	m_bGoBackFlag?GoBack():GoNext();
		//	break;
		//}
		//else
		//{
		//	m_bGoBackFlag = FALSE;
		//	TCHAR buf1[10] = L"";
		//	CDataFile::Read(L"SURCHARGEFEE",buf1);
		//	if ( !CheckAmount(CString(buf1)))
		//	{
		//		m_bGoBackFlag?GoBack():GoNext();
		//		break;
		//	}

		//	CLanguage::GetTextFromIndex(CLanguage::IDX_CANCEL,&pbuf);
		//	str.Format(L"  %s",CString(pbuf));
		//	m_Bt.SetValue(1, str, IDB_CANCEL);
		//	m_Bt.ShowWindow(SW_SHOW);
		//	CLanguage::GetTextFromIndex(CLanguage::IDX_OK2,&pbuf);
		//	str.Format(L"  %s",CString(pbuf));
		//	m_Bt1.SetValue(3, str, IDB_OK);
		//	m_Bt1.ShowWindow(SW_SHOW);
		//	CLanguage::GetTextFromIndex(CLanguage::IDX_FEE,&pbuf);
		//	str = CString(pbuf);//L"Fee";
		//	CLanguage::GetTextFromIndex(CLanguage::IDX_OK2,&pbuf);
		//	str1.Format(L"        %s  %s?", CString(buf1),CString(pbuf));
		//	str2= L"";
		//	ShowText(str, str1, str2);
		//	m_strSurcharge = CString(buf1);
		//	break;
		//}
	case DEBITTIPSELECT: //TipSelect
		GoNext();
		break;
	case DEBITTOTALCOMFIRM: // TotalComfirm
		GoNext();
		break;
	case DEBITSELECTACCOUNT: // SelectAccount
		GoNext();
		break;
//		m_bGoBackFlag = FALSE;
//		CLanguage::GetTextFromIndex(CLanguage::IDX_CHEQUE,&pbuf);
//		m_Bt.SetValue(1, CString(pbuf));
////		m_Bt.SetValue(1, L"  CHEQUE");
//		CLanguage::GetTextFromIndex(CLanguage::IDX_SAVING,&pbuf);
//		m_Bt1.SetValue(3, CString(pbuf));
////		m_Bt1.SetValue(3, L"  SAVING");
//		m_Bt.ShowWindow(SW_SHOW);
//		m_Bt1.ShowWindow(SW_SHOW);
//
//		CLanguage::GetTextFromIndex(CLanguage::IDX_SELECT_ACCOUNT,&pbuf);
//		str = CString(pbuf);//L"Select Account";
//		str1= L"";
//		str2= L"";
//		ShowText(str, str1, str2);
//			GoNext();
//		break;
	case DEBITONLINEPIN: //OnlinePin
		if (!EnterOnlinePin())
		{
			m_strErr1 = CLanguage::GetText(CLanguage::IDX_ENTER_ONLINE_PIN);
			PostMessage(ERROR_MESSAGE, 2, 1);
			strcpy(m_ErrCode,ERROR_ENTER_ONLINE_PIN);
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
		if (m_TRREC.ComStatus == ST_OK  &&
				m_TRREC.TranStatus == ST_APPROVED)
		{
			CLanguage::GetTextFromIndex(m_TRREC.bPartialAuth?CLanguage::IDX_PARTIAL_APPROVED:CLanguage::IDX_APPROVED,&pbuf);
			ShowPicture(1);
		}
		else
		{
			CLanguage::GetTextFromIndex(CLanguage::IDX_UNSUCCESSFUL,&pbuf);
			ShowPicture(2);
			if ( m_TRREC.ComStatus != ST_OK )
				strcpy(m_ErrCode,ERROR_CONDITION_CODE_NETWORK);//network error
			else
			{
				if ( m_TRREC.TranStatus != ST_APPROVED )
					strcpy(m_ErrCode,ERROR_CONDITION_CODE_DECLINE);//Host respond error
			}
		}

		str = CString(pbuf);
		m_strMsg2.m_nFormat = DT_LEFT;

		CLanguage::GetTextFromIndex(CLanguage::IDX_RETRIEVE_CARD,&pbuf);
		str1 = CString(pbuf);
		CLanguage::GetTextFromIndex(CLanguage::IDX_PASS_TO_CLERK,&pbuf2);
		str2 = CString(pbuf2);

		ShowText(str, str1, str2);
		sprintf(m_Customer_Text, "%s %s", pbuf, pbuf2);						//JC Mar 13/15 to save Customer Text
		CLanguage::SetCustomerFlag(FALSE);
		Print();
		break;
	case DEBITCUSTOMERCOPY: // for Customer copy of the receipt
		Sleep(2000);
		CLanguage::GetTextFromIndex(CLanguage::IDX_PRESS_OK_FOR,&pbuf);
		str = CString(pbuf);
		CLanguage::GetTextFromIndex(CLanguage::IDX_CUSTOMER_COPY,&pbuf);
		str1 = CString(pbuf);
		str2= L"";
		ShowText(str, str1, str2);
		SetTimer(1, 30000, NULL);
		break;
	case DEBITWAITCOPYEND: // waiting for Customer copy of the receipt ending...
		if (m_TRREC.ComStatus == ST_OK  &&
				m_TRREC.TranStatus == ST_APPROVED)
		{
			CLanguage::GetTextFromIndex(m_TRREC.bPartialAuth?CLanguage::IDX_PARTIAL_APPROVED:CLanguage::IDX_APPROVED,&pbuf);
			str = CString(pbuf);//L"APPROVED";
			CLanguage::GetTextFromIndex(CLanguage::IDX_AUTH_CODE,&pbuf2);
			str1.Format(L"%s : %s", CString(pbuf2),CString(m_TRREC.AuthCode));
			m_strMsg2.m_nFormat = DT_LEFT;
			CLanguage::GetTextFromIndex(CLanguage::IDX_INVO_NUMBER,&pbuf3);
			str2.Format(L"%s: %s", CString(pbuf3),CString(m_TRREC.InvoiceNo));
			ShowText( str, str1 ,str2);
			sprintf(m_Customer_Text, "%s %s %s %s %s", pbuf, pbuf2,m_TRREC.AuthCode, pbuf3, m_TRREC.InvoiceNo);					//JC Mar 13/15 to save Customer Text
			ShowPicture(1);
		}
		else
		{
			CLanguage::GetTextFromIndex(CLanguage::IDX_UNSUCCESSFUL,&pbuf);
			str = CString(pbuf);//L"Unsuccessful";
			CLanguage::GetTextFromIndex(CLanguage::IDX_DECLINED,&pbuf2);
			str1.Format(L"%s %s", CString(pbuf2),CString(m_TRREC.RespCode));
			str2 = L"";
			ShowText(str, str1,str2);
			sprintf(m_Customer_Text, "%s %s", pbuf, pbuf2);						//JC Mar 13/15 to save Customer Text
			ShowPicture(2);
			if ( m_TRREC.ComStatus != ST_OK )
				strcpy(m_ErrCode,ERROR_CONDITION_CODE_NETWORK);//network error
			else
			{
				if ( m_TRREC.TranStatus != ST_APPROVED )
					strcpy(m_ErrCode,ERROR_CONDITION_CODE_DECLINE);//Host respond error
			}
		}
		Print(2);
		break;
	case DEBITEND: // ending...
		if ( m_TRREC.EntryMode == ENTRY_MODE_CTLS && strcmp(m_TRREC.ISORespCode,"89") == 0)
		{
			m_strErr = L" ";
			m_strErr1 = CLanguage::GetText(CLanguage::IDX_PLEASE_INSERT);
			m_strErr2 = CLanguage::GetText(CLanguage::IDX_CHIP_CARD);
			ErrorExit(ENTERCARD_INPUT);
			break;
		}
		Sleep(2000);
		CDialog::OnCancel();
		break;
// == == == == == == == == == == == == == == == == == == == == == == == == == == == == == == == == == == 
	case MANUALENTRYY: //Manual Entry enter expiry
		CLanguage::GetTextFromIndex(CLanguage::IDX_ENTER_EXPIRY,&pbuf);
		str = CString(pbuf);
		CLanguage::GetTextFromIndex(CLanguage::IDX_DATE,&pbuf);
		str1.Format(L"%s           MMYY", CString(pbuf));
		str2= L"";
		ShowText(str, str1,str2);
		break;
	case MANUAL_IMPRINT_CARD:  //imprint card
		CLanguage::GetTextFromIndex(CLanguage::IDX_IMPRINT_CARD,&pbuf);
		str = CString(pbuf);
		CLanguage::GetTextFromIndex(CLanguage::IDX_PRESS_OK,&pbuf);
		str1 = CString(pbuf);
		str2= L"";
		ShowText(str, str1,str2);
		break;

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
	case CHIP_DEBITCASHBACK: //CASHBACK
		m_bCancelPrint = TRUE;
		GoNext();
		break;
		//if (m_TRREC.CardType == CARD_CREDIT)
		//{
		//	m_bGoBackFlag?GoBack():GoNext();
		//	return;
		//}

		//if (m_bReturnFlag)
		//{
		//	m_bGoBackFlag?GoBack():GoNext();
		//	break;
		//}

		//if (CDataFile::Read(L"CASHBACK", buf) && CString(buf) == L"Off")
		//{
		//	m_bGoBackFlag?GoBack():GoNext();
		//}
		//else
		//{
		//	m_bGoBackFlag = FALSE;
		//	CLanguage::GetTextFromIndex(CLanguage::IDX_ENTER_CASHBACK,&pbuf);
		//	str = CString(pbuf);
		//	CLanguage::GetTextFromIndex(CLanguage::IDX_AMOUNT,&pbuf);
		//	str1=  CString(pbuf);
		//	CString str2 = m_strCashback;//L"$0.00";
		//	ShowText(str, str1, str2);
		//}
		//break;
	case CHIP_DEBITSURCHARGEFEE: //surcharge fee
		GoNext();
		break;
		//if (m_TRREC.CardType == CARD_CREDIT)
		//{
		//	m_bGoBackFlag?GoBack():GoNext();
		//	return;
		//}

		//if (m_bReturnFlag)
		//{
		//	m_bGoBackFlag?GoBack():GoNext();
		//	break;
		//}

		//if ( CheckAmount(m_strCashback) && CDataFile::Read(L"CASHBACK", buf) && CString(buf) == L"On")
		//{
		//	m_bGoBackFlag = FALSE;
		//	TCHAR buf1[10] = L"";
		//	CDataFile::Read(L"CASHBACKFEE",buf1);
		//	if ( !CheckAmount(CString(buf1)))
		//	{
		//		m_bGoBackFlag?GoBack():GoNext();
		//		break;
		//	}

		//	CLanguage::GetTextFromIndex(CLanguage::IDX_CANCEL,&pbuf);
		//	str.Format(L"  %s",CString(pbuf));
		//	m_Bt.SetValue(1, str, IDB_CANCEL);
		//	m_Bt.ShowWindow(SW_SHOW);
		//	CLanguage::GetTextFromIndex(CLanguage::IDX_OK2,&pbuf);
		//	str.Format(L"  %s",CString(pbuf));
		//	m_Bt1.SetValue(3, str, IDB_OK);
		//	m_Bt1.ShowWindow(SW_SHOW);
		//	CLanguage::GetTextFromIndex(CLanguage::IDX_FEE,&pbuf);
		//	str = CString(pbuf);//L"Fee";
		//	CLanguage::GetTextFromIndex(CLanguage::IDX_OK2,&pbuf);
		//	str1.Format(L"        %s  %s?", CString(buf1),CString(pbuf));
		//	str2= L"";
		//	ShowText(str, str1, str2);
		//	m_strSurcharge = CString(buf1);
		//	break;
		//}

		//if ( CDataFile::Read(L"SURCHARGE", buf) && CString(buf) == L"Off")
		//{
		//	m_bGoBackFlag?GoBack():GoNext();
		//	break;
		//}
		//else
		//{
		//	m_bGoBackFlag = FALSE;
		//	TCHAR buf1[10] = L"";
		//	CDataFile::Read(L"SURCHARGEFEE",buf1);
		//	if ( !CheckAmount(CString(buf1)))
		//	{
		//		m_bGoBackFlag?GoBack():GoNext();
		//		break;
		//	}

		//	CLanguage::GetTextFromIndex(CLanguage::IDX_CANCEL,&pbuf);
		//	str.Format(L"  %s",CString(pbuf));
		//	m_Bt.SetValue(1, str, IDB_CANCEL);
		//	m_Bt.ShowWindow(SW_SHOW);
		//	CLanguage::GetTextFromIndex(CLanguage::IDX_OK2,&pbuf);
		//	str.Format(L"  %s",CString(pbuf));
		//	m_Bt1.SetValue(3, str, IDB_OK);
		//	m_Bt1.ShowWindow(SW_SHOW);
		//	CLanguage::GetTextFromIndex(CLanguage::IDX_FEE,&pbuf);
		//	str = CString(pbuf);//L"Fee";
		//	CLanguage::GetTextFromIndex(CLanguage::IDX_OK2,&pbuf);
		//	str1.Format(L"        %s  %s?", CString(buf1),CString(pbuf));
		//	str2= L"";

		//	ShowText(str, str1, str2);
		//	m_strSurcharge = CString(buf1);
		//	break;
		//}
	case CHIP_DEBITTIPSELECT: //TipSelect
		GoNext();
		break;
	case CHIP_DEBITTOTALCOMFIRM: // TotalComfirm
		if (m_bReturnFlag && m_TRREC.CardType == CARD_CREDIT)
		{
			m_bGoBackFlag?GoBack():GoNext();
			break;
		}
		m_bGoBackFlag = FALSE;
		DoConfirm(m_bReturnFlag,this);
		break;
	case CHIP_DEBITSELECTACCOUNT: // SelectAccount
		m_bCancelPrint = TRUE;

		if (m_TRREC.CardType == CARD_CREDIT)
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
			SetKeyReturn(KEY_ENTER);
			return;
		}

		m_bGoBackFlag = FALSE;
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
	case CHIP_DEBITENTERPIN: //EnterPin
		if (!EMVEnterPin())//Fix me later
		{
			m_strErr1 = L"EMVEnterPin() FAIL";
			PostMessage(ERROR_MESSAGE, 2, 1);
			strcpy(m_ErrCode,ERROR_EMV_ENTERPIN_FAIL);
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
				CLanguage::GetTextFromIndex(CLanguage::IDX_DO_NOT_REMOVE,&pbuf);
				str1 = CString(pbuf);
				CLanguage::GetTextFromIndex(CLanguage::IDX_CARD,&pbuf);
				str2=  CString(pbuf);
				ShowText(str, str1, str2);
				GoToLevel(DOTRANSACTION);
			}
		}
		break;
	case CHIP_PASSTOCLERK: //Pass to clerk
		GoToLevel(DOTRANSACTION);
		break;

	case CHIP_DEBITMERCHANTCOPY: // for merchant copy of the receipt
		if (m_TRREC.ComStatus == ST_OK  &&
				m_TRREC.TranStatus == ST_APPROVED)
		{
			CLanguage::GetTextFromIndex(m_TRREC.bPartialAuth?CLanguage::IDX_PARTIAL_APPROVED:CLanguage::IDX_APPROVED,&pbuf);
		}
		else
		{
			CLanguage::GetTextFromIndex(CLanguage::IDX_UNSUCCESSFUL,&pbuf);
			if ( m_TRREC.ComStatus != ST_OK )
				strcpy(m_ErrCode,ERROR_CONDITION_CODE_NETWORK);//network error
			else
			{
				if ( m_TRREC.TranStatus != ST_APPROVED )
					strcpy(m_ErrCode,ERROR_CONDITION_CODE_DECLINE);//Host respond error
			}
		}
		str = CString(pbuf);
		m_strMsg2.m_nFormat = DT_LEFT;
		str1 = L"";
		if(m_TRREC.EntryMode == ENTRY_MODE_CHIP)
		{
			CLanguage::GetTextFromIndex(CLanguage::IDX_REMOVE_CARD,&pbuf);
			str1 = CString(pbuf);
		}
		SetTimer(1,3000,NULL);
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
			CLanguage::GetTextFromIndex(m_TRREC.bPartialAuth?CLanguage::IDX_PARTIAL_APPROVED:CLanguage::IDX_APPROVED,&pbuf);
			str = CString(pbuf);//L"APPROVED";
			CLanguage::GetTextFromIndex(CLanguage::IDX_AUTH_CODE,&pbuf2);
			str1.Format(L"%s : %s", CString(pbuf2),CString(m_TRREC.AuthCode));
			m_strMsg2.m_nFormat = DT_LEFT;
			CLanguage::GetTextFromIndex(CLanguage::IDX_INVO_NUMBER,&pbuf3);
			str2.Format(L"%s: %s", CString(pbuf3),CString(m_TRREC.AuthCode));
			ShowText( str, str1 ,str2);
			sprintf(m_Customer_Text, "%s %s %s %s %s", pbuf, pbuf2, m_TRREC.AuthCode, pbuf3,m_TRREC.AuthCode);						//JC Mar 13/15 to save Customer Text
			ShowPicture(1);
		}
		else
		{
			CLanguage::GetTextFromIndex(CLanguage::IDX_UNSUCCESSFUL,&pbuf);
			str = CString(pbuf);//L"Unsuccessful";
			CLanguage::GetTextFromIndex(CLanguage::IDX_DECLINED,&pbuf2);
			str1.Format(L"%s %s", CString(pbuf2),CString(m_TRREC.RespCode));
			str2 = L"";
			ShowText(str, str1,str2);
			sprintf(m_Customer_Text, "%s %s", pbuf, pbuf2);						//JC Mar 13/15 to save Customer Text
			ShowPicture(2);
			if ( m_TRREC.ComStatus != ST_OK )
				strcpy(m_ErrCode,ERROR_CONDITION_CODE_NETWORK);//network error
			else
			{
				if ( m_TRREC.TranStatus != ST_APPROVED )
					strcpy(m_ErrCode,ERROR_CONDITION_CODE_DECLINE);//Host respond error
			}
		}
		Print(2);
		break;
	case CHIP_DEBITEND: // ending...
		Sleep(2000);
		if (GetScrStatus()!= SCR_CARD_ABSENT && m_TRREC.EntryMode == ENTRY_MODE_CHIP )
		{
			CLanguage::GetTextFromIndex(CLanguage::IDX_PLEASE,&pbuf);
			str = CString(pbuf);
			CLanguage::GetTextFromIndex(CLanguage::IDX_REMOVE_CARD,&pbuf);
			str1 = CString(pbuf);
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
	case CREDITSWIPEENTRYY://FRAUD CHECK
		GoNext();
		break;
	case CREDITSWIPETOCUSTOMER:
	case MANUAL_SWIPETOCUSTOMER:
		GoNext();
		break;
	case CREDITSWIPETIPSELECT: //TipSelect
	case MANUAL_SWIPETIPSELECT: //TipSelect
		GoNext();
		break;
	case CREDITSWIPETOTALCOMFIRM: // TotalComfirm
	case MANUAL_SWIPETOTALCOMFIRM: // TotalComfirm
		GoNext();
		break;
	case CREDITSWIPETOCLERK:  //Pass to clerk
	case MANUAL_SWIPETOCLERK:  //Pass to clerk
		GoNext();
		break;
	case CREDITMERCHANTCOPY: // for merchant copy of the receipt
		m_strMsg2.m_nFormat = DT_LEFT;
		if (m_TRREC.ComStatus == ST_OK  &&
				m_TRREC.TranStatus == ST_APPROVED)
		{
			CLanguage::GetTextFromIndex(m_TRREC.bPartialAuth?CLanguage::IDX_PARTIAL_APPROVED:CLanguage::IDX_APPROVED,&pbuf);
			str = CString(pbuf);//L"APPROVED";
			CLanguage::GetTextFromIndex(CLanguage::IDX_AUTH_CODE,&pbuf2);
			str1.Format(L"%s : %s", CString(pbuf2),CString(m_TRREC.AuthCode));
			m_strMsg2.m_nFormat = DT_LEFT;
			CLanguage::GetTextFromIndex(CLanguage::IDX_INVO_NUMBER,&pbuf3);
			str2.Format(L"%s: %s", CString(pbuf3),CString(m_TRREC.InvoiceNo));
			ShowText( str, str1 ,str2);
			sprintf(m_Customer_Text, "%s %s %s %s %s", pbuf, pbuf2, m_TRREC.AuthCode, pbuf3, m_TRREC.InvoiceNo);						//JC Mar 13/15 to save Customer Text
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

			if ( strcmp(m_ErrCode,ERROR_FORMAT_CC_DATA_WIRE) == 0)
				str2 = CString(m_Customer_Text);
			ShowText(str, str1,str2);
			sprintf(m_Customer_Text, "%s %s", pbuf, m_TRREC.HostRespText);					//JC Mar 13/15 to save Customer Text
			ShowPicture(2);
			if ( m_TRREC.ComStatus != ST_OK )
				strcpy(m_ErrCode,ERROR_CONDITION_CODE_NETWORK);//network error
			else
			{
				if ( m_TRREC.TranStatus != ST_APPROVED )
					strcpy(m_ErrCode,ERROR_CONDITION_CODE_DECLINE);//Host respond error
			}
		}
		SetTimer(3,100,NULL);
		break;
	case CREDITCUSTOMERCOPY: // for Customer copy of the receipt
		//Sleep(2000);
		CLanguage::GetTextFromIndex(CLanguage::IDX_PRESS_OK_FOR,&pbuf);
		str = CString(pbuf);
		CLanguage::GetTextFromIndex(CLanguage::IDX_CUSTOMER_COPY,&pbuf);
		str1 = CString(pbuf);
		str2 = L"";
		ShowText(str, str1, str2);
		SetTimer(1, 1000, NULL);
		break;
	case CREDITWAITCOPYEND: // waiting for Customer copy of the receipt ending...
		if (m_TRREC.ComStatus == ST_OK  &&
				m_TRREC.TranStatus == ST_APPROVED)
		{
			CLanguage::GetTextFromIndex(m_TRREC.bPartialAuth?CLanguage::IDX_PARTIAL_APPROVED:CLanguage::IDX_APPROVED,&pbuf);
			str = CString(pbuf);//L"APPROVED";
			CLanguage::GetTextFromIndex(CLanguage::IDX_AUTH_CODE,&pbuf2);
			str1.Format(L"%s : %s", CString(pbuf2),CString(m_TRREC.AuthCode));
			m_strMsg2.m_nFormat = DT_LEFT;
			CLanguage::GetTextFromIndex(CLanguage::IDX_INVO_NUMBER,&pbuf3);
			str2.Format(L"%s: %s", CString(pbuf3),CString(m_TRREC.InvoiceNo));
			ShowText( str, str1 ,str2);
			sprintf(m_Customer_Text, "%s %s %s %s %s", pbuf, pbuf2, m_TRREC.AuthCode, pbuf3, m_TRREC.InvoiceNo);						//JC Mar 13/15 to save Customer Text
			ShowPicture(1);
		}
		else
		{
			CLanguage::GetTextFromIndex(CLanguage::IDX_UNSUCCESSFUL,&pbuf);
			str = CString(pbuf);//L"Unsuccessful";
			CLanguage::GetTextFromIndex(CLanguage::IDX_DECLINED,&pbuf2);
			str1.Format(L"%s %s", CString(pbuf2),CString(m_TRREC.RespCode));
			str2 = L"";
			ShowText(str, str1,str2);
			sprintf(m_Customer_Text, "%s %s", pbuf, pbuf2);						//JC Mar 13/15 to save Customer Text
			ShowPicture(2);
			if ( m_TRREC.ComStatus != ST_OK )
				strcpy(m_ErrCode,ERROR_CONDITION_CODE_NETWORK);//network error
			else
			{
				if ( m_TRREC.TranStatus != ST_APPROVED )
					strcpy(m_ErrCode,ERROR_CONDITION_CODE_DECLINE);//Host respond error
			}
		}
		Print(2);
		break;
	case CREDITEND: // ending...
		//Sleep(3000);
		if (m_TRREC.EntryMode == ENTRY_MODE_CTLS_MSR)
		{
			if (strcmp(m_TRREC.ISORespCode,"01") == 0 || strcmp(m_TRREC.ISORespCode,"05") == 0)
			{
				m_strErr1 = CLanguage::GetText(CLanguage::IDX_INSERT_SWIPE_CARD);
				m_strErr2 = L"";
				ErrorExit(ENTERCARD_INPUT);
				break;
			}
		}
		else if( m_TRREC.EntryMode == ENTRY_MODE_CTLS && CEMV::IsMasterCardAID(m_TRREC.EmvData.AID))
		{
			if (m_TRREC.TranStatus != ST_APPROVED && (strcmp(m_TRREC.ISORespCode,"05") == 0))
			{
				m_strErr1 = CLanguage::GetText(CLanguage::IDX_INSERT_SWIPE_CARD);
				m_strErr2 = L"";
				ErrorExit(ENTERCARD_INPUT);
				break;
			}
		}
		else if( m_TRREC.EntryMode == ENTRY_MODE_CHIP)
		{
			GoToLevel(CHIP_DEBITEND);
			break;
		}
		else if( m_TRREC.EntryMode == ENTRY_MODE_CTLS && strcmp(m_TRREC.ISORespCode,"89") == 0)
		{
			m_strErr = L" ";
			m_strErr1 = CLanguage::GetText(CLanguage::IDX_PLEASE_INSERT);
			m_strErr2 = CLanguage::GetText(CLanguage::IDX_CHIP_CARD);
			ErrorExit(ENTERCARD_INPUT);
			break;
		}
		//Sleep(2000);
		CDialog::OnCancel();
		break;

	case DOTRANSACTION:
		DoTransaction();
		break;
	case CANCELENTRY:
		m_bCancelFlag = TRUE;
		CLanguage::GetTextFromIndex(CLanguage::IDX_TRANSACTION,&pbuf);
		str = CString(pbuf);
		CLanguage::GetTextFromIndex(CLanguage::IDX_CANCELLED,&pbuf2);
		str1 = CString(pbuf2);
		str2 = L"";	
		sprintf(m_Customer_Text, "%s %s", pbuf, pbuf2);
		ShowText(str, str1, str2);	
		SetTimer(1, 5000, NULL);
		strcpy(m_ErrCode,ERROR_CONDITION_CODE_CANCEL);
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
		if ( m_bCancelPrint )
		{
			Print(2);
			Sleep(200);
		}
		CDialog::OnCancel();
		break;

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
			CLanguage::GetTextFromIndex(CLanguage::IDX_REMOVE_CARD,&pbuf);
			str1 = CString(pbuf);
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
	case EMV_START: //EMV start
		m_strMsg2.m_nFormat = DT_LEFT;
		str = CLanguage::GetText(CLanguage::IDX_EMV_PLEASE_WAIT);

		CLanguage::GetTextFromIndex(CLanguage::IDX_DO_NOT_REMOVE,&pbuf);
		str1 = CString(pbuf);
		CLanguage::GetTextFromIndex(CLanguage::IDX_CARD,&pbuf);
		str2=  CString(pbuf);
		ShowText(str, str1, str2);
		StartEMV();
		m_iLevel = DISPLAY_WINDOW;
		break;
	}
}

//----------------------------------------------------------------------------
//!	\brief	chekc display window's button number and move to right position
//----------------------------------------------------------------------------
void CSaleDlg::SetDisplayButton()
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

//-----------------------------------------------------------------------------
//!	\CDialog class virtual function
//-----------------------------------------------------------------------------
void CSaleDlg::OnCancel()
{
	Beep1();
	MSG msg; 
	 while(::PeekMessage(&msg,NULL,0,0,PM_REMOVE))
	 {
		 if (msg.message==WM_PAINT || msg.message== ERROR_MESSAGE)
			::DispatchMessage(&msg);
	}
	switch(m_iLevel)
	{
	case PASSWORD_INPUT: //Password Input
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
		if ( m_TRREC.EntryMode == ENTRY_MODE_CTLS && strcmp(m_TRREC.ISORespCode,"89") == 0)
		{
			m_strErr = L" ";
			m_strErr1 = CLanguage::GetText(CLanguage::IDX_PLEASE_INSERT);
			m_strErr2 = CLanguage::GetText(CLanguage::IDX_CHIP_CARD);
			ErrorExit(ENTERCARD_INPUT);
			break;
		}
		CDialog::OnCancel();
		break;
	case DEBITWAITCOPYEND: // waiting for Customer copy of the receipt ending...
		break;
	case DEBITEND: // ending...
		break;
	case CHIPENTRYY:
		break;
	case SELECT_LANGUAGE: //select language
		GoToLevel(CANCELENTRY);
		break;
	case CHIP_DEBITENTERPIN: //EnterPin
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
		if (m_TRREC.EntryMode == ENTRY_MODE_CTLS_MSR)
		{
			if (strcmp(m_TRREC.ISORespCode,"01") == 0 || strcmp(m_TRREC.ISORespCode,"05") == 0)
			{
				KillTimer(1);
				m_strErr1 = CLanguage::GetText(CLanguage::IDX_INSERT_SWIPE_CARD);
				m_strErr2 = L"";
				ErrorExit(ENTERCARD_INPUT);
				break;
			}
		}
		else if( m_TRREC.EntryMode == ENTRY_MODE_CTLS && CEMV::IsMasterCardAID(m_TRREC.EmvData.AID))
		{
			if (m_TRREC.TranStatus != ST_APPROVED && (strcmp(m_TRREC.ISORespCode,"05") == 0))
			{
				m_strErr1 = CLanguage::GetText(CLanguage::IDX_INSERT_SWIPE_CARD);
				m_strErr2 = L"";
				ErrorExit(ENTERCARD_INPUT);
				break;
			}
		}

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

	case MANUALENTRYY: //?¡ãFraud Check
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
		break;
// == == == == == == == Display Window == == == == == 
	case DISPLAY_WINDOW:
		if (CDisplay::GetMask() & MASK_CANCEL)
			CDisplay::SetKeyReturn(KEY_CANCEL);
		break;
	case EMV_SELECT_LANGUAGE: //EMV Select language
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
void CSaleDlg::OnOK()
{
	Beep1();
	MSG msg; 
	 while(::PeekMessage(&msg,NULL,0,0,PM_REMOVE))
	 {
		 if (msg.message==WM_PAINT || msg.message== ERROR_MESSAGE)
			::DispatchMessage(&msg);
	}

	CString str,str1,str2;
	char* pbuf;
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
				CLanguage::GetTextFromIndex(CLanguage::IDX_CANNOT_SWIPE_CHIP,&pbuf);
			else if( m_TRREC.CardError == CARD_ERR_INVALID_MODE)
				CLanguage::GetTextFromIndex(CLanguage::IDX_INVALID_CARD_NUMBER,&pbuf);
			else
				CLanguage::GetTextFromIndex(CLanguage::IDX_CARD_NOT_SUPPORT,&pbuf);
			m_strErr1 = CString(pbuf);//L"CARD NOT SUPPORT";
			m_strErr2 = CLanguage::GetText(CLanguage::IDX_NO_SUPPORTED);
			SetErrorCode(ERROR_MSR_OPEN_FAIL, CLanguage::IDX_ERROR, CLanguage::IDX_NO_SUPPORTED);			//JC April 7/15 return a code
			ErrorExit(ENTERCARD_INPUT);
			return;
		}

		m_iLevel = MANUALENTRYY-1;
		break;
	case DEBITCARDENTRY:
		KillTimer(1);
		break;
	case DEBITCASHBACK: //CASHBACK
	case CHIP_DEBITCASHBACK: //CASHBACK
		{
			int iMax = 99999999;
			TCHAR buf[10]={L""};
			if ( CDataFile::Read(L"CASHBACKMAX",buf))
			{
				char temp[20];
				PutAmount(temp,CString(buf));
				iMax = atoi(temp);
			}

			char buf1[20];
			PutAmount(buf1,m_strMsg2.m_szCaption);

			if( atoi(buf1) > iMax)
			{
				Beep();
				m_strErr = CLanguage::GetText(CLanguage::IDX_ENTER_CASHBACK);
				m_strErr1 = CLanguage::GetText(CLanguage::IDX_LIMIT_EXCEEDED);
				m_strErr2 = L"";
				ShowAlert(m_iLevel);
				return;
			}
		}
		m_strCashback = m_strMsg2.m_szCaption;
		m_strCashback.Remove(' ');
		break;
	case DEBITSURCHARGEFEE: //surcharge fee
		break;
	case DEBITTIPSELECT: //TipSelect
		break;
	case DEBITTOTALCOMFIRM: // TotalComfirm
		break;
	case DEBITONLINEPIN: //OnlinePin
		break;
	case DEBITMERCHANTCOPY: // for merchant copy of the receipt
		return;
	case DEBITCUSTOMERCOPY: // for Customer copy of the receipt
		KillTimer(1);
		break;
	case DEBITWAITCOPYEND: // waiting for Customer copy of the receipt ending...
		return;
	case CHIPENTRYY:
		KillTimer(1);
		m_iLevel = DISPLAY_WINDOW;
		return;

//		break;
	case SELECT_LANGUAGE: //select language
		Beep();
		return;
	case CHIP_DEBITENTERPIN: //EnterPin
		break;
	case CHIP_PASSTOCLERK: //pass to clerk
		KillTimer(1);
		GoToLevel(DOTRANSACTION);
		return;
	case CHIP_DEBITMERCHANTCOPY: // for merchant copy of the receipt
		KillTimer(1);
		GoToLevel(DEBITEND);
		return;
	case CHIP_DEBITCUSTOMERCOPY: // for Customer copy of the receipt
		KillTimer(1);
		break;
	case CHIP_DEBITWAITCOPYEND: // waiting for Customer copy of the receipt ending...
		return;
	case CREDITSWIPEENTRYY: //Fraud Check
		str = m_strMsg2.m_szCaption;

		if( CheckFraud(str))
		{
			GoToLevel(CREDITSWIPETOCUSTOMER);
		}
		return;
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
		KillTimer(1);
		GoToLevel(CREDITEND);
		return;
	case CREDITCUSTOMERCOPY: // for Customer copy of the receipt
		KillTimer(1);
		break;
	case CANCELCUSTOMERCOPY: // for Customer copy of the receipt
		KillTimer(1);
		Print(2);
		break;
	case CREDITWAITCOPYEND: // waiting for Customer copy of the receipt ending...
		return;
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
		return;
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
//		return;
		KillTimer(1);
		break;
	case CANCELTOCLERK:
		KillTimer(1);
		break;
	case ERRORENTRY:
		KillTimer(1);
		GoToLevel ( CANCELEMVCARDBEREMOVED );
		return;
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
	case ALERTENTRY:
		KillTimer(1);
		m_iNextLevel = 0;
		return;

// == == == == == == == Display Window == == == == == 
	case DISPLAY_WINDOW:
		if (CDisplay::GetMask() & MASK_OK)
			CDisplay::SetKeyReturn(KEY_ENTER);
		return;
	case EMV_SELECT_LANGUAGE: //EMV Select language
	case EMV_SELECT_ACCOUNT: //EMV select account
		m_strMsg2.m_nFormat = DT_LEFT;
		CLanguage::GetTextFromIndex(CLanguage::IDX_PLEASE_WAIT,&pbuf);
		str = CString(pbuf);//L"Please Wait...";
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
	case CHIP_DEBITSURCHARGEFEE: //surcharge fee
	case CHIP_DEBITTIPSELECT: //TipSelect
	case CHIP_DEBITTOTALCOMFIRM: // TotalComfirm
		break;
	case CHIP_DEBITSELECTACCOUNT: // SelectAccount
		Beep();
		return;
	default:
		return;
	}
	GoNext();
	SetFocus();
}

//----------------------------------------------------------------------------
//!	\brief	handle key input function for any GUI's, include correction key(nChar == 8)
//----------------------------------------------------------------------------
void CSaleDlg::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	Beep1();
	MSG msg; 
	 while(::PeekMessage(&msg,NULL,0,0,PM_REMOVE))
	 {
		 if (msg.message==WM_PAINT || msg.message== ERROR_MESSAGE)
			::DispatchMessage(&msg);
	}
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
	case SELECT_LANGUAGE: //select language
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
// == == == == == == == == == == == == = End == == == == == == == == == == == == == == == = 
	}

	SetFocus();
}

//----------------------------------------------------------------------------
//!	\brief	handle button press function for any GUI's, 
//----------------------------------------------------------------------------
void CSaleDlg::OnBnClickedButton(UINT nID)
{
	CString str,str1,str2;
	char* pbuf;
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
			OnCancel();
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

//----------------------------------------------------------------------------
//!	\brief	handle event when Host connection is done 
//----------------------------------------------------------------------------
void CSaleDlg::Done()
{
	AfterHost();
	if (m_TRREC.ComStatus == ST_OK &&m_TRREC.TranStatus == ST_APPROVED)
		strcpy(m_ErrCode,ERROR_CONDITION_CODE_OK);

	if ( m_RapidConnect->pDataWire->DataWireError > 0 )
	{
		strcpy(m_ErrCode,ERROR_FORMAT_CC_DATA_WIRE);
		sprintf(m_Customer_Text,"DataWire Error: %02d", m_RapidConnect->pDataWire->DataWireError);
	}
	else
	{
		CString str = m_RapidConnect->pDataWire->StatusCode;
		if (str != L"")
		{
			strcpy(m_ErrCode,ERROR_FORMAT_CC_DATA_WIRE);
			char *p = alloc_tchar_to_char(str.GetBuffer(str.GetLength()));
			strcpy(m_Customer_Text,p);
			free(p);
			str.ReleaseBuffer();
		}
	}

	GoToLevel(CREDITMERCHANTCOPY);
}

//----------------------------------------------------------------------------
//!	\brief	Message entry function for any GUI's, 
//----------------------------------------------------------------------------
LRESULT CSaleDlg::DefWindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	CString strTemp; 
	DWORD len = 0;
   DWORD decode = 0;
   char* pbuf;

	if (wParam == 1 && m_bCloseDevice)
	{
		if (m_iLevel != ENTERCARD_INPUT)
			return NULL;
		BYTE buf[2048] = {0};
		DWORD len;
		switch(message)
		{
			case RFID_EVENT_READY:
				m_RFID.GetData();
				m_iKeyStatus =  KEY_CTLS;
				m_TRREC.EntryMode = ENTRY_MODE_CTLS;
				m_RFID.GetData((BYTE*)buf, &len);
				ProcessCtlsTLV((USHORT)len, (BYTE*)buf);
				//if (m_TRREC.CtlsData.C2[0] == 0x05)		// EMV
				//{
				//	m_TRREC.bEmvTransaction = TRUE;
				//	m_TRREC.bEmvCtlsTransaction = TRUE;
				//	m_TRREC.bCtlsTransaction = TRUE;
				//	m_TRREC.bEmvCardRead = TRUE;
				//}
				//else if (m_TRREC.CtlsData.C2[0] == 0x04)	// MSR
				if (m_TRREC.CtlsData.C2[0] == 0x04 || m_TRREC.CtlsData.C2[0] == 0x05) //EMV or MSR process as MSR
				{
					m_TRREC.EntryMode = ENTRY_MODE_CTLS_MSR;
					m_TRREC.bEmvTransaction = FALSE;
					m_TRREC.bEmvCtlsTransaction = FALSE;
					m_TRREC.bCtlsTransaction = TRUE;
					m_TRREC.bEmvCardRead = FALSE;
				}
				else if (m_TRREC.CtlsData.C2[0] == 0x06)	// Terminated
				{
					// Get error code from C3
					TRACE(L"RFID ERROR\n");
					return 0;
				}
				else if (m_TRREC.CtlsData.C2[0] == 0x02)	// Offline decline
				{
					m_bCancelPrint = TRUE;
					if ( m_TRREC.CardType != CARD_DEBIT )
					{
						if (CEMV::IsInteracRID(m_TRREC.EmvData.AID))	// Flash, set to debit card
						{
							m_TRREC.CardType = CARD_DEBIT;
						}
						else
							m_TRREC.CardType = CARD_CREDIT;
					}

					if ( m_TRREC.CardType == CARD_DEBIT )
							m_TRREC.AcctType = ACCT_DEFAULT;
					m_strErr1 = CLanguage::GetCharText(CLanguage::IDX_DECLINED_BY_CARD);
					m_strErr2 = L"";
					strcpy(m_ErrCode,ERROR_DECLINED_BY_CARD);
					ErrorExit();
					return 0;
				}
				else if (m_TRREC.CtlsData.C2[0] == 0x00 && m_TRREC.TranCode == TRAN_REFUND)	// Offline approved
				{	// for PPC_MCD_03_03_01 to work
//					m_TRREC.EntryMode = ENTRY_MODE_CTLS_MSR;
					m_TRREC.bEmvTransaction = TRUE;
					m_TRREC.bEmvCtlsTransaction = FALSE;
					m_TRREC.bCtlsTransaction = TRUE;
					m_TRREC.bEmvCardRead = FALSE;
				}
				else
				{
					CLanguage::GetTextFromIndex(CLanguage::IDX_CARD_NOT_SUPPORT,&pbuf);
					m_strErr1 = CString(pbuf);//L"CARD NOT SUPPORT";
					m_strErr2 = CLanguage::GetText(CLanguage::IDX_NO_SUPPORTED);
					SetErrorCode(ERROR_MSR_OPEN_FAIL, CLanguage::IDX_ERROR, CLanguage::IDX_NO_SUPPORTED);			//JC April 7/15 return a code
					ErrorExit(ENTERCARD_INPUT);
					return 0;
				}
				
				if ( m_TRREC.CardType == NO_CARD)
					m_TRREC.CardType = CARD_CREDIT;	// FIXME: will change when have FLASH support
				m_TRREC.CardError = ProcessSwipedCard(m_TRREC.Track2);

				if (m_TRREC.CardError != CARD_NO_ERROR)
				{
					CloseDevice();

					if ( m_TRREC.CardError == CARD_ERR_CANNOT_SWIPE_CHIP)
					{
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
					SetErrorCode(ERROR_MSR_OPEN_FAIL, CLanguage::IDX_ERROR, CLanguage::IDX_NO_SUPPORTED);			//JC April 7/15 return a code
					ErrorExit(ENTERCARD_INPUT);
					return 0;
				}
				
				if ( m_TRREC.CardType != CARD_DEBIT )
				{
					if (CEMV::IsInteracRID(m_TRREC.EmvData.AID))	// Flash, set to debit card
					{
						m_TRREC.CardType = CARD_DEBIT;
					}
					else
						m_TRREC.CardType = CARD_CREDIT;
				}

				if ( m_TRREC.CardType == CARD_DEBIT )
						m_TRREC.AcctType = ACCT_DEFAULT;

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
				m_RFID.GetData();
				m_RFID.GetData((BYTE*)buf, &len);
				ProcessCtlsTLV((USHORT)len, (BYTE*)buf);
				m_SCR.Close();
				CloseDevice();
				
				if (m_TRREC.CtlsData.C2[0] == 0x06)	// Terminated
				{
					if ( m_TRREC.CtlsData.C3[0] == 2 )
					{
						m_strErr = CLanguage::GetText(CLanguage::IDX_ERROR);
						m_strErr1 = CLanguage::GetText(CLanguage::IDX_TAP_FAILED_PLEASE);
						m_strErr2 = CLanguage::GetText(CLanguage::IDX_INSERT_SWIPE_CARD);
						ErrorExit(ENTERCARD_INPUT);
						return 0;
					}
					else if ( m_TRREC.CtlsData.C3[0] == 0x8D )
					{
						m_strErr = L"";
						m_strErr1 = CLanguage::GetText(CLanguage::IDX_PLEASE);
						m_strErr2 = CLanguage::GetText(CLanguage::IDX_SEE_PHONE);
						strcpy(m_ErrCode,ERROR_PLEASE_SEE_PHONE);
						ErrorExit();
						return 0;
					}
					else
						m_strErr1 =  CLanguage::GetText(CLanguage::IDX_TAP_FAILED_PLEASE);
				}
				else
					m_strErr1 =  CLanguage::GetText(CLanguage::IDX_TAP_FAILED_PLEASE);

				m_strErr2 = CLanguage::GetText(CLanguage::IDX_EMV_USE_CHIP_READER); 
				ErrorExit(ENTERCARD_INPUT);
				return 0;
			default:
				break;	
		}	
	}
	else if (wParam == 0)//  && m_bCloseDevice )
	{
		switch(message)
		{
			case MSR_EVENT_DEV_OFFLINE :
				m_strErr1 = CLanguage::GetText(CLanguage::IDX_MSR_OPEN_FAIL);
				strcpy(m_ErrCode,ERROR_MSR_OPEN_FAIL);
				ErrorExit();
				return NULL;
//			case MSR_EVENT_CT_STATUS :
			case MSR_EVENT_READ_ERROR :
				TRACE(L"MSR_EVENT_READ_ERROR \n");
				return NULL;
			case MSR_EVENT_DATA_READY :
				TRACE(L"MSR_EVENT_DATA_READY \n");
				//Sleep(100);
				
				//////Sometimes multiple swipes come in
				if (m_bTranInProgress)
					return NULL;

				//if (IsCardInSlot())
				//	return NULL;

				m_iWaitMSR = 0;

				//Sleep(500);
				m_MSR.MsrGetTrackData();
				m_MSR.GetMSRData(2, (BYTE*)m_TRREC.Track2, &len, &decode);

				CloseMSRDevice();
				if (len == 0)
					return NULL;

				m_bTranInProgress = TRUE;
				m_bInsertCard = FALSE;
				m_TRREC.EntryMode = ENTRY_MODE_SWIPED;
				m_TRREC.bEmvCardRead = FALSE;
				m_TRREC.bEmvTransaction = FALSE;
				m_TRREC.CardError = ProcessSwipedCard(m_TRREC.Track2);

				if ( m_TRREC.bChipCard )
					m_TRREC.bFallback = TRUE;

				if (m_TRREC.CardError != CARD_NO_ERROR && m_TRREC.CardError != CARD_ERR_CANNOT_SWIPE_CHIP)
				{
					m_SCR.Close();
					CloseDevice(FALSE);
/*
					if ( m_TRREC.CardError == CARD_ERR_CANNOT_SWIPE_CHIP)
					{
	//					CLanguage::GetTextFromIndex(CLanguage::IDX_CANNOT_SWIPE_CHIP,&pbuf);
						m_strErr = L" ";
						m_strErr1 = CLanguage::GetText(CLanguage::IDX_PLEASE_INSERT);
						m_strErr2 = CLanguage::GetText(CLanguage::IDX_CHIP_CARD);
					}
					else 
*/					if( m_TRREC.CardError == CARD_ERR_INVALID_MODE)
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
					SetErrorCode(ERROR_MSR_OPEN_FAIL, CLanguage::IDX_ERROR, CLanguage::IDX_NO_SUPPORTED);			//JC April 7/15 return a code
					ErrorExit(ENTERCARD_INPUT);
					return 0;
				}

				m_iKeyStatus =  KEY_MSR;

				if (m_TRREC.CardType == CARD_DEBIT)
					m_iLevel = DEBITSELECTACCOUNT;
				else if (m_TRREC.CardType == CARD_CREDIT)
					m_iLevel = DOTRANSACTION;

				GoToLevel(m_iLevel);
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

		case 8: //Can't send back to ECR
			DoECRReversal();
//			ShowPicture();
			m_strErr1 =CLanguage::GetText(CLanguage::IDX_ERROR);
			m_strErr2 = L"ECR " + CLanguage::GetText(CLanguage::IDX_SOCKET_CONNECT);
//			m_TRREC.CardError = CARD_ERR_CANNOT_SENT_ECR ;
			ErrorExit();
			break;
		case 9: //Send back to ECR OK
			SetTimer(1,6000,NULL);

/*			UpdateLogfile();
			Print();
			if ( m_bNoPrint )
			{
				m_iLevel = m_iECRLevel;
				SetTimer(6,2000,NULL);
			}
*/
			break;

		}
	}
	return CDialog::DefWindowProc(message, wParam, lParam);
}

//----------------------------------------------------------------------------
//!	\brief	call CFinan:: DoTransaction for connect host
//----------------------------------------------------------------------------
void CSaleDlg::DoTransaction()
{
	if (!IsNetWorkConnect())
	{
		m_strErr1 = CLanguage::GetText(CLanguage::IDX_NO_NETWORK);
		strcpy(m_ErrCode,ERROR_CONDITION_CODE_NETWORK);//network error
		ErrorExit();
		return;
	}

	m_strMsg.SetCaption(CLanguage::GetText(CLanguage::IDX_PLEASE_WAIT));
	m_strMsg1.SetCaption(L"");
	m_strMsg2.SetCaption(L"");
	CFinan::DoTransaction();
}

//-----------------------------------------------------------------------------
//!	\Set Error code 
//!	\param	code - error code string buffer
//!	\param	m1 - error1 text index
//!	\param	m2 - error2 text index
//-----------------------------------------------------------------------------
void CSaleDlg::SetErrorCode(char* code, int m1, int m2)											//JC April 7/15 pass back Cutomer Text to API
{
    char *pbuf;
	char *pbuf2;

	strcpy(m_ErrCode,code);
	if (m2 == 0)																				//JC April 7/15 pass back Cutomer Text to API
	{	
		CLanguage::GetTextFromIndex(m1,&pbuf);
		sprintf(m_Customer_Text, "%s", pbuf);									//JC April 7/15 pass back Cutomer Text to API
	}
	else																						//JC April 7/15 pass back Cutomer Text to API
	{
		CLanguage::GetTextFromIndex(m1,&pbuf);
		CLanguage::GetTextFromIndex(m2,&pbuf2);
		sprintf(m_Customer_Text, "%s %s", pbuf, pbuf2);		//JC April 7/15 pass back Cutomer Text to API
	}

}
//----------------------------------------------------------------------------
//!	\brief	Error display and exit
//----------------------------------------------------------------------------
void CSaleDlg::ErrorExit()
{
	m_strMsg.m_nFormat = DT_LEFT;
	m_strMsg1.m_nFormat = DT_LEFT;
	m_strMsg2.m_nFormat = DT_LEFT;
	m_strMsg.SetColor(RGB(255, 0, 0), RGB(255, 255, 255));
	m_strMsg1.SetColor(RGB(255, 0, 0), RGB(255, 255, 255));
	m_strMsg2.SetColor(RGB(255, 0, 0), RGB(255, 255, 255));
	m_strMsg.SetCaption(L"");
	m_strMsg1.SetCaption(m_strErr1);
	m_strMsg2.SetCaption(m_strErr2);
	m_Bt.ShowWindow(SW_HIDE);
	m_Bt1.ShowWindow(SW_HIDE);

	if (m_TRREC.bFallback)
	{
		OpenMSRDevice();

		char ReceiptNo[SZ_RECEIPT_TEXT+1];
		char Amount[SZ_AMOUNT+1];
		char InvoiceNo[SZ_INVOICE_NO+1];
		strcpy(ReceiptNo,m_TRREC.ReceiptNo);
		strcpy(Amount,m_TRREC.Amount);
		strcpy(InvoiceNo,m_TRREC.InvoiceNo);
		USHORT TranCode = m_TRREC.TranCode;
		m_TRREC.Init();
		m_TRREC.TranCode = TranCode;
		strcpy(m_TRREC.ReceiptNo,ReceiptNo);
		strcpy(m_TRREC.Amount,Amount);
		strcpy(m_TRREC.InvoiceNo,InvoiceNo);
		m_TRREC.bFallback = TRUE;
		m_iWaitMSR = 0;
		m_iLevel = ALERTENTRY;
		SetTimer(1,3000,NULL);
		m_iNextLevel = 0;
	}
	else
	{
		m_iLevel = ERRORENTRY;
		SetTimer(1, 10000, NULL);
		CloseDevice(FALSE);
	}
}
//----------------------------------------------------------------------------
//!	\brief	Error display and go to level
// param level is level index
//----------------------------------------------------------------------------
void CSaleDlg::ErrorExit(int level)
{
	m_bInsertCard = FALSE;
	m_strMsg.m_nFormat = DT_LEFT;
	m_strMsg1.m_nFormat = DT_LEFT;
	m_strMsg2.m_nFormat = DT_LEFT;
	m_strMsg.SetColor(RGB(255, 0, 0),RGB( 255,255,255));
	m_strMsg1.SetColor(RGB(255, 0, 0),RGB( 255,255,255));
	m_strMsg2.SetColor(RGB(255, 0, 0),RGB( 255,255,255));

	if(m_strErr == L"")
		m_strErr = CLanguage::GetText(CLanguage::IDX_ERROR);
	m_strMsg.SetCaption(m_strErr);
	m_strMsg1.SetCaption(m_strErr1);
	m_strMsg2.SetCaption(m_strErr2);
	m_iLevel = ERRORENTRY1;
	SetTimer(1,5000,NULL);

	m_iNextLevel = level;
	CloseDevice(FALSE);
}

//----------------------------------------------------------------------------
//!	\brief	Alert display and go to level
// param level is level index
//----------------------------------------------------------------------------
void CSaleDlg::ShowAlert(int level)
{
	m_strMsg.m_nFormat = DT_LEFT;
	m_strMsg1.m_nFormat = DT_LEFT;
	m_strMsg2.m_nFormat = DT_LEFT;
	m_strMsg.SetColor(RGB(255, 0, 0),RGB( 255,255,255));
	m_strMsg1.SetColor(RGB(255, 0, 0),RGB( 255,255,255));
	m_strMsg2.SetColor(RGB(255, 0, 0),RGB( 255,255,255));
	m_strMsg.SetCaption(m_strErr);
	m_strMsg1.SetCaption(m_strErr1);
	m_strMsg2.SetCaption(m_strErr2);
	m_iLevel = ALERTENTRY;
	SetTimer(1,3000,NULL);
	m_iNextLevel = level;
}	

//----------------------------------------------------------------------------
//!	\brief	window close
//----------------------------------------------------------------------------
void CSaleDlg::OnDestroy()
{
	KillTimer(1);
	CBaseDlg::OnDestroy();
	Sleep(100);
}

//----------------------------------------------------------------------------
//!	\brief	go back last GUI
//----------------------------------------------------------------------------
void CSaleDlg::GoBack()
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
void CSaleDlg::GoNext()
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
void CSaleDlg::GoToLevel(int level)
{
	m_iLevel = level;
	PostMessage(ERROR_MESSAGE, 2, 2);
}

//----------------------------------------------------------------------------
//!	\brief	check RRL limit
//----------------------------------------------------------------------------
BOOL CSaleDlg::IsRRLLimit()
{
	if (!(m_TRREC.ComStatus == ST_OK  &&
			m_TRREC.TranStatus == ST_APPROVED))
		return TRUE;

	TRAN_TranDataRec* pTRANDATA = &m_TRREC;
	if (pTRANDATA->TranCode != TRAN_PURCHASE)
		return TRUE;
	if (pTRANDATA->EntryMode != ENTRY_MODE_CTLS)
		return TRUE;
	if (strcmp(pTRANDATA->ServType,"P0") != 0)
		return TRUE;

	CRecordFile ctlsDataFile;
	AP_FILE_HANDLER fhCtlsData;
	if (!ctlsDataFile.Open(CTLS_DATA_FILE, &fhCtlsData))
		return TRUE;

	sCtlsParams CtlsData, *pCtlsData;
	pCtlsData = &CtlsData;
	
	USHORT numOfRecords;
	ctlsDataFile.GetNumberOfRecords(&fhCtlsData, &numOfRecords);

	USHORT len;

	int CvmAmount = 0 ;
	for (int n = 0; n < numOfRecords; n++)
	{
		ctlsDataFile.ReadRecord(&fhCtlsData, n, (BYTE*)pCtlsData, &len);

		if( strcmp(pCtlsData->serviceType,"P0") == 0)
		{
			CvmAmount = atoi(pCtlsData->interactRRL);
			break;
		}
	}
	ctlsDataFile.CloseFile(&fhCtlsData);

	if ( CvmAmount == 0)
		return TRUE;

	return CvmAmount <= atoi(pTRANDATA->TotalAmount);
}

//----------------------------------------------------------------------------
//!	\brief	Send back ECR message back
//----------------------------------------------------------------------------
void  CSaleDlg::SendECRBack()
{
//	if (!m_bECRTxn)
//		return;
	if ( m_bSendECRBackFlag )
		return;

	int len = 0;
	m_bSendECRBackFlag = TRUE;
	
    BYTE buf[256];
    memset(buf,0,256);

	GetTxnResult((TXN_Result*)buf);
	len = sizeof(TXN_Result);
	memcpy(&buf[len],(void*)m_ErrCode,SZ_ERRCODE_TEXT);
	len += SZ_ERRCODE_TEXT;
	memcpy(&buf[len],(void*)m_TRREC.RespCode,SZ_RESPONSE_CODE); 
	len += SZ_RESPONSE_CODE;
	memcpy(&buf[len],(void*)m_Customer_Text,SZ_CUSTOMER_TEXT); 
	len += strlen(m_Customer_Text);

	COPYDATASTRUCT cpd;
    cpd.dwData = 0;
    cpd.cbData = len + 1;
    cpd.lpData = (void*)buf;

    m_pParentWnd->SendMessage(WM_COPYDATA,(WPARAM)m_hWnd,(LPARAM)&cpd);

}
//-----------------------------------------------------------------------------
//!	\CDialog class virtual function
//-----------------------------------------------------------------------------
void CSaleDlg::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	CDialog::OnKeyDown(nChar, nRepCnt, nFlags);

	TRACE(L"Down nchar = %d \n",nChar);

	switch( nChar )
	{
	case 115:
		if ( m_Bt1.IsWindowVisible())
			OnBnClickedButton(IDC_BUTTON2);
		break;
	case 119: 
		if ( m_Bt.IsWindowVisible())
			OnBnClickedButton(IDC_BUTTON1);	
		break;
	}
}
