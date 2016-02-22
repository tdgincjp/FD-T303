#include "StdAfx.h"
#include <assert.h>
#include "common.h"
#include "emvapi.h"
#include "ByteStream.h"
#include "SaioReader.h"
#include "..\\utils\\string.h"
#include "..\\defs\\constant.h"
#include "..\\defs\\struct.h"
#include "..\\utils\\BinUtil.h"
#include "..\\utils\\StrUtil.h"
#include "..\\data\\DataFile.h"
#include "..\\data\\RecordFile.h"

#pragma comment(lib, "Lib\\EMVAPI.lib")

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//----------------------------------------------------------------------------
//!	\brief	Dump Error
//!	\param	err - error code
//----------------------------------------------------------------------------
void DumpError(DWORD err)
{

}


//----------------------------------------------------------------------------
//!	\brief	virtual function
//!	\param	str - HEX string
//!	\param	buf - point to BYTE data
//!	\param	bufSize - buf length
//! RETURNS:       byte length    
//----------------------------------------------------------------------------
int HexToByte(CString str, BYTE * buf, int bufSize)
{
	str.Remove(' ');
	int length = str.GetLength();
	length = (length % 2) ? length-1 : length;

	if ( bufSize < length/2 )
		return (-1)*length/2;

	wchar_t temp[3];
	BYTE value = 0;
	memset(buf,0,bufSize);

	for(int i = 0 ; i < length ; i+=2)
	{	
		temp[0] = str[i];
		temp[1] = str[i+1];
		swscanf_s(temp,L"%X", &value);
		buf[i/2] = value;
	}
	return length/2;
}

//----------------------------------------------------------------------------
//!	\brief	Delete Profiles function
//----------------------------------------------------------------------------
void DeleteProfiles()
{
	WORD index;	

	// ICS Profile
	EmvDelProfile(0x00, 0);

	// Terminal Profile
	EmvDelProfile(0x01, 0);
	
	// Application Profile
	if (EmvGetFirstProfile(0x02, &index) == 0)
	{
		EmvDelProfile(0x02, index);
		for (;;)
		{
			if (EmvGetNextProfile(&index) == 0)
			{
				EmvDelProfile(0x02, index);
			}
			else
			{
				break;
			}
		}		
	}
	
	// Key Profile
	if (EmvGetFirstProfile(0x03, &index) == 0)
	{
		EmvDelProfile(0x03, index);
		for (;;)
		{
			if (EmvGetNextProfile(&index) == 0)
			{
				EmvDelProfile(0x03, index);
			}
			else
			{
				break;
			}
		}		
	}
}

//----------------------------------------------------------------------------
//!	\brief	Set ICS Profiles function
//----------------------------------------------------------------------------
BOOL SetIcsProfile()
{
ICS_DATA icsData;
memset(&icsData, 0, sizeof(ICS_DATA));

// bit8: offline only					0
// bit7: offline with online			1
// bit6: online only					0		
// bit5: attend							0
// bit4: unattend						1
// bit3: financial						0
// bit2: cardholder						1
// bit1: merchant						0
icsData.byTerminalType = 0x25;			// Type 25, bit-7,4,2

// bit8: Manual Key Entry				0
// bit7: Magnetic Stripe				1
// bit6: IC with Contacts (M)			1
icsData.byCardDataInput = 0x60;			// bit-7,6

// bit8: offline plain text pin			1
// bit7: online encrypher pin			0
// bit6: signature						0	
// bit5: offline encipher pin			1
// bit4: no cvm required				1
icsData.byCVMCapability = 0x98;			// bit-8,5,4

// bit8: SDA							1	
// bit7: DDA							1
// bit6: Card capture					0
// bit5: RFU							0
// bit4: CDA							1	
icsData.bySecurityCapability = 0xC8;	// bit-8,7,4

// byte1 bit8: cash						0
// byte1 bit7: goods					1
// byte1 bit6: service					1
// byte1 bit5: cashback					0
// byte1 bit4: Inquiry					0
// byte1 bit3: Transfer					0
// byte1 bit2: Payment					0
// byte1 bit1: Administrative			0
// byte2 bit8: Cash Deposit				0
// byte2 bit7~bit1: RFU					0
icsData.byTxnType[0] = 0x60;			// bit-7,6

//	bit8: Keypad									1
//	bit7: Numeric Keys								1
//	bit6: Alphabetic and Special Character Keys		1
//	bit5: Command Keys								1
//	bit4: Function Keys								1
icsData.byTmDataInput = 0xF8;			// bit-8,7,6,5,4

//	byte1 bit8: Print Attendant				0
//	byte1 bit7: Print Cardholder			1
//	byte1 bit6: Display Attendant			0
//	byte1 bit5: Display Cardholder			1
//	byte1 bit4: RFU
//	byte1 bit3: RFU
//	byte1 bit2: Code Table 10				0
//	byte1 bit1: Code Table 9				0
icsData.byTmDataOutput[0] = 0x50;			// bit-7,5

//	byte2 bit8: Code Table 8
//	byte2 bit7: Code Table 7
//	byte2 bit6: Code Table 6
//	byte2 bit5: Code Table 5
//	byte2 bit4: Code Table 4
//	byte2 bit3: Code Table 3
//	byte2 bit2: Code Table 2
//	byte2 bit1: Code Table 1				1
icsData.byTmDataOutput[1] = 0x01;		// bit-1

// bit8: support PSE						1
// bit7: support cardholder confirm			1
// bit6: prefer order						0
// bit5: partial aid select(M)				1
// bit4: support multi language				0
// bit3: common character set(M)			1
// bit2: EMV Language Selection Method		0
icsData.byApplicationSelect = 0xD4;		// bit-8,7,5,3

icsData.byMaxCAPK = 248;

//	bit8: 3								0
//	bit7: 2^16+1						0
//	bit6: 3 and 2^16+1					1
icsData.byExponent = 0x20;				// bit-6

// bit8: support check revocation ca public key							1
// bit7: contain default DDOL											1
// bit6: operator ation required during loading ca public key fail		0
// bit5: verify ca public key checksum									1
icsData.byDataAuthen =	0xD0;			// bit-8,7,5

// bit8: support bypass pin entry					0
// bit7: support Sub bypass pin entry				0
// bit6: support get data for pin try counter		1
// bit5: support fail cvm							1
// bit4: amount knows before cvm					1
icsData.byCVM = 0x38;					// bit-6,5,4

// bit8: floor limit checking												1
// bit7: random selection													1
// bit6: velocity checking													1
// bit5: support transaction log											1
// bit4: support exception file												0
// bit3: performance of terminal risk management based on AIP setting		1
icsData.byTRM = 0xF4;					// bit-8,7,6,5,3

// bit8: support terminal action codes							1
// bit7: default action codes prior to first generate ac		0
// bit6: default action codes after first generate ac			0
// bit5: TAC/IAC default skipped								0
// bit4: TAC/IAC default normal									0
// bit3: CDA failure before TAA									1
// bit2: Can the values of TACs be changed						1
// bit1: Can the TACs be deleted or disabled					0
icsData.byActionAnalysis = 0x86;						// bit-8,3,2


// bit8: CDA always 1 GenAC					1		
// bit7: CDA never 1 GenAC					0
// bit6: CDA always 2 GenAC					1
// bit5: CDA never 2 GenAC					0
icsData.byCDAMode = 0xA0;				// bit-8,6 CDA Mode 1

// bit8: support force online							0
// bit7: support force accept							0
// bit6: support advice									0
// bit5: support issuer initiated voice referral		1
// bit4: support batch data capture						1
// bit3: support online data capture					1
// bit2: support default TDOL							1
icsData.byCompletion = 0x1E;					// bit-5,4,3,2

//BYTE	byException;		// POS entry mode
icsData.byException = 0x05;			//????? 80x

// bit8: PINPAD									1
// bit7: PIN and amount in same keypad			1
// bit6: ICC/Magstripe Reader commbined			1
// bit5: Magstripe Reader first					1
// bit4: Account Type select					1
// bit3: on fly script processing				0
// bit2: Issuer Script device limt >128?		1
// bit1: Internel date management				1
icsData.byMISC = 0xFB;				// bit-8,7,6,5,4,2,1

// bit8: Support Selectable Kernel Config
// bit7: Amount Select Range Criteria X
// bit6: Amount Select Range Criteria X and Y
// bit5: Amount Select Range Criteria X per AID
// bit4: Amount Select Range Criteria X and Y per AID
icsData.bySelKernelConfig = 0x00;

	WORD length = sizeof(ICS_DATA);
	CByteStream ics;
	ics.AddTLV(0xDF01, length, (BYTE *)&icsData);
	WORD index=0;

	WORD wRet = EmvSetProfile(0x00,							// Type : ICS Profile
														&index,						// Index : 0
														ics.GetBuffer(),	// TLV data
														ics.GetLength());	// TLV length

	if (wRet != 0)
	{
		DumpError(wRet);
		return FALSE;
	}	
	return TRUE;
}

//----------------------------------------------------------------------------
//!	\brief	Set terminal Profiles function
//----------------------------------------------------------------------------
BOOL SetTerminalProfile()
{
	CByteStream termData;
	
	// 9F1A	Terminal Country Code
	termData.AddTLV(0x9F1A, 2, "\x01\x24");
	
	// 9F1E	Interface Device(IFD) Serial Number
	termData.AddTLV(0x9F1E, 4, "\x12\x34\x56\x78");
	WORD index=0;

	WORD wRet = EmvSetProfile(0x01,										// Type : Terminal Profile
														&index,									// Index : 0
														termData.GetBuffer(),		// TLV data
														termData.GetLength());	// TLV length
	if (wRet != 0)
	{
		DumpError(wRet);
		return FALSE;
	}
	return TRUE;
}

//----------------------------------------------------------------------------
//!	\brief	Set application Profiles function
//----------------------------------------------------------------------------
BOOL SetApplicationProfile()
{
	CByteStream appData;
	CRecordFile emvDataFile;
	AP_FILE_HANDLER fhEmvData;
	char data[100];
	DWORD size;

	char ServType[4][3] = {"P0","V ","M ","AX"};
//	TRACE(L"SetApplicationProfile()\n");

	if (!emvDataFile.Open(EMV_DATA_FILE, &fhEmvData))
		return FALSE;

	USHORT numOfRecords;
	emvDataFile.GetNumberOfRecords(&fhEmvData, &numOfRecords);
	if (numOfRecords > 0)
	{
		for(int j = 0;j<=4;j++)
		{
			int n;
			WORD index=0;
			//emvDataFile.FirstRecord(&fhEmvData);
			for (n = 0; n < numOfRecords; n++)
			{
				sEmvData EmvData, *pEmvData;
				USHORT len1;
				pEmvData = &EmvData;
				appData.Empty();

				memset(pEmvData, 0, sizeof(sEmvData));
				emvDataFile.ReadRecord(&fhEmvData, n, (BYTE*)pEmvData, &len1);

				if( j< 4 && strcmp(pEmvData->serviceType,ServType[j]) != 0 )
					continue;
				
				if ( j == 4 ) //for other download service type setting
				{
					int i;
					for(i=0;i<4;i++)
					{
						if (strcmp(pEmvData->serviceType,ServType[i]) == 0 )
							break;
					}
					if ( i < 4 )
						continue;
				}

				// 9F06	Application Identifier (AID) - Terminal
				appData.AddTLV(0x9F06, pEmvData->AIDLength, pEmvData->AID);

				// 9F09	Application Version Number - Terminal
				STR_PackData((BYTE*)data, (char*)pEmvData->appVersion, sizeof(pEmvData->appVersion));
				appData.AddTLV(0x9F09, sizeof(pEmvData->appVersion)/2, data);

				// DF10	Application Selection Indicator
				appData.AddTLV(0xDF10, 1, &pEmvData->applicationSelectionIndicator);

				// 9F01	Acquirer Identifier
				appData.AddTLV(0x9F01, 6,  "\x00\x00\x00\x47\x61\x73");	// FIXME

				// 9F15	Merchant Category Code
				appData.AddTLV(0x9F15, 2, "\x53\x11");	// FIXME

				// 9F16	Merchant Identifier
				appData.AddTLV(0x9F16, 15, "123456789012345");

				// 9F1C	Terminal Identification
				if ( !CDataFile::Read(FID_MERCH_CFG_TERM_ID, (BYTE*) data, &size))
					return FALSE;
				appData.AddTLV(0x9F1C, size, data);

				// 9F1A	Terminal Country Code 
				appData.AddTLV(0x9F1A, 2, "\x01\x24");

				// 5F2A	Transaction Currency Code 
				//assert(CDataFile::Read(FID_EMV_TRAN_CURRENCY_CODE, (BYTE*) data, &size));
				appData.AddTLV(0x5F2A, 2, "\x01\x24");

				// 5F36	Transaction Current Exponent
				//assert(CDataFile::Read(FID_EMV_TRAN_CURRENCY_EXPONENT, (BYTE*) data, &size));
				appData.AddTLV(0x5F36, 1, "\x02");

				// 9F3C	Transaction Reference Currency Code
				//assert(CDataFile::Read(FID_EMV_TRAN_REFERENCE_CURRENCY_CODE, (BYTE*) data, &size));
				appData.AddTLV(0x9F3C, 2, "\x01\x24");

				// DF11	Transaction Reference Currency Conversion
				//assert(CDataFile::Read(FID_EMV_TRAN_REFERENCE_CURRENCY_CONVERSION, (BYTE*) data, &size));
				//appData.AddTLV(0xDF11, size, data);

				// 9F3D	Transaction Reference Currency Exponent
				//assert(CDataFile::Read(FID_EMV_TRAN_REFERENCE_CURRENCY_EXPONENT, (BYTE*) data, &size));
				appData.AddTLV(0x9F3D, 1, "\x02");


				// DF12	Default Transaction Certificate Data Object List (TDOL)
				appData.AddTLV(0xDF12, pEmvData->defaultTDOLLength, pEmvData->defaultTDOL);

				// DF13	Default Dynamic Data Authentication Data Object List (DDOL)
				appData.AddTLV(0xDF13, pEmvData->defaultDDOLLength, pEmvData->defaultDDOL);

				// DF14	Terminal Action Code - Default
				appData.AddTLV(0xDF14, sizeof(pEmvData->TACDefault), pEmvData->TACDefault);

				// DF15	Terminal Action Code - Denial
				appData.AddTLV(0xDF15, sizeof(pEmvData->TACDenial), pEmvData->TACDenial);

				// DF16	Terminal Action Code - Online
				appData.AddTLV(0xDF16, sizeof(pEmvData->TACOnline), pEmvData->TACOnline);

				// 9F1B	Terminal Floor Limit
				//appData.AddTLV(0x9F1B, 4, "\x00\x00\x00\x00");	// force online
				appData.AddTLV(0x9F1B, 4, pEmvData->emvOfflineFloorLimit);

				// 9F1D	Terminal Risk Management Data
				appData.Add("\x9F\x1D\x08\x0C\x08\x00\x00\x00\x00\x00\x00", 11);

				// 9F7E	Mobile Support Indicator
				appData.Add("\x9F\x7D\x01\x01", 4);

				// DF17	Maximum Target Percentage to be Used for Biased Random Selection
				appData.AddTLV(0xDF17, sizeof(pEmvData->maximumTargetPercentage), &pEmvData->maximumTargetPercentage);

				// DF18	Target Percentage to be Used for Random Selection
				appData.AddTLV(0xDF18, sizeof(pEmvData->targetPercentage), &pEmvData->targetPercentage);

				// DF19	Threshold Value for Biased Random Selection
				appData.AddTLV(0xDF19, sizeof(pEmvData->thresholdValue), pEmvData->thresholdValue);

				BYTE *pData = appData.GetBuffer();	// debug
				int len = appData.GetLength();

				WORD wRet = EmvSetProfile(0x02,			// Type : Application Profile
					&index,		// Index
					pData,		// TLV data
					len);			// TLV length

				if (wRet != 0)
				{
					DumpError(wRet);
					return FALSE;
				}
			}
		}
	}
					
	return TRUE;
}

//----------------------------------------------------------------------------
//!	\brief	Set Capk Profiles function
//----------------------------------------------------------------------------
BOOL SetCapkProfile()
{
	CByteStream keyData;
	CRecordFile emvDataFile;
	AP_FILE_HANDLER fhEmvData;
	
	// to find which RID is supported
	if (!emvDataFile.Open(EMV_DATA_FILE, &fhEmvData))
		return FALSE;
		
	USHORT numOfRecords;
	emvDataFile.GetNumberOfRecords(&fhEmvData, &numOfRecords);
	if (numOfRecords > 0)
	{
		int n; 
		WORD index=0;

		for (n = 0; n < numOfRecords; n++)
		{
			sEmvData EmvData, *pEmvData;
			USHORT len;
			char fileName[50];	// eg. EKA000000277
			CRecordFile keyFile;
			AP_FILE_HANDLER fh;

			pEmvData = &EmvData;
			memset(pEmvData, 0, sizeof(sEmvData));
			emvDataFile.ReadRecord(&fhEmvData, n, (BYTE*)pEmvData, &len);
			memset(fileName, 0, 50); 
			strcpy(fileName, EMV_KEY_DATA_DIR);
			strcat(fileName, EMV_KEY_FILE_PREFIX);
			strncat(fileName, pEmvData->asciiAID, 10);	// RID

			if (keyFile.Open(fileName, &fh))	
			{
				USHORT numOfKeys;
				keyFile.GetNumberOfRecords(&fh, &numOfKeys);
				if (numOfKeys > 0)
				{
					sEmvKeyData *pEmvKeyData, EmvKeyData;
					pEmvKeyData = &EmvKeyData;
					USHORT len1;

					for (int m = 0; m < numOfKeys; m++)
					{
						keyFile.ReadRecord(&fh, m, (BYTE *)pEmvKeyData, &len1);
						keyData.AddTLV(0xDF20, 5, pEmvKeyData->RID);
						
						// 9F22	CAPK Index - Terminal (PKI)
						keyData.AddTLV(0x9F22, 1, &pEmvKeyData->index);
										
						// DF21	CAPK Modulus
						keyData.AddTLV(0xDF21, pEmvKeyData->modulusLength, pEmvKeyData->modulus);

						// DF22	CAPK Exponent
						keyData.AddTLV(0xDF22, pEmvKeyData->exponentLength, pEmvKeyData->exponent);
						
						// DF23	CAPK Check Sum
						keyData.AddTLV(0xDF23, pEmvKeyData->checksumLength, pEmvKeyData->checksum);
												
						// DF24	CAPK Expiration Date (YYYYMMDD): 20200101 
						keyData.AddTLV(0xDF24, 4, "\x20\x20\x01\x01");

						// DF25	CAPK Revocation Serial Number
						keyData.AddTLV(0xDF25, 3, "\x00\x00\x00");

						// DF26	CAPK EffectiveDate (YYYYMMDD): 20080101
						keyData.AddTLV(0xDF26, 4, "\x20\x08\x01\x01");
						
						BYTE *pData = keyData.GetBuffer();	// debug
						int len = keyData.GetLength();

						WORD wRet = EmvSetProfile(0x03,										// Type : Key Profile
																			&index,
																			pData,									// TLV data
																			len);										// TLV length
										
						keyData.Empty();
					} // for (m)
				}
				keyFile.CloseFile(&fh);
			}	
		}
		emvDataFile.CloseFile(&fhEmvData);
	}
	return TRUE;
}
