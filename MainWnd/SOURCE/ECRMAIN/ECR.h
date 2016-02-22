// ECR.h : header file
//

#pragma once
#include "afxwin.h"

// CECR dialog
class CECR : public CDialog
{
// Construction
public:
	CECR(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_XACECR_DIALOG };
	enum
	{
		TRAN_UNKNOWN = 0,

		// Financial Transactions
		FUNCTION_FINAN_BEGIN = 20,
			TRAN_PURCHASE,
			TRAN_PREAUTH,
			TRAN_ADVICE,
			TRAN_FORCE_POST,
			TRAN_REFUND,
			TRAN_PURCHASE_VOID,
			TRAN_REFUND_VOID,
		FUNCTION_FINAN_END,

		FUNCTION_GENERAL_BEGIN,
			TRAN_CANCEL,
		FUNCTION_GENERAL_END,

		/* Administrative Transactions */
		FUNCTION_ADMIN_BEGIN = 100,
			TRAN_EXTENDED_CFG,
			TRAN_HOST_INIT,
		FUNCTION_ADMIN_END,

		FUNCTION_BATCH_BEGIN = 150,
			TRAN_CLOSE_BATCH,
		FUNCTION_BATCH_END,

		FUNCTION_UTIL_BEGIN = 200,
			TRAN_GET_TRACK_DATA,
			TRAN_GET_PED_SERIAL,
		FUNCTION_UTIL_END,
		
		FUNCTION_EXIT_APP_BEGIN = 600,
			FUNCTION_EXIT_APP,
		FUNCTION_EXIT_APP_END,

		// keep this last
		END_OF_TRAN_ID
	};

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support

// Implementation
protected:
	HICON m_hIcon;

	static DWORD WINAPI ThreadProc (LPVOID lpArg);

	// Generated message map functions
	virtual BOOL OnInitDialog();
#if defined(_DEVICE_RESOLUTION_AWARE) && !defined(WIN32_PLATFORM_WFSP)
	afx_msg void OnSize(UINT /*nType*/, int /*cx*/, int /*cy*/);
#endif
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedHostinit();
	afx_msg void OnBnClickedPurchase();
	afx_msg void OnBnClickedRefund();
	afx_msg void OnBnClickedConfig();
	afx_msg BOOL OnCopyData(CWnd* pWnd, COPYDATASTRUCT* pCopyDataStruct);
	afx_msg void OnTimer(UINT nIDEvent);

	afx_msg LRESULT ShowMenu(WPARAM wParam, LPARAM lParam);
	BOOL Write(CString txt);
protected:
	virtual void OnCancel();
	void OnOK();

public:
	afx_msg void OnBnClickedExit();
	afx_msg void OnPaint();

private:

	CFont m_font;

	CString m_Request;
	CString m_Response;

	int GetTerminalId();
	long GetAmount();
	long GetInvoice();
	inline void Move(char c);
	inline void Move(const char *str);
	inline void Move(const CString str);
	void MoveTransactionCode(int iTxnType);
	void MoveTerminalId();
	void MoveAmount();
	void MoveInvoice();
	CString FormatEcrRequest(int iTxnType);
	void ParseEcrResponse();

	int  m_iMenuState;
	void ChangeMenu(int state);

	int m_iTimeStep;
	CString m_strTemp;
	int m_iTimer;
	int ProcessSwipedCard(char *pTrack2);
	CString GetRand(int len);
	CString m_Title;
	BOOL    m_bFlag;

	void OnCloseBatch();

	BOOL    m_bCancel;
public:
	afx_msg void OnLbnSelchangeList1();
	CListBox m_List;
	void DisplayText(CPaintDC* dc);
};

