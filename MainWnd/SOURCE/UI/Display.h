#pragma once
#define MAX_DISPLAY_LEN		7 
#define MAX_BUTTON_NUMBER	3 

#define KEY_ENTER		1 
#define KEY_CANCEL		2 
#define KEY_CORR		3 
#define KEY_BUT1		4 
#define KEY_BUT2		5 
#define KEY_BUT3		6
#define KEY_TIMEOUT		7

#define MASK_OK			0x01
#define MASK_CANCEL		0x02
#define MASK_CORR		0x04

class CDisplay
{
public:

private:
	static CString m_strText[MAX_DISPLAY_LEN];
	static CString m_strButtonText[MAX_BUTTON_NUMBER];
	static int  m_iMask;

	static HANDLE m_hEvent;
	static int m_iKeyReturn;
	static int m_iButtonNumber;
	static BOOL m_bPINOK;

	// Construction
public:
	CDisplay();
	~CDisplay();
	static HWND  m_hDisplayWnd;
	static BOOL  m_bStopDisplay;
	static CString m_strAppName;

	static void SetText(const int line, const int TextIndex);
	static void SetText(const int line, const char* pText);
	static void SetText(const int line, const LPCTSTR pText);
	static LPCTSTR GetLine(const int line);
	static LPCTSTR GetButtonText(const int index);
	static void Clear();
	static void Display();
	static void SetDisplayWnd(HWND hwnd);
	static void DisplayWait(int timeout);	// timeout in seconds
	static int DisplayWait(int mask,int timeout);
	static int DisplayWait(int mask,const char *strBt1, int timeout);
	static int DisplayWait(int mask,const int idxBt1, int timeout);
	static int DisplayWait(int mask,const char *strBt1, const char *strBt2, int timeout);
	static int DisplayWait(int mask,const int idxBt1, const int idxBt2, int timeout);

	static void ClearScreen();

	static void SetKeyReturn(int value);
	static int GetMask();
	static int GetBtNumber();
	static BOOL GetPINOK();
	static void SetPINOK(BOOL flag);

	static int SelectLanguage();
	static int PassToClerk();
	static int PassToCustomer();
// Operations
private:
};
