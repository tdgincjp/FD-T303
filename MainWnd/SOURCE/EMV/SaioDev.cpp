// SaioDev.cpp : Defines the entry point for the DLL application.
//
#include "StdAfx.h"
#include <stdio.h>
#include <stdlib.h>

#include "SaioReader.h"

#pragma warning(disable: 4244)

//#pragma comment(lib, "SaioReader.lib")
#pragma comment(lib, "Lib\\SaioReader.lib")
//1----MSR

HANDLE	Msr_Event_Data_Ready	= CreateEvent( NULL, FALSE, FALSE, NULL );
HANDLE	Msr_Event_Read_Error	= CreateEvent( NULL, FALSE, FALSE, NULL );
HANDLE	Msr_Event_Others		= CreateEvent( NULL, FALSE, FALSE, NULL );

void MSR_event(void *user,unsigned long event)
{
	switch(event)
	{
	case MSR_EVENT_DATA_READY:
		SetEvent( Msr_Event_Data_Ready );
		break;
	case MSR_EVENT_READ_ERROR:
		SetEvent( Msr_Event_Read_Error );
		break;
	default:
		SetEvent( Msr_Event_Others );
		break;
	}
}
/*=======================================================================
// Function: Open MSR
// Purpose:	Open and Init MSR
// call timing:  Device Init
// Return Value:				
//			0x00: Success
//			0x01: Error
// Parameter:
//			null	
/======================================================================*/
DWORD OpenMsr()
{
	if(MsrOpen())
		return 1;

	if(MsrRegister(MSR_event,NULL))
	{
		return 1;
	}
//	MsrReadEnable(TRUE,TRUE,TRUE,MSR_ISO_DECODE);
	MsrReadEnable(FALSE,TRUE,FALSE,MSR_ISO_DECODE);
	return 0;
}

/*=======================================================================
// Function: Close MSR
// Purpose:	Close MSR
// call timing:  Device close
// Return Value:				
//			0x00: Success
//			0x01: Error
// Parameter:
//			null	
/======================================================================*/
DWORD CloseMsr()
{
	if(MsrClose())
		return 1;
	return 0;
}
/*=======================================================================
// Function: Get MSR Data
// Purpose:	Get MSR Data
// call timing:  Read MSR
// Return Value:				
//			0: Data ready
//			1: Read Error
//			2: Other Error
//			3: Timeout
// Parameter:
//			null	
/======================================================================*/
DWORD GetMsrData(char *track1, BYTE *t1Len,char *track2, BYTE *t2Len,char *track3, BYTE *t3Len, DWORD timeout)
{
	DWORD dwRtn, dwError;
	HANDLE		HandlesToWaitFor[3];
	
	HandlesToWaitFor[0] = Msr_Event_Data_Ready;
    HandlesToWaitFor[1] = Msr_Event_Read_Error;
	HandlesToWaitFor[2] = Msr_Event_Others;	
	
	dwRtn = WaitForMultipleObjects(3, HandlesToWaitFor, FALSE, timeout);
	
	switch(dwRtn)
	{	
	case WAIT_OBJECT_0:			
		dwError = 0;//Msr_Event_Data_Ready
		break;
	case WAIT_OBJECT_0+1:
		dwError = 1;//Msr_Event_Read_Error
		break;
	case WAIT_OBJECT_0+2:
		dwError = 2;//Msr_Event_Others
		break;
	case WAIT_TIMEOUT:
		dwError = 3;
		break;
	}
	if (dwError)
		return dwError;

	DWORD  tkLen, decode;
	
	if (track1)
	{
		tkLen = MSRGetDataLength(0);
		if (tkLen <= *t1Len)
		{
			MsrGetData(0, (BYTE *)track1, &decode);
		}
		*t1Len = tkLen;
	}
	if (track2)
	{
		tkLen = MSRGetDataLength(1);
		if (tkLen <= *t2Len)
		{
			MsrGetData(1, (BYTE *)track2, &decode);
		}
		*t2Len = tkLen;
	}
	if (track3)
	{
		tkLen = MSRGetDataLength(2);
		if (tkLen <= *t3Len)
		{
			MsrGetData(2, (BYTE *)track3, &decode);
		}
		*t3Len = tkLen;
	}
	return 0;
}

//2----SCR
HANDLE	DATA_READY_Event = CreateEvent( NULL, FALSE, FALSE, NULL );
HANDLE	PowerOn_Event	 = CreateEvent( NULL, FALSE, FALSE, NULL );
HANDLE	PROTOCOL_ERR	 = CreateEvent( NULL, FALSE, FALSE, NULL );
HANDLE	POWER_FAILURE	 = CreateEvent( NULL, FALSE, FALSE, NULL );
HANDLE	PARITY_ERR		 = CreateEvent( NULL, FALSE, FALSE, NULL );
HANDLE	NO_RESPONSE		 = CreateEvent( NULL, FALSE, FALSE, NULL );
HANDLE	CT_STATUS		 = CreateEvent( NULL, FALSE, FALSE, NULL );
HANDLE	PPS_NEGOTIATED	 = CreateEvent( NULL, FALSE, FALSE, NULL );
HANDLE  CARD_REMOVED	 = CreateEvent( NULL, FALSE, FALSE, NULL );
HANDLE  DEV_OFFLINE		 = CreateEvent( NULL, FALSE, FALSE, NULL );
HANDLE  DEV_ONLINE		 = CreateEvent( NULL, FALSE, FALSE, NULL );

/*=======================================================================
// Function: SCR_event
// Purpose:	deal scr event
// Return Value:				
//			null	
// Parameter:
//			null	
/======================================================================*/
void SCR_event( WORD iccID, void *user, DWORD event )
{
	switch (event)
	{
	case SCR_EVENT_RESET_DONE:
		SetEvent( PowerOn_Event );
		break;
		
	case SCR_EVENT_DATA_READY:
		SetEvent( DATA_READY_Event );
		break;
		
	case SCR_EVENT_CARD_REMOVED:
		SetEvent( CARD_REMOVED );
		break;
		
	case SCR_EVENT_PROTOCOL_ERR:
		SetEvent( PROTOCOL_ERR );
		break;
		
	case SCR_EVENT_POWER_FAILURE:
		SetEvent( POWER_FAILURE );
		break;
		
	case SCR_EVENT_PARITY_ERR:
		SetEvent( PARITY_ERR );
		break;
		
	case SCR_EVENT_NO_RESPONSE:
		SetEvent( NO_RESPONSE );
		break;
		
	case SCR_EVENT_CT_STATUS:			
		SetEvent( CT_STATUS );
		break;
		
	case SCR_EVENT_PPS_NEGOTIATED:
		SetEvent( PPS_NEGOTIATED );
		break;
		
	case SCR_EVENT_DEV_OFFLINE:
		SetEvent( DEV_OFFLINE );
		break;
		
	case SCR_EVENT_DEV_ONLINE:
		SetEvent( DEV_ONLINE );
		break;
		
	default:
		break;
	}
}

/*=======================================================================
// Function: Open SCR
// Purpose:	Open and Init SCR
// call timing:  Device Init
// Return Value:				
//			0x00: Success
//			0x01: Error
// Parameter:
//			null	
/======================================================================*/
DWORD OpenScr()
{
	DWORD dwRtn = 0;

	dwRtn = ScrOpen();
	if(dwRtn != 0)
	{
		return 1;
	}
	
	dwRtn = ScrRegister(SCR_event, NULL);	
	
	if(dwRtn != 0)
	{
		return 1;
	}
	
	return 0;	//Success
}

/*=======================================================================
// Function: Close SCR
// Purpose:	Close SCR
// call timing:  Device close
// Return Value:				
//			0x00: Success
//			0x01: Error
// Parameter:
//			null	
/======================================================================*/
DWORD CloseScr()
{
	DWORD dwRtn = 0;
	dwRtn = ScrClose();
	if(dwRtn != 0)
	{
		return 1;
	}	
	return 0;
}
/*=======================================================================
// Function: SCR Power ON
// Purpose:	Power on card
// call timing:  card instertted
// Return Value:				
//			0: Success
//			1: NO RESPONSE
//			2: POWER FAIL
//			3: DATA READY
//			4: PPS NEGOTIATED
//			5: CARD REMOVED
//			6: PROTOCOL ERR
//			7: PARITY_ERR
//			8: CT_STATUS
//			9: DEV OFFLINE
//			10: DEV ONLINE
//			11: Time Out
//			12: WAIT FAILED
//			other: Device Error
// Parameter:
//			null	
/======================================================================*/
DWORD PowerOnScr()
{
	DWORD dwRtn, dwRtn1, dwError;
	HANDLE		HandlesToWaitFor[11];
	
	HandlesToWaitFor[0] = PowerOn_Event;
  HandlesToWaitFor[1] = NO_RESPONSE;
	HandlesToWaitFor[2] = POWER_FAILURE;
	HandlesToWaitFor[3] = DATA_READY_Event;
	HandlesToWaitFor[4] = PPS_NEGOTIATED;
	HandlesToWaitFor[5] = CARD_REMOVED;
	HandlesToWaitFor[6] = PROTOCOL_ERR;
	HandlesToWaitFor[7] = PARITY_ERR;
	HandlesToWaitFor[8] = CT_STATUS;
	HandlesToWaitFor[9] = DEV_OFFLINE;
	HandlesToWaitFor[10]= DEV_ONLINE;
	
	
	dwRtn1 = ScrPowerOn(CARD_ICC_SLOT, 3);
	if (dwRtn1)
	{
		return dwRtn1;
	}
	
	Sleep(1000);
	
	dwRtn = WaitForMultipleObjects(11, HandlesToWaitFor, FALSE, 60000);
	
	switch(dwRtn)
	{	
	case WAIT_OBJECT_0:			
		dwError = 0;
		break;
	case WAIT_OBJECT_0+1:
		dwError = 1;//NO RESPONSE
		break;
	case WAIT_OBJECT_0+2:
		dwError = 2;//POWER FAIL
		break;	
	case WAIT_OBJECT_0+3:
		dwError = 3;//DATA READY
		break;
	case WAIT_OBJECT_0+4:
		dwError = 4;//PPS NEGOTIATED
		break;
	case WAIT_OBJECT_0+5:
		dwError = 5;//CARD REMOVED
		break;
	case WAIT_OBJECT_0+6:
		dwError = 6;//PROTOCOL ERR
		break;
	case WAIT_OBJECT_0+7:
		dwError = 7;//PARITY_ERR
		break;
	case WAIT_OBJECT_0+8:
		dwError = 8;//CT_STATUS
		break;
	case WAIT_OBJECT_0+9:
		dwError = 9;//DEV OFFLINE
		break;
	case WAIT_OBJECT_0+10:
		dwError = 10;//DEV ONLINE
		break;
	case WAIT_TIMEOUT:
		dwError = 11;//TIMEOUT
		break;
	case WAIT_FAILED:
		dwError = 12;//WAIT FAILED
		break;
	default:
		dwError = 11;//TIMEOUT	
		break;
	}
	
	return dwError;
}

/*=======================================================================
// Function: SCR Power Off
// Purpose:	Power off card
// call timing:  Power off card
// Return Value:				
//			0: Success
//			1: Error
// Parameter:
//			null	
/======================================================================*/
DWORD PowerOffScr()
{
	DWORD dwRtn ;
	dwRtn = ScrPowerOff(CARD_ICC_SLOT);
	if(dwRtn != 0)
	{
		return 1;
	}
	
	return 0;	//Success
}

/*=======================================================================
// Function: Get SCR Status
// Purpose:	Detect card in
// call timing:  wait for IC card in
// Return Value:				
//			0: CARD_ABSENT
//			1: Card insert
//			other: Device error
// Parameter:
//			null	
/======================================================================*/
DWORD GetScrStatus()
{
	DWORD dwRtn;
	DWORD status;

	dwRtn = ScrGetState(CARD_ICC_SLOT, &status);
	
	TRACE(TEXT("GetScrStatus()\n"));

	if (dwRtn != 0)
	{
		TRACE(L"dwRtn=%X\n", dwRtn);
//		return dwRtn;
		return SCR_CARD_ABSENT;
	}

	TRACE(L"status=%X\n", status);
	return status;
}

/*=======================================================================
// Function: SCR send data
// Purpose:	Send data
// call timing:  Send data
// Return Value:				
//			0: Success
//			1: Error
// Parameter:
//			[In] lpBuffer:Pointer to data buffer to be transmitted	
//			[In] wLen:Number of bytes in buffer to be transmitted
/======================================================================*/
DWORD SendSCR(BYTE *lpBuffer, WORD wLen)
{
	DWORD dwRtn = ScrSendData(CARD_ICC_SLOT, lpBuffer, wLen );
	if(dwRtn != 0)
	{
		return 1;
	}	
	return 0;	//Success
}

/*=======================================================================
// Function: SCR receive data
// Purpose:	Receive data
// call timing:  Receive data
// Return Value:				
//			0: Success
//			1: Error
// Parameter:
//			[Out] lpBuffer:Pointer to data buffer to be transmitted	
//			[Out] wLen:Number of bytes in buffer to be transmitted
/======================================================================*/
DWORD ReceiveSCR(BYTE *lpBuffer, WORD *pwLen)
{
	DWORD dwRtn, dwLen;
	
	TRACE(TEXT("ReceiveSCR()\n"));

	dwRtn = WaitForSingleObject(DATA_READY_Event, 6000);
	if(dwRtn != WAIT_OBJECT_0)
	{
		TRACE(L"dwRtn=%X\n", dwRtn);
		return 1;	
	}

	dwLen = ScrGetDataLength();
	dwRtn = ScrGetData(lpBuffer);
	
	TRACE(L"dwLen=%X\n", dwLen);
	TRACE(L"dwRtn=%X\n", dwRtn);

	if(dwRtn==-1)
		return -1;	
	
	*pwLen = (WORD)dwRtn;

	return 0;
}
