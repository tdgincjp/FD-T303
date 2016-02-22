#include "stdafx.h"
#include <assert.h>
#include "RecordFile.h"
#include "..\\Utils\\FolderCtrl.h"
#include "..\\Utils\\CeFileFind.h"

#pragma warning(disable: 4996)

/******************************************************************************/
CRecordFile::CRecordFile()
{
	m_iMaxRecord = 0;
	m_iRecordLength = 0;
}

/******************************************************************************/
CRecordFile::~CRecordFile()
{
	Release();
}

//----------------------------------------------------------------------------
//!	\brief	Create Record file function
//!	\param	FileName - Record file Name
//!	\param	recordLength - Record data length
//!	\param	maxNumberOfRecords - max Number Of Records
//! RETURNS:       TRUE    = process ok.                               
//!                FALSE    = process failed.
//----------------------------------------------------------------------------
BOOL CRecordFile::Create(const char* FileName, USHORT recordLength, USHORT maxNumberOfRecords)
{
	Delete(FileName);
	m_FileName = CString(FileName);
	m_iMaxRecord = maxNumberOfRecords;
	m_iRecordLength = recordLength + sizeof(USHORT);
	m_iCurRecord = 0;

	int iTemp = m_FileName.ReverseFind('\\');
	if (iTemp != -1)
		CFolderCtrl::CreateDir(m_FileName.Mid(0,iTemp));

	CFileException ex;
	BOOL bflag = CFile::Open(m_FileName, 
													 CFile::modeReadWrite|CFile::shareDenyNone|CFile::modeCreate, 
													 &ex);

	if (bflag)
	{
		AP_FILE_HEADER fileHeader;
		fileHeader.recordLength = m_iRecordLength;
		fileHeader.maxNumberOfRecords = maxNumberOfRecords;
		fileHeader.numberOfActiveRecords = 0;
		fileHeader.numberOfEmptyRecords = 0;
		fileHeader.recordListSize = maxNumberOfRecords * sizeof(short);
		fileHeader.recordListPos = sizeof(AP_FILE_HEADER);
		fileHeader.dataPos = fileHeader.recordListPos + fileHeader.recordListSize;

		Write((void *) &fileHeader, SZ_AP_FILE_HEADER);

		USHORT* list = (USHORT *) malloc(fileHeader.recordListSize);
		if(list == NULL)
			return FALSE;
		for (int n = 0; n < fileHeader.maxNumberOfRecords; n++)
			list[n] = n;

		Write((void *) list, fileHeader.recordListSize);
		free(list);
	}
	return bflag;
}

//----------------------------------------------------------------------------
//!	\brief	delete Record file function
//! RETURNS:       TRUE    = process ok.                               
//!                FALSE    = process failed.
//----------------------------------------------------------------------------
BOOL CRecordFile::Delete()
{
	Release();

	CCeFileFind temp;
	if (!temp.FindFile(m_FileName))
		return true;

	Sleep(100);
	try
	{
		CFile::Remove(m_FileName);
	}
	catch (CFileException* pEx)
	{
	    pEx->Delete();
		return false;
	}
	return true;
}

//----------------------------------------------------------------------------
//!	\brief	delete Record file function
//!	\param	FileName - Record file Name
//! RETURNS:       TRUE    = process ok.                               
//!                FALSE    = process failed.
//----------------------------------------------------------------------------
BOOL CRecordFile::Delete(const char* pFileName)
{
	CString str(pFileName);

	if (str == m_FileName)
		Release();

	CCeFileFind temp;
	if (!temp.FindFile(str))
		return true;

	Sleep(100);
	try
	{
		CFile::Remove(str);
	}
	catch (CFileException* pEx)
	{
		pEx->Delete();
		return false;
	}
	return true;
}
//----------------------------------------------------------------------------
//!	\brief	rename Record file function
//!	\param	pFileName - Record file Name
//!	\param	NewName - Record file new Name
//! RETURNS:       TRUE    = process ok.                               
//!                FALSE    = process failed.
//----------------------------------------------------------------------------
BOOL CRecordFile::Rename(const char* pFileName, CString NewName)
{
	CString str(pFileName);

	if (str == m_FileName)
		Release();

	CCeFileFind temp;
	if (!temp.FindFile(str))
		return true;

	try
	{
		CFile::Rename(str,NewName);
	}
	catch (CFileException* pEx)
	{
	  pEx->Delete();
		return false;
	}
	return true;
}
//----------------------------------------------------------------------------
//!	\brief	close Record file function
//----------------------------------------------------------------------------
void CRecordFile::Release()
{
	if (m_hFile == CFile::hFileNull)
		return;
	CFile::Close();
	m_hFile = CFile::hFileNull;
}

#define MAX_NUMBER_RECORDS    550
#define SZ_FMGFILE_BUFFER (2*MAX_NUMBER_RECORDS)

/******************************************************************************/
/* Given the record index, returns the location of the record in the file     */
/* (offset from the beginning of the file).                                   */
/******************************************************************************/
ULONG CRecordFile::GetRecordPos(AP_FILE_HANDLER *fh, USHORT index)
{
	return ((fh->recordList[index] & DB_RECORD_INDEX_MASK) * fh->finfo.recordLength +
																					fh->finfo.dataPos);
}

/******************************************************************************/
/* Read the current record (pointed by fh->currentIndex) and store it in      */
/* fh->currentRecord.                                                         */
/******************************************************************************/
void CRecordFile::ReadCurrentRecord(AP_FILE_HANDLER *fh)
{
	ULONG len, pos;

	if (fh->currentIndex < fh->finfo.numberOfActiveRecords)
	{
		pos = GetRecordPos(fh, fh->currentIndex);
		Seek(pos,CFile::begin);
		len = fh->finfo.recordLength;
		len = Read(fh->currentRecord, len);
		if (len != fh->finfo.recordLength)
			return;
//			assert(FALSE);
	}
}

//----------------------------------------------------------------------------
//!	\brief	get current index function
//!	\param	fh - AP_FILE_HANDLER point
//!	\param	n - point to current index
//----------------------------------------------------------------------------
void CRecordFile::CurrentIndex(AP_FILE_HANDLER *fh, USHORT *n)
{
	if (fh->recordList == NULL)
		return;
//		assert(FALSE);

	*n = fh->currentIndex;
}

//----------------------------------------------------------------------------
//!	\brief	open Record file function
//!	\param	pFileName - Record file Name
//!	\param	fh - point to AP_FILE_HANDLER
//! RETURNS:       TRUE    = process ok.                               
//!                FALSE    = process failed.
//----------------------------------------------------------------------------
BOOL CRecordFile::Open(const char* FileName, AP_FILE_HANDLER *fh)
{
	Release();
	m_FileName = CString(FileName);

	m_iCurRecord = 0;

	CFileException ex;
	BOOL bflag = CFile::Open(m_FileName, CFile::modeReadWrite |
																			 CFile::shareDenyNone, &ex);
	if (bflag)
	{ 
		strcpy(fh->name, (char*)FileName);
		fh->fid = (ULONG)m_hFile;

		SeekToBegin();

		UINT len = SZ_AP_FILE_HEADER;
		Read((void*) &(fh->finfo), len);
			
		fh->recordList = m_RecordList;

		if (fh->recordList == NULL)
			return FALSE;
		len = fh->finfo.recordListSize;
		len = Read((void*) fh->recordList, len);
		if (len != fh->finfo.recordListSize)
			return FALSE;

		fh->currentRecord =  m_CurRecord;

		if (fh->currentRecord == NULL)
			return FALSE;
		memset(fh->currentRecord, 0, fh->finfo.recordLength);

		fh->currentIndex = 0;
		ReadCurrentRecord(fh);
	}
	return bflag;
}

//----------------------------------------------------------------------------
//!	\brief	close Record file function
//!	\param	fh - point to AP_FILE_HANDLER
//----------------------------------------------------------------------------
void CRecordFile::CloseFile(AP_FILE_HANDLER *fh)
{
	Release();
}

//----------------------------------------------------------------------------
//!	\brief	add data to Record file function
//!	\param	fh - point to AP_FILE_HANDLER
//!	\param	record - point to record data
//!	\param	len - record data length
//----------------------------------------------------------------------------
void CRecordFile::AddRecord(AP_FILE_HANDLER *fh, BYTE *record, USHORT len)
{
	ULONG pos;
	if (fh->recordList == NULL)
		return;

	// If this is empty record, re-use those location first.
	if (fh->finfo.numberOfEmptyRecords > 0)
	{
		pos = GetRecordPos(fh, fh->finfo.numberOfActiveRecords);
		Seek(pos,CFile::begin);

		fh->finfo.numberOfEmptyRecords--;
	}
	// else if the file is full, overwrite the 1st record.
	else if (fh->finfo.numberOfActiveRecords == fh->finfo.maxNumberOfRecords)
	{
		USHORT n, m;
		fh->finfo.numberOfActiveRecords--;
		m = fh->recordList[0];
		for (n = 0; n < fh->finfo.numberOfActiveRecords; n++)
		{	
			fh->recordList[n] = fh->recordList[n + 1];
		}	
		fh->recordList[n] = m;
		pos = GetRecordPos(fh, n);
		Seek(pos,CFile::begin);
	}
	// otherwise, expand the file and write to the end of the file is full.
	else
	{
		SeekToEnd();
	}
	memset(fh->currentRecord,0,fh->finfo.recordLength);
	memcpy(fh->currentRecord,(void*)&len,sizeof(USHORT));
	memcpy(fh->currentRecord+sizeof(USHORT),(void*)record,len);
	Write((void*)fh->currentRecord,fh->finfo.recordLength);

	fh->currentIndex = fh->finfo.numberOfActiveRecords++;
	SeekToBegin();

	Write((void*) &fh->finfo, SZ_AP_FILE_HEADER);
}

//----------------------------------------------------------------------------
//!	\brief	add data to Record file function
//!	\param	fh - point to AP_FILE_HANDLER
//!	\param	record - point to record data
//!	\param	len - record data length
//!	\param	recordFlag - record flag
//----------------------------------------------------------------------------
void CRecordFile::ReplaceRecord(AP_FILE_HANDLER *fh, BYTE *record, USHORT len, USHORT recordFlag)
{
	ULONG pos;

	if (fh->recordList == NULL)
		return;//assert(FALSE);

	if (fh->currentIndex >= fh->finfo.numberOfActiveRecords)
		return;//assert(FALSE);

	pos = GetRecordPos(fh, fh->currentIndex);
	Seek(pos, CFile::begin);

	memset(fh->currentRecord,0,fh->finfo.recordLength);
	memcpy(fh->currentRecord,(void*)&len,sizeof(USHORT));
	memcpy(fh->currentRecord+sizeof(USHORT),(void*)record,len);
	Write((void*)fh->currentRecord,fh->finfo.recordLength);

	fh->recordList[fh->currentIndex] &= DB_RECORD_INDEX_MASK;
	fh->recordList[fh->currentIndex] += (recordFlag << DB_RECORD_FLAG_SHIFT);

	Seek(fh->finfo.recordListPos, CFile::begin);

	Write((void*) fh->recordList, fh->finfo.recordListSize);
}

//----------------------------------------------------------------------------
//!	\brief	read current data from Record file function
//!	\param	fh - point to AP_FILE_HANDLER
//!	\param	record - point to record data
//!	\param	len - record data length
//----------------------------------------------------------------------------
void CRecordFile::ReadRecord(AP_FILE_HANDLER *fh, BYTE *record, USHORT*  len)
{
	if (fh->currentIndex >= fh->finfo.numberOfActiveRecords)
		return;//assert(FALSE);

	memcpy(len,fh->currentRecord,sizeof(USHORT));
	memcpy(record, fh->currentRecord+sizeof(USHORT), *len);
}

//----------------------------------------------------------------------------
//!	\brief	read index data from Record file function
//!	\param	fh - point to AP_FILE_HANDLER
//!	\param	index - record data index
//!	\param	record - point to record data
//!	\param	len - record data length
//----------------------------------------------------------------------------
void CRecordFile::ReadRecord(AP_FILE_HANDLER *fh, USHORT index, BYTE *record, USHORT* len)
{
	fh->currentIndex = (USHORT)index;
	if (fh->currentIndex >= fh->finfo.numberOfActiveRecords)
		return;//assert(FALSE);

	ReadCurrentRecord(fh);
	memcpy(len,fh->currentRecord,sizeof(USHORT));
	memcpy(record, fh->currentRecord+sizeof(USHORT), *len);
}

//----------------------------------------------------------------------------
//!	\brief	delete current data from Record file function
//!	\param	fh - point to AP_FILE_HANDLER
//----------------------------------------------------------------------------
void CRecordFile::DeleteCurrentRecord(AP_FILE_HANDLER *fh)
{
	USHORT n, m;

	if (fh->recordList == NULL)
		return;//assert(FALSE);

	if (fh->currentIndex >= fh->finfo.numberOfActiveRecords)
		return;//assert(FALSE);

	// Move the index of the deleted record to the end of the indices for active records.
	m = fh->recordList[fh->currentIndex];
	for (n = fh->currentIndex; n < fh->finfo.numberOfActiveRecords +
															   fh->finfo.numberOfEmptyRecords - 1; n++)
	{
		fh->recordList[n] = fh->recordList[n + 1];
	}
	fh->recordList[n] = m;

	fh->finfo.numberOfActiveRecords--;
	fh->finfo.numberOfEmptyRecords++;
	if (fh->currentIndex == fh->finfo.numberOfActiveRecords && fh->currentIndex != 0)
		fh->currentIndex--;

	ReadCurrentRecord(fh);
	SeekToBegin();
	Write((void*) &fh->finfo, SZ_AP_FILE_HEADER);
	Write((void*) fh->recordList, fh->finfo.recordListSize);
	Sleep(10);
}

//----------------------------------------------------------------------------
//!	\brief	find data from Record file function
//!	\param	fh - point to AP_FILE_HANDLER
//!	\param	forward - 1: from begin, 0: from end
//!	\param	matchData - search data buffer point
//!	\param	off - offset position
//!	\param	len - search data lenght
//!	\param	flagCheckMask - Check Mask flag 
//!	\param	flagCheckValue - Check Value flag
//! RETURNS:       ">=0"    = process ok.                               
//!                "< 0"    = process failed.
//----------------------------------------------------------------------------
int CRecordFile::FindRecord(AP_FILE_HANDLER *fh, BOOL forward, BYTE *matchData,
					USHORT off, USHORT len, USHORT flagCheckMask, USHORT flagCheckValue)
{
	USHORT n, recordFlag;
	char *record;
	ULONG pos;

	if (fh->finfo.numberOfActiveRecords == 0)
		return (-1);

	n = fh->currentIndex;
	record = (char*)malloc(fh->finfo.recordLength);
	if (record == NULL)
		return (-1);

	memcpy(record, fh->currentRecord, fh->finfo.recordLength);

	while (1)
	{
		recordFlag = (fh->recordList[n] & DB_RECORD_FLAG_MASK)
																			>> DB_RECORD_FLAG_SHIFT;
		if (!(flagCheckMask & (flagCheckValue^recordFlag)))
		{
			if ((matchData == 0) || memcmp(record+sizeof(USHORT)+off, matchData, len) == 0)
			{
				fh->currentIndex = n;
				memcpy(fh->currentRecord, record, fh->finfo.recordLength);
				free(record);
				return n;
			}
		}
		
		if (forward)
		{
			if (n++ >= fh->finfo.numberOfActiveRecords - 1)
				break;
		}
		else
		{
			if (n-- == 0)
				break;
		}

		pos = GetRecordPos(fh, n);

		Seek(pos,CFile::begin);

		ULONG length = fh->finfo.recordLength;
		length = Read(record, length);
		if (length != fh->finfo.recordLength)
		{
			free(record);
			return (-1);
		}
	}
	free(record);
	return (-1);
}
	
//----------------------------------------------------------------------------
//!	\brief	find data from Record file function
//!	\param	fh - point to AP_FILE_HANDLER
//!	\param	matchData - search data buffer point
//!	\param	off - offset position
//!	\param	len - search data lenght
//!	\param	index - find resault > 1, this is found record index
//! RETURNS:       ">=0"    = process ok.                               
//!                "< 0"    = process failed.
//----------------------------------------------------------------------------
int  CRecordFile::FindRecord(AP_FILE_HANDLER *fh, BYTE *matchData,USHORT off, USHORT len,USHORT index)
{
	USHORT n;
	char *record;
	ULONG pos;

	int iCount = 0;

	if (fh->finfo.numberOfActiveRecords == 0)
		return (-1);

	n = fh->currentIndex;
	record = (char*)malloc(fh->finfo.recordLength);
	if (record == NULL)
		return (-1);

	memcpy(record, fh->currentRecord, fh->finfo.recordLength);

	while (1)
	{
		if ((matchData == 0) || memcmp(record+sizeof(USHORT)+off, matchData, len) == 0)
		{
			iCount++;
			if ( iCount == index)
			{
				fh->currentIndex = n;
				memcpy(fh->currentRecord, record, fh->finfo.recordLength);
				free(record);
				return n;
			}
		}
		
		if (n++ >= fh->finfo.numberOfActiveRecords - 1)
			break;

		pos = GetRecordPos(fh, n);

		Seek(pos,CFile::begin);

		ULONG length = fh->finfo.recordLength;
		length = Read(record, length);
		if (length != fh->finfo.recordLength)
		{
			free(record);
			return (-1);
		}
	}
	free(record);
	return (-1);
}

//----------------------------------------------------------------------------
//!	\brief	find record count number from Record file function
//!	\param	fh - point to AP_FILE_HANDLER
//!	\param	forward - 1: from begin, 0: from end
//!	\param	matchData - search data buffer point
//!	\param	off - offset position
//!	\param	len - search data lenght
//!	\param	flagCheckMask - Check Mask flag 
//!	\param	flagCheckValue - Check Value flag
//! RETURNS:       ">=0"    = process ok.                               
//!                "< 0"    = process failed.
//----------------------------------------------------------------------------
int CRecordFile::FindRecordCount(AP_FILE_HANDLER *fh, BOOL forward, BYTE *matchData,
					USHORT off, USHORT len, USHORT flagCheckMask, USHORT flagCheckValue)
{
	int iCount = 0;
	USHORT n, recordFlag;
	char *record;
	ULONG pos;

	if (fh->finfo.numberOfActiveRecords == 0)
		return 0;

	n = fh->currentIndex;
	record = (char*)malloc(fh->finfo.recordLength);
	if (record == NULL)
		return 0;

	memcpy(record, fh->currentRecord, fh->finfo.recordLength);

	while (1)
	{
		recordFlag = (fh->recordList[n] & DB_RECORD_FLAG_MASK)
																			>> DB_RECORD_FLAG_SHIFT;
		if (!(flagCheckMask & (flagCheckValue^recordFlag)))
		{
			if ((matchData == 0) || memcmp(record+sizeof(USHORT)+off, matchData, len) == 0)
			{
//				fh->currentIndex = n;
//				memcpy(fh->currentRecord, record, fh->finfo.recordLength);
				iCount++;
//				free(record);
//				return n;
			}
		}
		
		if (forward)
		{
			if (n++ >= fh->finfo.numberOfActiveRecords - 1)
				break;
		}
		else
		{
			if (n-- == 0)
				break;
		}

		pos = GetRecordPos(fh, n);

		Seek(pos,CFile::begin);

		ULONG length = fh->finfo.recordLength;
		length = Read(record, length);
		if (length != fh->finfo.recordLength)
			break;//assert(FALSE);
	}
	free(record);
	return iCount;
}

//----------------------------------------------------------------------------
//!	\brief	get first record from Record file function
//!	\param	fh - point to AP_FILE_HANDLER
//! RETURNS:       TRUE    = process ok.                               
//!                FALSE    = process failed.
//----------------------------------------------------------------------------
BOOL CRecordFile::FirstRecord(AP_FILE_HANDLER *fh)
{
	if (fh->recordList == NULL)
		return FALSE;//assert(FALSE);

	if (fh->finfo.numberOfActiveRecords)
	{
		if (fh->currentIndex != 0)
		{
			fh->currentIndex = 0;
			ReadCurrentRecord(fh);
		}
		return TRUE;
	}
	else
		return FALSE;
}

//----------------------------------------------------------------------------
//!	\brief	get last record from Record file function
//!	\param	fh - point to AP_FILE_HANDLER
//! RETURNS:       TRUE    = process ok.                               
//!                FALSE    = process failed.
//----------------------------------------------------------------------------
BOOL CRecordFile::LastRecord(AP_FILE_HANDLER *fh)
{
	if (fh->recordList == NULL)
		return FALSE;//assert(FALSE);

	if (fh->finfo.numberOfActiveRecords)
	{
		if (fh->currentIndex != fh->finfo.numberOfActiveRecords - 1)
		{
			fh->currentIndex = fh->finfo.numberOfActiveRecords - 1;
			ReadCurrentRecord(fh);
		}
		return TRUE;
	}
	else
		return FALSE;
}

//----------------------------------------------------------------------------
//!	\brief	get next record from Record file function
//!	\param	fh - point to AP_FILE_HANDLER
//! RETURNS:       TRUE    = process ok.                               
//!                FALSE    = process failed.
//----------------------------------------------------------------------------
BOOL CRecordFile::NextRecord(AP_FILE_HANDLER *fh)
{
	if (fh->recordList == NULL)
		return FALSE;//assert(FALSE);

	if (fh->currentIndex < fh->finfo.numberOfActiveRecords - 1)
	{
		fh->currentIndex++;
		ReadCurrentRecord(fh);
		return TRUE;
	}
	else
		return FALSE;
}

//----------------------------------------------------------------------------
//!	\brief	get previous record from Record file function
//!	\param	fh - point to AP_FILE_HANDLER
//! RETURNS:       TRUE    = process ok.                               
//!                FALSE    = process failed.
//----------------------------------------------------------------------------
BOOL CRecordFile::PreviousRecord(AP_FILE_HANDLER *fh)
{
	if (fh->recordList == NULL)
		return FALSE;//assert(FALSE);

	if (fh->currentIndex && fh->finfo.numberOfActiveRecords)
	{
		fh->currentIndex--;
		ReadCurrentRecord(fh);
		return TRUE;
	}
	else
		return FALSE;
}

//----------------------------------------------------------------------------
//!	\brief	get number record from Record file function
//!	\param	fh - point to AP_FILE_HANDLER
//!	\param	nbrRecords - point to record number
//----------------------------------------------------------------------------
void CRecordFile::GetNumberOfRecords(AP_FILE_HANDLER *fh, USHORT *nbrRecords)
{
	if (fh->recordList == NULL)
		return ;//assert(FALSE);

	*nbrRecords = fh->finfo.numberOfActiveRecords;
}

//----------------------------------------------------------------------------
//!	\brief	get remain number record from Record file function
//!	\param	fh - point to AP_FILE_HANDLER
//!	\param	nbrRecords - point to record number
//----------------------------------------------------------------------------
void CRecordFile::getRemainNumberOfRecords(AP_FILE_HANDLER *fh, USHORT *nbrRecords)
{
	if (fh->recordList == NULL)
		return ;//assert(FALSE);

	*nbrRecords = fh->finfo.numberOfEmptyRecords;
}

//----------------------------------------------------------------------------
//!	\brief	get Max number record from Record file function
//!	\param	fh - point to AP_FILE_HANDLER
//!	\param	nbrRecords - point to record number
//----------------------------------------------------------------------------
void CRecordFile::GetMaxNumberOfRecords(AP_FILE_HANDLER *fh, USHORT *nbrRecords)
{
	if (fh->recordList == NULL)
		return ;//assert(FALSE);

	*nbrRecords = fh->finfo.maxNumberOfRecords;
}

