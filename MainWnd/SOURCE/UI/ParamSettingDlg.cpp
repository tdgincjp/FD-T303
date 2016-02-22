#include "StdAfx.h"
#include "ParamSettingDlg.h"
#include "..\\Utils\\TimeUtil.h"
#include "..\\DEFS\\constant.h"
#include "TipOptionDlg.h"
#include "EnterAmountDlg.h"
#include "StaticIPDlg.h"
#include "AmountDlg.h"

#include "..\\data\\RecordFile.h"
#include "..\\utils\\FolderCtrl.h"
#include "..\\utils\\totalutil.h"
#include "..\\UTILS\\string.h"
#include "FlexTimerDlg.h"

IMPLEMENT_DYNAMIC(CParamSettingDlg, CBaseDlg)

CParamSettingDlg::CParamSettingDlg(CString name,CWnd* pParent )
	: CBaseDlg(CBaseDlg::IDD, pParent)
{
	m_Key = name;
//	m_Title = name;
}

CParamSettingDlg::~CParamSettingDlg(void)
{
}

void CParamSettingDlg::DoDataExchange(CDataExchange* pDX)
{
	CBaseDlg::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CParamSettingDlg, CBaseDlg)
		ON_COMMAND_RANGE(IDC_BUTTON1, IDC_BUTTON2, &CParamSettingDlg::OnBnClickedButton)
END_MESSAGE_MAP()


// CMainWnd message handlers

BOOL CParamSettingDlg::OnInitDialog()
{
	TCHAR buf[100]={L""};
	CString str;
	if(m_Key == L"DIAL MODE")
	{
		m_Title = CLanguage::GetText(CLanguage::IDX_DIAL_MODE_MENU);
		if(!CDataFile::Read(m_Key,buf))
			CDataFile::Save(m_Key,L"Tone");
	}
	else if(m_Key == L"BAUD RATE")
	{
		m_Title = CLanguage::GetText(CLanguage::IDX_BAUD_RATE_MENU);
		if(!CDataFile::Read(m_Key,buf))
			CDataFile::Save(m_Key,L"2400");
	}
	else if(m_Key == L"LANGUAGE")
		m_Title = CLanguage::GetText(CLanguage::IDX_LANGUAGE_MENU);
	else if(m_Key == L"SOUND")
		m_Title = CLanguage::GetText(CLanguage::IDX_SOUND_MENU);
	else if(m_Key == L"CLERK ID")
		m_Title = CLanguage::GetText(CLanguage::IDX_CLERK_ID);
/*	else if(m_Key == L"HEADLINE")
	{
		m_Title = CLanguage::GetText(CLanguage::IDX_HEADER_MSG);
		if(!CDataFile::Read(m_Key,buf))
			CDataFile::Save(m_Key,L"Off");
	}
*/	else if(m_Key == L"FRAUD CHECK")
		m_Title = CLanguage::GetText(CLanguage::IDX_FRAUD_CHECK);
	else if(m_Key == L"MANUAL ENTRY")
		m_Title = CLanguage::GetText(CLanguage::IDX_MANUAL_ENTRY);
	else if(m_Key == L"INVOICE")
	{
		m_Title = CLanguage::GetText(CLanguage::IDX_INVOICE_MENU);
//		if(!CDataFile::Read(m_Key,buf))
//			CDataFile::Save(m_Key,L"Off");
	}
	else if(m_Key == L"SETTLE TYPE")
	{
		m_Title = CLanguage::GetText(CLanguage::IDX_SETTLE_TYPE_MENU);
		if(!CDataFile::Read(m_Key,buf))
		{
			CDataFile::Save(m_Key,L"Flex");
			CDataFile::Save(L"FLEXTIME",L"99:99");
		}
	}
	else if(m_Key == L"RECEIPT")
		m_Title = CLanguage::GetText(CLanguage::IDX_RECEIPT_RECEIPT);
	else if(m_Key == L"SURCHARGE")
		m_Title = CLanguage::GetText(CLanguage::IDX_SURCHARGE_MENU);
	else if(m_Key == L"CASHBACK")
		m_Title = CLanguage::GetText(CLanguage::IDX_CASHBACK_MENU);
	else if(m_Key == L"PREAUTH AGING DAYS")
	{
		m_Title = CLanguage::GetText(CLanguage::IDX_PREAUTH_AGING_DAYS_MENU);
//		str.Format( L"%s:[ On ]",CLanguage::GetText(CLanguage::IDX_DAYS));
		if(!CDataFile::Read(m_Key,buf))
			CDataFile::Save(m_Key,L"15");
	}
	else if(m_Key == L"DIAL BACKUP")
		m_Title = CLanguage::GetText(CLanguage::IDX_DIAL_BACKUP);
	else if(m_Key == L"STATIC IP")
	{
		m_Title = CLanguage::GetText(CLanguage::IDX_STATIC_IP);
		if(!CDataFile::Read(m_Key,buf))
			CDataFile::Save(m_Key,L"Off");
	}
	else if(m_Key == L"TIP PROMPT")
		m_Title = CLanguage::GetText(CLanguage::IDX_TIP_PROMPT);
	else if(m_Key == L"CLEARBATCH")
		m_Title = CLanguage::GetText(CLanguage::IDX_CLEAR_BATCH_MENU);
	
	CBaseDlg::OnInitDialog();

/*
	if( m_Key == L"DIAL MODE")
		str.Format( L"%s:[ Tone ]",CLanguage::GetText(CLanguage::IDX_MODE));
	else if( m_Key == L"BAUD RATE")
		str.Format( L"%s:[ 2400 ]",CLanguage::GetText(CLanguage::IDX_MODE));
*/
	if(CDataFile::Read(m_Key,buf))
		str.Format(L"%s:[ %s ]",CLanguage::GetText(CLanguage::IDX_MODE),CString(buf));

	if(m_Key == L"LANGUAGE")
	{
		switch(buf[0])
		{
		case '1':
			str = L"Francais";
			break;
		default:
			str = L"English";
			break;
		}
	}
	else if(m_Key == L"PREAUTH AGING DAYS")
	{
		str.Format( L"%s:[ %s ]",CLanguage::GetText(CLanguage::IDX_DAYS),CString(buf));
	}
	
	if(m_Key == L"CLEARBATCH")
	{
		m_strMsg.SetCaption(CLanguage::GetText(CLanguage::IDX_WARNING_CLEAR));
		m_strMsg.ShowWindow(SW_SHOW);

		m_strMsg1.SetCaption(CLanguage::GetText(CLanguage::IDX_CURRENT_BATCH)+L"?");
		m_strMsg1.ShowWindow(SW_SHOW);
	}
	else
	{
		m_strMsg.SetCaption(str);
		m_strMsg.ShowWindow(SW_SHOW);

		m_strMsg1.SetCaption(CLanguage::GetText(CLanguage::IDX_CHANGE)+L"?");
		m_strMsg1.ShowWindow(SW_SHOW);
	}


	m_Bt.SetValue(-1,CLanguage::GetText(CLanguage::IDX_BT_YES),IDB_OK);
	m_Bt.SubclassDlgItem(IDC_BUTTON1, this);
	m_Bt.MoveWindow(screenX-128,  screenY- 50, 120,45);
	m_Bt.ShowWindow(SW_SHOW);

	m_Bt1.SetValue(-1,CLanguage::GetText(CLanguage::IDX_BT_NO),IDB_CANCEL);
	m_Bt1.SubclassDlgItem(IDC_BUTTON2, this);
	m_Bt1.MoveWindow( 8, screenY- 50, 120, 45);
	m_Bt1.ShowWindow(SW_SHOW);

	m_iLevel = 1;
	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CParamSettingDlg::OnBnClickedButton(UINT nID)
{
	if( nID == IDC_BUTTON2)
	{
		CDialog::OnCancel();
		return;
	}

	m_Bt.ShowWindow(SW_HIDE);
	m_Bt1.ShowWindow(SW_HIDE);
		// save and comfirm

	if(m_Key == L"LANGUAGE")
	{
		CDataFile::Save(m_Key,m_strMsg.m_szCaption==L"Francais"?L"0":L"1");
		CLanguage::SetLanguage();//m_strMsg.m_szCaption==L"Francais"?ENGLISH:FRENCH);

		DWORD language = m_strMsg.m_szCaption==L"Francais"?0:1;
		CDataFile::Save(FID_CFG_LANGUAGE, (BYTE *)&language, sizeof(DWORD));
	}
	else if ( m_Key == L"DIAL MODE")
	{
		if(m_strMsg.m_szCaption.Find(L"Tone")>0)
			CDataFile::Save(m_Key,L"Pulse");
		else
			CDataFile::Save(m_Key,L"Tone");
	}
	else if ( m_Key == L"SETTLE TYPE")
	{
		if(m_strMsg.m_szCaption.Find(L"Flex")>0)
			CDataFile::Save(m_Key,L"Auto");
		else
		{
			CDataFile::Save(m_Key,L"Flex");
			CFlexTimerDlg dlg;
			dlg.m_str1 = CLanguage::GetText(CLanguage::IDX_ENTER_SETTLE) ;
			dlg.m_str2 = CLanguage::GetText(CLanguage::IDX_FLEX_TIME) ;
			dlg.m_str3 = L"99:99";
			TCHAR buf12[100]={L""};
			if(CDataFile::Read(L"FLEXTIME",buf12))
				dlg.m_str3 = CString(buf12);
			if (dlg.DoModal() == IDOK)
			{
				CDataFile::Save(L"FLEXTIME",dlg.m_strAmount.GetBuffer(dlg.m_strAmount.GetLength()));
				dlg.m_strAmount.ReleaseBuffer();
			}

		}
	}
	else if(m_Key == L"PREAUTH AGING DAYS")
	{
		CAmountDlg dlg;
		dlg.m_str1 = CLanguage::GetText(CLanguage::IDX_ENTER_NEW) ;
		dlg.m_str2 = CLanguage::GetText(CLanguage::IDX_PREAUTH_AGING_DAYS_MENU) ;
		dlg.m_str3 = L"15";
		TCHAR buf[100]={L""};
		if(CDataFile::Read(m_Key,buf))
			dlg.m_str3 = CString(buf);
		dlg.m_Type = 2;
		if (dlg.DoModal() == IDOK)
		{
			CDataFile::Save(m_Key,dlg.m_strAmount.GetBuffer(dlg.m_strAmount.GetLength()));
			dlg.m_strAmount.ReleaseBuffer();
		}
	}
	else if ( m_Key == L"CLEARBATCH")
	{
		TCHAR buf[10]={L""};
		BOOL bTraining = FALSE;
		if(CDataFile::Read(L"TRAINING",buf))
			if(CString(buf) == L"On")
				bTraining = TRUE;

		CString strTemp[4];
		strTemp[0].Format(L"%s%s",CString(TRANSACTION_LOG_FILE),bTraining?L"TRAIN":L"");
		strTemp[1].Format(L"%s%s",CString(TOTALS_FILE),bTraining?L"TRAIN":L"");
		strTemp[2].Format(L"%s%s",CString(BATCH_LOG_FILE),bTraining?L"TRAIN":L"");
		strTemp[3].Format(L"%s",bTraining?L"":CString(HOST_TOTALS_FILE));
		CDataFile::UpdateStatistics(0,TRUE);

		for(int i=0; i<= 3;i++)
		{
			if(strTemp[i].GetLength() == 0)
				continue;

			char *FileName = alloc_tchar_to_char(strTemp[i].GetBuffer(strTemp[i].GetLength()));

			CRecordFile file;
			AP_FILE_HANDLER fh;
			if (!file.Open(FileName, &fh))
			{
				TRACE(L"Can't open file!\n");
				strTemp[i].ReleaseBuffer();
				continue;
			}
			free(FileName);
			strTemp[i].ReleaseBuffer();
			file.Delete();
		}
		CDataFile::Delete(FID_CLOSE_BATCH_FLAG);
	}
	else if ( m_Key == L"BAUD RATE")
	{
		if(m_strMsg.m_szCaption.Find(L"2400")>0)
			CDataFile::Save(m_Key,L"1200");
		else
			CDataFile::Save(m_Key,L"2400");
	}
	else if(m_strMsg.m_szCaption.Find(L"On")>0)
		CDataFile::Save(m_Key,L"Off");
	else
	{
		if(m_Key == L"TIP PROMPT")
		{
			CTipOptionDlg dlg;
			dlg.DoModal();
			dlg.m_iOption = 2;
			dlg.DoModal();
			dlg.m_iOption = 3;
			dlg.DoModal();
		}
		else if(m_Key == L"STATIC IP")
		{
			CStaticIPDlg dlg;
			if ( dlg.DoModal() != IDOK)
			{
				m_Bt.ShowWindow(SW_SHOW);
				m_Bt1.ShowWindow(SW_SHOW);
				return;
			}
		}
		else if(m_Key == L"CASHBACK")
		{
			TCHAR buf[10]={L""};
			CEnterAmountDlg dlg;
			char* pbuf;
			CLanguage::GetTextFromIndex(CLanguage::IDX_CASHBACK_MENU,&pbuf);
			dlg.m_Title = CString(pbuf);
			dlg.m_bShowConfirm = FALSE;
			dlg.m_iMaxAmountLen = 4;
			USHORT value = 0;
			DWORD size;
			if (CDataFile::Read((int)FID_CFG_SURCHARGE_LIMIT, (BYTE *)&value, &size))
				dlg.m_iMixAmount = (int)value;

			if ( CDataFile::Read(L"CASHBACKFEE",buf))
				dlg.m_strAmount = CString(buf);
			else
				dlg.m_strAmount =L"$0.00";

			if ( dlg.DoModal() == IDOK )
			{
				CDataFile::Save(L"CASHBACKFEE",dlg.m_strAmount.GetBuffer());
				dlg.m_strAmount.ReleaseBuffer();
			}	

			CEnterAmountDlg dlg1;
			dlg1.m_Title = CString(pbuf);
			dlg1.m_strText = CLanguage::GetText(CLanguage::IDX_ENTER_MAXIMUM);//L"Enter Maximum";
			dlg1.m_bShowConfirm = FALSE;

			dlg1.m_iMinAmountLen = 1;
			if ( CDataFile::Read(L"CASHBACKMAX",buf))
				dlg1.m_strAmount = CString(buf);
			else
				dlg1.m_strAmount =L"$0.00";//L"$99,999.99";

			if ( dlg1.DoModal() == IDOK )
			{
				CDataFile::Save(L"CASHBACKMAX",dlg1.m_strAmount.GetBuffer());
				dlg1.m_strAmount.ReleaseBuffer();
			}		
		}
		else if(m_Key == L"SURCHARGE")
		{
			TCHAR buf[10]={L""};
			char* pbuf;
			CLanguage::GetTextFromIndex(CLanguage::IDX_SURCHARGE_MENU,&pbuf);
			CEnterAmountDlg dlg;
			dlg.m_Title = CString(pbuf);
			dlg.m_strText = CLanguage::GetText(CLanguage::IDX_SURC_THRESHOLD);//L"Set Threshold";
			dlg.m_bShowConfirm = FALSE;
			dlg.m_iMaxAmountLen = 5;

			if ( CDataFile::Read(L"SURCHARGETHRESHOLD",buf))
				dlg.m_strAmount = CString(buf);
			else
				dlg.m_strAmount =L"$0.00";//L"$99,999.99";

			if ( dlg.DoModal() == IDOK )
			{
				CDataFile::Save(L"SURCHARGETHRESHOLD",dlg.m_strAmount.GetBuffer());
				dlg.m_strAmount.ReleaseBuffer();
			}	

			CEnterAmountDlg dlg1;
			dlg1.m_Title = CString(pbuf);
			dlg1.m_bShowConfirm = FALSE;
//			dlg1.m_iMixAmount = 0;
			USHORT value = 0;
			DWORD size;
			if (CDataFile::Read((int)FID_CFG_SURCHARGE_LIMIT, (BYTE *)&value, &size))
				dlg1.m_iMixAmount = (int)value;

			dlg1.m_iMaxAmountLen = 4;
			if ( CDataFile::Read(L"SURCHARGEFEE",buf))
				dlg1.m_strAmount = CString(buf);
			else
				dlg1.m_strAmount =L"$0.00";

			if ( dlg1.DoModal() == IDOK )
			{
				CDataFile::Save(L"SURCHARGEFEE",dlg1.m_strAmount.GetBuffer());
				dlg1.m_strAmount.ReleaseBuffer();
			}		
		}

		CDataFile::Save(m_Key,L"On");
	}

	m_strMsg.SetCaption(CLanguage::GetText(CLanguage::IDX_CHANGE));
	m_strMsg1.SetCaption(CLanguage::GetText(CLanguage::IDX_CONFIRMED));
	m_strMsg2.ShowWindow(SW_HIDE);
	SetTimer(1,3000,NULL);
	m_iLevel = 2;
}


void CParamSettingDlg::OnTimer(UINT_PTR nIDEvent)
{
	if(nIDEvent == 1)
		CDialog::OnOK();
	CBaseDlg::OnTimer(nIDEvent);
}

void CParamSettingDlg::OnDestroy()
{
	KillTimer(1);
	CBaseDlg::OnDestroy();
}

void CParamSettingDlg::OnCancel()
{
	Beep1();
	CDialog::OnCancel();
}

void CParamSettingDlg::OnOK()
{
	Beep1();
	if (m_iLevel == 1)
		OnBnClickedButton(IDC_BUTTON1);
	else
		CDialog::OnOK();
}

