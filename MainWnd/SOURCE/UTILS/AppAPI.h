// DataFile.h : header file
//
#pragma once

#include "Common.h"
#define MAX_APP 5
// CAppAPI
class CAppAPI
{
// Construction
public:
	CAppAPI();	// standard constructor

	static CString ByteToStr(BYTE *bBuf, int Len,int format=1);
	static int StringAtoI(CString str);

private:

};
