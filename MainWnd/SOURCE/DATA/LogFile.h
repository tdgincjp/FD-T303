#include "..\\defs\\constant.h"
#include "..\\defs\\struct.h"

struct LogRecord
{
	char LogDateTime[SZ_DATETIME-1];
	char PinpadSerial[SZ_INFO_SN_NUMBER+1];
	char TermID[SZ_DATAWIRE_TID+1];
	char RecordTypeCode[SZ_2CHAR+1];
	char EventType[SZ_2CHAR+1];

	char outbuf[60];
	int len;

	LogRecord()
	{
		Init();
		CTimeUtil time;
		time.GetSystemDateTimeLong((char *)&LogDateTime);
		len = 0;
	}

	LogRecord(int iRecordType, int iEventType)
	{
		Init();
		CTimeUtil time;
		time.GetSystemDateTimeLong((char *)&LogDateTime);
		len = 0;
		sprintf(RecordTypeCode, "%02d", iRecordType);
		sprintf(EventType,		"%02d", iEventType);
	}

	void Init()
	{
		memset(LogDateTime,		'\0', SZ_DATETIME-1);
		memset(PinpadSerial,	'\0', SZ_INFO_SN_NUMBER+1);
		memset(TermID,			'\0', SZ_DATAWIRE_TID+1);
		memset(RecordTypeCode,	'\0', SZ_CHAR+1);
		memset(EventType,		'\0', SZ_CHAR+1);
	}

	char * ToString()
	{
		sprintf(outbuf, "%s,%s,%s,%s,%s\n", LogDateTime, PinpadSerial, TermID, RecordTypeCode, EventType);
		len = strlen(outbuf);
		return outbuf;
	}
}; 


class CLogFile 
{
	static LPCTSTR m_lpszFileName;
	static LPCTSTR m_lpszOldFile;
	static LPCTSTR m_lpszUsbLocation;
	static LPCTSTR m_lpszOldUsbFile;
	static LPCTSTR m_lpszFileDebug;				//JC Nov 11/15
	static LPCTSTR m_lpszOldDebug;				//JC Nov 11/15
	static LPCTSTR m_lpszUsbDebug;				//JC Nov 11/15
	static LPCTSTR m_lpszOldUsbDebug;			//JC Nov 11/15
public:
	CLogFile();
	~CLogFile();


	static BOOL CLogFile::WriteRecord(LogRecord *rec);
	static CStringArray * CLogFile::CopyToUsb();
};