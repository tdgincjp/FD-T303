#include "StdAfx.h"
#include "MSR.h"
#include "language.h"
//#include "SaioReader.h" // necessary include file to use HAL reader API.
	
HWND m_hWndMSR;	      // Main Dialog Handle

CMSR::CMSR(void)
{
	for(int i = 0; i<3;i++)
	{
		m_TrackLen[i] = 0;   
		m_TrackDecode[i] = 0;  
		m_TrackError[i] = 0;
		m_TrackData[i] = 0;        
	}
	m_IPS = 0;
}

CMSR::~CMSR(void)
{
	MsrRegister(NULL, NULL);        // To un-register the callback function if there is any
	MsrClose();                     // Closes a opened MSR service

	for(int i = 0; i<3;i++)
	{
		if(m_TrackLen[i] != 0)
			delete [] m_TrackData[i];
		m_TrackLen[i] = 0;
	}
}

//-----------------------------------------------------------------------------
//!	\change buffer data to string
//!	\param	bBuf - buffer data point 
//!	\param	Len - buffer data length
//-----------------------------------------------------------------------------
CString CMSR::ByteToStr(BYTE *bBuf, int Len)
{
	int index;
	CString str,temp;

	for (index = 0; index < Len; index++)
	{
		if (bBuf[index] != 0x0d && bBuf[index] != 0x0a)
		{	
			temp.Format (L"%c", bBuf[index]);
			str += temp;
		}
	}
	return str;
}

//-----------------------------------------------------------------------------
//!	\MSR enent setting
//!	\param	user -  
//!	\param	event - msr event
//-----------------------------------------------------------------------------
void MsrEvent(void *user,unsigned long event)
{
	::PostMessage( m_hWndMSR, event, 0, 0 );
}

//-----------------------------------------------------------------------------
//!	\Open MSR
//!	\param	hwnd - event receive window handle
//-----------------------------------------------------------------------------
BOOL CMSR::Open (HWND hwnd)
{
	m_hWndMSR = hwnd;

	if( MsrOpen())
		return FALSE;

	MsrRegister(MsrEvent, NULL);
	MsrReadEnable(FALSE,TRUE,FALSE,MSR_ISO_DECODE);
	for(int i = 0; i<3;i++)
		m_TrackLen[i] = 0;
	return TRUE;
}

//-----------------------------------------------------------------------------
//!	\close MSR
//-----------------------------------------------------------------------------
void CMSR::Cancel()
{
	MsrCancel();
	for(int i = 0; i<3;i++)
	{
		if(m_TrackLen[i] != 0)
			delete [] m_TrackData[i];
		m_TrackLen[i] = 0;
	}
//	m_hWndMSR = 0;
}

//-----------------------------------------------------------------------------
//!	\Get Track data after receive MSR event
//-----------------------------------------------------------------------------
BOOL CMSR::MsrGetTrackData()
{
	if(m_hWndMSR == 0)
		return FALSE;

	CString str;
	WORD Speed = 0;

	Sleep(100);

	BOOL flag = FALSE;
	for ( int i = 0; i<3;i++)
	{
		if(m_TrackLen[i] != 0)
			delete [] m_TrackData[i];
		m_TrackLen[i] = 0;
		m_TrackLen[i] = MSRGetDataLength(i);   
		if ( m_TrackLen[i] == 0 )
			continue;
		m_TrackData[i] = new byte[m_TrackLen[i]];
		MsrGetData(i, m_TrackData[i], &m_TrackDecode[i]);
		MsrGetTrackError(i, &m_TrackError[i]);
		m_TrackText[i] = ByteToStr(m_TrackData[i],m_TrackLen[i]);
//		delete [] m_TrackData[i];
		m_TrackErrorText[i] = GetErrorMsg(m_TrackError[i]);
		flag = TRUE;
	}
	return flag;
}

//-----------------------------------------------------------------------------
//!	\Get Track data after receive MSR event
//!	\param	TrackIndex - Track index 
//!	\param	buf - track buffer point 
//!	\param	len - track buffer length 
//!	\param	decode - track decode  
//-----------------------------------------------------------------------------
BOOL CMSR::GetMSRData(int TrackIndex,BYTE* buf, DWORD* len, DWORD* decode)
{
	if(m_TrackLen[TrackIndex-1] == 0)
		return FALSE;

	*len = m_TrackLen[TrackIndex-1];
	memcpy(buf,m_TrackData[TrackIndex-1],*len);
	buf[*len] = 0;
	*decode = m_TrackDecode[TrackIndex-1];

	return TRUE;
}

//-----------------------------------------------------------------------------
//!	\Get error string
//!	\param	ErrCode - error code
//-----------------------------------------------------------------------------
CString CMSR::GetErrorMsg(DWORD ErrCode)
{
    CString str;

	switch (ErrCode)
	{
		  // Track read error
		  case MSR_NO_ERROR:                // 0:Pass
			  str = L"PASS";
			  break;
		  case MSR_UNABLE_READ:             // 1:Media was detected, but unable to be read
			  str = L"Err(#1)";
			  break;
		  case MSR_INVALID_ENCODE:          // 2:Media had invalid encoding
			  str = L"Err(#2)";
			  break;
		  case MSR_NO_ENCODE:               // 3:No media encoding
			  str = L"Err(#3)";
			  break;
		  case MSR_TRACK_UNSUPPORT:         // 4:Track is not supported by the reader
			  str = L"Err(#4)";
			  break;
		  case MSR_DATA_CORRUPT:            // 5:Characters was invalid or corrupt		  
			  str =  L"Err(#5)";
			  break;
		  case MSR_PARITY_ERROR:            // 6:A parity error was detected
			  str = L"Err(#6)";
			  break;
		  case MSR_LRC_ERROR:               // 7:An LRC error was detected 
			  str =  L"Err(#7)";
			  break;
		  case MSR_NO_DATA:                 // 8:No track data was present
			   str = L"Err(#8)";
			  break;
		  case MSR_NO_ENCODE_TRACK:         // 9:No encoded track signal was present
			   str =  L"Err(#9)";
			  break;
		  default:
			  str = CLanguage::GetText(CLanguage::IDX_UNKNOWN);             // Unknown Error
			  break;
	}
    return str;
}
