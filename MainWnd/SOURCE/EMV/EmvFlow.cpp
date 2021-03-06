#include "StdAfx.h"
#include "Common.h"
#include "EmvApi.h"
#include "EmvData.h"
#include "CallBack.h"
#include "SaioDev.h"
#include "..\\defs\\Constant.h"
#include "..\\defs\\struct.h"
#include "..\\data\\DataFile.h"
#include "..\\transactions\\Finan.h"
#include "Emvlog.h"

#pragma comment(lib, "Lib\\EMVAPI.lib")

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

EmvLog logger;

//static void FillAuthResponse(TXNCONDITION *pTxn, TRAN_EmvData *pEMVData, AUTHRESPONSE *pAuthResp);

//TXNCONDITION TXN;

//----------------------------------------------------------------------------
//!	\brief	EMV init data function
//! RETURNS:       TRUE    = process ok.                               
//!                FALSE    = process failed.
//----------------------------------------------------------------------------
BOOL InitData()
{
	DeleteProfiles();

	if (!SetIcsProfile())
		return FALSE;

	if (!SetTerminalProfile())
		return FALSE;

	if (!SetApplicationProfile())
		return FALSE;

	if (!SetCapkProfile())
		return FALSE;

	return TRUE;
}

//----------------------------------------------------------------------------
//!	\brief	EMV init call back function
//----------------------------------------------------------------------------
void InitCallBack()
{
	CB_EmvCallbackReg(MultiCallBack);
}

//----------------------------------------------------------------------------
//!	\brief	EMV init Device function
//! RETURNS:       TRUE    = process ok.                               
//!                FALSE    = process failed.
//----------------------------------------------------------------------------
BOOL InitDevice()
{
	DWORD dwRet = 0;

	//Init SCR
	dwRet = OpenScr();
	if (dwRet != 0)
	{
		return FALSE;
	}

	CloseScr();

	return TRUE;
}

//----------------------------------------------------------------------------
//!	\brief	EMV Close Device function
//----------------------------------------------------------------------------
void CloseDevice()
{
	CloseScr();
	CloseMsr();
}

//----------------------------------------------------------------------------
//!	\brief	EMV init function
//! RETURNS:       TRUE    = process ok.                               
//!                FALSE    = process failed.
//----------------------------------------------------------------------------
BOOL InitEmv()
{
	if (!InitDevice())
		return FALSE;
	
	if (!InitData())
		return FALSE;

	InitCallBack();

	return TRUE;
}
