#include "StdAfx.h"
#include "SaioBase.h"
#include "Finan.h"
#include "..\\EMV\\PinInput.h"
#include "..\\Utils\\StrUtil.h"
#include "FinUtil.h"
#include "..\\Utils\\string.h"
#include "..\\DEFS\\constant.h"
#include "..\\UI\\DoConfirmDlg.h"
#include "..\\utils\\util.h"
#include "..\\Utils\\TotalUtil.h"
#include "..\\EMV\\ByteStream.h"
#include "..\\EMV\\ErrCode.h"
#include "..\\EMV\\EMVTags.h"
#include "..\\EMV\\EMVLog.h"
#include "..\\UI\\Display.h"
#include "..\\EMV\\Common.h"
#include "..\\data\\AppData.h"
#include "..\\Utils\\CeFileFind.h"
#include "..\\data\\RecordFile.h"

extern EmvLog logger;
extern BOOL g_AppSelected;
extern BOOL g_AppBlocked;
extern BOOL g_LanguageSelected;
extern BYTE g_bWithPrimaryCandidates;
extern BOOL g_IsPosTerminal;
extern BOOL g_AppConfirmed;
extern BYTE g_bSetPrimaryKey;
extern BOOL g_GotoSelectApp;
extern BOOL g_PrimaryBlocked;
extern BOOL g_NoPrimary;
extern BOOL g_NotCheckRevData;
extern int g_TotalApplication;

extern int  g_iCandidates;
extern BOOL g_PinTimeOut;
extern CString g_strAppData;
#define PIN_OK 0

#define NUMBER_OF_ERRORS sizeof(errorTable)/sizeof(stErrTbl)
#define ENTERCARD_INPUT			5
#define CHIP_DEBITCASHBACK		64
#define CHIP_DEBITSELECTACCOUNT	68


const char EnglishLanguageString[] = "en";
const char FrenchLanguageString[] = "fr";

TXNCONDITION TXN;

void PinEntryBox()
{
	CPinInput PinDlg;
	PinDlg.m_bCheckCardRemoved = FALSE;
	PinDlg.DoModal();
}

//-----------------------------------------------------------------------------
//!	\CFinan structure function
//-----------------------------------------------------------------------------
CFinan::CFinan(void)
{
	m_KeyEntry = FALSE;
	m_bGoBackFlag = FALSE;
	m_bEMV = FALSE;
	m_bCancelPrint = FALSE;
	m_bCancelFlag = FALSE;
	m_bCancelInCustomer = FALSE;
	m_strPassword = L"";
	m_IncorrectTimes =0;

	m_iEMVStart = FALSE;
	m_bTraining = FALSE;

	m_bFraud = TRUE;

	m_pLogTRREC = NULL;

	m_iNextLevel = 0;

	EMV_PIN_MODE = FALSE;

	m_bCloseDevice = FALSE;
	m_bCloseMSRDevice = FALSE;

	declinedOffline = FALSE;

	CAppData::bSignatureRequired = FALSE;
	CAppData::bEMVError = FALSE;
	CAppData::SupportCTLS = TRUE;
	m_bHavePrint = FALSE;
	m_RapidConnect = new CRapidConnect(m_MsgWnd);
}

//-----------------------------------------------------------------------------
//!	\CFinan structure function
//-----------------------------------------------------------------------------
CFinan::~CFinan(void)
{
	delete m_RapidConnect;
	CheckAutoSet();
	if ( m_TRREC.bEmvTransaction )
		CloseScr();

	CAppData::bEMVError = FALSE;
}

//----------------------------------------------------------------------------
//!	\brief	make alarm beep voice
//!	\return	
//!			 
//----------------------------------------------------------------------------
void CFinan::Beep()
{
	SaioBeeper(1, 3000, 500, 0, 100);
}	

//----------------------------------------------------------------------------
//!	\brief	Open MSR, SCR and RFID device for entercard
//!	\return	TRUE - field processed
//!			FALSE - failed to process 
//----------------------------------------------------------------------------
BOOL CFinan::OpenDevice()
{
	if (m_bCloseDevice)
	{
		CloseDevice();
		Sleep(100);
	}
	CLanguage::SetLanguage();

	if (!m_MSR.Open(m_MsgWnd))
		return FALSE;
CDataFile::CheckMemory(L"m_MSR.Open");

	m_SCR.m_bPowerOff = FALSE;
	if (!m_SCR.Open(m_MsgWnd))
		return FALSE;
CDataFile::CheckMemory(L"m_SCR.Open");

	char amount[10];
	PutAmount(amount, m_strAmount);
	int iAmount = atoi(amount);
	PutAmountWithDot(amount, m_strAmount);

	if (!m_RFID.Open(m_MsgWnd, amount, m_TRREC.TranCode,iAmount))  
		return FALSE;
CDataFile::CheckMemory(L"m_RFID.Open");

    m_bCloseDevice = TRUE;
	return TRUE;
}
//----------------------------------------------------------------------------
//!	\brief	close MSR, SCR and RFID device for entercard
//!	\return	
//----------------------------------------------------------------------------
void CFinan::CloseDevice(BOOL flag)
{
	if (!m_bCloseDevice)
		return;
	m_bCloseDevice = FALSE;

DWORD dwOldTime = GetTickCount();

	m_MSR.Cancel();
DWORD dwTimeElapsed = GetTickCount() - dwOldTime;
TRACE(L"MSRCancel() time: %d \n",dwTimeElapsed);
	m_SCR.Cancel();
//	m_SCR.Close();
dwTimeElapsed = GetTickCount() - dwOldTime;
TRACE(L"SCRCancel() time: %d \n",dwTimeElapsed);

	if (flag)
		m_RFID.Reset();
	m_RFID.Cancel();
dwTimeElapsed = GetTickCount() - dwOldTime;
TRACE(L"RFIDCancel() time: %d \n",dwTimeElapsed);
}

//----------------------------------------------------------------------------
//!	\brief	Open MSR, SCR device for entercard,becasue Void transaction don't support RFID card
//!	\return	TRUE - field processed
//!			FALSE - failed to process 
//----------------------------------------------------------------------------
BOOL CFinan::OpenVoidDevice()
{
	CLanguage::SetLanguage();
	if (!m_MSR.Open(m_MsgWnd))
		return FALSE;

	if (!m_SCR.Open(m_MsgWnd))
		return FALSE;

	m_bCloseDevice = TRUE;
	return TRUE;
}
//----------------------------------------------------------------------------
//!	\brief	Open MSR,device for entercard,becasue Void transaction don't support RFID card
//!	\return	TRUE - field processed
//!			FALSE - failed to process 
//----------------------------------------------------------------------------
BOOL CFinan::OpenMSRDevice()
{
	CLanguage::SetLanguage();
	if (m_bCloseMSRDevice)
		CloseMSRDevice();
	if (!m_MSR.Open(m_MsgWnd))
		return FALSE;
	m_bCloseMSRDevice = TRUE;
	Sleep(200);
	return TRUE;
}
//----------------------------------------------------------------------------
//!	\brief	close MSR, SCR device for entercard
//!	\return	TRUE - field processed
//!			FALSE - failed to process 
//----------------------------------------------------------------------------
void CFinan::CloseVoidDevice()
{
	m_bCloseDevice = FALSE;
	m_MSR.Cancel();
	m_SCR.Cancel();
//	m_SCR.Close();
}
//----------------------------------------------------------------------------
//!	\brief	close MSR device for entercard
//!	\return	TRUE - field processed
//!			FALSE - failed to process 
//----------------------------------------------------------------------------
void CFinan::CloseMSRDevice()
{
	m_bCloseMSRDevice = FALSE;
	m_MSR.Cancel();
}

//----------------------------------------------------------------------------
//!	\brief	Enter online Pin with XAC's SDK
//!	\return	TRUE - field processed
//!			FALSE - failed to process 
//----------------------------------------------------------------------------
BOOL CFinan::EnterOnlinePin()
{
	DWORD dwRtn;
	BYTE bEpb[8] = {0};
	BYTE bKSN[10] = {0};
	BYTE bPinLen;


	dwRtn = PinOpen();
	if (dwRtn != PIN_OK)
		return FALSE;

	dwRtn = PinBypassEnable(PIN_BYPASS_DISABLE);

	dwRtn = PinEntry(PIN_DUKPT_KEY0,						// key id
					 4,										// min_pinlen
					 12,									// max_pinlen
					 m_TRREC.Account,						// pan
					 NULL,							// working key
					 NULL,						// working key length in bytes
					 (DWORD) m_CustDialogTimer,				// timeout
					 TRUE,									// beep
					 PinEntryBox,							// callback
					 &bPinLen,								// out: pin length
					 bEpb,									// out: epb
					 bKSN);									// out: ksn

	PinClose();
	switch (dwRtn)
	{
	case PIN_OK:
		STR_UnpackData(m_TRREC.PINBlock, bEpb, 8);
		STR_UnpackData(m_TRREC.PINKSN, bKSN, 10);
		m_TRREC.bRequestedPIN = TRUE;
		break;
	case PIN_TIMEOUT:
		return FALSE;
	case PIN_CANCEL:
		return FALSE;
	case PIN_DEV_NOT_OPEN:
	case PIN_BAD_PIN_LENGTH:
	case PIN_KEY_NO_LOADED:
	case PIN_DUKPT_RUNOUT:
	case PIN_DEVIO_FAILED:
	default:
		return FALSE;
	}

	return TRUE;
}

//-----------------------------------------------------------------------------
//!	\Process Swiped Card from Track2 data
//!	\param	pTrack2 - point to Track 2 data
//! RETURNS:       CARD_NO_ERROR    = Validate.                               
//!                other  = not Validate. 									  
//-----------------------------------------------------------------------------
int CFinan::ProcessSwipedCard(char *pTrack2)
{
	int cardError;
	BOOL result;
	int cardTypeFromTrack2;

	int iLanguage;
	result = CFinUtil::GetCardInfo(pTrack2, 
								 m_TRREC.Account, m_TRREC.ExpDate,
								 &m_TRREC.bChipCard, &cardTypeFromTrack2, 
								 &iLanguage, m_TRREC.ServCode);

	if ( cardTypeFromTrack2 == CARD_CREDIT )
	{
		if ( CDataFile::IsDebitOnlyTerminal())
		{
			cardError = CARD_ERR_CARD_NOT_SUPPORT;
			return cardError;
		}
	}

	if ( !m_TRREC.bChipCard )
	{
		m_TRREC.CustLang = iLanguage;
		m_TRREC.bFallback = FALSE;
	}
	CLanguage::SetLanguage(m_TRREC.CustLang);

	if (!result)
	{
		cardError = CARD_ERR_CARD_PROBLEM;
	}
	else
	{	// m_TRREC.bEmvCardRead is set right before calling ProcessSwipedCard
		// to indicate this is an EMV or MSR card.
		if (m_TRREC.bEmvCardRead)
		{
			char temp[SZ_APP_IDENTIFIER+1];
			if(strlen(m_TRREC.AID) == 0)
			{
				strcpy(temp,m_TRREC.EmvData.AID);
			}
			else
				strcpy(temp,m_TRREC.AID);

			if (CFinUtil::GetServTypeFromAID(temp, m_TRREC.ServType))
			{
				if( strcmp(m_TRREC.ServType,"P0" ) != 0)
					cardTypeFromTrack2 = CARD_CREDIT;
				else
					cardTypeFromTrack2 = CARD_DEBIT;
				cardError = CFinUtil::ValidateCard(m_TRREC.Account, m_TRREC.ServType, &m_TRREC.binRecord, cardTypeFromTrack2);
			}
			else
			{
				cardError = CARD_ERR_CARD_NOT_SUPPORT;
			}
		}
		else	// not a chip card
		{
			// return a valid bin record
			cardError = CFinUtil::ValidateCard(m_TRREC.Account, m_TRREC.ServType, &m_TRREC.binRecord,cardTypeFromTrack2);

			//For US Market there are Debit Cards which do not identify with service codes we would identify as debit. 
			// These will fall under P0
			if (cardError == CARD_NO_ERROR )
				if (m_TRREC.ServType[0] == 'P')
					cardTypeFromTrack2 = CARD_DEBIT;

/*			if (cardError == CARD_NO_ERROR)
			{
				if (!m_TRREC.bFallback && m_TRREC.EntryMode == ENTRY_MODE_SWIPED && m_TRREC.bChipCard)
					cardError = CARD_ERR_CANNOT_SWIPE_CHIP;
			}
*/		}
	}

	if (cardError == CARD_NO_ERROR) // if card ok, save card data
	{
		m_TRREC.CardType = cardTypeFromTrack2;
		if ((m_TRREC.CardType == CARD_DEBIT) &&
			(m_TRREC.TranCode == TRAN_FORCE_POST ||
			m_TRREC.TranCode == TRAN_ADVICE))
		{
			cardError = CARD_ERR_CARD_NOT_SUPPORT;
		}
	}
	if (m_TRREC.CardType == CARD_DEBIT && m_TRREC.AcctType == ACCT_CREDIT)
		m_TRREC.AcctType = ACCT_CHEQUE;
	if (m_TRREC.CardType == CARD_CREDIT)
		m_TRREC.AcctType = ACCT_CREDIT;

	return cardError;
}

//----------------------------------------------------------------------------
//!	\brief	Amount Input and copy it to char* target
//! \param	target is get date from param str
//----------------------------------------------------------------------------
void CFinan::PutAmount(char* target, CString str)
{
	memset(target,0,SZ_AMOUNT+1);
	str.Remove('$');
	str.Remove(' ');
	str.Remove('.');
	str.Remove(',');
	str.TrimLeft('0');
	if (str.GetLength() == 0)
		return;
	PutParam(target,str);
}

//----------------------------------------------------------------------------
//!	\brief	Amount Input and copy it to char* target
//! \param	target is get date from param str
//----------------------------------------------------------------------------
void CFinan::PutAmountWithDot(char* target, CString str)
{
	memset(target, 0, SZ_AMOUNT+1);
	str.Remove('$');
	str.Remove(' ');
	str.Remove(',');
	str.TrimLeft('0');
	if (str.GetLength() == 0)
		return;
	PutParam(target,str);
}

//----------------------------------------------------------------------------
//!	\brief	Amount Input and copy it to char* target
//! \param	target is get date from param str
//----------------------------------------------------------------------------
void CFinan::PutParam(char* target, CString str)
{
	char *p = alloc_tchar_to_char(str.GetBuffer(str.GetLength()));
	strcpy(target,p);
	free(p);
	str.ReleaseBuffer();
}

//----------------------------------------------------------------------------
//!	\brief	Deal Amount char* to CString
//! \param	*p is input char*
// return CString 
//----------------------------------------------------------------------------
CString CFinan::DealAmountStr(char* p)
{
	CString str(p);
	str.Remove('$');
	str.Remove(' ');
	str.Remove('.');
	str.Remove(',');
	str.TrimLeft('0');
	int len = str.GetLength();

	len = str.GetLength();

	CString str1;
	if (len > 5)
	{
		int begin = (len-2)%3;
		if (begin != 0)
		{
			str1 = str.Mid(0,begin);
			str1.AppendChar(',');
		}

		while(len - begin > 5)
		{
			str1 += str.Mid(begin,3);
			str1.AppendChar(',');
			begin += 3;
		}
		str1 += str.Mid(begin,3);
		str1.AppendChar('.');
		str1 += str.Right(2);
	}
	else if (len <= 2)
	{
		str1 = L"0.";
		while(str1.GetLength()+len != 4 )
			str1 += L"0";
		str1 += str;
	}
	else
	{
		str1 = str.Mid(0,len-2);
		str1.AppendChar('.');
		str1 += str.Right(2);
	}
	return str1;

}

//----------------------------------------------------------------------------
//!	\brief	Deal clerk ID char and add to CString
//! \param	nChar is input char +  CString source
// return CString 
//----------------------------------------------------------------------------
CString CFinan::DealClerkID(UINT nChar,CString source)
{
	CString str;
	str = source;
	int len = str.GetLength();
	if (nChar == 8)
	{
		if (len>0)
			str = str.Mid(0,len-1);
		else
			GoBack();
//			Beep();
	}
	else
		str.AppendChar(nChar);

	len = str.GetLength();
	if (len > SZ_CLERKID_NO)
	{
		Beep();
		return source;
	}
	return str;
}

//----------------------------------------------------------------------------
//!	\brief	Deal Approval Code char and add to CString
//! \param	nChar is input char +  CString source
// return CString 
//----------------------------------------------------------------------------
CString CFinan::DealApprovalCode(UINT nChar,CString source)
{
	CString str;
	str = source;
	int len = str.GetLength();
	if (nChar == 8)
	{
		if (len>0)
			str = str.Mid(0,len-1);
		else
			GoBack();
	}
	else
		str.AppendChar(nChar);

	len = str.GetLength();
	if (len > SZ_APPROVAL_CODE)
	{
		Beep();
		return source;
	}
	return str;
}

//----------------------------------------------------------------------------
//!	\brief	Deal Invoice char and add to CString
//! \param	nChar is input char +  CString source
// return CString 
//----------------------------------------------------------------------------
CString CFinan::DealInvoice(UINT nChar,CString source)
{
	if (!(nChar>='0' && nChar<='9' || nChar==8))
		return source;

	CString str;
	str = source;
	int len = str.GetLength();
	if (nChar == 8)
	{
		if (len>0)
			str = str.Mid(0,len-1);
		else
			GoBack();
	}
	else
		str.AppendChar(nChar);

	len = str.GetLength();
	if (len > SZ_INVOICE_NO)
	{
		Beep();
		return source;
	}
	return str;
}

//----------------------------------------------------------------------------
//!	\brief	Deal Fraud char and add to CString
//! \param	nChar is input char +  CString source
// return CString 
//----------------------------------------------------------------------------
CString CFinan::DealFraud(UINT nChar,CString source)
{
	if (!(nChar>='0' && nChar<='9' || nChar==8))
		return source;

	CString str;
	str = source;
	int len = str.GetLength();
	if (nChar == 8)
	{
		if (len>0)
			str = str.Mid(0,len-1);
		else
			Beep();
	}
	else
		str.AppendChar(nChar);

	len = str.GetLength();
	if (len > SZ_FRAUD_NO)
	{
		Beep();
		return source;
	}
	return str;
}

//----------------------------------------------------------------------------
//!	\brief	Deal Receipt char and add to CString
//! \param	nChar is input char +  CString source
// return CString 
//----------------------------------------------------------------------------
CString CFinan::DealReceipt(UINT nChar,CString source)
{
	CString str = source;
	int len = str.GetLength();
	if (nChar == 8)
	{
		if (len>0)
			str = str.Mid(0,len-1);
		else
			GoBack();
	}
	else
		str.AppendChar(nChar);

	len = str.GetLength();
	if (len > SZ_RECEIPT_TEXT)
	{
		Beep();
		return source;
	}
	return str;
}

//----------------------------------------------------------------------------
//!	\brief	Deal Key expiry char and add to CString
//! \param	nChar is input char +  CString source
// return CString 
//----------------------------------------------------------------------------
CString CFinan::KeyExpiry(UINT nChar,CString source)
{
	if (!(nChar>='0' && nChar<='9' || nChar==8))
		return source;

	CString str = source;
	int len = str.GetLength();
	if (nChar == 8)
	{
		if (len>0)
			str = str.Mid(0,len-1);
		else
			Beep();
	}
	else
		str.AppendChar(nChar);

	len = str.GetLength();
	if (len > SZ_EXPIRY_DATE)
	{
		Beep();
		return source;
	}
	return str;
}	

//----------------------------------------------------------------------------
//!	\brief	Deal Key account char and add to CString
//! \param	nChar is input char +  CString source
// return CString 
//----------------------------------------------------------------------------
CString CFinan::KeyAccount(UINT nChar,CString source)
{
	TCHAR buf[10]={L""};
	if(CDataFile::Read(L"MANUAL ENTRY",buf) && CString(buf) == L"Off")
	{
		return source;
	}

	if ( CDataFile::IsDebitOnlyTerminal())
		return source;

	if (!(nChar>='0' && nChar<='9' || nChar==8))
		return source;

	if(!m_KeyEntry)
	{
		if( nChar ==8 )
		{
			Beep();
			return source;
		}
		else
		{
			m_KeyEntry = TRUE;
			source = L"";
//			m_RFID.Reset();
			CloseDevice();
		}
	}

	CString str = L"";//source;

	for(int i = 0;i<source.GetLength();i++)
	{
		if( isdigit(source.GetAt(i) ))
			str += source.GetAt(i);
	}
	int len = str.GetLength();
	if (nChar == 8)
	{
		if (len>0)
			str = str.Mid(0,len-1);
		else
			Beep();
	}
	else
		str.AppendChar(nChar);

	len = str.GetLength();
	if (len > SZ_ACCOUNT)
	{
		Beep();
		return source;
	}
	return str;
}

//----------------------------------------------------------------------------
//!	\brief	Deal password char and add to CString
//! \param	nChar is input char +  CString source
// return CString 
//----------------------------------------------------------------------------
CString CFinan::DealPassword(UINT nChar,CString source)
{
	if (!(nChar>='0' && nChar<='9' || nChar==8))
	{
		CString str = source;
		for(int i=0;i<str.GetLength();i++)
			str.SetAt(i,'*');
		return str;
	}
	CString str = source;
	int len = str.GetLength();
	if (nChar == 8)
	{
		if (len>0)
			str = str.Mid(0,len-1);
		else
			Beep();
	}
	else
		str.AppendChar(nChar);

	len = str.GetLength();
	if (len > SZ_PASSWORD_TEXT)
	{
		Beep();
		str = source;
	}
	m_strPassword = str;

	for(int i=0;i<str.GetLength();i++)
		str.SetAt(i,'*');
	return str;

}
//----------------------------------------------------------------------------
//!	\brief	Deal device password char and add to CString
//! \param	nChar is input char +  CString source
// return CString 
//----------------------------------------------------------------------------
CString CFinan::DealDevicePassword(UINT nChar,CString source)
{
	if (!(nChar>='0' && nChar<='9' || nChar==8))
	{
		CString str = source;
		for(int i=0;i<str.GetLength();i++)
			str.SetAt(i,'*');
		return str;
	}
	CString str = source;
	int len = str.GetLength();
	if (nChar == 8)
	{
		if (len>0)
			str = str.Mid(0,len-1);
		else
			Beep();
	}
	else
		str.AppendChar(nChar);

	len = str.GetLength();
	if (len > SZ_PASSWORD_TEXT)//SZ_DEVICE_PASSWORD_TEXT)
	{
		Beep();
		str = source;
	}
	m_strPassword = str;

	for(int i=0;i<str.GetLength();i++)
		str.SetAt(i,'*');
	return str;

}

//-----------------------------------------------------------------------------
//!	\check amount string format
//!	\param	source - amount string
//! RETURNS:       TRUE    = format OK.                               
//!                FALSE  = amount string format erro. 									  
//-----------------------------------------------------------------------------
BOOL CFinan::CheckAmount(CString source)
{
	CString str=source;
	str = source;
	str.Remove('$');
	str.Remove(' ');
	str.Remove('.');
	str.Remove(',');
	str.TrimLeft('0');

	int len = str.GetLength();
	if (len == 0 )
		return FALSE;

	for (int i = 0;i< len;i++)
	{
		if(str.GetAt(i)<'0' || str.GetAt(i)>'9')
			return FALSE;
	}
	return TRUE;
}
//----------------------------------------------------------------------------
//!	\brief	Deal amount char and add to CString
//! \param	nChar is input char +  CString source
// return CString 
//----------------------------------------------------------------------------
CString CFinan::DealAmount(UINT nChar,CString source)
{
	if (!(nChar>='0' && nChar<='9' || nChar==8))
		return source;

	CString str, str1;
	str = source;
	str.Remove('$');
	str.Remove(' ');
	str.Remove('.');
	str.Remove(',');
	str.TrimLeft('0');
	int len = str.GetLength();
	if (nChar == 8)
	{
		if (len>0)
			str = str.Mid(0,len-1);
		else
			GoBack();
//			Beep();
	}
	else
		str.AppendChar(nChar);

	len = str.GetLength();
	if (len > SZ_AMOUNT)
	{
		Beep();
		return source;
	}

	if (len > 5)
	{
		int begin = (len-2)%3;
		if (begin != 0)
		{
			str1 = str.Mid(0,begin);
			str1.AppendChar(',');
		}

		while(len - begin > 5)
		{
			str1 += str.Mid(begin,3);
			str1.AppendChar(',');
			begin += 3;
		}
		str1 += str.Mid(begin,3);
		str1.AppendChar('.');
		str1 += str.Right(2);
	}
	else if (len <= 2)
	{
		str1 = L"0.";
		while(str1.GetLength()+len != 4 )
			str1 += L"0";
		str1 += str;
	}
	else
	{
		str1 = str.Mid(0,len-2);
		str1.AppendChar('.');
		str1 += str.Right(2);
	}
	str = L"$"+str1;
	return str;

}

//----------------------------------------------------------------------------
//!	\brief	Get Invoice number from FID_CFG_INVOICE and save it to m_TRREC
//----------------------------------------------------------------------------
void CFinan::GetInvoice()
{
	return;
	int invoice = 1;
	DWORD size = sizeof(int);
	if (CDataFile::Read((int)FID_CFG_INVOICE, (BYTE *)&invoice, &size))
		invoice++;

	if(invoice > 9999999)
		invoice = 1;
	CDataFile::Save(FID_CFG_INVOICE, (BYTE*) &invoice, sizeof(int));
	sprintf(m_TRREC.InvoiceNo, "%07d", invoice);
}

//----------------------------------------------------------------------------
//!	\brief	prepare data and create new thread for host connect
//----------------------------------------------------------------------------
void CFinan::DoTransaction()
{
	CString str;
	if (strlen(m_TRREC.Amount) == 0)
		PutAmount(m_TRREC.Amount, m_strAmount);

	if (strlen(m_TRREC.TotalAmount) == 0)
		strcpy(m_TRREC.TotalAmount, m_TRREC.Amount);

	m_TRREC.HostTranCode = m_TRREC.TranCode;

	if (strlen(m_TRREC.InvoiceNo) == 0)
		GetInvoice();

	TCHAR buf[10]={L""};
	if (CDataFile::Read(L"TRAINING", buf))
	{
		if (CString(buf) == L"On")
			m_bTraining = TRUE;
	}
	
	//m_HOST.m_hWnd = m_MsgWnd;
	m_RapidConnect->m_hWnd = m_MsgWnd;
	HANDLE hThread = CreateThread(NULL, 0,ThreadProc, (PVOID)this, 0, NULL);
	CloseHandle(hThread);
}

//----------------------------------------------------------------------------
//!	\brief	new thread entry
//----------------------------------------------------------------------------
DWORD WINAPI CFinan::ThreadProc(LPVOID lpArg)
{
	CFinan* dlg = reinterpret_cast<CFinan*>(lpArg);
	CDataFile::CheckMemory(L"m_HOST.Start");

	try
	{
		if(dlg->m_bEMV)
		{
			CardLock();
			dlg->DoEMV();
			dlg->m_bEMV = FALSE;
			CardUnlock();
		}
		else
		{
			//dlg->m_HOST.Start(&dlg->m_TRREC, &dlg->m_CONFIG);
			dlg->m_RapidConnect->Start(&dlg->m_TRREC, &dlg->m_CONFIG);
		}
	}
	catch(...)
	{
	}
CDataFile::CheckMemory(L"m_HOST.End");
	
	return 0;
}

//----------------------------------------------------------------------------
//!	\brief	show amount confirm window
//----------------------------------------------------------------------------
BOOL CFinan::DoConfirm(BOOL Flag,CWnd* pParent)
{
	CDoConfirmDlg dlg(pParent);
	int index = 1;
	BOOL flag = FALSE;
	PutAmount(m_TRREC.Amount,m_strAmount);

	if(Flag)
		dlg.SetString(index++, CLanguage::GetText(CLanguage::IDX_RECEIPT_RETURN),DealAmountStr(m_TRREC.Amount));
	else
		dlg.SetString(index++, CLanguage::GetText(CLanguage::IDX_RECEIPT_SALE),DealAmountStr(m_TRREC.Amount));

	if(strlen(m_TRREC.TipAmount)>0)
	{
		dlg.SetString(index++, CLanguage::GetText(CLanguage::IDX_RECEIPT_TIP),DealAmountStr(m_TRREC.TipAmount));
		flag = TRUE;
	}

	PutAmount(m_TRREC.CashbackAmount,m_strCashback);
	if(strlen(m_TRREC.CashbackAmount)>0)
	{
		dlg.SetString(index++, CLanguage::GetText(CLanguage::IDX_CASHBACK_MENU),DealAmountStr(m_TRREC.CashbackAmount));
		flag = TRUE;

		TCHAR buf[10] = L"";			
		if ( CDataFile::Read(L"CASHBACKFEE",buf))
		{
			if ( CheckAmount(CString(buf)))
			{
				PutAmount(m_TRREC.CashbackFee,CString(buf));
				dlg.SetString(index++, CLanguage::GetText(CLanguage::IDX_FEE),DealAmountStr(m_TRREC.CashbackFee));
			}
		}
	}
	else
	{
		PutAmount(m_TRREC.SurchargeAmount, m_strSurcharge);
		if(strlen(m_TRREC.SurchargeAmount)>0 )
		{
			dlg.SetString(index++, CLanguage::GetText(CLanguage::IDX_FEE),DealAmountStr(m_TRREC.SurchargeAmount));
			flag = TRUE;
		}
	}

	int iTotal = 0;
	if(flag)
	{
		iTotal = atoi(m_TRREC.Amount)
			+atoi(m_TRREC.TipAmount)
			+atoi(m_TRREC.CashbackAmount)
			+atoi(m_TRREC.CashbackFee)
			+atoi(m_TRREC.SurchargeAmount);
		sprintf(m_TRREC.TotalAmount,"%d",iTotal);
		dlg.SetString(index++, CLanguage::GetText(CLanguage::IDX_RECEIPT_TOTAL),DealAmountStr(m_TRREC.TotalAmount));
	}

	dlg.SetString(6,CLanguage::GetText(CLanguage::IDX_PRESS_OK_CONFIRM));
	int iReturn = dlg.DoModal();
	if (iReturn == IDOK)
	{
		if( iTotal > 9999999)
		{
			m_strErr1 = CLanguage::GetText(CLanguage::IDX_AMOUNT_ABOVE);
			m_strErr2 = CLanguage::GetText(CLanguage::IDX_MAXIMUM);
			SetErrorCode(ERROR_AMOUNT_ABOVE_MAXIMUM, CLanguage::IDX_AMOUNT_ABOVE, CLanguage::IDX_MAXIMUM);
			ErrorExit();
			return FALSE;
		}
		GoNext();
	}
	else if (iReturn == IDCANCEL)
	{
		GoToLevel(CANCELENTRY);
	}
	return TRUE;
}

//-----------------------------------------------------------------------------
//!	\Process Check Auto Set
//-----------------------------------------------------------------------------
void CFinan::CheckAutoSet()
{
	USHORT flag = 0;
	DWORD size;
	CDataFile::Read((int)FID_TERM_AUTOSET_FLAG, (BYTE *)&flag, &size);

	if ( flag == 0) // no clear batch
		return;

	{
		CString strTemp[4];
		BOOL bTraining = FALSE;
		strTemp[0].Format(L"%s%s",CString(TRANSACTION_LOG_FILE),bTraining?L"TRAIN":L"");
		strTemp[1].Format(L"%s%s",CString(TOTALS_FILE),bTraining?L"TRAIN":L"");
		strTemp[2].Format(L"%s%s",CString(BATCH_LOG_FILE),bTraining?L"TRAIN":L"");
		strTemp[3].Format(L"%s",bTraining?L"":CString(HOST_TOTALS_FILE));

		for(int i=0; i<= 3;i++)
		{
			if(strTemp[i].GetLength() == 0)
				continue;

			char *FileName = alloc_tchar_to_char(strTemp[i].GetBuffer(strTemp[i].GetLength()));

			CCeFileFind temp;
			if (!temp.FindFile(FileName))
				continue;
			CRecordFile file;
			file.Delete(FileName);
			Sleep(100);
			free(FileName);
			strTemp[i].ReleaseBuffer();
		}
		int iTemp = 0;
		CDataFile::Save(FID_TERM_AUTOSET_FLAG, (BYTE *) &iTemp, sizeof(int));
		CDataFile::Delete(FID_CLOSE_BATCH_FLAG);
	}
}

//----------------------------------------------------------------------------
//!	\brief	Do ECR reversal
//----------------------------------------------------------------------------
void CFinan::DoECRReversal()
{
	if (m_TRREC.ComStatus == ST_OK &&
			m_TRREC.TranStatus == ST_APPROVED)
	{
		if (m_TRREC.TranCode > FUNCTION_FINAN_BEGIN &&
			  m_TRREC.TranCode < FUNCTION_FINAN_END)
		{
			m_TRREC.bReversal = TRUE;
			m_TRREC.TranStatus = ST_TIMEOUT;		
			m_TRREC.ReversalType = TIMEOUT_REVERSAL;
			m_RapidConnect->Start(&m_TRREC, &m_CONFIG,FALSE);
			Print(2);
		}
	}
}
//----------------------------------------------------------------------------
//!	\brief	after host connect finish , automatic call this function 
//  So in this function do some save data job and check close batch flag
//----------------------------------------------------------------------------
void CFinan::AfterHost()
{
	if (strcmp(m_TRREC.RespCode, "998") == 0)// || strcmp(m_TRREC.RespCode,"898")==0)
	{
		int status = 1;
		CDataFile::Save(FID_CLOSE_BATCH_FLAG, (BYTE *)&status, sizeof(int));
		return;
	}
}

//-----------------------------------------------------------------------------
//!	\Process Manual Card from Track2 data
//!	\param	pTrack2 - point to Track 2 data
//! RETURNS:       CARD_NO_ERROR    = Validate.                               
//!                other  = not Validate. 									  
//-----------------------------------------------------------------------------
int CFinan::ProcessManualCard(char *pAccount)
{
	m_iKeyStatus =  KEY_MANUAL;
	m_TRREC.CardType = CARD_CREDIT;
	m_TRREC.EntryMode = ENTRY_MODE_MANUAL;

	BYTE cardError = CFinUtil::ValidateCard(pAccount, m_TRREC.ServType, &m_TRREC.binRecord);
	if (cardError == CARD_NO_ERROR)
	{
		if (strcmp("P0", m_TRREC.binRecord.serviceType) == 0)	// debit service type
		{
			cardError = CARD_ERR_CARD_NOT_SUPPORT;
		}
	}
	return cardError;
}

//----------------------------------------------------------------------------
//!	\brief	virtual function
//----------------------------------------------------------------------------
void CFinan::GoBack()
{

}

//----------------------------------------------------------------------------
//!	\brief	virtual function
//----------------------------------------------------------------------------
void CFinan::GoNext()
{

}
//----------------------------------------------------------------------------
//!	\brief	virtual function
//----------------------------------------------------------------------------
void CFinan::GoToLevel(int level)
{

}
//----------------------------------------------------------------------------
//!	\brief	virtual function
//----------------------------------------------------------------------------
void CFinan::ErrorExit()
{
}	
//----------------------------------------------------------------------------
//!	\brief	virtual function
//----------------------------------------------------------------------------
void CFinan::ErrorExit(int Level)
{
}	

//----------------------------------------------------------------------------
//!	\brief	virtual function
//----------------------------------------------------------------------------
void CFinan::SetErrorCode(char* code, int m1, int m2)
{
}
//----------------------------------------------------------------------------
//!	\brief	for print job
//----------------------------------------------------------------------------
void CFinan::Print(int status)
{
	if (strlen(m_TRREC.Amount) == 0)
		PutAmount(m_TRREC.Amount, m_strAmount);

	if (strlen(m_TRREC.TotalAmount) == 0)
		strcpy(m_TRREC.TotalAmount, m_TRREC.Amount);

	m_TRREC.HostTranCode = m_TRREC.TranCode;

	m_bHavePrint = TRUE;
	m_TRREC.CustLang = CLanguage::GetLanguage();
	if (CAppData::bEMVError)
	{
		m_Printer.m_bCancelFlag = FALSE;
		m_TRREC.EmvData.EmvState = EMV_STATE_ERROR;
	}
	((CMainWndApp*)AfxGetApp())->m_bCancelFlag = m_bCancelFlag;
	((CMainWndApp*)AfxGetApp())->m_declinedOffline = declinedOffline;
	memcpy(&(((CMainWndApp*)AfxGetApp())->m_TRREC),&m_TRREC,sizeof(m_TRREC));
}

//----------------------------------------------------------------------------
//!	\brief	open RFID
//----------------------------------------------------------------------------
void CFinan::ReOpenRFID()
{
	char amount[10];
	PutAmount(amount, m_strAmount);

	int iAmount = atoi(amount);
	PutAmountWithDot(amount, m_strAmount);

	m_RFID.Open(m_MsgWnd, amount, m_TRREC.TranCode,iAmount);
}

//----------------------------------------------------------------------------
//!	\brief	Check Password for clerk 
//----------------------------------------------------------------------------
BOOL CFinan::CheckPassword(BOOL Manager)
{
	char data[10]={0};
	DWORD size;
	CString str = L"0000";;
	if(CDataFile::Read(FID_PASSWORD_MANAGER, (BYTE*) data, &size))
		str = CString(data);

	if ( Manager)
		return str==m_strPassword;

	CString str1 = L"0000";;
	if(CDataFile::Read(FID_PASSWORD_CLERK, (BYTE*) data, &size))
		str1 = CString(data);

	return str==m_strPassword || str1==m_strPassword;

}

//-----------------------------------------------------------------------------
//!	\Process EMV TLV date
//!	\param	tlvLength - tls data length
//!	\param	tlv - point to tlv data
//!	\param	sentToHost - send to host or recv from EMV card
//-----------------------------------------------------------------------------
void CFinan::ProcessEmvTLV(USHORT tlvLength, BYTE *tlv, BOOL sentToHost)
{
	USHORT length, k, n, tag;
	BYTE *data;
	BOOL gotAID = FALSE;
	BYTE tempT2[100];
	USHORT tempT2Len = 0;
	USHORT tagSize, lenSize;
	BOOL msdTrack2Present = FALSE;
	USHORT C_tags_len = 0;

	TRACE(TEXT("ProcessEmvTLV()\n"));

	n = tlvLength;
	data = tlv;

	while (n > 0)
	{
		// check tag
		if ((*data & 0x1F) == 0x1F)          //2 bytes tag
		{
			tagSize = sizeof(USHORT);
			tag = (*data)*0x100 + *(data+1);
		}
		else
		{
			tagSize = sizeof(BYTE);
			tag = *data;
		}
		if( n < tagSize)
			return;
		n -= tagSize;
		data += tagSize;

		// check length
		if ((*data & 0x80) == 0x80)
		{
			lenSize = 1 + *data&0x7F;
			if (lenSize == 2)
			{
				length = *(data+1);
			}
			else if (lenSize == 3)
			{
				length = *(data+1)*0x100 + *(data+2);
			}
			else
			{
				length = 0;
			}	
		} 
		else
		{
			lenSize = 1;
			length = *data;
		}

		if( n < lenSize)
			return;
		n -= lenSize;
		data += lenSize;
		if (length == 0 || length > n)
		{
			return;
		}

		switch (tag)
		{
		case EMV_TAG_APPLICATION_IDENTIFIER_TERMINAL:
			{
				k = STR_UnpackData((char *)m_TRREC.EmvData.AID, data, length);
				m_TRREC.EmvData.AID[k] = 0;
			}
			gotAID = TRUE;
			break;

		case EMV_TRANSACTION_STATUS_INFORMATION:
			if (length == 2 && FullEmvTransaction())
			{
				memcpy(m_TRREC.EmvData.TransactionStatusInfo, data, length);
				//k = STR_UnpackData(m_TRREC.EmvData.TransactionStatusInfo, data, length);
			}
			break;

		case EMV_TAG_CARDHOLDER_NAME:
			// Cardholder Name - Tag 5F20 - Format ans 2 - 26 - Length 2 - 26
			k = length;
			if (k > sizeof(m_TRREC.CustName) - 1)
			{
				k = sizeof(m_TRREC.CustName) - 1;
			}
			memcpy(m_TRREC.CustName, data, k);
			m_TRREC.CustName[k] = '\0';
			break;

		case EMV_TAG_APPLICATION_LABEL:
			// Application Label - Tag 50 - Format an 1 - 16 - Length 1 - 16
			memset(m_TRREC.EmvData.ApplicationLabel, 0, sizeof(m_TRREC.EmvData.ApplicationLabel));
			memcpy(m_TRREC.EmvData.ApplicationLabel, data, length);
			break;

		case EMV_TAG_ISSUER_CODE_TABLE_INDEX:
			// Issuer Code Table Index - Tag 9F11 - Format n 2 - Length 1
			if (length == 1)
			{
				m_TRREC.EmvData.IssuerCodeTableIndex[0] = *data;
			}
			break;

		case EMV_TAG_APPLICATION_PREFERRED_NAME:
			// Application Preferred Name - Tag 9F12 - Format an 1 - 16 - Length 1 - 16
			memset(m_TRREC.EmvData.ApplicationPreferredName, 0, 
				sizeof(m_TRREC.EmvData.ApplicationPreferredName));
			memcpy(m_TRREC.EmvData.ApplicationPreferredName, data, length);														
			break;

		case EMV_TAG_APPLICATION_IDENTIFIER:
			// Application Identifier - Tag 4F - Format b - Length 5 - 16
			k = (size_t)STR_UnpackData(m_TRREC.EmvData.ICC_AID, data, length);
			m_TRREC.EmvData.ICC_AID[k] = 0;
			break;

		case EMV_TAG_LANGUAGE_PREFERENCE:
			{
				int tmplen = length;
				char *tmpptr = (char *)data;

				while (tmplen>1 && tmpptr)
				{
					if (memcmp(data, EnglishLanguageString, 2) == 0)
					{
						m_TRREC.CustLang = ENGLISH;//LANG_ENGLISH;
						break;
					}
					else if (memcmp(data, FrenchLanguageString, 2) == 0)
					{
						m_TRREC.CustLang = FRENCH;//LANG_FRENCH;
						break;
					}
					tmplen -= 2;
					tmpptr += 2;
				}
				break;
			}

		case EMV_TAG_TRACK_2_EQUIVALENT_DATA:
			// FID q
			// Track 2 Equivalent Data - Tag 57 - Format b - Length var. up to 19
			if (m_TRREC.bEmvTransaction)
			{
				ProcessTrack2EquivalentData(data, length, m_TRREC.bEmvTransaction);
			}
			else
			{
				tempT2Len = length;
				memcpy(tempT2, data, tempT2Len);
			}
			break;

		case EMV_TAG_MSR_TRACK2:
			// FID q
			// Track 2 Equivalent Data - Tag 57 - Format b - Length var. up to 19
			if (m_TRREC.bEmvTransaction || (m_TRREC.bEmvCtlsTransaction && gotAID))
			{
				if (!msdTrack2Present)
					ProcessTrack2EquivalentData(data, length, m_TRREC.bEmvTransaction);
			}
			break;

		case EMV_TAG_PRIMARY_ACCCOUNT_NUMBER:
			memset(m_TRREC.EmvData.PrimaryAccountNumber, 0, sizeof(m_TRREC.EmvData.PrimaryAccountNumber));
			memcpy(m_TRREC.EmvData.PrimaryAccountNumber, data, length);
			CDataFile::Save( FID_OFFLINE_5A, data, length);
			break;

		case EMV_TAG_CRYPTOGRAM_INFORMATION_DATA:
			// FID 6E - POS Entry Mode - Tag 9F39 - Constant "051"
			// FID 6N - Reason Online Code - no tag - Constant "1508"
			// FID 6O,1-2 - Smart Card Scheme - no tag - Constant "01"
			// FID 6O,3-4 - Cryptographic Information Data - Tag 9F27
			// Cryptogram Information Data - Tag 9F27 - Format b - Length 1
			if (length == 1)
			{
				memcpy(m_TRREC.EmvData.CryptogramInformationData, data, length);
				CDataFile::Save( FID_OFFLINE_9F27, data, length);
			}
			break;

		case EMV_TAG_TRANSACTION_DATE:
			// Transaction Date (YYMMDD) - Tag 9A - Format n 6 - Length 3
			if (length == 3)
			{
				memcpy(m_TRREC.EmvData.TransactionDate, data, length);
				CDataFile::Save( FID_OFFLINE_9A, data, length);
			}	
			break;

		case EMV_TAG_TRANSACTION_TIME:
			// Transaction Date (YYMMDD) - Tag 9F21 - Format n 6 - Length 3
			if (length == 3)
			{
				memcpy(m_TRREC.EmvData.TransactionTime, data, length);
			}	
			break;

		case EMV_TAG_APPLICATION_CRYPTOGRAM:
			// FID 6O,14-29 - Authorization Request Cryptogram (ARQC) [also TC and AAC] - Tag 9F26
			// Application Cryptogram - Tag 9F26 - Format b - Length 8
			if (length == 8)
			{
				if (sentToHost)
				{
					memcpy(m_TRREC.EmvData.ApplicationCryptogramForResponse, data, length);
				}
				else
				{
					memcpy(m_TRREC.EmvData.ApplicationCryptogramForRequest, data, length);
				}

				CDataFile::Save( FID_OFFLINE_9F26, data, length);
			}
			break;

		case EMV_TAG_APPLICATION_INTERCHANGE_PROFILE:
			// FID 6O,30-33 - Application Interchange Profile (AIP) - Tag 82
			// Application Interchange Profile (AIP)- Tag 82 - Format b - Length 2
			if (length == 2)
			{
				memcpy(m_TRREC.EmvData.ApplicationInterchangeProfile, data, length);
				CDataFile::Save( FID_OFFLINE_82, data, length);
			}	
			break;

		case EMV_TAG_APPLICATION_TRANSACTION_COUNTER:
			// FID 6O,34-37 - Application Transaction Counter (ATC) - Tag 9F36
			// Application Transaction Counter (ATC) - Tag 9F36 - Format b - Length 2
			if (length == 2)
			{
				memcpy(m_TRREC.EmvData.ApplicationTransactionCounter, data, length);
				CDataFile::Save( FID_OFFLINE_9F36, data, length);
			}	
			break;

		case EMV_TAG_UNPREDICTABLE_NUMBER:
			// FID 6O,38-45 - Unpredictable Number - Tag 9F37
			// Unpredictable Number - Tag 9F37 - Format b - Length 4
			if (length == 4)
			{
				memcpy(m_TRREC.EmvData.UnpredictableNumber, data, length);
				CDataFile::Save( FID_OFFLINE_9F37, data, length);
			}	
			break;

		case EMV_TAG_TERMINAL_VERIFICATION_RESULTS:
			// FID 6O,46-55 - Terminal Verification Result (TVR) - Tag 95
			// Terminal Verification Results (TVR) - Tag 95 - Format b - Length 5
			if (length == 5)
			{
				if (sentToHost)
					memcpy(m_TRREC.EmvData.TerminalVerificationResultsTC, data, length);
				else
					memcpy(m_TRREC.EmvData.TerminalVerificationResultsARQC, data, length);	
				CDataFile::Save( FID_OFFLINE_95, data, length);

			}
			break;

		case EMV_TAG_TRANSACTION_TYPE:
			// FID 6O,56-57 - Cryptogram Transaction Type - Tag 9C
			// Transaction Type - Tag 9C - Format n 2 - Length 1
			if (length == 1)
			{
				memcpy(m_TRREC.EmvData.TransactionType, data, length);
				CDataFile::Save( FID_OFFLINE_9C, data, length);
			}	
			break;

		case EMV_TAG_ISSUER_COUNTRY_CODE:
			// Issuer Country Code - Tag 5F28 - Format n 3 - Length 2
			if (length == 2)
			{
				memcpy(m_TRREC.EmvData.IssuerCountryCode, data, length);
			}	
			break;

		case EMV_TAG_TRANSACTION_CURRENCY_CODE:
			// FID 6O,58-60 - Cryptogram Currency Code - Tag 5F2A
			// Transaction Currency Code - Tag 5F2A - Format n 3 - Length 2
			if (length == 2)
			{
				memcpy(m_TRREC.EmvData.TransactionCurrencyCode, data, length);
				CDataFile::Save( FID_OFFLINE_5F2A, data, length);				
			}	
			break;

		case EMV_TAG_AMOUNT_AUTHORIZED_NUMERIC:
			// FID 6O,61-72 - Cryptogram Amount - Tag 9F02
			// Cryptogram Amount - Tag 9F02 - Format n 12 - Length 6
			if (length == 6)
			{
				memcpy(m_TRREC.EmvData.AmountAuthorizedNumeric, data, length);
				CDataFile::Save( FID_OFFLINE_9F02, data, length);
			}
			break;
		case EMV_TAG_AMOUNT_OTHER_NUMERIC:
			// FID 6O,61-72 - Cryptogram Amount - Tag 9F03
			// Cryptogram Amount - Tag 9F02 - Format n 12 - Length 6
			if (length == 6)
			{
				CDataFile::Save( FID_OFFLINE_9F03, data, length);
			}
			break;

		case EMV_TAG_ISSUER_APPLICATION_DATA:
			// FID 6O,73-74 - Issuer Application Data Length - Tag 9F10,1
			// FID 6O,75-76 - Certification Authority Public Key Index - Tag 9F10,2 or Tag 9F22 (Terminal) or 8F (ICC) - Format b - Length 1
			// FID 6O,77-78 - Cryptogram Version Number - Tag 9F10,3 - Constant "10"
			// FID 6O,79-86 - Card Verification Results - Tag 9F10,4-7
			// FID 6O,87-136 - Issuer Discretionary Data - Tag 9F10,8-32
			// Issuer Application Data - Tag 9F10 - Format b - Length variable up to 32
			if (length <= 32)
			{
				m_TRREC.EmvData.IssuerAuthenticationDataLen = (BYTE) length;	
				memcpy(m_TRREC.EmvData.IssuerAuthenticationData, data, length);
				CDataFile::Save( FID_OFFLINE_9F10, data, length);
			}
			break;

		case EMV_TAG_APPLICATION_PAN_SEQUENCE_NUMBER:
			// FID 6P,1-2 - Smart Card Scheme - no tag - Constant "01"
			// FID 6P,3-4 - Application PAN Sequence Number - Tag 5F34
			// Application Primary Account Number (PAN) Sequence Number - Tag 5F34 - Format n 2 - Length 1
			if (length == 1)
			{
				memcpy(m_TRREC.EmvData.ApplicationPANSequenceNumber, data, length);
				CDataFile::Save( FID_OFFLINE_5F34, data, length);
			}	
			break;
		case TAG_EMV_AUTHORIZATION_RESPONSE_CODE:
			if (length == 2)
			{
				CDataFile::Save( FID_OFFLINE_8A, data, length);
			}	
			break;
		case TAG_EMV_CVM_LIST://ADVT case 23
			{
				for ( int i=8;i<length;)
				{
					if ( *(data+i) == 0x03 )
					{
						m_TRREC.bRequestedPIN = FALSE;
						break;
					}
					i += 2;
				}
			}	
			break;

		case EMV_TAG_TERMINAL_TYPE:
			// FID 6P,5-6 - EMV Terminal Type - Tag 9F35
			// EMV Terminal Type - Tag 9F35 - Format n 2 - Length 1
			if (length == 1)
			{
				memcpy(m_TRREC.EmvData.TerminalType, data, length);
			}	
			break;

		case EMV_CHECK_TRY_PIN_TIME: //Tag 9F17
			if (length == 1)
			{
				memcpy(m_TRREC.EmvData.TerminalPINTryTimes, data, length);
			}	
			break;
		case EMV_TERMINAL_COUNTRY_CODE: //Tag 9F1A
			if (length == 2)
			{
				CDataFile::Save( FID_OFFLINE_9F1A, data, length);
			}	
			break;
		case EMV_TAG_POS_ENTRY_MODE_CODE: //Tag 9F39
			if (length == 1)
			{
				//					memcpy(m_TRREC.EmvData.TerminalType, data, length);
			}	
			break;
		case EMV_TAG_CARD_TRANSACTION_QUALIFIER: //Tag 9F6C
			if (length == 2)
			{
				if(*data & 0x40)
					CAppData::bSignatureRequired = TRUE;

				//					memcpy(m_TRREC.EmvData.TerminalType, data, length);
			}	
			break;
		case EMV_TAG_CARDHOLDER_VERIFICATION_METHOD_RESULTS:
			// FID 6P,7-12 - Cardholder Verification (CVM) Results - Tag 9F34
			// Cardholder Verification Method Results - Tag 9F34 - Format b - Length 3
			if (length == 3)
			{
				memcpy(m_TRREC.EmvData.CardholderVerificationMethodResults, data, length);
				CDataFile::Save( FID_OFFLINE_9F34, data, length);
			}
			break;
		case EMV_TAG_ICC_APPLICATION_VERSION_NUMBER:
			if (length == 2)
			{
				memcpy(m_TRREC.EmvData.ApplicationVersionNumber, data, length);
			}
			break;
		case EMV_TAG_TERM_APPLICATION_VERSION_NUMBER:
			// FID 6P,13-16 - Application Version Number - Tag 9F08 or 9F09
			// Application Version Number - Tag 9F08 (ICC) or 9F09 (terminal) - Format b - Length 2
			if (length == 2)
			{
				memcpy(m_TRREC.EmvData.ApplicationVersionNumber, data, length);
			}
			break;

		case EMV_TAG_DEDICATED_FILE_NAME:
			// FID 6P,17-48 - Dedicated File Name - Tag 84
			// Dedicated File Name - Tag 84 - Format b - Length 5 - 16
			if (length >= 5 && length <= 16)
			{
				m_TRREC.EmvData.DedicatedFilenameLen = (BYTE) length;	
				memcpy(m_TRREC.EmvData.DedicatedFilename, data, length);

				if ( strlen(m_TRREC.EmvData.AID) == 0)
				{
					k = STR_UnpackData((char *)m_TRREC.EmvData.AID, data, length);
					m_TRREC.EmvData.AID[k] = 0;
				}
			}
			break;

		case EMV_TAG_ISSUER_SCRIPT_RESULTS:
			// FID 9E - Chip Condition Code - no tag - determined by application
			// FID 9F - Authorization Response Code - Tag 8A - echo response FID 6Q,21-24
			// FID 9G,1 - Number of Issuer Script Results - no tag - calculated
			// FID 9G,2 - Issuer Script Processing Result - Tag DF11 ,1-high Nibble
			// FID 9G,3 - Issuer Script Sequence - Tag DF11 ,1-low Nibble
			// FID 9G,4-11 - Issuer Script Identifier - Tag DF11,2-5
			// Result of the execution of the scripts - format b 5 - Length 5 - 160
			/*
			if (length > 0 && length <= 20 && length % 5 == 0)
			{
			BYTE IssuerScriptResult71[100];								// (71)
			BYTE IssuerScriptResult71Len;
			BYTE IssuerScriptResult72[100];								// (72)
			BYTE IssuerScriptResult72Len;
			uint8 EMVIssuerScriptResults[20];

			uint8Buffer = (uint8) length;
			DAT_put_shared_data(TAG_EMVIssuerScriptResultsLength,
			&uint8Buffer, sizeof uint8Buffer);
			memset(EMVIssuerScriptResults, '\0', sizeof EMVIssuerScriptResults);
			memcpy(EMVIssuerScriptResults, data, length);
			DAT_put_shared_data(TAG_EMVIssuerScriptResults,
			EMVIssuerScriptResults, sizeof EMVIssuerScriptResults);
			}
			*/
			break;

		case EMV_TAG_TERMINAL_CAPABILITIES:
			if (length == 3)
				memcpy(m_TRREC.EmvData.TerminalCapabilities, data, length);
			break;

		case EMV_TAG_IAC_DEFAULT:
			if (length == 5)
				CDataFile::Save( FID_OFFLINE_IAC_DEFAULT, data, length);
			break;

		case EMV_TAG_IAC_DENIAL:
			if (length == 5)
				CDataFile::Save( FID_OFFLINE_IAC_DENIAL, data, length);
			break;

		case EMV_TAG_IAC_ONLINE:
			if (length == 5)
				CDataFile::Save( FID_OFFLINE_IAC_ONLINE, data, length);
			break;

		case EMV_TAG_TAC_DEFAULT:
			if (length == 5)
				CDataFile::Save( FID_OFFLINE_TAC_DEFAULT, data, length);
			break;

		case EMV_TAG_TAC_DENIAL:
			if (length == 5)
				CDataFile::Save( FID_OFFLINE_TAC_DENIAL, data, length);
			break;

		case EMV_TAG_TAC_ONLINE:
			if (length == 5)
				CDataFile::Save( FID_OFFLINE_TAC_ONLINE, data, length);
			break;

		default:
			// FID 9H,1-2 - EMV Kernel Main Version Number - no tag - calculated
			// FID 9H,3-5 - EMV Kernel Sub-version Number - no tag - calculated
			// FID 9H,6 - Number of Application Version Numbers - no tag - calculated
			// FID 9H,7-8 - Application Main Version Number - no tag - calculated
			// FID 9H,9-10 - Application Sub-version Number - no tag - calculated
			// tag EMV_TAG_TERMINAL_CAPABILITIES (0x9F33 - contactless terminal capabilities)
			// tag 0x9F1A - Terminal country code
			// tag 0xDF04 - TAC Denial
			// tag 0xDF05 - TAC online
			// tag 0xDF03 - TAC Default
			break;
		}

		n -= length;
		data += length;
	}

	if (tempT2Len > 0)
	{
		if (m_TRREC.bEmvTransaction)
		{
			ProcessTrack2EquivalentData(tempT2, tempT2Len, m_TRREC.bEmvTransaction);
		}
		else
		{
			if (msdTrack2Present)
				m_TRREC.CardError = ProcessSwipedCard(m_TRREC.Track2);
			else
				ProcessTrack2EquivalentData(tempT2, tempT2Len, m_TRREC.bEmvTransaction);
		}
	}
}

//-----------------------------------------------------------------------------
//!	\Process clts tag E0
//!	\param	tlvLength - tlv data length
//!	\param	tlv - point to tlv data
//-----------------------------------------------------------------------------
void CFinan::ProcessCtlsTagE0(USHORT tlvLength, BYTE *tlv)
{
	USHORT length, n, tag;
	BYTE *data;
	USHORT tagSize, lenSize;

	TRACE(TEXT("ProcessCtagsTLV()"));

	n = tlvLength;
	data = tlv;

	while (n > 0)
	{
		// check tag
		tagSize = sizeof(BYTE);
		tag = *data;

		//		assert(n >= tagSize);
		if( n < tagSize)
			return;
		n -= tagSize;
		data += tagSize;

		// length
		lenSize = 1;
		length = *data;

		//		assert(n >= lenSize);
		if( n < lenSize)
			return;
		n -= lenSize;
		data += lenSize;
		if (length == 0 || length > n)
		{
			return;
		}

		switch (tag)
		{
		case EMV_TAG_CTLS_C0:	// Brand Code
			m_TRREC.CtlsData.C0[0] = *data;
			break;
		case EMV_TAG_CTLS_C1:	// Card Scheme
			m_TRREC.CtlsData.C1[0] = *data;
			break;
		case EMV_TAG_CTLS_C2:	// Transaction Result
			m_TRREC.CtlsData.C2[0] = *data;
			break;
		case EMV_TAG_CTLS_C3:	// Error Code
			m_TRREC.CtlsData.C3[0] = *data;
			break;
		case EMV_TAG_CTLS_C7:	// Transaction CVM
			m_TRREC.CtlsData.C7[0] = *data;
			break;
		case EMV_TAG_CTLS_C8:	// Terminal Entry Capability (VISA)
			m_TRREC.CtlsData.C8[0] = *data;
			break;
		case EMV_TAG_CTLS_C9:	// Transaction Error Stage
			m_TRREC.CtlsData.C9[0] = *data;
			break;
		default:
			break;	
		}
		n -= length;
		data += length;
	}			
}				

//-----------------------------------------------------------------------------
//!	\Process clts tlv
//!	\param	tlvLength - tlv data length
//!	\param	tlv - point to tlv data
//-----------------------------------------------------------------------------
void CFinan::ProcessCtlsTLV(USHORT tlvLength, BYTE *tlv)
{
	USHORT length, n, tag;
	BYTE *data;
	USHORT tagSize, lenSize;

	TRACE(TEXT("ProcessCtlsTLV()"));

	for (int i=0; i<tlvLength; i++)
	{			
		TRACE(L"%02X ", tlv[i]);
	}

	n = tlvLength;
	data = tlv;

	while (n > 0)
	{
		// check tag
		if ((*data & 0x1F) == 0x1F)          //2 bytes tag
		{
			tagSize = sizeof(USHORT);
			tag = (*data)*0x100 + *(data+1);
		}
		else
		{
			tagSize = sizeof(BYTE);
			tag = *data;
		}
		//		assert(n >= tagSize);
		if( n < tagSize)
			return;
		n -= tagSize;
		data += tagSize;

		// check length
		if ((*data & 0x80) == 0x80)
		{
			lenSize = 1 + *data&0x7F;
			if (lenSize == 2)
			{
				length = *(data+1);
			}
			else if (lenSize == 3)
			{
				length = *(data+1)*0x100 + *(data+2);
			}
			else
			{
				length = 0;
			}	
		} 
		else
		{
			lenSize = 1;
			length = *data;
		}

		//		assert(n >= lenSize);
		if( n < lenSize)
			return;
		n -= lenSize;
		data += lenSize;
		if (length == 0 || length > n)
		{
			return;
		}

		switch (tag)
		{
		case EMV_TAG_CTLS_D1:
			break;

		case EMV_TAG_CTLS_D2:
			m_TRREC.bEmvTransaction = FALSE;
			memset(m_TRREC.Track2, 0, sizeof(m_TRREC.Track2));
			memcpy(m_TRREC.Track2, data, length);
			break;

		case EMV_TAG_CTLS_E0:
			ProcessCtlsTagE0(length, data);
			break;

		case EMV_TAG_CTLS_E2:
			ProcessEmvTLV(length, data, FALSE);
			break;

		default:
			break;
		}

		n -= length;
		data += length;
	}
}

//-----------------------------------------------------------------------------
//!	\Process Track2 Equivalent Data
//!	\param	data - point to Equivalent data
//!	\param	length - Equivalent data length
//!	\param	saveLang - language saved flag
//-----------------------------------------------------------------------------
void CFinan::ProcessTrack2EquivalentData(BYTE *data, USHORT length, BOOL saveLang)
{
	size_t n;
	char *p;

	//	assert((int)length * 2 <= (int)sizeof(m_TRREC.Track2) - 1);
	if ((int)length * 2 > (int)sizeof(m_TRREC.Track2) - 1)
		return;
	n = (size_t) STR_UnpackData(m_TRREC.Track2, data, (int) length);
	m_TRREC.Track2[n] = '\0';

	p = strchr(m_TRREC.Track2, 'F');
	if (p != NULL)
	{
		*p = '\0';
	}

	p = strchr(m_TRREC.Track2, 'D');
	if (p != NULL)
	{
		*p = '=';
	}

	if (strspn(m_TRREC.Track2, "0123456789=") == strlen(m_TRREC.Track2))
	{
		BYTE custLangSave;

		if (saveLang)
			custLangSave = m_TRREC.CustLang;

		int cardtype = m_TRREC.CardType;
		m_TRREC.CardError = ProcessSwipedCard(m_TRREC.Track2);
		if (saveLang)
			m_TRREC.CustLang = custLangSave;

		m_TRREC.CardType = cardtype;

		if (m_TRREC.CardError)
		{
			m_TRREC.EmvError = KEY_EMV_CARD_ERROR;
		}
	}
}

//----------------------------------------------------------------------------
//!	\brief	check it is Full Emv Transaction
//----------------------------------------------------------------------------
BOOL CFinan::FullEmvTransaction(void)
{
	if (m_TRREC.TranCode == TRAN_PURCHASE ||
		m_TRREC.TranCode == TRAN_PREAUTH ||
		m_TRREC.TranCode == TRAN_ADVICE ||
		m_TRREC.TranCode == TRAN_FORCE_POST ||
		m_TRREC.TranCode == TRAN_REFUND ||
		m_TRREC.TranCode == TRAN_PURCHASE_VOID ||
		m_TRREC.TranCode == TRAN_REFUND_VOID)
	{
		if (CEMV::IsInteracRID((char *)m_TRREC.EmvData.AID) || CEMV::IsMaestroAID((char *)m_TRREC.EmvData.AID))
		{
			return (m_TRREC.bEmvTransaction || m_TRREC.bEmvCtlsTransaction);
		}
		else
		{
			return (m_TRREC.bEmvTransaction || m_TRREC.bEmvCtlsTransaction) &&
				(m_TRREC.TranCode == TRAN_PURCHASE || m_TRREC.TranCode == TRAN_PREAUTH);
		}
	}
	else
	{
		return FALSE;
	}
}

//----------------------------------------------------------------------------
//!	\brief	check Support English Or French , not use this function yet
//----------------------------------------------------------------------------
BOOL CFinan::SupportEnglishOrFrench()
{
	//If the EMV card does not support English or French then the following prompt will be displayed. 
	//	Otherwise, the card language will automatically be selected. If the card supports both English
	//	and French, the first language found will be selected.
	return FALSE;
}


//-----------------------------------------------------------------------------
//!	\EMV Select Language
//!	\param	language - language index id
//-----------------------------------------------------------------------------
void CFinan::EMVSelectLanguage(int language)
{
	if(!m_iEMVStart)
		return;

	WORD ret = 0;
	BYTE byData[1024];
	WORD wLen = 1024;
	BYTE byTags[100];

	if (ret = EmvApplicationSelection())
	{
		EmvStop();
		PowerOffScr();
		m_iEMVStart = FALSE;
		return ;
	}

	memset(byData, 0, 1024);
	wLen = 1024;
	memcpy(byTags, "\x9F\x06", 2);
	EmvGetParam(byTags, 2, byData, &wLen);
	ProcessEmvTLV(wLen, byData,HostInProgress);

}

//----------------------------------------------------------------------------
//!	\brief	Set EMV Card Type
//----------------------------------------------------------------------------
void CFinan::SetEMVCardType()
{
	if (CEMV::IsInteracRID(m_TRREC.EmvData.AID))
		m_TRREC.CardType = CARD_DEBIT;
	else
		m_TRREC.CardType = CARD_CREDIT;
}

//----------------------------------------------------------------------------
//!	\brief	EMV Enter Pin GUI box
//----------------------------------------------------------------------------
BOOL CFinan::EMVEnterPin()
{
	PinEntryBox(); // this is demo
	return TRUE;
}

//----------------------------------------------------------------------------
//!	\brief	EMV transaction entry
//----------------------------------------------------------------------------
void CFinan::StartEMV()
{
	TRACE(TEXT("=============== StartEMV() ============================\n"));

	m_bEMV = TRUE;
	m_TRREC.EntryMode = ENTRY_MODE_CHIP;
	HANDLE hThread = CreateThread(NULL, 0,ThreadProc, (PVOID)this, 0, NULL);
	CloseHandle(hThread);
}

//----------------------------------------------------------------------------
//!	\brief	Process EMV transaction error entry
//----------------------------------------------------------------------------
void CFinan::ProcessEmvError()
{
	m_strErr = m_strErr1 = m_strErr2 = L"";
	int wErrMsg=0;										    //JC Apr 7/15 to pass message to API

	if (m_TRREC.TranStatus == ST_APPROVED)	// Host has approved, reverse txn
	{
		m_TRREC.TranStatus = ST_CARD_DECL_HOST_APPR;
		strcpy(m_TRREC.HostRespText, CLanguage::GetCharText(CLanguage::IDX_DECLINED_BY_CARD));//"DECLINED BY CARD");
		TRACE(TEXT("CHIP REVERSAL\n"));

		m_TRREC.EmvData.CryptogramInformationData[0] = 0; 

		DWORD size;
		BYTE data[512];
		CDataFile::Read(FID_OFFLINE_9F10,data,&size);
		m_TRREC.EmvData.IssuerAuthenticationDataLen = (BYTE) size;	
		memcpy(m_TRREC.EmvData.IssuerAuthenticationData, data, size);

		
	}

	if( IsCancel() && m_TRREC.EmvError != EMV_ERROR_CARD_ABSENT &&!g_PinTimeOut)
		m_TRREC.EmvError = EMVERR_USER_CANCEL;

	switch (m_TRREC.EmvError)
	{
	case EMV_PARTIAL_TXN:	// partial txn, continue as a swiped card.
		DoParitalTxn();
		return;
		//		break;
	case EMV_CARD_NOT_MATCH:	// partial txn, continue as a swiped card.
		m_strErr1 = CLanguage::GetText(CLanguage::IDX_CARD_NOT_MATCH);
		m_strErr2 = L"";
		ErrorExit(ENTERCARD_INPUT);
		break;
	case EMV_ERROR_CANCEL:
	case EMVERR_USER_CANCEL:
		if ( g_bWithPrimaryCandidates == 0 && g_GotoSelectApp)
		{
			m_strErr1 = CLanguage::GetText(CLanguage::IDX_APPLICATION);
			m_strErr2 = CLanguage::GetText(CLanguage::IDX_NO_SUPPORTED);
			SetErrorCode(ERROR_APPLICATION_NO_SUPPORTED, CLanguage::IDX_APPLICATION, CLanguage::IDX_NO_SUPPORTED);    //JC Apr 7/15 to pass message to API
			ErrorExit();
		}
		else
			GoToLevel(CANCELENTRY);
		break;
	case EMV_ERROR_TIMEOUT:
		m_strErr1 = CLanguage::GetText(CLanguage::IDX_TIME_OUT);
		m_strErr2 = L"";
		SetErrorCode(ERROR_CONDITION_CODE_TIMEOUT, CLanguage::IDX_TIME_OUT, 0);	   //JC Apr 7/15 to pass message to API
		ErrorExit();
		break;
	case EMV_ERROR_CARD_ABSENT:
		m_strErr1 = CLanguage::GetText(CLanguage::IDX_CARD_REMOVED);//L"CARD REMOVED";
		SetErrorCode(ERROR_CARD_REMOVED, CLanguage::IDX_CARD_REMOVED, 0);    //JC Apr 7/15 to pass message to API
		ErrorExit();
		break;
	case EMV_DEBIT_NOT_SUPPORT:
		m_strErr1 = CLanguage::GetText(CLanguage::IDX_DEBIT_CARD);
		m_strErr2 = CLanguage::GetText(CLanguage::IDX_NO_SUPPORTED);
		SetErrorCode(ERROR_DEBIT_CARD_NO_SUPPORTED, CLanguage::IDX_DEBIT_CARD, CLanguage::IDX_NO_SUPPORTED);    //JC Apr 7/15 to pass message to API
		ErrorExit();
		break;
	case EMV_CREDIT_NOT_SUPPORT:
		m_strErr1 = CLanguage::GetText(CLanguage::IDX_CREDIT_CARD);
		m_strErr2 = CLanguage::GetText(CLanguage::IDX_NO_SUPPORTED);
		SetErrorCode(ERROR_CREDIT_CARD_NO_SUPPORTED, CLanguage::IDX_CREDIT_CARD, CLanguage::IDX_NO_SUPPORTED);    //JC Apr 7/15 to pass message to API
		ErrorExit();
		break;
	case EMVERR_ICC_COMM_ERROR: // Error of communication between ICC and Smart Card Reader.
		if (GetScrStatus() == SCR_CARD_ABSENT)
		{
			m_TRREC.EmvError = EMV_ERROR_CARD_ABSENT;
			m_strErr1 = CLanguage::GetText(CLanguage::IDX_CARD_REMOVED);//L"CARD REMOVED";
			wErrMsg = CLanguage::IDX_CARD_REMOVED;     //JC Apr 7/15 to pass message to API
		}
		else
		{
			if (m_TRREC.EmvData.EmvState <= EMV_STATE_APP_SELECTION)
			{
				m_TRREC.bFallback = TRUE;
				m_strErr1 = L"";
			}
			else
			{
				m_strErr1 = CLanguage::GetText(CLanguage::IDX_EMV_CHIP_ERROR);//L"CHIP ERROR";
				wErrMsg = CLanguage::IDX_EMV_CHIP_ERROR;     //JC Apr 7/15 to pass message to API
			}
		}
		if(!m_TRREC.bFallback)
		{
			SetErrorCode(ERROR_EMVERR_ICC_COMM,wErrMsg, 0);		    //JC Apr 7/15 to pass message to API
			ErrorExit();
		}
		break;
	case EMVERR_CARD_BLOCK:
		m_strErr1 = CLanguage::GetText(CLanguage::IDX_CARD_BLOCKED);//L"CARD BLOCKED";
		SetErrorCode(ERROR_EMV_CARD_BLOCKED,CLanguage::IDX_CARD_BLOCKED,0);		    //JC Apr 7/15 to pass message to API
		ErrorExit();
		break;
	case EMVERR_NOSUPPORT_CARDAID:
		m_TRREC.bFallback = TRUE;
		break;
	case EMVERR_FRAUDCARD_DATA:
		m_strErr1 = CLanguage::GetText(CLanguage::IDX_FRAUD_CARD);//L"Fraud Card";
		SetErrorCode(ERROR_EMV_FRAUD_CARD,CLanguage::IDX_FRAUD_CARD, 0);		    //JC Apr 7/15 to pass message to API
		ErrorExit();
		break;
	case EMVERR_APPLICATION_BLOCKED:		
		m_strErr1.Format(L"%s %s",CLanguage::GetText(CLanguage::IDX_APPLICATION),CLanguage::GetText(CLanguage::IDX_CARD_BLOCKED));//L"APPLICATION BLOCKED";
		m_TRREC.bFallback = TRUE;
		break;
	case EMVERR_APPLICATION_NOTSUPPORT:		
		m_strErr1 = CLanguage::GetText(CLanguage::IDX_APPLICATION);
		if ( g_PrimaryBlocked )
		{
			m_strErr2 = CLanguage::GetText(CLanguage::IDX_CARD_BLOCKED);
			wErrMsg = CLanguage::IDX_CARD_BLOCKED;												//JC Apr 7/15 to pass message to API
		}
		else
		{
			m_strErr2 = CLanguage::GetText(CLanguage::IDX_NO_SUPPORTED);
			wErrMsg = CLanguage::IDX_NO_SUPPORTED;											    //JC Apr 7/15 to pass message to API
		}
		SetErrorCode(ERROR_EMV_APPLICATION_NOTSUPPORT, CLanguage::IDX_APPLICATION, wErrMsg);	    //JC Apr 7/15 to pass message to API
		ErrorExit();
		break;

	case EMVERR_NOT_ACCEPTED:
		m_strErr1 = CLanguage::GetText(CLanguage::IDX_NOT_ACCEPTED);//L"NOT ACCEPTED";
		m_TRREC.bFallback = TRUE;
		break;
	case EMVERR_STOP_TRANSACTION:
		if (m_TRREC.EmvData.EmvState <= EMV_STATE_APP_SELECTION)
		{
			m_strErr1 = CLanguage::GetText(CLanguage::IDX_NOT_ACCEPTED);//L"NOT ACCEPTED";
			m_strErr2 = CLanguage::GetText(CLanguage::IDX_ERROR);
			SetErrorCode(ERROR_EMV_STOP_TRANSACTION, CLanguage::IDX_NOT_ACCEPTED, CLanguage::IDX_ERROR);	    //JC Apr 7/15 to pass message to API
			ErrorExit();
		}
		else if(m_TRREC.EmvData.EmvState == EMV_STATE_INIT_APP)
		{
			if (m_TRREC.CardType == CARD_DEBIT)
			{
				m_strErr1 = CLanguage::GetText(CLanguage::IDX_EMV_CHIP_ERROR);//L"CHIP ERROR";
				m_strErr2 = CLanguage::GetText(CLanguage::IDX_PLEASE_REMOVE_CARD);
				SetErrorCode(ERROR_EMV_CHIP_ERROR, CLanguage::IDX_EMV_CHIP_ERROR, CLanguage::IDX_PLEASE_REMOVE_CARD);	    //JC Apr 7/15 to pass message to API
				ErrorExit();
			}
			else
				m_TRREC.bFallback = TRUE;
		}
		else if(m_TRREC.EmvData.EmvState == EMV_STATE_TERM_ACTION_ANALYSIS)
		{
			m_strErr1 = CLanguage::GetText(CLanguage::IDX_DECLINED_BY_CARD);
			m_strErr2 = CLanguage::GetText(CLanguage::IDX_PLEASE_REMOVE_CARD);
			SetErrorCode(ERROR_DECLINED_BY_CARD, CLanguage::IDX_DECLINED_BY_CARD, CLanguage::IDX_PLEASE_REMOVE_CARD);		    //JC Apr 7/15 to pass message to API
			ErrorExit();
		}
		else
		{
			m_TRREC.bFallback = TRUE;
		}	
		break;
	case EMVERR_MISS_ADFNAME:
	case EMVERR_MISS_APLABEL:
	case EMVERR_DATA_EXIST:
	case EMVERR_FILE_NOTFOUND:
	case EMVERR_RECORD_NOTFOUND:
	case EMVERR_INVALID_RSPDATA:
	case EMVERR_UNSUFFICIENTDATA:
	case EMVERR_GENERATEAC_WRONG:
		m_strErr1 = CLanguage::GetText(CLanguage::IDX_CARD_PROCESS);//L"CARD PROCESS";
		m_strErr2 = CLanguage::GetText(CLanguage::IDX_ERROR);
		SetErrorCode(ERROR_EMV_CARD_PROCESS, CLanguage::IDX_CARD_PROCESS, CLanguage::IDX_ERROR);	    //JC Apr 7/15 to pass message to API
		ErrorExit();
		break;
	case EMVERR_TRANSACTION_TYPE_NOT_SUPPORT:
	case EMVERR_INVALID_PARAMETER:
	case EMVERR_MAJOR_CONFIG_SETTING:
	case EMVERR_CMD_NOTALLOWED:
	case EMVERR_DATAPARENT_NULL:
	case EMVERR_CB_NOTSET:
	case EMVERR_INVALIDPARAMETER:
	case EMVERR_DATA_MISSING:
	case EMVERR_INVALID_TLV:
		TRACE(L"EMV error\n");
		m_strErr1 = CLanguage::GetText(CLanguage::IDX_CARD_PROCESS);//L"CARD PROCESS";
		m_strErr2 = CLanguage::GetText(CLanguage::IDX_ERROR);
		SetErrorCode(ERROR_EMV_CARD_PROCESS, CLanguage::IDX_CARD_PROCESS, CLanguage::IDX_ERROR);		    //JC Apr 7/15 to pass message to API
		ErrorExit();
		break;

	default:
		if(m_TRREC.bFallback )
			break ;

		m_strErr1.Format(L"%s: %d", CLanguage::GetText(CLanguage::IDX_ERROR),m_TRREC.EmvError);
		SetErrorCode(ERROR_EMV_CARD_PROCESS, CLanguage::IDX_ERROR, 0);			    //JC Apr 7/15 to pass message to API
		ErrorExit();
		break;
	}

	if (m_TRREC.bFallback )
	{
		m_strErr = CLanguage::GetText(CLanguage::IDX_EMV_CHIP_ERROR);

		m_strErr1 = CLanguage::GetText(CLanguage::IDX_FALLBACK_ALLOWED);
		m_strErr2 = CLanguage::GetText(CLanguage::IDX_REMOVE_CARD_QUICKLY);


	}
	CAppData::bEMVError = TRUE;

}
//----------------------------------------------------------------------------
//!	\brief	EMV transaction begin in the new thread
//----------------------------------------------------------------------------
int CFinan::DoEMV()
{
	CString str;
	if (strlen(m_TRREC.Amount) == 0)
		PutAmount(m_TRREC.Amount, m_strAmount);

	if (strlen(m_TRREC.TotalAmount) == 0)
		strcpy(m_TRREC.TotalAmount, m_TRREC.Amount);

	m_TRREC.HostTranCode = m_TRREC.TranCode;

	if (strlen(m_TRREC.InvoiceNo) == 0)
		GetInvoice();

	TCHAR buf[10]={L""};
	if (CDataFile::Read(L"TRAINING", buf))
	{
		if (CString(buf) == L"On")
			m_bTraining = TRUE;
	}

	m_TRREC.bFallback = FALSE;
	m_TRREC.bEmvTransaction = TRUE;
	m_bCancelPrint = TRUE;
	int txnSn;
	DWORD size=sizeof(int);

	TRACE(TEXT("=============== DoEMV() ============================\n"));
	// Get Txn SN
	if (!CDataFile::Read(FID_EMV_TRANSACTION_SN, (BYTE *) &txnSn, &size))
		txnSn = 0;

	txnSn++;

	size=sizeof(int);
	CDataFile::Save(FID_EMV_TRANSACTION_SN, (BYTE *) &txnSn, size);

	TXNCONDITION txn;
	memset( &txn, 0, sizeof(TXNCONDITION) );

	// BCD amount
	STR_LongToBCDString(atol(m_TRREC.TotalAmount), 6, txn.byTransAmount);	
	// Txn Type - 9C
	txn.wTxnType = EMV_TRANSTYPE_GOODS;
	// Account Type - 5F57
	txn.byAccountType = AccountType_Default;
	// txnSn - 9F41
	txn.wTxnSN = (WORD) txnSn;

	m_hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	BOOL error = EmvTransactionFlow(&txn);
	CloseHandle(m_hEvent); 
	m_hEvent = NULL;

	if (error)
	{
		// go to error level
		ProcessEmvError();

		if (m_TRREC.bFallback )
		{
			CDataFile::UpdateStatistics(4);
			CAppData::bEMVError = FALSE;
			ErrorExit();//ENTERCARD_INPUT);
		}
	}

	if ( m_TRREC.TranStatus == ST_CARD_DECLINE )
		::PostMessage(m_MsgWnd, WM_SOCK_RECV, 2,0);

	return 0;
}

//----------------------------------------------------------------------------
//!	\brief	check it is Full Emv Transaction
//----------------------------------------------------------------------------
BOOL CFinan::FullEmvTransacation()
{
	if (m_TRREC.TranCode == TRAN_PURCHASE_VOID || m_TRREC.TranCode == TRAN_REFUND_VOID)	
	{
		if ( m_pLogTRREC->CardType == CARD_DEBIT)
			return TRUE;

		if( m_pLogTRREC->TranCode == TRAN_PURCHASE )
			return TRUE;

		return FALSE;

	}

	if (m_TRREC.CardType == CARD_DEBIT)
		return TRUE;

	if(m_TRREC.TranCode == TRAN_PURCHASE || m_TRREC.TranCode == TRAN_PREAUTH)
		return TRUE;

	return FALSE;
}

//----------------------------------------------------------------------------
//!	\brief	Do Parital Txn
//----------------------------------------------------------------------------
void CFinan::DoParitalTxn()
{
	ProcessTran();
}

//----------------------------------------------------------------------------
//!	\brief	Select Chequing or savings account
//----------------------------------------------------------------------------
int CFinan::SelectAccount()
{
	DWORD size;
	int timer = 30;
	CDataFile::Read(FID_CFG_CUSTENTRY_TMO, (BYTE*) &timer, &size);

	GoToLevel(EMV_SELECT_ACCOUNT);
	if (WAIT_TIMEOUT == WaitForSingleObject(m_hEvent,timer*1000))
		return KEY_TIMEOUT;

	if (m_iKeyReturn == KEY_BUT1)	// chequing
	{
		m_TRREC.AcctType = ACCT_CHEQUE;
		m_iKeyReturn = KEY_ENTER;
	}
	else if (m_iKeyReturn == KEY_BUT2)	// savings
	{
		m_TRREC.AcctType = ACCT_SAVING;
		m_iKeyReturn = KEY_ENTER;
	}	
	return m_iKeyReturn;
}

//-----------------------------------------------------------------------------
//!	\Set Key Return value
//!	\param	Key - new key value
//-----------------------------------------------------------------------------
void CFinan::SetKeyReturn(int Key)
{
	m_iKeyReturn = Key;
	SetEvent(m_hEvent);
}

//----------------------------------------------------------------------------
//!	\brief	Select Language
//----------------------------------------------------------------------------
int CFinan::SelectLanguage()
{
	DWORD size;
	int timer = 30;
	CDataFile::Read(FID_CFG_CUSTENTRY_TMO, (BYTE*) &timer, &size);

	GoToLevel(EMV_SELECT_LANGUAGE);
	if (WAIT_TIMEOUT == WaitForSingleObject(m_hEvent,timer*1000))
		return KEY_TIMEOUT;
	else
		return m_iKeyReturn;
}

//----------------------------------------------------------------------------
//!	\brief	Show Cash back Dialog
//----------------------------------------------------------------------------
int CFinan::ShowCashbackDlg()
{
	m_bCancelPrint = TRUE;
	if (m_TRREC.CardType == CARD_CREDIT || m_TRREC.TranCode == TRAN_REFUND)
		return KEY_ENTER;

	TCHAR buf[10]={0}; 
	if (CDataFile::Read(L"CASHBACK", buf) && CString(buf) == L"Off")
		return KEY_ENTER;
	return KEY_ENTER;
}

//----------------------------------------------------------------------------
//!	\brief	Show surcharge Dialog
//----------------------------------------------------------------------------
int CFinan::ShowSurchargeDlg()
{
	m_bCancelPrint = TRUE;
	if (m_TRREC.CardType == CARD_CREDIT || m_TRREC.TranCode == TRAN_REFUND)
		return KEY_ENTER;


	TCHAR buf[10]={0}; 
	TCHAR buf1[10] = L"";
	if ( CheckAmount(m_strCashback) && CDataFile::Read(L"CASHBACK", buf) && CString(buf) == L"On")
	{
		CDataFile::Read(L"CASHBACKFEE",buf1);
		if ( !CheckAmount(CString(buf1)))
			return KEY_ENTER;
	}
	else
	{
		if ( CDataFile::Read(L"SURCHARGE", buf) && CString(buf) == L"Off")
			return KEY_ENTER;
		CDataFile::Read(L"SURCHARGEFEE",buf1);
		if ( !CheckAmount(CString(buf1)))
			return KEY_ENTER;
	}
	return KEY_ENTER;			
}

//----------------------------------------------------------------------------
//!	\brief	check card is blocked or not
//----------------------------------------------------------------------------
BOOL CFinan::CheckNOTBlocked()
{
	int index;
	int iEnd;
	CString str;
	str.Format(L"[AID]%s[LABEL]%s",CString(m_TRREC.EmvData.AID),CString(m_TRREC.EmvData.ApplicationLabel));
	index = g_strAppData.Find(str);
	if (index == -1)
	{
		str.Format(L"[AID]%s[",CString(m_TRREC.EmvData.AID));
		index = g_strAppData.Find(str);
		if (index == -1)
			return FALSE;
		str.Format(L"[AID]%s",CString(m_TRREC.EmvData.AID));
	}

	iEnd = g_strAppData.Find(L"[END]",index+str.GetLength());
	if (iEnd == -1)
		return FALSE;
	int index1 = g_strAppData.Find(L"[BLOCK]",index+str.GetLength());
	if( index1 != -1 && index1 < iEnd)
	{
		return FALSE;
	}
	return TRUE;
}

//----------------------------------------------------------------------------
//!	\brief	check card is Primary or not
//!	\param	str - primary data string
//----------------------------------------------------------------------------
BOOL CFinan::IsPrimary(CString str)
{
	if (g_GotoSelectApp)
		return TRUE;
	int index;
	int iEnd;
	int iCurrent = g_strAppData.Find(str);
	if (iCurrent == -1)
		return FALSE;

	int istart = 0;

	BOOL bPrimary = FALSE;
	while(1)
	{
		index = g_strAppData.Find(L"[AID]",istart);
		if (index == -1)
			break;
		if (index == iCurrent)
		{
			istart = index + str.GetLength();
			continue;
		}

		iEnd = g_strAppData.Find(L"[END]",index);
		if (iEnd == -1)
			break;

		int temp1 = g_strAppData.Find(L"[SFI]",index);
		if (temp1 >0 && temp1 < iEnd)
		{
			istart = iEnd;
			continue;
		}

		temp1 = g_strAppData.Find(L"[DF62:",index);
		if (temp1 == -1 || temp1 >= iEnd)
			bPrimary = TRUE;
		else
		{
			if (g_strAppData.Mid(temp1+9,2) == L"80")
				bPrimary = TRUE;
		}
		if ( bPrimary )
		{
			temp1 = g_strAppData.Find(L"[BLOCK]",index);
			if (temp1 >0 && temp1 < iEnd)
			{
				m_Blocked = TRUE;
				return TRUE;
			}
		}
	}
	return FALSE;
}

//----------------------------------------------------------------------------
//!	\brief	Check DF62 value
//----------------------------------------------------------------------------
BOOL CFinan::CheckDF62()
{
	if (g_GotoSelectApp)
		return TRUE;
	int index;
	int iEnd;
	CString str;
	str.Format(L"[AID]%s[LABEL]%s",CString(m_TRREC.EmvData.AID),CString(m_TRREC.EmvData.ApplicationLabel));
	index = g_strAppData.Find(str);
	if (index == -1)
	{
		str.Format(L"[AID]%s[",CString(m_TRREC.EmvData.AID));
		index = g_strAppData.Find(str);
		if (index == -1)
			return FALSE;
		str.Format(L"[AID]%s",CString(m_TRREC.EmvData.AID));
	}

	iEnd = g_strAppData.Find(L"[END]",index+str.GetLength());
	if (iEnd == -1)
		return FALSE;

	BOOL isCAN = TRUE;
	int iTemp = g_strAppData.Find(L"[5F56:CAN]",index+str.GetLength());
	if (iTemp == -1 || iTemp > iEnd)
		isCAN = FALSE;

	int index1 = g_strAppData.Find(L"[DF62:",index+str.GetLength());
	if( index1 != -1 && index1 < iEnd)
	{
		CString temp1 = g_strAppData.Mid(index1+6,2);
		CString temp = g_strAppData.Mid(index1+9,2);


		if (temp == L"80")
			return TRUE;
		else if (temp == L"40" )
		{
			if (IsPrimary(str))
			{
				if ( m_Blocked )
					return FALSE;
			}
			else
				return TRUE;
		}
	}
	else
	{
		return TRUE;
	}
	return FALSE;
}

//----------------------------------------------------------------------------
//!	\brief	Show select language Dialog
//----------------------------------------------------------------------------
int CFinan::ShowSelectLanguageDlg()
{
	int index;
	int iEnd;
	CString str;
	CString strLang = L"";

	if (!g_LanguageSelected)
	{
		g_LanguageSelected = TRUE;	

		str.Format(L"[AID]%s[LABEL]%s",CString(m_TRREC.EmvData.AID),CString(m_TRREC.EmvData.ApplicationLabel));

		index = g_strAppData.Find(str);
		if (index == -1)
		{
			str.Format(L"[AID]%s[",CString(m_TRREC.EmvData.AID));
			index = g_strAppData.Find(str);
			if (index == -1)
				return KEY_CANCEL;
			str.Format(L"[AID]%s",CString(m_TRREC.EmvData.AID));
		}

		iEnd = g_strAppData.Find(L"[END]",index+str.GetLength());
		if (iEnd == -1)
			return KEY_CANCEL;


		int index1 = g_strAppData.Find(L"[5F2D:",index+str.GetLength());
		if( index1 != -1 && index1 < iEnd)
		{
			int temp = g_strAppData.Find(L"]",index1);
			if( temp != -1 && temp-index1-6>4)
				return KEY_CANCEL;

			strLang = g_strAppData.Mid(index1+6,temp-index1-6);
		}

		if (strLang == L"")	// no language on card
		{
			int key, language;

			key = CDisplay::SelectLanguage();
			if (key != KEY_CANCEL && key != KEY_TIMEOUT && key != KEY_CARD_REMOVED)
			{ 														
				if (key == KEY_BUT1)
					language = ENGLISH;
				else
					language = FRENCH;
				CLanguage::SetLanguage(language);
			}
			else
			{
				return key;
			}
		}
		else
		{
			int iEn = strLang.Find(L"en");
			int iFr = strLang.Find(L"fr");
			if (iEn>=0 && iFr>=0 )	// both found
			{
			}
			else if (iEn>=0 && iFr==-1 )	// English found
			{
				CLanguage::SetLanguage(ENGLISH);
			}
			else if (iEn == -1 && iFr>=0 )	// French found
			{
				CLanguage::SetLanguage(FRENCH);
			}
			else
			{
				int key, language;

				key = CDisplay::SelectLanguage();
				Sleep(100);
				if (GetScrStatus() == SCR_CARD_ABSENT)
				{
					return KEY_CARD_REMOVED;
				}
				if (key != KEY_CANCEL && key != KEY_TIMEOUT && key != KEY_CARD_REMOVED)
				{ 														
					if (key == KEY_BUT1)
						language = ENGLISH;
					else
						language = FRENCH;
					CLanguage::SetLanguage(language);
				}
				else
				{
					return key;
				}
			}
		}
	}
	return KEY_ENTER;
}

//----------------------------------------------------------------------------
//!	\brief	Show EMV process Dialog
//----------------------------------------------------------------------------
int CFinan::EMVDialog() 
{
	CDisplay::m_bStopDisplay = TRUE;
	int temp = 1;
	CDataFile::Save(FID_CFG_EMVCARD_CHECK, (BYTE *)&temp, sizeof(int));
	GoToLevel(CHIP_DEBITSELECTACCOUNT);

	if(strlen(m_TRREC.Amount)==0)
		PutAmount(m_TRREC.Amount,m_strAmount);

	if(strlen(m_TRREC.TotalAmount)==0)
		strcpy(m_TRREC.TotalAmount,m_TRREC.Amount);

	m_TRREC.HostTranCode = m_TRREC.TranCode;

	int temp1 = 0;
	if (WAIT_TIMEOUT == WaitForSingleObject(m_hEvent, INFINITE))//180*1000))
		temp1 = KEY_TIMEOUT;
	else
	{
		temp1 = m_iKeyReturn;
		if(temp1 == KEY_CANCEL)
		{
			DWORD size;
			temp = 0;
			CDataFile::Read((int)FID_CFG_EMVCARD_CHECK, (BYTE *)&temp, &size);

			if (temp == 2)
				temp1 = KEY_CARD_REMOVED;
		}
	}

	temp = 0;
	CDataFile::Save(FID_CFG_EMVCARD_CHECK, (BYTE *)&temp, sizeof(int));

	CDisplay::m_bStopDisplay = FALSE;

	return temp1;

}

//----------------------------------------------------------------------------
//!	\brief	Set EMV be Cancelled
//----------------------------------------------------------------------------
void CFinan::SetEMVCancel()
{
	if( EMV_PIN_MODE)
	{
		EMV_PIN_MODE = FALSE;
		return;
	}

	m_bCancelFlag = TRUE;
	m_TRREC.EmvError = EMV_ERROR_CANCEL; 
	Sleep(1);
}

//----------------------------------------------------------------------------
//!	\brief	check EMV be Cancelled flag
//----------------------------------------------------------------------------
BOOL CFinan::IsCancel()
{
	Sleep(1);
	return m_bCancelFlag;
}

//----------------------------------------------------------------------------
//!	\brief	EMV Transaction Flow
//!	\param	pTxn - point to TXNCONDITION data
//! RETURNS:       TRUE    = process ok.                               
//!                FALSE    = process failed.
//----------------------------------------------------------------------------
BOOL CFinan::EmvTransactionFlow(TXNCONDITION *pTxn)
{
	DWORD ret = 0;
	BOOL error = TRUE;
	HostInProgress = FALSE;
	BYTE byData[1024];
	WORD wLen = 1024;
	BYTE byTags[100];
	BYTE onlineStatus = 0x00;	
	m_TRREC.EmvError = EMV_NO_ERROR;
	CByteStream TLV_Buff;
	BYTE amountBuff[6];
	declinedOffline = FALSE;
	CString strTemp = L"";

	m_Blocked = FALSE;
	TRACE(TEXT("=============== EmvTransactionFlow ======================\n"));
	CAppData::bEMVError = FALSE;

	ret = OpenScr();
	if (ret)
	{
		m_TRREC.bFallback = TRUE;
		switch(ret)
		{
		case SCR_DEV_NOT_READY:
			m_TRREC.EmvError = 100; 
			break;
		case SCR_DEV_NOT_EXIST:
			m_TRREC.EmvError = 101; 
			break;
		case SCR_DEV_NOT_OPEN:
			m_TRREC.EmvError = 102; 
			break;
		case SCR_INVALID_PARAMS:
			m_TRREC.EmvError = 103; 
			break;
		case SCR_OPT_TIMEOUT:
			m_TRREC.EmvError = 104; 
			break;
		default:
			m_TRREC.EmvError = 105;
			break;
		}
		CDataFile::UpdateStatistics(1);
		return error;
	}

	ret = PowerOnScr();
	if (ret)
	{
		m_TRREC.bFallback = TRUE;
		switch(ret)
		{
		case 1://NO RESPONSE
		case 2://POWER FAIL
		case 3://DATA READY
		case 4://PPS NEGOTIATED
		case 5://CARD REMOVED
		case 6://PROTOCOL ERR
		case 7://PARITY_ERR
		case 8://CT_STATUS
		case 9://DEV OFFLINE
		case 10://DEV ONLINE
		case 11://TIMEOUT
		case 12://WAIT FAILED
			m_TRREC.EmvError = 200 + ret;
			break;
		}
		CDataFile::UpdateStatistics(1);
		return error;
	}

	memcpy(&TXN, pTxn, sizeof(TXNCONDITION));
	//TXN.byTxnTypeCode = 0x53;	// 9C for CDOL2 defined by L3
	//	TXN.byDebug = 1; //R19

	m_TRREC.EmvData.EmvState = EMV_STATE_START;
	if (ret = EmvStart(&TXN))
	{
		m_TRREC.bFallback = TRUE;
		m_TRREC.EmvError = ret; // value between OxF101 to 0xF53E (page 55 in SAIO EMV L2 API.pdg
		CDataFile::UpdateStatistics(1);
		goto _ERROREXIT;
	}

	m_iEMVStart = TRUE;
	g_LanguageSelected = FALSE;
	g_AppSelected = FALSE;
	g_PinTimeOut = FALSE;
	g_GotoSelectApp = FALSE; 
	g_NotCheckRevData = FALSE;
	g_AppConfirmed = FALSE;
	g_AppBlocked = FALSE;
	g_IsPosTerminal = TRUE;
	g_bSetPrimaryKey = 0;
	g_bWithPrimaryCandidates = 0;
	g_iCandidates = 0;
	g_PrimaryBlocked = FALSE;
	g_NoPrimary = FALSE;
	g_TotalApplication = 0;
	//	memset(g_DataBackup,0,256);

	g_strAppData = L"";

	WCHAR tmp[50];
	memset(tmp, 0, 100);
	wsprintf(tmp, L"Transaction %i", TXN.wTxnSN);
	logger.TxnLog(tmp);

	// pull card out at this point and ScrGetState() doesn't detect SCR_CARD_ABSENT
	if (GetScrStatus() == SCR_CARD_ABSENT)
	{
		m_TRREC.EmvError = EMV_ERROR_CARD_ABSENT;
		goto _ERROREXIT;
	}

	TRACE(TEXT("=============== Application Selection ======================\n"));
	m_TRREC.EmvData.EmvState = EMV_STATE_APP_SELECTION;

	// pull card out at this point, it takes 6 secs for function to return with 
	// EMVERR_ICC_COMM_ERROR


	if (ret = EmvApplicationSelection()) //R19
	{
		if (GetScrStatus() == SCR_CARD_ABSENT)
		{
			m_TRREC.EmvError = EMV_ERROR_CARD_ABSENT;
			goto _ERROREXIT;
		}
		TRACE(TEXT("EmvApplicationSelection FAILED\n"));
		TRACE(L"ret=%X\n", ret);
		m_TRREC.EmvError = ret; // value between OxF101 to 0xF53E (page 55 in SAIO EMV L2 API.pdg
		if (g_PrimaryBlocked)
			m_TRREC.EmvError = EMVERR_APPLICATION_NOTSUPPORT;
		goto _ERROREXIT;
	}

	g_NotCheckRevData = TRUE;
	memset(byData, 0, 1024);
	wLen = 1024;
	memcpy(byTags, "\x9F\x06\x50\x57\x9F\x12\x9F\x17", 8);
	EmvGetParam(byTags, 8, byData, &wLen);
	ProcessEmvTLV(wLen, byData, HostInProgress);

	//TRACE(L"%s\n",g_strAppData);
	if (IsCancel())
	{
		goto _ERROREXIT;
	}

	if (GetScrStatus() == SCR_CARD_ABSENT)
	{
		m_TRREC.EmvError = EMV_ERROR_CARD_ABSENT;
		goto _ERROREXIT;
	}

	if (!CheckNOTBlocked())
	{
		g_PrimaryBlocked = TRUE;
		m_TRREC.EmvError = EMVERR_APPLICATION_NOTSUPPORT;
		goto _ERROREXIT;
	}

	if (!CheckDF62())
	{
		m_TRREC.EmvError = EMVERR_APPLICATION_NOTSUPPORT;
		goto _ERROREXIT;
	}


	if( !g_bWithPrimaryCandidates && g_GotoSelectApp)
	{
		m_TRREC.EmvError = EMVERR_APPLICATION_NOTSUPPORT;
		goto _ERROREXIT;
	}

	//Select language
	int dialog = ShowSelectLanguageDlg();
	if (dialog == KEY_CANCEL || dialog == KEY_TIMEOUT)
	{
		m_TRREC.EmvError = EMV_ERROR_CANCEL; 
		goto _ERROREXIT;
	}
	else if (dialog == KEY_CARD_REMOVED)
	{
		m_TRREC.EmvError = EMV_ERROR_CARD_ABSENT;
		goto _ERROREXIT;
	}

	if (GetScrStatus() == SCR_CARD_ABSENT)
	{
		m_TRREC.EmvError = EMV_ERROR_CARD_ABSENT;
		goto _ERROREXIT;
	}

	strTemp = L"";
	if (strlen(m_TRREC.EmvData.ApplicationPreferredName) > 0 &&
		STR_IsAlphanumericNoSpecialChars(m_TRREC.EmvData.ApplicationPreferredName, strlen(m_TRREC.EmvData.ApplicationPreferredName)))
	{
		strTemp = CString(m_TRREC.EmvData.ApplicationPreferredName);
	}
	else if (strlen(m_TRREC.EmvData.ApplicationLabel) > 0)
	{
		memset(m_TRREC.EmvData.ApplicationPreferredName, 0, sizeof(m_TRREC.EmvData.ApplicationPreferredName));
		strTemp = CString(m_TRREC.EmvData.ApplicationLabel);
	}
	else
	{
		memset(m_TRREC.EmvData.ApplicationPreferredName, 0, sizeof(m_TRREC.EmvData.ApplicationPreferredName));
	}

	if (strTemp.GetLength() > 0)
	{
		CDisplay::Clear();
		CDisplay::SetText(2, strTemp);//m_TRREC.EmvData.ApplicationLabel);
		CDisplay::SetText(3, CLanguage::IDX_SELECTED);
		CDisplay::DisplayWait(2);
	}
	else
	{
		m_TRREC.EmvError = EMVERR_APPLICATION_NOTSUPPORT;
		goto _ERROREXIT;
	}

	CDisplay::ClearScreen();
	CDisplay::SetText(2, CLanguage::IDX_EMV_PLEASE_WAIT);
	CDisplay::Display();


	SetEMVCardType();
	if ( m_TRREC.CardType == CARD_DEBIT && (m_TRREC.TranCode == TRAN_PREAUTH || m_TRREC.TranCode == TRAN_FORCE_POST))
	{
		m_TRREC.EmvError = EMV_DEBIT_NOT_SUPPORT;
		goto _ERROREXIT;
	}


	if ( m_TRREC.CardType == CARD_DEBIT && m_TRREC.TranCode == TRAN_REFUND)
		CDisplay::PassToCustomer();

	if (GetScrStatus() == SCR_CARD_ABSENT)
	{
		m_TRREC.EmvError = EMV_ERROR_CARD_ABSENT;
		goto _ERROREXIT;
	}

	STR_LongToBCDString(atol(m_TRREC.TotalAmount), 6, amountBuff);
	TLV_Buff.AddTLV(0x9F02, 6, amountBuff);
	EmvSetParam(TLV_Buff.GetBuffer(), TLV_Buff.GetLength());

	TRACE(TEXT("=============== Initiate Application =======================\n"));
	m_TRREC.EmvData.EmvState = EMV_STATE_INIT_APP;

	// pull card out at this point, it takes 6 secs for function to return with 
	// EMVERR_ICC_COMM_ERROR
	if (GetScrStatus() == SCR_CARD_ABSENT)
	{
		m_TRREC.EmvError = EMV_ERROR_CARD_ABSENT;
		goto _ERROREXIT;
	}

	if (ret = EmvInitiateApplication())
	{
		TRACE(TEXT("EmvInitiateApplication FAILED\n"));
		TRACE(L"ret=%X\n", ret);

		m_TRREC.EmvError = ret; // value between OxF101 to 0xF53E (page 55 in SAIO EMV L2 API.pdg
		if (ret == EMVERR_STOP_TRANSACTION)
			goto _ERROREXIT;
		memset(byData, 0, 1024);
		wLen = 1024;
		memcpy(byTags, "\x4F\x50\x57", 3);
		EmvGetParam(byTags, 3, byData, &wLen);
		ProcessEmvTLV(wLen, byData, HostInProgress);
		goto _ERROREXIT;
	}

	if (IsCancel())
	{
		goto _ERROREXIT;
	}

	if (GetScrStatus() == SCR_CARD_ABSENT)
	{
		m_TRREC.EmvError = EMV_ERROR_CARD_ABSENT;
		goto _ERROREXIT;
	}

	dialog = EMVDialog();
	if (dialog == KEY_CANCEL || dialog == KEY_TIMEOUT)
	{
		m_TRREC.EmvError = EMV_ERROR_CANCEL; 
		goto _ERROREXIT;
	}
	else if (dialog == KEY_CARD_REMOVED)
	{
		m_TRREC.EmvError = EMV_ERROR_CARD_ABSENT;
		goto _ERROREXIT;
	}

	if (GetScrStatus() == SCR_CARD_ABSENT)
	{
		m_TRREC.EmvError = EMV_ERROR_CARD_ABSENT;
		goto _ERROREXIT;
	}

	TRACE(TEXT("=============== Read Application Data ======================\n"));
	m_TRREC.EmvData.EmvState = EMV_STATE_READ_APP_DATA;
	// pull card out at this point, it takes 6 secs for function to return with 
	// EMVERR_ICC_COMM_ERROR

	Sleep(500);
	if (ret = EmvReadApplicationData())
	{
		TRACE(TEXT("EmvReadApplicationData FAILED\n"));
		TRACE(L"ret=%X\n", ret);
		m_TRREC.EmvError = ret; // value between OxF101 to 0xF53E (page 55 in SAIO EMV L2 API.pdg
		memset(byData, 0, 1024);
		wLen = 1024;
		memcpy(byTags, "\x4F\x50\x57", 3);
		EmvGetParam(byTags, 3, byData, &wLen);
		ProcessEmvTLV(wLen, byData, HostInProgress);
		goto _ERROREXIT;
	}

	// Check to see if not full EMV transaction, returm
	if (!FullEmvTransacation())	
	{
		memset(byData, 0, 1024);
		wLen = 1024;
		memcpy(byTags, "\x4F\x50\x57\x9f\x33", 5);
		EmvGetParam(byTags, 5, byData, &wLen);
		ProcessEmvTLV(wLen, byData, HostInProgress);
		m_TRREC.EmvError = EMV_PARTIAL_TXN;
		goto _ERROREXIT;
	}
	else
	{
		if (m_pLogTRREC !=  NULL)
		{
			memset(byData, 0, 1024);
			wLen = 1024;
			memcpy(byTags, "\x4F\x50\x57\x9f\x33", 5);
			EmvGetParam(byTags, 5, byData, &wLen);
			ProcessEmvTLV(wLen, byData, HostInProgress);

			if (strcmp(m_pLogTRREC->Track2, m_TRREC.Track2) != 0 ||
				strcmp(m_pLogTRREC->EmvData.AID, m_TRREC.EmvData.AID) != 0)
			{
				m_TRREC.EmvError = EMV_CARD_NOT_MATCH;
				goto _ERROREXIT;
			}
			else
			{
				memcpy((void*)&m_TRREC, (void*)m_pLogTRREC, sizeof(m_TRREC));
				if (m_TRREC.TranCode == TRAN_REFUND)
					m_TRREC.TranCode = TRAN_REFUND_VOID;
				else 
					m_TRREC.TranCode = TRAN_PURCHASE_VOID;

				memcpy(m_TRREC.EmvData.AuthorizationResponseCode, "05", 2);
			}
		}
	}

	if (IsCancel())
	{
		goto _ERROREXIT;
	}

	if (GetScrStatus() == SCR_CARD_ABSENT)
	{
		m_TRREC.EmvError = EMV_ERROR_CARD_ABSENT;
		goto _ERROREXIT;
	}

	TRACE(TEXT("=============== Offline Data Authentication ================\n"));
	m_TRREC.EmvData.EmvState = EMV_STATE_OFFLINE_DATA_AUTH;

	// pull card out at this point, it takes 12 secs for function to return with 
	// EMVERR_ICC_COMM_ERROR
	// For some other cards, it returns no error.

	ret = EmvOfflineDataAuthentication();
	if (ret)
	{
		TRACE(TEXT("EmvOfflineDataAuthentication FAILED\n"));
		TRACE(L"ret=%X\n", ret);
		m_TRREC.EmvError = ret; // value between OxF101 to 0xF53E (page 55 in SAIO EMV L2 API.pdg
		memset(byData, 0, 1024);
		wLen = 1024;
		memcpy(byTags, "\x4F\x50\x57\x95", 4);
		EmvGetParam(byTags, 4, byData, &wLen);
		ProcessEmvTLV(wLen, byData, HostInProgress);
		goto _ERROREXIT;
	}

	if (IsCancel())
	{
		goto _ERROREXIT;
	}

	if (GetScrStatus() == SCR_CARD_ABSENT)
	{
		m_TRREC.EmvError = EMV_ERROR_CARD_ABSENT;
		goto _ERROREXIT;
	}

	TRACE(TEXT("=============== Processing Restrictions ====================\n"));
	m_TRREC.EmvData.EmvState = EMV_STATE_PROCESS_RESTRICTION;

	// pull card out at this point, it takes 12 secs for function to return with 
	// EMVERR_ICC_COMM_ERROR

	if (ret = EmvProcessingRestrictions())
	{
		TRACE(TEXT("EmvProcessingRestrictions FAILED\n"));
		TRACE(L"ret=%X\n", ret);
		m_TRREC.EmvError = ret; // value between OxF101 to 0xF53E (page 55 in SAIO EMV L2 API.pdg
		memset(byData, 0, 1024);
		wLen = 1024;
		memcpy(byTags, "\x4F\x50\x57\x95", 4);
		EmvGetParam(byTags, 4, byData, &wLen);
		ProcessEmvTLV(wLen, byData, HostInProgress);
		goto _ERROREXIT;
	}

	if (IsCancel())
	{
		goto _ERROREXIT;
	}

	if (GetScrStatus() == SCR_CARD_ABSENT)
	{
		m_TRREC.EmvError = EMV_ERROR_CARD_ABSENT;
		goto _ERROREXIT;
	}

	TRACE(TEXT("=============== Cardholder Verification ====================\n"));
	m_TRREC.EmvData.EmvState = EMV_STATE_CARD_HOLDER_VERIFICATION;

	// pull card out at this point, it takes 12 secs for function to return with 
	// EMVERR_ICC_COMM_ERROR

	if (ret = EmvCardholderVerification())
	{
		TRACE(TEXT("EmvCardholderVerification FAILED\n"));
		TRACE(L"ret=%X\n", ret);
		m_TRREC.EmvError = ret; // value between OxF101 to 0xF53E (page 55 in SAIO EMV L2 API.pdg
		memset(byData, 0, 1024);
		wLen = 1024;
		memcpy(byTags, "\x4F\x50\x57\x95", 4);
		EmvGetParam(byTags, 4, byData, &wLen);
		ProcessEmvTLV(wLen, byData, HostInProgress);

		if ( g_PinTimeOut )
			m_TRREC.EmvError = EMV_ERROR_TIMEOUT;

		goto _ERROREXIT;
	}

	if (IsCancel())
	{
		goto _ERROREXIT;
	}

	if (GetScrStatus() == SCR_CARD_ABSENT)
	{
		m_TRREC.EmvError = EMV_ERROR_CARD_ABSENT;
		goto _ERROREXIT;
	}
	TRACE(TEXT("=============== Terminal Risk Management ===================\n"));
	m_TRREC.EmvData.EmvState = EMV_STATE_TERM_RISK_MANAGEMENT;

	// pull card out at this point, it takes 6 secs for function to return with 
	// EMVERR_ICC_COMM_ERROR

	if (ret = EmvTerminalRiskManagement())
	{
		TRACE(TEXT("EmvTerminalRiskManagement FAILED\n"));
		TRACE(L"ret=%X\n", ret);
		m_TRREC.EmvError = ret; // value between OxF101 to 0xF53E (page 55 in SAIO EMV L2 API.pdg
		memset(byData, 0, 1024);
		wLen = 1024;
		memcpy(byTags, "\x4F\x50\x57\x95", 4);
		EmvGetParam(byTags, 4, byData, &wLen);
		ProcessEmvTLV(wLen, byData, HostInProgress);
		goto _ERROREXIT;
	}

	if (IsCancel())
	{
		goto _ERROREXIT;
	}

	if (GetScrStatus() == SCR_CARD_ABSENT)
	{
		m_TRREC.EmvError = EMV_ERROR_CARD_ABSENT;
		goto _ERROREXIT;
	}

	TRACE(TEXT("=============== Terminal Action Analysis ===================\n"));
	BYTE result = 0;
	m_TRREC.EmvData.EmvState = EMV_STATE_TERM_ACTION_ANALYSIS;

	// pull card out at this point, it takes 6 secs for function to return with 
	// EMVERR_ICC_COMM_ERROR

	if (ret = EmvTerminalActionAnalysis(&result))
	{
		TRACE(TEXT("EmvTerminalActionAnalysis FAILED\n"));
		TRACE(L"ret=%X\n", ret);
		m_TRREC.EmvError = ret; // value between OxF101 to 0xF53E (page 55 in SAIO EMV L2 API.pdg
		memset(byData, 0, 1024);
		wLen = 1024;
		memcpy(byTags, "\x4F\x50\x57\x95\x9B\x9F\x27", 7);
		EmvGetParam(byTags, 7, byData, &wLen);
		ProcessEmvTLV(wLen, byData, HostInProgress);
		goto _ERROREXIT;
	}

	// 0X00 - approved offline
	// 0X01 - declined offline
	// 0X02 - send online for an authorization
	if (result == 2) 	// ARQC Process for Reference
	{
		if (IsCancel())
		{
			goto _ERROREXIT;
		}

		if (GetScrStatus() == SCR_CARD_ABSENT)
		{
			m_TRREC.EmvError = EMV_ERROR_CARD_ABSENT;
			goto _ERROREXIT;
		}

		TRACE(TEXT("=============== Online Processing ==========================\n"));
		memset(byData, 0, 1024);
		wLen = 1024;
		memcpy(byTags, "\x4F\x50\x57\x82\x84\x95\x9A\x9C\x5F\x2A\x5F\x34\x9B\x9F\x02\x9F\x06\x9F\x09\x9F\x10\x9F\x1A\x9F\x21\x9F\x26\x9F\x27\x9F\x33\x9F\x34\x9F\x35\x9F\x36\x9F\x37\x9F\x39", 41);		
		if (EmvGetParam(byTags, 41, byData, &wLen) != 0)
			goto _ERROREXIT;//assert(FALSE);

		if (wLen >= 1024)
			goto _ERROREXIT; //assert(FALSE);

		ProcessEmvTLV(wLen, byData, HostInProgress);

		HostInProgress = TRUE;

		ProcessTran();		

		// 0x00 : fail to online
		// 0x01 : Online OK
		if (m_TRREC.ComStatus == ST_OK)
		{
			onlineStatus = 0x01;
		}		

		if (GetScrStatus() == SCR_CARD_ABSENT)
		{
			m_TRREC.EmvError = EMV_ERROR_CARD_ABSENT;
			goto _ERROREXIT;
		}

		// Fill in ARPC DATA...
		AUTHRESPONSE response;
		memset(&response, 0, sizeof(AUTHRESPONSE));

		FillAuthResponse(&TXN, &m_TRREC.EmvData, &response);

		m_TRREC.EmvData.EmvState = EMV_STATE_ONLINE_PROCESSING;

		// pull card out at this point, it takes 6 secs for function to return with 
		// EMVERR_ICC_COMM_ERROR

		if (ret = EmvOnlineProcessing(onlineStatus, &response))
		{
			TRACE(TEXT("EmvOnlineProcessing FAILED\n"));
			TRACE(L"ret=%X\n", ret);
			m_TRREC.EmvError = ret; // value between OxF101 to 0xF53E (page 55 in SAIO EMV L2 API.pdg
			// Get EMV Tags
			memset(byData, 0, 1024);
			wLen = 1024;
			memcpy(byTags, "\x9F\x26\x9F\x27\x9F\x33\x9F\x34\x9F\x35\x9F\x36\x9F\x37", 14);
			EmvGetParam(byTags, 14, byData, &wLen);
			ProcessEmvTLV(wLen, byData, HostInProgress);
			goto _ERROREXIT;
		}
	}
	else
	{
		declinedOffline = TRUE;
		CDataFile::UpdateStatistics(3);
	}

	if (IsCancel())
	{
		goto _ERROREXIT;
	}

	if (GetScrStatus() == SCR_CARD_ABSENT)
	{
		m_TRREC.EmvError = EMV_ERROR_CARD_ABSENT;
		goto _ERROREXIT;
	}

	TRACE(TEXT("=============== Completion =================================\n"));
	m_TRREC.EmvData.EmvState = EMV_STATE_COMPLETION;

	// pull card out at this point, it takes 6 secs for function to return with 
	// EMVERR_ICC_COMM_ERROR

	if (!declinedOffline)
	{
		if (ret = EmvCompletion(&result) )
		{
			TRACE(TEXT("EmvCompletion FAILED\n"));
			TRACE(L"ret=%X\n", ret);

			// Get EMV Tags
			if (ret != EMVERR_STOP_TRANSACTION)
			{
				m_TRREC.EmvError = ret; // value between OxF101 to 0xF53E (page 55 in SAIO EMV L2 API.pdg
				memset(byData, 0, 1024);
				wLen = 1024;
				memcpy(byTags, "\x95\x9F\x26\x9F\x27\x9F\x33\x9F\x34\x9F\x35\x9F\x36\x9F\x37", 15);
				EmvGetParam(byTags, 15, byData, &wLen);
				ProcessEmvTLV(wLen, byData, HostInProgress);
				goto _ERROREXIT;
			}
		}
	}

	// get EMV data to fill pFinan->m_TRREC
	if (result & 0x01)
	{
		strcpy(m_TRREC.HostRespText, "CARD DECLINED");
		switch(m_TRREC.TranStatus)
		{
		case ST_APPROVED:
			{
				memset(byData, 0, 1024);
				wLen = 1024;
				memcpy(byTags, "\x9F\x10\x9F\x27\x9F\x33\x9F\x34\x9F\x35\x9F\x36\x9F\x37", 14);
				EmvGetParam(byTags, 14, byData, &wLen);
				ProcessEmvTLV(wLen, byData, HostInProgress);
			}

			m_TRREC.TranStatus = ST_CARD_DECL_HOST_APPR;
			strcpy(m_TRREC.HostRespText, CLanguage::GetCharText(CLanguage::IDX_DECLINED_BY_CARD));//"DECLINED BY CARD");
			//if (!m_bTraining)
			//{
			//	m_HOST.SetReversal(CHIP_REVERSAL);
			//	m_HOST.FormatReversal();
			//	m_HOST.ProcessReversals();
			//}

			m_TRREC.bReversal = TRUE;
			break;
		case ST_DECLINED:
		default:
			if (declinedOffline)
				m_TRREC.TranStatus = ST_CARD_DECLINE;
			else if (m_TRREC.ComStatus == ST_OK)
				m_TRREC.TranStatus = ST_CARD_DECL_HOST_DECL;
			else if (m_TRREC.ComStatus == ST_RESPONSE_ERROR)
				m_TRREC.TranStatus = ST_CARD_DECL_BAD_HOST;
			else
				m_TRREC.TranStatus = ST_CARD_DECL_NO_HOST;

			break;
		}
		TRACE(TEXT("=============== DECLINED =================================\n"));
	}
	else
	{
		TRACE(TEXT("=============== APPROVED =================================\n"));
	}

	// Get EMV Tags
	memset(byData, 0, 1024);
	wLen = 1024;

	if (declinedOffline)
	{
		CDataFile::DelOfflineData();
		memcpy(byTags, "\x57\x5A\x82\x8A\x8E\x95\x9A\x9B\x9C\x5F\x2A\x5F\x34\x9F\x03\x9F\x02\x9F\x10\x9F\x1A\x9F\x26\x9F\x27\x9F\x34\x9F\x36\x9F\x37\xDF\x14\xDF\x15\xDF\x16", 37);		
		EmvGetParam(byTags, 37, byData, &wLen);
		CDataFile::Save( FID_OFFLINE_DE55, byData, (DWORD)wLen);

		BYTE temp = m_TRREC.CardType==CARD_DEBIT?1:0;
		DWORD temp1 = 1;
		CDataFile::Save( FID_OFFLINE_DEBIT, &temp, temp1);
	}
	else
	{
		memcpy(byTags, "\x8E\x95\x9B\x9F\x21\x9F\x26\x9F\x27\x9F\x33\x9F\x34\x9F\x35\x9F\x36\x9F\x37", 19);
		EmvGetParam(byTags, 19, byData, &wLen);
	}
	ProcessEmvTLV(wLen, byData, HostInProgress);

	if (declinedOffline)
	{
		CDataFile::Save( FID_OFFLINE_PAN, (BYTE*)m_TRREC.Account, strlen(m_TRREC.Account));
		CDataFile::Save( FID_OFFLINE_PAN_SEQ, (BYTE*)m_TRREC.EmvData.ApplicationPANSequenceNumber, 1);
	}

	// Error exit 
_ERROREXIT:
	EmvStop();
	PowerOffScr();

	if (GetScrStatus() == SCR_CARD_ABSENT)
	{
		m_TRREC.EmvError = EMV_ERROR_CARD_ABSENT;
	}
	if (m_TRREC.EmvError != EMV_NO_ERROR)
	{
		return error;
	}

	return FALSE;
}

//----------------------------------------------------------------------------
//!	\brief	Process Tran
//----------------------------------------------------------------------------
void CFinan::ProcessTran()
{
	if(m_TRREC.CardType != CARD_DEBIT && (m_TRREC.TranCode == TRAN_PURCHASE || m_TRREC.TranCode == TRAN_PREAUTH))
	{
		CDisplay::PassToClerk();
	}

	if (strlen(m_TRREC.InvoiceNo) == 0)
		GetInvoice();

	m_TRREC.HostTranCode = m_TRREC.TranCode;
	m_TRREC.bRequestedPIN = CDisplay::GetPINOK();

	//m_HOST.m_hWnd = m_MsgWnd;
	m_RapidConnect->m_hWnd = m_MsgWnd;
	//m_HOST.Start(&m_TRREC, &m_CONFIG,FALSE);
	m_RapidConnect->Start(&m_TRREC, &m_CONFIG,FALSE);

}

//----------------------------------------------------------------------------
//!	\brief	EMV Fill Auth Response
//!	\param	pTxn - point to TXNCONDITION data
//!	\param	pEMVData - point to TRAN_EmvData data
//!	\param	pAuthResp - point to AUTHRESPONSE data
//----------------------------------------------------------------------------
void CFinan::FillAuthResponse(TXNCONDITION *pTxn, TRAN_EmvData *pEMVData, 
							  AUTHRESPONSE *pAuthResp)
{
	char data[100];
	DWORD size;

	memcpy(pAuthResp->byAcquirerIdentifier, "\x00\x00\x00\x47\x61\x73", 6);
	memcpy(pAuthResp->byAmountAuthorized, pTxn->byTransAmount, 6);

	//BYTE byAuthorizationCode[6];					// 89 Authorisation Code 	
	pAuthResp->byARCLen = 2;
	if (m_TRREC.TranStatus == ST_APPROVED)
		memcpy(pEMVData->AuthorizationResponseCode, "00", 2);

	memcpy(pAuthResp->byAuthorizationResponseCode, pEMVData->AuthorizationResponseCode, pAuthResp->byARCLen);

	pAuthResp->byIssuerAuthenticationDataLength = pEMVData->IssuerAuthenticationDataLen;
	memcpy(pAuthResp->byIssuerAuthenticationData, pEMVData->IssuerAuthenticationData, 
		pAuthResp->byIssuerAuthenticationDataLength);

	pAuthResp->wIssuerScriptLength = pEMVData->IssuerScriptDataLen;
	memcpy(pAuthResp->byIssuerScript, pEMVData->IssuerScriptData, pAuthResp->wIssuerScriptLength);

	CDataFile::Read(FID_MERCH_CFG_TERM_ID, (BYTE*) data, &size);
	memcpy(pAuthResp->byTermIdentifier, data, size);

	STR_PackData(pEMVData->TransactionDate,m_TRREC.TranDate,6);
	STR_PackData(pEMVData->TransactionTime,m_TRREC.TranTime,6);
	memcpy(pAuthResp->byTransDate, pEMVData->TransactionDate, 3);
	memcpy(pAuthResp->byTransTime, pEMVData->TransactionTime, 3);
}

//----------------------------------------------------------------------------
//!	\brief	Check Fraud
//!	\param	str - fraud string data
//! RETURNS:       TRUE    = process ok.                               
//!                FALSE    = process failed.
//----------------------------------------------------------------------------
BOOL CFinan::CheckFraud(CString str)
{
	CString temp;
	temp = CString(m_TRREC.Account);
	m_bFraud = (str == temp.Mid(temp.GetLength()-4,4));

	return m_bFraud;
}

//----------------------------------------------------------------------------
//!	\brief	Check expired date
//!	\param	str - expired string data
//! RETURNS:       TRUE    = process ok.                               
//!                FALSE    = process failed.
//----------------------------------------------------------------------------
BOOL CFinan::CheckMMYY(CString str)
{
	int Month = (str.GetAt(0)- '0')*10 + str.GetAt(1)-'0';
	if(Month < 1 || Month > 12)
		return FALSE;

	int Year = (str.GetAt(2)- '0')*10 + str.GetAt(3)-'0';
	CString str1 = CTimeUtil::CurrentDate();
	int curYear = (str1.GetAt(2)- '0')*10 + str1.GetAt(3)-'0';
	int curMonth = (str1.GetAt(5)- '0')*10 + str1.GetAt(6)-'0';

	if( Year < curYear)
		return FALSE;
	else if( Year == curYear)
	{
		if(Month < curMonth)
			return FALSE;
	}

	return TRUE;
}

//----------------------------------------------------------------------------
//!	\brief	Set EMV Entry Mode
//----------------------------------------------------------------------------
BYTE CFinan::SetEMVEntryMode()
{
	BYTE data = 0;
	switch ( m_TRREC.EntryMode )
	{
	case ENTRY_MODE_MANUAL:
		if (m_TRREC.bFallback)
			data = 81;
		else
			data = 1;
		break;
	case ENTRY_MODE_SWIPED:
		data = 2;
		break;
	case ENTRY_MODE_CHIP:
		data = 5;
		break;
	case ENTRY_MODE_CTLS:
		data = 7;
		break;
	case ENTRY_MODE_CTLS_MSR:
		data = 91;
		break;
	}
	return data;
}

//----------------------------------------------------------------------------
//!	\brief	change Byte To String
//!	\param	bBuf - point to BYTE data
//!	\param	Len - BYTE data length
//!	\param	format - string format
//! RETURNS:       string data
//----------------------------------------------------------------------------
CString CFinan::ByteToStr(BYTE *bBuf, int Len,int format)
{
	int index;
	CString str,temp;

	for (index = 0; index < Len; index++)
	{
		if (bBuf[index] != 0x0d && bBuf[index] != 0x0a)
		{
			if( format == 1)
				temp.Format (L"%02X", bBuf[index]);
			else
				temp.Format (L"%c", bBuf[index]);
			str += temp;
		}
	}
	return str;
}
//-----------------------------------------------------------------------------
//!	\Get Txn result for response data
//!	\param	pTxnResult - Txn result data
//-----------------------------------------------------------------------------	
void CFinan::GetTxnResult( TXN_Result* pTxnResult)
{

	if ( strlen(m_TRREC.ServType) == 0)  // not present on a successful card read
		return;

	strcpy(pTxnResult->ServType,m_TRREC.ServType);

	strcpy(pTxnResult->Account,m_TRREC.Account);

	int len = strlen(pTxnResult->Account);
	if ( m_TRREC.CardType == CARD_CREDIT)
	{
		strcpy(pTxnResult->CardType,"C");
		for(int i = 0;i<len-4;i++)
			pTxnResult->Account[i] = '*';
	}
	else if ( m_TRREC.CardType == CARD_DEBIT)
	{
		strcpy(pTxnResult->CardType,"D");
		for(int i = 4;i<len-4;i++)
			pTxnResult->Account[i] = '*';
	}

	if (pTxnResult->ServType[1] == 0)
	{
		pTxnResult->ServType[1] = ' ';
		pTxnResult->ServType[2] = 0;
	}

	if ( m_TRREC.CustLang == FRENCH)
		strcpy(pTxnResult->CustLang,"F");
	else
		strcpy(pTxnResult->CustLang,"E");

	if ( m_TRREC.ComStatus == ST_OK )
		strcpy(pTxnResult->InvoiceNo,m_TRREC.InvoiceNo);

	if ( m_TRREC.TranStatus == ST_APPROVED)
		strcpy(pTxnResult->AuthCode,m_TRREC.AuthCode);
	
}
