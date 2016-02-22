//---------------------------------------------------------------------------
//    Copyright (C) 1998 XAC Automation Corp.
//    All rights reserved.
//
//  File: RawCom.c
//
//  Purpose:
//        Raw Communication Module -- for Windows 95, NT
//            Basic operation for communication.
//
//    Created:    April 22, 1998 by Jeng-Ming Duann
//    Revised:    July 16, 1998 by Jeng-Ming Duann
//
//---------------------------------------------------------------------------

#include <windows.h>
#include <memory.h>
#include "RawCom.h"
#pragma warning(disable: 4996)
//----------------------------
// static function protocols:
//----------------------------

static LPCOMMINFO CreateCommInfo();
static void DestroyCommInfo(LPCOMMINFO hCom);
static BOOL OpenConnection(    LPCOMMINFO hCom, LPCTSTR lpPortName);
static BOOL CloseConnection(LPCOMMINFO hCom);

//==============================================
//
// public functions:
//
//==============================================

//---------------------------------------------------------------------------
//    FUNCTION:    ComOpen(LPCSTR)
//
//    PURPOSE:
//
//    PARAMETERS:
//        lpPortName    - Comm port name, i.e. "COM1"
//
//    RETURN VALUE:
//
//    COMMENTS:
//
//
//    Description:
//        Opens communication port specified in the COMMINFO struct.
//        It also sets the CommState and notifies the window via
//        the fConnected flag in the COMMINFO struct.
//
//---------------------------------------------------------------------------

LPCOMMINFO ComOpen(LPCTSTR lpPortName)
{
    LPCOMMINFO   hCom;

    // create communication information COMMINFO
    if (NULL == (hCom = CreateCommInfo()))
        return (NULL);

    // save port name
    wcsncpy(hCom->gbPortName, lpPortName, 14);

    // open connection
    if (!OpenConnection(hCom, lpPortName))
    {
        DestroyCommInfo(hCom);
        return (NULL);
    }
    
    return (hCom);
} // end of ComOpen()

//---------------------------------------------------------------------------
//    FUNCTION:    ComClose(LPCOMMINFO)
//
//    PURPOSE:    
//
//    PARAMETERS:
//        hCom
//
//    RETURN VALUE:
//        
//
//    COMMENTS:
//        
//---------------------------------------------------------------------------

BOOL ComClose(LPCOMMINFO hCom)
{
    BOOL ret;

    if (hCom == NULL)
        return (FALSE);

    ret = CloseConnection(hCom);
    DestroyCommInfo(hCom);
    return (ret);
}    // end of IfdClose()

//---------------------------------------------------------------------------
//    FUNCTION:    ComSetup(LPCOMMINFO, DWORD, BYTE, BYTE, BYTE)
//
//    PURPOSE:    
//
//    PARAMETERS:
//        hCom
//        BaudRate
//        ByteSize
//        Parity
//        StopBits
//
//    RETURN VALUE:
//        
//
//    COMMENTS:
//        
//---------------------------------------------------------------------------

BOOL ComSetup(LPCOMMINFO hCom, DWORD BaudRate, 
            BYTE ByteSize, BYTE Parity, BYTE StopBits)
{
    DCB        dcb;

    if (hCom == NULL)
        return (FALSE);

    if (!GetCommState(hCom->hComPort, &dcb))
        return (FALSE);

    dcb.BaudRate    = BaudRate;
    dcb.ByteSize    = ByteSize;
    dcb.Parity        = Parity;
    dcb.StopBits    = StopBits;

    dcb.fBinary        = TRUE;
    dcb.fParity        = TRUE;

    if (!SetCommState(hCom->hComPort, &dcb))
        return (FALSE);

    return (TRUE);
} // end of ComSetup()

//---------------------------------------------------------------------------
//    FUNCTION:    ComRead(LPCOMMINFO, LPVOID, DWORD)
//
//    PURPOSE:    
//
//    PARAMETERS:
//        hCom
//        lpBuffer
//        dwBytesToRead
//
//    RETURN VALUE:
//        
//
//    COMMENTS:
//        
//---------------------------------------------------------------------------

BOOL ComRead(LPCOMMINFO hCom, LPVOID lpBuffer, DWORD dwBytesToRead, DWORD *pReaded)
{
    //DWORD    dwBytesRead;
    COMSTAT    ComStat;
    DWORD    dwErrorFlags;
    DWORD    dwError;

    if (hCom == NULL)
        return (FALSE);

    if (dwBytesToRead == 0)
        return (TRUE);

    if (!ReadFile(hCom->hComPort, lpBuffer, 
        dwBytesToRead, pReaded, NULL))
        //dwBytesToRead, &dwBytesRead, &hCom->ovRead))
        {
        dwError = GetLastError();
        if (dwError == ERROR_IO_PENDING)
            {
            // We have to wait for read to complete.
            // This function will timeout according to the
            // CommTimeOuts.ReadTotalTimeoutConstant variable
            // Every time it times out, check for port errors
            return FALSE;
            /*
            while(!GetOverlappedResult(hCom->hComPort,
                &hCom->ovRead, &dwBytesRead, TRUE))
                {
                dwError = GetLastError();
                if(dwError == ERROR_IO_INCOMPLETE)    // normal result if not finished
                    continue;
                else
                    {
                    // an error occurred, try to recover
                    ClearCommError(hCom->hComPort, &dwErrorFlags, &ComStat);
                    if (dwErrorFlags > 0)
                        ComSetError(hCom, dwErrorFlags);
                    else
                        ComSetError(hCom, dwError);
                    return (FALSE);
                    }
                }    // end of while
                */
            }
        else
            {
            // some other error occurred
            ClearCommError(hCom->hComPort, &dwErrorFlags, &ComStat);
            if (dwErrorFlags > 0)
                ComSetError(hCom, dwErrorFlags);
            else
                ComSetError(hCom, dwError);
            return (FALSE);
            }
        }

    return (TRUE);

} // end of ComRead()

//---------------------------------------------------------------------------
//    FUNCTION:    ComWrite(LPCOMMINFO, LPVOID, DWORD)
//
//    PURPOSE:    
//
//    PARAMETERS:
//        hCom
//        lpBuffer
//        dwBytesToWrite
//
//    RETURN VALUE:
//        
//
//    COMMENTS:
//        
//---------------------------------------------------------------------------

BOOL ComWrite(LPCOMMINFO hCom, LPVOID lpBuffer, DWORD dwBytesToWrite)
{
    DWORD    dwBytesWritten;
    DWORD    dwErrorFlags;
    DWORD    dwError;
    COMSTAT    ComStat;

    if (hCom == NULL)
        return (FALSE);

    if (dwBytesToWrite == 0)
        return (TRUE);

    if (!WriteFile(hCom->hComPort, lpBuffer, dwBytesToWrite,
            &dwBytesWritten, &hCom->ovWrite))
        {
        dwError = GetLastError();
        if (dwError == ERROR_IO_PENDING)
            {
            // We should wait for the completion of the write operation
            // so we know if it worked or not

            // This is only one way to do this. It might be beneficial to
            // place the write operation in a separate thread
            // so that blocking on completion will not negatively
            // affect the responsiveness of the UI

            // If the write takes too long to complete, this
            // function will timeout according to the
            // CommTimeOuts.WriteTotalTimeoutMultiplier variable.
            // This code logs the timeout but does not retry
            // the write.
            return FALSE;
            /*
            while (!GetOverlappedResult(hCom->hComPort,
                &hCom->ovWrite, &dwBytesWritten, TRUE))
                {
                dwError = GetLastError();
                if (dwError == ERROR_IO_INCOMPLETE)    // normal result if not finished
                    continue;
                else
                    {
                    // an error occurred, try to recover
                    ClearCommError(hCom->hComPort, &dwErrorFlags, &ComStat);
                    if (dwErrorFlags > 0)
                        ComSetError(hCom, dwErrorFlags);
                    else
                        ComSetError(hCom, dwError);
                    return (FALSE);
                    }
                }    // end of while
                */
            }
        else
            {
            // some other error occurred
            ClearCommError(hCom->hComPort, &dwErrorFlags, &ComStat);
            if (dwErrorFlags > 0)
                ComSetError(hCom, dwErrorFlags);
            else
                ComSetError(hCom, dwError);
            return (FALSE);
            }
        }

    return (TRUE);

} // end of ComWrite()

//---------------------------------------------------------------------------
//    FUNCTION:    ComSetError(LPCOMMINFO, DWORD)
//
//    PURPOSE:    
//
//    PARAMETERS:
//        hCom
//        nError
//
//    RETURN VALUE:
//        
//
//    COMMENTS:
//        
//---------------------------------------------------------------------------

VOID ComSetError(LPCOMMINFO hCom, DWORD nError)
{
    if (hCom == NULL)
        return;

    // set error code
    hCom->dwErrorCode = nError;

    // purge any information in the buffer
    PurgeComm(hCom->hComPort, PURGE_TXABORT | PURGE_RXABORT |
            PURGE_TXCLEAR | PURGE_RXCLEAR);
}    // end of ComSetError()

//==============================================
//
// static functions:
//
//==============================================

//---------------------------------------------------------------------------
//    FUNCTION:    CreateCommInfo()
//
//    PURPOSE:    
//
//    PARAMETERS:
//        None
//
//    RETURN VALUE:
//        
//
//    COMMENTS:
//        
//---------------------------------------------------------------------------

static LPCOMMINFO CreateCommInfo()
{
    LPCOMMINFO   hCom;

    // alloc CommInfo
    if (NULL == (hCom = (LPCOMMINFO) malloc(sizeof(COMMINFO))))
        return (NULL);

    // initialize COMM info structure
    memset(hCom, 0, sizeof(COMMINFO));

    // create I/O event used for overlapped reads, writes, and watch
    hCom->ovRead.hEvent 
        = CreateEvent(NULL,    // security attri
                    TRUE,    // explicit reset req
                    FALSE,    // initial event reset
                    NULL);    // no name
    hCom->ovWrite.hEvent 
        = CreateEvent(NULL,    // security attri
                    TRUE,    // explicit reset req
                    FALSE,    // initial event reset
                    NULL);    // no name
    hCom->ovWatch.hEvent 
        = CreateEvent(NULL,    // security attri
                    FALSE,    // auto-reset
                    FALSE,    // initial event reset
                    NULL);    // no name
    hCom->hCloseEvent 
        = CreateEvent(NULL,    // security attri
                    TRUE,    // explicit reset req
                    FALSE,    // initial event reset
                    NULL);    // no name

    // check resource allocated OK
    if ((hCom->ovRead.hEvent == NULL) ||
        (hCom->ovWrite.hEvent == NULL) ||
        (hCom->ovWatch.hEvent == NULL) ||
        (hCom->hCloseEvent == NULL))
        {
        DestroyCommInfo(hCom);
        return (NULL);
        }

    return (hCom);
} // end of CreateCommInfo()

//---------------------------------------------------------------------------
//    FUNCTION:    DestroyCommInfo(LPCOMMINFO)
//
//    PURPOSE:    
//
//    PARAMETERS:
//        hCom
//
//    RETURN VALUE:
//        
//
//    COMMENTS:
//        
//---------------------------------------------------------------------------

static void DestroyCommInfo(LPCOMMINFO hCom)
{
    if (NULL == hCom)
        return;

    // clean up event objects
    if (hCom->ovRead.hEvent != NULL)
        CloseHandle(hCom->ovRead.hEvent);
    if (hCom->ovWrite.hEvent != NULL)
        CloseHandle(hCom->ovWrite.hEvent);
    if (hCom->ovWatch.hEvent != NULL)
        CloseHandle(hCom->ovWatch.hEvent);
    if (hCom->hCloseEvent != NULL)
        CloseHandle(hCom->hCloseEvent);

    free(hCom);
    return;
} // end of DestroyCommInfo()

//---------------------------------------------------------------------------
//    FUNCTION:    OpenConnection(LPCOMMINFO, LPCTSTR)
//
//    PURPOSE:    
//
//    PARAMETERS:
//        hCom
//        lpPortName
//
//    RETURN VALUE:
//        
//
//    COMMENTS:
//        
//---------------------------------------------------------------------------

static BOOL OpenConnection(LPCOMMINFO hCom, LPCTSTR lpPortName)
{
    COMMTIMEOUTS CommTimeOuts;
    BOOL bRet;

    if (NULL == hCom)
        return (FALSE);

    // open communication resource
    hCom->hComPort = CreateFile(lpPortName, 
            GENERIC_READ | GENERIC_WRITE,    // access mode
            0,                                // exclusive share
            NULL,                            // security attrs
            OPEN_EXISTING,
            0,								// 0  FILE_ATTRIBUTE_NORMAL
            NULL);

    if (hCom->hComPort == INVALID_HANDLE_VALUE)
        return (FALSE);

    // setup device buffers
    SetupComm(hCom->hComPort, 4096, 4096);

    // purge any information in the buffer
    PurgeComm(hCom->hComPort, PURGE_TXABORT | PURGE_RXABORT |
            PURGE_TXCLEAR | PURGE_RXCLEAR);

    // setup timeouts
//    CommTimeOuts.ReadIntervalTimeout = 10;//50;
//    CommTimeOuts.ReadTotalTimeoutMultiplier = 10;//5;
//    CommTimeOuts.ReadTotalTimeoutConstant = 200;//100;
//    CommTimeOuts.WriteTotalTimeoutMultiplier = 10;//5;
//    CommTimeOuts.WriteTotalTimeoutConstant = 50;//100;
    CommTimeOuts.ReadIntervalTimeout = 3;//50;
    CommTimeOuts.ReadTotalTimeoutMultiplier = 3;//5;
    CommTimeOuts.ReadTotalTimeoutConstant = 2;//100;
    CommTimeOuts.WriteTotalTimeoutMultiplier = 3;//5;
    CommTimeOuts.WriteTotalTimeoutConstant = 2;//100;
    bRet = SetCommTimeouts(hCom->hComPort, &CommTimeOuts);

    return (TRUE);
} // end of OpenConnection()

//---------------------------------------------------------------------------
//    FUNCTION:    CloseConnection(LPCOMMINFO)
//
//    PURPOSE:    
//
//    PARAMETERS:
//        hCom
//
//    RETURN VALUE:
//        
//
//    COMMENTS:
//        
//---------------------------------------------------------------------------

static BOOL CloseConnection(LPCOMMINFO hCom)
{

    if (NULL == hCom)
      return (FALSE);

    // drop DTR
    EscapeCommFunction(hCom->hComPort, CLRDTR);

    // purge any outstanding reads/writes and close device handle
    PurgeComm(hCom->hComPort, PURGE_TXABORT | PURGE_RXABORT |
                            PURGE_TXCLEAR | PURGE_RXCLEAR);

    // close handle
    CloseHandle(hCom->hComPort);

    return (TRUE);

} // end of CloseConnection()

//----------------------------
// end of RawCom.c
//----------------------------
