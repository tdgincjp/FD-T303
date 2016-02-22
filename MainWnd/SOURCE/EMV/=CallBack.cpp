#include "StdAfx.h"
#include "SaioPinpad.h"
#include "SaioDev.h"
#include "Common.h"
#include "..\\defs\\constant.h"
#include "..\\UI\\language.h"
//#include "..\\UI\\display.h"
#include "..\\utils\\StrUtil.h"
#include "..\\utils\\Util.h"
#include "..\\data\\DataFile.h"
#include "PinInput.h"
#include "EmvLog.h"

#pragma comment(lib, "SaioPinpad.lib")


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//extern EmvLog logger;
extern TXNCONDITION TXN;

static int DisplayAmount();
static int ConfirmAmount();
static int DisplayApproved();
static int DisplayCallYourBank();
static int DisplayCancelOrEnter();
static int DisplayCardError();
static int DisplayDeclined();
static int DisplayEnterPIN();
static int DisplayIncorrectPIN();
static int DisplayNotAccepted();
static int DisplayPINOk();
static int DisplayPleaseWait();
static int DisplayProcessingError();
static int DisplayRemoveCard();
static int DisplayUseChipReader();
static int DisplayUseMagStripe();
static int DisplayTryAgain();

WORD SelectApp(BYTE *pbySelectIndex, CANDIDATE_LIST *pCandidate, int nCandidateNum)
{
	WORD ret=0;
/*	BYTE numberOfRetry = *pbySelectIndex;	// Input parameter
	int key, i;
	CANDIDATE_LIST *p = pCandidate;
	WORD ret;
	DWORD size;
	int timer;

	assert(CDataFile::Read(FID_CFG_CUSTENTRY_TMO, (BYTE*) &timer, &size));

	PCANDIDATE_LIST pCandidateList = new CANDIDATE_LIST[nCandidateNum];
	i = 0;
	while (1)
	{
		//for (int i = 0 ; i < nCandidateNum ; i++)
		memcpy(&pCandidateList[i], pCandidate, sizeof(CANDIDATE_LIST));	// Get Next Candidate

		// pCandidateList[i].byIndex			// Application Index for Selection
		// pCandidateList[i].byAppPI			// 87, Application Priority Indicator
		// pCandidateList[i].szApplName		// 9F12 or 50, Candidate Application Name
		// pCandidateList[i].byIssuerCTI	// 9F11, Candidate Code Table
		CDisplay::Clear();
		CDisplay::SetText(1, CLanguage::IDX_SELECT);
		CDisplay::SetText(2, pCandidateList[i].szApplName);
		
		key = CDisplay::DisplayWait(MASK_CANCEL, 
									CLanguage::IDX_YES,
									CLanguage::IDX_NO,
									timer);

		if (key == KEY_BUT1 || key == KEY_CANCEL || key == KEY_TIMEOUT)
		{
			break;
		}

		if (i++ > nCandidateNum)
		{
			i = 0;
			pCandidate = p;
		}
		else
		{	
			pCandidate++;	// Jump to next Candidate
		}					
	}

	if (key == KEY_BUT1)
	{
		*pbySelectIndex = pCandidateList[i].byIndex;	// set pbySelectIndex as selected AP
		TRACE(L"select %s \n", pCandidateList[i].szApplName);
		// Selection OK
		ret = 0;
	}
	else
	{
		// Cancel selection
		ret = 1;
	}

	
	CDisplay::Clear();
	CDisplay::SetText(1, pCandidateList[i].szApplName);
	CDisplay::SetText(2, CLanguage::IDX_SELECTED);
	CDisplay::DisplayWait(0, 300);
		
	delete [] pCandidateList;
*/
	return ret;
}

WORD VoiceReferral(BYTE *pbyBuf, BYTE *pbyLen, BYTE byOption)
{
	// 0 : APPROVAL
	// 1 : DECLINE
	return 0;
}

//nPoint = 1--Send, 0--Rev
WORD ScrRW(BYTE *pbyBuffer, WORD *pwLen, int nPoint)
{
	WORD wRet = 0;
	if (nPoint == 0)
	{
		//Rev
		*pwLen = 0;
		wRet = (WORD)ReceiveSCR(pbyBuffer, pwLen);
	}
	else if (nPoint == 1)
	{
		//Send
		wRet = (WORD)SendSCR(pbyBuffer, *pwLen);
	}
//	logger.WriteLog(pbyBuffer, pwLen, nPoint);

	return wRet;
}

WORD ReadLogInfo(LPVOID lpVoid, WORD wPoint)
{
	//wPoint
	//0	PTRANSRECORD
	//1 PEXCEPTIONFILE
	if (wPoint == 0)	// PTRANSRECORD
	{
		PTRANSRECORD pTxnRecord = (PTRANSRECORD)lpVoid;

		// Information From EMV Kernel
		// pTxnRecord->by5A	    // PAN
		// pTxnRecord->byPanLen // PAN Length
		// pTxnRecord->by5F34   // PAN SN
		// pTxnRecord->by9F02   // Amount
		// pTxnRecord->by9F03   // Amount, Cashback
		// pTxnRecord->by9A     // Trans Date YYMMDD
		// pTxnRecord->by9F21   // Trans Time HHMMSS

		// AP Send Amount to EMV Kernel for Velocity Checking
		// memcpy(pTxnRecord->byAmountCount, (BYTE *)"\x00\x00\x00\x00\x01\x23", 6);
		
		// Record Exist, Kernel will perform Velocity Checking
		// return 0;

		// Record Not Exist, Ignore Checking
		return 1;
	}
	else if (wPoint == 1)
	{
		PEXCEPTIONFILE pException = (PEXCEPTIONFILE)lpVoid;

		// Information From EMV Kernel
		// pException->by5A	    // PAN
		// pException->by5ALen  // PAN Length
		// pException->by5F34   // PAN SN

		// Account is in Black List
		// return 0;

		// Account is not in Black List or Black List does not exist
		return 1;
	}
	return 1;
}

void CALLBACK PinInput()
{
	CPinInput PinDlg;
	PinDlg.DoModal();
}

/*
	0: Verify APDU Command OK 
	1: pinpad error (Pinpad function returns error). In this case, kernel will continue to try next available CVM method.
	2: Cancel PIN (User Cancel PIN entry or Force Stop Transaction)
	3: ByPass PIN (EMV spec.)
*/
WORD PINEntryPro(BYTE byIsLastTry, BYTE *pbyAmount, BYTE byPanLen, BYTE *pbyPan, 
								 BYTE byType, BYTE *pbyPubKey, BYTE byPubKeyLen, 
								 DWORD dwExponents, BYTE *pbyChallenge, BYTE* pbyPinBlock)
{
	DWORD dwRtn = 0;
	BYTE epb[8];
	memset(epb,0,8);

	// Bypass PIN
	//if (byPassPin)
	//	return 3;

 	if (byType == 1) // Online PIN by DUKPT-0 for Reference
 	{
		BYTE bEWK[16] = {0x0D,0x42,0xDA,0x0F,0x8F,0xE0,0xFC,0xF1,0x0D,0x42,0xDA,0x0F,0x8F,0xE0,0xFC,0xF1};
		BYTE bKCV[3] = {0};
		BYTE bEpb[8] = {0};
		BYTE bKSN[10] = {0};

		BYTE bPAN[11] = {0};
		memcpy(bPAN, pbyPan, byPanLen);		// NOTE: pbyPan, not null terminate

		BYTE bPinLen;
		CString strMsg;

		dwRtn = PinOpen();

		if (dwRtn)
			return 1;

		dwRtn = PinEntry(PIN_DUKPT_KEY0,	// key id
										 4,								// min_pinlen
										 12,							// max_pinlen
										 (char*) bPAN,		// pan
										 bEWK,						// working key
										 (WORD)sizeof(bEWK),		// working key length in bytes
										 NULL,						// timeout
										 TRUE,						// beep
										 PinInput,				// callback
										 &bPinLen,				// out: pin length
										 bEpb,						// out: epb
										 bKSN);						// out: ksn
						 

		if (dwRtn == PIN_CANCEL_TIMEOUT)
			return 2;
		else if (dwRtn != 0)	// Other Errors
			return 1;

		PinClose();

		pbyPinBlock = epb;

		// Send EPB - "pbyPinBlock" to Authentication Server ...
 	}
	else if (byType == 2)	// Offline Plaintext PIN
	{		
		DWORD dwRtn = PinOpen();

		if (dwRtn)
			return 1;

		dwRtn = PinOffLineVerify(FALSE,			// encipher
														 NULL,			// pubkey
														 0,					// pubkey_len
														 0,					// exponents
														 NULL,			// challenge
														 4,					// min_pinlen
														 12,				// max_pinlen
														 0,					// timeout
														 TRUE,			// beep
														 PinInput);	// callback

		if (dwRtn == PIN_CANCEL_TIMEOUT)
			return 2;
		else if (dwRtn != 0)	// Other Errors
			return 1;

		PinClose();
	}
	else if (byType == 3)	// Offline Encryped PIN
	{		
		DWORD dwRtn = PinOpen();

		if (dwRtn)
			return 1;

		dwRtn = PinOffLineVerify(TRUE,					// encipher
														 pbyPubKey,			// pubkey
														 byPubKeyLen,		// pubkey_len
														 dwExponents,		// exponents
														 pbyChallenge,	// challenge
														 4,							// min_pinlen
														 12,						// max_pinlen
														 0,							// timeout
														 TRUE,					// beep
														 PinInput);			// callback
		if (dwRtn == PIN_CANCEL_TIMEOUT)
			return 2;
		else if (dwRtn != 0)	// Other Errors
			return 1;

		PinClose();
	}
	return 0;
}

WORD DisplayMsg(BYTE byMsgIndex)
{
	// Message Table defined in EMV v4.2 Book4 11.2 Standard Message
	/*
	TEXT("AMOUNT"),						//1
	TEXT("AMOUNT OK?"),				//2
	TEXT("APPROVED"),					//3
	TEXT("CALL YOUR BANK"),		//4
	TEXT("CANCEL OR ENTER"),	//5
	TEXT("CARD ERROR"),				//6
	TEXT("DECLINED"),					//7
	TEXT("ENTER AMOUNT"),			//8
	TEXT("ENTER PIN"),				//9
	TEXT("INCORRECT PIN"),		//a
	TEXT("INSERT CARD"),			//b
	TEXT("NOT ACCEPTED"),			//c
	TEXT("PIN OK"),						//d
	TEXT("Please Wait"),			//e
	TEXT("PROCESSING ERROR"),	//f
	TEXT("REMOVE CARD"),			//10
	TEXT("USE CHIP READER"),	//11
	TEXT("USE MAG STRIPE"),		//12
	TEXT("TRY AGAIN"),				//13
	*/
	
	// map these indexes to application index
	switch (byMsgIndex)
	{
	case 1:
		DisplayAmount();
		break;
	case 2:
		ConfirmAmount();
		break;
	case 3:
		DisplayApproved();
		break;
	case 4:
		DisplayCallYourBank();
		break;
	case 5:
		DisplayCancelOrEnter();
		break;
	case 6:
		DisplayCardError();
		break;
	case 7:
		DisplayDeclined();
		break;
	case 8:
		assert(FALSE);
		break;
	case 9:
		DisplayEnterPIN();
		break;
	case 0x0A:
		DisplayIncorrectPIN();
		break;
	case 0x0B:
		assert(FALSE);
		break;
	case 0x0C:
		DisplayNotAccepted();
		break;
	case 0x0D:
		DisplayPINOk();
		break;
	case 0x0E:
		DisplayPleaseWait();
		break;
	case 0x0F:
		DisplayProcessingError();
		break;
	case 0x10:
		DisplayRemoveCard();
		break;
	case 0x11:
		DisplayUseChipReader();
		break;
	case 0x12:
		DisplayUseMagStripe();
		break;
	case 0x13:
		DisplayTryAgain();
		break;
	default:
		assert(FALSE);
		break;
	}
	
	//PostMessage(hWnd, WM_EMV_MSG, 0, (LPARAM)byMsgIndex);
	return 0;
}

static int	DisplayAmount()
{
/*	char formattedAmount[20];
//	UTIL_FormatAmountWithoutComma(atol(TXN.byTransAmount), formattedAmount, '$'); FIXME
	CDisplay::ClearScreen();
	CDisplay::SetText(3, formattedAmount);
	CDisplay::Display();
*/	return 0;
}

static int ConfirmAmount()
{
/*	char formattedAmount[20];
//	UTIL_FormatAmountWithoutComma(TXN.byTransAmount, formattedAmount, '$');
	strcat(formattedAmount, "-OK?");

	CDisplay::ClearScreen();
	CDisplay::SetText(3, formattedAmount);
	DWORD size;
	int timer;
	assert(CDataFile::Read(FID_CFG_CUSTENTRY_TMO, (BYTE*) &timer, &size));

	int ret = CDisplay::DisplayWait(MASK_OK|MASK_CANCEL, 
																  CLanguage::IDX_OK, CLanguage::IDX_CANCEL, 
																  timer);
	if (ret == KEY_CANCEL || ret == KEY_BUT2 || ret == KEY_TIMEOUT)
		return -1;
*/		
	return 0;																	  
}

static int DisplayApproved()
{
//	CDisplay::ClearScreen();
//	CDisplay::SetText(2, CLanguage::IDX_EMV_APPROVED);
//	CDisplay::Display();
	return 0;
}

static int DisplayCallYourBank()
{
	assert(FALSE);
	return -1;
}

static int DisplayCancelOrEnter()
{
//	CDisplay::ClearScreen();
//	CDisplay::SetText(2, CLanguage::IDX_EMV_CANCEL_OR_ENTER);
//	CDisplay::Display();
	return 0;
}

static int DisplayCardError()
{
//	CDisplay::ClearScreen();
//	CDisplay::SetText(2, CLanguage::IDX_EMV_CARD_ERROR);
//	CDisplay::Display();
	return 0;
}

static int DisplayDeclined()
{
//	CDisplay::ClearScreen();
//	CDisplay::SetText(2, CLanguage::IDX_EMV_DECLINED);
//	CDisplay::Display();
	return 0;
}

static int DisplayEnterPIN()
{
//	CDisplay::ClearScreen();
//	CDisplay::SetText(2, CLanguage::IDX_EMV_ENTER_PIN);
//	CDisplay::Display();
	return 0;
}

static int DisplayIncorrectPIN()
{
//	CDisplay::ClearScreen();
//	CDisplay::SetText(2, CLanguage::IDX_EMV_INCORRECT_PIN);
//	CDisplay::Display();
	return 0;
}

static int DisplayNotAccepted()
{
//	CDisplay::ClearScreen();
//	CDisplay::SetText(2, CLanguage::IDX_EMV_CHIP_ERROR);
//	CDisplay::Display();
	return 0;
}

static int DisplayPINOk()
{
//	CDisplay::ClearScreen();
//	CDisplay::SetText(2, CLanguage::IDX_EMV_PIN_OK);
//	CDisplay::Display();
	return 0;
}

static int DisplayPleaseWait()
{
//	CDisplay::ClearScreen();
//	CDisplay::SetText(2, CLanguage::IDX_EMV_PLEASE_WAIT);
//	CDisplay::Display();
	return 0;
}

static int DisplayProcessingError()
{
//	CDisplay::ClearScreen();
//	CDisplay::SetText(2, CLanguage::IDX_EMV_PROCESSING);
//	CDisplay::SetText(2, CLanguage::IDX_EMV_ERROR);
//	CDisplay::Display();
	return 0;
}

static int DisplayRemoveCard()
{
//	CDisplay::ClearScreen();
//	CDisplay::SetText(2, CLanguage::IDX_EMV_REMOVE_CARD);
//	CDisplay::Display();
	return 0;
}

static int DisplayUseChipReader()
{
//	CDisplay::ClearScreen();
//	CDisplay::SetText(2, CLanguage::IDX_EMV_USE_CHIP_READER);
//	CDisplay::Display();
	return 0;
}

static int DisplayUseMagStripe()
{
//	CDisplay::ClearScreen();
//	CDisplay::SetText(2, CLanguage::IDX_EMV_USE_MAGSTRIPE);
//	CDisplay::Display();
	return 0;
}

static int DisplayTryAgain()
{
//	CDisplay::ClearScreen();
//	CDisplay::SetText(2, CLanguage::IDX_EMV_TRY_AGAIN);
//	CDisplay::Display();
	return 0;
}


