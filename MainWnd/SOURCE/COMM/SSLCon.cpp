// SSLCon.cpp: implementation of the CSslConnection class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "SSLCon.h"
#include "..\\UI\\language.h"



//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CSslConnection::CSslConnection(HWND hwnd)
{
	m_hInternet = NULL;
	m_hRequest = NULL;
	m_certStoreType = certStoreMY;
	m_hStore = NULL;
	m_hSession = NULL;
	m_pContext = NULL;
	m_wPort = 443;
	m_strAgentName = "";	
	m_secureFlags = INTERNET_FLAG_RELOAD|INTERNET_FLAG_KEEP_CONNECTION|INTERNET_FLAG_NO_CACHE_WRITE|
					INTERNET_FLAG_SECURE|INTERNET_FLAG_IGNORE_CERT_CN_INVALID;
	
	m_hWnd = hwnd;
}

CSslConnection::~CSslConnection()
{
	ClearHandles();
}


bool CSslConnection::ConnectToHttpsServer(string strVerb)
{
	try {	
		TCHAR wsAgent[20], wsServer[40], wsUser[20], wsPwd[20], wsVerb[5], wsObjectName[40];
		wsprintf(wsAgent, L"%hs", m_strAgentName.c_str());
		
 		m_hInternet = InternetOpen(wsAgent, INTERNET_OPEN_TYPE_PRECONFIG , NULL, NULL, m_ReqID);/*INTERNET_OPEN_TYPE_DIRECT*/
		if (!m_hInternet) 
		{
			m_strLastError = "Cannot open internet";
			m_lastErrorCode = GetLastError();
			return false;
		}
			
		wsprintf(wsServer, L"%hs", m_strServerName.c_str());
		wsprintf(wsUser, L"%hs", m_strUserName.c_str());
		wsprintf(wsPwd, L"%hs", m_strPassword.c_str());

		m_hSession = InternetConnect(m_hInternet, wsServer, m_wPort, wsUser, wsPwd, INTERNET_SERVICE_HTTP, 0, m_ReqID);
		if (!m_hSession) 
		{			
			m_strLastError = "Cannot connect to internet";
			m_lastErrorCode = GetLastError();
			ClearHandles();
			return false;
		}
		wsprintf(wsObjectName, L"%hs", m_strObjectName.c_str());
		wsprintf(wsVerb, L"%hs", strVerb.c_str());
		
 		m_hRequest = HttpOpenRequest(m_hSession, wsVerb, wsObjectName, NULL, L"", NULL, m_secureFlags, m_ReqID);
		if (!m_hRequest) 
		{
			m_strLastError = "Cannot perform http request";
			m_lastErrorCode = GetLastError();
			ClearHandles();		
			return false;
		}
		
		::PostMessage(m_hWnd, WM_SOCK_RECV, 1,CLanguage::IDX_SOCKET_SENDING);
		m_ReqID++;
	}
	catch(...) 
	{
		m_strLastError = "Memory Exception occured";
		m_lastErrorCode = GetLastError();
		return false;
	}
	return true;
}

bool CSslConnection::SendHttpsRequest(__in_bcount_opt(dwOptionalLength) IN LPVOID lpOptional OPTIONAL,IN DWORD dwOptionalLength)
{
	DWORD dwFlags; 
	DWORD dwBuffLen = sizeof(dwFlags);
	InternetQueryOption (m_hRequest, INTERNET_OPTION_SECURITY_FLAGS, (LPVOID)&dwFlags, &dwBuffLen);
	dwFlags |= SECURITY_FLAG_IGNORE_UNKNOWN_CA;   
	dwFlags |= SECURITY_FLAG_IGNORE_CERT_DATE_INVALID;
	dwFlags |= SECURITY_FLAG_IGNORE_CERT_CN_INVALID;
	InternetSetOption (m_hRequest, INTERNET_OPTION_SECURITY_FLAGS, &dwFlags, sizeof (dwFlags) );

	/// <Annoying conversion>
	int len;
	int slength = (int)m_strHeaders.length() + 1;
	len = MultiByteToWideChar(CP_ACP, 0, m_strHeaders.c_str(), slength, 0, 0); 
	wchar_t* buf = new wchar_t[len];
	MultiByteToWideChar(CP_ACP, 0, m_strHeaders.c_str(), slength, buf, len);
	std::wstring r(buf);
	delete[] buf;
	LPCWSTR strHeaders = r.c_str();
	/// </Annoying conversion>
				
	try 
	{	
		for (int tries = 0; tries < 20; ++tries) 
		{		
			
			::PostMessage(m_hWnd, WM_SOCK_RECV, 1,CLanguage::IDX_SOCKET_RECEIVING);
			if (HttpSendRequest(m_hRequest, strHeaders, slength-1, lpOptional, dwOptionalLength))
			{
				::PostMessage(m_hWnd, WM_SOCK_RECV, 1,CLanguage::IDX_SOCKET_PROCESSING);
				return true;			
			}
			else
			{
				int lastErr = GetLastError();
				if (lastErr == ERROR_INTERNET_CLIENT_AUTH_CERT_NEEDED) 
				{
					if (!SetClientCert()) 
					{
						m_strLastError = "Cannot perform http request, client authentication needed but couldnt detect required client certificate";
						m_lastErrorCode = GetLastError();
						return false;
					}					
				}
				else if (lastErr == ERROR_INTERNET_INVALID_CA) 
				{
					m_strLastError = "Cannot perform http request, client authentication needed, invalid client certificate is used";
					m_lastErrorCode = GetLastError();
					DWORD dwFlags; 
					DWORD dwBuffLen = sizeof(dwFlags);
					InternetQueryOption (m_hRequest, INTERNET_OPTION_SECURITY_FLAGS, (LPVOID)&dwFlags, &dwBuffLen);
					dwFlags |= SECURITY_FLAG_IGNORE_UNKNOWN_CA;      
					InternetSetOption (m_hRequest, INTERNET_OPTION_SECURITY_FLAGS, &dwFlags, sizeof (dwFlags) );
				}
				else {
					m_strLastError = "Cannot perform http request";
					m_lastErrorCode = lastErr;
					return false;
				}
			}
		} 
	}
	catch(...) 
	{
		m_strLastError = "Memory Exception occured";
		m_lastErrorCode = GetLastError();
		return false;
	}

	return false;
}

void CSslConnection::ClearHandles()
{
	if (m_hInternet) 
	{
		InternetCloseHandle(m_hInternet);
		m_hInternet = NULL;
	}
		
	if (m_hSession) 
	{
		InternetCloseHandle(m_hSession);
		m_hSession = NULL;
	}
		
	if (m_pContext) 
	{
		CertFreeCertificateContext(m_pContext);
		m_pContext = NULL;
	}
	if (m_hStore) 
	{
		CertCloseStore(m_hStore, CERT_CLOSE_STORE_FORCE_FLAG);
		m_hStore = NULL;
	}
}

#define CERTIFICATE_BUFFER_SIZE 1024
bool CSslConnection::SetClientCert()
{
	char *lpszStoreName;
	switch (m_certStoreType) 
	{
	case certStoreMY:
		lpszStoreName = "MY";
		break;
	case certStoreCA:
		lpszStoreName = "CA";
		break;
	case certStoreROOT:
		lpszStoreName = "ROOT";
		break;
	case certStoreSPC:
		lpszStoreName = "SPC";
		break;
	}


 	m_hStore = CertOpenSystemStore(NULL, lpszStoreName);
	if (!m_hStore) 
	{
		m_strLastError = "Cannot open system store ";
		m_strLastError += lpszStoreName;
		m_lastErrorCode = GetLastError();
		ClearHandles();
		return false;
	}
	
	m_pContext = FindCertWithOUNITName();

	if (!m_pContext) 
	{
		m_strLastError = "Cannot find the required certificate";
		m_lastErrorCode = GetLastError();
		ClearHandles();
		return false;
	}
	
	// INTERNET_OPTION_CLIENT_CERT_CONTEXT is 84
	int res = InternetSetOption(m_hRequest, INTERNET_OPTION_CLIENT_CERT_CONTEXT, (void *) m_pContext, sizeof(CERT_CONTEXT));
	if (!res) 
	{
		m_strLastError = "Cannot set certificate context";
		m_lastErrorCode = GetLastError();
		ClearHandles();
		return false;
	}
	
	return true;
}

void CSslConnection::AddCertToCertStore()
{
//HRESULT  hr                  = S_OK;
//	BYTE     IsoOfficeCert[CERTIFICATE_BUFFER_SIZE] = {0};
//	DWORD    certEncodedSize     = 0L;
//	HANDLE   certFileHandle      = NULL;
//	BOOL     result              = FALSE;
//
//	certFileHandle = CreateFile(TEXT("..\\..\\res\\iso-office.cer"), 
//		GENERIC_READ, 
//		0, 
//		NULL, 
//		OPEN_EXISTING, 
//		FILE_ATTRIBUTE_NORMAL, 
//		NULL);
//	if (INVALID_HANDLE_VALUE == certFileHandle) {
//		hr = HRESULT_FROM_WIN32(GetLastError());
//	}
//
//	if (SUCCEEDED(hr)) {
//		// if the buffer is large enough
//		//  read the certificate file into the buffer
//		if (GetFileSize (certFileHandle, NULL) <= CERTIFICATE_BUFFER_SIZE) {
//			result = ReadFile(certFileHandle, 
//				IsoOfficeCert, 
//				CERTIFICATE_BUFFER_SIZE, 
//				&certEncodedSize, 
//				NULL);
//			if (!result) {
//				// the read failed, return the error as an HRESULT
//				hr = HRESULT_FROM_WIN32(GetLastError());
//			} else {
//				hr = S_OK;
//			}
//		} else {
//			// The certificate file is larger than the allocated buffer.
//			//  To handle this error, you could dynamically allocate
//			//  the certificate buffer based on the file size returned or 
//			//  use a larger static buffer.
//			hr = HRESULT_FROM_WIN32(ERROR_MORE_DATA);
//		}    
//	}
//
//
//	PCCERT_CONTEXT  pCertContext = NULL; 
//	if (SUCCEEDED(hr))
//	{
//		// create a certificate from the contents of the buffer
//		if(pCertContext = CertCreateCertificateContext(X509_ASN_ENCODING, 
//			IsoOfficeCert, 
//			certEncodedSize))
//		{
//			hr = HRESULT_FROM_WIN32(GetLastError());
//			CloseHandle(certFileHandle);
//			hr = E_FAIL;
//		} else {
//			hr = S_OK;
//		}
//	}
//	// close the certificate file
//	if (NULL != certFileHandle) CloseHandle(certFileHandle);
//	
//	
//	if(!CertAddEncodedCertificateToSystemStoreW(TEXT("CA"),  IsoOfficeCert, certEncodedSize ))
//	{
//		m_strLastError = "Cannot Add the required certificate to Cert Store"; // which is not a big deal because it should already be there after the first run...  :)
//		m_lastErrorCode = GetLastError();
//		ClearHandles();
//	}
}



PCCERT_CONTEXT CSslConnection::FindCertWithOUNITName()
{
	//This function performs a certificate contex search
	//by the organizational unit name of the issuer
	//Take this function as a sample for your possible different search functions
	PCCERT_CONTEXT pCertContext = NULL;		
	CERT_RDN certRDN;

	certRDN.cRDNAttr = 1;
	certRDN.rgRDNAttr = new CERT_RDN_ATTR;
	certRDN.rgRDNAttr->pszObjId = szOID_ORGANIZATIONAL_UNIT_NAME;
	certRDN.rgRDNAttr->dwValueType = CERT_RDN_ANY_TYPE;
	certRDN.rgRDNAttr->Value.pbData = (BYTE *) m_strOName.c_str();
	certRDN.rgRDNAttr->Value.cbData = strlen(m_strOName.c_str());

	pCertContext = CertFindCertificateInStore(m_hStore, 
		X509_ASN_ENCODING | PKCS_7_ASN_ENCODING, 
		0, CERT_FIND_ISSUER_ATTR, &certRDN, NULL);
	

	delete certRDN.rgRDNAttr;
	return pCertContext;
}

string CSslConnection::GetRequestResult()
{ 
	DWORD   dwSize = 0;  
	char    sz[1024] = {'\0'};  
	DWORD   dwCode = 0;  
	DWORD   dwCodeSize = 0;  

	dwCodeSize = sizeof(DWORD);  
	if (!HttpQueryInfo(m_hRequest, HTTP_QUERY_STATUS_CODE | HTTP_QUERY_FLAG_NUMBER, &dwCode, &dwCodeSize, NULL))  
	{  
		m_strLastError = "HttpQueryInfo FAILED!";
		m_lastErrorCode = GetLastError();
		return 0;  
	}   

	DWORD dwNumberOfBytesRead;
	string strResult;
	int result; 
	do 
	{
		result = InternetReadFile(m_hRequest, sz, 1023, &dwNumberOfBytesRead);												
		sz[dwNumberOfBytesRead] = '\0';
		int x = strlen(sz);
		strResult += sz;
		memset(sz, 0, 1024);

	} 
	while(result && dwNumberOfBytesRead != 0);	
	return strResult;
}