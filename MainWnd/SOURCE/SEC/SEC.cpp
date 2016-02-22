#include "StdAfx.h"
#include "SaioPinpad.h"
#include "..\\defs\\constant.h"
#include "..\\data\\DataFile.h"
#include "..\\utils\\StrUtil.h"
#include "SEC.h"
#include "DesECB.h"
#pragma comment(lib, "VenusCrypt4WinCE.lib")

//----------------------------------------------------------------------------
//!	\brief	Constructor
//----------------------------------------------------------------------------
CSEC::CSEC(void)
{
}

//----------------------------------------------------------------------------
//!	\brief	Destructor
//----------------------------------------------------------------------------
CSEC::~CSEC(void)
{
}

//----------------------------------------------------------------------------
//!	\brief	Generate MAC value
//----------------------------------------------------------------------------
BOOL CSEC::CalculateMAC(BYTE *macData, DWORD macDataLength, char* macValue)
{
	DWORD size=SZ_KEY/2;
	BYTE key[SZ_KEY];
	BYTE MAC[SZ_MAC/2];	// hex
	DWORD status;
	BOOL result=FALSE;

	memset(key,0,16);
	if (CDataFile::Read(FID_TERM_MAC_KEY, key, &size))
	{
		int len = SZ_KEY/2;
		if (size != len)
			return FALSE;

		status = PinOpen();
		if (status == PIN_ACCESS_DENY || status == PIN_DEV_NOT_EXIST)
		{
			return FALSE;
		}
		
		// macDataLength has to be multiple of 8
		int rem = macDataLength%8;
		if (rem > 0)
			len = macDataLength + (8-rem);
		else
			len = macDataLength;
			
		BYTE *macBuf = new BYTE[len];
		if (macBuf == NULL)
			return FALSE;
		
		memset(macBuf, 0, len);
		memcpy(macBuf, macData, macDataLength); 
		
		status = PinGenMac(PIN_MASTER_KEY0,
											 TRUE,
											 PIN_MAC_ISO_1,
											 key,
											 (WORD) size,
											 macBuf,
											 len);
		if (status == 0)
		{
			status = PinGetMac(MAC);
			if (status == 0)
			{
				// unpack MAC value for host
				STR_UnpackData(macValue, MAC, 4);
				macValue[SZ_MAC] = 0;
				result = TRUE;
			}
		}

		delete [] macBuf;
 		PinClose();
	}

	return result;
}


//----------------------------------------------------------------------------
//!	\brief	Validate MAC 
//----------------------------------------------------------------------------
BOOL CSEC::VerifyMAC(BYTE *macData, DWORD macDataLength, const char* macValue)
{
	char calcMAC[SZ_MAC+1];
	
	if (CalculateMAC(macData, macDataLength, calcMAC))
	{
		CString traceData1 = CString(macValue);
		TRACE(L"HOST MAC: %s\n", traceData1);
		CString traceData2 = CString(calcMAC);
		TRACE(L"CALC MAC: %s\n", traceData2);

		if (strcmp(macValue, calcMAC) == 0)
			return TRUE;
	}

	return FALSE;
}

