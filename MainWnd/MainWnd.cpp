// MainWnd.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "MainWnd.h"
#include "Source\\UI\\ECRiComm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

typedef struct _REG_TZI_FORMAT
{
    LONG Bias;
    LONG StandardBias;
    LONG DaylightBias;
    SYSTEMTIME StandardDate;
    SYSTEMTIME DaylightDate;
} REG_TZI_FORMAT;

// CMainWndApp

BEGIN_MESSAGE_MAP(CMainWndApp, CWinApp)
END_MESSAGE_MAP()


// CMainWndApp construction
CMainWndApp::CMainWndApp()
	: CWinApp()
{
}

void CMainWndApp::MoveXACWindow()
{
/*	m_hXACwnd = FindWindow(L"Dialog",NULL);
	if ( m_hXACwnd == NULL)
		return;
#ifdef _DEBUG
	MoveWindow(m_hXACwnd,0,0,320,240,FALSE);
#else
	MoveWindow(m_hXACwnd,400,400,320,240,FALSE);
#endif
*/
}
// The one and only CMainWndApp object
CMainWndApp theApp;

// CMainWndApp initialization
BOOL CMainWndApp::InitInstance()
{
	if (!AfxSocketInit())
	{
		AfxMessageBox(IDP_SOCKETS_INIT_FAILED);
		return FALSE;
	}

	SetRegistryKey(_T("Local AppWizard-Generated Applications"));
	CSetTimeZone(L"GMT Standard Time");

	MoveXACWindow();

	CECRiCommDlg dlg;

	m_pMainWnd = &dlg;
	INT_PTR nResponse = dlg.DoModal();
	if (nResponse == IDOK)
	{
	}
	// Since the dialog has been closed, return FALSE so that we exit the
	//  application, rather than start the application's message pump.
	return FALSE;
}

//-----------------------------------------------------------------------------
// CMainWndApp Exit entry
//-----------------------------------------------------------------------------
int CMainWndApp::ExitInstance()
{
//	if ( m_hXACwnd != NULL)
//		MoveWindow(m_hXACwnd,0,0,320,240,TRUE);
	return CWinApp::ExitInstance();
}
	
void CMainWndApp::MoveBackWnd()
{
//	if ( m_hXACwnd != NULL)
//		MoveWindow(m_hXACwnd,0,0,320,240,TRUE);
}

//-----------------------------------------------------------------------------
//CMainWndApp Set time Zone
//param	subkey - CString timezone name
//-----------------------------------------------------------------------------
BOOL CMainWndApp::CSetTimeZone(CString subKey)
{
      HKEY hKey;
     TIME_ZONE_INFORMATION tziNew;
     REG_TZI_FORMAT regTZI;
     DWORD dwBufLen=sizeof(regTZI);
     LONG lRet;
     CString keyName = _T("Time Zones\\") + subKey;
     CString strStd = _T("");
     CString strDlt = _T("");

     unsigned char szData[256];
     DWORD dwDataType, dwBufSize;
     dwBufSize = 256;

     lRet = RegOpenKeyEx(HKEY_LOCAL_MACHINE,keyName,0,KEY_QUERY_VALUE,&hKey ); 
     if(lRet != ERROR_SUCCESS )
	 {
         return FALSE;
	 }

     lRet = RegQueryValueEx(hKey,TEXT("TZI"),NULL,NULL,(LPBYTE)&regTZI, &dwBufLen);
     if(RegQueryValueEx(hKey,_T("Dlt"),0,&dwDataType,szData, &dwBufSize) == ERROR_SUCCESS)
	 {
         strDlt = (LPCTSTR)szData;
	 }

     dwBufSize = 256;
     if(RegQueryValueEx(hKey,_T("Std"),0,&dwDataType,szData,&dwBufSize) == ERROR_SUCCESS)
	 {
         strStd = (LPCTSTR)szData;
	 }
     RegCloseKey(hKey);

     if( (lRet != ERROR_SUCCESS) || (dwBufLen > sizeof(regTZI)))
	 {
         return FALSE; 
	 }
     ZeroMemory(&tziNew, sizeof(tziNew));
     tziNew.Bias = regTZI.Bias;
     tziNew.StandardDate = regTZI.StandardDate;
     wcscpy(tziNew.StandardName, strStd);
     wcscpy(tziNew.DaylightName, strDlt);
     tziNew.DaylightDate = regTZI.DaylightDate;
     tziNew.DaylightBias = regTZI.DaylightBias;

     if( !SetTimeZoneInformation( &tziNew ) ) 
     {
         TRACE(_T("Failure!"));
         return FALSE;
     }
     return TRUE;
}