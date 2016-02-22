#include "stdafx.h"
#include "..\\Utils\\string.h"
#include "..\\defs\\constant.h"
#include "..\\defs\\struct.h"
#include "..\\Utils\\BinUtil.h"
#include "..\\utils\\StrUtil.h"
#include "..\\utils\\Util.h"
#include "..\\data\\DataFile.h"
//#include "..\\data\\DBFile.h"
#include "..\\data\\RecordFile.h"
#include "FinUtil.h"

/******************************************************************************/
CFinUtil::CFinUtil(void)
{
}

/******************************************************************************/
CFinUtil::~CFinUtil(void)
{
}

/******************************************************************************/
/*                                                                            */
/* RTN NAME:  GetCardInfo																											*/
/*                                                                            */
/* FUNCTION:  Get Required Card Information From Track 2.                     */
/*                                                                            */
/* ARGS:					pTrack2		-- pointer to customer track2 data.								*/
/*                                    0 = not required.                       */
/*                pAcct			-- pointer to account number.											*/
/*                                    0 = not required.                       */
/*                pExp			-- pointer to expiry date.												*/
/*                                    0 = not required.                       */
/*                pCardType -- pointer to card type.													*/
/*                                    0 = not required.                       */
/*                pLang			-- pointer to language code.											*/
/*                                    0 = not required.                       */
/*                                                                            */
/* RETURNS:       TRUE    = process ok.                                       */
/*                FALSE   = track 2 data error.																*/
/*                                                                            */
/* NOTES:     customer card data "cust_track2" structure:                     */
/*                                                                            */
/*            swiped card data: "aaaaa...aaaaaaaa=bbbbccc00...000d"           */
/*                                                                            */
/*            aaaaa...aaaaaaaa : account number(min 10 max 19)                */
/*            bbbb : expiry date  (YYMM)                                      */
/*            ccc : service code                                              */
/*            d : language code ('1' or '2')                                  */
/*                                                                            */
/******************************************************************************/
BOOL CFinUtil::GetCardInfo(char *pTrack2, char *pAcct, char *pExp, BOOL *pChipCard,
													 int *pCardType, int *pLang, char *pServ)
{
	char *p, servCode[4];
	int n, serv;
	BOOL manual = FALSE;

	if ((p = strchr(pTrack2, '=')) == NULL) // find separator '='
		return FALSE;

	if (pAcct != 0) // account number is required
	{
		if (*pTrack2 == ';') // skip start sentinel
		{
			pTrack2++;
		}
		else if (*pTrack2 == 'M')
		{
			pTrack2++;
			manual = TRUE;
		}

		n = (int) (p - pTrack2); // get length of acct_no

		if (n < 10 || n > 19) // too small account number or over size
			return FALSE;

		memcpy(pAcct, pTrack2, n); // get account no
		*(pAcct + n) = '\0';

		if (!STR_IsDecimal(pAcct, strlen(pAcct)))
			return FALSE;
	}

	p++;
	if (pExp != 0) // expiry date is required
	{
		if ((manual && (strlen(p) > 4) && p[4] != '?') ||
			  (!manual && (strlen(p) < 4)))
		{	  
			return FALSE;
		}
		
		memcpy(pExp, p, 4); // get expire date
		*(pExp + 4) = '\0';

		//add expiry date checking (first must be all digits, no alphas, then month must be 1-12)

		// Skip expiry date checking for TRANSACTION_GET_TRACK_DATA, in CT mode
		//first digit of Month must be 0 or 1
		if (pExp[2] < '0' || pExp[2] > '1')
			return FALSE;

		// if first digit of Month is 0, then all second digits should be 1-9
		if (pExp[2] == '0')
		{
			if (pExp[3] < '0' || pExp[3] > '9')
				return FALSE;
		}
		// if first digit of Month is 1, then second digit must be 0, 1, or 2
		if (pExp[2] == '1')
		{
			if (pExp[3] < '0' || pExp[3] > '2')
				return FALSE;
		}

		// Year digits must be between 0 and 9
		// all 4 characters digits?
		if (pExp[0] < '0' || pExp[0] > '9')
			return FALSE;
			
		if (pExp[1] < '0' || pExp[1] > '9')
			return FALSE;
	}

	p += 4;
	if (manual)
	{
		if (pCardType != NULL)
			*pCardType = CARD_CREDIT;
	}
	else if (pCardType != 0 || pChipCard != NULL) // service code is required
	{
		if (strlen(p) >= 3)
		{
			memcpy(servCode, p, 3); // get service code
			*(servCode + 3) = '\0';
			if (pServ != 0)
				strcpy(pServ, servCode);
		}
		else
		{
			return FALSE;
		}
		
		serv = atoi(servCode);
		if (pCardType != 0)
		{
			if ((serv == 120) || (serv == 220) || (serv == 798) || (serv == 799))
			{
				*pCardType = CARD_DEBIT;
			}	
			else
			{
				*pCardType = CARD_CREDIT;
			}
		}

		if (pChipCard != NULL)
		{
			if ((servCode[0] == '2' || servCode[0] == '6'))
			{
				*pChipCard = TRUE;
			}
			else
			{
				*pChipCard = FALSE;
			}
		}
	}

	p += 3;
	if (pLang != 0) // language code is required
	{
		if (strlen(p) > 1)
		{
			p += strlen(p) - 1; // point to the last char
			if (isdigit(*p) == 0)
				p--; // skip end sentinel

			if (*p == '1')
			{
				*pLang = ENGLISH;
			}	
			else if (*p == '2')
			{
				*pLang = FRENCH;
			}	
			else
			{
				int termLang;
				DWORD size;
				CDataFile::Read((int)FID_CFG_LANGUAGE, (BYTE *)&termLang, &size);
				*pLang = termLang; // set to terminal language
			}
		}
	}
	return TRUE;
}

/******************************************************************************/
/*                                                                            */
/* RTN NAME:  GetServTypeFromAID											  */
/*                                                                            */
/* FUNCTION:  Get ServType From AID.						                  */
/*                                                                            */
/* ARGS:		  AID		    -- pointer AID data.						  */
/*                servType		-- pointer to Serv Type date.   			  */
/*                                                                            */
/* RETURNS:       TRUE    = process ok.                                       */
/*                FALSE   = can not find. 									  */
/*                                                                            */
/******************************************************************************/
BOOL CFinUtil::GetServTypeFromAID(char *AID, char *servType)	// from Did k format 1
{
	CRecordFile emvDataFile;
	AP_FILE_HANDLER fh;
	USHORT numOfRecs = 0;
	BOOL found = FALSE;
	if (!emvDataFile.Open(EMV_DATA_FILE, &fh))
		return FALSE;

	emvDataFile.GetNumberOfRecords(&fh, &numOfRecs);

	if (numOfRecs > 0)
	{
		for (int n = 0; n < numOfRecs; n++)
		{
			sEmvData emvData;
			USHORT len;
			emvDataFile.ReadRecord(&fh, n, (BYTE *)&emvData, &len);
			if (len!=sizeof(sEmvData))
				break;
			if (strncmp(emvData.asciiAID, AID,strlen(emvData.asciiAID)) == 0)
			{
				strcpy(servType, emvData.serviceType);
				found = TRUE;
				break;
			}			
		}
	}
	return found;	
}

/******************************************************************************/
/*                                                                            */
/* RTN NAME:  ValidateCard      											  */
/*                                                                            */
/* FUNCTION:  check card is validate or not From account.		              */
/*                                                                            */
/* ARGS:		  account		-- pointer AID data.	    				  */
/*                servType		-- pointer to Serv Type date.   		      */
/*                pBinRecord	-- pointer to sBinRangeData date.  		      */
/*                CardType		-- card Type                 .   		      */
/*                EMVRead		-- is for EMV data.   		                  */
/*                                                                            */
/* RETURNS:       CARD_NO_ERROR    = Validate.                                */
/*                other  = not Validate. 									  */
/*                                                                            */
/******************************************************************************/
int CFinUtil::ValidateCard(char *account, char *servType, sBinRangeData *pBinRecord,int CardType,BOOL EMVRead)
{
	if (strlen(account) == 0)
	{
		return CARD_ERR_CARD_PROBLEM;
	}
	else
	{
		if (CardType != CARD_DEBIT) //TDGPJ DEC28
		{
			if (!BIN_SearchRecord(account, servType, pBinRecord))
			{
				return CARD_ERR_CARD_NOT_SUPPORT;
			}
			else
			{
				if (!CheckMod10(account, strlen(account)))
				{
					return CARD_ERR_INVALID_MODE;
				}
			}
		}
	}

	//if ( !EMVRead &&!IsSupportCard(servType))
	//	return CARD_ERR_CARD_NOT_SUPPORT;

	return CARD_NO_ERROR;
}

/******************************************************************************/
/*                                                                            */
/* RTN NAME:  IsSupportCard											          */
/*                                                                            */
/* FUNCTION:  check this card be sopport by host or not		                  */
/*                                                                             */
/* ARGS:		  servType		-- pointer to Serv Type date.                 */
/*                                                                            */
/* RETURNS:       TRUE    = process ok.                                       */
/*                FALSE   = can not find. 									  */
/*                                                                            */
/******************************************************************************/
BOOL CFinUtil::IsSupportCard(char *servType)
{
	if ( strlen(servType) == 0)
		return TRUE;

	CRecordFile servDataFile;
	AP_FILE_HANDLER fh1;

	char buf[3] = {0};
	if ( servType[0] == 'M' || servType[0] == 'V')
	{
		buf[0] = servType[0];
		buf[1] = ' ';
		buf[2] = 0;
	}
	else
		strcpy(buf,servType);

	BOOL flag = FALSE;
	if (servDataFile.Open(SERVICE_DATA_FILE, &fh1))
	{
		USHORT numOfRecords1;
		servDataFile.GetNumberOfRecords(&fh1, &numOfRecords1);

		for (int i = 0; i< numOfRecords1;i++)
		{
			sServiceData ServiceData,*pServiceData;
			USHORT len1;
			pServiceData = &ServiceData;
			memset(pServiceData, 0, sizeof(sServiceData));
			servDataFile.ReadRecord(&fh1, i, (BYTE*)pServiceData, &len1);

			if ( strcmp(pServiceData->serviceType,buf) == 0 )
			{
				flag = TRUE;
				break;
			}
		}
	}
	servDataFile.CloseFile(&fh1);
	return flag;
}
