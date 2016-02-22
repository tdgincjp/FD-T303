#pragma once

// Win32 Header
#include "Iphlpapi.h"		// Link lib Iphlpapi.lib
#include "Winsock2.h"		// Link lib ws2.lib
#include "ntddndis.h"
#include "winioctl.h"
#include "nuiouser.h"
#include "string.h"

// SAIO Header
#include "SaioBase.h"		// Link lib SaioBase.lib 

#pragma comment(lib, "Iphlpapi.lib")
#pragma comment(lib, "corelibc.lib")
#pragma comment(lib, "SaioBase.lib")
#pragma comment(lib, "ws2.lib")

typedef void (*ADAPTER_NOTIFICATION_CALLBACK)(DWORD adapterEvent, LPVOID user_data);

class IpAdapter
{
public:

	// Try to find Known Adapter Name
	IpAdapter(void);
	// Manual Set Adapter Name
	IpAdapter(WCHAR *adapterName, WORD _nameLength, WCHAR *adapterKey, WORD _keyLength);
	~IpAdapter(void);

	// Force to Rebind Adapter
	BOOL RebindAdapter ();

	// Get/Set registry IP config
	BOOL GetRegIP ( LPWSTR ip, LPWSTR mask, LPWSTR gate, LPWSTR dns, BOOL *dhcp );
	BOOL SetRegIP ( LPCWSTR ip, LPCWSTR mask, LPCWSTR gate, LPCWSTR dns );

	// Get Current IP Config
	BOOL GetIP ( LPWSTR ip, LPWSTR mask, LPWSTR gate, LPWSTR dns, BOOL *dhcp );

	// Create Notification for Adapter Event
	BOOL EnableAdapterNotification(ADAPTER_NOTIFICATION_CALLBACK pCallback, LPVOID pUserData);
	void DisableAdapterNotification();

	// Query Ethernet Cable Connection Status
	DWORD IsEthernetConnected();

	// DHCP
	BOOL SetDHCP (BOOL enable);
	BOOL RenewDHCP ();

	BOOL IsAdapterAvaiable();
	BOOL TryToFindAdapter();	

	// user data for notifiction
	LPVOID user_data;
	// notification callback
	ADAPTER_NOTIFICATION_CALLBACK pAdapterNotificate;
	// Event Handle to terminate notification
	HANDLE fContinue; 

private:

	WCHAR *ADAPTER_NAME;
	WORD nameLength;
	WCHAR *ADAPTER_KEY;
	WORD keyLength;
};
