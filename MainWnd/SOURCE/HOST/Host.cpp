#include "stdafx.h"
#include "Winbase.h"
#include <assert.h>
#include "..\\Utils\\string.h"
#include "..\\defs\\constant.h"
#include "..\\defs\\struct.h"
#include "..\\utils\\BinUtil.h"
#include "..\\utils\\StrUtil.h"
#include "..\\data\\DataFile.h"
#include "..\\data\\RecordFile.h"
#include "..\\sec\\SHA1.h"
#include "..\\sec\\sec.h"
#include "..\\Utils\\FolderCtrl.h"
#include "..\\Utils\\TimeUtil.h"
#include "..\\UI\\language.h"
#include "..\\EMV\\EMV.h"
#include "..\\utils\\totalutil.h"

#include "Host.h"

//-----------------------------------------------------------------------------
//!	\brief	constructor
//-----------------------------------------------------------------------------
CHost::CHost()
{
	m_EMVDownload = FALSE;
	m_hWnd = NULL;

	pTRANDATA = NULL;

	m_KeyError = FALSE;

	bCloseBatchFlag = FALSE;

	m_bNewKey = FALSE;

	m_iSeqNo = 0;
}

//-----------------------------------------------------------------------------
//!	\brief	destructor
//-----------------------------------------------------------------------------
CHost::~CHost(void)
{
}


//-----------------------------------------------------------------------------
//!	\brief	clear request buffer
//-----------------------------------------------------------------------------
void CHost::ClearRequest()
{
	memset((void*)m_Request, 0, SZ_HOST_BUFF_MAX);
	m_RequestLength = 0;
}

//-----------------------------------------------------------------------------
//!	\brief	check if Card is manually entered
//-----------------------------------------------------------------------------
BOOL CHost::IsManualPAN()
{
	return (pTRANDATA->EntryMode == ENTRY_MODE_MANUAL);
}

//-----------------------------------------------------------------------------
//!	\brief	check if Card is swiped
//-----------------------------------------------------------------------------
BOOL CHost::IsSwiped()
{
	return (pTRANDATA->EntryMode == ENTRY_MODE_SWIPED);
}
//-----------------------------------------------------------------------------
//!	\brief	check if Card is swiped
//-----------------------------------------------------------------------------
BOOL CHost::IsCtlsMSR()
{
	return (pTRANDATA->EntryMode == ENTRY_MODE_CTLS_MSR);
}

//-----------------------------------------------------------------------------
//!	\brief	check if Card is Chip Card
//-----------------------------------------------------------------------------
BOOL CHost::IsChip()
{
	return (pTRANDATA->EntryMode == ENTRY_MODE_CHIP);
}

//-----------------------------------------------------------------------------
//!	\brief	check if Card is CTLS Card
//-----------------------------------------------------------------------------
BOOL CHost::IsCtls()
{
	return (pTRANDATA->EntryMode == ENTRY_MODE_CTLS);
}

//-----------------------------------------------------------------------------
//!	\brief	check if Financial txn
//-----------------------------------------------------------------------------
BOOL CHost::IsFinancial()
{
	if (pTRANDATA->TranCode > FUNCTION_FINAN_BEGIN &&
			pTRANDATA->TranCode < FUNCTION_FINAN_END)
	{
		return TRUE;	
	}
	return FALSE;		
}

//-----------------------------------------------------------------------------
//!	\brief	put char to request
//!	\param	c - char to put
//-----------------------------------------------------------------------------
void CHost::Put(char c)
{
	m_Request[m_RequestLength++] = c;
}

//-----------------------------------------------------------------------------
//!	\brief	put chars to request
//!	\param	c - char to move
//!	\param	count - numbers of c
//-----------------------------------------------------------------------------
void CHost::Put(char c, int count)
{
	for (int i=0; i<count; i++)
	{
		Put(c);
	}		
}

//-----------------------------------------------------------------------------
//!	\brief	Append zero-terminated string to request
//!	\param	s - zero-terminated string
//-----------------------------------------------------------------------------
void CHost::Put(const char *str)
{
	strcpy(&m_Request[m_RequestLength], str);
	m_RequestLength += strlen(str);
}

//-----------------------------------------------------------------------------
//!	\brief	Append zero-terminated string to request
//!	\param	s - zero-terminated string
//!	\param	maxLength - safety counter, maximal allowed length
//-----------------------------------------------------------------------------
void CHost::Put(const char *str, int maxLength)
{
	int len = (maxLength < (int)strlen(str))? maxLength : strlen(str);
	strncpy(&m_Request[m_RequestLength], str, len);
	m_RequestLength += len;
}

//-----------------------------------------------------------------------------
//!	\brief	go back number of bytes
//!	\param	Len - length to go back
//-----------------------------------------------------------------------------
void CHost::Back(int len)
{
	m_RequestLength -= len;
	m_Request[m_RequestLength] = 0;
}

//-----------------------------------------------------------------------------
//!	\brief	Append zero-terminated string to request, right fill till 
//!					totalLength characters
//!	\param	s - zero-terminated string
//!	\param	totalLength - total length
//!	\param	fill - character to fill with
//-----------------------------------------------------------------------------
void CHost::PutRightFilled(const char *str, int totalLength, char fill)
{
	int len = (totalLength < (int)strlen(str))? totalLength : strlen(str);

	Put(str, len);

	// Fill till totalLength bytes
	int d = totalLength - len;
	if (d > 0)
	{
		Put(fill, d);
	}
}

//-----------------------------------------------------------------------------
//!	\brief	Convert BCD->ASCII and move to request.
//!	\param	buffer - points to the address where the binary or BCD bytes are located.
//!	\param	size - contains the number of input bytes.
//-----------------------------------------------------------------------------
void CHost::PutBCDAscii(const BYTE *buffer, int size)
{
	char *ascii = new char[2*size + 1];
	memset(ascii, 0, 2*size + 1);

	// unpack data then move to request.
	STR_UnpackData(ascii, (BYTE *)buffer, size); //TDGJP
	ascii[2*size] = 0; // make C-string
	Put(ascii);
	delete[] ascii;
}

//-----------------------------------------------------------------------------
//!	\brief	Convert BCD number to ASCII number and move rightmost characters 
//!					to request
//!	\param	bcd - BCD number
//!	\param	bcdSize	- size in bytes of BCD number
//!	\param	count - number of ASCII character to move from end
//-----------------------------------------------------------------------------
void CHost::PutBCDRight(const BYTE *bcd, int bcdSize, int count)
{
	char *ascii = new char[2*bcdSize + 1];

	// Convert to ASCII
	STR_UnpackData(ascii, (BYTE *)bcd, bcdSize);
	ascii[2*bcdSize] = 0; // make C-string

	// Copy to request count rightmost characters
	// if requested size <count> is greater than actual data size
	if (count > (2*bcdSize))
	{
		count = 2 * bcdSize;
	}
	Put(&ascii[2*bcdSize - count]); 

	delete[] ascii;
}

//----------------------------------------------------------------------------
//!	\brief	Move buffer to request, left justified with padded character
//!	\param	str - buffer to move
//!	\param	size - size of buffer in bytes
//!	\note	Move at least size bytes, and fill with padded character till
//!			totalSize characters
//----------------------------------------------------------------------------
void CHost::PutLeftJustified(const char *str, int size, int totalSize, char pad)
{
	int len = strlen(str);
	int n = (size < len) ? size : len;

	Put(str, n);
	if (totalSize > n)
	{
		Put(pad, totalSize-n);
	}
}

//----------------------------------------------------------------------------
//!	\brief	Move buffer to request, right justified with padded character
//!	\param	str - buffer to move
//!	\param	size - size of buffer in bytes
//!	\note	Move at least size bytes, and fill with padded character till
//!			totalSize characters
//----------------------------------------------------------------------------
void CHost::PutRightJustified(const char *str, int size, int totalSize, char pad)
{
	int len = strlen(str);
	int n = (size < len) ? size : len;
	if (totalSize > n)
	{
		Put(pad, totalSize-n);
	}
	
	Put(str, n);
}

//----------------------------------------------------------------------------
//!	\brief	Move device type to request message
//----------------------------------------------------------------------------
BOOL CHost::PutDeviceType()
{
	Put("9.");
	return TRUE;
}

//----------------------------------------------------------------------------
//!	\brief	check buffer for null character
//----------------------------------------------------------------------------
BOOL CHost::LengthCheck(const char* pBuffer, unsigned short NumBytes)
{
	for (int i=0; i < NumBytes; i++)
	{
		if (pBuffer[i] == 0)
			return FALSE;
	}
	return TRUE;
}

//----------------------------------------------------------------------------
//!	\brief	Move transmission number to request message
//----------------------------------------------------------------------------
BOOL CHost::PutTransmissionNumber()
{
	BYTE tranNo=0;
	DWORD size = sizeof(BYTE);
	char temp[2];

	CDataFile::Read(FID_TERM_TRANSMISSION_NUMBER, &tranNo, &size);

	if (tranNo == 0)
		tranNo = 1;
		
	temp[0] = tranNo / 10 + '0';
	temp[1] = tranNo % 10 + '0';
	Put(temp[0]);
	Put(temp[1]);

	if (++tranNo == 100)
		tranNo = 1;

	CDataFile::Save(FID_TERM_TRANSMISSION_NUMBER, &tranNo, sizeof(BYTE));
	return FALSE;
}

//----------------------------------------------------------------------------
//!	\brief	Move terminal ID to request message
//----------------------------------------------------------------------------
BOOL CHost::PutTerminalID()
{
	PutLeftJustified((const char *)pCONFIG->TermID, strlen(pCONFIG->TermID), 16 , ' ');
	return true;
}

//----------------------------------------------------------------------------
//!	\brief	Move Clerk ID to request message
//----------------------------------------------------------------------------
BOOL CHost::PutClerkID()
{
	PutLeftJustified((const char *)pTRANDATA->ClerkID, strlen(pTRANDATA->ClerkID), 6 , ' ');
	return true;
}

//----------------------------------------------------------------------------
//!	\brief	Move terminal date & time to request message
//----------------------------------------------------------------------------
BOOL CHost::PutTerminalDateTime()
{
	// Host will send back the date in the response message
	Put('0', 12);
	return true;

}

//----------------------------------------------------------------------------
//!	\brief	Move message type to request message
//----------------------------------------------------------------------------
BOOL CHost::PutMessageType()
{
	switch (pTRANDATA->TranCode)
	{
		case TRAN_HOST_INIT:
		case TRAN_KEY_EXCHANGE:
		case TRAN_HANDSHAKRE:
		case TRAN_CLOSE_BATCH:
		case TRAN_BATCH_TOTAL:
			Put('A'); // Administrative
			break;
		default:
			Put('F'); // Financial
	}
	return TRUE;
}

//----------------------------------------------------------------------------
//!	\brief	Move message subtype to request message
//----------------------------------------------------------------------------
BOOL CHost::PutMessageSubtype()
{
	if (pTRANDATA->bReversal)
	{
		// Reversal
		if (pTRANDATA->ReversalType == MAC_REVERSAL)
		{
			Put('R'); // due to MAC validation failure

//			Put('0'); // Fix me later
		}
		else if (pTRANDATA->ReversalType == CHIP_REVERSAL)
		{
			Put('C'); // EMV Reversal
		}
		else if (pTRANDATA->ReversalType == TIMEOUT_REVERSAL)
		{
			Put('T'); // due to a timeout
		}
		else
		{
			return FALSE;
		}
	}
	else
	{
		// Administration and financial messages
		Put('O'); // Online
	}
	return TRUE;
}

//----------------------------------------------------------------------------
//!	\brief	Move transaction code to request message
//----------------------------------------------------------------------------
BOOL CHost::PutTransactionCode()
{
	switch (pTRANDATA->TranCode)
	{
		case TRAN_PURCHASE:
			Put("00"); // Purchase
			break;
		case TRAN_PREAUTH:
			Put("01"); // Preauth
			break;
		case TRAN_ADVICE:
		case TRAN_FORCE_POST:
			Put("02"); // Completion
			break;
		case TRAN_REFUND:
			Put("04"); // Return
			break;
		case TRAN_PURCHASE_VOID:
			Put("11"); // Purchase Correction
			break;
		case TRAN_REFUND_VOID:
			Put("12"); // Return Correction
			break;
		case TRAN_HOST_INIT:
			if (m_EMVDownload)
			{
				Put("91");
			}
			else
			{
				Put("90");
			}
			break;
		case TRAN_HANDSHAKRE:
			Put("95"); // Hand Shake
			break;
		case TRAN_KEY_EXCHANGE:
			Put("96"); // Key Exchange
			break;
		case TRAN_CLOSE_BATCH:
			Put("60"); // Close batch
			break;
		case TRAN_BATCH_TOTAL:
			Put("65"); // batch total
			break;
		default:
			return FALSE;
	}
	return TRUE;
}

//----------------------------------------------------------------------------
//!	\brief	Move processing flag 1 to request message
//----------------------------------------------------------------------------
BOOL CHost::PutProcessingFlag1()
{
	// CHost has requested that we set Processing Flag 1 to a value of ??all the time. 
	// The end of session will then be triggered by EOT from terminal instead of host.
	Put('0'); // keep connection open (for multiple messages)
	return TRUE;
}

//----------------------------------------------------------------------------
//!	\brief	Move processing flag 2 to request message
//----------------------------------------------------------------------------
BOOL CHost::PutProcessingFlag2()
{
//	if (m_EMVDownload)
	if (pCONFIG->EMVEnabled)
	{
		Put('5');		// Terminal is EMV capable
	}
	else 
	{
		Put('0');		//Terminal is not EMV capable
	}

	return TRUE;
}

//----------------------------------------------------------------------------
//!	\brief	Move processing flag 3 to request message
//----------------------------------------------------------------------------
BOOL CHost::PutProcessingFlag3()
{
	Put('0'); // Not Used
	return TRUE;
}

//----------------------------------------------------------------------------
//!	\brief	Move response code to request message
//----------------------------------------------------------------------------
BOOL CHost::PutResponseCode()
{
	Put("000");
	return TRUE;
}

//----------------------------------------------------------------------------
//!	\brief	Move amount to request message
//----------------------------------------------------------------------------
BOOL CHost::PutCashBackAmount()
{
	Put(pTRANDATA->CashbackAmount);
	return TRUE;
}

//----------------------------------------------------------------------------
//!	\brief	Move amount to request message
//----------------------------------------------------------------------------
BOOL CHost::PutAmount()
{
	Put(pTRANDATA->TotalAmount);
	return TRUE;
}

//----------------------------------------------------------------------------
//!	\brief	Move account type to request message
//----------------------------------------------------------------------------
BOOL CHost::PutAccountType()
{
//	if (pTRANDATA->CardType == CARD_DEBIT && pTRANDATA->bCtlsTransaction)
//		pTRANDATA->AcctType = ACCT_DEFAULT;

	Put(pTRANDATA->AcctType);
	
	return TRUE;
}

//----------------------------------------------------------------------------
//!	\brief	Move Invoice Number to request message
//----------------------------------------------------------------------------
BOOL CHost::PutInvoiceNumber()
{
	USHORT seqNo;
	if ( strlen(pTRANDATA->InvoiceNo) == 0)
	{
		DWORD size = sizeof(USHORT);

		USHORT flag;
		if (!CDataFile::Read((int)FID_TERM_SEQ_FLAG, (BYTE *)&flag, &size))
			flag = 0;

		if (CDataFile::Read((int)FID_TERM_SEQ_NUMBER, (BYTE *)&seqNo, &size,FALSE))
		{
			char temp[SZ_SEQ_NO+1];
			memset(temp, 0, sizeof(temp));
			if(flag)
			{
				seqNo++;
				if ( seqNo == 1000)
					seqNo = 1;
			}
		}
		else
			seqNo = 1;

		sprintf(pTRANDATA->InvoiceNo, "%07d", seqNo);

		m_iSeqNo = seqNo;
	}
	else
		seqNo = atoi(pTRANDATA->InvoiceNo);


	if (pTRANDATA->CardType == CARD_DEBIT)
		Put("       ");
	else
		Put(pTRANDATA->InvoiceNo);

	if(CDataFile::m_bTraining)
	{
		seqNo++;
		CDataFile::Save(FID_TERM_SEQ_NUMBER, (BYTE *)&seqNo, sizeof(USHORT));
	}

	return TRUE;
}

//----------------------------------------------------------------------------
//!	\brief	Move customer language to request message
//----------------------------------------------------------------------------
BOOL CHost::PutCustomerLanguage()
{
	if (pTRANDATA->CustLang == FRENCH || pTRANDATA->CustLang == LANG_FRENCH)
		Put('2');
	else
		Put('1');
//	Put(pTRANDATA->CustLang);
	return TRUE;
}

//----------------------------------------------------------------------------
//!	\brief	Move sequence number to request message
//----------------------------------------------------------------------------
BOOL CHost::PutFullSequenceNumber()
{
	BOOL result = PutShiftNumber() && PutBatchNumber() && PutSequenceNumber();
	Put('0');

	return result;
}

//----------------------------------------------------------------------------
//!	\brief	Move shift number to request message
//----------------------------------------------------------------------------
BOOL CHost::PutShiftNumber()
{
	USHORT seqNo;
	DWORD size = sizeof(USHORT);
	if (CDataFile::Read((int)FID_TERM_SHIFT_NUMBER, (BYTE *)&seqNo, &size))
	{
		char temp[SZ_SHIFT_NO+1];
		memset(temp, 0, sizeof(temp));
		STR_itoa(seqNo, SZ_SHIFT_NO, temp);
		Put(temp);
	}
	else
	{
		Put("001");
	}
	return TRUE;
}

//----------------------------------------------------------------------------
//!	\brief	Move batch number to request message
//----------------------------------------------------------------------------
BOOL CHost::PutBatchNumber()
{
	USHORT seqNo;
	DWORD size = sizeof(USHORT);
	if (CDataFile::Read((int)FID_TERM_BATCH_NUMBER, (BYTE *)&seqNo, &size))
	{
		char temp[SZ_BATCH_NO+1];
		memset(temp, 0, sizeof(temp));
		STR_itoa(seqNo, SZ_BATCH_NO, temp);
		Put(temp);
	}
	else
	{
		Put("001");
	}
	return TRUE;
}

//----------------------------------------------------------------------------
//!	\brief	Move sequential number to request message
//----------------------------------------------------------------------------
BOOL CHost::PutSequenceNumber()
{
	USHORT seqNo;
	DWORD size = sizeof(USHORT);

	if ( m_iSeqNo > 0)
	{
		char temp[SZ_SEQ_NO+1];
		memset(temp, 0, sizeof(temp));
		STR_itoa(m_iSeqNo, SZ_SEQ_NO, temp);
		Put(temp);
		return TRUE;
	}

	USHORT flag;
	if (!CDataFile::Read((int)FID_TERM_SEQ_FLAG, (BYTE *)&flag, &size))
		flag = 0;

	if (CDataFile::Read((int)FID_TERM_SEQ_NUMBER, (BYTE *)&seqNo, &size,FALSE))
	{
		char temp[SZ_SEQ_NO+1];
		memset(temp, 0, sizeof(temp));
		if(flag)
		{
			seqNo++;
			if ( seqNo == 1000)
				seqNo = 1;
			CDataFile::Save(FID_TERM_SEQ_NUMBER, (BYTE *)&seqNo, sizeof(USHORT));
		}

		STR_itoa(seqNo, SZ_SEQ_NO, temp);
		Put(temp);
	}
	else
	{
		seqNo = 1;
		Put("001");	//put Sequence = from 001 to 999
	}

//	if ( strlen(pTRANDATA->InvoiceNo) == 0)
//		sprintf(pTRANDATA->InvoiceNo, "%07d", seqNo);

	size = sizeof(USHORT);
	flag = 0;
	CDataFile::Save(FID_TERM_SEQ_FLAG, (BYTE *)&flag, size);

	m_iSeqNo = seqNo;
	return TRUE;
}

//----------------------------------------------------------------------------
//!	\brief	Move track 2 data to request message
//----------------------------------------------------------------------------
BOOL CHost::PutTrack2Data()
{
	if (strlen(pTRANDATA->Track2))
	{
		if( pTRANDATA->CardType == CARD_CREDIT)
		{
			if (pTRANDATA->TranCode == TRAN_ADVICE || 
				pTRANDATA->TranCode ==TRAN_PURCHASE_VOID ||
				pTRANDATA->TranCode ==TRAN_REFUND_VOID)
			{
				Put('M');
			}
			else
				Put(';');
		}
		else
			Put(';'); // Start sentinel

		if (  pTRANDATA->CardType == CARD_CREDIT && (pTRANDATA->TranCode == TRAN_ADVICE 
			|| ( pTRANDATA->TranCode == TRAN_PURCHASE_VOID ||pTRANDATA->TranCode == TRAN_REFUND_VOID)))
		{
			Put(pTRANDATA->Account);
			Put('=');
			Put(pTRANDATA->ExpDate);
			if (strlen(pTRANDATA->ServCode))
				Put(pTRANDATA->ServCode, sizeof(pTRANDATA->ServCode) );
		}
		else
			Put(pTRANDATA->Track2);
		Put('?'); // End sentinel
	}
	else // manual entry
	{
		// Entry ID
		if ((pTRANDATA->TranCode == TRAN_PURCHASE_VOID || pTRANDATA->TranCode == TRAN_REFUND_VOID) &&				
					pTRANDATA->bEmvTransaction)  
		{
			// Credit EMV Purchase Corrections and Credit EMV Refund Corrections
			Put(';');
		}
		else
		{
			Put('M');
		}
	
		// PAN
		Put(pTRANDATA->Account);
	
		// Separator character
		Put('=');
	
		// Expiration date (YYMM)
		Put(pTRANDATA->ExpDate);

		if (strlen(pTRANDATA->ServCode))
		{
			Put(pTRANDATA->ServCode, strlen(pTRANDATA->ServCode));
		}
		
		// SkipResponse member number
	
		// End sentinel
		Put('?');
	}

	return TRUE;
}

//----------------------------------------------------------------------------
//!	\brief	Move MAC value to request message
//----------------------------------------------------------------------------
BOOL CHost::PutMACValue()
{
	char mac[SZ_MAC+1];

	memset(mac, 0, sizeof(mac));

	CString traceData = CString(m_MACedData);
	TRACE(L"PutMACValue: %s\n", traceData);
	if (!CSEC::CalculateMAC((BYTE *)m_MACedData, strlen(m_MACedData), mac))
	{
		return FALSE;
	}
	
	Put(mac, sizeof(mac));
	return TRUE;
}

//----------------------------------------------------------------------------
//!	\brief	Move surcharge amount to request message
//----------------------------------------------------------------------------
BOOL CHost::PutSurchargeAmount()
{
//	Put(pTRANDATA->SurchargeAmount);
	Put("00000"); 
	return TRUE;
}

//----------------------------------------------------------------------------
//!	\brief	Move FID 6 to request message
//----------------------------------------------------------------------------
BOOL CHost::PutFID6EMV()
{
	PutFid6POSEntryMode();
	if (!pTRANDATA->bEmvTransaction)
	{
		return TRUE;
	}
	PutFid6TransactionCurrencyCode();
	PutFid6EMVRequestData();
	PutFid6EMVAdditionalRequestData();
	return TRUE;
}

//----------------------------------------------------------------------------
//!	\brief	Move FID 6 to request message
//----------------------------------------------------------------------------
BOOL CHost::PutFID6EMVReversal()
{
	if (!pTRANDATA->bEmvTransaction)
	{
		return FALSE;
	}
	PutFid6EMVRequestData();
	PutFid6EMVAdditionalRequestData();
	return TRUE;
}

//----------------------------------------------------------------------------
//!	\brief	Move AID to request message
//----------------------------------------------------------------------------
BOOL CHost::PutAID()
{
	Put(RS);
	Put('B');
	Put((char *)pTRANDATA->EmvData.AID);
	return true; //TDGJP
}

//----------------------------------------------------------------------------
//!	\brief	Move EMV terminal capabilities to request message
//----------------------------------------------------------------------------
BOOL CHost::PutFid9EMVTerminalCapabilities()
{
	if (pTRANDATA->bEmvTransaction)
	{
		// Move identifier
		Put(RS);
		Put('C');
		PutBCDAscii(pTRANDATA->EmvData.TerminalCapabilities, sizeof(pTRANDATA->EmvData.TerminalCapabilities));
		return TRUE;
	}
	return FALSE;
}

//----------------------------------------------------------------------------
//!	\brief	Move authorization number to request message
//----------------------------------------------------------------------------
BOOL CHost::PutAuthorizationNumber()
{
	Put(pTRANDATA->AuthCode); //PJ change from OrigAuthCode to AuthCode
	return TRUE;
}

//----------------------------------------------------------------------------
//!	\brief	Move move revised and other amount to request message
//----------------------------------------------------------------------------
BOOL CHost::PutRevisedAndOtherAmount()
{
	Put('0');  // Jun Add,should be fix later
	return TRUE;


	if (pTRANDATA->bEmvTransaction)
	{
		// ICC Other Amount (Cashback Amount - EMV Tag 9F03)
		// Amount Other (in chip card transactions)
		PutCashBackAmount();
		return TRUE;
	}

	// Revised Amount (in magnetic stripe card transactions)
	Put('0');

	return TRUE;
}

//----------------------------------------------------------------------------
//!	\brief	Move encrypted PIN to request message
//----------------------------------------------------------------------------
BOOL CHost::PutPINBlock()
{
	if (/*pTRANDATA->bCtlsTransaction || */pTRANDATA->bEmvTransaction)
		return FALSE;
//	strcpy(pTRANDATA->PINBlock,"0123456789ABCDEF");
	Put(pTRANDATA->PINBlock);
	
	return TRUE;
}

//----------------------------------------------------------------------------
//!	\brief	Move POS Condition Code
//----------------------------------------------------------------------------
BOOL CHost::PutPOSConditionCode()
{
	if (!pCONFIG->EMVEnabled)
	{
		// This field should only be included if the terminal supports EMV.
		return FALSE;
	}

	Put("27"); //unattended teminal

	return TRUE;

	if (pTRANDATA->TranCode == TRAN_PREAUTH
		||pTRANDATA->TranCode == TRAN_ADVICE
		||pTRANDATA->TranCode == TRAN_FORCE_POST)
	{
		Put("06");
	}
	else if (IsManualPAN() && !pTRANDATA->bCardPresent)
	{
		// 05 = Customer present but card not present
		Put("05");
	}
	else
	{
		// 00 = Normal presentment
		Put("00");
	}

	return TRUE;
}

//----------------------------------------------------------------------------
//!	\brief	Move download data to request message
//----------------------------------------------------------------------------
BOOL CHost::PutDownload()
{
	PutCategoryCode();
	PutAccessCode();
	PutProcessingFlag();
	PutDownloadFiller();
	return TRUE;
}

//----------------------------------------------------------------------------
//!	\brief	Move initial download data to request message
//----------------------------------------------------------------------------
BOOL CHost::PutInitialDownload()
{
	if (m_ContinuationRequest)
	{
		return FALSE;
	}

	Put(RS);
	Put('A');

	// Vendor
	Put("07"); // 09 ?XAC		// FIXME: get from CHost

	// Device Model
	PutRightFilled(pCONFIG->DeviceModel, 10, ' ');

	// Application Version Number
	PutRightFilled(APP_VERS, 10, ' ');

	Put("01"); // Internal PINPad

	// Communication
	Put("01"); // IP

	// EMV Kernel Version
	if (pCONFIG->EMVEnabled)
	{
		BYTE Ver[10];
		DWORD size;
		CDataFile::Read(FID_EMV_VERSION,Ver,&size);
		Ver[4] = 0;

		char version[16+1];
		sprintf(version,"%d%d%d%d",Ver[0],Ver[1],Ver[2],Ver[3]);
		PutRightFilled(version, 40, ' ');
	}
	else
	{
		Put(' ', 40);
	}

	return TRUE;
}

//----------------------------------------------------------------------------
//!	\brief	Move FID 9 to request message
//----------------------------------------------------------------------------
BOOL CHost::PutFID9()
{
	BOOL result = FALSE;

	if (pTRANDATA->TranCode == TRAN_HOST_INIT)
	{
		result |= PutInitialDownload();
	}

	result |= PutAID();

	result |= PutFid9EMVTerminalCapabilities();

	return result;
}

//----------------------------------------------------------------------------
//!	\brief	Move category code to request message
//----------------------------------------------------------------------------
BOOL CHost::PutCategoryCode()
{
	// If more data for DID
	if (m_MoreData)
	{
		Put("11"); // continue receiving data for current DID
	}
	else
	{
		Put("00");
	}
	return TRUE;
}

//----------------------------------------------------------------------------
//!	\brief	Move access code to request message
//----------------------------------------------------------------------------
BOOL CHost::PutAccessCode()
{
	if (m_ContinuationRequest)
	{
		Put("2"); // this is a continuation
	}
	else
	{
		Put("1"); // read first download record		
	}
	return TRUE;
}

//----------------------------------------------------------------------------
//!	\brief	Move processing code to request message
//----------------------------------------------------------------------------
BOOL CHost::PutProcessingFlag()
{
	if (m_LastDID)
	{
		Put(m_LastDID); // last processing downloaded field
	}
	else
	{
		Put(' '); // not set
	}

	Put(' ');

	return TRUE;
}

//----------------------------------------------------------------------------
//!	\brief	Move host initialization filler to request message
//----------------------------------------------------------------------------
BOOL CHost::PutDownloadFiller()
{
	if (strlen(m_DownloadFiller)== 0)
	{
		Put('0', 10);
		return TRUE;
	}

	if (LengthCheck(m_DownloadFiller, sizeof(m_DownloadFiller)))
	{
		Put(m_DownloadFiller, sizeof(m_DownloadFiller));
	}
	else
	{
		Put('0', 10);
	}
	return TRUE;
}

//----------------------------------------------------------------------------
//!	\brief	Move POS entry mode to request message
//----------------------------------------------------------------------------
void CHost::PutFid6POSEntryMode()
{
	Put(RS);	// 0x1E

	// Subfield indentifier
	Put("E");

	// Entry Mode
	if (IsManualPAN())
	{
		if (pTRANDATA->bFallback)
		{
			Put("81");	// EMV Fallback to Manual
		}
		else
		{
			Put("01");	// Manually
		}
	}
	else if (IsSwiped())
	{
		Put("02");		// Magnetic stripe
	}
	else if (IsChip())
	{
		Put("05"); // Integrated circuit card
	}
	else if (IsCtls())
	{
		Put("07"); // Contactless chip card transaction
	}
	else if (IsCtlsMSR())
	{
		Put("91"); // Contactless MSD
	}
	else
	{
		Put("00"); // Unspecified
	}

	// Terminal PIN Entry Capability
	Put('1'); // PIN entry capability
}

//----------------------------------------------------------------------------
//!	\brief	Move transaction currency code to request message
//----------------------------------------------------------------------------
void CHost::PutFid6TransactionCurrencyCode()
{
	Put(RS);	// sub-field separator 0x1E

	// Subfield indentifier
	Put("I");

	{
		// Default for transactions performed at Canadian POS devices.
		Put("124"); 
	}
}

//----------------------------------------------------------------------------
//!	\brief	Move EMV request data to request message
//----------------------------------------------------------------------------
void CHost::PutFid6EMVRequestData()
{
	// Only for EMV transactions
	if (!pTRANDATA->bEmvTransaction)
	{
		return;
	}

	// Credit EMV Refunds/void do not need this subfield
	if( pTRANDATA->CardType == CARD_CREDIT)
	{
		if (pTRANDATA->TranCode == TRAN_REFUND || 
			pTRANDATA->TranCode ==TRAN_PURCHASE_VOID ||
			pTRANDATA->TranCode ==TRAN_REFUND_VOID ||
			pTRANDATA->TranCode ==TRAN_FORCE_POST)
		{
			return;
		}
	}
	Put(RS);

	// Subfield indentifier
	Put("O");

	// Card Scheme
	Put("01"); // EMV 2000 (EMV version 2)

	// Cryptogram Information Data - 9F27
	//if ((pTRANDATA->bEmvCtlsTransaction && CEMV::IsVisaRID(pTRANDATA->EmvData.AID)))
	//	Put("  ");
	//else
		PutBCDAscii(pTRANDATA->EmvData.CryptogramInformationData, 1);

	// Terminal Country Code - 9F1A
	Put((const char *)pCONFIG->EmvData.TerminalCountryCode, sizeof(pCONFIG->EmvData.TerminalCountryCode));
	// EMV Date - 9A
	SetTransactionDateTime();
	Put(pTRANDATA->TranDate);

	// Authorization Request Cryptogram - 9F26
	PutBCDAscii(pTRANDATA->EmvData.ApplicationCryptogramForRequest, 
										sizeof(pTRANDATA->EmvData.ApplicationCryptogramForRequest));

	// Application Interchange Profile - 82
	PutBCDAscii(pTRANDATA->EmvData.ApplicationInterchangeProfile, 
										sizeof(pTRANDATA->EmvData.ApplicationInterchangeProfile));

	// Application Transaction Counter - 9F36
	PutBCDAscii(pTRANDATA->EmvData.ApplicationTransactionCounter, 
										sizeof(pTRANDATA->EmvData.ApplicationTransactionCounter));

	// Unpredictable Number - 9F37
	PutBCDAscii(pTRANDATA->EmvData.UnpredictableNumber, 
										sizeof(pTRANDATA->EmvData.UnpredictableNumber));

	// Terminal Verification Results - 95
	PutBCDAscii(pTRANDATA->EmvData.TerminalVerificationResultsARQC, 
										sizeof(pTRANDATA->EmvData.TerminalVerificationResultsARQC));

  // Cryptogram Transaction Type - 9C
	PutBCDAscii(pTRANDATA->EmvData.TransactionType, 
										sizeof(pTRANDATA->EmvData.TransactionType));

	// Transaction Currency Code - 5F2A
	//PutBCDRight(pTRANDATA->EmvData.TransactionCurrencyCode, 
	//									sizeof(pTRANDATA->EmvData.TransactionCurrencyCode), 3);
	Put("124");

	// Cryptogram Amount - 9F02
	PutBCDAscii(pTRANDATA->EmvData.AmountAuthorizedNumeric, 
										sizeof(pTRANDATA->EmvData.AmountAuthorizedNumeric));

	// Issuer Application Data - 9F10
	PutBCDAscii(pTRANDATA->EmvData.IssuerAuthenticationData, 
										pTRANDATA->EmvData.IssuerAuthenticationDataLen);
}

//----------------------------------------------------------------------------
//!	\brief	Move EMV additional request data to request message
//----------------------------------------------------------------------------
void CHost::PutFid6EMVAdditionalRequestData()
{
	// Only for EMV transactions
	if (!pTRANDATA->bEmvTransaction)
	{
		return;
	}

	// Credit EMV Refunds/void do not need this subfield

	if( pTRANDATA->CardType == CARD_CREDIT)
	{
		if (pTRANDATA->TranCode == TRAN_REFUND || 
			pTRANDATA->TranCode ==TRAN_PURCHASE_VOID ||
			pTRANDATA->TranCode ==TRAN_REFUND_VOID ||
			pTRANDATA->TranCode ==TRAN_FORCE_POST)
		{
			return;
		}
	}

	Put(RS);

	// Subfield indentifier
	Put("P");

	// Card Scheme
	Put("01"); // EMV 2000 (EMV version 2)

	if (pTRANDATA->EmvData.ApplicationPANSequenceNumber[0] != 0)
	{
		// Application PAN Sequence Number - 5F34
		PutBCDAscii(pTRANDATA->EmvData.ApplicationPANSequenceNumber, 1);
	}
	else
	{
		Put("  ");
	}

	// EMV Terminal Type - 9F35
	//Put(pTRANDATA->EmvData.TerminalType[0]);

	if ( (strcmp(pTRANDATA->ServType,"P0") == 0) && IsCtls()) //9f35 terminal type
		Put("24");
	else
		Put("25");

	// Cardholder Verification Results (CVM) - 9F34
	PutBCDAscii(pTRANDATA->EmvData.CardholderVerificationMethodResults, 
									sizeof(pTRANDATA->EmvData.CardholderVerificationMethodResults));

	// Application Version Number - 9F09
	PutBCDAscii(pTRANDATA->EmvData.ApplicationVersionNumber, 
								sizeof(pTRANDATA->EmvData.ApplicationVersionNumber));	// FIXME
	
//	Put("001C");

	// Dedicated File Name - 84	
	if (strlen(pTRANDATA->EmvData.ICC_AID) > 0)
		Put((char *)pTRANDATA->EmvData.ICC_AID);	
	else
		Put((char *)pTRANDATA->EmvData.AID);	
}

//----------------------------------------------------------------------------
//!	\brief	Move field to request message and save its data for
//!					MACing
//----------------------------------------------------------------------------
BOOL CHost::PutMACedField(char id)
{
	char *start = &m_Request[m_RequestLength];
	BOOL result = PutField(id);
	if (result)
	{
		char *end = &m_Request[m_RequestLength];
		strncat(m_MACedData, start, end-start);
	}
	return result;
}

//----------------------------------------------------------------------------
//!	\brief	Move field identifier to request message
//----------------------------------------------------------------------------
BOOL CHost::PutField(char id)
{
	BOOL result = FALSE;
	switch (id)
	{
		case 'B' :
			result = PutAmount();
			break;

		case 'C' :
			result = PutRevisedAndOtherAmount();
			break;

		case 'D' :
			result = PutAccountType();
			break;

		case 'F' :
			result = PutAuthorizationNumber();
			break;

		case 'G' :
			result = PutMACValue();
			break;

		case 'S' :
			result = PutInvoiceNumber();
			break;
	
		case 'U' :
			result = PutCustomerLanguage();
			break;

		case 'V' :
			result = PutDownload();
			break;

		case 'b' :
			result = PutPINBlock();
			break;

		case 'e' :
			result = PutPOSConditionCode();
			break;

		case 'h' :
			result = PutFullSequenceNumber();
			break;

		case 'q' :
			result = PutTrack2Data();
			break;
	
		case 'y' :
			result = PutSurchargeAmount();
			break;
	
		case '6' :
			result = PutFID6EMV();
			break;
	
		case '9' :
			result = PutFID9();
			break;

		default:
			result = FALSE;
	}

	return result;
}

//----------------------------------------------------------------------------
//!	\brief	Format the host request header.
//!	\use m_Request as the buffer
//----------------------------------------------------------------------------
BOOL CHost::FormatHeader()
{
	memset(m_MACedData, 0, sizeof(m_MACedData));
	ClearRequest();
	PutDeviceType();
	PutTransmissionNumber();
	PutTerminalID();
	PutClerkID();
//	Put(' ', 6); // RFU
	PutTerminalDateTime();
	PutMessageType();
	PutMessageSubtype();
	PutTransactionCode();
	PutProcessingFlag1();
	PutProcessingFlag2();
	PutProcessingFlag3();
	PutResponseCode();
	strncat(m_MACedData, &m_Request[2], 2 );
	strncat(m_MACedData, &m_Request[4], 16 );
	strncat(m_MACedData, &m_Request[40], 2 );
	return TRUE;
}

//----------------------------------------------------------------------------
//!	\brief	Format the host request.
//!	\use m_Request as the buffer
//----------------------------------------------------------------------------
BOOL CHost::FormatRequestData()
{
	char FIDs[20];
	BOOL unsupported = FALSE;

	memset(FIDs, 0, sizeof(FIDs));
	memset(m_MACedData, 0, sizeof(m_MACedData));
	ClearRequest();

	FormatHeader();

	switch (pTRANDATA->TranCode)
	{
		case TRAN_PURCHASE:
		case TRAN_REFUND:
			if (pTRANDATA->bEmvTransaction)
			{
				strcpy(FIDs, "BDSUehqyG69");
			}
			else
			{
				if (pTRANDATA->CardType == CARD_DEBIT)
				{
					strcpy(FIDs, "BDSUbehqyG6");
				}
				else
				{
					strcpy(FIDs, "BDSUehqyG6");
				}
			}
			break;

		case TRAN_PREAUTH:
			if (pTRANDATA->bEmvTransaction)
			{
				strcpy(FIDs, "BDSUehqyG69");
			}
			else
			{
				strcpy(FIDs, "BDSUehqyG6");
			}
			break;
		case TRAN_ADVICE:
		case TRAN_FORCE_POST:
			if (pTRANDATA->bEmvTransaction)
			{
				strcpy(FIDs, "BDFSUehqyG69");
			}
			else
			{
				strcpy(FIDs, "BDFSUehqyG6");
			}
			break;

		case TRAN_PURCHASE_VOID:
		case TRAN_REFUND_VOID:
			if (pTRANDATA->bEmvTransaction)
			{
				strcpy(FIDs, "BCDFSUbehqyG69");
			}
			else
			{
				if (pTRANDATA->CardType == CARD_DEBIT)
				{
					strcpy(FIDs, "BCDFSUbehqyG6");
				}
				else
				{
					strcpy(FIDs, "BCDFSUehqyG6");
				}
			}
			break;

		case TRAN_HOST_INIT:
			strcpy(FIDs, "V9");
			break;

		case TRAN_KEY_EXCHANGE:
		case TRAN_HANDSHAKRE:
		case TRAN_CLOSE_BATCH:
		case TRAN_BATCH_TOTAL:
			Put(ETX);  //TDGPJ 2012NOV28
			break;

		default:
			unsupported = TRUE;
			break;
	}

	// if unsupported transaction
	if (unsupported)
	{
		ClearRequest();
		SetCommStatus(ST_REQUEST_ERROR);
		return FALSE;
	}

	char ReqMacFids[50];
	DWORD size;

	memset(ReqMacFids, 0, sizeof(ReqMacFids));
	CDataFile::Read(FID_TERM_MAC_REQ_FIDS, (BYTE*)ReqMacFids, &size);

	for (USHORT i=0; i < strlen(FIDs); ++i)
	{
		int currLen = m_RequestLength;
		BOOL result = FALSE;

		Put(FS);
		Put(FIDs[i]);

		if (memchr(ReqMacFids, FIDs[i], strlen(ReqMacFids)))
		{
			result = PutMACedField(FIDs[i]);
		}
		else
		{
			result = PutField(FIDs[i]);
		}

		// if field is invalid
		if (!result)
		{
			// Rollback
			Back(m_RequestLength - currLen);
		}
	}

	return TRUE;
}

//----------------------------------------------------------------------------
//!	\brief	Format EMV Reversal Request
//!	\use m_Request as the buffer
//----------------------------------------------------------------------------
BOOL CHost::FormatEMVReversal()
{
	// EMV reversal request should echo back the host response with
	// fid 6O and 6P added

	// Change Message SubType
	m_Request[39] = 'C';
	PutFID6EMVReversal();
	return TRUE;
}

//-----------------------------------------------------------------------------
//!	\brief	Get byte from response
//!	\param	x - destination byte
//-----------------------------------------------------------------------------
inline void CHost::Get(BYTE &x)
{
	x = *m_Response;
	m_Response++;
}

//-----------------------------------------------------------------------------
//!	\brief	Get byte from response
//!	\param	x - destination byte
//-----------------------------------------------------------------------------
inline void CHost::Get(char &x)
{
	x = *m_Response;
	m_Response++;
}

//-----------------------------------------------------------------------------
//!	\brief	Get bytes from response
//!	\param	dest - destination buffer
//!	\param	length - number of bytes to be copied
//-----------------------------------------------------------------------------
inline void CHost::Get(BYTE *dest, int length)
{
	memcpy(dest, m_Response, length);
	m_Response += length;
}

//-----------------------------------------------------------------------------
//!	\brief	Get bytes from response
//!	\param	dest - destination buffer
//!	\param	maxLength - max number of bytes to be copied(without FS)
//!	\note	Field is right padded with 0x0 till maxLength bytes
//-----------------------------------------------------------------------------
inline void CHost::GetFS(BYTE *dest, int maxLength)
{
	memset(dest, 0, maxLength);
	int length = FieldLen(maxLength);
	memcpy(dest, m_Response, length);
	m_Response += length;
	if (*m_Response == FS)
	{
		++m_Response;
	}
}

//-----------------------------------------------------------------------------
//!	\brief	Get bytes from response
//!	\param	dest - destination buffer
//!	\param	maxLength - maiximal number of bytes to be copied(without GS)
//-----------------------------------------------------------------------------
inline void CHost::GetGS(BYTE *dest, int maxLength)
{
	int length = SubfieldLen(maxLength);
	memcpy(dest, m_Response, length);
	m_Response += length;
	if (*m_Response == GS)
	{
		++m_Response;
	}
}

//-----------------------------------------------------------------------------
//!	\brief	Get 2 ASCII chars from response and convert it to hex
//!	\param	x - converted 2 ASCII characters
//-----------------------------------------------------------------------------
inline void CHost::GetAscHex(BYTE &x) 
{
	char ascii[3];
	BYTE n;

	memset(ascii, 0, 3);
	Get((BYTE *)ascii, 2);
	STR_PackData(&n, ascii, 2);
	x = n;
}

//-----------------------------------------------------------------------------
//!	\brief	Get ASCII chars from response and convert it to hex
//!	\param	dest - destination buffer
//!	\param	length - destination buffer size
//-----------------------------------------------------------------------------
inline void CHost::GetAscHex(BYTE *dest, int length) 
{
	char *ascii = new char[2*length + 1];
	memset(ascii, 0, 2*length + 1);

	Get((BYTE *)ascii, length*2);
	
	STR_PackData(dest, ascii, length*2); //TDGJP

	delete[] ascii;
}

//-----------------------------------------------------------------------------
//!	\brief	Get string from response
//!	\param	str - destination string
//!	\param	size - size of string including zero-character
//-----------------------------------------------------------------------------
inline void CHost::GetCStr(char *str, int size)
{
	memcpy(str, m_Response, size);
	str[size] = 0; // make C-string
	m_Response += size;
}

//-----------------------------------------------------------------------------
//!	\brief	Get BCD number from response
//!	\param	num	- pointer to BCD number buffer
//!	\param	numSize - size in bytes of BCD number buffer
//!	\param	length - count of ASCII characters to get from response
//!	\return	TRUE - number correctly converted
//!			FALSE - overflow occurs, mostright digits taken
//!	\note	<2 * numSize> can be greater, lesser or equal to <length>
//!			<num> will be filled with leading zeroes if needed.
//!			The mostright digits will be taken if <length> is greater than
//!			<2 * num>.
//-----------------------------------------------------------------------------
BOOL CHost::GetBCDR(BYTE *num, int numSize, int length)
{
	BOOL overflow = FALSE;
	char *s = new char[length + 1];

	// Copy from response buffer
	memset(s, 0, sizeof(s));
	memcpy(s, m_Response, length);

	int n = numSize * 2 - length;

	// If number in response is smaller than requested
	if (n > 0)
	{
		// Add leading zeroes
		for (int i= 0; i< n; i++) //TDGJP
			strcat(s, "0");
	}
	// If number in response is bigger than requested
	else if (n < 0)
	{
		int i;

		// Check overflow
		for (i = 0; (i < length) && (s[i] == '0'); ++i) {}; // calculate leading zeroes
		if ((length - i) > (2 * numSize))
		{
			overflow = TRUE;
		}

		// Remove leftmost digits
		memcpy(s, &s[n], strlen(s)-n);
	}

	// Convert to BCD
	STR_PackData(num, s, length);

	SkipResponse(length);

	delete [] s;
	return !overflow;
}

//-----------------------------------------------------------------------------
//!	\brief	Get int from response
//!	\param	x - number
//!	\param	size - count of ASCII characters to use from response
//!	\note	sscanf with format '%<count>d' is used to get int from response
//-----------------------------------------------------------------------------
void CHost::GetInt(int &x, int size)
{
	char temp[100];
	GetCStr(temp, size);
	x = STR_atoi(temp, size);
}

//-----------------------------------------------------------------------------
//!	\brief	Get long int from response
//!	\param	x - number
//!	\param	size - count of ASCII characters to use from response
//!	\note	sscanf with format '%<count>ld' is used to get int from response
//-----------------------------------------------------------------------------
void CHost::GetLongInt(long int &x, int size)
{
	char temp[100];
	GetCStr(temp, size);
	char* p = temp;
	int len = size;
	while( *p < '0' || *p > '9')
	{
		p++;
		len--;
	}

	x = STR_atol( p , len);
}

//-----------------------------------------------------------------------------
//!	\brief	Calculate field length in response
//!	\param	maxlen - maximal allowed length
//-----------------------------------------------------------------------------
int CHost::FieldLen(int maxlen)
{
	int length = 0;
	while (!FieldEnd(m_Response + length))
	{
		if (length >= maxlen)
		{
			break;
		}
		length++;
	}
	return length;
}

//-----------------------------------------------------------------------------
//!	\brief	Calculate GS-separated subfield length in response
//!	\param	maxlen - maximal allowed length
//-----------------------------------------------------------------------------
int CHost::SubfieldLen(int maxlen)
{
	int length = 0;
	while (!SubfieldEnd(m_Response + length))
	{
		if (length >= maxlen)
		{
			break;
		}
		length++;
	}
	return length;
}

//-----------------------------------------------------------------------------
//!	\brief	Calculate subfield length in response
//!	\param	maxlen - maximal allowed length
//!	\note	RS(0x1E) is mark of subfield end
//-----------------------------------------------------------------------------
int CHost::SubfieldLenRS(int maxlen)
{
	int length = 0;
	while (!SubfieldEndRS(m_Response + length))
	{
		if (length >= maxlen)
		{
			break;
		}
		length++;
	}
	return length;
}

//-----------------------------------------------------------------------------
//!	\brief	Check if it is end of field in buffer
//!	\param	data - buffer to check from, default - response buffer
//-----------------------------------------------------------------------------
BOOL CHost::FieldEnd(const char *data /* = 0 */)
{
	if (!data)
	{
		data = m_Response;
	}

	switch(*data)
	{
		case FS:		// end of field
		case 0x00:	// end data
			return TRUE;
	}
	return FALSE;
}

//-----------------------------------------------------------------------------
//!	\brief	Check if it is end of GS-separted subfield in buffer
//!	\param	data - buffer to check from, default - response buffer
//-----------------------------------------------------------------------------
BOOL CHost::SubfieldEnd(const char *data /* = 0 */)
{
	if (!data)
	{
		data = m_Response;
	}

	switch(*data)
	{
		case FS:		// end of field
		case GS:		// end of subfield
		case 0x00:	// end data
			return TRUE;
	}
	return FALSE;
}

//-----------------------------------------------------------------------------
//!	\brief	Check if it is end of subfield in buffer
//!	\param	data - buffer to check from, default - response buffer
//!	\note	RS(0x1E) is mark of subfield end
//-----------------------------------------------------------------------------
BOOL CHost::SubfieldEndRS(const char *data /* = 0 */)
{
	if (!data)
	{
		data = m_Response;
	}

	switch(*data)
	{
		case RS:		// end of field
		case FS:		// end of field
		case 0x00:	// end data
			return TRUE;
	}
	return FALSE;
}

//-----------------------------------------------------------------------------
//!	\brief	Skip field data and FS
//-----------------------------------------------------------------------------
void CHost::SkipFieldFS()
{
	while (!FieldEnd(m_Response))
	{
		SkipResponse(1);
	}

	if (*m_Response == FS) 
	{
		SkipResponse(1);
	}
}

//-----------------------------------------------------------------------------
//!	\brief	Skip num bytes in response
//-----------------------------------------------------------------------------
inline void CHost::SkipResponse(int num)
{
	m_Response += num;
}

//----------------------------------------------------------------------------
//!	\brief	Process amount in response message
//!	\return	TRUE  - field processed
//!					FALSE - failed to process
//----------------------------------------------------------------------------
BOOL CHost::ProcessAmount()
{
	char temp[19];
	GetCStr(temp, FieldLen(99));

	sprintf(pTRANDATA->AuthAmount,"%d",atoi(temp));

	// Check for partial authorization
	if (atoi(pTRANDATA->AuthAmount) < atoi(pTRANDATA->TotalAmount)) //TDGJP
	{
		TRACE(L"Partial authorization detected");
		int dueAmount = atoi(pTRANDATA->TotalAmount) - atoi(pTRANDATA->AuthAmount);
		STR_itoa(dueAmount, SZ_AMOUNT, pTRANDATA->DueAmount);
		memcpy(pTRANDATA->RequestAmount, pTRANDATA->TotalAmount, sizeof(pTRANDATA->TotalAmount));
		memcpy(pTRANDATA->TotalAmount, pTRANDATA->AuthAmount, sizeof(pTRANDATA->AuthAmount));

		// Flag this transaction as partially approved
		pTRANDATA->bPartialAuth = TRUE;
	}
	
	return TRUE;
}

//----------------------------------------------------------------------------
//!	\brief	Process authorization code in response message
//!	\return	TRUE  - field processed
//!					FALSE - failed to process
//----------------------------------------------------------------------------
BOOL CHost::ProcessAuthorizationCode()
{
	GetCStr(pTRANDATA->AuthCode, FieldLen(9));
	return TRUE;
}

//----------------------------------------------------------------------------
//!	\brief	Process MAC value in response message
//!	\return	TRUE  - field processed
//!					FALSE - failed to process
//----------------------------------------------------------------------------
BOOL CHost::ProcessMACValue()
{
	GetCStr(m_ReceivedMAC, SZ_MAC);
	return TRUE;
}

//----------------------------------------------------------------------------
//!	\brief	Process MAC value in response message
//!	\return	TRUE  - field processed
//!					FALSE - failed to process
//----------------------------------------------------------------------------
BOOL CHost::ProcessNewMACKey()
{
	DWORD size=SZ_KEY/2;
	memset(m_BeiKey,0,16);
	CDataFile::Read(FID_TERM_MAC_KEY, m_BeiKey, &size);
	m_bNewKey = TRUE;


	BYTE key[SZ_KEY];
	GetBCDR(key, SZ_KEY/2, SZ_KEY);
	CDataFile::Save(FID_TERM_MAC_KEY, key, SZ_KEY/2);

	return TRUE;
}

//----------------------------------------------------------------------------
//!	\brief	Process Terminal Posting Date in response message
//!	\return	TRUE  - field processed
//!					FALSE - failed to process
//----------------------------------------------------------------------------
BOOL CHost::ProcessTerminalPostingDate()
{
	char buf[SZ_AUTOSET+1];
	GetCStr(buf, SZ_AUTOSET);

	TCHAR buf1[SZ_AUTOSET+1];
	if(CDataFile::Read(L"SETTLE TYPE",buf1) && CString(buf1) == L"Auto")
	{
		char temp[SZ_AUTOSET+1];
		char temp1[SZ_AUTOSET+1];
		DWORD size=SZ_AUTOSET;

		{
			CTimeUtil systemTime;
			systemTime.GetSystemDate(temp);
		}

		if ( CDataFile::Read(FID_TERM_AUTOSET_TIME, (BYTE*)temp1, &size))
		{
			temp1[size] = 0;
			if ( strcmp(buf,temp1) == 0)
				return TRUE;
		}
		else
			CDataFile::Save(FID_TERM_AUTOSET_TIME, (BYTE *) &temp, strlen(temp)+1);

		int iTemp = strcmp(temp,buf) < 0 ? 1:0;
		if (iTemp == 1)
		{
			CDataFile::Save(FID_TERM_AUTOSET_FLAG, (BYTE *) &iTemp, sizeof(int));
			CDataFile::Save(FID_TERM_AUTOSET_TIME, (BYTE *) &buf, strlen(buf)+1);
		}

	}
	return TRUE;
}

//----------------------------------------------------------------------------
//!	\brief	:Close Batch
//----------------------------------------------------------------------------
void CHost::CloseBatch()
{
		char Temp[4][128];
		strcpy(Temp[0],TRANSACTION_LOG_FILE);
		strcpy(Temp[1],TOTALS_FILE);
		strcpy(Temp[2],BATCH_LOG_FILE);
		strcpy(Temp[3],HOST_TOTALS_FILE);
	
		CDataFile::UpdateStatistics(0,TRUE);

		for(int i=0; i<= 3;i++)
		{
			if(strlen(Temp[i]) == 0)
				continue;
			CRecordFile file;
			AP_FILE_HANDLER fh;
			if (!file.Open(Temp[i], &fh))
			{
				TRACE(L"Can't open file!\n");
				continue;
			}
			file.Delete();
		}
		CDataFile::Delete(FID_CLOSE_BATCH_FLAG);

}

//----------------------------------------------------------------------------
//!	\brief	:Process Reinitilize Or MFC Close
//!	\return	TRUE  - field processed
//!					FALSE - failed to process
//----------------------------------------------------------------------------
BOOL CHost::ProcessReinitilizeOrMFCClose()
{
	char temp = *m_Response;
	SkipResponse(1);

	if ( pTRANDATA->TranCode == TRAN_HOST_INIT || pTRANDATA->TranCode == TRAN_KEY_EXCHANGE )
		return TRUE;

	int status;
	TCHAR buf1[12] = {L""};
	switch (temp)
	{
	case 'Y': //Terminal must re-initialize
	case 'E': //EMV Initialization Required
		status = STS_DLL_FORCEDOWNLOAD;
		CDataFile::Save(FID_TERM_HOST_INIT_STATUS, (BYTE *)&status, sizeof(int));
		break;
	case 'B': //Batch Close Required 
		if(CDataFile::Read(L"SETTLE TYPE",buf1) && CString(buf1) == L"Auto")
			break;

		status = 1; //2 = auto
		CDataFile::Save(FID_CLOSE_BATCH_FLAG, (BYTE *)&status, sizeof(int));
		break;
	}
	return TRUE;
}
//----------------------------------------------------------------------------
//!	\brief	Process available balance in response message
//!	\return	TRUE  - field processed
//!					FALSE - failed to process
//----------------------------------------------------------------------------
BOOL CHost::ProcessAvailableBalance()
{
	char temp[19];
	GetCStr(temp, FieldLen(18));
	temp[18] = 0;

	int i;
	for ( i = 1 ; i < 18; i++)
		if ( temp[i] != '0')
			break;

	pTRANDATA->AvailableBalance[0] = temp[0];
	memcpy(&pTRANDATA->AvailableBalance[1],&temp[i],18-i);

	return TRUE;
}

//----------------------------------------------------------------------------
//!	\brief	Process PIN encryption key in response message
//!	\return	TRUE  - field processed
//!					FALSE - failed to process
//----------------------------------------------------------------------------
BOOL CHost::ProcessPINEncryptionKey()
{
	BYTE key[SZ_KEY];
	GetBCDR(key, SZ_KEY/2, SZ_KEY);
	CDataFile::Save(FID_TERM_PIN_KEY, key, SZ_KEY/2);
	return TRUE;
}

//----------------------------------------------------------------------------
//!	\brief	Process ISO response code in response message
//!	\return	TRUE  - field processed
//!					FALSE - failed to process
//----------------------------------------------------------------------------
BOOL CHost::ProcessISOResponseCode()
{
	GetCStr(pTRANDATA->ISORespCode, SZ_ISO_CODE);

	if ((strcmp(pTRANDATA->ISORespCode, "87") == 0) ||		// Pin key sync error
			(strcmp(pTRANDATA->ISORespCode, "88") == 0) ||		// Mac key sync error
			(strcmp(pTRANDATA->ISORespCode, "63") == 0))			// Mac value incorrect
	{
		m_KeyError = TRUE;
		++m_KeyErrorCount;

	}

	return TRUE;
}

//----------------------------------------------------------------------------
//!	\brief	Process rsponse display data in response message
//!	\return	TRUE  - field processed
//!					FALSE - failed to process
//----------------------------------------------------------------------------
BOOL CHost::ProcessResponseDisplayData()
{
	GetFS((BYTE*)pTRANDATA->HostRespText, SZ_RESPONSE_TEXT); //TDGJP
	return TRUE;
}

//----------------------------------------------------------------------------
//!	\brief	Process sequence number in response message
//!	\return	TRUE  - field processed
//!					FALSE - failed to process
//----------------------------------------------------------------------------
BOOL CHost::ProcessFullSequenceNumber()
{
	memcpy(pTRANDATA->SequenceNum, m_Response, SZ_FULL_SEQUENCE_NO);
	ProcessShiftNumber();
	ProcessBatchNumber();
	ProcessSequenceNumber();
	SkipResponse(1);
	return TRUE;
}

//----------------------------------------------------------------------------
//!	\brief	Process shift number in response message
//!	\return	TRUE  - field processed
//!					FALSE - failed to process
//----------------------------------------------------------------------------
BOOL CHost::ProcessShiftNumber()
{
	char temp[SZ_SHIFT_NO+1];

	GetCStr(temp, SZ_SHIFT_NO); 
	DWORD size = sizeof(USHORT);
	USHORT itemp = atoi(temp);
	CDataFile::Save(FID_TERM_SHIFT_NUMBER, (BYTE *) &itemp, size);
	return TRUE;
}

//----------------------------------------------------------------------------
//!	\brief	Process batch number in response message
//!	\return	TRUE  - field processed
//!					FALSE - failed to process
//----------------------------------------------------------------------------
BOOL CHost::ProcessBatchNumber()
{
	char temp[SZ_SHIFT_NO+1];
	DWORD size = sizeof(USHORT);

	GetCStr(temp, SZ_BATCH_NO); 
	USHORT itemp = atoi(temp);
	CDataFile::Save(FID_TERM_BATCH_NUMBER, (BYTE *) &itemp, size);
	return TRUE;
}

//----------------------------------------------------------------------------
//!	\brief	Process sequence in response message
//!	\return	TRUE  - field processed
//!					FALSE - failed to process
//----------------------------------------------------------------------------
BOOL CHost::ProcessSequenceNumber()
{
	char temp[SZ_SHIFT_NO+1];
	DWORD size = sizeof(USHORT);

	GetCStr(temp, SZ_SEQ_NO); 
	USHORT itemp = atoi(temp);
	if ((strcmp(pTRANDATA->ISORespCode, "84") != 0))
		itemp++;

	m_iSeqNo = itemp;
	CDataFile::Save(FID_TERM_SEQ_NUMBER, (BYTE *)&itemp, size);
	USHORT flag = 0;
	CDataFile::Save(FID_TERM_SEQ_FLAG, (BYTE *)&flag, size);
	return TRUE;
}

//----------------------------------------------------------------------------
//!	\brief	Process FID 6 EMV in response message
//!	\return	TRUE  - field processed
//!					FALSE - failed to process
//----------------------------------------------------------------------------
BOOL CHost::ProcessFID6EMV()
{
	char *fieldEnd = m_Response + FieldLen(999);

	BOOL result = TRUE;
	while (result && (m_Response < fieldEnd))
	{
		SkipResponse(1); // RS
		char tag;
		Get(tag);

		switch (tag) 
		{
			case 'Q' :
				result = ProcessEMVResponseData();
				break;

			case 'R' :
				result = ProcessEMVAdditionalResponseData();
				break;

			// Ignored subfiedls
			case 'E' : // POS Entry Mode
			case 'I' : // Transaction Currency Code
				m_Response += SubfieldLenRS(999);
				break;

			default:
				TRACE(L"Unknown FID 6 subfield %c", tag);
				// SkipResponse this subfield
				m_Response += SubfieldLenRS(999);
		}
	}
	return result;
}

//----------------------------------------------------------------------------
//!	\brief	Process EMV response data in response message
//!	\return	TRUE  - field processed
//!					FALSE - failed to process
//----------------------------------------------------------------------------
BOOL CHost::ProcessEMVResponseData()
{
	int scheme;
	
	GetInt(scheme, 2);
	
	// EMV version 2
	if (scheme == 1)
	{
		// Authorization Response Code
		Get(pTRANDATA->EmvData.AuthorizationResponseCode, 
												sizeof(pTRANDATA->EmvData.AuthorizationResponseCode)); 
	}

	// Issuer Authentication Data
	int length = SubfieldLenRS(32) / 2;
	pTRANDATA->EmvData.IssuerAuthenticationDataLen = length;
	GetAscHex(pTRANDATA->EmvData.IssuerAuthenticationData, length);
	return TRUE;
}

//----------------------------------------------------------------------------
//!	\brief	Process EMV additional response data in response message
//!	\return	TRUE  - field processed
//!					FALSE - failed to process
//----------------------------------------------------------------------------
BOOL CHost::ProcessEMVAdditionalResponseData()
{
	SkipResponse(2); // card sheme

	int length = SubfieldLenRS(256) / 2;
	pTRANDATA->EmvData.IssuerScriptDataLen = length;
	GetAscHex(pTRANDATA->EmvData.IssuerScriptData, length);
	return TRUE;
}

//----------------------------------------------------------------------------
//!	\brief	Process download field in response message
//!	\return	TRUE  - field processed
//!					FALSE - failed to process
//----------------------------------------------------------------------------
BOOL CHost::ProcessDownload()
{
	char c;

	// Category Code first byte
	Get(c);
	if (c == '1')
	{
		m_MoreData = TRUE;
	}
	else
	{
		m_MoreData = FALSE;
	}

	// Category Code first byte
	SkipResponse(1);

	// Access Code
	SkipResponse(1);
	m_ContinuationRequest = TRUE;

	// Processing Flag
	Get(m_LastDID); // last processing downloaded field
	SkipResponse(1);

	// Filler
	Get(reinterpret_cast<BYTE *>(m_DownloadFiller), sizeof(m_DownloadFiller));

	return TRUE;
}

//----------------------------------------------------------------------------
//!	\brief	Process downloaded data in response message
//!	\return	TRUE  - field processed
//!					FALSE - failed to process
//----------------------------------------------------------------------------
BOOL CHost::ProcessDownloadedData()
{
	char *fieldEnd = m_Response + FieldLen(9999);

	// For non-EMV download data, did k contains might contain
	// multiple card types in one response
	// For EMV download data, a record might span more than one did k
	// we should base on m_MoreData flag to concatenate the data

	CRecordFile tempInitFile;
	AP_FILE_HANDLER fh;
	tempInitFile.Open(TEMP_INIT_FILE, &fh);
	int length = 0;

	SkipResponse(2); // skip 0<GS>
	
	while (m_Response < fieldEnd)
	{
		char temp[HOST_RESP_LENGTH];
		char id;
		Get(id);
		char *nextSubfield = m_Response + SubfieldLen(9999) + 1;

		if (id == 'k')
		{
			// check if this is regular or EMV DID k format
			if (!m_EMVDownload)	// fix length, full DID
			{
				{
					memset(temp, 0, sizeof(temp));
					temp[0] = id;
					GetGS((BYTE*)&temp[1], sizeof(temp));
					temp[strlen(temp)] = '0';	// no more data for this did
					tempInitFile.AddRecord(&fh, (BYTE*)temp, strlen(temp));
				}
			}
			else	// variable length, maybe partial field	
			{
				memset(temp, 0, sizeof(temp));
				temp[0] = id;
				GetGS((BYTE *)&temp[1], sizeof(temp));
				if (m_MoreData)
				{
					temp[strlen(temp)] = '1';	// more data for this did
				}
				else
				{
					temp[strlen(temp)] = '0';	// no more data for this did
				}
				tempInitFile.AddRecord(&fh, (BYTE*)temp, strlen(temp));
			}
		}
		else
		{
			memset(temp, 0, sizeof(temp));
			temp[0] = id;
			GetGS((BYTE *)&temp[1], sizeof(temp));
			temp[strlen(temp)] = '0';	// no more data for this did
			tempInitFile.AddRecord(&fh, (BYTE*)temp, strlen(temp));
		}

		// Bump pointer to next DID
		m_Response = nextSubfield;
	}
	m_Response = fieldEnd;

	tempInitFile.CloseFile(&fh);

	return TRUE;
}

//----------------------------------------------------------------------------
//!	\brief	Save downloaded data
//!	\return	TRUE  - field processed
//!					FALSE - failed to process
//----------------------------------------------------------------------------
BOOL CHost::SaveDownloadedData()
{
	BOOL status = TRUE;
	char rec[HOST_RESP_LENGTH];
	USHORT numOfRecs=0;

	CRecordFile tempInitFile;
	AP_FILE_HANDLER fh;
	
	tempInitFile.Open(TEMP_INIT_FILE, &fh);
	
	tempInitFile.GetNumberOfRecords(&fh, &numOfRecs);
	
	if (numOfRecs > 0)
	{
		char *temp = new char[numOfRecs*HOST_RESP_LENGTH];
		if (temp ==NULL )
			return FALSE;
		
		memset(temp, 0, numOfRecs*HOST_RESP_LENGTH);
		
		BOOL moreData=FALSE;

		int iOfRecs = numOfRecs;
		for (int n = 0; n < iOfRecs; n++)
		{
			memset(rec, 0, HOST_RESP_LENGTH);
			USHORT len;
			tempInitFile.ReadRecord(&fh, n, (BYTE *)rec, &len);

			char id = rec[0];
		
			memset(temp, 0, numOfRecs*HOST_RESP_LENGTH);
			strncpy(temp, &rec[2], len-3);
			
			switch (id)
			{
				// Request 
				case 'A':
					if (strlen(temp) > 40)
					{
						status = FALSE;
						break;
					}	
					
					if (!STR_IsAlphanumeric(temp, strlen(temp)))
					{
						status = FALSE;
						break;
					}

					CDataFile::Save(FID_TERM_MAC_REQ_FIDS, (BYTE *)temp, strlen(temp)+1);
					break;

				// Response 
				case 'B':
					if (strlen(temp) > 40)
					{
						status = FALSE;
						break;
					}	

					if (!STR_IsAlphanumeric(temp, strlen(temp)))
					{
						status = FALSE;
						break;
					}

					CDataFile::Save(FID_TERM_MAC_RSP_FIDS, (BYTE *)temp, strlen(temp)+1);
					break;

				// Term-Name-LOC
				case 'a':
					if (strlen(temp) > 25)
					{
						status = FALSE;
						break;
					}	
					CDataFile::Save(FID_MERCH_CFG_ADDRESS, (BYTE *)temp, strlen(temp)+1);
					break;

				// Term-City-St
				case 'b':
					if (strlen(temp) > 16)
					{
						status = FALSE;
						break;
					}	
					CDataFile::Save(FID_MERCH_CFG_CITY_PROV, (BYTE *)temp, strlen(temp)+1);
					break;

				// Term-Owner-Name
				case 'c':
					CDataFile::Save(FID_MERCH_CFG_NAME, (BYTE *)temp, strlen(temp)+1);
					break;

				// Service
				case 'k':
					if (m_EMVDownload)
					{
						SaveEMVDownloadedData(temp);
					}
					else
					{
						SaveDownloadedData(temp);
					}
					break;

				// Allowed transactions
				case 'p':
					CDataFile::Save(FID_TERM_ALLOWED_TRANS, (BYTE *)temp, strlen(temp)+1);
					break;

				default:	
					TRACE(L"Unknown subfield %c", id);
					break;
			}
		}
		delete [] temp;
	}
	
	tempInitFile.CloseFile(&fh);
	if (!pCONFIG->EMVEnabled)
		tempInitFile.Delete(TEMP_INIT_FILE);
	return status;
}

//----------------------------------------------------------------------------
//!	\brief	Process and Save downloaded data subfield service in response message
//!	\return	TRUE  - field processed
//!					FALSE - failed to process
//----------------------------------------------------------------------------
BOOL CHost::SaveDownloadedData(char *data)
{
	sServiceData ServiceData;
	int len = strlen(data);
	char *pData = &data[0];
	
	CRecordFile servDataFile;
	AP_FILE_HANDLER fh;
	
	if (!servDataFile.Open(SERVICE_DATA_FILE, &fh))
		return FALSE;

	// for non-EMV download, did k contains many records, one for each card type
	while (len >= DID_k_LENGTH)
	{
		memset(ServiceData.serviceType, 0, sizeof(ServiceData.serviceType));
		memset(ServiceData.surchargeLimit, 0, sizeof(ServiceData.surchargeLimit));
		
		memcpy(ServiceData.serviceType, &pData[0], SZ_BIN_CARD_SERVICE_TYPE);
		memcpy(ServiceData.surchargeLimit, &pData[20], SZ_BIN_SURCHARGE_LIMIT);
		ServiceData.tranProfile = pData[38];

		if ( strcmp( ServiceData.serviceType,"P0") == 0 )
		{
			USHORT itemp = atoi(ServiceData.surchargeLimit);
			CDataFile::Save(FID_CFG_SURCHARGE_LIMIT, (BYTE *) &itemp, sizeof(USHORT));
		}

		servDataFile.AddRecord(&fh, (BYTE*)&ServiceData, sizeof(sServiceData));
		
		pData += DID_k_LENGTH;
		len -= DID_k_LENGTH;
	}
	servDataFile.CloseFile(&fh);

	return TRUE;
}

//----------------------------------------------------------------------------
//!	\brief	Process and Save EMV downloaded data subfield service in response message
//!	\return	TRUE  - field processed
//!					FALSE - failed to process
//----------------------------------------------------------------------------
BOOL CHost::SaveEMVDownloadedData(char *data)
{
	BOOL result = TRUE;
	char *pData = data;
	char *pEnd = data + strlen(data);

	while (result && (pData < pEnd))
	{
		int length;	// length processed
		char format = *pData;
		switch (format) 
		{
			case '1' :
				SaveEMVDownloadedData_F1(pData+2, &length);
				break;

			case '2' :
				SaveEMVDownloadedData_F2(pData+2, &length);
				break;

			default:
				TRACE(L"Unknown DID k subfield format %c", format);
				// Do not continue to parse this DID
				result = FALSE;
				break;//TDGPJ 2012NOV28
		}
		pData += (length+2);
	}

	return result;
}

//----------------------------------------------------------------------------
//!	\brief	Process and Save EMV service format 1 in response message
//!	\return	TRUE - field processed
//!			FALSE - failed to process
//----------------------------------------------------------------------------
BOOL CHost::SaveEMVDownloadedData_F1(char *data, int *length)
{
	sEmvData EmvData;
	sCtlsParams ctlsParams;
	BYTE temp[50];
	char temp2[100];
	int recLen = strlen(data);
	int currLen = 0;
	long limit;

	memset(&EmvData, 0, sizeof(sEmvData));

	// Service Type
	memcpy(EmvData.serviceType, &data[currLen], SZ_BIN_CARD_SERVICE_TYPE);
	currLen += SZ_BIN_CARD_SERVICE_TYPE;

	// ascii AID
	memset(temp, 0, sizeof(temp));
	memcpy(temp, &data[currLen], SZ_BIN_AID);
	STR_TrimSpace((char *) temp);	// remove trailing spaces
	strcpy((char *)EmvData.asciiAID, (char *)temp);
	
	// AID - packed
	memset(temp, 0, sizeof(temp));
	memcpy(temp, &data[currLen], SZ_BIN_AID);
	STR_TrimSpace((char *) temp);	// remove trailing spaces
	STR_PackData(EmvData.AID, (char *) temp, strlen((char *)temp));
	EmvData.AIDLength = strlen((char *)temp)/2;
	currLen += SZ_BIN_AID;

	// EMV Flag
	EmvData.emvFlag	= data[currLen++];

	// Fallback Enabled
	EmvData.fallbackAllowed = data[currLen++]=='Y'?TRUE:FALSE;

	// EMV Offline Floor Limit
	memcpy(temp2, &data[currLen], 9);
	limit = atol(temp2);
	STR_LongToBCDString(limit, SZ_BIN_EMV_OFFLINE_FLOOR_LIMIT, (BYTE*)EmvData.emvOfflineFloorLimit);	
	currLen += 9;

	// CTLS Offline Floor Limit
	memset(temp2, 0, sizeof(temp2));
	memcpy(temp2, &data[currLen], 9);
	limit = atol(temp2);
	STR_LongToBCDString(limit, SZ_BIN_CTLS_EMV_OFFLINE_FLOOR_LIMIT, (BYTE*)EmvData.ctlsOfflineFloorLimit);	
	currLen += 9;

	// TAC Default - packed
	STR_PackData(EmvData.TACDefault, &data[currLen], SZ_EMV_TAC*2);
	currLen += SZ_EMV_TAC*2;

	// TAC Denial - packed
	STR_PackData(EmvData.TACDenial, &data[currLen], SZ_EMV_TAC*2);
	currLen += SZ_EMV_TAC*2;

	// TAC Online - packed
	STR_PackData(EmvData.TACOnline, &data[currLen], SZ_EMV_TAC*2);
	currLen += SZ_EMV_TAC*2;

	// CTLS TAC Default - packed
	STR_PackData(EmvData.ctlsTACDefault, &data[currLen], SZ_EMV_TAC*2);
	currLen += SZ_EMV_TAC*2;

	// CTLS TAC Denial - packed
	STR_PackData(EmvData.ctlsTACDenial, &data[currLen], SZ_EMV_TAC*2);
	currLen += SZ_EMV_TAC*2;

	// CTLS TAC Online - packed
	STR_PackData(EmvData.ctlsTACOnline, &data[currLen], SZ_EMV_TAC*2);
	currLen += SZ_EMV_TAC*2;

	// MTI (Merchant Type Indicator)
	STR_PackData(&EmvData.MTI, &data[currLen], 2);
	currLen += 2;

	// Application
	memcpy(EmvData.appVersion, &data[currLen], SZ_EMV_APPLICATION_NUMBER);
	currLen += SZ_EMV_APPLICATION_NUMBER;

	// Threshold Value
	STR_NumericToBCD(&data[currLen], temp, 9);
	memcpy(EmvData.thresholdValue, &temp[1], SZ_EMV_THRESHOLD_VALUE);
	currLen += 9;

	// Target Percentage
	STR_NumericToBCD(&data[currLen], &EmvData.targetPercentage, 2);
	currLen += 2;

	// Maximum Target Percentage
	STR_NumericToBCD(&data[currLen], &EmvData.maximumTargetPercentage, 2);
	currLen += 2;

	// Default DDOL Length
	EmvData.defaultDDOLLength = STR_atoi(&data[currLen], 3)/2;
	if (EmvData.defaultDDOLLength > SZ_EMV_MAX_DOL)
		return FALSE;
	currLen += 3;

	// Default DDOL, packed
	if (EmvData.defaultDDOLLength > 0)
	{
		STR_PackData(EmvData.defaultDDOL, &data[currLen], EmvData.defaultDDOLLength*2);
		currLen += EmvData.defaultDDOLLength*2;
	}

	// Default TDOL Length
	EmvData.defaultTDOLLength = STR_atoi(&data[currLen], 3)/2;
	if (EmvData.defaultTDOLLength > SZ_EMV_MAX_DOL)
		return FALSE;
	currLen += 3;

	// Default TDOL, packed
	if (EmvData.defaultTDOLLength > 0)
	{
		STR_PackData(EmvData.defaultTDOL, &data[currLen], EmvData.defaultTDOLLength*2);
		currLen += EmvData.defaultTDOLLength*2;
	}

	EmvData.applicationSelectionIndicator = 1;	
															
	CRecordFile emvDataFile;
	AP_FILE_HANDLER fh;
	
	if (!emvDataFile.Open(EMV_DATA_FILE, &fh))
		return FALSE;

	emvDataFile.AddRecord(&fh, (BYTE*)&EmvData, sizeof(sEmvData));
	emvDataFile.CloseFile(&fh);

	if( EmvData.emvFlag == 'C')
	{
		*length = currLen;
		return TRUE;
	}

	if (currLen < recLen)	// CTLS data is sent by the host
	{
		// **********  CTLS Data ***********
		memset(&ctlsParams, 0, sizeof(sCtlsParams));
		ctlsParams.hasData = TRUE;

		TRACE(L"CTLS ServType: %s \n", CString(EmvData.serviceType));

		// Service Type
		memcpy(ctlsParams.serviceType, EmvData.serviceType, SZ_BIN_CARD_SERVICE_TYPE);

		// CTLS TDOL Length
		ctlsParams.defaultTDOLLength = STR_atoi(&data[currLen], 3)/2;
		if (ctlsParams.defaultTDOLLength > SZ_EMV_MAX_DOL)
			return FALSE;
		currLen += 3;

		// CTLS TDOL, packed
		if (ctlsParams.defaultTDOLLength > 0)
		{
			STR_PackData(ctlsParams.defaultTDOL, &data[currLen], 
																							ctlsParams.defaultTDOLLength*2);
			currLen += ctlsParams.defaultTDOLLength*2;
		}

		// CTLS DDOL Length
		ctlsParams.defaultDDOLLength = STR_atoi(&data[currLen], 3)/2;
		if (ctlsParams.defaultDDOLLength > SZ_EMV_MAX_DOL)
			return FALSE;
		currLen += 3;

		// CTLS DDOL, packed
		if (ctlsParams.defaultDDOLLength > 0)
		{
			STR_PackData(ctlsParams.defaultDDOL, &data[currLen], 
																							ctlsParams.defaultDDOLLength*2);
			currLen += ctlsParams.defaultDDOLLength*2;
		}

		// EMV CTLS App Version
		memcpy(ctlsParams.emvAppVersion, &data[currLen], SZ_EMV_APPLICATION_NUMBER);
		currLen += SZ_EMV_APPLICATION_NUMBER;

		// MSD CTLS App Version
		memcpy(ctlsParams.msdAppVersion, &data[currLen], SZ_EMV_APPLICATION_NUMBER);
		currLen += SZ_EMV_APPLICATION_NUMBER;

		// CTLS CVM Limit
		char temp2[SZ_BIN_CTLS_EMV_CVM_LIMIT*2+1];
		memset(temp2, 0, sizeof(temp2));
		memcpy(temp2, &data[currLen], 9);
		limit = atol(temp2);
		STR_LongToBCDString(limit, SZ_BIN_CTLS_EMV_CVM_LIMIT, (BYTE*)ctlsParams.ctlsCvmLimit);	
		currLen += 9;

		// CTLS Transaction Limit
		memcpy(&temp2[3], &data[currLen], 9);
		limit = atol(temp2);
		STR_LongToBCDString(limit, SZ_BIN_CTLS_EMV_CVM_LIMIT, (BYTE*)ctlsParams.ctlsTxnLimit);	
		currLen += 9;

		// CTLS Interac TOS (Contactless Terminal Option Status)
		STR_PackData(ctlsParams.interactTOS, &data[currLen], SZ_EMV_INTERAC_TOS*2);
		currLen += SZ_EMV_INTERAC_TOS*2;

		// CTLS Interac RRL (Contactless Receipt Required Limit)
		memcpy(ctlsParams.interactRRL, &data[currLen], SZ_EMV_INTERAC_RRL);
		currLen += SZ_EMV_INTERAC_RRL;

		// CTLS Visa TTQ 
		STR_PackData(ctlsParams.visaTTQ, &data[currLen], SZ_EMV_VISA_TTQ*2);
		currLen += SZ_EMV_VISA_TTQ*2;

		// TTI: hard-coded.	(Terminal Transaction Info)
		if (strcmp(EmvData.serviceType,"P0") == 0)
			memcpy(ctlsParams.interactTTI, "\xC0\x80\x00", SZ_EMV_INTERAC_TTI);
		else
			memcpy(ctlsParams.interactTTI, "\xD8\x80\x00", SZ_EMV_INTERAC_TTI);

		CRecordFile ctlsDataFile;
		AP_FILE_HANDLER ctlsfh;
		if (!ctlsDataFile.Open(CTLS_DATA_FILE, &ctlsfh))
			return FALSE;

		ctlsDataFile.AddRecord(&ctlsfh, (BYTE*)&ctlsParams, sizeof(sCtlsParams));
		ctlsDataFile.CloseFile(&fh);
	}
	else	// save an empty record for easy searching later, # of CTLS recs = # of EMV recs
	{
		memset(&ctlsParams, 0, sizeof(sCtlsParams));
		ctlsParams.hasData = FALSE;
		CRecordFile ctlsDataFile;
		AP_FILE_HANDLER ctlsfh;
		if (!ctlsDataFile.Open(CTLS_DATA_FILE, &ctlsfh))
			return FALSE;

		ctlsDataFile.AddRecord(&ctlsfh, (BYTE*)&ctlsParams, sizeof(sCtlsParams));
		ctlsDataFile.CloseFile(&fh);
	}

	*length = currLen;

	return TRUE;
}

//----------------------------------------------------------------------------
//!	\brief	Process and Save EMV service format 2 in response message
//!	\return	TRUE - field processed
//!					FALSE - failed to process
//----------------------------------------------------------------------------
BOOL CHost::SaveEMVDownloadedData_F2(char *data, int *length)
{
	sEmvKeyData emvKeyData;
	USHORT offset, len;
	USHORT recLen = strlen(data);

	// **********  EMV Key Data ***********
	memset(&emvKeyData, 0, sizeof(sEmvKeyData));

	// RID
	offset = 0;
	if (!STR_IsHex(&data[offset], SZ_RID))
	{
		return FALSE;
	}
	else
	{
		STR_PackData(emvKeyData.RID, &data[offset], SZ_RID);
	}	
	offset += SZ_RID;
	
	// INDEX
	if (!STR_IsHex(&data[offset], 2))
	{
		return FALSE;
	}
	else
	{
		STR_PackData(&emvKeyData.index, &data[offset], 2);
	}	
	offset += 2;
	
	// MODULUS
	if (!STR_IsNumeric(&data[offset], 4))	// modulus length should be numeric
	{
		return FALSE;
	}
	
	len = STR_atoi(&data[offset], 4);
	if (len == 0)
	{
		return FALSE;
	}
	emvKeyData.modulusLength = len/2;
	offset += 4;

	if (!STR_IsHex(&data[offset], len))
	{
		return FALSE;
	}
	else
	{
		STR_PackData(emvKeyData.modulus, &data[offset], len);
	}	
	offset += len;
	
	// CHECKSUM
	if (!STR_IsNumeric(&data[offset], 2))	// checksum length should be numeric	
	{
		return FALSE;
	}

	len = STR_atoi(&data[offset], 2);
	if (len == 0)
	{
		return FALSE;
	}
	emvKeyData.checksumLength = len/2;
	offset += 2;

	if (!STR_IsHex(&data[offset], len))
	{
		return FALSE;
	}
	else
	{
		STR_PackData(emvKeyData.checksum, &data[offset], len);
	}	
	offset += 40;
	
	// EXPONENT
	if (!STR_IsNumeric(&data[offset], 2))	// exponent length should be numeric	
	{
		return FALSE;
	}

	len = STR_atoi(&data[offset], 2);
	if (len == 0)
	{
		return FALSE;
	}
	offset += 2;
	
	if (!STR_IsHex(&data[offset], len))
	{
		return FALSE;
	}
	else
	{
		emvKeyData.exponentLength = STR_PackData(emvKeyData.exponent, &data[offset], len);
	}	
	offset += 6;
	
	// patch for new Kernel
	if (emvKeyData.exponent[0] == 0x03 && emvKeyData.exponentLength == 1)
	{
		emvKeyData.exponent[0] = 0x00;
		emvKeyData.exponent[1] = 0x00;
		emvKeyData.exponent[2] = 0x03;
		emvKeyData.exponentLength = 3;
	}

	// HASH ALGORITHM INDICATOR
	if (memcmp(&data[offset], "01", 2) != 0)
	{
		return FALSE;
	}
	else
	{
		STR_PackData(&emvKeyData.hashAlgorithmIndicator, &data[offset], 2);
	}
	offset += 2;
	
	// SIGNATURE ALGORITHM INDICATOR
	if (memcmp(&data[offset], "01", 2) != 0)
	{
		return FALSE;
	}
	else
	{
		STR_PackData(&emvKeyData.signatureAlgorithmIndicator, &data[offset], 2);
	}
	offset += 2;

	{
		// Keys are saved per RID file
		// so one file might contain more than 1 key (different index)
		char fileName[50];	// eg. KFA000000277
		
		memset(fileName, 0, 50); 
		strcpy(fileName, EMV_KEY_DATA_DIR);
		strcat(fileName, EMV_KEY_FILE_PREFIX);
		strncat(fileName, data, 10);	// RID
		CRecordFile keyFile;
		AP_FILE_HANDLER fh;
		if (!keyFile.Open(fileName, &fh))	
		{
			// create it
			keyFile.Create(fileName, sizeof(sEmvKeyData), 30);
			keyFile.Open(fileName, &fh);
		}
		keyFile.AddRecord(&fh, (BYTE*)&emvKeyData, sizeof(sEmvKeyData));
		keyFile.CloseFile(&fh);
	}

	if (offset != recLen)
		return FALSE;
	
	*length = offset;
	
	return TRUE;
}

//----------------------------------------------------------------------------
//!	\brief	Process field by ID in response message
//!	\return	TRUE	- field processed
//!				  FALSE - failed to process
//----------------------------------------------------------------------------
BOOL CHost::ProcessField(char id)
{
	BOOL result = FALSE;
	int fieldLength = FieldLen(9999);
	char *nextField = m_Response + fieldLength + 1;

	if (pTRANDATA->TranCode != TRAN_HOST_INIT &&
		  pTRANDATA->TranCode != TRAN_BATCH_TOTAL &&
		  pTRANDATA->TranCode != TRAN_CLOSE_BATCH)
	{ 
		char respMacFids[20];
		DWORD size;
		CDataFile::Read(FID_TERM_MAC_RSP_FIDS, (BYTE*) respMacFids, &size);

		if (memchr(respMacFids, id, strlen(respMacFids)))
		{
			strncat(m_MACedData, m_Response, fieldLength);
		}
	}
	
	switch (id)
	{
		case 'B':	// Amount
			result = ProcessAmount();
			break;

		case 'F':
			result = ProcessAuthorizationCode();
			break;

		case 'G':
			result = ProcessMACValue();
			break;

		case 'H':
			result = ProcessNewMACKey();
			break;

		case 'J':
			result = ProcessAvailableBalance();
			break;

		case 'M':
			result = ProcessPINEncryptionKey();
			break;

		case 'V':
			result = ProcessDownload();
			break;

		case 'W':
			result = ProcessDownloadedData();
			break;

		case 'X':
			result = ProcessISOResponseCode();
			break;

		case 'g':
			result = ProcessResponseDisplayData();
			break;

		case 'l':
			result = ProcessBatchData();
			break;
/*
		case 'm':
			result = ProcessEndOfDayData();
			break;
*/
		case 'z':
			result = ProcessReinitilizeOrMFCClose();
			break;

		case 'K':	// Terminal Posting Date
			result = ProcessTerminalPostingDate();
			break;

		case 'h':	// Sequence Number
			result = ProcessFullSequenceNumber();//ProcessSequenceNumber();
			break;

		case '6':
			result = ProcessFID6EMV();
			break;

		// These fields is ignored by terminal
		case 'I':	// Data Encryption Key
			result = TRUE;
			break;

		default:
			TRACE(L"ProcessField() - Unexpected FID: %c\n", id);
			result = TRUE;
			break;
	}
	// Bump pointer to next field
	m_Response = nextField;
	return result;
}

//----------------------------------------------------------------------------
//!	\brief	Process terminal date and time in response message
//!	\return	TRUE - field processed
//!			FALSE - failed to process
//----------------------------------------------------------------------------
BOOL CHost::ProcessTerminalDateTime()
{
	char TranDateTime[SZ_TRAN_DATE_TIME+1];
	GetCStr(TranDateTime, SZ_TRAN_DATE_TIME);
	memcpy(pTRANDATA->TranDate, TranDateTime, SZ_TRAN_DATE);
	memcpy(pTRANDATA->TranTime, &TranDateTime[6], SZ_TRAN_TIME);
	CTimeUtil time;
	time.SetDateAndTime(TranDateTime);
	return TRUE;
}

//----------------------------------------------------------------------------
//!	\brief	Process response code in response message
//!	\return	TRUE - field processed
//!			FALSE - failed to process 
//----------------------------------------------------------------------------
BOOL CHost::ProcessResponseCode()
{
	GetCStr(pTRANDATA->RespCode, SZ_RESPONSE_CODE);

	switch (pTRANDATA->TranCode)
	{
		case TRAN_HOST_INIT:
			// There is more data to be processed
			if (strcmp(pTRANDATA->RespCode, "881") == 0)
			{
				m_ContinueDownload = TRUE;
			}
			else if (strcmp(pTRANDATA->RespCode, "880") == 0)
			{
				SetTranStatus(ST_APPROVED);
			}
			break;

		// Authorization response processing
		default:
			// Sequence number is incorrect, terminal should resend current transaction
			if (strcmp(pTRANDATA->RespCode, "899") == 0)
			{
				// Resend this transaction
				m_Resend = TRUE;
			}
	}
	return TRUE;
}

//-----------------------------------------------------------------------------
//!	\brief	check if transaction needs to be Mac verified
//!	\return	TRUE - need Mac check
// 					FALSE - no
//-----------------------------------------------------------------------------
BOOL CHost::MACedTransactions()
{
	if (pTRANDATA->TranCode == TRAN_PURCHASE ||
			pTRANDATA->TranCode == TRAN_PREAUTH ||
			pTRANDATA->TranCode == TRAN_ADVICE ||
			pTRANDATA->TranCode == TRAN_FORCE_POST ||
			pTRANDATA->TranCode == TRAN_REFUND ||
			pTRANDATA->TranCode == TRAN_PURCHASE_VOID ||
			pTRANDATA->TranCode == TRAN_REFUND_VOID ||
			pTRANDATA->TranCode == TRAN_KEY_EXCHANGE)
	{
		return TRUE;
	}
		
	return FALSE;
}

//-----------------------------------------------------------------------------
//!	\brief	Analyze standard POS header in response
//!	\return	TRUE - succesfull analysis
// 					FALSE - failed to analyze
//-----------------------------------------------------------------------------
BOOL CHost::AnalyzeHeader()
{
	BOOL result = TRUE;
	SkipResponse(2);	// Device Type
	strncat(m_MACedData, m_Response, 2);
	strncat(pTRANDATA->TransmissionNo,m_Response, 2);
	SkipResponse(2);	// Transmission Number
	strncat(m_MACedData, m_Response, 16);
	SkipResponse(16);	// Terminal ID
	SkipResponse(6);	// Reserved
	ProcessTerminalDateTime();
	SkipResponse(1);	// Message Type
	SkipResponse(1);	// Message Subtype
	strncat(m_MACedData, m_Response, 2);
	SkipResponse(2);	// Transaction Code
	SkipResponse(3);	// Processing Flag 1, 2, 3
	strncat(m_MACedData, m_Response, 3);
	ProcessResponseCode();
	SkipResponse(1);	// FS
	return result;
}

//----------------------------------------------------------------------------
//!	\brief	Analyze the response from the host.
//! \param	data buffer managed blob with the response data from the host.
//!	\return	TRUE if analysis went okay, FALSE if something went wrong.
//----------------------------------------------------------------------------
BOOL CHost::AnalyzeResponse(char *data)
{
	CString traceData = CString(data);
	traceData.Replace(L"\x1C", L"?");
	TRACE(L"m_Response(len = %d): %s\n",traceData.GetLength(), traceData);

	BOOL result = FALSE;
	
	// Remove ETX if it exists
	if (data[strlen(data)- 1] == ETX)
	{
		data[strlen(data)- 1] = 0;
	}
	
	ClearRequest();
	strcpy(m_Request, data);	// for chip reversal
	m_RequestLength = strlen(data);
	
	m_Response = data;
	int responseLength = strlen(data);
	char *responseEnd = m_Response + responseLength;

	// Clear MAC data
	memset(m_MACedData, 0, sizeof(m_MACedData));
	memset(m_ReceivedMAC, 0, sizeof(m_ReceivedMAC));

	// Error by default
	SetCommStatus(ST_HOST_ERROR);

	if (responseLength < SZ_HEADER)
	{
		TRACE(L"Not enough data in response");
		SetReversal(TIMEOUT_REVERSAL);
		return result;
	}
	else
	{
		result = AnalyzeHeader();
	}

	if (result)
	{
		BYTE id;
		while (m_Response < responseEnd)
		{
			Get(id);
			result = ProcessField(id);
		}
	}

	// Copy the received data into pTRANDATA
	CopyFields();

	// No reason to analyze MAC from response if there is key error returned by host
	// Because terminal has incorrect keys and MAC check always fails
	if (result && !m_KeyError && MACedTransactions())
	{
		result = AnalyzeMACValue();
	}
  
  return result;
}

//-----------------------------------------------------------------------------
//!	\brief	Analyze MAC from response
//-----------------------------------------------------------------------------
BOOL CHost::AnalyzeMACValue()
{
	BOOL reversed = FALSE;
	
	// If MAC was received from response
	if (strlen(m_ReceivedMAC) == SZ_MAC)
	{
		CString traceData = CString(m_MACedData);
		TRACE(L"MAC DATA: %s", traceData);
		if (!CSEC::VerifyMAC((BYTE *)m_MACedData, strlen(m_MACedData), m_ReceivedMAC))
		{
			reversed = TRUE;
		}
	}
	else
	{
		reversed = TRUE;
	}
	
	// check if a financial transaction
	if (IsFinancial() && reversed)
		SetReversal(MAC_REVERSAL);

	if ( reversed )
		SetCommStatus(ST_MAC_ERR);


	return (reversed?FALSE:TRUE);	
}

//-----------------------------------------------------------------------------
//!	\brief	Set Reversal type
//-----------------------------------------------------------------------------
void CHost::SetReversal(BYTE type)
{
	if(pTRANDATA == NULL)
		return;

	if( pTRANDATA->TranCode == TRAN_HOST_INIT)
		return;

	pTRANDATA->bReversal = TRUE;
	pTRANDATA->ReversalType = type;

	if ( type != TIMEOUT_REVERSAL && m_bNewKey)
		CDataFile::Save(FID_TERM_MAC_KEY, m_BeiKey, SZ_KEY/2);
}

//-----------------------------------------------------------------------------
//!	\brief	Set Reversal type
//-----------------------------------------------------------------------------
void CHost::ClearReversal()
{
	if(pTRANDATA == NULL)
		return;
	pTRANDATA->bReversal = FALSE;
	pTRANDATA->ReversalType = NO_REVERSAL;
}

//----------------------------------------------------------------------------
//!	\brief Init Transaction Data
//----------------------------------------------------------------------------
void CHost::InitTransaction()
{
	CRecordFile tempInitFile, servDataFile, EmvDataFile, CtlsDataFile;

	if (pTRANDATA->TranCode == TRAN_HOST_INIT)
	{
		// create
		if (!tempInitFile.Create(TEMP_INIT_FILE, HOST_RESP_LENGTH, 100))
		{
//			assert(FALSE);
		}
		
		if (!servDataFile.Create(SERVICE_DATA_FILE, sizeof(sServiceData), BIN_MAX_NUMBER_RECORDS))
		{
//			assert(FALSE);
		}
		
		if (!EmvDataFile.Create(EMV_DATA_FILE, sizeof(sEmvData), BIN_MAX_NUMBER_RECORDS))
		{
//			assert(FALSE);
		}
		
		if (!CtlsDataFile.Create(CTLS_DATA_FILE, sizeof(sCtlsParams), BIN_MAX_NUMBER_RECORDS))
		{
//			assert(FALSE);
		}
	}
	
	ClearReversal();
	ClearRequest();
	m_Response = NULL;
	m_Resend = FALSE;
	memset(m_MACedData, 0, sizeof(m_MACedData));
}

//----------------------------------------------------------------------------
//!	\brief Process Reversals
//----------------------------------------------------------------------------
void CHost::ProcessReversals()
{
	if(pTRANDATA == NULL)
		return;
	DWORD size;
	char temp[SZ_HOST_BUFF_MAX];

	// Check reveral file, it exists, send it first
	BOOL res = CDataFile::Read(FID_TERM_REVERSAL_RECORD, (BYTE*)temp, &size);
	if (res && (size > 0))
	{
		pTRANDATA->bReversal = TRUE;
		memcpy(m_Request, temp, size);
		DoSubTransaction();
		HostDisconnect();
		ClearReversal();
		ClearRequest();
	}
}

//----------------------------------------------------------------------------
//!	\brief	set status for host communication
//! \param	status
//----------------------------------------------------------------------------
void CHost::SetCommStatus(int status)
{
	pTRANDATA->ComStatus = status;
}

//----------------------------------------------------------------------------
//!	\brief	get status of host communication
//! \param	status
//----------------------------------------------------------------------------
int CHost::GetCommStatus()
{
	return pTRANDATA->ComStatus;
}

//----------------------------------------------------------------------------
//!	\brief	set transaction status: Approved, Declined...
//! \param	status
//----------------------------------------------------------------------------
void CHost::SetTranStatus(int status)
{
	pTRANDATA->TranStatus = status;
}

//----------------------------------------------------------------------------
//!	\brief	get transaction status: Approved, Declined...
//! \param	status
//----------------------------------------------------------------------------
int CHost::GetTranStatus()
{
	return pTRANDATA->TranStatus;
}

//----------------------------------------------------------------------------
//!	\brief	Format reversal request
//----------------------------------------------------------------------------
BOOL CHost::FormatReversal()
{
	if(pTRANDATA == NULL)
		return FALSE;
	BOOL status;

	switch (pTRANDATA->ReversalType)
	{
	case CONTROLLER_REVERSAL:	// just the header
		ClearRequest();
		status = FormatHeader();
		SetTranStatus(CONTROLLER_REVERSAL);
		break;
	case MAC_REVERSAL:
		m_Request[39] = 'R';
		m_RequestLength = 40;
		PutTransactionCode();
		Put('0');
		if (pCONFIG->EMVEnabled)
			Put('5');
		else
			Put('0');
		Put("0000");
		m_Request[48] = 0;
		SetTranStatus(ST_MAC_ERR);
		break;
	case TIMEOUT_REVERSAL:
		for(int i= 0;i< 12;i++)
			m_Request[26+i] = '0';
		m_Request[39] = 'T';
		m_Request[48] = 0;
		SetTranStatus(ST_HOST_TIMEOUT);
		break;
	case CHIP_REVERSAL:
		status = FormatEMVReversal();
		SetTranStatus(CHIP_REVERSAL);
		break;
	default:
		return FALSE;
	}

	// save it to Reversal File
	CDataFile::Save(FID_TERM_REVERSAL_RECORD, 
														(BYTE*)m_Request, (DWORD)strlen(m_Request));

	memset((BYTE*)pTRANDATA->HostRespText, 0,SZ_RESPONSE_TEXT);

	return status;
}

//----------------------------------------------------------------------------
//!	\brief	Perform transaction post processing
//					Handles transaction continuation
//----------------------------------------------------------------------------
BOOL CHost::TransactionPostProcess()
{
	if (pTRANDATA->ReversalType != TIMEOUT_REVERSAL)
		DoReversals();
	ResendTransaction();
	HostInitializationCont();		// need host init
	return TRUE;
}

//-----------------------------------------------------------------------------
//!	\brief	Send Reversal
//-----------------------------------------------------------------------------
void CHost::DoReversals()
{
	if(pTRANDATA == NULL)
		return;
	// Need to resend transaction with updated data
	if (!pTRANDATA->bReversal)
	{
		return;
	}

	DWORD size;
	char temp[SZ_HOST_BUFF_MAX];

	// Check reveral file, it exists, send it first
	BOOL res = CDataFile::Read(FID_TERM_REVERSAL_RECORD, (BYTE*)temp, &size);
	if (res && (size > 0))
	{
		pTRANDATA->bReversal = TRUE;
		memcpy(m_Request, temp, size);
		DoSubTransaction();
		HostDisconnect();

		CDataFile::Delete(FID_TERM_REVERSAL_RECORD);
//		ClearReversal();
		ClearRequest();
	}
}

//-----------------------------------------------------------------------------
//!	\brief	Resend current transaction
//-----------------------------------------------------------------------------
void CHost::ResendTransaction()
{
	// Need to resend transaction with updated data
	while (m_Resend)
	{
		m_Resend = FALSE;
		DoReversibleSubTransaction();
	}
}

//-----------------------------------------------------------------------------
//!	\brief	Perform Host Init continuation
//-----------------------------------------------------------------------------
void CHost::HostInitializationCont()
{
	if (pTRANDATA->TranCode != TRAN_HOST_INIT)
	{
		return;
	}

	// Standard initialization
	if (!m_EMVDownload)
	{
		while (m_ContinueDownload)
		{
			m_ContinueDownload = FALSE;
			pTRANDATA->TranCode = TRAN_HOST_INIT;
			DoSubTransaction();
		}
	}
	else
	{	
		// EMV intitialization
		while (m_ContinueDownload)
		{
			m_ContinueDownload = FALSE;
			pTRANDATA->TranCode = TRAN_HOST_INIT;
			DoSubTransaction();
		}
	}
}

//-----------------------------------------------------------------------------
//!	\brief	Populate transaction date and time
//-----------------------------------------------------------------------------
void CHost::SetTransactionDateTime()
{
	CTimeUtil systemTime;
	systemTime.GetSystemDate(pTRANDATA->TranDate);
	systemTime.GetSystemTime(pTRANDATA->TranTime);
}

//-----------------------------------------------------------------------------
//!	\brief	Copy fields
//-----------------------------------------------------------------------------
void CHost::CopyFields()
{

}

//----------------------------------------------------------------------------
//!	\brief Actually build, send and complete a Transaction
//!	\return TRUE if successful, FALSE if not.
//	
//	Sequence through the steps needed to build a request,
//	send it, and get the results.
//	The information in pTRANDATA must have the information needed to build
//	and send the response. The pTRANDATA->TranCode is a key in what is to be done.
//	
//	The results should be placed into pTRANDATA by AnalyzeResponse().
//----------------------------------------------------------------------------
BOOL CHost::DoSubTransaction()
{
	CString data;
	BOOL  result = FALSE;

	// Open the connection if it hasn't been opened yet.
	if (!HostConnect())
	{
		TRACE(TEXT("HostConnect Failed\n"));
		return FALSE;
	}

	if (!pTRANDATA->bReversal)
	{
		TRACE(TEXT("not bReversal\n"));
		if (!FormatRequestData()) 
		{
			SetCommStatus(ST_REQUEST_ERROR);
			SetTransactionDateTime();
			return FALSE;
		}
	}
	// Perform the Send operation.
	CString traceData = CString(m_Request);
	if (traceData.GetLength() == 0)
		return FALSE;
	traceData.Replace(L"\x1C", L"?");
	TRACE(L"m_Request(len = %d): %s\n", traceData.GetLength(), traceData);

	if (!HostSend(m_Request, strlen(m_Request))) 
	{
		m_bBackupFlag = TRUE;
		m_Connected = FALSE;
		SetCommStatus(ST_SEND_ERROR);	// Communication Error
		SetTransactionDateTime();
		return FALSE;
	}

	// Reversal don't need to wait for response TDGPJ 2013APR16
	if (pTRANDATA->bReversal)
	{
		ClearRequest();
		return TRUE;
	}

	// Receive the data and analyze it.
	char recvData[SZ_HOST_BUFF_MAX];
	memset(recvData, 0, sizeof(recvData));
	if (!HostRecv(recvData) || (strlen(recvData) == 0)) 
	{
		m_bBackupFlag = FALSE;
		m_Connected = FALSE;
		SetCommStatus(ST_HOST_TIMEOUT);	// NO RESPONSE
		result = FALSE;

		USHORT seqNo = m_iSeqNo+1;
		CDataFile::Save(FID_TERM_SEQ_NUMBER, (BYTE *)&seqNo, sizeof(USHORT));				
		SetReversal(TIMEOUT_REVERSAL);

		return result;
	}
	else if (!AnalyzeResponse(recvData)) 
	{
		m_Connected = FALSE;

		if ( GetCommStatus() != ST_MAC_ERR)
			SetCommStatus(ST_RESPONSE_ERROR);
		result = FALSE;
	}
	else
	{
		SetCommStatus(ST_OK);
		// check for response code and set Status
		int respCode = atoi(pTRANDATA->RespCode);
		if (respCode >= 0 && respCode < 50)
		{
			SetTranStatus(ST_APPROVED);
		}
		else
		{
			if (pTRANDATA->TranCode > FUNCTION_FINAN_BEGIN && pTRANDATA->TranCode < FUNCTION_FINAN_END)
				SetTranStatus(ST_DECLINED);
		}
	}
	return result;
}

//----------------------------------------------------------------------------
//!	\brief	send, receive host messages, allow reversals
//----------------------------------------------------------------------------
BOOL CHost::DoReversibleSubTransaction()
{
	BOOL  result;

	// Record the transaction for possible future reversal
	result = DoSubTransaction();
	HostDisconnect();

	// Send the request to the host and get response
	// handle reversals
	if (pTRANDATA->bReversal)
	{
		FormatReversal();
	}
	TransactionPostProcess();
	return result;
}

//----------------------------------------------------------------------------
//!	\brief	do host connect
//----------------------------------------------------------------------------
BOOL CHost::HostConnect()
{
	if(m_Connected)
		return TRUE;
	{	
		TCHAR buf [100] = {0};

		char host[100] = {"everlinkpos1.everlink.ca"};//67.23.53.155"};
		CString str = L"HOST ADDRESS";
		if(CDataFile::Read(str,buf))
			strcpy(host,alloc_tchar_to_char(buf));

		int Port = 443;
		str = L"IP PORT";
		
		if(CDataFile::Read(str,buf))
			Port = _ttoi(buf);

		BOOL res = m_SSLSock.Connect(host, Port, m_hWnd);
		if (res)
			m_Connected = TRUE;
	}
	return m_Connected;
}

//----------------------------------------------------------------------------
//!	\brief	Send date to host
//! \param	buf - send data buffer point
//! \param	len - send data length
//----------------------------------------------------------------------------
BOOL CHost::HostSend(char *buf, WORD len)
{
	int bytes;
	{
		bytes = m_SSLSock.Send(buf, len);
		if ( bytes != len+2 )
			bytes = m_SSLSock.Resend(buf,len);
	}

	USHORT flag = 0;
	if( bytes > 2 )
	{
		CDataFile::Delete(FID_TERM_REVERSAL_RECORD);
		if ( !pTRANDATA->bReversal && pTRANDATA->TranCode != TRAN_CLOSE_BATCH && pTRANDATA->TranCode != TRAN_BATCH_TOTAL)
			flag = 1;
	}
	int size = sizeof(USHORT);
	CDataFile::Save(FID_TERM_SEQ_FLAG, (BYTE *)&flag, size);
	return (bytes == len+2); // 2 len bytes
}

//----------------------------------------------------------------------------
//!	\brief	Send date to host
//! \param	buff - receive data buffer point
//----------------------------------------------------------------------------
BOOL CHost::HostRecv(char *buff)
{
	int len;
	BOOL res;
	res = m_SSLSock.Read((BYTE *)buff, &len);
	if (res)
	{
		buff[len] = 0;
	}
	return res;	
}

//----------------------------------------------------------------------------
//!	\brief	disconnect host server
//----------------------------------------------------------------------------
void CHost::HostDisconnect(BOOL flag)
{
	if(pTRANDATA == NULL)
		return;

	m_SSLSock.Close();
	Sleep(300);
	m_Connected = FALSE;
}
//----------------------------------------------------------------------------
//!	\brief Main method to start through completion of a transaction
//!	\return TRUE if successful, FALSE if not.
//----------------------------------------------------------------------------
void CHost::HostComm()
{
	if (!HostConnect())
	{
		SetCommStatus(ST_CONNECT_FAILED);
		return;
	}
	else
	{
		SetCommStatus(ST_OK);
	}

	ProcessReversals();	// outstanding reversals
	
	if (GetCommStatus() != ST_OK)
	{
		HostDisconnect();
		return;
	}
	DoReversibleSubTransaction();

	HostDisconnect();
}

//-----------------------------------------------------------------------------
//!	\brief	Start Host Process
//-----------------------------------------------------------------------------
void CHost::Start(TRAN_TranDataRec *pTranReq, CONF_Data *pConfig, BOOL stopflag)
{
	// Initialize 			
	m_ContinuationRequest = FALSE;
	m_MoreData = FALSE;
	m_LastDID = 0;
	m_DownloadFiller[0] = 0;
	m_bIsDial = FALSE;
	m_bBackupFlag = FALSE;

	pTRANDATA = pTranReq;
	pCONFIG = pConfig;

	if (pTRANDATA->CardType == CARD_DEBIT)
		strcpy(pTRANDATA->ServType,"P0");
	// Init tran data
CDataFile::CheckMemory(L"HostComm");

	InitTransaction();

	m_Connected = FALSE;

	if( CDataFile::m_bConnected )
		HostComm();
	else
		m_bBackupFlag = TRUE;
CDataFile::CheckMemory(L"HostComm1");

	if( !m_bIsDial && m_bBackupFlag)
	{
		TCHAR buf[10]={L""};
		CDataFile::Read(L"DIAL BACKUP",buf);
		if(CString(buf) != L"Off")
		{
			m_bIsDial = TRUE;
			HostComm();
		}
	}
CDataFile::CheckMemory(L"HostComm2");

	if (pTRANDATA->TranCode == TRAN_HOST_INIT &&
			GetCommStatus() == ST_OK &&
			GetTranStatus() == ST_APPROVED)
	{
		SaveDownloadedData();
		if (pCONFIG->EMVEnabled)	// perform EMV download if enabled
		{
			CRecordFile tempInitFile;
			if (!tempInitFile.Create(TEMP_INIT_FILE, HOST_RESP_LENGTH, 100))
			{
				//assert(FALSE);
				return;
			}
			ClearRequest();
			m_ContinuationRequest = FALSE;
			m_MoreData = FALSE;
			m_LastDID = 0;
			m_EMVDownload = TRUE;
			SetTranStatus(ST_DECLINED);
			HostComm();
			if (GetCommStatus() == ST_OK &&
					GetTranStatus() == ST_APPROVED)
			{
				if (!SaveDownloadedData())
				{
					SetTranStatus(ST_RESPONSE_ERROR);
				}
			}
			tempInitFile.Delete(TEMP_INIT_FILE);
		}
	}
	if( stopflag )
		Stop();   
}

//----------------------------------------------------------------------------
//!	\brief	Stop host connect thread
//----------------------------------------------------------------------------
void CHost::Stop()
{
	if ( pTRANDATA == NULL)
		return;

	Sleep(200);
	if (m_Connected)
	{
		HostDisconnect();
		Sleep(200);
	}
	if(m_hWnd != NULL)
	{
		if(!CDataFile::m_bTraining)
			::PostMessage(m_hWnd, WM_SOCK_RECV, 2,0);
		m_hWnd = NULL;
	}
}
//----------------------------------------------------------------------------
//!	\brief	Process batch data in response message
//!	\return	true - field processed
//!			false - failed to process
//----------------------------------------------------------------------------
bool CHost::ProcessBatchData()
{
	ProcessShiftNumber();
	ProcessBatchNumber();
	ProcessDateTimeFrom();
	ProcessDateTimeTo();

    AP_FILE_HANDLER fh;
	CRecordFile hostfile;
	if ( !hostfile.Create(HOST_TOTALS_FILE,sizeof(sCardTotalRecord),MAX_NUMBER_CARD_TOTALS) )
	{
		TRACE(L"Can't create file!\n");
		return FALSE;
	}

	if (!hostfile.Open(HOST_TOTALS_FILE, &fh))
	{
		TRACE(L"Can't open file!\n");
		return FALSE;
	}

	// Total number of Card Totals in this message
	int cardTotals;
	GetInt( cardTotals, 2 );

	for ( int i = 0; i < cardTotals; ++i )
	{
		sCardTotalRecord total;
		char cardType[SZ_SERVICE_TYPE+1];
		int count;
		long int amount;
		int subtotalCount = 0;

		GetCStr( cardType, SZ_SERVICE_TYPE );
		total.serviceType[0] = cardType[0];
		total.serviceType[1] = cardType[1];
		for( int j=0; j<3; j++)
		{
			GetInt( count, 4 );
			subtotalCount += count;
			GetLongInt( amount, 19 );
			total.total[j].count = (USHORT)count;
			total.total[j].amount= amount;
		}
		if (subtotalCount > 0)
			hostfile.AddRecord(&fh, (BYTE*)&total, sizeof(sCardTotalRecord));
	}
	hostfile.CloseFile(&fh);
	return TRUE;
}
//----------------------------------------------------------------------------
//!	\brief	Process batch start date and time in response message
//!	\return	true - field processed
//!			false - failed to process
//----------------------------------------------------------------------------
bool CHost::ProcessDateTimeFrom()
{
	int temp[5];
	for ( int i=0 ;i < 5; i++)
		GetInt( temp[i], 2 );

	sprintf(pTRANDATA->DateTimeFrom,"20%02d/%02d/%02d %02d:%02d",temp[0],temp[1],temp[2],temp[3],temp[4]);

	return true;
}

//----------------------------------------------------------------------------
//!	\brief	Process current date and time in response message
//!	\return	true - field processed
//!			false - failed to process
//----------------------------------------------------------------------------
bool CHost::ProcessDateTimeTo()
{
	int temp[5];
	for ( int i=0 ;i < 5; i++)
		GetInt( temp[i], 2 );

	sprintf(pTRANDATA->DateTimeTo,"20%02d/%02d/%02d %02d:%02d",temp[0],temp[1],temp[2],temp[3],temp[4]);

	return true;
}
