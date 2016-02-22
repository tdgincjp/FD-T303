
#include "stdafx.h"

#include "Language.h"
#include "Display.h"
//#include "DisplayWnd.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CString CDisplay::m_strText[MAX_DISPLAY_LEN] = {0};
CString CDisplay::m_strButtonText[MAX_BUTTON_NUMBER] = {0};
HWND  CDisplay::m_hDisplayWnd = 0;
int CDisplay::m_iMask = 0;
HANDLE CDisplay::m_hEvent = 0;//CreateEvent(NULL, FALSE, FALSE, NULL);
int CDisplay::m_iKeyReturn = 0;
int CDisplay::m_iButtonNumber = 0;
BOOL CDisplay::m_bPINOK = FALSE;
CString CDisplay::m_strAppName = 0;
BOOL CDisplay::m_bStopDisplay = FALSE;

//-----------------------------------------------------------------------------
//!	\CDisplay class virtual function
//-----------------------------------------------------------------------------
CDisplay::CDisplay()
{
	if(m_hEvent == 0)
		m_hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
}

//-----------------------------------------------------------------------------
//!	\CDisplay class virtual function
//-----------------------------------------------------------------------------
CDisplay::~CDisplay()
{
	if(m_hEvent)
		CloseHandle(m_hEvent);
	m_hEvent = 0;
}

//-----------------------------------------------------------------------------
//!	\Set text buffer data
//!	\param	line - show text line id 
//!	\param	TextIndex - text id index 
//-----------------------------------------------------------------------------
void CDisplay::SetText(const int line, const int TextIndex)
{
	char* pText;
	CLanguage::GetTextFromIndex(TextIndex,&pText);
	m_strText[line-1] = CString(pText);
}

//-----------------------------------------------------------------------------
//!	\Set text buffer data
//!	\param	line - show text line id 
//!	\param	pText - text buffer point 
//-----------------------------------------------------------------------------
void CDisplay::SetText(const int line, const char* pText)
{
	m_strText[line-1] = CString(pText);
}

//-----------------------------------------------------------------------------
//!	\Set text buffer data
//!	\param	line - show text line id 
//!	\param	pText - text string point 
//-----------------------------------------------------------------------------
void CDisplay::SetText(const int line, const LPCTSTR pText)
{
	m_strText[line-1] = pText;
}

//-----------------------------------------------------------------------------
//!	\Display text
//-----------------------------------------------------------------------------
void CDisplay::Display()
{
	if (m_bStopDisplay)
		return;
	if (m_hDisplayWnd != 0)
	{
		::PostMessage(m_hDisplayWnd,ERROR_MESSAGE,2,6);
		Sleep(10);
	}
}

//-----------------------------------------------------------------------------
//!	\Display text and wait timeout second to go ahead
//!	\param	timeout - wait timeout second 
//-----------------------------------------------------------------------------
void CDisplay::DisplayWait(int timeout)	// timeout in seconds
{
	Display();
	Sleep(timeout*1000);		
}

//-----------------------------------------------------------------------------
//!	\Display text and wait timeout second to go ahead
//!	\param	mask - it is for MASK_OK,MASK_CANCEL,MASK_CORR
//!	\param	timeout - wait timeout second 
//-----------------------------------------------------------------------------
int CDisplay::DisplayWait(int mask,int timeout)
{
	m_iMask = mask;
	m_iButtonNumber = 0;

	Display();
	if (WAIT_TIMEOUT == WaitForSingleObject(m_hEvent,timeout*1000))
		return KEY_TIMEOUT;
	return m_iKeyReturn;
}

//-----------------------------------------------------------------------------
//!	\Display text and wait timeout second to go ahead
//!	\param	mask - it is for MASK_OK,MASK_CANCEL,MASK_CORR
//!	\param	strBt1 - string buffer point that be display on the button
//!	\param	timeout - wait timeout second 
//-----------------------------------------------------------------------------
int CDisplay::DisplayWait(int mask,const char *strBt1, int timeout)
{
	if(m_hEvent == 0)
		m_hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	m_iButtonNumber = 1;
	m_strButtonText[0] = CString(strBt1);
	m_iMask = mask;
	Display();
	if (WAIT_TIMEOUT == WaitForSingleObject(m_hEvent,timeout*1000))
		return KEY_TIMEOUT;
	return m_iKeyReturn;
}

//-----------------------------------------------------------------------------
//!	\Display text and wait timeout second to go ahead
//!	\param	mask - it is for MASK_OK,MASK_CANCEL,MASK_CORR
//!	\param	idxBt1 - button index
//!	\param	timeout - wait timeout second 
//-----------------------------------------------------------------------------
int CDisplay::DisplayWait(int mask,const int idxBt1, int timeout)
{
	if(m_hEvent == 0)
		m_hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	m_iButtonNumber = 1;
	char* pText1;
	CLanguage::GetTextFromIndex(idxBt1, &pText1);
	m_strButtonText[0] = CString(pText1);
	m_iMask = mask;
	Display();
	if (WAIT_TIMEOUT == WaitForSingleObject(m_hEvent,timeout*1000))
		return KEY_TIMEOUT;
	return m_iKeyReturn;
}

//-----------------------------------------------------------------------------
//!	\Display text and wait timeout second to go ahead
//!	\param	mask - it is for MASK_OK,MASK_CANCEL,MASK_CORR
//!	\param	strBt1 - string buffer point that be display on the button1
//!	\param	strBt2 - string buffer point that be display on the button2
//!	\param	timeout - wait timeout second 
//-----------------------------------------------------------------------------
int CDisplay::DisplayWait(int mask,const char *strBt1, const char *strBt2, int timeout)
{
	if(m_hEvent == 0)
		m_hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	m_iButtonNumber = 2;
	m_strButtonText[0] = CString(strBt1);
	m_strButtonText[1] = CString(strBt2);
	m_iMask = mask;
	Display();
	if (WAIT_TIMEOUT == WaitForSingleObject(m_hEvent,timeout*1000))
		return KEY_TIMEOUT;
	return m_iKeyReturn;
}

//-----------------------------------------------------------------------------
//!	\Display text and wait timeout second to go ahead
//!	\param	mask - it is for MASK_OK,MASK_CANCEL,MASK_CORR
//!	\param	idxBt1 - button index1
//!	\param	idxBt2 - button index2
//!	\param	timeout - wait timeout second 
//-----------------------------------------------------------------------------
int CDisplay::DisplayWait(int mask,const int idxBt1, const int idxBt2, int timeout)
{
	if(m_hEvent == 0)
		m_hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

	m_iButtonNumber = 2;
	char* pText1, *pText2;
	CLanguage::GetTextFromIndex(idxBt1, &pText1);
	CLanguage::GetTextFromIndex(idxBt2, &pText2);
	m_strButtonText[0] = CString(pText1);
	m_strButtonText[1] = CString(pText2);
	m_iMask = mask;
	Display();
	if (WAIT_TIMEOUT == WaitForSingleObject(m_hEvent,timeout*1000))
		return KEY_TIMEOUT;
	return m_iKeyReturn;
}

//-----------------------------------------------------------------------------
//!	\Clear screen display
//-----------------------------------------------------------------------------
void CDisplay::ClearScreen()
{
	Clear();
	Display();
}

//-----------------------------------------------------------------------------
//!	\Clear screen display text string to empty
//-----------------------------------------------------------------------------
void CDisplay::Clear()
{
	m_iMask = 0;
	m_iButtonNumber = 0;

	for(int i=1;i<MAX_DISPLAY_LEN;i++)
		m_strText[i-1] = L"";

	for(int i=1;i<MAX_BUTTON_NUMBER;i++)
		m_strButtonText[i-1] = L"";
}

//-----------------------------------------------------------------------------
//!	\Get display text string line
//!	\param	line - show text line id
//-----------------------------------------------------------------------------
LPCTSTR CDisplay::GetLine(const int line)
{
	return m_strText[line-1];
}
	
//-----------------------------------------------------------------------------
//!	\Get display button text string
//!	\param	index - button index id
//-----------------------------------------------------------------------------
LPCTSTR CDisplay::GetButtonText(const int index)
{
	return m_strButtonText[index-1];
}

//-----------------------------------------------------------------------------
//!	\Set display window
//!	\param	hwnd - window handle
//-----------------------------------------------------------------------------
void CDisplay::SetDisplayWnd(HWND hwnd)
{
	m_hDisplayWnd = hwnd;
	m_bPINOK = FALSE;

	m_bStopDisplay = FALSE;
}

//-----------------------------------------------------------------------------
//!	\Set Key return value
//!	\param	value - return value
//-----------------------------------------------------------------------------
void CDisplay::SetKeyReturn(int value)
{
	m_iKeyReturn = value;

	if(m_hEvent != 0)
		SetEvent(m_hEvent);
	ClearScreen();
}

//-----------------------------------------------------------------------------
//!	\Get current mask 
//-----------------------------------------------------------------------------
int CDisplay::GetMask()
{
	return m_iMask;
}

//-----------------------------------------------------------------------------
//!	\Get current display button number
//-----------------------------------------------------------------------------
int CDisplay::GetBtNumber()
{
	return m_iButtonNumber;

}
	
//-----------------------------------------------------------------------------
//!	\Get PINOK flag
//-----------------------------------------------------------------------------
BOOL CDisplay::GetPINOK()
{
	return m_bPINOK;
}

//-----------------------------------------------------------------------------
//!	\Set PINOK flag
//-----------------------------------------------------------------------------
void CDisplay::SetPINOK(BOOL flag)
{
	m_bPINOK = flag;
}

//-----------------------------------------------------------------------------
//!	\Select display language
//-----------------------------------------------------------------------------
int CDisplay::SelectLanguage()
{
	Clear();
	if(m_hEvent == 0)
		m_hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

	m_iMask = MASK_CANCEL;
	m_iButtonNumber = 2;
	m_strButtonText[0] = L" English";
	m_strButtonText[1] = L" Francais";
	m_strText[1] = CLanguage::GetText(CLanguage::IDX_SELECT_LANGUAGE);//L"Select language";
	m_strText[2]= L"";//L"Choisir langue";

	int timer = 30;
	Display();
	if (WAIT_TIMEOUT == WaitForSingleObject(m_hEvent,timer*1000))
		return KEY_TIMEOUT;
	else
		return m_iKeyReturn;
}

//-----------------------------------------------------------------------------
//!	\show pass to clerk string on screen
//-----------------------------------------------------------------------------
int CDisplay::PassToClerk()
{
	return 0;
	Clear();
	if(m_hEvent == 0)
		m_hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	m_iButtonNumber = 0;
	m_strText[1] = CLanguage::GetText(CLanguage::IDX_PASS_TO_CLERK);
	m_strText[2] = CLanguage::GetText(CLanguage::IDX_DO_NOT_REMOVE);
	m_strText[3] = CLanguage::GetText(CLanguage::IDX_CARD);

	int timer = 3;
	Display();
	if (WAIT_TIMEOUT == WaitForSingleObject(m_hEvent,timer*1000))
		return KEY_TIMEOUT;
	else
		return m_iKeyReturn;
}

//-----------------------------------------------------------------------------
//!	\show pass to customer k string on screen
//-----------------------------------------------------------------------------
int CDisplay::PassToCustomer()
{
	Clear();
	if(m_hEvent == 0)
		m_hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	m_iButtonNumber = 0;
	m_strText[1] = CLanguage::GetText(CLanguage::IDX_PASS_TO);
	m_strText[2] = CLanguage::GetText(CLanguage::IDX_CUSTOMER);
	m_strText[3] = L"";

	CLanguage::SetCustomerFlag(TRUE);
	int timer = 3;
	Display();
	if (WAIT_TIMEOUT == WaitForSingleObject(m_hEvent,timer*1000))
		return KEY_TIMEOUT;
	else
		return m_iKeyReturn;
}

