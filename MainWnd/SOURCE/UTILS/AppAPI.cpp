
// AppAPI.cpp : implementation file
//

#include "stdafx.h"
#include <stdio.h>

#include "AppAPI.h"
#include "..\\defs\\constant.h"
#include "..\\UTILS\\string.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//BOOL CAppAPI::m_bTraining = FALSE;


CAppAPI::CAppAPI()
{
}
	
//-----------------------------------------------------------------------------
//!	\change Byte string to CString
//!	\param	bBuf - Byte string buffer point 
//!	\param	len - Byte string buffer sizze 
//!	\param	format - 1: "%02X", other:"%c" format 
//-----------------------------------------------------------------------------
CString CAppAPI::ByteToStr(BYTE *bBuf, int Len,int format)
{
	int index;
	CString str,temp;

	for (index = 0; index < Len; index++)
	{
		if (bBuf[index] != 0x0d && bBuf[index] != 0x0a)
		{
			if( format == 1)
				temp.Format (L"%02X", bBuf[index]);
			else
				temp.Format (L"%c", bBuf[index]);
			str += temp;
		}
	}
	return str;
}

//-----------------------------------------------------------------------------
//!	\change string to int
//!	\param	str - string  
//-----------------------------------------------------------------------------
int CAppAPI::StringAtoI(CString str)
{
	if (str == L"")
		return 0;

	char *p = alloc_tchar_to_char(str.GetBuffer(str.GetLength()));
	int value = atoi(p);
	free(p);
	str.ReleaseBuffer();
	return value;
}
