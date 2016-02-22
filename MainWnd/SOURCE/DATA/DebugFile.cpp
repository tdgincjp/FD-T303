
// DebugFile.cpp : implementation file
//
// JC Nov 11/15 to add debug.txt write 
//

#include "stdafx.h"
#include <stdio.h>

#include "DebugFile.h"
#include "..\\defs\\constant.h"
#include "..\\UTILS\\string.h"
//#include <iostream>
#include <fstream>
using namespace std;

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

char rstr[100];
char * DaysOfWeek[] = { "LogSun.txt", "LogMon.txt", "LogTue.txt", "LogWed.txt", "LogThu.txt", "LogFri.txt", "LogSat.txt" };


CDebugFile::CDebugFile()
{
}
	
//////////////////////////////////////////////////////////////////////////////////////////////////
//JC Nov 11/15
// Writes data for Debugging JC Nov 2015
// This file will be written to a USB stick when copied from config menu
// 
//////////////////////////////////////////////////////////////////////////////////////////////////	
	
void CDebugFile::DebugWrite(char *data)
{
  char dstr[30];
  
  std::ofstream outfile;
  outfile.open("\\SAIO\\User Programs\\INIT\\debug.txt", std::ios_base::app);
  SYSTEMTIME LocalTime;
	::GetSystemTime( &LocalTime );
  CTime t1( LocalTime );
  sprintf(dstr,"%04d/%02d/%02d %02d:%02d:%02d ",t1.GetYear(), t1.GetMonth(), t1.GetDay(), t1.GetHour(),t1.GetMinute(),t1.GetSecond());
  outfile << dstr;
  outfile << data;
  outfile << "\n";		//Add CR
  outfile.close();
//JC Nov 23/15  Moved to start of each transaction  DebugSizeCheck();
  return;


}

//////////////////////////////////////////////////////////////////////////////////////////////////
//JC Nov 11/15
// Writes data for Debugging JC Nov 2015
// This file will be written to a USB stick when copied from config menu
// 
//////////////////////////////////////////////////////////////////////////////////////////////////	
	
void CDebugFile::DebugLogHost(char *data)
{
  char dstr[20];
  char fn[100];
  char Data2Log[1024];
  
  CheckLogFileName();
  std::ofstream outfile;
  strcpy(fn,LogFileName());
  outfile.open(fn, std::ios_base::app);
  SYSTEMTIME LocalTime;
	::GetSystemTime( &LocalTime );
  CTime t1( LocalTime );
  sprintf(dstr,"%04d/%02d/%02d %02d:%02d:%02d ",t1.GetYear(), t1.GetMonth(), t1.GetDay(), t1.GetHour(),t1.GetMinute(),t1.GetSecond());
  outfile << dstr;
  strcpy(Data2Log, data);				//Copy the data to log so that we don't change it
  CheckLogPCI(Data2Log);				//Modify the log record to remove any PCI sensitive data
  outfile << Data2Log;
  outfile << "\n";		//Add CR
  outfile.close();

  //DebugSizeCheck();		moved to ECRicomm.cpp start of each transaction.

}

//////////////////////////////////////////////////////////////////////////////////////////////////
// JC Nov 11/15
// Checks debug.txt file size if > 30K bytes will rename to debug.old
// debug.txt will be recreated automatically on next write
//////////////////////////////////////////////////////////////////////////////////////////////////

void CDebugFile::DebugSizeCheck()
{
  std::ifstream infile;
  infile.open("\\SAIO\\User Programs\\INIT\\debug.txt", std::ifstream::binary);
  infile.seekg (0, infile.end);
  long length = infile.tellg();
  //int length=0;
  infile.close();
  if (length > 30000)
  {
	  //delete debug.old
	  DeleteFile(L"\\SAIO\\User Programs\\INIT\\debug.old");
	  //Rename file debug.txt to debug.old
	  MoveFile(L"\\SAIO\\User Programs\\INIT\\debug.txt",L"\\SAIO\\User Programs\\INIT\\debug.old");
  }
  return; 
}
//////////////////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////////////////////
// JC Nov 13/15
// Determine LogFileName by Day of Week
// Checks if LogFileName is current or old. If Old Delete Old File.
//////////////////////////////////////////////////////////////////////////////////////////////////

char * CDebugFile::LogFileName()
{
  char wstr[100];
  strcpy(wstr, "\\SAIO\\User Programs\\INIT\\");
  SYSTEMTIME LocalTime;
	::GetSystemTime( &LocalTime );
  CTime t1( LocalTime );
  sprintf(rstr,"%s%s",wstr,DaysOfWeek[t1.GetDayOfWeek()-1]);
  return(rstr);
}
//////////////////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////////////////////
// JC Nov 13/15
// 
// Checks if LogFileName is current or old. If Old Delete Old File.
//////////////////////////////////////////////////////////////////////////////////////////////////

void  CDebugFile::CheckLogFileName()
{
  char wstr[MAX_PATH];
  strcpy(wstr, "\\SAIO\\User Programs\\INIT\\");
  SYSTEMTIME LocalTime;
	::GetSystemTime( &LocalTime );
  CTime t1( LocalTime );
  sprintf(rstr,"%s%s",wstr,DaysOfWeek[t1.GetDayOfWeek()-1]);


  FILETIME ftCreate, ftAccess, ftWrite;
  SYSTEMTIME stUTC;
  HANDLE hFile;

  wchar_t Crstr[MAX_PATH];
  mbstowcs(Crstr, rstr, strlen(rstr)+1);
  hFile = CreateFile(Crstr, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
  // Retrieve the file times for the file.

  if (!GetFileTime(hFile, &ftCreate, &ftAccess, &ftWrite)) 
	  return;
  // Convert the last-write time to local time.
  CloseHandle(hFile);
  FileTimeToSystemTime(&ftWrite, &stUTC);
  // Convert the last-write time to local time.
  int y=t1.GetYear();
  int m=t1.GetMonth();
  int d=t1.GetDay();
  if ((stUTC.wYear != t1.GetYear()) || (stUTC.wMonth != t1.GetMonth()) || ( stUTC.wDay != t1.GetDay()))
  {
		//Delete the file
	    DeleteFile(Crstr);
  }
	  
  return;
}
//////////////////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////////////////////
// JC Nov 13/15
// Determine LogFileName by Day of Week
// Checks if LogFileName is current or old. If Old Delete Old File.
//
// This is directly from the Everlink Base24 version, not applicable to FD, but we need to parse
// and remove any PCI data. ie. PAN, PIN Block
//////////////////////////////////////////////////////////////////////////////////////////////////

void CDebugFile::CheckLogPCI(char *data)
{
/*	char FID[3];
	char d1[1024];
	char d2[1024];
	char dz[1024];
	char wtrackII[100];
	char *posptr;
	int pos=0;
	int i;
	int savei;
	FID[0] = 28;			//FS 
	FID[1] = 'q';
	FID[2] = 0;
	posptr = strstr(data, FID);
	if (posptr != NULL)
	{
		pos = int (posptr - data); 
	}
	if (pos > 0) //we have fid q
	{
		memcpy(d1,data,pos+7);			//up to fid q + first 4 chars of PAN
		d1[pos+7]=0;					//make it a string
		memcpy(dz, &data[pos+7], (strlen(data)-(pos+7)));			//rest of data after start of PAN
		dz[strlen(data)-(pos+7)] = 0;
		FID[1]=0;
		posptr = strstr(dz, FID);
		if (posptr != NULL)
			pos = int (posptr - dz);
		else
			pos = 0;
		wtrackII[0] = 0;											//ensure Null terminated
		if (pos>0) 
		{
			memcpy(d2,&dz[pos-1], (strlen(dz)-(pos-1)));			//from next FS to end
			d2[(strlen(dz)-(pos-1))] = 0;							//null terminate
			memcpy(wtrackII, dz, pos);								//rest of fid 'q'
			savei=0;
			for (i=0;i<pos;i++)
			{
				if (dz[i] == '=')									//expiry date
				{
					savei=i;
				}
				wtrackII[i]='*';
				wtrackII[i+1]=0;
			}
			if (savei > 0)
			{
				memcpy(&wtrackII[savei-4], &dz[savei-4],4);
			}
		}
		else
			d2[0]=0;
		strcat(d1, wtrackII);										//add masked TRACKII
		strcat(d1,d2);												//add rest of data
		strcpy(data, d1);											//modify data with masked FID q
	}
	*/
}


