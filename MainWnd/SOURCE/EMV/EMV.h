#pragma once

#include "..\\defs\\Constant.h"
#include "..\\defs\\struct.h"
#include "..\\transactions\\Finan.h"

class CEMV
{
public:
	CEMV(void);
	~CEMV(void);
	BOOL Init();
	void Start(int txnType, long amount, void *p=NULL);//(int txnType, long amount, int txnSn, void *p=NULL);
	static BOOL IsInteracRID(char *RID);
	static BOOL IsMasterCardRID(char *RID);
	static BOOL IsVisaRID(char *RID);
	static BOOL IsMaestroAID(char *AID);
	static BOOL IsMasterCardAID(char *AID);
	static BOOL IsVisaAID(char *AID);
	static BOOL IsVisaDebitAID(char *AID);
	static BOOL IsAmexRID(char *RID);
	static BOOL IsJcbRID(char *RID);
};
