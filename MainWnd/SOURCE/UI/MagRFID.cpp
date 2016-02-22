#include "StdAfx.h"
#include "MagRFID.h"
#include "Cmd.h"
#include "SaioReader.h"
#include "..\\utils\\StrUtil.h"
#include "..\\data\\AppData.h"
#include "..\\data\\DataFile.h"
#include "..\\data\\RecordFile.h"
#include "..\\EMV\\EMV.h"
//#include "..\\data\\AppAPI.h"
#include "..\\UTILS\\string.h"
#include "..\\UTILS\\AppAPI.h"

#pragma comment(lib, "Lib\\SaioReader.lib")
#pragma warning(disable: 4244)
#pragma warning(disable: 4018)

#define	PURCHASE_FOR_GOODS		1
#define	PURCHASE_FOR_SERVICE	2
#define	REFUND_TRANSACTION		3
#define RCmdLen 1024

HWND m_hWndRFID;	      // Main Dialog Handle

CMagRFID::CMagRFID(void)
{
}

CMagRFID::~CMagRFID(void)
{
}

/****************************************************************/
void RFIDEvent(DWORD event,void *user_data) //R19
{
	::PostMessage(m_hWndRFID, event, 1, 0);
}

/****************************************************************/
BOOL CMagRFID::SetMasterCardApplication()
{
	CByteStream appData, cmdData;
	BYTE RCmd[RCmdLen];
	DWORD RLen,dwRtn;
	BOOL res = FALSE;
		
	TRACE(TEXT("=============== SetMasterCardApplication =======================\n"));
		
	appData.Empty();

	// Tag 9F06:	Application Identifier (AID) - Terminal
	appData.AddTLV(0x9F06, 7, "\xA0\x00\x00\x00\x04\x10\x10");
			
	// Tag 50: Application Label
	char applicationLabel[17];
	strcpy(applicationLabel, "MasterCard");
	appData.AddTLV(0x50, strlen(applicationLabel), applicationLabel);
			
	// Tag C0: Appl Brand - this will decide which kernel to enter.
	appData.AddTLV(0xC0, 0x01, "\x01");
			
	// Tag DF0B: Appl Index in firmware
	// Appl Index (Tag DF0B): Application index in firmware. Total 6 applications at most can be
	// installed. If the application of some index exists, the old application will be replaced.
	appData.AddTLV(0xDF0B, 0x01, "\x02");

	// Tag DF0C: Appl Scheme Indicator
	//appData.AddTLV(0xDF0C, 0x01, "\x09"); //moblile bit 4= 1
	appData.AddTLV(0xDF0C, 0x01, "\x01");  //mobile off (we MSD)

	// Tag 9F09: Application Version Number ? Please set to 0x0002.
	appData.AddTLV(0x9F09, 0x02, "\x00\x02");
	/*char temp[3];
	BCDtoChar(pCtlsData->emvAppVersion,temp,2);
	temp[2] = 0;
	appData.AddTLV(0x9F09, 0x02, temp);*/

	
	// Tag 9F6D: MagStripe Application Version Number - MasterCard proprietary tag. Please set to 0x0001.
	appData.AddTLV(0x9F6D, 0x02, "\x00\x01");
	//BCDtoChar(pCtlsData->msdAppVersion,temp,2);
	//temp[2] = 0;
	//appData.AddTLV(0x9F6D, 0x02, temp);

	// Tag 9F15: Merchant Category Code
	//appData.AddTLV(0x9F15, 2, "\x00\x11");
	appData.AddTLV(0x9F15, 2, "\x53\x11");	// FIXME

	// Tag 9F1B: Terminal Floor Limit
	appData.AddTLV(0x9F1B, SZ_BIN_EMV_OFFLINE_FLOOR_LIMIT, "\x00\x00\x00\x00");



	// Tag DF8126: Terminal/Reader CVM Required Limit
	//appData.AddTLV(0xDF26, SZ_BIN_CTLS_EMV_CVM_LIMIT, pCtlsData->ctlsCvmLimit);
			
	// 9F1D	Terminal Risk Management Data
	//appData.AddTLV(0x9F1D, 0x08, "\x08\x08\x00\x00\x00\x00\x00\x00");  //Version V1.10

			

	// Tag DF0D: Terminal Action Code (TAC) ? denial
	//appData.AddTLV(0xDF0D, SZ_EMV_TAC, pEmvData->ctlsTACDenial);
	appData.AddTLV(0xDF0D, SZ_EMV_TAC, "\x00\x00\x00\x00\x00");

	// Tag DF0E: Terminal Action Code (TAC) ? online
	//appData.AddTLV(0xDF0E, SZ_EMV_TAC, pEmvData->ctlsTACOnline);
	appData.AddTLV(0xDF0E, SZ_EMV_TAC, "\x00\x00\x00\x00\x00");
			
	// Tag DF0F: Terminal Action Code (TAC) ? default
	//appData.AddTLV(0xDF0F, SZ_EMV_TAC, pEmvData->ctlsTACDefault);
	appData.AddTLV(0xDF0F, SZ_EMV_TAC, "\x00\x00\x00\x00\x00");

	// Tag DF10: Terminal/Reader Contactless Transaction Limit
	//appData.AddTLV(0xDF10, SZ_BIN_CTLS_EMV_CVM_LIMIT, pCtlsData->ctlsTxnLimit);
	//CDataFile::Save(FID_EMV_TAG_MAST_CTLS_TXN_LIMIT,pCtlsData->ctlsTxnLimit,SZ_BIN_CTLS_EMV_CVM_LIMIT);
			
	// Tag DF11: Terminal/Reader Contactless Floor Limit
	//appData.AddTLV(0xDF11, SZ_BIN_CTLS_EMV_CVM_LIMIT, pEmvData->ctlsOfflineFloorLimit);
			
	// Tag DF12: Terminal/Reader CVM Required Limit
	//appData.AddTLV(0xDF12, SZ_BIN_CTLS_EMV_CVM_LIMIT, pCtlsData->ctlsCvmLimit);

	appData.AddTLV(0xDF11, SZ_BIN_CTLS_EMV_CVM_LIMIT, "\x00\x00\x00\x01\x00\x00");
	appData.AddTLV(0xDF10, SZ_BIN_CTLS_EMV_CVM_LIMIT, "\x00\x00\x00\x03\x00\x00");
	appData.AddTLV(0xDF12, SZ_BIN_CTLS_EMV_CVM_LIMIT, "\x00\x00\x00\x00\x10\x00");

	
	// Tag DF19: Transaction Type Indicator - Not support Cashback yet
	appData.AddTLV(0xDF19, 0x01, "\x01");	// suppport Refund

	// For refund, document says do the following....
	//BYTE refundData[] =	"\xE7\x03\x9C\x01\x20\xE8\x09\xDF\x11\x06\x00\x00\x00\x00\x00\x00\xE7\x03\x9C\x01\x20\xE8\x08\xDF\x0D\x05\xFF\xFF\xFF\xFF\xFF";
	//appData.AddTLV(0xE6, 31, refundData);

	// Config Application
	cmdData.Empty();
	cmdData.AddCmdTLV(0x42, appData.GetLength(), appData.GetBuffer());
	memset(RCmd, 0, RCmdLen);
	RLen = RCmdLen;

	/////////// DEBUG ////////////////
	TRACE(TEXT("=== Request ===\n"));
	BYTE *pData = cmdData.GetBuffer();
	int len = cmdData.GetLength();
	for (int i=0; i<len; i++)
	{			
		TRACE(L"%02X ", pData[i]);
	}
	TRACE(L"\n");
	/////////// DEBUG ////////////////

	dwRtn = RfidConfig(cmdData.GetBuffer(), cmdData.GetLength(), RCmd, &RLen);
	//Stupid Hardcode
	//dwRtn = RfidConfig(addMasterCardApp, sizeof(addMasterCardApp), RCmd, &RLen);
	if (dwRtn == 0)
	{
		if (RCmd[0] == 0x42 && RCmd[3] == 0x00)
		{
			res = TRUE;
		}
		else
		{
			TRACE(L"Install APP Mastercard failed.\n");
		}
	}
	else
	{
		TRACE(L"RfidConfig() failed. Error code: %X.\n", dwRtn);
	}

	return res;
}

/****************************************************************/
BOOL CMagRFID::SetVisaApplication(int n, sEmvData *pEmvData, 
																		  sCtlsParams *pCtlsData,
																		  BOOL isEMV)
{
	CByteStream appData, cmdData;
	BYTE RCmd[RCmdLen];
	DWORD RLen,dwRtn;
	BOOL res = FALSE;
	
	TRACE(TEXT("=============== SetVisaApplication =======================\n"));
		
	appData.Empty();

	// Tag 9F06:	Application Identifier (AID) - Terminal
	if (isEMV)
		appData.AddTLV(0x9F06, pEmvData->AIDLength, pEmvData->AID);
	else
		appData.AddTLV(0x9F06, 7, "\xA0\x00\x00\x00\x99\x90\x90");
			
	// Tag 50: Application Label - ??? shouldn't this come from card?
	char applicationLabel[17];
	strcpy(applicationLabel, "Visa");
	appData.AddTLV(0x50, strlen(applicationLabel), applicationLabel);
			
	// Tag C0: Appl Brand - this will decide which kernel to enter.
	appData.AddTLV(0xC0, 0x01, "\x02");

	// Tag DF0B: Appl Index in firmware
	// Appl Index (Tag DF0B): Application index in firmware. Total 6 applications at most can be
	// installed. If the application of some index exists, the old application will be replaced.
	appData.AddTLV(0xDF0B, 0x01, &n);

	// Tag DF0C: Appl Scheme Indicator
	appData.AddTLV(0xDF0C, 0x01, "\x05");

	// Tag 9F09: Application Version Number ? Please set to 0x0105.
//	appData.AddTLV(0x9F09, 0x02, "\x00\x01");

	char temp[3];
	BCDtoChar(pCtlsData->emvAppVersion,temp,2);
	temp[2] = 0;
	appData.AddTLV(0x9F09, 0x02, temp);

	// Tag 9F15: Merchant Category Code
	appData.AddTLV(0x9F15, 2, "\x00\x11");	// FIXME

	// Tag 9F1B: Terminal Floor Limit
	appData.AddTLV(0x9F1B, SZ_BIN_EMV_OFFLINE_FLOOR_LIMIT, "\x00\x00\x00\x00");

	// Tag DF10: Terminal/Reader Contactless Transaction Limit
	appData.AddTLV(0xDF10, SZ_BIN_CTLS_EMV_CVM_LIMIT, pCtlsData->ctlsTxnLimit);
	//CDataFile::Save(FID_EMV_TAG_VISA_CTLS_TXN_LIMIT, pCtlsData->ctlsTxnLimit,SZ_BIN_CTLS_EMV_CVM_LIMIT);
			
	// Tag DF11: Terminal/Reader Contactless Floor Limit
	appData.AddTLV(0xDF11, SZ_BIN_CTLS_EMV_CVM_LIMIT, pEmvData->ctlsOfflineFloorLimit);

	// Tag DF12: Terminal/Reader CVM Required Limit
	appData.AddTLV(0xDF12, SZ_BIN_CTLS_EMV_CVM_LIMIT, pCtlsData->ctlsCvmLimit);
			
	// Tag DF13
	appData.AddTLV(0xDF13, 1, "\x00");

	// Tag DF19: Transaction Type Indicator - Not support Cashback yet
	appData.AddTLV(0xDF19, 0x01, "\x01");	// suppport Refund

	// Config Application
	cmdData.Empty();
	cmdData.AddCmdTLV(0x42, appData.GetLength(), appData.GetBuffer());
	memset(RCmd, 0, RCmdLen);
	RLen = RCmdLen;
	
	dwRtn = RfidConfig(cmdData.GetBuffer(), cmdData.GetLength(), RCmd, &RLen);
	if (dwRtn == 0)
	{
		if (RCmd[0] == 0x42 && RCmd[3] == 0x00)
		{
			res = TRUE;
		}
		else
		{
			TRACE(L"Install APP %X failed.\n", pEmvData->AID);
		}
	}
	else
	{
		TRACE(L"RfidConfig() failed. Error code: %X.\n", dwRtn);
	}
	
	return res;
}

BOOL CMagRFID::SetVisaApplicationMsr()
{
	CByteStream appData, cmdData;
	BYTE RCmd[RCmdLen];
	DWORD RLen,dwRtn;
	BOOL res = FALSE;
	BOOL isEMV = FALSE;
	
	TRACE(TEXT("=============== SetVisaApplication =======================\n"));
		
	appData.Empty();

	// Tag 9F06:	Application Identifier (AID) - Terminal
	//if (isEMV)
		appData.AddTLV(0x9F06, 7, "\xA0\x00\x00\x00\x03\x10\x10");
	//else
	//	appData.AddTLV(0x9F06, 7, "\xA0\x00\x00\x00\x99\x90\x90");
			
	// Tag 50: Application Label - ??? shouldn't this come from card?  - This is where we name the applications we load to the terminal, the card identifies with the AID. 
	char applicationLabel[17];
	strcpy(applicationLabel, "Visa");
	appData.AddTLV(0x50, strlen(applicationLabel), applicationLabel);
			
	// Tag C0: Appl Brand - this will decide which kernel to enter.
	appData.AddTLV(0xC0, 0x01, "\x02");

	// Tag DF0B: Appl Index in firmware
	// Appl Index (Tag DF0B): Application index in firmware. Total 6 applications at most can be
	// installed. If the application of some index exists, the old application will be replaced.
	// For MSR We are hardcoding, MC -2, V - 1, Ax - 3
	appData.AddTLV(0xDF0B, 0x01, "\x01");

	// Tag DF0C: Appl Scheme Indicator
	appData.AddTLV(0xDF0C, 0x01, "\x05");

	// Tag 9F09: Application Version Number ? Please set to 0x0105.
	appData.AddTLV(0x9F09, 0x02, "\x01\x19");

	//char temp[3];
	//BCDtoChar(pCtlsData->emvAppVersion,temp,2);
	//temp[2] = 0;
	//appData.AddTLV(0x9F09, 0x02, temp);

	// Tag 9F15: Merchant Category Code
	appData.AddTLV(0x9F15, 2, "\x00\x11");	// FIXME

	// Tag 9F1B: Terminal Floor Limit
	appData.AddTLV(0x9F1B, SZ_BIN_EMV_OFFLINE_FLOOR_LIMIT, "\x00\x00\x00\x00");

	//// Tag DF10: Terminal/Reader Contactless Transaction Limit
	//appData.AddTLV(0xDF10, SZ_BIN_CTLS_EMV_CVM_LIMIT, pCtlsData->ctlsTxnLimit);
	////CDataFile::Save(FID_EMV_TAG_VISA_CTLS_TXN_LIMIT, pCtlsData->ctlsTxnLimit,SZ_BIN_CTLS_EMV_CVM_LIMIT);
	//		
	//// Tag DF11: Terminal/Reader Contactless Floor Limit
	//appData.AddTLV(0xDF11, SZ_BIN_CTLS_EMV_CVM_LIMIT, pEmvData->ctlsOfflineFloorLimit);

	//// Tag DF12: Terminal/Reader CVM Required Limit
	//appData.AddTLV(0xDF12, SZ_BIN_CTLS_EMV_CVM_LIMIT, pCtlsData->ctlsCvmLimit);
	//		
	// Tag DF13
	appData.AddTLV(0xDF13, 1, "\x00");

	// Tag DF19: Transaction Type Indicator - Not support Cashback yet
	appData.AddTLV(0xDF19, 0x01, "\x01");	// suppport Refund

	// Config Application
	cmdData.Empty();
	cmdData.AddCmdTLV(0x42, appData.GetLength(), appData.GetBuffer());
	memset(RCmd, 0, RCmdLen);
	RLen = RCmdLen;
	
	dwRtn = RfidConfig(cmdData.GetBuffer(), cmdData.GetLength(), RCmd, &RLen);
	if (dwRtn == 0)
	{
		if (RCmd[0] == 0x42 && RCmd[3] == 0x00)
		{
			res = TRUE;
		}
		else
		{
			TRACE(L"Install APP %X failed.\n", "\xA0\x00\x00\x00\x03\x10\x10");
		}
	}
	else
	{
		TRACE(L"RfidConfig() failed. Error code: %X.\n", dwRtn);
	}
	
	return res;
}

/****************************************************************/
BOOL CMagRFID::SetAmexApplication(int n, sEmvData *pEmvData,  sCtlsParams *pCtlsData)
{
	CByteStream appData, cmdData;
	BYTE RCmd[RCmdLen];
	DWORD RLen,dwRtn;
	BOOL res = FALSE;
	
	if ( pCtlsData != NULL )
		return res;
	
	TRACE(TEXT("=============== SetAmexApplication =======================\n"));
		
	appData.Empty();

	// Tag 9F06:	Application Identifier (AID) - Terminal
	appData.AddTLV(0x9F06, pEmvData->AIDLength, pEmvData->AID);
			
	// Tag 50: Application Label - ??? shouldn't this come from card?
	char applicationLabel[17];
	strcpy(applicationLabel, "Amex");
	appData.AddTLV(0x50, strlen(applicationLabel), applicationLabel);
			
	// Tag C0: Appl Brand - this will decide which kernel to enter.
	appData.AddTLV(0xC0, 0x01, "\x03");

	// Tag DF0B: Appl Index in firmware
	// Appl Index (Tag DF0B): Application index in firmware. Total 6 applications at most can be
	// installed. If the application of some index exists, the old application will be replaced.
	appData.AddTLV(0xDF0B, 0x01, &n);

	// Tag DF0C: Appl Scheme Indicator
	appData.AddTLV(0xDF0C, 0x01, "\x01");

	// Tag 9F09: Application Version Number ? Please set to 0x0105.
	appData.AddTLV(0x9F09, 0x02, "\x00\x01");

	// Tag 9F15: Merchant Category Code
	appData.AddTLV(0x9F15, 2, "\x00\x11");	// FIXME

	// Tag 9F1B: Terminal Floor Limit
	appData.AddTLV(0x9F1B, SZ_BIN_EMV_OFFLINE_FLOOR_LIMIT, "\x00\x00\x00\x00");

	// Tag DF10: Terminal/Reader Contactless Transaction Limit
	//appData.AddTLV(0xDF10, SZ_BIN_CTLS_EMV_CVM_LIMIT, pCtlsData->ctlsTxnLimit);
			
	// Tag DF11: Terminal/Reader Contactless Floor Limit
	//appData.AddTLV(0xDF11, SZ_BIN_CTLS_EMV_CVM_LIMIT, pEmvData->ctlsOfflineFloorLimit);
			
	// Tag DF12: Terminal/Reader CVM Required Limit
	//appData.AddTLV(0xDF12, SZ_BIN_CTLS_EMV_CVM_LIMIT, pCtlsData->ctlsCvmLimit);
			
	// Tag DF13
	appData.AddTLV(0xDF13, 1, "\x00");

	// Tag DF19: Transaction Type Indicator - Not support Cashback yet
	appData.AddTLV(0xDF19, 0x01, "\x01");	// suppport Refund

	// Config Application
	cmdData.Empty();
	cmdData.AddCmdTLV(0x42, appData.GetLength(), appData.GetBuffer());
	memset(RCmd, 0, RCmdLen);
	RLen = RCmdLen;
	
	dwRtn = RfidConfig(cmdData.GetBuffer(), cmdData.GetLength(), RCmd, &RLen);
	if (dwRtn == 0)
	{
		if (RCmd[0] == 0x42 && RCmd[3] == 0x00)
		{
			res = TRUE;
		}
		else
		{
			TRACE(L"Install APP %X failed.\n", pEmvData->AID);
		}
	}
	else
	{
		TRACE(L"RfidConfig() failed. Error code: %X.\n", dwRtn);
	}

	
	return res;
}


/****************************************************************/
BOOL CMagRFID::SetAmexApplicationMsr()
{
	CByteStream appData, cmdData;
	BYTE RCmd[RCmdLen];
	DWORD RLen,dwRtn;
	BOOL res = FALSE;
		
	TRACE(TEXT("=============== SetAmexApplicationMsr =======================\n"));
		
	appData.Empty();

	// Tag 9F06:	Application Identifier (AID) - Terminal
	appData.AddTLV(0x9F06, 6, "\xA0\x00\x00\x00\x25\x01");
			
	// Tag 50: Application Label - ??? shouldn't this come from card?
	char applicationLabel[17];
	strcpy(applicationLabel, "Amex");
	appData.AddTLV(0x50, strlen(applicationLabel), applicationLabel);
			
	// Tag C0: Appl Brand - this will decide which kernel to enter.
	appData.AddTLV(0xC0, 0x01, "\x03");

	// Tag DF0B: Appl Index in firmware
	// Appl Index (Tag DF0B): Application index in firmware. Total 6 applications at most can be
	// installed. If the application of some index exists, the old application will be replaced.
	appData.AddTLV(0xDF0B, 0x01, "\x03");

	// Tag DF0C: Appl Scheme Indicator
	appData.AddTLV(0xDF0C, 0x01, "\x01");

	// Tag 9F09: Application Version Number ? Please set to 0x0105.
	appData.AddTLV(0x9F09, 0x02, "\x01\x19");

	// Tag 9F15: Merchant Category Code
	appData.AddTLV(0x9F15, 2, "\x00\x11");	// FIXME

	// Tag 9F1B: Terminal Floor Limit
	appData.AddTLV(0x9F1B, SZ_BIN_EMV_OFFLINE_FLOOR_LIMIT, "\x00\x00\x00\x00");

	// Tag DF10: Terminal/Reader Contactless Transaction Limit
	//appData.AddTLV(0xDF10, SZ_BIN_CTLS_EMV_CVM_LIMIT, pCtlsData->ctlsTxnLimit);
			
	// Tag DF11: Terminal/Reader Contactless Floor Limit
	//appData.AddTLV(0xDF11, SZ_BIN_CTLS_EMV_CVM_LIMIT, pEmvData->ctlsOfflineFloorLimit);
			
	// Tag DF12: Terminal/Reader CVM Required Limit
	//appData.AddTLV(0xDF12, SZ_BIN_CTLS_EMV_CVM_LIMIT, pCtlsData->ctlsCvmLimit);
			
	// Tag DF13
	appData.AddTLV(0xDF13, 1, "\x00");

	// Tag DF19: Transaction Type Indicator - Not support Cashback yet
	appData.AddTLV(0xDF19, 0x01, "\x01");	// suppport Refund

	// Config Application
	cmdData.Empty();
	cmdData.AddCmdTLV(0x42, appData.GetLength(), appData.GetBuffer());
	memset(RCmd, 0, RCmdLen);
	RLen = RCmdLen;
	
	dwRtn = RfidConfig(cmdData.GetBuffer(), cmdData.GetLength(), RCmd, &RLen);
	if (dwRtn == 0)
	{
		if (RCmd[0] == 0x42 && RCmd[3] == 0x00)
		{
			res = TRUE;
		}
		else
		{
			TRACE(L"Install APP %X failed.\n", "\xA0\x00\x00\x00\x25\x01");
		}
	}
	else
	{
		TRACE(L"RfidConfig() failed. Error code: %X.\n", dwRtn);
	}

	
	return res;
}

/****************************************************************/
BOOL CMagRFID::SetInteracApplication(int n, sEmvData *pEmvData, sCtlsParams *pCtlsData)
{
	CByteStream appData, cmdData;
	BYTE RCmd[RCmdLen];
	DWORD RLen, dwRtn;
	BOOL res = FALSE;
	
	TRACE(TEXT("=============== SetInteracApplication =======================\n"));
		
	appData.Empty();

	// Tag 9F06:	Application Identifier (AID) - Terminal
	appData.AddTLV(0x9F06, pEmvData->AIDLength, pEmvData->AID);
			
	// Tag 50: Application Label - ??? shouldn't this come from card?
	char applicationLabel[17];
	memcpy(applicationLabel, "\x49\x6E\x74\x65\x72\x61\x63", 7);	// INTERAC
	appData.AddTLV(0x50, 0x07, applicationLabel);
			
	// Tag C0: Appl Brand - this will decide which kernel to enter.
	appData.AddTLV(0xC0, 0x01, "\x08");

	// Tag DF0B: Appl Index in firmware
	// Appl Index (Tag DF0B): Application index in firmware. Total 6 applications at most can be
	// installed. If the application of some index exists, the old application will be replaced.
	appData.AddTLV(0xDF0B, 0x01, &n);

	// Tag DF0C: Appl Scheme Indicator
	appData.AddTLV(0xDF0C, 0x01, "\x01");

	// DF0D	Terminal Action Code - Denial
	appData.AddTLV(0xDF0D, sizeof(pEmvData->ctlsTACDenial), pEmvData->ctlsTACDenial);
					
	// DF0E	Terminal Action Code - Online
	appData.AddTLV(0xDF0E, sizeof(pEmvData->ctlsTACOnline), pEmvData->ctlsTACOnline);

	// DF0F	Terminal Action Code - Default
	appData.AddTLV(0xDF0F, sizeof(pEmvData->ctlsTACDefault), pEmvData->ctlsTACDefault);
			
	// Tag 9F09: Application Version Number
//	appData.AddTLV(0x9F09, 0x02, "\x00\x02");
	char temp[3];
	BCDtoChar(pCtlsData->emvAppVersion,temp,2);
	temp[2] = 0;
	appData.AddTLV(0x9F09, 0x02, temp);

	// Tag 9F15: Merchant Category Code
	appData.AddTLV(0x9F15, 0x02, "\x01\x01");	// FIXME

	// Tag 9F58:	MTI
//	appData.AddTLV(0x9F58, 0x01, &pEmvData->MTI);

	// Tag 9F59:	TTI
	appData.AddTLV(0x9F59, SZ_EMV_INTERAC_TTI, pCtlsData->interactTTI);

	// Tag 9F1B: Terminal Floor Limit
	// appData.AddTLV(0x9F1B, SZ_BIN_EMV_OFFLINE_FLOOR_LIMIT, "\x00\x00\x00\x00");	// FIXME - put back when XAC has fix
	appData.AddTLV(0x9F1B, SZ_BIN_EMV_OFFLINE_FLOOR_LIMIT, "\x00\x00\x13\x88");

	// Tag DF10: Terminal/Reader Contactless Transaction Limit
	appData.AddTLV(0xDF10, SZ_BIN_CTLS_EMV_CVM_LIMIT, pCtlsData->ctlsTxnLimit);
			
	// Tag DF11: Terminal/Reader Contactless Floor Limit
	appData.AddTLV(0xDF11, SZ_BIN_CTLS_EMV_CVM_LIMIT, pEmvData->ctlsOfflineFloorLimit);

	// Tag DF12: Terminal/Reader CVM Required Limit
	appData.AddTLV(0xDF12, SZ_BIN_CTLS_EMV_CVM_LIMIT, pCtlsData->ctlsCvmLimit);

	// Tag DF13
	//appData.AddTLV(0xDF13, 1, "\x00");

	// Tag DF19: Transaction Type Indicator - Not support Cashback yet
	appData.AddTLV(0xDF19, 0x01, "\x01");	// suppport Refund

	// Tag DF22: Status Check Switch ????
	appData.AddTLV(0xDF22, 0x01, "\x00");	// not sure what this tag is, just follow doc

	// Tag DF23: Taget Percent
	appData.AddTLV(0xDF23, 0x01, &pEmvData->targetPercentage);

	// Tag DF24: Threshold Value
	appData.AddTLV(0xDF24, 0x04, pEmvData->thresholdValue);

	// Tag DF25: Maximum Target Percent
	appData.AddTLV(0xDF25, 0x01, &pEmvData->maximumTargetPercentage);

	// Config Application
	cmdData.Empty();
	cmdData.AddCmdTLV(0x42, appData.GetLength(), appData.GetBuffer());
	memset(RCmd, 0, RCmdLen);
	RLen = RCmdLen;
/*	
	/////////// DEBUG ////////////////
	TRACE(TEXT("=== Request ===\n"));
	BYTE *pData = cmdData.GetBuffer();
	int len = cmdData.GetLength();
	for (int i=0; i<len; i++)
	{			
		TRACE(L"%02X ", pData[i]);
	}
	TRACE(L"\n");
	/////////// DEBUG ////////////////
*/
	dwRtn = RfidConfig(cmdData.GetBuffer(), cmdData.GetLength(), RCmd, &RLen);
	if (dwRtn == 0)
	{
		/////////// DEBUG ////////////////
		TRACE(TEXT("=== Response ===\n"));
		for (int i=0; i<RLen; i++)
		{			
			TRACE(L"%02X ", RCmd[i]);
		}
		TRACE(L"\n");
		/////////// DEBUG ////////////////

		if (RCmd[0] == 0x42 && RCmd[3] == 0x00)
		{
			res = TRUE;
		}
		else
		{
			TRACE(L"Install APP %X failed.\n", pEmvData->AID);
		}
	}
	
	return res;
}

/****************************************************************/
BOOL CMagRFID::SetMasterCardTermCapability(int amount)
{
	if( !CAppData::SupportPaypass )
		return TRUE;

	CByteStream appData, cmdData;
	BYTE RCmd[RCmdLen];
	DWORD RLen,dwRtn;
	BOOL res=FALSE;

	TRACE(TEXT("=============== SetMasterCardTermCapability =======================\n"));
		
	appData.Empty();
	
	// Tag C0: Appl Brand - this will decide which kernel to enter.
	appData.AddTLV(0xC0, 0x01, "\x01");

	// Tag 9F33: Terminal Capability
	BYTE termCap[10];
	DWORD size;
	CDataFile::Read(FID_EMV_TERMINAL_CAPABILITIES, (BYTE*) termCap, &size);

// Compare txn amount with Paypass CVM limit here
// Please call this function before you initiate the ctls txn

	termCap[0] = 0x40;
	if ( amount > GetPaypassCvmLimit())
		termCap[1] = 0x00;
	else
		termCap[1] = 0x08;
	termCap[2] = 0x88;


	appData.AddTLV(0x9F33, size, termCap);

	// Tag DF16???
	appData.AddTLV(0xDF16, 3, "\x60\x68\x00");
	
	// Tag 9F35: Online/Offline Capability
	appData.AddTLV(0x9F35, 1, "\x25");	// online only

	// Tag DF16???
	appData.AddTLV(0xDF16, 1, "\x07");
	
	// Set Config
	cmdData.Empty();
	cmdData.AddCmdTLV(0x63, appData.GetLength(), appData.GetBuffer());
	memset(RCmd, 0, RCmdLen);
	RLen = RCmdLen;
/*
	/////////// DEBUG ////////////////
	TRACE(TEXT("=== Request ===\n"));
	BYTE *pData = cmdData.GetBuffer();
	int len = cmdData.GetLength();
	for (int i=0; i<len; i++)
	{			
		TRACE(L"%02X ", pData[i]);
	}
	TRACE(L"\n");
	/////////// DEBUG ////////////////
*/
	dwRtn = RfidConfig(cmdData.GetBuffer(), cmdData.GetLength(), RCmd, &RLen);
	if (dwRtn == 0)
	{
		if (RCmd[0] == 0x63 && RCmd[3] == 0x00)
		{
			res = TRUE;
		}
		else
		{
			TRACE(TEXT("Set MasterCard Terminal Capabilities failed.\n"));
		}
	}
	else
	{
		TRACE(L"RfidConfig() failed. Error code: %X.\n", dwRtn);
	}
	
	return res;
}

/****************************************************************/
BOOL CMagRFID::SetVisaTermCapabilityMsr()
{
	CByteStream appData, cmdData;
	BYTE RCmd[RCmdLen];
	DWORD RLen,dwRtn;
	BOOL res=FALSE;

	TRACE(TEXT("=============== SetVisaTermCapability =======================\n"));
		
	appData.Empty();
	
	// Tag C0: Appl Brand - this will decide which kernel to enter.
	appData.AddTLV(0xC0, 0x01, "\x02");

	// Tag 9F33: Terminal Capability
	BYTE termCap[10];
	DWORD size;
	CDataFile::Read(FID_EMV_TERMINAL_CAPABILITIES, (BYTE*) termCap, &size);
	appData.AddTLV(0x9F33, size, termCap);
	
	// Tag DF16???
	//appData.AddTLV(0xDF16, 3, "\x60\x70\x00");

	// Tag 9F35: Online/Offline Capability
	appData.AddTLV(0x9F35, 1, "\x25");
	
	// Tag DF16???
	//appData.AddTLV(0xDF16, 1, "\x07");

	// Tag 9F66: 
	//appData.AddTLV(0x9F66, 4, "\xA6\x00\x40\x00"); EMV
	//appData.AddTLV(0x9F66, SZ_EMV_VISA_TTQ, pCtlsData->visaTTQ);
	appData.AddTLV(0x9F66, 4, "\x86\x00\x40\x00"); // MAG

	// Tag DF16???
	appData.AddTLV(0xDF16, 4, "\xA0\x00\x40\x00");
	
	// Set Config
	cmdData.Empty();
	cmdData.AddCmdTLV(0x63, appData.GetLength(), appData.GetBuffer());
	memset(RCmd, 0, RCmdLen);
	RLen = RCmdLen;
	dwRtn = RfidConfig(cmdData.GetBuffer(), cmdData.GetLength(), RCmd, &RLen);
	if (dwRtn == 0)
	{
		if (RCmd[0] == 0x63 && RCmd[3] == 0x00)
		{
			res = TRUE;
		}
		else
		{
			TRACE(TEXT("Set Visa Terminal Capabilities failed.\n"));
		}
	}
	else
	{
		TRACE(L"RfidConfig() failed. Error code: %X.\n", dwRtn);
	}
	
	return res;
}

/****************************************************************/
BOOL CMagRFID::SetAmexTermCapability()
{
	CByteStream appData, cmdData;
	BYTE RCmd[RCmdLen];
	DWORD RLen,dwRtn;
	BOOL res=FALSE;

	TRACE(TEXT("=============== SetAmexTermCapability =======================\n"));
		
	appData.Empty();
	
	// Tag C0: Appl Brand - this will decide which kernel to enter.
	appData.AddTLV(0xC0, 0x01, "\x03");

	// Tag 9F33: Terminal Capability
	//BYTE termCap[10];
	//DWORD size;
	//CDataFile::Read(FID_EMV_TERMINAL_CAPABILITIES, (BYTE*) termCap, &size);
	//appData.AddTLV(0x9F33, size, termCap);
	appData.AddTLV(0x9F33, 3, "\x40\x40\x88");

	// Tag DF16???
	appData.AddTLV(0xDF16, 3, "\x60\x60\x00");
	
	// Tag 9F35: Online/Offline Capability
	appData.AddTLV(0x9F35, 1, "\x25");	// online only

	// Tag DF16???
	appData.AddTLV(0xDF16, 1, "\x07");

	// Tag 9F6D: ExpressPay Terminal Capabilities
	appData.AddTLV(0x9F6D, 1, "\x40");	// MSD only

	// Tag DF16???
	appData.AddTLV(0xDF16, 1, "\xC0");
	
	// Set Config
	cmdData.Empty();
	cmdData.AddCmdTLV(0x63, appData.GetLength(), appData.GetBuffer());
	memset(RCmd, 0, RCmdLen);
	RLen = RCmdLen;
	dwRtn = RfidConfig(cmdData.GetBuffer(), cmdData.GetLength(), RCmd, &RLen);
	if (dwRtn == 0)
	{
		if (RCmd[0] == 0x63 && RCmd[3] == 0x00)
		{
			res = TRUE;
		}
		else
		{
			TRACE(TEXT("Set Amex Terminal Capabilities failed.\n"));
		}
	}
	else
	{
		TRACE(L"RfidConfig() failed. Error code: %X.\n", dwRtn);
	}
	
	return res;
}

/****************************************************************/
BOOL CMagRFID::SetInteracTermCapability()
{
	CByteStream appData, cmdData;
	BYTE RCmd[RCmdLen];
	DWORD RLen,dwRtn;
	BOOL res=FALSE;

	TRACE(TEXT("=============== SetInteracTermCapability =======================\n"));
		
	appData.Empty();
	
	// Tag C0: Appl Brand - Interac = 0x08
	appData.AddTLV(0xC0, 0x01, "\x08");

	// Tag 9F33: Terminal Capability
	BYTE termCap[SZ_TERM_CAPABILITY];
	DWORD size;
	CDataFile::Read(FID_EMV_FLASH_TERMINAL_CAPABILITIES, (BYTE*) termCap, &size);
	appData.AddTLV(0x9F33, size, termCap);
//	appData.AddTLV(0x9F33, 0x03, "\xE0\x98\xC0");
	
	// Tag DF16???
	appData.AddTLV(0xDF16, 0x03, "\x60\x40\x00");

	// Tag 9F35: Online/Offline Capability
	appData.AddTLV(0x9F35, 0x01, "\x24");	// online only

	// Tag DF16???
	appData.AddTLV(0xDF16, 0x01, "\x07");

	// Tag 9F40: Additional Terminal Capability
	BYTE addTermCap[SZ_ADD_TERM_CAPABILITY];
	CDataFile::Read(FID_EMV_ADDITIONAL_TERMINAL_CAPABILITIES, (BYTE*) addTermCap, &size);
	appData.AddTLV(0x9F40, size, addTermCap);
	
	// Tag DF16???
	appData.AddTLV(0xDF16, 0x05, "\x00\x00\x00\x00\x00");

	// Tag 9F59: TTI
	appData.AddTLV(0x9F59, 0x03, "\xC8\x80\x00"); // computed value using host specs

	// Tag DF16???
	appData.AddTLV(0xDF16, 0x03, "\xA0\x00\x00");

	// Set Config
	cmdData.Empty();
	cmdData.AddCmdTLV(0x63, appData.GetLength(), appData.GetBuffer());
	memset(RCmd, 0, RCmdLen);
	RLen = RCmdLen;

	/////////// DEBUG ////////////////
	TRACE(TEXT("=== Request ===\n"));
	BYTE *pData = cmdData.GetBuffer();
	int len = cmdData.GetLength();
	for (int i=0; i<len; i++)
	{			
		TRACE(L"%02X ", pData[i]);
	}
	TRACE(L"\n");
	/////////// DEBUG ////////////////

	dwRtn = RfidConfig(cmdData.GetBuffer(), cmdData.GetLength(), RCmd, &RLen);
	if (dwRtn == 0)
	{
		/////////// DEBUG ////////////////
		TRACE(TEXT("=== Response ===\n"));
		for (int i=0; i<RLen; i++)
		{			
			TRACE(L"%02X ", RCmd[i]);
		}
		TRACE(L"\n");
		/////////// DEBUG ////////////////

		if (RCmd[0] == 0x63 && RCmd[3] == 0x00)
		{
			res = TRUE;
		}
		else
		{
			TRACE(TEXT("Set Interact Terminal Capabilities failed.\n"));
			return FALSE;
		}
	}
	else
	{
		TRACE(L"RfidConfig() failed. Error code: %X.\n", dwRtn);
		return FALSE;
	}

/*	
	BYTE data[] = "\xC0\x01\x08\x9F\x33\x03\x00\x48\x08\xDF\x16\x03\x60\x40\x00\x9F\x35\x01\x22\xDF\x16\x01\x07\x9F\x40\x05\x60\x00\xB0\x10\x01\xDF\x16\x05\x00\x00\x00\x00\x00\x9F\x59\x03\x9C\x00\x00\xDF\x16\x03\xA0\x00\x00";
	// Set Config
	cmdData.Empty();
	cmdData.AddCmdTLV(0x63, 51, data);
	memset(RCmd, 0, RCmdLen);
	RLen = RCmdLen;

	/////////// DEBUG ////////////////
	BYTE *pData = cmdData.GetBuffer();
	DWORD len = cmdData.GetLength();
	for (int i=0; i<len; i++)
	{			
		TRACE(L"%02X ", pData[i]);
	}
	TRACE(L"\n");
	/////////// DEBUG ////////////////

	dwRtn = RfidConfig(cmdData.GetBuffer(), cmdData.GetLength(), RCmd, &RLen);
	
	if (dwRtn == 0)
	{
		if (RCmd[0] == 0x63 && RCmd[3] == 0x00)
		{
			res = TRUE;
		}
		else
		{
			TRACE(TEXT("Set Interact Terminal Capabilities failed.\n"));
		}
	}
	else
	{
		TRACE(L"RfidConfig() failed. Error code: %X.\n", dwRtn);
	}
*/
	return res;
}

/****************************************************************/
BOOL CMagRFID::SetMasterCardMChipMode()
{
	BYTE RCmd[RCmdLen];
	DWORD RLen,dwRtn;
	BOOL res=FALSE;

	TRACE(TEXT("=============== SetMasterCardMChipMode =======================\n"));

	memset(RCmd, 0, RCmdLen);
	RLen = RCmdLen;
	dwRtn = RfidConfig(setMasterCardModeMsr, sizeof(setMasterCardModeMsr), RCmd, &RLen);
	if (dwRtn == 0)
	{
		if (RCmd[0] == 0x6B && RCmd[3] == 0x00)
		{
			res = TRUE;
		}
		else
		{
			TRACE(TEXT("Set MasterCard kernel to M/Chip mode failed\n"));
		}
	}
	else
	{
		TRACE(L"RfidConfig() failed. Error code: %X.\n", dwRtn);
	}

	return res;
}

/****************************************************************/
BOOL CMagRFID:: SetMasterCardAppParam()
{
	BYTE RCmd[RCmdLen];
	DWORD RLen,dwRtn;
	BOOL res=FALSE;

	TRACE(TEXT("=============== SetMasterCardAppParam =======================\n"));
	
	memset(RCmd, 0, RCmdLen);
	RLen = RCmdLen;
	
	dwRtn = RfidConfig(setMasterCardAppPar, sizeof(setMasterCardAppPar), RCmd, &RLen);
	if (dwRtn == 0)
	{
		if (RCmd[0] == 0x21 && RCmd[3] == 0x00)
		{
			res = TRUE;
		}
		else
		{
			TRACE(TEXT("SetMasterCardAppParam failed\n"));
		}
	}
	else
	{
		TRACE(L"RfidConfig() failed. Error code: %X.\n", dwRtn);
	}
	
	return res;
}

/****************************************************************/
BOOL CMagRFID:: SetVisaAppParam()
{
	BYTE RCmd[RCmdLen];
	DWORD RLen,dwRtn;
	BOOL res=FALSE;

	TRACE(TEXT("=============== SetVisaAppParam =======================\n"));
	
	memset(RCmd, 0, RCmdLen);
	RLen = RCmdLen;
	
	dwRtn = RfidConfig(setVisaAppPar, sizeof(setVisaAppPar), RCmd, &RLen);
	if (dwRtn == 0)
	{
		if (RCmd[0] == 0x21 && RCmd[3] == 0x00)
		{
			res = TRUE;
		}
		else
		{
			TRACE(TEXT("SetVisaAppParam failed\n"));
		}
	}
	else
	{
		TRACE(L"RfidConfig() failed. Error code: %X.\n", dwRtn);
	}
	
	return res;
}

/****************************************************************/
BOOL CMagRFID:: SetAmexAppParam()
{
	BYTE RCmd[RCmdLen];
	DWORD RLen,dwRtn;
	BOOL res=FALSE;

	TRACE(TEXT("=============== SetAmexAppParam =======================\n"));
	
	memset(RCmd, 0, RCmdLen);
	RLen = RCmdLen;
	
	dwRtn = RfidConfig(setAmexAppPar, sizeof(setAmexAppPar), RCmd, &RLen);
	if (dwRtn == 0)
	{
		if (RCmd[0] == 0x21 && RCmd[3] == 0x00)
		{
			res = TRUE;
		}
		else
		{
			TRACE(TEXT("setAmexAppPar failed\n"));
		}
	}
	else
	{
		TRACE(L"RfidConfig() failed. Error code: %X.\n", dwRtn);
	}
	
	return res;
}

/****************************************************************/
BOOL CMagRFID:: SetInteracAppParam(BYTE MTI)
{
	BYTE RCmd[RCmdLen];
	DWORD RLen,dwRtn;
	BOOL res=FALSE;

	TRACE(TEXT("=============== SetInteractAppParam =======================\n"));
	
	memset(RCmd, 0, RCmdLen);
	RLen = RCmdLen;
/*	
	/////////// DEBUG ////////////////
	TRACE(TEXT("=== Request ===\n"));
	for (int i=0; i<sizeof(setInteracAppPar); i++)
	{			
		TRACE(L"%02X ", setInteracAppPar[i]);
	}
	TRACE(L"\n");
	/////////// DEBUG ////////////////
*/
	setInteracAppPar[9] = MTI;
	dwRtn = RfidConfig(setInteracAppPar, sizeof(setInteracAppPar), RCmd, &RLen);
	if (dwRtn == 0)
	{
		/////////// DEBUG ////////////////
		TRACE(TEXT("=== Response ===\n"));
		for (int i=0; i<RLen; i++)
		{			
			TRACE(L"%02X ", RCmd[i]);
		}
		TRACE(L"\n");
		/////////// DEBUG ////////////////

		if (RCmd[0] == 0x21 && RCmd[3] == 0x00)
		{
			res = TRUE;
		}
		else
		{
			TRACE(TEXT("SetInteractAppParam failed\n"));
		}
	}
	else
	{
		TRACE(L"RfidConfig() failed. Error code: %X.\n", dwRtn);
	}
	
	return res;
}

/****************************************************************/
BOOL CMagRFID::SetTermEmvData()
{
	CByteStream appData, cmdData;
	BYTE RCmd[RCmdLen];
	DWORD RLen,dwRtn;
	BOOL res=FALSE;

	TRACE(TEXT("=============== SetTermEmvData =======================\n"));
	memset(RCmd,0,RCmdLen);
	RLen = RCmdLen;
	
	
	// to set terminal EMV data(9F1C, 9F1A, 5F2A, 5F36, 9F16, 9F4E).
	// Transaction Currency Exponent (Tag 5F36): To change the decimal point of transaction amount. E.g. 9F02 = 1500, 5F36 = 2, then transaction amount = 15.00.
	// Transaction Currency Code (Tag 5F2A).
	// Terminal Country Code (Tag 9F1A).	
	// 9F1C	Terminal Identification
	BYTE data[100];
	DWORD size;
	CDataFile::Read(FID_MERCH_CFG_TERM_ID, (BYTE *) data, &size);
	appData.AddTLV(0x9F1C, size, data);
	
	// 9F1A	Terminal Country Code 
	appData.AddTLV(0x9F1A, 2, "\x01\x24");
	
	// 5F2A	Transaction Currency Code 
	//assert(CDataFile::Read(FID_EMV_TRAN_CURRENCY_CODE, (BYTE*) data, &size));
	appData.AddTLV(0x5F2A, 2, "\x01\x24");

	cmdData.Empty();
	cmdData.AddCmdTLV(0x61, appData.GetLength(), appData.GetBuffer());
	memset(RCmd, 0, RCmdLen);
	RLen = RCmdLen;
	dwRtn = RfidConfig(cmdData.GetBuffer(), cmdData.GetLength(), RCmd, &RLen);
	if (dwRtn == 0)
	{
		if (RCmd[0] == 0x61 && RCmd[3] == 0x00)
		{
			res = TRUE;
		}
		else
		{
			TRACE(TEXT("SetTermEmvData failed\n"));
		}
	}
	else
	{
		TRACE(L"RfidConfig() failed. Error code: %X.\n", dwRtn);
	}
	
	return res;
}

/****************************************************************/
BOOL CMagRFID::SetTermEmvDataMsr()
{
	CByteStream appData, cmdData;
	BYTE RCmd[RCmdLen];
	DWORD RLen,dwRtn;
	BOOL res=FALSE;

	TRACE(TEXT("=============== SetTermEmvDataMsr =======================\n"));
	memset(RCmd,0,RCmdLen);
	RLen = RCmdLen;
	
	
	// to set terminal EMV data(9F1C, 9F1A, 5F2A, 5F36, 9F16, 9F4E).
	// Transaction Currency Exponent (Tag 5F36): To change the decimal point of transaction amount. E.g. 9F02 = 1500, 5F36 = 2, then transaction amount = 15.00.
	// Transaction Currency Code (Tag 5F2A).
	// Terminal Country Code (Tag 9F1A).	
	// 9F1C	Terminal Identification
	BYTE data[100];
	DWORD size;
	CDataFile::Read(FID_MERCH_CFG_TERM_ID, (BYTE *) data, &size);
	//appData.AddTLV(0x9F1C, size, data);
	
	// 9F1A	Terminal Country Code 
	appData.AddTLV(0x9F1A, 2, "\x01\x24");
	
	// 5F2A	Transaction Currency Code 
	//assert(CDataFile::Read(FID_EMV_TRAN_CURRENCY_CODE, (BYTE*) data, &size));
	appData.AddTLV(0x5F2A, 2, "\x01\x24");

	//5F36 Transaction Currency Exponent (always make 2
	//Optional
	appData.AddTLV(0x5F36, 1, "\x02");

	//9F16

	//9F4E

	cmdData.Empty();
	cmdData.AddCmdTLV(0x61, appData.GetLength(), appData.GetBuffer());
	memset(RCmd, 0, RCmdLen);
	RLen = RCmdLen;

	/////////// DEBUG ////////////////
	TRACE(TEXT("=== Request ===\n"));
	BYTE *pData = cmdData.GetBuffer();
	int len = cmdData.GetLength();
	for (int i=0; i<len; i++)
	{			
		TRACE(L"%02X ", pData[i]);
	}
	TRACE(L"\n");
	/////////// DEBUG ////////////////


	dwRtn = RfidConfig(cmdData.GetBuffer(), cmdData.GetLength(), RCmd, &RLen);
	if (dwRtn == 0)
	{
		if (RCmd[0] == 0x61 && RCmd[3] == 0x00)
		{
			res = TRUE;
		}
		else
		{
			TRACE(TEXT("SetTermEmvDataMsr failed\n"));
		}
	}
	else
	{
		TRACE(L"RfidConfig() failed. Error code: %X.\n", dwRtn);
	}
	
	return res;
}

/****************************************************************/
BOOL CMagRFID::Config()
{
	DWORD dwRtn;
	CRecordFile emvDataFile, ctlsDataFile;
	AP_FILE_HANDLER fhEmvData, fhCtlsData;
	CByteStream appData;
	BOOL res = FALSE;

	CAppData::SupportCTLS = FALSE;
	CAppData::SupportPaypass = FALSE;
	
	dwRtn = RfidOpen();
	if (dwRtn)
	{
		TRACE(L"RfidOpen() failed. Error code: 0x%04x\n", dwRtn);
		return res;
	}

//	InstallCAKey();
//	SetCAKey();
	DeleteApp();

	// Install Apps
	// Get EMV CTLS data from EMV data file saved by host init
	if (!emvDataFile.Open(EMV_DATA_FILE, &fhEmvData))
		return res;

	// Get CTLS data from CTLS data file saved by host init
	if (!ctlsDataFile.Open(CTLS_DATA_FILE, &fhCtlsData))
		return res;

	USHORT numOfRecords;
	int iTemp = 0;
	emvDataFile.GetNumberOfRecords(&fhEmvData, &numOfRecords);
	if (numOfRecords > 0)
	{
		int n, m=numOfRecords;
		
		for (n = 0; n < numOfRecords; n++)
		{
			CByteStream cmdData;
			sEmvData EmvData, *pEmvData;
			sCtlsParams CtlsData, *pCtlsData;
			USHORT len1, len2;
			
			pEmvData = &EmvData;
			pCtlsData = &CtlsData;
			
			appData.Empty();

			memset(pEmvData, 0, sizeof(sEmvData));
			emvDataFile.ReadRecord(&fhEmvData, n, (BYTE*)pEmvData, &len1);
			iTemp = 0;
			do
			{
				if ( iTemp >= numOfRecords )
				{
					pCtlsData->hasData = FALSE;
					break;
				}
				ctlsDataFile.ReadRecord(&fhCtlsData, iTemp, (BYTE*)pCtlsData, &len2);
				iTemp++;
			}while(strcmp(pEmvData->serviceType,pCtlsData->serviceType)!=0);

			if (pCtlsData->hasData)
			{
				CAppData::SupportCTLS = TRUE;
				if (CEMV::IsMasterCardRID(pEmvData->asciiAID))	// MChip
				{
					//SetMasterCardApplication(n, pEmvData, pCtlsData);
//					SetMasterCardTermCapability();
					SetMasterCardMChipMode();
					SetMasterCardAppParam();
					CAppData::SupportPaypass = TRUE;
				}
				else if (CEMV::IsVisaRID(pEmvData->asciiAID))	// qVsdc
				{
					SetVisaApplication(n, pEmvData, pCtlsData, TRUE);
					if (m == numOfRecords)	// set once
					{
						SetVisaApplication(m, pEmvData, pCtlsData, FALSE);
						m = 0;
					}	
//					SetVisaTermCapability(pCtlsData);
					SetVisaAppParam();
				}
				else if (CEMV::IsAmexRID(pEmvData->asciiAID))
				{
					SetAmexApplication(n, pEmvData, NULL);
					SetAmexTermCapability();
					SetAmexAppParam();
				}
				else if (CEMV::IsInteracRID(pEmvData->asciiAID))
				{
					SetInteracApplication(n, pEmvData, pCtlsData);
					SetInteracTermCapability();
					SetInteracAppParam(pEmvData->MTI);
				}
			}	// if (pCtlsData->hasData)
			/*
			else if (CEMV::IsAmexRID(pEmvData->asciiAID))
			{
					SetAmexApplication(n, pEmvData, NULL);
					SetAmexTermCapability();
					SetAmexAppParam();
			}
			*/
		}
		SetTermEmvData();
		res = TRUE;
	}

	return res;
}
/****************************************************************/
BOOL CMagRFID::ConfigMsr()
{
	DWORD dwRtn;
	CRecordFile emvDataFile, ctlsDataFile;
//	AP_FILE_HANDLER fhCtlsData; //fhEmvData
	BOOL res = FALSE;

	CAppData::SupportCTLS = FALSE;
	CAppData::SupportPaypass = FALSE;
	
	dwRtn = RfidOpen();
	if (dwRtn)
	{
		TRACE(L"RfidOpen() failed. Error code: 0x%04x\n", dwRtn);
		return res;
	}

//	InstallCAKey();
	//SetCAKey();
	DeleteApp();

	SetVisaApplicationMsr();
	SetVisaTermCapabilityMsr();

	SetMasterCardApplication();
	SetMasterCardMChipMode();

	SetAmexApplicationMsr();
	SetAmexTermCapability();

	SetTermEmvDataMsr();
	res= TRUE;

	
//					SetVisaApplication(n, pEmvData, pCtlsData, TRUE);
//					if (m == numOfRecords)	// set once
//					{
//						SetVisaApplication(m, pEmvData, pCtlsData, FALSE);
//						m = 0;
//					}	
//					SetVisaTermCapability(pCtlsData);
//					SetVisaAppParam();
//				}
//				else if (CEMV::IsAmexRID(pEmvData->asciiAID))
//				{
//					SetAmexApplication(n, pEmvData, NULL);
//					SetAmexTermCapability();
//					SetAmexAppParam();
//				}
//				else if (CEMV::IsInteracRID(pEmvData->asciiAID))
//				{
//					SetInteracApplication(n, pEmvData, pCtlsData);
//					SetInteracTermCapability();
//					SetInteracAppParam(pEmvData->MTI);
//				}
//	}

	return res;
}

BOOL CMagRFID::Open(HWND hwnd, char *amount, unsigned short txnCode,int iAmount)
{
//	Cancel();

	m_hWndRFID = hwnd;

	if( !CAppData::SupportCTLS )
		return TRUE;

	BYTE RCmd[RCmdLen];
	DWORD RLen;
	DWORD dwRtn;
	
	RfidClose();
	Sleep(100);


	dwRtn = RfidOpen();
	if (dwRtn)
		return FALSE;

	SetMasterCardTermCapability(iAmount);

	RLen = RCmdLen;
	dwRtn = RfidConfig(checkRemoveCard, sizeof(checkRemoveCard), RCmd, &RLen);

	if (dwRtn)
	{
		goto _EXIT;
	}

	if (RCmd[0] == 0x58 && RCmd[3] == 0x0) //Check remove card OK
	{
		if (RCmd[4] == 0x0) // Card has removed from last transaction
		{ 
			if (txnCode == TRAN_PURCHASE)
				dwRtn = RfidTransaction(PURCHASE_FOR_GOODS, amount, NULL);
			else if (txnCode == TRAN_REFUND)
				dwRtn = RfidTransaction(REFUND_TRANSACTION, amount, NULL);
			else
				goto _EXIT;

			if (dwRtn)
			{
				goto _EXIT;
			}
			
			dwRtn = RfidScanEnable(NULL, RFIDEvent, 18000);
			if (dwRtn)
			{
				goto _EXIT;
			}
			return TRUE;
		}
		else if (RCmd[4] == 0x01)  // Card has not removed from last transaction
		{
			TRACE(_T("=>Please remove card and press Start button again"));
		}
	}
	else
	{
		TRACE(_T("=>Check remove card failed"));
	}
	
_EXIT:
	RfidClose();
	return FALSE;
}

void CMagRFID::Cancel()
{
	RfidClose();
}

void CMagRFID::Reset()
{
	RfidReset();
}

BOOL CMagRFID::GetData(BYTE* buf, DWORD* len)
{
	if (m_Len == 0 && m_Len == 0xFFFFFFFF)
		return FALSE;

	*len = m_Len;
	memcpy(buf, m_Data, *len);
	buf[*len] = 0;
	return TRUE;
}

void CMagRFID::GetData()
{
	m_Len = RfidGetData(m_Data);
}

BOOL CMagRFID::DeleteApp()
{
	BYTE RCmd[128];
	BYTE RCmd1[128];
	BYTE SendCmd[128];
	DWORD RLen;
	DWORD dwRtn;
	DWORD SendLen;

	memset(RCmd1,0,128);
	RLen = 128;

	dwRtn = RfidConfig(QueryApplication,sizeof(QueryApplication),RCmd1,&RLen);
	if (dwRtn )
		return FALSE;

	if( RCmd1[0] != 0x40)
		return FALSE;

	int iCount = RCmd1[4];
	if ( iCount == 0 )
		return TRUE;

	BYTE *pC0,*pDF0B,*p9F06;
	pC0 = &RCmd1[5];
	for(int i= 0;i<iCount;i++)
	{
		if (*pC0 != 0xC0 || *(pC0+1) != 0x01)
			return FALSE;
		pDF0B = pC0+3;
		if (*pDF0B != 0xDF || *(pDF0B+1) != 0x0B)
			return FALSE;
		p9F06 = pC0+7;
		if (*p9F06 != 0x9F || *(p9F06+1) != 0x06)
			return FALSE;

		pC0 += 10+*(p9F06+2);

		memset( SendCmd,0,128);
		SendCmd[0] = 0x43;
		SendCmd[2] = 7+*(p9F06+2);
		memcpy(&SendCmd[3],pDF0B,SendCmd[2]);
		SendLen = SendCmd[2]+3;

		memset(RCmd,0,128);
		RLen = 128;
		dwRtn = RfidConfig(SendCmd,SendLen,RCmd,&RLen);
		if (dwRtn )
			return FALSE;
	}
	return TRUE;
}

int CMagRFID::GetPaypassCvmLimit()
{
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
		if(pCtlsData->serviceType[1] == ' ')
			pCtlsData->serviceType[1] = 0;

		if( strcmp(pCtlsData->serviceType,"M") == 0)
		{
			CString str = CAppAPI::ByteToStr(pCtlsData->ctlsCvmLimit,SZ_BIN_CTLS_EMV_CVM_LIMIT);
			CvmAmount = atoi(alloc_tchar_to_char(str.GetBuffer(str.GetLength())));
			str.ReleaseBuffer();
			return CvmAmount;
		}
	}
	return 0;
}
