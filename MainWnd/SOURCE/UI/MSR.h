#pragma once
#include "afxwin.h"
#include "SaioReader.h" // necessary include file to use HAL reader API.

class CMSR
{
public:
	CMSR(void);
	~CMSR(void);

	virtual BOOL Open (HWND hwnd);
	void Cancel();
	BOOL MsrGetTrackData();
	CString m_TrackText[3];
	CString m_TrackErrorText[3];
	byte *m_TrackData[3];        // Track-1 data array
	BOOL GetMSRData(int TrackIndex,BYTE* buf, DWORD* len, DWORD* decode);

private:
	CString ByteToStr(BYTE *bBuf, int Len);
	DWORD m_TrackLen[3];     // Track length
	DWORD m_TrackDecode[3];  // Track decode
	DWORD m_TrackError[3];
	CString GetErrorMsg(DWORD ErrCode);
	float m_IPS;
};
