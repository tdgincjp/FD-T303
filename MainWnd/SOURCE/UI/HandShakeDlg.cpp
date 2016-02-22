#include "StdAfx.h"
//
//#include "HandShakeDlg.h"
//#include "..\\data\\DataFile.h"
//#include "..\\data\\RecordFile.h"
//#include "..\\utils\\FolderCtrl.h"
//#include "..\\EMV\\EMV.h"
//
//#define PASSWORD_INPUT			0
//
//IMPLEMENT_DYNAMIC(CHandShakeDlg, CBaseDlg)
//
////-----------------------------------------------------------------------------
////!	\CDialog class virtual function
////-----------------------------------------------------------------------------
//CHandShakeDlg::CHandShakeDlg(CWnd* pParent )
//	: CBaseDlg(CBaseDlg::IDD, pParent)
//
//{
//	m_iTranCode = TRAN_HANDSHAKRE;
//}
//
////-----------------------------------------------------------------------------
////!	\CDialog class virtual function
////-----------------------------------------------------------------------------
//CHandShakeDlg::~CHandShakeDlg(void)
//{
//}
//
////-----------------------------------------------------------------------------
////!	\CDialog class virtual function
////-----------------------------------------------------------------------------
//void CHandShakeDlg::DoDataExchange(CDataExchange* pDX)
//{
//	CBaseDlg::DoDataExchange(pDX);
//}
//
//
//BEGIN_MESSAGE_MAP(CHandShakeDlg, CBaseDlg)
//		ON_WM_CHAR()
//END_MESSAGE_MAP()
//
//
//// CMainWnd message handlers
//
////-----------------------------------------------------------------------------
////!	\CDialog class virtual function
////-----------------------------------------------------------------------------
//BOOL CHandShakeDlg::OnInitDialog()
//{
////	m_Title = L"COMMS TEST";
//	if ( m_iTranCode == TRAN_HANDSHAKRE )
//		m_Title.Format(L" %s",CLanguage::GetText(CLanguage::IDX_COMMS_TEST_MENU));
//	else
//		m_Title.Format(L" %s",CLanguage::GetText(CLanguage::IDX_KEY_EXCHANGE_MENU));
//
//	CBaseDlg::OnInitDialog();
//
//	m_strMsg.SetCaption(CLanguage::GetText(CLanguage::IDX_ENTER_PASSWORD));
//	m_strMsg.ShowWindow(SW_SHOW);
//	m_strMsg2.SetCaption(L"");
//	m_strMsg2.ShowWindow(SW_SHOW);
//	m_iLevel = PASSWORD_INPUT;
//	SetTimer(1,100,NULL);
//	
//	DoHandShake();
//	return TRUE;  // return TRUE unless you set the focus to a control
//	// EXCEPTION: OCX Property Pages should return FALSE
//}
//
////-----------------------------------------------------------------------------
////!	\CDialog class virtual function
////-----------------------------------------------------------------------------
//void CHandShakeDlg::OnCancel()
//{
//	Beep1();
//	if(!m_RapidConnect.m_Connected)
//	{
//		strcpy(m_ErrCode,ERROR_CONDITION_CODE_CANCEL);
//		CDialog::OnCancel();
//	}
//}
//
////-----------------------------------------------------------------------------
////!	\CDialog class virtual function
////-----------------------------------------------------------------------------
//void CHandShakeDlg::OnOK()
//{
//	Beep1();
//	return;
//
//	if( m_iLevel == PASSWORD_INPUT) //Password Input
//	{
//		if(m_strPassword.GetLength() == 0)
//		{
//			Beep();
//			return;
//		}
//		if(!CheckPassword())
//		{
//			m_IncorrectTimes++;
//			if(m_IncorrectTimes>=5)
//			{
//				CDataFile::Save(L"CLERK PW",L"On");
//				CDialog::OnCancel();
//			}
//			else
//			{
//				Beep();
//				m_strPassword = L"";
//				m_strMsg2.SetCaption(L"");
//			}
//		}
//		else
//			DoHandShake();
//		return;
//	}
//
//	if(!m_RapidConect.m_Connected)
//		CDialog::OnOK();
//}
//
////-----------------------------------------------------------------------------
////!	\CDialog class virtual function
////-----------------------------------------------------------------------------
//void CHandShakeDlg::OnTimer(UINT_PTR nIDEvent)
//{
//	KillTimer(1);
//	if(nIDEvent == 1)
//	{
//		KillTimer(1);
//		SetFocus();
//		return;
//	}
//
//	if(nIDEvent == 2)
//		CDialog::OnOK();
//
//	CBaseDlg::OnTimer(nIDEvent);
//}
//
////-----------------------------------------------------------------------------
////!	\Do hand shake txn
////-----------------------------------------------------------------------------
//void CHandShakeDlg::DoHandShake()
//{
//	m_iLevel++;
//
//	char* buf;
//	CLanguage::GetTextFromIndex(CLanguage::IDX_SOCKET_CONNECT,&buf);
//	m_strMsg.SetCaption(CString(buf));
//	m_strMsg2.SetCaption(L"");
//
//	m_TRREC.TranCode = TRAN_HANDSHAKRE;
//	m_TRREC.HostTranCode = m_TRREC.TranCode;
//
//	m_HOST.m_hWnd = m_hWnd;
//	HANDLE hThread = CreateThread(NULL, 0,ThreadProc, (PVOID)this, 0, NULL);
//	CloseHandle(hThread);
//}
//
////-----------------------------------------------------------------------------
////!	\Do hand shake txn thread entry
////-----------------------------------------------------------------------------
//DWORD WINAPI CHandShakeDlg::ThreadProc (LPVOID lpArg)
//{
//	CHandShakeDlg* dlg = reinterpret_cast<CHandShakeDlg*>(lpArg);
//	dlg->m_HOST.Start(&dlg->m_TRREC, &dlg->m_CONFIG);
//	return 0;
//}
//
////-----------------------------------------------------------------------------
////!	\hand shake txn id done
////-----------------------------------------------------------------------------
//void CHandShakeDlg::Done()
//{
//	CString str,str1,str2;
////	if(m_iLevel == 2)
//	{
//		char* buf;
//
//		int iTemp = m_HOST.GetCommStatus();
//		if (iTemp == ST_OK)
//		{
//			iTemp = m_HOST.GetTranStatus();
//			if ( iTemp == ST_APPROVED)
//			{
//				str = L"Successful";
//				str1=L"";
//				str2=L"";
//				ShowText(str, str1, str2);
//				SetTimer(2,2500,NULL);
//				strcpy(m_ErrCode,ERROR_CONDITION_CODE_OK);//Host respond error
//				return;
//			}
//			else
//				strcpy(m_ErrCode,ERROR_CONDITION_CODE_DECLINE);//Host respond error
//		}
//		else
//		{
//			if ( iTemp == ST_HOST_TIMEOUT)
//				strcpy(m_ErrCode,ERROR_CONDITION_CODE_TIMEOUT);//Host respond error
//			else
//				strcpy(m_ErrCode,ERROR_CONDITION_CODE_NETWORK);//network error
//		}
//
//		CLanguage::GetTextFromIndex(CLanguage::IDX_ERROR,&buf);
//		str = CString(buf);
//		str1=L"";
//		str2=L"";
//		ShowText(str, str1, str2);
//
//		SetTimer(2,2500,NULL);
//		return;
//	}
//}
//
////-----------------------------------------------------------------------------
////!	\CDialog class virtual function
////-----------------------------------------------------------------------------
//void CHandShakeDlg::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags)
//{
//	Beep1();
//	m_strMsg2.SetCaption(DealPassword(nChar,m_strPassword));
//	SetFocus();
//}
