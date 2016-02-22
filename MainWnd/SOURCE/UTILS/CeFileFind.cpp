// CeFileFind.cpp : implementation file
//

#include "stdafx.h"
#include "CeFileFind.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


#define DELETE_POINTER(ptr)		if ( ptr != NULL )	\
								{					\
									delete ptr;		\
									ptr = NULL;		\
								}

#define DIR_SEPERATOR		'\\'

/////////////////////////////////////////////////////////////////////////////
// CCeFileFind

CCeFileFind::CCeFileFind()
			:m_hFileHandle(NULL), // initialize to NULL
			m_pfiledata(NULL),m_pNextdata(NULL)
{
}

CCeFileFind::~CCeFileFind()
{
	Close();
}

//-----------------------------------------------------------------------------
//!	\find file
//!	\param	pstrName - file name 
//-----------------------------------------------------------------------------
BOOL CCeFileFind::FindFile( char* pstrName)
{
	CString str(pstrName);
	return FindFile(str);
}

// Operations
//-----------------------------------------------------------------------------
//!	\find file
//!	\param	pstrName - file name 
//-----------------------------------------------------------------------------
BOOL CCeFileFind::FindFile(LPCTSTR pstrName)
{

	Close();

	// if NULL , wild card search 
	if ( NULL == pstrName )
	{
		m_csRoot = DIR_SEPERATOR;
		pstrName = _T("\\*.*");
		
	}
	else
	{

		m_csRoot = pstrName;
		int nPos = m_csRoot.ReverseFind( '\\' );
		if ( nPos == 0 )
			m_csRoot = '\\';
		else
			m_csRoot = m_csRoot.Left( nPos );
	
	}

	m_pNextdata = new WIN32_FIND_DATA;
	// search for file
	m_hFileHandle = FindFirstFile( pstrName, m_pNextdata );
	
	if ( m_hFileHandle == INVALID_HANDLE_VALUE )
	{
		Close();
		return FALSE;
	}

	// file was found
	return TRUE;
}

//-----------------------------------------------------------------------------
//!	\find next file
//-----------------------------------------------------------------------------
BOOL CCeFileFind::FindNextFile()
{
	ASSERT(m_hFileHandle != NULL);

	if (m_hFileHandle == NULL)
		return FALSE;

	if (m_pfiledata == NULL)
		m_pfiledata = new WIN32_FIND_DATA;

	AssertDoneNext();

	LPWIN32_FIND_DATA pTemp = m_pfiledata;
	m_pfiledata = m_pNextdata;
	m_pNextdata = pTemp;

	return ::FindNextFile(m_hFileHandle, m_pNextdata);
}

//-----------------------------------------------------------------------------
//!	\close find file class
//-----------------------------------------------------------------------------
void CCeFileFind::Close()
{
	DELETE_POINTER( m_pfiledata );
	DELETE_POINTER( m_pNextdata );

	if ( m_hFileHandle!= NULL && m_hFileHandle != INVALID_HANDLE_VALUE )
	{
		::FindClose( m_hFileHandle );
		m_hFileHandle = NULL;
	}
}

//-----------------------------------------------------------------------------
//!	\check match mask
//!	\param	dwMask - mask value 
//-----------------------------------------------------------------------------
BOOL CCeFileFind::MatchesMask(DWORD dwMask) const
{
	ASSERT(m_hFileHandle != NULL);
	AssertDoneNext();

	if ( m_pfiledata != NULL)
		return (!!(m_pfiledata->dwFileAttributes & dwMask) );
	else
		return FALSE;
}


//-----------------------------------------------------------------------------
//!	\get root file name
//-----------------------------------------------------------------------------
CString CCeFileFind::GetRoot() const
{
	ASSERT(m_hFileHandle != NULL);
	AssertDoneNext();

	if (m_pfiledata != NULL)
		return m_csRoot;
	else
		return L"";

}

//-----------------------------------------------------------------------------
//!	\Get Last Access Time
//!	\param	pTimeStamp - time stamp 
//-----------------------------------------------------------------------------
BOOL CCeFileFind::GetLastAccessTime(FILETIME* pTimeStamp) const
{
	ASSERT(m_hFileHandle != NULL);
	ASSERT(pTimeStamp != NULL);
	AssertDoneNext();

	if (m_pfiledata != NULL && pTimeStamp != NULL)
	{
		*pTimeStamp = m_pfiledata -> ftLastAccessTime;
		return TRUE;
	}
	else
		return FALSE;
}

//-----------------------------------------------------------------------------
//!	\Get Last write Time
//!	\param	pTimeStamp - time stamp 
//-----------------------------------------------------------------------------
BOOL CCeFileFind::GetLastWriteTime(FILETIME* pTimeStamp) const
{
	ASSERT(m_hFileHandle != NULL);
	ASSERT(pTimeStamp != NULL);
	AssertDoneNext();

	if (m_pfiledata != NULL && pTimeStamp != NULL)
	{
		*pTimeStamp = m_pfiledata -> ftLastWriteTime;
		return TRUE;
	}
	else
		return FALSE;
}

//-----------------------------------------------------------------------------
//!	\Get creation Time
//!	\param	pTimeStamp - time stamp 
//-----------------------------------------------------------------------------
BOOL CCeFileFind::GetCreationTime(FILETIME* pTimeStamp) const
{
	ASSERT(m_hFileHandle != NULL);
	AssertDoneNext();

	if (m_pfiledata != NULL && pTimeStamp != NULL)
	{
		*pTimeStamp = m_pfiledata -> ftCreationTime;
		return TRUE;
	}
	else
		return FALSE;
}

//-----------------------------------------------------------------------------
//!	\Get Last Access Time
//!	\param	refTime - time stamp 
//-----------------------------------------------------------------------------
BOOL CCeFileFind::GetLastAccessTime(CTime& refTime) const
{
	ASSERT(m_hFileHandle != NULL);
	AssertDoneNext();

	if (m_pfiledata != NULL)
	{
		refTime = CTime( m_pfiledata -> ftLastAccessTime );
		return TRUE;
	}
	else
		return FALSE;
}

//-----------------------------------------------------------------------------
//!	\Get Last write Time
//!	\param	refTime - time stamp 
//-----------------------------------------------------------------------------
BOOL CCeFileFind::GetLastWriteTime(CTime& refTime) const
{
	ASSERT(m_hFileHandle != NULL);
	AssertDoneNext();

	if (m_pfiledata != NULL)
	{
		refTime = CTime( m_pfiledata -> ftLastWriteTime );
		return TRUE;
	}
	else
		return FALSE;
}

//-----------------------------------------------------------------------------
//!	\Get creation Time
//!	\param	refTime - time stamp 
//-----------------------------------------------------------------------------
BOOL CCeFileFind::GetCreationTime(CTime& refTime) const
{
	ASSERT(m_hFileHandle != NULL);
	AssertDoneNext();

	if (m_pfiledata != NULL)
	{
		refTime = CTime( m_pfiledata -> ftCreationTime );
		return TRUE;
	}
	else
		return FALSE;
}

//-----------------------------------------------------------------------------
//!	\check is dots file or not
//-----------------------------------------------------------------------------
BOOL CCeFileFind::IsDots() const
{
	ASSERT(m_hFileHandle != NULL);
	AssertDoneNext();

	BOOL bResult = FALSE;
	if (m_pfiledata != NULL && IsDirectory())
	{
		LPWIN32_FIND_DATA pFindData = m_pfiledata;
		if (pFindData->cFileName[0] == '.')
		{
			if (pFindData->cFileName[1] == '\0' ||
				(pFindData->cFileName[1] == '.' &&
				 pFindData->cFileName[2] == '\0'))
			{
				bResult = TRUE;
			}
		}
	}

	return bResult;
}

//-----------------------------------------------------------------------------
//!	\check file is archived
//-----------------------------------------------------------------------------
BOOL CCeFileFind::IsArchived( ) const
{
	ASSERT(m_hFileHandle != NULL);
	AssertDoneNext();

	if (m_pfiledata != NULL)
	{
		if ( m_pfiledata -> dwFileAttributes == FILE_ATTRIBUTE_ARCHIVE )
			return TRUE;
		else
			return FALSE;
	}
	return FALSE;

}

//-----------------------------------------------------------------------------
//!	\check file is compressed
//-----------------------------------------------------------------------------
BOOL CCeFileFind::IsCompressed() const
{
	ASSERT(m_hFileHandle != NULL);
	AssertDoneNext();

	if (m_pfiledata != NULL)
	{
		if ( m_pfiledata -> dwFileAttributes == FILE_ATTRIBUTE_COMPRESSED )
			return TRUE;
		else
			return FALSE;
	}
	return FALSE;
}

//-----------------------------------------------------------------------------
//!	\check file is directory
//-----------------------------------------------------------------------------
BOOL CCeFileFind::IsDirectory() const
{
	ASSERT(m_hFileHandle != NULL);
	AssertDoneNext();

	if (m_pfiledata != NULL)
	{
		if ( m_pfiledata -> dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY )
			return TRUE;
		else
			return FALSE;
	}
	return FALSE;
}

//-----------------------------------------------------------------------------
//!	\check file is hidden
//-----------------------------------------------------------------------------
BOOL CCeFileFind::IsHidden() const
{
	ASSERT(m_hFileHandle != NULL);
	AssertDoneNext();

	if (m_pfiledata != NULL)
	{
		if ( m_pfiledata -> dwFileAttributes & FILE_ATTRIBUTE_HIDDEN )
			return TRUE;
		else
			return FALSE;
	}
	return FALSE;
}


//-----------------------------------------------------------------------------
//!	\check file is normal
//-----------------------------------------------------------------------------
BOOL CCeFileFind::IsNormal() const
{
	ASSERT(m_hFileHandle != NULL);
	AssertDoneNext();

	if (m_pfiledata != NULL)
	{
		if ( m_pfiledata -> dwFileAttributes & FILE_ATTRIBUTE_NORMAL )
			return TRUE;
		else
			return FALSE;
	}
	return FALSE;
}

//-----------------------------------------------------------------------------
//!	\check file is read only
//-----------------------------------------------------------------------------
BOOL CCeFileFind::IsReadOnly() const
{
	ASSERT(m_hFileHandle != NULL);
	AssertDoneNext();

	if (m_pfiledata != NULL)
	{
		if ( m_pfiledata -> dwFileAttributes & FILE_ATTRIBUTE_READONLY )
			return TRUE;
		else
			return FALSE;
	}
	return FALSE;
}

//-----------------------------------------------------------------------------
//!	\check file is system file
//-----------------------------------------------------------------------------
BOOL CCeFileFind::IsSystem() const
{
	ASSERT(m_hFileHandle != NULL);
	AssertDoneNext();

	if (m_pfiledata != NULL)
	{
		if ( m_pfiledata -> dwFileAttributes & FILE_ATTRIBUTE_SYSTEM )
			return TRUE;
		else
			return FALSE;
	}
	return FALSE;
}


//-----------------------------------------------------------------------------
//!	\check file is temporary file
//-----------------------------------------------------------------------------
BOOL CCeFileFind::IsTemporary() const
{
	ASSERT(m_hFileHandle != NULL);
	AssertDoneNext();

	if (m_pfiledata != NULL)
	{
		if ( m_pfiledata -> dwFileAttributes & FILE_ATTRIBUTE_TEMPORARY )
			return TRUE;
		else
			return FALSE;
	}
	return FALSE;
}


//-----------------------------------------------------------------------------
//!	\get file path
//-----------------------------------------------------------------------------
CString CCeFileFind::GetFilePath() const
{
	ASSERT(m_hFileHandle != NULL);
	AssertDoneNext();

	CString csResult = m_csRoot;

	if (csResult[csResult.GetLength()-1] != DIR_SEPERATOR )
		csResult += DIR_SEPERATOR;
	csResult += GetFileName();
	return csResult;
}


//-----------------------------------------------------------------------------
//!	\get file name
//-----------------------------------------------------------------------------
CString CCeFileFind::GetFileName() const
{
	ASSERT(m_hFileHandle != NULL);
	AssertDoneNext();

	CString ret;

	if (m_pfiledata != NULL)
		ret = m_pfiledata->cFileName;
	return ret;
}

//-----------------------------------------------------------------------------
//!	\get file length
//-----------------------------------------------------------------------------
DWORD CCeFileFind::GetLength() const
{
	ASSERT(m_hFileHandle != NULL);
	AssertDoneNext();

	if (m_pfiledata != NULL)
		return m_pfiledata -> nFileSizeLow;
	else
		return 0;
}

//-----------------------------------------------------------------------------
//!	\check can do next file or not
//-----------------------------------------------------------------------------
void CCeFileFind::AssertDoneNext() const
{
	if (m_hFileHandle == NULL)
		ASSERT( m_pfiledata == NULL && m_pNextdata == NULL);
	else
		ASSERT( m_pfiledata != NULL && m_pNextdata != NULL);
}

