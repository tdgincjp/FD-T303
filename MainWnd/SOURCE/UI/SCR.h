#pragma once
#include "afxwin.h"
#include "SaioReader.h" 

class CSCR
{
public:
	CSCR(void);
	~CSCR(void);

	virtual BOOL Open (HWND hwnd);
	void Cancel();
	BOOL PowerOn();
	BOOL GetState();
	
	BOOL GetSCRData();
	void Close();

	BOOL m_bPowerOff;
	void Reset();

private:

	int m_IccSlot;
	int m_IccPower;
	WORD Event_time_out;

	BOOL Scr_PowerOn(WORD iccID, WORD voltage, DWORD *errCode);
	BOOL Scr_Reset(WORD iccID, DWORD *errCode);
	BOOL Scr_SendData(WORD iccID, BYTE *buffer, DWORD len, DWORD *errCode);

	BYTE m_ATR[256];
	int m_DataLen;

};
