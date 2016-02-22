#pragma once

#include "..\\DEFS\\struct.h"

#include "..\\UI\\MSR.h"
#include "..\\UI\\SCR.h"
#include "..\\UI\\MagRFID.h"
#include "..\\utils\\\Printer.h"

#include "EmvApi.h"
#include "..\\EMV\\EmvData.h"
#include "..\\EMV\\CallBack.h"
#include "..\\EMV\\SaioDev.h"
#include "..\\EMV\\EMV.h"

#include "..\\HOST\\RapidConnect.h"

class CFinan
{
	public:
		CFinan(void);
		~CFinan(void);

	TRAN_TranDataRec* m_pLogTRREC;

	CMSR m_MSR;
	CSCR m_SCR;
	CMagRFID m_RFID;

	void ReOpenRFID();

	BOOL OpenDevice();
	void CloseDevice(BOOL flag = TRUE);
	BOOL OpenVoidDevice();
	BOOL OpenMSRDevice();
	void CloseMSRDevice();
	void CloseVoidDevice();
	BOOL EnterOnlinePin();
	TRAN_TranDataRec m_TRREC;
	CONF_Data m_CONFIG;
	CRapidConnect * m_RapidConnect;

	int m_CustDialogTimer;
	int ProcessSwipedCard(char *pTrack2);
	void PutAmount(char* target,CString str);
	void PutAmountWithDot(char* target,CString str);
	void PutParam(char* target,CString str);
	CString DealAmountStr(char* p);

	CString DealPassword(UINT nChar,CString source);
	CString DealDevicePassword(UINT nChar,CString source);
	BOOL CheckAmount(CString source);
	CString DealAmount(UINT nChar,CString source);
	CString DealClerkID(UINT nChar,CString source);
	CString DealReceipt(UINT nChar,CString source);
	CString KeyAccount(UINT nChar,CString source);
	CString KeyExpiry(UINT nChar,CString source);
	CString DealFraud(UINT nChar,CString source);
	CString DealInvoice(UINT nChar,CString source);
	CString DealApprovalCode(UINT nChar,CString source);

	void GetInvoice();
	CString m_strPassword;
	CString m_strInvoice;
	CString m_strAmount;
	CString m_strCashback;
	CString m_strSurcharge;

	int m_iKeyStatus;
	void DoTransaction();
	static DWORD WINAPI ThreadProc (LPVOID lpArg);
	BOOL DoConfirm(BOOL Flag = FALSE,CWnd* pParent = NULL);

	BOOL m_KeyEntry	;
	int ProcessManualCard(char *pAccount);
	void AfterHost();
	void CheckAutoSet();


	BOOL m_bGoBackFlag;

	virtual void GoBack();
	virtual void GoNext();
	virtual void GoToLevel(int level);
	virtual void ErrorExit();
	virtual void ErrorExit(int Level);
	virtual void SetErrorCode(char* code, int m1, int m2);				//JC Apr 7/15 to pass message back to API);
	CString m_strErr, m_strErr1, m_strErr2;

	void Print(int status = 1);
 
	HWND m_MsgWnd;
	BOOL m_bPrintDone;

	BOOL m_bCancelPrint;
	BOOL m_bCancelInCustomer;
	void StartEMV();
	int DoEMV();
	BOOL EmvTransactionFlow(TXNCONDITION *pTxn);
	void ProcessEmvTLV(USHORT tlvLength, BYTE *tlv, BOOL sentToHost);
	void ProcessCtlsTagE0(USHORT tlvLength, BYTE *tlv);
	void ProcessCtlsTLV(USHORT tlvLength, BYTE *tlv);
	void ProcessEmvError();

	void Beep();
	BOOL CheckPassword(BOOL Manager = FALSE);
	int m_IncorrectTimes;
	BOOL m_bTraining;

// For EMV function

	BOOL m_iEMVStart;
	BOOL SupportEnglishOrFrench();
	void EMVSelectLanguage(int language);

	CString m_strCurrentApplicationLabel;
	BOOL EMVEnterPin();
	void SetEMVCardType();
	void SetKeyReturn(int Key);

	BOOL m_bCancelFlag;

	void SetEMVCancel();
	BOOL IsCancel();

	int m_iNextLevel;

	BOOL m_bCloseDevice;
	BOOL m_bCloseMSRDevice;

	BOOL m_bHavePrint;
private:

	CPrinter m_Printer;

	BOOL FullEmvTransaction(void);
	void ProcessTrack2EquivalentData(BYTE *data, USHORT length, BOOL saveLang);
	void FillAuthResponse(TXNCONDITION *pTxn, TRAN_EmvData *pEMVData, AUTHRESPONSE *pAuthResp);


	BOOL m_bEMV;
	HANDLE m_hEvent;
	int SelectAccount();
	int SelectLanguage();
	int m_iKeyReturn;
	void ProcessTran();
	BOOL HostInProgress;

	BOOL FullEmvTransacation();

	void DoParitalTxn();
	BYTE SetEMVEntryMode();
	CString ByteToStr(BYTE *bBuf, int Len =1,int format=1);

	BOOL declinedOffline;

	int ShowCashbackDlg();
	int ShowSurchargeDlg();
	int ShowSelectLanguageDlg();
	BOOL CheckDF62();
	BOOL IsPrimary(CString str);
	BOOL CheckNOTBlocked();

	BOOL m_Blocked;

public:
	int EMVDialog();
	BOOL EMV_PIN_MODE;
	BOOL m_bFraud;
	BOOL CheckFraud(CString str);
	BOOL CheckMMYY(CString str);
	BOOL m_bSendECRBackFlag;
	void GetTxnResult( TXN_Result* pTxnResult);
	void DoECRReversal();

};
