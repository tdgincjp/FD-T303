// ECRi.cpp : implementation file
//

#include "stdafx.h"
#include "stdio.h"
#include "..\\..\\XACECR.h"
#include "..\\defs\\constant.h"
#include "..\\utils\\binUtil.h"
#include "..\\defs\\struct.h"
#include "..\\utils\\StrUtil.h"
#include "ECRi.h"
#include "SaioBase.h"

// CECRi 
CECRi::CECRi()
{
//	m_hMainWnd  = 0;
}

CECRi::~CECRi()
{
}

int CECRi::ECR2TxnCode(int EcrCode)
{
	int code;
	switch(EcrCode)
	{
		case ECR_PURCHASE:
			code = TRAN_PURCHASE;
			break;
		case ECR_PRE_AUTH:
			code = TRAN_PREAUTH;
			break;
		case ECR_ADVICE:
			code = TRAN_ADVICE;
			break;
		case ECR_FORCE_POST:
			code = TRAN_FORCE_POST;
			break;
		case ECR_REFUND:
			code = TRAN_REFUND;
			break;
		//case ECR_BALANCE_INQ:
		//	code = TRAN_PURCHASE;
		//	break;
		case ECR_PURCHASE_CORR:
			code = TRAN_PURCHASE_VOID;
			break;
		case ECR_REFUND_CORR:
			code = TRAN_REFUND_VOID;
			break;
		//case ECR_GET_PED_SERIAL:
		//	code = TRAN_PURCHASE;
		//	break;
		case ECR_CLOSE_BATCH:
			code = TRAN_CLOSE_BATCH;
			break;
		//case ECR_CLERK_TOTALS:
		//	code = TRAN_PURCHASE;
		//	break;
		//case ECR_EXTENDED_CFG:
		//	code = TRAN_PURCHASE;
		//	break;
			
		case ECR_INITIALIZATION:
			code = TRAN_HOST_INIT;
			break;
		case ECR_EMV_INITIALIZATION:
			code = TRAN_HOST_INIT;
			break;
		case ECR_HANDSHAKE:
			code = TRAN_HANDSHAKRE;
			break;
		case ECR_KEY_EXCHANGE:
			code = TRAN_KEY_EXCHANGE;
			break;
		case ECR_CANCEL:
			code = TRAN_CANCEL;
			break;
		default:
			assert(FALSE);
			break;
	}
	return code;
}

BOOL CECRi::Write(char *txt)
{
/*	if (m_hMainWnd == NULL) return false;
		
	COPYDATASTRUCT cpd;
	cpd.dwData = 0;
	cpd.cbData = txt.GetLength()*2;
	cpd.lpData = (void*)txt.GetBuffer(txt.GetLength());
	::SendMessage(m_hMainWnd,WM_COPYDATA,(WPARAM)AfxGetApp()->m_pMainWnd->GetSafeHwnd(),(LPARAM)&cpd);
	txt.ReleaseBuffer();
*/	return true;
}

//-----------------------------------------------------------------------------
//!	\brief	Skip num bytes in request
//-----------------------------------------------------------------------------
inline void CECRi::Skip(int num)
{
	m_Request += num;
}

//-----------------------------------------------------------------------------
//!	\brief	back num bytes in request
//-----------------------------------------------------------------------------
inline void CECRi::Back(int num)
{
	m_Request -= num;
}

//-----------------------------------------------------------------------------
//!	\brief	Check if it is end of field in buffer
//!	\param	data - buffer to check from, default - request buffer
//-----------------------------------------------------------------------------
bool CECRi::FieldEnd(char *data)
{
	if (!data)
	{
		data = m_Request;
	}

	switch(*data)
	{
		case FS:				// end of field
		case EOT:               //end of data
		case 0x00:			// end data
			return true;
	}

	return false;
}

//-----------------------------------------------------------------------------
//!	\brief	Calculate field length in response
//!	\param	maxLen - maximal allowed length
//-----------------------------------------------------------------------------
int CECRi::FieldLen(int maxLen)
{
	int length = 0;
	while (!FieldEnd(m_Request + length))
	{
		if (length >= maxLen)
		{
			break;
		}
		length++;
	}

	return length;
}

//-----------------------------------------------------------------------------
//!	\brief	Get int from ECR request
//!	\param	x - number
//!	\param	size - count of ASCII characters to use from request
//!	\note	sscanf with format '%<count>d' is used to get int from request
//-----------------------------------------------------------------------------
void CECRi::GetInt(int &x, int size)
{
	char str[10];
	GetStr(str, size);
	x = atoi(str);
//	m_Request += size;
}

//-----------------------------------------------------------------------------
//!	\brief	Get long int from ECR request
//!	\param	x - number
//!	\param	size - count of ASCII characters to use from request
//!	\note	sscanf with format '%<count>ld' is used to get int from request
//-----------------------------------------------------------------------------
void CECRi::GetLongInt(long int &x, int size)
{
	char str[15];
	GetStr(str, size);
	x = atol(str);
	m_Request += size+1;
}

//-----------------------------------------------------------------------------
//!	\brief	Get string from ECR request
//!	\param	str - destination string
//!	\param	size - size of data, should be smaller than sizeof str
//-----------------------------------------------------------------------------
inline void CECRi::GetStr(char *str, int size)
{
	memcpy(str, m_Request, size*sizeof(char));
	str[size] = 0; // make string
	m_Request += size+1;
}

//-----------------------------------------------------------------------------
//!	\brief	Get Transaction Code
//-----------------------------------------------------------------------------
int CECRi::GetTransactionCode()
{
	int length = FieldLen(99);

	if (length != SZ_TRAN_CODE)
		return ST_BAD_TRNCD;

	GetInt(ECR_TRANREC.EcrCode, length);
	if (ECR_TRANREC.EcrCode != ECR_HOST_INIT &&
			ECR_TRANREC.EcrCode != ECR_HANDSHAKE &&
			ECR_TRANREC.EcrCode != ECR_KEY_EXCHANGE &&
			ECR_TRANREC.EcrCode != ECR_CLOSE_BATCH &&
			ECR_TRANREC.EcrCode != ECR_PURCHASE)
	{
		return ST_BAD_TRNCD;
	}
	ECR_TRANREC.TranCode = ECR2TxnCode(ECR_TRANREC.EcrCode);

	return ST_ECR_OK;
}

//-----------------------------------------------------------------------------
//!	\brief	Get Terminal ID
//-----------------------------------------------------------------------------
int CECRi::GetTerminalId()
{
	int length = FieldLen(99);

	if (length == 0)
		return ST_NO_ECR_ID;

	if (length != SZ_TERMID /*|| !IsAlphanumeric(m_Request, length)*/)
		return ST_BAD_ECR_ID;

	GetStr(ECR_TRANREC.TerminalID, length);
	return ST_ECR_OK;
}

//-----------------------------------------------------------------------------
//!	\brief	Get Amount
//-----------------------------------------------------------------------------
int CECRi::GetAmount()
{
	long amount;
	int length = FieldLen(99);
	char tmpBuf[50];

	if (length == 0)
		return ST_NO_AMT;

	memset(tmpBuf, 0, sizeof(tmpBuf));
	GetStr(tmpBuf, length);
	amount = STR_atol(tmpBuf, length);
	if (amount == 0)
		return ST_BAD_AMT;

	if (length > SZ_AMOUNT /*|| !IsDecimalString(pField, length)*/)
		return ST_BAD_AMT;

	ECR_TRANREC.Amount = amount;
	return ST_ECR_OK;
}
//-----------------------------------------------------------------------------
//!	\brief	Get Invoice
//-----------------------------------------------------------------------------
int CECRi::GetInvoice()
{
	long Invoice;
	int length = FieldLen(99);
	char tmpBuf[50];

	if (length == 0)
		return ST_NO_AMT;

	memset(tmpBuf, 0, sizeof(tmpBuf));
	GetStr(tmpBuf, length);
	Invoice = STR_atol(tmpBuf, length);
	if (Invoice == 0)
		return ST_BAD_AMT;

	if (length > SZ_INVOICE_NO /*|| !IsDecimalString(pField, length)*/)
		return ST_BAD_AMT;

	ECR_TRANREC.Invoice = Invoice;
	return ST_ECR_OK;
}
//-----------------------------------------------------------------------------
//!	\brief	Parse Host Init command
//-----------------------------------------------------------------------------
int CECRi::ParseHostInit()
{
	return GetTerminalId();
}

//-----------------------------------------------------------------------------
//!	\brief	Parse Purchase command
//-----------------------------------------------------------------------------
int CECRi::ParsePurchase()
{
	int err = GetAmount();
	if( err != ST_ECR_OK)
		return err;

	return GetInvoice();
}

//-----------------------------------------------------------------------------
//!	\brief	Parse Request from ECR
//-----------------------------------------------------------------------------
int CECRi::ParseEcrRequest(char *reqBuf)
{
	int err;

	// points at beginning of receive buffer
	// this is the request received from the ECR
	m_Request = reqBuf;

	// first get transaction code
	err = GetTransactionCode();
	if (err != ST_ECR_OK)
	{
		return err;
	}

//	switch (ECR_TRANREC.TranCode) //?? wrong? should be ECR_TRANREC.EcrCode
	switch (ECR_TRANREC.EcrCode) 
	{
	case ECR_INITIALIZATION:
	case ECR_KEY_EXCHANGE:
	case ECR_CLOSE_BATCH:
		err = ParseHostInit();
		break;
	case ECR_PURCHASE:
		err = ParsePurchase();
		break;
	case ECR_CANCEL:
		break;
	default:
		// error handling here
		break;
	}

	return err;
}

//-----------------------------------------------------------------------------
//!	\brief	put char to request
//!	\param	c - char to put
//-----------------------------------------------------------------------------
void CECRi::Put(char c)
{
	m_Response[m_ResponseLength++] = c;
}

//-----------------------------------------------------------------------------
//!	\brief	put chars to request
//!	\param	c - char to move
//!	\param	count - numbers of c
//-----------------------------------------------------------------------------
void CECRi::Put(char c, int count)
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
void CECRi::Put(const char *str)
{
	strcpy(&m_Response[m_ResponseLength], str);
	m_ResponseLength += strlen(str);
}

//-----------------------------------------------------------------------------
//!	\brief	Append zero-terminated string to request
//!	\param	s - zero-terminated string
//!	\param	maxLength - safety counter, maximal allowed length
//-----------------------------------------------------------------------------
void CECRi::Put(const char *str, int maxLength)
{
	int len = (maxLength < (int)strlen(str))? maxLength : strlen(str);
	strncpy(&m_Response[m_ResponseLength], str, len);
	m_ResponseLength += len;
}


//-----------------------------------------------------------------------------
//!	\brief	Append zero-terminated string to request, right fill till 
//!					totalLength characters
//!	\param	s - zero-terminated string
//!	\param	totalLength - total length
//!	\param	fill - character to fill with
//-----------------------------------------------------------------------------
void CECRi::PutRightFilled(const char *str, int totalLength, char fill)
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
void CECRi::PutBCDAscii(const BYTE *buffer, int size)
{
	char *ascii = new char[2*size + 1];
	memset(ascii, 0, 2*size + 1);

	// unpack data then move to request.
	STR_UnpackData(ascii, (BYTE *)buffer, size);
	ascii[2*size] = 0;
	Put(ascii);
	delete[] ascii;
}

//-----------------------------------------------------------------------------
//!	\brief	Format Response to ECR
//-----------------------------------------------------------------------------
void CECRi::FormatEcrResponse(int status, TRAN_TranDataRec *pRec)
{
	m_ResponseLength = 0;
	memset(m_Response, 0, sizeof(m_Response));
	if (pRec != NULL)
	{
//		assert(FALSE);
	}
	else
	{
//		assert(FALSE);
	}
	
	/*
	Condition Code							F3	N		M	
	Transaction Code						F2	N		M	00 (Purchase)
	Date/Time										F12	N		M	
	Amount (Base24 transaction)	V9	N		M	
	Customer Account Number2		V19	AN	C1	
	Expiry Date									F4	N		C1	
	ISO Response Code						F2	AN	C1, 2	
	Base24 Response Code				F3	N		C1, 2	
	Approval Number							V8	AN	C1	
	Unique Transaction ID				F10	N		M1	
	Response Language Code			F1	N		M	
	ECR Number									F8	AN	M	
	Swiped Indicator						F1	A		M	
	Card/Account Type						F1	N		C1	
	Card Name 									V12	AN	C1	
	CVM Indicator								F1	A		C1	
	EMV Application ID (AID)		V32	AN	C1	
	Application Label						V16	ANS	C		Present if provided by the card
	Application Preferred Name	V16	AN	C		Present if provided by the card
	ARQC												F16	AN	C1	
	TVR for ARQC								F10	AN	C1	
	TC or AAC										F16	AN	C1	
	TVR use for TC/AAC					F10	AN	C1	
	TSI													F4	AN	C1	
	*/
}