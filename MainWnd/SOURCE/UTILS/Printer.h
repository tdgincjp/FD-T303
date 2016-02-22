// Printer.h : header file
//

#pragma once
#include "..\\defs\\struct.h"

#define MAX_VALUE_LEN 2500
// CDataFile
class CPrinter
{
// Construction
public:
	CPrinter();	// standard constructor

public:
	void Exec(TRAN_TranDataRec *pTranReq,HWND Event,int status,BOOL bTrain = FALSE);
	void GetReceipt(TRAN_TranDataRec *pTranReq, BYTE *RecType);								//JC May 13/15

	static DWORD WINAPI ThreadProc (LPVOID lpArg);

	BOOL m_bCancelFlag;

	int m_iValueLen;
	char m_Value[MAX_VALUE_LEN];

	BOOL m_declinedOffline;

private:
	int m_iStartPos;
	int m_iEndPos;
	int m_iLineHeight;
	int m_iCenter;
	int m_iLine;
	void TextLine(HDC& dc,CString str,int OffPos=0, int format = TA_LEFT);
	void TextLine(HDC& dc,CString str,CString str1,int OffPos=0,int OffPos1=0);
	void DrawLine(HDC& dc);
	void EmptyLine();

	DWORD Printing();

	TRAN_TranDataRec *pTRANDATA;
	CONF_Data m_CONFIG;

	DWORD PrintDebit();
	DWORD PrintCredit();
	DWORD PrintFirstDataReceipt();
	CString DealAmount(char* p);
	HWND m_hWnd;
	int  m_iStatus; //1-merchant print;2-customer copy;3-reprint
	BOOL m_bTraining;

	void PrintEMVData(HDC& dc);
	void PrintHeader(HDC& dc);
	void PrintTrailer(HDC& dc);
	CString GetEntryName();
	CString GetTransactionType();
	CString GetServiceTypeName(char* type);
	CString GetAIDName(char* type);

	BOOL IsCvmLimit();
	CString ByteToStr(BYTE *bBuf, int Len = 1, int format = 1);
};

