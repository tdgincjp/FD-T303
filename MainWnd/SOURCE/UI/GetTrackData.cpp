#include "StdAfx.h"

#include "GetTrackData.h"
#include "..\\data\\DataFile.h"
#include "..\\data\\RecordFile.h"
#include "..\\utils\\FolderCtrl.h"

IMPLEMENT_DYNAMIC(CGetTrackDataDlg, CBaseDlg)

CGetTrackDataDlg::CGetTrackDataDlg(CWnd* pParent )
	: CBaseDlg(CBaseDlg::IDD, pParent)
{
}

CGetTrackDataDlg::~CGetTrackDataDlg(void)
{
}

//-----------------------------------------------------------------------------
//!	\CDialog class virtual function
//-----------------------------------------------------------------------------
void CGetTrackDataDlg::DoDataExchange(CDataExchange* pDX)
{
	CBaseDlg::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CGetTrackDataDlg, CBaseDlg)
	//	ON_WM_PAINT()
END_MESSAGE_MAP()


// CMainWnd message handlers

//-----------------------------------------------------------------------------
//!	\CDialog class virtual function
//-----------------------------------------------------------------------------
BOOL CGetTrackDataDlg::OnInitDialog()
{
	m_Title = L"Track Data";
	CBaseDlg::OnInitDialog();

	m_strMsg.SetCaption(L"");
	m_strMsg2.SetCaption(L"");


	char* pbuf;
	m_strMsg.m_nFormat = DT_CENTER;
	m_strMsg2.m_nFormat = DT_LEFT;
	CLanguage::GetTextFromIndex(CLanguage::IDX_INSERT_SWIPE_TAP,&pbuf);
	m_strMsg1.SetCaption( CString(pbuf));
//	str1=  L"";
//	ShowText(m_Title,str ,str1 );
	SetTimer(1,30000,NULL);
	m_strMsg.ShowWindow(SW_SHOW);
	m_strMsg1.ShowWindow(SW_SHOW);
	m_strMsg2.ShowWindow(SW_SHOW);

	OpenDevice();
	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

//-----------------------------------------------------------------------------
//!	\CDialog class virtual function
//-----------------------------------------------------------------------------
void CGetTrackDataDlg::OnCancel()
{
	strcpy(m_ErrCode,ERROR_CONDITION_CODE_CANCEL);
	CloseDevice();
	CDialog::OnCancel();
}

//-----------------------------------------------------------------------------
//!	\CDialog class virtual function
//-----------------------------------------------------------------------------
void CGetTrackDataDlg::OnOK()
{
	Beep1();
}

//-----------------------------------------------------------------------------
//!	\CDialog class virtual function
//-----------------------------------------------------------------------------
void CGetTrackDataDlg::OnTimer(UINT_PTR nIDEvent)
{
	if(nIDEvent == 1)
	{
		KillTimer(1);
		strcpy(m_ErrCode,ERROR_CONDITION_CODE_TIMEOUT);
		CloseDevice();
		CDialog::OnCancel();
	}
	CBaseDlg::OnTimer(nIDEvent);
}

//-----------------------------------------------------------------------------
//!	\CGetTrackDataDlg class virtual function
//-----------------------------------------------------------------------------
void CGetTrackDataDlg::Done()
{
}
	
//-----------------------------------------------------------------------------
//!	\Open Device
//-----------------------------------------------------------------------------
BOOL CGetTrackDataDlg::OpenDevice()
{
	if (!m_MSR.Open(m_hWnd))
		return FALSE;

//	if (!m_SCR.Open(m_MsgWnd))
//		return FALSE;

	char amount[16];
	strcpy(amount,"$1.00");
	int iAmount = atoi(amount);

	if (!m_RFID.Open(m_hWnd, amount,TRAN_PURCHASE,iAmount))  
		return FALSE;

	return TRUE;
}
	
//-----------------------------------------------------------------------------
//!	\Close device
//-----------------------------------------------------------------------------
void CGetTrackDataDlg::CloseDevice()
{
	m_MSR.Cancel();
//	m_SCR.Cancel();
	m_RFID.Cancel();

}
//----------------------------------------------------------------------------
//!	\brief	Message entry function for any GUI's, 
//----------------------------------------------------------------------------
LRESULT CGetTrackDataDlg::DefWindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	CString strTemp; 
	DWORD len = 0;
   DWORD decode = 0;
//   char* pbuf;

	if (wParam == 1)
	{
//	TRACE(L"CEnterCardDlg RFID event = %x\n", message);
		BYTE buf[2048] = {0};
		DWORD len;
		switch(message)
		{
			case RFID_EVENT_READY:
				m_RFID.GetData();
				m_RFID.GetData((BYTE*)buf, &len);
				CloseDevice();
				ProcessCtlsTLV((USHORT)len, (BYTE*)buf);
				strcpy(Track2,m_TRREC.Track2);
				CDialog::OnCancel();
				return 0;
				break;
			case RFID_EVENT_TIMEOUT:
				m_RFID.Cancel();
				TRACE(L"RFID_EVENT_TIMEOUT\n");
				PostMessage(ERROR_MESSAGE, 2, 5);
				break;
			case RFID_EVENT_CANCELLED:
				break;
			case RFID_EVENT_ERROR:
				m_RFID.Cancel();
				ReOpenRFID();
				break;
			default:
				break;	
		}	
	}
	else if (wParam == 0)
	{
		int iTemp = 0;
		switch(message)
		{
			case MSR_EVENT_DEV_OFFLINE :
				m_strErr1 = CLanguage::GetText(CLanguage::IDX_MSR_OPEN_FAIL);
				strcpy(m_ErrCode,ERROR_MSR_OPEN_FAIL);
				ErrorExit();
				return NULL;
			case MSR_EVENT_DATA_READY :
			case MSR_EVENT_CT_STATUS :
			case MSR_EVENT_READ_ERROR :
				if( !m_MSR.MsrGetTrackData())
					return 0;
				m_MSR.GetMSRData(1, (BYTE*)Track1, &len, &decode);
				m_MSR.GetMSRData(2, (BYTE*)Track2, &len, &decode);
				CloseDevice();
				CDialog::OnCancel();
				return 0;
		}
	}
	else if (wParam == 2 && message == ERROR_MESSAGE)
	{ 
		if (lParam == 5) //RFID time out and restart it
		{
			ReOpenRFID();
		}
	}
	return CDialog::DefWindowProc(message, wParam, lParam);
}
