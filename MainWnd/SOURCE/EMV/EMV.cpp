#include "StdAfx.h"
#include "Common.h"
#include "..\\defs\\constant.h"
#include "..\\utils\\StrUtil.h"
#include "EMVFlow.h"
#include "EMV.h"
#include "..\\data\\DataFile.h"

CEMV::CEMV(void)
{
}

CEMV::~CEMV(void)
{
}

BOOL CEMV::Init()
{
 return InitEmv();
}

//----------------------------------------------------------------------------
//!	\brief	EMV start
//!	\param	txnType - txn amount type
//!	\param	amount - txn amount data
//!	\param	p - point to txn data
//----------------------------------------------------------------------------
void CEMV::Start(int txnType, long amount, void *p)
{
	int txnSn;
	DWORD size=sizeof(int);
	
	// Get Txn SN
	if (!CDataFile::Read(FID_EMV_TRANSACTION_SN, (BYTE *) &txnSn, &size))
	{
		txnSn = 0;
	}
	txnSn++;

	CDataFile::Save(FID_EMV_TRANSACTION_SN, (BYTE *) &txnSn, size);

	TXNCONDITION txn;

	// BCD amount
	STR_LongToBCDString(amount, 6, txn.byTransAmount);	
	// Txn Type - 9C
	txn.wTxnType = EMV_TRANSTYPE_GOODS;	
	// Account Type - 5F57
	txn.byAccountType = AccountType_Default;
	// txnSn - 9F41
	txn.wTxnSN = (WORD) txnSn;
}

//----------------------------------------------------------------------------
//!	\brief	check Is Interac RID
//!	\param	RID - point to RID data
//! RETURNS:       TRUE    = yes                              
//!                FALSE    = no.
//----------------------------------------------------------------------------
BOOL CEMV::IsInteracRID(char *RID)
{
	if (memcmp(RID, RID_INTERAC, 10) == 0)
		return TRUE;
	else
		return FALSE;
}

//----------------------------------------------------------------------------
//!	\brief	check Is master card RID
//!	\param	RID - point to RID data
//! RETURNS:       TRUE    = yes                              
//!                FALSE    = no.
//----------------------------------------------------------------------------
BOOL CEMV::IsMasterCardRID(char *RID)
{
	if (memcmp(RID, RID_MASTER_CARD, 10) == 0)
		return TRUE;
	else
		return FALSE;
}

//----------------------------------------------------------------------------
//!	\brief	check Is visa RID
//!	\param	RID - point to RID data
//! RETURNS:       TRUE    = yes                              
//!                FALSE    = no.
//----------------------------------------------------------------------------
BOOL CEMV::IsVisaRID(char *RID)
{
	if (memcmp(RID, RID_VISA_CARD, 10) == 0)
		return TRUE;
	else
		return FALSE;
}

//----------------------------------------------------------------------------
//!	\brief	check Is maestro AID
//!	\param	AID - point to AID data
//! RETURNS:       TRUE    = yes                              
//!                FALSE    = no.
//----------------------------------------------------------------------------
BOOL CEMV::IsMaestroAID(char *AID)
{
	if (memcmp(AID, AID_MAESTRO, 14) == 0)
		return TRUE;
	else
		return FALSE;
}

//----------------------------------------------------------------------------
//!	\brief	check Is master card AID
//!	\param	AID - point to AID data
//! RETURNS:       TRUE    = yes                              
//!                FALSE    = no.
//----------------------------------------------------------------------------
BOOL CEMV::IsMasterCardAID(char *AID)
{
	if (memcmp(AID, AID_MASTERCARD, 14) == 0)
		return TRUE;
	else
		return FALSE;
}

//----------------------------------------------------------------------------
//!	\brief	check Is visa AID
//!	\param	AID - point to AID data
//! RETURNS:       TRUE    = yes                              
//!                FALSE    = no.
//----------------------------------------------------------------------------
BOOL CEMV::IsVisaAID(char *AID)
{
	if (memcmp(AID, AID_VISA, 14) == 0)
		return TRUE;
	else
		return FALSE;
}

//----------------------------------------------------------------------------
//!	\brief	check Is visa debit AID
//!	\param	AID - point to AID data
//! RETURNS:       TRUE    = yes                              
//!                FALSE    = no.
//----------------------------------------------------------------------------
BOOL CEMV::IsVisaDebitAID(char *AID)
{
	if (memcmp(AID, AID_VISA_DEBIT, 14) == 0)
		return TRUE;
	else
		return FALSE;
}
//----------------------------------------------------------------------------
//!	\brief	check Is amex AID
//!	\param	AID - point to AID data
//! RETURNS:       TRUE    = yes                              
//!                FALSE    = no.
//----------------------------------------------------------------------------
BOOL CEMV::IsAmexRID(char *RID)
{
  if (memcmp(RID, RID_AMEX_CARD, 10) == 0)
    return TRUE;
  else
    return FALSE;
}
//----------------------------------------------------------------------------
//!	\brief	check Is JCB RID
//!	\param	RID - point to RID data
//! RETURNS:       TRUE    = yes                              
//!                FALSE    = no.
//----------------------------------------------------------------------------
BOOL CEMV::IsJcbRID(char *RID)
{
  if (memcmp(RID, RID_JCB_CARD, 10) == 0)
    return TRUE;
  else
    return FALSE;
}
