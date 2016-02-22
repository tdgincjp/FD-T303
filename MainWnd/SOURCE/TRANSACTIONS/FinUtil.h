#pragma once

class CFinUtil
{
	public:
		CFinUtil(void);
		~CFinUtil(void);
		static BOOL GetCardInfo(char *pTrack2, char *pAcct, char *pExp, BOOL *pChipCard,
													 int *pCardType, int *pLang, char *pServ);
		static BOOL GetServTypeFromAID(char *AID, char *servType);	// from Did k format 1
		static int ValidateCard(char *account, char *servType, sBinRangeData *pBinRecord,int CardType=CARD_CREDIT,BOOL EMVRead = FALSE);
		static BOOL IsSupportCard(char *servType);

};
