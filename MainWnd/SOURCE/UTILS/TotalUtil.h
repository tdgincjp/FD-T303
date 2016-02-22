/* 
 * File:   Totals.h
 * Author: tan
 *
 * Created on June 4, 2012, 11:50 AM
 */

#ifndef TOTALUTIL_H
#define	TOTALUTIL_H

#ifdef	__cplusplus
extern "C" {
#endif

typedef enum
{
  TOT_SALE,
  TOT_REFUND,
  TOT_CORRECTION,
  NUMBER_TRAN_TOTALS
} eTranTotalType;

#define TOT_NOT_BALANCED  0x0000
#define TOT_BALANCED      0x0001

struct sTranTotal
{
  USHORT      flag;
	USHORT      count;
	long       amount;
	sTranTotal()
	{
		count = 0;
		amount = 0;
	}
} ;

typedef struct
{
	USHORT       key;
	BYTE				 serviceType[2];
	sTranTotal   total[NUMBER_TRAN_TOTALS];
} sCardTotalRecord;

#define MAX_NUMBER_CARD_TOTALS     30    // number of BINs downloaded
#define MAX_NUMBER_HOST_TOTALS     30

#define GRAND_TOTAL_SERVICE_TYPE   "  "

void TOT_CreateEmptyTotalsFile(USHORT maxNumberRecords);
void TOT_UpdateTranTotalsFile(int tranType, char *servType, long amount);
void TOT_UpdateTranTotal(sCardTotalRecord *pTotal, int tranType, long amount);
BOOL TOT_CompareTermHostTotals();
void TOT_AddCardTotalsRecords(USHORT numberOfRecords, sCardTotalRecord *pTotal);
BOOL TOT_IsBatchOpen();

#ifdef	__cplusplus
}
#endif

#endif	/* TOTALUTIL_H */

