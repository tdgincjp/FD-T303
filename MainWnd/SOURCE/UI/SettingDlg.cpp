// SettingDlg.cpp : implementation file
//

#include "stdafx.h"
#include "..\\..\\MainWnd.h"
#include "SettingDlg.h"
#include "Language.h"
#include "..\\UTILS\\string.h"
#include "SaioBase.h"
#include "..\\data\\DebugFile.h"				//JC Nov 11/15 for DebugWrite

// CSettingDlg dialog
#define PASSWORD_INPUT				-1
#define MAIN_GUI					0
#define SHOW_HOST					1
#define SHOW_TERMINALID				2
#define SHOW_IPADDR					3
#define SHOW_PRODUCTHOST			4
#define SHOW_TESTHOST				5
#define EDIT_HOST					6
#define EDIT_TERMINALID				7
#define EDIT_STATICIP				8
#define INPUT_STATICIP				9
#define SHOW_MORE					11
#define SHOW_COPYLOG				12
#define SHOW_COPYLOGRESULTS			13
#define SHOW_DEBUGAPI				14
#define SHOW_TIMEZONE				15
#define EDIT_TIMEZONE				16


IMPLEMENT_DYNAMIC(CSettingDlg, CDialog)


//-----------------------------------------------------------------------------
//!	\CDialog class virtual function
//-----------------------------------------------------------------------------
CSettingDlg::CSettingDlg(CWnd* pParent)
	: CDialog(CSettingDlg::IDD, pParent)
{
	m_ExitFlag = FALSE;
	m_DebugApiOn = FALSE;					//JC Nov 11/15
}

//-----------------------------------------------------------------------------
//!	\CDialog class virtual function
//-----------------------------------------------------------------------------
CSettingDlg::~CSettingDlg()
{
}

//-----------------------------------------------------------------------------
//!	\CDialog class virtual function
//-----------------------------------------------------------------------------
void CSettingDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CSettingDlg, CDialog)
	ON_WM_TIMER()
	ON_WM_KEYDOWN()
END_MESSAGE_MAP()


// CSettingDlg message handlers
//-----------------------------------------------------------------------------
//!	\CDialog class virtual function
//-----------------------------------------------------------------------------
BOOL CSettingDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	screenX = GetSystemMetrics(SM_CXSCREEN);  
	screenY = GetSystemMetrics(SM_CYSCREEN);  
	MoveWindow( 0, 0, screenX, screenY);

	textsize = 160;
	m_strTitle.Create(_T(" "), WS_CHILD| WS_VISIBLE, CRect(100, 1, screenX, 30), this);
	m_strTitle.SetPointFont(true,160);
	m_strTitle.SetCaption(CLanguage::GetText(CLanguage::IDX_CONFIG_MENU));
	m_strTitle.SetColor(RGB(0, 0, 255),RGB( 255,255,255));

	m_strInputPW[0].Create(_T(" "), WS_CHILD| WS_VISIBLE, CRect(40, 80, screenX, 120), this);
	m_strInputPW[0].SetPointFont(true,160);
	m_strInputPW[0].SetCaption(CLanguage::GetText(CLanguage::IDX_ENTER_PASSWORD));
	m_strInputPW[0].SetColor(RGB(0, 0, 255),RGB( 255,255,255));

	m_strInputPW[1].Create(_T(" "), WS_CHILD| WS_VISIBLE, CRect(40, 120, screenX, 160), this);
	m_strInputPW[1].SetPointFont(true,160);
	m_strInputPW[1].SetColor(RGB(0, 0, 255),RGB( 255,255,255));

	m_iLevel = PASSWORD_INPUT;
	SetTimer(1,100,NULL);
	m_IncorrectTimes = 0;
	m_LogFile = CLogFile();
	m_strPassword = L"";
	m_bMerchantIDInputFlag = FALSE;

	ClearLastKeySequence();
	m_ExitFlag = FALSE;

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}
//-----------------------------------------------------------------------------
//!	\CDialog class virtual function
//-----------------------------------------------------------------------------
void CSettingDlg::OnCancel()
{
	m_ExitFlag = FALSE;
	CDialog::OnCancel();

}
//-----------------------------------------------------------------------------
//!	\CDialog class virtual function
//-----------------------------------------------------------------------------
void CSettingDlg::OnOK()
{
	switch(m_iLevel)
	{
	case PASSWORD_INPUT: //Password Input
		if (m_strPassword.GetLength() == 0)
		{
			Beep1();
			return;
		}
		if (!CheckPassword())
		{
			m_IncorrectTimes++;
			if (m_IncorrectTimes < 3)
			{
				m_strPassword = L"";
				m_strInputPW[1].SetCaption(L"");
				Beep1();
				return;
			}
		}
		else
		{
			CreateMainGUIBt();
			CreateHostBt();
			CreateTermIdBt();
			CreateTimeZoneBt();
			CreateRestartBt();
			CreateLogBt();
			CreateIPAddrBt();
			m_strInputPW[0].ShowWindow(SW_HIDE);
			m_strInputPW[1].ShowWindow(SW_HIDE);
			m_iLevel = MAIN_GUI;
			return;
		}
		CDialog::OnOK();
		break;
	}
}

int m_secretSeq[8] = {49, 50, 51, 52, 52, 51, 50, 49};
void CSettingDlg::ClearLastKeySequence()
{
	for (int i = 0; i < 8; i++)
		m_lastKeySeq[i] = 0; 
	m_keySeqPos = 0;
}

void CSettingDlg::AddLastKeySequence(UINT nChar)
{
	if (m_keySeqPos < 8)
	{
		m_lastKeySeq[m_keySeqPos] = nChar;
		m_keySeqPos ++;
	}
	else if (m_keySeqPos == 8)
	{
		shift_left(m_lastKeySeq, 8);
		m_lastKeySeq[7] = nChar;
	}

	if (compare_array(m_lastKeySeq, m_secretSeq, 8))
		Exit();
}

void CSettingDlg::shift_left (int myarray[], int size)
{
	int temp = myarray[0];

	for (int i=0; i<(size - 1); i++)
	{
		myarray[i] = myarray[i+1] ;
	}
	myarray[size-1] = temp;
}

BOOL CSettingDlg::compare_array(int array1[], int array2[], int size)
{
	for (int i = 0; i < size; i++)
	{
		if (array1[i] == array2[i])
			continue; 
		else
			return FALSE;
	}
	return TRUE;

}

//-----------------------------------------------------------------------------
//!	\CDialog class virtual function
//-----------------------------------------------------------------------------
void CSettingDlg::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	int can;										//JC Nov 23/15
	TCHAR wDID[SZ_DATAWIRE_DID] = _T("");			//JC Nov 23/15
	CDialog::OnKeyDown(nChar, nRepCnt, nFlags);

	if (m_iLevel != MAIN_GUI && m_iLevel != SHOW_MORE)
		ClearLastKeySequence();

	TRACE(L"Down nchar = %d \n",nChar);

	CString str = L"";
	switch( m_iLevel )
	{
	case PASSWORD_INPUT: //Password Input
		m_strInputPW[1].SetCaption(DealPassword(nChar, m_strPassword));
		break;
	case MAIN_GUI:
		switch( nChar )
		{
		case 116: //host
			ShowHost();
			break;
		case 117: //terminal ID
			can=false;
			if (CDataFile::Read(L"Datawire DID", wDID)) // DID exists so.....
				can = DisplayMIDwarning();
			if (!can)						//JC Cancel was pressed
			{
				m_bMerchantIDInputFlag = FALSE;
				ShowTermId();
			}
			break;
		case 118: //merchant ID
			can=false;
			if (CDataFile::Read(L"Datawire DID", wDID)) // DID exists so.....
				can = DisplayMIDwarning();
			if (!can)
			{
				m_bMerchantIDInputFlag = TRUE;
				ShowTermId();
			}
			break;
		case 119: //show IP address
			ShowIPAddr();
			break;
		case 115: //next page
			ShowMore();
			break;
		default:
			AddLastKeySequence(nChar);
			break;
		}
		break;
	case SHOW_MORE: // next page
		switch(nChar)
		{
		case 116: //timezone
			ShowTimeZone();
			break;
		case 117: //copy log
			ShowCopyLog();
			break;
		case 118: //debug API ON
			ShowDebugApi();					//JC Nov 11/15 Debug Api 
			break;							//JC Nov 11/15 Debug API
		case 119:// go back
			GoBack();
			break;
		default:
			AddLastKeySequence(nChar);
			break;
		}
		break;
	case SHOW_COPYLOG:
		switch (nChar)
		{
		case 115:
			ShowCopyLog(FALSE);
			break;
		case 119:
			CopyLog();
			break;
		}
		break;
	case SHOW_COPYLOGRESULTS:
		switch(nChar)
		{
		case 115:
			ShowCopyLogResults(FALSE, CString(""), CString(""));			
			break;
		}
		break;
	case SHOW_DEBUGAPI:
		switch (nChar)
		{
		case 115:
			ShowDebugApi(FALSE);
			break;
		case 119:
			if(m_DebugApiOn)						//JC Nov 11/15
			{
				m_DebugApiOn = FALSE;				//JC Nov 11/15
				ShowDebugApi(TRUE);
			}
			else
			{
				m_DebugApiOn = TRUE;				//JC Nov 11/15
				ShowDebugApi(TRUE);
			}
			break;
		}
		break;
	case SHOW_HOST: // host
		switch( nChar )
		{
		case 115:
			ShowHost(FALSE);
			break;
		case 119:
			ToggleHost(FALSE);			//Show Host value but don't Toggle
			break;
		}
		break;
	case SHOW_TERMINALID: //terminal id
		switch( nChar )
		{
		case 115:
			ShowTermId(FALSE);
			break;
		case 119:
			EditTermId();
			break;
		}
		break;
	case SHOW_TIMEZONE: // SHOW_TIMEZONE
		switch( nChar )
		{
		case 116:
			CDataFile::Save(L"TZ Offset",L"5");
			ShowTimeZone(FALSE);
			break;
		case 117:  
			CDataFile::Save(L"TZ Offset",L"6");
			ShowTimeZone(FALSE);
			break;
		case 118:  
			CDataFile::Save(L"TZ Offset",L"7");
			ShowTimeZone(FALSE);
			break;
		case 119:
			CDataFile::Save(L"TZ Offset",L"8");
			ShowTimeZone(FALSE);
			break;
		case 115:  
			ShowTimeZone(FALSE);
			break;
		}
		break;
	case SHOW_IPADDR: // IP address
		switch( nChar )
		{
		case 118:  //static IP ON
			CDataFile::Save(L"STATIC IP",L"On");
			str = CLanguage::GetText(CLanguage::IDX_STATIC_IP) + L": On";
			m_strIPAddrText.SetCaption(str);
			EditStaticIP();
			break;
		case 114://static IP Off
			CDataFile::Save(L"STATIC IP",L"Off");
			str = CLanguage::GetText(CLanguage::IDX_STATIC_IP) + L": Off";
			m_strIPAddrText.SetCaption(str);
			break;
		case 115:  
			ShowIPAddr(FALSE);
			break;
		}
		break;
	case EDIT_HOST: //edit host address
		switch( nChar )
		{
		case 119:						//JC Oct 28/15 modified save to change, now toggles between stg and prod
			ToggleHost(TRUE);			//JC Oct 28/15 Toggle and switch value
			break;
		case 115:						//JC Oct 28/15 Back Button
			ShowHost(TRUE);				//JC Oct 28/15
			m_iLevel = SHOW_HOST;		//JC Oct 28/15
			break;
		default:
			break;
		}
		break;
	case EDIT_STATICIP: //edit static IP
		switch( nChar )
		{
		case 116: 
		case 117: 
		case 118: 
		case 119:
			InputIP(nChar-115);
			break;
		case 115: 
			EditStaticIP(FALSE);
			break;
		}
		break;
	case INPUT_STATICIP: //input static IP
		switch( nChar )
		{
		case 119: //save
			{
				switch( m_iInputID )
				{
				case 1: 
					str = CLanguage::GetText(CLanguage::IDX_IP_ADDR);
					break;
				case 2: 
					str = CLanguage::GetText(CLanguage::IDX_IP_MASK);
					break;
				case 3: 
					str = CLanguage::GetText(CLanguage::IDX_GATEWAY);
					break;
				case 4:
					str = CLanguage::GetText(CLanguage::IDX_DNS);
					break;
				default:
					return;
				}
		
				str += L": ";
				str = m_strIPAddrText.m_szCaption.Mid(str.GetLength());

				CString str1;
				str1.Format(L"StaticIP%d",m_iInputID);
				CDataFile::Save(str1,str.GetBuffer());
				str.ReleaseBuffer();
			}
		case 115: 
			m_strTermIdBt[3].ShowWindow(SW_HIDE);
			EditStaticIP();
			break;
		case 8:
			{
				switch( m_iInputID )
				{
				case 1: 
					str = CLanguage::GetText(CLanguage::IDX_IP_ADDR);
					break;
				case 2: 
					str = CLanguage::GetText(CLanguage::IDX_IP_MASK);
					break;
				case 3: 
					str = CLanguage::GetText(CLanguage::IDX_GATEWAY);
					break;
				case 4:
					str = CLanguage::GetText(CLanguage::IDX_DNS);
					break;
				default:
					return;
				}
		
				str += L": ";
				if (m_strIPAddrText.m_szCaption.GetLength() <= str.GetLength())
				{
					Beep1();
					return;
				}
				str = m_strIPAddrText.m_szCaption;
				int len = str.GetLength();
				m_strIPAddrText.SetCaption(str.Mid(0,len-1));
			}
			break;
		default:
			{
				if (( nChar < '0' || nChar > '9') && nChar != 190)
					return;
				switch( m_iInputID )
				{
				case 1: 
					str = CLanguage::GetText(CLanguage::IDX_IP_ADDR);
					break;
				case 2: 
					str = CLanguage::GetText(CLanguage::IDX_IP_MASK);
					break;
				case 3: 
					str = CLanguage::GetText(CLanguage::IDX_GATEWAY);
					break;
				case 4:
					str = CLanguage::GetText(CLanguage::IDX_DNS);
					break;
				default:
					return;
				}
		
				str += L": ";
				if (m_strIPAddrText.m_szCaption.GetLength() - str.GetLength() >= 15)
				{
					Beep1();
					return;
				}
				str = m_strIPAddrText.m_szCaption;
				if (nChar == 190)
					nChar = '.';
				str.AppendChar(nChar);
				m_strIPAddrText.SetCaption(str);
			}
			break;
		}
		break;
	case EDIT_TERMINALID: //edit term id
		switch( nChar )
		{
		case 119: // save
			{
				str = m_strTermIdBt[4].m_szCaption;

				//if ( str.GetLength() < (m_bMerchantIDInputFlag?15:8 ))		//Checks to ensure length is not < MIN length MID = 15, TID = 8
				if ( str.GetLength() < (m_bMerchantIDInputFlag?5:1 ))		//JC Nov 9/15 Checks to ensure length is not < MIN length MID = 5, TID = 3
				{
					Beep1();
					return;
				}
				if ( m_bMerchantIDInputFlag )
				{
					CDataFile::Save(L"Datawire MID",str.GetBuffer());
					if (str != SaveMID)
					{
						CDebugFile::DebugWrite("Datawire DID: Erased due to MID change");	//JC Nov 23/15
						CDataFile::Delete(L"Datawire DID");									//JC NOv 23/15 Should only Delete DID if changes made, will force registration
					}
				}
				else
				{
					//TID is numeric 8 long, should have leading zeros
					str = L"00000000"+str;
					str = str.Right(8);
					CDataFile::Save(L"Datawire TID",str.GetBuffer());
					if (str != SaveTID) 
					{
						CDebugFile::DebugWrite("Datawire DID: Erased due to TID change");		//JC Nov 23/15
						CDataFile::Delete(L"Datawire DID");										//JC NOv 23/15 Should only Delete DID if changes made, will force registration
					}
				}
				str.ReleaseBuffer();

			}

			EditTermId(FALSE);
			break;
		case 115:
			EditTermId(FALSE);
			break;
		case 112:
//			str = CLanguage::GetText(CLanguage::IDX_TERMINAL_ID) + L": TS";
//			m_strTermIdBt[4].SetCaption(str);
			break;
		case 113:
//			str = CLanguage::GetText(CLanguage::IDX_TERMINAL_ID) + L": PS";
//			m_strTermIdBt[4].SetCaption(str);
			break;
		case 114:
//			str = CLanguage::GetText(CLanguage::IDX_TERMINAL_ID) + L": L0";
//			m_strTermIdBt[4].SetCaption(str);
			break;
		case 8:
			{
				if (m_strTermIdBt[4].m_szCaption.GetLength() <= 0)
				{
					Beep1();
					return;
				}
				str = m_strTermIdBt[4].m_szCaption;
				int len = str.GetLength();
				m_strTermIdBt[4].SetCaption(str.Mid(0,len-1));
			}
			break;
		default:
			{
				if (! m_bMerchantIDInputFlag)      //Entering TID
				{
					if ( nChar < '0' || nChar > '9')
					{
						Beep1();
						return;
					}
				}
				if (m_strTermIdBt[4].m_szCaption.GetLength() >= (m_bMerchantIDInputFlag?15:8))		//Checks if length is > MAX MID=15, TID=8
				{
					Beep1();
					return;
				}
				str = m_strTermIdBt[4].m_szCaption;
				str.AppendChar(nChar);
				m_strTermIdBt[4].SetCaption(str);
			}
			break;
		}
		break;
	}
}

//-----------------------------------------------------------------------------
//!	\CDialog class virtual function
//-----------------------------------------------------------------------------
void CSettingDlg::OnTimer(UINT_PTR nIDEvent)
{
	if(nIDEvent == 1) 
	{
		KillTimer(1);
		SetFocus();
	}
	CDialog::OnTimer(nIDEvent);
}

//-----------------------------------------------------------------------------
//!	\Show Main setting GUI
//!	\param	flag - true: show , false: hide
//-----------------------------------------------------------------------------
void CSettingDlg::ShowMainGUI(BOOL flag)
{
	if ( flag )
	{
		m_strTitle.SetCaption(CLanguage::GetText(CLanguage::IDX_CONFIG_MENU));
		m_iLevel = MAIN_GUI;
	}
	for(int i=0;i<4;i++)
		m_strBtText[i].ShowWindow(flag?SW_SHOW:SW_HIDE);
	m_strBtText[4].ShowWindow(!flag?SW_SHOW:SW_HIDE);

	m_strBtText[5].ShowWindow(flag?SW_SHOW:SW_HIDE);   //IP address button
}


//-----------------------------------------------------------------------------
//!	\Show Main setting GUI
//!	\param	flag - true: show , false: hide
//-----------------------------------------------------------------------------
void CSettingDlg::ShowMore(BOOL flag)
{
	m_strBtText[0].SetCaption(CLanguage::GetText(CLanguage::IDX_COPY_LOG));
	m_strBtText[1].SetCaption(CLanguage::GetText(CLanguage::IDX_DEBUG_API));			//JC NOV 11/15 ADD DEBUG API - will write the next 20 api req/resp pairs to debug.txt
	m_strBtText[2].SetCaption(CLanguage::GetText(CLanguage::IDX_GO_BACK));
	m_strBtText[3].SetCaption(CLanguage::GetText(CLanguage::IDX_TIMEZONE_OFFSET));
	m_strBtText[5].ShowWindow(SW_HIDE);
	m_iLevel = SHOW_MORE;
}
	
//-----------------------------------------------------------------------------
//!	\Show Host setting GUI
//!	\param	flag - true: show , false: hide
//-----------------------------------------------------------------------------
void CSettingDlg::ShowHost(BOOL flag)
{
	m_strTermIdBt[3].SetCaption(CLanguage::GetText(CLanguage::IDX_EDIT));

	TCHAR buf[24]={0};
	CString temp = L"";
	CString temp1 = L"prod.dw.us.fdcnet.biz";    //JC Oct 28/15 "stg.dw.us.fdcnet.biz"
	
	temp= CLanguage::GetText(CLanguage::IDX_HOST_ADDRESS) + L": ";
	if(CDataFile::Read(L"DataWire SD URL", buf))
		temp1 = CString(buf);
	m_strHostBt[0].SetCaption(temp1);
	m_strHostBt[1].SetCaption(temp);

	for(int i=0;i<2;i++)
		m_strHostBt[i].ShowWindow(flag?SW_SHOW:SW_HIDE);

	m_strTermIdBt[3].ShowWindow(flag?SW_SHOW:SW_HIDE);

	if ( !flag )
	{
		ShowMainGUI(TRUE);
		return;
	}
	ShowMainGUI(FALSE);
	m_strTitle.SetCaption(CLanguage::GetText(CLanguage::IDX_HOST_SETUP));
	m_iLevel = SHOW_HOST;
}	

//-----------------------------------------------------------------------------
//!	\Copy Log setting GUI
//!	\param	flag - true: show , false: hide
//-----------------------------------------------------------------------------
void CSettingDlg::ShowCopyLog(BOOL flag)
{
	m_strTermIdBt[3].SetCaption(CLanguage::GetText(CLanguage::IDX_COPY));

	CString temp = L"Copy Log File to Disk?";
	CString temp1 = L"usb-root/download/";
	
	m_strCopyLog[0].SetCaption(temp1);
	m_strCopyLog[1].SetCaption(temp);

	for(int i=0;i<2;i++)
		m_strCopyLog[i].ShowWindow(flag?SW_SHOW:SW_HIDE);

	m_strTermIdBt[3].ShowWindow(flag?SW_SHOW:SW_HIDE);

	if ( !flag )
	{
		ShowMainGUI(TRUE);
		ShowMore();
		return;
	}
	ShowMainGUI(FALSE);
	m_strTitle.SetCaption(CLanguage::GetText(CLanguage::IDX_COPY_LOG));
	m_iLevel = SHOW_COPYLOG;
}


//-----------------------------------------------------------------------------
//!	\Show Debug Api Menu
//!	\param	flag - true: show , false: hide
// JC Nov 11/15
// JC will set a boolean to write the next 20 API req/rsp pairs to debug.txt
// Once counter hits 20 the boolean will be set to off
//-----------------------------------------------------------------------------
void CSettingDlg::ShowDebugApi(BOOL flag)
{
	CString temp;
	CString temp1;

	if (m_DebugApiOn)	   //DEBUG IS ON												//JC Nov 11/15
	{
		m_strTermIdBt[3].SetCaption(CLanguage::GetText(CLanguage::IDX_DISABLE));		//JC Nov 11/15
		temp = L"DISABLE Debug Write?" ;
		temp1 = L"(Debug is currently ENABLED)";

	}
	else																				//JC Nov 11/15
	{
		m_strTermIdBt[3].SetCaption(CLanguage::GetText(CLanguage::IDX_ENABLE));			//JC Nov 11/15
		temp = L"ENABLE Debug Write?" ;
		temp1 = L"(Debug is currently DISABLED)";
	}
	
	m_strCopyLog[0].SetCaption(temp1);
	m_strCopyLog[1].SetCaption(temp);

	for(int i=0;i<2;i++)
		m_strCopyLog[i].ShowWindow(flag?SW_SHOW:SW_HIDE);

	m_strTermIdBt[3].ShowWindow(flag?SW_SHOW:SW_HIDE);

	if ( !flag )
	{
		ShowMainGUI(TRUE);
		ShowMore();
		return;
	}
	ShowMainGUI(FALSE);
	m_strTitle.SetCaption(CLanguage::GetText(CLanguage::IDX_DEBUG_API));
	m_iLevel = SHOW_DEBUGAPI;
}

//-----------------------------------------------------------------------------
//!	\Copy Log setting GUI
//!	\param	flag - true: show , false: hide
//-----------------------------------------------------------------------------
void CSettingDlg::ShowCopyLogResults(BOOL flag, CString line, CString line1)				//JC Nov 11/15 to add extra lines of info
{
	m_strCopyLog[0].SetCaption(line);
	m_strCopyLog[1].SetCaption(line1);


	for(int i=0;i<2;i++)
		m_strCopyLog[i].ShowWindow(flag?SW_SHOW:SW_HIDE);

	m_strTermIdBt[3].ShowWindow(SW_HIDE);								//JC Nov 11/15 to add extra lines of info

	if ( !flag )
	{
		ShowMainGUI(TRUE);
		ShowMore();
		return;
	}
	ShowMainGUI(FALSE);
	m_strTitle.SetCaption(CLanguage::GetText(CLanguage::IDX_COPY_LOG));
	m_iLevel = SHOW_COPYLOGRESULTS;
}
	
//-----------------------------------------------------------------------------
//!	\Restart GUI
//!	\param	flag - true: show , false: hide
//-----------------------------------------------------------------------------
//void CSettingDlg::ShowRestart(BOOL flag)
//{
//	m_strTermIdBt[3].SetCaption(CLanguage::GetText(CLanguage::IDX_RESTART));
//
//	CString temp = L"Restart the application?";
//	m_strRestart.SetCaption(temp);
//
//	m_strRestart.ShowWindow(flag?SW_SHOW:SW_HIDE);
//
//	m_strTermIdBt[3].ShowWindow(flag?SW_SHOW:SW_HIDE);
//
//	if ( !flag )
//	{
//		ShowMainGUI(TRUE);
//		ShowMore();
//		return;
//	}
//	ShowMainGUI(FALSE);
//	m_strTitle.SetCaption(CLanguage::GetText(CLanguage::IDX_RESTART));
//	m_iLevel = SHOW_RESTART;
//}

//-----------------------------------------------------------------------------
//!	\Show term id setting GUI
//!	\param	flag - true: show , false: hide
//-----------------------------------------------------------------------------
void CSettingDlg::ShowTermId(BOOL flag)
{
	m_strTermIdBt[3].SetCaption(CLanguage::GetText(CLanguage::IDX_EDIT));

//	for(int i=0;i<3;i++)
//		m_strTermIdBt[i].ShowWindow(SW_HIDE);

	TCHAR buf[24]={0};
	CString temp = L"";
	CString temp1 = L"";
	
	if ( !m_bMerchantIDInputFlag )
	{
		temp= CLanguage::GetText(CLanguage::IDX_TERMINAL_ID) + L": ";
		if(CDataFile::Read(L"Datawire TID", buf))
			temp1 = CString(buf);
	}
	else
	{
		temp= CLanguage::GetText(CLanguage::IDX_MERCHANT_ID) + L": ";
		if(CDataFile::Read(L"Datawire MID", buf))
			temp1 = CString(buf);
	}
	m_strTermIdBt[4].SetCaption(temp1);
	m_strTermIdBt[5].SetCaption(temp);

	for(int i=3;i<=5;i++)
		m_strTermIdBt[i].ShowWindow(flag?SW_SHOW:SW_HIDE);
	if ( !flag )
	{
		ShowMainGUI(TRUE);
		return;
	}
	ShowMainGUI(FALSE);
	m_strTitle.SetCaption(CLanguage::GetText(CLanguage::IDX_TERM_CONFIG));
	m_iLevel = SHOW_TERMINALID;
}

//-----------------------------------------------------------------------------
//!	\Show IP address setting GUI
//!	\param	flag - true: show , false: hide
//-----------------------------------------------------------------------------
void CSettingDlg::ShowIPAddr(BOOL flag)
{
	TCHAR buf[10] = {L""};
	CString temp = L"Off";
	if(CDataFile::Read(L"STATIC IP",buf))
		temp = CString(buf);

	CString temp1 = CLanguage::GetText(CLanguage::IDX_STATIC_IP) + L": "+temp;
	m_strIPAddrText.SetCaption(temp1);
	m_strIPAddrText.ShowWindow(flag?SW_SHOW:SW_HIDE);
	for(int i=0;i<2;i++)
		m_strIPAddrBt[i].ShowWindow(flag?SW_SHOW:SW_HIDE);
	for(int i=2;i<10;i++)
		m_strIPAddrBt[i].ShowWindow(SW_HIDE);
	if ( !flag )
	{
		ShowMainGUI(TRUE);
		return;
	}
	ShowMainGUI(FALSE);
	m_strTitle.SetCaption(CLanguage::GetText(CLanguage::IDX_IP_ADDR));
	m_iLevel = SHOW_IPADDR;
}
//-----------------------------------------------------------------------------
//!	\Show edit static IP setting GUI
//!	\param	flag - true: show , false: hide
//-----------------------------------------------------------------------------
void CSettingDlg::EditStaticIP(BOOL flag)
{
	TCHAR buf[128]={L""};
	if (!flag)
	{
		CString Key;
		CString str[4];
		m_ShowIPError = FALSE;
		for(int i=0;i<4;i++)
		{
			Key.Format(L"StaticIP%d",i+1);
			memset(buf,0,128);
			CDataFile::Read(Key,buf);
			str[i] = CString(buf);
			if (!IsValid_IP(buf))
			{
//				InputIP(i+1);
				m_ShowIPError = TRUE;
				break;
			}
		}
	}

	CString temp;
	memset(buf,0,128);
	CDataFile::Read(L"StaticIP1", buf);
	if (m_ShowIPError && !IsValid_IP(buf))
	{
		m_strIPAddrBt[2].SetColor(RGB(255, 0, 0), RGB(255, 255, 255));
		temp = CLanguage::GetText(CLanguage::IDX_ERROR) + L": " + CString(buf);
	}
	else
	{
		m_strIPAddrBt[2].SetColor(RGB(0, 0, 0), RGB(255, 255, 255));
		temp = CLanguage::GetText(CLanguage::IDX_IP_ADDR) + L": " + CString(buf);
	}

	m_strIPAddrBt[2].SetCaption(temp);

	memset(buf,0,128);
	CDataFile::Read(L"StaticIP2", buf);
	if (m_ShowIPError && !IsValid_IP(buf))
	{
		m_strIPAddrBt[3].SetColor(RGB(255, 0, 0), RGB(255, 255, 255));
		temp = CLanguage::GetText(CLanguage::IDX_ERROR) + L": " + CString(buf);
	}
	else
	{
		m_strIPAddrBt[3].SetColor(RGB(0, 0, 0), RGB(255, 255, 255));
		temp = CLanguage::GetText(CLanguage::IDX_IP_MASK) + L": " + CString(buf);
	}

	m_strIPAddrBt[3].SetCaption(temp);

	memset(buf,0,128);
	CDataFile::Read(L"StaticIP3", buf);
	if (m_ShowIPError && !IsValid_IP(buf))
	{
		m_strIPAddrBt[4].SetColor(RGB(255, 0, 0), RGB(255, 255, 255));
		temp = CLanguage::GetText(CLanguage::IDX_ERROR) + L": " + CString(buf);
	}
	else
	{
		m_strIPAddrBt[4].SetColor(RGB(0, 0, 0), RGB(255, 255, 255));
		temp = CLanguage::GetText(CLanguage::IDX_GATEWAY) + L": " + CString(buf);
	}

	m_strIPAddrBt[4].SetCaption(temp);

	memset(buf,0,128);
	CDataFile::Read(L"StaticIP4", buf);
	if (m_ShowIPError && !IsValid_IP(buf))
	{
		m_strIPAddrBt[5].SetColor(RGB(255, 0, 0), RGB(255, 255, 255));
		temp = CLanguage::GetText(CLanguage::IDX_ERROR) + L": " + CString(buf);
	}
	else
	{
		m_strIPAddrBt[5].SetColor(RGB(0, 0, 0), RGB(255, 255, 255));
		temp = CLanguage::GetText(CLanguage::IDX_DNS) + L": " + CString(buf);
	}

	m_strIPAddrBt[5].SetCaption(temp);

	if ( m_ShowIPError && !flag )
		return;

	/*
	CString temp = CLanguage::GetText(CLanguage::IDX_IP_ADDR) + L": ";
	if(CDataFile::Read(L"StaticIP1", buf))
		temp += CString(buf);
	m_strIPAddrBt[2].SetCaption(temp);

	temp = CLanguage::GetText(CLanguage::IDX_IP_MASK) + L": ";
	if(CDataFile::Read(L"StaticIP2", buf))
		temp += CString(buf);
	m_strIPAddrBt[3].SetCaption(temp);

	temp = CLanguage::GetText(CLanguage::IDX_GATEWAY) + L": ";
	if(CDataFile::Read(L"StaticIP3", buf))
		temp += CString(buf);
	m_strIPAddrBt[4].SetCaption(temp);

	temp = CLanguage::GetText(CLanguage::IDX_DNS) + L": ";
	if(CDataFile::Read(L"StaticIP4", buf))
		temp += CString(buf);
	m_strIPAddrBt[5].SetCaption(temp);
	*/
	m_strIPAddrText.ShowWindow(!flag?SW_SHOW:SW_HIDE);
	for(int i=0;i<2;i++)
		m_strIPAddrBt[i].ShowWindow(!flag?SW_SHOW:SW_HIDE);
	for(int i=2;i<10;i++)
		m_strIPAddrBt[i].ShowWindow(flag?SW_SHOW:SW_HIDE);

	if ( !flag )
	{
		ShowIPAddr();
		return;
	}
	m_iLevel = EDIT_STATICIP;
}
//-----------------------------------------------------------------------------
//!	\Show Time Zone setting GUI
//!	\param	flag - true: show , false: hide
//-----------------------------------------------------------------------------
void CSettingDlg::ShowTimeZone(BOOL flag)
{
	for(int i=0;i<8;i++)
		m_strTimeZoneBt[i].ShowWindow(flag?SW_SHOW:SW_HIDE);
	if ( !flag )
	{
		ShowMainGUI(TRUE);
		return;
	}
	ShowMainGUI(FALSE);
	m_strTitle.SetCaption(CLanguage::GetText(CLanguage::IDX_TIMEZONE_OFFSET));
	m_iLevel = SHOW_TIMEZONE;
}
//-----------------------------------------------------------------------------
//!	\Show edit host GUI
//!	\param	toggle - switch hosts , false just display current value
//-----------------------------------------------------------------------------
void CSettingDlg::ToggleHost(BOOL toggle)
{
	TCHAR buf[24]={0};
	CString temp = L"";    // Host address variable
	CString temp1 = L"";   // Host Address Text display
	temp1= CLanguage::GetText(CLanguage::IDX_HOST_ADDRESS) + L": ";
	if(CDataFile::Read(L"DataWire SD URL", buf)) temp += CString(buf); //get current
	if (toggle)
	{
		if (temp == "prod.dw.us.fdcnet.biz")
			temp =L"stg.dw.us.fdcnet.biz";
		else
			temp= L"prod.dw.us.fdcnet.biz";
		CDataFile::Save(L"DataWire SD URL",temp.GetBuffer());
	}
	m_strTermIdBt[3].SetCaption(CLanguage::GetText(CLanguage::IDX_CHANGE));
	m_strHostBt[0].SetCaption(temp);    //(flag?L"":temp);  //displays blanks or value dependaant on flag
	m_strHostBt[1].SetCaption(temp1);
	m_iLevel = EDIT_HOST;
}
//-----------------------------------------------------------------------------
//!	\Show edit term id setting GUI
//!	\param	flag - true: show , false: hide
//-----------------------------------------------------------------------------
void CSettingDlg::EditTermId(BOOL flag)
{
	TCHAR buf[24]={0};
	CString temp = L"";
	CString temp1 = temp;

	
	
	if ( !m_bMerchantIDInputFlag )
	{
		//int w = DisplayMIDwarning();
		temp= CLanguage::GetText(CLanguage::IDX_TERMINAL_ID) + L": ";
		temp1 = temp;
		if(CDataFile::Read(L"Datawire TID", buf))
		{
			temp += CString(buf);
			SaveTID = CString(buf);												//JC Nov 26/15 Save TID for compare
		}
		else
			SaveTID = "";														//JC Nov 26/15 Save TID for compare
	}
	else
	{
		temp= CLanguage::GetText(CLanguage::IDX_MERCHANT_ID) + L": ";
		temp1 = temp;
		if(CDataFile::Read(L"Datawire MID", buf))
		{
			temp += CString(buf);
			SaveMID = CString(buf);												//JC Nov 26/15 Save MID for compare
		}
		else																	
			SaveMID = "";														//JC Nov 26/15 Save MID for compare											
	}
	m_strTermIdBt[3].SetCaption(CLanguage::GetText(flag?CLanguage::IDX_SAVE:CLanguage::IDX_EDIT));
	m_strTermIdBt[4].SetCaption(flag?L"":temp);
	m_strTermIdBt[5].SetCaption(temp1);

//	for(int i=0;i<3;i++)
//		m_strTermIdBt[i].ShowWindow(flag?SW_SHOW:SW_HIDE);
	if ( !flag )
	{
		ShowTermId();
		return;
	}
	m_iLevel = EDIT_TERMINALID;
}
bool CSettingDlg::DisplayMIDwarning()
{
	Beep1();

	int msgboxID = ::MessageBox(
        NULL,
        (LPCWSTR)L"This terminal is already registered with FirstData\nChanging this value will require re-registration\nYou may only do this once!!!\n\nAre you sure you want to do this?\n\nPress Enter to continue\n  Or Cancel to Exit\n",
        (LPCWSTR)L"Terminal Registration",
        MB_OKCANCEL | MB_ICONWARNING
    );
	bool rv;
    switch (msgboxID)
    {
    case IDCANCEL:
        // TODO: add code
		rv=true;
        break;
    case IDOK:
        // TODO: add code
		rv=false;
        break;
    }
	SetFocus();
    return rv;
}
	
//-----------------------------------------------------------------------------
//!	\create Main setting GUI button
//-----------------------------------------------------------------------------
void CSettingDlg::CreateMainGUIBt()
{
	int xlength = 170;
	m_strBtText[0].Create(_T(" "), WS_CHILD| WS_VISIBLE, CRect(0, 80, xlength, 120), this);
	m_strBtText[0].SetPointFont(true,textsize);
	m_strBtText[0].SetCaption(CLanguage::GetText(CLanguage::IDX_TERM_CONFIG));
	m_strBtText[0].SetColor(RGB(0, 0, 0),RGB( 127,127,255));
	m_strBtText[0].m_nBtType = 1;

	m_strBtText[1].Create(_T(" "), WS_CHILD| WS_VISIBLE, CRect(0, 132, xlength, 172), this);
	m_strBtText[1].SetPointFont(true,textsize);
	m_strBtText[1].SetCaption(CLanguage::GetText(CLanguage::IDX_MERCHANT_ID));
	m_strBtText[1].SetColor(RGB(0, 0, 0),RGB( 127,127,255));
	m_strBtText[1].m_nBtType = 1;

	m_strBtText[2].Create(_T(" "), WS_CHILD| WS_VISIBLE, CRect(0, 184, xlength, 224), this);
	m_strBtText[2].SetPointFont(true,textsize);
	m_strBtText[2].SetCaption(CLanguage::GetText(CLanguage::IDX_IP_ADDR));
	m_strBtText[2].SetColor(RGB(0, 0, 0),RGB( 127,127,255));
	m_strBtText[2].m_nBtType = 1;

	m_strBtText[3].Create(_T(" "), WS_CHILD| WS_VISIBLE, CRect(0, 30, xlength, 70), this);
	m_strBtText[3].SetPointFont(true,textsize);
	m_strBtText[3].SetCaption(CLanguage::GetText(CLanguage::IDX_HOST_ADDR));
	m_strBtText[3].SetColor(RGB(0, 0, 0),RGB( 127,127,255));
	m_strBtText[3].m_nBtType = 1;

	m_strBtText[4].Create(_T(" "), WS_CHILD, CRect(screenX-80, 192, screenX, 232), this);
	m_strBtText[4].SetPointFont(true,textsize);
	m_strBtText[4].SetCaption(CLanguage::GetText(CLanguage::IDX_BACK));
	m_strBtText[4].SetColor(RGB(0, 0, 0),RGB( 127,127,255));
	m_strBtText[4].m_nBtType = 2;

	m_strBtText[5].Create(_T(" "), WS_CHILD| WS_VISIBLE,CRect(screenX-140, 184, screenX, 224) , this);
	m_strBtText[5].SetPointFont(true,textsize);
	m_strBtText[5].SetCaption(CLanguage::GetText(CLanguage::IDX_SHOW_MORE));
	m_strBtText[5].SetColor(RGB(0, 0, 0),RGB( 127,127,255));
	m_strBtText[5].m_nBtType = 2;

}

//-----------------------------------------------------------------------------
//!	\create host setting GUI button
//-----------------------------------------------------------------------------
void CSettingDlg::CreateHostBt()
{
	m_strHostBt[0].Create(_T(" "), WS_CHILD, CRect(40, 120, 320, 160), this);
	m_strHostBt[0].SetPointFont(true,textsize);

	m_strHostBt[1].Create(_T(" "), WS_CHILD, CRect(10, 80, 320, 120), this);
	m_strHostBt[1].SetPointFont(true,textsize);
}

//-----------------------------------------------------------------------------
//!	\create termid setting GUI button
//-----------------------------------------------------------------------------
void CSettingDlg::CreateTermIdBt()
{
	m_strTermIdBt[0].Create(_T(" "), WS_CHILD, CRect(screenX-80, 30, screenX, 70), this);
	m_strTermIdBt[0].SetPointFont(true,textsize);
	m_strTermIdBt[0].SetCaption(L" TS ");
	m_strTermIdBt[0].SetColor(RGB(0, 0, 0),RGB( 127,127,255));
	m_strTermIdBt[0].m_nBtType = 2;

	m_strTermIdBt[1].Create(_T(" "), WS_CHILD, CRect(screenX-80, 80, screenX, 120), this);
	m_strTermIdBt[1].SetPointFont(true,textsize);
	m_strTermIdBt[1].SetCaption(L" PS ");
	m_strTermIdBt[1].SetColor(RGB(0, 0, 0),RGB( 127,127,255));
	m_strTermIdBt[1].m_nBtType = 2;

	m_strTermIdBt[2].Create(_T(" "), WS_CHILD, CRect(screenX-80, 136, screenX, 176), this);
	m_strTermIdBt[2].SetPointFont(true,textsize);
	m_strTermIdBt[2].SetCaption(L" L0 ");
	m_strTermIdBt[2].SetColor(RGB(0, 0, 0),RGB( 127,127,255));
	m_strTermIdBt[2].m_nBtType = 2;

	m_strTermIdBt[3].Create(_T(" "), WS_CHILD, CRect(0, 192, 80, 232), this);
	m_strTermIdBt[3].SetPointFont(true,textsize);
	m_strTermIdBt[3].SetCaption(CLanguage::GetText(CLanguage::IDX_EDIT));
	m_strTermIdBt[3].SetColor(RGB(0, 0, 0),RGB( 127,127,255));
	m_strTermIdBt[3].m_nBtType = 1;

	m_strTermIdBt[4].Create(_T(" "), WS_CHILD, CRect(40, 120, 320, 160), this);
	m_strTermIdBt[4].SetPointFont(true,textsize);
	m_strTermIdBt[5].Create(_T(" "), WS_CHILD, CRect(10, 80, 320, 120), this);
	m_strTermIdBt[5].SetPointFont(true,textsize);
}

//----------------------------------------------------------------------------
//!	\error beep
//-----------------------------------------------------------------------------
void CSettingDlg::Beep1()
{
	TCHAR buf[10]={L""};
	if(CDataFile::Read(L"SOUND",buf) && CString(buf) == L"Off")
		return;
	SaioBeeper(1, 5000, 100, 0, 40);
}

//-----------------------------------------------------------------------------
//!	\create IP address setting GUI button
//-----------------------------------------------------------------------------
void CSettingDlg::CreateTimeZoneBt()
{
	m_strTimeZoneBt[0].Create(_T(" "), WS_CHILD, CRect(92, 33,screenX, 73), this);
	m_strTimeZoneBt[0].SetPointFont(true,textsize);
	m_strTimeZoneBt[0].SetCaption(L"-5 (Eastern)");

	m_strTimeZoneBt[1].Create(_T(" "), WS_CHILD, CRect(92, 83, screenX, 123), this);
	m_strTimeZoneBt[1].SetPointFont(true,textsize);
	m_strTimeZoneBt[1].SetCaption(L"-6 (Central)");

	m_strTimeZoneBt[2].Create(_T(" "), WS_CHILD, CRect(92, 139, screenX, 179), this);
	m_strTimeZoneBt[2].SetPointFont(true,textsize);
	m_strTimeZoneBt[2].SetCaption(L"-7 (Mountain)");

	m_strTimeZoneBt[3].Create(_T(" "), WS_CHILD, CRect(92, 195, screenX-80, 235), this);
	m_strTimeZoneBt[3].SetPointFont(true,textsize);
	m_strTimeZoneBt[3].SetCaption(L"-8 (Pacific)");

	m_strTimeZoneBt[4].Create(_T(" "), WS_CHILD, CRect(0, 26, 90, 66), this);
	m_strTimeZoneBt[4].SetPointFont(true,textsize);
	m_strTimeZoneBt[4].SetCaption(CLanguage::GetText(CLanguage::IDX_SELECT));
	m_strTimeZoneBt[4].SetColor(RGB(0, 0, 0),RGB( 127,127,255));
	m_strTimeZoneBt[4].m_nBtType = 1;

	m_strTimeZoneBt[5].Create(_T(" "), WS_CHILD, CRect(0, 80, 90, 120), this);
	m_strTimeZoneBt[5].SetPointFont(true,textsize);
	m_strTimeZoneBt[5].SetCaption(CLanguage::GetText(CLanguage::IDX_SELECT));
	m_strTimeZoneBt[5].SetColor(RGB(0, 0, 0),RGB( 127,127,255));
	m_strTimeZoneBt[5].m_nBtType = 1;

	m_strTimeZoneBt[6].Create(_T(" "), WS_CHILD, CRect(0, 134, 90, 174), this);
	m_strTimeZoneBt[6].SetPointFont(true,textsize);
	m_strTimeZoneBt[6].SetCaption(CLanguage::GetText(CLanguage::IDX_SELECT));
	m_strTimeZoneBt[6].SetColor(RGB(0, 0, 0),RGB( 127,127,255));
	m_strTimeZoneBt[6].m_nBtType = 1;

	m_strTimeZoneBt[7].Create(_T(" "), WS_CHILD, CRect(0, 188, 90, 228), this);
	m_strTimeZoneBt[7].SetPointFont(true,textsize);
	m_strTimeZoneBt[7].SetCaption(CLanguage::GetText(CLanguage::IDX_SELECT));
	m_strTimeZoneBt[7].SetColor(RGB(0, 0, 0),RGB( 127,127,255));
	m_strTimeZoneBt[7].m_nBtType = 1;
}


//-----------------------------------------------------------------------------
//!	\create restart setting GUI button
//-----------------------------------------------------------------------------
void CSettingDlg::CreateRestartBt()
{
	m_strRestart.Create(_T(" "), WS_CHILD, CRect(40, 120, 320, 160), this);
	m_strRestart.SetPointFont(true,textsize);
}

//-----------------------------------------------------------------------------
//!	\create copy log setting GUI button
//-----------------------------------------------------------------------------
void CSettingDlg::CreateLogBt()
{
	m_strCopyLog[0].Create(_T(" "), WS_CHILD, CRect(40, 120, 320, 160), this);
	m_strCopyLog[0].SetPointFont(true,textsize);
	m_strCopyLog[1].Create(_T(" "), WS_CHILD, CRect(10, 80, 320, 120), this);
	m_strCopyLog[1].SetPointFont(true,textsize);

}
//----------------------------------------------------------------------------
//!	\brief	Check Password for clerk 
//----------------------------------------------------------------------------
BOOL CSettingDlg::CheckPassword()
{
	return m_strPassword == L"8303";
}
//----------------------------------------------------------------------------
//!	\brief	Deal password char and add to CString
//! \param	nChar is input char +  CString source
// return CString 
//----------------------------------------------------------------------------
CString CSettingDlg::DealPassword(UINT nChar,CString source)
{
	if (!(nChar>='0' && nChar<='9' || nChar==8))
	{
		CString str = source;
		for(int i=0;i<str.GetLength();i++)
			str.SetAt(i,'*');
		return str;
	}
	CString str = source;
	int len = str.GetLength();
	if (nChar == 8)
	{
		if (len>0)
			str = str.Mid(0,len-1);
		else
			Beep1();
	}
	else
		str.AppendChar(nChar);

	len = str.GetLength();
	if (len > SZ_PASSWORD_TEXT)
	{
		Beep1();
		str = source;
	}
	m_strPassword = str;

	for(int i=0;i<str.GetLength();i++)
		str.SetAt(i,'*');
	return str;

}


//----------------------------------------------------------------------------
//!	\brief	Copies the log file to usb
//----------------------------------------------------------------------------
void CSettingDlg::CopyLog()
{
	CStringArray *ResultArray = m_LogFile.CopyToUsb();
	CString Line = ResultArray->GetAt(0);
	CString Line1 = ResultArray->GetAt(1);

	ShowCopyLogResults(TRUE, Line, Line1);	
}


//----------------------------------------------------------------------------
//!	\brief	Restarts the application
//----------------------------------------------------------------------------
void CSettingDlg::Exit()
{
	LogRecord *restart_log = new LogRecord(LOG_APPLICATION, EVENT_EXIT);
	CLogFile::WriteRecord(restart_log);
    m_ExitFlag = TRUE;
	delete restart_log;
	CDialog::OnOK();
}


//----------------------------------------------------------------------------
//!	\brief	Go back to main menu
//----------------------------------------------------------------------------
void CSettingDlg::GoBack()
{
	m_strBtText[0].SetCaption(CLanguage::GetText(CLanguage::IDX_TERM_CONFIG));
	m_strBtText[1].SetCaption(CLanguage::GetText(CLanguage::IDX_MERCHANT_ID));
	m_strBtText[2].SetCaption(CLanguage::GetText(CLanguage::IDX_IP_ADDR));
	m_strBtText[3].SetCaption(CLanguage::GetText(CLanguage::IDX_HOST_ADDR));
	m_strBtText[5].ShowWindow(SW_SHOW);
	m_iLevel = MAIN_GUI;
}

//-----------------------------------------------------------------------------
//!	\create IP address setting GUI button
//-----------------------------------------------------------------------------
void CSettingDlg::CreateIPAddrBt()
{
	m_strIPAddrText.Create(_T(" "), WS_CHILD, CRect(20, 85, screenX, 135), this);
	m_strIPAddrText.SetPointFont(true,textsize+20);

	m_strIPAddrBt[0].Create(_T(" "), WS_CHILD, CRect(0, 136, 80, 176), this);
	m_strIPAddrBt[0].SetPointFont(true,textsize);
	m_strIPAddrBt[0].SetCaption(L" On ");
	m_strIPAddrBt[0].SetColor(RGB(0, 0, 0),RGB( 127,127,255));
	m_strIPAddrBt[0].m_nBtType = 1;

	m_strIPAddrBt[1].Create(_T(" "), WS_CHILD, CRect(screenX-80, 136, screenX, 176), this);
	m_strIPAddrBt[1].SetPointFont(true,textsize);
	m_strIPAddrBt[1].SetCaption(L"Off ");
	m_strIPAddrBt[1].SetColor(RGB(0, 0, 0),RGB( 127,127,255));
	m_strIPAddrBt[1].m_nBtType = 2;

	TCHAR buf[24]={L""};
	CString temp = CLanguage::GetText(CLanguage::IDX_IP_ADDR) + L": ";
	if(CDataFile::Read(L"StaticIP1", buf))
		temp += CString(buf);
	m_strIPAddrBt[2].Create(_T(" "), WS_CHILD, CRect(62, 33,screenX, 73), this);
	m_strIPAddrBt[2].SetPointFont(true,textsize);
	m_strIPAddrBt[2].SetCaption(temp);

	temp = CLanguage::GetText(CLanguage::IDX_IP_MASK) + L": ";
	if(CDataFile::Read(L"StaticIP2", buf))
		temp += CString(buf);
	m_strIPAddrBt[3].Create(_T(" "), WS_CHILD, CRect(62, 83, screenX, 123), this);
	m_strIPAddrBt[3].SetPointFont(true,textsize);
	m_strIPAddrBt[3].SetCaption(temp);

	temp = CLanguage::GetText(CLanguage::IDX_GATEWAY) + L": ";
	if(CDataFile::Read(L"StaticIP3", buf))
		temp += CString(buf);
	m_strIPAddrBt[4].Create(_T(" "), WS_CHILD, CRect(62, 139, screenX, 179), this);
	m_strIPAddrBt[4].SetPointFont(true,textsize);
	m_strIPAddrBt[4].SetCaption(temp);

	temp = CLanguage::GetText(CLanguage::IDX_DNS) + L": ";
	if(CDataFile::Read(L"StaticIP4", buf))
		temp += CString(buf);
	m_strIPAddrBt[5].Create(_T(" "), WS_CHILD, CRect(62, 195, screenX-80, 235), this);
	m_strIPAddrBt[5].SetPointFont(true,textsize);
	m_strIPAddrBt[5].SetCaption(temp);

	m_strIPAddrBt[6].Create(_T(" "), WS_CHILD, CRect(0, 30, 60, 60), this);
	m_strIPAddrBt[6].SetPointFont(true,textsize);
	m_strIPAddrBt[6].SetCaption(CLanguage::GetText(CLanguage::IDX_EDIT));
	m_strIPAddrBt[6].SetColor(RGB(0, 0, 0),RGB( 127,127,255));
	m_strIPAddrBt[6].m_nBtType = 1;

	m_strIPAddrBt[7].Create(_T(" "), WS_CHILD, CRect(0, 80, 60, 110), this);
	m_strIPAddrBt[7].SetPointFont(true,textsize);
	m_strIPAddrBt[7].SetCaption(CLanguage::GetText(CLanguage::IDX_EDIT));
	m_strIPAddrBt[7].SetColor(RGB(0, 0, 0),RGB( 127,127,255));
	m_strIPAddrBt[7].m_nBtType = 1;

	m_strIPAddrBt[8].Create(_T(" "), WS_CHILD, CRect(0, 136, 60, 166), this);
	m_strIPAddrBt[8].SetPointFont(true,textsize);
	m_strIPAddrBt[8].SetCaption(CLanguage::GetText(CLanguage::IDX_EDIT));
	m_strIPAddrBt[8].SetColor(RGB(0, 0, 0),RGB( 127,127,255));
	m_strIPAddrBt[8].m_nBtType = 1;

	m_strIPAddrBt[9].Create(_T(" "), WS_CHILD, CRect(0, 192, 60, 222), this);
	m_strIPAddrBt[9].SetPointFont(true,textsize);
	m_strIPAddrBt[9].SetCaption(CLanguage::GetText(CLanguage::IDX_EDIT));
	m_strIPAddrBt[9].SetColor(RGB(0, 0, 0),RGB( 127,127,255));
	m_strIPAddrBt[9].m_nBtType = 1;
}
//-----------------------------------------------------------------------------
//!	\Show input IP value setting GUI
//!	\param	id - 1: IP addr 2:Mask, 3: gateway, 4:DSN
//-----------------------------------------------------------------------------
void CSettingDlg::InputIP(int id)
{
	CString str;
	switch( id )
	{
	case 1: 
		str = CLanguage::GetText(CLanguage::IDX_IP_ADDR);
		break;
	case 2: 
		str = CLanguage::GetText(CLanguage::IDX_IP_MASK);
		break;
	case 3: 
		str = CLanguage::GetText(CLanguage::IDX_GATEWAY);
		break;
	case 4:
		str = CLanguage::GetText(CLanguage::IDX_DNS);
		break;
	default:
		return;
	}
	m_iInputID = id;
	str += L": ";
	m_strTermIdBt[3].SetCaption(CLanguage::GetText(CLanguage::IDX_SAVE));
	m_strTermIdBt[3].ShowWindow(SW_SHOW);
	m_strIPAddrText.SetCaption(str);
	m_strIPAddrText.ShowWindow(SW_SHOW);
	for(int i=0;i<10;i++)
		m_strIPAddrBt[i].ShowWindow(SW_HIDE);

	m_iLevel = INPUT_STATICIP;

}
BOOL CSettingDlg::ValidDigit(char *ip_str)
{
    while (*ip_str) {
        if (*ip_str >= '0' && *ip_str <= '9')
            ++ip_str;
        else
            return FALSE;
    }
    return TRUE;
}

// JC Jan 12/15 Routine to check the format of an IP address
BOOL CSettingDlg::IsValid_IP(TCHAR* pIP)
{

	char *ip_str = alloc_tchar_to_char(pIP);
    int num, dots = 0;
    char *ptr;
	char seps[]   = ".\n";
 
    if (ip_str == NULL)
        return FALSE;
 
    ptr = strtok(ip_str, seps);
 
    if (ptr == NULL)
	{
		free(ip_str);
        return 0;
	}
 
    while (ptr) {
 
        /* after parsing string, it must contain only digits */
        if (!ValidDigit(ptr))
		{
			free(ip_str);
			return 0;
		}
 
        num = atoi(ptr);
 
        /* check for valid IP */
        if (num >= 0 && num <= 255) {
            /* parse remaining string */
            ptr = strtok(NULL, seps);
            if (ptr != NULL)
                ++dots;
        } else
		{
			free(ip_str);
			return 0;
		}
    }
 
    /* valid IP string must contain 3 dots */
    if (dots != 3)
	{
		free(ip_str);
        return 0;
	}
	
	free(ip_str);
    return TRUE;
}
