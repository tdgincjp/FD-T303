#pragma once

#include "StaticText.h"
#include "..\\data\\LogFile.h"

// CSettingDlg dialog

class CSettingDlg : public CDialog
{
	DECLARE_DYNAMIC(CSettingDlg)

public:
	CSettingDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CSettingDlg();

	CStaticText m_strTitle;
	CStaticText m_strInputPW[2];
	CStaticText m_strBtText[6];
	CStaticText m_strHostBt[2];
	CStaticText m_strTermIdBt[6];
	CStaticText m_strTimeZoneBt[8];
	CStaticText m_strCopyLog[2];				
	CStaticText m_strRestart;

	CStaticText m_strIPAddrBt[10];
	CStaticText m_strIPAddrText;

// Dialog Data
	enum { IDD = IDD_SETTINGDLG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	int screenX;  
	int screenY;  

	int m_iLevel;

	DECLARE_MESSAGE_MAP()

public:
	virtual BOOL OnInitDialog();
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	virtual void OnOK();
	virtual void OnCancel();
	BOOL m_ExitFlag;
	BOOL m_DebugApiOn;						//JC Nov 11/15 Debug API to debug.txt 
	
private:
	int textsize;
	CLogFile m_LogFile;
	void ShowMainGUI(BOOL flag);
	void ShowMore(BOOL flag = TRUE);
	void ShowHost(BOOL flag = TRUE);
	void ShowTermId(BOOL flag = TRUE);
	void ToggleHost(BOOL flag = TRUE);					//JC Oct 28/15
	void EditTermId(BOOL flag = TRUE);
	void ShowTimeZone(BOOL flag = TRUE);
	void ShowCopyLog(BOOL flag = TRUE);
	void ShowCopyLogResults(BOOL flag, CString line, CString line1);
	void ShowDebugApi(BOOL flag = TRUE);				//JC Nov 11/15 Add debug API
	//void ShowRestart(BOOL flag = TRUE);
	bool CSettingDlg::DisplayMIDwarning();				//JC Nov 26/15

	void ShowIPAddr(BOOL flag = TRUE);
	void EditStaticIP(BOOL flag = TRUE);

	void CreateMainGUIBt();
	void CreateMoreGUIBt();
	void CreateHostBt();
	void CreateTermIdBt();
	void CreateTimeZoneBt();
	void CreateRestartBt();
	void CreateLogBt();

	void CreateIPAddrBt();
	void InputIP(int id);
	int m_iInputID;

	BOOL CheckPassword();
	void Beep1();
	CString m_strPassword;
	int m_IncorrectTimes;
	CString DealPassword(UINT nChar,CString source);
	BOOL m_bMerchantIDInputFlag;
	void CopyLog();
	void Exit();
	void GoBack();
	CString SaveMID;									//JC Nov 23/15 Save MID and TID
	CString SaveTID;									//JC Nov 23/15 Save MID and TID



	//UTILITY CLASSES for secret password
	void ClearLastKeySequence();
	void AddLastKeySequence(UINT nChar);
	void shift_left (int myarray[], int size);
	BOOL compare_array(int array1[], int array2[], int size);
	int m_lastKeySeq[8];
	int m_keySeqPos;

	BOOL ValidDigit(char *);											
	BOOL IsValid_IP(TCHAR * pIP);
	BOOL m_ShowIPError;

};
