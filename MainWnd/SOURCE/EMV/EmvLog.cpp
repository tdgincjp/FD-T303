#include "StdAfx.h"
#include "EmvLog.h"
#pragma warning(disable: 4996)

EmvLog::EmvLog(void)
{
	enabled = false;
}

//----------------------------------------------------------------------------
//!	\brief	EMV Create Log file function
//!	\param	logName - log file Name
//! RETURNS:       TRUE    = process ok.                               
//!                FALSE    = process failed.
//----------------------------------------------------------------------------
BOOL EmvLog::CreateLog(WCHAR *logName)
{
	TCHAR  t_moduleName[255];
	memset (t_moduleName, 0, 510);

	int length = GetModuleFileName(NULL,t_moduleName,255);

	int i = 0;
	for(i = length-1 ; i >=0 ; i--)
	{
		if(t_moduleName[i] == L'\\')
			break;
	}
	memset( t_moduleName + i + 1, 0, (length - i - 1) * 2 );
	memcpy( t_moduleName + i + 1, logName, wcslen(logName) * 2 );

	hFile = CreateFile(t_moduleName, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

	SetFilePointer(hFile, NULL, NULL, FILE_END);

	if ( hFile == INVALID_HANDLE_VALUE )
	{
		hFile = CreateFile(t_moduleName, GENERIC_READ | GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
		if ( hFile == INVALID_HANDLE_VALUE )
			return FALSE;
	}
	enabled = true;

	return TRUE;
}

//----------------------------------------------------------------------------
//!	\brief	EMV write Log file function
//!	\param	pbyBuffer - point to write data
//!	\param	pwLen - data length
//!	\param	nPoint - log type
//! RETURNS:       TRUE    = process ok.                               
//!                FALSE    = process failed.
//----------------------------------------------------------------------------
BOOL EmvLog::WriteLog(BYTE *pbyBuffer, WORD *pwLen, int nPoint)
{
	if (!enabled)
		return FALSE;

	memset (strBuf, 0, 1024);

	DWORD index  = 0;

	if(nPoint == 0)
		sprintf( strBuf, "[RCV] " );
	else if(nPoint == 1)
		sprintf( strBuf, "[SND] " );
	else
		sprintf( strBuf, "[DUG] " );
	index += 6;

	int i = 0;
	for (i = 0 ; i < *pwLen ; i++)
	{
		sprintf( strBuf + index, "%02X ", pbyBuffer[i] );
		index += 3;
	}
	sprintf( strBuf + index, "\r\n", pbyBuffer[i] );
	index += 2;

	DWORD nWrite = 0;
	return WriteFile( hFile, strBuf, index, &nWrite, NULL);
}

//----------------------------------------------------------------------------
//!	\brief	EMV write Log file function
//!	\param	txnName - file name
//! RETURNS:       TRUE    = process ok.                               
//!                FALSE    = process failed.
//----------------------------------------------------------------------------
BOOL EmvLog::TxnLog(WCHAR *txnName)
{
	memset (strBuf, 0, 1024);

	CHAR tmp[255];
	memset (tmp, 0, 255);

	WideCharToMultiByte( CP_ACP, 0, txnName, -1, tmp, 255, NULL, NULL);
	sprintf( strBuf, "[TXN] %s\r\n", tmp );
	DWORD length = strlen(strBuf);

	DWORD nWrite = 0;
	return WriteFile( hFile, strBuf, length, &nWrite, NULL);
}

//----------------------------------------------------------------------------
//!	\brief	EMV close Log file function
//! RETURNS:       TRUE    = process ok.                               
//!                FALSE    = process failed.
//----------------------------------------------------------------------------
BOOL EmvLog::CloseLog()
{
	if (hFile)
	{
		hFile = NULL;
		return CloseHandle( hFile );
	}
	return FALSE;	
}

//----------------------------------------------------------------------------
//!	\brief	EMV write Log file function
//!	\param	pbyBuffer - point to write data
//! RETURNS:       TRUE    = process ok.                               
//!                FALSE    = process failed.
//----------------------------------------------------------------------------
BOOL EmvLog::WriteStringLog(CHAR *pbyBuffer)
{
	if (!enabled)
		return FALSE;

	memset (strBuf, 0, 1024);
	sprintf( strBuf, "%s\r\n", pbyBuffer );

	DWORD nWrite = 0;
	return WriteFile( hFile, strBuf, strlen(strBuf), &nWrite, NULL);
}

EmvLog::~EmvLog(void)
{
	if (hFile)
		CloseHandle( hFile );
}
