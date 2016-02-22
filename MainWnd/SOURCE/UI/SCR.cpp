#include "StdAfx.h"
#include "SCR.h"

#include "SaioReader.h" 
#include "..\\EMV\\SaioDev.h"
#include "..\\Data\\datafile.h"
#pragma comment(lib, "saiobase.lib")
#pragma comment(lib, "Lib\\saioReader.lib")

HWND m_hWndSCR;	      

CSCR::CSCR(void)
{
	m_IccSlot = CARD_ICC_SLOT;
	m_IccPower = 0;//3;
	Event_time_out = 6000;

	m_DataLen = 0;
}

CSCR::~CSCR(void)
{
	Close();
}

//----------------------------------------------------------------------------
//!	\brief	Close SCR
//----------------------------------------------------------------------------
void CSCR::Close()
{
	ScrClose();
	ScrPowerOff(m_IccSlot);
}

//----------------------------------------------------------------------------
//!	\brief	Close SCR
//!	\param	hwnd - SCR event receive window handle
//----------------------------------------------------------------------------
BOOL CSCR::Open (HWND hwnd)
{
	m_hWndSCR = hwnd;
	if ( ScrOpen())
		return FALSE;
	return TRUE;
}

//----------------------------------------------------------------------------
//!	\brief	Check SCR data after receive scr event
//----------------------------------------------------------------------------
BOOL CSCR::GetSCRData()
{
	if (m_DataLen == 0)
		return FALSE;
	return TRUE;
}

//----------------------------------------------------------------------------
//!	\brief	Get Card state (in slot , not in slot, power off
//----------------------------------------------------------------------------
BOOL CSCR::GetState()
{
	DWORD   ret;
	DWORD   status;
	m_bPowerOff = FALSE;
	// m_IccSlot: (0->CARD_ICC_SLOT, 1->CARD_SAM_1, 2->CARD_SAM_2, 3->CARD_SAM_3, 4->CARD_SAM_4)
	ret = ScrGetState( m_IccSlot, &status );  //Return zero if the function succeeds else nonzero error code
	if ( ret != 0 )
		return FALSE;

	if ( status ==  SCR_POWER_OFF )
		return PowerOn();
	else if ( status ==  SCR_POWER_ON)
		return TRUE;

	return FALSE;
}

//----------------------------------------------------------------------------
//!	\brief	Power on SCR
//----------------------------------------------------------------------------
BOOL CSCR::PowerOn()
{
	DWORD    err;
	CString  error;

	if ( !Scr_PowerOn(m_IccSlot, m_IccPower, &err) )
	{
		CDataFile::UpdateStatistics(1);
		m_bPowerOff = TRUE;
		return false;
	}	
	return true;
}

//----------------------------------------------------------------------------
//!	\brief	Close SCR and power off
//----------------------------------------------------------------------------
void CSCR::Cancel()
{
	ScrCancel();
	PowerOffScr();
}

//----------------------------------------------------------------------------
//!	\brief	Close SCR
//!	\param	iccID - slot id
//!	\param	voltage - power on voltage
//!	\param	errCode - err code
//----------------------------------------------------------------------------
BOOL CSCR::Scr_PowerOn(WORD iccID, WORD voltage, DWORD *errCode)
{
	DWORD   ret = 0;
	DWORD   Scr_event;	

	for (int i=0;i<3;i++)
	{
		ret = ScrPowerOn( iccID, voltage );  //Return zero if the function succeeds else nonzero error code

		if (ret != 0)
		{
			*errCode = ret;
			return FALSE;
		}
		ret = ScrWaitForEvent( &Scr_event, Event_time_out );  //Return zero if an event is received else nonzero error code

		if (ret != 0)
		{
			*errCode = ret;
			return FALSE;
		}
		else
		{
			if (Scr_event == SCR_EVENT_RESET_DONE) 
				return TRUE;
			else if (Scr_event == SCR_EVENT_NO_RESPONSE) 
			{
				ScrCancel();
				Sleep(100);
				continue;
			}
			else  //Other Error Event
			{ 
				*errCode = Scr_event;
				return FALSE;
			}
		}
	}
	Close();
	*errCode = Scr_event;
	return FALSE;
}

//----------------------------------------------------------------------------
//!	\brief	reset SCR
//----------------------------------------------------------------------------
void CSCR::Reset()
{
	DWORD    err;
	Scr_Reset(m_IccSlot,&err);
}

//----------------------------------------------------------------------------
//!	\brief	reset SCR
//!	\param	iccID - slot id
//!	\param	errCode - err code
//----------------------------------------------------------------------------
BOOL CSCR::Scr_Reset(WORD iccID, DWORD *errCode)
{
	DWORD   ret = 0;
	DWORD   Scr_event;
	
	ret = ScrReset( iccID );  //Return zero if the function succeeds else nonzero error code

	if (ret != 0)
	{
		*errCode = ret;
		return FALSE;
	}
	
	ret = ScrWaitForEvent( &Scr_event, Event_time_out );  //Return zero if an event is received else nonzero error code

	if (ret != 0)
	{
		*errCode = ret;
		return FALSE;
	}
	else
	{
		if (Scr_event == SCR_EVENT_RESET_DONE)
			return TRUE;
		else  //Other Error Event
		{ 
			*errCode = Scr_event;
			return FALSE;
		}
	}
}

//----------------------------------------------------------------------------
//!	\brief	SCR Send data
//!	\param	iccID - slot id
//!	\param	buffer - send data buffer
//!	\param	len - send data buffer length
//!	\param	errCode - err code
//----------------------------------------------------------------------------
BOOL CSCR::Scr_SendData(WORD iccID, BYTE *buffer, DWORD len, DWORD *errCode)
{
	DWORD   ret = 0;
	DWORD   Scr_event;

	ret = ScrSendData( iccID, buffer, len );  //Return zero if the function succeeds else nonzero error code

	if (ret != 0)
	{
		*errCode = ret;
		return FALSE;
	}
	
	ret = ScrWaitForEvent( &Scr_event, Event_time_out );  //Return zero if an event is received else nonzero error code

	if (ret != 0)
	{
		*errCode = ret;
		return FALSE;
	}
	else
	{
		if (Scr_event == SCR_EVENT_DATA_READY)
			return TRUE;
		else  //Other Error Event
		{ 
			*errCode = Scr_event;
			return FALSE;
		}
	}
}
