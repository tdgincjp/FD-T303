#include "StdAfx.h"
#include "emvapi.h"
#include "SaioPinpad.h"
#include "SaioDev.h"
#include "SaioReader.h"
#include "Common.h"
#include "CallBack.h"

#include "..\\defs\\constant.h"
#include "..\\defs\\struct.h"
#include "..\\UI\\language.h"
#include "..\\UI\\display.h"
#include "..\\utils\\StrUtil.h"
#include "..\\utils\\Util.h"
#include "..\\data\\DataFile.h"
#include "..\\utils\\AppAPI.h"

#include "..\\data\\AppData.h"

#include "PinInput.h"
#include "EmvLog.h"
#include "ErrCode.h"

#pragma comment(lib, "SaioPinpad.lib")


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define PIN_INPUT_TIMEOUT   15

#define MAX_CADIDATES 10
typedef CHAR *PCHAR;

extern EmvLog logger;
extern TXNCONDITION TXN;
extern const char EnglishLanguageString[] = "en";
extern const char FrenchLanguageString[] = "fr";

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
static void WriteDebugLog(PCHAR wParam);

//BOOL g_MultiApp;
BOOL g_LanguageSelected;
BOOL g_AppSelected;
BOOL g_AppBlocked;
BYTE g_bWithPrimaryCandidates;
BYTE g_bSetPrimaryKey;
int  g_iCandidates;
BOOL g_AppConfirmed;
BOOL g_GotoSelectApp; //Get Select App event
BOOL g_NotCheckRevData;
BOOL g_PinTimeOut;

char F6Language[5];
BOOL LanguageFound_70;
int  stage_level;

BOOL g_IsPosTerminal;
BOOL g_PrimaryBlocked;
BOOL g_NoPrimary;

CString g_strAppData;
BOOL IsPrimary;

int  g_TotalApplication;
BOOL IsNewAID;

//----------------------------------------------------------------------------
//!	\brief	Process TLV 6F tag
//!	\param	tlvLength - tlv data length
//!	\param	tlv - point to tlv data
//!	\return	0  - processed OK
//!			other - failed to process
//----------------------------------------------------------------------------
WORD ProcessTLV_6F(USHORT tlvLength, BYTE *tlv)
{
	USHORT length, n, tag;
	BYTE *data;
	USHORT tagSize, lenSize;
	char temp[24];
	
	n = tlvLength;
	if ( n < 3)
		return 1;
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
			return 1;
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
			return 1;
		n -= lenSize;
		data += lenSize;
		if (length == 0 || length > n)
		{
			return 1;
		}

		switch (tag)
		{
			case 0x84: //Dedicated File (DF) Name
			case 0x4F: //ADF Name
				{
					memcpy(temp,data,length);
					temp[length] = 0;

					CString str = L"[AID]" + CAppAPI::ByteToStr((BYTE*)temp,length);
					int index = g_strAppData.Find( str );
					if ( index == -1)
					{
						g_strAppData += L"[END]" + str;
						IsNewAID = TRUE;
						g_TotalApplication++;
					}
					else
					{
						IsNewAID = TRUE;

						int iEnd = g_strAppData.Find(L"[END]",index);
						if (iEnd == -1)
							g_strAppData = g_strAppData.Mid(0,index)+L"[END]" + str;
						else
						{
							CString str1 = g_strAppData.Mid(iEnd+5);
							g_strAppData = g_strAppData.Mid(0,index)+str1+L"[END]" + str;
						}
					}
					g_strAppData.Replace(L"[END][END]", L"[END]");
				}
				break;
			case 0x50: //Application Label
				if (!IsNewAID)
					break;
				memcpy(temp,data,length);
				temp[length] = 0;
				g_strAppData += L"[LABEL]" + CString(temp);
				break;
			case 0x87: //Application Priority Indicator
				break;
			case 0x88: //Short File Identifier (SFI)
				if (!IsNewAID)
					break;
				g_strAppData += L"[SFI]";
				g_TotalApplication--;
				break;
			case 0xDF62: //DS ODS Info
				if (length != 2)
					break;
				if (!IsNewAID)
					break;

				if (*(data+1) != 0x80)
					IsPrimary = FALSE;
				sprintf(temp,"%02X %02X",*data,*(data+1));
				g_strAppData += L"[DF62:" + CString(temp) + L"]";
				break;
			case 0x5F56: //Application Label
				if (!IsNewAID)
					break;
				if (length !=3 )
					break;
				memcpy(temp,data,3);
				temp[3]=0;
				g_strAppData += L"[5F56:" + CString(temp) + L"]";
				break;
			case 0x5F2D: //Language
				if (!IsNewAID)
					break;
				if ( length != 2 && length != 4 )
					break;
				memcpy(temp,data,length);
				temp[length]=0;
				g_strAppData += L"[5F2D:" + CString(temp) + L"]";

				if ( stage_level == 2)
					LanguageFound_70 = TRUE;
				else if( stage_level == 1)
					memcpy(F6Language,temp,length);
				break;

			default:
				ProcessTLV_6F(length,data);
				break;
		}

		n -= length;
		data += length;
	}
	return 0;
}

//----------------------------------------------------------------------------
//!	\brief	Process Rev TLV data
//!	\param	tlvLength - tlv data length
//!	\param	tlv - point to tlv data
//!	\return	0  - processed OK
//!			other - failed to process
//----------------------------------------------------------------------------
WORD ProcessRevTLV(USHORT tlvLength, BYTE *tlv)
{
	USHORT length, n, tag;
	BYTE *data;
	USHORT tagSize, lenSize;
	
	BOOL bTemp = FALSE;
	n = tlvLength;

	if ( n == 2 )
	{
		g_PrimaryBlocked = FALSE;
		if ( *tlv == 0x62 && *(tlv+1) == 0x83)
			g_PrimaryBlocked = TRUE;
	}
	if ( n < 14)
		return 1;
	data = tlv;
	for (int i=0; i<tlvLength; i++)
	{			
		TRACE(L"%02X ", tlv[i]);
	}

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
			return 1;
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
			return 1;
		n -= lenSize;
		data += lenSize;
		if (length == 0 || length > n)
		{
			return 1;
		}

		switch (tag)
		{
			case 0x6F:
				memset(F6Language,0,5);
				stage_level = 1;
			case 0x70:
				if ( tag == 0x70)
				{
					LanguageFound_70 = FALSE;
					stage_level = 2;
				}
				IsPrimary = TRUE;
				ProcessTLV_6F(length,data);
				if (IsPrimary) // && IsCanadeCode)
					g_NoPrimary = TRUE;

				if(*(tlv+tlvLength-2) == 0x62 && *(tlv+tlvLength-1) == 0x83)
				{
					g_strAppData += L"[BLOCK]";
					if (IsPrimary && !bTemp)
						g_PrimaryBlocked = TRUE;
				}
				else
				{
					if (IsPrimary)
					{
						g_PrimaryBlocked = FALSE;
						bTemp = TRUE;
					}
				}

				if (stage_level == 2 && !LanguageFound_70)
				{
					if (strlen(F6Language)>=2)
					{
						g_strAppData += L"[5F2D:"+ CString(F6Language) + L"]";
					}
				}
				g_strAppData += L"[END]";
				break;
			default:
				break;
		}

		n -= length;
		data += length;
	}
	return 0;
}

//----------------------------------------------------------------------------
//!	\brief	Process Multi Call Back
//!	\param	wCBType - call back data type
//!	\param	wParam - point to data 
//!	\param	lParam - data length
//!	\return	0  - processed OK
//!			other - failed to process
//----------------------------------------------------------------------------
WORD MultiCallBack(WORD wCBType, WPARAM wParam, LPARAM lParam)
{
	WORD wRet = 0;
	switch(wCBType)
	{
	case CB_SCRSEND:		//Send
		wRet = (WORD)SendSCR((PBYTE)wParam, *((PWORD)lParam));
		logger.WriteLog((PBYTE)wParam, ((PWORD)lParam), 1);
		break;
	case CB_SCRREV:			//Rev
		*((PWORD)lParam) = 0;
		wRet = (WORD)ReceiveSCR((PBYTE)wParam, (PWORD)lParam);
		logger.WriteLog((PBYTE)wParam, ((PWORD)lParam), 0);
		if (!g_NotCheckRevData)
			ProcessRevTLV(*(USHORT*)lParam,(BYTE*)wParam);
		break;
	case CB_DISPLAYMSG:
		wRet = DisplayMsg((BYTE)wParam);
		break;
	case CB_SELECTAPP:		//AppSelect
		wRet = SelectApp((PCANDIDATE_LIST)wParam, (PDWORD)lParam);
		break;
	case CB_TRANAMOUNT:		//TransAmount Count
		wRet = ReadLogInfo(LPVOID(wParam), 0);
		break;
	case CB_EXCEPTION:		//Back Card list
		wRet = ReadLogInfo(LPVOID(wParam), 1);
		break;
	case CB_VOICEREFERRAL:	//offer a chance to attendant to call bank
		wRet = VoiceReferral((PBYTE)wParam, (BYTE*)lParam, 0);
		break;
	case CB_PINENTRY:		//PinEntry
		wRet = PINEntryPro((void*)wParam);
		break;
	case CB_DEBUGINFO:
		logger.WriteStringLog((CHAR*)wParam);
		break;
	}
	return wRet;
}

//----------------------------------------------------------------------------
//!	\brief	Process Select App
//!	\param	pCandidate - point to CANDIDATE_LIST data 
//!	\param	pData - CANDIDATE_LIST index
//!	\return	0  - processed OK
//!			other - failed to process
//----------------------------------------------------------------------------
WORD SelectApp(CANDIDATE_LIST *pCandidate, PDWORD pData)
{
	DWORD dwData = *pData;
	int key=0;
	int i;
	CANDIDATE_LIST *p = pCandidate;
	char *pAppName=NULL;
	WORD ret;
	DWORD size;
	int timer=30;

	WORD nCandidateNum = HIWORD(dwData);	// Input parameter
	WORD numberOfRetry = LOWORD(dwData);	

	g_GotoSelectApp = TRUE;

	CDataFile::Read(FID_CFG_CUSTENTRY_TMO, (BYTE*) &timer, &size);

	g_iCandidates = nCandidateNum;
	if ( g_AppSelected )
	{
		if ( nCandidateNum == 1 && g_AppBlocked)
		{
			g_bWithPrimaryCandidates = 0;
			return 1;
		}
		CDisplay::Clear();
		CDisplay::SetText(2, CDisplay::m_strAppName);
		CDisplay::SetText(3, CLanguage::GetText(CLanguage::IDX_NO_SUPPORTED));
		CDisplay::DisplayWait(3);

		g_AppSelected = FALSE;
	}

	PCANDIDATE_LIST_EX pCandidateList = new CANDIDATE_LIST_EX[nCandidateNum];

	BOOL bShowAPPSelect = FALSE;
	int iTemp = 0;
	BOOL bPrimaryFound = FALSE;
	int index;
	int iEnd;
	BOOL bFlag = FALSE;
	for( i = 0; i< nCandidateNum;i++)
	{
		CString str;

		if ( strlen(p->szAppLabel) != 0)
		{
			str.Format(L"[AID]%s[LABEL]%s",CString(p->szAid),CString(p->szAppLabel));
			bFlag = FALSE;
		}
		else
		{
			str.Format(L"[AID]%s[",CString(p->szAid));
			bFlag = TRUE;
		}

		memcpy(&pCandidateList[i], p++, sizeof(CANDIDATE_LIST));	// Get Next Candidate

		index = g_strAppData.Find(str);
		if (index != -1)
		{
			if(bFlag)
				str.Format(L"[AID]%s",CString(p->szAid));
			iEnd = g_strAppData.Find(L"[END]",index+str.GetLength());
			if (iEnd == -1)
				continue;

			int index1 = g_strAppData.Find(L"[DF62:",index+str.GetLength());
			if( index1 != -1 && index1 < iEnd)
			{
				CString temp = g_strAppData.Mid(index1+6,2);
				pCandidateList[i].POSTerm[0] = CAppAPI::StringAtoI(temp);
				temp = g_strAppData.Mid(index1+9,2);
				pCandidateList[i].POSTerm[1] = CAppAPI::StringAtoI(temp);
			}

			index1 = index+str.GetLength();
			int index2 = g_strAppData.Find(L"[5F56:CAN]",index1);
			if (index2 != -1 && index2 < iEnd)
				pCandidateList[i].bCANADA = TRUE;

			if ( !pCandidateList[i].bCANADA )
				pCandidateList[i].POSTerm[1] = 80;

			index2 = g_strAppData.Find(L"[BLOCK]",index1);
			if (index2 != -1 && index2 < iEnd)
				pCandidateList[i].bBlocked = TRUE;

			index2 = g_strAppData.Find(L"[SFI]",index1);
			if (index2 != -1 && index2 < iEnd)
				memset(pCandidateList[i].POSTerm,0,2);
		}

		if (pCandidateList[i].POSTerm[1] == 80)
			bPrimaryFound = TRUE;

		if (pCandidateList[i].szAppPreName[0] != 0 &&
			  STR_IsAlphanumericNoSpecialChars(pCandidateList[i].szAppPreName, strlen(pCandidateList[i].szAppPreName)))
		{
			pAppName = pCandidateList[i].szAppPreName;
		}
		else
		{
			pAppName = pCandidateList[i].szAppLabel;
		}

		if( strlen(pAppName) > 0 && 
			((pCandidateList[i].POSTerm[1] == 80 && !pCandidateList[i].bBlocked) 
			|| (pCandidateList[i].POSTerm[1] == 40 && !pCandidateList[i].bCANADA && !bPrimaryFound) ))
		{
			iTemp++;
			if ( iTemp > 1)
			{
				bShowAPPSelect = TRUE;
			}
		}
		else
			g_AppBlocked = TRUE;

		if ( pCandidateList[i].byAppPI & 0x80)
			g_AppConfirmed = TRUE;

	}

	if(iTemp == 1 && g_AppConfirmed)
		bShowAPPSelect = TRUE;

	if(iTemp == 0 && nCandidateNum == 1 && !bPrimaryFound)
	{
		if ( pCandidateList[0].POSTerm[0] == 80 && pCandidateList[0].POSTerm[1] == 40 )
		{
			g_AppBlocked = FALSE;
			iTemp = 1;
		}
	}
		
	g_bWithPrimaryCandidates = iTemp;
	if(iTemp == 0)
	{
		delete [] pCandidateList;
		return 1;
	}

	i = 0;
	while (1)
	{
		if ( nCandidateNum > 1 && (pCandidateList[i].POSTerm[1] < 40))
		{
			if (++i >= nCandidateNum)
					i = 0;
			continue;
		}

		if ( pCandidateList[i].POSTerm[1] == 40 && pCandidateList[i].bCANADA && bPrimaryFound)
		{
			if (++i >= nCandidateNum)
					i = 0;
			continue;
		}

		if ( pCandidateList[i].POSTerm[1] == 80 && pCandidateList[i].bBlocked)
		{
			if (++i >= nCandidateNum)
					i = 0;
			continue;
		}
		// get language
		if (!g_LanguageSelected)// && g_IsPosTerminal)
		{
			g_LanguageSelected = TRUE;	// should be cleared when EMV is started
			if (pCandidateList[i].byLanguagePre[0] == 0 || pCandidateList[i].byAppPI & 0x80)//g_AppConfirmed )	// no language on card
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
					break;
				}
			}
			else
			{
				if (strstr((const char *) pCandidateList[i].byLanguagePre, EnglishLanguageString) &&
				    strstr((const char *) pCandidateList[i].byLanguagePre, FrenchLanguageString) )	// both found
				{
				}
				else if (strstr((const char *) pCandidateList[i].byLanguagePre, EnglishLanguageString) &&
				    strstr((const char *) pCandidateList[i].byLanguagePre, FrenchLanguageString) == NULL)	// English found
				{
					CLanguage::SetLanguage(ENGLISH);
				}
				else if (strstr((const char *) pCandidateList[i].byLanguagePre, EnglishLanguageString) == NULL &&
								 strstr((const char *) pCandidateList[i].byLanguagePre, FrenchLanguageString))	// French found
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
						break;
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
						break;
					}
				}
			}
		}

		if( !bShowAPPSelect )
		{
			key = KEY_BUT1;
			break;
		}

		if (pCandidateList[i].szAppPreName[0] != 0 &&
			  STR_IsAlphanumericNoSpecialChars(pCandidateList[i].szAppPreName, strlen(pCandidateList[i].szAppPreName)))
			pAppName = pCandidateList[i].szAppPreName;
		else
			pAppName = pCandidateList[i].szAppLabel;

		if ( strlen(pAppName) > 0)
		{
			if ( iTemp == 1 && pCandidateList[i].byAppPI < 0x80)//!g_AppConfirmed)
			{
				key = KEY_BUT1;
				break;
			}
			CDisplay::Clear();
			CDisplay::SetText(1, L"");//CLanguage::IDX_PURCHASE);	// This should be the transaction type PURCHASE, REFUND.....
			CDisplay::SetText(2, CLanguage::IDX_SELECT);
			CDisplay::SetText(3, pAppName);
			key = CDisplay::DisplayWait(MASK_CANCEL, 
				CLanguage::IDX_YES,
				CLanguage::IDX_NO,
				timer);

			if (key == KEY_BUT1 || key == KEY_CANCEL || key == KEY_TIMEOUT)
				break;

			if( key == KEY_BUT2 && iTemp == 1)
			{
				break;
			}
		}
		else
			key = KEY_CANCEL;

		if (++i >= nCandidateNum)
			i = 0;
	}

	if (key == KEY_BUT1)
	{
		g_AppSelected = TRUE;
		CDisplay::m_strAppName = CString(pAppName);
		*pData = pCandidateList[i].byIndex;	// set pbySelectIndex as selected AP
		ret = 0;
	}
	else
	{
		// Cancel selection
		ret = 1;
	}
	
	delete [] pCandidateList;
	return ret;
}

//----------------------------------------------------------------------------
//!	\brief	call back function
//----------------------------------------------------------------------------
WORD VoiceReferral(BYTE *pbyBuf, BYTE *pbyLen, BYTE byOption)
{
	// 0 : APPROVAL
	// 1 : DECLINE
	return 0;
}

//----------------------------------------------------------------------------
//!	\brief	call back function
//----------------------------------------------------------------------------
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

//----------------------------------------------------------------------------
//!	\brief	call back function
//----------------------------------------------------------------------------
void CALLBACK PinInput()
{
	DisplayPleaseWait();
	::PostMessage( CDisplay::m_hDisplayWnd,ERROR_MESSAGE, 2,7 );
}

/*
	0: Verify APDU Command OK 
	1: pinpad error (Pinpad function returns error). In this case, kernel will continue to try next available CVM method.
	2: Cancel PIN (User Cancel PIN entry or Force Stop Transaction)
	3: ByPass PIN (EMV spec.)
*/
//WORD PINEntryPro(PEPBINFO pPinEntry)
WORD PINEntryPro(void* pPinEntry1)
{
	PEPBINFO pPinEntry = (PEPBINFO)pPinEntry1;
	WORD wRet = PINEntryProcess(pPinEntry->byIsLastTry, pPinEntry->byAmount, 
		strlen(pPinEntry->szPan), (PBYTE)pPinEntry->szPan, pPinEntry->byPinType, 
		pPinEntry->byPubkey, pPinEntry->byPubKeyLen, pPinEntry->dwExponents, 
		pPinEntry->byChallenge, pPinEntry->byPinBlock);

	return wRet;
}

//----------------------------------------------------------------------------
//!	\brief	call back function
//----------------------------------------------------------------------------
WORD PINEntryProcess(BYTE byIsLastTry, BYTE *pbyAmount, BYTE byPanLen, BYTE *pbyPan, 
										 BYTE byType, BYTE *pbyPubKey, BYTE byPubKeyLen, 
										 DWORD dwExponents, BYTE *pbyChallenge, BYTE* pbyPinBlock)
{
	DWORD dwRtn = 0;
	BYTE epb[8];
	memset(epb,0,8);
	
	if(byIsLastTry == 1)
	{
		CDisplay::ClearScreen();
		CDisplay::SetText(2, CLanguage::IDX_LAST_PIN_TRY);
		CDisplay::SetText(3, CLanguage::IDX_DO_NOT_REMOVE);
		CDisplay::SetText(4, CLanguage::IDX_CARD);
		CDisplay::DisplayWait(3);
	}
	// Bypass PIN

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
	
		dwRtn = PinBypassEnable(PIN_BYPASS_DISABLE);

		dwRtn = PinEntry(PIN_DUKPT_KEY0,	// key id
										 4,								// min_pinlen
										 12,							// max_pinlen
										 (char*) bPAN,		// pan
										 bEWK,						// working key
										 (WORD)sizeof(bEWK),		// working key length in bytes
										 PIN_INPUT_TIMEOUT,						// timeout
										 TRUE,						// beep
										 PinInput,				// callback
										 &bPinLen,				// out: pin length
										 bEpb,						// out: epb
										 bKSN);						// out: ksn
						 

		if (dwRtn == PIN_CANCEL_TIMEOUT || dwRtn == PIN_TIMEOUT)
		{
			g_PinTimeOut = TRUE;
			return 2;
		}
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
		dwRtn = PinBypassEnable(PIN_BYPASS_DISABLE);

		dwRtn = PinOffLineVerify(FALSE,			// encipher
														 NULL,			// pubkey
														 0,					// pubkey_len
														 0,					// exponents
														 NULL,			// challenge
														 4,					// min_pinlen
														 12,				// max_pinlen
														 PIN_INPUT_TIMEOUT,					// timeout
														 TRUE,			// beep
														 PinInput);	// callback

		Sleep(500);	
		//EMV_PIN_MODE = TRUE;
		if (dwRtn == PIN_CANCEL_TIMEOUT || dwRtn == PIN_TIMEOUT)
		{
			g_PinTimeOut = TRUE;
			return 2;
		}
		else if (dwRtn != 0)	// Other Errors
			return 1;

		PinClose();
	}
	else if (byType == 3)	// Offline Encryped PIN
	{		
		DWORD dwRtn = PinOpen();

		if (dwRtn)
			return 1;

		dwRtn = PinBypassEnable(PIN_BYPASS_DISABLE);

		dwRtn = PinOffLineVerify(TRUE,					// encipher
														 pbyPubKey,			// pubkey
														 byPubKeyLen,		// pubkey_len
														 dwExponents,		// exponents
														 pbyChallenge,	// challenge
														 4,							// min_pinlen
														 12,						// max_pinlen
														 PIN_INPUT_TIMEOUT,							// timeout
														 TRUE,					// beep
														 PinInput);			// callback
		Sleep(500);	
//		EMV_PIN_MODE = TRUE;
		if (dwRtn == PIN_CANCEL_TIMEOUT || dwRtn == PIN_TIMEOUT)
		{
			g_PinTimeOut = TRUE;
			return 2;
		}
		else if (dwRtn != 0)	// Other Errors
			return 1;

		PinClose();
	}
	return 0;
}

//----------------------------------------------------------------------------
//!	\brief	call back function
//----------------------------------------------------------------------------
WORD DisplayMsg(BYTE byMsgIndex)
{
//TRACE(L"DisplayMsg(BYTE byMsgIndex = %d) \n",byMsgIndex);
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
		break;
	case 9:
		DisplayEnterPIN();
		break;
	case 0x0A:
		DisplayIncorrectPIN();
		break;
	case 0x0B:
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
		break;
	}
	return 0;
}

//----------------------------------------------------------------------------
//!	\brief	call back function
//----------------------------------------------------------------------------
static int DisplayAmount()
{
	char formattedAmount[20];
	CDisplay::ClearScreen();
	CDisplay::SetText(3, formattedAmount);
	CDisplay::Display();
	return 0;
}

//----------------------------------------------------------------------------
//!	\brief	call back function
//----------------------------------------------------------------------------
static int ConfirmAmount()
{
	char formattedAmount[20];
	strcat(formattedAmount, "-OK?");

	CDisplay::ClearScreen();
	CDisplay::SetText(3, formattedAmount);
	DWORD size;
	int timer = 30;
	CDataFile::Read(FID_CFG_CUSTENTRY_TMO, (BYTE*) &timer, &size);

	int ret = CDisplay::DisplayWait(MASK_OK|MASK_CANCEL, 
																  CLanguage::IDX_OK, CLanguage::IDX_CANCEL, 
																  timer);
	if (ret == KEY_CANCEL || ret == KEY_BUT2 || ret == KEY_TIMEOUT)
		return -1;
		
	return 0;																	  
}

//----------------------------------------------------------------------------
//!	\brief	call back function
//----------------------------------------------------------------------------
static int DisplayApproved()
{
	CDisplay::ClearScreen();
	CDisplay::SetText(2, CLanguage::IDX_EMV_APPROVED);
	CDisplay::Display();
	return 0;
}

//----------------------------------------------------------------------------
//!	\brief	call back function
//----------------------------------------------------------------------------
static int DisplayCallYourBank()
{
	return -1;
}

//----------------------------------------------------------------------------
//!	\brief	call back function
//----------------------------------------------------------------------------
static int DisplayCancelOrEnter()
{
	CDisplay::ClearScreen();
	CDisplay::SetText(2, CLanguage::IDX_EMV_CANCEL_OR_ENTER);
	CDisplay::Display();
	return 0;
}

//----------------------------------------------------------------------------
//!	\brief	call back function
//----------------------------------------------------------------------------
static int DisplayCardError()
{
	CDisplay::ClearScreen();
	CDisplay::SetText(2, CLanguage::IDX_EMV_CARD_ERROR);
	CDisplay::Display();
	return 0;
}

//----------------------------------------------------------------------------
//!	\brief	call back function
//----------------------------------------------------------------------------
static int DisplayDeclined()
{
	CDisplay::ClearScreen();
	CDisplay::SetText(2, CLanguage::IDX_EMV_DECLINED);
	CDisplay::Display();
	return 0;
}

//----------------------------------------------------------------------------
//!	\brief	call back function
//----------------------------------------------------------------------------
static int DisplayEnterPIN()
{
	return 0;
}

//----------------------------------------------------------------------------
//!	\brief	call back function
//----------------------------------------------------------------------------
static int DisplayIncorrectPIN()
{
	CDisplay::ClearScreen();
	CDisplay::SetText(2, CLanguage::IDX_EMV_INCORRECT_PIN);
	CDisplay::SetText(3, CLanguage::IDX_DO_NOT_REMOVE);
	CDisplay::SetText(4, CLanguage::IDX_CARD);
	CDisplay::SetPINOK(FALSE);
	CDisplay::DisplayWait(3);

	CDataFile::UpdateStatistics(2);
	return 0;
}

//----------------------------------------------------------------------------
//!	\brief	call back function
//----------------------------------------------------------------------------
static int DisplayNotAccepted()
{
	CDisplay::ClearScreen();
	CDisplay::SetText(2, CLanguage::IDX_EMV_CHIP_ERROR);
	CDisplay::Display();
	return 0;
}

//----------------------------------------------------------------------------
//!	\brief	call back function
//----------------------------------------------------------------------------
static int DisplayPINOk()
{
	CDisplay::ClearScreen();
	CDisplay::SetText(2, CLanguage::IDX_EMV_PIN_OK);
	CDisplay::SetText(3, CLanguage::IDX_DO_NOT_REMOVE);
	CDisplay::SetText(4, CLanguage::IDX_CARD);

	CDisplay::Display();
	CDisplay::SetPINOK(TRUE);
	return 0;
}

//----------------------------------------------------------------------------
//!	\brief	call back function
//----------------------------------------------------------------------------
static int DisplayPleaseWait()
{
	CDisplay::ClearScreen();
	CDisplay::SetText(2, CLanguage::IDX_EMV_PLEASE_WAIT);
	CDisplay::SetText(3, CLanguage::IDX_DO_NOT_REMOVE);
	CDisplay::SetText(4, CLanguage::IDX_CARD);
	CDisplay::Display();
	return 0;
}

//----------------------------------------------------------------------------
//!	\brief	call back function
//----------------------------------------------------------------------------
static int DisplayProcessingError()
{
	CDisplay::ClearScreen();
	CDisplay::SetText(2, CLanguage::IDX_EMV_PROCESSING);
	CDisplay::SetText(2, CLanguage::IDX_EMV_ERROR);
	CDisplay::Display();
	return 0;
}

//----------------------------------------------------------------------------
//!	\brief	call back function
//----------------------------------------------------------------------------
static int DisplayRemoveCard()
{
	if (GetScrStatus() == SCR_POWER_ON)
	{
		CDisplay::ClearScreen();
		CDisplay::SetText(2, CLanguage::IDX_EMV_REMOVE_CARD);
		CDisplay::Display();
	}
	return 0;
}

//----------------------------------------------------------------------------
//!	\brief	call back function
//----------------------------------------------------------------------------
static int DisplayUseChipReader()
{
	CDisplay::ClearScreen();
	CDisplay::SetText(2, CLanguage::IDX_EMV_USE_CHIP_READER);
	CDisplay::Display();
	return 0;
}

//----------------------------------------------------------------------------
//!	\brief	call back function
//----------------------------------------------------------------------------
static int DisplayUseMagStripe()
{
	CDisplay::ClearScreen();
	CDisplay::SetText(2, CLanguage::IDX_EMV_USE_MAGSTRIPE);
	CDisplay::SetText(3, CLanguage::IDX_DO_NOT_REMOVE);
	CDisplay::SetText(4, CLanguage::IDX_CARD);
	CDisplay::Display();
	return 0;
}

//----------------------------------------------------------------------------
//!	\brief	call back function
//----------------------------------------------------------------------------
static int DisplayTryAgain()
{
	CDisplay::ClearScreen();
	CDisplay::SetText(2, CLanguage::IDX_EMV_TRY_AGAIN);
	CDisplay::SetText(3, CLanguage::IDX_DO_NOT_REMOVE);
	CDisplay::SetText(4, CLanguage::IDX_CARD);
	CDisplay::Display();
	return 0;
}

//----------------------------------------------------------------------------
//!	\brief	call back function
//----------------------------------------------------------------------------
static void WriteDebugLog(PCHAR wParam)
{
	char* p = (char*)wParam;
	WORD len = strlen(p);
	logger.WriteLog((PBYTE)p, &len, 2);
}
