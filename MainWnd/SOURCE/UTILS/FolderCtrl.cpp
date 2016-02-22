	
#include "stdafx.h"
#include <stdio.h>

#include "FolderCtrl.h"
#include "CeFileFind.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#pragma warning(disable: 4996)

CFolderCtrl::CFolderCtrl()
{
}

//-----------------------------------------------------------------------------
//!	\Delete directory
//! \param Path - directory path
//-----------------------------------------------------------------------------
void CFolderCtrl::DeleteDir(const char* Path)
{
	CString str(Path);
	DeleteDir(str);
}
	
//-----------------------------------------------------------------------------
//!	\Delete directory
//! \param szPath - directory path
//-----------------------------------------------------------------------------
void CFolderCtrl::DeleteDir(const LPCTSTR szPath)
{
	CCeFileFind ff;
    CString path = szPath;
    
    if(path.Right(1) != "\\")
        path += "\\";

    path += "*.*";

    BOOL res = ff.FindFile(path);

    while(res)
    {
        res = ff.FindNextFile();
        if (!ff.IsDots() && !ff.IsDirectory())
            DeleteFile(ff.GetFilePath());
        else if (ff.IsDirectory())
        {
            path = ff.GetFilePath();
            DeleteDir(path);
            RemoveDirectory(path);
        }
    }
    RemoveDirectory(szPath);
}

//-----------------------------------------------------------------------------
//!	\create directory
//! \param szPath - directory path
//-----------------------------------------------------------------------------
void CFolderCtrl::CreateDir(const char* Path)
{
	CString str(Path);
	CreateDir(str);
}
//-----------------------------------------------------------------------------
//!	\create directory
//! \param szPath - directory path
//-----------------------------------------------------------------------------
void CFolderCtrl::CreateDir(const LPCTSTR Path)
{
		 CString strPath(Path);
		 TCHAR DirName[256];
		 TCHAR* p = strPath.GetBuffer(strPath.GetLength());
		 TCHAR* q = DirName; 
		 while(*p)
		 {
		   if (('\\' == *p) || ('/' == *p))
		   {
			 if (':' != *(p-1))
			 {
				CreateDirectory(DirName, NULL);
			 }
		   }
		   *q++ = *p++;
		   *q = '\0';
		 }
		 CreateDirectory(DirName, NULL);
		 strPath.ReleaseBuffer();
}
