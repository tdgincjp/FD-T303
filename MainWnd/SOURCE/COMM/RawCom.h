//---------------------------------------------------------------------------
//	Copyright (C) 1998 XAC Automation Corp.
//	All rights reserved.
//
//  File: RawCom.h
//
//  Purpose:
//		Raw Communication Module
//			typedefs 
//			function protocols
//
//	Created:	April 22, 1998 by Jeng-Ming Duann
//	Revised:	July 16, 1998 by Jeng-Ming Duann
//
//---------------------------------------------------------------------------

#ifndef _RAWCOM_H_
#define _RAWCOM_H_

//===========================================================================
//
// Typedefs
//
//===========================================================================

typedef struct
	{		// structure to save information related to comm port

	HANDLE	hComPort;		// handle of comm. port, return from CreateFile
	TCHAR	gbPortName[15];	// serial port name
	DWORD	dwErrorCode;	// error code

	// overlapped structures
	OVERLAPPED	ovRead;		// overlapped structure for reading
	OVERLAPPED	ovWrite;	// overlapped structure for writing
	OVERLAPPED	ovWatch;	// overlapped structure for watching
	HANDLE		hCloseEvent;// handle of close event 

	} COMMINFO, *LPCOMMINFO ;

#ifdef __cplusplus
extern "C" {
#endif  /* __cplusplus */

//===========================================================================
//
// Function Protocols
//
//===========================================================================

LPCOMMINFO		// handle of comm port, pointer to comm port information structure
ComOpen(		// open a comm port
	LPCTSTR		lpPortName		// string pointer to comm port name, i.e. COM1, COM2
	);

BOOL			// return TRUE, if operation succeed
ComClose(		// close a comm port
	LPCOMMINFO	hCom			// handle of comm port
	);

BOOL 
ComSetup(
	LPCOMMINFO hCom,
	DWORD BaudRate, 
	BYTE ByteSize,
	BYTE Parity,
	BYTE StopBits
	);

BOOL			// return TRUE, if operation succeed
ComRead(		// read data from comm port
	LPCOMMINFO	hCom,			// handle of comm port
	LPVOID		lpBuffer,		// pointer to the buffer into which data is to be read
	DWORD		dwBytesToRead,	// length in bytes of the data to be read
    DWORD       *pReaded
	);

BOOL			// return TRUE, if operation succeed
ComWrite(		// write data to comm port
	LPCOMMINFO	hCom,			// handle of comm port
	LPVOID		lpBuffer, 		// pointer to the buffer which contains data to write
	DWORD		dwBytesToWrite	// length in bytes of the data to write
	);

VOID 
ComSetError(	// set the comm port error code of operation
	LPCOMMINFO	hCom,			// handle of comm port
	DWORD		nError			// error code
	);

#ifdef __cplusplus
}
#endif
#endif /* _RAWCOM_H_ */
//----------------------------
// end of RawCom.h
//----------------------------