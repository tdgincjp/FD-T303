//1--- MSR
DWORD OpenMsr();
DWORD CloseMsr();
DWORD GetMsrData(char *track1, BYTE *t1Len,char *track2, BYTE *t2Len,char *track3, BYTE *t3Len, DWORD timeout);

//2--- SCR
DWORD OpenScr();
DWORD CloseScr();
DWORD PowerOnScr();
DWORD PowerOffScr();
DWORD GetScrStatus();
DWORD SendSCR(BYTE *lpBuffer, WORD wLen);
DWORD ReceiveSCR(BYTE *lpBuffer, WORD *pwLen);