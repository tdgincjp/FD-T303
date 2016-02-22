#include "StdAfx.h"
#include "..\\..\\MainWnd.h"
#include "..\\UTILS\\string.h"
#include "..\\TRANSACTIONS\\TransactionLog.h"

#include "MsgDlg.h"
#include "ReprintDlg.h"

IMPLEMENT_DYNAMIC(CReprintDlg, CBaseDlg)

CReprintDlg::CReprintDlg(CWnd* pParent )
	: CBaseDlg(CBaseDlg::IDD, pParent)

{
	m_strInvoice = L"";
}

CReprintDlg::~CReprintDlg(void)
{
}

void CReprintDlg::DoDataExchange(CDataExchange* pDX)
{
	CBaseDlg::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CReprintDlg, CBaseDlg)
		ON_WM_CHAR()
END_MESSAGE_MAP()


// CMainWnd message handlers

BOOL CReprintDlg::OnInitDialog()
{
	m_Title = CLanguage::GetText(CLanguage::IDX_REPRINT_MENU);//L"REPRINT";
	CBaseDlg::OnInitDialog();

	m_strMsg.SetCaption(CLanguage::GetText(CLanguage::IDX_PRINTING));
	m_strMsg.ShowWindow(SW_SHOW);
	m_strMsg1.SetCaption(CLanguage::GetText(CLanguage::IDX_PLEASE_WAIT));
	m_strMsg1.ShowWindow(SW_SHOW);

	SetTimer(1,50,NULL);
	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CReprintDlg::OnCancel()
{
	Beep1();
}

void CReprintDlg::OnOK()
{
	Beep1();
}
void CReprintDlg::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	Beep1();
	SetFocus();
}

void CReprintDlg::OnTimer(UINT_PTR nIDEvent)
{
	if(nIDEvent == 1)
	{
		KillTimer(1);
		CTransactionLog BatchLog(BATCH_LOG_FILE);

		int index;
		if ( m_strInvoice == L"" )
		{
			index = BatchLog.GetRecordCount();
//			memcpy(&DataRec,&(((CMainWndApp*)AfxGetApp())->m_TRREC),sizeof(TRAN_TranDataRec));
		}
		else
		{
			char buf[10];
			char* p = alloc_tchar_to_char(m_strInvoice.GetBuffer(m_strInvoice.GetLength()));
			sprintf(buf,"%07d",atoi(p));
			free(p);
			m_strInvoice.ReleaseBuffer();
			index = BatchLog.Search((BYTE*)buf);

			if (index == 0)
			{
				CMsgDlg dlg;
				dlg.m_iTimeOut = 5;
				dlg.m_bNoButtonFlag = TRUE;
				dlg.m_str1 = CLanguage::GetText(CLanguage::IDX_INVOICE_NUMBER) ;
				dlg.m_str2 = m_strInvoice+L" "+CLanguage::GetText(CLanguage::IDX_IS_NOT_FOUND);
				dlg.DoModal();
				CDialog::OnCancel();
				return;
			}

		}
		BatchLog.Read(index);
		memcpy(&DataRec,&BatchLog.m_Record.TranRec,sizeof(TRAN_TranDataRec));

		TRAN_TranDataRec *pTRANDATA = &DataRec;
//		TRAN_TranDataRec *pTRANDATA = &(((CMainWndApp*)AfxGetApp())->m_TRREC);
		if (pTRANDATA->TranCode > FUNCTION_FINAN_BEGIN &&
				pTRANDATA->TranCode < FUNCTION_FINAN_END)
		{
			BOOL bTraining = FALSE;
			TCHAR buf[10]={L""};
			if(CDataFile::Read(L"TRAINING",buf))
				if(CString(buf) == L"On")
					bTraining = TRUE;

			m_Printer.m_bReprint = TRUE;
			
			CMsgDlg dlg;
			dlg.m_iTimeOut = 30;
			dlg.m_iButtonType = 2;
			dlg.m_str1 = CLanguage::GetText(CLanguage::IDX_RECEIPT_TYPE);
			dlg.m_str2 = L"";
			dlg.m_strBt1 = CLanguage::GetText(CLanguage::IDX_CUST);
			dlg.m_strBt2 = CLanguage::GetText(CLanguage::IDX_MERCH);
			
			m_Printer.m_bCancelFlag = FALSE;
			if( dlg.DoModal() != IDCANCEL)
			{
				if (dlg.m_bTimeOutFlag )
				{
					CDialog::OnCancel();
					return;
				}

				if ( dlg.m_iButtonValue == 1)
					m_Printer.Exec(pTRANDATA, m_hWnd,1,bTraining);
				else
					m_Printer.Exec(pTRANDATA, m_hWnd,2,bTraining);
			}
			else
				CDialog::OnCancel();
//				m_Printer.Exec(pTRANDATA, m_hWnd,1,bTraining);

		}
		else
			CDialog::OnCancel();
	}
	CBaseDlg::OnTimer(nIDEvent);
}


void CReprintDlg::Done()
{
	CDialog::OnCancel();
}

LRESULT CReprintDlg::DefWindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	CString strTemp; 
	DWORD len = 0;
    DWORD decode=0;

    if(wParam == 2 && message == ERROR_MESSAGE)
	{ 
		if(lParam == 3) // print done
		{
			CDialog::OnCancel();
			return 0;
		}
	}
	return CDialog::DefWindowProc(message, wParam, lParam);
}
