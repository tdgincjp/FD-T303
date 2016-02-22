
/* 
 * File:   struch.h
 * Author: tan
 *
 * Created on Sep 20, 2012, 12:00 PM
 */

#ifndef STRUCT_H
#define	STRUCT_H

#include "..\\Utils\\BinUtil.h"
#include "..\\Utils\\TimeUtil.h"
#include "..\\Data\\datafile.h"
#include "constant.h"
#include "CommMessage.h"

#pragma warning(disable: 4996)

#ifdef	__cplusplus
extern "C" {
#endif
/*
typedef struct
{
	int EcrCode;
	int TranCode;
	char TerminalID[SZ_TERMID+1];
	long Amount;
	long Invoice;
	long ApprovalCode;
} ECR_TranDataRec;
*/
typedef struct
{
  BYTE TerminalCapabilities[SZ_TERM_CAPABILITY];
	BYTE AdditionalTerminalCapabilities[SZ_ADD_TERM_CAPABILITY];
	BYTE TerminalCountryCode[SZ_TERM_COUNTRY_CODE];
} CONF_EmvData;

struct CONF_Data
{
	TCHAR TermID[SZ_TERMID+1];
	TCHAR MerchID[SZ_MERCHID+1];
	char DeviceModel[SZ_DEVICE_MODEL+1];
	char MACKey[SZ_KEY+1];
	char PINKey[SZ_KEY+1];
	BOOL EMVEnabled;
	BOOL CTLSEnabled;
	CONF_EmvData EmvData;
	CONF_Data()
	{
		memset(TermID,0,SZ_TERMID+1);
		memset(MerchID, 0, SZ_MERCHID+1);
		memset(DeviceModel,0,SZ_DEVICE_MODEL+1);
		memset(MACKey,0,SZ_KEY+1);
		memset(PINKey,0,SZ_KEY+1);

		DWORD size;

		CDataFile::Read(L"Datawire TID", TermID);
		CDataFile::Read(L"Datawire MID", MerchID);
		CDataFile::Read(FID_CFG_DEVICE_MODEL, (BYTE*) DeviceModel, &size);
		CDataFile::Read(FID_TERM_MAC_KEY, (BYTE*) MACKey, &size);
		CDataFile::Read(FID_TERM_PIN_KEY, (BYTE*) PINKey, &size);
		CDataFile::Read(FID_TERM_EMV_ENABLE_FLG, (BYTE*) &EMVEnabled, &size);
		CDataFile::Read(FID_TERM_CTLS_ENABLE_FLG, (BYTE*) &CTLSEnabled, &size);
		CDataFile::Read(FID_EMV_TERMINAL_CAPABILITIES, (BYTE*) EmvData.TerminalCapabilities, &size);
		CDataFile::Read(FID_EMV_ADDITIONAL_TERMINAL_CAPABILITIES, (BYTE*) EmvData.AdditionalTerminalCapabilities, &size);
		CDataFile::Read(FID_EMV_TERMINAL_COUNTRY_CODE_NUMERIC, (BYTE*) EmvData.TerminalCountryCode, &size);
	}
};

typedef struct _TRAN_EmvData
{
	char AID[SZ_APP_IDENTIFIER+1];								// Application Identifier (84)	- ascii
	char ICC_AID[SZ_APP_IDENTIFIER+1];						// ICC Application Identifier (4F)	- ascii
	char ApplicationLabel[SZ_APPLICATION_LABEL+1];// (50)	- ascii
	char ApplicationPreferredName[SZ_APPLICATION_LABEL+1];	// (9F12)		- ascii
	BYTE TerminalCapabilities[3];	
	BYTE TransactionDate[3];											// (9A)
	BYTE TransactionTime[3];											// (9F21)
	BYTE TransactionType[1];											// (9C)
	BYTE TerminalPINTryTimes[1];													// (9F17)
	BYTE TerminalType[1];													// (9F35)
	BYTE ApplicationCryptogramForRequest[8];			// (9F26)
	BYTE ApplicationCryptogramForResponse[8];			// 
	BYTE ApplicationInterchangeProfile[2];				// (82)
	BYTE ApplicationTransactionCounter[2];				// (9F36)
	BYTE UnpredictableNumber[4];									// (9F37)
	BYTE TerminalVerificationResultsARQC[5];			// (95)
	BYTE TerminalVerificationResultsTC[5];				// (95)
	BYTE TransactionCurrencyCode[2];							// (5F2A)				
	BYTE AmountAuthorizedNumeric[6];							// (9F02)
	BYTE ApplicationPANSequenceNumber[1];					// (5F34)
	BYTE CardholderVerificationMethodResults[3];	// (9F34)
	BYTE ApplicationVersionNumber[2];							// (9F09)
	BYTE ChipConditionCode[1];
	BYTE AuthorizationResponseCode[2];						// (8A)
	BYTE IssuerApplicationData[32];								// (9F10)
  BYTE IssuerScriptData[256];								
  BYTE IssuerScriptDataLen;
  //BYTE IssuerScriptResult71[100];								// (71)
  //BYTE IssuerScriptResult71Len;
  //BYTE IssuerScriptResult72[100];								// (72)
  //BYTE IssuerScriptResult72Len;
  BYTE IssuerAuthenticationData[100];						// (91)
  BYTE IssuerAuthenticationDataLen;
  BYTE IssuerCountryCode[2];										// (5F28)		- Format n 3 - Length 2
  BYTE TransactionStatusInfo[2];								// (9B)			- binary
  BYTE IssuerCodeTableIndex[1];									// (9F11)		- binary
  char PrimaryAccountNumber[SZ_ACCOUNT+1];			// (5A)			- ascii
  BYTE CryptogramInformationData[1];						// (9F27)		- binary
  BYTE DedicatedFilename[16];										// (84)			- binary - Length 5 - 16
  BYTE DedicatedFilenameLen;
  BYTE EmvState;
} TRAN_EmvData;

typedef struct
{
	BYTE C0[1];		// (C0)
	BYTE C1[1];		// (C1)
	BYTE C2[1];		// (C2)
	BYTE C3[1];		// (C3)
	BYTE C7[1];		// (C7)
	BYTE C8[1];		// (C7)
	BYTE C9[1];		// (C7)
} TRAN_CtlsData;

struct TRAN_TranDataRec
{
	USHORT TranCode;
	USHORT HostTranCode;
	ULONG Title;
	char EntryMode;
	BYTE CardType;
	BYTE TenderType;
	BYTE AcctType; // ACCT_CREDIT, ACCT_SAVING, ACCT_CHEQUING
	int CustLang; // customer language
	int TermLang; // terminal language
	BYTE ReversalType;
	BOOL bPartialAuth;
	BOOL bCardPresent;
	BOOL bCtlsTransaction;
	BOOL bEmvEnabled;
	BOOL bEmvTransaction;
	BOOL bEmvCtlsTransaction;
  BOOL bEmvHostInProgress;
	BOOL bChipCard;
	BOOL bFallback;
	int EmvError;
	BOOL bEmvCardRead;
	BOOL bRequestedPIN;
	BOOL bRequestedAmount;
	BOOL bEmvConfirmAmt;
	int CardError;
	BOOL bReversal;
	BOOL bManualAllowed;
	BOOL bCardEntryRetry;
  int ComStatus;
	int TranStatus;

	char TranDate[SZ_TRAN_DATE+1];
	char TranTime[SZ_TRAN_TIME+1];
	char Track1[SZ_TRACK2+1];
	char Track2[SZ_TRACK2+1];
	char Account[SZ_ACCOUNT+1];
	char ExpDate[SZ_EXPIRY_DATE+1];
	char ServCode[SZ_SERVICE_CODE+1];
	char ServType[SZ_SERVICE_TYPE+1];
	char CustName[SZ_CUSTNAME+1];
	char InvoiceNo[SZ_INVOICE_NO+1];
	char Amount[SZ_AMOUNT+1];
	char CashbackAmount[SZ_AMOUNT+1];
	char CashbackFee[SZ_AMOUNT+1];
	char TotalAmount[SZ_AMOUNT+1];
	char AuthAmount[SZ_AMOUNT+1];
	char RequestAmount[SZ_AMOUNT+1];
	char DueAmount[SZ_AMOUNT+1];
	char SequenceNum[SZ_FULL_SEQUENCE_NO+1];
	char AuthCode[SZ_APPROVAL_CODE+1];
	char OrigAuthCode[SZ_APPROVAL_CODE+1];
	char OrigAmount[SZ_AMOUNT+1];
	char SurchargeAmount[SZ_AMOUNT+1];
	char AvailableBalance[SZ_AMOUNT+1];
	char PINBlock[SZ_PIN_BLOCK+1];
	char PINKSN[SZ_PIN_KSN+1];
	char AID[SZ_APP_IDENTIFIER+1];
	char ISORespCode[SZ_ISO_CODE+1];
	char RespCode[SZ_RESPONSE_CODE+1];
	char TipAmount[SZ_AMOUNT+1];
	char ClerkID[SZ_CLERKID_TEXT+1];
	char ReceiptNo[SZ_RECEIPT_TEXT+1];
	char TransmissionNo[SZ_TRANSMISSIONNO_TEXT+1];
	char HostRespText[SZ_RESPONSE_TEXT+1];
	char DateTimeFrom[SZ_DATATIMEFORMAT+1];
	char DateTimeTo[SZ_DATATIMEFORMAT+1];
  sBinRangeData binRecord;
	TRAN_EmvData EmvData; 
	TRAN_CtlsData CtlsData; 

	TRAN_TranDataRec()
	{
		Init();
	}
	void Init()
	{
		memset((void*)&TranCode, 0, sizeof(TRAN_TranDataRec));
		bEmvTransaction = FALSE;
		AcctType = ACCT_CREDIT;	
		bEmvEnabled = TRUE;
		CardType = NO_CARD;
		bEmvConfirmAmt = TRUE;
		bEmvCardRead = FALSE;
		DWORD size;
		CDataFile::Read((int)FID_CFG_LANGUAGE, (BYTE *)&TermLang, &size);
		CustLang = TermLang;

		memcpy(EmvData.AuthorizationResponseCode,"05",2);
		CTimeUtil systemTime;
		systemTime.GetSystemDate(TranDate);
		systemTime.GetSystemTime(TranTime);
	}
};

struct FD_Rec
{
	int FdStatus; 
	int iRespCode;
	bool isCredit;
	bool isRetry;
	char RapidConnect_Payload[SZ_RAPIDCONNECT_MAX_PAYLOAD+1];
	char DataWire_FullRequest[SZ_DATAWIRE_MAX_BUFFER+1];
	char DataWire_FullResponse[SZ_DATAWIRE_MAX_BUFFER+1];
	char OrigAuthDateTime[SZ_DATETIME+1];
	char OrigLocalDateTime[SZ_DATETIME+1];
	char OrigAuthSTAN[SZ_STAN+1];
	char ReferenceNum[SZ_REFERENCE+1];

	FD_Rec()
	{
		Init();
	}

	void Init()
	{
		FdStatus = FD_STATUS_INIT;
		iRespCode = -1;
		isRetry = false;
		memset(RapidConnect_Payload, '\0', SZ_RAPIDCONNECT_MAX_PAYLOAD +1);
		memset(DataWire_FullRequest, '\0', SZ_DATAWIRE_MAX_BUFFER+1);
		memset(DataWire_FullResponse, '\0', SZ_DATAWIRE_MAX_BUFFER+1);
		memset(OrigAuthDateTime, '\0', SZ_DATETIME+1);
		memset(OrigLocalDateTime, '\0', SZ_DATETIME+1);
		memset(OrigAuthSTAN, '\0', SZ_STAN+1);
		memset(ReferenceNum, '\0', SZ_REFERENCE+1);
	}
};

//Added for LogFile // Alex Mckay
struct SAIO_Info
{
	BYTE FWVersion[SZ_INFO_FW_VERSION];
	BYTE HWVersion[SZ_INFO_HW_VERSION];
	BYTE OSBuild[SZ_INFO_OS_BUILD];
	BYTE SNNumber[SZ_INFO_SN_NUMBER];

	//default constructor pulls from memory
	SAIO_Info()
	{
		memset(FWVersion,	0,SZ_INFO_FW_VERSION);
		memset(HWVersion,	0,SZ_INFO_HW_VERSION);
		memset(OSBuild,		0,SZ_INFO_OS_BUILD);
		memset(SNNumber,	0,SZ_INFO_SN_NUMBER);
		DWORD size; 
		CDataFile::Read(FID_INFO_FW_VERSION, (BYTE*) FWVersion, &size);
		CDataFile::Read(FID_INFO_HW_VERSION, (BYTE*) HWVersion, &size);
		CDataFile::Read(FID_INFO_OS_BUILD, (BYTE*) OSBuild, &size);
		CDataFile::Read(FID_INFO_SN_NUMBER, (BYTE*) SNNumber, &size);
	}
	
	void save()
	{
		CDataFile::Save(FID_INFO_FW_VERSION, FWVersion, SZ_INFO_FW_VERSION);
		CDataFile::Save(FID_INFO_HW_VERSION, HWVersion, SZ_INFO_HW_VERSION);
		CDataFile::Save(FID_INFO_OS_BUILD, OSBuild, SZ_INFO_OS_BUILD);
		CDataFile::Save(FID_INFO_SN_NUMBER, SNNumber, SZ_INFO_SN_NUMBER);
	}
};

struct CardPlan
{
	char ServType[SZ_SERVICE_TYPE+1];
	char Name[SZ_CUSTNAME+1];
	CardPlan()
	{
		memset((void*)&ServType, 0, sizeof(CardPlan));
	}
};

struct CardPlanList
{
	CardPlan Card[7];
	CardPlanList()
	{
		strcpy(Card[0].ServType,"AX");
		strcpy(Card[0].Name,"AMEX");
		strcpy(Card[1].ServType,"V");
		strcpy(Card[1].Name,"VISA");
		strcpy(Card[2].ServType,"M");
		strcpy(Card[2].Name,"MASTERCARD");
		strcpy(Card[3].ServType,"DS");
		strcpy(Card[3].Name,"DISCOVER");
		strcpy(Card[4].ServType,"DC");
		strcpy(Card[4].Name,"DINERS CLUB");
		strcpy(Card[5].ServType,"JC");
		strcpy(Card[5].Name,"JCB");
		strcpy(Card[6].ServType,"P0");
		strcpy(Card[6].Name,"PROPRIETARY DEBIT");
	}
};
//CADIDATELIST
typedef struct tag_Candidate_List_Ex
{
	tag_Candidate_List_Ex()
	{
		byIndex = 0;
		byAppPI = 0;
		memset(szAid, 0, 33);
		memset(szAppLabel, 0, 17);
		memset(szAppPreName, 0, 17);
		memset(byLanguagePre, 0, 8);
		byIssuerCTI = 0;
		memset(POSTerm, 80, 2);
		bCANADA = FALSE;
		bBlocked = FALSE;
	}
	
	BYTE byIndex;
	BYTE byAppPI;						// 87 ApplPriorityIndicator
	char szAid[33];
	char szAppLabel[17];				//Tag 50 Application Label
	char szAppPreName[17];				//9F12 ans 1~16 Application Preferred Name
	BYTE byIssuerCTI;					//9F11 IssuerCodeTableIndex
	BYTE byLanguagePre[8];				//5F2D Language preference;
	BYTE POSTerm[2];					//DF62 ABM or POS
	BOOL bCANADA;
	BOOL bBlocked;
}CANDIDATE_LIST_EX, * PCANDIDATE_LIST_EX;
//////////////////////////////////////////////////////////////////////////
#ifdef	__cplusplus
}
#endif

#endif	/* STRUCT_H */

