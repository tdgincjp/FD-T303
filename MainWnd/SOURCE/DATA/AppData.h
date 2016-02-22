// DataFile.h : header file
//
#pragma once

#include "Common.h"
#define MAX_APP 5
// CAppData
class CAppData
{
// Construction
public:
	CAppData();	// standard constructor

	static BOOL bSignatureRequired;
	static BOOL bEMVError;

	static void SetIPAddr(char* Addr, int index = 1);
	static CString GetIPAddr(int index = 1);

	static BOOL SupportCTLS;
	static BOOL SupportPaypass;
private:
	static CString IPAddr1;
	static CString IPAddr2;

};
