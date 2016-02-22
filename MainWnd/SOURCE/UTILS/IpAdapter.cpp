#include "StdAfx.h"
#include "IpAdapter.h"

void c2w(LPWSTR pwstr,size_t len,const char *str)  
{ 
	if(str)  
    {  
		size_t nu = strlen(str);  
		size_t n =(size_t)MultiByteToWideChar(CP_ACP,0,(const char *)str,(int)nu,NULL,0);  
		if(n>=len)n=len-1;  
		MultiByteToWideChar(CP_ACP,0,(const char *)str,(int)nu,pwstr,(int)n);  
		pwstr[n]=0;  
    }  
} 
BOOL IpAdapter::IsAdapterAvaiable()
{
	return (ADAPTER_NAME != NULL);
}

BOOL IpAdapter::TryToFindAdapter()
{
	PIP_ADAPTER_INFO pIpAdapterInfo = NULL;
	ULONG adapterLength = 0;
	
	// get interface list buffer size
	GetAdaptersInfo(pIpAdapterInfo,&adapterLength);

	pIpAdapterInfo = (PIP_ADAPTER_INFO)malloc(adapterLength);

	// get interface list
	GetAdaptersInfo(pIpAdapterInfo,&adapterLength);

	while( pIpAdapterInfo != NULL )
	{
		// find "DM90001"
		if ( strcmp(pIpAdapterInfo->AdapterName, "DM90001") == 0 )
		{
			nameLength = sizeof (TEXT("DM90001"))/2;
			keyLength = sizeof (TEXT("\\Comm\\DM90001\\Parms\\TcpIp"))/2;

			ADAPTER_NAME = new WCHAR[ nameLength ];
			ADAPTER_KEY  = new WCHAR[ keyLength ];

			memcpy ( ADAPTER_NAME, TEXT("DM90001"), nameLength*2 );
			memcpy ( ADAPTER_KEY, TEXT("\\Comm\\DM90001\\Parms\\TcpIp"), keyLength*2 );

			break;
		}
		// find "AX887721
		else if ( strcmp(pIpAdapterInfo->AdapterName, "AX887721") == 0)
		{
			nameLength = sizeof (TEXT("AX887721"))/2;
			keyLength = sizeof (TEXT("\\Comm\\AX887721\\Parms\\TcpIp"))/2;

			ADAPTER_NAME = new WCHAR[ nameLength ];
			ADAPTER_KEY  = new WCHAR[ keyLength ];

			memcpy ( ADAPTER_NAME, TEXT("AX887721"), nameLength*2 );
			memcpy ( ADAPTER_KEY, TEXT("\\Comm\\AX887721\\Parms\\TcpIp"), keyLength*2 );

			break;
		}
		// find "RT28701"
		else if ( strcmp(pIpAdapterInfo->AdapterName, "RT28701") == 0)
		{
			nameLength = sizeof (TEXT("RT28701"))/2;
			keyLength = sizeof (TEXT("\\Comm\\RT28701\\Parms\\TcpIp"))/2;

			ADAPTER_NAME = new WCHAR[ nameLength ];
			ADAPTER_KEY  = new WCHAR[ keyLength ];

			memcpy ( ADAPTER_NAME, TEXT("RT28701"), nameLength*2 );
			memcpy ( ADAPTER_KEY, TEXT("\\Comm\\RT28701\\Parms\\TcpIp"), keyLength*2 );

			break;
		}
		// check next interface
		pIpAdapterInfo = pIpAdapterInfo->Next;
	}	
	free(pIpAdapterInfo);

	return (ADAPTER_NAME != NULL);
}

IpAdapter::IpAdapter(void)
{ 
	TryToFindAdapter();
}
IpAdapter::IpAdapter(WCHAR *adapterName, WORD _nameLength, WCHAR *adapterKey, WORD _keyLength)
{
	ADAPTER_NAME = NULL;
	ADAPTER_KEY = NULL;
	nameLength = _nameLength;
	keyLength = _keyLength;

	ADAPTER_NAME = new WCHAR[ nameLength ];
	ADAPTER_KEY = new WCHAR[ keyLength ];

	memcpy ( ADAPTER_NAME, adapterName, nameLength*2 );
	memcpy ( ADAPTER_KEY, adapterKey, keyLength*2 );

	ULONG adapterIndex = 0;

	if ( GetAdapterIndex( ADAPTER_NAME, &adapterIndex) != S_OK )
	{
		delete [] ADAPTER_NAME;
		delete [] ADAPTER_KEY;
		ADAPTER_NAME = NULL;
		ADAPTER_KEY = NULL;
	}
}

IpAdapter::~IpAdapter(void)
{
	delete [] ADAPTER_NAME;
	delete [] ADAPTER_KEY;
	ADAPTER_NAME = NULL;
	ADAPTER_KEY = NULL;

	DisableAdapterNotification();
}

BOOL IpAdapter::RebindAdapter()
{
	HANDLE hNdis=CreateFile(_T("NDS0:"),GENERIC_READ|GENERIC_WRITE,0,NULL,OPEN_EXISTING,0,NULL);
	BOOL ret = DeviceIoControl(hNdis,IOCTL_NDIS_REBIND_ADAPTER,ADAPTER_NAME,nameLength*2,NULL,NULL,NULL,NULL);

	CloseHandle(hNdis);

	if(ret)
		return TRUE;
	else 
		return FALSE;
}

BOOL IpAdapter::GetRegIP ( LPWSTR ip, LPWSTR mask, LPWSTR gate, LPWSTR dns, BOOL *dhcp )
{ 
	HKEY hKey; 
	DWORD dwDataType, dwBufSize; 
	
	if(RegOpenKeyEx(HKEY_LOCAL_MACHINE,ADAPTER_KEY,0,KEY_READ,&hKey) != ERROR_SUCCESS) 
		return false; 
	
	if (dhcp)
	{
		dwBufSize = sizeof(DWORD);
		RegQueryValueEx(hKey, _T("EnableDHCP"), 0, &dwDataType, (BYTE*)dhcp, &dwBufSize);
	}
	if (ip)
	{
		dwBufSize = 256; 
		RegQueryValueEx(hKey, _T("IPAddress"), 0, &dwDataType, (BYTE *)ip, &dwBufSize);
	}
	if (mask)
	{
		dwBufSize = 256; 
		RegQueryValueEx(hKey, _T("Subnetmask"), 0, &dwDataType, (BYTE *)mask, &dwBufSize);
	}	
	if (gate)
	{
		dwBufSize = 256; 
		RegQueryValueEx(hKey, _T("DefaultGateway"), 0, &dwDataType, (BYTE *)gate, &dwBufSize);
	}
	if (dns)
	{
		dwBufSize = 256; 
		RegQueryValueEx(hKey, _T("DNS"), 0, &dwDataType, (BYTE *)dns, &dwBufSize);
	}

	RegCloseKey(hKey); 
	return true; 
} 
BOOL IpAdapter::SetRegIP ( LPCWSTR ip, LPCWSTR mask, LPCWSTR gate, LPCWSTR dns )
{ 
//	BOOL ret;
	HKEY hKey;
	int nIP,nMask,nGate,nDNS;
	
	RegistryLock(FALSE);
	if(RegOpenKeyEx(HKEY_LOCAL_MACHINE,ADAPTER_KEY,0,KEY_WRITE,&hKey) != ERROR_SUCCESS) 
	{
		RegistryLock(TRUE);
		return FALSE;
	}

	DWORD EnableDHCPTMP = 0; 
	RegSetValueEx(hKey, _T("EnableDHCP"), 0, REG_DWORD, (BYTE*)&EnableDHCPTMP, sizeof(DWORD)); //Disable DHCP
	
    if(EnableDHCPTMP == 0)
	{
		nIP=wcslen(ip)*2+2; 
		nMask=wcslen(mask)*2+2; 
		nGate=wcslen(gate)*2+2;
		nDNS=wcslen(dns)*2+2; 

		if(ip) 
			RegSetValueEx(hKey, _T("IPAddress"), 0, REG_MULTI_SZ, (unsigned char*)ip, nIP); 
		if(mask)
			RegSetValueEx(hKey, _T("SubnetMask"), 0, REG_MULTI_SZ, (unsigned char*)mask, nMask); 
		if(gate)
			RegSetValueEx(hKey, _T("DefaultGateway"), 0, REG_MULTI_SZ, (unsigned char*)gate, nGate); 
		if(dns)
			RegSetValueEx(hKey, _T("DNS"), 0, REG_MULTI_SZ, (unsigned char*)dns, nDNS); 
	}

	RegCloseKey(hKey); 
	RegFlushKey(HKEY_LOCAL_MACHINE); 
	RegistryLock(TRUE);

	return TRUE;
} 

BOOL IpAdapter::GetIP ( LPWSTR ip, LPWSTR mask, LPWSTR gate, LPWSTR dns, BOOL *dhcp )
{
	ULONG adapterIndex = 0;
	BOOL ret = FALSE;

	if ( GetAdapterIndex( ADAPTER_NAME, &adapterIndex) != S_OK )
		return FALSE;
 
	PIP_ADAPTER_INFO pIpAdapterInfo = NULL;
	ULONG adapterInfoLength = 0;
	
	// get interface list buffer size
	GetAdaptersInfo(pIpAdapterInfo,&adapterInfoLength);

	pIpAdapterInfo = (PIP_ADAPTER_INFO)malloc(adapterInfoLength);

	// get interface list
	GetAdaptersInfo(pIpAdapterInfo,&adapterInfoLength);

	while( pIpAdapterInfo != NULL )
	{
		if ( pIpAdapterInfo->Index == adapterIndex )
		{

			if(dhcp)
			{
				if (pIpAdapterInfo->DhcpEnabled)
					*dhcp = TRUE;
				else
					*dhcp = FALSE;
			}
			if (ip)
				c2w(ip,16,pIpAdapterInfo->CurrentIpAddress->IpAddress.String);

			if (mask)
				c2w(mask,16,pIpAdapterInfo->CurrentIpAddress->IpMask.String);

			if (gate)
				c2w(gate,16,pIpAdapterInfo->GatewayList.IpAddress.String);

			if (dns)
			{
				PIP_PER_ADAPTER_INFO  pIpPreAdapterInfo = NULL;
				ULONG preAdapterInfoLength = 0;

				GetPerAdapterInfo(adapterIndex, pIpPreAdapterInfo, &preAdapterInfoLength);

				pIpPreAdapterInfo = (PIP_PER_ADAPTER_INFO)malloc(preAdapterInfoLength);
				// get interface list
				GetPerAdapterInfo(adapterIndex, pIpPreAdapterInfo,&preAdapterInfoLength);

				if (pIpPreAdapterInfo)
				{
					c2w(dns,16,pIpPreAdapterInfo->DnsServerList.IpAddress.String);
					free(pIpPreAdapterInfo);
				}
			}

			ret = TRUE;
			break;
		}
		// check next interface
		pIpAdapterInfo = pIpAdapterInfo->Next;
	}	
	free(pIpAdapterInfo);

	return ret;
}
BOOL IpAdapter::SetDHCP (BOOL enable)
{
	HKEY hKey;
	
	RegistryLock(FALSE);				// * SAIO API

	if(RegOpenKeyEx(HKEY_LOCAL_MACHINE,ADAPTER_KEY,0,KEY_WRITE,&hKey) != ERROR_SUCCESS) 
	{
		RegistryLock(TRUE);
	}
	//Disable DHCP
	DWORD EnableDHCPTMP = enable; 
	RegSetValueEx(hKey, _T("EnableDHCP"), 0, REG_DWORD, (BYTE*)&EnableDHCPTMP, sizeof(DWORD)); 
	
	RegCloseKey(hKey); 
	RegFlushKey(HKEY_LOCAL_MACHINE); 

	RegistryLock(TRUE);					// * SAIO API	

	if (enable)
		this->RenewDHCP();

	return TRUE;
}
BOOL IpAdapter::RenewDHCP ()
{
	ULONG adapterIndex = 0;
	BOOL ret = FALSE;

	if ( GetAdapterIndex( ADAPTER_NAME, &adapterIndex) != S_OK )
		return FALSE;

	// Renew IP Address
	PIP_INTERFACE_INFO pIpInterfaceInfo = NULL;
	ULONG bufSize = 0;
	
	// Get Adapter List Buffer Size
	GetInterfaceInfo(pIpInterfaceInfo,&bufSize);

	pIpInterfaceInfo = (PIP_INTERFACE_INFO)malloc(bufSize);

	// Get Adapter List Buffer
	GetInterfaceInfo(pIpInterfaceInfo,&bufSize);

	for(LONG i = 0 ; i < pIpInterfaceInfo->NumAdapters ; i++)
	{
		// find adpter
		if ( pIpInterfaceInfo->Adapter[i].Index == adapterIndex )
		{
			if ( IpReleaseAddress( &(pIpInterfaceInfo->Adapter[i]) ) == NO_ERROR )
			{
				Sleep(500);
				ret = TRUE;
			}
		}
	}	
	free(pIpInterfaceInfo);

	return ret;
}


// 0: Disconnected
// 1: Connected
// 2: No Device
// 3: Get LastError
// 4: Unbind
// 5: Bind
DWORD WINAPI AdpaterNotificateThread(LPVOID lpParam)
{   
	IpAdapter * pAdapter = (IpAdapter*)lpParam;

    MSGQUEUEOPTIONS                 sOptions;   
    NDISUIO_REQUEST_NOTIFICATION    sRequestNotification;   
       
    NDISUIO_DEVICE_NOTIFICATION     sDeviceNotification;   
    DWORD                           dwBytesReturned;   
    DWORD                           dwFlags;   
   
    sOptions.dwSize                 = sizeof(MSGQUEUEOPTIONS);   
    sOptions.dwFlags                = 0;   
    sOptions.dwMaxMessages          = 4;   
    sOptions.cbMaxMessage           = sizeof(NDISUIO_DEVICE_NOTIFICATION);   
    sOptions.bReadAccess            = TRUE;   
    HANDLE g_hMsgQueue = NULL;   
   
    g_hMsgQueue = CreateMsgQueue(NULL,&sOptions);   
   
    if (g_hMsgQueue == NULL)                   
    {   
        RETAILMSG (1,(TEXT("NdisPower:: Error CreateMsgQueue()..\r\n")));   

		pAdapter->pAdapterNotificate(3, pAdapter->user_data);

		return 0;   
    }   
       
    HANDLE g_hNdisuio = CreateFile(   
        (PTCHAR)NDISUIO_DEVICE_NAME,                    //  Object name.   
        0x00,                                           //  Desired access.   
        0x00,                                           //  Share Mode.   
        NULL,                                           //  Security Attr   
        OPEN_EXISTING,                                  //  Creation Disposition.   
        FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED,   //  Flag and Attributes..   
        (HANDLE)INVALID_HANDLE_VALUE);     
       
    if (g_hNdisuio == INVALID_HANDLE_VALUE)   
    {   
		pAdapter->pAdapterNotificate(2, pAdapter->user_data);
        return 0;   
    }   
       
    sRequestNotification.hMsgQueue              =  g_hMsgQueue;   
    sRequestNotification.dwNotificationTypes    =  NDISUIO_NOTIFICATION_BIND | NDISUIO_NOTIFICATION_UNBIND | NDISUIO_NOTIFICATION_MEDIA_CONNECT | NDISUIO_NOTIFICATION_MEDIA_DISCONNECT;   
   
    if (!DeviceIoControl(   
        g_hNdisuio,   
        IOCTL_NDISUIO_REQUEST_NOTIFICATION,   
        &sRequestNotification,   
        sizeof(NDISUIO_REQUEST_NOTIFICATION),   
        NULL,   
        0x00,   
        NULL,   
        NULL))   
    {   
		pAdapter->pAdapterNotificate(3, pAdapter->user_data);
        return 0;   
    }   
   
	HANDLE handles[2];

	handles[0] = g_hMsgQueue;
	handles[1] = pAdapter->fContinue;

    while (WaitForMultipleObjects(2, handles, FALSE, INFINITE) == WAIT_OBJECT_0)   
    {   
        while (ReadMsgQueue(   
            g_hMsgQueue,   
            &sDeviceNotification,   
            sizeof(NDISUIO_DEVICE_NOTIFICATION),   
            &dwBytesReturned,   
            1,   
            &dwFlags))   
        {   
            if (sDeviceNotification.dwNotificationType & NDISUIO_NOTIFICATION_MEDIA_DISCONNECT)   
            {   
				pAdapter->pAdapterNotificate(0, pAdapter->user_data);
                Sleep(100);        
            }   
            else if (sDeviceNotification.dwNotificationType & NDISUIO_NOTIFICATION_MEDIA_CONNECT)   
            {   
				pAdapter->pAdapterNotificate(1, pAdapter->user_data);
                Sleep(100);        
            }   
            else if (sDeviceNotification.dwNotificationType & NDISUIO_NOTIFICATION_UNBIND)   
            {   
				pAdapter->pAdapterNotificate(4, pAdapter->user_data);
                Sleep(100);        
            }   
            else if (sDeviceNotification.dwNotificationType & NDISUIO_NOTIFICATION_BIND)   
            {   
				pAdapter->pAdapterNotificate(5, pAdapter->user_data);
                Sleep(100);        
            }   
        }   
    }  
	CloseHandle(g_hNdisuio);

    return 0;   

}
void IpAdapter::DisableAdapterNotification()
{
	if (pAdapterNotificate != NULL)
	{
		SetEvent(fContinue);
		pAdapterNotificate = NULL;
		CloseHandle(fContinue);
		fContinue = NULL;
	}
}
BOOL IpAdapter::EnableAdapterNotification(ADAPTER_NOTIFICATION_CALLBACK pCallback, LPVOID pUserData)
{
	if (ADAPTER_NAME == NULL)
	{
		if (TryToFindAdapter() == NULL)
			return FALSE;
	}

	if (pCallback != NULL)
	{
		fContinue = CreateEvent(NULL, TRUE, FALSE, NULL);
		pAdapterNotificate = pCallback;
		user_data = pUserData;
	}
	else
		return FALSE;

	DWORD adapterThreadID = 0;
    HANDLE notificationThread = CreateThread(NULL,NULL,AdpaterNotificateThread,this,NULL, &adapterThreadID);
	CloseHandle(notificationThread);

	return TRUE;
}

// 0: Disconnected
// 1: Connected
// 2: No Device
// 3: Get LastError
DWORD IpAdapter::IsEthernetConnected()
{
	if (ADAPTER_NAME == NULL)
	{
		if (TryToFindAdapter() == NULL)
			return 2;
	}
	HANDLE m_hNDUIO = CreateFile(NDISUIO_DEVICE_NAME, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED, INVALID_HANDLE_VALUE);
    
	if (m_hNDUIO == INVALID_HANDLE_VALUE)   
        return 3;   

	// TODO: Add your control notification handler code here
	NIC_STATISTICS nicStatistics = {0};
	DWORD          dwBytesReturned = 0;
	int dwConnected = 0;
	nicStatistics.ptcDeviceName = ADAPTER_NAME;

	if(!DeviceIoControl(m_hNDUIO, IOCTL_NDISUIO_NIC_STATISTICS, NULL, 0, &nicStatistics, sizeof(NIC_STATISTICS), &dwBytesReturned, NULL))
	{
		dwConnected = 2;
	}
	else
	{
		if (nicStatistics.MediaState == MEDIA_STATE_CONNECTED)
			dwConnected = 1;
		else if (nicStatistics.MediaState == MEDIA_STATE_DISCONNECTED)
			dwConnected = 0;
		else 
			dwConnected = 3;
	}

	CloseHandle(m_hNDUIO);
	return dwConnected;
}
