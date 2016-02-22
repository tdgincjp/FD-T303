#include "stdafx.h"
#include "LogFile.h"
#include "afx.h"


/******************************************************************************/

LPCTSTR CLogFile::m_lpszFileName = L"\\SAIO\\User Programs\\INIT\\LogFile";;
LPCTSTR CLogFile::m_lpszOldFile= L"\\SAIO\\User Programs\\INIT\\OldLog";
LPCTSTR CLogFile::m_lpszUsbLocation= L"\\hard disk\\download\\LogFile";
LPCTSTR CLogFile::m_lpszOldUsbFile= L"\\hard disk\\download\\OldLog";  //JC June 23/15 
LPCTSTR CLogFile::m_lpszFileDebug = L"\\SAIO\\User Programs\\INIT\\debug.txt";	//JC Nov 11/15
LPCTSTR CLogFile::m_lpszOldDebug= L"\\SAIO\\User Programs\\INIT\\debug.old";		//JC Nov 11/15
LPCTSTR CLogFile::m_lpszUsbDebug= L"\\hard disk\\download\\debug.txt";		//JC Nov 11/15
LPCTSTR CLogFile::m_lpszOldUsbDebug= L"\\hard disk\\download\\debug.old";		//JC Nov 11/15 



CLogFile::CLogFile()
{
}

CLogFile::~CLogFile()
{

}


BOOL CLogFile::WriteRecord(LogRecord *rec)
{
	HANDLE hFile = CreateFile(m_lpszFileName, GENERIC_WRITE, FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE)
	{
		HANDLE hFile2 = CreateFile(m_lpszFileName, GENERIC_WRITE, FILE_SHARE_WRITE, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);
		if (hFile2 == INVALID_HANDLE_VALUE)
			return FALSE;
		LogRecord create = LogRecord(LOG_APPLICATION, EVENT_CREATE);
		strcpy(create.PinpadSerial,rec->PinpadSerial);
		strcpy(create.TermID, rec->TermID);
		CFile logfile (hFile2);
		logfile.Write(create.ToString(), create.len);
		logfile.Close();
		CloseHandle(hFile2);
		return TRUE;					//JC June 23/15 We wrote our create record so exit now
	}
	if (hFile == INVALID_HANDLE_VALUE)
		hFile = CreateFile(m_lpszFileName, GENERIC_WRITE, FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile != INVALID_HANDLE_VALUE)
	{
		CFile logfile (hFile);
		logfile.SeekToEnd();
		logfile.Write(rec->ToString(), rec->len);

		//Check size and move to backup
		if (logfile.GetLength() > 40000)
		{
			logfile.Close();
			try
			{
			   CFile::Remove(m_lpszOldFile);
			}
			catch (CFileException* pEx)
			{
			   TRACE(_T("File %20s cannot be removed\n"), m_lpszOldFile);
			   pEx->Delete();
			}
			CFile::Rename(m_lpszFileName, m_lpszOldFile);
			TRACE(_T("File %20s renamed to %20s\n"), m_lpszFileName, m_lpszOldFile);
			hFile = CreateFile(m_lpszFileName, GENERIC_WRITE, FILE_SHARE_WRITE, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);
			LogRecord create = LogRecord(LOG_APPLICATION, EVENT_CREATE);
			strcpy(create.PinpadSerial,rec->PinpadSerial);
			strcpy(create.TermID, rec->TermID);
			CFile logfile (hFile);
			logfile.Write(create.ToString(), create.len);
			return TRUE;
		}
		logfile.Close();
		CloseHandle(hFile);
		return TRUE;
	}

	return FALSE;
}


CStringArray ResultArray;
CStringArray * CLogFile::CopyToUsb()
{
	CString strReturn1, strReturn2, strReturn3, strReturn4;			//JC Nov 11/15 add debug info 
	BOOL OldLog=FALSE;
	BOOL Log=FALSE;
	BOOL OldDebug=FALSE;
	BOOL Debug=FALSE;
	CString LogFilePath=L"\\SAIO\\User Programs\\INIT\\";			//JC Nov 16/15
	CString LogUSBPath=L"\\hard disk\\download\\";					//JC Nov 16/15
	CString SAIOF, USBF;											//JC Nov 16/15
	int i;															//JC Nov 16/15

	char * DaysOfWeek[] = { "LogSun.txt", "LogMon.txt", "LogTue.txt", "LogWed.txt", "LogThu.txt", "LogFri.txt", "LogSat.txt" };
	
	if (CopyFile(m_lpszOldFile, m_lpszOldUsbFile, FALSE)) //JC June 23/15 change to copy from move FALSE param will overwrite a file if it exists on USB already
		OldLog=TRUE;

	
	if (CopyFile(m_lpszFileName,m_lpszUsbLocation, FALSE)) //JC June 23/15 change to copy from move FALSE param will overwrite a file if it exists on USB already
		Log=TRUE;


	if (CopyFile(m_lpszOldDebug, m_lpszOldUsbDebug, FALSE)) //JC Nov 11/15 Added debug log (old) debug.old.txt
		OldDebug=TRUE;


	
	if (CopyFile(m_lpszFileDebug,m_lpszUsbDebug, FALSE)) //JC June 23/15 change to copy from move FALSE param will overwrite a file if it exists on USB already
		Debug=TRUE;

	for (i=0; i<7; i++)									//JC Nov 16/15 Write the Log files to USB
	{
		SAIOF = LogFilePath	+ CString(DaysOfWeek[i]);	//JC Nov 16/15
		USBF = LogUSBPath	+ CString(DaysOfWeek[i]);	//JC Nov 16/15
		CopyFile(SAIOF, USBF, FALSE);					//JC Nov 16/15
	}


	
	if ((Log) && (OldLog)) strReturn1 = CString("Log and OldLog copied to USB");							//JC Nov 11/15
	if ( (Log) && (!OldLog) ) strReturn1 = CString("Log copied, OldLog does NOT exist");					//JC Nov 11/15
	if ( (!Log) && (OldLog) ) strReturn1 = CString("Log does NOT exist, OldLog copied to USB");					//JC Nov 11/15
	if ( (!Log) && (!OldLog) ) strReturn1 = CString("ERROR- No Log Files to copy to USB!");					//JC Nov 11/15

	if ((Debug) && (OldDebug)) strReturn2 = CString("debug.txt and debug.old copied to USB");				//JC Nov 11/15
	if ( (Debug) && (!OldDebug) ) strReturn2 = CString("debug.txt copied, debug.old does not exist");	//JC Nov 11/15
	if ( (!Debug) && (OldDebug) ) strReturn2 = CString("debug.txt does NOT exist, debug.old copied to USB");	//JC Nov 11/15
	if ( (!Debug) && (!OldDebug) ) strReturn2 = CString("No debug files to copy to USB!");					//JC Nov 11/15

	ResultArray.Add(strReturn1);
	ResultArray.Add(strReturn2);
	return &ResultArray;
}