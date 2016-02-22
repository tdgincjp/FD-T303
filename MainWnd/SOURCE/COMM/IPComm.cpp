	
#include "stdafx.h"
//#include <stdio.h>
#include "Winsock2.h"
#include "Winbase.h"
#include <security.h>
#include "sslsock.h"
//#include "General.h"
#include "SaioSSL.h"

#include "..\\UTILS\\string.h"
/*#include "..\\UI\\display.h"
*/#include "..\\UI\\language.h"
#include "..\\data\\DataFile.h"
#include "..\\data\\AppData.h"

#include "IPComm.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#pragma warning(disable: 4996)

#define BUF_SIZE				256	

#pragma comment(lib,"Lib\\SaioSSL.lib")
#pragma comment(lib,"SaioBase.lib")
#pragma comment(lib,"ws2.lib")

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CIPComm::CIPComm()
{
	m_Socket = INVALID_SOCKET;
	m_bStatus = FALSE;
	m_ssl = NULL;
	m_Pkey = L"";
	m_Pass = L"";
	m_CaPath = L"";
	m_Verify = 0;
	RecvEvent = NULL;
	m_RecvLen = 0;
	m_hWnd = NULL;

	m_bResendFlag = FALSE;

}

CIPComm::~CIPComm()
{
}

//----------------------------------------------------------------------------
//!	\brief	recv thread entry
//----------------------------------------------------------------------------
DWORD WINAPI CIPComm::ThreadProc (LPVOID lpArg)
{
	CIPComm* pIPComm = reinterpret_cast<CIPComm*>(lpArg);
	pIPComm->Recv_Proc();
	return 0;
}
	
//----------------------------------------------------------------------------
//!	\brief	IP recv thread
//----------------------------------------------------------------------------
void CIPComm::Recv_Proc()
{
	int buf_len;
	// 
	while(TRUE)
	{
		WaitForSingleObject(RecvEvent,INFINITE);
		if(m_bStatus==0)
		{
			break;
		}
		WSAEventSelect(m_Socket,RecvEvent,0);
		while(TRUE)
		{
			if( RecvEvent != NULL)
				ResetEvent(RecvEvent);
			buf_len = ssl_recv(m_ssl, (char*)m_RecvBuf, RECV_SIZE);
			if (buf_len == SOCKET_ERROR || buf_len == 0) 
			{
				Sleep(10);
				break;
			}
			m_RecvLen = buf_len;
			SetEvent(m_hEvent);

			if (WSAEWOULDBLOCK == WSAGetLastError())                        
			{                                
				continue;                        
			}
		}
		WSAEventSelect(m_Socket, RecvEvent, FD_READ);
	}
}

//----------------------------------------------------------------------------
//!	\brief	close IP comm
//----------------------------------------------------------------------------
void CIPComm::Cancel()
{
	Close();
}

//----------------------------------------------------------------------------
//!	\brief	IP comm connect
//!	\param	IP - IP server
//!	\param	Port - IP port
//!	\param	hwnd - message window handle
//!	\param	flag - reconnect flag
//! RETURNS:       TRUE    = process ok.                               
//!                FALSE    = process failed.
//----------------------------------------------------------------------------
BOOL CIPComm::Connect(const char* IP, const int Port, HWND hwnd, BOOL flag)
{
	m_hWnd = hwnd;
 	WSADATA WsaData;
	
	if ( strlen(IP) == 0 )
		return FALSE;

	int iret;
	
    if(WSAStartup(MAKEWORD(2, 2), &WsaData) == SOCKET_ERROR)
	{
	    ::SendMessage(m_hWnd, WM_SOCK_RECV, 0,GetLastError());
		return FALSE;
	}

	::PostMessage(m_hWnd, WM_SOCK_RECV, 1,CLanguage::IDX_SOCKET_CONNECT);

	if ( flag )
	{
		iret = ReConnectToServer();
		if (iret)
		{
			::SendMessage(m_hWnd, WM_SOCK_RECV,0,iret);
			return FALSE;
		}
	}
	else
	{
		if(iret = ConnectToServer(IP, Port))
		{
			iret = ReConnectToServer();
			if (iret)
			{
				::SendMessage(m_hWnd, WM_SOCK_RECV,0,iret);
				return FALSE;
			}
		}
	}

	m_hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

	HANDLE hThread = CreateThread(NULL, 0,ThreadProc, (PVOID)this, 0, NULL);
	CloseHandle(hThread);

	return TRUE;
}
	
//----------------------------------------------------------------------------
//!	\brief	IP comm close
//----------------------------------------------------------------------------
void CIPComm::Close()
{
	if(!m_bStatus)
		return;
	m_bStatus = FALSE;

	SetEvent(m_hEvent);
	Sleep(50);
	m_RecvLen = 0;
	WSASetEvent(RecvEvent);
	Sleep(50);
	WSACloseEvent(RecvEvent);
	RecvEvent = NULL;
	//shut down the SSL connection
	ssl_close(m_ssl);
	if (m_Socket != -1) 
	{
		//
		shutdown(m_Socket, 2);
		closesocket(m_Socket);
		m_Socket = INVALID_SOCKET;
	}

	// SSL
	if( m_ssl != NULL )
		ssl_free(m_ssl); // release m_ssl , handle it ??
	m_ssl = NULL;

	WSACleanup();
}
	
//----------------------------------------------------------------------------
//!	\brief	IP comm send out data
//!	\param	wbuf - send out data buffer point
//!	\param	len - data length
//! RETURNS:       send data length
//----------------------------------------------------------------------------
int CIPComm::Send(char *wbuf,WORD len)
{
    ::PostMessage(m_hWnd, WM_SOCK_RECV, 1,CLanguage::IDX_SOCKET_SENDING);

	int nSent = 0, nToSend = len+2, iRet, iErr;
	BYTE* pToSendHead = new BYTE[len+2];
	BYTE* pOld = pToSendHead;
	pToSendHead[0] = HIBYTE(len);
	pToSendHead[1] = LOBYTE(len);

	memcpy(	pToSendHead +2,(void*)wbuf,len);

	while(1)
	{
		iRet = ssl_send(m_ssl, (char*)pToSendHead, nToSend);
		if(SOCKET_ERROR == iRet)
		{
			iErr = WSAGetLastError();
			if(WSAEWOULDBLOCK == iErr)
			{
				continue;			
			}
			WSASetLastError(iErr);
			delete [] pOld;
			return SOCKET_ERROR;
		}
		pToSendHead += iRet;
		nToSend -= iRet;
		nSent += iRet;

		if(nToSend == 0)
		{
			delete [] pOld;
			Sleep(100);
			return nSent;
		}
	}
	::SendMessage(m_hWnd, WM_SOCK_RECV, 0, CLanguage::IDX_SOCKET_SENDING_ERROR);
	delete [] pOld;
	return 0;
}

//----------------------------------------------------------------------------
//!	\brief	IP comm resend out data
//!	\param	wbuf - send out data buffer point
//!	\param	len - data length
//! RETURNS:       send data length
//----------------------------------------------------------------------------
int CIPComm::Resend(char *wbuf,WORD len)
{
	if ( !m_bResendFlag )
	{
		Close();
		Sleep(100);

		TCHAR buf [100] = {0};

		char host[100] = {"everlinkpos2.everlink.ca"};//67.23.53.155"};
		CString str = L"HOST ADDRESS2";
		if(CDataFile::Read(str,buf))
			strcpy(host,alloc_tchar_to_char(buf));

		int Port = 443;
		str = L"IP PORT2";
		if(CDataFile::Read(str,buf))
			Port = _ttoi(buf);

		if ( !Connect( host,Port,m_hWnd,TRUE))
			return 0;

		m_bResendFlag = TRUE;
		Sleep(100);
	}

	return Send(wbuf,len);
}

//----------------------------------------------------------------------------
//!	\brief	IP comm read recv data
//!	\param	buf - read data buffer point
//!	\param	len - data length
//! RETURNS:       TRUE    = process ok.                               
//!                FALSE    = process failed.
//----------------------------------------------------------------------------
BOOL CIPComm::Read(BYTE* buf,int* len)
{
   ::PostMessage(m_hWnd, WM_SOCK_RECV, 1,CLanguage::IDX_SOCKET_RECEIVING);
	if (WAIT_TIMEOUT == WaitForSingleObject(m_hEvent,45000))
		return FALSE;

	if (m_RecvLen <= 2 || !m_bStatus)
		return FALSE;

	WORD len1 = m_RecvBuf[0]*256+m_RecvBuf[1];
	if(len1 != m_RecvLen-2 || len1 > 1024)
		return FALSE;

    ::PostMessage(m_hWnd, WM_SOCK_RECV, 1,CLanguage::IDX_SOCKET_PROCESSING);

	*len = len1;
	memcpy(buf,&m_RecvBuf[2],len1);
	*(buf+len1)=0;
	m_RecvLen = 0;
	return TRUE;
}

//----------------------------------------------------------------------------
//!	\brief	IP comm connect to server
//!	\param	ServerIP - server IP
//!	\param	iPortNumber - port number
//! RETURNS:       SEC_E_OK    = process ok.                               
//!                other    = process failed.
//----------------------------------------------------------------------------
int CIPComm::ConnectToServer(const char *ServerIP, const int  iPortNumber)       
{
    struct sockaddr_in sin;
	
	if(INVALID_SOCKET!=m_Socket)
	{
		return SEC_E_OK;
	}

	char temp[100];
	CString strIP = CAppData::GetIPAddr();
	BOOL flag = FALSE;
	if (strIP != L"")
	{
		char *p = alloc_tchar_to_char(strIP.GetBuffer(strIP.GetLength()));
		strcpy(temp,p);
		free(p);
		strIP.ReleaseBuffer();
	}
	else if (strspn(ServerIP, "0123456789.") != strlen(ServerIP))
	{
		struct hostent *h;
		h = gethostbyname(ServerIP);
		if( h == NULL)
		{
			temp[0] = 0; 
			CAppData::SetIPAddr(temp,2);
			return -1;
		}

		BYTE* p = (BYTE*)(*h->h_addr_list);
		sprintf(temp,"%d.%d.%d.%d",*p,*(p+1),*(p+2),*(p+3));
		CAppData::SetIPAddr(temp);
		flag = TRUE;
	}
	else
		strcpy(temp,ServerIP);

	//set the socket as TCP
	m_Socket = socket(PF_INET, SOCK_STREAM, 0);
    if(m_Socket == INVALID_SOCKET)
        return WSAGetLastError();

	//debug
    sin.sin_family = AF_INET;
    sin.sin_port = htons((u_short)iPortNumber);
	sin.sin_addr.s_addr = inet_addr(temp);

    if(connect(m_Socket, (struct sockaddr *)&sin, sizeof(sin)) == SOCKET_ERROR)
    {
		if (!flag && strspn(ServerIP, "0123456789.") != strlen(ServerIP))
		{
			struct hostent *h;
			h = gethostbyname(ServerIP);

			if( h == NULL)
			{
				temp[0] = 0; 
				CAppData::SetIPAddr(temp);
				return -1;
			}

			BYTE* p = (BYTE*)(*h->h_addr_list);
			sprintf(temp,"%d.%d.%d.%d",*p,*(p+1),*(p+2),*(p+3));

			CAppData::SetIPAddr(temp);
			sin.sin_addr.s_addr = inet_addr(temp);
			if(connect(m_Socket, (struct sockaddr *)&sin, sizeof(sin)) == SOCKET_ERROR)
			{
				closesocket(m_Socket);
				WSACleanup();
				return WSAGetLastError();
			}
		}
		closesocket(m_Socket);
        WSACleanup();
        return WSAGetLastError();
    }

	RecvEvent = ::WSACreateEvent();   
	::WSAEventSelect(m_Socket, RecvEvent, FD_READ);

	CloseHandle(m_hEvent);
	if(Init_SSL() == -1)
	{
		closesocket(m_Socket);
        WSACleanup();
		return -1;
	}
	m_bStatus = TRUE;
    return SEC_E_OK;
}

//----------------------------------------------------------------------------
//!	\brief	IP comm re connect to server
//! RETURNS:       TRUE    = process ok.                               
//!                FALSE    = process failed.
//----------------------------------------------------------------------------
int CIPComm::ReConnectToServer()       
{
    struct sockaddr_in sin;
	
	if(INVALID_SOCKET!=m_Socket)
	{
		Close();
		Sleep(1000);
		WSADATA WsaData;
		if(WSAStartup(MAKEWORD(2, 2), &WsaData) == SOCKET_ERROR)
			return FALSE;
	}

	TCHAR buf [100] = {0};

	char host[100] = {"everlinkpos2.everlink.ca"};//67.23.53.155"};
	CString str = L"HOST ADDRESS2";
	if(CDataFile::Read(str,buf))
		strcpy(host,alloc_tchar_to_char(buf));

	if ( strlen(host) == 0 )
		return FALSE;

	int Port = 443;
	str = L"IP PORT2";
	if(CDataFile::Read(str,buf))
		Port = _ttoi(buf);
	char *ServerIP = host;
	int  iPortNumber = Port;

	char temp[100];
	CString strIP = CAppData::GetIPAddr(2);
	BOOL flag = FALSE;
	if (strIP != L"")
	{
		char *p = alloc_tchar_to_char(strIP.GetBuffer(strIP.GetLength()));
		strcpy(temp,p);
		free(p);
		strIP.ReleaseBuffer();
	}
	else if (strspn(ServerIP, "0123456789.") != strlen(ServerIP))
	{
		struct hostent *h;
		h = gethostbyname(ServerIP);
		if( h == NULL)
		{
			temp[0] = 0; 
			CAppData::SetIPAddr(temp,2);
			return -1;
		}

		BYTE* p = (BYTE*)(*h->h_addr_list);
		sprintf(temp,"%d.%d.%d.%d",*p,*(p+1),*(p+2),*(p+3));
		CAppData::SetIPAddr(temp,2);
		flag = TRUE;
	}
	else
		strcpy(temp,ServerIP);

	//set the socket as TCP
	m_Socket = socket(PF_INET, SOCK_STREAM, 0);
    if(m_Socket == INVALID_SOCKET)
        return WSAGetLastError();

	//debug
    sin.sin_family = AF_INET;
    sin.sin_port = htons((u_short)iPortNumber);
	sin.sin_addr.s_addr = inet_addr(temp);

    if(connect(m_Socket, (struct sockaddr *)&sin, sizeof(sin)) == SOCKET_ERROR)
    {
		if (!flag && strspn(ServerIP, "0123456789.") != strlen(ServerIP))
		{
			struct hostent *h;
			h = gethostbyname(ServerIP);

			if( h == NULL)
			{
				temp[0] = 0; 
				CAppData::SetIPAddr(temp,2);
				return -1;
			}

			BYTE* p = (BYTE*)(*h->h_addr_list);
			sprintf(temp,"%d.%d.%d.%d",*p,*(p+1),*(p+2),*(p+3));

			CAppData::SetIPAddr(temp,2);
			sin.sin_addr.s_addr = inet_addr(temp);
			if(connect(m_Socket, (struct sockaddr *)&sin, sizeof(sin)) == SOCKET_ERROR)
			{
				closesocket(m_Socket);
				WSACleanup();
				return WSAGetLastError();
			}
		}
		closesocket(m_Socket);
        WSACleanup();
        return WSAGetLastError();
    }

	RecvEvent = ::WSACreateEvent();   
	::WSAEventSelect(m_Socket, RecvEvent, FD_READ);

	CloseHandle(m_hEvent);
	if(Init_SSL() == -1)
	{
		closesocket(m_Socket);
        WSACleanup();
		return -1;
	}
	m_bStatus = TRUE;

	m_bResendFlag = TRUE;

    return SEC_E_OK;
}
//-------------------------------------------------------------------------------------------------------------------
// Function Name  : init_ssl
// Description    : This function will init the SSL with winsocket,and handshake with server.
//
// Input          : 
//			soc   :the initialized winsocket.
//
// Return         : 0 if Succeed, -1 if Fail
//-------------------------------------------------------------------------------------------------------------------
int CIPComm::Init_SSL()
{
	TCHAR buf[BUF_SIZE];
	int ssl_type = 1;
	if (m_ssl != NULL) 
	{
		return 0;
	}

	TCHAR buf1[100]={L""};
	CString str = L"entrustL1Cchain.P12";
	if(CDataFile::Read(L"Cert File",buf1))
		str = CString(buf1);

	m_Cert = L"\\SAIO\\user programs\\" + str;

	str = L"Everlinktest1_Everlink_ca.crt";
	if(CDataFile::Read(L"CA File",buf1))
		str = CString(buf1);

	m_CaFile = L"\\SAIO\\user programs\\" + str;

	// SSL
#ifdef UNICODE
	{
		//The pem root certificate file
		char *ca = alloc_tchar_to_char(m_CaFile.GetBuffer(m_CaFile.GetLength()));
		//The pem root path
		char *capath = alloc_tchar_to_char(m_CaPath.GetBuffer(m_CaPath.GetLength()));
		//The client certificate file
		char *cert = alloc_tchar_to_char(m_Cert.GetBuffer(m_Cert.GetLength()));
		// The pem private key file
		char *pkey = alloc_tchar_to_char(m_Pkey.GetBuffer(m_Pkey.GetLength()));
		//The pointer to the password
		char *pass = alloc_tchar_to_char(m_Pass.GetBuffer(m_Pass.GetLength()));
		char err[BUF_SIZE];
		*err = '\0';
		//Init the ssl
		m_ssl = ssl_init(m_Socket, ssl_type, m_Verify, 1, ca, capath, cert, pkey, pass, err);
		char_to_tchar(err, buf, BUF_SIZE - 1);

		//free the buffer
		free(ca);
		free(capath);
		free(cert);
		free(pkey);
		free(pass);
		m_CaFile.ReleaseBuffer();
		m_CaPath.ReleaseBuffer();
		m_Cert.ReleaseBuffer();
		m_Pkey.ReleaseBuffer();
		m_Pass.ReleaseBuffer();
	}
#else
	*buf = TEXT('\0');
	ssl = (HANDLE)ssl_init(soc, ssl_type, ssl_info.Verify, ssl_info.Depth, op.CAFile, NULL, ssl_info.Cert, ssl_info.Pkey, ssl_info.Pass, buf);
#endif
	if (m_ssl == NULL ) 
	{
		return -1;
	}
	return 0;
}


	
	
