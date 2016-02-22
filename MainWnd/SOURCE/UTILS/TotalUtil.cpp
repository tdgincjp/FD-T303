#include "stdafx.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "..\\defs\\constant.h"
#include "..\\data\\DataFile.h"
#include "..\\data\\RecordFile.h"

#include "totalutil.h"

#pragma warning(disable: 4996)

#define   SERVICE_TYPE_OFF  offsetof(sCardTotalRecord, serviceType)

static void TOT_GetGrandTotals(char *fileName, sCardTotalRecord *pTotal);
static BOOL CompareTotals(sCardTotalRecord *pTotal1,
                          sCardTotalRecord *pTotal2);

/******************************************************************************/
static BOOL CompareTotals(sCardTotalRecord *pTotal1,
                          sCardTotalRecord *pTotal2)
{
  BOOL balance = TRUE;
  BYTE n;

  if (pTotal1)
  {
    for (n=0; n<NUMBER_TRAN_TOTALS; n++)
    {
      if (pTotal1->total[n].count == pTotal2->total[n].count &&
           pTotal1->total[n].amount == pTotal2->total[n].amount)
      {
        pTotal1->total[n].flag = pTotal2->total[n].flag = TOT_BALANCED;
      }
      else
      {
        pTotal1->total[n].flag = pTotal2->total[n].flag = TOT_NOT_BALANCED;
        balance = FALSE;
      }
    }
  }
  else
  {
    for (n=0; n<NUMBER_TRAN_TOTALS; n++)
    {
      if (pTotal2->total[n].count == 0 && pTotal2->total[n].amount == 0)
      {
        pTotal2->total[n].flag = TOT_BALANCED;
      }
      else
      {
        pTotal2->total[n].flag = TOT_NOT_BALANCED;
        balance = FALSE;
      }
    }
  }
  return balance;
}

/******************************************************************************/
static void UpdateTotalRecord(char* serviceType, USHORT tranType, long amount)
{
  AP_FILE_HANDLER fh;
	CRecordFile totalFile;
  sCardTotalRecord total;

//	assert(totalFile.Open(TOTALS_FILE, &fh));
  if (!totalFile.Open(CDataFile::m_bTraining?TOTALS_FILE_TRAIN:TOTALS_FILE, &fh))
		return;

  if (totalFile.FindRecord(&fh, TRUE, (BYTE*)serviceType, SERVICE_TYPE_OFF, 2, 0, 0) < 0)
  {
    memset(&total, 0, sizeof(total));
    memcpy(total.serviceType, serviceType, 2);
    TOT_UpdateTranTotal(&total, tranType, amount);
    totalFile.AddRecord(&fh, (BYTE*)&total, sizeof(sCardTotalRecord));
  }
  else
  {
    memcpy(&total, fh.currentRecord+sizeof(USHORT), sizeof(total));
    TOT_UpdateTranTotal(&total, tranType, amount);
    totalFile.ReplaceRecord(&fh, (BYTE*)&total, sizeof(sCardTotalRecord));
  }
  totalFile.CloseFile(&fh);
}


/******************************************************************************/
/*                                                                            */
/* RTN NAME:   TOT_CreateEmptyTotalsFile																			*/
/*                                                                            */
/* FUNCTION:   Create an empty total record file                              */
/*                                                                            */
/* ARGS:                                                                      */
/*                                                                            */
/* RETURNS:																																		*/
/*                                                                            */
/* NOTES:                                                                     */
/*                                                                            */
/******************************************************************************/
void TOT_CreateEmptyTotalsFile(USHORT maxNumberRecords)
{
	CRecordFile totalFile;
	
	totalFile.Delete(CDataFile::m_bTraining?TOTALS_FILE_TRAIN:TOTALS_FILE);
	totalFile.Create(CDataFile::m_bTraining?TOTALS_FILE_TRAIN:TOTALS_FILE,
													sizeof(sCardTotalRecord),
													maxNumberRecords);
}

/******************************************************************************/
/*                                                                            */
/* RTN NAME:   TOT_UpdateTranTotalsFile																				*/
/*                                                                            */
/* FUNCTION:   Update card transsaction totals file                           */
/*                                                                            */
/* ARGS:                                                                      */
/*                                                                            */
/* RETURNS:																																		*/
/*                                                                            */
/* NOTES:                                                                     */
/*                                                                            */
/******************************************************************************/
void TOT_UpdateTranTotalsFile(int tranType, char *servType, long amount)
{
  AP_FILE_HANDLER fh;
	CRecordFile totalFile;

  if (!totalFile.Open(CDataFile::m_bTraining?TOTALS_FILE_TRAIN:TOTALS_FILE, &fh))
  {
		TOT_CreateEmptyTotalsFile(MAX_NUMBER_CARD_TOTALS);
		totalFile.Open(CDataFile::m_bTraining?TOTALS_FILE_TRAIN:TOTALS_FILE, &fh);
  }
//  assert(fh.finfo.recordLength == sizeof(sCardTotalRecord)+sizeof(USHORT));
  if (fh.finfo.recordLength != sizeof(sCardTotalRecord)+sizeof(USHORT))
	  return;

	totalFile.CloseFile(&fh);
	
  UpdateTotalRecord(servType, tranType, amount);
}

/******************************************************************************/
/*                                                                            */
/* RTN NAME:   TOT_UpdateTranTotal																						*/
/*                                                                            */
/* FUNCTION:   Update a transaction total                                     */
/*                                                                            */
/* ARGS:                                                                      */
/*                                                                            */
/* RETURNS:																																		*/
/*                                                                            */
/* NOTES:                                                                     */
/*                                                                            */
/******************************************************************************/
void TOT_UpdateTranTotal(sCardTotalRecord *pTotal, int tranType, long amount)
{
  eTranTotalType totalType=TOT_SALE;

  switch (tranType)
  {
    case TRAN_PURCHASE:
    case TRAN_ADVICE:
    case TRAN_FORCE_POST:
      totalType = TOT_SALE;
      break;
    case TRAN_REFUND:
      totalType = TOT_REFUND;
      break;
    case TRAN_REFUND_VOID:
      amount = -amount;
    case TRAN_PURCHASE_VOID:
      totalType = TOT_CORRECTION;
      break;
    case TRAN_PREAUTH:
    case TRAN_CLOSE_BATCH:
	case TRAN_BATCH_TOTAL:
      return;
	default:
		return;
 //     assert(FALSE);
  }
  pTotal->total[totalType].count++;
  pTotal->total[totalType].amount += amount;
}

/******************************************************************************/
/*                                                                            */
/* RTN NAME:   TOT_GetGrandTotals																							*/
/*                                                                            */
/* FUNCTION:   Calculate grand totals                                         */
/*                                                                            */
/* ARGS:                                                                      */
/*                                                                            */
/* RETURNS:																																		*/
/*                                                                            */
/* NOTES:                                                                     */
/*                                                                            */
/******************************************************************************/
static void TOT_GetGrandTotals(char *fileName, sCardTotalRecord *pTotal)
{
  AP_FILE_HANDLER fh;
  USHORT recordIndex, totalIndex;
  sCardTotalRecord *pCurr;
	CRecordFile totalFile;

  memset(pTotal, 0, sizeof(sCardTotalRecord));
  if (!totalFile.Open(fileName, &fh))
  {
    return;
  }

//  assert(fh.finfo.recordLength == sizeof(sCardTotalRecord));
  if (fh.finfo.recordLength != sizeof(sCardTotalRecord))
	  return;

  for (recordIndex=0; recordIndex<fh.finfo.numberOfActiveRecords; recordIndex++)
  {
    pCurr = (sCardTotalRecord*)fh.currentRecord;
    for (totalIndex=0; totalIndex<NUMBER_TRAN_TOTALS; totalIndex++)
    {
      pTotal->total[totalIndex].count += pCurr->total[totalIndex].count;
      pTotal->total[totalIndex].amount += pCurr->total[totalIndex].amount;
    }
    totalFile.NextRecord(&fh);
  }
  totalFile.CloseFile(&fh);
}

/******************************************************************************/
/*                                                                            */
/* RTN NAME:   TOT_CompareTermHostTotals																			*/
/*                                                                            */
/* FUNCTION:   Compare two totals files, set balance flag in host total file  */
/*             and add grand totals to host totals file                       */
/* ARGS:                                                                      */
/*                                                                            */
/* RETURNS:																																		*/
/*                                                                            */
/* NOTES:                                                                     */
/*                                                                            */
/******************************************************************************/
BOOL TOT_CompareTermHostTotals()
{
  AP_FILE_HANDLER fhTerm, fhHost;
  BOOL balanced, bResult;
  sCardTotalRecord termGrandTotal, hostGrandTotal, *pHostTotal, *pTermTotal;
	CRecordFile termFile, hostFile;
  USHORT index;

  balanced = TRUE;

  TOT_GetGrandTotals(CDataFile::m_bTraining?TOTALS_FILE_TRAIN:TOTALS_FILE, &termGrandTotal);
  TOT_GetGrandTotals(CDataFile::m_bTraining?TOTALS_FILE_TRAIN:HOST_TOTALS_FILE, &hostGrandTotal);

//  assert(termFile.Open(TOTALS_FILE, &fhTerm));
  if (!termFile.Open(CDataFile::m_bTraining?TOTALS_FILE_TRAIN:TOTALS_FILE, &fhTerm))
	  return FALSE;

//  assert(hostFile.Open(HOST_TOTALS_FILE, &fhHost));
  if (!hostFile.Open(CDataFile::m_bTraining?TOTALS_FILE_TRAIN:HOST_TOTALS_FILE, &fhHost))
	  return FALSE;
 
//  assert(fhHost.finfo.recordLength == sizeof(sCardTotalRecord));
  if (fhHost.finfo.recordLength != fhTerm.finfo.recordLength)
	  return FALSE;

  for (index=0; index<fhHost.finfo.numberOfActiveRecords; index++)
  {
    hostFile.FirstRecord(&fhHost);
    pHostTotal = (sCardTotalRecord*)fhHost.currentRecord;
    if (termFile.FindRecord(&fhTerm, TRUE, (BYTE*)pHostTotal->serviceType,
                         SERVICE_TYPE_OFF, 2, 0, 0) >= 0)
    {
      pTermTotal = (sCardTotalRecord*)fhTerm.currentRecord;
      bResult = CompareTotals(pTermTotal, pHostTotal);
    }
    else
    {
      bResult = CompareTotals(NULL, pHostTotal);
    }
    if (!bResult)
    {
      balanced = FALSE;
    }
    hostFile.ReplaceRecord(&fhHost, (BYTE*)pHostTotal, 0);
    hostFile.NextRecord(&fhHost);
  }

  bResult = CompareTotals(&termGrandTotal, &hostGrandTotal);
//TDG JP  memcpy(hostGrandTotal.serviceType, GRAND_TOTAL_SERViCE_TYPE, 2);
  hostFile.AddRecord(&fhHost, (BYTE*)&hostGrandTotal, 0);
  if (!bResult)
  {
    balanced = FALSE;
  }

  termFile.CloseFile(&fhTerm);
  hostFile.CloseFile(&fhHost);
  return balanced;
}


/******************************************************************************/
/*                                                                            */
/* RTN NAME:   TOT_AddCardTotalsRecords																				*/
/*                                                                            */
/* FUNCTION:   add card totals records to file                                */
/* ARGS:                                                                      */
/*                                                                            */
/* RETURNS:																																		*/
/*                                                                            */
/* NOTES:                                                                     */
/*                                                                            */
/******************************************************************************/
void TOT_AddCardTotalsRecords(USHORT numberOfRecords,
                              sCardTotalRecord *pTotal)
{
  AP_FILE_HANDLER fh;
  USHORT n;
	CRecordFile totalFile;
	
//  assert(totalFile.Open(TOTALS_FILE, &fh));
//  assert(fh.finfo.recordLength == sizeof(sCardTotalRecord));
  if (!totalFile.Open(CDataFile::m_bTraining?TOTALS_FILE_TRAIN:TOTALS_FILE, &fh))
	  return;
  if (fh.finfo.recordLength != sizeof(sCardTotalRecord))
	  return;

  for (n=0; n<numberOfRecords; n++, pTotal++)
  {
    totalFile.AddRecord(&fh, (BYTE*)pTotal, 0);
  }
  totalFile.CloseFile(&fh);
}

BOOL TOT_IsBatchOpen()
{
	CRecordFile file;
    AP_FILE_HANDLER fh;
	if (!file.Open(CDataFile::m_bTraining?TOTALS_FILE_TRAIN:TOTALS_FILE, &fh))
	{
		TRACE(L"Can't open file!\n");
		return FALSE;
	}

	if( fh.finfo.numberOfActiveRecords == 0)
		return FALSE;

	CString str,str1;
	sCardTotalRecord* pTotal;

    file.FirstRecord(&fh);
	for (int index=0; index<fh.finfo.numberOfActiveRecords; index++)
	{
		sCardTotalRecord temp;
		pTotal = &temp;
		memcpy((void*)pTotal,(void*)(fh.currentRecord+sizeof(USHORT)),sizeof(sCardTotalRecord));

		if ( (pTotal->total[0].count+pTotal->total[1].count+pTotal->total[2].count) > 0)
		{
			file.CloseFile(&fh);
			return TRUE;
		}
		file.NextRecord(&fh);
	}
	file.CloseFile(&fh);

	return FALSE;
}
