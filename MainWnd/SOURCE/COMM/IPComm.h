// IPComm.h : header file
//
#pragma once
#define RECV_SIZE				2048

// CIPComm
class CIPComm
{
// Construction
public:
	CIPComm();	// standard constructor
	~CIPComm();

private:
	SOCKET  m_Socket;
	long	m_ssl;

	CString m_Cert;
	CString m_Pkey;
	CString m_Pass;
	CString m_CaFile;
	CString m_CaPath;
	int     m_Verify;

	BOOL    m_bStatus ;
	WSAEVENT RecvEvent;

	int ConnectToServer(const char *ServerIP, const int  iPortNumber);       
	int ReConnectToServer();       
	int Init_SSL();

public:
	static DWORD WINAPI ThreadProc (LPVOID lpArg);
	void Recv_Proc();

	BOOL Connect(const char* IP, const int Port, HWND hWnd=NULL,BOOL flag = FALSE);
	void Close();
	
	int Send(char *wbuf,WORD len);
	BOOL Read(BYTE* buf,int* len);
	void Cancel();

	int Resend(char *wbuf,WORD len);

private:
	HWND m_hWnd;

	BOOL m_bResendFlag;
public:
	HANDLE m_hEvent;
	int  m_RecvLen;
	BYTE m_RecvBuf[RECV_SIZE];
};
