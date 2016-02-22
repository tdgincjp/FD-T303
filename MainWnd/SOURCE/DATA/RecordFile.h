#pragma once

#define     AP_FILE_NAME_SZ               128
#define  DB_RECORD_INDEX_MASK    0x07ff
#define  DB_RECORD_FLAG_MASK     0xf800
#define  DB_RECORD_FLAG_SHIFT    11

#define  DB_RECORD_MAX_LENGTH    2048
#define  DB_RECORD_MAX_NUMBER    500

#ifndef USHORT
#define USHORT unsigned short
#endif

#ifndef ULONG
#define ULONG unsigned long
#endif
typedef struct
{
  USHORT    recordLength;
  USHORT    maxNumberOfRecords;
  USHORT    numberOfActiveRecords;
  USHORT    numberOfEmptyRecords;
  USHORT    recordListSize;
  ULONG     recordListPos;
  ULONG     dataPos;
} AP_FILE_HEADER;

#define SZ_AP_FILE_HEADER   sizeof(AP_FILE_HEADER)

typedef struct
{
   ULONG             fid;
   char              name[AP_FILE_NAME_SZ+1];
   AP_FILE_HEADER    finfo;
   USHORT            currentIndex;
   char             *currentRecord; // point to a buffer to store current record
                                    //   must larger than record size
   USHORT           *recordList;    // point to a buufer to stroe record list
                                    //   must lager than 2*maxNumberOfRecords
} AP_FILE_HANDLER;

class CRecordFile : CFile
{

private:
	CString m_FileName;
	int m_iMaxRecord;
	USHORT m_iRecordLength;
	int m_iCurRecord;
	void Release();

	char m_CurRecord[DB_RECORD_MAX_LENGTH+1];
	USHORT m_RecordList[DB_RECORD_MAX_NUMBER*2];

public:
	CRecordFile();
	~CRecordFile();

	BOOL Create(const char* FileName, USHORT recordLength=2048, USHORT maxNumberOfRecords=500 );
	BOOL Delete(const char* FileName);
	BOOL Delete();
	BOOL Rename(const char* FileName, CString NewName);

	ULONG GetRecordPos(AP_FILE_HANDLER *fh, USHORT index);
	void ReadCurrentRecord(AP_FILE_HANDLER *fh);
	void CurrentIndex(AP_FILE_HANDLER *fh, USHORT *n);
	BOOL Open(const char* FileName, AP_FILE_HANDLER *fh);

	void CloseFile(AP_FILE_HANDLER *fh);
	void DeleteCurrentRecord(AP_FILE_HANDLER *fh);
	int  FindRecord(AP_FILE_HANDLER *fh, BOOL forward, BYTE *matchData,
					USHORT off, USHORT len, USHORT flagCheckMask, USHORT flagCheckValue);
	int  FindRecord(AP_FILE_HANDLER *fh, BYTE *matchData,USHORT off, USHORT len,USHORT index);
	int  FindRecordCount(AP_FILE_HANDLER *fh, BOOL forward, BYTE *matchData,
					USHORT off, USHORT len, USHORT flagCheckMask, USHORT flagCheckValue);
	BOOL FirstRecord(AP_FILE_HANDLER *fh);
	BOOL LastRecord(AP_FILE_HANDLER *fh);
	BOOL NextRecord(AP_FILE_HANDLER *fh);
	BOOL PreviousRecord(AP_FILE_HANDLER *fh);
	void GetNumberOfRecords(AP_FILE_HANDLER *fh, USHORT *nbrRecords);
	void getRemainNumberOfRecords(AP_FILE_HANDLER *fh, USHORT *nbrRecords);
	void GetMaxNumberOfRecords(AP_FILE_HANDLER *fh, USHORT *nbrRecords);

	void AddRecord(AP_FILE_HANDLER *fh, BYTE *record,USHORT len );
	void ReplaceRecord(AP_FILE_HANDLER *fh, BYTE *record, USHORT len,USHORT recordFlag=1);
	void ReadRecord(AP_FILE_HANDLER *fh, BYTE *record, USHORT* len);
	void ReadRecord(AP_FILE_HANDLER *fh, USHORT index, BYTE *record, USHORT* len);
};


