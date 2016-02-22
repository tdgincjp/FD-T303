
// AppData.cpp : implementation file
//

#include "stdafx.h"
#include <stdio.h>

#include "AppData.h"
#include "..\\defs\\constant.h"
#include "..\\UTILS\\string.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//BOOL CAppData::m_bTraining = FALSE;
BOOL  CAppData::bSignatureRequired = FALSE;
BOOL  CAppData::bEMVError = FALSE;
CString  CAppData::IPAddr1 = L"";
CString  CAppData::IPAddr2 = L"";

BOOL  CAppData::SupportCTLS = FALSE;
BOOL  CAppData::SupportPaypass = FALSE;

CAppData::CAppData()
{
}
	
//----------------------------------------------------------------------------
//!	\brief	Set IP address
//!	\param	Addr - point to IP data
//!	\param	index - IP index
//----------------------------------------------------------------------------
void CAppData::SetIPAddr(char* Addr, int index)
{
	if ( index == 1)
		IPAddr1 = CString(Addr);
	else if( index == 2)
		IPAddr2 = CString(Addr);
}

//----------------------------------------------------------------------------
//!	\brief	Get IP address
//!	\param	index - IP index
//!	\return	IP string
//----------------------------------------------------------------------------
CString CAppData::GetIPAddr(int index)
{
	if ( index == 1)
		return IPAddr1;
	else if ( index == 2 )
		return IPAddr2;
	return L"";
}
