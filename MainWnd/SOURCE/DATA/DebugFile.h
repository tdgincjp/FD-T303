// DebugFile.h : header file
//

#pragma once

// CDebugFile
class CDebugFile
{
// Construction
public:
	CDebugFile();	// standard constructor

public:

	static	void	DebugWrite(char *);						//JC Nov 9/15
	static  void	DebugLogHost(char *);					//JC Nov 13/15
	static	void	DebugSizeCheck();						//JC Nov 11/15
	static	char *	LogFileName();							//JC Nov 13/15
	static	void	CheckLogFileName();						//JC Nov 13/15
	static	void	CheckLogPCI(char *);					//JC Nov 13/15
private:

	
};
