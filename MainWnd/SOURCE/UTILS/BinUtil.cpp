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
#include "BinUtil.h"

#pragma warning(disable: 4996)
#define BIN_RANGE_FILE "\\Saio\\user programs\\INIT\\BINFILE"

/******************************************************************************/
/*                                                                            */
/* RTN NAME:   BIN_SearchRecord												*/
/*                                                                            */
/* FUNCTION:   Search BIN Record for card name based on service type          */
/*                                                                            */
/* IN:       servType - service type										*/
/*                                                                            */
/* OUT:	   pRec - record buffer point										*/
/*                                                                            */
/******************************************************************************/
BOOL BIN_SearchRecord(char *account, char *servType, sBinRangeData *pRec)
{
	int n, acctLen;
	BOOL found;
	sBinRangeData *pBinRec;
//	BYTE serviceType[SZ_BIN_CARD_SERVICE_TYPE];
	CRecordFile binFile;
	AP_FILE_HANDLER fh;
	USHORT numOfRecs = 0;

	if( !binFile.Open(BIN_RANGE_FILE, &fh))
		return FALSE;

	binFile.GetNumberOfRecords(&fh, &numOfRecs);
	if (numOfRecs > 0)
	{
		for (n = 0; n < numOfRecs; n++)
		{
			sBinRangeData binRec;
			USHORT len;
			binFile.ReadRecord(&fh, n, (BYTE *)&binRec, &len);
			pBinRec = &binRec;

			// range check
			acctLen = strlen(account);
			if (memcmp(account, pBinRec->low, strlen(pBinRec->low)) < 0)
				continue;
			if (memcmp(account, pBinRec->high, strlen(pBinRec->high)) > 0)
				continue;
			
//			if (servType != NULL)
			if (strlen(servType) != 0 && servType != NULL)
			{
				if( pBinRec->serviceType[0] == 'V' || pBinRec->serviceType[0] == 'M')
					pBinRec->serviceType[1] = ' ';

				if (memcmp(servType, pBinRec->serviceType, 2) != 0)
				{
					continue;
				}
			}
				
			memcpy(pRec, pBinRec, sizeof(sBinRangeData));
			memcpy(servType, pBinRec->serviceType, SZ_BIN_CARD_SERVICE_TYPE);
			found = TRUE;
			break;
		}
	} // for loop

	binFile.CloseFile(&fh);

	return found;
}


/******************************************************************************/
/*                                                                            */
/* RTN NAME:   BIN_GetCardName												*/
/*                                                                            */
/* FUNCTION:   Search BIN Record for card name based on service type          */
/*                                                                            */
/* IN:       servType - service type										*/
/*                                                                            */
/* OUT:	   cardName - card name												*/
/*                                                                            */
/******************************************************************************/
BOOL BIN_GetCardName(char *servType, char *cardName)
{
	int n;
	BOOL found = FALSE;
	sBinRangeData *pBinRec;
	CRecordFile binFile;
	AP_FILE_HANDLER fh;
	USHORT numOfRecs = 0;

	if (!binFile.Open(BIN_RANGE_FILE, &fh))
		return FALSE;

	binFile.GetNumberOfRecords(&fh, &numOfRecs);

	if (numOfRecs > 0)
	{
		for (n = 0; n < numOfRecs; n++)
		{
			sBinRangeData binRec;
			USHORT len;
			binFile.ReadRecord(&fh, n, (BYTE *)&binRec, &len);
			pBinRec = &binRec;
			if (strcmp(pBinRec->serviceType, servType) == 0)
			{
				strcpy(cardName, pBinRec->name);
				found = TRUE;
				break;	
			}
		}
	}
	binFile.CloseFile(&fh);
	return found;	
}
