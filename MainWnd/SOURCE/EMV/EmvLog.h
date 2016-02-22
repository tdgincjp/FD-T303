#pragma once

class EmvLog
{
public:
	EmvLog(void);
	~EmvLog(void);

	BOOL CreateLog(WCHAR *logName);
	BOOL WriteLog(BYTE *pbyBuffer, WORD *pwLen, int nPoint);
	BOOL TxnLog(WCHAR *txnName);
	BOOL CloseLog();
	BOOL WriteStringLog(CHAR *pbyBuffer);

	CHAR strBuf[1024];
	HANDLE hFile;

	bool enabled;

};
