// SslCon: interface for the CSslConnection class.
#pragma once

#include <Wininet.h>
#include <wincrypt.h>

#pragma warning(disable:4786)

#pragma comment(lib, "wininet.lib")
#pragma comment(lib, "Crypt32.lib")

#include <string>
using namespace std;

enum CertStoreType {certStoreMY, certStoreCA, certStoreROOT, certStoreSPC};

class CSslConnection  
{
public:
	CSslConnection(HWND hwnd);
	virtual ~CSslConnection();	
public:	
	bool ConnectToHttpsServer(string strVerb);
	bool SendHttpsRequest(){return SendHttpsRequest(NULL, 0);}
	bool SendHttpsRequest(
						__in_bcount_opt(dwOptionalLength) IN LPVOID lpOptional OPTIONAL,
						IN DWORD dwOptionalLength);
	string GetRequestResult();
public: //accessors
	void SetAgentName(string &strAgentName) { m_strAgentName = strAgentName; }
	void SetCertStoreType(CertStoreType storeID) { m_certStoreType = storeID; }
	void SetServerName(string &strServerName) { m_strServerName = strServerName; }
	void SetObjectName(string &strObjectName) { m_strObjectName = strObjectName; }
	void SetPort(INTERNET_PORT wPort = INTERNET_DEFAULT_HTTPS_PORT) { m_wPort = wPort; }
	void SetRequestID(int reqID) { m_ReqID = reqID; }
	void SetSecurityFlags(int flags) { m_secureFlags = flags; }
	void SetHttpHeaders(string &strHeaders) { m_strHeaders = strHeaders; }
	//Search indicators	
	void SetOrganizationName(string &strOName) { m_strOName = strOName;} 
	//void AddPostData(string &strPostData) {m_strPost = strPostData;}
	void AddCertToCertStore();
	string GetLastErrorString() { return m_strLastError; }
	int GetLastErrorCode() { return m_lastErrorCode; }
	bool SetClientCert();
private:
	// examine the following function in order to perform different certificate 
	// property searchs in stores. It detects the desired certificate with the organization name
	PCCERT_CONTEXT FindCertWithOUNITName();	
	/////////////////////////////////////
	void ClearHandles();
private:
	HINTERNET m_hInternet;
	HINTERNET m_hRequest;
	HINTERNET m_hSession;
	
	string m_strServerName;
	string m_strObjectName;
	INTERNET_PORT m_wPort;
	int m_secureFlags;

	HCERTSTORE m_hStore;
	PCCERT_CONTEXT m_pContext;
	CertStoreType m_certStoreType;	
	string m_strUserName;
	string m_strPassword;		
	string m_strAgentName;
	string m_strOName;
	string m_strLastError;
	string m_strHeaders;
	//string m_strPost;
	int m_lastErrorCode;
	int m_ReqID;
	
	HWND m_hWnd;
};

//#endif // !defined(AFX_WINETSEC_H__91AD1B9B_5B03_457E_A6B6_D66BB03147B7__INCLUDED_)

