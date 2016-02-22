#pragma once
#include "afxwin.h"
#include "SaioReader.h" // necessary include file to use HAL reader API.
#include "..\\EMV\\ByteStream.h"
#include "..\\defs\\Constant.h"
#include "..\\utils\\BinUtil.h"

class CMagRFID
{
public:
	CMagRFID(void);
	~CMagRFID(void);

	virtual BOOL Open(HWND hwnd, char *amount, unsigned short txnCode,int iAmount);
	void Cancel();
	void Reset();
	void GetData();
	BOOL GetData(BYTE* buf, DWORD* len);
	BOOL Config();
	BOOL ConfigMsr();

private:
	BYTE m_Data[1024];
//	sCtlsAppInstalled m_AppInstalled[10];
	DWORD m_NumOfAppInstalled;
	DWORD m_Len;
	BOOL SetMasterCardApplication();
	BOOL SetMasterCardTermCapability(int amount);	
	BOOL SetMasterCardMChipMode();
	BOOL SetMasterCardAppParam();

	BOOL SetVisaApplication(int n, sEmvData *pEmvData, 
																 sCtlsParams *pCtlsData,
																 BOOL isEMV);

	BOOL SetVisaApplicationMsr();
	BOOL SetVisaTermCapabilityMsr();	
	BOOL SetVisaAppParam();
	BOOL SetAmexApplicationMsr();
	BOOL SetAmexApplication(int n, sEmvData *pEmvData, 
																 sCtlsParams *pCtlsData);
	BOOL SetAmexTermCapability();	
	BOOL SetAmexAppParam();
	BOOL SetInteracApplication(int n, sEmvData *pEmvData, 
																 sCtlsParams *pCtlsData);
	BOOL SetInteracTermCapability();	
	BOOL SetInteracAppParam(BYTE MTI);

	BOOL SetTermEmvData();
	BOOL SetTermEmvDataMsr();
	BOOL DeleteApp();

	BOOL InstallCAKey();
	int GetPaypassCvmLimit();

	BOOL SetCAKey();
};
