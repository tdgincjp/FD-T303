
// DataFile.cpp : implementation file
//

#include "stdafx.h"
#include <stdio.h>

#include "Printer.h"
#include "SaioBase.h"
#include "..\\UI\\language.h"
#include "..\\EMV\\EMV.h"
#include "..\\data\\AppData.h"
#include "..\\data\\RecordFile.h"
#include "..\\UTILS\\string.h"

#define ERR_INVALIDPARAM	1
#define ERR_KEY_NOTFOUND	2
#define ERR_CREATEDC_FAILED	3

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CPrinter::CPrinter()
{
	m_iLine = 0;
	m_iLineHeight = 32;

	m_iStartPos = 0;
	m_iEndPos = 380;
	m_iCenter = (m_iEndPos - m_iStartPos)/2;

	m_declinedOffline = FALSE;

}

//-----------------------------------------------------------------------------
//!	\print entry
//-----------------------------------------------------------------------------
DWORD CPrinter::Printing()
{	
	memset(m_Value,0,MAX_VALUE_LEN);
	m_iValueLen = 0;
	m_bCancelFlag = FALSE;
	//if(pTRANDATA->CardType == CARD_DEBIT)
	//	PrintDebit();
	//else //if(pTRANDATA->CardType == CARD_CREDIT)
	//	PrintCredit();

	PrintFirstDataReceipt();

	return 0;
}

//-----------------------------------------------------------------------------
//!	\print text line
//!	\param	dc - HDC id 
//!	\param	str - print text string 
//!	\param	OffPos - Offsize position 
//!	\param	format - print format 
//-----------------------------------------------------------------------------
void CPrinter::TextLine(HDC& dc,CString str,int OffPos, int format)
{
	if (str.GetLength() == 0)
		return;

	char p[128] = {0};
	int nIndex = WideCharToMultiByte(CP_ACP, 0, str, -1, NULL, 0, NULL, NULL);
	WideCharToMultiByte(CP_ACP, 0, str, -1, (char*)p, nIndex, NULL, NULL);
	m_iLine++;

	char p1[128] ={0};
	sprintf(p1,"[%d][%d][%d]%s",m_iLine,format == TA_CENTER ? 0x03:0x01,nIndex-1,p);
	// JC April 24/15 This is a bug, contrary to spec.
	// should be %c with no braces, just the byte value of the int
	//sprintf(p1,"%c%c%c%s|",m_iLine,format == TA_CENTER ? 0x03:0x01,nIndex,p);
	// JC April 24/15

	strcat((char*)&m_Value,p1);
	m_iValueLen += strlen(p1);
}

//-----------------------------------------------------------------------------
//!	\print text line
//!	\param	dc - HDC id 
//!	\param	str - print text string1 
//!	\param	OffPos - Offsize position1 
//!	\param	str1 - print text string2 
//!	\param	OffPos1 - Offsize position2 
//!	\param	format - print format 
//-----------------------------------------------------------------------------
void CPrinter::TextLine(HDC& dc,CString str,CString str1,int OffPos,int OffPos1)
{
	if (str.GetLength() == 0 && str1.GetLength() == 0)
		return;

	char p[128] = {0};
	int nIndex = WideCharToMultiByte(CP_ACP, 0, str, -1, NULL, 0, NULL, NULL);
	WideCharToMultiByte(CP_ACP, 0, str, -1, (char*)p, nIndex, NULL, NULL);
	m_iLine++;

	char p1[128] ={0};
	sprintf(p1,"[%d][1][%d]%s",m_iLine,nIndex-1,p);
	//sprintf(p1,"%c%c%c%s|",m_iLine,1,nIndex,p);		//JC April 24/15
	strcat((char*)&m_Value,p1);
	m_iValueLen += strlen(p1);

	nIndex = WideCharToMultiByte(CP_ACP, 0, str1, -1, NULL, 0, NULL, NULL);
	WideCharToMultiByte(CP_ACP, 0, str1, -1, (char*)p, nIndex, NULL, NULL);

	sprintf(p1,"[%d][2][%d]%s",m_iLine,nIndex-1,p);
	//sprintf(p1,"%c%c%c%s|",m_iLine,2,nIndex,p);		//JC April 24/15
	strcat((char*)&m_Value,p1);
	m_iValueLen += strlen(p1);
}
	
//-----------------------------------------------------------------------------
//!	\draw a line
//!	\param	dc - HDC id 
//-----------------------------------------------------------------------------
void CPrinter::DrawLine(HDC& dc)
{
	CString str= _T("-------------------------------------");
	TextLine(dc,str);
}
	
//-----------------------------------------------------------------------------
//!	\print one empty line
//-----------------------------------------------------------------------------
void CPrinter::EmptyLine()
{
	m_iLine++;
}

//-----------------------------------------------------------------------------
//!	\print debit receipt
//-----------------------------------------------------------------------------
DWORD CPrinter::PrintDebit()
{
	CString str = L"";
	CString str1 = L"";
	USHORT value;

	m_iLine = 0;
	HDC dc;
	//4 to 7.Merchant name and Address.

	TCHAR buf[100]={L""};
	EmptyLine();
	PrintHeader(dc);
	EmptyLine();


	//Terminal ID

	TextLine(dc,CLanguage::GetText(CLanguage::IDX_RECEIPT_TERM),CString(m_CONFIG.TermID));
	//str += _T("INVOICE #             9999999\n");	
	//Entered Invoice Number
	TextLine(dc,CLanguage::GetText(CLanguage::IDX_RECEIPT_INVOICE),CString(pTRANDATA->InvoiceNo));

	//Clerk ID and Batch # 
	//str += _T("CLERK:NNNN             B:NNN\n");
	DWORD size = sizeof(USHORT);
	if (!CDataFile::Read((int)FID_TERM_BATCH_NUMBER, (BYTE *)&value, &size))
		value = 1;

	CString strTemp = CString(pTRANDATA->ClerkID);
	if (CDataFile::Read(L"CLERK ID", buf) && CString(buf) == L"Off")
		strTemp = L"";

	if(strTemp == L"")
	{
		str = L"";
	}
	else
	{
		str.Format(L"%s:%s",CLanguage::GetText(CLanguage::IDX_RECEIPT_CLERK),CString(pTRANDATA->ClerkID));
	}
	str1.Format(L"%s:%03d",CLanguage::GetText(CLanguage::IDX_RECEIPT_BATCH_ONE),value);
	TextLine(dc,str,str1);

	if(pTRANDATA->ComStatus == ST_OK && pTRANDATA->TranStatus == ST_APPROVED )
		str.Format(L"%s:  %s",CLanguage::GetText(CLanguage::IDX_RECEIPT_AUTH),CString(pTRANDATA->AuthCode));
	else
		str = CLanguage::GetText(CLanguage::IDX_RECEIPT_AUTH);
	TextLine(dc,str);
	str.Format(L"%s: %s",CLanguage::GetText(CLanguage::IDX_RECEIPT_TRANS),CString(pTRANDATA->SequenceNum));

	CString str2 = GetEntryName();

	TextLine(dc,str);

	str = CLanguage::GetText(CLanguage::IDX_ENTRY_MODE);
	TextLine(dc,str,str2);

	DrawLine(dc);

	str1.Format(L"%s",CString(pTRANDATA->Account));
	for(int i=4;i<str1.GetLength()-4;i++)
		str1.SetAt(i,'*');
	str = CLanguage::GetText(CLanguage::IDX_RECEIPT_CARD);
	TextLine(dc,str,str1);

	switch(pTRANDATA->AcctType)
	{
	case ACCT_SAVING:
		str1 = CLanguage::GetText(CLanguage::IDX_SAVINGS);
		break;
	case ACCT_DEFAULT:
		str1 = CLanguage::GetText(CLanguage::IDX_DEFAULT);
		break;
	default:
		str1 = CLanguage::GetText(CLanguage::IDX_CHEQUING);
		break;
	}

	str.Format(L"%s / %s",CLanguage::GetText(CLanguage::IDX_RECEIPT_DEBIT),str1);
	TextLine(dc,str);

	//str += _T("DATE          YYYY/MM/DD\n");Transaction Date
	str1 = CString(pTRANDATA->TranDate);
	str = CLanguage::GetText(CLanguage::IDX_RECEIPT_DATE);
	str2.Format(L"20%s/%s/%s",str1.Mid(0,2),str1.Mid(2,2),str1.Mid(4,2));
	TextLine(dc,str,str2);

	//str += _T("TIME            HH:MM:SS\n");Transaction Time
	str1 = CString(pTRANDATA->TranTime);
	str = CLanguage::GetText(CLanguage::IDX_RECEIPT_TIME);
	str2.Format(L"%s:%s:%s",str1.Mid(0,2),str1.Mid(2,2),str1.Mid(4,2));
	TextLine(dc,str,str2);

	//str += _T("RECEIPT     RRRRRRRRRRRR\n");The entered receipt number.
	str1 = CString(pTRANDATA->ReceiptNo);
	if (CDataFile::Read(L"RECEIPT", buf) && CString(buf) == L"Off")
		str1 = L"";

	if (str1 != L"")
	{
		str = CLanguage::GetText(CLanguage::IDX_RECEIPT_RECEIPT);
		TextLine(dc,str,str1);
	}

	DrawLine(dc);

	str1 = GetTransactionType();
	TextLine(dc,str1,0,TA_CENTER);

	str = CLanguage::GetText(CLanguage::IDX_RECEIPT_AMOUNT);
	str1.Format(L"$%s",DealAmount(pTRANDATA->Amount));
	TextLine(dc,str,str1);

	BOOL TotalAmountFlag = FALSE;

	if(strlen(pTRANDATA->CashbackAmount)>0 && (pTRANDATA->TranCode == TRAN_PURCHASE || pTRANDATA->TranCode == TRAN_PURCHASE_VOID ))
	{
		str = CLanguage::GetText(CLanguage::IDX_RECEIPT_CASHBACK);
		str1.Format(L"$%s",DealAmount(pTRANDATA->CashbackAmount));
		TextLine(dc,str,str1);
		TotalAmountFlag = TRUE;
	}

	if(strlen(pTRANDATA->TipAmount)>1 && (pTRANDATA->TranCode == TRAN_PURCHASE || pTRANDATA->TranCode == TRAN_PURCHASE_VOID ))
	{
		str = CLanguage::GetText(CLanguage::IDX_RECEIPT_TIP);
		str1.Format(L"$%s",DealAmount(pTRANDATA->TipAmount));
		TextLine(dc,str,str1);
		TotalAmountFlag = TRUE;
	}

	if(strlen(pTRANDATA->SurchargeAmount)>0 && (pTRANDATA->TranCode == TRAN_PURCHASE || pTRANDATA->TranCode == TRAN_PURCHASE_VOID ))
	{
		str = CLanguage::GetText(CLanguage::IDX_RECEIPT_SURCHANGE);
		str1.Format(L"$%s",DealAmount(pTRANDATA->SurchargeAmount));
		TextLine(dc,str,str1);
		TotalAmountFlag = TRUE;
	}
	else if(strlen(pTRANDATA->CashbackFee)>0 && (pTRANDATA->TranCode == TRAN_PURCHASE || pTRANDATA->TranCode == TRAN_PURCHASE_VOID ))
	{
		str = CLanguage::GetText(CLanguage::IDX_RECEIPT_SURCHANGE);
		str1.Format(L"$%s",DealAmount(pTRANDATA->CashbackFee));
		TextLine(dc,str,str1);
		TotalAmountFlag = TRUE;
	}
	
	if(TotalAmountFlag)
	{
		str = CLanguage::GetText(CLanguage::IDX_RECEIPT_TOTAL);
		str1.Format(L"$%s",DealAmount(pTRANDATA->TotalAmount));
		TextLine(dc,str,str1);
		m_iLine++;
	}

	if ( pTRANDATA->bPartialAuth )
	{
		TextLine(dc,CLanguage::GetText(CLanguage::IDX_RECEIPT_APPROVED),L"$"+CString(DealAmount(pTRANDATA->AuthAmount)));
		TextLine(dc,CLanguage::GetText(CLanguage::IDX_AMOUNT_DUE),L"$"+CString(DealAmount(pTRANDATA->DueAmount)));
		EmptyLine();
	}

	if ( strlen(pTRANDATA->AvailableBalance) > 0 &&  m_iStatus== 2)
	{
		str = pTRANDATA->AvailableBalance[0] == '-'? L"-": L"" ;
		TextLine(dc,CLanguage::GetText(CLanguage::IDX_AVAIL_BAL),str + L"$" + CString(DealAmount(&pTRANDATA->AvailableBalance[1])));
	}

	PrintEMVData(dc);
	str = L"";

	DrawLine(dc);
	str= CLanguage::GetText(CLanguage::IDX_RECEIPT_TRANSACTION);//_T("TRANSACTION");
	TextLine(dc,str,0,TA_CENTER);

	str1.Format(L"%s",(pTRANDATA->bReversal && (CString(pTRANDATA->ISORespCode) == L"00"))?L"999":CString(pTRANDATA->ISORespCode));
	if(pTRANDATA->ComStatus == ST_OK)
	{
		if(pTRANDATA->TranStatus == ST_APPROVED)
		{
			if (pTRANDATA->bPartialAuth)
				str.Format( _T("%s–%s–%s"),str1,CLanguage::GetText(CLanguage::IDX_PARTIAL_APPROVED),CString(pTRANDATA->RespCode));
			else
				str.Format( _T(" %s – %s – %s"),str1,CLanguage::GetText(CLanguage::IDX_RECEIPT_APPROVED),CString(pTRANDATA->RespCode));
		}
		else if (pTRANDATA->bReversal)
			str.Format( _T(" – %s – "),CLanguage::GetText(CLanguage::IDX_DECLINED));
		else
		{
			if (str1 == L"00")
				str1 = L"";
			str2 = CString(pTRANDATA->RespCode);
			if (str2 == L"001")
				str2 = L"";
			str.Format( _T(" %s – %s – %s"),str1,CLanguage::GetText(CLanguage::IDX_RECEIPT_NOT_APPROVED),str2);
		}
	}
	else 
	{
		if ( m_declinedOffline )
			str.Format( _T(" – %s – "),CLanguage::GetText(CLanguage::IDX_DECLINED));
		else
			str.Format( _T(" – %s – "),CLanguage::GetText(CLanguage::IDX_RECEIPT_NOT_COMPLETED));
	}

	TextLine(dc,str,0,TA_CENTER);
	EmptyLine();

	if(pTRANDATA->bRequestedPIN && pTRANDATA->bEmvTransaction)
	{
		str =  CLanguage::GetText(CLanguage::IDX_RECEIPT_VERIFIED_BY_PIN);
		TextLine(dc,str,0,TA_CENTER);
	}
	
	EmptyLine();

	if ( m_iStatus == 1 )
		str =  CLanguage::GetText(CLanguage::IDX_RECEIPT_MERCHANT_COPY);
	else if ( m_iStatus== 2 )
		str =  CLanguage::GetText(CLanguage::IDX_RECEIPT_CUSTOMER_COPY);
	else 
		str = L"";

	if ( str.GetLength() > 0 )
		TextLine(dc,str,0,TA_CENTER);
		
	if( m_bCancelFlag )
		 str.Format( L"***%s***",CLanguage::GetText(CLanguage::IDX_RECEIPT_CANCELLED));
	else
		str = L"";

	if(str.GetLength()>0)
		TextLine(dc,str,0,TA_CENTER);

	PrintTrailer(dc);
	return 0;
}

DWORD CPrinter::PrintFirstDataReceipt()
{
	CString str = L"";
	CString str1 = L"";
	CString str2 = L"";
//	USHORT value;

	m_iLine = 0;
	HDC dc;
	
	TCHAR buf[100]={L""};
	EmptyLine();
	//		Merchant Name
	//		Merchant Address
	//		City, State
	//lines 2 to 4 (center)
	PrintHeader(dc);
	EmptyLine();



	//str += _T("DATE          YYYY/MM/DD\n");Transaction Date
	str1 = CString(pTRANDATA->TranDate);
	str = CLanguage::GetText(CLanguage::IDX_RECEIPT_DATE);
	str2.Format(L" 20%s/%s/%s",str1.Mid(0,2),str1.Mid(2,2),str1.Mid(4,2));
	str.Append(str2);
	//TextLine(dc,str,str2);

	//str1 += _T("TIME            HH:MM:SS\n");Transaction Time
	str1 = CString(pTRANDATA->TranTime);
	str2.Format(L" %s:%s:%s",str1.Mid(0,2),str1.Mid(2,2),str1.Mid(4,2));
	str1 = CLanguage::GetText(CLanguage::IDX_RECEIPT_TIME);
	str1.Append(str2);
	TextLine(dc,str,str1);

	EmptyLine();

	//Merchant ID
	TCHAR MID[SZ_DATAWIRE_MID] = _T("");
	CDataFile::Read(L"Datawire MID", MID);
	CString sMID = CString(MID);
	TextLine(dc,CLanguage::GetText(CLanguage::IDX_RECEIPT_MERCH),sMID);

	//Terminal ID
	TextLine(dc,CLanguage::GetText(CLanguage::IDX_RECEIPT_TERM),CString(m_CONFIG.TermID));


	//Entered Invoice Number
	TextLine(dc,CLanguage::GetText(CLanguage::IDX_RECEIPT_RECEIPTNUM),CString(pTRANDATA->InvoiceNo));

	EmptyLine();
	//Card Type
	
	if (pTRANDATA->ServType[0] == 'V')
		str1 = L"Visa";
	else if (pTRANDATA->ServType[0] == 'M')
		str1 = L"MasterCard";
	else if (strcmp(pTRANDATA->ServType, "AX" ) == 0 )
		str1 = L"American Express";
	else if (strcmp(pTRANDATA->ServType, "DS" ) == 0 ) 
		str1 = L"Discover";
	else if (strcmp(pTRANDATA->ServType, "DC" ) == 0 ) 
		str1 = L"Diners Club";
	else if (strcmp(pTRANDATA->ServType, "JC" ) == 0)
		str1 = L"JCB";
	else if (strcmp(pTRANDATA->ServType, "P0" ) == 0)
		str1 = L"Debit";
	str.Format(L"%s  %s", CLanguage::GetText(CLanguage::IDX_RECEIPT_CARDTYPE), str1);
	TextLine(dc, str);

	//ACCOUNT
	str1.Format(L"%s",CString(pTRANDATA->Account));
	for(int i=0;i<str1.GetLength()-4;i++)
		str1.SetAt(i,'*');
	str = CLanguage::GetText(CLanguage::IDX_RECEIPT_CARD);
	TextLine(dc,str,str1);
	
	// AUTH: xxxxxx
	if(pTRANDATA->ComStatus == ST_OK && (pTRANDATA->TranStatus == ST_APPROVED ||  pTRANDATA->bReversal ))
		str.Format(L"%s:  %s",CLanguage::GetText(CLanguage::IDX_RECEIPT_AUTH),CString(pTRANDATA->AuthCode));
	else
		str = CLanguage::GetText(CLanguage::IDX_RECEIPT_AUTH);
	TextLine(dc,str);

	EmptyLine();
	DrawLine(dc);

/*
		The transaction type (left-justified) (XXX…) can be:
		 “SALE” (ACHAT);
		 “SALE VOID” (CORRECTION D’ACHAT);
		 “REFUND” (REMISE D’ACHAT);
		 “REFUND VOID” (CORRECT. REMISE D’ACHAT).
	str += _T("XXXXXXXXXXXXXXXXXXXXXXXX\n");
*/
	str1 = GetTransactionType();
	TextLine(dc,str1,0,TA_CENTER);

/*
	Transaction amount excluding cashback, tip and surcharge.
		OR
		Amount for debit refund, debit sale void or debit refund void.
		str += _T("AMOUNT           $99,999.99\n");
*/
	str = CLanguage::GetText(CLanguage::IDX_RECEIPT_AMOUNT);
	str1.Format(L"$%s",DealAmount(pTRANDATA->Amount));
	TextLine(dc,str,str1);

	BOOL TotalAmountFlag = FALSE;


/*
		This line is only printed if tip was entered in debit sale transactions.
		Not to be printed on debit refund, debit sale void or debit refund void.
		str += _T("TIP      $99,999.99\n");
*/
	if(strlen(pTRANDATA->TipAmount)>0 && (pTRANDATA->TranCode == TRAN_PURCHASE || pTRANDATA->TranCode == TRAN_PURCHASE_VOID || pTRANDATA->TranCode == TRAN_ADVICE))
	{
		str = CLanguage::GetText(CLanguage::IDX_RECEIPT_TIP);
		str1.Format(L"$%s",DealAmount(pTRANDATA->TipAmount));
		TextLine(dc,str,str1);
		TotalAmountFlag = TRUE;
	}

/*
	Total amount including cashback, tip and surcharge. This line is printed only if at least one of the following lines are printed:
	 Cashback line;
	 Tip line;
	 Surcharge line.
	str += _T("TOTAL         $99,999.99\n");
*/
	if(TotalAmountFlag)
	{
		str = CLanguage::GetText(CLanguage::IDX_RECEIPT_TOTAL);
		str1.Format(L"$%s",DealAmount(pTRANDATA->TotalAmount));
//		SetFont(dc,FW_BOLD,44);
		TextLine(dc,str,str1);
		m_iLine++;
//		SetFont(dc,FW_NORMAL);
	}
//	DrawLine(dc);

	if ( pTRANDATA->bPartialAuth )
	{
//		SetFont(dc,FW_BOLD);
		TextLine(dc,CLanguage::GetText(CLanguage::IDX_RECEIPT_APPROVED),L"$"+CString(DealAmount(pTRANDATA->AuthAmount)));
		TextLine(dc,CLanguage::GetText(CLanguage::IDX_AMOUNT_DUE),L"$"+CString(DealAmount(pTRANDATA->DueAmount)));
//		SetFont(dc,FW_NORMAL);
		EmptyLine();
	}

	if ( strlen(pTRANDATA->AvailableBalance) > 0 &&  m_iStatus== 2)
	{
		str = pTRANDATA->AvailableBalance[0] == '-'? L"-": L"" ;
		TextLine(dc,CLanguage::GetText(CLanguage::IDX_AVAIL_BAL),str + L"$" + CString(DealAmount(&pTRANDATA->AvailableBalance[1])));
//		EmptyLine();
	}

	if ( pTRANDATA->TranCode == TRAN_PREAUTH && pTRANDATA->TranStatus == ST_APPROVED)
	{
		CDataFile::Read(L"TIP PROMPT", buf);
		
		if ( CString(buf) != L"Off")
		{
			EmptyLine();
			TextLine(dc,CLanguage::GetText(CLanguage::IDX_RECEIPT_TIP),L"---------------------");
			EmptyLine();
			TextLine(dc,CLanguage::GetText(CLanguage::IDX_RECEIPT_TOTAL),L"---------------------");
		}
	}

	PrintEMVData(dc);

/*
This line is printed for a fall back situation and if the card swiped was a chip card.
*/
	EmptyLine();

	str = L"";
/*	if (pTRANDATA->bFallback)
	{
		if(pTRANDATA->EntryMode == ENTRY_MODE_SWIPED)
			str = CLanguage::GetText(CLanguage::IDX_RECEIPT_CHIP_CARD_SWIPED);
		else if (pTRANDATA->EntryMode == ENTRY_MODE_MANUAL)
			str = CLanguage::GetText(CLanguage::IDX_RECEIPT_CHIP_CARD_KEYED);
		TextLine(dc,str);
	}
*/

	DrawLine(dc);

	str= CLanguage::GetText(CLanguage::IDX_RECEIPT_TRANSACTION);//_T("TRANSACTION");
	TextLine(dc,str,0,TA_CENTER);

/*
This line is printed for approved transactions
“HHH” represents the host response code.
“II” represents the ISO code coming from the host response.
If a debit reversal is done, ‘99’ is printed if original ISO code was ‘00’.
	str += _T(" II – APPROVED – HHH\n");

This line is printed for not approved transactions.
“HHH” represents the host response code.
“II” represents the ISO code coming from the host response.
If a debit reversal is done, ‘999’ is printed if ISO code was ‘00’; otherwise, original response code is printed.
	str += _T(" II – NOT APPROVED – HHH\n");

This line is printed for not completed transactions.
“HHH” represents the host response code.
“II” represents the ISO code coning from the host response.
If a debit reversal is done, ‘999’ is printed if ISO code was ‘00’; otherwise, original response code is printed.
	str += _T("II – NOT COMPLETED – HHH\n\n");



*/
	BOOL bApproved = FALSE;
	str1 = CString(pTRANDATA->ISORespCode);
	if(pTRANDATA->ComStatus == ST_OK)
	{
		if(pTRANDATA->TranStatus == ST_APPROVED)
		{
			if (pTRANDATA->bPartialAuth)
				str.Format( _T("%s–%s–%s"),str1,CLanguage::GetText(CLanguage::IDX_PARTIAL_APPROVED),CString(pTRANDATA->RespCode));
			else
				str.Format( _T(" %s – %s – %s"),str1,CLanguage::GetText(CLanguage::IDX_RECEIPT_APPROVED),CString(pTRANDATA->RespCode));
			bApproved = TRUE;
		}
		else if (pTRANDATA->bReversal)
			str.Format( _T(" – %s – "),CLanguage::GetText(CLanguage::IDX_DECLINED));
//			str.Format( _T(" %s – %s – %s\n"),str1,CLanguage::GetText(CLanguage::IDX_RECEIPT_NOT_COMPLETED),CString(pTRANDATA->RespCode));
		else
		{
			if (str1 == L"00")
				str1 = L"";
			str2 = CString(pTRANDATA->RespCode);
			if (str2 == L"001")
				str2 = L"";
			str.Format( _T(" %s – %s – %s"),str1,CLanguage::GetText(CLanguage::IDX_RECEIPT_NOT_APPROVED),str2);
		}
	}
	else
	{
		if ( m_declinedOffline )
			str.Format( _T(" – %s – "),CLanguage::GetText(CLanguage::IDX_DECLINED));
		else
			str.Format( _T(" – %s – "),CLanguage::GetText(CLanguage::IDX_RECEIPT_NOT_COMPLETED));
	}

	TextLine(dc,str,0,TA_CENTER);
	EmptyLine();

	if ( pTRANDATA->EntryMode == ENTRY_MODE_CHIP  //chip card
			&& strcmp(pTRANDATA->ServType,"M")==0 //Master Card
			&& pTRANDATA->bEmvTransaction)        // EMV
	{

		if( pTRANDATA->TranStatus == ST_APPROVED && ( pTRANDATA->EmvError != EMV_NO_ERROR ))
		{
			str = CLanguage::GetText(CLanguage::IDX_RECEIPT_CHIP_CARD_MALFUNCTION); //L"CHIP CARD MALFUNCTION";
//			TextLine(dc,str,0,TA_CENTER);
		}
	}
	EmptyLine();

	if ( bApproved &&// m_iStatus == 1 && 
		(pTRANDATA->TranCode == TRAN_PURCHASE
//		|| pTRANDATA->TranCode == TRAN_REFUND
		|| pTRANDATA->TranCode == TRAN_PREAUTH
		|| pTRANDATA->TranCode == TRAN_FORCE_POST))
	{


		if ( !pTRANDATA->bRequestedPIN)
		{
			TextLine(dc,CLanguage::GetText(CLanguage::IDX_RECEIPT_THANK_YOU21));
			TextLine(dc,CLanguage::GetText(CLanguage::IDX_RECEIPT_THANK_YOU22));
			//TextLine(dc,CLanguage::GetText(CLanguage::IDX_RECEIPT_THANK_YOU23));
			//TextLine(dc,CLanguage::GetText(CLanguage::IDX_RECEIPT_THANK_YOU24));
			//TextLine(dc,CLanguage::GetText(CLanguage::IDX_RECEIPT_THANK_YOU25));

			if ( m_iStatus == 1  && IsCvmLimit())
			{
				EmptyLine();
				EmptyLine();
				DrawLine(dc);
				TextLine(dc,CLanguage::GetText(CLanguage::IDX_RECEIPT_CARDHOLDER_SIGNATURE),0,TA_CENTER);
			}
		}

	}

	if( m_iStatus == 1 && (pTRANDATA->TranCode == TRAN_PURCHASE_VOID || pTRANDATA->TranCode == TRAN_REFUND_VOID))
	{
		str =  CLanguage::GetText(CLanguage::IDX_SIGNATURE_NOT_REQUIRED);
		TextLine(dc,str,0,TA_CENTER);
	}

	if ( bApproved && pTRANDATA->bRequestedPIN && !pTRANDATA->bReversal
		&& (pTRANDATA->TranCode == TRAN_PURCHASE
//		|| pTRANDATA->TranCode == TRAN_REFUND
		|| pTRANDATA->TranCode == TRAN_PREAUTH
		|| pTRANDATA->TranCode == TRAN_FORCE_POST))
	{
		TextLine(dc,CLanguage::GetText(CLanguage::IDX_RECEIPT_THANK_YOU11),L"");
		TextLine(dc,CLanguage::GetText(CLanguage::IDX_RECEIPT_THANK_YOU12),L"");
		TextLine(dc,CLanguage::GetText(CLanguage::IDX_RECEIPT_THANK_YOU13),L"");
		TextLine(dc,CLanguage::GetText(CLanguage::IDX_RECEIPT_THANK_YOU14),L"");
		TextLine(dc,CLanguage::GetText(CLanguage::IDX_RECEIPT_THANK_YOU15),L"");

//		str =  CLanguage::GetText(CLanguage::IDX_RECEIPT_VERIFIED_BY_PIN);
//		TextLine(dc,str,0,TA_CENTER);
	}
	EmptyLine();


	if ( m_iStatus == 1 )
		str =  CLanguage::GetText(CLanguage::IDX_RECEIPT_MERCHANT_COPY);
	else if ( m_iStatus== 2 )
		str =  CLanguage::GetText(CLanguage::IDX_RECEIPT_CUSTOMER_COPY);
	else 
		str = L"";

	if ( str.GetLength() > 0 )
		TextLine(dc,str,0,TA_CENTER);
		
	if( m_bCancelFlag )
		 str.Format( L"***%s***",CLanguage::GetText(CLanguage::IDX_RECEIPT_CANCELLED));
	else
		str = L"";

	if(str.GetLength()>0)
		TextLine(dc,str,0,TA_CENTER);

//	DrawLine(dc);

	PrintTrailer(dc);
	return 0;
}

DWORD CPrinter::PrintCredit()
{
	CString str = L"";
	CString str1 = L"";
	CString str2 = L"";
	USHORT value;

	m_iLine = 0;
	HDC dc;

	//4 to 7.Merchant name and Address.
	TCHAR buf[100]={L""};
	EmptyLine();
	PrintHeader(dc);
	EmptyLine();

	//Terminal ID

	TextLine(dc,CLanguage::GetText(CLanguage::IDX_RECEIPT_TERM),CString(m_CONFIG.TermID));
	TextLine(dc,CLanguage::GetText(CLanguage::IDX_RECEIPT_INVOICE),CString(pTRANDATA->InvoiceNo));

	//Clerk ID and Batch # 
	DWORD size = sizeof(USHORT);
	if (!CDataFile::Read((int)FID_TERM_BATCH_NUMBER, (BYTE *)&value, &size))
		value = 1;
	CString strTemp = CString(pTRANDATA->ClerkID);
	if (CDataFile::Read(L"CLERK ID", buf) && CString(buf) == L"Off")
		strTemp = L"";

	if(strTemp == L"")
	{
		str = L"";
	}
	else
	{
		str.Format(L"%s:%s",CLanguage::GetText(CLanguage::IDX_RECEIPT_CLERK),CString(pTRANDATA->ClerkID));
	}
	str1.Format(L"%s:%03d",CLanguage::GetText(CLanguage::IDX_RECEIPT_BATCH_ONE),value);
	TextLine(dc,str,str1);


	if(pTRANDATA->ComStatus == ST_OK && (pTRANDATA->TranStatus == ST_APPROVED ||  pTRANDATA->bReversal ))
		str.Format(L"%s:  %s",CLanguage::GetText(CLanguage::IDX_RECEIPT_AUTH),CString(pTRANDATA->AuthCode));
	else
		str = CLanguage::GetText(CLanguage::IDX_RECEIPT_AUTH);
	TextLine(dc,str);

	str2 = GetEntryName();
	str.Format(L"%s: %s",CLanguage::GetText(CLanguage::IDX_RECEIPT_TRANS),CString(pTRANDATA->SequenceNum));
	TextLine(dc,str);
	str = CLanguage::GetText(CLanguage::IDX_ENTRY_MODE);
	TextLine(dc,str,str2);

	DrawLine(dc);
	str1.Format(L"%s",CString(pTRANDATA->Account));
	for(int i=0;i<str1.GetLength()-4;i++)
		str1.SetAt(i,'*');
	str = CLanguage::GetText(CLanguage::IDX_RECEIPT_CARD);
	TextLine(dc,str,str1);

	str = GetAIDName(pTRANDATA->EmvData.AID);
	if( str == L"")
		str = GetServiceTypeName(pTRANDATA->ServType);

	if (pTRANDATA->EmvData.EmvState == EMV_STATE_ERROR)
	{
		if ( strlen( pTRANDATA->EmvData.AID) == 0)
			str1 = L"";
		else
			str1.Format(L"%s /",CEMV::IsInteracRID(pTRANDATA->EmvData.AID)?CLanguage::GetText(CLanguage::IDX_RECEIPT_DEBIT):CLanguage::GetText(CLanguage::IDX_RECEIPT_CREDIT));
	}
	else
		str1.Format(L"%s / %s",CLanguage::GetText(CLanguage::IDX_RECEIPT_CREDIT),str);  
	TextLine(dc,str1,L"");

	//str += _T("DATE          YYYY/MM/DD\n");Transaction Date
	str1 = CString(pTRANDATA->TranDate);
	str = CLanguage::GetText(CLanguage::IDX_RECEIPT_DATE);
	str2.Format(L"20%s/%s/%s",str1.Mid(0,2),str1.Mid(2,2),str1.Mid(4,2));
	TextLine(dc,str,str2);

	//str += _T("TIME            HH:MM:SS\n");Transaction Time
	str1 = CString(pTRANDATA->TranTime);
	str = CLanguage::GetText(CLanguage::IDX_RECEIPT_TIME);
	str2.Format(L"%s:%s:%s",str1.Mid(0,2),str1.Mid(2,2),str1.Mid(4,2));
	TextLine(dc,str,str2);

	//str += _T("RECEIPT     RRRRRRRRRRRR\n");The entered receipt number.
	str1 = CString(pTRANDATA->ReceiptNo);
	if (CDataFile::Read(L"RECEIPT", buf) && CString(buf) == L"Off")
		str1 = L"";

	if (str1 != L"")
	{
		str = CLanguage::GetText(CLanguage::IDX_RECEIPT_RECEIPT);
		TextLine(dc,str,str1);
	}

	DrawLine(dc);

	str1 = GetTransactionType();
	TextLine(dc,str1,0,TA_CENTER);

	str = CLanguage::GetText(CLanguage::IDX_RECEIPT_AMOUNT);
	str1.Format(L"$%s",DealAmount(pTRANDATA->Amount));
	TextLine(dc,str,str1);

	BOOL TotalAmountFlag = FALSE;


	if(strlen(pTRANDATA->TipAmount)>0 && (pTRANDATA->TranCode == TRAN_PURCHASE || pTRANDATA->TranCode == TRAN_PURCHASE_VOID || pTRANDATA->TranCode == TRAN_ADVICE))
	{
		str = CLanguage::GetText(CLanguage::IDX_RECEIPT_TIP);
		str1.Format(L"$%s",DealAmount(pTRANDATA->TipAmount));
		TextLine(dc,str,str1);
		TotalAmountFlag = TRUE;
	}

	if(TotalAmountFlag)
	{
		str = CLanguage::GetText(CLanguage::IDX_RECEIPT_TOTAL);
		str1.Format(L"$%s",DealAmount(pTRANDATA->TotalAmount));
		TextLine(dc,str,str1);
		m_iLine++;
	}

	if ( pTRANDATA->bPartialAuth )
	{
		TextLine(dc,CLanguage::GetText(CLanguage::IDX_RECEIPT_APPROVED),L"$"+CString(DealAmount(pTRANDATA->AuthAmount)));
		TextLine(dc,CLanguage::GetText(CLanguage::IDX_AMOUNT_DUE),L"$"+CString(DealAmount(pTRANDATA->DueAmount)));
		EmptyLine();
	}

	if ( strlen(pTRANDATA->AvailableBalance) > 0 &&  m_iStatus== 2)
	{
		str = pTRANDATA->AvailableBalance[0] == '-'? L"-": L"" ;
		TextLine(dc,CLanguage::GetText(CLanguage::IDX_AVAIL_BAL),str + L"$" + CString(DealAmount(&pTRANDATA->AvailableBalance[1])));
	}

	if ( pTRANDATA->TranCode == TRAN_PREAUTH && pTRANDATA->TranStatus == ST_APPROVED)
	{
		CDataFile::Read(L"TIP PROMPT", buf);
		
		if ( CString(buf) != L"Off")
		{
			EmptyLine();
			TextLine(dc,CLanguage::GetText(CLanguage::IDX_RECEIPT_TIP),L"---------------------");
			EmptyLine();
			TextLine(dc,CLanguage::GetText(CLanguage::IDX_RECEIPT_TOTAL),L"---------------------");
		}
	}

	PrintEMVData(dc);

	EmptyLine();

	str = L"";

	DrawLine(dc);

	str= CLanguage::GetText(CLanguage::IDX_RECEIPT_TRANSACTION);//_T("TRANSACTION");
	TextLine(dc,str,0,TA_CENTER);

	BOOL bApproved = FALSE;
	str1 = CString(pTRANDATA->ISORespCode);
	if(pTRANDATA->ComStatus == ST_OK)
	{
		if(pTRANDATA->TranStatus == ST_APPROVED)
		{
			if (pTRANDATA->bPartialAuth)
				str.Format( _T("%s–%s–%s"),str1,CLanguage::GetText(CLanguage::IDX_PARTIAL_APPROVED),CString(pTRANDATA->RespCode));
			else
				str.Format( _T(" %s – %s – %s"),str1,CLanguage::GetText(CLanguage::IDX_RECEIPT_APPROVED),CString(pTRANDATA->RespCode));
			bApproved = TRUE;
		}
		else if (pTRANDATA->bReversal)
			str.Format( _T(" – %s – "),CLanguage::GetText(CLanguage::IDX_DECLINED));
		else
		{
			if (str1 == L"00")
				str1 = L"";
			str2 = CString(pTRANDATA->RespCode);
			if (str2 == L"001")
				str2 = L"";
			str.Format( _T(" %s – %s – %s"),str1,CLanguage::GetText(CLanguage::IDX_RECEIPT_NOT_APPROVED),str2);
		}
	}
	else
	{
		if ( m_declinedOffline )
			str.Format( _T(" – %s – "),CLanguage::GetText(CLanguage::IDX_DECLINED));
		else
			str.Format( _T(" – %s – "),CLanguage::GetText(CLanguage::IDX_RECEIPT_NOT_COMPLETED));
	}

	TextLine(dc,str,0,TA_CENTER);
	EmptyLine();

	if ( pTRANDATA->EntryMode == ENTRY_MODE_CHIP  //chip card
			&& strcmp(pTRANDATA->ServType,"M")==0 //Master Card
			&& pTRANDATA->bEmvTransaction)        // EMV
	{

		if( pTRANDATA->TranStatus == ST_APPROVED && ( pTRANDATA->EmvError != EMV_NO_ERROR ))
		{
			str = CLanguage::GetText(CLanguage::IDX_RECEIPT_CHIP_CARD_MALFUNCTION); //L"CHIP CARD MALFUNCTION";
		}
	}
	EmptyLine();

	if ( bApproved &&// m_iStatus == 1 && 
		(pTRANDATA->TranCode == TRAN_PURCHASE
//		|| pTRANDATA->TranCode == TRAN_REFUND
		|| pTRANDATA->TranCode == TRAN_PREAUTH
		|| pTRANDATA->TranCode == TRAN_FORCE_POST))
	{
		if ( !pTRANDATA->bRequestedPIN)
		{
			TextLine(dc,CLanguage::GetText(CLanguage::IDX_RECEIPT_THANK_YOU1),L"");
			TextLine(dc,CLanguage::GetText(CLanguage::IDX_RECEIPT_THANK_YOU2),L"");
			TextLine(dc,CLanguage::GetText(CLanguage::IDX_RECEIPT_THANK_YOU3),L"");
			TextLine(dc,CLanguage::GetText(CLanguage::IDX_RECEIPT_THANK_YOU4),L"");
			TextLine(dc,CLanguage::GetText(CLanguage::IDX_RECEIPT_THANK_YOU5),L"");

			if ( m_iStatus == 1  && IsCvmLimit())
			{
				EmptyLine();
				EmptyLine();
				DrawLine(dc);
				TextLine(dc,CLanguage::GetText(CLanguage::IDX_RECEIPT_CARDHOLDER_SIGNATURE),0,TA_CENTER);
			}
		}
	}

	if( m_iStatus == 1 && (pTRANDATA->TranCode == TRAN_PURCHASE_VOID || pTRANDATA->TranCode == TRAN_REFUND_VOID))
	{
		str =  CLanguage::GetText(CLanguage::IDX_SIGNATURE_NOT_REQUIRED);
		TextLine(dc,str,0,TA_CENTER);
	}

	if ( bApproved && pTRANDATA->bRequestedPIN && !pTRANDATA->bReversal
		&& (pTRANDATA->TranCode == TRAN_PURCHASE
//		|| pTRANDATA->TranCode == TRAN_REFUND
		|| pTRANDATA->TranCode == TRAN_PREAUTH
		|| pTRANDATA->TranCode == TRAN_FORCE_POST))
	{
		TextLine(dc,CLanguage::GetText(CLanguage::IDX_RECEIPT_THANK_YOU11),L"");
		TextLine(dc,CLanguage::GetText(CLanguage::IDX_RECEIPT_THANK_YOU12),L"");
		TextLine(dc,CLanguage::GetText(CLanguage::IDX_RECEIPT_THANK_YOU13),L"");
		TextLine(dc,CLanguage::GetText(CLanguage::IDX_RECEIPT_THANK_YOU14),L"");
		TextLine(dc,CLanguage::GetText(CLanguage::IDX_RECEIPT_THANK_YOU15),L"");

	}
	EmptyLine();

	if ( m_iStatus == 1 )
		str =  CLanguage::GetText(CLanguage::IDX_RECEIPT_MERCHANT_COPY);
	else if ( m_iStatus== 2 )
		str =  CLanguage::GetText(CLanguage::IDX_RECEIPT_CUSTOMER_COPY);
	else 
		str = L"";

	if ( str.GetLength() > 0 )
		TextLine(dc,str,0,TA_CENTER);
		
	if( m_bCancelFlag )
		 str.Format( L"***%s***",CLanguage::GetText(CLanguage::IDX_RECEIPT_CANCELLED));
	else
		str = L"";

	if(str.GetLength()>0)
		TextLine(dc,str,0,TA_CENTER);

	PrintTrailer(dc);
	return 0;
}

//-----------------------------------------------------------------------------
//!	\print thread entry
//-----------------------------------------------------------------------------
void  CPrinter::Exec(TRAN_TranDataRec *pTranReq,HWND hWnd,int status,BOOL bTrain)
{
	pTRANDATA = pTranReq;
	m_hWnd = hWnd;
	m_iStatus = status;
	m_bTraining = bTrain;

	HANDLE hThread = CreateThread(NULL, 0,ThreadProc, (PVOID)this, 0, NULL);
	CloseHandle(hThread);
}

//-----------------------------------------------------------------------------
//!	\print the same receipt with txn data
//!	\param	pTranReq - txn data buffer point
//-----------------------------------------------------------------------------
void  CPrinter::GetReceipt(TRAN_TranDataRec *pTranReq, BYTE *RecType)				//JC May 13/15 Add Receipt Type
{
	pTRANDATA = pTranReq;
	m_hWnd = NULL;
	m_iStatus = 2;										//JC May 13/15 Cardholder Receipt 1=Merchant Receipt
	if (RecType[0] == 'M') m_iStatus = 1;				//JC May 13/15 Cardholder Receipt 1=Merchant Receipt
	m_bTraining = FALSE;
	CLanguage::SetLanguage(pTRANDATA->CustLang);
	Printing();
}

//-----------------------------------------------------------------------------
//!	\Deal Amount format
//!	\param	p - amount string data buffer point
//-----------------------------------------------------------------------------
CString CPrinter::DealAmount(char* p)
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

//-----------------------------------------------------------------------------
//!	\run print thread
//-----------------------------------------------------------------------------
DWORD WINAPI CPrinter::ThreadProc (LPVOID lpArg)
{
	CPrinter* p = reinterpret_cast<CPrinter*>(lpArg);
	p->Printing();
	return 0;
}

//-----------------------------------------------------------------------------
//!	\Print receipt header strings
//!	\param	dc - HDC id 
//-----------------------------------------------------------------------------
void CPrinter::PrintHeader(HDC& dc)
{
	char buf[100]={""};
	CString str,str1;

	DWORD size;
	CDataFile::Read(FID_MERCH_CFG_NAME, (BYTE*)buf, &size);
	str = CString(buf);
	if(str.GetLength()>0)
		TextLine(dc,str,0,TA_CENTER);

	CDataFile::Read(FID_MERCH_CFG_ADDRESS, (BYTE*)buf, &size);
	str = CString(buf);
	if(str.GetLength()>0)
		TextLine(dc,str,0,TA_CENTER);

	CDataFile::Read(FID_MERCH_CFG_CITY_PROV, (BYTE*)buf, &size);
	str = CString(buf);
	if(str.GetLength()>0)
		TextLine(dc,str,0,TA_CENTER);

}

//-----------------------------------------------------------------------------
//!	\Pirnt receipt trailer strings
//!	\param	dc - HDC id 
//-----------------------------------------------------------------------------
void CPrinter::PrintTrailer(HDC& dc)
{
	if( m_iStatus != 2) // for customer copy
		return;

	TCHAR buf[100]={L""};
	BOOL bTrailerShow = TRUE;
		
	if(CDataFile::Read(L"TRAILER MSG",buf) && CString(buf) == L"Off")
	  bTrailerShow = FALSE;

	EmptyLine();

	CString str,str1;
	if ( bTrailerShow )
	{
		for(int i = 0;i<4;i++)
		{
			memset(buf,0,100);
			str.Format(L"TrailerLine%d",i+1);

			if(CDataFile::Read(str,buf))
				str1.Format(L"%s",buf);
			if(str1.GetLength()>0)
				TextLine(dc,str1);
		}
	}
}

//-----------------------------------------------------------------------------
//!	\Print EMV data
//!	\param	dc - HDC id 
//-----------------------------------------------------------------------------
void CPrinter::PrintEMVData(HDC& dc)
{
	if ( !pTRANDATA->bEmvTransaction )
		return;

	if ( strlen( pTRANDATA->EmvData.AID) == 0)
		return;

	CString str = L"";
	CString str1= L"";
	DrawLine(dc);

	str = CString(pTRANDATA->EmvData.ApplicationLabel);
	str1 = CString(pTRANDATA->EmvData.ApplicationPreferredName);

	if ( str1.GetLength() > 0 )
		TextLine(dc,str1);
	else if(  str.GetLength() > 0 )
		TextLine(dc,str);
	else
	{
		str = CString(pTRANDATA->ServType);
		TextLine(dc,str);
	}

	if (strlen(pTRANDATA->EmvData.ICC_AID) > 0)
		str = CString(pTRANDATA->EmvData.ICC_AID);
	else
		str = CString(pTRANDATA->EmvData.AID);

	if(str.GetLength() > 0)	
	{
		str1 = L"AID:"+str;
		TextLine(dc,str1,L"");
	}
	else 
	{
		if (pTRANDATA->EmvData.DedicatedFilenameLen >= 7)
		{
			str1 = L"AID:"+ ByteToStr(pTRANDATA->EmvData.DedicatedFilename,pTRANDATA->EmvData.DedicatedFilenameLen);
			TextLine(dc,str1);
		}
	}
	if ( pTRANDATA->CardType != CARD_DEBIT &&
		(pTRANDATA->TranCode == TRAN_REFUND || pTRANDATA->TranCode == TRAN_REFUND_VOID))
		return;

	BOOL bTemp = FALSE;
	str = L"";
	for( int i = 0;i < 5;i++)
	{
		str1.Format(L"%02X",pTRANDATA->EmvData.TerminalVerificationResultsARQC[i]);
		str += str1+L" ";
	}

	str1 = L"TVR: "+str;
	TextLine(dc,str1);

	if (pTRANDATA->EntryMode == ENTRY_MODE_CTLS)
		return;
	bTemp = FALSE;
	str = L"";
	for( int i = 0;i < 2;i++)
	{
		str1.Format(L"%02X",pTRANDATA->EmvData.TransactionStatusInfo[i]);
		str += str1+L" ";
	}

	str1 = L"TSI: "+str;
	TextLine(dc,str1);

	return;
	str = L"";
	bTemp = FALSE;
	for( int i = 0;i < 8;i++)
	{
		if (pTRANDATA->bEmvCtlsTransaction)
			str1.Format(L"%02X",pTRANDATA->EmvData.ApplicationCryptogramForRequest[i]);
		else
			str1.Format(L"%02X",pTRANDATA->EmvData.ApplicationCryptogramForResponse[i]);
		if ( str1 != L"00")
			bTemp = TRUE;
		str += str1;
	}
	if ( bTemp )
		TextLine(dc,str);

	bTemp = FALSE;
	str = L"";
	for( int i = 0;i < 5;i++)
	{
		if (pTRANDATA->bEmvCtlsTransaction)
			str1.Format(L"%02X",pTRANDATA->EmvData.TerminalVerificationResultsARQC[i]);
		else
			str1.Format(L"%02X",pTRANDATA->EmvData.TerminalVerificationResultsTC[i]);
		
		if ( str1 != L"00")
			bTemp = TRUE;
		str += str1;
	}
	if ( bTemp )
		TextLine(dc,str);
}

//-----------------------------------------------------------------------------
//!	\Get card entry name
//-----------------------------------------------------------------------------
CString CPrinter::GetEntryName()
{
	CString str2 = L"SWIPE";
	if (pTRANDATA->bCtlsTransaction)
	{
		if (pTRANDATA->bEmvCtlsTransaction)
			str2 = L"TAPPED";
		else
			str2 = L"TAPPED";
	}
	else if (pTRANDATA->bFallback)
	{
		if( pTRANDATA->EntryMode == ENTRY_MODE_MANUAL)
			str2 = L"CHIP/MANUAL";
		else
			str2 = L"CHIP/MAG";
	}
	else if (pTRANDATA->EntryMode == ENTRY_MODE_MANUAL)
		str2 = L"MANUAL";
	else if (pTRANDATA->EntryMode == ENTRY_MODE_SWIPED)
		str2 = L"SWIPE";
	else if (pTRANDATA->EntryMode == ENTRY_MODE_CHIP)
		str2 = L"CHIP";
	else if (pTRANDATA->EntryMode == ENTRY_MODE_CTLS)
		str2 = L"TAPPED";

	return str2;

}
//-----------------------------------------------------------------------------
//!	\get transaction type
//-----------------------------------------------------------------------------
CString CPrinter::GetTransactionType()
{
	CString str1 = L"";
	switch(pTRANDATA->TranCode)
	{
	case TRAN_PURCHASE_VOID:
		str1 = CLanguage::GetText(CLanguage::IDX_RECEIPT_SALE_VOID);
		break;
	case TRAN_REFUND:
		str1 = CLanguage::GetText(CLanguage::IDX_RECEIPT_REFUND);
		break;
	case TRAN_REFUND_VOID:
		str1 = CLanguage::GetText(CLanguage::IDX_RECEIPT_REFUND_VOID);
		break;
	case TRAN_PREAUTH:
		str1 = CLanguage::GetText(CLanguage::IDX_RECEIPT_PREAUTH);
		break;
	case TRAN_ADVICE:
		str1 = CLanguage::GetText(CLanguage::IDX_RECEIPT_COMPLETION);
		break;
	case TRAN_FORCE_POST:
		str1 = CLanguage::GetText(CLanguage::IDX_RECEIPT_FORCE_POST);
		break;
	default:
		str1 = CLanguage::GetText(CLanguage::IDX_RECEIPT_SALE);
		break;
	}
	return str1;
}

//-----------------------------------------------------------------------------
//!	\Get Service Type Name
//!	\param	type - service type 
//-----------------------------------------------------------------------------
CString CPrinter::GetServiceTypeName(char* type)
{
	CString str;
	if ( type[0] == 'V') 
		str = CLanguage::GetText(CLanguage::IDX_VISA);
	else if ( type[0] == 'M') 
		str = CLanguage::GetText(CLanguage::IDX_MASTERCARD);
	else if (strcmp(type,"AX") == 0)
		str = CLanguage::GetText(CLanguage::IDX_AMERICAN_EXPRESS);
	else if (strcmp(type,"DS") == 0)
		str = CLanguage::GetText(CLanguage::IDX_DISCOVER);
	else if (strcmp(type,"JC") == 0)
		str = CLanguage::GetText(CLanguage::IDX_JCB);
	else if (strcmp(type,"DC") == 0)
		str = CLanguage::GetText(CLanguage::IDX_DINERS_CLUB);
	else if (strcmp(type,"P0") == 0)
		str = CLanguage::GetText(CLanguage::IDX_PROPRIETARY_DEBIT);
	else
		str = CString(type);

	return str;
}

//-----------------------------------------------------------------------------
//!	\Get EMV AID Name
//!	\param	AID - EMV AID 
//-----------------------------------------------------------------------------
CString CPrinter::GetAIDName(char* AID)
{
	CString str = L"";

	if (CEMV::IsMasterCardRID(AID))	// MChip
	{
		str = CLanguage::GetText(CLanguage::IDX_MASTERCARD);
	}
	else if (CEMV::IsVisaRID(AID))	// qVsdc
	{
		str = CLanguage::GetText(CLanguage::IDX_VISA);
	}
	else if (CEMV::IsAmexRID(AID))
	{
		str = CLanguage::GetText(CLanguage::IDX_AMERICAN_EXPRESS);
	}
	else if (CEMV::IsInteracRID(AID))
	{
		str = CLanguage::GetText(CLanguage::IDX_PROPRIETARY_DEBIT);
	}
	return str;
}

//-----------------------------------------------------------------------------
//!	\Check CVM limit
//-----------------------------------------------------------------------------
BOOL CPrinter::IsCvmLimit()
{
	if (pTRANDATA->EntryMode != ENTRY_MODE_CTLS && pTRANDATA->EntryMode !=ENTRY_MODE_CTLS_MSR )
		return TRUE;

	if (CAppData::bSignatureRequired)
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
		if(pCtlsData->serviceType[1] == ' ')
			pCtlsData->serviceType[1] = 0;

		if( strcmp(pCtlsData->serviceType,pTRANDATA->ServType) == 0)
		{
			CString str = ByteToStr(pCtlsData->ctlsCvmLimit,SZ_BIN_CTLS_EMV_CVM_LIMIT);
			CvmAmount = atoi(alloc_tchar_to_char(str.GetBuffer(str.GetLength())));
			str.ReleaseBuffer();
			break;
		}
	}
	ctlsDataFile.CloseFile(&fhCtlsData);

	if ( CvmAmount == 0)
		return TRUE;

	if ( pTRANDATA->ServType[0] == 'M')
		return CvmAmount < atoi(pTRANDATA->TotalAmount);
	return CvmAmount <= atoi(pTRANDATA->TotalAmount);
}

//-----------------------------------------------------------------------------
//!	\change Byte buffer to string
//!	\param	bBuf - BYTE* data 
//!	\param	Len - bBuf size 
//!	\param	format - string format 
//-----------------------------------------------------------------------------
CString CPrinter::ByteToStr(BYTE *bBuf, int Len,int format)
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

