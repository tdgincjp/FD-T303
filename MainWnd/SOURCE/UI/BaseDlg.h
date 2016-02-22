#pragma once

#include "..\\..\\resource.h"
#include "StaticText.h"
#include "MyButton.h"
#include "Language.h"
#include "..\\data\\DataFile.h"
#include "..\\data\\DebugFile.h"				//JC Nov 11/15 to add debug logging 


#define SZ_CUSTOMER_TEXT					100
#define SZ_ERRCODE_TEXT						4
// CBaseDlg dialog

class CBaseDlg : public CDialog
{
	DECLARE_DYNAMIC(CBaseDlg)

public:
	CBaseDlg(CWnd* pParent = NULL);   // standard constructor
	CBaseDlg(int ID,CWnd* pParent = NULL);   // standard constructor

	virtual ~CBaseDlg();

	CStaticText m_strTitle;
	CStaticText m_strMsg;
	CStaticText m_strMsg1;
	CStaticText m_strMsg2;

	CString m_Title;
	CString m_Key;

	int m_TitleIndex;

	void ShowText(const CString str1,const CString str2,const CString str3);
	void ShowPicture(int flag = 0);
	char m_ErrCode[SZ_ERRCODE_TEXT+1];
	char m_Customer_Text[SZ_CUSTOMER_TEXT+1];										//JC Mar 13/15 

// Dialog Data
	enum { IDD = IDD_BASEDLG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	int screenX;  
	int screenY;  

	int m_iLevel;
	int m_iECRLevel;

	int m_iPos;
	virtual void Done();

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnPaint();
	virtual BOOL OnInitDialog();
	virtual afx_msg void OnTimer(UINT_PTR nIDEvent);
	virtual	afx_msg LRESULT SockRecvData(WPARAM wParam, LPARAM lParam);
	virtual afx_msg void OnDestroy();
	virtual BOOL PreTranslateMessage(MSG* pMsg);

	BOOL IsNetWorkConnect();

	void Beep1();
	int m_iDisplayErrorTime;

private:
	void CheckNetWorkStatus();
};
