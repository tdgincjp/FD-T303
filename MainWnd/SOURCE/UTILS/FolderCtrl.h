
// FolderCtrl.h : header file
//

#pragma once

// CFolderCtrl
class CFolderCtrl
{
// Construction
public:
	CFolderCtrl();	// standard constructor

public:
	static void DeleteDir(const LPCTSTR Path);
	static void DeleteDir(const char* Path);
	static void CreateDir(const LPCTSTR Path);
	static void CreateDir(const  char* Path);

};