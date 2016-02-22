#include "StdAfx.h"

#include "RapidConnectDownloadDlg.h"
#include "..\\data\\DataFile.h"
#include "..\\data\\RecordFile.h"
#include "..\\utils\\FolderCtrl.h"
#include "..\\EMV\\EMV.h"

IMPLEMENT_DYNAMIC(CRapidConnectDownloadDlg, CBaseDlg)

CRapidConnectDownloadDlg::CRapidConnectDownloadDlg(CWnd* pParent )
	: CBaseDlg(CBaseDlg::IDD, pParent)
{
	m_bExitFlag = FALSE;
//	CDataFile::Save(FID_MERCH_CFG_TERM_ID, (BYTE*) "TS090302", 8);
	CDataFile::Save(FID_CFG_DEVICE_MODEL, (BYTE*) "T303      ", 10);
	BOOL enable = TRUE;
	CDataFile::Save(FID_TERM_EMV_ENABLE_FLG, (BYTE*) &enable, sizeof(BOOL));
	CDataFile::Save(FID_TERM_CTLS_ENABLE_FLG, (BYTE*) &enable, sizeof(BOOL));
	CDataFile::Save(FID_EMV_TERMINAL_CAPABILITIES, (BYTE*) "\x60\x98\xC8", 3);
	CDataFile::Save(FID_EMV_FLASH_TERMINAL_CAPABILITIES, (BYTE*) "\x60\x98\xC0", 3);
	CDataFile::Save(FID_EMV_ADDITIONAL_TERMINAL_CAPABILITIES, (BYTE*) "\x60\x00\xF0\x50\x01", 5);
	CDataFile::Save(FID_EMV_TERMINAL_COUNTRY_CODE_NUMERIC, (BYTE*) "124", 3);
	int timer = 30;
	CDataFile::Save(FID_CFG_CUSTENTRY_TMO, (BYTE*) &timer, sizeof(int));
	int seqNo = 1;
	CDataFile::Save(FID_TERM_SEQ_NUMBER, (BYTE *) &seqNo, sizeof(int));
	CDataFile::Save(FID_TERM_SHIFT_NUMBER, (BYTE *) &seqNo, sizeof(int));
	CDataFile::Save(FID_TERM_BATCH_NUMBER, (BYTE *) &seqNo, sizeof(int));
//	BYTE tranNo = 0;
//	CDataFile::Save(FID_TERM_TRANSMISSION_NUMBER, &tranNo, sizeof(BYTE));
//	int termLang = ENGLISH;
//	CDataFile::Save(FID_CFG_LANGUAGE, (BYTE *)&termLang, sizeof(int));

	enable = FALSE;
	CDataFile::Save(FID_DEBIT_ONLY_TERMINAL_FLAG, (BYTE*) &enable, sizeof(BOOL));
	m_bWaitingDoneEvent = FALSE;
	m_RapidConnect = new CRapidConnect(m_hWnd);
}

CRapidConnectDownloadDlg::~CRapidConnectDownloadDlg(void)
{
}

void CRapidConnectDownloadDlg::DoDataExchange(CDataExchange* pDX)
{
	CBaseDlg::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CRapidConnectDownloadDlg, CBaseDlg)
	//	ON_WM_PAINT()
END_MESSAGE_MAP()


// CMainWnd message handlers
BOOL CRapidConnectDownloadDlg::OnInitDialog()
{
	m_Title.Format(L"%s",CLanguage::GetText(CLanguage::IDX_DOWNLOAD_MENU));
	CBaseDlg::OnInitDialog();

	m_strMsg.SetCaption(CLanguage::GetText(CLanguage::IDX_PLEASE_WAIT));
	m_strMsg.ShowWindow(SW_SHOW);

	SetTimer(1,100,NULL);

	SetTimer(3,18000,NULL);

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CRapidConnectDownloadDlg::OnCancel()
{
	Beep1();
	if(m_bExitFlag)
		CDialog::OnCancel();
}

void CRapidConnectDownloadDlg::OnOK()
{
	Beep1();
	if(m_bExitFlag)
		CDialog::OnOK();
}

void CRapidConnectDownloadDlg::OnTimer(UINT_PTR nIDEvent)
{
	KillTimer(1);
	if(nIDEvent == 1)
	{
		KillTimer(1);
		DoHostInit();
	}
	else if(nIDEvent == 2) // Close window
	{
		KillTimer(2);
		CDialog::OnOK();
	}
	else if(nIDEvent == 3) 
	{
		KillTimer(3);
		m_strMsg.SetCaption(CLanguage::GetText(CLanguage::IDX_ERROR));
		SetTimer(2,5000,NULL);
		m_bExitFlag = TRUE;
	}

	CBaseDlg::OnTimer(nIDEvent);
}

void CRapidConnectDownloadDlg::DoHostInit()
{
	DWORD size;
	
	CDataFile::Read(FID_MERCH_CFG_TERM_ID, (BYTE*) m_CONFIG.TermID, &size);
	CDataFile::Read(FID_CFG_DEVICE_MODEL, (BYTE*) m_CONFIG.DeviceModel, &size);
	m_CONFIG.EMVEnabled = TRUE;
	m_TRREC.TranCode = TRAN_HOST_INIT;
	m_TRREC.HostTranCode = m_TRREC.TranCode;

	if( !CDataFile::m_bTraining)
	{
		CRecordFile HostFile;
		HostFile.Delete(SERVICE_DATA_FILE);
		HostFile.Delete(EMV_DATA_FILE);
		HostFile.Delete(CTLS_DATA_FILE);
	}
	CDataFile::Delete(FID_TERM_MAC_REQ_FIDS);
	CDataFile::Delete(FID_TERM_MAC_RSP_FIDS);
	CDataFile::Delete(FID_MERCH_CFG_ADDRESS);
	CDataFile::Delete(FID_MERCH_CFG_CITY_PROV);
	CDataFile::Delete(FID_MERCH_CFG_NAME);	
	CDataFile::Delete(FID_TERM_ALLOWED_TRANS);
	CFolderCtrl::DeleteDir(EMV_KEY_DATA_DIR);
//	CDataFile::Delete(L"SURCHARGEFEE");
//	SetInitStatus((int)STS_DLL_DOWNLOAD_REQUIRED);

	m_RapidConnect->m_hWnd = m_hWnd;
	HANDLE hThread = CreateThread(NULL, 0,ThreadProc, (PVOID)this, 0, NULL);
	CloseHandle(hThread);
}

DWORD WINAPI CRapidConnectDownloadDlg::ThreadProc (LPVOID lpArg)
{
	CRapidConnectDownloadDlg* dlg = reinterpret_cast<CRapidConnectDownloadDlg*>(lpArg);
	dlg->m_RapidConnect->Start(&dlg->m_TRREC, &dlg->m_CONFIG);
	return 0;
}

void CRapidConnectDownloadDlg::SetInitStatus(int status)
{
	CDataFile::Save(FID_TERM_HOST_INIT_STATUS, (BYTE *)&status, sizeof(int));
}


void CRapidConnectDownloadDlg::Done()
{
	char* buf;
	m_bExitFlag = TRUE;

	if(m_iLevel == 1)
	{
		if (m_RapidConnect->m_TranStatus == ST_APPROVED)
		{
			KillTimer(3);
			//SetInitStatus(STS_DLL_OK);
			CLanguage::GetTextFromIndex(CLanguage::IDX_HOST_INIT_DONE,&buf);
			strcpy(m_ErrCode, ERROR_CONDITION_CODE_OK);
		}
		else
		{
			CLanguage::GetTextFromIndex(CLanguage::IDX_ERROR,&buf);
			buf = m_TRREC.HostRespText;										//JC Nov 9/15 to pick up message from Datawire/Rapid Connect
		}
		m_strMsg.SetCaption(CString(buf));
		SetTimer(2,10000,NULL);
	}
	return;
}