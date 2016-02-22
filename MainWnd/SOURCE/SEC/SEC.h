#pragma once

class CSEC
{
public:
	CSEC(void);
	~CSEC(void);
	static BOOL CalculateMAC(BYTE *macData, DWORD macDataLength, char* macValue);
	static BOOL VerifyMAC(BYTE *macData, DWORD macDataLength, const char* macValue);
};
