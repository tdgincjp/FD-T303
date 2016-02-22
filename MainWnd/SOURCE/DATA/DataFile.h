// DataFile.h : header file
//

#pragma once

// CDataFile
class CDataFile
{
// Construction
public:
	CDataFile();	// standard constructor

public:
	static BOOL Read(LPCTSTR ptItemName, TCHAR* ptValue,BOOL bDefault = TRUE);
	static BOOL Save(LPCTSTR ptItemName, TCHAR* ptValue);

	static BOOL Read(LPCTSTR ptItemName, BYTE* ptValue, LPDWORD size,BOOL bDefault = TRUE);
	static BOOL Save(LPCTSTR ptItemName, BYTE* ptValue, DWORD size);

	static BOOL Read(int index, BYTE* ptValue, LPDWORD size,BOOL bDefault = TRUE);
	static BOOL Save(int index, BYTE* ptValue, DWORD size);
	static BOOL Delete(int index);
	static BOOL Delete(CString str);
	static BOOL m_bTraining;
	static BOOL m_bConnected;

	static BOOL SetIPAddress(char* buf);
	static BOOL SaveTcpIp(LPCTSTR ptItemName, char* ptValue);
	static void UpdateStatistics(int index, BOOL bDel = FALSE);

	static BOOL IsDebitOnlyTerminal();

	static void DelOfflineData();

	static void CheckSurcharge(int value);

	static void CheckMemory(CString str,BOOL flag = TRUE);
	static int  m_iMemory;


private:
	static CString AmountFormat(int amount);
};
