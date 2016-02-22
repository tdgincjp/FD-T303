#include "StdAfx.h"
#include "winioctl.h"
#include "ntddndis.h"

#include "..\\..\\MainWnd.h"
#include "..\\DEFS\\struct.h"
#include "..\\DEFS\\CommMessage.h"
#include "..\\UTILS\\crc.h" 
#include "..\\UI\\HandShakeDlg.h"
#include "..\\UI\\RapidConnectDownloadDlg.h"
#include "..\\UI\\SaleDlg.h"
#include "..\\UI\\DisplayMessage.h"
#include "..\\UTILS\\Printer.h"
#include "..\\UTILS\\StrUtil.h"
#include "..\\DEFS\\constant.h"
#include "..\\UTILS\\TagUtil.h"
#include "..\\data\\AppData.h"
#include "..\\UI\\GetTrackData.h"
#include "..\\data\\RecordFile.h"
#include "..\\UI\\MsgDlg.h"
#include "..\\DEFS\\GTPin.h"
//#include "..\\data\\LogFile.h"

#include "ECRiComm.h"


#define WM_NOTIFY_RXCHAR				(WM_USER + 3)
#define WM_NOTIFY_RETURN				(WM_USER + 4)
#define WM_SHOW_PICTURE					(WM_USER + 5)

#pragma comment(lib,"Lib\\GTPin.lib")

#define SEND_RESPONSE_TIMEOUT  2000

#define NUMBER_OF_ECR_MESSAGES  (sizeof(EcrMsgTagTable)/sizeof(sEcrMsgProcess))

IMPLEMENT_DYNAMIC(CECRiCommDlg, CBaseDlg)

BEGIN_MESSAGE_MAP(CECRiCommDlg, CBaseDlg)
	ON_WM_DESTROY()
	ON_WM_COPYDATA()
END_MESSAGE_MAP()

//-----------------------------------------------------------------------------
//!	\CDialog class virtual function
//-----------------------------------------------------------------------------
CECRiCommDlg::CECRiCommDlg(CWnd* pParent )
	: CBaseDlg(CBaseDlg::IDD, pParent)
{
	 dwBytesToWrite = BUFFERSIZE - 1;
	 m_bHavePrint = FALSE;

	 m_iPressKeyTime = 0;
	 m_iExitPressKeyTime = 0;
	 m_iShowInfoTime = 0;

	 m_iModeState = 1;

	 m_hTxnWnd = NULL;

}

//-----------------------------------------------------------------------------
//!	\CDialog class virtual function
//-----------------------------------------------------------------------------
CECRiCommDlg::~CECRiCommDlg(void)
{
}

//-----------------------------------------------------------------------------
//!	\CDialog class virtual function
//-----------------------------------------------------------------------------
void CECRiCommDlg::DoDataExchange(CDataExchange* pDX)
{
	CBaseDlg::DoDataExchange(pDX);
}

//-----------------------------------------------------------------------------
//!	\CDialog class virtual function
//-----------------------------------------------------------------------------
BOOL CECRiCommDlg::OnInitDialog()
{
//	CDataFile::Delete(L"Datawire DID"); 
	DbgWriteOn = FALSE;					//JC Feb 2/15 write to debug port on for now
	DbgNext20 = FALSE;					//JC Nov 11/15 is On will write next 20 api command pairs to debug.txt
	DbgNext20Count =0;					//JC Nov 11/15
	isDevMode = FALSE;					//AM June 2015, will enable the cancel 5 times, and will print build number to screen on startup. 
	display_message = FALSE;
	DWORD language = ENGLISH;											  //JP April 9/15 to fix issues after firmware re-load
    	CDataFile::Save(FID_CFG_LANGUAGE, (BYTE *)&language, sizeof(DWORD));  //JP April 9/15 to fix issues after firmware re-load

//CDataFile::Save(L"Datawire MID", L"RCTST0000000514");
//CDataFile::Save(L"Datawire TID", L"00000003");

	TCHAR buf [SZ_ECR_HOST_SELECTION+1] = {0};																						//JC Jan 14/15
	DWORD size = 0;			
//	if (CDataFile::Read(L"DEV_MODE", buf))
//		isDevMode = TRUE;
	CDataFile::Save(L"SETTLE TYPE",L"Auto");
	CDataFile::Save(L"MANUAL ENTRY",L"Off");																						//JC Jan 14/15

////////////////////////VERSION///////////////////////////////////
	// WHEN CHANGING VERSION PLEASE CHANGE HERE ONLY!!!!
	CDataFile::Save(L"VERSION",L"01.01.0000");
	CDataFile::Read(L"VERSION", buf);
	version = CString("Ver: ") + CString(buf);
	// WHEN CHANGING BUILD NUMBER PLEASE CHANGE HERE
	version = version + CString(" Build 117");									//JC Nov 16/15
//////////////////////////////////////////////////////////////////

	m_Title = L"     ";					//JC Jan 1/15 remove UPT from Title
	CBaseDlg::OnInitDialog();
	m_strMsg.SetCaption(version);
	m_strMsg.ShowWindow(SW_SHOW);
	m_strMsg1.SetCaption(CLanguage::GetText(CLanguage::IDX_INITIALIZING));
	m_strMsg1.ShowWindow(SW_SHOW);

	hCom = NULL;
	dwCommWatchThreadID  = 0;
	hCommWatchThread = NULL;

	CheckNetWorkStatus();
    
	SetTimer(1,1000,NULL);

	BackLightCtrl(0,TRUE,TRUE,FALSE); 

	crcInit();

	m_iExit = 0;
	m_bBusy = FALSE;
	m_bTransactionInProgress = FALSE;  //JC Flag used to indicate host transaction is in process wraps around dlg.domodal

	if (DbgWriteOn) OpenCommDbg();				//JC Feb 2/15 open debug write for now....

	m_str1 = CString("");
	m_str2 = CString("");
	m_str3 = CString("");

	LogRecord *StartLog = new LogRecord(LOG_APPLICATION, EVENT_START);
	if(CDataFile::Read(L"DATAWIRE TID",buf))
		memcpy(StartLog->TermID, alloc_tchar_to_char(buf), SZ_DATAWIRE_TID);
	if(CDataFile::Read(L"DATAWIRE TID",buf))
		memcpy(StartLog->TermID, alloc_tchar_to_char(buf), SZ_DATAWIRE_TID);
	if(SaioGetInfo((BYTE*)&m_INFO))
	{
		m_INFO.save();
		memcpy(StartLog->PinpadSerial, m_INFO.SNNumber, SZ_INFO_SN_NUMBER);
	}
	CLogFile::WriteRecord(StartLog);
	delete StartLog;

	return TRUE;  // return TRUE unless you set the focus to a control
}

//-----------------------------------------------------------------------------
//!	\CDialog class virtual function
//-----------------------------------------------------------------------------
void CECRiCommDlg::OnCancel()
{
	Beep1();
	if (isDevMode)
		m_iExit++;	
	if( m_iExit >= 5 )
	{
		CloseComm();
		CDialog::OnCancel();
	}
	else if( m_iExit == 1)
		SetTimer(3,2000,NULL);
}

//-----------------------------------------------------------------------------
//!	\CDialog class virtual function
//-----------------------------------------------------------------------------
void CECRiCommDlg::OnOK()
{
	Beep1();
}

//-----------------------------------------------------------------------------
//!	\Check Mode State
//-----------------------------------------------------------------------------
void CECRiCommDlg::CheckModeState()
{
	BYTE  info[44];
	memset(info,0,44);
	PinOpen();
	DWORD ret=0;
	ret = SREDGetInfo(info);
	PinClose();
	if (ret == 0)
		m_iModeState = info[39];

#ifdef _DEBUG
	m_iModeState = 1;
#endif

	if ( m_iModeState != 1)
	{
		m_strMsg.m_nFormat = DT_LEFT;
		m_strMsg1.m_nFormat = DT_LEFT;
		m_strMsg2.m_nFormat = DT_LEFT;
		m_strMsg.SetColor(RGB(255, 0, 0), RGB(255, 255, 255));
		m_strMsg1.SetColor(RGB(255, 0, 0), RGB(255, 255, 255));
		m_strMsg2.SetColor(RGB(255, 0, 0), RGB(255, 255, 255));
		m_strMsg.SetCaption(CLanguage::GetText(CLanguage::IDX_ERROR));
		CString str = L"";
		switch (m_iModeState)
		{
		case 0:
			str = L"Init state";
			break;
		case 2:
			str = L"Tamper state";
			break;
		case 4:
			str = L"Un_Auth state";
			break;
		default:
			str = L"Stop state";
			break;
		}
		m_strMsg1.SetCaption(str);
		m_strMsg2.SetCaption(L"");
	}
}


//-----------------------------------------------------------------------------
//!	\CDialog class virtual function, for timer run
//-----------------------------------------------------------------------------
void CECRiCommDlg::OnTimer(UINT_PTR nIDEvent)
{
	if(nIDEvent == 1) // init timer
	{
		KillTimer(1);
		//Check Run/Stop mode
		CheckModeState();
		if ( m_iModeState != 1)
		{
			Sleep(1000);
			return;
		}
		SetTimer(5,5000,NULL);

		if (GetAsyncKeyState(49) < 0)
		{
			if (GetAsyncKeyState(119) < 0)
			{
				TCHAR buf[100] = {L""};
				CString tempbei = L"Off";
				if(CDataFile::Read(L"STATIC IP",buf))
					tempbei = CString(buf);

				CSettingDlg dlg;
				dlg.DoModal();
				if (dlg.m_ExitFlag)
				{
					CDialog::OnOK();
					((CMainWndApp*)AfxGetApp())->MoveBackWnd();
					return;
				}
				if (dlg.m_DebugApiOn)				//JC Nov 11/15 API Debug is ON from settings.dlg
				{
					DbgNext20 = TRUE;				//JC Nov 11/15 API set DbgNext20 to TRUE..
					DbgNext20Count =0;				//JC Set to zero just to be sure....
				}

				CString temp = L"Off";
				if(CDataFile::Read(L"STATIC IP",buf))
					temp = CString(buf);

				if (temp == L"Off")
				{
					if (tempbei == L"On")
					{
						m_NetworkAdapter.SetDHCP(TRUE);
						m_NetworkAdapter.RebindAdapter();
					}
				}
				else
				{
					CString str[4];
					TCHAR buf1[128]={L""};
					BOOL emptyflag = FALSE;
					for(int i=0;i<4;i++)
					{
						CString Key;
						Key.Format(L"StaticIP%d",i+1);
						memset(buf1,0,128);
						CDataFile::Read(Key,buf1);
						str[i] = CString(buf1);

						char* p = alloc_tchar_to_char(buf1);
						if ( !IsValid_IP(p))
						{
							emptyflag = TRUE;
							free(p);
							break;
						}
						free(p);
					}

					if ( emptyflag )
					{
						CDataFile::Save(L"STATIC IP",L"Off");
						if (tempbei == L"On")
						{
							m_NetworkAdapter.SetDHCP(TRUE);
							m_NetworkAdapter.RebindAdapter();
						}
					}
					else
					{
//						if (tempbei == L"Off" || matchflag)
						{
							m_NetworkAdapter.SetRegIP(str[0],str[1],str[2],str[3]);
							m_NetworkAdapter.SetDHCP(FALSE);
							m_NetworkAdapter.RebindAdapter();
						}
					}
				}

				m_strMsg.SetCaption(version);
				m_strMsg1.SetCaption(CLanguage::GetText(CLanguage::IDX_INITIALIZING));
				SetTimer(4,500,NULL);
				return;
			}
		}
		InitEmv();
		OpenComm();
		Sleep(4000);	//JC June 23/15 to let Version number display on boot for a bit
		m_strMsg.SetCaption(L"");
		m_strMsg1.SetCaption(L"       WELCOME ");
		m_strMsg2.SetCaption(L"");
	}

	if(nIDEvent == 2) // show pic timer
	{
		KillTimer(2);
		OnShowPic(m_iPicture);
	}

	if(nIDEvent == 3) // Exit flag
	{
		KillTimer(3);
		m_iExit = 0;
	}

	if(nIDEvent == 4) 
	{
		KillTimer(4);

		InitEmv();
		OpenComm();
		Sleep(4000); // Remove when EMV is active. 
		m_strMsg.SetCaption(L"");
		m_strMsg1.SetCaption(L"       WELCOME ");
		m_strMsg2.SetCaption(L"");
	}

	if(nIDEvent == 5) // Check Mode state
	{
		//Check Run/Stop mode
		CheckModeState();
		if ( m_iModeState == 1)
			return;
		KillTimer(5);
	}

	if ( nIDEvent == 99 ) // network status check timer
	{
		if ( m_iModeState == 3) //stop mode
		{
//			if (GetAsyncKeyState(112) < 0)
			{
				if (GetAsyncKeyState(119) < 0)
				{
					if (GetAsyncKeyState(112) < 0) // in some special case,all 8 button's state < 0, avoid automatically run install client
						return;

					if ( ++m_iPressKeyTime < 5)
						return;
					PROCESS_INFORMATION pi; 
					CString FileName = L"\\SAIO\\user programs\\ServiceTools_CLIENT.exe";
					BOOL fRet = CreateProcess(FileName, NULL, NULL, NULL, NULL , NULL, NULL, NULL, NULL, &pi);

					if(fRet == TRUE)
					{
					   WaitForSingleObject(pi.hProcess,INFINITE);
					   CloseHandle(pi.hThread);
					   CloseHandle(pi.hProcess);

					   SaioShutdown(SAIO_SYS_REBOOT);
					}

					CDialog::OnOK();
					return;
				}
				else
					m_iPressKeyTime = 0;
			}

			{  // right up gray key be pressed more than 5 sec, exit to XAC menu. Jun 2015/12/07 build#019
				if (GetAsyncKeyState(112) < 0)
				{
					if (GetAsyncKeyState(119) < 0) // in some special case,all 8 button's state < 0, avoid automatically run install client
						return;

					if ( ++m_iExitPressKeyTime < 5)
						return;

					CDialog::OnOK();
					return;
				}
				else
					m_iExitPressKeyTime = 0;
			}
		}
		else if ( m_iModeState == 1 )
		{
				if (GetAsyncKeyState(112) < 0)
				{
					if ( ++m_iShowInfoTime < 5)
						return;
					CMsgDlg dlg;
					dlg.DoModal();
					return;
				}
				else
					m_iShowInfoTime = 0;

		}
		CheckNetWorkStatus();
		return;
	}

	if ( nIDEvent == SEND_RESPONSE_TIMEOUT ) //comm port time out timer
	{
		if ( m_iCommSendTime >= 3 )
		{
			KillTimer(SEND_RESPONSE_TIMEOUT);
			DoReversal();
			return;
		}
		ComWrite(hCom, gbBuffer, dwBytesToWrite);
		m_iCommSendTime++;
TRACE(L"ComWrite [%d] Time = %d \n", dwBytesToWrite,m_iCommSendTime);

		return;
	}

	CBaseDlg::OnTimer(nIDEvent);
}

//-----------------------------------------------------------------------------
//!	\Open RS232 Comm port for receive date
//-----------------------------------------------------------------------------
void CECRiCommDlg::OpenComm()
{
	TCHAR ComPort[6]=_T("COM1:");
	hCom = ComOpen(ComPort);
	if (hCom == NULL)
	{
		return;
	}
	DCB dcb;
	BOOL bRet = GetCommState(hCom->hComPort, &dcb);
	if (!bRet)
		return;

	dcb.BaudRate = CBR_19200;
	dcb.ByteSize = 8;

	dcb.Parity = NOPARITY;
	dcb.StopBits = ONESTOPBIT;


	dcb.fOutxCtsFlow = 0;
	dcb.fRtsControl = 0;
	dcb.fOutX = 0;
	dcb.fInX = 0;

	dcb.fDtrControl =0;
	dcb.XonLim = 0x800;
	dcb.XoffLim = 0x200;
	dcb.XonChar = 0x11;
	dcb.XoffChar = 0x13;
	dcb.ErrorChar = 0x3f;
	dcb.EofChar = 0x1a;
	dcb.EvtChar = 0x00;
	bRet = SetCommState(hCom->hComPort, &dcb);
	if (!bRet)
		return ;

	hCommWatchThread = CreateThread(NULL, 0,ThreadProc, (PVOID)this, 0, NULL);
	SetThreadPriority(hCommWatchThread, THREAD_PRIORITY_ABOVE_NORMAL);		
}

//-----------------------------------------------------------------------------
//!	\Open RS232 Comm port for debug 
//-----------------------------------------------------------------------------
void CECRiCommDlg::OpenCommDbg()
{
	TCHAR ComPort[6]=_T("COM2:");
	hComDbg = ComOpen(ComPort);
	if (hComDbg == NULL)
	{
		return;
	}
	DCB dcb;
	BOOL bRet = GetCommState(hComDbg->hComPort, &dcb);
	if (!bRet)
		return;

	dcb.BaudRate = CBR_19200;
	dcb.ByteSize = 8;

	dcb.Parity = NOPARITY;
	dcb.StopBits = ONESTOPBIT;


	dcb.fOutxCtsFlow = 0;
	dcb.fRtsControl = 0;
	dcb.fOutX = 0;
	dcb.fInX = 0;

	dcb.fDtrControl =0;
	dcb.XonLim = 0x800;
	dcb.XoffLim = 0x200;
	dcb.XonChar = 0x11;
	dcb.XoffChar = 0x13;
	dcb.ErrorChar = 0x3f;
	dcb.EofChar = 0x1a;
	dcb.EvtChar = 0x00;
	bRet = SetCommState(hComDbg->hComPort, &dcb);
	if (!bRet)
		return ;

}
//-----------------------------------------------------------------------------
//!	\Parse write debug data to Comm port
//!	\param	buf - BYTE* debug output data 
//!	\param	len - data length
//-----------------------------------------------------------------------------
void CECRiCommDlg::WriteCommDbg(BYTE *buf, unsigned long int len )
{
	if (DbgWriteOn)
	{
		ComWrite(hComDbg, buf, len);
	}
}

//-----------------------------------------------------------------------------
//!	\RS232 Comm port receive thread entry
//-----------------------------------------------------------------------------
DWORD WINAPI CECRiCommDlg::ThreadProc (LPVOID lpArg)
{
	CECRiCommDlg* dlg = reinterpret_cast<CECRiCommDlg*>(lpArg);
	dlg->Recv_Proc();
	return 0;
}

//-----------------------------------------------------------------------------
//!	\Parse Comm data thread entry
//-----------------------------------------------------------------------------
DWORD WINAPI CECRiCommDlg::ThreadProc1 (LPVOID lpArg)
{
	CECRiCommDlg* dlg = reinterpret_cast<CECRiCommDlg*>(lpArg);
	dlg->DealCommData();
	return 0;
}

//-----------------------------------------------------------------------------
//!	\RS232 Comm port receive thread
//-----------------------------------------------------------------------------
void CECRiCommDlg::Recv_Proc()
{
    LPCTSTR    pszTitle = _T("Watch Thread");
    HANDLE    HandlesToWaitFor[2];
    DWORD    dwHandleSignaled;
    COMSTAT    ComStat;
    DWORD    dwEvtMask, dwBytes;


    // We will be waiting on these objects.
    HandlesToWaitFor[0] = hCom->hCloseEvent;
    HandlesToWaitFor[1] = hCom->ovWatch.hEvent;

    // Set the comm mask so we receive signals.
    //if (!SetCommMask(hCom->hComPort, EV_RXCHAR | EV_CTS | EV_DSR))
    if (!SetCommMask(hCom->hComPort, EV_RXCHAR))
        {
        goto EndWatchThread;
        }

    // Keep looping until we break out.
    while (TRUE)
        {
        if (!WaitCommEvent(hCom->hComPort, &dwEvtMask, &hCom->ovWatch))
        //if (!WaitCommEvent(hCom->hComPort, &dwEvtMask, NULL))
            {
            if (GetLastError() != ERROR_IO_PENDING)
                {
                goto EndWatchThread;
                }

            // Wait until some event occurs (data to read; error; stopping).
            dwHandleSignaled = WaitForMultipleObjects(2, HandlesToWaitFor,    FALSE, INFINITE);

            // Which event occured?
            switch (dwHandleSignaled)
                {
            case WAIT_OBJECT_0:        // Signal to end the thread.
                // Time to exit.
                goto EndWatchThread;
            case WAIT_OBJECT_0 + 1: // CommEvent signaled.
                // fall througt
                break;
            default:                // This case should never occur.
                goto EndWatchThread;
                }
            }

            ClearCommError(hCom->hComPort, &dwBytes, &ComStat);
            if (ComStat.cbInQue != 0)
            {
                DWORD dwBytesRead;
                // wait until all data read
                do
                {
                    dwBytesRead = ComStat.cbInQue;
                    //Sleep(40);
                    Sleep(40);
                    ClearCommError(hCom->hComPort, &dwBytes, &ComStat);
                } while (dwBytesRead != ComStat.cbInQue);
                //PostMessage(hWnd, WM_NOTIFY_RXCHAR, ComStat.cbInQue, NULL);
				::SendMessage(m_hWnd, WM_NOTIFY_RXCHAR, ComStat.cbInQue, NULL);
            }
        } // End of while(TRUE) loop.

    // Time to clean up Read Thread.
EndWatchThread:

    PurgeComm(hCom->hComPort, PURGE_RXABORT | PURGE_RXCLEAR);

    CloseHandle(hCommWatchThread);
    hCommWatchThread = NULL;
    dwCommWatchThreadID = 0;

    return;
}

//-----------------------------------------------------------------------------
//!	\CBaseDlg class virtual function
//-----------------------------------------------------------------------------
void CECRiCommDlg::Done()
{
}

//-----------------------------------------------------------------------------
//!	\Send Response code to RS232 comm port
//!	\param	vale - 1: NAK, 2:ACK 
//-----------------------------------------------------------------------------
void CECRiCommDlg::SendResponse(int value)
{
	BYTE temp;
	switch (value)
	{
	case 1:
		temp = NAK;
	TRACE(L"SendResponse NAK [%X] \n",temp);
		break;
	case 2:
		temp = ACK;
		dwBytesToWrite = 0;
	TRACE(L"SendResponse ACK [%X] \n",temp);
		break;
	default:
		return;
	}
	ComWrite(hCom, (void*)&temp, 1);
}

//-----------------------------------------------------------------------------
//!	\CDialog class virtual function
//-----------------------------------------------------------------------------
LRESULT CECRiCommDlg::DefWindowProc(UINT message, WPARAM wParam, LPARAM lParam) 
{
	CString str = L"", strTmp;
 //   DWORD    dwBytesToWrite = BUFFERSIZE - 1;

	int iTime = 0;
	switch (message)
	{
	case WM_NOTIFY_RXCHAR:

		DWORD dwBytesRead, DataRead;

		dwBytesRead = wParam;
		if (dwBytesRead > BUFFERSIZE)
		{
			break;
		}
		if (!ComRead(hCom, ReadBuffer, dwBytesRead, &DataRead))
		{
			break;
		}

		if (DataRead)
		{
			if(DataRead == 1)
			{
				if( ReadBuffer[0] == ACK && dwBytesToWrite != 0)
				{
					dwBytesToWrite = 0;
					KillTimer(SEND_RESPONSE_TIMEOUT);
					m_iCommSendTime = 0;

					if ( m_hTxnWnd )
					{
						::PostMessage(m_hTxnWnd,ERROR_MESSAGE, 2,9);
						m_hTxnWnd = NULL;
					}
TRACE(L"Read ACK, ComWrite [%d] Time = %d \n", dwBytesToWrite,m_iCommSendTime);
				}
				else if(ReadBuffer[0] == NAK && dwBytesToWrite != 0)
				{
					KillTimer(SEND_RESPONSE_TIMEOUT);
					if ( m_iCommSendTime >= 3 )
					{
						DoReversal();
						return NULL;
					}
					ComWrite(hCom, gbBuffer, dwBytesToWrite);
					m_iCommSendTime ++;
					SetTimer(SEND_RESPONSE_TIMEOUT,3000,NULL);
TRACE(L"Read NAK, ComWrite [%d] Time = %d \n", dwBytesToWrite,m_iCommSendTime);

				}
				return NULL;
			}

			ReadBuffer[DataRead] = 0;

			if(DataRead <= 9 )  //message mini length
				return NULL;

			if(!CheckCRC((UCHAR*)ReadBuffer,(int)DataRead))
			{
				SendResponse(1); //NAK
				return NULL;
			}
			else
				SendResponse(2); //ACK


			m_DataRead = DataRead;
			HANDLE hThread = CreateThread(NULL, 0,ThreadProc1, (PVOID)this, 0, NULL);
			CloseHandle(hThread);

//			ParseCommData(ReadBuffer,DataRead);
		}
		return NULL;
	case WM_NOTIFY_RETURN:						// JC Event posted so we send data back
		if (dwBytesToWrite != 0)
		{
			ComWrite(hCom, gbBuffer, dwBytesToWrite);
			m_iCommSendTime = 0;
			SetTimer(SEND_RESPONSE_TIMEOUT,3000,NULL);
			TRACE(L"ComWrite %d \n", dwBytesToWrite);
		}
		//if (display_message)					//JC Nov 11/15 Removed ....
		//{
		//	display_message = FALSE;			
		//	m_bTransactionInProgress = TRUE;
		//	CDisplayMsgDlg display; 
		//	display.SetString(m_str1, m_str2, m_str3);
		//	display.DisplayForTime(15000);
		//	display.DoModal();
		//	m_bTransactionInProgress = FALSE;
		//}
		return NULL;
	case WM_SHOW_PICTURE:
		OnShowPic((int)wParam);
		return NULL;
	default:
		return CDialog::DefWindowProc(message, wParam, lParam);
	}	
	return CDialog::DefWindowProc(message, wParam, lParam);
}

//-----------------------------------------------------------------------------
//!	\Close RS232 Comm port
//-----------------------------------------------------------------------------
void CECRiCommDlg::CloseComm() 
{
	if (hCommWatchThread != NULL)
    {
        // Signal the event to close the worker threads.
        SetEvent(hCom->hCloseEvent);

        // Wait 10 seconds for it to exit.  Shouldn't happen.
        if (hCommWatchThread != NULL &&
                WaitForSingleObject(hCommWatchThread, 300) == WAIT_TIMEOUT)
        {
            // Purge all outstanding reads
            PurgeComm(hCom->hComPort, PURGE_RXABORT | PURGE_RXCLEAR);

            TerminateThread(hCommWatchThread, 0);

            // The ReadThread cleans up these itself if it terminates
            // normally.
            CloseHandle(hCommWatchThread);
            hCommWatchThread = NULL;
            dwCommWatchThreadID = 0;
        }
    }
    ComClose(hCom);
    hCom = NULL;
}

//-----------------------------------------------------------------------------
//!	\Parse change ASC code to Byte buffer
//!	\param	strAsc - ASC code string 
//!	\param	lpBuffer - Byte buffer point
//!	\param	lpByteLen - Byte buffer length
//-----------------------------------------------------------------------------
void CECRiCommDlg::AscToByte(LPCTSTR strAsc, BYTE *lpBuffer, LPDWORD lpByteLen)
{
    DWORD ix;
    CString tmp;

    DWORD len = wcslen(strAsc);
    DWORD i;

    if (len > *lpByteLen)
    {
        *lpByteLen = 0;
    }
    
    for (i=0, ix=0; i<len; i++)
    {
        lpBuffer[ix] = strAsc[i] & 0xff;
        ix++;
    }
    *lpByteLen = len;
}

//-----------------------------------------------------------------------------
//!	\Check network is connected or not
//-----------------------------------------------------------------------------
void CECRiCommDlg::CheckNetWorkStatus()
{
	if (m_NetworkAdapter.IsAdapterAvaiable())
	{
		if (m_NetworkAdapter.IsEthernetConnected() == 1)
		{
			if ( GetDlgItem(IDC_CONNECT1)->IsWindowVisible() == FALSE)
			{
				CDataFile::m_bConnected = TRUE;
				((CMainWndApp*)AfxGetApp())->m_bConnect = TRUE;
				GetDlgItem(IDC_CONNECT1)->ShowWindow(SW_SHOW);
				GetDlgItem(IDC_DISCONNECT1)->ShowWindow(SW_HIDE);
			}
			return;
		}
	}
	if ( GetDlgItem(IDC_DISCONNECT1)->IsWindowVisible() == FALSE )
	{
		CDataFile::m_bConnected = FALSE;
		((CMainWndApp*)AfxGetApp())->m_bConnect = FALSE;
		GetDlgItem(IDC_CONNECT1)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_DISCONNECT1)->ShowWindow(SW_SHOW);
	}
}

//-----------------------------------------------------------------------------
//!	\CDialog class virtual function
//-----------------------------------------------------------------------------
void CECRiCommDlg::OnDestroy()
{
	CBaseDlg::OnDestroy();
	CloseComm();
}

//-----------------------------------------------------------------------------
//!	\Parse Set static IP address
//!	\param	buf - came from configuration request, "D" for Dynamic DNS
//        "S" for static IP, format is "IP,mask,gateway,dsn,"
//-----------------------------------------------------------------------------
BOOL CECRiCommDlg::SetIPAddress(char* buf)
{
	BOOL    result = TRUE;
	DWORD value=1;
	if(buf[0] == 'D')
	{
		m_NetworkAdapter.SetDHCP(TRUE);
		m_NetworkAdapter.RebindAdapter();
		return result;
	}

	if(buf[0] != 'S')
		return FALSE;

	CString str[4];
	if(buf[1] != ',')
		return FALSE;

	char* p = &buf[2];
	char* p1 = strchr( p,',');

	if( p1 == NULL )
		return FALSE;

	*p1 = 0;

	str[0] = CString(p);

	p = p1+1;
	p1 = strchr(p,',');

	if( p1 == NULL )
		return FALSE;

	*p1 = 0;
	str[1] = CString(p);

	p = p1+1;
	p1 = strchr(p,',');

	if( p1 == NULL )
		return FALSE;

	*p1 = 0;
	str[2] = CString(p);

	p = p1+1;
	str[3] = CString(p);
					
	m_NetworkAdapter.SetRegIP(str[0],str[1],str[2],str[3]);
	m_NetworkAdapter.SetDHCP(FALSE);
	m_NetworkAdapter.RebindAdapter();
	return result;
}

//-----------------------------------------------------------------------------
//!	\Reconnect device network Adaper
//! \param adaptername - adapter name buffer point
//-----------------------------------------------------------------------------
void CECRiCommDlg::RebindAdapter( TCHAR *adaptername )
{
      HANDLE hNdis;
      BOOL fResult = FALSE;
      int count;
      // Make this function easier to use - hide the need to have two null characters.
      int length = wcslen(adaptername);
      int AdapterSize = (length + 2) * sizeof( TCHAR );
      TCHAR *Adapter = (TCHAR*)malloc(AdapterSize);
      wcscpy( Adapter, adaptername );
      Adapter[ length ] = '\0';
      Adapter[ length +1 ] = '\0';
      hNdis = CreateFile(DD_NDIS_DEVICE_NAME, 
                  GENERIC_READ | GENERIC_WRITE,
                  FILE_SHARE_READ | FILE_SHARE_WRITE, 
                  NULL, 
                  OPEN_ALWAYS,
                  0, 
                  NULL);

      if (INVALID_HANDLE_VALUE != hNdis)
      {
            fResult = DeviceIoControl(hNdis, 
                        IOCTL_NDIS_REBIND_ADAPTER, 
                        Adapter, 
                        AdapterSize,
                        NULL,
                        0,
                       (LPDWORD)&count,
                        NULL);
 
            if( !fResult )
            {
                  RETAILMSG( 1, (TEXT("DeviceIoControl failed %d\n"), GetLastError() ));
            }
            CloseHandle(hNdis);
      }
      else
      {
            RETAILMSG( 1, (TEXT("Failed to open NDIS Handle\n")));
      }
}

//-----------------------------------------------------------------------------
//!	\Check CRC value
//! \param buf - CRC value buffer
//! \param len - CRC value buffer's length
//-----------------------------------------------------------------------------
BOOL CECRiCommDlg::CheckCRC(UCHAR* buf, int len)
{
	BYTE temp[4];
	crc* p = (crc*)temp;
	*p = crcFast(buf,len - 4);

	for(int i=0;i<4;i++)
	{
		if(temp[i] != buf[len-4+i])
			return FALSE;
	}
	return TRUE;
}

//-----------------------------------------------------------------------------
//!	\Show picture on screen
//! \param index - picture index that came from picture name "MAP%d.BMP"
//-----------------------------------------------------------------------------
void CECRiCommDlg::OnShowPic(int index) 
{
	CClientDC LCDdc(this);
	CBitmap bmp;
	CString str;
	str.Format(L"\\SAIO\\user programs\\INIT\\MAP%d.BMP",index);
	HBITMAP hBmp =  SHLoadDIBitmap(str);
	bmp.Attach((HBITMAP)hBmp);
	CDC LCDdcMem;
	LCDdcMem.CreateCompatibleDC(&LCDdc);
	LCDdcMem.SelectObject(&bmp);
	LCDdc.BitBlt(0,0,320,240,&LCDdcMem,0,0,SRCCOPY);
	DeleteDC(LCDdc);
}

//-----------------------------------------------------------------------------
//!	Do EMV setting
//-----------------------------------------------------------------------------
void CECRiCommDlg::InitEmv()
{
	DWORD status;
	DWORD size;
	if ( !CDataFile::Read((int)FID_TERM_HOST_INIT_STATUS, (BYTE *)&status, &size))
		status = -1;
	if ( status != STS_DLL_OK )
		return;

	AP_FILE_HANDLER fh;
	CRecordFile HostFile;
	CMagRFID RFID;
	if ( HostFile.Open(EMV_DATA_FILE, &fh) )	// EMV download
	{
		HostFile.CloseFile(&fh);
		CEMV EMV;

		// Initialize EMV kernel
		BOOL enable = EMV.Init();
		CDataFile::Save(FID_TERM_EMV_ENABLE_FLG, (BYTE *) &enable, sizeof(BOOL)); // EMV can't work , no insert information 
					
		// Configure CTLS reader			
		enable = RFID.Config();					
		CDataFile::Save(FID_TERM_CTLS_ENABLE_FLG, (BYTE *) &enable, sizeof(BOOL));
	}
	else
	{
		
		// Configure CTLS reader (as MSR)			
		BOOL enable = RFID.ConfigMsr();					
		CDataFile::Save(FID_TERM_CTLS_ENABLE_FLG, (BYTE *) &enable, sizeof(BOOL));

	}
}
//-----------------------------------------------------------------------------
// This is the main routine to Deal comm data, validate and respond to ECR requests
//
void CECRiCommDlg::DealCommData()
{
	if ( m_iModeState != 1)
		return;
	KillTimer(5);
	ParseCommData();
	SetTimer(5,5000,NULL);
}
//-----------------------------------------------------------------------------
// This is the main routine to Parse, validate and respond to ECR requests
// 
// This routine has simillar parts to routine in Emulator
// Is position here for comparison
// JC Jan 12/15
//
void CECRiCommDlg::ParseCommData()
{
	BYTE* Buf = ReadBuffer;
	DWORD len = m_DataRead;
	ECRREC.Init();
	DWORD ret = ParseEcrRequest(Buf,len-4,&ECRREC);   // JC validate request message

	if (ret != ST_ECR_OK) // JC if message is invalid (ie. format error)
	{
		// Need case statement to set Correct Error condition
		// ie. Invalid Header
		// Edit Errors...
		// XYZ
		WriteCommDbg((BYTE *)"Bad FMT REQ --> \n", 17);
		switch (ret)
		{
			case ST_ECR_INVALID_HEADER:
				strcpy(ECRREC.ConditionCode,ERROR_FORMAT_CC_HEADER);
				break;
			case ST_ECR_INVALID_MESSAGE_LENGTH: 
				strcpy(ECRREC.ConditionCode,ERROR_FORMAT_CC_LENGTH);
				break;
			case ST_ECR_INVALID_MESSAGE_TYPE: 
				strcpy(ECRREC.ConditionCode,ERROR_FORMAT_CC_MTYPE);
				break;
			case ST_ECR_INVALID_TRAN_CODE:
				strcpy(ECRREC.ConditionCode,ERROR_FORMAT_CC_TRANCODE);
				break;
			case ST_ECR_MISSING_TAG:
				strcpy(ECRREC.ConditionCode,ERROR_FORMAT_CC_MISSING_TAG);
				break;
			case ST_ECR_INVALID_TAG_LENGTH:
				strcpy(ECRREC.ConditionCode,ERROR_FORMAT_CC_TAG_LENGTH);
				break;
			case ST_ECR_INVALID_TAG_FORMAT:
				strcpy(ECRREC.ConditionCode,ERROR_FORMAT_CC_TAG_DATA);
				break;
			case ST_ECR_INVALID_SYNC:
				strcpy(ECRREC.ConditionCode,ERROR_FORMAT_CC_MISSING_SYNC);
				break;
			case ST_ECR_INVALID_AUDIT:
				strcpy(ECRREC.ConditionCode,ERROR_FORMAT_CC_INVALID_AUDIT);
				break;
			default:
				strcpy(ECRREC.ConditionCode,ERROR_FORMAT_CC_GENERAL);
		}
		//JC Dec 6/14
		m_strTitle.ShowWindow(SW_SHOW);
		m_strMsg.ShowWindow(SW_SHOW);
		m_strMsg1.ShowWindow(SW_SHOW);
		dwBytesToWrite = BUFFERSIZE - 1;
		strcpy(ECRREC.MessageType,"RSP");
		ECRREC.EditError = TRUE;						//JC Dec 8/14 We have an edit error so return ET tag
		FormatEcrResponse(&ECRREC,gbBuffer,(int*)&dwBytesToWrite);
		memcpy(&ECRREC, &saveECRREC, sizeof(ECRREC));				//put back orginal data
		::PostMessage(m_hWnd,WM_NOTIFY_RETURN,0,0);   // JC post event so main window will send response
		return;
	}


	//
	//
	// JC Dec 6/14
	// Now we have properly formatted message so try process the request
	//
	//

	//if(strcmp(ECRREC.TranCode,"999") != 0) m_bBusy = TRUE;

	int TranC = atoi(ECRREC.TranCode);

	if(TranC < 900) //Anything under 900 are not cancel or status trancodes
	{
		m_bBusy = TRUE;
	}

	if (m_bTransactionInProgress)	//JC Dec 18/14 Transaction in progress so return "busy"
	{
		TRACE(L"TxnInProcess\n");
		dwBytesToWrite = BUFFERSIZE - 1;
		strcpy(ECRREC.MessageType,"RSP");
		
		strcpy(ECRREC.ConditionCode,ERROR_PED_BUSY);  //JC We are already doing a transaction

		if (TranC == 900)
		{
			strcpy(ECRREC.ConditionCode,ERROR_CONDITION_CODE_OK);  //JC Oct 28/15 always return 0000 We are already doing a transaction 
		}
		FormatEcrResponse(&ECRREC,gbBuffer,(int*)&dwBytesToWrite);
		memcpy(&ECRREC, &saveECRREC, sizeof(ECRREC));				//put back orginal data
		::PostMessage(m_hWnd,WM_NOTIFY_RETURN,0,0);
		if (TranC == 900)							//Cancel Txn JC Oct 28/15
		{
			INPUT ip;
 
			// Set up a generic keyboard event.
			ip.type = INPUT_KEYBOARD;
			ip.ki.wScan = 0; // hardware scan code for key
			ip.ki.time = 0;
			ip.ki.dwExtraInfo = 0;
 
			// Press the "CANCEL" key
			ip.ki.wVk = VK_CANCEL; // virtual-key code for the "a" key
			ip.ki.dwFlags = 0; // 0 for key press
			SendInput(1, &ip, sizeof(INPUT));

			// Release the "CANCEL" key
			ip.ki.dwFlags = KEYEVENTF_KEYUP; // KEYEVENTF_KEYUP for key release
			SendInput(1, &ip, sizeof(INPUT));
		}
		return;
	}
	
	memcpy(&saveECRREC, &ECRREC, sizeof(ECRREC));	//JC Oct 28/15 save current data for response
													//could be overwritten by status or cancel, if txninprocess
	if (TranC == 90) //Download
	{
		TRACE(L"Download\n");
		m_strTitle.ShowWindow(SW_SHOW);
		m_strMsg.ShowWindow(SW_SHOW);
		m_strMsg1.ShowWindow(SW_SHOW);
		m_bTransactionInProgress = TRUE;
		CRapidConnectDownloadDlg dlg;
		dlg.DoModal();
		m_bTransactionInProgress = FALSE;
		strcpy(ECRREC.ConditionCode,dlg.m_ErrCode);
		strcpy(ECRREC.RespCode,dlg.m_TRREC.RespCode);
		
		dwBytesToWrite = BUFFERSIZE - 1;
		strcpy(ECRREC.MessageType,"RSP");
		FormatEcrResponse(&ECRREC,gbBuffer,(int*)&dwBytesToWrite);
		::PostMessage(m_hWnd,WM_NOTIFY_RETURN,0,0);
	}
	
	if (TranC == 95)		//Handshake
	{
		TRACE(L"Handshake\n");
	//	m_strTitle.ShowWindow(SW_SHOW);
	//	m_strMsg.ShowWindow(SW_SHOW);
	//	m_strMsg1.ShowWindow(SW_SHOW);
	//	m_bTransactionInProgress = TRUE;
	//	CHandShakeDlg dlg;
	//	dlg.DoModal();
	//	m_bTransactionInProgress = FALSE;
		strcpy(ECRREC.ConditionCode,ERROR_CONDITION_CODE_OK);
		strcpy(ECRREC.RespCode,"000");
		dwBytesToWrite = BUFFERSIZE - 1;
		strcpy(ECRREC.MessageType,"RSP");
		FormatEcrResponse(&ECRREC,gbBuffer,(int*)&dwBytesToWrite);
		::PostMessage(m_hWnd,WM_NOTIFY_RETURN,0,0);
	}
	
	if (TranC == 999)		//Status
	{
		TRACE(L"Statsu\n");
		if (!m_bBusy)	
			strcpy(ECRREC.ConditionCode,ERROR_CONDITION_CODE_OK);
		else
			strcpy(ECRREC.ConditionCode,ERROR_PED_BUSY);   //JC Dec 18/14 We are busy processing another Trancode
		dwBytesToWrite = BUFFERSIZE - 1;
		strcpy(ECRREC.MessageType,"RSP");
		FormatEcrResponse(&ECRREC,gbBuffer,(int*)&dwBytesToWrite);
		::PostMessage(m_hWnd,WM_NOTIFY_RETURN,0,0);
	}

	if (TranC == 900)		//Cancel Transaction
	{
		TRACE(L"Cancel Track Data\n");	
		strcpy(ECRREC.ConditionCode,ERROR_CONDITION_CODE_OK);   //always return 0000
		dwBytesToWrite = BUFFERSIZE - 1;
		strcpy(ECRREC.MessageType,"RSP");
		FormatEcrResponse(&ECRREC,gbBuffer,(int*)&dwBytesToWrite);
		::PostMessage(m_hWnd,WM_NOTIFY_RETURN,0,0);
	}

	if (TranC == 0)			//sale
	{
		TRACE(L"Sale\n");
		m_strTitle.ShowWindow(SW_SHOW);
		m_strMsg.ShowWindow(SW_SHOW);
		m_strMsg1.ShowWindow(SW_SHOW);
		m_bTransactionInProgress = TRUE;
		CSaleDlg dlg(this);
		dlg.SetAmount(ECRREC.Amount);
		dlg.DoModal();
/*		strcpy(ECRREC.ConditionCode,dlg.m_ErrCode);
		strcpy(ECRREC.Customer_Text, dlg.m_Customer_Text);		//JC April 4/15 for TAG CT
		strcpy(ECRREC.RespCode,dlg.m_TRREC.RespCode);
*/		
		DWORD status;
		DWORD size;

		if (!CDataFile::Read((int)FID_TERM_HOST_INIT_STATUS, (BYTE *)&status, &size))
			status = -1;
		if ( status != STS_DLL_OK)
		{
			CRapidConnectDownloadDlg dlg;
			dlg.DoModal();
		}
		m_bTransactionInProgress = FALSE;

		if( !dlg.m_bSendECRBackFlag)
		{
			strcpy(ECRREC.ConditionCode,dlg.m_ErrCode);
			strcpy(ECRREC.Customer_Text, dlg.m_Customer_Text);		//JC April 4/15 for TAG CT
			strcpy(ECRREC.RespCode,dlg.m_TRREC.RespCode);

			dlg.GetTxnResult(&ECRREC.strTxnResult);
			dwBytesToWrite = BUFFERSIZE - 1;
			strcpy(ECRREC.MessageType,"RSP");
			FormatEcrResponse(&ECRREC,gbBuffer,(int*)&dwBytesToWrite);
			::PostMessage(m_hWnd,WM_NOTIFY_RETURN,0,0);
		}

/*		dwBytesToWrite = BUFFERSIZE - 1;
		strcpy(ECRREC.MessageType,"RSP");
		FormatEcrResponse(&ECRREC,gbBuffer,(int*)&dwBytesToWrite);
		display_message = TRUE;
		if (dlg.m_TRREC.TranStatus == ST_APPROVED)
		{
			char* pbuf;
			CLanguage::GetTextFromIndex(dlg.m_TRREC.bPartialAuth?CLanguage::IDX_PARTIAL_APPROVED:CLanguage::IDX_APPROVED,&pbuf);
			m_str1 = CString(pbuf);
			CLanguage::GetTextFromIndex(CLanguage::IDX_AUTH_CODE,&pbuf);
			m_str2.Format(L"%s : %s", CString(pbuf),CString(dlg.m_TRREC.AuthCode));
			CLanguage::GetTextFromIndex(CLanguage::IDX_INVO_NUMBER,&pbuf);
			m_str3.Format(L"%s: %s", CString(pbuf),CString(dlg.m_TRREC.InvoiceNo));
		}
		else
		{
			m_str1 = CString(ECRREC.Customer_Text);
			m_str2 = CString("Declined");
			m_str3 = CString(ECRREC.RespCode);
		}
		::PostMessage(m_hWnd,WM_NOTIFY_RETURN,0,0);
*/	}
	
	if (TranC == 82)			//Display Wallpaper
	{
		TRACE(L"Wallpaper\n");
		m_strTitle.ShowWindow(SW_HIDE);
		m_strMsg.ShowWindow(SW_HIDE);
		m_strMsg1.ShowWindow(SW_HIDE);
		m_iPicture = atoi(ECRREC.ImageIndex);
		strcpy(ECRREC.MessageType,"RSP");
		strcpy(ECRREC.ConditionCode,"000");
		dwBytesToWrite = BUFFERSIZE - 1;
		FormatEcrResponse(&ECRREC,gbBuffer,(int*)&dwBytesToWrite);
		::PostMessage(m_hWnd,WM_NOTIFY_RETURN,0,0);
		SetTimer(2,1,NULL);
	}

	if (TranC == 80)			//Get Receipt
	{
		TRACE(L"GetReceipt\n");
		m_strTitle.ShowWindow(SW_SHOW);
		m_strMsg.ShowWindow(SW_SHOW);
		m_strMsg1.ShowWindow(SW_SHOW);
		Invalidate(TRUE);
		CPrinter printer;
		TRAN_TranDataRec *pTRANDATA = &(((CMainWndApp*)AfxGetApp())->m_TRREC);
		strcpy(ECRREC.ConditionCode,"0000"); 
		
		USHORT jtc = pTRANDATA->TranCode;

		if (pTRANDATA->TranCode > FUNCTION_FINAN_BEGIN &&
			pTRANDATA->TranCode < FUNCTION_FINAN_END )//&& m_bHavePrint)
		{
			printer.m_declinedOffline = ((CMainWndApp*)AfxGetApp())->m_declinedOffline;
			printer.m_bCancelFlag = ((CMainWndApp*)AfxGetApp())->m_bCancelFlag;
			printer.GetReceipt(pTRANDATA, ECRREC.ReceiptType);								//JC May 13/15 to pass the Receipt Type to print
		}
		else
			strcpy(ECRREC.ConditionCode,ERROR_CONDITION_CODE_NO_RECEIPT); //no receipt to print error
		strcpy(ECRREC.MessageType,"RSP");
		dwBytesToWrite = BUFFERSIZE - 1;
		memcpy((void*)ECRREC.FormattedReceipt,(void*)printer.m_Value,printer.m_iValueLen);
		ECRREC.ReceiptLength = printer.m_iValueLen;
		FormatEcrResponse(&ECRREC,gbBuffer,(int*)&dwBytesToWrite);
		::PostMessage(m_hWnd,WM_NOTIFY_RETURN,0,0);
	}

	if (TranC == 79)		//Configuration
	{
		TRACE(L"Configuration\n");
		// - Need to use config info from ERCREC.
		// HostSelection, HostSelectionPort, HostSelectionIsURL (Boolean T or F, If T the this is URL, if F it is an IP address
		// HostSelection2, HostSelection2Port, HostSelection2IsURL
		// ParamHostSelection, ParamHostSelectionPort (port isn't needed to do HTTPS:// but for completion it has been included			
		// ParamHostSelectionIsURL
		m_strTitle.ShowWindow(SW_SHOW);
		m_strMsg.ShowWindow(SW_SHOW);
		m_strMsg1.ShowWindow(SW_SHOW);
		Invalidate(TRUE);
		strcpy(ECRREC.ConditionCode,"000");
		DWORD size;
//		int port;
		TCHAR buf [100] = {0};
		if(strlen(ECRREC.IP_Settings) > 0)
		{
			SetIPAddress(ECRREC.IP_Settings);
		}

		////// Set Primary Host values JC Jan 14/15
		if(strlen(ECRREC.HostSelection) > 0) //JC Jan 1/15 we have received a value to set.... 
			CDataFile::Save(L"HOST ADDRESS",alloc_char_to_tchar(ECRREC.HostSelection));
		else  //read the current value
		{	
			CDataFile::Read(L"HOST ADDRESS",buf);
			strcpy(ECRREC.HostSelection,alloc_tchar_to_char(buf));
			ECRREC.HostSelectionIsURL = TRUE;							//JC Jan 14/15 we need to save this value and use it correctly!!!!!!							
		}
		
		if (ECRREC.HostSelectionPort == 0)  // No primary port number in ECRREC
			CDataFile::Read(L"IP PORT",(BYTE *)&ECRREC.HostSelectionPort, &size); 
		else // We have a value to set from config message
			CDataFile::Save(L"IP PORT",(BYTE *)&ECRREC.HostSelectionPort, sizeof(int));


		////// Set Secondary Host values JC Jan 14/15
		if(strlen(ECRREC.HostSelection2) > 0) //JC Jan 1/15 we have received a value to set.... 
			CDataFile::Save(L"HOST ADDRESS2",alloc_char_to_tchar(ECRREC.HostSelection2));
		else  //read the current value
		{	
			CDataFile::Read(L"HOST ADDRESS2",buf);
			strcpy(ECRREC.HostSelection2,alloc_tchar_to_char(buf));
			ECRREC.HostSelection2IsURL = TRUE;							//JC Jan 14/15 we need to save this value and use it correctly!!!!!!							
		}
		
		if (ECRREC.HostSelection2Port == 0)  // No primary port number in ECRREC
			CDataFile::Read(L"IP PORT2",(BYTE *)&ECRREC.HostSelection2Port, &size); 
		else // We have a value to set from config message
			CDataFile::Save(L"IP PORT2",(BYTE *)&ECRREC.HostSelection2Port, sizeof(int));
		//////End Secondary Host Values

		////// Set Parameter Host values JC Jan 14/15
		if(strlen(ECRREC.ParamHostSelection) > 0) //JC Jan 1/15 we have received a value to set.... 
			CDataFile::Save(L"PARAM ADDRESS",alloc_char_to_tchar(ECRREC.ParamHostSelection));
		else  //read the current value
		{	
			CDataFile::Read(L"PARAM ADDRESS",buf);
			strcpy(ECRREC.ParamHostSelection,alloc_tchar_to_char(buf));
			ECRREC.ParamHostSelectionIsURL = TRUE;							//JC Jan 14/15 we need to save this value and use it correctly!!!!!!							
		}
		
		if (ECRREC.ParamHostSelectionPort == 0)  // No primary port number in ECRREC
			CDataFile::Read(L"PARAM PORT",(BYTE *)&ECRREC.ParamHostSelectionPort, &size); 
		else // We have a value to set from config message
			CDataFile::Save(L"PARAM PORT",(BYTE *)&ECRREC.ParamHostSelectionPort, sizeof(int));
		//////End Parameter Host Values
		
		if(strlen(ECRREC.Terminal_ID) > 0)
			CDataFile::Save(FID_MERCH_CFG_TERM_ID, (BYTE *)ECRREC.Terminal_ID, 8);
		else
			CDataFile::Read((int)FID_MERCH_CFG_TERM_ID, (BYTE*)ECRREC.Terminal_ID, &size); //JC Jan 1/15 get current Term ID 
		dwBytesToWrite = BUFFERSIZE - 1;
		strcpy(ECRREC.MessageType,"RSP");
		FormatEcrResponse(&ECRREC,gbBuffer,(int*)&dwBytesToWrite);
		::PostMessage(m_hWnd,WM_NOTIFY_RETURN,0,0);
	}

	if (TranC == 58)			//Reversal
	{
		TRACE(L"Reversal\n");
	//		OnReversal(Buf,len);
	}

	if(TranC<900) m_bBusy = FALSE;  //Anything under 900
}

//////////////////////////////////////
// JC Jan 12/15 This section of code should be more or less the same
// JC Jan 12/15 as the code in the Emulator
// JC Jan 12/15 restructured to make copying the code between projects easier
//


// A wrapper around CTagUtil::AddToTlvBuff so we can send data to CommDebug if on
//
int CECRiCommDlg::AddToTlvBuff(char *Tag, USHORT TagLen, BYTE *pTagData, BYTE *pTargetBuff, USHORT MaxBuffLen)
{
	
	if (DbgWriteOn)
	{
		CHAR dstr[2048];			//JC Feb 2/15
		sprintf(dstr,"     [T{%s} L{%d} D{%s}]\n",Tag,TagLen,pTagData);
		WriteCommDbg((BYTE *)dstr, strlen(dstr));
	}
	return CTagUtil::AddToTlvBuff(Tag, TagLen, pTagData, pTargetBuff, MaxBuffLen);
}

/* return 1 if string contain only digits, else return 0 */
BOOL CECRiCommDlg::ValidDigit(char *ip_str)
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
BOOL CECRiCommDlg::IsValid_IP(char *ip_str)
{
    int num, dots = 0;
    char *ptr;
	char seps[]   = ".\n";
 
    if (ip_str == NULL)
        return FALSE;
 
    ptr = strtok(ip_str, seps);
 
    if (ptr == NULL)
        return 0;
 
    while (ptr) {
 
        /* after parsing string, it must contain only digits */
        if (!ValidDigit(ptr))
            return FALSE;
 
        num = atoi(ptr);
 
        /* check for valid IP */
        if (num >= 0 && num <= 255) {
            /* parse remaining string */
            ptr = strtok(NULL, seps);
            if (ptr != NULL)
                ++dots;
        } else
            return FALSE;
    }
 
    /* valid IP string must contain 3 dots */
    if (dots != 3)
        return FALSE;
    return TRUE;
}

// JC Dec 31/14   
// Function checks format for HOST/Gateway value
// where:
// if first Char == I then it is an IP address "I,iii.iii.iii.111,ppppp"
//  
// if first Char == U then it is URL address URL "U,uuuuuu...uuuuu,ppppp"
//	
// ppppp is port number, is an optional value. Typically 443 for SSL.
//

BOOL CECRiCommDlg::DecodeHostAddress(BYTE *tagData, int len, BYTE *wHost, BYTE *wPort, BOOL FormatCheck)
{
	char seps[]   = ",\n";
	BYTE wtData[SZ_ECR_HOST_SELECTION+1];		//JC temporary storage
	char *type;
	char *URL;
	char *port;
//	int iPort;			//JC Feb 9/15

	BOOL FormatOK = TRUE;
	memcpy(wtData, tagData, len);
	wtData[len]=0;
	type = strtok( (char *)wtData, seps );
	URL = strtok( NULL, seps ); 
	port = strtok( NULL, seps );
	
	if (FormatCheck)
	{	
		if (! ((type[0] == 'I') || (type[0] == 'U')) ) 
			FormatOK=FALSE;
		else
		{
			if (type[0] =='I')						// JC Jan 11/15 SHould be a valid IP address
			{
				if (! (IsValid_IP(URL))) FormatOK = FALSE;
			}
			if (type[0] =='U')						// JC Jan 11/15 SHould be a valid IP address
			{
//				if (! (IsValidURL(URL))) FormatOK = FALSE;    JC Jan 11/15 
			}
		}
		if (port == NULL)
		{
			FormatOK = FALSE;
		}

	}
	else
	{
		if (type[0] == 'I')
			FormatOK = FALSE;
		else
			FormatOK = TRUE;
		strcpy((char *)wHost, URL);
		strcpy((char *)wPort, port);
	}
	return FormatOK;
}
//-----------------------------------------------------------------------------
//!	\Parse ECR request from read data from COM1
//!	\param	buf - BYTE* read data 
//!	\param	len - read data length
//!	\param	pECRREC - ECR tran record
//-----------------------------------------------------------------------------
DWORD CECRiCommDlg::ParseEcrRequest(BYTE* buf, int len, ECR_TranDataRec *pECRREC)
{
//	ECR_TranDataRec ECRREC;
	int i, step;
	CHAR dstr[512];			//JC Feb 2/15
	CHAR dstr1[512];

	


	// parse buf and save data to m_TRREC struct

	if (DbgWriteOn)					//JC Added Oct 28/15 to log raw request.
	{
		SYSTEMTIME LocalTime;
		::GetSystemTime( &LocalTime );
		CTime t1( LocalTime );
		sprintf(dstr,"%02d:%02d:%02d REQ---> [",t1.GetHour(),t1.GetMinute(),t1.GetSecond());
		WriteCommDbg((BYTE *)dstr, strlen(dstr));
		dstr1[0]=0;
		for(int j=0;j<len;j++)
		{
			// str.Format(" %02X",m_Recv[i]); JC Dec 5/14 to display characters.... more readable.
			if (buf[j] < 32)  //most likely binary field so display HEX
				sprintf(dstr,"[%02X]", buf[j]);
			else
				sprintf(dstr,"%c", buf[j]);
			strcat(dstr1,dstr);
		}
		sprintf(dstr,"%s]\n",dstr1);
		WriteCommDbg((BYTE *)dstr, strlen(dstr));
	}

	//////////////////////////////////////////////////////////////////////////////////////////////////
	//JC Nov 11/15 Wite Debug log to debug.txt
	//
	//
	//////////////////////////////////////////////////////////////////////////////////////////////////
	if (DbgNext20)
	{
		dstr1[0]=0;
		for(int j=0;j<len;j++)
		{
			// str.Format(" %02X",m_Recv[i]); JC Dec 5/14 to display characters.... more readable.
			if (buf[j] < 32)  //most likely binary field so display HEX
				sprintf(dstr,"[%02X]", buf[j]);
			else
				sprintf(dstr,"%c", buf[j]);
			strcat(dstr1,dstr);
		}
		sprintf(dstr,">>REQ>>[%s]<<<\n",dstr1);
		CDebugFile::DebugWrite(dstr);									//Write to the file
	}
	//JC Nov 11/15
	//////////////////////////////////////////////////////////////////////////////////////////////////
	
	memset(pECRREC, 0, sizeof(ECR_TranDataRec));

	// parse header
	int ret = ParseEcrHeader(buf, len, pECRREC);
	if (ret != ST_ECR_OK)
	{
		strcpy(pECRREC->EditErrorTag, TAG_ECR_MESSAGE_TYPE);
		return ret;	// JC Invalid header
	}

	unsigned short plLen = buf[3]*255+buf[4];  // get length of payload
	
	if (plLen > SZ_ECR_PAYLOAD_MAX_LENGTH)
	{
		strcpy(pECRREC->EditErrorTag, TAG_ECR_MESSAGE_TYPE);
		return ST_ECR_INVALID_MESSAGE_LENGTH;	// Bad message body length;
	}

	if (plLen > 0)	// valid length
	{
		unsigned short tag;
		int len;
		BYTE tagData[2500];
		sEcrMsgProcess *table;
		
		BYTE *pTagBuf = &buf[3];	// start of payload

		// get the message type
		len = CTagUtil::ReadFromTlvBuff(pTagBuf, TAG_ECR_MESSAGE_TYPE, tagData);
		if (len != SZ_ECR_MESSAGE_TYPE ||
				memcmp("REQ", tagData, len) != 0)
		{
			strcpy(pECRREC->EditErrorTag, TAG_ECR_MESSAGE_TYPE);
			return ST_ECR_INVALID_MESSAGE_TYPE;
		}

		// Transaction Code check that it is the correct length
		len = CTagUtil::ReadFromTlvBuff(pTagBuf, TAG_ECR_TRAN_CODE, tagData);
		if (len != SZ_ECR_TRAN_CODE)
		{
			strcpy(pECRREC->EditErrorTag, TAG_ECR_TRAN_CODE);  //JC Dec 9/14 no Trancode in message
			return ST_ECR_INVALID_TRAN_CODE;
		}
		
		// Validate transaction code is one we understand					//JC Dec 8/14
		
		memset(pECRREC->TranCode, 0, sizeof(pECRREC->TranCode));
		memcpy(pECRREC->TranCode, tagData, len);

		// Now that we know the ECR transaction code, we can search through the 
		// table to find out which tags we should look for	//JC Dec 8/14

		for (i = 0; i < NUMBER_OF_ECR_MESSAGES; i++)
		{
			if (strcmp(pECRREC->TranCode, EcrMsgTagTable[i].TranCode) == 0)
				break;
		}

		if (i == NUMBER_OF_ECR_MESSAGES) // Trandcode isn't identified.... JC Dec 6/14
		{
			strcpy(pECRREC->EditErrorTag, TAG_ECR_TRAN_CODE);		//JC Dec 6/14
			return ST_ECR_INVALID_TRAN_CODE;						// JC Dec 6/14
		}

		table = (sEcrMsgProcess *) &EcrMsgTagTable[i];

		// Now look for the Entry in the request table
		#define NUMBER_OF_REQUEST_MESSAGES  (sizeof(EcrReqTagTable)/sizeof(EcrReqTagTable[0]))
		for (i = 0; i < NUMBER_OF_REQUEST_MESSAGES; i++)
		{
			if (EcrReqTagTable[i][0] == table->ReqTblID)
				break;
		}
	
		if (i == NUMBER_OF_REQUEST_MESSAGES) //JC ??
			return ST_ECR_OK;				 //JC ?? this should never happen

		const unsigned short *tagTable = &EcrReqTagTable[i][0];

		// find the tags we need in the ECR request

		step = 1;

		if (DbgWriteOn)
		{
			SYSTEMTIME LocalTime;
			::GetSystemTime( &LocalTime );
			CTime t1( LocalTime );
			sprintf(dstr,"%02d:%02d:%02d REQ TAGS\n",t1.GetHour(),t1.GetMinute(),t1.GetSecond());
			WriteCommDbg((BYTE *)dstr, strlen(dstr));
		}

		while ((tag = tagTable[step]) != END_OF_TAGS)
		{
 
			char *tagStr = GetTagName(tag);
			int len = CTagUtil::ReadFromTlvBuff(pTagBuf, tagStr, tagData);
			// send data to debug port
			if (DbgWriteOn)
			{
				sprintf(dstr,"     [T{%s} L{%d} D{%s}]\n",tagStr,len,tagData);
				WriteCommDbg((BYTE *)dstr, strlen(dstr));
			}
			// check for mandatory tags
			if (len == 0 && IsMandatoryTag(tag))
			{
				strcpy(pECRREC->EditErrorTag, tagStr);		//JC Dec 9/14
				return ST_ECR_MISSING_TAG;
			}
			
			if (len > 0)
			{
				// check length
				if (!CheckDataLength(tag, len))
				{
					strcpy(pECRREC->EditErrorTag, tagStr);		//JC Dec 9/14
					return ST_ECR_INVALID_TAG_LENGTH;			//JC DEC 9/14
				}

				// check format
				if (!CheckDataFormat(tag, tagData, len, pECRREC))
				{
					strcpy(pECRREC->EditErrorTag, tagStr);		//JC Dec 9/14
					return ST_ECR_INVALID_TAG_FORMAT;
				}

				// save tag to structure
				SaveTag(tag, tagData, len, pECRREC);
			}
			step++;
		}
	}
	
	return ST_ECR_OK;
}

//-----------------------------------------------------------------------------
//!	\Format ECR header
//!	\param	buf - BYTE* read data 
//!	\param	len - read data length
//!	\param	pECRREC - ECR tran record
//-----------------------------------------------------------------------------
void CECRiCommDlg::FormatEcrHeader(ECR_TranDataRec *pECRREC, BYTE *buf, int *len)
{
	int currLen=0;
	
	// sync bytes "!!"
	memcpy(buf, "!!", 2);
	currLen += 2;

	// audit byte
	buf[currLen++] = pECRREC->AuditByte;

	buf[currLen++] = 0; // init Payload length = 0;
	buf[currLen++] = 0; 
	*len = currLen; // include payload length

}

//-----------------------------------------------------------------------------
//!	\Parse ECR request from read data from COM1
//!	\param	buf - BYTE* read data 
//!	\param	len - read data length
//!	\param	pECRREC - ECR tran record
//Name changed to FormatEcrResponse (was FormatEcrRespond)  JC Dec 8/14
//-----------------------------------------------------------------------------
void CECRiCommDlg::FormatEcrResponse(ECR_TranDataRec *pECRREC, BYTE* buf, int *len)
{
	int i, step;
	sEcrMsgProcess *table;
	int currLen, maxLen = *len;	// save max buf length
	unsigned short tag;
	BYTE	jtemp[SZ_ECR_HOST_SELECTION+1];			//JC Jan 1/15
	char	jHostType;								//JC Jan 9/15
	CHAR	dstr[512];								//JC Feb 2/15 for debug
	char outbuf[100];


	// format response to send back to ECR, data is from pECRREC
	
	// format header
	FormatEcrHeader(pECRREC, buf, &currLen);
//	currLen = *len;	// currLen = header length

	// format body
	for (i = 0; i < NUMBER_OF_ECR_MESSAGES; i++)
	{
		if (strcmp(EcrMsgTagTable[i].TranCode, pECRREC->TranCode) == 0)
			break;
	}
	int j = NUMBER_OF_ECR_MESSAGES; //jc
	if (i == NUMBER_OF_ECR_MESSAGES)    //At this point we have a transaction code we don't know so return general FO
		i = i - 1;						//set to last item "Unknown"

	table = (sEcrMsgProcess *) &EcrMsgTagTable[i];

	// Now look for the Entry in the request table
	int NUMBER_OF_RESPONSE_MESSAGES = NUMBER_OF_ECR_MESSAGES; //JC Nov 12/15 fix assert  req/rsp always pair so numberof is the same (sizeof(EcrReqTagTable)/sizeof(EcrRespTagTable[0]))


	for (i = 0; i < NUMBER_OF_RESPONSE_MESSAGES; i++)
	{
		if (EcrRespTagTable[i][0] == table->RespTblID)
			break;
	}
	
	if (i == NUMBER_OF_RESPONSE_MESSAGES)
	{
		assert(FALSE);										//JC Nov 11/15 was asserting with an UNKNOWN txn code
	}

	const unsigned short *tagTable = &EcrRespTagTable[i][0];

	step = 1;

	if (DbgWriteOn)
	{
		SYSTEMTIME LocalTime;
		::GetSystemTime( &LocalTime );
		CTime t1( LocalTime );
		sprintf(dstr,"%02d:%02d:%02d RESPONSE\n",t1.GetHour(),t1.GetMinute(),t1.GetSecond());
		WriteCommDbg((BYTE *)dstr, strlen(dstr));
	}

	while ((tag = tagTable[step]) != END_OF_TAGS)
	{
		switch (tag)
		{
		case MTAG_ECR_MESSAGE_TYPE:
			if (strlen(pECRREC->MessageType) != SZ_ECR_MESSAGE_TYPE)
				assert(FALSE);
			currLen = AddToTlvBuff(GetTagName(tag), SZ_ECR_MESSAGE_TYPE, (BYTE *)pECRREC->MessageType, buf + 3, maxLen - currLen);						//JC Feb 2/15 use Dbg Wrapper
			break;
		case MTAG_ECR_TRAN_CODE:
			if (!(strlen(pECRREC->TranCode) != SZ_ECR_TRAN_CODE)) //JC Dec 10/14 Will occur when error in header because we don't know Tran code yet
				currLen = AddToTlvBuff(GetTagName(tag), SZ_ECR_TRAN_CODE, (BYTE *)pECRREC->TranCode, buf + 3, maxLen - currLen);
			break;
		case MTAG_ECR_ECHO_DATA:
			if (pECRREC->EchoDataLength == 0)
				assert(FALSE);
			currLen = AddToTlvBuff(GetTagName(tag), pECRREC->EchoDataLength, (BYTE *)pECRREC->EchoData, buf+ 3, maxLen - currLen);
			break;
		case MTAG_ECR_CONDITION_CODE:
			currLen = AddToTlvBuff(GetTagName(tag), SZ_ECR_CONDITION_CODE, (BYTE *)pECRREC->ConditionCode, buf+ 3, maxLen - currLen);
			break;
		case MTAG_ECR_DL_REQUIRED:
			assert(FALSE);
			break;
		case MTAG_ECR_FORMATTED_RCPT:
			if (pECRREC->ReceiptLength == 0)
				assert(FALSE);
			currLen = AddToTlvBuff(GetTagName(tag), pECRREC->ReceiptLength, (BYTE *)pECRREC->FormattedReceipt, buf+ 3, maxLen - currLen);
			break;
		case MTAG_ECR_TRACK1:
			if (strlen(pECRREC->Track1) == 0)
				assert(FALSE);
			currLen = AddToTlvBuff(GetTagName(tag), strlen(pECRREC->Track1), (BYTE *)pECRREC->Track1, buf+ 3, maxLen - currLen);
			break;
		case MTAG_ECR_TRACK2:
			if (strlen(pECRREC->Track2) == 0)
				assert(FALSE);
			currLen = AddToTlvBuff(GetTagName(tag), strlen(pECRREC->Track2), (BYTE *)pECRREC->Track2, buf+ 3, maxLen - currLen);
			break;
		case MTAG_ECR_IMAGE_INDEX:
			if (strlen(pECRREC->ImageIndex) == 0)
				assert(FALSE);
			currLen = AddToTlvBuff(GetTagName(tag), strlen(pECRREC->ImageIndex), (BYTE *)pECRREC->ImageIndex, buf+ 3, maxLen - currLen);
			break;
		case MTAG_ECR_AMOUNT:
			if (strlen(pECRREC->Amount) == 0)
				assert(FALSE);
			currLen = AddToTlvBuff(GetTagName(tag), strlen(pECRREC->Amount), (BYTE *)pECRREC->Amount, buf+ 3, maxLen - currLen);
			break;

		case OTAG_ECR_MESSAGE_TYPE:
			if (strlen(pECRREC->MessageType) == SZ_ECR_MESSAGE_TYPE)
				currLen = AddToTlvBuff(GetTagName(tag), SZ_ECR_MESSAGE_TYPE, (BYTE *)pECRREC->MessageType, buf+ 3, maxLen - currLen);
			break;
		case OTAG_ECR_TRAN_CODE:
			if (strlen(pECRREC->MessageType) == SZ_ECR_TRAN_CODE)
				currLen = AddToTlvBuff(GetTagName(tag), SZ_ECR_TRAN_CODE, (BYTE *)pECRREC->TranCode, buf+ 3, maxLen - currLen);
			break;
		case OTAG_ECR_ECHO_DATA:
			if (pECRREC->EchoDataLength > 0)
				currLen = AddToTlvBuff(GetTagName(tag), pECRREC->EchoDataLength, (BYTE *)pECRREC->EchoData, buf+ 3, maxLen - currLen);
			break;
		case OTAG_ECR_CONDITION_CODE:
			if (strlen(pECRREC->ConditionCode) == SZ_ECR_CONDITION_CODE)
				currLen = AddToTlvBuff(GetTagName(tag), SZ_ECR_CONDITION_CODE, (BYTE *)pECRREC->ConditionCode, buf+ 3, maxLen - currLen);
			break;
		case OTAG_ECR_DL_REQUIRED:
			if (pECRREC->DL_Required)
				currLen = AddToTlvBuff(GetTagName(tag), 0, (BYTE *)"", buf+ 3, maxLen - currLen);
			break;
		case OTAG_ECR_FORMATTED_RCPT:
			if (pECRREC->ReceiptLength > 0)
				currLen = AddToTlvBuff(GetTagName(tag), pECRREC->ReceiptLength, (BYTE *)pECRREC->FormattedReceipt, buf+ 3, maxLen - currLen);
			break;
		case OTAG_ECR_TRACK1:
			if (strlen(pECRREC->Track1) > 0)
				currLen = AddToTlvBuff(GetTagName(tag), strlen(pECRREC->Track1), (BYTE *)pECRREC->Track1, buf+ 3, maxLen - currLen);
			break;
		case OTAG_ECR_TRACK2:
			if (strlen(pECRREC->Track2) > 0)
				currLen = AddToTlvBuff(GetTagName(tag), strlen(pECRREC->Track2), (BYTE *)pECRREC->Track2, buf+ 3, maxLen - currLen);
			break;

// JC Dec 30/14 Add configuration parameters on response
		case OTAG_ECR_HOST_SELECTION:
			if (pECRREC->HostSelectionIsURL)																		//JC Jan 9/15    Add I or U to response message
				jHostType = 'U';																					//JC Jan 9/15
			else																									//JC Jan 9/15
				jHostType = 'I';																					//JC Jan 9/15
			sprintf((char *)jtemp,"%c,%s,%d",jHostType ,pECRREC->HostSelection, pECRREC->HostSelectionPort);		//JC Jan 9/15
			if (strlen(pECRREC->HostSelection) > 0)
				currLen = AddToTlvBuff(GetTagName(tag), strlen((char*)jtemp), (BYTE *)jtemp, buf+ 3, maxLen - currLen);
			break;
		case OTAG_ECR_HOST_SELECTION2:
			if (pECRREC->HostSelection2IsURL)																		//JC Jan 9/15    Add I or U to response message
				jHostType = 'U';																					//JC Jan 9/15
			else																									//JC Jan 9/15
				jHostType = 'I';																					//JC Jan 9/15
			sprintf((char *)jtemp,"%c,%s,%d",jHostType ,pECRREC->HostSelection2, pECRREC->HostSelection2Port);		//JC Jan 9/15
			if (strlen(pECRREC->HostSelection2) > 0)
				currLen = AddToTlvBuff(GetTagName(tag), strlen((char*)jtemp), (BYTE *)jtemp, buf+ 3, maxLen - currLen);
			break;
		case OTAG_ECR_PARAM_HOST_SELECTION:
			if (pECRREC->ParamHostSelectionIsURL)																	//JC Jan 9/15    Add I or U to response message
				jHostType = 'U';																					//JC Jan 9/15
			else																									//JC Jan 9/15
				jHostType = 'I';																						//JC Jan 9/15
			sprintf((char *)jtemp,"%c,%s,%d",jHostType ,pECRREC->ParamHostSelection, pECRREC->ParamHostSelectionPort);	//JC Jan 9/15
			if (strlen(pECRREC->ParamHostSelection) > 0)
				currLen = AddToTlvBuff(GetTagName(tag), strlen((char*)jtemp), (BYTE *)jtemp, buf+ 3, maxLen - currLen);
			break;
		case OTAG_ECR_TERMINAL_ID:
			if (strlen(pECRREC->Terminal_ID) > 0)
				currLen = AddToTlvBuff(GetTagName(tag), strlen(pECRREC->Terminal_ID), (BYTE *)pECRREC->Terminal_ID, buf+ 3, maxLen - currLen);
			break;
		case OTAG_ECR_IP_SETTINGS:
			if (strlen(pECRREC->IP_Settings) > 0)
				currLen = AddToTlvBuff(GetTagName(tag), strlen(pECRREC->IP_Settings), (BYTE *)pECRREC->IP_Settings, buf+ 3, maxLen - currLen);
			break;

// JC Dec 30/14 End add new fields for Configuration request		
		case CTAG_ECR_MESSAGE_TYPE:
			if (strlen(pECRREC->MessageType) == SZ_ECR_MESSAGE_TYPE)
				currLen = AddToTlvBuff(GetTagName(tag), SZ_ECR_MESSAGE_TYPE, (BYTE *)pECRREC->MessageType, buf+ 3, maxLen - currLen);
			break;
		case CTAG_ECR_TRAN_CODE:
			if (strlen(pECRREC->MessageType) == SZ_ECR_TRAN_CODE)
				currLen = AddToTlvBuff(GetTagName(tag), SZ_ECR_TRAN_CODE, (BYTE *)pECRREC->TranCode, buf+ 3, maxLen - currLen);
			break;
		case CTAG_ECR_ECHO_DATA:
			if (pECRREC->EchoDataLength > 0)
				currLen = AddToTlvBuff(GetTagName(tag), pECRREC->EchoDataLength, (BYTE *)pECRREC->EchoData, buf+ 3, maxLen - currLen);
			break;
		case CTAG_ECR_CONDITION_CODE:
			if (strlen(pECRREC->ConditionCode) == SZ_ECR_CONDITION_CODE)
				currLen = AddToTlvBuff(GetTagName(tag), SZ_ECR_CONDITION_CODE, (BYTE *)pECRREC->ConditionCode, buf+ 3, maxLen - currLen);
			break;
		case CTAG_ECR_DL_REQUIRED:
			if (pECRREC->DL_Required)
				currLen = AddToTlvBuff(GetTagName(tag), 0, (BYTE *)"", buf+ 3, maxLen - currLen);	// Tag with Len=0
			break;
		case CTAG_ECR_FORMATTED_RCPT:
			if (pECRREC->ReceiptLength > 0)
				currLen = AddToTlvBuff(GetTagName(tag), pECRREC->ReceiptLength, (BYTE *)pECRREC->FormattedReceipt, buf+ 3, maxLen - currLen);
			break;
		case CTAG_ECR_TRACK1:
			if (strlen(pECRREC->Track1) > 0)
				currLen = AddToTlvBuff(GetTagName(tag), strlen(pECRREC->Track1), (BYTE *)pECRREC->Track1, buf+ 3, maxLen - currLen);
			break;
		case CTAG_ECR_TRACK2:
			if (strlen(pECRREC->Track2) > 0)
				currLen = AddToTlvBuff(GetTagName(tag), strlen(pECRREC->Track2), (BYTE *)pECRREC->Track2, buf+ 3, maxLen - currLen);
			break;
		case CTAG_ECR_EDIT_ERROR:					//JC Dec 8/14 Which Tag has the ERROR in it
			if (pECRREC->EditError)
				currLen = AddToTlvBuff(GetTagName(tag), strlen(pECRREC->EditErrorTag), (BYTE *)pECRREC->EditErrorTag, buf+ 3, maxLen - currLen);
			break;
		case CTAG_ECR_HOST_RESPONSE_CODE:					//Jun May 4/15 Host Response Code TAG
			if (pECRREC->RespCode)
				currLen = AddToTlvBuff(GetTagName(tag), strlen(pECRREC->RespCode), (BYTE *)pECRREC->RespCode, buf+ 3, maxLen - currLen);
			break;
		case CTAG_ECR_APPLICATION_INFORMATION:
			TCHAR cdataBuf[50];
			char *p;
			char TID[SZ_DATAWIRE_TID+1];
			if(CDataFile::Read(L"DATAWIRE TID",cdataBuf))
			{
				p = alloc_tchar_to_char(cdataBuf);
				if ( p != NULL)
				{
					memcpy(TID, p, SZ_DATAWIRE_TID);
					free(p);
					TID[SZ_DATAWIRE_TID] = 0;
				}
			}
			char DID[SZ_DATAWIRE_DID+1];
			if(CDataFile::Read(L"DATAWIRE DID",cdataBuf))
			{
				p = alloc_tchar_to_char(cdataBuf);
				if ( p != NULL)
				{
					memcpy(DID, p, SZ_DATAWIRE_DID);
					free(p);
					DID[SZ_DATAWIRE_DID] = 0;
				}
			}
			else
				strcpy(DID,"0000000000000000000000");
			char SERIAL[SZ_INFO_SN_NUMBER+1];
			memcpy(SERIAL, m_INFO.SNNumber, SZ_INFO_SN_NUMBER);
			char VERSION[SZ_VERSION +1];
			if(CDataFile::Read(L"VERSION",cdataBuf))
			{
				p = alloc_tchar_to_char(cdataBuf);
				if ( p != NULL)
				{
					memcpy(VERSION, p, SZ_VERSION);
					free(p);
					VERSION[SZ_VERSION] = 0;
				}
			}
			sprintf(outbuf, "%s,%s,%s,%s", VERSION, TID, SERIAL, DID);
			currLen = AddToTlvBuff(GetTagName(tag), strlen(outbuf), (BYTE *)outbuf, buf + 3, maxLen - currLen);
			break;
		case MTAG_ECR_STATUS:					//JC Dec 10/14 Return "IDLE  " may build out later
			if ( m_bBusy)
				currLen = AddToTlvBuff(GetTagName(tag), 6, (BYTE *)"BUSY  ", buf+ 3, maxLen - currLen);
			else
				currLen = AddToTlvBuff(GetTagName(tag), 6, (BYTE *)"IDLE  ", buf+ 3, maxLen - currLen);
			break;
		case CTAG_ECR_CUSTOMER_TEXT:			//JC Mar 15/15 Customer Text last displayed on PED
			currLen = AddToTlvBuff(GetTagName(tag), strlen(pECRREC->Customer_Text), (BYTE *)pECRREC->Customer_Text, buf+ 3, maxLen - currLen);
			break;

		case CTAG_ECR_INVOICE_NUMBER_CODE:					//Jun Oct 22/15 Txn result TAG
			if (pECRREC->strTxnResult.InvoiceNo)
				currLen = AddToTlvBuff(GetTagName(tag), strlen(pECRREC->strTxnResult.InvoiceNo), (BYTE *)pECRREC->strTxnResult.InvoiceNo, buf+ 3, maxLen - currLen);
			break;
		case CTAG_ECR_AUTHORIZATION_NUMBER_CODE:					//Jun Oct 22/15 Txn result TAG
			if (pECRREC->strTxnResult.AuthCode)
				currLen = AddToTlvBuff(GetTagName(tag), strlen(pECRREC->strTxnResult.AuthCode), (BYTE *)pECRREC->strTxnResult.AuthCode, buf+ 3, maxLen - currLen);
			break;
		case CTAG_ECR_CARD_TYPE_CODE:					//Jun Oct 22/15 Txn result TAG
			if (pECRREC->strTxnResult.CardType)
				currLen = AddToTlvBuff(GetTagName(tag), strlen(pECRREC->strTxnResult.CardType), (BYTE *)pECRREC->strTxnResult.CardType, buf+ 3, maxLen - currLen);
			break;
		case CTAG_ECR_CARD_PLAN_CODE:					//Jun Oct 22/15 Txn result TAG
			if (pECRREC->strTxnResult.ServType)
				currLen = AddToTlvBuff(GetTagName(tag), strlen(pECRREC->strTxnResult.ServType), (BYTE *)pECRREC->strTxnResult.ServType, buf+ 3, maxLen - currLen);
			break;
		case CTAG_ECR_CARDHOLDER_LANGUAGE_CODE:					//Jun Oct 22/15 Txn result TAG
			if (pECRREC->strTxnResult.CustLang)
				currLen = AddToTlvBuff(GetTagName(tag), strlen(pECRREC->strTxnResult.CustLang), (BYTE *)pECRREC->strTxnResult.CustLang, buf+ 3, maxLen - currLen);
			break;
		case CTAG_ECR_CARD_NUMBER_CODE:					//Jun Oct 22/15 Txn result TAG
			if (pECRREC->strTxnResult.Account)
				currLen = AddToTlvBuff(GetTagName(tag), strlen(pECRREC->strTxnResult.Account), (BYTE *)pECRREC->strTxnResult.Account, buf+ 3, maxLen - currLen);
			break;

		default:
			break;
		}
		step++;
	}
	
	// now assign payload length
	// header byte 8
	buf[3] = currLen / 256;
	buf[4] = currLen % 256;

	currLen +=  5;  // add message header length

	BYTE temp[4];
	crc* p = (crc*)temp;

	*p = crcFast(buf,currLen);
	for(int i=0;i<4;i++)
	{
		buf[currLen++] = temp[i];
	}

	*len = currLen;// + 4; // add crc length

	if (DbgWriteOn)					//JC Added Oct 28/15 to log raw reponse.
	{
		CHAR	dstr1[2048];								//JC Feb 2/15 for debug

		SYSTEMTIME LocalTime;
		::GetSystemTime( &LocalTime );
		CTime t1( LocalTime );
		sprintf(dstr,"%02d:%02d:%02d RSP---> [",t1.GetHour(),t1.GetMinute(),t1.GetSecond());
		WriteCommDbg((BYTE *)dstr, strlen(dstr));
		dstr1[0]=0;
		for(int j=0;j<currLen;j++)
		{
			// str.Format(" %02X",m_Recv[i]); JC Dec 5/14 to display characters.... more readable.
			if (buf[j] < 32)  //most likely binary field so display HEX
				sprintf(dstr,"[%02X]", buf[j]);
			else
				sprintf(dstr,"%c", buf[j]);
			strcat(dstr1,dstr);
		}
		WriteCommDbg((BYTE *)dstr1, strlen(dstr1));
		sprintf(dstr,"]\n");
		WriteCommDbg((BYTE *)dstr, strlen(dstr));
	}
	//////////////////////////////////////////////////////////////////////////////////////////////////
	//JC Nov 11/15 Wite Debug log to debug.txt
	//
	//Incrment DbgNext20Count if > 20 the turn off the debug write
	//
	//////////////////////////////////////////////////////////////////////////////////////////////////
	if (DbgNext20)
	{
		CHAR	dstr1[2048];								//JC Feb 2/15 for debug
		dstr1[0]=0;
		for(int j=0;j<currLen;j++)
		{
			// str.Format(" %02X",m_Recv[i]); JC Dec 5/14 to display characters.... more readable.
			if (buf[j] < 32)  //most likely binary field so display HEX
				sprintf(dstr,"[%02X]", buf[j]);
			else
				sprintf(dstr,"%c", buf[j]);
			strcat(dstr1,dstr);
		}
		sprintf(dstr,">>RSP>>[%s]<<<<\n",dstr1);
		DbgNext20Count += 1;
		if (DbgNext20Count > 20)
		{
			DbgNext20 = FALSE;															//Turn it off so it doesn't fill log files.
			CDebugFile::DebugWrite("Debug Api Turned off Maximum number of requests received");		//Put a Marker in the file.
		}
		CDebugFile::DebugWrite(dstr);									//Write to the file
	}
	//JC Nov 11/15
	//////////////////////////////////////////////////////////////////////////////////////////////////
}

//-----------------------------------------------------------------------------
//!	\Parse ECR header
//!	\param	buf - BYTE* read data 
//!	\param	len - read data length
//!	\param	pECRREC - ECR tran record
//-----------------------------------------------------------------------------
DWORD CECRiCommDlg::ParseEcrHeader(BYTE* buf, int len, ECR_TranDataRec *pECRREC)
{
	if (len < SZ_ECR_HEADER)
		return ST_ECR_INVALID_HEADER;

	// audit byte Pick up what ever it is in case we jump out
	pECRREC->AuditByte = buf[2];
	
	// sync bytes "!!"
	if (memcmp("!!", buf, 2) != 0)
		return ST_ECR_INVALID_SYNC;		//JC Dec 10/14 

	
	//now process Audit Byte
	if (pECRREC->AuditByte != '0' &&
			pECRREC->AuditByte != 'A' &&
			pECRREC->AuditByte != 'B' &&
			pECRREC->AuditByte != 'C')
	{
		return ST_ECR_INVALID_AUDIT;
	}

	return ST_ECR_OK;
}

//-----------------------------------------------------------------------------
//!	\Check ECR request Data length
//!	\param	tag - request data tag index 
//!	\param	len - tag length
//-----------------------------------------------------------------------------
BOOL CECRiCommDlg::CheckDataLength(int tag, int len)
{
	BOOL ret = TRUE;
	switch (tag)
	{
	case MTAG_ECR_MESSAGE_TYPE:	// F3
	case OTAG_ECR_MESSAGE_TYPE:
	case CTAG_ECR_MESSAGE_TYPE:
		if (len != SZ_ECR_MESSAGE_TYPE)
			ret = FALSE;
		break;
	case MTAG_ECR_TRAN_CODE:	// F3
	case OTAG_ECR_TRAN_CODE:
	case CTAG_ECR_TRAN_CODE:
		if (len != SZ_ECR_TRAN_CODE)
			ret = FALSE;
		break;
	case MTAG_ECR_ECHO_DATA:	// V50
	case OTAG_ECR_ECHO_DATA:
	case CTAG_ECR_ECHO_DATA:
		if (len > SZ_ECR_ECHO_DATA_MAX)
			ret = FALSE;
		break;
	case MTAG_ECR_CONDITION_CODE:	// F3
	case OTAG_ECR_CONDITION_CODE:
	case CTAG_ECR_CONDITION_CODE:
		if (len != SZ_ECR_CONDITION_CODE)
			ret = FALSE;
		break;
	case MTAG_ECR_AMOUNT:	// V9
	case OTAG_ECR_AMOUNT:
	case CTAG_ECR_AMOUNT:
		if (len > SZ_ECR_AMOUNT_MAX)
			ret = FALSE;
		break;
	case MTAG_ECR_IP_SETTINGS:	// V49
	case OTAG_ECR_IP_SETTINGS:
	case CTAG_ECR_IP_SETTINGS:
		if (len > SZ_ECR_IP_SETTINGS)
			ret = FALSE;
		break;
	case MTAG_ECR_HOST_SELECTION:	// V108
	case OTAG_ECR_HOST_SELECTION:
	case CTAG_ECR_HOST_SELECTION:
	case OTAG_ECR_HOST_SELECTION2:					//JC Dec 31/14 Check Max Length
	case OTAG_ECR_PARAM_HOST_SELECTION:				//JC Dec 31/14 Check Max Length
		if (len > SZ_ECR_HOST_SELECTION)
			ret = FALSE;
		break;
	case OTAG_ECR_TERMINAL_ID:						//JC Jan 2/15 Add Terminal ID
		if (len > SZ_ECR_TERMINAL_ID)
			ret = FALSE;
		break;
	case MTAG_ECR_IMAGE_INDEX:	// F2
	case OTAG_ECR_IMAGE_INDEX:
	case CTAG_ECR_IMAGE_INDEX:
		if (len != SZ_ECR_IMAGE_INDEX)
			ret = FALSE;
		break;
	case MTAG_ECR_STATUS:							//JC Dec 10/14 added for completeness should never see status word in request
		if (len != SZ_ECR_STATUS)
			ret = FALSE;
		break;
	case OTAG_ECR_RECEIPT_TYPE:							//JC May 13/15 added to match API, Receipt type to print either "C" - Cardholder or "M" - Merchant
		if (len != SZ_ECR_RECEIPT_TYPE)
			ret = FALSE;
		break;
	default:
		assert(FALSE);
	}
	return ret;
}

//-----------------------------------------------------------------------------
//!	\Check ECR request Data Format
//!	\param	tag - request data tag index 
//!	\param	data - request data tag data buffer point
//!	\param	len - tag length
//!	\param	pECRREC - ECR_TranDataRec buffer point
//-----------------------------------------------------------------------------
BOOL CECRiCommDlg::CheckDataFormat(int tag, BYTE *data, int len, ECR_TranDataRec *pECRREC)
{
	BOOL ret = TRUE;
	char *wstrHost = NULL;
	char *wstrPort = NULL;
	BOOL isURL=FALSE;

	switch (tag)
	{
	case MTAG_ECR_MESSAGE_TYPE:	// A
	case OTAG_ECR_MESSAGE_TYPE:
	case CTAG_ECR_MESSAGE_TYPE:
		if (!STR_IsAlpha((char *)data,(USHORT) len))
			ret = FALSE;
		break;
	case MTAG_ECR_TRAN_CODE:	// N
	case OTAG_ECR_TRAN_CODE:
	case CTAG_ECR_TRAN_CODE:
		if (!STR_IsNumeric((char *)data,(USHORT) len))
			ret = FALSE;
		break;
	case MTAG_ECR_ECHO_DATA:	// B
	case OTAG_ECR_ECHO_DATA:
	case CTAG_ECR_ECHO_DATA:
		break;
	case MTAG_ECR_AMOUNT:	// N JC Oct 28/15 Must also be greater than 0
	case OTAG_ECR_AMOUNT:
	case CTAG_ECR_AMOUNT:
		if (!STR_IsNumeric((char *)data, len)) ret = FALSE;
		if ( STR_AscToint((char *)data, len) <= 0)
		{
			ret = FALSE;		//JC Oct 28/15 ensure amount is > 0, passing 0 cause severe ERROR in sale
		}
		break;
	case MTAG_ECR_IP_SETTINGS:	// ANS
	case OTAG_ECR_IP_SETTINGS:
	case CTAG_ECR_IP_SETTINGS:
		if (!STR_IsANS((char *)data, len))
			ret = FALSE;
		break;
	case MTAG_ECR_HOST_SELECTION:	// JC Dec 31/14 IP "I,iii.iii.iii.111,ppppp"
	case OTAG_ECR_HOST_SELECTION:	// JC Dec 31/14 URL "U,uuuuuu...uuuuu,ppppp"
	case CTAG_ECR_HOST_SELECTION:
		if (! DecodeHostAddress(data, len, (BYTE *)wstrHost, (BYTE *)wstrPort, TRUE)) //JC Jan 1/15 just checking format, will save data later
			ret = FALSE;
		break;
	case OTAG_ECR_HOST_SELECTION2:	//  			JC Jan 8/15 Add secondary host
		if (! DecodeHostAddress(data, len, (BYTE *)wstrHost, (BYTE *)wstrPort, TRUE)) //JC Jan 1/15 just checking format, will save data later
			ret = FALSE;
		break;
	case OTAG_ECR_PARAM_HOST_SELECTION:	//    			JC Jan 8/15 Add POSX Gateway
		if (! DecodeHostAddress(data, len, (BYTE *)wstrHost, (BYTE *)wstrPort, TRUE)) //JC Jan 1/15 just checking format, will save data later
			ret = FALSE;
		break;
	case OTAG_ECR_TERMINAL_ID:		// ANS			JC Dec 30/14 Add terminal ID
		if (!STR_IsANS((char *)data, len))
			ret = FALSE;
		break;
	case MTAG_ECR_IMAGE_INDEX:	// N
	case OTAG_ECR_IMAGE_INDEX:
	case CTAG_ECR_IMAGE_INDEX:
		if (!STR_IsNumeric((char *)data, len))
			ret = FALSE;
		break;
	case OTAG_ECR_RECEIPT_TYPE:							//JC May 13/15 added to match API, Receipt type to print either "C" - Cardholder or "M" - Merchant
		if ((data[0] != 'C') && (data[0] != 'M'))
			ret = FALSE;
		break;
	default:
		assert(FALSE);
	}
	return ret;
}

//-----------------------------------------------------------------------------
//!	\Save ECR request Data
//!	\param	tag - request data tag index 
//!	\param	tagData - request data tag data buffer point
//!	\param	len - tag length
//!	\param	pECRREC - ECR_TranDataRec buffer point
//-----------------------------------------------------------------------------
void CECRiCommDlg::SaveTag(int tag, BYTE *tagData, int len, ECR_TranDataRec *pECRREC)
{
	char wstrHost[100];				// JC Dec 31/14
	char wstrPort[10];				// JC Dec 31/14

	switch (tag)
	{
	case MTAG_ECR_MESSAGE_TYPE:
	case OTAG_ECR_MESSAGE_TYPE:
	case CTAG_ECR_MESSAGE_TYPE:
		memcpy(pECRREC->MessageType, tagData, len);
		pECRREC->MessageType[len] = 0;
		break;
	case MTAG_ECR_TRAN_CODE:
	case OTAG_ECR_TRAN_CODE:
	case CTAG_ECR_TRAN_CODE:
		memcpy(pECRREC->TranCode, tagData, len);
		pECRREC->TranCode[len] = 0;
		break;
	case MTAG_ECR_ECHO_DATA:
	case OTAG_ECR_ECHO_DATA:
	case CTAG_ECR_ECHO_DATA:
		memcpy(pECRREC->EchoData, tagData, len);
		pECRREC->EchoDataLength = len;
		break;
	case MTAG_ECR_AMOUNT:
	case OTAG_ECR_AMOUNT:
	case CTAG_ECR_AMOUNT:
		memcpy(pECRREC->Amount, tagData, len);
		pECRREC->Amount[len] = 0;
		break;
	case MTAG_ECR_IP_SETTINGS:
	case OTAG_ECR_IP_SETTINGS:
	case CTAG_ECR_IP_SETTINGS:
		memcpy(pECRREC->IP_Settings, tagData, len);
		pECRREC->IP_Settings[len] = 0;
		break;
	case MTAG_ECR_HOST_SELECTION:
	case OTAG_ECR_HOST_SELECTION:				// JC Dec 31/14 URL "U,uuuuuu...uuuuu,ppppp"
	case CTAG_ECR_HOST_SELECTION:				// JC Dec 31/14 IP "I,iii.iii.iii.111,ppppp"
		pECRREC->HostSelectionIsURL = DecodeHostAddress(tagData, len, (BYTE *)wstrHost, (BYTE *)wstrPort, FALSE);  //JC Jan 1/15 now saving data
		strcpy(pECRREC->HostSelection, wstrHost);
		pECRREC->HostSelectionPort = atoi(wstrPort);
		break;
	case OTAG_ECR_HOST_SELECTION2:						//JC Dec 30/14 - Add Secondary Host
		pECRREC->HostSelection2IsURL = DecodeHostAddress(tagData, len, (BYTE *)wstrHost, (BYTE *)wstrPort, FALSE);  //JC Jan 1/15 now saving data
		strcpy(pECRREC->HostSelection2, wstrHost);
		pECRREC->HostSelection2Port = atoi(wstrPort);
		break;
	case OTAG_ECR_PARAM_HOST_SELECTION:						//JC Dec 30/14 - add Gateway to POSX
		pECRREC->ParamHostSelectionIsURL = DecodeHostAddress(tagData, len, (BYTE *)wstrHost, (BYTE *)wstrPort, FALSE);  //JC Jan 1/15 now saving data
		strcpy(pECRREC->ParamHostSelection, wstrHost);
		pECRREC->ParamHostSelectionPort = atoi(wstrPort);
		break;
	case OTAG_ECR_TERMINAL_ID:							//JC Dec 30/14 - add Terminal ID
		memcpy(pECRREC->Terminal_ID, tagData, len);
		pECRREC->Terminal_ID[len] = 0;
		break;
	case MTAG_ECR_IMAGE_INDEX:
	case OTAG_ECR_IMAGE_INDEX:
	case CTAG_ECR_IMAGE_INDEX:
		memcpy(pECRREC->ImageIndex, tagData, len);
		pECRREC->ImageIndex[len] = 0;
		break;
	case OTAG_ECR_RECEIPT_TYPE:							//JC May 13/15 to ADD RT Tag
		memcpy(pECRREC->ReceiptType, tagData, len);		//JC May 13/15 to ADD RT Tag
		pECRREC->ReceiptType[len] = 0;					//JC May 13/15 null terminate for length	
		break;
	default:
		assert(FALSE);
	}
}

//-----------------------------------------------------------------------------
//!	\Get Tag's name
//!	\param	tag - request data tag index 
//-----------------------------------------------------------------------------
char* CECRiCommDlg::GetTagName(unsigned short tag)
{
	char *p;

	switch ((sEcrTags) tag)
	{
	case MTAG_ECR_MESSAGE_TYPE:
	case OTAG_ECR_MESSAGE_TYPE:
	case CTAG_ECR_MESSAGE_TYPE:
		p = TAG_ECR_MESSAGE_TYPE;
		break;
	case MTAG_ECR_TRAN_CODE:
	case OTAG_ECR_TRAN_CODE:
	case CTAG_ECR_TRAN_CODE:
		p = TAG_ECR_TRAN_CODE;
		break;
	case MTAG_ECR_ECHO_DATA:
	case OTAG_ECR_ECHO_DATA:
	case CTAG_ECR_ECHO_DATA:
		p = TAG_ECR_ECHO_DATA;
		break;
	case MTAG_ECR_CONDITION_CODE:
	case OTAG_ECR_CONDITION_CODE:
	case CTAG_ECR_CONDITION_CODE:
		p = TAG_ECR_CONDITION_CODE;
		break;
	case MTAG_ECR_AMOUNT:
	case OTAG_ECR_AMOUNT:
	case CTAG_ECR_AMOUNT:
		p = TAG_ECR_AMOUNT;
		break;
	case MTAG_ECR_DL_REQUIRED:
	case OTAG_ECR_DL_REQUIRED:
	case CTAG_ECR_DL_REQUIRED:
		p = TAG_ECR_DL_REQUIRED;
		break;
	case MTAG_ECR_IP_SETTINGS:
	case OTAG_ECR_IP_SETTINGS:
	case CTAG_ECR_IP_SETTINGS:
		p = TAG_ECR_IP_SETTINGS;
		break;
	case MTAG_ECR_HOST_SELECTION:
	case OTAG_ECR_HOST_SELECTION:
	case CTAG_ECR_HOST_SELECTION:
		p = TAG_ECR_HOST_SELECTION;
		break;
	case OTAG_ECR_HOST_SELECTION2:				//JC Dec 30/14 Add secondary host
		p = TAG_ECR_HOST_SELECTION2;
		break;
	case OTAG_ECR_PARAM_HOST_SELECTION:				//JC Dec 30/14 Add Gateway for Parameter Downloads				
		p = TAG_ECR_PARAM_HOST_SELECTION;
		break;
	case OTAG_ECR_TERMINAL_ID:					//JC Dec 30/14 Add Terminal ID
		p = TAG_ECR_TERMINAL_ID;
		break;
	case MTAG_ECR_FORMATTED_RCPT:
	case OTAG_ECR_FORMATTED_RCPT:
	case CTAG_ECR_FORMATTED_RCPT:
		p = TAG_ECR_FORMATTED_RCPT;
		break;
	case MTAG_ECR_TRACK1:
	case OTAG_ECR_TRACK1:
	case CTAG_ECR_TRACK1:
		p = TAG_ECR_TRACK1;
		break;
	case MTAG_ECR_TRACK2:
	case OTAG_ECR_TRACK2:
	case CTAG_ECR_TRACK2:
		p = TAG_ECR_TRACK2;
		break;
	case MTAG_ECR_IMAGE_INDEX:
	case OTAG_ECR_IMAGE_INDEX:
	case CTAG_ECR_IMAGE_INDEX:
		p = TAG_ECR_IMAGE_INDEX;
		break;
	case CTAG_ECR_EDIT_ERROR:			//JC Dec 8/14 Add edit checking and error tag
		p = TAG_ECR_EDIT_ERROR;
		break;
	case CTAG_ECR_HOST_RESPONSE_CODE:			//Jun May 4/15 Host Response Code TAG
		p = TAG_ECR_HOST_RESPONSE_CODE;
		break;
	case MTAG_ECR_STATUS:				//JC Dec 10/14 Add status tag
		p = TAG_ECR_STATUS;
		break;
	case CTAG_ECR_CUSTOMER_TEXT:				//JC Mar 15/15 Add Customer Text
		p = TAG_ECR_CUSTOMER_TEXT;
		break;
	case OTAG_ECR_RECEIPT_TYPE:				//JC May 13/15 Add receipt type tag
		p = TAG_ECR_RECEIPT_TYPE;
		break;
	case CTAG_ECR_APPLICATION_INFORMATION:			//AM June 18/15
		p = TAG_ECR_APPLICATION_INFORMATION;
		break;
	case CTAG_ECR_INVOICE_NUMBER_CODE:			//Jun Oct 22/15 Txn result TAG
		p = TAG_ECR_INVOICE_NUMBER_CODE;
		break;
	case CTAG_ECR_AUTHORIZATION_NUMBER_CODE:			//Jun Oct 22/15 Txn result TAG
		p = TAG_ECR_AUTHORIZATION_NUMBER_CODE;
		break;
	case CTAG_ECR_CARD_TYPE_CODE:			//Jun Oct 22/15 Txn result TAG
		p = TAG_ECR_CARD_TYPE_CODE;
		break;
	case CTAG_ECR_CARD_PLAN_CODE:			//Jun Oct 22/15 Txn result TAG
		p = TAG_ECR_CARD_PLAN_CODE;
		break;
	case CTAG_ECR_CARDHOLDER_LANGUAGE_CODE:			//Jun Oct 22/15 Txn result TAG
		p = TAG_ECR_CARDHOLDER_LANGUAGE_CODE;
		break;
	case CTAG_ECR_CARD_NUMBER_CODE:			//Jun Oct 22/15 Txn result TAG
		p = TAG_ECR_CARD_NUMBER_CODE;
		break;
	default:
		break;
		//assert(FALSE);
	}
	return p;
}

//-----------------------------------------------------------------------------
//!	\Check Mandatory Tag
//!	\param	tag - request data tag index 
//-----------------------------------------------------------------------------
BOOL CECRiCommDlg::IsMandatoryTag(int tag)
{
	if (tag > MTAG_START && tag < MTAG_END)
		return TRUE;

	return FALSE;
}


//-----------------------------------------------------------------------------
//!	\Copy data
//!	virtual function
//-----------------------------------------------------------------------------
BOOL CECRiCommDlg::OnCopyData( CWnd* pWnd, COPYDATASTRUCT* pCopyDataStruct )
{
	if ( pCopyDataStruct->cbData < 7)
		return FALSE;

	this->m_hTxnWnd = (HWND)pWnd->m_hWnd;
	BYTE *p = (BYTE*)pCopyDataStruct->lpData;

	memcpy(ECRREC.strTxnResult.InvoiceNo,p,sizeof(TXN_Result));
	p += sizeof(TXN_Result);
	memcpy(ECRREC.ConditionCode,p,SZ_ERRCODE_TEXT);
	ECRREC.ConditionCode[SZ_ERRCODE_TEXT] = 0;
	p += SZ_ERRCODE_TEXT;
	memcpy(ECRREC.RespCode,p,SZ_RESPONSE_CODE);
	ECRREC.RespCode[SZ_RESPONSE_CODE] = 0;
	p += SZ_RESPONSE_CODE;
	strcpy(ECRREC.Customer_Text, (char*)p);		
	dwBytesToWrite = BUFFERSIZE - 1;
	strcpy(ECRREC.MessageType,"RSP");
	FormatEcrResponse(&ECRREC,gbBuffer,(int*)&dwBytesToWrite);
	::PostMessage(m_hWnd,WM_NOTIFY_RETURN,0,0);

//    AfxMessageBox((LPCSTR)(pCopyDataStruct->lpData));
    return CWnd::OnCopyData(pWnd, pCopyDataStruct);
}

//-----------------------------------------------------------------------------
//!	Do ECR Reversal ADDED by Jun Nov 2015

// Disabled for FD MSR until testing complete
//
//-----------------------------------------------------------------------------
void CECRiCommDlg::DoReversal()
{
	if ( m_hTxnWnd )
	{
		::PostMessage(m_hTxnWnd,ERROR_MESSAGE, 2,8);
		m_hTxnWnd = NULL;
	}
}
//////////////////////////////////////


