#pragma once
#include "basedlg.h"
#include "..\\COMM\\RawCom.h"
#include "..\\DEFS\\CommMessage.h"
#include "..\\UTILS\\IpAdapter.h"
#include "SettingDlg.h"

class CECRiCommDlg :
	public CBaseDlg
{
	DECLARE_DYNAMIC(CECRiCommDlg)

public:
	CECRiCommDlg(CWnd* pParent = NULL);
	virtual ~CECRiCommDlg(void);
	virtual void OnCancel();
	virtual void OnOK();

public:
	static DWORD WINAPI ThreadProc (LPVOID lpArg);
	static DWORD WINAPI ThreadProc1 (LPVOID lpArg);
	void Recv_Proc();

protected:

	void Done();

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual LRESULT DefWindowProc(UINT message, WPARAM wParam, LPARAM lParam);

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();

//	afx_msg LRESULT SockRecvData(WPARAM wParam, LPARAM lParam);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	BOOL			DbgWriteOn;					//JC Feb 5/15
	BOOL			DbgNext20;					//JC Nov 11/15 is On will write next 20 api command pairs to debug.txt
	int				DbgNext20Count;				//JC Nov 11/15

private:
	void OpenComm();
	void OpenCommDbg();							//JC Feb 2/15
	void WriteCommDbg(BYTE *buf, DWORD len );
	LPCOMMINFO		hCom;
	LPCOMMINFO		hComDbg;					//JC Feb 2/15 - Added for debug data out COM2:

	BYTE			gbBuffer[BUFFERSIZE];
	BYTE			ReadBuffer[BUFFERSIZE];
	DWORD           m_DataRead;
	DWORD			dwCommWatchThreadID;
	HANDLE			hCommWatchThread;
	void AscToByte(LPCTSTR strAsc, BYTE *lpBuffer, LPDWORD lpByteLen);
	void CloseComm(); 
	void CheckNetWorkStatus();
	IpAdapter m_NetworkAdapter;

	void ParseCommData();
	void DealCommData();

	BOOL CheckCRC(UCHAR* buf, int len);

    DWORD    dwBytesToWrite;

	int  m_iExit;
	SAIO_Info m_INFO;

public:
	afx_msg void OnDestroy();
	afx_msg BOOL OnCopyData(CWnd* pWnd, COPYDATASTRUCT* pCopyDataStruct);

private:
	void OnShowPic(int index);
	int AddToTlvBuff(char *Tag, USHORT TagLen, BYTE *pTagData, BYTE *pTargetBuff, USHORT MaxBuffLen);				//JC Feb 5/15 to create wrapper for debug

	int m_iPicture;
	BOOL DecodeHostAddress(BYTE *tagData, int len, BYTE *wHost, BYTE *wPort, BOOL FormatCheck);		//JC Jan 12/15
	BOOL ValidDigit(char *);											// JC Jan 10/15
	BOOL IsValid_IP(char *);											// JC Jan 10/15

	char* GetTagName(unsigned short tag);
	BOOL IsMandatoryTag(int tag);
	BOOL CheckDataLength(int tag, int len);
	BOOL CheckDataFormat(int tag, BYTE *data, int len, ECR_TranDataRec *pECRREC);		//JC Dec 31/14 add ECR_TranDataRec
	void SaveTag(int tag, BYTE *tagData, int len, ECR_TranDataRec *pECRREC);
	DWORD ParseEcrHeader(BYTE* buf, int len, ECR_TranDataRec *pECRREC);
	DWORD ParseEcrRequest(BYTE* buf, int len, ECR_TranDataRec *pECRREC);
	void FormatEcrHeader(ECR_TranDataRec *pECRREC, BYTE *buf, int *len);
	void FormatEcrResponse(ECR_TranDataRec *pECRREC, BYTE* buf, int *len);

	void InitEmv();
	void SendResponse(int value);

	int m_iCommSendTime;
	void RebindAdapter( TCHAR *adaptername );

	BOOL SetIPAddress(char* buf);
	BOOL m_bHavePrint;

	BOOL m_bBusy;
	BOOL m_bTransactionInProgress;
	BOOL display_message;
	CString m_str1, m_str2, m_str3;
	BOOL isDevMode;

	int m_iModeState;
	void CheckModeState();
	int m_iPressKeyTime;
	int m_iShowInfoTime;

	CString version;


	HWND m_hTxnWnd;
	void DoReversal();
	ECR_TranDataRec ECRREC;
	ECR_TranDataRec saveECRREC;			//JC Oct 28/15 to save old record from being overwritten by status or cancel

	int m_iExitPressKeyTime;

};

