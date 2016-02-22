#pragma once
#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif
#include <msxml2.h>
#include "..\\defs\\struct.h"
#include "..\\COMM\\SSLCon.h"
#include "DataWire.h"
#include "msxml2.h"
#import "MSXML3.dll" rename_namespace(_T("MSXML"))

class CRapidConnect 
{
	public:
	CRapidConnect(HWND hwnd);  
	~CRapidConnect();
	void Start (TRAN_TranDataRec *pTranReq, CONF_Data *pConfig, BOOL stopflag = TRUE); //public entry point into Rapid Connect 
	void Stop();
	HWND m_hWnd;
	int m_TranStatus;
	CDataWire * pDataWire;
	
	private:
	FD_Rec * pFDTranRec;
	int m_iDownloadStage;
	int m_iFBSeqNum;
	string m_sHBINTblVer;
	TRAN_TranDataRec *pTRANDATA;
	CONF_Data *m_pConfig;
	MSXML::IXMLDOMDocumentPtr m_IXMLDOMDocument;  


	////////////////TRANSACTION METHODS
	BOOL			CRapidConnect::GenerateTransactionPayload();			//This will create the <CreditRequest> or <DebitRequest> that will serve as the transaction payload
	BOOL			CRapidConnect::SendHttpsRequest(const char * request, BOOL);  //Here we create the connection to TxnURL1 or TxnURL2, send the request BOOL controls Write to DebugLog JC NOv 9/15
	BOOL			CRapidConnect::ParseXmlResponse(CString response);		//Here we parse the response, apply logic to return correct code to the user (approve / decline)	
	BOOL			CRapidConnect::ParseDatawireResponse(CString response);
	////////////////DOWNLOAD METHODS
	void			CRapidConnect::DoHostDownload(int *);						//Host download section, get merchant info and BIN table info. 
	BOOL			CRapidConnect::GenerateDownloadPayload(int iDLReqNum);	//This will create the <FileDownload> request that will serve as the payload
	BOOL			CRapidConnect::ParseDownloadResponse(CString response);	//Parse the download response
	void			CRapidConnect::SetInitStatus(int status);
	////////////////SHARED METHODS
	void			CRapidConnect::AddSharedCommonGroupElements(MSXML::IXMLDOMDocument2Ptr pXMLDoc, MSXML::IXMLDOMElementPtr pXmlCommonGrp);
	const char *	CRapidConnect::GetXmlValue(IXMLDOMNodeListPtr pIDOMNodeList, string value);
	BOOL			CRapidConnect::ApplyGMTDateOffset(char * dtLocal, char *dtGmt);


	char* error;
	int ErrorCode;

	char* GetErrorCode();
	char* GetErrorText();

};


typedef enum
{
	CardType_Visa =100,
	CardType_MasterCard,
	CardType_Amex, 
	CardType_Discover,
	CardType_Diners,
	CardType_JCB,
	CardType_PropietaryDebit
} eRapidConnectCardType;
