
// DataFile.cpp : implementation file
//

#include "stdafx.h"
#include <stdio.h>

#include "DataFile.h"
#include "..\\defs\\constant.h"
#include "..\\UTILS\\string.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

BOOL CDataFile::m_bTraining = FALSE;
BOOL CDataFile::m_bConnected = FALSE;
int  CDataFile::m_iMemory = 0;

CDataFile::CDataFile()
{
}
	
//----------------------------------------------------------------------------
//!	\brief	read registry data
//!	\param	ptItemName - item name
//!	\param	ptValue - point to Item data buffer
//!	\param	bDefault - use default value or not
//! RETURNS:       TRUE    = process ok.                               
//!                FALSE    = process failed.
//----------------------------------------------------------------------------
BOOL CDataFile::Read(LPCTSTR ptItemName, TCHAR* ptValue,BOOL bDefault)
{
	HKEY hkKey;

	// Open the specified key
	CString str = L"Init";
	if (ERROR_SUCCESS != RegOpenKeyEx(HKEY_LOCAL_MACHINE, str , 0, KEY_READ, &hkKey))
		return FALSE;

	DWORD   type;
	DWORD   size = MAX_PATH;
	TCHAR   buffer [MAX_PATH];
	TCHAR * bufPointer = buffer;
	BOOL    result     = TRUE;
 
	if(m_bTraining)
		str = L"\\TRAINMODE_" + CString(ptItemName);
	else
		str = CString(ptItemName);
	long ret = RegQueryValueEx(hkKey, str, NULL, &type, 
		(unsigned char *)bufPointer,&size);
	if (ERROR_SUCCESS != ret && ERROR_MORE_DATA != ret)
		result = FALSE;

	if (bDefault && m_bTraining && !result)
	{
		result = TRUE;
		ret = RegQueryValueEx(hkKey, ptItemName, NULL, &type, 
			(unsigned char *)bufPointer,&size);
		if (ERROR_SUCCESS != ret && ERROR_MORE_DATA != ret)
			result = FALSE;
	}

	if(result)
		_tcscpy_s(ptValue, MAX_PATH, buffer);

	// Close the key
	RegCloseKey(hkKey);

	return result;
}

//----------------------------------------------------------------------------
//!	\brief	save registry data
//!	\param	ptItemName - item name
//!	\param	ptValue - point to Item data buffer
//! RETURNS:       TRUE    = process ok.                               
//!                FALSE    = process failed.
//----------------------------------------------------------------------------
BOOL CDataFile::Save(LPCTSTR ptItemName, TCHAR* ptValue)
{
	HKEY hkKey;

	// Open the specified key
	CString str = L"Init";
	if (ERROR_SUCCESS != RegOpenKeyEx(HKEY_LOCAL_MACHINE, str , 0, KEY_READ, &hkKey))
		return FALSE;
	BOOL    result     = TRUE;

	if(m_bTraining)
		str = L"\\TRAINMODE_" + CString(ptItemName);
	else
		str = CString(ptItemName);
	if (RegSetValueEx(hkKey,str,
                       NULL,REG_SZ,(unsigned char *)ptValue,
                       MAX_PATH) != ERROR_SUCCESS)
		result = FALSE;
	RegFlushKey(hkKey);
	RegCloseKey(hkKey);
	return result;
}

//----------------------------------------------------------------------------
//!	\brief	read registry data
//!	\param	ptItemName - item name
//!	\param	ptValue - point to Item data buffer
//!	\param	size - Item data buffer size
//!	\param	bDefault - use default value or not
//! RETURNS:       TRUE    = process ok.                               
//!                FALSE    = process failed.
//----------------------------------------------------------------------------
BOOL CDataFile::Read(LPCTSTR ptItemName, BYTE* ptValue, LPDWORD size,BOOL bDefault)
{
	HKEY hkKey;

	// Open the specified key
	CString str = L"Init";
	if (ERROR_SUCCESS != RegOpenKeyEx(HKEY_LOCAL_MACHINE, str , 0, KEY_READ, &hkKey))
		return FALSE;

	DWORD   type;
	BOOL    result     = TRUE;
	*size = 1024;

	if(m_bTraining)
		str = L"\\TRAINMODE_" + CString(ptItemName);
	else
		str = CString(ptItemName);
	if (ERROR_SUCCESS != RegQueryValueEx(hkKey, str, NULL, &type, 
		(unsigned char *)ptValue,size))
		result = FALSE;

	if (bDefault && m_bTraining && !result)
	{
		result = TRUE;
		if (ERROR_SUCCESS != RegQueryValueEx(hkKey, ptItemName, NULL, &type, 
			(unsigned char *)ptValue,size))
			result = FALSE;
	}

	RegCloseKey(hkKey);

	if(result == FALSE)
		*size = 0;

//	*(ptValue+*size) = 0;
	return result;
}


//----------------------------------------------------------------------------
//!	\brief	save registry data
//!	\param	ptItemName - item name
//!	\param	ptValue - point to Item data buffer
//!	\param	size - Item data buffer size
//! RETURNS:       TRUE    = process ok.                               
//!                FALSE    = process failed.
//----------------------------------------------------------------------------
BOOL CDataFile::Save(LPCTSTR ptItemName, BYTE* ptValue, DWORD size)
{
	HKEY hkKey;

	// Open the specified key
	CString str = L"Init";
	if (ERROR_SUCCESS != RegOpenKeyEx(HKEY_LOCAL_MACHINE, str , 0, KEY_READ, &hkKey))
		return FALSE;
	BOOL    result     = TRUE;

	if(m_bTraining)
		str = L"\\TRAINMODE_" + CString(ptItemName);
	else
		str = CString(ptItemName);
	if (RegSetValueEx(hkKey,str,
                       NULL,REG_BINARY,(unsigned char *)ptValue,
                       size) != ERROR_SUCCESS)
		result = FALSE;
	RegFlushKey(hkKey);
	RegCloseKey(hkKey);
	return result;
}

//----------------------------------------------------------------------------
//!	\brief	read registry data
//!	\param	index - item index
//!	\param	ptValue - point to Item data buffer
//!	\param	size - Item data buffer size
//!	\param	bDefault - use default value or not
//! RETURNS:       TRUE    = process ok.                               
//!                FALSE    = process failed.
//----------------------------------------------------------------------------
BOOL CDataFile::Read(int index, BYTE* ptValue, LPDWORD size,BOOL bDefault)
{
	CString str;
	str.Format(L"%d",index);
	return Read(str,ptValue,size,bDefault);
}

//----------------------------------------------------------------------------
//!	\brief	read registry data
//!	\param	index - item index
//!	\param	ptValue - point to Item data buffer
//!	\param	size - Item data buffer size
//! RETURNS:       TRUE    = process ok.                               
//!                FALSE    = process failed.
//----------------------------------------------------------------------------
BOOL CDataFile::Save(int index, BYTE* ptValue, DWORD size)
{
	CString str;
	str.Format(L"%d",index);
	return Save(str,ptValue,size);
}

//----------------------------------------------------------------------------
//!	\brief	delete registry data
//!	\param	index - item index
//! RETURNS:       TRUE    = process ok.                               
//!                FALSE    = process failed.
//----------------------------------------------------------------------------
BOOL CDataFile::Delete(int index)
{
	HKEY hkKey;
	CString str;
	str.Format(L"%d",index);

	// Open the specified key
	CString str1 = L"Init";
	if (ERROR_SUCCESS != RegOpenKeyEx(HKEY_LOCAL_MACHINE, str1 , 0, KEY_READ, &hkKey))
		return FALSE;

	BOOL   result     = TRUE;
	if(m_bTraining)
		str1 = L"\\TRAINMODE_" + str;
	else
		str1 = str;
	if (RegDeleteValue(hkKey,str1) != ERROR_SUCCESS)
		result = FALSE;
	RegFlushKey(hkKey);
	RegCloseKey(hkKey);
	return result;
}

//----------------------------------------------------------------------------
//!	\brief	read registry data
//!	\param	str - item name
//! RETURNS:       TRUE    = process ok.                               
//!                FALSE    = process failed.
//----------------------------------------------------------------------------
BOOL CDataFile::Delete(CString str)
{
	HKEY hkKey;

	// Open the specified key
	CString str1 = L"Init";
	if (ERROR_SUCCESS != RegOpenKeyEx(HKEY_LOCAL_MACHINE, str1 , 0, KEY_READ, &hkKey))
		return FALSE;

	BOOL   result     = TRUE;
	if(m_bTraining)
		str1 = L"\\TRAINMODE_" + str;
	else
		str1 = str;
	if (RegDeleteValue(hkKey,str1) != ERROR_SUCCESS)
		result = FALSE;
	RegFlushKey(hkKey);
	RegCloseKey(hkKey);
	return result;
}

//----------------------------------------------------------------------------
//!	\brief	set IP address registry data
//!	\param	buf - point to IP address buffer
//! RETURNS:       TRUE    = process ok.                               
//!                FALSE    = process failed.
//----------------------------------------------------------------------------
BOOL CDataFile::SetIPAddress(char* buf)
{
	HKEY hkKey;
	// Open the specified key
	if (ERROR_SUCCESS != RegOpenKeyEx(HKEY_LOCAL_MACHINE, L"Comm\\DM90001\\Parms\\TcpIp", 0, KEY_READ, &hkKey))
		return FALSE;
	
	BOOL    result = TRUE;

	DWORD value=1;
	if(buf[0] == 'D')
	{
		if (RegSetValueEx(hkKey,L"EnableDHCP",
						   NULL,REG_DWORD,(const BYTE*)&value, sizeof(value)) != ERROR_SUCCESS)
			result = FALSE;
		RegFlushKey(hkKey);
		RegCloseKey(hkKey);
		return result;
	}

	if(buf[0] != 'S')
		return FALSE;

	value = 0;
	if (RegSetValueEx(hkKey,L"EnableDHCP",
			NULL,REG_DWORD,(const BYTE*)&value, sizeof(value)) != ERROR_SUCCESS)
	{
		result = FALSE;
		RegCloseKey(hkKey);
		return result;
	}

	if(buf[1] != ',')
		return FALSE;

	char* p = &buf[2];
	char* p1 = strchr( p,',');

	if( p1 == NULL )
		return FALSE;

	*p1 = 0;

	if ( !SaveTcpIp(L"IPAdrress",p))
		return FALSE;

	p = p1+1;
	p1 = strchr(p,',');

	if( p1 == NULL )
		return FALSE;

	*p1 = 0;
	if ( !SaveTcpIp(L"SubnetMask",p))
		return FALSE;

	p = p1+1;
	p1 = strchr(p,',');

	if( p1 == NULL )
		return FALSE;

	*p1 = 0;
	if ( !SaveTcpIp(L"DefaultGateway",p))
		return FALSE;

	p = p1+1;
	if ( !SaveTcpIp(L"DNS",p))
		return FALSE;

	return result;
}

//----------------------------------------------------------------------------
//!	\brief	save tcp IP address registry data
//!	\param	ptItemName - item name
//!	\param	ptValue - point to IP address buffer
//! RETURNS:       TRUE    = process ok.                               
//!                FALSE    = process failed.
//----------------------------------------------------------------------------
BOOL CDataFile::SaveTcpIp(LPCTSTR ptItemName, char* ptValue)
{
	HKEY hkKey;

	// Open the specified key
	if (ERROR_SUCCESS != RegOpenKeyEx(HKEY_LOCAL_MACHINE, L"Comm\\DM90001\\Parms\\TcpIp", 0, KEY_READ, &hkKey))
		return FALSE;

	BOOL    result     = TRUE;
	if (RegSetValueEx(hkKey,ptItemName,
                       NULL,REG_SZ,(unsigned char *)alloc_char_to_tchar(ptValue),
                       MAX_PATH) != ERROR_SUCCESS)
		result = FALSE;
	RegFlushKey(hkKey);
	RegCloseKey(hkKey);
	return result;
}

//----------------------------------------------------------------------------
//!	\brief	Update Statistics registry data
//!	\param	index - item index
//!	\param	bDel - delete flag
//----------------------------------------------------------------------------
void CDataFile::UpdateStatistics(int index, BOOL bDel)
{
	if( bDel)
	{
		int value = 0;
		Save(FID_CFG_INVOICE, (BYTE*) &value, sizeof(int));
		Save(FID_TERM_SEQ_FLAG, (BYTE*) &value, sizeof(int));

		value = 1;
		Save(FID_TERM_SEQ_NUMBER, (BYTE*) &value, sizeof(int));

		value = 0;
		for ( int i = 1;i <= 4;i++ )
			Save( FID_STATICS_BEGIN+i, (BYTE *)&value, sizeof(int));
		return;
	}

	DWORD status = 0;
	DWORD size;
	if ( !Read((int)(FID_STATICS_BEGIN+index), (BYTE *)&status, &size) )
		status = 0;
	status++;
	Save( (int)(FID_STATICS_BEGIN+index), (BYTE *)&status, sizeof(int));
}

//----------------------------------------------------------------------------
//!	\brief	check Is Debit Only Terminal 
//----------------------------------------------------------------------------
BOOL CDataFile::IsDebitOnlyTerminal()
{
	BOOL Enable = FALSE;
	DWORD size;
	CDataFile::Read(FID_DEBIT_ONLY_TERMINAL_FLAG, (BYTE*) &Enable, &size);
	return Enable==1;
}

//----------------------------------------------------------------------------
//!	\brief	Del Offline Data 
//----------------------------------------------------------------------------
void CDataFile::DelOfflineData()
{
	for(int i = FID_OFFLINE_BEGIN+1;i< FID_STATICS_END;i++)
		Delete(i);
}

//----------------------------------------------------------------------------
//!	\brief	Check Surcharge value
//!	\param	value - Surcharge value
//----------------------------------------------------------------------------
void CDataFile::CheckSurcharge(int value)
{
	TCHAR buf[10]={L""};
	CString str = L"";
	if ( CDataFile::Read(L"SURCHARGEFEE",buf))
		str = CString(buf);

	if(str != L"")
	{	
		str.Remove('$');
		str.Remove(' ');
		str.Remove('.');
		str.Remove(',');
		str.TrimLeft('0');
		char *p = alloc_tchar_to_char(str.GetBuffer(str.GetLength()));
		if( atoi(p) > value)
		{
			CString str1 = AmountFormat(value);
			CDataFile::Save(L"SURCHARGEFEE",str1.GetBuffer());
			str1.ReleaseBuffer();
		}
		free(p);
		str.ReleaseBuffer();
	}

	if ( CDataFile::Read(L"CASHBACKFEE",buf))
		str = CString(buf);

	if(str != L"")
	{	
		str.Remove('$');
		str.Remove(' ');
		str.Remove('.');
		str.Remove(',');
		str.TrimLeft('0');
		char *p = alloc_tchar_to_char(str.GetBuffer(str.GetLength()));
		if( atoi(p) > value)
		{
			CString str1 = AmountFormat(value);
			CDataFile::Save(L"CASHBACKFEE",str1.GetBuffer());
			str1.ReleaseBuffer();
		}
		free(p);
		str.ReleaseBuffer();
	}
}

//----------------------------------------------------------------------------
//!	\brief	change amount format from int to string
//!	\param	amount - amount value
//!	\return	amount string
//----------------------------------------------------------------------------
CString CDataFile::AmountFormat(int amount)
{
	CString str;
	str.Format(L"%d",amount);
	int len = str.GetLength();

	CString str1;
	if (len > 5)
	{
		int begin = (len-2)%3;
		if (begin != 0)
		{
			str1 = str.Mid(0,begin);
			str1.AppendChar(',');
		}

		while(len - begin > 5)
		{
			str1 += str.Mid(begin,3);
			str1.AppendChar(',');
			begin += 3;
		}
		str1 += str.Mid(begin,3);
		str1.AppendChar('.');
		str1 += str.Right(2);
	}
	else if (len <= 2)
	{
		str1 = L"0.";
		while(str1.GetLength()+len != 4 )
			str1 += L"0";
		str1 += str;
	}
	else
	{
		str1 = str.Mid(0,len-2);
		str1.AppendChar('.');
		str1 += str.Right(2);
	}

	str = L"$"+str1;
	return str;
}

//----------------------------------------------------------------------------
//!	\brief	Check Memory for code debug
//----------------------------------------------------------------------------
void CDataFile::CheckMemory(CString str,BOOL flag)
{
/*	MEMORYSTATUS Buffer;
	GlobalMemoryStatus( &Buffer );
	if ( !flag )
		m_iMemory = Buffer.dwAvailPhys/1024;
	TRACE(L"%s, start = %d, now = %dkB free\n",str, m_iMemory, Buffer.dwAvailPhys/1024);
*/	
}

