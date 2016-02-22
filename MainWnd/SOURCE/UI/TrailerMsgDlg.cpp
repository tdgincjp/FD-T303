#include "stdafx.h"
#include "TrailerMsgDlg.h"
#include "..\\data\\RecordFile.h"

#define MODE_INPUT				1
#define ENTER_LINE1				2
#define ENTER_LINE2				3
#define ENTER_LINE3				4
#define ENTER_LINE4				5
#define CHANGE_COMFIRM			6

IMPLEMENT_DYNAMIC(CTrailerMsgDlg, CBaseDlg)
CTrailerMsgDlg::CTrailerMsgDlg(CWnd* pParent )
	: CBaseDlg(CBaseDlg::IDD, pParent)
{
	m_iType = 1;
}

CTrailerMsgDlg::~CTrailerMsgDlg(void)
{
}

void CTrailerMsgDlg::DoDataExchange(CDataExchange* pDX)
{
	CBaseDlg::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CTrailerMsgDlg, CBaseDlg)
		ON_COMMAND_RANGE(IDC_BUTTON1, IDC_BUTTON2, &CTrailerMsgDlg::OnBnClickedButton)
		ON_WM_CHAR()
END_MESSAGE_MAP()

// CMainWnd message handlers
BOOL CTrailerMsgDlg::OnInitDialog()
{
	if ( m_iType == 2) //header
	{
		m_Key = L"HEADER MSG";
		m_Title = CLanguage::GetText(CLanguage::IDX_HEADER_MSG);
	}
	else
	{
		m_Key = L"TRAILER MSG";
		m_Title = CLanguage::GetText(CLanguage::IDX_TRAILER_MSG);
	}

	CBaseDlg::OnInitDialog();

	m_iLevel = MODE_INPUT;

//	m_strMsg.SetPointFont(true,160);

	m_strMsg.ShowWindow(SW_SHOW);
	m_strMsg1.ShowWindow(SW_SHOW);
	m_strMsg2.ShowWindow(SW_SHOW);

	m_Bt.SetValue(3, CLanguage::GetText(CLanguage::IDX_BT_YES),IDB_OK);
	m_Bt.SubclassDlgItem(IDC_BUTTON1, this);
	m_Bt.MoveWindow(screenX-128,  screenY- 50, 120,45);
	m_Bt.ShowWindow(SW_SHOW);

	m_Bt1.SetValue(1,CLanguage::GetText(CLanguage::IDX_BT_NO),IDB_CANCEL);
	m_Bt1.SubclassDlgItem(IDC_BUTTON2, this);
	m_Bt1.MoveWindow( 8, screenY- 50, 120, 45);
	m_Bt1.ShowWindow(SW_SHOW);

	m_Bt2.SetValue(-1,CLanguage::GetText(CLanguage::IDX_SOFT_KEYBOARD));
	m_Bt2.SubclassDlgItem(IDC_BUTTON3, this);
	m_Bt2.MoveWindow(70,  screenY- 50, 180,45);
//	m_Bt2.ShowWindow(SW_SHOW);

	WorkLoop(m_iLevel);
	SetTimer(1,50,NULL);

	return TRUE;  // return TRUE unless you set the focus to a control
}

void CTrailerMsgDlg::OnTimer(UINT_PTR nIDEvent)
{
	if(nIDEvent == 1)
	{
		SetFocus();
		KillTimer(1);
		return;
	}
		
	if(nIDEvent == 2)
	{
		KillTimer(2);
		CDialog::OnCancel();
		return;
	}

	CBaseDlg::OnTimer(nIDEvent);
}

void CTrailerMsgDlg::WorkLoop(int index)
{
	TCHAR buf[100]={L""};

	CString str=L"";
	CString str1=L"";
	int i = 0;
	switch(index)
	{
	case MODE_INPUT: //Mode choice
		m_Bt.ShowWindow(SW_SHOW);
		m_Bt1.ShowWindow(SW_SHOW);
		str.Format( L"%s:[ Off ]",CLanguage::GetText(CLanguage::IDX_MODE));
		if(CDataFile::Read(m_Key,buf))
			str.Format(L"%s:[ %s ]",CLanguage::GetText(CLanguage::IDX_MODE),buf);
		ShowText(str,CLanguage::GetText(CLanguage::IDX_CHANGE)+L"?",L"");
		break;
	case ENTER_LINE1:  //enter new line
//		m_strMsg1.SetPointFont(true,160);
	case ENTER_LINE2:  //enter new line
	case ENTER_LINE3:  //enter new line
	case ENTER_LINE4:  //enter new line
		m_Bt.ShowWindow(SW_HIDE);
		m_Bt1.ShowWindow(SW_HIDE);
		m_Bt2.ShowWindow(SW_SHOW);
/*
		if (m_iType == 2)
		{
			str1 = CString(bufText1);
			if(CDataFile::Read(L"HeaderLine",buf))
				str1.Format(L"%s",buf);
			str = L"HeaderLine";
			ShowText(str,str1,L"");
			break;
		}
*/
		if (m_iType == 2)
		{
			str.Format(L"HeaderLine%d",index-ENTER_LINE1+1);
//			str1.Format(L"Header line %d.",index-ENTER_LINE1+1);
			str1.Format(L"%s %d.",CLanguage::GetText(CLanguage::IDX_HEADER_LINE),index-ENTER_LINE1+1);
		}
		else
		{
			str.Format(L"TrailerLine%d",index-ENTER_LINE1+1);
//			str1.Format(L"Trailer line %d.",index-ENTER_LINE1+1);
			str1.Format(L"%s %d.",CLanguage::GetText(CLanguage::IDX_TRAILER_LINE),index-ENTER_LINE1+1);
		}
//		str1 = CString(bufText[ index-ENTER_LINE1 ]);
		if(CDataFile::Read(str,buf))
			str1.Format(L"%s",buf);

		str.Format(L"   %s #%d",CLanguage::GetText(CLanguage::IDX_LINE),index-ENTER_LINE1+1);
		ShowText(str,str1,L"");
		break;
	case CHANGE_COMFIRM:  //comfirm
	//	SaveData();
		ShowText(CLanguage::GetText(CLanguage::IDX_CHANGE), CLanguage::GetText(CLanguage::IDX_CONFIRMED)+L"!",L"");
		SetTimer(2,3000,NULL);
		break;
	}
}

void CTrailerMsgDlg::SaveData()
{
	CString str;
	for(int i=0;i<4;i++)
	{
		str.Format(L"TrailerLine%d",i+1);
		CDataFile::Save(str,m_strLine[i].GetBuffer());
		m_strLine[i].ReleaseBuffer();
	}
}

void CTrailerMsgDlg::OnCancel()
{
	Beep1();

	if ( m_iLevel == MODE_INPUT)
	{
		GoNext();
		return;
	}

	CDialog::OnCancel();
}

void CTrailerMsgDlg::OnOK()
{
	Beep1();
	CString str;
	switch(m_iLevel)
	{
	case MODE_INPUT: //mode choice
		OnBnClickedButton(IDC_BUTTON1);
		return;
	case ENTER_LINE1:  //enter new line
/*		if (m_iType == 2 )
		{
			CDataFile::Save(L"HeaderLine",m_strMsg1.m_szCaption.GetBuffer());
			m_strMsg1.m_szCaption.ReleaseBuffer();
			CDialog::OnOK();
			return;
		}
*/	case ENTER_LINE2:  //enter new line
	case ENTER_LINE3:  //enter new line
	case ENTER_LINE4:  //enter new line
		if (m_iType == 2 )
			str.Format(L"HeaderLine%d",m_iLevel-ENTER_LINE1+1);
		else
			str.Format(L"TrailerLine%d",m_iLevel-ENTER_LINE1+1);
		CDataFile::Save(str,m_strMsg1.m_szCaption.GetBuffer());
		m_strMsg1.m_szCaption.ReleaseBuffer();

//		CDataFile::Save(m_Title,m_strMsg2.m_szCaption.GetBuffer());
//		m_strMsg2.m_szCaption.ReleaseBuffer();
		break;
	default:
		CDialog::OnOK();
		return;
	}
	GoNext();
	SetFocus();
}

void CTrailerMsgDlg::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	Beep1();
	CString str;
	int len;
	switch(m_iLevel)
	{
	case MODE_INPUT: // mode choice
		if (nChar=='3')
			OnBnClickedButton(IDC_BUTTON1);
		if (nChar=='1')
			OnBnClickedButton(IDC_BUTTON2);
		if(nChar == 8)
			Beep();
		break;
	case ENTER_LINE1:  //enter new line
	case ENTER_LINE2:  //enter new line
	case ENTER_LINE3:  //enter new line
	case ENTER_LINE4:  //enter new line
		len = m_strMsg1.m_szCaption.GetLength();
		if(nChar == 8)
		{
			if(len > 0)
			{
				str = m_strMsg1.m_szCaption.Mid(0,len-1);
				m_strMsg1.SetCaption(str);
			}
			else
				Beep();
			return;
		}
		if(len >= 24)
		{
			Beep();
			return;
		}
		str.Format(L"%s%c",m_strMsg1.m_szCaption,nChar);
		m_strMsg1.SetCaption(str);
		break;
	}
	MSG msg; 
	 while(::PeekMessage(&msg,NULL,0,0,PM_REMOVE))
	 {
		 if (msg.message==WM_PAINT || msg.message== ERROR_MESSAGE)
			::DispatchMessage(&msg);
	}
	SetFocus();
}

void CTrailerMsgDlg::OnBnClickedButton(UINT nID)
{
//	if( nID == IDC_BUTTON2)
//	{
//		CDialog::OnCancel();
//		return;
//	}

	m_Bt.ShowWindow(SW_HIDE);
	m_Bt1.ShowWindow(SW_HIDE);
		// save and comfirm
	if( nID == IDC_BUTTON2)
	{
		GoNext();
//		CDialog::OnCancel();
		return;
	}

	if(m_strMsg.m_szCaption.Find(L"On")>0)
	{
		CDataFile::Save(m_Key,L"Off");
		GoNext();
//		GoToLevel(CHANGE_COMFIRM);
	}
	else
	{
		CDataFile::Save(m_Key,L"On");
		GoNext();
	}

}

void CTrailerMsgDlg::Done()
{
}

void CTrailerMsgDlg::OnDestroy()
{
	CBaseDlg::OnDestroy();
}

void CTrailerMsgDlg::GoBack()
{

}

void CTrailerMsgDlg::GoNext()
{
	m_iLevel++;
	PostMessage( ERROR_MESSAGE, 2,2 );
}

void CTrailerMsgDlg::GoToLevel(int level)
{
	m_iLevel = level;
	PostMessage( ERROR_MESSAGE, 2,2 );
}

LRESULT CTrailerMsgDlg::DefWindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	if(wParam == 2 && message == ERROR_MESSAGE)
	{ 
		if(lParam == 2) //for jump to other level
			WorkLoop(m_iLevel);
	}
	return CDialog::DefWindowProc(message, wParam, lParam);
}

