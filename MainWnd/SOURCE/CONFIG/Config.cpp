#include "StdAfx.h"
#include "..\\defs\\constant.h"
#include "..\\data\\DataFile.h"
#include "Config.h"

CConfig::CConfig(void)
{
}

CConfig::~CConfig(void)
{
}

void CConfig::AddConfigTestData()
{
	CDataFile::Save(FID_MERCH_CFG_TERM_ID, (BYTE*) "TS090302", 8);
	CDataFile::Save(FID_CFG_DEVICE_MODEL, (BYTE*) "07        ", 10);
	BOOL enable = TRUE;
	CDataFile::Save(FID_TERM_EMV_ENABLE_FLG, (BYTE*) &enable, sizeof(BOOL));
	CDataFile::Save(FID_EMV_TERMINAL_CAPABILITIES, (BYTE*) "\xE0\xB0\xC8", 3);
	CDataFile::Save(FID_EMV_ADDITIONAL_TERMINAL_CAPABILITIES, (BYTE*) "\xF0\x00\xF0\xF0\x01", 5);
	CDataFile::Save(FID_EMV_TERMINAL_COUNTRY_CODE_NUMERIC, (BYTE*) "124", 3);
	int timer = 30;
	CDataFile::Save(FID_CFG_CUSTENTRY_TMO, (BYTE*) &timer, sizeof(int));
	int seqNo = 1;
	CDataFile::Save(FID_TERM_SEQ_NUMBER, (BYTE *) &seqNo, sizeof(int));
	CDataFile::Save(FID_TERM_SHIFT_NUMBER, (BYTE *) &seqNo, sizeof(int));
	CDataFile::Save(FID_TERM_BATCH_NUMBER, (BYTE *) &seqNo, sizeof(int));
	BYTE tranNo = 0;
	CDataFile::Save(FID_TERM_TRANSMISSION_NUMBER, &tranNo, sizeof(BYTE));
	int termLang = ENGLISH;
	CDataFile::Save(FID_CFG_LANGUAGE, (BYTE *)&termLang, sizeof(int));

	/*
	Vendor 03 - Ingenico
	Device Model IWL250
	
	
	 9.36TS090095              000000000000AO91150000?V001  0000000000?9?A03
        IWL250    0308      010330650430
	*/
}
